/* 
 * Copyright © 2008-2011 Kristian Høgsberg
 * Copyright © 2010-2011 Intel Corporation
 * 
 * Permission to use, copy, modify, distribute, and sell this
 * software and its documentation for any purpose is hereby granted
 * without fee, provided that the above copyright notice appear in
 * all copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * the copyright holders not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 * 
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#ifndef WAYLAND_SERVER_PROTOCOL_H
#define WAYLAND_SERVER_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-util.h"

struct wl_client;
struct wl_resource;

struct wl_display;
struct wl_registry;
struct wl_callback;
struct wl_compositor;
struct wl_shm_pool;
struct wl_shm;
struct wl_buffer;
struct wl_data_offer;
struct wl_data_source;
struct wl_data_device;
struct wl_data_device_manager;
struct wl_shell;
struct wl_shell_surface;
struct wl_surface;
struct wl_seat;
struct wl_pointer;
struct wl_keyboard;
struct wl_touch;
struct wl_output;
struct wl_region;

extern const struct wl_interface wl_display_interface;
extern const struct wl_interface wl_registry_interface;
extern const struct wl_interface wl_callback_interface;
extern const struct wl_interface wl_compositor_interface;
extern const struct wl_interface wl_shm_pool_interface;
extern const struct wl_interface wl_shm_interface;
extern const struct wl_interface wl_buffer_interface;
extern const struct wl_interface wl_data_offer_interface;
extern const struct wl_interface wl_data_source_interface;
extern const struct wl_interface wl_data_device_interface;
extern const struct wl_interface wl_data_device_manager_interface;
extern const struct wl_interface wl_shell_interface;
extern const struct wl_interface wl_shell_surface_interface;
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface wl_seat_interface;
extern const struct wl_interface wl_pointer_interface;
extern const struct wl_interface wl_keyboard_interface;
extern const struct wl_interface wl_touch_interface;
extern const struct wl_interface wl_output_interface;
extern const struct wl_interface wl_region_interface;

#ifndef WL_DISPLAY_ERROR_ENUM
#define WL_DISPLAY_ERROR_ENUM
/**
 * wl_display_error - global error values
 * @WL_DISPLAY_ERROR_INVALID_OBJECT: server couldn't find object
 * @WL_DISPLAY_ERROR_INVALID_METHOD: method doesn't exist on the
 *	specified interface
 * @WL_DISPLAY_ERROR_NO_MEMORY: server is out of memory
 *
 * These errors are global and can be emitted in response to any server
 * request.
 */
enum wl_display_error {
	WL_DISPLAY_ERROR_INVALID_OBJECT = 0,
	WL_DISPLAY_ERROR_INVALID_METHOD = 1,
	WL_DISPLAY_ERROR_NO_MEMORY = 2,
};
#endif /* WL_DISPLAY_ERROR_ENUM */

/**
 * wl_display - core global object
 * @sync: asynchronous roundtrip
 * @get_registry: get global registry object
 *
 * The core global object. This is a special singleton object. It is used
 * for internal Wayland protocol features.
 */
struct wl_display_interface {
	/**
	 * sync - asynchronous roundtrip
	 * @callback: (none)
	 *
	 * The sync request asks the server to emit the 'done' event on
	 * the provided wl_callback object. Since requests are handled
	 * in-order, this can be used as a barrier to ensure all previous
	 * requests have been handled.
	 */
	void (*sync)(struct wl_client *client,
		     struct wl_resource *resource,
		     uint32_t callback);
	/**
	 * get_registry - get global registry object
	 * @callback: (none)
	 *
	 * This request creates a registry object that allows the client
	 * to list and bind the global objects available from the
	 * compositor.
	 */
	void (*get_registry)(struct wl_client *client,
			     struct wl_resource *resource,
			     uint32_t callback);
};

#define WL_DISPLAY_ERROR	0
#define WL_DISPLAY_DELETE_ID	1

/**
 * wl_registry - global registry object
 * @bind: bind an object to the display
 *
 * The global registry object. The server has a number of global objects
 * that are available to all clients. These objects typically represent an
 * actual object in the server (for example, an input device) or they are
 * singleton objects that provides extension functionality.
 *
 * When a client creates a registry object, the registry object will emit a
 * global event for each global currently in the registry. Globals come and
 * go as a result of device hotplugs, reconfiguration or other events, and
 * the registry will send out @global and @global_remove events to keep the
 * client up to date with the changes. To mark the end of the initial burst
 * of events, the client can use the wl_display.sync request immediately
 * after calling wl_display.get_registry.
 *
 * A client can 'bind' to a global object by using the bind request. This
 * creates a client side handle that lets the object emit events to the
 * client and lets the client invoke requests on the object.
 */
struct wl_registry_interface {
	/**
	 * bind - bind an object to the display
	 * @name: unique number id for object
	 * @interface: name of the objects interface
	 * @version: version of the objects interface
	 * @id: (none)
	 *
	 * Binds a new, client-created object to the server using @name
	 * as the identifier.
	 */
	void (*bind)(struct wl_client *client,
		     struct wl_resource *resource,
		     uint32_t name,
		     const char *interface, uint32_t version, uint32_t id);
};

#define WL_REGISTRY_GLOBAL	0
#define WL_REGISTRY_GLOBAL_REMOVE	1

static inline void
wl_registry_send_global(struct wl_resource *resource_, uint32_t name, const char *interface, uint32_t version)
{
	wl_resource_post_event(resource_, WL_REGISTRY_GLOBAL, name, interface, version);
}

static inline void
wl_registry_send_global_remove(struct wl_resource *resource_, uint32_t name)
{
	wl_resource_post_event(resource_, WL_REGISTRY_GLOBAL_REMOVE, name);
}

#define WL_CALLBACK_DONE	0

static inline void
wl_callback_send_done(struct wl_resource *resource_, uint32_t serial)
{
	wl_resource_post_event(resource_, WL_CALLBACK_DONE, serial);
}

/**
 * wl_compositor - the compositor singleton
 * @create_surface: create new surface
 * @create_region: create new region
 *
 * A compositor. This object is a singleton global. The compositor is in
 * charge of combining the contents of multiple surfaces into one
 * displayable output.
 */
struct wl_compositor_interface {
	/**
	 * create_surface - create new surface
	 * @id: (none)
	 *
	 * Ask the compositor to create a new surface.
	 */
	void (*create_surface)(struct wl_client *client,
			       struct wl_resource *resource,
			       uint32_t id);
	/**
	 * create_region - create new region
	 * @id: (none)
	 *
	 * Ask the compositor to create a new region.
	 */
	void (*create_region)(struct wl_client *client,
			      struct wl_resource *resource,
			      uint32_t id);
};

