#if !defined(SW_HEADER)
#define SW_HEADER

#if !defined(SW_WITH_DEBUG)
#define SW_WITH_DEBUG 0
#endif /* !defined(SW_WITH_DEBUG) */
#if !defined(SW_WITH_WAYLAND)
#define SW_WITH_WAYLAND 1
#endif /* !defined(SW_WITH_WAYLAND) */
#if !defined(SW_WITH_TEXT)
#define SW_WITH_TEXT 1
#endif /* !defined(SW_WITH_TEXT) */
#if !defined(SW_WITH_PNG)
#define SW_WITH_PNG 1
#endif /* !defined(SW_WITH_PNG) */
#if !defined(SW_WITH_SVG)
#define SW_WITH_SVG 1
#endif /* !defined(SW_WITH_SVG) */
#if !defined(SW_WITH_JPG)
#define SW_WITH_JPG 1
#endif /* !defined(SW_WITH_JPG) */
#if !defined(SW_WITH_TGA)
#define SW_WITH_TGA 1
#endif /* !defined(SW_WITH_TGA) */
#if !defined(SW_WITH_BMP)
#define SW_WITH_BMP 1
#endif /* !defined(SW_WITH_BMP) */
#if !defined(SW_WITH_PSD)
#define SW_WITH_PSD 1
#endif /* !defined(SW_WITH_PSD) */
#if !defined(SW_WITH_GIF)
#define SW_WITH_GIF 1
#endif /* !defined(SW_WITH_GIF) */
#if !defined(SW_WITH_HDR)
#define SW_WITH_HDR 1
#endif /* !defined(SW_WITH_HDR) */
#if !defined(SW_WITH_PIC)
#define SW_WITH_PIC 1
#endif /* !defined(SW_WITH_PIC) */
#if !defined(SW_WITH_PNM)
#define SW_WITH_PNM 1
#endif /* !defined(SW_WITH_PNM) */

#if !defined(SW_EXPORT)
#define SW_EXPORT static
#endif

/* TODO: remove */
/*#define SW_IMPLEMENTATION*/
/*#define SW_STRIP_PREFIXES*/
#if defined(_XOPEN_SOURCE)
#define SW__USER_XOPEN_SOURCE _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif /* defined(_XOPEN_SOURCE) */
#define _XOPEN_SOURCE 700
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

#define SW__PRIVATE_FIELDS(size) size_t sw__private[size / sizeof(size_t)]

#if !defined(SU_WITH_DEBUG)
#define SU_WITH_DEBUG SW_WITH_DEBUG
#endif /* !defined(SU_WITH_DEBUG) */
#include "su.h"

SU_STATIC_ASSERT(SW_WITH_WAYLAND); /* TODO */

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

typedef struct sw_context sw_context_t;

SW_EXPORT su_bool32_t sw_init(sw_context_t *);
SW_EXPORT void sw_fini(sw_context_t *);
SW_EXPORT void sw_set(sw_context_t *);

SW_EXPORT su_bool32_t sw_flush(sw_context_t *);
SW_EXPORT su_bool32_t sw_process(sw_context_t *);

typedef enum sw_status {
	SW_STATUS_SUCCESS = 0,
	SW_STATUS_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER = 1,
	SW_STATUS_SURFACE_ERROR_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK = 2,
	SW_STATUS_SURFACE_ERROR_LAYOUT_FAILED = 3,
	SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE = 4
#if SW_WITH_TEXT
	,SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_FONT = 5
	,SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_TEXT = 6
#endif /* SW_WITH_TEXT */
} sw_status_t;

#if SW_WITH_WAYLAND

typedef enum sw_wayland_pointer_button_state {
	SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED = 0,
	SW_WAYLAND_POINTER_BUTTON_STATE_PRESSED = 1
} sw_wayland_pointer_button_state_t;

typedef enum sw_wayland_pointer_scroll_axis {
	SW_WAYLAND_POINTER_AXIS_VERTICAL_SCROLL,
	SW_WAYLAND_POINTER_AXIS_HORIZONTAL_SCROLL
} sw_wayland_pointer_scroll_axis_t;

typedef enum sw_wayland_output_transform {
	SW_WAYLAND_OUTPUT_TRANSFORM_NORMAL,
	SW_WAYLAND_OUTPUT_TRANSFORM_90,
	SW_WAYLAND_OUTPUT_TRANSFORM_180,
	SW_WAYLAND_OUTPUT_TRANSFORM_270,
	SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED,
	SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED_90,
	SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED_180,
	SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED_270
} sw_wayland_output_transform_t;

typedef struct sw_wayland_output sw_wayland_output_t;
typedef void (*sw_wayland_output_func_t)(sw_wayland_output_t *, sw_context_t *);
typedef sw_wayland_output_t *(*sw_wayland_output_create_func_t)(sw_context_t *);

typedef struct sw_wayland_output_in {
	sw_wayland_output_func_t destroy; /* may be NULL */
} sw_wayland_output_in_t;

typedef struct sw_wayland_output_out {
	su_string_t name;
	int32_t scale, width, height;
	sw_wayland_output_transform_t transform;
} sw_wayland_output_out_t;

SU_LLIST_DECLARE(sw_wayland_output_t);

struct sw_wayland_output {
    sw_wayland_output_in_t in;
	sw_wayland_output_out_t out;
	SW__PRIVATE_FIELDS(16);
	SU_LLIST_FIELDS(sw_wayland_output_t);
};

typedef struct sw_wayland_pointer sw_wayland_pointer_t;
typedef void (*sw_wayland_pointer_func_t)(sw_wayland_pointer_t *, sw_context_t *);
typedef sw_wayland_pointer_t *(*sw_wayland_pointer_create_func_t)(sw_context_t *);

typedef struct sw_wayland_seat sw_wayland_seat_t;
typedef struct sw_wayland_surface sw_wayland_surface_t;

typedef struct sw_wayland_pointer_in {
	sw_wayland_pointer_func_t destroy; /* may be NULL */
} sw_wayland_pointer_in_t;

typedef struct sw_wayland_pointer_out {
	sw_wayland_seat_t *seat;
	sw_wayland_surface_t *focused_surface;
	int32_t pos_x, pos_y;
	uint32_t btn_code;
	sw_wayland_pointer_button_state_t btn_state;
	SU_PAD32;
	sw_wayland_pointer_scroll_axis_t scroll_axis;
	double scroll_vector_length;
} sw_wayland_pointer_out_t;

struct sw_wayland_pointer {
	sw_wayland_pointer_in_t in;
	sw_wayland_pointer_out_t out;
	SW__PRIVATE_FIELDS(24);
};

SU_LLIST_DECLARE(sw_wayland_seat_t);

typedef void (*sw_wayland_seat_func_t)(sw_wayland_seat_t *, sw_context_t *);
typedef sw_wayland_seat_t *(*sw_wayland_seat_create_func_t)(sw_context_t *);

typedef struct sw_wayland_seat_in {
	sw_wayland_seat_func_t destroy; /* may be NULL */
	sw_wayland_pointer_create_func_t pointer_create; /* may be NULL */
} sw_wayland_seat_in_t;

typedef struct sw_wayland_seat_out {
	su_string_t name;
	sw_wayland_pointer_t *pointer;
} sw_wayland_seat_out_t;

struct sw_wayland_seat {
    sw_wayland_seat_in_t in;
	sw_wayland_seat_out_t out;
	SW__PRIVATE_FIELDS(16);
	SU_LLIST_FIELDS(sw_wayland_seat_t);
};

typedef enum sw_wayland_surface_layer_anchor {
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_NONE = 0,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_TOP = 1,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_BOTTOM = 2,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT = 4,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT = 8

#define SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL (SW_WAYLAND_SURFACE_LAYER_ANCHOR_TOP |  \
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_BOTTOM | SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT | \
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT)

} sw_wayland_surface_layer_anchor_t;

typedef enum sw_wayland_surface_layer_layer {
	SW_WAYLAND_SURFACE_LAYER_LAYER_BACKGROUND,
	SW_WAYLAND_SURFACE_LAYER_LAYER_BOTTOM,
	SW_WAYLAND_SURFACE_LAYER_LAYER_TOP,
	SW_WAYLAND_SURFACE_LAYER_LAYER_OVERLAY
} sw_wayland_surface_layer_layer_t;

typedef enum sw_wayland_cursor_shape {
	SW_WAYLAND_CURSOR_SHAPE_DEFAULT = 0, /* SW_WAYLAND_CURSOR_SHAPE_DEFAULT_ */
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
	SW_WAYLAND_CURSOR_SHAPE_ZOOM_OUT = 34
	/*SW_WAYLAND_CURSOR_SHAPE_DND_ASK = 35, */
	/*SW_WAYLAND_CURSOR_SHAPE_ALL_RESIZE = 36 */
} sw_wayland_cursor_shape_t;

typedef struct sw_wayland_region {
	int32_t x, y;
	int32_t width, height;
} sw_wayland_region_t;

SU_ARRAY_DECLARE(sw_wayland_region_t);

typedef enum sw_wayland_surface_popup_gravity {
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_NONE = 0,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_TOP = 1,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM = 2,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_LEFT = 3,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_RIGHT = 4,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_TOP_LEFT = 5,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM_LEFT = 6,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_TOP_RIGHT = 7,
	SW_WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM_RIGHT = 8
} sw_wayland_surface_popup_gravity_t;

typedef enum sw_wayland_surface_popup_constraint_adjustment {
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_NONE = 0,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_SLIDE_X = 1,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_SLIDE_Y = 2,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_X = 4,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_Y = 8,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_RESIZE_X = 16,
	SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_RESIZE_Y = 32
} sw_wayland_surface_popup_constraint_adjustment_t;

typedef enum sw_wayland_surface_type {
	SW_WAYLAND_SURFACE_TYPE_LAYER,
	SW_WAYLAND_SURFACE_TYPE_POPUP
} sw_wayland_surface_type_t;

typedef struct sw_layout_block sw_layout_block_t;

typedef void (*sw_wayland_surface_destroy_func_t)(sw_wayland_surface_t *, sw_context_t *);
typedef void (*sw_wayland_surface_error_func_t)(sw_wayland_surface_t *, sw_context_t *, sw_status_t status);

typedef struct sw_wayland_surface_layer {
	sw_wayland_output_t *output;
	int32_t exclusive_zone;
	uint32_t anchor; /* sw_wayland_surface_layer_anchor_t | */
	sw_wayland_surface_layer_layer_t layer;
	int32_t margins[4]; /* top right bottom left */
	SU_PAD32;
} sw_wayland_surface_layer_t;

typedef struct sw_wayland_surface_popup {
	int32_t x, y;
	sw_wayland_surface_popup_gravity_t gravity;
	uint32_t constraint_adjustment; /* sw_wayland_surface_popup_constraint_adjustment_t | */
	sw_wayland_pointer_t *grab;
} sw_wayland_surface_popup_t;

typedef union sw_wayland_surface_ {
	sw_wayland_surface_layer_t layer;
	sw_wayland_surface_popup_t popup;
} sw_wayland_surface__t;

SU_LLIST_DECLARE(sw_wayland_surface_t);

typedef struct sw_wayland_surface_in {
	sw_wayland_surface__t _;
	sw_wayland_surface_type_t type;
    sw_wayland_cursor_shape_t cursor_shape;
	int32_t width, height;
	/* ? TODO: prepare, prepared cbs */
	sw_wayland_surface_error_func_t error; /* may be NULL */
	sw_wayland_surface_destroy_func_t destroy; /* must call out.fini */
	sw_wayland_pointer_func_t enter; /* may be NULL */
	sw_wayland_pointer_func_t leave; /* may be NULL */
	sw_wayland_pointer_func_t motion; /* may be NULL */
	sw_wayland_pointer_func_t button; /* may be NULL */
	sw_wayland_pointer_func_t scroll; /* may be NULL */
    su_array__sw_wayland_region_t__t input_regions;
    su_llist__sw_wayland_surface_t__t popups;
    sw_layout_block_t *root;
} sw_wayland_surface_in_t;

typedef struct sw_wayland_surface_out {
	int32_t width, height, scale;
	SU_PAD32;
	sw_wayland_surface_destroy_func_t fini;
} sw_wayland_surface_out_t;

struct sw_wayland_surface {
    sw_wayland_surface_in_t in;
	sw_wayland_surface_out_t out;
	SW__PRIVATE_FIELDS(136);
	SU_LLIST_FIELDS(sw_wayland_surface_t);
};

#endif /* SW_WITH_WAYLAND */

typedef struct sw_point {
	/* ? TODO: % */
	int32_t x, y;
} sw_point_t;

typedef union sw_color_argb32 {
	uint32_t u32;
	struct { /* ? TODO: extract */
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	} c;
} sw_color_argb32_t;

typedef struct sw_color_gradient_stop {
	sw_color_argb32_t color;
	SU_PAD32;
	double pos; /* position along gradient line (0..1) */
} sw_color_gradient_stop_t;

SU_ARRAY_DECLARE(sw_color_gradient_stop_t);

typedef struct sw_color_linear_gradient {
	sw_point_t p1, p2;
	su_array__sw_color_gradient_stop_t__t stops;
} sw_color_linear_gradient_t;

typedef struct sw_color_conical_gradient {
	sw_point_t center;
	double angle;
	su_array__sw_color_gradient_stop_t__t stops;
} sw_color_conical_gradient_t;

typedef struct sw_color_radial_gradient {
	sw_point_t inner_p, outer_p;
	int32_t inner_r, outer_r;
	su_array__sw_color_gradient_stop_t__t stops;
} sw_color_radial_gradient_t;

typedef enum sw_color_type {
	SW_COLOR_TYPE_ARGB32,
	SW_COLOR_TYPE_LINEAR_GRADIENT,
	SW_COLOR_TYPE_CONICAL_GRADIENT,
	SW_COLOR_TYPE_RADIAL_GRADIENT
} sw_color_type_t;

typedef union sw_color_ {
	sw_color_argb32_t argb32;
	sw_color_linear_gradient_t linear_gradient;
	sw_color_conical_gradient_t conical_gradient;
	sw_color_radial_gradient_t radial_gradient;
} sw_color__t;

typedef struct sw_color {
	SU_PAD32;
	sw_color_type_t type;
	sw_color__t _;
} sw_color_t;

typedef enum sw_layout_block_type {
	SW_LAYOUT_BLOCK_TYPE_SPACER = 0
#if SW_WITH_TEXT
	,SW_LAYOUT_BLOCK_TYPE_TEXT = 1
#endif /* SW_WITH_TEXT */ 
	,SW_LAYOUT_BLOCK_TYPE_IMAGE = 2
	,SW_LAYOUT_BLOCK_TYPE_COMPOSITE = 3
} sw_layout_block_type_t;

typedef enum sw_layout_block_anchor {
	SW_LAYOUT_BLOCK_ANCHOR_LEFT, /* top in vertical layouts */
	SW_LAYOUT_BLOCK_ANCHOR_TOP, /* left in horizontal layouts */
	SW_LAYOUT_BLOCK_ANCHOR_RIGHT, /* bottom in vertical layouts */
	SW_LAYOUT_BLOCK_ANCHOR_BOTTOM, /* right in horizontal layouts */
	SW_LAYOUT_BLOCK_ANCHOR_CENTER,
	SW_LAYOUT_BLOCK_ANCHOR_NONE /* use x and y to position */
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
	SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_BOTTOM
} sw_layout_block_content_anchor_t;

typedef enum sw_layout_block_content_transform {
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_NORMAL,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_90,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_180,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_270,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_90,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_180,
	SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_270
} sw_layout_block_content_transform_t;

typedef enum sw_layout_block_expand {
	SW_LAYOUT_BLOCK_EXPAND_NONE = 0,
	SW_LAYOUT_BLOCK_EXPAND_LEFT = (1 << 1),
	SW_LAYOUT_BLOCK_EXPAND_RIGHT = (1 << 2),
	SW_LAYOUT_BLOCK_EXPAND_BOTTOM = (1 << 3),
	SW_LAYOUT_BLOCK_EXPAND_TOP = (1 << 4),
	SW_LAYOUT_BLOCK_EXPAND_CONTENT = (1 << 5)

#define SW_LAYOUT_BLOCK_EXPAND_ALL_SIDES \
	(SW_LAYOUT_BLOCK_EXPAND_LEFT | SW_LAYOUT_BLOCK_EXPAND_RIGHT \
	| SW_LAYOUT_BLOCK_EXPAND_BOTTOM | SW_LAYOUT_BLOCK_EXPAND_TOP)

#define SW_LAYOUT_BLOCK_EXPAND_ALL_SIDES_CONTENT \
	(SW_LAYOUT_BLOCK_EXPAND_LEFT | SW_LAYOUT_BLOCK_EXPAND_RIGHT \
	| SW_LAYOUT_BLOCK_EXPAND_BOTTOM | SW_LAYOUT_BLOCK_EXPAND_TOP \
	| SW_LAYOUT_BLOCK_EXPAND_CONTENT)

} sw_layout_block_expand_t;

