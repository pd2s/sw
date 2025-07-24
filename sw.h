#if !defined(SW_HEADER)
#define SW_HEADER

#if !defined(SW_WITH_DEBUG)
#define SW_WITH_DEBUG 0
#endif // !defined(SW_WITH_DEBUG)
#if !defined(SW_WITH_WAYLAND)
#define SW_WITH_WAYLAND 1
#endif // !defined(SW_WITH_WAYLAND)
#if !defined(SW_WITH_TEXT)
#define SW_WITH_TEXT 1
#endif // !defined(SW_WITH_TEXT)
#if !defined(SW_WITH_PNG)
#define SW_WITH_PNG 1
#endif // !defined(SW_WITH_PNG)
#if !defined(SW_WITH_SVG)
#define SW_WITH_SVG 1
#endif // !defined(SW_WITH_SVG)
#if !defined(SW_WITH_JPG)
#define SW_WITH_JPG 1
#endif // !defined(SW_WITH_JPG)
#if !defined(SW_WITH_TGA)
#define SW_WITH_TGA 1
#endif // !defined(SW_WITH_TGA)
#if !defined(SW_WITH_BMP)
#define SW_WITH_BMP 1
#endif // !defined(SW_WITH_BMP)
#if !defined(SW_WITH_PSD)
#define SW_WITH_PSD 1
#endif // !defined(SW_WITH_PSD)
#if !defined(SW_WITH_GIF)
#define SW_WITH_GIF 1
#endif // !defined(SW_WITH_GIF)
#if !defined(SW_WITH_HDR)
#define SW_WITH_HDR 1
#endif // !defined(SW_WITH_HDR)
#if !defined(SW_WITH_PIC)
#define SW_WITH_PIC 1
#endif // !defined(SW_WITH_PIC)
#if !defined(SW_WITH_PNM)
#define SW_WITH_PNM 1
#endif // !defined(SW_WITH_PNM)

// TODO: remove
//#define SW_IMPLEMENTATION
#if defined(_XOPEN_SOURCE)
#define SW__USER_XOPEN_SOURCE _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif // defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <uchar.h>
#include <poll.h>

#include <stdint.h>
#include <stddef.h>

#if !defined(SW_EXPORT)
#define SW_EXPORT static
#endif

#if !defined(SU_DEBUG)
#define SU_DEBUG SW_WITH_DEBUG
#endif // !defined(SU_DEBUG)
#include "su.h"

SU_STATIC_ASSERT(SW_WITH_WAYLAND); // TODO

typedef struct sw_state sw_state_t;

SW_EXPORT su_bool32_t sw_init(sw_state_t *);
SW_EXPORT void sw_fini(sw_state_t *);
SW_EXPORT void sw_set(sw_state_t *);

SW_EXPORT su_bool32_t sw_flush(sw_state_t *);
SW_EXPORT su_bool32_t sw_process(sw_state_t *);

#if SW_WITH_WAYLAND

typedef enum sw_wayland_pointer_button_state {
	SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED = 0,
	SW_WAYLAND_POINTER_BUTTON_STATE_PRESSED = 1,
} sw_wayland_pointer_button_state_t;

typedef enum sw_wayland_pointer_scroll_axis {
	SW_WAYLAND_POINTER_AXIS_VERTICAL_SCROLL,
	SW_WAYLAND_POINTER_AXIS_HORIZONTAL_SCROLL,
} sw_wayland_pointer_scroll_axis_t;

typedef enum sw_wayland_output_transform {
	SW_WAYLAND_OUTPUT_TRANSFORM_NORMAL,
	SW_WAYLAND_OUTPUT_TRANSFORM_90,
	SW_WAYLAND_OUTPUT_TRANSFORM_180,
	SW_WAYLAND_OUTPUT_TRANSFORM_270,
	SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED,
	SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED_90,
	SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED_180,
	SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED_270,
} sw_wayland_output_transform_t;

typedef struct sw_wayland_output sw_wayland_output_t;
typedef void (*sw_wayland_output_func_t)(sw_wayland_output_t *, sw_state_t *);
typedef sw_wayland_output_t *(*sw_wayland_output_create_func_t)(sw_state_t *);

SU_LLIST_DECLARE(sw_wayland_output_t);

struct sw_wayland_output {
    struct {
		sw_wayland_output_func_t destroy;
	} in;
	struct {
    	su_string_t name;
		int32_t scale, width, height;
		sw_wayland_output_transform_t transform;
	} out;
	struct {
		sw_state_t *sw;
		uint32_t wl_name;
		SU_PAD32;
		struct wl_output *wl_output;
	} private;
	SU_LLIST_STRUCT_FIELDS(sw_wayland_output_t);
};

typedef struct sw_wayland_pointer sw_wayland_pointer_t;
typedef void (*sw_wayland_pointer_func_t)(sw_wayland_pointer_t *, sw_state_t *);
typedef sw_wayland_pointer_t *(*sw_wayland_pointer_create_func_t)(sw_state_t *);

typedef struct sw_wayland_seat sw_wayland_seat_t;
typedef struct sw_wayland_surface sw_wayland_surface_t;

struct sw_wayland_pointer {
	struct {
		sw_wayland_pointer_func_t destroy;
	} in;
	struct {
		sw_wayland_seat_t *seat;
		sw_wayland_surface_t *focused_surface;
		int32_t pos_x, pos_y;
		uint32_t btn_code;
		sw_wayland_pointer_button_state_t btn_state;
		SU_PAD32;
		sw_wayland_pointer_scroll_axis_t scroll_axis;
		double scroll_vector_length;
	} out;
	struct {
		sw_state_t *sw;
		struct wl_pointer *wl_pointer;
		struct wp_cursor_shape_device_v1 *cursor_shape_device;
		uint32_t enter_serial, button_serial;
	} private;
};

SU_LLIST_DECLARE(sw_wayland_seat_t);

typedef void (*sw_wayland_seat_func_t)(sw_wayland_seat_t *, sw_state_t *);
typedef sw_wayland_seat_t *(*sw_wayland_seat_create_func_t)(sw_state_t *);

struct sw_wayland_seat {
    struct {
		sw_wayland_seat_func_t destroy;
		sw_wayland_pointer_create_func_t pointer_create;
	} in;
	struct {
		su_string_t name;
		sw_wayland_pointer_t *pointer;
	} out;
	struct {
		sw_state_t *sw;
		struct wl_seat *wl_seat;
		uint32_t wl_name;
		SU_PAD32;
	} private;
	SU_LLIST_STRUCT_FIELDS(sw_wayland_seat_t);
};

typedef enum sw_wayland_surface_layer_anchor {
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_NONE = 0,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_TOP = 1,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_BOTTOM = 2,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT = 4,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT = 8,

#define SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL (SW_WAYLAND_SURFACE_LAYER_ANCHOR_TOP | \
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_BOTTOM | SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT | \
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT)

} sw_wayland_surface_layer_anchor_t;

typedef enum sw_wayland_surface_layer_layer {
	SW_WAYLAND_SURFACE_LAYER_LAYER_BACKGROUND,
	SW_WAYLAND_SURFACE_LAYER_LAYER_BOTTOM,
	SW_WAYLAND_SURFACE_LAYER_LAYER_TOP,
	SW_WAYLAND_SURFACE_LAYER_LAYER_OVERLAY,
} sw_wayland_surface_layer_layer_t;

typedef enum sw_wayland_cursor_shape {
	SW_WAYLAND_CURSOR_SHAPE_DEFAULT = 0, // SW_WAYLAND_CURSOR_SHAPE_DEFAULT_
	SW_WAYLAND_CURSOR_SHAPE_DEFAULT_ = 1,
	SW_WAYLAND_CURSOR_SHAPE_CONTEXT_MENU = 2,
	SW_WAYLAND_CURSOR_SHAPE_HELP = 3,
	SW_WAYLAND_CURSOR_SHAPE_POINTER = 4,
	SW_WAYLAND_CURSOR_SHAPE_PROGRESS = 5,
	SW_WAYLAND_CURSOR_SHAPE_WAIT = 6,
	SW_WAYLAND_CURSOR_SHAPE_CELL = 7,
	SW_WAYLAND_CURSOR_SHAPE_CROSSHAIR = 8,
	SW_WAYLAND_CURSOR_SHAPE_TEXT = 9,
	SW_WAYLAND_CURSOR_SHAPE_VERTICAL_TEXT = 10,
	SW_WAYLAND_CURSOR_SHAPE_ALIAS = 11,
	SW_WAYLAND_CURSOR_SHAPE_COPY = 12,
	SW_WAYLAND_CURSOR_SHAPE_MOVE = 13,
	SW_WAYLAND_CURSOR_SHAPE_NO_DROP = 14,
	SW_WAYLAND_CURSOR_SHAPE_NOT_ALLOWED = 15,
	SW_WAYLAND_CURSOR_SHAPE_GRAB = 16,
	SW_WAYLAND_CURSOR_SHAPE_GRABBING = 17,
	SW_WAYLAND_CURSOR_SHAPE_E_RESIZE = 18,
	SW_WAYLAND_CURSOR_SHAPE_N_RESIZE = 19,
	SW_WAYLAND_CURSOR_SHAPE_NE_RESIZE = 20,
	SW_WAYLAND_CURSOR_SHAPE_NW_RESIZE = 21,
	SW_WAYLAND_CURSOR_SHAPE_S_RESIZE = 22,
	SW_WAYLAND_CURSOR_SHAPE_SE_RESIZE = 23,
	SW_WAYLAND_CURSOR_SHAPE_SW_RESIZE = 24,
	SW_WAYLAND_CURSOR_SHAPE_W_RESIZE = 25,
	SW_WAYLAND_CURSOR_SHAPE_EW_RESIZE = 26,
	SW_WAYLAND_CURSOR_SHAPE_NS_RESIZE = 27,
	SW_WAYLAND_CURSOR_SHAPE_NESW_RESIZE = 28,
	SW_WAYLAND_CURSOR_SHAPE_NWSE_RESIZE = 29,
	SW_WAYLAND_CURSOR_SHAPE_COL_RESIZE = 30,
	SW_WAYLAND_CURSOR_SHAPE_ROW_RESIZE = 31,
	SW_WAYLAND_CURSOR_SHAPE_ALL_SCROLL = 32,
	SW_WAYLAND_CURSOR_SHAPE_ZOOM_IN = 33,
	SW_WAYLAND_CURSOR_SHAPE_ZOOM_OUT = 34,
	//SW_WAYLAND_CURSOR_SHAPE_DND_ASK = 35,
	//SW_WAYLAND_CURSOR_SHAPE_ALL_RESIZE = 36,
} sw_wayland_cursor_shape_t;

typedef struct sw_wayland_region sw_wayland_region_t;
SU_LLIST_DECLARE(sw_wayland_region_t);

struct sw_wayland_region {
    struct {
		int32_t x, y;
		int32_t width, height;
	} in;
	SU_LLIST_STRUCT_FIELDS(sw_wayland_region_t);
};

typedef enum sw_wayland_surface_popup_gravity {
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_NONE = 0,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_TOP = 1,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM = 2,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_LEFT = 3,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_RIGHT = 4,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_TOP_LEFT = 5,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM_LEFT = 6,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_TOP_RIGHT = 7,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM_RIGHT = 8,
} sw_wayland_surface_popup_gravity_t;

typedef enum sw_wayland_surface_popup_constraint_adjustment {
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_NONE = 0,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_SLIDE_X = 1,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_SLIDE_Y = 2,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_X = 4,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_Y = 8,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_RESIZE_X = 16,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_RESIZE_Y = 32,
} sw_wayland_surface_popup_constraint_adjustment_t;

typedef enum sw_wayland_surface_type {
	SW_WAYLAND_SURFACE_TYPE_LAYER,
	SW_WAYLAND_SURFACE_TYPE_POPUP,
} sw_wayland_surface_type_t;

typedef enum sw_wayland_surface_status {
	SW_WAYLAND_SURFACE_STATUS_ALIVE,
	SW_WAYLAND_SURFACE_STATUS_CLOSED,
	SW_WAYLAND_SURFACE_STATUS_ERROR,
} sw_wayland_surface_status_t;

typedef union pixman_image pixman_image_t;

struct sw__wayland_surface_buffer {
	struct wl_buffer *wl_buffer;
	pixman_image_t *image; // bits
	uint32_t *pixels;
	uint32_t size;
	su_bool32_t busy;
};

typedef struct sw_layout_block sw_layout_block_t;

typedef void (*sw_wayland_surface_func_t)(sw_wayland_surface_t *, sw_state_t *);

SU_LLIST_DECLARE(sw_wayland_surface_t);

struct sw_wayland_surface {
	struct {
		sw_wayland_surface_status_t status;
		int32_t width, height, scale;
		sw_wayland_surface_func_t fini;
	} out;
    struct {
		union {
			struct {
				sw_wayland_output_t *output;
    			int32_t exclusive_zone;
    			uint32_t anchor; // sw_wayland_surface_layer_anchor_t |
    			sw_wayland_surface_layer_layer_t layer;
				union {
					struct {
						int32_t margin_top;
						int32_t margin_right;
						int32_t margin_bottom;
						int32_t margin_left;
					};
					struct {
						int32_t margins[4]; // top, right, bottom, left
					};
				};
				SU_PAD32;
			} layer;
			struct {
    			int32_t x, y;
    			sw_wayland_surface_popup_gravity_t gravity;
    			uint32_t constraint_adjustment; // sw_wayland_surface_popup_constraint_adjustment_t |
				sw_wayland_pointer_t *grab;
			} popup;
		};
		sw_wayland_surface_type_t type;
    	sw_wayland_cursor_shape_t cursor_shape;
		int32_t width, height;
		// ? TODO: prepare, prepared cbs
		sw_wayland_surface_func_t error;
		sw_wayland_surface_func_t destroy; // must call out.fini, if not NULL
		sw_wayland_pointer_func_t enter;
		sw_wayland_pointer_func_t leave;
		sw_wayland_pointer_func_t motion;
		sw_wayland_pointer_func_t button;
		sw_wayland_pointer_func_t scroll;
    	su_llist__sw_wayland_region_t__t input_regions;
    	su_llist__sw_wayland_surface_t__t popups;
    	sw_layout_block_t *root;
	} in;
	struct {
		sw_state_t *sw;
		struct wl_surface *wl_surface;
		struct sw__wayland_surface_buffer buffer;
		su_bool32_t dirty;
		sw_wayland_cursor_shape_t cursor_shape;
		sw_wayland_output_t *output;
		union {
			struct {
				struct zwlr_layer_surface_v1 *layer_surface;
    			int32_t exclusive_zone;
    			uint32_t anchor; // sw_wayland_surface_layer_anchor_t |
    			sw_wayland_surface_layer_layer_t layer;
				union {
					struct {
						int32_t margin_top;
						int32_t margin_right;
						int32_t margin_bottom;
						int32_t margin_left;
					};
					struct {
						int32_t margins[4]; // top, right, bottom, left
					};
				};
				SU_PAD32;
			} layer;
			struct {
				sw_wayland_surface_t *parent;
				sw_wayland_pointer_t *grab;
				struct xdg_surface *xdg_surface;
				struct xdg_popup *xdg_popup;
				struct xdg_positioner *xdg_positioner;
				int32_t x, y;
    			sw_wayland_surface_popup_gravity_t gravity;
    			uint32_t constraint_adjustment; // sw_wayland_surface_popup_constraint_adjustment_t |
			} popup;
		};
	} private;
	SU_LLIST_STRUCT_FIELDS(sw_wayland_surface_t);
};

#endif // SW_WITH_WAYLAND

typedef union sw_color {
	struct {
		uint32_t u32;
	};
	struct {
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	};
} sw_color_t;

typedef enum sw_layout_block_type {
	SW_LAYOUT_BLOCK_TYPE_SPACER = 0,
#if SW_WITH_TEXT
	SW_LAYOUT_BLOCK_TYPE_TEXT = 1,
#endif // SW_WITH_TEXT 
	SW_LAYOUT_BLOCK_TYPE_IMAGE = 2,
	SW_LAYOUT_BLOCK_TYPE_COMPOSITE = 3,
} sw_layout_block_type_t;

typedef enum sw_layout_block_anchor {
	SW_LAYOUT_BLOCK_ANCHOR_LEFT, // top in vertical layouts
	SW_LAYOUT_BLOCK_ANCHOR_TOP, // left in horizontal layouts
	SW_LAYOUT_BLOCK_ANCHOR_RIGHT, // bottom in vertical layouts
	SW_LAYOUT_BLOCK_ANCHOR_BOTTOM, // right in horizontal layouts
	SW_LAYOUT_BLOCK_ANCHOR_CENTER,
	SW_LAYOUT_BLOCK_ANCHOR_NONE, // use x and y to position
} sw_layout_block_anchor_t;