/**
 * wl_shm_pool - a shared memory pool
 * @create_buffer: create wl_buffer from pool
 * @destroy: destroy the pool
 * @resize: change the size of the pool mapping
 *
 * The wl_shm_pool object encapsulates a piece of memory shared between
 * the compositor and client. Through the wl_shm_pool object, the client
 * can allocate shared memory wl_buffer objects. The objects will share the
 * same underlying mapped memory. Reusing the mapped memory avoids the
 * setup/teardown overhead and is useful when interactively resizing a
 * surface or for many small buffers.
 */
struct wl_shm_pool_interface {
	/**
	 * create_buffer - create wl_buffer from pool
	 * @id: (none)
	 * @offset: (none)
	 * @width: (none)
	 * @height: (none)
	 * @stride: (none)
	 * @format: (none)
	 *
	 * Create a wl_buffer from the pool. The buffer is created a
	 * offset bytes into the pool and has width and height as
	 * specified. The stride arguments specifies the number of bytes
	 * from beginning of one row to the beginning of the next. The
	 * format is the pixel format of the buffer and must be one of
	 * those advertised through the wl_shm.format event.
	 *
	 * A buffer will keep a reference to the pool it was created from
	 * so it is valid to destroy the pool immediately after creating a
	 * buffer from it.
	 */
	void (*create_buffer)(struct wl_client *client,
			      struct wl_resource *resource,
			      uint32_t id,
			      int32_t offset,
			      int32_t width,
			      int32_t height,
			      int32_t stride,
			      uint32_t format);
	/**
	 * destroy - destroy the pool
	 *
	 * Destroy the pool.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * resize - change the size of the pool mapping
	 * @size: (none)
	 *
	 * This request will cause the server to remap the backing memory
	 * for the pool from the fd passed when the pool was creating but
	 * using the new size.
	 */
	void (*resize)(struct wl_client *client,
		       struct wl_resource *resource,
		       int32_t size);
};

#ifndef WL_SHM_ERROR_ENUM
#define WL_SHM_ERROR_ENUM
enum wl_shm_error {
	WL_SHM_ERROR_INVALID_FORMAT = 0,
	WL_SHM_ERROR_INVALID_STRIDE = 1,
	WL_SHM_ERROR_INVALID_FD = 2,
};
#endif /* WL_SHM_ERROR_ENUM */

#ifndef WL_SHM_FORMAT_ENUM
#define WL_SHM_FORMAT_ENUM
enum wl_shm_format {
	WL_SHM_FORMAT_ARGB8888 = 0,
	WL_SHM_FORMAT_XRGB8888 = 1,
};
#endif /* WL_SHM_FORMAT_ENUM */

/**
 * wl_shm - shared memory support
 * @create_pool: create a shm pool
 *
 * Support for shared memory buffers.
 */
struct wl_shm_interface {
	/**
	 * create_pool - create a shm pool
	 * @id: (none)
	 * @fd: (none)
	 * @size: (none)
	 *
	 * This creates wl_shm_pool object, which can be used to create
	 * shared memory based wl_buffer objects. The server will mmap size
	 * bytes of the passed fd, to use as backing memory for then pool.
	 */
	void (*create_pool)(struct wl_client *client,
			    struct wl_resource *resource,
			    uint32_t id,
			    int32_t fd,
			    int32_t size);
};

#define WL_SHM_FORMAT	0

static inline void
wl_shm_send_format(struct wl_resource *resource_, uint32_t format)
{
	wl_resource_post_event(resource_, WL_SHM_FORMAT, format);
}

/**
 * wl_buffer - content for a wl_surface
 * @destroy: destroy a buffer
 *
 * A buffer provides the content for a wl_surface. Buffers are created
 * through factory interfaces such as wl_drm, wl_shm or similar. It has a
 * width and a height and can be attached to a wl_surface, but the
 * mechanism by which a client provides and updates the contents is defined
 * by the buffer factory interface.
 */