typedef struct sw_layout_block_border {
	int32_t width;
	SU_PAD32;
	sw_color_t color;
} sw_layout_block_border_t;

typedef enum sw_layout_block_composite_children_layout {
	SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_HORIZONTAL,
	SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_VERTICAL
} sw_layout_block_composite_children_layout_t;

typedef struct sw_layout_block_dimensions {
	int32_t x, y, width, height;
	int32_t content_width, content_height;
	int32_t borders[4]; /* left right bottom top */ /* ? TODO: union/#define sides */
} sw_layout_block_dimensions_t;

#if SW_WITH_TEXT
typedef struct sw_layout_block_text {
	su_string_t text;
	su_array__su_string_t__t font_names;
	sw_color_t color;
} sw_layout_block_text_t;
#endif /* SW_WITH_TEXT */

typedef struct sw_pixmap {
	uint32_t width;
	uint32_t height;
	sw_color_argb32_t pixels[1]; /* flexible */
} sw_pixmap_t;

typedef enum sw_layout_block_image_image_type {
	SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_AUTO = 0
	,SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SW_PIXMAP = 1 /* see sw_pixmap_t */
#if SW_WITH_PNG
	,SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNG = 2
#endif /* SW_WITH_PNG */
#if SW_WITH_JPG
	,SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_JPG = 3
#endif /* SW_WITH_JPG */
#if SW_WITH_SVG
	,SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SVG = 4
#endif /* SW_WITH_SVG */
#if SW_WITH_TGA
	,SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_TGA = 5
#endif /* SW_WITH_TGA */
#if SW_WITH_BMP
	,SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_BMP = 6
#endif /* SW_WITH_BMP */
#if SW_WITH_PSD
	,SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PSD = 7
#endif /* SW_WITH_PSD */
#if SW_WITH_GIF
	,SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_GIF = 8
#endif /* SW_WITH_GIF */
#if SW_WITH_HDR
	,SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_HDR = 9
#endif /* SW_WITH_HDR */
#if SW_WITH_PIC
	,SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PIC = 10
#endif /* SW_WITH_PIC */
#if SW_WITH_PNM
	,SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNM = 11
#endif /* SW_WITH_PNM */
} sw_layout_block_image_image_type_t;

typedef struct sw_layout_block_image {
	SU_PAD32;
	sw_layout_block_image_image_type_t type;
	su_fat_ptr_t data;
} sw_layout_block_image_t;

SU_LLIST_DECLARE(sw_layout_block_t);

typedef struct sw_layout_block_composite {
	su_llist__sw_layout_block_t__t children;
	sw_layout_block_composite_children_layout_t layout;
	SU_PAD32;
} sw_layout_block_composite_t;

typedef su_bool32_t (*sw_layout_block_prepare_func_t)(sw_layout_block_t *, sw_context_t *);
typedef void (*sw_layout_block_error_func_t)(sw_layout_block_t *, sw_context_t *, sw_status_t);
typedef void (*sw_layout_block_fini_func_t)(sw_layout_block_t *, sw_context_t *);

typedef union sw_layout_block_ {
#if SW_WITH_TEXT
	sw_layout_block_text_t text;
#endif /* SW_WITH_TEXT */
	sw_layout_block_image_t image;
	sw_layout_block_composite_t composite;
} sw_layout_block__t;

typedef struct sw_layout_block_in {
	sw_layout_block__t _;
	sw_layout_block_type_t type;
	sw_layout_block_anchor_t anchor;
	sw_color_t color;
	uint32_t expand; /* sw_layout_block_expand_t | */
	int32_t x, y;
	int32_t min_width, max_width;
	int32_t min_height, max_height;
	int32_t content_width, content_height;
	sw_layout_block_content_anchor_t content_anchor;
	sw_layout_block_content_transform_t content_transform;
	SU_PAD32;
	sw_layout_block_border_t borders[4]; /* left right bottom top */
	sw_layout_block_prepare_func_t prepare; /* may be NULL */
	sw_layout_block_prepare_func_t prepared; /* may be NULL */
	sw_layout_block_error_func_t error; /* may be NULL */
} sw_layout_block_in_t;

typedef struct sw_layout_block_out {
	sw_layout_block_dimensions_t dim;
	sw_layout_block_fini_func_t fini; /* must be called at destruction */
} sw_layout_block_out_t;

struct sw_layout_block {
	sw_layout_block_in_t in;
	sw_layout_block_out_t out;
	SW__PRIVATE_FIELDS(8);
	SU_LLIST_FIELDS(sw_layout_block_t);
};

typedef enum sw_backend_type {
#if SW_WITH_WAYLAND
	SW_BACKEND_TYPE_WAYLAND
#endif /* SW_WITH_WAYLAND */
} sw_backend_type_t;

#if SW_WITH_WAYLAND
typedef struct sw_backend_wayland_in {
	sw_wayland_output_create_func_t output_create; /* may be NULL */
	sw_wayland_seat_create_func_t seat_create; /* may be NULL */
	su_llist__sw_wayland_surface_t__t layers;
} sw_backend_wayland_in_t;
#endif /* SW_WITH_WAYLAND */

typedef union sw_backend_in {
#if SW_WITH_WAYLAND
	sw_backend_wayland_in_t wayland;
#endif /* SW_WITH_WAYLAND */
} sw_backend_in_t;

#if SW_WITH_WAYLAND
typedef struct sw_backend_wayland_out {
	struct pollfd pfd;
	su_llist__sw_wayland_output_t__t outputs;
	su_llist__sw_wayland_seat_t__t seats;
} sw_backend_wayland_out_t;
#endif /* SW_WITH_WAYLAND */

typedef union sw_backend_out {
#if SW_WITH_WAYLAND
	sw_backend_wayland_out_t wayland;
#endif /* SW_WITH_WAYLAND */
} sw_backend_out_t;

typedef struct sw_context_in {
	su_allocator_t *gp_alloc;
	su_allocator_t *scratch_alloc;
	SU_PAD32;
	sw_backend_type_t backend_type;
	sw_backend_in_t backend;
} sw_context_in_t;

typedef struct sw_context_out {
	int64_t t; /* absolute timeout in ms at which to call sw_set */
	sw_backend_out_t backend;
} sw_context_out_t;

struct sw_context {
	sw_context_in_t in;
	sw_context_out_t out;
	SW__PRIVATE_FIELDS(104);
};

#if defined(SW_STRIP_PREFIXES)

typedef sw_status_t status_t;

#if SW_WITH_WAYLAND

typedef sw_wayland_output_in_t wayland_output_in_t;
typedef sw_wayland_output_out_t wayland_output_out_t;
typedef sw_wayland_output_t wayland_output_t;
typedef sw_wayland_output_transform_t wayland_output_transform_t;
typedef sw_wayland_output_func_t wayland_output_func_t;
typedef sw_wayland_output_create_func_t wayland_output_create_func_t;
typedef sw_wayland_pointer_button_state_t wayland_pointer_button_state_t;
typedef sw_wayland_pointer_scroll_axis_t wayland_pointer_scroll_axis_t;
typedef sw_wayland_pointer_in_t wayland_pointer_in_t;
typedef sw_wayland_pointer_out_t wayland_pointer_out_t;
typedef sw_wayland_pointer_t wayland_pointer_t;
typedef sw_wayland_pointer_func_t wayland_pointer_func_t;
typedef sw_wayland_pointer_create_func_t wayland_pointer_create_func_t;
typedef sw_wayland_seat_in_t wayland_seat_in_t;
typedef sw_wayland_seat_out_t wayland_seat_out_t;
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
typedef sw_wayland_surface_layer_t wayland_surface_layer_t;
typedef sw_wayland_surface_popup_t wayland_surface_popup_t;
typedef sw_wayland_surface__t wayland_surface__t;
typedef sw_wayland_surface_in_t wayland_surface_in_t;
typedef sw_wayland_surface_out_t wayland_surface_out_t;
typedef sw_wayland_surface_t wayland_surface_t;
typedef sw_wayland_surface_destroy_func_t wayland_surface_destroy_func_t;
typedef sw_wayland_surface_error_func_t wayland_surface_error_func_t;

typedef sw_backend_wayland_in_t backend_wayland_in_t;
typedef sw_backend_wayland_out_t backend_wayland_out_t;

#endif /* SW_WITH_WAYLAND */

typedef sw_pixmap_t pixmap_t;
typedef sw_point_t point_t;

typedef sw_color_argb32_t color_argb32_t;
typedef sw_color_gradient_stop_t color_gradient_stop_t;
typedef sw_color_linear_gradient_t color_linear_gradient_t;
typedef sw_color_conical_gradient_t color_conical_gradient_t;
typedef sw_color_radial_gradient_t color_radial_gradient_t;
typedef sw_color__t color__t;
typedef sw_color_type_t color_type_t;
typedef sw_color_t color_t;

typedef sw_layout_block_type_t layout_block_type_t;
typedef sw_layout_block_anchor_t layout_block_anchor_t;
typedef sw_layout_block_content_anchor_t layout_block_content_anchor_t;
typedef sw_layout_block_content_transform_t layout_block_content_transform_t;
typedef sw_layout_block_expand_t layout_block_expand_t;
typedef sw_layout_block_border_t layout_block_border_t;
typedef sw_layout_block_composite_children_layout_t layout_block_composite_children_layout_t;
typedef sw_layout_block_dimensions_t layout_block_dimensions_t;
typedef sw_layout_block_text_t layout_block_text_t;
typedef sw_layout_block_image_image_type_t layout_block_image_image_type_t;
typedef sw_layout_block_image_t layout_block_image_t;
typedef sw_layout_block_composite_t layout_block_composite_t;
typedef sw_layout_block__t layout_block__t;
typedef sw_layout_block_in_t layout_block_in_t;
typedef sw_layout_block_out_t layout_block_out_t;
typedef sw_layout_block_t layout_block_t;
typedef sw_layout_block_prepare_func_t layout_block_prepare_func_t;
typedef sw_layout_block_fini_func_t layout_block_fini_func_t;
typedef sw_layout_block_error_func_t layout_block_error_func_t;

typedef sw_backend_type_t backend_type_t;
typedef sw_backend_in_t backend_in_t;
typedef sw_backend_out_t backend_out_t;

typedef sw_context_in_t context_in_t;
typedef sw_context_out_t context_out_t;
typedef sw_context_t context_t;

/*#define init sw_init */
/*#define fini sw_fini */
/*#define set sw_set */
/*#define flush sw_flush */
/*#define process sw_process */

#endif /* defined(SW_STRIP_PREFIXES) */

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#undef _XOPEN_SOURCE
#if defined(SW__USER_XOPEN_SOURCE)
#define _XOPEN_SOURCE SW__USER_XOPEN_SOURCE
#endif /* defined(SW__USER_XOPEN_SOURCE) */

#endif /* SW_HEADER */


#if defined(SW_IMPLEMENTATION) && !defined(SW__REIMPLEMENTATION_GUARD)
#define SW__REIMPLEMENTATION_GUARD

#if !defined(SU_IMPLEMENTATION)
#define SU_IMPLEMENTATION
#endif /* !defined(SU_IMPLEMENTATION) */
#include "su.h"

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

SU_IGNORE_WARNINGS_START

SU_IGNORE_WARNING("-Wc99-extensions")
SU_IGNORE_WARNING("-Wcomment")
SU_IGNORE_WARNING("-Wc++98-compat-pedantic")

#if SU_HAS_INCLUDE(<pixman/pixman.h>)
#include <pixman/pixman.h>
#else
#include <pixman.h>
#endif /* SU_HAS_INCLUDE(<pixman.h>) */

#if SW_WITH_TEXT
#if SU_HAS_INCLUDE(<fcft.h>)
#include <fcft.h>
#else
#include <fcft/fcft.h>
#endif /* SU_HAS_INCLUDE(<fcft.h>) */
#endif /* SW_WITH_TEXT */

#if SW_WITH_SVG
#if SU_HAS_INCLUDE(<resvg/resvg.h>)
#include <resvg/resvg.h>
#elif SU_HAS_INCLUDE(<resvg.h>)
#include <resvg.h>
#else
#include "resvg.h"
#endif
#endif /* SW_WITH_SVG */

SU_IGNORE_WARNINGS_END

#if SW_WITH_PNG || SW_WITH_JPG || SW_WITH_TGA || SW_WITH_BMP || SW_WITH_PSD || SW_WITH_GIF || SW_WITH_HDR || SW_WITH_PIC || SW_WITH_PNM

static void *sw__malloc_stbi(size_t size);
static void sw__free_stbi(void *ptr);
static void *sw__realloc_sized_stbi(void *ptr, size_t old_size, size_t new_size);

#define STBI_NO_STDIO
#define STBI_ASSERT SU_ASSERT
#define STBI_MALLOC sw__malloc_stbi
#define STBI_FREE sw__free_stbi
#define STBI_REALLOC_SIZED sw__realloc_sized_stbi

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_FAILURE_STRINGS

#if !SW_WITH_PNG
#define STBI_NO_PNG
#endif /* !SW_WITH_PNG */
#if !SW_WITH_JPG
#define STBI_NO_JPEG
#endif /* !SW_WITH_JPG */
#if !SW_WITH_TGA
#define STBI_NO_TGA
#endif /* !SW_WITH_TGA */
#if !SW_WITH_BMP
#define STBI_NO_BMP
#endif /* !SW_WITH_BMP */
#if !SW_WITH_PSD
#define STBI_NO_PSD
#endif /* !SW_WITH_PSD */
#if !SW_WITH_GIF
#define STBI_NO_GIF
#endif /*  !SW_WITH_GIF */
#if !SW_WITH_HDR
#define STBI_NO_HDR
#endif /* !SW_WITH_HDR */
#if !SW_WITH_PIC
#define STBI_NO_PIC
#endif /* !SW_WITH_PIC */
#if !SW_WITH_PNM
#define STBI_NO_PNM
#endif /* !SW_WITH_PNM */

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
SU_IGNORE_WARNING("-Wcomment")
SU_IGNORE_WARNING("-Wcomma")
SU_IGNORE_WARNING("-Wzero-as-null-pointer-constant")
SU_IGNORE_WARNING("-Wcast-qual")
SU_IGNORE_WARNING("-Wuseless-cast")

#include "stb_image.h"

SU_IGNORE_WARNINGS_END

#endif /* SW_WITH_PNG || SW_WITH_JPG || SW_WITH_TGA || SW_WITH_BMP || SW_WITH_PSD || SW_WITH_GIF || SW_WITH_HDR || SW_WITH_PIC || SW_WITH_PNM */

#if SW_WITH_WAYLAND

SU_IGNORE_WARNINGS_START

SU_IGNORE_WARNING("-Wc99-extensions")
SU_IGNORE_WARNING("-Wc++98-compat-pedantic")
SU_IGNORE_WARNING("-Wcast-qual")

#if SU_HAS_INCLUDE(<wayland-client/wayland-client.h>)
#include <wayland-client/wayland-client.h>
#else
#include <wayland-client.h>
#endif

#include "wlr-layer-shell-unstable-v1.h"
#include "xdg-shell.h"
#include "cursor-shape-v1.h"

SU_IGNORE_WARNINGS_END

typedef struct sw__wayland_output {
	uint32_t wl_name;
	SU_PAD32;
	struct wl_output *wl_output;
} sw__wayland_output_t;

typedef struct sw__wayland_pointer {
	struct wl_pointer *wl_pointer;
	struct wp_cursor_shape_device_v1 *cursor_shape_device;
	uint32_t enter_serial, button_serial;
} sw__wayland_pointer_t;

typedef struct sw__wayland_seat {
	struct wl_seat *wl_seat;
	uint32_t wl_name;
	SU_PAD32;
} sw__wayland_seat_t;

typedef struct sw__wayland_surface_buffer {
	struct wl_buffer *wl_buffer;
	pixman_image_t *image; /* bits */
	uint32_t *pixels;
	uint32_t size;
	su_bool32_t busy;
} sw__wayland_surface_buffer_t;

typedef struct sw__wayland_surface_layer {
	struct zwlr_layer_surface_v1 *layer_surface;
	int32_t exclusive_zone;
	uint32_t anchor; /* sw_wayland_surface_layer_anchor_t | */
	sw_wayland_surface_layer_layer_t layer;
	int32_t margins[4]; /* top right bottom left */
	SU_PAD32;
} sw__wayland_surface_layer_t;