typedef enum sw_layout_block_content_anchor {
	SW_LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_CENTER,
	SW_LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_TOP,
	SW_LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_BOTTOM,
	SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_TOP,
	SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER,
	SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_BOTTOM,
	SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_TOP,
	SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_CENTER,
	SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_BOTTOM,
} sw_layout_block_content_anchor_t;

typedef enum sw_layout_block_content_transform {
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_NORMAL,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_90,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_180,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_270,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_90,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_180,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_270,
} sw_layout_block_content_transform_t;

typedef enum sw_layout_block_expand {
	SW_LAYOUT_BLOCK_EXPAND_NONE = 0,
	SW_LAYOUT_BLOCK_EXPAND_LEFT = (1 << 1),
	SW_LAYOUT_BLOCK_EXPAND_RIGHT = (1 << 2),
	SW_LAYOUT_BLOCK_EXPAND_BOTTOM = (1 << 3),
	SW_LAYOUT_BLOCK_EXPAND_TOP = (1 << 4),
	SW_LAYOUT_BLOCK_EXPAND_CONTENT = (1 << 5),

#define SW_LAYOUT_BLOCK_EXPAND_ALL_SIDES \
	(SW_LAYOUT_BLOCK_EXPAND_LEFT | SW_LAYOUT_BLOCK_EXPAND_RIGHT \
	| SW_LAYOUT_BLOCK_EXPAND_BOTTOM | SW_LAYOUT_BLOCK_EXPAND_TOP)

#define SW_LAYOUT_BLOCK_EXPAND_ALL_SIDES_CONTENT \
	(SW_LAYOUT_BLOCK_EXPAND_LEFT | SW_LAYOUT_BLOCK_EXPAND_RIGHT \
	| SW_LAYOUT_BLOCK_EXPAND_BOTTOM | SW_LAYOUT_BLOCK_EXPAND_TOP \
	| SW_LAYOUT_BLOCK_EXPAND_CONTENT)

} sw_layout_block_expand_t;

typedef struct sw_layout_block_border {
	struct {
		int32_t width;
		sw_color_t color;
	} in;
} sw_layout_block_border_t;

typedef enum sw_layout_block_composite_children_layout {
	SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_HORIZONTAL,
	SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_VERTICAL,
} sw_layout_block_composite_children_layout_t;

typedef struct sw_layout_block_dimensions {
	int32_t x, y, width, height;
	int32_t content_width, content_height;
	union {
		struct {
			int32_t border_left;
			int32_t border_right;
			int32_t border_bottom;
			int32_t border_top;
		};
		struct {
			int32_t borders[4]; // left, right, bottom, top
		};
	};
} sw_layout_block_dimensions_t;

typedef enum sw_layout_block_status {
	SW_LAYOUT_BLOCK_STATUS_ALIVE,
	SW_LAYOUT_BLOCK_STATUS_ERROR,
} sw_layout_block_status_t;

#if SW_WITH_TEXT
typedef struct sw_layout_block_text {
	su_string_t text;
	su_array__su_string_t__t font_names;
	sw_color_t color;
	SU_PAD32;
} sw_layout_block_text_t;
#endif // SW_WITH_TEXT

typedef struct sw_pixmap {
	uint32_t width;
	uint32_t height;
	uint32_t pixels[]; // ARGB32
} sw_pixmap_t;

typedef enum sw_layout_block_image_image_type {
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SW_PIXMAP = 1, // see sw_pixmap_t
#if SW_WITH_PNG
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNG = 2,
#endif // SW_WITH_PNG
#if SW_WITH_JPG
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_JPG = 3,
#endif // SW_WITH_JPG
#if SW_WITH_SVG
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SVG = 4,
#endif // SW_WITH_SVG
#if SW_WITH_TGA
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_TGA = 5,
#endif // SW_WITH_TGA
#if SW_WITH_BMP
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_BMP = 6,
#endif // SW_WITH_BMP
#if SW_WITH_PSD
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PSD = 7,
#endif // SW_WITH_PSD
#if SW_WITH_GIF
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_GIF = 8,
#endif // SW_WITH_GIF
#if SW_WITH_HDR
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_HDR = 9,
#endif // SW_WITH_HDR
#if SW_WITH_PIC
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PIC = 10,
#endif // SW_WITH_PIC
#if SW_WITH_PNM
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNM = 11,
#endif // SW_WITH_PNM
} sw_layout_block_image_image_type_t;

typedef enum sw_layout_block_image_image_format {
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH,
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_MEMORY,
} sw_layout_block_image_image_format_t;

typedef struct sw_layout_block_image {
	sw_layout_block_image_image_format_t format;
	sw_layout_block_image_image_type_t type;
	union {
		struct {
			su_string_t path;
		};
		struct {
			void *data;
			size_t len;
		};
	};
} sw_layout_block_image_t;

SU_LLIST_DECLARE(sw_layout_block_t);

typedef struct sw_layout_block_composite {
	su_llist__sw_layout_block_t__t children;
	sw_layout_block_composite_children_layout_t layout;
	SU_PAD32;
} sw_layout_block_composite_t;

typedef void (*sw_layout_block_func_t)(sw_layout_block_t *, sw_state_t *);
typedef su_bool32_t (*sw_layout_block_prepare_func_t)(sw_layout_block_t *, sw_state_t *);

struct sw_layout_block {
	struct {
		union {
			// spacer;
#if SW_WITH_TEXT
			sw_layout_block_text_t text;
#endif // SW_WITH_TEXT
			sw_layout_block_image_t image;
			sw_layout_block_composite_t composite;
		};
		sw_layout_block_type_t type;
		sw_layout_block_anchor_t anchor;
		sw_color_t color;
		uint32_t expand; // sw_layout_block_expand_t |
		int32_t x, y;
		int32_t min_width, max_width;
		int32_t min_height, max_height;
		int32_t content_width, content_height;
		sw_layout_block_content_anchor_t content_anchor;
		sw_layout_block_content_transform_t content_transform;
		union {
			struct {
				sw_layout_block_border_t border_left;
				sw_layout_block_border_t border_right;
				sw_layout_block_border_t border_bottom;
				sw_layout_block_border_t border_top;
			};
			struct {
				sw_layout_block_border_t borders[4]; // left, right, bottom, top
			};
		};
		sw_layout_block_prepare_func_t prepare;
		sw_layout_block_prepare_func_t prepared;
		sw_layout_block_func_t error;
	} in;
	struct {
		sw_layout_block_status_t status;
		SU_PAD32;
		sw_layout_block_dimensions_t dim;
		sw_layout_block_func_t fini;
	} out;
	struct {
		sw_state_t *sw;
		pixman_image_t *content_image; // bits
		pixman_image_t *color; // solid fill
		pixman_image_t *border_left_color; // solid fill
		pixman_image_t *border_right_color; // solid fill
		pixman_image_t *border_bottom_color; // solid fill
		pixman_image_t *border_top_color; // solid fill
	} private;
	SU_LLIST_STRUCT_FIELDS(sw_layout_block_t);
};

#if SW_WITH_GIF
struct sw__image_gif_frame {
	pixman_image_t *image; // bits
	uint16_t delay; // ms
	SU_PAD16;
	SU_PAD32;
};

typedef struct sw__image_gif_frame struct_sw__image_gif_frame;
SU_ARRAY_DECLARE(struct_sw__image_gif_frame);

struct sw__image_multiframe_gif {
	su_array__struct_sw__image_gif_frame__t frames;
	size_t frame_idx;
	int64_t frame_end; // absolute ms
};
#endif // SW_WITH_GIF

enum sw__image_data_type {
	SW__IMAGE_DATA_TYPE_NONE = 0,
#if SW_WITH_SVG
	SW__IMAGE_DATA_TYPE_SVG = 1,
#endif // SW_WITH_SVG
#if SW_WITH_GIF
	SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF = 2,
#endif // SW_WITH_GIF
};

typedef struct resvg_render_tree resvg_render_tree;

struct sw__image_data {
	enum sw__image_data_type type;
	SU_PAD32;
	#if SW_WITH_SVG || SW_WITH_GIF
	union {
#if SW_WITH_SVG
		resvg_render_tree *svg;
#endif // SW_WITH_SVG
#if SW_WITH_GIF
		struct sw__image_multiframe_gif *multiframe_gif;
#endif // SW_WITH_GIF
	};
#endif // SW_WITH_SVG || SW_WITH_GIF

	uint32_t *pixels;
	su_allocator_t *gp_alloc;
};

struct sw__image_cache {
	SU_HASH_TABLE_STRUCT_FIELDS(su_string_t)
	sw_layout_block_image_image_type_t type;
	SU_PAD32;
	struct timespec mtim_ts;
	pixman_image_t *image; // bits
};

typedef struct sw__image_cache struct_sw__image_cache;
SU_HASH_TABLE_DECLARE(struct_sw__image_cache, su_stbds_hash, su_string_equal, 16);

#if SW_WITH_TEXT
struct sw__text_run_cache_entry {
	struct fcft_font *font;
	struct fcft_text_run *text_run;
};

typedef struct sw__text_run_cache_entry struct_sw__text_run_cache_entry;
SU_ARRAY_DECLARE(struct_sw__text_run_cache_entry);

struct sw__text_run_cache {
	SU_HASH_TABLE_STRUCT_FIELDS(su_string_t)
	su_array__struct_sw__text_run_cache_entry__t items;
};

typedef struct sw__text_run_cache struct_sw__text_run_cache;
SU_HASH_TABLE_DECLARE(struct_sw__text_run_cache, su_stbds_hash, su_string_equal, 16);
#endif // SW_WITH_TEXT

typedef enum sw_backend_type {
	// TODO SW_BACKEND_TYPE_IMAGE,
#if SW_WITH_WAYLAND
	SW_BACKEND_TYPE_WAYLAND,
#endif // SW_WITH_WAYLAND
} sw_backend_type_t;

struct sw_state {
	struct {
		su_allocator_t *gp_alloc;
		su_allocator_t *scratch_alloc;
		SU_PAD32;
		sw_backend_type_t type;
		union {
			// TODO: generic window
#if SW_WITH_WAYLAND
			struct {
				sw_wayland_output_create_func_t output_create;
				sw_wayland_seat_create_func_t seat_create;
				su_llist__sw_wayland_surface_t__t layers;
			} wayland;
#endif // SW_WITH_WAYLAND
		};
	} in;
	struct {
		int64_t t; // absolute timeout in ms
		union {
#if SW_WITH_WAYLAND
			struct {
				struct pollfd pfd;
				su_llist__sw_wayland_output_t__t outputs;
				su_llist__sw_wayland_seat_t__t seats;
			} wayland;
#endif // SW_WITH_WAYLAND
		};
	} out;
	struct {
#if SW_WITH_WAYLAND
		struct {
			struct wl_display *display;
			struct wl_registry *registry;
			struct wl_compositor *compositor;
			struct wl_shm *shm;
			struct zwlr_layer_shell_v1 *layer_shell;
			struct xdg_wm_base *wm_base;
			struct wp_cursor_shape_manager_v1 *cursor_shape_manager;
		} wayland;
#endif // SW_WITH_WAYLAND
		su_hash_table__struct_sw__image_cache__t image_cache;
#if SW_WITH_TEXT
		su_hash_table__struct_sw__text_run_cache__t text_run_cache;
#endif // SW_WITH_TEXT
	} private;
};

#if defined(SW_STRIP_PREFIXES)

typedef sw_wayland_pointer_button_state_t wayland_pointer_button_state_t;
typedef sw_wayland_pointer_scroll_axis_t wayland_pointer_scroll_axis_t;
typedef sw_wayland_output_transform_t wayland_output_transform_t;

typedef sw_wayland_output_t wayland_output_t;
typedef sw_wayland_output_func_t wayland_output_func_t;
typedef sw_wayland_output_create_func_t wayland_output_create_func_t; 

typedef sw_wayland_pointer_t wayland_pointer_t;
typedef sw_wayland_pointer_func_t wayland_pointer_func_t;
typedef sw_wayland_pointer_create_func_t wayland_pointer_create_func_t;

typedef sw_wayland_seat_t wayland_seat_t;
typedef sw_wayland_seat_func_t wayland_seat_func_t;
typedef sw_wayland_seat_create_func_t wayland_seat_create_func_t;

typedef sw_wayland_surface_layer_anchor_t wayland_surface_layer_anchor_t;
typedef sw_wayland_surface_layer_layer_t wayland_surface_layer_layer_t;

typedef sw_wayland_cursor_shape_t wayland_cursor_shape_t;
typedef sw_wayland_region_t wayland_region_t;

typedef sw_wayland_surface_popup_gravity_t wayland_surface_popup_gravity_t;
typedef sw_wayland_surface_popup_constraint_adjustment_t wayland_surface_popup_constraint_adjustment_t;

typedef sw_wayland_surface_type_t wayland_surface_type_t;
typedef sw_wayland_surface_status_t wayland_surface_status_t;
typedef sw_wayland_surface_t wayland_surface_t;
typedef sw_wayland_surface_func_t wayland_surface_func_t;

typedef sw_color_t color_t;
typedef sw_pixmap_t pixmap_t;

typedef sw_layout_block_type_t layout_block_type_t;
typedef sw_layout_block_anchor_t layout_block_anchor_t;
typedef sw_layout_block_content_anchor_t layout_block_content_anchor_t;
typedef sw_layout_block_content_transform_t layout_block_content_transform_t;
typedef sw_layout_block_expand_t layout_block_expand_t;
typedef sw_layout_block_border_t layout_block_border_t;
typedef sw_layout_block_composite_children_layout_t layout_block_composite_children_layout_t;
typedef sw_layout_block_dimensions_t layout_block_dimensions_t;
typedef sw_layout_block_status_t layout_block_status_t;
typedef sw_layout_block_text_t layout_block_text_t;
typedef sw_layout_block_image_image_type_t layout_block_image_image_type_t;
typedef sw_layout_block_image_image_format_t layout_block_image_image_format_t;
typedef sw_layout_block_image_t layout_block_image_t;
typedef sw_layout_block_composite_t layout_block_composite_t;
typedef sw_layout_block_t layout_block_t;
typedef sw_layout_block_func_t layout_block_func_t;

typedef sw_backend_type_t backend_type_t;
typedef sw_state_t state_t;

//#define init sw_init
//#define fini sw_fini
//#define set sw_set
#define flush sw_flush
#define process sw_process


#endif // defined(SW_STRIP_PREFIXES)

#undef _XOPEN_SOURCE
#if defined(SW__USER_XOPEN_SOURCE)
#define _XOPEN_SOURCE SW__USER_XOPEN_SOURCE
#endif // defined(SW__USER_XOPEN_SOURCE)

#endif // SW_HEADER

#if defined(SW_IMPLEMENTATION) && !defined(SW__REIMPLEMENTATION_GUARD)
#define SW__REIMPLEMENTATION_GUARD

#if !defined(SU_IMPLEMENTATION)
#define SU_IMPLEMENTATION
#endif // !defined(SU_IMPLEMENTATION)
#include "su.h"

#if SU_HAS_INCLUDE(<pixman.h>)
#include <pixman.h>
#else
#include <pixman/pixman.h>
#endif // SU_HAS_INCLUDE(<pixman.h>)

#if SW_WITH_TEXT
#if SU_HAS_INCLUDE(<fcft.h>)
#include <fcft.h>
#else
#include <fcft/fcft.h>
#endif // SU_HAS_INCLUDE(<fcft.h>)
#endif // SW_WITH_TEXT

#if SW_WITH_SVG
#if SU_HAS_INCLUDE(<resvg.h>)
#include <resvg.h>
#elif SU_HAS_INCLUDE(<resvg/resvg.h>)
#include <resvg/resvg.h>
#else
#include "resvg.h"
#endif
#endif // SW_WITH_SVG

#if SW_WITH_WAYLAND

#if SU_HAS_INCLUDE(<wayland-client.h>)
#include <wayland-client.h>
#else
#include <wayland-client/wayland-client.h>
#endif // SU_HAS_INCLUDE(<wayland-client.h>)

#include "wlr-layer-shell-unstable-v1.h"
#include "xdg-shell.h"
#include "cursor-shape-v1.h"

