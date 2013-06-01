/*
 * Copyright (C) 2013 Martin Minarik <minarik11@student.fiit.stuba.sk>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "wayland-server-protocol.h"
#include "compositor.h"
#include "evdev.h"
#include "evemu-impl.h"
#include "fakeston.h"

size_t hash_seek(void ** table, size_t cnt, size_t item_size, void *ptr, void *seek)
{
	uintptr_t *tb;
	size_t off = (uintptr_t)ptr ^ ((uintptr_t)ptr >> 16) ^ (uintptr_t) 0x752ece3898c734a9;
	size_t offx = cnt + off;

	do {
		tb = (uintptr_t *) ((char *) table + (++off % cnt) * item_size);
	} while ((*tb != (uintptr_t) seek) && (off != offx));

	return off == offx ? cnt : off % cnt;
}

void usage()
{
	fprintf(stderr, "fakeston_run ftestcase.txt \n\n ftestcase.txt - the test case file\n");
}


void try_free(char** x)
{
	if (*x != NULL){
		free(*x);
		*x = NULL;
	}
}

void try_replace_n(char** x, const char *y, size_t n)
{
	try_free(x);
	*x = malloc(n);
	if (*x)
		memcpy(*x, y, n);
}


static void read_prop(struct evemu_device *dev, FILE *fp)
{
	unsigned int mask[8];
	int i;
	while (fscanf(fp, "P: %02x %02x %02x %02x %02x %02x %02x %02x\n",
		      mask + 0, mask + 1, mask + 2, mask + 3,
		      mask + 4, mask + 5, mask + 6, mask + 7) > 0) {
		for (i = 0; i < 8; i++)
			dev->prop[dev->pbytes++] = mask[i];
	}
}

static void read_mask(struct evemu_device *dev, FILE *fp)
{
	unsigned int mask[8];
	int index, i;
	while (fscanf(fp, "B: %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
		      &index, mask + 0, mask + 1, mask + 2, mask + 3,
		      mask + 4, mask + 5, mask + 6, mask + 7) > 0) {
		for (i = 0; i < 8; i++)
			dev->mask[index][dev->mbytes[index]++] = mask[i];
	}
}

static void read_abs(struct evemu_device *dev, FILE *fp)
{
	struct input_absinfo abs;
	int index;
	while (fscanf(fp, "A: %02x %d %d %d %d\n", &index,
		      &abs.minimum, &abs.maximum, &abs.fuzz, &abs.flat) > 0)
		dev->abs[index] = abs;
}

int evemu_has_event(const struct evemu_device *dev, int type, int code);

void fakeston_evdev_dev_load_desc(struct fakeston_evdev_dev *device, FILE *fp)
{

	struct evemu_device dev;

	memset(&dev, 0, sizeof(dev));

	struct input_id id;

	unsigned bustype, vendor, product, version;
	int ret;
	char name[85];
	memset(name, 0, sizeof(name));
	ret = fscanf(fp, "N: %79[^\n]\n", name);
	if (ret <= 0)
		return;


	try_replace_n(&device->ioctl_EVIOCGNAME, name, strlen(name) + 1);



	ret = fscanf(fp, "I: %04x %04x %04x %04x\n",
		     &bustype, &vendor, &product, &version);
	id.bustype = bustype;
	id.vendor = vendor;
	id.product = product;
	id.version = version;

	try_replace_n(&device->ioctl_EVIOCGID, (char *) &id, sizeof(id));

	dev.pbytes = 0;
	read_prop(&dev, fp);
	device->pbytes = dev.pbytes;

	try_replace_n(&device->ioctl_EVIOCGPROP, (char *) dev.prop, dev.pbytes);

	read_mask(&dev, fp);
	int index;
	for (index = 0; index < 32; index++) {
		if (dev.mbytes[index] == 0)
			continue;

		char **chlievik = &(device->ioctl_EVIOCGBIT_EV_BITS[index]);
		char *src = (char *)&(dev.mask[index]);

		size_t bb = device->bitsbytes[index] = dev.mbytes[index];
		try_replace_n(chlievik, src, bb);

		src = *chlievik;

	}


	read_abs(&dev, fp);

	for (index = 0; index < 64; index++) {

		if (!evemu_has_event(&dev, EV_ABS, index)) {

			continue;
		}

		char **chlievik = &(device->ioctl_EVIOCGBIT_EV_ABS[index]);

		struct input_absinfo *aa = &dev.abs[index];

		try_replace_n(chlievik, (char *)aa, sizeof(struct input_absinfo));

		device->is_abs |= 1<<index;



	}


}

int evemu_read_event(FILE *fp, struct input_event *ev);

struct pload *fixed_p = NULL;

void fakeston_line_handler(void*data, char*tag, FILE *tcase)
{
	struct pload *p = ( struct pload *) data;
	struct fakeston_evdev_dev *d = (struct fakeston_evdev_dev *) p->d;
	struct fakeston_evdev_seat *s = (struct fakeston_evdev_seat *) p->s;
	struct fakeston_evdev_rev *r = (struct fakeston_evdev_rev *) p->r;
	struct fakeston_evdev_rev *z = (struct fakeston_evdev_rev *) p->z;
	size_t doff, soff, roff, zoff;
	size_t sitem_s = sizeof(struct fakeston_evdev_seat);
	size_t ditem_s = sizeof(struct fakeston_evdev_dev);
	size_t ritem_s = sizeof(struct fakeston_evdev_rev);
	if (0 == strcmp(tag, "seatfocus:")) {
		void *id;

		fscanf(tcase, "%p", &id);

		soff = hash_seek((void*)s, p->shtsz, sitem_s,  id, id);
		if (soff == p->shtsz) {
			return;
		}
		fixed_p = p;

		evdev_notify_keyboard_focus(&s[soff].whatever, &p->devices_list);

		fixed_p = NULL;
	} else
	if (0 == strcmp(tag, "EcreateDEV:")) {
		void *id, *seatid;

		fscanf(tcase, "%p", &id);

		doff = hash_seek((void*)d, p->dhtsz, ditem_s,  id, id);
		if (doff == p->dhtsz) {
			return;
		}

		seatid = (void *) d[doff].seatid;

		soff = hash_seek((void*)s, p->shtsz, sitem_s,  seatid, seatid);
		if (soff == p->shtsz) {
			return;
		}

		int dev_fd = d[doff].fd;

		fixed_p = p;

		struct evdev_device *device = evdev_device_create(&s[soff].whatever, "<mock-dev-path>", dev_fd);

		fixed_p = NULL;

		if ((device == NULL) || (device == EVDEV_UNHANDLED_DEVICE)) {
			fprintf(stdout, "FAKESTON: ERR: Cannot create device %p. \n", id);
			return;
		}

		d[doff].device = device;
		d[doff].device->output = p->output;
		d[doff].device->abs.max_x = 1024;
		d[doff].device->abs.max_y = 768;
		d[doff].device->abs.min_x = 0;
		d[doff].device->abs.min_y = 0;
		d[doff].created = 1;

		wl_list_insert(&p->devices_list, &d[doff].device->link);


	} else if (0 == strcmp(tag, "EprepareDEV:")) {
		void *id, *seatid, *seatptr;

		fscanf(tcase, "%p %p", &id, &seatid);

		doff = hash_seek((void*)d, p->dhtsz, ditem_s,  id, id);
		if (doff != p->dhtsz) {
			return;
		}
		doff = hash_seek((void*)d, p->dhtsz, ditem_s,  id, 0);

		soff = hash_seek((void*)s, p->shtsz, sitem_s,  seatid, seatid);
		if (soff == p->shtsz) {
			soff = hash_seek((void*)s, p->shtsz, sitem_s,  seatid, 0);
			if (soff == p->shtsz)
				return;

			s[soff].id = (uintptr_t) seatid;
			s[soff].whatever.compositor = &p->comp;
			s[soff].whatever.keyboard = (void *) &p->k;

		}

		seatptr = &s[soff].whatever;

		int dev_fd = p->fd_seq++;

		roff = hash_seek((void*)r, p->dhtsz, ritem_s, (void*)(intptr_t)dev_fd, 0);
		zoff = hash_seek((void*)z, p->shtsz, ritem_s, seatptr, seatptr);
		if (zoff == p->shtsz) {
			zoff = hash_seek((void*)z, p->shtsz, ritem_s, seatptr, 0);
		}


		z[zoff].id = (uintptr_t)(intptr_t) &s[soff].whatever;
		z[zoff].off = soff;

		r[roff].id = (uintptr_t)(intptr_t) dev_fd;
		r[roff].off = doff;

		d[doff].id = (uintptr_t) id;
		d[doff].seatid = (uintptr_t) seatid;
		d[doff].init_serial = p->seq++;
		d[doff].device = NULL;
		d[doff].fd = dev_fd;

		d[doff].created = 0;

	} else if (0 == strcmp(tag, "EdestroyDEV:")) {
		void *id;
		fscanf(tcase, "%p", &id);

		doff = hash_seek((void *)d, p->dhtsz, ditem_s,  id, id);
		if (doff == p->dhtsz)
			return;

		if (d[doff].created) {
			fixed_p = p;
			evdev_device_destroy(d[doff].device);
			fixed_p = NULL;
			d[doff].created = 0;
		}

		d[doff].created = 0;

		d[doff].id = (uintptr_t) 0;
		if (d[doff].evt) {
			fclose(d[doff].evt);
		}

		try_free(&d[doff].ioctl_eviocgabs_abs_x);
		try_free(&d[doff].ioctl_eviocgabs_abs_y);
		try_free(&d[doff].ioctl_eviocgabs_abs_mt_pos_x);
		try_free(&d[doff].ioctl_eviocgabs_abs_mt_pos_y);

		try_free(&d[doff].ioctl_EVIOCGNAME);
		try_free(&d[doff].ioctl_EVIOCGID);
		try_free(&d[doff].ioctl_EVIOCGPROP);
		try_free(&d[doff].ioctl_EVIOCGKEY);
		int slot;
		for (slot = 0; slot < 32; slot++)
			try_free(&(d[doff].ioctl_EVIOCGBIT_EV_BITS[slot]));
		try_free(&d[doff].ioctl_EVIOCGBIT_EV_KEY);
		try_free(&d[doff].ioctl_EVIOCGBIT_EV_REL);
		try_free(&d[doff].ioctl_EVIOCGBIT_EV_ABS_REAL);
		for (slot = 0; slot < 64; slot++)
			try_free(&(d[doff].ioctl_EVIOCGBIT_EV_ABS[slot]));

	} else if (0 == strcmp(tag, "IOCTLDUMP:")) {
		void *id;
		char type[128] = {0};
		char baf[1024] = {0};
		size_t siz;

		fscanf(tcase, "%p %127s %zu", &id, type, &siz);

		doff = hash_seek((void*)d, p->dhtsz, ditem_s,  id, id);
		if (doff == p->dhtsz) {
			return;
		}

		size_t i;
		for (i = 0; i < siz; i++) {
			unsigned int tmp;
			fscanf(tcase, "%02x", &tmp);
			baf[i] = tmp;
		}

		if (0 == strcmp(type, "key_bits")) {
			try_replace_n(&d[doff].ioctl_EVIOCGBIT_EV_KEY, baf, siz);
			d[doff].keybytes = siz;

		}

		if (0 == strcmp(type, "evdev_keys")) {
			try_replace_n(&d[doff].ioctl_EVIOCGKEY, baf, siz);
			d[doff].EVIOCGKEYsize = siz;

		}


		if ((0 == strcmp(type, "rel_bits"))) {
			try_replace_n(&d[doff].ioctl_EVIOCGBIT_EV_REL, baf, siz);
			d[doff].relbits = siz;
		}

		if ((0 == strcmp(type, "eviocgabs_abs_x"))) {
			try_replace_n(&d[doff].ioctl_eviocgabs_abs_x, baf, siz);
			d[doff].size_abs_x = siz;
		}

		if ((0 == strcmp(type, "eviocgabs_abs_y"))) {
			try_replace_n(&d[doff].ioctl_eviocgabs_abs_y, baf, siz);
			d[doff].size_abs_y = siz;
		}

		if ((0 == strcmp(type, "eviocgabs_abs_mt_pos_x"))) {
			try_replace_n(&d[doff].ioctl_eviocgabs_abs_mt_pos_x, baf, siz);
			d[doff].size_abs_mt_pos_x = siz;
		}

		if ((0 == strcmp(type, "eviocgabs_abs_mt_pos_y"))) {
			try_replace_n(&d[doff].ioctl_eviocgabs_abs_mt_pos_y, baf, siz);
			d[doff].size_abs_mt_pos_y = siz;
		}

		if ((0 == strcmp(type, "eviocg_abs_pressure"))) {


			try_replace_n(&d[doff].ioctl_EVIOCGABS_ABS_PRESSURE, baf, siz);
			d[doff].evabspressure = siz;
		}


		if (0 == strcmp(type, "abs_bits")) {

			try_replace_n(&d[doff].ioctl_EVIOCGBIT_EV_ABS_REAL, baf, siz);
			d[doff].realabsbits = siz;

		}


	} else if (0 == strcmp(tag, "Erecd:")) {
		void *id;
		char fname[128] = {0};
		int orig_rand = -1;
		FILE *fil = NULL;
		fscanf(tcase, "%p %127s", &id, fname);
		sscanf(fname, "evemucase%i.txt", &orig_rand);

		doff = hash_seek((void *)d, p->dhtsz, ditem_s,  id, id);
		if (doff == p->dhtsz) {

			return;
		}

		fil = fopen(fname, "r");

		if (fil == NULL) {
			char bfname[1024] = {0};

			strcat(bfname, (p->subfolder));
			strcat(bfname, "/");
			strcat(bfname, fname);

			fil = fopen(bfname, "r");

			if (fil == NULL) {
				fprintf(stderr, "cannot find . %s . \n", bfname);
				return;
			}
		}

		d[doff].emu_file_id = orig_rand;

		d[doff].evt = fil;
	} else if (0 == strcmp(tag, "Edesc:")) {
		void *id;
		char fname[128] = {0};
		int orig_fd = -1;
		FILE *fil = NULL;
		fscanf(tcase, "%p %s", &id, fname);
		sscanf(fname, "evemudesc%i.txt", &orig_fd);

		doff = hash_seek((void *)d, p->dhtsz, ditem_s,  id, id);
		if (doff == p->dhtsz)
			return;

		fil = fopen(fname, "r");

		if (fil == NULL) {
			char bfname[1024] = {0};

			strcat(bfname, (p->subfolder));
			strcat(bfname, "/");
			strcat(bfname, fname);

			fil = fopen(bfname, "r");

			if (fil == NULL) {
				fprintf(stderr, "cannot find . %s . \n", bfname);
				return;
			}
		}

		fakeston_evdev_dev_load_desc(&d[doff], fil);

		fclose(fil);

		d[doff].emu_desc_id = orig_fd;


	} else if (0 == strcmp(tag, "EnewBURST:")) {
		size_t i;
		void *id;
		unsigned long a, b, c, n;
		fscanf(tcase, "%lu %lu.%lu %p %lu", &a, &b, &c, &id, &n);

		doff = hash_seek((void*)d, p->dhtsz, ditem_s,  id, id);
		if (doff == p->dhtsz)
			return;

		if (d[doff].evt == NULL) {

			return;
		}

		if (n > 33) {
			fprintf(stdout, "error: too big burst\n");
			return;
		}

		struct input_event e[33];

		for (i = 0; i < n; i++) {
			evemu_read_event(d[doff].evt, &e[i]);
		}

		write(p->pajpa[1], e, n * sizeof(e[0]));

		if (d[doff].device == NULL) {
			fprintf(stdout, "error: device %p %zu is null\n", id, doff);
			return;
		}

		struct wl_event_source_fd *fdsource = (struct wl_event_source_fd *) d[doff].device->source;

		wl_event_loop_fd_func_t funkcia = fdsource->func;



		fixed_p = p;

		funkcia(p->pajpa[0] , 1337, d[doff].device);

		fixed_p = NULL;

	}
}

void fakeston_api_handler(void**dst, int call, void *data)
{
	struct fakeston_evdev_rev *r = (struct fakeston_evdev_rev *) fixed_p->r;
	struct fakeston_evdev_rev *z = (struct fakeston_evdev_rev *) fixed_p->z;
	struct fakeston_evdev_dev *d = (struct fakeston_evdev_dev *) fixed_p->d;
	struct fakeston_evdev_seat *s = (struct fakeston_evdev_seat *) fixed_p->s;
	size_t ritem_s = sizeof(struct fakeston_evdev_rev);
	size_t roff, doff, zoff, soff;

	if (fixed_p == NULL)
		return;

	if (call == 5) {
		zoff = hash_seek((void*)z, fixed_p->shtsz, ritem_s, data, data);
		if (zoff == fixed_p->shtsz) {
			return;
		}
		soff = z[zoff].off;
		*dst = (void*) s[soff].id;

	}

	roff = hash_seek((void*)r, fixed_p->dhtsz, ritem_s, data, data);
	if (roff == fixed_p->dhtsz)
		return;
	doff = r[roff].off;

	if (d[doff].id == 0)
		return;

	if (call == 3)
		*dst = (void*) (intptr_t) d[doff].emu_file_id;
	if (call == 2)
		*dst = (void*) d[doff].id;
	if (call == 1)
		*dst = (void*)(intptr_t)  d[doff].emu_desc_id;
	if (call == 4) {
		*dst = (void*) d[doff].seatid;

	}
}

void fakeston_parse(FILE *tcase, fakestonph_f dispatch, void*data)
{
	char buf[13] = {0};

	while (1 == fscanf(tcase, "%12s", buf)) {

		dispatch(data, buf, tcase);

		while ('\n' != fgetc(tcase));
	}

}

void sf(char *s)
{
	char *l = NULL;
	while(*s != 0) {
		s++;
		if (*s == '/')
			l = s;

	}
	if (l)
		*l = 0;
}


void foobar(){}

typedef int (*type_ioctl)(int __fd, unsigned long int __request, ...);

int ioctl (int __fd, unsigned long int __request, ...) {
	static type_ioctl original_ioctl = NULL;
	if(original_ioctl == NULL) {
		void* libc = dlopen("libc.so.6", RTLD_LAZY);
		original_ioctl = (type_ioctl)dlsym(libc, "ioctl");
		dlclose(libc);
	}
	va_list argp;
	va_start(argp, __request);
	char* dst = va_arg(argp, void*);

	if ((fixed_p != NULL) && (__fd > 1337)) {
		struct fakeston_evdev_rev *r = (struct fakeston_evdev_rev *) fixed_p->r;
		struct fakeston_evdev_dev *d = (struct fakeston_evdev_dev *) fixed_p->d;
		size_t ritem_s = sizeof(struct fakeston_evdev_rev);
		void *seek = (void *)(intptr_t)__fd;
		
		size_t roff = hash_seek((void*)r, fixed_p->dhtsz, ritem_s, seek, seek);
		if (roff != fixed_p->dhtsz) {

		size_t off = r[roff].off;

		if (__request == 2148025632) {

			char **source = &(d[off].ioctl_EVIOCGBIT_EV_BITS[0]);

			if (source) {

				memcpy(dst, *source, d[off].bitsbytes[0]);
				return d[off].bitsbytes[0];
			}
		} else

		if ((__request == 2164278534) || (__request == 18446744071567328518ULL)) {

			if (d[off].ioctl_EVIOCGNAME) {
				void *src = d[off].ioctl_EVIOCGNAME;
				size_t len = strlen(src);
				strncpy(dst, src, len);
				dst[len] = 0;
				return 0;

			}

		} else if ((__request ==  18446744071562609922ULL) || (__request ==  2148025602)) {
			if (d[off].ioctl_EVIOCGID) {
				size_t s = sizeof(struct input_id);
				char *rid = d[off].ioctl_EVIOCGID;

				memcpy(dst, rid, s);

				return 0;
			}


		} else if ((__request == 18446744071595640073ULL) || (__request == 2163754249)) {

			if (d[off].ioctl_EVIOCGPROP) {

				memcpy(dst, d[off].ioctl_EVIOCGPROP, d[off].pbytes);

				return d[off].pbytes;
			}

		} else if ((__request >= 18446744071595640096ULL) && (__request < 18446744071595640128ULL)) {

			unsigned int slot = __request & 31;

			char **source = &(d[off].ioctl_EVIOCGBIT_EV_BITS[slot]);

			if (source) {
				
				memcpy(dst, *source, d[off].bitsbytes[slot]);

				return d[off].bitsbytes[slot];
			}


		} else if ((__request >= 0xFFFFFFFF80184540ULL) && (__request <= 0xFFFFFFFF80184579ULL)) {

			unsigned int slot = __request & 63;

			char **source = &(d[off].ioctl_EVIOCGBIT_EV_ABS[slot]);

			if ((d[off].is_abs & (1 << slot)) && (source)) {

				memcpy(dst, *source, sizeof(struct input_absinfo));

				return sizeof(struct input_absinfo);
			}

		} else if (__request == 2149074240) {
			char *source = d[off].ioctl_eviocgabs_abs_x;
			if (source) {

				memcpy(dst, source, d[off].size_abs_x);
				return d[off].size_abs_x;
			}
		} else if (__request == 2149074241) {
			char *source = d[off].ioctl_eviocgabs_abs_y;
			if (source) {
				memcpy(dst, source, d[off].size_abs_y);
				return d[off].size_abs_y;
			}
		} else if (__request == 2149074293) {
			char *source = d[off].ioctl_eviocgabs_abs_mt_pos_x;
			if (source) {
				memcpy(dst, source, d[off].size_abs_mt_pos_x);
				return d[off].size_abs_mt_pos_x;
			}
		} else if (__request == 2149074294) {
			char *source = d[off].ioctl_eviocgabs_abs_mt_pos_y;
			if (source) {
				memcpy(dst, source, d[off].size_abs_mt_pos_y);
				return d[off].size_abs_mt_pos_y;
			}
		} else if (__request == 2149074264) {
			char *source = d[off].ioctl_EVIOCGABS_ABS_PRESSURE;
			if (source) {

				memcpy(dst, source, d[off].evabspressure);
				return d[off].evabspressure;
			}
		} else if (__request == 2148025635) {
			char *source = d[off].ioctl_EVIOCGBIT_EV_ABS_REAL;

			if (source) {
				memcpy(dst, source, d[off].realabsbits);
				return d[off].realabsbits;
			}

		} else if (__request == 2148025634) {
			char **source = &(d[off].ioctl_EVIOCGBIT_EV_REL);

			if (source) {
				memcpy(dst, *source, d[off].relbits);
				return d[off].relbits;
			}
		} else if ((__request == 2153792792ULL) || (__request == 2197832984ULL)) {

			char *source = d[off].ioctl_EVIOCGKEY;

			if (source) {

				memcpy(dst, source, d[off].EVIOCGKEYsize);

				return d[off].EVIOCGKEYsize;
			}

		} else if ((__request == 2153792801ULL)) {
			char *source = d[off].ioctl_EVIOCGBIT_EV_KEY;

			if (source) {

				memcpy(dst, source, d[off].keybytes);

				return d[off].keybytes;
			}

		} else



		{
				foobar();
			fprintf(stderr, "Fakeston: Warning: unknown ioctl req:%lu fd:%i.\n", __request, __fd);foobar();
		}

		}

	}
	int ret;
	ret = original_ioctl(__fd, __request, argp);
	va_end(argp);
	return ret;
}


int fakeston_main(char *filename)
{
	FILE *tcase;
	tcase = fopen(filename, "r");

	if (tcase == NULL) {
		fprintf(stderr, "Error: cannot open ftf file '%s'\n", filename);
		return -1;
	}

	char form[128];
	unsigned int format;

	fscanf(tcase, "%127s %u\n", form, &format);

	if (0 != strcmp("FAKESTONTESTCASEFORMAT", form)) {
		fprintf(stderr, "Error: bad format tft file '%s'\n", filename);
		return -2;
	}

	if (2 != format) {
		fprintf(stderr, "Error: unsupported format tft file '%s'\n", filename);
		return -3;
	}

	const size_t shtsz = 8;/* how many seats in hashtable */
	const size_t dhtsz = 128;/* how many devices in hashtable */

	struct weston_mode mode;
	mode.width = 1024;
	mode.height = 768;
	struct weston_output output;
	struct fakeston_evdev_rev reverseseats_htable[shtsz];
	struct fakeston_evdev_seat seats_htable[shtsz];
	struct fakeston_evdev_dev devices_htable[dhtsz];
	struct fakeston_evdev_rev reversedev_htable[dhtsz];
	struct pload p;
	memset(reverseseats_htable, 0, sizeof(reverseseats_htable));
	memset(seats_htable, 0, sizeof(seats_htable));
	memset(devices_htable, 0, sizeof(devices_htable));
	memset(reversedev_htable, 0, sizeof(reversedev_htable));
	p.shtsz = shtsz;
	p.dhtsz = dhtsz;
	p.d = devices_htable;
	p.s = seats_htable;
	p.r = reversedev_htable;
	p.z = reverseseats_htable;
	p.seq = 0;
	p.fd_seq = 1338;
	p.subfolder = strdup(filename);
	sf(p.subfolder);
	p.comp.focus = 1;
	p.output = &output;
	output.current = &mode;
	p.comp.config = (void *) fakeston_api_handler;
	p.comp.idle_inhibit = 0x1337;
	p.comp.state = 0x7331;

	wl_list_init(&p.devices_list);

	if (pipe(p.pajpa) < 0) {
		fprintf(stderr, "Failed pipe\n");
		return -4;
	}

	fcntl(p.pajpa[0], F_SETFD, fcntl(p.pajpa[0], F_GETFD) | FD_CLOEXEC);
	fcntl(p.pajpa[0], F_SETFL, fcntl(p.pajpa[0], F_GETFL) | O_NONBLOCK);
	fcntl(p.pajpa[1], F_SETFD, fcntl(p.pajpa[1], F_GETFD) | FD_CLOEXEC);

	fakeston_parse(tcase, fakeston_line_handler, (void*)&p);

	fclose(tcase);

	struct evdev_device *device, *previous = NULL;
	wl_list_for_each(device, &p.devices_list, link) {
		if (previous) {
			fixed_p = &p;
			evdev_device_destroy(previous);
			fixed_p = NULL;
		}
		previous = device;
	}
	if (previous) {
		fixed_p = &p;
		evdev_device_destroy(previous);
		fixed_p = NULL;
	}

	free(p.subfolder);
	return 0;
}
