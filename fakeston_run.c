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

#include "fakeston.h"

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
notify_motion_absolute(struct weston_seat *seat, uint32_t time,
		       wl_fixed_t x, wl_fixed_t y)
{
/*
	verbose
*/

	fprintf(stdout, "notify_motion_absolute\t%p\t%11u %11i %11i\n",
		seat, time, x, y);
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
	/* verbose */


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

	if (2 != format) {
		fprintf(stderr, "Error: unsupported format tft file '%s'\n", argv[1]);
		return -3;
	}

	const size_t shtsz = 16;/* how many seats in hashtable */
	const size_t dhtsz = 128;/* how many devices in hashtable */

	struct weston_mode mode;
	mode.width = 1024;
	mode.height = 768;
	struct weston_output output;
	struct fakeston_evdev_seat seats_htable[shtsz];
	struct fakeston_evdev_dev devices_htable[dhtsz];
	struct fakeston_evdev_rdev reversedev_htable[dhtsz];
	struct pload p;
	memset(seats_htable, 0, sizeof(seats_htable));
	memset(devices_htable, 0, sizeof(devices_htable));
	memset(reversedev_htable, 0, sizeof(reversedev_htable));
	p.shtsz = shtsz;
	p.dhtsz = dhtsz;
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
	return 0;
}