struct wl_buffer_interface {
	/**
	 * destroy - destroy a buffer
	 *
	 * Destroy a buffer. If and how you need to release the backing
	 * storage is defined by the buffer factory interface.
	 *
	 * For possible side-effects to a surface, see wl_surface.attach.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
};

#define WL_BUFFER_RELEASE	0

static inline void
wl_buffer_send_release(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_BUFFER_RELEASE);
}

/**
 * wl_data_offer - offer to transfer data
 * @accept: accept one of the offered mime-types
 * @receive: request that the data is transferred
 * @destroy: (none)
 *
 * A wl_data_offer represents a piece of data offered for transfer by
 * another client (the source client). It is used by the copy-and-paste and
 * drag-and-drop mechanisms. The offer describes the different mime types
 * that the data can be converted to and provides the mechanism for
 * transferring the data directly from the source client.
 */
struct wl_data_offer_interface {
	/**
	 * accept - accept one of the offered mime-types
	 * @serial: (none)
	 * @type: (none)
	 *
	 * Indicate that the client can accept the given mime-type, or
	 * NULL for not accepted. Use for feedback during drag and drop.
	 */
	void (*accept)(struct wl_client *client,
		       struct wl_resource *resource,
		       uint32_t serial,
		       const char *type);
	/**
	 * receive - request that the data is transferred
	 * @mime_type: (none)
	 * @fd: (none)
	 *
	 * To transfer the offered data, the client issues this request
	 * and indicates the mime-type it wants to receive. The transfer
	 * happens through the passed fd (typically a pipe(7) file
	 * descriptor). The source client writes the data in the mime-type
	 * representation requested and then closes the fd. The receiving
	 * client reads from the read end of the pipe until EOF and the
	 * closes its end, at which point the transfer is complete.
	 */
	void (*receive)(struct wl_client *client,
			struct wl_resource *resource,
			const char *mime_type,
			int32_t fd);
	/**
	 * destroy - (none)
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
};

#define WL_DATA_OFFER_OFFER	0

static inline void
wl_data_offer_send_offer(struct wl_resource *resource_, const char *type)
{
	wl_resource_post_event(resource_, WL_DATA_OFFER_OFFER, type);
}

/**
 * wl_data_source - offer to transfer data
 * @offer: add an offered mime type
 * @destroy: destroy the data source
 *
 * The wl_data_source object is the source side of a wl_data_offer. It is
 * created by the source client in a data transfer and provides a way to
 * describe the offered data and a way to respond to requests to transfer
 * the data.
 */
struct wl_data_source_interface {
	/**
	 * offer - add an offered mime type
	 * @type: (none)
	 *
	 * This request adds a mime-type to the set of mime-types
	 * advertised to targets. Can be called several times to offer
	 * multiple types.
	 */
	void (*offer)(struct wl_client *client,
		      struct wl_resource *resource,
		      const char *type);
	/**
	 * destroy - destroy the data source
	 *
	 * Destroy the data source.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
};

#define WL_DATA_SOURCE_TARGET	0
#define WL_DATA_SOURCE_SEND	1
#define WL_DATA_SOURCE_CANCELLED	2

static inline void
wl_data_source_send_target(struct wl_resource *resource_, const char *mime_type)
{
	wl_resource_post_event(resource_, WL_DATA_SOURCE_TARGET, mime_type);
}

static inline void
wl_data_source_send_send(struct wl_resource *resource_, const char *mime_type, int32_t fd)
{
	wl_resource_post_event(resource_, WL_DATA_SOURCE_SEND, mime_type, fd);
}

static inline void
wl_data_source_send_cancelled(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_DATA_SOURCE_CANCELLED);
}

struct wl_data_device_interface {
	/**
	 * start_drag - start drag and drop operation
	 * @source: (none)
	 * @origin: (none)
	 * @icon: (none)
	 * @serial: (none)
	 *
	 * This request asks the compositor to start a drag and drop
	 * operation on behalf of the client.
	 *
	 * The source argument is the data source that provides the data
	 * for the eventual data transfer. If source is NULL, enter, leave
	 * and motion events are sent only to the client that initiated the
	 * drag and the client is expected to handle the data passing
	 * internally.
	 *
	 * The origin surface is the surface where the drag originates and
	 * the client must have an active implicit grab that matches the
	 * serial.
	 *
	 * The icon surface is an optional (can be nil) surface that
	 * provides an icon to be moved around with the cursor. Initially,
	 * the top-left corner of the icon surface is placed at the cursor
	 * hotspot, but subsequent wl_surface.attach request can move the
	 * relative position. Attach requests must be confirmed with
	 * wl_surface.commit as usual.
	 *
	 * The current and pending input regions of the icon wl_surface are
	 * cleared, and wl_surface.set_input_region is ignored until the
	 * wl_surface is no longer used as the icon surface. When the use
	 * as an icon ends, the the current and pending input regions
	 * become undefined, and the wl_surface is unmapped.
	 */
	void (*start_drag)(struct wl_client *client,
			   struct wl_resource *resource,
			   struct wl_resource *source,
			   struct wl_resource *origin,
			   struct wl_resource *icon,
			   uint32_t serial);
	/**
	 * set_selection - (none)
	 * @source: (none)
	 * @serial: (none)
	 */
	void (*set_selection)(struct wl_client *client,
			      struct wl_resource *resource,
			      struct wl_resource *source,
			      uint32_t serial);
};

#define WL_DATA_DEVICE_DATA_OFFER	0
#define WL_DATA_DEVICE_ENTER	1
#define WL_DATA_DEVICE_LEAVE	2
#define WL_DATA_DEVICE_MOTION	3
#define WL_DATA_DEVICE_DROP	4
#define WL_DATA_DEVICE_SELECTION	5

static inline void
wl_data_device_send_data_offer(struct wl_resource *resource_, struct wl_resource *id)
{
	wl_resource_post_event(resource_, WL_DATA_DEVICE_DATA_OFFER, id);
}

static inline void
wl_data_device_send_enter(struct wl_resource *resource_, uint32_t serial, struct wl_resource *surface, wl_fixed_t x, wl_fixed_t y, struct wl_resource *id)
{
	wl_resource_post_event(resource_, WL_DATA_DEVICE_ENTER, serial, surface, x, y, id);
}

static inline void
wl_data_device_send_leave(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_DATA_DEVICE_LEAVE);
}

static inline void
wl_data_device_send_motion(struct wl_resource *resource_, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
	wl_resource_post_event(resource_, WL_DATA_DEVICE_MOTION, time, x, y);
}

static inline void
wl_data_device_send_drop(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_DATA_DEVICE_DROP);
}

static inline void
wl_data_device_send_selection(struct wl_resource *resource_, struct wl_resource *id)
{
	wl_resource_post_event(resource_, WL_DATA_DEVICE_SELECTION, id);
}

/**
 * wl_data_device_manager - data transfer interface
 * @create_data_source: (none)
 * @get_data_device: (none)
 *
 * The wl_data_device_manager is a a singleton global object that
 * provides access to inter-client data transfer mechanisms such as copy
 * and paste and drag and drop. These mechanisms are tied to a wl_seat and
 * this interface lets a client get a wl_data_device corresponding to a
 * wl_seat.
 */
struct wl_data_device_manager_interface {
	/**
	 * create_data_source - (none)
	 * @id: (none)
	 */
	void (*create_data_source)(struct wl_client *client,
				   struct wl_resource *resource,
				   uint32_t id);
	/**
	 * get_data_device - (none)
	 * @id: (none)
	 * @seat: (none)
	 */
	void (*get_data_device)(struct wl_client *client,
				struct wl_resource *resource,
				uint32_t id,
				struct wl_resource *seat);
};

struct wl_shell_interface {
	/**
	 * get_shell_surface - (none)
	 * @id: (none)
	 * @surface: (none)
	 */
	void (*get_shell_surface)(struct wl_client *client,
				  struct wl_resource *resource,
				  uint32_t id,
				  struct wl_resource *surface);
};

#ifndef WL_SHELL_SURFACE_RESIZE_ENUM
#define WL_SHELL_SURFACE_RESIZE_ENUM
enum wl_shell_surface_resize {
	WL_SHELL_SURFACE_RESIZE_NONE = 0,
	WL_SHELL_SURFACE_RESIZE_TOP = 1,
	WL_SHELL_SURFACE_RESIZE_BOTTOM = 2,
	WL_SHELL_SURFACE_RESIZE_LEFT = 4,
	WL_SHELL_SURFACE_RESIZE_TOP_LEFT = 5,
	WL_SHELL_SURFACE_RESIZE_BOTTOM_LEFT = 6,
	WL_SHELL_SURFACE_RESIZE_RIGHT = 8,
	WL_SHELL_SURFACE_RESIZE_TOP_RIGHT = 9,
	WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT = 10,
};
#endif /* WL_SHELL_SURFACE_RESIZE_ENUM */

#ifndef WL_SHELL_SURFACE_TRANSIENT_ENUM
#define WL_SHELL_SURFACE_TRANSIENT_ENUM
enum wl_shell_surface_transient {
	WL_SHELL_SURFACE_TRANSIENT_INACTIVE = 0x1,
};
#endif /* WL_SHELL_SURFACE_TRANSIENT_ENUM */

