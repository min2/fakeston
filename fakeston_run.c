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

#include "evemu-impl.h"

/* ovveriding of ioctl and read*/

#include <stdarg.h>
#include <dlfcn.h>
#include <unistd.h>


static struct pload *fixed_p = NULL;

/* the fake compositor*/

#include <linux/input.h>
#include <fcntl.h>
#include "wayland-server-protocol.h"
#include "compositor.h"
#include "evdev.h"


int
weston_log(const char *fmt, ...)
{
	int l;
	va_list argp;
	va_start(argp, fmt);
	l = vfprintf(stdout, fmt, argp);
	va_end(argp);
	return l;
}

void
notify_button(struct weston_seat *seat, uint32_t time, int32_t button,
	      enum wl_pointer_button_state state)
{
	fprintf(stdout, "notify_button\t%p\t%11u %11i %11u\n",
		seat, time, button, state);
}

void
notify_axis(struct weston_seat *seat, uint32_t time, uint32_t axis,
	    wl_fixed_t value)
{
	fprintf(stdout, "notify_axis\t%p\t%11u %11u %11u\n",
		seat, time, axis, value);
}

void
notify_modifiers(struct weston_seat *seat, uint32_t serial)
{
	fprintf(stdout, "notify_modifiers\t%p\t%11u\n", seat, serial);
}

void
notify_motion(struct weston_seat *seat,
	      uint32_t time, wl_fixed_t dx, wl_fixed_t dy)
{
/*
	verbose
*/

	fprintf(stdout, "notify_motion\t%p\t%11u %11i %11i\n",
		seat, time, dx, dy);

}

void
notify_key(struct weston_seat *seat, uint32_t time, uint32_t key,
	   enum wl_keyboard_key_state state,
	   enum weston_key_state_update update_state)
{
	fprintf(stdout, "notify_key\t%p\t%11u %11u %11u %11u\n",
		seat, time, key, state, update_state);
}

void
notify_touch(struct weston_seat *seat, uint32_t time, int touch_id,
             wl_fixed_t x, wl_fixed_t y, int touch_type)
{
	fprintf(stdout, "notify_touch\t%p\t%11u %11i %11u %11u %11i\n",
		seat, time, touch_id, x, y, touch_type);
}

void
weston_seat_init_pointer(struct weston_seat *seat)
{

}

int
weston_seat_init_keyboard(struct weston_seat *seat, struct xkb_keymap *keymap)
{
	return 0;
}

void
weston_seat_init_touch(struct weston_seat *seat)
{

}

struct mtdev *mtdev_new_open(int fd)
{
	return NULL;
}

int mtdev_get(struct mtdev *dev, int fd, struct input_event* ev, int ev_max)
{
	return 0;
}

void mtdev_close_delete(struct mtdev *dev)
{

}

struct wl_event_loop *wl_display_get_event_loop(struct wl_display *display)
{
	return NULL;
}
struct wl_event_source *wl_event_loop_add_timer(struct wl_event_loop *loop,
						wl_event_loop_timer_func_t func,
						void *data)
{
	return NULL;
}

int wl_event_source_remove(struct wl_event_source *source)
{
	free(source);
	return 0;
}


struct wl_event_source {
	struct wl_event_source_interface *interface;
	struct wl_event_loop *loop;
	struct wl_list link;
	void *data;
	int fd;
};

struct wl_event_source_fd {
	struct wl_event_source base;
	wl_event_loop_fd_func_t func;
	int fd;
};


struct wl_event_source *wl_event_loop_add_fd(struct wl_event_loop *loop,
					     int fd, uint32_t mask,
					     wl_event_loop_fd_func_t func,
					     void *data)
{
	struct wl_event_source_fd *new;
	new = malloc(sizeof(struct wl_event_source_fd));

	new->func = func;

	return (struct wl_event_source *) new;
}
int wl_event_source_timer_update(struct wl_event_source *source,
				 int ms_delay)
{
	return 0;
}

uint32_t
weston_compositor_get_time(void)
{
	return 0;
}

void
notify_keyboard_focus_in(struct weston_seat *seat, struct wl_array *keys,
			 enum weston_key_state_update update_state)
{

}