SU_LLIST_DEFINE(sw_wayland_output_t)
SU_LLIST_DEFINE(sw_wayland_seat_t)
SU_LLIST_DEFINE(sw_wayland_region_t)
SU_LLIST_DEFINE(sw_wayland_surface_t)

#endif // SW_WITH_WAYLAND

SU_LLIST_DEFINE(sw_layout_block_t)

#if SW_WITH_GIF
SU_ARRAY_DEFINE(struct_sw__image_gif_frame)
#endif // SW_WITH_GIF

SU_HASH_TABLE_DEFINE(struct_sw__image_cache, su_stbds_hash, su_string_equal, 16)

#if SW_WITH_TEXT
SU_ARRAY_DEFINE(struct_sw__text_run_cache_entry)
SU_HASH_TABLE_DEFINE(struct_sw__text_run_cache, su_stbds_hash, su_string_equal, 16)
#endif // SW_WITH_TEXT

#if SW_WITH_PNG || SW_WITH_JPG || SW_WITH_TGA || SW_WITH_BMP || SW_WITH_PSD || SW_WITH_GIF || SW_WITH_HDR || SW_WITH_PIC || SW_WITH_PNM

static void *sw__malloc_stbi(size_t size);
static void sw__free_stbi(void *ptr);
static void *sw__realloc_sized_stbi(void *ptr, size_t old_size, size_t new_size);

#define STBI_ASSERT SU_ASSERT
#define STBI_MALLOC sw__malloc_stbi
#define STBI_FREE sw__free_stbi
#define STBI_REALLOC_SIZED sw__realloc_sized_stbi

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_THREAD_LOCALS

#if !SW_WITH_PNG
#define STBI_NO_PNG
#endif // !SW_WITH_PNG
#if !SW_WITH_JPG
#define STBI_NO_JPEG
#endif // !SW_WITH_JPG
#if !SW_WITH_TGA
#define STBI_NO_TGA
#endif // !SW_WITH_TGA
#if !SW_WITH_BMP
#define STBI_NO_BMP
#endif // !SW_WITH_BMP
#if !SW_WITH_PSD
#define STBI_NO_PSD
#endif // !SW_WITH_PSD
#if !SW_WITH_GIF
#define STBI_NO_GIF
#endif //  !SW_WITH_GIF
#if !SW_WITH_HDR
#define STBI_NO_HDR
#endif // !SW_WITH_HDR
#if !SW_WITH_PIC
#define STBI_NO_PIC
#endif // !SW_WITH_PIC
#if !SW_WITH_PNM
#define STBI_NO_PNM
#endif // !SW_WITH_PNM

SU_IGNORE_WARNINGS_START
SU_IGNORE_WARNING("-Wduplicated-branches")
SU_IGNORE_WARNING("-Warith-conversion")
SU_IGNORE_WARNING("-Wconversion")
SU_IGNORE_WARNING("-Wsuggest-attribute=pure")
SU_IGNORE_WARNING("-Walloc-zero")
SU_IGNORE_WARNING("-Wpadded")
SU_IGNORE_WARNING("-Wsign-conversion")
SU_IGNORE_WARNING("-Wcast-align")
SU_IGNORE_WARNING("-Wdouble-promotion")
SU_IGNORE_WARNING("-Wextra-semi-stmt")
SU_IGNORE_WARNING("-Wswitch-default")
SU_IGNORE_WARNING("-Wimplicit-int-conversion")
SU_IGNORE_WARNING("-Wimplicit-fallthrough")
SU_IGNORE_WARNING("-Wmissing-prototypes")
#include "stb_image.h"
SU_IGNORE_WARNINGS_END

#endif // SW_WITH_PNG || SW_WITH_JPG || SW_WITH_TGA || SW_WITH_BMP || SW_WITH_PSD || SW_WITH_GIF || SW_WITH_HDR || SW_WITH_PIC || SW_WITH_PNM

static su_allocator_t *sw__scratch_alloc; // TODO: remove

static void *sw__malloc_stbi(size_t size) {
	return sw__scratch_alloc->alloc(sw__scratch_alloc, size, SU_ALIGNOF(max_align_t));
}

static void sw__free_stbi(void *ptr) {
	sw__scratch_alloc->free(sw__scratch_alloc, ptr);
}

static void *sw__realloc_sized_stbi(void *ptr, size_t old_size, size_t new_size) {
	void *ret = sw__scratch_alloc->alloc(sw__scratch_alloc, new_size, SU_ALIGNOF(max_align_t));
	if (ptr) {
		memcpy(ret, ptr, SU_MIN(old_size, new_size));
		sw__scratch_alloc->free(sw__scratch_alloc, ptr);
	}
	return ret;
}

static void sw__update_t(sw_state_t *sw, int64_t t) {
	if ((sw->out.t == -1) || (t < sw->out.t)) {
		sw->out.t = t;
	}
}

static pixman_color_t sw__color_to_pixman_color(sw_color_t color) {
	color.r = (uint8_t)((((uint32_t)color.r * (uint32_t)color.a + 127) * 257) >> 16);
	color.g = (uint8_t)((((uint32_t)color.g * (uint32_t)color.a + 127) * 257) >> 16);
	color.b = (uint8_t)((((uint32_t)color.b * (uint32_t)color.a + 127) * 257) >> 16);
	return (pixman_color_t) {
		.alpha = (uint16_t)(color.a * 257),
		.red = (uint16_t)(color.r * 257),
		.green = (uint16_t)(color.g * 257),
		.blue = (uint16_t)(color.b * 257),
	};
}

static void sw__image_handle_destroy(pixman_image_t *image, void *data) {
	SU_NOTUSED(image);
	struct sw__image_data *image_data = data;
	switch (image_data->type) {
	case SW__IMAGE_DATA_TYPE_NONE:
		break;
#if SW_WITH_SVG
	case SW__IMAGE_DATA_TYPE_SVG:
		resvg_tree_destroy(image_data->svg);
		break;
#endif // SW_WITH_SVG
#if SW_WITH_GIF
	case SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF: {
		for (size_t i = 0; i < image_data->multiframe_gif->frames.len; ++i) {
			pixman_image_t *frame_image = su_array__struct_sw__image_gif_frame__get(
				&image_data->multiframe_gif->frames, i).image;
			struct sw__image_data *frame_data = pixman_image_get_destroy_data(frame_image);
			frame_data->type = SW__IMAGE_DATA_TYPE_NONE;
			pixman_image_unref(frame_image);
		}
		su_array__struct_sw__image_gif_frame__fini(&image_data->multiframe_gif->frames, image_data->gp_alloc);
		image_data->gp_alloc->free(image_data->gp_alloc, image_data->multiframe_gif);
		break;
	}
#endif // SW_WITH_GIF
	default:
		SU_ASSERT_UNREACHABLE;
	}

	image_data->gp_alloc->free(image_data->gp_alloc, image_data->pixels);

	image_data->gp_alloc->free(image_data->gp_alloc, image_data);
}

static pixman_image_t *sw__image_create( su_allocator_t *gp_alloc,
		int width, int height, struct sw__image_data **data_out) {
	SU_ASSERT(width > 0);
	SU_ASSERT(height > 0);

	struct sw__image_data *data = gp_alloc->alloc(gp_alloc, sizeof(*data), SU_ALIGNOF(*data));
	data->gp_alloc = gp_alloc;
	data->type = SW__IMAGE_DATA_TYPE_NONE;

	int stride = width * 4;
	size_t size = (size_t)height * (size_t)stride;
	data->pixels = gp_alloc->alloc(gp_alloc, size, 64);
	memset(data->pixels, 0, size);
	pixman_image_t *image = pixman_image_create_bits(PIXMAN_a8r8g8b8, width, height, data->pixels, stride);
	pixman_image_set_destroy_function(image, sw__image_handle_destroy, data);

	if (data_out) {
		*data_out = data;
	}
	return image;
}

static pixman_image_t *sw__load_pixmap_file(su_allocator_t *gp_alloc, su_string_t path) {
	SU_ASSERT(path.nul_terminated); // TODO: handle properly

	FILE *f = fopen(path.s, "r");
	if (f == NULL) {
		return NULL;
	}

	sw_pixmap_t pixmap;

	pixman_image_t *image = NULL;
	if (fread(&pixmap, 1, sizeof(pixmap), f) != sizeof(pixmap)) {
		errno = EINVAL;
		goto cleanup;
	}

	if ((pixmap.width == 0) || (pixmap.height == 0)) {
		errno = EINVAL;
		goto cleanup;
	}

	struct sw__image_data *image_data;
	image = sw__image_create(gp_alloc, (int)pixmap.width, (int)pixmap.height, &image_data);

	size_t size = pixmap.width * pixmap.height * 4;
	if (fread(image_data->pixels, 1, size, f) != size) {
		pixman_image_unref(image);
		image = NULL;
		errno = EINVAL;
		goto cleanup;
	}
	
	su_argb_premultiply_alpha(image_data->pixels, image_data->pixels, pixmap.width * pixmap.height);

cleanup:
	fclose(f);
    return image;
}

static pixman_image_t *sw__load_pixmap_memory(su_allocator_t *gp_alloc, void *data, size_t len) {
	if (len <= sizeof(sw_pixmap_t)) {
		goto error;
	}

	sw_pixmap_t pixmap;

	memcpy(&pixmap, data, sizeof(pixmap));

	if ((pixmap.width == 0) || (pixmap.height == 0)) {
		goto error;
	}

	struct sw__image_data *image_data;
	pixman_image_t *image = sw__image_create(gp_alloc, (int)pixmap.width, (int)pixmap.height, &image_data);

	su_argb_premultiply_alpha(image_data->pixels,
		(uint32_t *)(void *)((uint8_t *)data + sizeof(pixmap)), pixmap.width * pixmap.height);

    return image;
error:
	errno = EINVAL;
	return NULL;
}

#if SW_WITH_SVG
static pixman_image_t *sw__render_svg(su_allocator_t *gp_alloc, resvg_render_tree *tree,
		struct sw__image_data **image_data_out, int32_t target_width, int32_t target_height) {
	resvg_size image_size = resvg_get_image_size(tree); // ? TODO: resvg_get_image_bbox
	int32_t width = (int32_t)image_size.width;
	int32_t height = (int32_t)image_size.width;

	resvg_transform transform = resvg_transform_identity();
	if (target_width > 0) {
		width = target_width;
		transform.a = (float)target_width / image_size.width;
	}
	if (target_height > 0) {
		height = target_height;
		transform.d = (float)target_height / image_size.height;
	}

	SU_ASSERT(width > 0);
	SU_ASSERT(height > 0);

	struct sw__image_data *image_data;
	pixman_image_t *image = sw__image_create(gp_alloc, width, height, &image_data);

	resvg_render(tree, transform, (uint32_t)width, (uint32_t)height, (char *)image_data->pixels);

	su_abgr_to_argb(image_data->pixels, image_data->pixels, (size_t)width * (size_t)height);
	pixman_image_set_filter(image, PIXMAN_FILTER_BEST, NULL, 0);

	if (image_data_out) {
		*image_data_out = image_data;
	}
	return image;
}

static pixman_image_t *sw__load_svg(su_allocator_t *gp_alloc, sw_layout_block_image_image_format_t format,
		su_string_t path, void *data, size_t len) {
	SU_ASSERT(path.nul_terminated); // TODO: handle properly

	pixman_image_t *image = NULL;
	resvg_render_tree *tree = NULL;
	resvg_options *opt = resvg_options_create();
	int32_t ret = (format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH)
		? resvg_parse_tree_from_file(path.s, opt, &tree)
		: resvg_parse_tree_from_data(data, len, opt, &tree);
    if (ret == RESVG_OK) {
		struct sw__image_data *image_data;
		image = sw__render_svg(gp_alloc, tree, &image_data, -1, -1);
		image_data->type = SW__IMAGE_DATA_TYPE_SVG;
		image_data->svg = tree;
    } else if (tree) {
		resvg_tree_destroy(tree);
		errno = EINVAL;
	}

	if (opt) {
		resvg_options_destroy(opt);
	}

	return image;
}
#endif // SW_WITH_SVG