#ifndef WL_SHELL_SURFACE_FULLSCREEN_METHOD_ENUM
#define WL_SHELL_SURFACE_FULLSCREEN_METHOD_ENUM
/**
 * wl_shell_surface_fullscreen_method - different method to set the
 *	surface fullscreen
 * @WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT: (none)
 * @WL_SHELL_SURFACE_FULLSCREEN_METHOD_SCALE: (none)
 * @WL_SHELL_SURFACE_FULLSCREEN_METHOD_DRIVER: (none)
 * @WL_SHELL_SURFACE_FULLSCREEN_METHOD_FILL: (none)
 *
 * Hints to indicate compositor how to deal with a conflict between the
 * dimensions for the surface and the dimensions of the output. As a hint
 * the compositor is free to ignore this parameter.
 *
 * "default" The client has no preference on fullscreen behavior, policies
 * are determined by compositor.
 *
 * "scale" The client prefers scaling by the compositor. Scaling would
 * always preserve surface's aspect ratio with surface centered on the
 * output
 *
 * "driver" The client wants to switch video mode to the smallest mode that
 * can fit the client buffer. If the sizes do not match the compositor must
 * add black borders.
 *
 * "fill" The surface is centered on the output on the screen with no
 * scaling. If the surface is of insufficient size the compositor must add
 * black borders.
 */
enum wl_shell_surface_fullscreen_method {
	WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT = 0,
	WL_SHELL_SURFACE_FULLSCREEN_METHOD_SCALE = 1,
	WL_SHELL_SURFACE_FULLSCREEN_METHOD_DRIVER = 2,
	WL_SHELL_SURFACE_FULLSCREEN_METHOD_FILL = 3,
};
#endif /* WL_SHELL_SURFACE_FULLSCREEN_METHOD_ENUM */

/**
 * wl_shell_surface - desktop style meta data interface
 * @pong: respond to a ping event
 * @move: (none)
 * @resize: (none)
 * @set_toplevel: make the surface a top level surface
 * @set_transient: make the surface a transient surface
 * @set_fullscreen: make the surface a fullscreen surface
 * @set_popup: make the surface a popup surface
 * @set_maximized: make the surface a maximized surface
 * @set_title: set surface title
 * @set_class: set surface class
 * @set_minimized: request minimize
 *
 * An interface implemented by a wl_surface. On server side the object is
 * automatically destroyed when the related wl_surface is destroyed. On
 * client side, wl_shell_surface_destroy() must be called before destroying
 * the wl_surface object.
 */
struct wl_shell_surface_interface {
	/**
	 * pong - respond to a ping event
	 * @serial: (none)
	 *
	 * A client must respond to a ping event with a pong request or
	 * the client may be deemed unresponsive.
	 */
	void (*pong)(struct wl_client *client,
		     struct wl_resource *resource,
		     uint32_t serial);
	/**
	 * move - (none)
	 * @seat: (none)
	 * @serial: (none)
	 */
	void (*move)(struct wl_client *client,
		     struct wl_resource *resource,
		     struct wl_resource *seat,
		     uint32_t serial);
	/**
	 * resize - (none)
	 * @seat: (none)
	 * @serial: (none)
	 * @edges: (none)
	 */
	void (*resize)(struct wl_client *client,
		       struct wl_resource *resource,
		       struct wl_resource *seat,
		       uint32_t serial,
		       uint32_t edges);
	/**
	 * set_toplevel - make the surface a top level surface
	 *
	 * Make the surface a toplevel window.
	 */
	void (*set_toplevel)(struct wl_client *client,
			     struct wl_resource *resource);
	/**
	 * set_transient - make the surface a transient surface
	 * @parent: (none)
	 * @x: (none)
	 * @y: (none)
	 * @flags: (none)
	 *
	 * Map the surface relative to an existing surface. The x and y
	 * arguments specify the locations of the upper left corner of the
	 * surface relative to the upper left corner of the parent surface.
	 * The flags argument controls overflow/clipping behaviour when the
	 * surface would intersect a screen edge, panel or such. And
	 * possibly whether the offset only determines the initial position
	 * or if the surface is locked to that relative position during
	 * moves.
	 */
	void (*set_transient)(struct wl_client *client,
			      struct wl_resource *resource,
			      struct wl_resource *parent,
			      int32_t x,
			      int32_t y,
			      uint32_t flags);
	/**
	 * set_fullscreen - make the surface a fullscreen surface
	 * @method: (none)
	 * @framerate: (none)
	 * @output: (none)
	 *
	 * Map the surface as a fullscreen surface. If an output
	 * parameter is given then the surface will be made fullscreen on
	 * that output. If the client does not specify the output then the
	 * compositor will apply its policy - usually choosing the output
	 * on which the surface has the biggest surface area.
	 *
	 * The client may specify a method to resolve a size conflict
	 * between the output size and the surface size - this is provided
	 * through the fullscreen_method parameter.
	 *
	 * The framerate parameter is used only when the fullscreen_method
	 * is set to "driver", to indicate the preferred framerate.
	 * framerate=0 indicates that the app does not care about
	 * framerate. The framerate is specified in mHz, that is framerate
	 * of 60000 is 60Hz.
	 *
	 * The compositor must reply to this request with a configure event
	 * with the dimensions for the output on which the surface will be
	 * made fullscreen.
	 */
	void (*set_fullscreen)(struct wl_client *client,
			       struct wl_resource *resource,
			       uint32_t method,
			       uint32_t framerate,
			       struct wl_resource *output);
	/**
	 * set_popup - make the surface a popup surface
	 * @seat: (none)
	 * @serial: (none)
	 * @parent: (none)
	 * @x: (none)
	 * @y: (none)
	 * @flags: (none)
	 *
	 * Popup surfaces. Will switch an implicit grab into owner-events
	 * mode, and grab will continue after the implicit grab ends
	 * (button released). Once the implicit grab is over, the popup
	 * grab continues until the window is destroyed or a mouse button
	 * is pressed in any other clients window. A click in any of the
	 * clients surfaces is reported as normal, however, clicks in other
	 * clients surfaces will be discarded and trigger the callback.
	 *
	 * TODO: Grab keyboard too, maybe just terminate on any click
	 * inside or outside the surface?
	 */
	void (*set_popup)(struct wl_client *client,
			  struct wl_resource *resource,
			  struct wl_resource *seat,
			  uint32_t serial,
			  struct wl_resource *parent,
			  int32_t x,
			  int32_t y,
			  uint32_t flags);
	/**
	 * set_maximized - make the surface a maximized surface
	 * @output: (none)
	 *
	 * A request from the client to notify the compositor the
	 * maximized operation. The compositor will reply with a configure
	 * event telling the expected new surface size. The operation is
	 * completed on the next buffer attach to this surface. A maximized
	 * client will fill the fullscreen of the output it is bound to,
	 * except the panel area. This is the main difference between a
	 * maximized shell surface and a fullscreen shell surface.
	 */
	void (*set_maximized)(struct wl_client *client,
			      struct wl_resource *resource,
			      struct wl_resource *output);
	/**
	 * set_title - set surface title
	 * @title: (none)
	 *
	 * 
	 */
	void (*set_title)(struct wl_client *client,
			  struct wl_resource *resource,
			  const char *title);
	/**
	 * set_class - set surface class
	 * @class_: (none)
	 *
	 * The surface class identifies the general class of applications
	 * to which the surface belongs. The class is the file name of the
	 * applications .desktop file (absolute path if non-standard
	 * location).
	 */
	void (*set_class)(struct wl_client *client,
			  struct wl_resource *resource,
			  const char *class_);
	/**
	 * set_minimized - request minimize
	 *
	 * A request from the client to notify the compositor that it
	 * wants to be minimized.
	 */
	void (*set_minimized)(struct wl_client *client,
			      struct wl_resource *resource);
};