typedef struct sw__wayland_surface_popup {
	sw_wayland_surface_t *parent;
	sw_wayland_pointer_t *grab;
	struct xdg_surface *xdg_surface;
	struct xdg_popup *xdg_popup;
	struct xdg_positioner *xdg_positioner;
	int32_t x, y;
    sw_wayland_surface_popup_gravity_t gravity;
    uint32_t constraint_adjustment; /* sw_wayland_surface_popup_constraint_adjustment_t | */
} sw__wayland_surface_popup_t;

typedef union sw__wayland_surface_ {
	sw__wayland_surface_layer_t layer;
	sw__wayland_surface_popup_t popup;
} sw__wayland_surface__t;

typedef struct sw__wayland_surface {
	sw__wayland_surface__t _;
	struct wl_surface *wl_surface;
	sw__wayland_surface_buffer_t buffer;
	su_bool32_t dirty;
	sw_wayland_cursor_shape_t cursor_shape;
	sw_wayland_output_t *output;
	su_array__sw_wayland_region_t__t input_regions;
} sw__wayland_surface_t;

SU_LLIST_DEFINE(sw_wayland_output_t)
SU_LLIST_DEFINE(sw_wayland_seat_t)
SU_ARRAY_DEFINE(sw_wayland_region_t)
SU_LLIST_DEFINE(sw_wayland_surface_t)

#endif /* SW_WITH_WAYLAND */

typedef struct sw__layout_block {
	pixman_image_t *content_image;
} sw__layout_block_t;

#if SW_WITH_GIF
typedef struct sw__image_gif_frame {
	pixman_image_t *image; /* bits */
	uint16_t delay; /* ms */
	SU_PAD16;
	SU_PAD32;
} sw__image_gif_frame_t;

SU_ARRAY_DECLARE_DEFINE(sw__image_gif_frame_t)

typedef struct sw__image_multiframe_gif {
	su_array__sw__image_gif_frame_t__t frames;
	size_t frame_idx;
	int64_t frame_end; /* absolute ms */
} sw__image_multiframe_gif_t;
#endif /* SW_WITH_GIF */

#if SW_WITH_SVG || SW_WITH_GIF
typedef enum sw__image_data_type {
	SW__IMAGE_DATA_TYPE_NONE = 0
#if SW_WITH_SVG
	,SW__IMAGE_DATA_TYPE_SVG = 1
#endif /* SW_WITH_SVG */
#if SW_WITH_GIF
	,SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF = 2
#endif /* SW_WITH_GIF */
} sw__image_data_type_t;
#endif /* SW_WITH_SVG || SW_WITH_GIF */

typedef struct sw__image_data {
#if SW_WITH_SVG || SW_WITH_GIF
	SU_PAD32;
	sw__image_data_type_t type;
	void *data;
#endif /* SW_WITH_SVG || SW_WITH_GIF */

	uint32_t *pixels;
} sw__image_data_t;

typedef struct sw__image_cache {
	SU_HASH_TABLE_FIELDS(su_fat_ptr_t);
	pixman_image_t *image; /* bits */
} sw__image_cache_t;

/* TODO: better hash function */
SU_HASH_TABLE_DECLARE_DEFINE(sw__image_cache_t, su_fat_ptr_t, su_stbds_hash, su_fat_ptr_equal, 16)

#if SW_WITH_TEXT
typedef struct sw__text_run_cache_entry {
	struct fcft_font *font;
	struct fcft_text_run *text_run;
} sw__text_run_cache_entry_t;

SU_ARRAY_DECLARE_DEFINE(sw__text_run_cache_entry_t)

typedef struct sw__text_run_cache {
	SU_HASH_TABLE_FIELDS(su_string_t);
	su_array__sw__text_run_cache_entry_t__t items;
} sw__text_run_cache_t;

/* TODO: better hash function */
SU_HASH_TABLE_DECLARE_DEFINE(sw__text_run_cache_t, su_string_t, su_stbds_hash_string, su_string_equal, 16)
#endif /* SW_WITH_TEXT */

typedef struct sw__context_wayland {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct xdg_wm_base *wm_base;
	struct wp_cursor_shape_manager_v1 *cursor_shape_manager;
} sw__context_wayland_t;

typedef struct sw__context {
#if SW_WITH_WAYLAND
	sw__context_wayland_t wayland;
#endif /* SW_WITH_WAYLAND */

	/* TODO: arena for content: */
	su_hash_table__sw__image_cache_t__t image_cache;
#if SW_WITH_TEXT
	su_hash_table__sw__text_run_cache_t__t text_run_cache;
#endif /* SW_WITH_TEXT */
} sw__context_t;

SU_LLIST_DEFINE(sw_layout_block_t)
SU_ARRAY_DEFINE(sw_color_gradient_stop_t)


static SU_THREAD_LOCAL sw_context_t *sw__context;

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

SU_STATIC_ASSERT(sizeof(sw__context->sw__private) == sizeof(sw__context_t));
#if SW_WITH_WAYLAND
SU_STATIC_ASSERT(sizeof(sw__context->out.backend.wayland.outputs.head->sw__private) == sizeof(sw__wayland_output_t));
SU_STATIC_ASSERT(sizeof(sw__context->out.backend.wayland.seats.head->sw__private) == sizeof(sw__wayland_seat_t));
SU_STATIC_ASSERT(sizeof(sw__context->out.backend.wayland.seats.head->out.pointer->sw__private) == sizeof(sw__wayland_pointer_t));
SU_STATIC_ASSERT(sizeof(sw__context->in.backend.wayland.layers.head->sw__private) == sizeof(sw__wayland_surface_t));
/* TODO: assert outside SW_WITH_WAYLAND */
SU_STATIC_ASSERT(sizeof(sw__context->in.backend.wayland.layers.head->in.root->sw__private) == sizeof(sw__layout_block_t));
#endif /* sw__layout_block_t */

#if SW_WITH_PNG || SW_WITH_JPG || SW_WITH_TGA || SW_WITH_BMP || SW_WITH_PSD || SW_WITH_GIF || SW_WITH_HDR || SW_WITH_PIC || SW_WITH_PNM
static void *sw__malloc_stbi(size_t size) {
	return sw__context->in.scratch_alloc->alloc(sw__context->in.scratch_alloc, size, 64);
}

static void sw__free_stbi(void *ptr) {
	sw__context->in.scratch_alloc->free(sw__context->in.scratch_alloc, ptr);
}

static void *sw__realloc_sized_stbi(void *ptr, size_t old_size, size_t new_size) {
	su_allocator_t *scratch_alloc = sw__context->in.scratch_alloc;
	void *ret = scratch_alloc->alloc(scratch_alloc, new_size, 64);
	if (ptr) {
		memcpy(ret, ptr, SU_MIN(old_size, new_size));
		scratch_alloc->free(scratch_alloc, ptr);
	}
	return ret;
}
#endif /* SW_WITH_PNG || SW_WITH_JPG || SW_WITH_TGA || SW_WITH_BMP || SW_WITH_PSD || SW_WITH_GIF || SW_WITH_HDR || SW_WITH_PIC || SW_WITH_PNM */

static void sw__update_t(int64_t t) {
	if ((sw__context->out.t == -1) || (t < sw__context->out.t)) {
		sw__context->out.t = t;
	}
}

static pixman_color_t sw__color_argb32_to_pixman_color(sw_color_argb32_t color) {
	pixman_color_t c;
	c.alpha = (uint16_t)(color.c.a * 257);
	c.red = (uint16_t)((uint8_t)((((uint32_t)color.c.r * (uint32_t)color.c.a + 127) * 257) >> 16) * 257);
	c.green = (uint16_t)((uint8_t)((((uint32_t)color.c.g * (uint32_t)color.c.a + 127) * 257) >> 16) * 257);
	c.blue = (uint16_t)((uint8_t)((((uint32_t)color.c.b * (uint32_t)color.c.a + 127) * 257) >> 16) * 257);

	return c;
}