#if SW_WITH_PNG
static pixman_image_t *sw__load_png(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		sw_layout_block_image_image_format_t format, su_string_t path, void *data, size_t len) {
	FILE *f = NULL;
	stbi__context ctx;
	if (format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH) {
		SU_ASSERT(path.nul_terminated); // TODO: handle properly

		if (!(f = fopen(path.s, "rb"))) {
			return NULL;
		}

		stbi__start_file(&ctx, f);
	} else {
		stbi__start_mem(&ctx, data, (int)len);
	}

	stbi__result_info ri = {
		.bits_per_channel = 8,
	};

	pixman_image_t *image = NULL;
	if (stbi__png_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__png_load(&ctx, &width, &height, &unused, 4, &ri);
		if (src && (width > 0) && (height > 0)) {
			if (ri.bits_per_channel != 8) {
				SU_ASSERT(ri.bits_per_channel == 16);
				src = (uint32_t *)(void *)stbi__convert_16_to_8((stbi__uint16 *)src, width, height, 4);
			}
			struct sw__image_data *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	if (f) {
		fclose(f);
	}
	if (!image) {
		errno = EINVAL;
	}
	return image;
}
#endif // SW_WITH_PNG

#if SW_WITH_JPG
static pixman_image_t *sw__load_jpg(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		sw_layout_block_image_image_format_t format, su_string_t path, void *data, size_t len) {
	FILE *f = NULL;
	stbi__context ctx;
	if (format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH) {
		SU_ASSERT(path.nul_terminated); // TODO: handle properly

		if (!(f = fopen(path.s, "rb"))) {
			return NULL;
		}

		stbi__start_file(&ctx, f);
	} else {
		stbi__start_mem(&ctx, data, (int)len);
	}

	pixman_image_t *image = NULL;
	if (stbi__jpeg_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__jpeg_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			struct sw__image_data *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	if (f) {
		fclose(f);
	}
	if (!image) {
		errno = EINVAL;
	}
	return image;
}
#endif // SW_WITH_JPG

#if SW_WITH_TGA
static pixman_image_t *sw__load_tga(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		sw_layout_block_image_image_format_t format, su_string_t path, void *data, size_t len) {
	FILE *f = NULL;
	stbi__context ctx;
	if (format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH) {
		SU_ASSERT(path.nul_terminated); // TODO: handle properly

		if (!(f = fopen(path.s, "rb"))) {
			return NULL;
		}

		stbi__start_file(&ctx, f);
	} else {
		stbi__start_mem(&ctx, data, (int)len);
	}

	pixman_image_t *image = NULL;
	if (stbi__tga_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__tga_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			struct sw__image_data *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	if (f) {
		fclose(f);
	}
	if (!image) {
		errno = EINVAL;
	}
	return image;
}
#endif // SW_WITH_TGA

#if SW_WITH_BMP
static pixman_image_t *sw__load_bmp(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		sw_layout_block_image_image_format_t format, su_string_t path, void *data, size_t len) {
	FILE *f = NULL;
	stbi__context ctx;
	if (format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH) {
		SU_ASSERT(path.nul_terminated); // TODO: handle properly

		if (!(f = fopen(path.s, "rb"))) {
			return NULL;
		}

		stbi__start_file(&ctx, f);
	} else {
		stbi__start_mem(&ctx, data, (int)len);
	}

	pixman_image_t *image = NULL;
	if (stbi__bmp_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__bmp_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			struct sw__image_data *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	if (f) {
		fclose(f);
	}
	if (!image) {
		errno = EINVAL;
	}
	return image;
}
#endif // SW_WITH_BMP

#if SW_WITH_PSD
static pixman_image_t *sw__load_psd(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		sw_layout_block_image_image_format_t format, su_string_t path, void *data, size_t len) {
	FILE *f = NULL;
	stbi__context ctx;
	if (format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH) {
		SU_ASSERT(path.nul_terminated); // TODO: handle properly

		if (!(f = fopen(path.s, "rb"))) {
			return NULL;
		}

		stbi__start_file(&ctx, f);
	} else {
		stbi__start_mem(&ctx, data, (int)len);
	}

	stbi__result_info ri = {
		.bits_per_channel = 8,
	};

	pixman_image_t *image = NULL;
	if (stbi__psd_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__psd_load(&ctx, &width, &height, &unused, 4, &ri, 8);
		if (src && (width > 0) && (height > 0)) {
			if (ri.bits_per_channel != 8) {
				SU_ASSERT(ri.bits_per_channel == 16);
				src = (uint32_t *)(void *)stbi__convert_16_to_8((stbi__uint16 *)src, width, height, 4);
			}
			struct sw__image_data *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	if (f) {
		fclose(f);
	}
	if (!image) {
		errno = EINVAL;
	}
	return image;
}
#endif // SW_WITH_PSD

#if SW_WITH_GIF
static pixman_image_t *sw__load_gif(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		sw_layout_block_image_image_format_t format, su_string_t path, void *data, size_t len) {
	FILE *f = NULL;
	stbi__context ctx;
	if (format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH) {
		SU_ASSERT(path.nul_terminated); // TODO: handle properly

		if (!(f = fopen(path.s, "rb"))) {
			return NULL;
		}

		stbi__start_file(&ctx, f);
	} else {
		stbi__start_mem(&ctx, data, (int)len);
	}

	pixman_image_t *image = NULL;
	int width, height, unused, *frame_delays, frame_count;
	uint32_t *src = stbi__load_gif_main(&ctx, &frame_delays, &width, &height, &frame_count, &unused, 4);
	if (src && (width > 0) && (height > 0) && (frame_count > 0)) {
		if (frame_count > 1) {
			struct sw__image_multiframe_gif *multiframe_gif = gp_alloc->alloc(
				gp_alloc, sizeof(*multiframe_gif), SU_ALIGNOF(*multiframe_gif));
			multiframe_gif->frame_idx = 0;
			multiframe_gif->frame_end = 0;
			su_array__struct_sw__image_gif_frame__init(
				&multiframe_gif->frames, gp_alloc, (size_t)frame_count);

			for (int i = 0; i < frame_count; ++i) {
				struct sw__image_data *frame_data;
				su_array__struct_sw__image_gif_frame__add_nocheck(&multiframe_gif->frames,
					(struct sw__image_gif_frame){
						.image = sw__image_create(gp_alloc, width, height, &frame_data),
						.delay = (uint16_t)frame_delays[i],
				});

				size_t size = (size_t)width * (size_t)height * 4;
				su_abgr_to_argb_premultiply_alpha(frame_data->pixels,
					(uint32_t *)(void *)&((uint8_t *)src)[size * (size_t)i],
					(size_t)width * (size_t)height);
				frame_data->type = SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF;
				frame_data->multiframe_gif = multiframe_gif;
			}

			image = su_array__struct_sw__image_gif_frame__get(&multiframe_gif->frames, 0).image;
		} else {
			struct sw__image_data *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
	}
	scratch_alloc->free(scratch_alloc, src);
	scratch_alloc->free(scratch_alloc, frame_delays);

	if (f) {
		fclose(f);
	}
	if (!image) {
		errno = EINVAL;
	}
	return image;
}
#endif // SW_WITH_GIF

#if SW_WITH_HDR
static pixman_image_t *sw__load_hdr(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		sw_layout_block_image_image_format_t format, su_string_t path, void *data, size_t len) {
	FILE *f = NULL;
	stbi__context ctx;
	if (format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH) {
		SU_ASSERT(path.nul_terminated); // TODO: handle properly

		if (!(f = fopen(path.s, "rb"))) {
			return NULL;
		}

		stbi__start_file(&ctx, f);
	} else {
		stbi__start_mem(&ctx, data, (int)len);
	}

	pixman_image_t *image = NULL;
	if (stbi__hdr_test(&ctx)) {
		int width = 0, height = 0, unused;
		uint32_t *src = (uint32_t *)(void *)stbi__hdr_to_ldr(
			stbi__hdr_load(&ctx, &width, &height, &unused, 4, NULL),
			width, height, 4
		);
		if (src && (width > 0) && (height > 0)) {
			struct sw__image_data *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	if (f) {
		fclose(f);
	}
	if (!image) {
		errno = EINVAL;
	}
	return image;
}
#endif // SW_WITH_HDR

#if SW_WITH_PIC
static pixman_image_t *sw__load_pic(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		sw_layout_block_image_image_format_t format, su_string_t path, void *data, size_t len) {
	FILE *f = NULL;
	stbi__context ctx;
	if (format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH) {
		SU_ASSERT(path.nul_terminated); // TODO: handle properly

		if (!(f = fopen(path.s, "rb"))) {
			return NULL;
		}

		stbi__start_file(&ctx, f);
	} else {
		stbi__start_mem(&ctx, data, (int)len);
	}

	pixman_image_t *image = NULL;
	if (stbi__pic_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__pic_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			struct sw__image_data *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	if (f) {
		fclose(f);
	}
	if (!image) {
		errno = EINVAL;
	}
	return image;
}
#endif // SW_WITH_PIC

#if SW_WITH_PNM
static pixman_image_t *sw__load_pnm(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		sw_layout_block_image_image_format_t format, su_string_t path, void *data, size_t len) {
	FILE *f = NULL;
	stbi__context ctx;
	if (format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH) {
		SU_ASSERT(path.nul_terminated); // TODO: handle properly

		if (!(f = fopen(path.s, "rb"))) {
			return NULL;
		}

		stbi__start_file(&ctx, f);
	} else {
		stbi__start_mem(&ctx, data, (int)len);
	}

	stbi__result_info ri = {
		.bits_per_channel = 8,
	};

	pixman_image_t *image = NULL;
	if (stbi__pnm_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__pnm_load(&ctx, &width, &height, &unused, 4, &ri);
		if (src && (width > 0) && (height > 0)) {
			if (ri.bits_per_channel != 8) {
				SU_ASSERT(ri.bits_per_channel == 16);
				src = (uint32_t *)(void *)stbi__convert_16_to_8((stbi__uint16 *)src, width, height, 4);
			}
			struct sw__image_data *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	if (f) {
		fclose(f);
	}
	if (!image) {
		errno = EINVAL;
	}
	return image;
}
#endif // SW_WITH_PNM

static void sw__layout_block_fini(sw_layout_block_t *block, sw_state_t *sw) {
	SU_NOTUSED(sw);
	if (block->private.content_image) {
		pixman_image_unref(block->private.content_image);
	}
	if (block->private.color) {
		pixman_image_unref(block->private.color);
	}
	if (block->private.border_left_color) {
		pixman_image_unref(block->private.border_left_color);
	}
	if (block->private.border_right_color) {
		pixman_image_unref(block->private.border_right_color);
	}
	if (block->private.border_bottom_color) {
		pixman_image_unref(block->private.border_bottom_color);
	}
	if (block->private.border_top_color) {
		pixman_image_unref(block->private.border_top_color);
	}
}

static su_bool32_t sw__layout_block_init(sw_layout_block_t *block, sw_state_t *sw) {
	// TODO: remove recursion

	sw__layout_block_fini(block, sw);
	memset(&block->private, 0 , sizeof(block->private));
	block->private.sw = sw;

	block->out.status = SW_LAYOUT_BLOCK_STATUS_ALIVE;
	block->out.fini = sw__layout_block_fini;

	if (block->in.color.u32) {
		pixman_color_t c = sw__color_to_pixman_color(block->in.color);
		block->private.color = pixman_image_create_solid_fill(&c);
	}
	if (block->in.border_left.in.color.u32) {
		pixman_color_t c = sw__color_to_pixman_color(block->in.border_left.in.color);
		block->private.border_left_color = pixman_image_create_solid_fill(&c);
	}
	if (block->in.border_right.in.color.u32) {
		pixman_color_t c = sw__color_to_pixman_color(block->in.border_right.in.color);
		block->private.border_right_color = pixman_image_create_solid_fill(&c);
	}
	if (block->in.border_bottom.in.color.u32) {
		pixman_color_t c = sw__color_to_pixman_color(block->in.border_bottom.in.color);
		block->private.border_bottom_color = pixman_image_create_solid_fill(&c);
	}
	if (block->in.border_top.in.color.u32) {
		pixman_color_t c = sw__color_to_pixman_color(block->in.border_top.in.color);
		block->private.border_top_color = pixman_image_create_solid_fill(&c);
	}

	switch (block->in.type) {
	case SW_LAYOUT_BLOCK_TYPE_SPACER:
		break;
#if SW_WITH_TEXT
	case SW_LAYOUT_BLOCK_TYPE_TEXT: {
		char **font_names = sw->in.scratch_alloc->alloc(sw->in.scratch_alloc,
			(block->in.text.font_names.len + 1) * sizeof(*font_names), SU_ALIGNOF(*font_names));
		for (size_t i = 0; i < block->in.text.font_names.len; ++i) {
			su_string_t s = su_array__su_string_t__get(&block->in.text.font_names, i);
			SU_ASSERT(s.nul_terminated == SU_TRUE); // TODO: handle properly
			font_names[i] = s.s;
		}
		font_names[block->in.text.font_names.len] = "monospace:size=16";
    	struct fcft_font *font = fcft_from_name(block->in.text.font_names.len + 1, (const char **)font_names, NULL);
		sw->in.scratch_alloc->free(sw->in.scratch_alloc, font_names);
		if (font == NULL) {
			// ? TODO: set errno
			goto error;
		}

		struct fcft_text_run *text_run = NULL;
		struct sw__text_run_cache *cache;
		if (su_hash_table__struct_sw__text_run_cache__get(&sw->private.text_run_cache,
				(struct sw__text_run_cache){ .key = block->in.text.text },
				&cache)) {
			for (size_t i = 0; i < cache->items.len; ++i) {
				struct sw__text_run_cache_entry entry =
					su_array__struct_sw__text_run_cache_entry__get(&cache->items, i);
				if (entry.font == font) {
					text_run = entry.text_run;
					break;
				}
			}
		}

		if (text_run == NULL) {
			char32_t *text = sw->in.scratch_alloc->alloc( sw->in.scratch_alloc,
				block->in.text.text.len * sizeof(*text) + 1, SU_ALIGNOF(*text));
			size_t text_len = 0;
			mbstate_t s = { 0 };
			size_t consumed = 0;
			while (consumed < block->in.text.text.len) {
				char32_t c32;
				size_t ret = mbrtoc32(&c32, &block->in.text.text.s[consumed],
						block->in.text.text.len - consumed, &s);
				switch (ret) {
				case 0: // ? TODO: do not treat as error
				case (size_t)-1:
				case (size_t)-2:
				case (size_t)-3:
					sw->in.scratch_alloc->free(sw->in.scratch_alloc, text);
					// ? TODO: set errno
					goto error;
				default:
					text[text_len++] = c32;
					consumed += ret;
				}
			}

			text_run = fcft_rasterize_text_run_utf32(font, text_len, text, FCFT_SUBPIXEL_NONE);
			sw->in.scratch_alloc->free(sw->in.scratch_alloc, text);
			if ((text_run == NULL) || (text_run->count == 0)) {
				fcft_text_run_destroy(text_run);
				// ? TODO: set errno
				goto error;
			}

			if (su_hash_table__struct_sw__text_run_cache__add(&sw->private.text_run_cache,
					sw->in.gp_alloc,
					(struct sw__text_run_cache){ .key = block->in.text.text },
					&cache)) {
				su_string_init_string(&cache->key, sw->in.gp_alloc, cache->key);
			}

			su_array__struct_sw__text_run_cache_entry__add(&cache->items,
				sw->in.gp_alloc,
				(struct sw__text_run_cache_entry){
					.font = font,
					.text_run = text_run,
			});
		}

		int image_width = 0, image_height = font->height;
		for (size_t i = 0; i < text_run->count; ++i) {
			image_width += text_run->glyphs[i]->advance.x;
		}
		if ((image_width <= 0) || (image_height <= 0)) {
			errno = EINVAL;
			goto error;
		}

		block->private.content_image = sw__image_create(sw->in.gp_alloc, image_width, image_height, NULL);

		pixman_color_t color = sw__color_to_pixman_color(block->in.text.color);
		pixman_image_t *text_color = pixman_image_create_solid_fill(&color);

		int x = 0, y = font->height - font->descent;
		for (size_t i = 0; i < text_run->count; ++i) {
			const struct fcft_glyph *glyph = text_run->glyphs[i];
			if (pixman_image_get_format(glyph->pix) == PIXMAN_a8r8g8b8) {
				pixman_image_composite32(PIXMAN_OP_OVER, glyph->pix, NULL, block->private.content_image,
						0, 0, 0, 0, x + glyph->x, y - glyph->y,
						glyph->width, glyph->height);
			} else {
				pixman_image_composite32(PIXMAN_OP_OVER, text_color, glyph->pix, block->private.content_image,
						0, 0, 0, 0, x + glyph->x, y - glyph->y,
						glyph->width, glyph->height);
			}
			x += glyph->advance.x;
		}

		pixman_image_unref(text_color);
		break;
	}
#endif // SW_WITH_TEXT
	case SW_LAYOUT_BLOCK_TYPE_IMAGE: {
		char abspath_buf[PATH_MAX];
		su_string_t abspath = { 0 };
		struct stat sb;
		struct sw__image_cache *cache;

		if (block->in.image.format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH) {
			SU_ASSERT(block->in.image.path.nul_terminated); // TODO: handle properly
			if (realpath(block->in.image.path.s, abspath_buf) == NULL) {
				goto error;
			}

			abspath = (su_string_t){
				.s = abspath_buf,
				.len = strlen(abspath_buf),
				.free_contents = SU_FALSE,
				.nul_terminated = SU_TRUE,
			};

			if (stat(abspath.s, &sb) == -1) {
				struct sw__image_cache del;
				if (su_hash_table__struct_sw__image_cache__del(&sw->private.image_cache,
						(struct sw__image_cache){ .key = abspath }, &del)) {
					su_string_fini(&del.key, sw->in.gp_alloc);
					pixman_image_unref(del.image);
				}
				goto error;
			}

			if (su_hash_table__struct_sw__image_cache__get(&sw->private.image_cache,
					(struct sw__image_cache){ .key = abspath },
					&cache)) {
				if ((memcmp(&sb.st_mtim, &cache->mtim_ts, sizeof(sb.st_mtim)) == 0)
						&& (cache->type == block->in.image.type)) {
					block->private.content_image = pixman_image_ref(cache->image);
				} else {
					pixman_image_unref(cache->image);
				}
			}
		}

		// TODO: cache for memory images

		sw__scratch_alloc = sw->in.scratch_alloc; // TODO: remove

		if (block->private.content_image == NULL) {
			switch (block->in.image.type) {
			case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SW_PIXMAP:
				block->private.content_image = (block->in.image.format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH)
					? sw__load_pixmap_file(sw->in.gp_alloc, abspath)
					: sw__load_pixmap_memory(sw->in.gp_alloc, block->in.image.data, block->in.image.len);
				break;
#if SW_WITH_SVG
			case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SVG:
				block->private.content_image = sw__load_svg(sw->in.gp_alloc,
					block->in.image.format, abspath, block->in.image.data, block->in.image.len);
				break;
#endif // SW_WITH_SVG
#if SW_WITH_PNG
			case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNG:
				block->private.content_image = sw__load_png(sw->in.gp_alloc, sw->in.scratch_alloc,
					block->in.image.format, abspath, block->in.image.data, block->in.image.len);
				break;
#endif // SW_WITH_PNG
#if SW_WITH_JPG
			case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_JPG:
				block->private.content_image = sw__load_jpg(sw->in.gp_alloc, sw->in.scratch_alloc,
					block->in.image.format, abspath, block->in.image.data, block->in.image.len);
				break;
#endif // SW_WITH_JPG
#if SW_WITH_TGA
			case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_TGA:
				block->private.content_image = sw__load_tga(sw->in.gp_alloc, sw->in.scratch_alloc,
					block->in.image.format, abspath, block->in.image.data, block->in.image.len);
				break;
#endif // SW_WITH_TGA
#if SW_WITH_BMP
			case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_BMP:
				block->private.content_image = sw__load_bmp(sw->in.gp_alloc, sw->in.scratch_alloc,
					block->in.image.format, abspath, block->in.image.data, block->in.image.len);
				break;
#endif // SW_WITH_BMP
#if SW_WITH_PSD
			case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PSD:
				block->private.content_image = sw__load_psd(sw->in.gp_alloc, sw->in.scratch_alloc,
					block->in.image.format, abspath, block->in.image.data, block->in.image.len);
				break;
#endif // SW_WITH_PSD
#if SW_WITH_GIF
			case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_GIF:
				block->private.content_image = sw__load_gif(sw->in.gp_alloc, sw->in.scratch_alloc,
					block->in.image.format, abspath, block->in.image.data, block->in.image.len);
				break;
#endif // SW_WITH_GIF
#if SW_WITH_HDR
			case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_HDR:
				block->private.content_image = sw__load_hdr(sw->in.gp_alloc, sw->in.scratch_alloc,
					block->in.image.format, abspath, block->in.image.data, block->in.image.len);
				break;
#endif // SW_WITH_HDR
#if SW_WITH_PIC
			case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PIC:
				block->private.content_image = sw__load_pic(sw->in.gp_alloc, sw->in.scratch_alloc,
					block->in.image.format, abspath, block->in.image.data, block->in.image.len);
				break;
#endif // SW_WITH_PIC
#if SW_WITH_PNM
			case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNM:
				block->private.content_image = sw__load_pnm(sw->in.gp_alloc, sw->in.scratch_alloc,
					block->in.image.format, abspath, block->in.image.data, block->in.image.len);
				break;
#endif // SW_WITH_PNM
			default:
				SU_ASSERT_UNREACHABLE;
			}

			if (block->private.content_image == NULL) {
				if (abspath.len > 0) {	
					struct sw__image_cache del;
					if (su_hash_table__struct_sw__image_cache__del(&sw->private.image_cache,
							(struct sw__image_cache){ .key = abspath },
							&del)) {
						su_string_fini(&del.key, sw->in.gp_alloc);
					}
				}
				goto error;
			}

			if (block->in.image.format == SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_FILE_PATH) {
				if (su_hash_table__struct_sw__image_cache__add(&sw->private.image_cache,
						sw->in.gp_alloc,
						(struct sw__image_cache){ .key = abspath }, &cache)) {
					su_string_init_string(&cache->key, sw->in.gp_alloc, cache->key);
				}

				cache->image = pixman_image_ref(block->private.content_image);
				cache->mtim_ts = sb.st_mtim;
				cache->type = block->in.image.type;
			}

			pixman_image_set_filter(block->private.content_image, PIXMAN_FILTER_BEST, NULL, 0);
		}
		break;
	}
	case SW_LAYOUT_BLOCK_TYPE_COMPOSITE: {
		for (sw_layout_block_t *b = block->in.composite.children.head; b; b = b->next) {
			sw__layout_block_init(b, sw);
		}
		break;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}

	return SU_TRUE;
error:
	sw__layout_block_fini(block, sw);
	memset(&block->private, 0, sizeof(block->private));
	block->out.status = SW_LAYOUT_BLOCK_STATUS_ERROR;
	if (block->in.error) {
		block->in.error(block, sw);
	}
	errno = 0;
	return SU_FALSE;
}

static su_bool32_t sw__layout_block_prepare(sw_layout_block_t *, sw_layout_block_dimensions_t *overrides);

static su_bool32_t sw__layout_block_expand(sw_layout_block_t *block, int32_t available_width, int32_t available_height) {
	if ((block->out.status == SW_LAYOUT_BLOCK_STATUS_ERROR) || (block->in.expand == SW_LAYOUT_BLOCK_EXPAND_NONE)) {
		return SU_TRUE;
	}

	int32_t x = block->out.dim.x;
	int32_t y = block->out.dim.y;
	int32_t width = block->out.dim.width;
	int32_t height = block->out.dim.height;

	if (block->in.expand & SW_LAYOUT_BLOCK_EXPAND_LEFT) {
		width += x;
		x = 0;
	}

	if (block->in.expand & SW_LAYOUT_BLOCK_EXPAND_RIGHT) {
		width = (available_width - x);
	}

	if (block->in.expand & SW_LAYOUT_BLOCK_EXPAND_TOP) {
		height += y;
		y = 0;
	}

	if (block->in.expand & SW_LAYOUT_BLOCK_EXPAND_BOTTOM) {
		height = (available_height - y);
	}

	if (block->in.expand & SW_LAYOUT_BLOCK_EXPAND_CONTENT) {
		return sw__layout_block_prepare(
			block,
			&(sw_layout_block_dimensions_t){
				.border_left = block->out.dim.border_left,
				.border_bottom = block->out.dim.border_bottom,
				.border_right = block->out.dim.border_right,
				.border_top = block->out.dim.border_top,
				.x = x,
				.y = y,
				.content_width = width - block->out.dim.border_left - block->out.dim.border_right,
				.width = width,
				.content_height = height - block->out.dim.border_top - block->out.dim.border_bottom,
				.height = height,
			}
		);
	}

	block->out.dim.x = x;
	block->out.dim.y = y;
	block->out.dim.width = width;
	block->out.dim.height = height;

	return SU_TRUE;
}

static su_bool32_t sw__layout_block_prepare(sw_layout_block_t *block, sw_layout_block_dimensions_t *overrides) {
	// TODO: remove recursion
	if (block->out.status == SW_LAYOUT_BLOCK_STATUS_ERROR) {
		return SU_TRUE;
	}

	if (block->in.prepare) {
		// ? TODO: overrides
		if (!block->in.prepare(block, block->private.sw)) {
			return SU_FALSE;
		}
	}

	if (block->in.type == SW_LAYOUT_BLOCK_TYPE_COMPOSITE) {
		int32_t content_width, content_height;
		if (overrides) {
			content_width = overrides->content_width;
			content_height = overrides->content_height;
		} else {
			content_width = block->in.content_width;
			content_height = block->in.content_height;
		}

		su_bool32_t auto_content_width = (content_width == 0);
		su_bool32_t auto_content_height = (content_height == 0);
		su_bool32_t vertical = (block->in.composite.layout == SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_VERTICAL);
		int32_t l = 0, c = 0, r;

		for (sw_layout_block_t *b = block->in.composite.children.head; b; b = b->next) {
			if (b->out.status == SW_LAYOUT_BLOCK_STATUS_ERROR) {
				continue;
			}
			if (!sw__layout_block_prepare(b, NULL)) {
				return SU_FALSE;
			}
			if (b->in.anchor != SW_LAYOUT_BLOCK_ANCHOR_NONE) {
				if (vertical) {
					if (b->in.anchor == SW_LAYOUT_BLOCK_ANCHOR_CENTER) {
						c += b->out.dim.height;
					} else if (auto_content_height) {
						content_height += b->out.dim.height;
					}
					if (auto_content_width && (b->out.dim.width > content_width)) {
						content_width = b->out.dim.width;
					}
				} else {
					if (b->in.anchor == SW_LAYOUT_BLOCK_ANCHOR_CENTER) {
						c += b->out.dim.width;
					} else if (auto_content_width) {
						content_width += b->out.dim.width;
					}
					if (auto_content_height && (b->out.dim.height > content_height)) {
						content_height = b->out.dim.height;
					}
				}
			}
		}

		if (vertical) {
			if (c < content_height) {
				c = (content_height - c) / 2;
			} else if (auto_content_height) {
				content_height = c;
				c = 0;
			} else {
				c = 0;
			}
			r = content_height;
		} else {
			if (c < content_width) {
				c = (content_width - c) / 2;
			} else if (auto_content_width) {
				content_width = c;
				c = 0;
			} else {
				c = 0;
			}
			r = content_width;
		}

		for (sw_layout_block_t *b = block->in.composite.children.head; b; b = b->next) {
			if (b->out.status == SW_LAYOUT_BLOCK_STATUS_ERROR) {
				continue;
			}
			if (b->in.anchor != SW_LAYOUT_BLOCK_ANCHOR_NONE) {
				if (vertical) {
					switch (b->in.anchor) {
					case SW_LAYOUT_BLOCK_ANCHOR_TOP:
					case SW_LAYOUT_BLOCK_ANCHOR_LEFT:
						b->out.dim.x = 0;
						b->out.dim.y = l;
						l += b->out.dim.height;
						break;
					case SW_LAYOUT_BLOCK_ANCHOR_CENTER:
						b->out.dim.y = c;
						c += b->out.dim.height;
						b->out.dim.x = 0;
						break;
					case SW_LAYOUT_BLOCK_ANCHOR_BOTTOM:
					case SW_LAYOUT_BLOCK_ANCHOR_RIGHT:
						b->out.dim.x = 0;
						r -= b->out.dim.height;
						b->out.dim.y = r;
						break;
					case SW_LAYOUT_BLOCK_ANCHOR_NONE:
						break;
					default:
						SU_ASSERT_UNREACHABLE;
					}
				} else {
					switch (b->in.anchor) {
					case SW_LAYOUT_BLOCK_ANCHOR_TOP:
					case SW_LAYOUT_BLOCK_ANCHOR_LEFT:
						b->out.dim.y = 0;
						b->out.dim.x = l;
						l += b->out.dim.width;
						break;
					case SW_LAYOUT_BLOCK_ANCHOR_CENTER:
						b->out.dim.x = c;
						c += b->out.dim.width;
						b->out.dim.y = 0;
						break;
					case SW_LAYOUT_BLOCK_ANCHOR_BOTTOM:
					case SW_LAYOUT_BLOCK_ANCHOR_RIGHT:
						b->out.dim.y = 0;
						r -= b->out.dim.width;
						b->out.dim.x = r;
						break;
					case SW_LAYOUT_BLOCK_ANCHOR_NONE:
						break;
					default:
						SU_ASSERT_UNREACHABLE;
					}
				}
			}
			if (!sw__layout_block_expand(b, content_width, content_height)) {
				return SU_FALSE;
			}
		}

		if (!block->private.content_image || (content_width != pixman_image_get_width(block->private.content_image))
				|| (content_height != pixman_image_get_height(block->private.content_image))) {
			if (block->private.content_image) {
				pixman_image_unref(block->private.content_image);
			}
			block->private.content_image = ((content_width > 0) && (content_height > 0))
				? sw__image_create(block->private.sw->in.gp_alloc, content_width, content_height, NULL) : NULL;
		} else {
			memset(pixman_image_get_data(block->private.content_image),
				0,
				(size_t)content_width * 4 * (size_t)content_height);
		}
	}

	if (overrides) {
		block->out.dim = *overrides;
	} else {
		int32_t content_width = (((block->in.content_width <= 0) && block->private.content_image)
			? pixman_image_get_width(block->private.content_image) : block->in.content_width);
		int32_t content_height = (((block->in.content_height <= 0) && block->private.content_image)
			? pixman_image_get_height(block->private.content_image) : block->in.content_height);
		int32_t min_width = block->in.min_width;
		int32_t max_width = block->in.max_width;
		int32_t min_height = block->in.min_height;
		int32_t max_height = block->in.max_height;
		int32_t border_left = block->in.border_left.in.width;
		int32_t border_right = block->in.border_right.in.width;
		int32_t border_bottom = block->in.border_bottom.in.width;
		int32_t border_top = block->in.border_top.in.width;

		if ((block->private.content_image) && block->in.content_transform && ((block->in.content_transform % 2) == 0)) {
			int32_t tmp = content_width;
			content_width = content_height;
			content_height = tmp;
		}

		int32_t width = content_width + border_left + border_right;
		int32_t height = content_height + border_bottom + border_top;

		SU_ASSERT((min_width <= 0) || (max_width <= 0) || (max_width >= min_width));
		if (width < min_width) {
			width = min_width;
		} else if ((max_width > 0) && (width > max_width)) {
			width = max_width;
		}

		SU_ASSERT((min_height <= 0) || (max_height <= 0) || (max_height >= min_height));
		if (height < min_height) {
			height = min_height;
		} else if ((max_height > 0) && (height > max_height)) {
			height = max_height;
		}

		block->out.dim.x = block->in.x;
		block->out.dim.y = block->in.y;
		block->out.dim.width = width;
		block->out.dim.height = height;
		block->out.dim.content_width = content_width;
		block->out.dim.content_height = content_height;
		block->out.dim.border_left = border_left;
		block->out.dim.border_right = border_right;
		block->out.dim.border_bottom = border_bottom;
		block->out.dim.border_top = border_top;
	}

	if (block->in.prepared) {
		return block->in.prepared(block, block->private.sw);
	}

	return SU_TRUE;
}

static void sw__layout_block_render(sw_layout_block_t *block, pixman_image_t *dest) {
	// TODO: remove recursion
	if (block->out.status == SW_LAYOUT_BLOCK_STATUS_ERROR) {
		return;
	}

	if (block->in.type == SW_LAYOUT_BLOCK_TYPE_COMPOSITE) {
		for (sw_layout_block_t *b = block->in.composite.children.head; b; b = b->next) {
			sw__layout_block_render(b, block->private.content_image);
		}
	}

	sw_layout_block_dimensions_t dim = block->out.dim;

	if (block->private.color) {
		pixman_image_composite32(PIXMAN_OP_OVER, block->private.color, NULL, dest,
			0, 0, 0, 0,
			dim.x + dim.border_left,
			dim.y + dim.border_top,
			dim.width - dim.border_right - dim.border_left,
			dim.height - dim.border_bottom - dim.border_top);
	}

	if ((dim.border_left > 0) && block->private.border_left_color) {
		pixman_image_composite32(PIXMAN_OP_OVER, block->private.border_left_color, NULL, dest,
			0, 0, 0, 0, dim.x, dim.y, dim.border_left, dim.height);
	}

	if ((dim.border_right > 0) && block->private.border_right_color) {
		pixman_image_composite32(PIXMAN_OP_OVER, block->private.border_right_color, NULL, dest,
			0, 0, 0, 0, dim.x + dim.width - dim.border_right, dim.y,
			dim.border_right, dim.height);
	}

	if ((dim.border_bottom > 0) && block->private.border_bottom_color) {
		pixman_image_composite32(PIXMAN_OP_OVER, block->private.border_bottom_color, NULL, dest,
			0, 0, 0, 0, dim.x + dim.border_left,
			dim.y + dim.height - dim.border_bottom,
			dim.width - dim.border_left - dim.border_right,
			dim.border_bottom);
	}

	if ((dim.border_top > 0) && block->private.border_top_color) {
		pixman_image_composite32(PIXMAN_OP_OVER, block->private.border_top_color, NULL, dest,
			0, 0, 0, 0, dim.x + dim.border_left, dim.y,
			dim.width - dim.border_left - dim.border_right,
			dim.border_top);
	}

	if (block->private.content_image) {
		struct sw__image_data *image_data = pixman_image_get_destroy_data(block->private.content_image);
		SU_NOTUSED(image_data);
#if SW_WITH_GIF
		if (image_data->type == SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF) {
			int64_t now_msec = su_now_ms();
			struct sw__image_multiframe_gif *data = image_data->multiframe_gif;
			struct sw__image_gif_frame frame = su_array__struct_sw__image_gif_frame__get(
				&data->frames, data->frame_idx);
			if (now_msec >= data->frame_end) {
				if (++data->frame_idx >= data->frames.len) {
					data->frame_idx = 0;
				}
				data->frame_end = (now_msec + frame.delay);
			}
			pixman_image_unref(block->private.content_image);
			block->private.content_image = pixman_image_ref(frame.image);
			sw__update_t(block->private.sw, data->frame_end);
		}
#endif // SW_WITH_GIF

		int content_image_width = pixman_image_get_width(block->private.content_image);
		int content_image_height = pixman_image_get_height(block->private.content_image);
		if (block->in.content_transform && ((block->in.content_transform % 2) == 0)) {
			int tmp = content_image_width;
			content_image_width = content_image_height;
			content_image_height = tmp;
		}

		pixman_transform_t transform;
		pixman_transform_init_identity(&transform);

		if ((dim.content_width != content_image_width)
				|| (dim.content_height != content_image_height)) {
#if SW_WITH_SVG
			if (image_data->type == SW__IMAGE_DATA_TYPE_SVG) {
				pixman_image_unref(block->private.content_image);
				block->private.content_image = sw__render_svg(block->private.sw->in.gp_alloc,
					image_data->svg, NULL, dim.content_width, dim.content_height);
			} else
#endif // SW_WITH_SVG
				pixman_transform_scale(&transform, NULL,
					pixman_int_to_fixed(content_image_width) / dim.content_width,
					pixman_int_to_fixed(content_image_height) / dim.content_height);
		}

		switch (block->in.content_transform) {
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_NORMAL:
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED:
			break;
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_90:
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_90:
			pixman_transform_rotate(&transform, NULL, 0, pixman_fixed_1);
			pixman_transform_translate(&transform, NULL,
				pixman_int_to_fixed(pixman_image_get_width(block->private.content_image)), 0);
			break;
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_180:
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_180:
			pixman_transform_rotate(&transform, NULL, pixman_fixed_minus_1, 0);
			pixman_transform_translate(&transform, NULL,
				pixman_int_to_fixed(pixman_image_get_width(block->private.content_image)),
				pixman_int_to_fixed(pixman_image_get_height(block->private.content_image)));
			break;
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_270:
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_270:
			pixman_transform_rotate(&transform, NULL, 0, pixman_fixed_minus_1);
			pixman_transform_translate(&transform, NULL, 0,
				pixman_int_to_fixed(pixman_image_get_height(block->private.content_image)));
			break;
		default:
			SU_ASSERT_UNREACHABLE;
		}

		if (block->in.content_transform >= SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED) {
			pixman_transform_translate(&transform, NULL,
				-pixman_int_to_fixed(pixman_image_get_width(block->private.content_image)), 0);
			pixman_transform_scale(&transform, NULL, pixman_fixed_minus_1, pixman_fixed_1);
		}

		pixman_image_set_transform(block->private.content_image, &transform);

		pixman_region32_t clip_region;
		pixman_region32_init_rect(&clip_region, dim.x, dim.y, (unsigned int)dim.width, (unsigned int)dim.height);
		pixman_image_set_clip_region32(dest, &clip_region);

		// ? TODO: move to sw__layout_block_prepare
		int32_t available_width = dim.width - dim.border_left - dim.border_right;
		int32_t available_height = dim.height - dim.border_bottom - dim.border_top;
		int32_t content_x = dim.x + dim.border_left;
		int32_t content_y = dim.y + dim.border_top;
		switch (block->in.content_anchor) {
		case SW_LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_TOP:
			break;
		case SW_LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_CENTER:
			content_y += ((available_height - dim.content_height) / 2);
			break;
		case SW_LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_BOTTOM:
			content_y += (available_height - dim.content_height);
			break;
		case SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_TOP:
			content_x += ((available_width - dim.content_width) / 2);
			break;
		case SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER:
			content_x += ((available_width - dim.content_width) / 2);
			content_y += ((available_height - dim.content_height) / 2);
			break;
		case SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_BOTTOM:
			content_x += ((available_width - dim.content_width) / 2);
			content_y += (available_height - dim.content_height);
			break;
		case SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_TOP:
			content_x += (available_width - dim.content_width);
			break;
		case SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_CENTER:
			content_x += (available_width - dim.content_width);
			content_y += ((available_height - dim.content_height) / 2);
			break;
		case SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_BOTTOM:
			content_x += (available_width - dim.content_width);
			content_y += (available_height - dim.content_height);
			break;
		default:
			SU_ASSERT_UNREACHABLE;
		}

		pixman_image_composite32(PIXMAN_OP_OVER, block->private.content_image, NULL, dest,
			0, 0, 0, 0, content_x, content_y, dim.content_width, dim.content_height);

		pixman_image_set_transform(block->private.content_image, NULL);
		pixman_image_set_clip_region32(dest, NULL);
	}
}

#if SW_WITH_WAYLAND
static void sw__wayland_surface_buffer_fini(struct sw__wayland_surface_buffer *buffer) {
	if (buffer->image) {
		pixman_image_unref(buffer->image);
	}
	if (buffer->wl_buffer) {
		wl_buffer_destroy(buffer->wl_buffer);
	}
	if (buffer->pixels) {
		munmap(buffer->pixels, buffer->size);
	}
}

static void sw__wayland_surface_fini(sw_wayland_surface_t *surface, sw_state_t *sw) {
	switch (surface->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_LAYER:
		if (surface->private.layer.layer_surface) {
			zwlr_layer_surface_v1_destroy(surface->private.layer.layer_surface);
		}
		break;
	case SW_WAYLAND_SURFACE_TYPE_POPUP:
		if (surface->private.popup.xdg_positioner) {
			xdg_positioner_destroy(surface->private.popup.xdg_positioner);
		}
		if (surface->private.popup.xdg_popup) {
			xdg_popup_destroy(surface->private.popup.xdg_popup);
		}
		if (surface->private.popup.xdg_surface) {
			xdg_surface_destroy(surface->private.popup.xdg_surface);
		}
		break;
	default:
		SU_ASSERT_UNREACHABLE;
	}
	sw__wayland_surface_buffer_fini(&surface->private.buffer);
	if (surface->private.wl_surface) {
		wl_surface_destroy(surface->private.wl_surface);
	}

	for (sw_wayland_seat_t *seat = sw->out.wayland.seats.head; seat; seat = seat->next) {
		if (seat->out.pointer && (seat->out.pointer->out.focused_surface == surface)) {
			seat->out.pointer->out.focused_surface = NULL;
		}
	}
}

static void sw__wayland_surface_set_error(sw_wayland_surface_t *surface) {
	sw_state_t *sw = surface->private.sw;
	sw__wayland_surface_fini(surface, sw);
	memset(&surface->private, 0, sizeof(surface->private));
	surface->out.status = SW_WAYLAND_SURFACE_STATUS_ERROR;
	if (surface->in.error) {
		surface->in.error(surface, sw);
	}
	errno = 0;
}

static void sw__wayland_surface_popup_init_stage2(sw_wayland_surface_t *);

static void sw__wayland_surface_render(sw_wayland_surface_t *surface) {
	if (surface->out.status != SW_WAYLAND_SURFACE_STATUS_ALIVE) {
		return;
	}

	if (surface->private.buffer.wl_buffer && surface->private.buffer.busy) {
		surface->private.dirty = SU_TRUE;
		return;
	}

	if (!sw__layout_block_prepare(surface->in.root, NULL)) {
		return;
	}

	int32_t surface_width, surface_height;
	switch (surface->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_LAYER: {
		// ? TODO: allow surface sizing with SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL (the same can be achieved with 0 anchor)
		if (surface->private.layer.anchor != SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL) {
			if (surface->in.width < 0) {
				SU_ASSERT(surface->in.root->out.dim.width > 0);
				surface_width = surface->in.root->out.dim.width;
			} else {
				surface_width = surface->in.width;
			}
			if (surface->in.height < 0) {
				SU_ASSERT(surface->in.root->out.dim.height > 0);
				surface_height = surface->in.root->out.dim.height;
			} else {
				surface_height = surface->in.height;
			}
			if (!surface->private.buffer.wl_buffer ||
						((surface_width != 0) && (surface->out.width != surface_width)) ||
						((surface_height != 0) && (surface->out.height != surface_height))) {
				zwlr_layer_surface_v1_set_size(surface->private.layer.layer_surface,
					(uint32_t)(surface_width / surface->out.scale),
					(uint32_t)(surface_height / surface->out.scale));
			}
		} else {
			surface_width = surface->out.width;
			surface_height = surface->out.height;
		}
		int32_t exclusive_zone = surface->in.layer.exclusive_zone;
		if (exclusive_zone < -1) {
			switch (surface->private.layer.anchor) {
			case ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP:
			case (ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
					ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
					ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT):
			case ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM:
			case (ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
					ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
					ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT):
				exclusive_zone = surface_height;
				break;
			case ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT:
			case (ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
					ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
					ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT):
			case ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT:
			case (ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
					ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
					ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT):
				exclusive_zone = surface_width;
				break;
			default:
				exclusive_zone = 0;
				break;
			}
		}
		if (surface->private.layer.exclusive_zone != exclusive_zone) {
			zwlr_layer_surface_v1_set_exclusive_zone(surface->private.layer.layer_surface,
				exclusive_zone / surface->out.scale);
			surface->private.layer.exclusive_zone = exclusive_zone;
		}
		break;
	}
	case SW_WAYLAND_SURFACE_TYPE_POPUP: {
		surface_width = (surface->in.width <= 0) ? surface->in.root->out.dim.width : surface->in.width;
		surface_height = (surface->in.height <= 0) ? surface->in.root->out.dim.height : surface->in.height;
		SU_ASSERT((surface_width > 0) && (surface_height > 0));
		if (!surface->private.buffer.wl_buffer ||
				(surface->out.width != surface_width) || (surface->out.height != surface_height)) {
			xdg_positioner_set_size(surface->private.popup.xdg_positioner,
				surface_width / surface->out.scale, surface_height / surface->out.scale);
			if (!surface->private.popup.xdg_popup) {
				sw__wayland_surface_popup_init_stage2(surface);
				return;
			}
			xdg_popup_reposition(surface->private.popup.xdg_popup, surface->private.popup.xdg_positioner, 0);
		}
		break;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}

	if (!sw__layout_block_expand(surface->in.root,
			(surface_width <= 0) ? surface->out.width : surface_width,
			(surface_height <= 0) ? surface->out.height : surface_height)) {
		return;
	}

	if (surface->private.buffer.wl_buffer) {
		memset(surface->private.buffer.pixels, 0, surface->private.buffer.size);

		sw__layout_block_render(surface->in.root, surface->private.buffer.image);

		wl_surface_set_buffer_scale(surface->private.wl_surface, surface->out.scale);
		wl_surface_attach(surface->private.wl_surface, surface->private.buffer.wl_buffer, 0, 0);
		wl_surface_damage_buffer(surface->private.wl_surface, 0, 0, surface->out.width, surface->out.height);

		surface->private.buffer.busy = SU_TRUE;
		surface->private.dirty = SU_FALSE;
	}

	wl_surface_commit(surface->private.wl_surface);
}

static void sw__wayland_surface_buffer_handle_release(void *data, struct wl_buffer *wl_buffer) {
	SU_NOTUSED(wl_buffer);
	sw_wayland_surface_t *surface = data;
	surface->private.buffer.busy = SU_FALSE;
	if (surface->private.dirty) {
		sw__wayland_surface_render(surface);
	}
}

static su_bool32_t sw__wayland_surface_buffer_init(struct sw__wayland_surface_buffer *buffer,
		sw_wayland_surface_t *surface, int32_t width, int32_t height) {
	*buffer = (struct sw__wayland_surface_buffer){ 0 };

	struct timespec ts = { 0 };
	pid_t pid = getpid();
	char shm_name[NAME_MAX];

// TODO: limit retry count
generate_shm_name:
	clock_gettime(CLOCK_MONOTONIC, &ts);
	su_snprintf(shm_name, sizeof(shm_name),"/sw-%d-%ld-%ld",
			pid, ts.tv_sec, ts.tv_nsec);

	int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0600);
	if (shm_fd == -1) {
		if (errno == EEXIST) {
			goto generate_shm_name;
		} else {
			goto error;
		}
	}
	shm_unlink(shm_name);

	int32_t stride = width * 4;
	buffer->size = (uint32_t)stride * (uint32_t)height;
	while (ftruncate(shm_fd, buffer->size) == -1) {
		if (errno == EINTR) {
			continue;
		} else {
			goto error;
		}
	}

	buffer->pixels = mmap(NULL, buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (buffer->pixels == MAP_FAILED) {
		goto error;
	}

	buffer->image = pixman_image_create_bits_no_clear(PIXMAN_a8r8g8b8, width, height,
			buffer->pixels, stride);

	struct wl_shm_pool *wl_shm_pool =
		wl_shm_create_pool(surface->private.sw->private.wayland.shm, shm_fd, (int32_t)buffer->size);
	buffer->wl_buffer = wl_shm_pool_create_buffer(wl_shm_pool, 0, width,
			height, stride, WL_SHM_FORMAT_ARGB8888);
	static struct wl_buffer_listener surface_buffer_listener = {
		.release = sw__wayland_surface_buffer_handle_release,
	};
	wl_buffer_add_listener(buffer->wl_buffer, &surface_buffer_listener, surface);
	wl_shm_pool_destroy(wl_shm_pool);
	close(shm_fd);

	buffer->busy = SU_FALSE;

	return SU_TRUE;
error:
	sw__wayland_surface_buffer_fini(buffer);
	return SU_FALSE;
}

static void sw__wayland_surface_destroy(sw_wayland_surface_t *surface) {
	// TODO: remove recursion
	for (sw_wayland_surface_t *popup = surface->in.popups.head; popup; ) {
		sw_wayland_surface_t *next = popup->next;
		sw__wayland_surface_destroy(popup);
		popup = next;
	}
	surface->out.status = SW_WAYLAND_SURFACE_STATUS_CLOSED;
	surface->in.destroy(surface, surface->private.sw);
}

static void sw__wayland_surface_handle_enter(void *data, struct wl_surface *wl_surface, struct wl_output *output) {
	SU_NOTUSED(data); SU_NOTUSED(wl_surface); SU_NOTUSED(output);
}

static void sw__wayland_surface_handle_leave(void *data, struct wl_surface *wl_surface, struct wl_output *output) {
	SU_NOTUSED(data); SU_NOTUSED(wl_surface); SU_NOTUSED(output);
}

static void sw__wayland_surface_handle_preferred_buffer_transform(void *data,
		struct wl_surface *wl_surface, uint32_t transform) {
	SU_NOTUSED(data); SU_NOTUSED(wl_surface); SU_NOTUSED(transform);
}

static void sw__wayland_surface_layer_handle_preferred_buffer_scale(void *data,
		struct wl_surface *wl_surface, int32_t factor) {
	SU_NOTUSED(wl_surface);
	sw_wayland_surface_t *layer = data;
	if (layer->out.scale != factor) {
		layer->out.scale = factor;
		layer->private.dirty = SU_TRUE;
		if (layer->private.layer.anchor != SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL) {
			zwlr_layer_surface_v1_set_size(layer->private.layer.layer_surface,
				(layer->in.width == 0) ? 0 : (uint32_t)(layer->out.width / factor),
				(layer->in.height == 0) ? 0 : (uint32_t)(layer->out.height / factor));
		}
		if (layer->private.layer.exclusive_zone > 0) {
			zwlr_layer_surface_v1_set_exclusive_zone(layer->private.layer.layer_surface,
				layer->private.layer.exclusive_zone / factor);
		}
		zwlr_layer_surface_v1_set_margin(layer->private.layer.layer_surface,
			layer->private.layer.margins[0] / factor,
			layer->private.layer.margins[1] / factor,
			layer->private.layer.margins[2] / factor,
			layer->private.layer.margins[3] / factor);
		wl_surface_commit(layer->private.wl_surface);
	}
}

static void sw__wayland_surface_layer_handle_layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *layer_surface,
		uint32_t serial, uint32_t width_, uint32_t height_) {
	SU_NOTUSED(layer_surface);
	sw_wayland_surface_t *layer = data;

	zwlr_layer_surface_v1_ack_configure(layer->private.layer.layer_surface, serial);

	int32_t width = (int32_t)width_ * layer->out.scale;
	int32_t height = (int32_t)height_ * layer->out.scale;
	if (((layer->out.height != height) || (layer->out.width != width))
			&& (width != 0) && (height != 0)) {
		sw__wayland_surface_buffer_fini(&layer->private.buffer);
		if (sw__wayland_surface_buffer_init(&layer->private.buffer, layer, width, height)) {
			layer->out.width = width;
			layer->out.height = height;
			layer->private.dirty = SU_TRUE;
		} else {
			sw__wayland_surface_set_error(layer);
			return;
		}
	}

	if (layer->private.dirty) {
		sw__wayland_surface_render(layer);
	}
}

static void sw__wayland_surface_layer_handle_layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *layer_surface) {
	SU_NOTUSED(layer_surface);
	sw_wayland_surface_t *layer = data;
	sw__wayland_surface_destroy(layer);
}

static void sw__wayland_surface_popup_handle_preferred_buffer_scale(void *data,
		struct wl_surface *wl_surface, int32_t factor) {
	SU_NOTUSED(wl_surface);
	sw_wayland_surface_t *popup = data;
	if (popup->out.scale != factor) {
		popup->out.scale = factor;
		popup->private.dirty = SU_TRUE;
		xdg_positioner_set_size(popup->private.popup.xdg_positioner,
			popup->out.width / factor, popup->out.height / factor);
		xdg_positioner_set_anchor_rect(popup->private.popup.xdg_positioner,
			popup->private.popup.x / factor,
			popup->private.popup.y / factor,
			1, 1);
		xdg_popup_reposition(popup->private.popup.xdg_popup, popup->private.popup.xdg_positioner, 0);
		wl_surface_commit(popup->private.wl_surface);
	}
}

static void sw__wayland_surface_popup_xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface,
		uint32_t serial) {
	SU_NOTUSED(xdg_surface);
	sw_wayland_surface_t *popup = data;

	xdg_surface_ack_configure(popup->private.popup.xdg_surface, serial);

	if (popup->private.dirty) {
		sw__wayland_surface_render(popup);
	}
}

static void sw__wayland_surface_layer_init(sw_wayland_surface_t *surface, sw_wayland_surface_layer_layer_t layer,
		sw_state_t *sw) {
	surface->private.sw = sw;
	surface->out.fini = sw__wayland_surface_fini;
	surface->out.scale = surface->in.layer.output->out.scale;
	surface->out.status = SW_WAYLAND_SURFACE_STATUS_ALIVE;
	surface->out.width = 0;
	surface->out.height = 0;
	surface->private.output = surface->in.layer.output;
	surface->private.layer.exclusive_zone = INT32_MIN;
	surface->private.layer.anchor = UINT32_MAX;
	surface->private.layer.layer = layer;
	surface->private.layer.margin_top = INT32_MIN;
	surface->private.layer.margin_right = INT32_MIN;
	surface->private.layer.margin_bottom = INT32_MIN;
	surface->private.layer.margin_left = INT32_MIN;
	surface->private.wl_surface = wl_compositor_create_surface(surface->private.sw->private.wayland.compositor);
	static struct wl_surface_listener wl_surface_listener = {
		.enter = sw__wayland_surface_handle_enter,
		.leave = sw__wayland_surface_handle_leave,
		.preferred_buffer_transform = sw__wayland_surface_handle_preferred_buffer_transform,
		.preferred_buffer_scale = sw__wayland_surface_layer_handle_preferred_buffer_scale,
	};
	wl_surface_add_listener(surface->private.wl_surface, &wl_surface_listener, surface);
	surface->private.layer.layer_surface = zwlr_layer_shell_v1_get_layer_surface(
		surface->private.sw->private.wayland.layer_shell, surface->private.wl_surface,
		surface->in.layer.output->private.wl_output, layer, "sw");
	static struct zwlr_layer_surface_v1_listener layer_surface_listener = {
		.configure = sw__wayland_surface_layer_handle_layer_surface_configure,
		.closed = sw__wayland_surface_layer_handle_layer_surface_closed,
	};
	zwlr_layer_surface_v1_add_listener(
		surface->private.layer.layer_surface, &layer_surface_listener, surface);
}

static void sw__wayland_surface_popup_init_stage1(sw_wayland_surface_t *popup,
		sw_wayland_surface_t *parent, sw_state_t *sw) {
	popup->private.sw = sw;
	popup->private.popup.parent = parent;
	popup->private.output = parent->private.output;
	popup->out.fini = sw__wayland_surface_fini;
	popup->out.scale = parent->private.output->out.scale;
	popup->out.status = SW_WAYLAND_SURFACE_STATUS_ALIVE;
	popup->out.width = 0;
	popup->out.height = 0;
	popup->private.popup.x = INT32_MIN;
	popup->private.popup.y = INT32_MIN;
	popup->private.popup.gravity = (sw_wayland_surface_popup_gravity_t)UINT32_MAX;
	popup->private.popup.constraint_adjustment = UINT32_MAX;

	popup->private.wl_surface = wl_compositor_create_surface(popup->private.sw->private.wayland.compositor);
	static struct wl_surface_listener wl_surface_listener = {
		.enter = sw__wayland_surface_handle_enter,
		.leave = sw__wayland_surface_handle_leave,
		.preferred_buffer_transform = sw__wayland_surface_handle_preferred_buffer_transform,
		.preferred_buffer_scale = sw__wayland_surface_popup_handle_preferred_buffer_scale,
	};
	wl_surface_add_listener(popup->private.wl_surface, &wl_surface_listener, popup);

	popup->private.popup.xdg_surface = xdg_wm_base_get_xdg_surface(
		popup->private.sw->private.wayland.wm_base, popup->private.wl_surface);
	static struct xdg_surface_listener xdg_surface_listener = {
		.configure = sw__wayland_surface_popup_xdg_surface_handle_configure,
	};
	xdg_surface_add_listener(popup->private.popup.xdg_surface, &xdg_surface_listener, popup);

	popup->private.popup.xdg_positioner = xdg_wm_base_create_positioner(popup->private.sw->private.wayland.wm_base);
}

static void sw__wayland_surface_popup_handle_configure(void *data, struct xdg_popup *xdg_popup,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	SU_NOTUSED(xdg_popup); SU_NOTUSED(x); SU_NOTUSED(y);
	sw_wayland_surface_t *popup = data;
	width *= popup->out.scale;
	height *= popup->out.scale;
	if ((popup->out.width != width) || (popup->out.height != height)) {
		sw__wayland_surface_buffer_fini(&popup->private.buffer);
		if (sw__wayland_surface_buffer_init(&popup->private.buffer, popup, width, height)) {
			popup->out.width = width;
			popup->out.height = height;
			popup->private.dirty = SU_TRUE;
		} else {
			sw__wayland_surface_set_error(popup);
		}
	}
}

static void sw__wayland_surface_popup_handle_done(void *data, struct xdg_popup *xdg_popup) {
	SU_NOTUSED(xdg_popup);
	sw_wayland_surface_t *popup = data;
	sw__wayland_surface_destroy(popup);
}

static void sw__wayland_surface_popup_handle_repositioned(void *data, struct xdg_popup *xdg_popup,
		uint32_t token) {
	SU_NOTUSED(data); SU_NOTUSED(xdg_popup); SU_NOTUSED(token);
}

static void sw__wayland_surface_prepare(sw_wayland_surface_t *surface, sw_wayland_surface_t *parent, sw_state_t *sw) {
	// TODO: remove recursion

	surface->private.sw = sw;

	if (!sw__layout_block_init(surface->in.root, sw)) {
		sw__wayland_surface_set_error(surface);
		return;
	}

	switch (surface->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_LAYER:
		if (surface->private.output != surface->in.layer.output) {
			sw__wayland_surface_fini(surface, sw);
			memset(&surface->private, 0, sizeof(surface->private));
		}

		static uint32_t horiz = (ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
		static uint32_t vert = (ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
		SU_NOTUSED(horiz); SU_NOTUSED(vert);
		SU_ASSERT((surface->in.layer.anchor == SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL) ||
			(((surface->in.width != 0) || ((surface->in.layer.anchor & horiz) == horiz)) &&
			((surface->in.height != 0) || ((surface->in.layer.anchor & vert) == vert)))
		);

		if (!surface->private.wl_surface) {
			sw__wayland_surface_layer_init(surface, surface->in.layer.layer, sw);
		}

		// ? TODO: exclusive_edge

		if (surface->private.layer.anchor != surface->in.layer.anchor) {
			zwlr_layer_surface_v1_set_anchor(surface->private.layer.layer_surface, surface->in.layer.anchor);
			surface->private.layer.anchor = surface->in.layer.anchor;
		}

		if (surface->private.layer.layer != surface->in.layer.layer) {
			zwlr_layer_surface_v1_set_layer(surface->private.layer.layer_surface, surface->in.layer.layer);
			surface->private.layer.layer = surface->in.layer.layer;
		}

		if (memcmp(surface->private.layer.margins, surface->in.layer.margins,
				sizeof(surface->in.layer.margins)) != 0) {
			zwlr_layer_surface_v1_set_margin(surface->private.layer.layer_surface,
				surface->in.layer.margins[0] / surface->out.scale,
				surface->in.layer.margins[1] / surface->out.scale,
				surface->in.layer.margins[2] / surface->out.scale,
				surface->in.layer.margins[3] / surface->out.scale);
			memcpy(surface->private.layer.margins, surface->in.layer.margins,
				sizeof(surface->in.layer.margins));
		}

		for (sw_wayland_surface_t *popup = surface->in.popups.head; popup; popup = popup->next) {
			SU_ASSERT(popup->in.type == SW_WAYLAND_SURFACE_TYPE_POPUP);
			sw__wayland_surface_prepare(popup, surface, sw);
		}
		break;
	case SW_WAYLAND_SURFACE_TYPE_POPUP:
		if (!surface->private.wl_surface) {
			sw__wayland_surface_popup_init_stage1(surface, parent, sw);
		}

		su_bool32_t reposition = SU_FALSE;
		if ((surface->private.popup.x != surface->in.popup.x) ||
				(surface->private.popup.y != surface->in.popup.y)) {
			xdg_positioner_set_anchor_rect(surface->private.popup.xdg_positioner,
				surface->in.popup.x / surface->out.scale, surface->in.popup.y / surface->out.scale, 1, 1);
			surface->private.popup.x = surface->in.popup.x;
			surface->private.popup.y = surface->in.popup.y;
			reposition = SU_TRUE;
		}

		if (surface->private.popup.gravity != surface->in.popup.gravity) {
			xdg_positioner_set_gravity(surface->private.popup.xdg_positioner, surface->in.popup.gravity);
			surface->private.popup.gravity = surface->in.popup.gravity;
			reposition = SU_TRUE;
		}

		if (surface->private.popup.constraint_adjustment != surface->in.popup.constraint_adjustment) {
			xdg_positioner_set_constraint_adjustment(
				surface->private.popup.xdg_positioner, surface->in.popup.constraint_adjustment);
			surface->private.popup.constraint_adjustment = surface->in.popup.constraint_adjustment;
			reposition = SU_TRUE;
		}

		if (surface->private.popup.xdg_popup) {
			if (reposition) {
				xdg_popup_reposition(surface->private.popup.xdg_popup, surface->private.popup.xdg_positioner, 0);
			}
			for (sw_wayland_surface_t *popup = surface->in.popups.head; popup; popup = popup->next) {
				SU_ASSERT(popup->in.type == SW_WAYLAND_SURFACE_TYPE_POPUP);
				sw__wayland_surface_prepare(popup, surface, sw);
			}
		}
		break;
	default:
		SU_ASSERT_UNREACHABLE;
	}

	sw_wayland_cursor_shape_t cursor_shape = (surface->in.cursor_shape == SW_WAYLAND_CURSOR_SHAPE_DEFAULT)
		? SW_WAYLAND_CURSOR_SHAPE_DEFAULT_ : surface->in.cursor_shape;
	if (surface->private.cursor_shape != cursor_shape) {
		for (sw_wayland_seat_t *seat = sw->out.wayland.seats.head; seat; seat = seat->next) {
			if (seat->out.pointer && seat->out.pointer->private.cursor_shape_device
					&& (seat->out.pointer->out.focused_surface == surface)) {
				wp_cursor_shape_device_v1_set_shape(seat->out.pointer->private.cursor_shape_device,
					seat->out.pointer->private.enter_serial, cursor_shape);
			}
		}
		surface->private.cursor_shape = cursor_shape;
	}

	// TODO: set only if changed
	if (surface->in.input_regions.len > 0) {
		struct wl_region *input_region = wl_compositor_create_region(sw->private.wayland.compositor);
		for (sw_wayland_region_t *r = surface->in.input_regions.head; r; r = r->next) {
			wl_region_add(input_region, r->in.x, r->in.y, r->in.width, r->in.height);
		}
		wl_surface_set_input_region(surface->private.wl_surface, input_region);
		wl_region_destroy(input_region);
	} else {
		wl_surface_set_input_region(surface->private.wl_surface, NULL);
	}

	sw__wayland_surface_render(surface);
}

static void sw__wayland_surface_popup_init_stage2(sw_wayland_surface_t *popup) {
	switch (popup->private.popup.parent->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_LAYER:
		popup->private.popup.xdg_popup = xdg_surface_get_popup(
			popup->private.popup.xdg_surface, NULL, popup->private.popup.xdg_positioner);
		zwlr_layer_surface_v1_get_popup(popup->private.popup.parent->private.layer.layer_surface,
			popup->private.popup.xdg_popup);
		popup->private.popup.grab = popup->in.popup.grab;
		break;
	case SW_WAYLAND_SURFACE_TYPE_POPUP:
		popup->private.popup.xdg_popup = xdg_surface_get_popup( popup->private.popup.xdg_surface,
			popup->private.popup.parent->private.popup.xdg_surface, popup->private.popup.xdg_positioner);
		popup->private.popup.grab = popup->private.popup.parent->private.popup.grab;
		break;
	default:
		SU_ASSERT_UNREACHABLE;
	}

	static struct xdg_popup_listener xdg_popup_listener = {
		.configure = sw__wayland_surface_popup_handle_configure,
		.popup_done = sw__wayland_surface_popup_handle_done,
		.repositioned = sw__wayland_surface_popup_handle_repositioned,
	};
	xdg_popup_add_listener(popup->private.popup.xdg_popup, &xdg_popup_listener, popup);

	// TODO: handle grab with invalid serial, touch serial
	if (popup->private.popup.grab) {
		xdg_popup_grab(popup->private.popup.xdg_popup,
			popup->private.popup.grab->out.seat->private.wl_seat,
			popup->private.popup.grab->private.button_serial);
	}

	wl_surface_commit(popup->private.wl_surface);

	for (sw_wayland_surface_t *p = popup->in.popups.head; p; p = p->next) {
		SU_ASSERT(p->in.type == SW_WAYLAND_SURFACE_TYPE_POPUP);
		sw__wayland_surface_prepare(p, popup, popup->private.sw);
	}
}

static void sw__wayland_output_handle_geometry(void *data, struct wl_output *wl_output,
		int32_t x, int32_t y, int32_t physical_width, int32_t physical_height,
		int32_t subpixel, const char *make, const char *model, int32_t transform) {
	SU_NOTUSED(wl_output); SU_NOTUSED(x); SU_NOTUSED(y); SU_NOTUSED(physical_width); SU_NOTUSED(physical_height);
	SU_NOTUSED(subpixel); SU_NOTUSED(make); SU_NOTUSED(model);
	sw_wayland_output_t *output = data;
	output->out.transform = (sw_wayland_output_transform_t)transform;
}

static void sw__wayland_output_handle_mode(void *data, struct wl_output *wl_output,
		uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
	SU_NOTUSED(wl_output); SU_NOTUSED(flags); SU_NOTUSED(refresh);
	sw_wayland_output_t *output = data;
	output->out.width = width;
	output->out.height = height;
}

static void sw__wayland_output_handle_done(void *data, struct wl_output *wl_output) {
	SU_NOTUSED(wl_output); SU_NOTUSED(data);
}

static void sw__wayland_output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	SU_NOTUSED(wl_output);
	sw_wayland_output_t *output = data;
	output->out.scale = factor;
}

static void sw__wayland_output_handle_name(void *data, struct wl_output *wl_output, const char *name) {
	SU_NOTUSED(wl_output);
	sw_wayland_output_t *output = data;
	size_t len = strlen(name);
	if (len > 0) {
		su_string_init_len(&output->out.name, output->private.sw->in.gp_alloc, (char *)name, len, SU_TRUE);
	}
}

static void sw__wayland_output_handle_description(void *data, struct wl_output *wl_output, const char *description) {
	SU_NOTUSED(data); SU_NOTUSED(wl_output); SU_NOTUSED(description);
}

static sw_wayland_output_t *sw__wayland_output_create(uint32_t wl_name, sw_state_t *sw) {
	sw_wayland_output_t *output = NULL;
	if (sw->in.wayland.output_create && (output = sw->in.wayland.output_create(sw))) {
		output->private.sw = sw;
		output->private.wl_output = wl_registry_bind(sw->private.wayland.registry, wl_name, &wl_output_interface, 4);
		output->private.wl_name = wl_name;
		output->out.scale = 1;
		static struct wl_output_listener output_listener = {
			.geometry = sw__wayland_output_handle_geometry,
			.mode = sw__wayland_output_handle_mode,
			.done = sw__wayland_output_handle_done,
			.scale = sw__wayland_output_handle_scale,
			.name = sw__wayland_output_handle_name,
			.description = sw__wayland_output_handle_description,
		};
		wl_output_add_listener(output->private.wl_output, &output_listener, output);
	}

	return output;
}

static void sw__wayland_output_destroy(sw_wayland_output_t *output) {
	for (sw_wayland_surface_t *layer = output->private.sw->in.wayland.layers.head; layer; ) {
		sw_wayland_surface_t *next = layer->next;
		if (layer->in.layer.output == output) {
			sw__wayland_surface_destroy(layer);
		}
		layer = next;
	}
	if (output->private.wl_output) {
		wl_output_destroy(output->private.wl_output);
	}
	su_string_fini(&output->out.name, output->private.sw->in.gp_alloc);
	if (output->in.destroy) {
		output->in.destroy(output, output->private.sw);
	}
}

static void sw__wayland_pointer_handle_enter(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *wl_surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	SU_NOTUSED(wl_pointer);
	if (!wl_surface) {
		return;
	}

	sw_wayland_pointer_t *pointer = data;
	sw_wayland_surface_t *surface = wl_surface_get_user_data(wl_surface);

	if (pointer->private.cursor_shape_device) {
		wp_cursor_shape_device_v1_set_shape(pointer->private.cursor_shape_device,
				serial, surface->private.cursor_shape);
	}

	pointer->out.focused_surface = surface;
	pointer->private.enter_serial = serial;

	pointer->out.pos_x = (int32_t)(wl_fixed_to_double(surface_x) * (double)surface->out.scale);
	pointer->out.pos_y = (int32_t)(wl_fixed_to_double(surface_y) * (double)surface->out.scale);

	if (surface->in.enter) {
		surface->in.enter(pointer, pointer->private.sw);
	}
}

static void sw__wayland_pointer_handle_leave(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface) {
	SU_NOTUSED(wl_pointer); SU_NOTUSED(serial); SU_NOTUSED(surface);
	sw_wayland_pointer_t *pointer = data;
	if (pointer->out.focused_surface) {
		if (pointer->out.focused_surface->in.leave) {
			pointer->out.focused_surface->in.leave(pointer, pointer->private.sw);
		}
		pointer->out.focused_surface = NULL;
	}
}

static void sw__wayland_pointer_handle_motion(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	SU_NOTUSED(wl_pointer); SU_NOTUSED(time);
	sw_wayland_pointer_t *pointer = data;
	if (!pointer->out.focused_surface) {
		return;
	}

	int32_t x = (int32_t)(wl_fixed_to_double(surface_x) * (double)pointer->out.focused_surface->out.scale);
	int32_t y = (int32_t)(wl_fixed_to_double(surface_y) * (double)pointer->out.focused_surface->out.scale);
	if ((x != pointer->out.pos_x) || (y != pointer->out.pos_y)) {
		pointer->out.pos_x = x;
		pointer->out.pos_y = y;
		if (pointer->out.focused_surface->in.motion) {
			pointer->out.focused_surface->in.motion(pointer, pointer->private.sw);
		}
	}
}

static void sw__wayland_pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t state_) {
	SU_NOTUSED(wl_pointer); SU_NOTUSED(time);
	sw_wayland_pointer_t *pointer = data;
	if (!pointer->out.focused_surface) {
		return;
	}

	pointer->out.btn_code = button;
	pointer->out.btn_state = state_;

	pointer->private.button_serial = serial;

	if (pointer->out.focused_surface->in.button) {
		pointer->out.focused_surface->in.button(pointer, pointer->private.sw);
	}
}

static void sw__wayland_pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	SU_NOTUSED(wl_pointer); SU_NOTUSED(data); SU_NOTUSED(time);
	sw_wayland_pointer_t *pointer = data;
	if (!pointer->out.focused_surface) {
		return;
	}

	pointer->out.scroll_axis = axis;
	pointer->out.scroll_vector_length = wl_fixed_to_double(value);

	if (pointer->out.focused_surface->in.scroll) {
		pointer->out.focused_surface->in.scroll(pointer, pointer->private.sw);
	}
}

static sw_wayland_pointer_t *sw__wayland_pointer_create(sw_wayland_seat_t *seat, sw_state_t *sw) {
	sw_wayland_pointer_t *pointer = NULL;
	if (seat->in.pointer_create && (pointer = seat->in.pointer_create(sw))) {
		pointer->private.sw = sw;
		pointer->out.seat = seat;
		pointer->private.wl_pointer = wl_seat_get_pointer(seat->private.wl_seat);
		if (sw->private.wayland.cursor_shape_manager) {
			pointer->private.cursor_shape_device = wp_cursor_shape_manager_v1_get_pointer(
				sw->private.wayland.cursor_shape_manager, pointer->private.wl_pointer);
		} else {
			pointer->private.cursor_shape_device = NULL;
		}
		static struct wl_pointer_listener pointer_listener = {
			.enter = sw__wayland_pointer_handle_enter,
			.leave = sw__wayland_pointer_handle_leave,
			.motion = sw__wayland_pointer_handle_motion,
			.button = sw__wayland_pointer_handle_button,
			.axis = sw__wayland_pointer_handle_axis,
		};
		wl_pointer_add_listener(pointer->private.wl_pointer, &pointer_listener, pointer);
	}

	return pointer;
}

static void sw__wayland_pointer_destroy(sw_wayland_pointer_t *pointer) {
	if (pointer->private.cursor_shape_device) {
		wp_cursor_shape_device_v1_destroy(pointer->private.cursor_shape_device);
	}
	if (pointer->private.wl_pointer) {
		wl_pointer_destroy(pointer->private.wl_pointer);
	}
	if (pointer->in.destroy) {
		pointer->in.destroy(pointer, pointer->private.sw);
	}
}

static void sw__wayland_seat_handle_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
	SU_NOTUSED(wl_seat);
	sw_wayland_seat_t *seat = data;

	su_bool32_t have_pointer = (capabilities & WL_SEAT_CAPABILITY_POINTER);
	// TODO: touch, keyboard

	if (have_pointer && !seat->out.pointer) {
		seat->out.pointer = sw__wayland_pointer_create(seat, seat->private.sw);
	} else if (!have_pointer && seat->out.pointer) {
		sw__wayland_pointer_destroy(seat->out.pointer);
		seat->out.pointer = NULL;
	}
}

static void sw__wayland_seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name) {
	SU_NOTUSED(wl_seat);
	sw_wayland_seat_t *seat = data;
	size_t len = strlen(name);
	if (len > 0) {
		su_string_init_len(&seat->out.name, seat->private.sw->in.gp_alloc, (char *)name, len, SU_TRUE);
	}
}

static sw_wayland_seat_t *sw__wayland_seat_create(uint32_t wl_name, sw_state_t *sw) {
	sw_wayland_seat_t *seat = NULL;
	if (sw->in.wayland.seat_create && (seat = sw->in.wayland.seat_create(sw))) {
		seat->private.sw = sw;
		seat->private.wl_seat = wl_registry_bind(sw->private.wayland.registry, wl_name, &wl_seat_interface, 2);
		seat->private.wl_name = wl_name;
		static struct wl_seat_listener seat_listener = {
			.capabilities = sw__wayland_seat_handle_capabilities,
			.name = sw__wayland_seat_handle_name,
		};
		wl_seat_add_listener(seat->private.wl_seat, &seat_listener, seat);
	}

	return seat;
}

static void sw__wayland_seat_destroy(sw_wayland_seat_t *seat) {
	if (seat->out.pointer) {
		sw__wayland_pointer_destroy(seat->out.pointer);
	}
	if (seat->private.wl_seat) {
		wl_seat_destroy(seat->private.wl_seat);
	}
	su_string_fini(&seat->out.name, seat->private.sw->in.gp_alloc);
	if (seat->in.destroy) {
		seat->in.destroy(seat, seat->private.sw);
	}
}

static void sw__wayland_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
	SU_NOTUSED(xdg_wm_base);
	sw_state_t *sw = data;
	xdg_wm_base_pong(sw->private.wayland.wm_base, serial);
}

static void sw__wayland_registry_handle_global(void *data, struct wl_registry *wl_registry,
		uint32_t wl_name, const char *interface, uint32_t version) {
	SU_NOTUSED(wl_registry); SU_NOTUSED(version);
	sw_state_t *sw = data;
	if (strcmp(interface, wl_output_interface.name) == 0) {
		sw_wayland_output_t *output = sw__wayland_output_create(wl_name, sw);
		if (output) {
			su_llist__sw_wayland_output_t__insert_tail(&sw->out.wayland.outputs, output);
		}
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		sw_wayland_seat_t *seat = sw__wayland_seat_create(wl_name, sw);
		if (seat) {
			su_llist__sw_wayland_seat_t__insert_tail(&sw->out.wayland.seats, seat);
		}
    } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
		sw->private.wayland.compositor = wl_registry_bind(sw->private.wayland.registry, wl_name,
			&wl_compositor_interface, 6);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		sw->private.wayland.shm = wl_registry_bind(sw->private.wayland.registry, wl_name, &wl_shm_interface, 1);
		// ? TODO: wl_shm_add_listener (check for ARGB32)
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		sw->private.wayland.layer_shell = wl_registry_bind(sw->private.wayland.registry, wl_name,
			&zwlr_layer_shell_v1_interface, 2);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		sw->private.wayland.wm_base = wl_registry_bind(sw->private.wayland.registry,
			wl_name, &xdg_wm_base_interface, 3);
		static struct xdg_wm_base_listener wm_base_listener = {
			.ping = sw__wayland_wm_base_handle_ping,
		};
		xdg_wm_base_add_listener(sw->private.wayland.wm_base, &wm_base_listener, sw);
	} else if (strcmp(interface, wp_cursor_shape_manager_v1_interface.name) == 0) {
		sw->private.wayland.cursor_shape_manager = wl_registry_bind(sw->private.wayland.registry, wl_name,
			&wp_cursor_shape_manager_v1_interface, 1);
	}
}

static void sw__wayland_registry_handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
	SU_NOTUSED(wl_registry);
	sw_state_t *sw = data;
	for (sw_wayland_output_t *output = sw->out.wayland.outputs.head; output; output = output->next) {
		if (output->private.wl_name == name) {
			su_llist__sw_wayland_output_t__pop(&sw->out.wayland.outputs, output);
			sw__wayland_output_destroy(output);
			return;
		}
	}

	for (sw_wayland_seat_t *seat = sw->out.wayland.seats.head; seat; seat = seat->next) {
		if (seat->private.wl_name == name) {
			su_llist__sw_wayland_seat_t__pop(&sw->out.wayland.seats, seat);
			sw__wayland_seat_destroy(seat);
			return;
		}
	}
}

#endif // SW_WITH_WAYLAND

SW_EXPORT su_bool32_t sw_init(sw_state_t *sw) {
	SU_ASSERT(locale_is_utf8());

	*sw = (sw_state_t){
		.in = sw->in,
	};
	sw->out.t = -1;

	switch (sw->in.type) {
#if SW_WITH_WAYLAND
	case SW_BACKEND_TYPE_WAYLAND:
		// ? TODO: add all data from wl_* to sw_wayland_*.out

		sw->private.wayland.display = wl_display_connect(NULL);
		if (!sw->private.wayland.display) {
			goto error;
		}

		static struct wl_registry_listener registry_listener = {
		    .global = sw__wayland_registry_handle_global,
			.global_remove = sw__wayland_registry_handle_global_remove,
		};
		sw->private.wayland.registry = wl_display_get_registry(sw->private.wayland.display);
		wl_registry_add_listener(sw->private.wayland.registry, &registry_listener, sw);
		if (wl_display_roundtrip(sw->private.wayland.display) == -1) {
			goto error;
		}

		if (!sw->private.wayland.layer_shell) {
			errno = EPROTONOSUPPORT;
			goto error;
		}
		if (!sw->private.wayland.wm_base) {
			errno = EPROTONOSUPPORT;
			goto error;
		}
		if (!sw->private.wayland.cursor_shape_manager) {
			// TODO: warning
		}

		if (wl_display_roundtrip(sw->private.wayland.display) == -1) {
			goto error;
		}

		sw->out.wayland.pfd = (struct pollfd){
			.fd = wl_display_get_fd(sw->private.wayland.display),
			.events = POLLIN,
		};
		break;
#endif // SW_WITH_WAYLAND
	default:
		ASSERT_UNREACHABLE;
	}

//#if SW_WITH_SVG
//	// ? TODO: resvg_init_log();
//#endif // SW_WITH_SVG

	su_hash_table__struct_sw__image_cache__init(&sw->private.image_cache, sw->in.gp_alloc, 512);

#if SW_WITH_TEXT
	if (!fcft_init(FCFT_LOG_COLORIZE_NEVER, SU_FALSE, FCFT_LOG_CLASS_ERROR)) {
		goto error;
	}

	su_hash_table__struct_sw__text_run_cache__init(&sw->private.text_run_cache, sw->in.gp_alloc, 1024);
#endif // SW_WITH_TEXT

	return sw_process(sw);
error:
	return SU_FALSE;
}

SW_EXPORT void sw_fini(sw_state_t *sw) {
	//SU_ASSERT(locale_is_utf8());

	switch (sw->in.type) {
#if SW_WITH_WAYLAND
	case SW_BACKEND_TYPE_WAYLAND:
		for (sw_wayland_output_t *output = sw->out.wayland.outputs.head; output; ) {
			sw_wayland_output_t *next = output->next;
			sw__wayland_output_destroy(output);
			output = next;
		}

		for (sw_wayland_seat_t *seat = sw->out.wayland.seats.head; seat; ) {
			sw_wayland_seat_t *next = seat->next;
			sw__wayland_seat_destroy(seat);
			seat = next;
		}

		if (sw->private.wayland.cursor_shape_manager) {
			wp_cursor_shape_manager_v1_destroy(sw->private.wayland.cursor_shape_manager);
		}
		if (sw->private.wayland.layer_shell) {
			zwlr_layer_shell_v1_destroy(sw->private.wayland.layer_shell);
		}
		if (sw->private.wayland.wm_base) {
			xdg_wm_base_destroy(sw->private.wayland.wm_base);
		}
		if (sw->private.wayland.shm) {
			wl_shm_destroy(sw->private.wayland.shm);
		}
		if (sw->private.wayland.compositor) {
			wl_compositor_destroy(sw->private.wayland.compositor);
		}
		if (sw->private.wayland.registry) {
			wl_registry_destroy(sw->private.wayland.registry);
		}
		if (sw->private.wayland.display) {
			wl_display_flush(sw->private.wayland.display);
			wl_display_disconnect(sw->private.wayland.display);
		}
		break;
#endif // SW_WITH_WAYLAND
	default:
		ASSERT_UNREACHABLE;
	}

	for (size_t i = 0; i < sw->private.image_cache.items.len; ++i) {
		struct sw__image_cache cache = su_array__struct_sw__image_cache__get(&sw->private.image_cache.items, i);
		su_string_fini(&cache.key, sw->in.gp_alloc);
		if (cache.image) {
			pixman_image_unref(cache.image);
		}
	}
	su_hash_table__struct_sw__image_cache__fini(&sw->private.image_cache, sw->in.gp_alloc);

#if SW_WITH_TEXT
	for (size_t i = 0; i < sw->private.text_run_cache.items.len; ++i) {
		struct sw__text_run_cache cache = su_array__struct_sw__text_run_cache__get(&sw->private.text_run_cache.items, i);
		su_string_fini(&cache.key, sw->in.gp_alloc);
		for (size_t j = 0; j < cache.items.len; ++j) {
			struct sw__text_run_cache_entry entry = su_array__struct_sw__text_run_cache_entry__get(&cache.items, j);
			fcft_text_run_destroy(entry.text_run);
			fcft_destroy(entry.font);
		}
		su_array__struct_sw__text_run_cache_entry__fini(&cache.items, sw->in.gp_alloc);
	}
	su_hash_table__struct_sw__text_run_cache__fini(&sw->private.text_run_cache, sw->in.gp_alloc);

	fcft_fini();
#endif // SW_WITH_TEXT

	*sw = (sw_state_t){
		.in = sw->in,
	};
}

SW_EXPORT su_bool32_t sw_flush(sw_state_t *sw) {
	//SU_ASSERT(locale_is_utf8());

	switch (sw->in.type) {
#if SW_WITH_WAYLAND
	case SW_BACKEND_TYPE_WAYLAND:
		sw->out.wayland.pfd.events = POLLIN;
		if (wl_display_flush(sw->private.wayland.display) == -1) {
			if (errno == EAGAIN) {
				sw->out.wayland.pfd.events = (POLLIN | POLLOUT);
			} else {
				return SU_FALSE;
			}
		}
		break;
#endif // SW_WITH_WAYLAND
	default:
		ASSERT_UNREACHABLE;
	}

	return SU_TRUE;
}

SW_EXPORT su_bool32_t sw_process(sw_state_t *sw) {
	SU_ASSERT(locale_is_utf8());

	switch (sw->in.type) {
#if SW_WITH_WAYLAND
	case SW_BACKEND_TYPE_WAYLAND:
		if (wl_display_prepare_read(sw->private.wayland.display) != -1) {
			if (wl_display_read_events(sw->private.wayland.display) == -1) {
				return SU_FALSE;
			}
		}
		wl_display_dispatch_pending(sw->private.wayland.display);
		break;
#endif // SW_WITH_WAYLAND
	default:
		ASSERT_UNREACHABLE;
	}

	return SU_TRUE;
}

SW_EXPORT void sw_set(sw_state_t *sw) {
	SU_ASSERT(locale_is_utf8());

	if (su_now_ms() >= sw->out.t) {
		sw->out.t = -1;
	}

	switch (sw->in.type) {
#if SW_WITH_WAYLAND
	case SW_BACKEND_TYPE_WAYLAND:
		for (sw_wayland_surface_t *surface = sw->in.wayland.layers.head; surface; surface = surface->next) {
			SU_ASSERT(surface->in.type == SW_WAYLAND_SURFACE_TYPE_LAYER);
			sw__wayland_surface_prepare(surface, NULL, sw);
		}
		break;
#endif // SW_WITH_WAYLAND
	default:
		ASSERT_UNREACHABLE;
	}
}

#endif // defined(SW_IMPLEMENTATION) && !defined(SW__REIMPLEMENTATION_GUARD)