/*end of fake compositor*/

size_t shtsz = 4;/*4seats*/
size_t dhtsz = 128;/*64evdevdevices*/

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

struct fakeston_evdev_seat {
	uintptr_t id;
	struct weston_seat whatever;
};

struct fakeston_evdev_rdev {
	uintptr_t id;
	size_t off;
};

struct fakeston_evdev_dev {
	uintptr_t id;
	uintptr_t seatid;
	unsigned int init_serial;
	struct evdev_device *device;
	FILE * evt;
	char *ioctl_eviocgabs_abs_x;
	char *ioctl_eviocgabs_abs_y;
	char *ioctl_eviocgabs_abs_mt_pos_x;
	char *ioctl_eviocgabs_abs_mt_pos_y;
	char *ioctl_EVIOCGNAME;
	char *ioctl_EVIOCGID;
	char *ioctl_EVIOCGPROP;
	char *ioctl_EVIOCGKEY;
	char *(ioctl_EVIOCGBIT_EV_BITS[32]);
	char *ioctl_EVIOCGBIT_EV_KEY;
	char *ioctl_EVIOCGBIT_EV_REL;
	char *(ioctl_EVIOCGBIT_EV_ABS[64]);
	char *ioctl_EVIOCGBIT_EV_ABS_REAL;
	char *ioctl_EVIOCGABS_ABS_PRESSURE;
	size_t pbytes;
	size_t bitsbytes[32];
	unsigned int is_abs;
	size_t 	EVIOCGKEYsize;
	size_t realabsbits;
	size_t keybytes;
	size_t absbits, relbits;
	size_t size_abs_x, size_abs_y, size_abs_mt_pos_x, size_abs_mt_pos_y;
	size_t evabspressure;
	int created;
	int fd;
};

struct pload {
	struct fakeston_evdev_rdev *r;
	struct fakeston_evdev_seat *s;
	struct fakeston_evdev_dev *d;
	unsigned int seq;
	int fd_seq;
	int pajpa[2];
	char *subfolder;
	struct wl_list devices_list;
	struct weston_compositor comp;
	struct weston_output *output;
	struct wl_keyboard k;
};

typedef void (*fakestonph_f)(void*, char*, FILE *);

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
/*
	struct input_absinfo abs;
*/
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
/*
		fprintf(stdout, ": %02x %02x %02x %02x \n", src[0], src[1], src[2], src[3]);
*/
		size_t bb = device->bitsbytes[index] = dev.mbytes[index];
		try_replace_n(chlievik, src, bb);

		src = *chlievik;
/*
		fprintf(stdout, "%p: %02x %02x %02x %02x \n", src, src[0], src[1], src[2], src[3]);
*/
	}

/*
	try_replace_n(&device->ioctl_EVIOCGID, &id, 0);
*/
	read_abs(&dev, fp);

	for (index = 0; index < 64; index++) {

		if (!evemu_has_event(&dev, EV_ABS, index)) {
/*			fprintf(stdout, "skipping %u \n", index);
*/
			continue;
		}

		char **chlievik = &(device->ioctl_EVIOCGBIT_EV_ABS[index]);

		struct input_absinfo *aa = &dev.abs[index];
/*
		foobar();
*/

/*
		fprintf(stdout, "aa??%p\n", *device);
		fprintf(stdout, "aaaA%p: %u %d %d %d %d\n", aa, index,
		aa->minimum, aa->maximum, aa->fuzz, aa->flat);
*/

		try_replace_n(chlievik, (char *)aa, sizeof(struct input_absinfo));

		device->is_abs |= 1<<index;
/*
		struct input_absinfo *ddd = *chlievik;

		fprintf(stdout, "aaaA%p: %u %d %d %d %d\n", ddd, index,
		ddd->minimum, ddd->maximum, ddd->fuzz, ddd->flat);
*/


	}

/*
	try_replace_n(&device->ioctl_EVIOCGID, &id, 0);
*/
}

int evemu_read_event(FILE *fp, struct input_event *ev);