#define WL_SHELL_SURFACE_PING	0
#define WL_SHELL_SURFACE_CONFIGURE	1
#define WL_SHELL_SURFACE_POPUP_DONE	2
#define WL_SHELL_SURFACE_MAXIMIZE	3
#define WL_SHELL_SURFACE_UNMAXIMIZE	4
#define WL_SHELL_SURFACE_MINIMIZE	5
#define WL_SHELL_SURFACE_UNMINIMIZE	6
#define WL_SHELL_SURFACE_CLOSE	7

static inline void
wl_shell_surface_send_ping(struct wl_resource *resource_, uint32_t serial)
{
	wl_resource_post_event(resource_, WL_SHELL_SURFACE_PING, serial);
}

static inline void
wl_shell_surface_send_configure(struct wl_resource *resource_, uint32_t edges, int32_t width, int32_t height)
{
	wl_resource_post_event(resource_, WL_SHELL_SURFACE_CONFIGURE, edges, width, height);
}

static inline void
wl_shell_surface_send_popup_done(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_SHELL_SURFACE_POPUP_DONE);
}

static inline void
wl_shell_surface_send_maximize(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_SHELL_SURFACE_MAXIMIZE);
}

static inline void
wl_shell_surface_send_unmaximize(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_SHELL_SURFACE_UNMAXIMIZE);
}

static inline void
wl_shell_surface_send_minimize(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_SHELL_SURFACE_MINIMIZE);
}

static inline void
wl_shell_surface_send_unminimize(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_SHELL_SURFACE_UNMINIMIZE);
}

static inline void
wl_shell_surface_send_close(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_SHELL_SURFACE_CLOSE);
}

/**
 * wl_surface - an onscreen surface
 * @destroy: delete surface
 * @attach: set the surface contents
 * @damage: mark part of the surface damaged
 * @frame: request repaint feedback
 * @set_opaque_region: set opaque region
 * @set_input_region: set input region
 * @commit: commit pending surface state
 * @set_buffer_transform: sets the buffer transformation
 *
 * A surface. This is an image that is displayed on the screen. It has a
 * location, size and pixel contents.
 */
