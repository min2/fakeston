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

#include <stdlib.h>
#include "fakeston.h"
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
/*
	fprintf(stdout, "notify_motion\t%p\t%11u %11i %11i\n",
		seat, time, dx, dy);
*/
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

	return fakeston_main(argv[1]);
}

