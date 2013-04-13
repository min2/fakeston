#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <linux/input.h>
#include <fcntl.h>

#include "compositor.h"
#include "evdev.h"

#include "wayland-server-protocol.h"
#include "evdev.h"

int
weston_log(const char *fmt, ...)
{
}

void
notify_button(struct weston_seat *seat, uint32_t time, int32_t button,
	      enum wl_pointer_button_state state)
{
}

void
notify_axis(struct weston_seat *seat, uint32_t time, uint32_t axis,
	    wl_fixed_t value)
{
}

void
notify_modifiers(struct weston_seat *seat, uint32_t serial)
{
}

void
notify_motion(struct weston_seat *seat,
	      uint32_t time, wl_fixed_t dx, wl_fixed_t dy)
{
}

void
notify_key(struct weston_seat *seat, uint32_t time, uint32_t key,
	   enum wl_keyboard_key_state state,
	   enum weston_key_state_update update_state)
{

}

void
notify_touch(struct weston_seat *seat, uint32_t time, int touch_id,
             wl_fixed_t x, wl_fixed_t y, int touch_type)
{

}

void
weston_seat_init_pointer(struct weston_seat *seat)
{

}

int
weston_seat_init_keyboard(struct weston_seat *seat, struct xkb_keymap *keymap)
{

}

void
weston_seat_init_touch(struct weston_seat *seat)
{

}

struct mtdev *mtdev_new_open(int fd)
{

}

int mtdev_get(struct mtdev *dev, int fd, struct input_event* ev, int ev_max)
{

}

void mtdev_close_delete(struct mtdev *dev)
{

}

struct wl_event_loop *wl_display_get_event_loop(struct wl_display *display)
{

}
struct wl_event_source *wl_event_loop_add_timer(struct wl_event_loop *loop,
						wl_event_loop_timer_func_t func,
						void *data)
{

}

int wl_event_source_remove(struct wl_event_source *source)
{

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
	new = calloc(1, sizeof(new));

	fprintf(stderr, "fdsrc%p\n", new);
	fprintf(stderr, "func%p\n", func);

	new->func = func;

	return (struct wl_event_source *) new;
}
int wl_event_source_timer_update(struct wl_event_source *source,
				 int ms_delay)
{

}

uint32_t
weston_compositor_get_time(void)
{

}

void
notify_keyboard_focus_in(struct weston_seat *seat, struct wl_array *keys,
			 enum weston_key_state_update update_state)
{

}


int main()
{
	struct weston_compositor dummy_c;
	struct weston_seat dummy_seat_1;
	dummy_seat_1.compositor = &dummy_c;
	struct evdev_device *device;
	struct wl_list devices_list;
	struct wl_list *linka = NULL;


	wl_list_init(&devices_list);

	device = evdev_device_create(&dummy_seat_1, "foo", 0);

	if (device == NULL)
		return;

	struct wl_event_source_fd *fdsource = device->source;

	fprintf(stderr, "fdsrc %p\n", fdsource);

	wl_event_loop_fd_func_t funkcia = fdsource->func;

	fprintf(stderr, "funkcia %p\n", funkcia);

	wl_list_insert(&devices_list, &device->link);


/*
	struct input_event ev[128];
*/

/*
	evdev_process_events(device, ev, 5);
*/

/*
	int pajpa[2];
	if (pipe(pajpa) < 0) {
		fprintf(stderr, "Evdev process events\n");
		return -1;
	}
*/


	fprintf(stderr, "funkcia %p\n", funkcia);

	fprintf(stderr, "fdsrc%p\n", fdsource);


	funkcia(3 , 1337, device);

	evdev_device_destroy(device);

	return 0;
}