struct wl_surface_interface {
	/**
	 * destroy - delete surface
	 *
	 * Deletes the surface and invalidates its object id.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * attach - set the surface contents
	 * @buffer: (none)
	 * @x: (none)
	 * @y: (none)
	 *
	 * Set the contents of a buffer into this surface. The x and y
	 * arguments specify the location of the new pending buffer's upper
	 * left corner, relative to the current buffer's upper left corner.
	 * In other words, the x and y, and the width and height of the
	 * wl_buffer together define in which directions the surface's size
	 * changes.
	 *
	 * Surface contents are double-buffered state, see
	 * wl_surface.commit.
	 *
	 * The initial surface contents are void; there is no content.
	 * wl_surface.attach assigns the given wl_buffer as the pending
	 * wl_buffer. wl_surface.commit makes the pending wl_buffer the new
	 * surface contents, and the size of the surface becomes the size
	 * of the wl_buffer. After commit, there is no pending buffer until
	 * the next attach.
	 *
	 * Committing a pending wl_buffer allows the compositor to read the
	 * pixels in the wl_buffer. The compositor may access the pixels at
	 * any time after the wl_surface.commit request. When the
	 * compositor will not access the pixels anymore, it will send the
	 * wl_buffer.release event. Only after receiving wl_buffer.release,
	 * the client may re-use the wl_buffer. A wl_buffer, that has been
	 * attached and then replaced by another attach instead of
	 * committed, will not receive a release event, and is not used by
	 * the compositor.
	 *
	 * Destroying the wl_buffer after wl_buffer.release does not change
	 * the surface contents. However, if the client destroys the
	 * wl_buffer before receiving wl_buffer.release, the surface
	 * contents become undefined immediately.
	 *
	 * Only if wl_surface.attach is sent with a nil wl_buffer, the
	 * following wl_surface.commit will remove the surface content.
	 */
	void (*attach)(struct wl_client *client,
		       struct wl_resource *resource,
		       struct wl_resource *buffer,
		       int32_t x,
		       int32_t y);
	/**
	 * damage - mark part of the surface damaged
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * This request is used to describe the regions where the pending
	 * buffer is different from the current surface contents, and where
	 * the surface therefore needs to be repainted. The pending buffer
	 * must be set by wl_surface.attach before sending damage. The
	 * compositor ignores the parts of the damage that fall outside of
	 * the surface.
	 *
	 * Damage is double-buffered state, see wl_surface.commit.
	 *
	 * The initial value for pending damage is empty: no damage.
	 * wl_surface.damage adds pending damage: the new pending damage is
	 * the union of old pending damage and the given rectangle.
	 * wl_surface.commit assigns pending damage as the current damage,
	 * and clears pending damage. The server will clear the current
	 * damage as it repaints the surface.
	 */
	void (*damage)(struct wl_client *client,
		       struct wl_resource *resource,
		       int32_t x,
		       int32_t y,
		       int32_t width,
		       int32_t height);
	/**
	 * frame - request repaint feedback
	 * @callback: (none)
	 *
	 * Request notification when the next frame is displayed. Useful
	 * for throttling redrawing operations, and driving animations. The
	 * frame request will take effect on the next wl_surface.commit.
	 * The notification will only be posted for one frame unless
	 * requested again.
	 *
	 * A server should avoid signalling the frame callbacks if the
	 * surface is not visible in any way, e.g. the surface is
	 * off-screen, or completely obscured by other opaque surfaces.
	 *
	 * A client can request a frame callback even without an attach,
	 * damage, or any other state changes. wl_surface.commit triggers a
	 * display update, so the callback event will arrive after the next
	 * output refresh where the surface is visible.
	 */
	void (*frame)(struct wl_client *client,
		      struct wl_resource *resource,
		      uint32_t callback);
	/**
	 * set_opaque_region - set opaque region
	 * @region: (none)
	 *
	 * This request sets the region of the surface that contains
	 * opaque content. The opaque region is an optimization hint for
	 * the compositor that lets it optimize out redrawing of content
	 * behind opaque regions. Setting an opaque region is not required
	 * for correct behaviour, but marking transparent content as opaque
	 * will result in repaint artifacts. The compositor ignores the
	 * parts of the opaque region that fall outside of the surface.
	 *
	 * Opaque region is double-buffered state, see wl_surface.commit.
	 *
	 * wl_surface.set_opaque_region changes the pending opaque region.
	 * wl_surface.commit copies the pending region to the current
	 * region. Otherwise the pending and current regions are never
	 * changed.
	 *
	 * The initial value for opaque region is empty. Setting the
	 * pending opaque region has copy semantics, and the wl_region
	 * object can be destroyed immediately. A nil wl_region causes the
	 * pending opaque region to be set to empty.
	 */
	void (*set_opaque_region)(struct wl_client *client,
				  struct wl_resource *resource,
				  struct wl_resource *region);
	/**
	 * set_input_region - set input region
	 * @region: (none)
	 *
	 * This request sets the region of the surface that can receive
	 * pointer and touch events. Input events happening outside of this
	 * region will try the next surface in the server surface stack.
	 * The compositor ignores the parts of the input region that fall
	 * outside of the surface.
	 *
	 * Input region is double-buffered state, see wl_surface.commit.
	 *
	 * wl_surface.set_input_region changes the pending input region.
	 * wl_surface.commit copies the pending region to the current
	 * region. Otherwise the pending and current regions are never
	 * changed, except cursor and icon surfaces are special cases, see
	 * wl_pointer.set_cursor and wl_data_device.start_drag.
	 *
	 * The initial value for input region is infinite. That means the
	 * whole surface will accept input. Setting the pending input
	 * region has copy semantics, and the wl_region object can be
	 * destroyed immediately. A nil wl_region causes the input region
	 * to be set to infinite.
	 */
	void (*set_input_region)(struct wl_client *client,
				 struct wl_resource *resource,
				 struct wl_resource *region);
	/**
	 * commit - commit pending surface state
	 *
	 * Surface state (input, opaque, and damage regions, attached
	 * buffers, etc.) is double-buffered. Protocol requests modify the
	 * pending state, as opposed to current state in use by the
	 * compositor. Commit request atomically applies all pending state,
	 * replacing the current state. After commit, the new pending state
	 * is as documented for each related request.
	 *
	 * On commit, a pending wl_buffer is applied first, all other state
	 * second. This means that all coordinates in double-buffered state
	 * are relative to the new wl_buffer coming into use, except for
	 * wl_surface.attach itself. If there is no pending wl_buffer, the
	 * coordinates are relative to the current surface contents.
	 *
	 * All requests that need a commit to become effective are
	 * documented to affect double-buffered state.
	 *
	 * Other interfaces may add further double-buffered surface state.
	 */
	void (*commit)(struct wl_client *client,
		       struct wl_resource *resource);
	/**
	 * set_buffer_transform - sets the buffer transformation
	 * @transform: (none)
	 *
	 * This request sets an optional transformation on how the
	 * compositor interprets the contents of the buffer attached to the
	 * surface. The accepted values for the transform parameter are the
	 * values for wl_output.transform.
	 *
	 * Buffer transform is double-buffered state, see
	 * wl_surface.commit.
	 *
	 * A newly created surface has its buffer transformation set to
	 * normal.
	 *
	 * The purpose of this request is to allow clients to render
	 * content according to the output transform, thus permiting the
	 * compositor to use certain optimizations even if the display is
	 * rotated. Using hardware overlays and scanning out a client
	 * buffer for fullscreen surfaces are examples of such
	 * optmizations. Those optimizations are highly dependent on the
	 * compositor implementation, so the use of this request should be
	 * considered on a case-by-case basis.
	 *
	 * Note that if the transform value includes 90 or 270 degree
	 * rotation, the width of the buffer will become the surface height
	 * and the height of the buffer will become the surface width.
	 * @since: 2
	 */
	void (*set_buffer_transform)(struct wl_client *client,
				     struct wl_resource *resource,
				     int32_t transform);
};

#define WL_SURFACE_ENTER	0
#define WL_SURFACE_LEAVE	1

static inline void
wl_surface_send_enter(struct wl_resource *resource_, struct wl_resource *output)
{
	wl_resource_post_event(resource_, WL_SURFACE_ENTER, output);
}

static inline void
wl_surface_send_leave(struct wl_resource *resource_, struct wl_resource *output)
{
	wl_resource_post_event(resource_, WL_SURFACE_LEAVE, output);
}

#ifndef WL_SEAT_CAPABILITY_ENUM
#define WL_SEAT_CAPABILITY_ENUM
/**
 * wl_seat_capability - seat capability bitmask
 * @WL_SEAT_CAPABILITY_POINTER: wl_pointer
 * @WL_SEAT_CAPABILITY_KEYBOARD: wl_keyboard
 * @WL_SEAT_CAPABILITY_TOUCH: wl_touch
 *
 * This is a bitmask of capabilities this seat has; if a member is set,
 * then it is present on the seat.
 */
enum wl_seat_capability {
	WL_SEAT_CAPABILITY_POINTER = 1,
	WL_SEAT_CAPABILITY_KEYBOARD = 2,
	WL_SEAT_CAPABILITY_TOUCH = 4,
};
#endif /* WL_SEAT_CAPABILITY_ENUM */

/**
 * wl_seat - seat
 * @get_pointer: return pointer object
 * @get_keyboard: return pointer object
 * @get_touch: return pointer object
 *
 * A group of keyboards, pointer (mice, for example) and touch devices .
 * This object is published as a global during start up, or when such a
 * device is hot plugged. A seat typically has a pointer and maintains a
 * keyboard_focus and a pointer_focus.
 */
