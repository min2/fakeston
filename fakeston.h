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

#ifndef _FAKESTON_H_
#define _FAKESTON_H_

#include <stdarg.h>
#include <stdint.h>

#include "wayland-server-protocol.h"
#include "compositor.h"

/*copied from event-loop.c */
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
/*end of copied*/

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
	size_t shtsz, dhtsz;
	unsigned int seq;
	int fd_seq;
	int pajpa[2];
	char *subfolder;
	struct wl_list devices_list;
	struct weston_compositor comp;
	struct weston_output *output;
/*
	struct wl_keyboard k;
*/
};

typedef void (*fakestonph_f)(void*, char*, FILE *);

extern struct pload *fixed_p;

void fakeston_line_handler(void*data, char*tag, FILE *tcase);

#endif