void fakeston_line_handler(void*data, char*tag, FILE *tcase)
{
	struct pload *p = ( struct pload *) data;
	struct fakeston_evdev_dev *d = (struct fakeston_evdev_dev *) p->d;
	struct fakeston_evdev_seat *s = (struct fakeston_evdev_seat *) p->s;
	struct fakeston_evdev_rdev *r = (struct fakeston_evdev_rdev *) p->r;
	size_t doff, soff, roff;
	size_t sitem_s = sizeof(struct fakeston_evdev_seat);
	size_t ditem_s = sizeof(struct fakeston_evdev_dev);
	size_t ritem_s = sizeof(struct fakeston_evdev_rdev);
	if (0 == strcmp(tag, "seatfocus:")) {
		void *id;

		fscanf(tcase, "%p", &id);

		soff = hash_seek((void*)s, shtsz, sitem_s,  id, id);
		if (soff == shtsz) {
			return;
		}

		evdev_notify_keyboard_focus(&s[soff].whatever, &p->devices_list);

	} else
	if (0 == strcmp(tag, "EcreateDEV:")) {
		void *id, *seatid;

		fscanf(tcase, "%p", &id);

		doff = hash_seek((void*)d, dhtsz, ditem_s,  id, id);
		if (doff == dhtsz) {
			return;
		}

		seatid = (void *) d[doff].seatid;

		soff = hash_seek((void*)s, shtsz, sitem_s,  seatid, seatid);
		if (soff == shtsz) {
			return;
		}

		int dev_fd = d[doff].fd;

		void *device = evdev_device_create(&s[soff].whatever, "<mock-dev-path>", dev_fd);
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
		void *id, *seatid;

		fscanf(tcase, "%p %p", &id, &seatid);

		doff = hash_seek((void*)d, dhtsz, ditem_s,  id, id);
		if (doff != dhtsz) {
			return;
		}
		doff = hash_seek((void*)d, dhtsz, ditem_s,  id, 0);

		soff = hash_seek((void*)s, shtsz, sitem_s,  seatid, seatid);
		if (soff == shtsz) {
			soff = hash_seek((void*)s, shtsz, sitem_s,  seatid, 0);
			if (soff == shtsz)
				return;
			s[soff].id = (uintptr_t) seatid;
			s[soff].whatever.compositor = &p->comp;
			s[soff].whatever.seat.keyboard = &p->k;
		}

		int dev_fd = p->fd_seq++;
/*
		fprintf(stdout, "EprepareDEV %i %u \n", dev_fd, doff);
*/
		roff = hash_seek((void*)r, dhtsz, ritem_s, (void*)(intptr_t)dev_fd, 0);
/*
		fprintf(stdout, "%pput on %u= %p \n", r, roff, dev_fd);
*/
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

		doff = hash_seek((void *)d, dhtsz, ditem_s,  id, id);
		if (doff == dhtsz)
			return;

		d[doff].id = (uintptr_t) 0;
		if (d[doff].evt) {
			fclose(d[doff].evt);
		}
		if (d[doff].created) {
			evdev_device_destroy(d[doff].device);
			d[doff].created = 0;
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

		doff = hash_seek((void*)d, dhtsz, ditem_s,  id, id);
		if (doff == dhtsz) {
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
/*
			printf("{%p}keybits ", d[doff].ioctl_EVIOCGBIT_EV_KEY);

		for (i = 0; i < siz; i++) {
			fprintf(stdout, "%02x ", (unsigned char)baf[i]);
		}
		putchar('\n');

			printf("{%p}keybitscopied ", d[doff].ioctl_EVIOCGBIT_EV_KEY);

		for (i = 0; i < siz; i++) {
			fprintf(stdout, "%02x ", (unsigned char)d[doff].ioctl_EVIOCGBIT_EV_KEY[i]);
		}
		putchar('\n');
*/
		}

		if (0 == strcmp(type, "evdev_keys")) {
			try_replace_n(&d[doff].ioctl_EVIOCGKEY, baf, siz);
			d[doff].EVIOCGKEYsize = siz;

/*
			printf("{%p}ekeys ", d[doff].ioctl_EVIOCGKEY);

		for (i = 0; i < siz; i++) {
			fprintf(stdout, "%02x ", (unsigned char)baf[i]);
		}
		putchar('\n');

			printf("{%p}ekeyscopied ", d[doff].ioctl_EVIOCGKEY);

		for (i = 0; i < siz; i++) {
			fprintf(stdout, "%02x ", (unsigned char)d[doff].ioctl_EVIOCGKEY[i]);
		}
		putchar('\n');
*/
		}


/*
		if ((0 == strcmp(type, "ev_bits"))) {
			try_replace_n(&d[doff].ioctl_EVIOCGBITS, baf, siz);
			d[doff].cgbits = siz;
		}
*/
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

/*
		for (i = 0; i < siz; i++) {
			fprintf(stdout, "%02x ", (unsigned char)baf[i]);
		}
		putchar('\n');
*/
			try_replace_n(&d[doff].ioctl_EVIOCGABS_ABS_PRESSURE, baf, siz);
			d[doff].evabspressure = siz;
		}



/*
		this gives wrong result:
*/


		if (0 == strcmp(type, "abs_bits")) {

			try_replace_n(&d[doff].ioctl_EVIOCGBIT_EV_ABS_REAL, baf, siz);
			d[doff].realabsbits = siz;
/*
		for (i = 0; i < siz; i++) {
			fprintf(stdout, "%02x ", (unsigned char)baf[i]);
		}
		putchar('\n');
*/
		}


	} else if (0 == strcmp(tag, "Erecd:")) {
		void *id;
		char fname[128] = {0};
		FILE *fil = NULL;
		fscanf(tcase, "%p %127s", &id, fname);
/*
		fprintf(stdout, "file %p %s \n", id, fname);
*/
		doff = hash_seek((void *)d, dhtsz, ditem_s,  id, id);
		if (doff == dhtsz) {
/*
			printf("id %p not prepared %u \n", id, doff);
*/
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
/*
		printf("set evt %u to %p \n", doff, fil);
*/
		d[doff].evt = fil;
	} else if (0 == strcmp(tag, "Edesc:")) {
		void *id;
		char fname[128] = {0};
		FILE *fil = NULL;
		fscanf(tcase, "%p %s", &id, fname);

		doff = hash_seek((void *)d, dhtsz, ditem_s,  id, id);
		if (doff == dhtsz)
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

	} else if (0 == strcmp(tag, "EnewBURST:")) {
		size_t i;
		void *id;
		unsigned long a, b, c, n;
		fscanf(tcase, "%lu %lu.%lu %p %lu", &a, &b, &c, &id, &n);
/*
		fprintf(stdout, "EnewBURST %p\n", id);
*/
		doff = hash_seek((void*)d, dhtsz, ditem_s,  id, id);
		if (doff == dhtsz)
			return;

		if (d[doff].evt == NULL) {
/*
			fprintf(stdout, "error: no evt\n");
*/
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
/*
		printf("Pajpujem %u \n", n);
*/
		funkcia(p->pajpa[0] , 1337, d[doff].device);

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

int main(int argc, char**argv)
{
	if (argc < 2) {
		usage();
		return -1;
	}

	FILE *tcase;
	tcase = fopen(argv[1], "r");

	if (tcase == NULL) {
		fprintf(stderr, "Error: cannot open ftf file '%s'\n", argv[1]);
		return -1;
	}

	char form[128];
	unsigned int format;

	fscanf(tcase, "%127s %u\n", form, &format);

	if (0 != strcmp("FAKESTONTESTCASEFORMAT", form)) {
		fprintf(stderr, "Error: bad format tft file '%s'\n", argv[1]);
		return -2;
	}

	if (1 != format) {
		fprintf(stderr, "Error: unsupported format tft file '%s'\n", argv[1]);
		return -3;
	}

	struct weston_mode mode;
	mode.width = 1024;
	mode.height = 768;
	struct weston_output output;
	struct fakeston_evdev_seat seats_htable[shtsz];
	struct fakeston_evdev_dev devices_htable[dhtsz];
	struct fakeston_evdev_rdev reversedev_htable[dhtsz];
	struct pload p;
	fixed_p = &p;
	memset(seats_htable, 0, sizeof(seats_htable));
	memset(devices_htable, 0, sizeof(devices_htable));
	memset(reversedev_htable, 0, sizeof(reversedev_htable));
	p.d = devices_htable;
	p.s = seats_htable;
	p.r = reversedev_htable;
	p.seq = 0;
	p.fd_seq = 1338;
	p.subfolder = strdup(argv[1]);
	sf(p.subfolder);
	p.comp.focus = 1;
	p.output = &output;
	output.current = &mode;

	wl_list_init(&p.devices_list);

	if (pipe(p.pajpa) < 0) {
		fprintf(stderr, "Failed pipe\n");
		return -1;
	}

	fcntl(p.pajpa[0], F_SETFD, fcntl(p.pajpa[0], F_GETFD) | FD_CLOEXEC);
	fcntl(p.pajpa[0], F_SETFL, fcntl(p.pajpa[0], F_GETFL) | O_NONBLOCK);
	fcntl(p.pajpa[1], F_SETFD, fcntl(p.pajpa[1], F_GETFD) | FD_CLOEXEC);

	fakeston_parse(tcase, fakeston_line_handler, (void*)&p);

	fclose(tcase);

	struct evdev_device *device;
	wl_list_for_each(device, &p.devices_list, link) {
		evdev_device_destroy(device);
	}

	free(p.subfolder);
	fixed_p = NULL;
	return 0;
}

/*overriding ioctl */

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
		struct fakeston_evdev_rdev *r = (struct fakeston_evdev_rdev *) fixed_p->r;
		struct fakeston_evdev_dev *d = (struct fakeston_evdev_dev *) fixed_p->d;
		size_t ritem_s = sizeof(struct fakeston_evdev_rdev);
		void *seek = (void *)(intptr_t)__fd;
		
		size_t roff = hash_seek((void*)r, dhtsz, ritem_s, seek, seek);
		if (roff != dhtsz) {
/*
		fprintf(stdout, "%pfound %p=%i -> %lu :D\n",r, seek, __fd, roff);
*/
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


		} else if (__request == 18446744071595640073ULL) {

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
/*
				struct input_absinfo *aa = *source;

				if (slot == 0)

				fprintf(stdout, "A [%p]: %02x %d %d %d %d\n", aa, slot,
						aa->minimum, aa->maximum, aa->fuzz, aa->flat);
*/
				return sizeof(struct input_absinfo);
			}

/*
Fakeston: Warning: unknown ioctl req:2149074240 fd:1348.ABS_X
Fakeston: Warning: unknown ioctl req:2149074241 fd:1348.y
Fakeston: Warning: unknown ioctl req:2149074293 fd:1348.posx
Fakeston: Warning: unknown ioctl req:2149074294 fd:1348.posy
Fakeston: Warning: unknown ioctl req:2149074264 fd:1348.abspressure
*/
		} else if (__request == 2149074240) {
			char *source = d[off].ioctl_eviocgabs_abs_x;
			if (source) {
/*
		size_t i;
		for (i = 0; i < d[off].size_abs_x; i++) {
			fprintf(stdout, "%02x ", (unsigned char)(source)[i]);
		}
		putchar('\n');
*/
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
/*
		size_t i;
		for (i = 0; i < d[off].evabspressure; i++) {
			fprintf(stdout, "%02x ", (unsigned char)(source)[i]);
		}
		putchar('\n');
*/
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
		} else if ((__request == 2153792792ULL)) {

			char *source = d[off].ioctl_EVIOCGKEY;

			if (source) {
/*
			printf("[%p] ", source);

		size_t i;
		for (i = 0; i < d[off].keybytes; i++) {
			fprintf(stdout, "%02x ", (unsigned char)(source)[i]);
		}
		putchar('\n');
*/
				memcpy(dst, source, d[off].EVIOCGKEYsize);

				return d[off].EVIOCGKEYsize;
			}

		} else if ((__request == 2153792801ULL)) {
			char *source = d[off].ioctl_EVIOCGBIT_EV_KEY;

			if (source) {
/*
			printf("[%p] ", source);

		size_t i;
		for (i = 0; i < d[off].keybytes; i++) {
			fprintf(stdout, "%02x ", (unsigned char)(source)[i]);
		}
		putchar('\n');
*/
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


/**/