struct wl_seat_interface {
	/**
	 * get_pointer - return pointer object
	 * @id: (none)
	 *
	 * The ID provided will be initialized to the wl_pointer
	 * interface for this seat.
	 */
	void (*get_pointer)(struct wl_client *client,
			    struct wl_resource *resource,
			    uint32_t id);
	/**
	 * get_keyboard - return pointer object
	 * @id: (none)
	 *
	 * The ID provided will be initialized to the wl_keyboard
	 * interface for this seat.
	 */
	void (*get_keyboard)(struct wl_client *client,
			     struct wl_resource *resource,
			     uint32_t id);
	/**
	 * get_touch - return pointer object
	 * @id: (none)
	 *
	 * The ID provided will be initialized to the wl_touch interface
	 * for this seat.
	 */
	void (*get_touch)(struct wl_client *client,
			  struct wl_resource *resource,
			  uint32_t id);
};

#define WL_SEAT_CAPABILITIES	0

static inline void
wl_seat_send_capabilities(struct wl_resource *resource_, uint32_t capabilities)
{
	wl_resource_post_event(resource_, WL_SEAT_CAPABILITIES, capabilities);
}

#ifndef WL_POINTER_BUTTON_STATE_ENUM
#define WL_POINTER_BUTTON_STATE_ENUM
/**
 * wl_pointer_button_state - physical button state
 * @WL_POINTER_BUTTON_STATE_RELEASED: button is not pressed
 * @WL_POINTER_BUTTON_STATE_PRESSED: button is pressed
 *
 * Describes the physical state of a button which provoked the button
 * event.
 */
enum wl_pointer_button_state {
	WL_POINTER_BUTTON_STATE_RELEASED = 0,
	WL_POINTER_BUTTON_STATE_PRESSED = 1,
};
#endif /* WL_POINTER_BUTTON_STATE_ENUM */

#ifndef WL_POINTER_AXIS_ENUM
#define WL_POINTER_AXIS_ENUM
/**
 * wl_pointer_axis - axis types
 * @WL_POINTER_AXIS_VERTICAL_SCROLL: (none)
 * @WL_POINTER_AXIS_HORIZONTAL_SCROLL: (none)
 *
 * 
 */
enum wl_pointer_axis {
	WL_POINTER_AXIS_VERTICAL_SCROLL = 0,
	WL_POINTER_AXIS_HORIZONTAL_SCROLL = 1,
};
#endif /* WL_POINTER_AXIS_ENUM */

struct wl_pointer_interface {
	/**
	 * set_cursor - set the pointer surface
	 * @serial: (none)
	 * @surface: (none)
	 * @hotspot_x: (none)
	 * @hotspot_y: (none)
	 *
	 * Set the pointer surface, i.e., the surface that contains the
	 * pointer image (cursor). This request only takes effect if the
	 * pointer focus for this device is one of the requesting client's
	 * surfaces or the surface parameter is the current pointer
	 * surface. If there was a previous surface set with this request
	 * it is replaced. If surface is NULL, the pointer image is hidden.
	 *
	 * The parameters hotspot_x and hotspot_y define the position of
	 * the pointer surface relative to the pointer location. Its
	 * top-left corner is always at (x, y) - (hotspot_x, hotspot_y),
	 * where (x, y) are the coordinates of the pointer location.
	 *
	 * On surface.attach requests to the pointer surface, hotspot_x and
	 * hotspot_y are decremented by the x and y parameters passed to
	 * the request. Attach must be confirmed by wl_surface.commit as
	 * usual.
	 *
	 * The hotspot can also be updated by passing the currently set
	 * pointer surface to this request with new values for hotspot_x
	 * and hotspot_y.
	 *
	 * The current and pending input regions of the wl_surface are
	 * cleared, and wl_surface.set_input_region is ignored until the
	 * wl_surface is no longer used as the cursor. When the use as a
	 * cursor ends, the current and pending input regions become
	 * undefined, and the wl_surface is unmapped.
	 */
	void (*set_cursor)(struct wl_client *client,
			   struct wl_resource *resource,
			   uint32_t serial,
			   struct wl_resource *surface,
			   int32_t hotspot_x,
			   int32_t hotspot_y);
};

#define WL_POINTER_ENTER	0
#define WL_POINTER_LEAVE	1
#define WL_POINTER_MOTION	2
#define WL_POINTER_BUTTON	3
#define WL_POINTER_AXIS	4

static inline void
wl_pointer_send_enter(struct wl_resource *resource_, uint32_t serial, struct wl_resource *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	wl_resource_post_event(resource_, WL_POINTER_ENTER, serial, surface, surface_x, surface_y);
}

static inline void
wl_pointer_send_leave(struct wl_resource *resource_, uint32_t serial, struct wl_resource *surface)
{
	wl_resource_post_event(resource_, WL_POINTER_LEAVE, serial, surface);
}

static inline void
wl_pointer_send_motion(struct wl_resource *resource_, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	wl_resource_post_event(resource_, WL_POINTER_MOTION, time, surface_x, surface_y);
}

static inline void
wl_pointer_send_button(struct wl_resource *resource_, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	wl_resource_post_event(resource_, WL_POINTER_BUTTON, serial, time, button, state);
}

static inline void
wl_pointer_send_axis(struct wl_resource *resource_, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	wl_resource_post_event(resource_, WL_POINTER_AXIS, time, axis, value);
}

#ifndef WL_KEYBOARD_KEYMAP_FORMAT_ENUM
#define WL_KEYBOARD_KEYMAP_FORMAT_ENUM
/**
 * wl_keyboard_keymap_format - keyboard mapping format
 * @WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1: (none)
 *
 * This enum specifies the format of the keymap provided to the client
 * with the wl_keyboard::keymap event.
 */
enum wl_keyboard_keymap_format {
	WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 = 1,
};
#endif /* WL_KEYBOARD_KEYMAP_FORMAT_ENUM */

#ifndef WL_KEYBOARD_KEY_STATE_ENUM
#define WL_KEYBOARD_KEY_STATE_ENUM
/**
 * wl_keyboard_key_state - physical key state
 * @WL_KEYBOARD_KEY_STATE_RELEASED: key is not pressed
 * @WL_KEYBOARD_KEY_STATE_PRESSED: key is pressed
 *
 * Describes the physical state of a key which provoked the key event.
 */
enum wl_keyboard_key_state {
	WL_KEYBOARD_KEY_STATE_RELEASED = 0,
	WL_KEYBOARD_KEY_STATE_PRESSED = 1,
};
#endif /* WL_KEYBOARD_KEY_STATE_ENUM */

#define WL_KEYBOARD_KEYMAP	0
#define WL_KEYBOARD_ENTER	1
#define WL_KEYBOARD_LEAVE	2
#define WL_KEYBOARD_KEY	3
#define WL_KEYBOARD_MODIFIERS	4