static pixman_image_t *sw__color_to_pixman_image(sw_color_t color, su_allocator_t *scratch_alloc) {
	switch (color.type) {
	case SW_COLOR_TYPE_ARGB32: {
		pixman_color_t c = sw__color_argb32_to_pixman_color(color._.argb32);
		return pixman_image_create_solid_fill(&c);
	}
	case SW_COLOR_TYPE_LINEAR_GRADIENT: {
		pixman_gradient_stop_t *stops = (pixman_gradient_stop_t *)scratch_alloc->alloc(
			scratch_alloc, sizeof(*stops) * color._.linear_gradient.stops.len, SU_ALIGNOF(*stops));
		sw_color_gradient_stop_t prev_stop = su_array__sw_color_gradient_stop_t__get(&color._.linear_gradient.stops, 0);
		pixman_point_fixed_t p1, p2;
		pixman_image_t *image;
		size_t i = 0;

		SU_NOTUSED(prev_stop);
		SU_ASSERT(color._.linear_gradient.stops.len >= 2);

		for ( ; i < color._.linear_gradient.stops.len; ++i) {
			sw_color_gradient_stop_t stop = su_array__sw_color_gradient_stop_t__get(&color._.linear_gradient.stops, i);
			SU_ASSERT(stop.pos >= prev_stop.pos);
			stops[i].color = sw__color_argb32_to_pixman_color(stop.color);
			stops[i].x = pixman_double_to_fixed(stop.pos);
			prev_stop = stop;
		}

		p1.x = pixman_int_to_fixed(color._.linear_gradient.p1.x);
		p1.y = pixman_int_to_fixed(color._.linear_gradient.p1.y);
		p2.x = pixman_int_to_fixed(color._.linear_gradient.p2.x);
		p2.y = pixman_int_to_fixed(color._.linear_gradient.p2.y);
		image = pixman_image_create_linear_gradient(
			&p1, &p2, stops, (int)color._.linear_gradient.stops.len);

		scratch_alloc->free(scratch_alloc, stops);
		return image;
	}
	case SW_COLOR_TYPE_CONICAL_GRADIENT: {
		pixman_gradient_stop_t *stops = (pixman_gradient_stop_t *)scratch_alloc->alloc(
			scratch_alloc, sizeof(*stops) * color._.conical_gradient.stops.len, SU_ALIGNOF(*stops));
		sw_color_gradient_stop_t prev_stop = su_array__sw_color_gradient_stop_t__get(&color._.conical_gradient.stops, 0);
		size_t i = 0;
		pixman_point_fixed_t center;
		pixman_image_t *image;

		SU_NOTUSED(prev_stop);
		SU_ASSERT(color._.conical_gradient.stops.len >= 2);

		for ( ; i < color._.conical_gradient.stops.len; ++i) {
			sw_color_gradient_stop_t stop = su_array__sw_color_gradient_stop_t__get(&color._.conical_gradient.stops, i); 
			SU_ASSERT(stop.pos >= prev_stop.pos);
			stops[i].color = sw__color_argb32_to_pixman_color(stop.color);
			stops[i].x = pixman_double_to_fixed(stop.pos);
			prev_stop = stop;
		}
		center.x = pixman_int_to_fixed(color._.conical_gradient.center.x);
		center.y = pixman_int_to_fixed(color._.conical_gradient.center.y);
		image = pixman_image_create_conical_gradient(
			&center, pixman_double_to_fixed(color._.conical_gradient.angle),
			stops, (int)color._.conical_gradient.stops.len);
		scratch_alloc->free(scratch_alloc, stops);
		return image;
	}
	case SW_COLOR_TYPE_RADIAL_GRADIENT: {
		pixman_gradient_stop_t *stops = (pixman_gradient_stop_t *)scratch_alloc->alloc(
			scratch_alloc, sizeof(*stops) * color._.radial_gradient.stops.len, SU_ALIGNOF(*stops));
		sw_color_gradient_stop_t prev_stop = su_array__sw_color_gradient_stop_t__get(&color._.radial_gradient.stops, 0);
		size_t i = 0;
		pixman_point_fixed_t p_inner, p_outer;
		pixman_image_t *image;

		SU_NOTUSED(prev_stop);
		SU_ASSERT(color._.radial_gradient.stops.len >= 2);

		for ( ; i < color._.radial_gradient.stops.len; ++i) {
			sw_color_gradient_stop_t stop = su_array__sw_color_gradient_stop_t__get(&color._.radial_gradient.stops, i);
			SU_ASSERT(stop.pos >= prev_stop.pos);
			stops[i].color = sw__color_argb32_to_pixman_color(stop.color);
			stops[i].x = pixman_double_to_fixed(stop.pos);
			prev_stop = stop;
		}
		p_inner.x = pixman_int_to_fixed(color._.radial_gradient.inner_p.x);
		p_inner.y = pixman_int_to_fixed(color._.radial_gradient.inner_p.y);
		p_outer.x = pixman_int_to_fixed(color._.radial_gradient.outer_p.x);
		p_outer.y = pixman_int_to_fixed(color._.radial_gradient.outer_p.y);
		image = pixman_image_create_radial_gradient(
			&p_inner, &p_outer,
			pixman_int_to_fixed(color._.radial_gradient.inner_r),
			pixman_int_to_fixed(color._.radial_gradient.outer_r),
			stops, (int)color._.radial_gradient.stops.len);
		scratch_alloc->free(scratch_alloc, stops);
		return image;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}
}

static void sw__image_handle_destroy(pixman_image_t *image, void *data) {
	su_allocator_t *gp_alloc = sw__context->in.gp_alloc;
	sw__image_data_t *image_data = (sw__image_data_t *)data;

#if SW_WITH_SVG || SW_WITH_GIF
	switch (image_data->type) {
	case SW__IMAGE_DATA_TYPE_NONE:
		break;
#if SW_WITH_SVG
	case SW__IMAGE_DATA_TYPE_SVG: {
		resvg_render_tree *render_tree = (resvg_render_tree *)image_data->data;
		resvg_tree_destroy(render_tree);
		break;
	}
#endif /* SW_WITH_SVG */
#if SW_WITH_GIF
	case SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF: {
		sw__image_multiframe_gif_t *gif = (sw__image_multiframe_gif_t *)image_data->data;
		size_t i = 0;
		for ( ; i < gif->frames.len; ++i) {
			pixman_image_t *frame_image = su_array__sw__image_gif_frame_t__get(
				&gif->frames, i).image;
			sw__image_data_t *frame_data = (sw__image_data_t *)pixman_image_get_destroy_data(frame_image);
			frame_data->type = SW__IMAGE_DATA_TYPE_NONE;
			pixman_image_unref(frame_image);
		}
		su_array__sw__image_gif_frame_t__fini(&gif->frames, gp_alloc);
		gp_alloc->free(gp_alloc, gif);
		break;
	}
#endif /* SW_WITH_GIF */
	default:
		SU_ASSERT_UNREACHABLE;
	}
#endif /* SW_WITH_SVG || SW_WITH_GIF */

	SU_NOTUSED(image);

	gp_alloc->free(gp_alloc, image_data->pixels);

	gp_alloc->free(gp_alloc, image_data);
}

static pixman_image_t *sw__image_create( su_allocator_t *gp_alloc,
		int width, int height, sw__image_data_t **data_out) {
	sw__image_data_t *data;
	int stride;
	size_t size;
	pixman_image_t *image;

	SU_ASSERT(width > 0);
	SU_ASSERT(height > 0);

	data = (SU_TYPEOF(data))gp_alloc->alloc(gp_alloc, sizeof(*data), SU_ALIGNOF(*data));
#if SW_WITH_SVG || SW_WITH_GIF
	data->type = SW__IMAGE_DATA_TYPE_NONE;
#endif /* SW_WITH_SVG || SW_WITH_GIF */

	stride = width * 4;
	size = (size_t)height * (size_t)stride;
	data->pixels = (SU_TYPEOF(data->pixels))gp_alloc->alloc(gp_alloc, size, 64);
	memset(data->pixels, 0, size);
	image = pixman_image_create_bits(PIXMAN_a8r8g8b8, width, height, data->pixels, stride);
	pixman_image_set_destroy_function(image, sw__image_handle_destroy, data);

	if (data_out) {
		*data_out = data;
	}
	return image;
}

static pixman_image_t *sw__load_pixmap(su_allocator_t *gp_alloc, su_fat_ptr_t data, sw_status_t *status_out) {
	sw_pixmap_t *pixmap;
	static size_t pixmap_size = (sizeof(pixmap->width) + sizeof(pixmap->height));
	sw__image_data_t *image_data;
	pixman_image_t *image;
	
	if (data.len <= pixmap_size) {
		goto error;
	}

	pixmap = data.ptr;

	if ((pixmap->width * pixmap->height * 4) != (data.len - pixmap_size)) {
		goto error;
	}

	image = sw__image_create(gp_alloc, (int)pixmap->width, (int)pixmap->height, &image_data);

	su_argb_premultiply_alpha( image_data->pixels,
		(uint32_t *)(void *)((uint8_t *)pixmap + pixmap_size),
		pixmap->width * pixmap->height);

	*status_out = SW_STATUS_SUCCESS;
    return image;
error:
	*status_out = SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE;
	return NULL;
}

#if SW_WITH_SVG
static pixman_image_t *sw__render_svg(su_allocator_t *gp_alloc, resvg_render_tree *tree,
		sw__image_data_t **image_data_out, int32_t target_width, int32_t target_height) {
	resvg_size image_size = resvg_get_image_size(tree); /* ? TODO: resvg_get_image_bbox */
	int32_t width = (int32_t)image_size.width;
	int32_t height = (int32_t)image_size.width;
	resvg_transform transform = resvg_transform_identity();
	sw__image_data_t *image_data;
	pixman_image_t *image;
	
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

	image = sw__image_create(gp_alloc, width, height, &image_data);

	resvg_render(tree, transform, (uint32_t)width, (uint32_t)height, (char *)image_data->pixels);

	su_abgr_to_argb(image_data->pixels, image_data->pixels, (size_t)width * (size_t)height);
	pixman_image_set_filter(image, PIXMAN_FILTER_BEST, NULL, 0);

	if (image_data_out) {
		*image_data_out = image_data;
	}
	return image;
}

static pixman_image_t *sw__load_svg(su_allocator_t *gp_alloc, su_fat_ptr_t data, sw_status_t *status_out) {
	pixman_image_t *image = NULL;
	resvg_render_tree *tree = NULL;
	resvg_options *opt = resvg_options_create();
	int32_t ret;

	ret = resvg_parse_tree_from_data(data.ptr, data.len, opt, &tree);
    if (ret == RESVG_OK) {
		sw__image_data_t *image_data;
		image = sw__render_svg(gp_alloc, tree, &image_data, -1, -1);
		image_data->type = SW__IMAGE_DATA_TYPE_SVG;
		image_data->data = tree;
		*status_out = SW_STATUS_SUCCESS;
    } else if (tree) {
		resvg_tree_destroy(tree);
		*status_out = SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE;
	}

	if (opt) {
		resvg_options_destroy(opt);
	}

	return image;
}
#endif /* SW_WITH_SVG */

#if SW_WITH_PNG
static pixman_image_t *sw__load_png(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data, sw_status_t *status_out) {
	pixman_image_t *image = NULL;
	stbi__result_info ri;
	stbi__context ctx;
	memset(&ri, 0, sizeof(ri));
	ri.bits_per_channel = 8;
	stbi__start_mem(&ctx, (stbi_uc *)data.ptr, (int)data.len);

	if (stbi__png_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = (uint32_t *)stbi__png_load(&ctx, &width, &height, &unused, 4, &ri);
		if (src && (width > 0) && (height > 0)) {
			sw__image_data_t *image_data;
			if (ri.bits_per_channel != 8) {
				SU_ASSERT(ri.bits_per_channel == 16);
				src = (uint32_t *)(void *)stbi__convert_16_to_8((stbi__uint16 *)src, width, height, 4);
			}
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	*status_out = (image ? SW_STATUS_SUCCESS : SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE);
	return image;
}
#endif /* SW_WITH_PNG */

#if SW_WITH_JPG
static pixman_image_t *sw__load_jpg(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data, sw_status_t *status_out) {
	pixman_image_t *image = NULL;
	stbi__context ctx;
	stbi__start_mem(&ctx, (stbi_uc *)data.ptr, (int)data.len);

	if (stbi__jpeg_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = (uint32_t *)stbi__jpeg_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			sw__image_data_t *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	*status_out = (image ? SW_STATUS_SUCCESS : SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE);
	return image;
}
#endif /* SW_WITH_JPG */

#if SW_WITH_TGA
static pixman_image_t *sw__load_tga(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data, sw_status_t *status_out) {
	pixman_image_t *image = NULL;
	stbi__context ctx;
	stbi__start_mem(&ctx, (stbi_uc *)data.ptr, (int)data.len);

	if (stbi__tga_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = (uint32_t *)stbi__tga_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			sw__image_data_t *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	*status_out = (image ? SW_STATUS_SUCCESS : SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE);
	return image;
}
#endif /* SW_WITH_TGA */

#if SW_WITH_BMP
static pixman_image_t *sw__load_bmp(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data, sw_status_t *status_out) {
	pixman_image_t *image = NULL;
	stbi__context ctx;
	stbi__start_mem(&ctx, (stbi_uc *)data.ptr, (int)data.len);

	if (stbi__bmp_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = (uint32_t *)stbi__bmp_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			sw__image_data_t *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	*status_out = (image ? SW_STATUS_SUCCESS : SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE);
	return image;
}
#endif /* SW_WITH_BMP */

#if SW_WITH_PSD
static pixman_image_t *sw__load_psd(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data, sw_status_t *status_out) {
	pixman_image_t *image = NULL;
	stbi__result_info ri;
	stbi__context ctx;
	memset(&ri, 0, sizeof(ri));
	ri.bits_per_channel = 8;
	stbi__start_mem(&ctx, (stbi_uc *)data.ptr, (int)data.len);

	if (stbi__psd_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = (uint32_t *)stbi__psd_load(&ctx, &width, &height, &unused, 4, &ri, 8);
		if (src && (width > 0) && (height > 0)) {
			sw__image_data_t *image_data;
			if (ri.bits_per_channel != 8) {
				SU_ASSERT(ri.bits_per_channel == 16);
				src = (uint32_t *)(void *)stbi__convert_16_to_8((stbi__uint16 *)src, width, height, 4);
			}
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	*status_out = (image ? SW_STATUS_SUCCESS : SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE);
	return image;
}
#endif /* SW_WITH_PSD */

#if SW_WITH_GIF
static pixman_image_t *sw__load_gif(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data, sw_status_t *status_out) {
	pixman_image_t *image = NULL;
	int width, height, unused, *frame_delays, frame_count;
	uint32_t *src;
	stbi__context ctx;
	stbi__start_mem(&ctx, (stbi_uc *)data.ptr, (int)data.len);

	if ((src = (uint32_t *)stbi__load_gif_main(&ctx, &frame_delays, &width, &height, &frame_count, &unused, 4))
			&& (width > 0) && (height > 0) && (frame_count > 0)) {
		if (frame_count > 1) {
			int i = 0;
			sw__image_multiframe_gif_t *gif = (sw__image_multiframe_gif_t *)gp_alloc->alloc(
				gp_alloc, sizeof(*gif), SU_ALIGNOF(*gif));
			sw__image_gif_frame_t frame;
			su_array__sw__image_gif_frame_t__init(&gif->frames, gp_alloc, (size_t)frame_count);

			for ( ; i < frame_count; ++i) {
				sw__image_data_t *frame_data;
				size_t size = (size_t)width * (size_t)height * 4;
				frame.image = sw__image_create(gp_alloc, width, height, &frame_data);
				frame.delay = (uint16_t)frame_delays[i];
				su_array__sw__image_gif_frame_t__add_nocheck(&gif->frames, frame);

				su_abgr_to_argb_premultiply_alpha(frame_data->pixels,
					(uint32_t *)(void *)&((uint8_t *)src)[size * (size_t)i],
					(size_t)width * (size_t)height);
				frame_data->type = SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF;
				frame_data->data = gif;
			}

			frame = su_array__sw__image_gif_frame_t__get(&gif->frames, 0);
			gif->frame_idx = 0;
			gif->frame_end = su_now_ms() + frame.delay; /* TODO: set 0 */
			image = frame.image;
		} else {
			sw__image_data_t *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
	}
	scratch_alloc->free(scratch_alloc, src);
	scratch_alloc->free(scratch_alloc, frame_delays);

	*status_out = (image ? SW_STATUS_SUCCESS : SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE);
	return image;
}
#endif /* SW_WITH_GIF */

#if SW_WITH_HDR
static pixman_image_t *sw__load_hdr(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data, sw_status_t *status_out) {
	pixman_image_t *image = NULL;
	stbi__context ctx;
	stbi__start_mem(&ctx, (stbi_uc *)data.ptr, (int)data.len);

	if (stbi__hdr_test(&ctx)) {
		int width = 0, height = 0, unused;
		uint32_t *src = (uint32_t *)(void *)stbi__hdr_to_ldr(
			stbi__hdr_load(&ctx, &width, &height, &unused, 4, NULL),
			width, height, 4
		);
		if (src && (width > 0) && (height > 0)) {
			sw__image_data_t *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	*status_out = (image ? SW_STATUS_SUCCESS : SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE);
	return image;
}
#endif /* SW_WITH_HDR */

#if SW_WITH_PIC
static pixman_image_t *sw__load_pic(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data, sw_status_t *status_out) {
	pixman_image_t *image = NULL;
	stbi__context ctx;
	stbi__start_mem(&ctx, (stbi_uc *)data.ptr, (int)data.len);
			
	if (stbi__pic_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = (uint32_t *)stbi__pic_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			sw__image_data_t *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	*status_out = (image ? SW_STATUS_SUCCESS : SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE);
	return image;
}
#endif /* SW_WITH_PIC */

#if SW_WITH_PNM
static pixman_image_t *sw__load_pnm(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data, sw_status_t *status_out) {
	pixman_image_t *image = NULL;
	stbi__result_info ri;
	stbi__context ctx;
	memset(&ri, 0, sizeof(ri));
	ri.bits_per_channel = 8;
	stbi__start_mem(&ctx, (stbi_uc *)data.ptr, (int)data.len);

	if (stbi__pnm_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = (uint32_t *)stbi__pnm_load(&ctx, &width, &height, &unused, 4, &ri);
		if (src && (width > 0) && (height > 0)) {
			sw__image_data_t *image_data;
			if (ri.bits_per_channel != 8) {
				SU_ASSERT(ri.bits_per_channel == 16);
				src = (uint32_t *)(void *)stbi__convert_16_to_8((stbi__uint16 *)src, width, height, 4);
			}
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
		scratch_alloc->free(scratch_alloc, src);
	}

	*status_out = (image ? SW_STATUS_SUCCESS : SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_IMAGE);
	return image;
}
#endif /* SW_WITH_PNM */

static void sw__layout_block_fini(sw_layout_block_t *block, sw_context_t *sw) {
	sw__layout_block_t *priv = (sw__layout_block_t *)&block->sw__private;

	SU_NOTUSED(sw);

	if (priv->content_image) {
		pixman_image_unref(priv->content_image);
	}
}

static su_bool32_t sw__layout_block_init(sw_layout_block_t *block) {
	/* TODO: remove recursion */

	sw__context_t *sw_private = (sw__context_t *)&sw__context->sw__private;
	sw__layout_block_t *priv = (sw__layout_block_t *)&block->sw__private;
	su_allocator_t *gp_alloc = sw__context->in.gp_alloc;
	su_allocator_t *scratch_alloc = sw__context->in.scratch_alloc;
	sw_status_t status = SW_STATUS_SUCCESS;

	sw__layout_block_fini(block, sw__context);
	memset(priv, 0 , sizeof(*priv));

	block->out.fini = sw__layout_block_fini;

	switch (block->in.type) {
	case SW_LAYOUT_BLOCK_TYPE_SPACER:
		break;
#if SW_WITH_TEXT
	case SW_LAYOUT_BLOCK_TYPE_TEXT: {
		const char **font_names = (const char **)scratch_alloc->alloc(scratch_alloc,
			(block->in._.text.font_names.len + 1) * sizeof(*font_names), SU_ALIGNOF(*font_names));
		struct fcft_font *font;
		struct fcft_text_run *text_run = NULL;
		sw__text_run_cache_t *cache;
		sw__text_run_cache_entry_t entry;
		int image_width = 0, image_height;
		int x = 0, y;
		pixman_image_t *text_color;
		size_t i = 0;
		for ( ; i < block->in._.text.font_names.len; ++i) {
			su_string_t s = su_array__su_string_t__get(&block->in._.text.font_names, i);
			SU_ASSERT(s.nul_terminated == SU_TRUE); /* TODO: handle properly */
			font_names[i] = s.s;
		}
		font_names[block->in._.text.font_names.len] = "monospace:size=16";
		font = fcft_from_name(block->in._.text.font_names.len + 1, font_names, NULL);
		scratch_alloc->free(scratch_alloc, font_names);
		if (font == NULL) {
			status = SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_FONT;
			goto error;
		}

		if (su_hash_table__sw__text_run_cache_t__get(&sw_private->text_run_cache,
				block->in._.text.text, &cache)) {
			size_t j = 0;
			for ( ; j < cache->items.len; ++j) {
				entry = su_array__sw__text_run_cache_entry_t__get(&cache->items, j);
				if (entry.font == font) {
					text_run = entry.text_run;
					break;
				}
			}
		}

		if (text_run == NULL) {
			char32_t *text = (char32_t *)scratch_alloc->alloc(scratch_alloc,
				block->in._.text.text.len * sizeof(*text) + 1, SU_ALIGNOF(*text));
			size_t text_len = 0;
			size_t consumed = 0;
			mbstate_t s;
			memset(&s, 0, sizeof(s));
			while (consumed < block->in._.text.text.len) {
				char32_t c32;
				size_t ret = mbrtoc32(&c32, &block->in._.text.text.s[consumed],
						block->in._.text.text.len - consumed, &s);
				switch (ret) {
				case 0: /* ? TODO: do not treat as error */
				case (size_t)-1:
				case (size_t)-2:
				case (size_t)-3:
					scratch_alloc->free(scratch_alloc, text);
					status = SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_TEXT;
					goto error;
				default:
					text[text_len++] = c32;
					consumed += ret;
				}
			}

			text_run = fcft_rasterize_text_run_utf32(font, text_len, (uint32_t *)text, FCFT_SUBPIXEL_NONE);
			scratch_alloc->free(scratch_alloc, text);
			if ((text_run == NULL) || (text_run->count == 0)) {
				fcft_text_run_destroy(text_run);
				status = SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_TEXT;
				goto error;
			}

			if (su_hash_table__sw__text_run_cache_t__add(&sw_private->text_run_cache,
					gp_alloc, block->in._.text.text, &cache)) {
				su_string_init_string(&cache->key, gp_alloc, cache->key);
			}

			entry.font = font;
			entry.text_run = text_run;
			su_array__sw__text_run_cache_entry_t__add(&cache->items,
				gp_alloc, entry);
		}

		image_height = font->height;
		for ( i = 0; i < text_run->count; ++i) {
			image_width += text_run->glyphs[i]->advance.x;
		}
		if ((image_width <= 0) || (image_height <= 0)) {
			status = SW_STATUS_LAYOUT_BLOCK_ERROR_INVALID_TEXT;
			goto error;
		}

		priv->content_image = sw__image_create(gp_alloc, image_width, image_height, NULL);

		/* TODO: move to sw__layout_block_render */
		text_color = sw__color_to_pixman_image(block->in._.text.color, scratch_alloc);

		y = font->height - font->descent;
		for ( i = 0; i < text_run->count; ++i) {
			const struct fcft_glyph *glyph = text_run->glyphs[i];
			if (pixman_image_get_format(glyph->pix) == PIXMAN_a8r8g8b8) {
				pixman_image_composite32(PIXMAN_OP_OVER, glyph->pix, NULL, priv->content_image,
						0, 0, 0, 0, x + glyph->x, y - glyph->y,
						glyph->width, glyph->height);
			} else {
				pixman_image_composite32(PIXMAN_OP_OVER, text_color, glyph->pix, priv->content_image,
						0, 0, 0, 0, x + glyph->x, y - glyph->y,
						glyph->width, glyph->height);
			}
			x += glyph->advance.x;
		}

		pixman_image_unref(text_color);
		break;
	}
#endif /* SW_WITH_TEXT */
	case SW_LAYOUT_BLOCK_TYPE_IMAGE: {
		su_fat_ptr_t data = block->in._.image.data;
		sw__image_cache_t *cache;

		if (su_hash_table__sw__image_cache_t__get(&sw_private->image_cache,
				data, &cache)) {
#if SW_WITH_GIF
			sw__image_data_t *d = (sw__image_data_t *)pixman_image_get_destroy_data(cache->image);
			if (d->type == SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF) {
				sw__image_multiframe_gif_t *gif = (sw__image_multiframe_gif_t *)d->data;
				priv->content_image = pixman_image_ref(
					su_array__sw__image_gif_frame_t__get(&gif->frames, gif->frame_idx).image);
			} else
#endif /* SW_WITH_GIF */
				priv->content_image = pixman_image_ref(cache->image);
		} else {
			if (block->in._.image.type == SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_AUTO) {
#if SW_WITH_SVG
				if ((priv->content_image = sw__load_svg(gp_alloc, data, &status))) goto process_content_image;
#endif /* SW_WITH_SVG */
#if SW_WITH_PNG
				if ((priv->content_image = sw__load_png(gp_alloc, scratch_alloc, data, &status))) goto process_content_image;
#endif /* SW_WITH_PNG */
#if SW_WITH_JPG
				if ((priv->content_image = sw__load_jpg(gp_alloc, scratch_alloc, data, &status))) goto process_content_image;
#endif /* SW_WITH_JPG */
#if SW_WITH_TGA
				if ((priv->content_image = sw__load_tga(gp_alloc, scratch_alloc, data, &status))) goto process_content_image;
#endif /* SW_WITH_TGA */
#if SW_WITH_BMP
				if ((priv->content_image = sw__load_bmp(gp_alloc, scratch_alloc, data, &status))) goto process_content_image;
#endif /* SW_WITH_BMP */
#if SW_WITH_PSD
				if ((priv->content_image = sw__load_psd(gp_alloc, scratch_alloc, data, &status))) goto process_content_image;
#endif /* SW_WITH_PSD */
#if SW_WITH_GIF
				if ((priv->content_image = sw__load_gif(gp_alloc, scratch_alloc, data, &status))) goto process_content_image;
#endif /* SW_WITH_GIF */
#if SW_WITH_HDR
				if ((priv->content_image = sw__load_hdr(gp_alloc, scratch_alloc, data, &status))) goto process_content_image;
#endif /* SW_WITH_HDR */
#if SW_WITH_PIC
				if ((priv->content_image = sw__load_pic(gp_alloc, scratch_alloc, data, &status))) goto process_content_image;
#endif /* SW_WITH_PIC */
#if SW_WITH_PNM
				if ((priv->content_image = sw__load_pnm(gp_alloc, scratch_alloc, data, &status))) goto process_content_image;
#endif /* SW_WITH_PNM */
				priv->content_image = sw__load_pixmap(gp_alloc, data, &status); goto process_content_image;
			} else {
				switch (block->in._.image.type) {
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SW_PIXMAP: priv->content_image = sw__load_pixmap(gp_alloc, data, &status); break;
#if SW_WITH_SVG
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SVG: priv->content_image = sw__load_svg(gp_alloc, data, &status); break;
#endif /* SW_WITH_SVG */
#if SW_WITH_PNG
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNG: priv->content_image = sw__load_png(gp_alloc, scratch_alloc, data, &status); break;
#endif /* SW_WITH_PNG */
#if SW_WITH_JPG
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_JPG: priv->content_image = sw__load_jpg(gp_alloc, scratch_alloc, data, &status); break;
#endif /* SW_WITH_JPG */
#if SW_WITH_TGA
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_TGA: priv->content_image = sw__load_tga(gp_alloc, scratch_alloc, data, &status); break;
#endif /* SW_WITH_TGA */
#if SW_WITH_BMP
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_BMP: priv->content_image = sw__load_bmp(gp_alloc, scratch_alloc, data, &status); break;
#endif /* SW_WITH_BMP */
#if SW_WITH_PSD
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PSD: priv->content_image = sw__load_psd(gp_alloc, scratch_alloc, data, &status); break;
#endif /* SW_WITH_PSD */
#if SW_WITH_GIF
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_GIF: priv->content_image = sw__load_gif(gp_alloc, scratch_alloc, data, &status); break;
#endif /* SW_WITH_GIF */
#if SW_WITH_HDR
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_HDR: priv->content_image = sw__load_hdr(gp_alloc, scratch_alloc, data, &status); break;
#endif /* SW_WITH_HDR */
#if SW_WITH_PIC
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PIC: priv->content_image = sw__load_pic(gp_alloc, scratch_alloc, data, &status); break;
#endif /* SW_WITH_PIC */
#if SW_WITH_PNM
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNM: priv->content_image = sw__load_pnm(gp_alloc, scratch_alloc, data, &status); break;
#endif /* SW_WITH_PNM */
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_AUTO:
				default:
					SU_ASSERT_UNREACHABLE;
				}
			}

process_content_image:
			if (!priv->content_image) {
				goto error;
			}

			pixman_image_set_filter(priv->content_image, PIXMAN_FILTER_BEST, NULL, 0);

			su_hash_table__sw__image_cache_t__add(&sw_private->image_cache,
				gp_alloc, data, &cache);

			cache->key.ptr = gp_alloc->alloc(gp_alloc, data.len, 64);
			memcpy(cache->key.ptr, data.ptr, data.len);
			cache->key.len = data.len;
			cache->image = pixman_image_ref(priv->content_image);
		}
		break;
	}
	case SW_LAYOUT_BLOCK_TYPE_COMPOSITE: {
		sw_layout_block_t *b = block->in._.composite.children.head;
		SU_ASSERT(block->in._.composite.children.len > 0);
		for ( ; b; b = b->next) {
			sw__layout_block_init(b);
		}
		break;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}

	return SU_TRUE;
error:
	sw__layout_block_fini(block, sw__context);
	memset(priv, 0, sizeof(*priv));
	if (block->in.error) {
		block->in.error(block, sw__context, status);
	}
	return SU_FALSE;
}

static su_bool32_t sw__layout_block_prepare(sw_layout_block_t *, sw_layout_block_dimensions_t *overrides);

static su_bool32_t sw__layout_block_expand(sw_layout_block_t *block, int32_t available_width, int32_t available_height) {
	sw_layout_block_dimensions_t *dim = &block->out.dim;
	int32_t x = dim->x;
	int32_t y = dim->y;
	int32_t width = dim->width;
	int32_t height = dim->height;
	
	if ((block->out.fini == NULL) || (block->in.expand == SW_LAYOUT_BLOCK_EXPAND_NONE)) {
		return SU_TRUE;
	}

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
		sw_layout_block_dimensions_t dims;
		memcpy(dims.borders, dim->borders, sizeof(dim->borders));
		dims.x = x;
		dims.y = y;
		dims.width = width;
		dims.height = height;
		dims.content_width = (width - dim->borders[0] - dim->borders[1]);
		dims.content_height = (height - dim->borders[3] - dim->borders[2]);
		return sw__layout_block_prepare(block, &dims);
	}

	dim->x = x;
	dim->y = y;
	dim->width = width;
	dim->height = height;

	return SU_TRUE;
}

static su_bool32_t sw__layout_block_prepare(sw_layout_block_t *block, sw_layout_block_dimensions_t *overrides) {
	/* TODO: remove recursion */

	sw__layout_block_t *priv = (sw__layout_block_t *)&block->sw__private;
	sw_layout_block_dimensions_t *dim;

	if (block->out.fini == NULL) {
		return SU_TRUE;
	}

	if (block->in.prepare) {
		/* ? TODO: overrides */
		if (!block->in.prepare(block, sw__context)) {
			return SU_FALSE;
		}
	}

	if (block->in.type == SW_LAYOUT_BLOCK_TYPE_COMPOSITE) {
		int32_t content_width, content_height;
		su_bool32_t auto_content_width;
		su_bool32_t auto_content_height;
		su_bool32_t vertical = (block->in._.composite.layout == SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_VERTICAL);
		int32_t l = 0, c = 0, r;
		sw_layout_block_t *b;

		if (overrides) {
			content_width = overrides->content_width;
			content_height = overrides->content_height;
		} else {
			content_width = block->in.content_width;
			content_height = block->in.content_height;
		}

		auto_content_width = (content_width == 0);
		auto_content_height = (content_height == 0);

		for ( b = block->in._.composite.children.head; b; b = b->next) {
			if (b->out.fini == NULL) {
				continue;
			}
			if (!sw__layout_block_prepare(b, NULL)) {
				return SU_FALSE;
			}
			dim = &b->out.dim;
			if (b->in.anchor != SW_LAYOUT_BLOCK_ANCHOR_NONE) {
				if (vertical) {
					if (b->in.anchor == SW_LAYOUT_BLOCK_ANCHOR_CENTER) {
						c += dim->height;
					} else if (auto_content_height) {
						content_height += dim->height;
					}
					if (auto_content_width && (dim->width > content_width)) {
						content_width = dim->width;
					}
				} else {
					if (b->in.anchor == SW_LAYOUT_BLOCK_ANCHOR_CENTER) {
						c += dim->width;
					} else if (auto_content_width) {
						content_width += dim->width;
					}
					if (auto_content_height && (dim->height > content_height)) {
						content_height = dim->height;
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

		for (b = block->in._.composite.children.head; b; b = b->next) {
			dim = &b->out.dim;
			if (b->out.fini == NULL) {
				continue;
			}
			if (b->in.anchor != SW_LAYOUT_BLOCK_ANCHOR_NONE) {
				if (vertical) {
					switch (b->in.anchor) {
					case SW_LAYOUT_BLOCK_ANCHOR_TOP:
					case SW_LAYOUT_BLOCK_ANCHOR_LEFT:
						dim->x = 0;
						dim->y = l;
						l += dim->height;
						break;
					case SW_LAYOUT_BLOCK_ANCHOR_CENTER:
						dim->y = c;
						c += dim->height;
						dim->x = 0;
						break;
					case SW_LAYOUT_BLOCK_ANCHOR_BOTTOM:
					case SW_LAYOUT_BLOCK_ANCHOR_RIGHT:
						dim->x = 0;
						r -= dim->height;
						dim->y = r;
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
						dim->y = 0;
						dim->x = l;
						l += dim->width;
						break;
					case SW_LAYOUT_BLOCK_ANCHOR_CENTER:
						dim->x = c;
						c += dim->width;
						dim->y = 0;
						break;
					case SW_LAYOUT_BLOCK_ANCHOR_BOTTOM:
					case SW_LAYOUT_BLOCK_ANCHOR_RIGHT:
						dim->y = 0;
						r -= dim->width;
						dim->x = r;
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

		if (!priv->content_image || (content_width != pixman_image_get_width(priv->content_image))
				|| (content_height != pixman_image_get_height(priv->content_image))) {
			if (priv->content_image) {
				pixman_image_unref(priv->content_image);
			}
			priv->content_image = ((content_width > 0) && (content_height > 0))
				? sw__image_create(sw__context->in.gp_alloc, content_width, content_height, NULL) : NULL;
		} else {
			memset(pixman_image_get_data(priv->content_image),
				0,
				(size_t)content_width * 4 * (size_t)content_height);
		}
	}

	dim = &block->out.dim;
	if (overrides) {
		*dim = *overrides;
	} else {
		int32_t content_width = (((block->in.content_width <= 0) && priv->content_image)
			? pixman_image_get_width(priv->content_image) : block->in.content_width);
		int32_t content_height = (((block->in.content_height <= 0) && priv->content_image)
			? pixman_image_get_height(priv->content_image) : block->in.content_height);
		int32_t min_width = block->in.min_width;
		int32_t max_width = block->in.max_width;
		int32_t min_height = block->in.min_height;
		int32_t max_height = block->in.max_height;
		int32_t border_left = block->in.borders[0].width;
		int32_t border_right = block->in.borders[1].width;
		int32_t border_bottom = block->in.borders[2].width;
		int32_t border_top = block->in.borders[3].width;
		int32_t width, height;


		if ((priv->content_image) && block->in.content_transform && ((block->in.content_transform % 2) == 0)) {
			int32_t tmp = content_width;
			content_width = content_height;
			content_height = tmp;
		}

		width = content_width + border_left + border_right;
		height = content_height + border_bottom + border_top;

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

		dim->x = block->in.x;
		dim->y = block->in.y;
		dim->width = width;
		dim->height = height;
		dim->content_width = content_width;
		dim->content_height = content_height;
		dim->borders[0] = border_left;
		dim->borders[1] = border_right;
		dim->borders[2] = border_bottom;
		dim->borders[3] = border_top;
	}

	if (block->in.prepared) {
		return block->in.prepared(block, sw__context);
	}

	return SU_TRUE;
}

static void sw__layout_block_render(sw_layout_block_t *block, pixman_image_t *dest) {
	/* TODO: remove recursion */

	sw__layout_block_t *priv = (sw__layout_block_t *)&block->sw__private;
	sw_layout_block_dimensions_t dim = block->out.dim;
	su_allocator_t *scratch_alloc = sw__context->in.scratch_alloc;
	pixman_image_t *color;

	if (block->out.fini == NULL) {
		return;
	}

	if (block->in.type == SW_LAYOUT_BLOCK_TYPE_COMPOSITE) {
		sw_layout_block_t *b = block->in._.composite.children.head;
		for ( ; b; b = b->next) {
			sw__layout_block_render(b, priv->content_image);
		}
	}

	color = sw__color_to_pixman_image(block->in.color, scratch_alloc);
	pixman_image_composite32(PIXMAN_OP_OVER, color, NULL, dest,
		0, 0, 0, 0,
		dim.x + dim.borders[0],
		dim.y + dim.borders[3],
		dim.width - dim.borders[1] - dim.borders[0],
		dim.height - dim.borders[2] - dim.borders[3]);
	pixman_image_unref(color);

	if (dim.borders[0] > 0) {
		pixman_image_t *c = sw__color_to_pixman_image(block->in.borders[0].color, scratch_alloc);
		pixman_image_composite32(PIXMAN_OP_OVER, c, NULL, dest,
			0, 0, 0, 0, dim.x, dim.y, dim.borders[0], dim.height);
		pixman_image_unref(c);
	}

	if (dim.borders[1] > 0) {
		pixman_image_t *c = sw__color_to_pixman_image(block->in.borders[1].color, scratch_alloc);
		pixman_image_composite32(PIXMAN_OP_OVER, c, NULL, dest,
			0, 0, 0, 0, dim.x + dim.width - dim.borders[1], dim.y,
			dim.borders[1], dim.height);
		pixman_image_unref(c);
	}

	if (dim.borders[2] > 0) {
		pixman_image_t *c = sw__color_to_pixman_image(block->in.borders[2].color, scratch_alloc);
		pixman_image_composite32(PIXMAN_OP_OVER, c, NULL, dest,
			0, 0, 0, 0, dim.x + dim.borders[0],
			dim.y + dim.height - dim.borders[2],
			dim.width - dim.borders[0] - dim.borders[1],
			dim.borders[2]);
		pixman_image_unref(c);
	}

	if (dim.borders[3] > 0) {
		pixman_image_t *c = sw__color_to_pixman_image(block->in.borders[3].color, scratch_alloc);
		pixman_image_composite32(PIXMAN_OP_OVER, c, NULL, dest,
			0, 0, 0, 0, dim.x + dim.borders[0], dim.y,
			dim.width - dim.borders[0] - dim.borders[1],
			dim.borders[3]);
		pixman_image_unref(c);
	}

	if (priv->content_image) {
		int content_image_width = pixman_image_get_width(priv->content_image);
		int content_image_height = pixman_image_get_height(priv->content_image);
		pixman_transform_t transform;
		pixman_region32_t clip_region;
		int32_t available_width = dim.width - dim.borders[0] - dim.borders[1];
		int32_t available_height = dim.height - dim.borders[2] - dim.borders[3];
		int32_t content_x = dim.x + dim.borders[0];
		int32_t content_y = dim.y + dim.borders[3];
		sw__image_data_t *image_data = (sw__image_data_t *)pixman_image_get_destroy_data(priv->content_image);
		SU_NOTUSED(image_data);
#if SW_WITH_GIF
		if (image_data->type == SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF) {
			int64_t now_msec = su_now_ms();
			sw__image_multiframe_gif_t *gif = (sw__image_multiframe_gif_t *)image_data->data;
			if (now_msec >= gif->frame_end) {
				sw__image_gif_frame_t frame = su_array__sw__image_gif_frame_t__get(
					&gif->frames, gif->frame_idx);
				if (++gif->frame_idx >= gif->frames.len) {
					gif->frame_idx = 0;
				}
				gif->frame_end = (now_msec + frame.delay);
				pixman_image_unref(priv->content_image);
				priv->content_image = pixman_image_ref(frame.image);
			}
			sw__update_t(gif->frame_end);
		}
#endif /* SW_WITH_GIF */

		if (block->in.content_transform && ((block->in.content_transform % 2) == 0)) {
			int tmp = content_image_width;
			content_image_width = content_image_height;
			content_image_height = tmp;
		}

		pixman_transform_init_identity(&transform);

		if ((dim.content_width != content_image_width)
				|| (dim.content_height != content_image_height)) {
#if SW_WITH_SVG
			if (image_data->type == SW__IMAGE_DATA_TYPE_SVG) {
				resvg_render_tree *render_tree = (resvg_render_tree *)image_data->data;
				pixman_image_unref(priv->content_image);
				priv->content_image = sw__render_svg(sw__context->in.gp_alloc,
					render_tree, NULL, dim.content_width, dim.content_height);
			} else
#endif /* SW_WITH_SVG */
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
				pixman_int_to_fixed(pixman_image_get_width(priv->content_image)), 0);
			break;
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_180:
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_180:
			pixman_transform_rotate(&transform, NULL, pixman_fixed_minus_1, 0);
			pixman_transform_translate(&transform, NULL,
				pixman_int_to_fixed(pixman_image_get_width(priv->content_image)),
				pixman_int_to_fixed(pixman_image_get_height(priv->content_image)));
			break;
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_270:
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_270:
			pixman_transform_rotate(&transform, NULL, 0, pixman_fixed_minus_1);
			pixman_transform_translate(&transform, NULL, 0,
				pixman_int_to_fixed(pixman_image_get_height(priv->content_image)));
			break;
		default:
			SU_ASSERT_UNREACHABLE;
		}

		if (block->in.content_transform >= SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED) {
			pixman_transform_translate(&transform, NULL,
				-pixman_int_to_fixed(pixman_image_get_width(priv->content_image)), 0);
			pixman_transform_scale(&transform, NULL, pixman_fixed_minus_1, pixman_fixed_1);
		}

		pixman_image_set_transform(priv->content_image, &transform);

		pixman_region32_init_rect(&clip_region, dim.x, dim.y, (unsigned int)dim.width, (unsigned int)dim.height);
		pixman_image_set_clip_region32(dest, &clip_region);

		/* ? TODO: move to sw__layout_block_prepare */
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

		pixman_image_composite32(PIXMAN_OP_OVER, priv->content_image, NULL, dest,
			0, 0, 0, 0, content_x, content_y, dim.content_width, dim.content_height);

		pixman_image_set_transform(priv->content_image, NULL);
		pixman_image_set_clip_region32(dest, NULL);
	}
}

#if SW_WITH_WAYLAND
static void sw__wayland_surface_buffer_fini(sw__wayland_surface_buffer_t *buffer) {
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

static void sw__wayland_surface_fini(sw_wayland_surface_t *surface, sw_context_t *sw) {
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw_wayland_seat_t *seat;

	SU_NOTUSED(sw);

	switch (surface->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_LAYER: {
		sw__wayland_surface_layer_t *layer = &priv->_.layer;
		if (layer->layer_surface) {
			zwlr_layer_surface_v1_destroy(layer->layer_surface);
		}
		break;
	}
	case SW_WAYLAND_SURFACE_TYPE_POPUP: {
		sw__wayland_surface_popup_t *popup = &priv->_.popup;
		if (popup->xdg_positioner) {
			xdg_positioner_destroy(popup->xdg_positioner);
		}
		if (popup->xdg_popup) {
			xdg_popup_destroy(popup->xdg_popup);
		}
		if (popup->xdg_surface) {
			xdg_surface_destroy(popup->xdg_surface);
		}
		break;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}
	sw__wayland_surface_buffer_fini(&priv->buffer);
	if (priv->wl_surface) {
		wl_surface_destroy(priv->wl_surface);
	}

	su_array__sw_wayland_region_t__fini(&priv->input_regions, sw__context->in.gp_alloc);

	for ( seat = sw__context->out.backend.wayland.seats.head; seat; seat = seat->next) {
		if (seat->out.pointer && (seat->out.pointer->out.focused_surface == surface)) {
			seat->out.pointer->out.focused_surface = NULL;
		}
	}
}

static void sw__wayland_surface_set_error(sw_wayland_surface_t *surface, sw_status_t status) {
	sw__wayland_surface_fini(surface, sw__context);
	memset(&surface->sw__private, 0, sizeof(surface->sw__private));
	if (surface->in.error) {
		surface->in.error(surface, sw__context, status);
	}
}

static void sw__wayland_surface_popup_init_stage2(sw_wayland_surface_t *);

static void sw__wayland_surface_render(sw_wayland_surface_t *surface) {
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&surface->sw__private;
	int32_t surface_width, surface_height;

	if (surface->out.fini == NULL) {
		return;
	}

	if (priv->buffer.wl_buffer && priv->buffer.busy) {
		priv->dirty = SU_TRUE;
		return;
	}

	if (!sw__layout_block_prepare(surface->in.root, NULL) ||
			((surface->in.root->out.dim.width <= 0) &&
				(surface->in.root->out.dim.height <= 0))) {
		sw__wayland_surface_set_error(surface, SW_STATUS_SURFACE_ERROR_LAYOUT_FAILED);
		return;
	}

	switch (surface->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_LAYER: {
		sw__wayland_surface_layer_t *layer = &priv->_.layer;
		int32_t exclusive_zone = surface->in._.layer.exclusive_zone;
		/* ? TODO: allow surface sizing with SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL (the same can be achieved with 0 anchor) */
		if (layer->anchor != SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL) {
			if (surface->in.width < 0) {
				surface_width = surface->in.root->out.dim.width;
			} else {
				surface_width = surface->in.width;
			}
			if (surface->in.height < 0) {
				surface_height = surface->in.root->out.dim.height;
			} else {
				surface_height = surface->in.height;
			}
			if (!priv->buffer.wl_buffer ||
						((surface_width != 0) && (surface->out.width != surface_width)) ||
						((surface_height != 0) && (surface->out.height != surface_height))) {
				zwlr_layer_surface_v1_set_size(layer->layer_surface,
					(uint32_t)(surface_width / surface->out.scale),
					(uint32_t)(surface_height / surface->out.scale));
			}
		} else {
			surface_width = surface->out.width;
			surface_height = surface->out.height;
		}
		if (exclusive_zone < -1) {
			switch (layer->anchor) {
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
		if (layer->exclusive_zone != exclusive_zone) {
			zwlr_layer_surface_v1_set_exclusive_zone(layer->layer_surface,
				exclusive_zone / surface->out.scale);
			layer->exclusive_zone = exclusive_zone;
		}
		break;
	}
	case SW_WAYLAND_SURFACE_TYPE_POPUP: {
		sw__wayland_surface_popup_t *popup = &priv->_.popup;
		surface_width = (surface->in.width <= 0) ? surface->in.root->out.dim.width : surface->in.width;
		surface_height = (surface->in.height <= 0) ? surface->in.root->out.dim.height : surface->in.height;
		if (!priv->buffer.wl_buffer ||
				(surface->out.width != surface_width) || (surface->out.height != surface_height)) {
			xdg_positioner_set_size(popup->xdg_positioner,
				surface_width / surface->out.scale, surface_height / surface->out.scale);
			if (!popup->xdg_popup) {
				sw__wayland_surface_popup_init_stage2(surface);
				return;
			}
			xdg_popup_reposition(popup->xdg_popup, popup->xdg_positioner, 0);
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

	if (priv->buffer.wl_buffer) {
		memset(priv->buffer.pixels, 0, priv->buffer.size);

		sw__layout_block_render(surface->in.root, priv->buffer.image);

		wl_surface_set_buffer_scale(priv->wl_surface, surface->out.scale);
		wl_surface_attach(priv->wl_surface, priv->buffer.wl_buffer, 0, 0);
		wl_surface_damage_buffer(priv->wl_surface, 0, 0, surface->out.width, surface->out.height);

		priv->buffer.busy = SU_TRUE;
		priv->dirty = SU_FALSE;
	}

	wl_surface_commit(priv->wl_surface);
}

static void sw__wayland_surface_buffer_handle_release(void *data, struct wl_buffer *wl_buffer) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&surface->sw__private;

	SU_NOTUSED(wl_buffer);

	priv->buffer.busy = SU_FALSE;
	if (priv->dirty) {
		sw__wayland_surface_render(surface);
	}
}

static su_bool32_t sw__wayland_surface_buffer_init(sw__wayland_surface_buffer_t *buffer,
		sw_wayland_surface_t *surface, int32_t width, int32_t height, sw_status_t *status) {
	sw__context_t *sw_private = (sw__context_t *)&sw__context->sw__private;
	struct timespec ts;
	pid_t pid = getpid();
	char shm_name[NAME_MAX];
	int shm_fd;
	int32_t stride = width * 4;
	struct wl_shm_pool *wl_shm_pool;

	static struct wl_buffer_listener surface_buffer_listener = { sw__wayland_surface_buffer_handle_release };

	memset(buffer, 0, sizeof(*buffer));

/* TODO: limit retry count */
generate_shm_name:
	clock_gettime(CLOCK_MONOTONIC, &ts); /* ? TODO: assert success */
	su_snprintf(shm_name, sizeof(shm_name),"/sw-%d-%ld-%ld", pid, ts.tv_sec, ts.tv_nsec);

	if ((shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0600)) == -1) {
		if (errno == EEXIST) {
			goto generate_shm_name;
		} else {
			goto error;
		}
	}
	shm_unlink(shm_name);

	buffer->size = (uint32_t)stride * (uint32_t)height;
	while (ftruncate(shm_fd, buffer->size) == -1) {
		if (errno == EINTR) {
			continue;
		} else {
			goto error;
		}
	}

	buffer->pixels = (SU_TYPEOF(buffer->pixels))mmap(
		NULL, buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (buffer->pixels == MAP_FAILED) {
		goto error;
	}

	buffer->image = pixman_image_create_bits_no_clear(PIXMAN_a8r8g8b8, width, height,
			buffer->pixels, stride);

	wl_shm_pool = wl_shm_create_pool(sw_private->wayland.shm, shm_fd, (int32_t)buffer->size);
	buffer->wl_buffer = wl_shm_pool_create_buffer(wl_shm_pool, 0, width,
			height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_buffer_add_listener(buffer->wl_buffer, &surface_buffer_listener, surface);
	wl_shm_pool_destroy(wl_shm_pool);
	close(shm_fd);

	buffer->busy = SU_FALSE;

	*status = SW_STATUS_SUCCESS;
	return SU_TRUE;
error:
	sw__wayland_surface_buffer_fini(buffer);
	*status = SW_STATUS_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER;
	return SU_FALSE;
}

static void sw__wayland_surface_destroy(sw_wayland_surface_t *surface) {
	/* TODO: remove recursion */
	sw_wayland_surface_t *popup = surface->in.popups.head;
	for ( ; popup; ) {
		sw_wayland_surface_t *next = popup->next;
		sw__wayland_surface_destroy(popup);
		popup = next;
	}
	surface->in.destroy(surface, sw__context);
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
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_layer_t *layer = &priv->_.layer;

	SU_NOTUSED(wl_surface);

	if (surface->out.scale != factor) {
		surface->out.scale = factor;
		priv->dirty = SU_TRUE;
		if (layer->anchor != SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL) {
			zwlr_layer_surface_v1_set_size(layer->layer_surface,
				(surface->in.width == 0) ? 0 : (uint32_t)(surface->out.width / factor),
				(surface->in.height == 0) ? 0 : (uint32_t)(surface->out.height / factor));
		}
		if (layer->exclusive_zone > 0) {
			zwlr_layer_surface_v1_set_exclusive_zone(layer->layer_surface,
				layer->exclusive_zone / factor);
		}
		zwlr_layer_surface_v1_set_margin(layer->layer_surface,
			layer->margins[0] / factor,
			layer->margins[1] / factor,
			layer->margins[2] / factor,
			layer->margins[3] / factor);
		wl_surface_commit(priv->wl_surface);
	}
}

static void sw__wayland_surface_layer_handle_layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *layer_surface,
		uint32_t serial, uint32_t width_, uint32_t height_) {
	sw_wayland_surface_t *layer = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&layer->sw__private;
	int32_t width = (int32_t)width_ * layer->out.scale;
	int32_t height = (int32_t)height_ * layer->out.scale;
	sw_status_t status = SW_STATUS_SUCCESS;

	SU_NOTUSED(layer_surface);

	zwlr_layer_surface_v1_ack_configure(priv->_.layer.layer_surface, serial);

	if (((layer->out.height != height) || (layer->out.width != width))
			&& (width != 0) && (height != 0)) {
		sw__wayland_surface_buffer_fini(&priv->buffer);
		if (sw__wayland_surface_buffer_init(&priv->buffer, layer, width, height, &status)) {
			layer->out.width = width;
			layer->out.height = height;
			priv->dirty = SU_TRUE;
		} else {
			sw__wayland_surface_set_error(layer, status);
			return;
		}
	}

	if (priv->dirty) {
		sw__wayland_surface_render(layer);
	}
}

static void sw__wayland_surface_layer_handle_layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *layer_surface) {
	sw_wayland_surface_t *layer = (sw_wayland_surface_t *)data;
	SU_NOTUSED(layer_surface);
	sw__wayland_surface_destroy(layer);
}

static void sw__wayland_surface_popup_handle_preferred_buffer_scale(void *data,
		struct wl_surface *wl_surface, int32_t factor) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_popup_t *popup = &priv->_.popup;

	SU_NOTUSED(wl_surface);

	if (surface->out.scale != factor) {
		surface->out.scale = factor;
		priv->dirty = SU_TRUE;
		xdg_positioner_set_size( popup->xdg_positioner,
			surface->out.width / factor, surface->out.height / factor);
		xdg_positioner_set_anchor_rect(popup->xdg_positioner,
			popup->x / factor, popup->y / factor, 1, 1);
		xdg_popup_reposition(popup->xdg_popup, popup->xdg_positioner, 0);
		wl_surface_commit(priv->wl_surface);
	}
}

static void sw__wayland_surface_popup_xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface,
		uint32_t serial) {
	sw_wayland_surface_t *popup = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&popup->sw__private;

	SU_NOTUSED(xdg_surface);

	xdg_surface_ack_configure(priv->_.popup.xdg_surface, serial);

	if (priv->dirty) {
		sw__wayland_surface_render(popup);
	}
}

static void sw__wayland_surface_layer_init(sw_wayland_surface_t *surface, sw_wayland_surface_layer_layer_t l) {
	sw__context_t *sw_private = (sw__context_t *)&sw__context->sw__private;
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_output_t *output_private = (sw__wayland_output_t *)&surface->in._.layer.output->sw__private;
	sw__wayland_surface_layer_t *layer = &priv->_.layer;

	static struct wl_surface_listener wl_surface_listener = {
		sw__wayland_surface_handle_enter,
		sw__wayland_surface_handle_leave,
		sw__wayland_surface_layer_handle_preferred_buffer_scale,
		sw__wayland_surface_handle_preferred_buffer_transform
	};
	static struct zwlr_layer_surface_v1_listener layer_surface_listener = {
		sw__wayland_surface_layer_handle_layer_surface_configure,
		sw__wayland_surface_layer_handle_layer_surface_closed
	};

	surface->out.fini = sw__wayland_surface_fini;
	surface->out.scale = surface->in._.layer.output->out.scale;
	surface->out.width = 0;
	surface->out.height = 0;
	priv->output = surface->in._.layer.output;
	layer->exclusive_zone = INT32_MIN;
	layer->anchor = UINT32_MAX;
	layer->layer = l;
	layer->margins[0] = INT32_MIN;
	layer->margins[1] = INT32_MIN;
	layer->margins[2] = INT32_MIN;
	layer->margins[3] = INT32_MIN;

	priv->wl_surface = wl_compositor_create_surface(sw_private->wayland.compositor);
	wl_surface_add_listener(priv->wl_surface, &wl_surface_listener, surface);
	layer->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
		sw_private->wayland.layer_shell, priv->wl_surface,
		output_private->wl_output, l, "sw");
	zwlr_layer_surface_v1_add_listener( layer->layer_surface, &layer_surface_listener, surface);
}

static void sw__wayland_surface_popup_init_stage1(sw_wayland_surface_t *surface, sw_wayland_surface_t *parent) {
	sw__context_t *sw_private = (sw__context_t *)&sw__context->sw__private;
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_popup_t *popup = &priv->_.popup;
	sw__wayland_surface_t *parent_private = (sw__wayland_surface_t *)&parent->sw__private;
	static struct wl_surface_listener wl_surface_listener = {
		sw__wayland_surface_handle_enter,
		sw__wayland_surface_handle_leave,
		sw__wayland_surface_popup_handle_preferred_buffer_scale,
		sw__wayland_surface_handle_preferred_buffer_transform,
	};
	static struct xdg_surface_listener xdg_surface_listener = { sw__wayland_surface_popup_xdg_surface_handle_configure };

	popup->parent = parent;
	priv->output = parent_private->output;
	surface->out.fini = sw__wayland_surface_fini;
	surface->out.scale = parent_private->output->out.scale;
	surface->out.width = 0;
	surface->out.height = 0;
	popup->x = INT32_MIN;
	popup->y = INT32_MIN;
	popup->gravity = (sw_wayland_surface_popup_gravity_t)UINT32_MAX;
	popup->constraint_adjustment = UINT32_MAX;

	priv->wl_surface = wl_compositor_create_surface(sw_private->wayland.compositor);
	wl_surface_add_listener(priv->wl_surface, &wl_surface_listener, surface);

	popup->xdg_surface = xdg_wm_base_get_xdg_surface(
		sw_private->wayland.wm_base, priv->wl_surface);
	xdg_surface_add_listener(popup->xdg_surface, &xdg_surface_listener, surface);

	popup->xdg_positioner = xdg_wm_base_create_positioner(sw_private->wayland.wm_base);
}

static void sw__wayland_surface_popup_handle_configure(void *data, struct xdg_popup *xdg_popup,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	sw_wayland_surface_t *popup = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&popup->sw__private;
	sw_status_t status = SW_STATUS_SUCCESS;

	SU_NOTUSED(xdg_popup); SU_NOTUSED(x); SU_NOTUSED(y);

	width *= popup->out.scale;
	height *= popup->out.scale;
	if ((popup->out.width != width) || (popup->out.height != height)) {
		sw__wayland_surface_buffer_fini(&priv->buffer);
		if (sw__wayland_surface_buffer_init(&priv->buffer, popup, width, height, &status)) {
			popup->out.width = width;
			popup->out.height = height;
			priv->dirty = SU_TRUE;
		} else {
			sw__wayland_surface_set_error(popup, status);
		}
	}
}

static void sw__wayland_surface_popup_handle_done(void *data, struct xdg_popup *xdg_popup) {
	sw_wayland_surface_t *popup = (sw_wayland_surface_t *)data;
	SU_NOTUSED(xdg_popup);
	sw__wayland_surface_destroy(popup);
}

static void sw__wayland_surface_popup_handle_repositioned(void *data, struct xdg_popup *xdg_popup,
		uint32_t token) {
	SU_NOTUSED(data); SU_NOTUSED(xdg_popup); SU_NOTUSED(token);
}

static void sw__wayland_surface_prepare(sw_wayland_surface_t *surface, sw_wayland_surface_t *parent) {
	/* TODO: remove recursion */
	
	sw__context_t *sw_private = (sw__context_t *)&sw__context->sw__private;
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw_wayland_cursor_shape_t cursor_shape = (surface->in.cursor_shape == SW_WAYLAND_CURSOR_SHAPE_DEFAULT)
		? SW_WAYLAND_CURSOR_SHAPE_DEFAULT_ : surface->in.cursor_shape;

	SU_ASSERT(surface->in.destroy != NULL);
	SU_ASSERT(surface->in.root != NULL);

	if (!sw__layout_block_init(surface->in.root)) {
		sw__wayland_surface_set_error(surface, SW_STATUS_SURFACE_ERROR_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK);
		return;
	}

	switch (surface->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_LAYER: {
		sw__wayland_surface_layer_t *layer = &priv->_.layer;
		static uint32_t horiz = (ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
		static uint32_t vert = (ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
		sw_wayland_surface_t *popup;

		SU_NOTUSED(horiz); SU_NOTUSED(vert);
		SU_ASSERT(surface->in._.layer.output != NULL);
		SU_ASSERT((surface->in._.layer.anchor == SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL) ||
			(((surface->in.width != 0) || ((surface->in._.layer.anchor & horiz) == horiz)) &&
			((surface->in.height != 0) || ((surface->in._.layer.anchor & vert) == vert)))
		);

		if (priv->output != surface->in._.layer.output) {
			sw__wayland_surface_fini(surface, sw__context);
			memset(priv, 0, sizeof(*priv));
		}

		if (!priv->wl_surface) {
			sw__wayland_surface_layer_init(surface, surface->in._.layer.layer);
		}

		/* ? TODO: exclusive_edge */

		if (layer->anchor != surface->in._.layer.anchor) {
			zwlr_layer_surface_v1_set_anchor(layer->layer_surface, surface->in._.layer.anchor);
			layer->anchor = surface->in._.layer.anchor;
		}

		if (layer->layer != surface->in._.layer.layer) {
			zwlr_layer_surface_v1_set_layer(layer->layer_surface, surface->in._.layer.layer);
			layer->layer = surface->in._.layer.layer;
		}

		if (memcmp(layer->margins, surface->in._.layer.margins,
				sizeof(surface->in._.layer.margins)) != 0) {
			zwlr_layer_surface_v1_set_margin(layer->layer_surface,
				surface->in._.layer.margins[0] / surface->out.scale,
				surface->in._.layer.margins[1] / surface->out.scale,
				surface->in._.layer.margins[2] / surface->out.scale,
				surface->in._.layer.margins[3] / surface->out.scale);
			memcpy(layer->margins, surface->in._.layer.margins,
				sizeof(surface->in._.layer.margins));
		}

		for ( popup = surface->in.popups.head; popup; popup = popup->next) {
			SU_ASSERT(popup->in.type == SW_WAYLAND_SURFACE_TYPE_POPUP);
			sw__wayland_surface_prepare(popup, surface);
		}
		break;
	}
	case SW_WAYLAND_SURFACE_TYPE_POPUP: {
		sw__wayland_surface_popup_t *popup = &priv->_.popup;
		su_bool32_t reposition = SU_FALSE;

		if (!priv->wl_surface) {
			sw__wayland_surface_popup_init_stage1(surface, parent);
		}

		if ((popup->x != surface->in._.popup.x) ||
				(popup->y != surface->in._.popup.y)) {
			xdg_positioner_set_anchor_rect(popup->xdg_positioner,
				surface->in._.popup.x / surface->out.scale, surface->in._.popup.y / surface->out.scale, 1, 1);
			popup->x = surface->in._.popup.x;
			popup->y = surface->in._.popup.y;
			reposition = SU_TRUE;
		}

		if (popup->gravity != surface->in._.popup.gravity) {
			xdg_positioner_set_gravity(popup->xdg_positioner, surface->in._.popup.gravity);
			popup->gravity = surface->in._.popup.gravity;
			reposition = SU_TRUE;
		}

		if (popup->constraint_adjustment != surface->in._.popup.constraint_adjustment) {
			xdg_positioner_set_constraint_adjustment(
				popup->xdg_positioner, surface->in._.popup.constraint_adjustment);
			popup->constraint_adjustment = surface->in._.popup.constraint_adjustment;
			reposition = SU_TRUE;
		}

		if (popup->xdg_popup) {
			sw_wayland_surface_t *p;
			if (reposition) {
				xdg_popup_reposition(popup->xdg_popup, popup->xdg_positioner, 0);
			}
			for ( p = surface->in.popups.head; p; p = p->next) {
				SU_ASSERT(p->in.type == SW_WAYLAND_SURFACE_TYPE_POPUP);
				sw__wayland_surface_prepare(p, surface);
			}
		}
		break;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}

	if (priv->cursor_shape != cursor_shape) {
		sw_wayland_seat_t *seat = sw__context->out.backend.wayland.seats.head;
		for ( ; seat; seat = seat->next) {
			sw__wayland_pointer_t *pointer_private = (sw__wayland_pointer_t *)&seat->out.pointer->sw__private;
			if (seat->out.pointer && pointer_private->cursor_shape_device
					&& (seat->out.pointer->out.focused_surface == surface)) {
				wp_cursor_shape_device_v1_set_shape(pointer_private->cursor_shape_device,
					pointer_private->enter_serial, cursor_shape);
			}
		}
		priv->cursor_shape = cursor_shape;
	}

	if ((priv->input_regions.len != surface->in.input_regions.len) ||
			(memcmp(priv->input_regions.items, surface->in.input_regions.items,
				sizeof(*surface->in.input_regions.items) * surface->in.input_regions.len) != 0)) {
		size_t i;
		struct wl_region *input_region = NULL;
		if (surface->in.input_regions.len > 0) {
			input_region = wl_compositor_create_region(sw_private->wayland.compositor);
			for ( i = 0; i < surface->in.input_regions.len; ++i) {
				sw_wayland_region_t region = su_array__sw_wayland_region_t__get(&surface->in.input_regions, i);
				wl_region_add(input_region, region.x, region.y, region.width, region.height);
			}
		}
		wl_surface_set_input_region(priv->wl_surface, input_region);
		if (input_region) {
			wl_region_destroy(input_region);
		}
		
		priv->input_regions.len = 0;
		for ( i = 0; i < surface->in.input_regions.len; ++i) {
			su_array__sw_wayland_region_t__add(&priv->input_regions, sw__context->in.gp_alloc,
				su_array__sw_wayland_region_t__get(&surface->in.input_regions, i));
		}
	}

	sw__wayland_surface_render(surface);
}

static void sw__wayland_surface_popup_init_stage2(sw_wayland_surface_t *surface) {
	sw__wayland_surface_t *priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_popup_t *popup = &priv->_.popup;
	sw__wayland_surface_t *parent_private = (sw__wayland_surface_t *)&popup->parent->sw__private;
	sw_wayland_surface_t *p;

	static struct xdg_popup_listener xdg_popup_listener = {
		sw__wayland_surface_popup_handle_configure,
		sw__wayland_surface_popup_handle_done,
		sw__wayland_surface_popup_handle_repositioned,
	};

	switch (popup->parent->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_LAYER:
		popup->xdg_popup = xdg_surface_get_popup(
			popup->xdg_surface, NULL, popup->xdg_positioner);
		zwlr_layer_surface_v1_get_popup(parent_private->_.layer.layer_surface, popup->xdg_popup);
		popup->grab = surface->in._.popup.grab;
		break;
	case SW_WAYLAND_SURFACE_TYPE_POPUP:
		popup->xdg_popup = xdg_surface_get_popup( popup->xdg_surface,
			parent_private->_.popup.xdg_surface, popup->xdg_positioner);
		popup->grab = parent_private->_.popup.grab;
		break;
	default:
		SU_ASSERT_UNREACHABLE;
	}

	xdg_popup_add_listener(popup->xdg_popup, &xdg_popup_listener, surface);

	/* TODO: handle grab with invalid serial, touch serial */
	if (popup->grab) {
		sw__wayland_seat_t *seat_private = (sw__wayland_seat_t *)&popup->grab->out.seat->sw__private;
		sw__wayland_pointer_t *pointer_private = (sw__wayland_pointer_t *)&popup->grab->sw__private;
		xdg_popup_grab( popup->xdg_popup,
			seat_private->wl_seat, pointer_private->button_serial);
	}

	wl_surface_commit(priv->wl_surface);

	for ( p = surface->in.popups.head; p; p = p->next) {
		SU_ASSERT(p->in.type == SW_WAYLAND_SURFACE_TYPE_POPUP);
		sw__wayland_surface_prepare(p, surface);
	}
}

static void sw__wayland_output_handle_geometry(void *data, struct wl_output *wl_output,
		int32_t x, int32_t y, int32_t physical_width, int32_t physical_height,
		int32_t subpixel, const char *make, const char *model, int32_t transform) {
	sw_wayland_output_t *output = (sw_wayland_output_t *)data;

	SU_NOTUSED(wl_output); SU_NOTUSED(x); SU_NOTUSED(y); SU_NOTUSED(physical_width); SU_NOTUSED(physical_height);
	SU_NOTUSED(subpixel); SU_NOTUSED(make); SU_NOTUSED(model);
	
	output->out.transform = (sw_wayland_output_transform_t)transform;
}

static void sw__wayland_output_handle_mode(void *data, struct wl_output *wl_output,
		uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
	sw_wayland_output_t *output = (sw_wayland_output_t *)data;

	SU_NOTUSED(wl_output); SU_NOTUSED(flags); SU_NOTUSED(refresh);

	output->out.width = width;
	output->out.height = height;
}

static void sw__wayland_output_handle_done(void *data, struct wl_output *wl_output) {
	SU_NOTUSED(wl_output); SU_NOTUSED(data);
}

static void sw__wayland_output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	sw_wayland_output_t *output = (sw_wayland_output_t *)data;
	SU_NOTUSED(wl_output);
	output->out.scale = factor;
}

static void sw__wayland_output_handle_name(void *data, struct wl_output *wl_output, const char *name) {
	sw_wayland_output_t *output = (sw_wayland_output_t *)data;
	size_t len = strlen(name);

	SU_NOTUSED(wl_output);

	if (len > 0) {
		su_string_init_len(&output->out.name, sw__context->in.gp_alloc, name, len, SU_TRUE);
	}
}

static void sw__wayland_output_handle_description(void *data, struct wl_output *wl_output, const char *description) {
	SU_NOTUSED(data); SU_NOTUSED(wl_output); SU_NOTUSED(description);
}

static sw_wayland_output_t *sw__wayland_output_create(uint32_t wl_name) {
	sw__context_t *sw_private = (sw__context_t *)&sw__context->sw__private;
	sw_wayland_output_t *output = NULL;
	if (sw__context->in.backend.wayland.output_create && (output = sw__context->in.backend.wayland.output_create(sw__context))) {
		static struct wl_output_listener output_listener = {
			sw__wayland_output_handle_geometry,
			sw__wayland_output_handle_mode,
			sw__wayland_output_handle_done,
			sw__wayland_output_handle_scale,
			sw__wayland_output_handle_name,
			sw__wayland_output_handle_description,
		};

		sw__wayland_output_t *priv = (sw__wayland_output_t *)&output->sw__private;

		output->out.scale = 1;
		priv->wl_output = (SU_TYPEOF(priv->wl_output))wl_registry_bind(sw_private->wayland.registry,
			wl_name, &wl_output_interface, 4);
		priv->wl_name = wl_name;
		wl_output_add_listener(priv->wl_output, &output_listener, output);
	}

	return output;
}

static void sw__wayland_output_destroy(sw_wayland_output_t *output) {
	sw__wayland_output_t *priv = (sw__wayland_output_t *)&output->sw__private;
	sw_wayland_surface_t *layer = sw__context->in.backend.wayland.layers.head;
	for ( ; layer; ) {
		sw_wayland_surface_t *next = layer->next;
		if (layer->in._.layer.output == output) {
			sw__wayland_surface_destroy(layer);
		}
		layer = next;
	}
	if (priv->wl_output) {
		wl_output_destroy(priv->wl_output);
	}
	su_string_fini(&output->out.name, sw__context->in.gp_alloc);
	if (output->in.destroy) {
		output->in.destroy(output, sw__context);
	}
}

static void sw__wayland_pointer_handle_enter(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *wl_surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;
	sw__wayland_pointer_t *priv = (sw__wayland_pointer_t *)&pointer->sw__private;
	sw_wayland_surface_t *surface;

	SU_NOTUSED(wl_pointer);

	if (!wl_surface) {
		return;
	}

	surface = (sw_wayland_surface_t *)wl_surface_get_user_data(wl_surface);

	if (priv->cursor_shape_device) {
		sw__wayland_surface_t *surface_private = (sw__wayland_surface_t *)&surface->sw__private;
		wp_cursor_shape_device_v1_set_shape(priv->cursor_shape_device,
				serial, surface_private->cursor_shape);
	}

	pointer->out.focused_surface = surface;
	priv->enter_serial = serial;

	pointer->out.pos_x = (int32_t)(wl_fixed_to_double(surface_x) * (double)surface->out.scale);
	pointer->out.pos_y = (int32_t)(wl_fixed_to_double(surface_y) * (double)surface->out.scale);

	if (surface->in.enter) {
		surface->in.enter(pointer, sw__context);
	}
}

static void sw__wayland_pointer_handle_leave(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;

	SU_NOTUSED(wl_pointer); SU_NOTUSED(serial); SU_NOTUSED(surface);

	if (pointer->out.focused_surface) {
		if (pointer->out.focused_surface->in.leave) {
			pointer->out.focused_surface->in.leave(pointer, sw__context);
		}
		pointer->out.focused_surface = NULL;
	}
}

static void sw__wayland_pointer_handle_motion(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;
	int32_t x, y;

	SU_NOTUSED(wl_pointer); SU_NOTUSED(time);

	if (!pointer->out.focused_surface) {
		return;
	}

	x = (int32_t)(wl_fixed_to_double(surface_x) * (double)pointer->out.focused_surface->out.scale);
	y = (int32_t)(wl_fixed_to_double(surface_y) * (double)pointer->out.focused_surface->out.scale);
	if ((x != pointer->out.pos_x) || (y != pointer->out.pos_y)) {
		pointer->out.pos_x = x;
		pointer->out.pos_y = y;
		if (pointer->out.focused_surface->in.motion) {
			pointer->out.focused_surface->in.motion(pointer, sw__context);
		}
	}
}

static void sw__wayland_pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t st) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;
	sw__wayland_pointer_t *priv = (sw__wayland_pointer_t *)&pointer->sw__private;

	SU_NOTUSED(wl_pointer); SU_NOTUSED(time);

	if (!pointer->out.focused_surface) {
		return;
	}

	pointer->out.btn_code = button;
	pointer->out.btn_state = (SU_TYPEOF(pointer->out.btn_state))st;

	priv->button_serial = serial;

	if (pointer->out.focused_surface->in.button) {
		pointer->out.focused_surface->in.button(pointer, sw__context);
	}
}

static void sw__wayland_pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;

	SU_NOTUSED(wl_pointer); SU_NOTUSED(data); SU_NOTUSED(time);

	if (!pointer->out.focused_surface) {
		return;
	}

	pointer->out.scroll_axis = (SU_TYPEOF(pointer->out.scroll_axis))axis;
	pointer->out.scroll_vector_length = wl_fixed_to_double(value);

	if (pointer->out.focused_surface->in.scroll) {
		pointer->out.focused_surface->in.scroll(pointer, sw__context);
	}
}

static sw_wayland_pointer_t *sw__wayland_pointer_create(sw_wayland_seat_t *seat) {
	sw__context_t *sw_private = (sw__context_t *)&sw__context->sw__private;
	sw_wayland_pointer_t *pointer = NULL;
	if (seat->in.pointer_create && (pointer = seat->in.pointer_create(sw__context))) {
		sw__wayland_pointer_t *priv = (sw__wayland_pointer_t *)&pointer->sw__private;
		sw__wayland_seat_t *seat_private = (sw__wayland_seat_t *)&seat->sw__private;

		static struct wl_pointer_listener pointer_listener = {
			sw__wayland_pointer_handle_enter,
			sw__wayland_pointer_handle_leave,
			sw__wayland_pointer_handle_motion,
			sw__wayland_pointer_handle_button,
			sw__wayland_pointer_handle_axis,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
		};

		pointer->out.seat = seat;
		priv->wl_pointer = wl_seat_get_pointer(seat_private->wl_seat);
		if (sw_private->wayland.cursor_shape_manager) {
			priv->cursor_shape_device = wp_cursor_shape_manager_v1_get_pointer(
				sw_private->wayland.cursor_shape_manager, priv->wl_pointer);
		} else {
			priv->cursor_shape_device = NULL;
		}
		wl_pointer_add_listener(priv->wl_pointer, &pointer_listener, pointer);
	}

	return pointer;
}

static void sw__wayland_pointer_destroy(sw_wayland_pointer_t *pointer) {
	sw__wayland_pointer_t *priv = (sw__wayland_pointer_t *)&pointer->sw__private;
	if (priv->cursor_shape_device) {
		wp_cursor_shape_device_v1_destroy(priv->cursor_shape_device);
	}
	if (priv->wl_pointer) {
		wl_pointer_destroy(priv->wl_pointer);
	}
	if (pointer->in.destroy) {
		pointer->in.destroy(pointer, sw__context);
	}
}

static void sw__wayland_seat_handle_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
	sw_wayland_seat_t *seat = (sw_wayland_seat_t *)data;

	su_bool32_t have_pointer = (capabilities & WL_SEAT_CAPABILITY_POINTER);
	/* TODO: touch, keyboard */

	SU_NOTUSED(wl_seat);

	if (have_pointer && !seat->out.pointer) {
		seat->out.pointer = sw__wayland_pointer_create(seat);
	} else if (!have_pointer && seat->out.pointer) {
		sw__wayland_pointer_destroy(seat->out.pointer);
		seat->out.pointer = NULL;
	}
}

static void sw__wayland_seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name) {
	sw_wayland_seat_t *seat = (sw_wayland_seat_t *)data;
	size_t len = strlen(name);

	SU_NOTUSED(wl_seat);

	if (len > 0) {
		su_string_init_len(&seat->out.name, sw__context->in.gp_alloc, name, len, SU_TRUE);
	}
}

static sw_wayland_seat_t *sw__wayland_seat_create(uint32_t wl_name) {
	sw__context_t *sw_private = (sw__context_t *)&sw__context->sw__private;
	sw_wayland_seat_t *seat = NULL;
	if (sw__context->in.backend.wayland.seat_create && (seat = sw__context->in.backend.wayland.seat_create(sw__context))) {
		sw__wayland_seat_t *priv = (sw__wayland_seat_t *)&seat->sw__private;

		static struct wl_seat_listener seat_listener = {
			sw__wayland_seat_handle_capabilities,
			sw__wayland_seat_handle_name,
		};

		priv->wl_seat = (SU_TYPEOF(priv->wl_seat))wl_registry_bind(
			sw_private->wayland.registry, wl_name, &wl_seat_interface, 2);
		priv->wl_name = wl_name;
		wl_seat_add_listener(priv->wl_seat, &seat_listener, seat);
	}

	return seat;
}

static void sw__wayland_seat_destroy(sw_wayland_seat_t *seat) {
	sw__wayland_seat_t *priv = (sw__wayland_seat_t *)&seat->sw__private;
	if (seat->out.pointer) {
		sw__wayland_pointer_destroy(seat->out.pointer);
	}
	if (priv->wl_seat) {
		wl_seat_destroy(priv->wl_seat);
	}
	su_string_fini(&seat->out.name, sw__context->in.gp_alloc);
	if (seat->in.destroy) {
		seat->in.destroy(seat, sw__context);
	}
}

static void sw__wayland_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
	sw__context_t *sw_private = (sw__context_t *)&sw__context->sw__private;
	SU_NOTUSED(data); SU_NOTUSED(xdg_wm_base);
	xdg_wm_base_pong(sw_private->wayland.wm_base, serial);
}

static void sw__wayland_registry_handle_global(void *data, struct wl_registry *wl_registry,
		uint32_t wl_name, const char *interface, uint32_t version) {
	sw__context_t *sw_private = (sw__context_t *)&sw__context->sw__private;
	sw__context_wayland_t *wayland = &sw_private->wayland;

	SU_NOTUSED(data); SU_NOTUSED(wl_registry); SU_NOTUSED(version);
	
	if (strcmp(interface, wl_output_interface.name) == 0) {
		sw_wayland_output_t *output = sw__wayland_output_create(wl_name);
		if (output) {
			su_llist__sw_wayland_output_t__insert_tail(
				&sw__context->out.backend.wayland.outputs, output);
		}
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		sw_wayland_seat_t *seat = sw__wayland_seat_create(wl_name);
		if (seat) {
			su_llist__sw_wayland_seat_t__insert_tail(&sw__context->out.backend.wayland.seats, seat);
		}
    } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
		wayland->compositor = (SU_TYPEOF(wayland->compositor))wl_registry_bind(
			wayland->registry, wl_name, &wl_compositor_interface, 6);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		wayland->shm = (SU_TYPEOF(wayland->shm))wl_registry_bind(
			wayland->registry, wl_name, &wl_shm_interface, 1);
		/* ? TODO: wl_shm_add_listener (check for ARGB32) */
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		wayland->layer_shell = (SU_TYPEOF(wayland->layer_shell))wl_registry_bind(
			wayland->registry, wl_name, &zwlr_layer_shell_v1_interface, 2);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		static struct xdg_wm_base_listener wm_base_listener = { sw__wayland_wm_base_handle_ping };
		wayland->wm_base = (SU_TYPEOF(wayland->wm_base))wl_registry_bind(
			wayland->registry,wl_name, &xdg_wm_base_interface, 3);
		xdg_wm_base_add_listener(wayland->wm_base, &wm_base_listener, NULL);
	} else if (strcmp(interface, wp_cursor_shape_manager_v1_interface.name) == 0) {
		wayland->cursor_shape_manager = (SU_TYPEOF(wayland->cursor_shape_manager))wl_registry_bind(
			wayland->registry, wl_name, &wp_cursor_shape_manager_v1_interface, 1);
	}
}

static void sw__wayland_registry_handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
	sw_backend_wayland_out_t *wayland = &sw__context->out.backend.wayland;
	sw_wayland_output_t *output;
	sw_wayland_seat_t *seat;

	SU_NOTUSED(data); SU_NOTUSED(wl_registry);

	for ( output = wayland->outputs.head; output; output = output->next) {
		sw__wayland_output_t *priv = (sw__wayland_output_t *)&output->sw__private;
		if (priv->wl_name == name) {
			su_llist__sw_wayland_output_t__pop(&wayland->outputs, output);
			sw__wayland_output_destroy(output);
			return;
		}
	}

	for ( seat = wayland->seats.head; seat; seat = seat->next) {
		sw__wayland_seat_t *priv = (sw__wayland_seat_t *)&seat->sw__private;
		if (priv->wl_name == name) {
			su_llist__sw_wayland_seat_t__pop(&wayland->seats, seat);
			sw__wayland_seat_destroy(seat);
			return;
		}
	}
}

#endif /* SW_WITH_WAYLAND */

SW_EXPORT su_bool32_t sw_init(sw_context_t *sw) {
	sw__context_t *priv;

	SU_ASSERT(su_locale_is_utf8());
	SU_ASSERT(sw->in.gp_alloc != NULL);
	SU_ASSERT(sw->in.scratch_alloc != NULL);

	sw__context = sw;
	priv = (sw__context_t *)&sw->sw__private;

	memset(&sw->out, 0, sizeof(sw->out));
	memset(priv, 0, sizeof(*priv));
	sw->out.t = -1;

	switch (sw->in.backend_type) {
#if SW_WITH_WAYLAND
	case SW_BACKEND_TYPE_WAYLAND: {
		/* ? TODO: add all data from wl_* to sw_wayland_*.out */

		sw__context_wayland_t *wayland = &priv->wayland;

		static struct wl_registry_listener registry_listener = {
			sw__wayland_registry_handle_global,
			sw__wayland_registry_handle_global_remove,
		};

		wayland->display = wl_display_connect(NULL);
		if (!wayland->display) {
			goto error;
		}

		wayland->registry = wl_display_get_registry(wayland->display);
		wl_registry_add_listener(wayland->registry, &registry_listener, NULL);
		if (wl_display_roundtrip(wayland->display) == -1) {
			goto error;
		}

		if (!wayland->layer_shell) {
			errno = EPROTONOSUPPORT;
			goto error;
		}
		if (!wayland->wm_base) {
			errno = EPROTONOSUPPORT;
			goto error;
		}
		if (!wayland->cursor_shape_manager) {
			/* TODO: warning */
		}

		if (wl_display_roundtrip(wayland->display) == -1) {
			goto error;
		}

		sw->out.backend.wayland.pfd.fd = wl_display_get_fd(wayland->display);
		sw->out.backend.wayland.pfd.events = POLLIN;
		break;
	}
#endif /* SW_WITH_WAYLAND */
	default:
		SU_ASSERT_UNREACHABLE;
	}

	/* ? TODO: resvg_init_log(); */

	su_hash_table__sw__image_cache_t__init(&priv->image_cache, sw->in.gp_alloc, 512);

#if SW_WITH_TEXT
	if (!fcft_init(FCFT_LOG_COLORIZE_NEVER, SU_FALSE, FCFT_LOG_CLASS_ERROR)) {
		goto error;
	}

	su_hash_table__sw__text_run_cache_t__init(&priv->text_run_cache, sw->in.gp_alloc, 1024);
#endif /* SW_WITH_TEXT */

	return sw_process(sw);
error:
	return SU_FALSE;
}

SW_EXPORT void sw_fini(sw_context_t *sw) {
	sw__context_t *priv = (sw__context_t *)&sw__context->sw__private;
	su_allocator_t *gp_alloc = sw->in.gp_alloc;
	size_t i;

	/*SU_ASSERT(su_locale_is_utf8());*/
	SU_ASSERT(sw->in.gp_alloc != NULL);
	SU_ASSERT(sw->in.scratch_alloc != NULL);

	sw__context = sw;

	switch (sw->in.backend_type) {
#if SW_WITH_WAYLAND
	case SW_BACKEND_TYPE_WAYLAND: {
		sw__context_wayland_t *wayland = &priv->wayland;
		sw_wayland_output_t *output;
		sw_wayland_seat_t *seat;

		for ( output = sw->out.backend.wayland.outputs.head; output; ) {
			sw_wayland_output_t *next = output->next;
			sw__wayland_output_destroy(output);
			output = next;
		}

		for ( seat = sw->out.backend.wayland.seats.head; seat; ) {
			sw_wayland_seat_t *next = seat->next;
			sw__wayland_seat_destroy(seat);
			seat = next;
		}

		if (wayland->cursor_shape_manager) {
			wp_cursor_shape_manager_v1_destroy(wayland->cursor_shape_manager);
		}
		if (wayland->layer_shell) {
			zwlr_layer_shell_v1_destroy(wayland->layer_shell);
		}
		if (wayland->wm_base) {
			xdg_wm_base_destroy(wayland->wm_base);
		}
		if (wayland->shm) {
			wl_shm_destroy(wayland->shm);
		}
		if (wayland->compositor) {
			wl_compositor_destroy(wayland->compositor);
		}
		if (wayland->registry) {
			wl_registry_destroy(wayland->registry);
		}
		if (wayland->display) {
			wl_display_flush(wayland->display);
			wl_display_disconnect(wayland->display);
		}
		break;
	}
#endif /* SW_WITH_WAYLAND */
	default:
		SU_ASSERT_UNREACHABLE;
	}

	for ( i = 0; i < priv->image_cache.items.len; ++i) {
		sw__image_cache_t cache = su_array__sw__image_cache_t__get(&priv->image_cache.items, i);
		gp_alloc->free(gp_alloc, cache.key.ptr);
		if (cache.image) {
			pixman_image_unref(cache.image);
		}
	}
	su_hash_table__sw__image_cache_t__fini(&priv->image_cache, gp_alloc);

#if SW_WITH_TEXT
	for ( i = 0; i < priv->text_run_cache.items.len; ++i) {
		size_t j = 0;
		sw__text_run_cache_t cache = su_array__sw__text_run_cache_t__get(
			&priv->text_run_cache.items, i);
		su_string_fini(&cache.key, gp_alloc);
		for ( ; j < cache.items.len; ++j) {
			sw__text_run_cache_entry_t entry = su_array__sw__text_run_cache_entry_t__get(&cache.items, j);
			fcft_text_run_destroy(entry.text_run);
			fcft_destroy(entry.font);
		}
		su_array__sw__text_run_cache_entry_t__fini(&cache.items, gp_alloc);
	}
	su_hash_table__sw__text_run_cache_t__fini(&priv->text_run_cache, gp_alloc);

	fcft_fini();
#endif /* SW_WITH_TEXT */

	memset(&sw->out, 0, sizeof(sw->out));
	memset(priv, 0, sizeof(*priv));
}

SW_EXPORT su_bool32_t sw_flush(sw_context_t *sw) {
	sw__context_t *priv = (sw__context_t *)&sw__context->sw__private;

	/*SU_ASSERT(su_locale_is_utf8()); */
	/*SU_ASSERT(sw->in.gp_alloc != NULL); */
	/*SU_ASSERT(sw->in.scratch_alloc != NULL); */

	sw__context = sw;

	switch (sw->in.backend_type) {
#if SW_WITH_WAYLAND
	case SW_BACKEND_TYPE_WAYLAND:
		sw->out.backend.wayland.pfd.events = POLLIN;
		if (wl_display_flush(priv->wayland.display) == -1) {
			if (errno == EAGAIN) {
				sw->out.backend.wayland.pfd.events = (POLLIN | POLLOUT);
			} else {
				return SU_FALSE;
			}
		}
		break;
#endif /* SW_WITH_WAYLAND */
	default:
		SU_ASSERT_UNREACHABLE;
	}

	return SU_TRUE;
}

SW_EXPORT su_bool32_t sw_process(sw_context_t *sw) {
	sw__context_t *priv = (sw__context_t *)&sw__context->sw__private;

	SU_ASSERT(su_locale_is_utf8());
	SU_ASSERT(sw->in.gp_alloc != NULL);
	SU_ASSERT(sw->in.scratch_alloc != NULL);

	sw__context = sw;

	switch (sw->in.backend_type) {
#if SW_WITH_WAYLAND
	case SW_BACKEND_TYPE_WAYLAND:
		if (wl_display_prepare_read(priv->wayland.display) != -1) {
			if (wl_display_read_events(priv->wayland.display) == -1) {
				return SU_FALSE;
			}
		}
		wl_display_dispatch_pending(priv->wayland.display);
		break;
#endif /* SW_WITH_WAYLAND */
	default:
		SU_ASSERT_UNREACHABLE;
	}

	return SU_TRUE;
}

SW_EXPORT void sw_set(sw_context_t *sw) {
	SU_ASSERT(su_locale_is_utf8());
	SU_ASSERT(sw->in.gp_alloc != NULL);
	SU_ASSERT(sw->in.scratch_alloc != NULL);

	sw__context = sw;

	if (su_now_ms() >= sw->out.t) {
		sw->out.t = -1;
	}

	switch (sw->in.backend_type) {
#if SW_WITH_WAYLAND
	case SW_BACKEND_TYPE_WAYLAND: {
		sw_wayland_surface_t *surface = sw->in.backend.wayland.layers.head;
		for ( ; surface; surface = surface->next) {
			SU_ASSERT(surface->in.type == SW_WAYLAND_SURFACE_TYPE_LAYER);
			sw__wayland_surface_prepare(surface, NULL);
		}
		break;
	}
#endif /* SW_WITH_WAYLAND */
	default:
		SU_ASSERT_UNREACHABLE;
	}
}

#endif /* defined(SW_IMPLEMENTATION) && !defined(SW__REIMPLEMENTATION_GUARD) */