static inline void
wl_keyboard_send_keymap(struct wl_resource *resource_, uint32_t format, int32_t fd, uint32_t size)
{
	wl_resource_post_event(resource_, WL_KEYBOARD_KEYMAP, format, fd, size);
}

static inline void
wl_keyboard_send_enter(struct wl_resource *resource_, uint32_t serial, struct wl_resource *surface, struct wl_array *keys)
{
	wl_resource_post_event(resource_, WL_KEYBOARD_ENTER, serial, surface, keys);
}

static inline void
wl_keyboard_send_leave(struct wl_resource *resource_, uint32_t serial, struct wl_resource *surface)
{
	wl_resource_post_event(resource_, WL_KEYBOARD_LEAVE, serial, surface);
}

static inline void
wl_keyboard_send_key(struct wl_resource *resource_, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	wl_resource_post_event(resource_, WL_KEYBOARD_KEY, serial, time, key, state);
}

static inline void
wl_keyboard_send_modifiers(struct wl_resource *resource_, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
	wl_resource_post_event(resource_, WL_KEYBOARD_MODIFIERS, serial, mods_depressed, mods_latched, mods_locked, group);
}

#define WL_TOUCH_DOWN	0
#define WL_TOUCH_UP	1
#define WL_TOUCH_MOTION	2
#define WL_TOUCH_FRAME	3
#define WL_TOUCH_CANCEL	4

static inline void
wl_touch_send_down(struct wl_resource *resource_, uint32_t serial, uint32_t time, struct wl_resource *surface, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
	wl_resource_post_event(resource_, WL_TOUCH_DOWN, serial, time, surface, id, x, y);
}

static inline void
wl_touch_send_up(struct wl_resource *resource_, uint32_t serial, uint32_t time, int32_t id)
{
	wl_resource_post_event(resource_, WL_TOUCH_UP, serial, time, id);
}

static inline void
wl_touch_send_motion(struct wl_resource *resource_, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
	wl_resource_post_event(resource_, WL_TOUCH_MOTION, time, id, x, y);
}

static inline void
wl_touch_send_frame(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_TOUCH_FRAME);
}

static inline void
wl_touch_send_cancel(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WL_TOUCH_CANCEL);
}

#ifndef WL_OUTPUT_SUBPIXEL_ENUM
#define WL_OUTPUT_SUBPIXEL_ENUM
enum wl_output_subpixel {
	WL_OUTPUT_SUBPIXEL_UNKNOWN = 0,
	WL_OUTPUT_SUBPIXEL_NONE = 1,
	WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB = 2,
	WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR = 3,
	WL_OUTPUT_SUBPIXEL_VERTICAL_RGB = 4,
	WL_OUTPUT_SUBPIXEL_VERTICAL_BGR = 5,
};
#endif /* WL_OUTPUT_SUBPIXEL_ENUM */

#ifndef WL_OUTPUT_TRANSFORM_ENUM
#define WL_OUTPUT_TRANSFORM_ENUM
/**
 * wl_output_transform - transform from framebuffer to output
 * @WL_OUTPUT_TRANSFORM_NORMAL: (none)
 * @WL_OUTPUT_TRANSFORM_90: (none)
 * @WL_OUTPUT_TRANSFORM_180: (none)
 * @WL_OUTPUT_TRANSFORM_270: (none)
 * @WL_OUTPUT_TRANSFORM_FLIPPED: (none)
 * @WL_OUTPUT_TRANSFORM_FLIPPED_90: (none)
 * @WL_OUTPUT_TRANSFORM_FLIPPED_180: (none)
 * @WL_OUTPUT_TRANSFORM_FLIPPED_270: (none)
 *
 * This describes the transform that a compositor will apply to a surface
 * to compensate for the rotation or mirroring of an output device.
 *
 * The flipped values correspond to an initial flip around a vertical axis
 * followed by rotation.
 *
 * The purpose is mainly to allow clients render accordingly and tell the
 * compositor, so that for fullscreen surfaces, the compositor will still
 * be able to scan out directly from client surfaces.
 */
enum wl_output_transform {
	WL_OUTPUT_TRANSFORM_NORMAL = 0,
	WL_OUTPUT_TRANSFORM_90 = 1,
	WL_OUTPUT_TRANSFORM_180 = 2,
	WL_OUTPUT_TRANSFORM_270 = 3,
	WL_OUTPUT_TRANSFORM_FLIPPED = 4,
	WL_OUTPUT_TRANSFORM_FLIPPED_90 = 5,
	WL_OUTPUT_TRANSFORM_FLIPPED_180 = 6,
	WL_OUTPUT_TRANSFORM_FLIPPED_270 = 7,
};
#endif /* WL_OUTPUT_TRANSFORM_ENUM */

#ifndef WL_OUTPUT_MODE_ENUM
#define WL_OUTPUT_MODE_ENUM
/**
 * wl_output_mode - values for the flags bitfield in the mode event
 * @WL_OUTPUT_MODE_CURRENT: indicates this is the current mode
 * @WL_OUTPUT_MODE_PREFERRED: indicates this is the preferred mode
 *
 * 
 */
enum wl_output_mode {
	WL_OUTPUT_MODE_CURRENT = 0x1,
	WL_OUTPUT_MODE_PREFERRED = 0x2,
};
#endif /* WL_OUTPUT_MODE_ENUM */

#define WL_OUTPUT_GEOMETRY	0
#define WL_OUTPUT_MODE	1

static inline void
wl_output_send_geometry(struct wl_resource *resource_, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char *make, const char *model, int32_t transform)
{
	wl_resource_post_event(resource_, WL_OUTPUT_GEOMETRY, x, y, physical_width, physical_height, subpixel, make, model, transform);
}

static inline void
wl_output_send_mode(struct wl_resource *resource_, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
	wl_resource_post_event(resource_, WL_OUTPUT_MODE, flags, width, height, refresh);
}

/**
 * wl_region - region interface
 * @destroy: destroy region
 * @add: add rectangle to region
 * @subtract: subtract rectangle from region
 *
 * Region.
 */
struct wl_region_interface {
	/**
	 * destroy - destroy region
	 *
	 * Destroy the region. This will invalidate the object id.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * add - add rectangle to region
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * Add the specified rectangle to the region
	 */
	void (*add)(struct wl_client *client,
		    struct wl_resource *resource,
		    int32_t x,
		    int32_t y,
		    int32_t width,
		    int32_t height);
	/**
	 * subtract - subtract rectangle from region
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * Subtract the specified rectangle from the region
	 */
	void (*subtract)(struct wl_client *client,
			 struct wl_resource *resource,
			 int32_t x,
			 int32_t y,
			 int32_t width,
			 int32_t height);
};

#ifdef  __cplusplus
}
#endif

#endif