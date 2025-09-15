#if !defined(SW_HEADER)
#define SW_HEADER

/*#define SW_IMPLEMENTATION*/

#if !defined(SW_WITH_DEBUG)
#define SW_WITH_DEBUG 1
#endif /* !defined(SW_WITH_DEBUG) */
#if !defined(SW_WITH_MEMORY_BACKEND)
#define SW_WITH_MEMORY_BACKEND 1
#endif /* !defined(SW_WITH_MEMORY_BACKEND) */
#if !defined(SW_WITH_WAYLAND_BACKEND)
#define SW_WITH_WAYLAND_BACKEND 1
#endif /* !defined(SW_WITH_WAYLAND_BACKEND) */
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

#if !defined(SW_FUNC_DEF)
#define SW_FUNC_DEF static
#endif

#if !defined(SU_WITH_DEBUG)
#define SU_WITH_DEBUG SW_WITH_DEBUG
#endif /* !defined(SU_WITH_DEBUG) */
#include <sutil.h>

SU_STATIC_ASSERT(SW_WITH_WAYLAND_BACKEND || SW_WITH_MEMORY_BACKEND);

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

typedef struct sw_context sw_context_t;

SW_FUNC_DEF su_bool32_t sw_set(sw_context_t *);
SW_FUNC_DEF void sw_cleanup(sw_context_t *);

SW_FUNC_DEF su_bool32_t sw_flush(sw_context_t *);
SW_FUNC_DEF su_bool32_t sw_process(sw_context_t *);

/* TODO: strings */
typedef enum sw_layout_block_event {
	SW_LAYOUT_BLOCK_EVENT_DESTROY,
	SW_LAYOUT_BLOCK_EVENT_PREPARE,
	SW_LAYOUT_BLOCK_EVENT_PREPARED,
	SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_IMAGE
#if SW_WITH_TEXT
	,SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_FONT
	,SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_TEXT
#endif /* SW_WITH_TEXT */
} sw_layout_block_event_t;

typedef struct sw_layout_block sw_layout_block_t;

#define SW__PRIVATE_FIELDS(size) size_t sw__private[size / sizeof(size_t)]

#if SW_WITH_WAYLAND_BACKEND

#include <poll.h>

/* TODO: strings */
/* ? TODO: surface prepare, prepared */
typedef enum sw_wayland_event {
	SW_WAYLAND_EVENT_SURFACE_CLOSE,
	SW_WAYLAND_EVENT_SURFACE_FAILED_TO_SET_CURSOR_SHAPE,
	SW_WAYLAND_EVENT_SURFACE_FAILED_TO_SET_DECORATIONS,
	SW_WAYLAND_EVENT_SURFACE_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK,
	SW_WAYLAND_EVENT_SURFACE_LAYOUT_FAILED,
	SW_WAYLAND_EVENT_SURFACE_ERROR_MISSING_PROTOCOL,
	SW_WAYLAND_EVENT_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER,
	SW_WAYLAND_EVENT_POINTER_ENTER,
	SW_WAYLAND_EVENT_POINTER_LEAVE,
	SW_WAYLAND_EVENT_POINTER_MOTION,
	SW_WAYLAND_EVENT_POINTER_BUTTON,
	SW_WAYLAND_EVENT_POINTER_SCROLL
} sw_wayland_event_t;

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

typedef enum sw_wayland_output_subpixel {
	SW_WAYLAND_OUTPUT_SUBPIXEL_UNKNOWN = 0,
	SW_WAYLAND_OUTPUT_SUBPIXEL_NONE = 1,
	SW_WAYLAND_OUTPUT_SUBPIXEL_HORIZONTAL_RGB = 2,
	SW_WAYLAND_OUTPUT_SUBPIXEL_HORIZONTAL_BGR = 3,
	SW_WAYLAND_OUTPUT_SUBPIXEL_VERTICAL_RGB = 4,
	SW_WAYLAND_OUTPUT_SUBPIXEL_VERTICAL_BGR = 5
} sw_wayland_output_subpixel_t;

typedef struct sw_wayland_output sw_wayland_output_t;
typedef sw_wayland_output_t *(*sw_wayland_output_create_func_t)(sw_wayland_output_t *, sw_context_t *);
typedef void (*sw_wayland_output_destroy_func_t)(sw_wayland_output_t *, sw_context_t *);

typedef struct sw_wayland_output_in {
	sw_wayland_output_destroy_func_t destroy; /* may be NULL */
} sw_wayland_output_in_t;

typedef struct sw_wayland_output_out {
	su_string_t name, description;
	su_string_t make, model;
	int32_t width, height, scale;
	sw_wayland_output_subpixel_t subpixel;
	int32_t x, y, refresh, physical_width, physical_height;
	sw_wayland_output_transform_t transform;
} sw_wayland_output_out_t;

struct sw_wayland_output {
    sw_wayland_output_in_t in;
	sw_wayland_output_out_t out;
	SW__PRIVATE_FIELDS(16);
	SU_LLIST_NODE_FIELDS(sw_wayland_output_t);
};

typedef struct sw_wayland_seat sw_wayland_seat_t;

typedef struct sw_wayland_pointer sw_wayland_pointer_t;
typedef sw_wayland_pointer_t *(*sw_wayland_pointer_create_func_t)(sw_wayland_seat_t *, sw_context_t *);
typedef void (*sw_wayland_pointer_func_t)(sw_wayland_pointer_t *, sw_context_t *);

typedef struct sw_wayland_pointer_in {
	sw_wayland_pointer_func_t destroy; /* may be NULL */
} sw_wayland_pointer_in_t;

typedef struct sw_wayland_surface sw_wayland_surface_t;

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

typedef sw_wayland_seat_t *(*sw_wayland_seat_create_func_t)(sw_wayland_seat_t *, sw_context_t *);
typedef void (*sw_wayland_seat_destroy_func_t)(sw_wayland_seat_t *, sw_context_t *);

typedef struct sw_wayland_seat_in {
	sw_wayland_seat_destroy_func_t destroy; /* may be NULL */
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
	SU_LLIST_NODE_FIELDS(sw_wayland_seat_t);
};

typedef enum sw_wayland_surface_layer_anchor {
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_NONE = 0,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_TOP = 1,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_BOTTOM = 2,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT = 4,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT = 8,
	SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL =
		(SW_WAYLAND_SURFACE_LAYER_ANCHOR_TOP | SW_WAYLAND_SURFACE_LAYER_ANCHOR_BOTTOM |
		SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT | SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT)
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
	SW_WAYLAND_SURFACE_TYPE_TOPLEVEL,
	SW_WAYLAND_SURFACE_TYPE_LAYER,
	SW_WAYLAND_SURFACE_TYPE_POPUP
} sw_wayland_surface_type_t;

typedef union sw_wayland_notify {
	sw_wayland_surface_t *surface;
	sw_wayland_pointer_t *pointer;
} sw_wayland_event_source_t;

typedef su_bool32_t (*sw_wayland_notify_func_t)(sw_wayland_event_source_t *, sw_context_t *, sw_wayland_event_t );
typedef void (*sw_wayland_surface_fini_func_t)(sw_wayland_surface_t *, sw_context_t *);

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

typedef enum sw_wayland_toplevel_decoration_mode {
	SW_WAYLAND_TOPLEVEL_DECORATION_MODE_COMPOSITOR_DEFAULT = 0,
	SW_WAYLAND_TOPLEVEL_DECORATION_MODE_CLIENT_SIDE = 1,
	SW_WAYLAND_TOPLEVEL_DECORATION_MODE_SERVER_SIDE = 2
} sw_wayland_toplevel_decoration_mode_t;

typedef struct sw_wayland_surface_toplevel {
	su_string_t title, app_id;
	su_bool32_t maximized, fullscreen;
	sw_wayland_output_t *fullscreen_output; /* may be NULL */
	int32_t min_width, min_height;
	int32_t max_width, max_height;
	su_bool32_t minimized;
	sw_wayland_toplevel_decoration_mode_t decoration_mode;
} sw_wayland_surface_toplevel_t;

typedef union sw_wayland_surface_in_ {
	sw_wayland_surface_toplevel_t toplevel;
	sw_wayland_surface_layer_t layer;
	sw_wayland_surface_popup_t popup;
} sw_wayland_surface_in__t;

typedef struct sw_wayland_surfaces {
	SU_LLIST_FIELDS(sw_wayland_surface_t);
} sw_wayland_surfaces_t;

typedef struct sw_wayland_surface_in {
	sw_wayland_surface_in__t _;
	sw_wayland_surface_type_t type;
    sw_wayland_cursor_shape_t cursor_shape;
	int32_t width, height;
	sw_wayland_notify_func_t notify; /* may be NULL */
    sw_wayland_region_t *input_regions;
	size_t input_regions_count;
    sw_wayland_surfaces_t popups;
    sw_layout_block_t *root;
} sw_wayland_surface_in_t;

typedef enum sw_wayland_surface_toplevel_state {
	SW_WAYLAND_SURFACE_TOPLEVEL_STATE_MAXIMIZED = (1 << 1),
	SW_WAYLAND_SURFACE_TOPLEVEL_STATE_FULLSCREEN = (1 << 2),
	SW_WAYLAND_SURFACE_TOPLEVEL_STATE_RESIZING = (1 << 3),
	SW_WAYLAND_SURFACE_TOPLEVEL_STATE_ACTIVATED = (1 << 4),
	SW_WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_LEFT = (1 << 5),
	SW_WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_RIGHT = (1 << 6),
	SW_WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_TOP = (1 << 7),
	SW_WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_BOTTOM = (1 << 8)
} sw_wayland_surface_toplevel_state_t;

typedef struct sw_wayland_surface_out_toplevel {
	uint32_t states; /* sw_wayland_surface_toplevel_state_t | */
	SU_PAD32;
} sw_wayland_surface_out_toplevel_t;

typedef union sw_wayland_surface_out_ {
	sw_wayland_surface_out_toplevel_t toplevel;
} sw_wayland_surface_out__t;

typedef struct sw_wayland_surface_out {
	sw_wayland_surface_out__t _;
	int32_t width, height, scale;
	sw_wayland_output_transform_t transform;
	sw_wayland_surface_fini_func_t fini; /* must be called at destruction */
} sw_wayland_surface_out_t;

struct sw_wayland_surface {
    sw_wayland_surface_in_t in;
	sw_wayland_surface_out_t out;
	SW__PRIVATE_FIELDS(168);
	SU_LLIST_NODE_FIELDS(sw_wayland_surface_t);
};

#endif /* SW_WITH_WAYLAND_BACKEND */

typedef struct sw_point {
	/* ? TODO: % */
	int32_t x, y;
} sw_point_t;

typedef union sw_color_argb32 {
	uint32_t u32;
	struct {
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

typedef struct sw_color_linear_gradient {
	sw_point_t p1, p2;
	sw_color_gradient_stop_t *stops;
	size_t stops_count;
} sw_color_linear_gradient_t;

typedef struct sw_color_conical_gradient {
	sw_point_t center;
	double angle;
	sw_color_gradient_stop_t *stops;
	size_t stops_count;
} sw_color_conical_gradient_t;

typedef struct sw_color_radial_gradient {
	sw_point_t inner_p, outer_p;
	int32_t inner_r, outer_r;
	sw_color_gradient_stop_t *stops;
	size_t stops_count;
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

typedef enum sw_layout_block_fill {
	SW_LAYOUT_BLOCK_FILL_NONE = 0,
	SW_LAYOUT_BLOCK_FILL_LEFT = (1 << 1),
	SW_LAYOUT_BLOCK_FILL_RIGHT = (1 << 2),
	SW_LAYOUT_BLOCK_FILL_BOTTOM = (1 << 3),
	SW_LAYOUT_BLOCK_FILL_TOP = (1 << 4),
	SW_LAYOUT_BLOCK_FILL_CONTENT = (1 << 5),

	SW_LAYOUT_BLOCK_FILL_ALL_SIDES =
		(SW_LAYOUT_BLOCK_FILL_LEFT | SW_LAYOUT_BLOCK_FILL_RIGHT |
		SW_LAYOUT_BLOCK_FILL_BOTTOM | SW_LAYOUT_BLOCK_FILL_TOP),

	SW_LAYOUT_BLOCK_FILL_ALL_SIDES_CONTENT =
		(SW_LAYOUT_BLOCK_FILL_LEFT | SW_LAYOUT_BLOCK_FILL_RIGHT |
		SW_LAYOUT_BLOCK_FILL_BOTTOM | SW_LAYOUT_BLOCK_FILL_TOP |
		SW_LAYOUT_BLOCK_FILL_CONTENT)

} sw_layout_block_fill_t;

typedef struct sw_layout_block_border {
	int32_t width;
	SU_PAD32;
	sw_color_t color;
} sw_layout_block_border_t;

typedef enum sw_layout_block_composite_children_layout {
	SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_HORIZONTAL,
	SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_VERTICAL
} sw_layout_block_composite_children_layout_t;

typedef enum sw_layout_block_content_repeat {
    SW_LAYOUT_BLOCK_CONTENT_REPEAT_NONE,
    SW_LAYOUT_BLOCK_CONTENT_REPEAT_NORMAL,
    SW_LAYOUT_BLOCK_CONTENT_REPEAT_PAD,
    SW_LAYOUT_BLOCK_CONTENT_REPEAT_REFLECT
} sw_layout_block_content_repeat_t;

typedef struct sw_layout_block_dimensions {
	int32_t x, y, width, height;
	int32_t content_width, content_height;
	int32_t borders[4]; /* left right bottom top */ /* ? TODO: union/#define sides */
} sw_layout_block_dimensions_t;

#if SW_WITH_TEXT
typedef struct sw_layout_block_text {
	su_string_t text;
	su_string_t *font_names; /* fallback order */
	size_t font_names_count;
	sw_color_t color;
} sw_layout_block_text_t;
#endif /* SW_WITH_TEXT */

typedef struct sw_pixmap {
	uint32_t width;
	uint32_t height;
	sw_color_argb32_t pixels[1]; /* width * height * 4, premultiplied alpha */
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
	su_fat_ptr_t data;
	sw_layout_block_image_image_type_t type;
#if SW_WITH_GIF
	su_bool32_t gif_static;
	size_t gif_frame_idx;
#else
	SU_PAD32;
#endif
} sw_layout_block_image_t;

typedef struct sw_layout_blocks {
	SU_LLIST_FIELDS(sw_layout_block_t);
} sw_layout_blocks_t;

typedef struct sw_layout_block_composite {
	sw_layout_blocks_t children;
	sw_layout_block_composite_children_layout_t layout;
	SU_PAD32;
} sw_layout_block_composite_t;

typedef su_bool32_t (*sw_layout_block_notify_func_t)(sw_layout_block_t *, sw_context_t *, sw_layout_block_event_t );
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
	uint32_t fill; /* sw_layout_block_fill_t | */
	int32_t x, y;
	int32_t min_width, max_width;
	int32_t min_height, max_height;
	int32_t content_width, content_height;
	sw_layout_block_content_repeat_t content_repeat;
	sw_layout_block_content_anchor_t content_anchor;
	sw_layout_block_content_transform_t content_transform;
	sw_layout_block_border_t borders[4]; /* left right bottom top */
	sw_layout_block_notify_func_t notify; /* may be NULL */
} sw_layout_block_in_t;

#if SW_WITH_TEXT
typedef struct sw_glyph {
	uint32_t codepoint;
	int32_t cluster; /* TODO: rework */
	int32_t x, y;
	int32_t width, height;
} sw_glyph_t;

typedef struct sw_layout_block_out_text {
	sw_glyph_t *glyphs;
	size_t glyphs_count;
} sw_layout_block_out_text_t;
#endif /* SW_WITH_TEXT */

#if SW_WITH_GIF
typedef struct sw_layout_block_out_gif {
	size_t frame_idx;
} sw_layout_block_out_gif_t;
#endif /* SW_WITH_GIF */

#if SW_WITH_TEXT || SW_WITH_GIF
typedef union sw_layout_block_out_ {
#if SW_WITH_TEXT
	sw_layout_block_out_text_t text;
#endif /* SW_WITH_TEXT */
#if SW_WITH_GIF
	sw_layout_block_out_gif_t gif;
#endif /* SW_WITH_GIF */
} sw_layout_block_out__t;
#endif /* SW_WITH_TEXT || SW_WITH_GIF */

typedef struct sw_layout_block_out {
#if SW_WITH_TEXT || SW_WITH_GIF
	sw_layout_block_out__t _;
#endif /* SW_WITH_TEXT || SW_WITH_GIF */
	sw_layout_block_dimensions_t dim;
	sw_layout_block_fini_func_t fini; /* must be called at destruction */
} sw_layout_block_out_t;

struct sw_layout_block {
	sw_layout_block_in_t in;
	sw_layout_block_out_t out;
	SW__PRIVATE_FIELDS(16);
	SU_LLIST_NODE_FIELDS(sw_layout_block_t);
};

typedef enum sw_backend_type {
	SW_BACKEND_TYPE_INVALID
#if SW_WITH_MEMORY_BACKEND
	,SW_BACKEND_TYPE_MEMORY
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
	,SW_BACKEND_TYPE_WAYLAND
#endif /* SW_WITH_WAYLAND_BACKEND */
} sw_backend_type_t;

#if SW_WITH_MEMORY_BACKEND
typedef struct sw_backend_memory_in {
	sw_layout_block_t *root;
	uint32_t width, height;
	/* TODO: more formats */
	sw_color_argb32_t *memory; /* width * height * 4 */
} sw_backend_memory_in_t;
#endif /* SW_WITH_MEMORY_BACKEND */

#if SW_WITH_WAYLAND_BACKEND
typedef struct sw_backend_wayland_in {
	sw_wayland_output_create_func_t output_create; /* may be NULL */
	sw_wayland_seat_create_func_t seat_create; /* may be NULL */
	sw_wayland_surfaces_t toplevels;
	sw_wayland_surfaces_t layers;
} sw_backend_wayland_in_t;
#endif /* SW_WITH_WAYLAND_BACKEND */

typedef union sw_backend_in {
#if SW_WITH_MEMORY_BACKEND
	sw_backend_memory_in_t memory;
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
	sw_backend_wayland_in_t wayland;
#endif /* SW_WITH_WAYLAND_BACKEND */
} sw_backend_in_t;

#if SW_WITH_MEMORY_BACKEND
typedef struct sw_backend_memory_out {
	size_t reserved;
} sw_backend_memory_out_t;
#endif /* SW_WITH_MEMORY_BACKEND */

#if SW_WITH_WAYLAND_BACKEND
typedef struct sw_wayland_outputs {
	SU_LLIST_FIELDS(sw_wayland_output_t);
} sw_wayland_outputs_t;

typedef struct sw_wayland_seats {
	SU_LLIST_FIELDS(sw_wayland_seat_t);
} sw_wayland_seats_t;

typedef struct sw_backend_wayland_out {
	struct pollfd pfd; /* TODO: generic structure to support select, epoll, kqueue etc */
	sw_wayland_outputs_t outputs;
	sw_wayland_seats_t seats;
} sw_backend_wayland_out_t;
#endif /* SW_WITH_WAYLAND_BACKEND */

typedef union sw_backend_out {
#if SW_WITH_MEMORY_BACKEND
	sw_backend_memory_out_t memory;
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
	sw_backend_wayland_out_t wayland;
#endif /* SW_WITH_WAYLAND_BACKEND */
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

/* ? TODO: strip by default, flag to enable */
#if defined(SW_STRIP_PREFIXES)

typedef sw_layout_block_event_t layout_block_event_t;
#define LAYOUT_BLOCK_EVENT_PREPARE SW_LAYOUT_BLOCK_EVENT_PREPARE
#define LAYOUT_BLOCK_EVENT_PREPARED SW_LAYOUT_BLOCK_EVENT_PREPARED
#define LAYOUT_BLOCK_EVENT_ERROR_INVALID_IMAGE SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_IMAGE
#if SW_WITH_TEXT
#define LAYOUT_BLOCK_EVENT_ERROR_INVALID_FONT SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_FONT
#define LAYOUT_BLOCK_EVENT_ERROR_INVALID_TEXT SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_TEXT
#endif /* SW_WITH_TEXT */

#if SW_WITH_MEMORY_BACKEND

typedef sw_backend_memory_in_t backend_memory_in_t;
typedef sw_backend_memory_out_t backend_memory_out_t;

#endif /* SW_WITH_MEMORY_BACKEND */

#if SW_WITH_WAYLAND_BACKEND

typedef sw_backend_wayland_in_t backend_wayland_in_t;
typedef sw_backend_wayland_out_t backend_wayland_out_t;

typedef sw_wayland_event_t wayland_event_t;
#define WAYLAND_EVENT_SURFACE_CLOSE SW_WAYLAND_EVENT_SURFACE_CLOSE
#define WAYLAND_EVENT_SURFACE_FAILED_TO_SET_CURSOR_SHAPE SW_WAYLAND_EVENT_SURFACE_FAILED_TO_SET_CURSOR_SHAPE
#define WAYLAND_EVENT_SURFACE_FAILED_TO_SET_DECORATIONS SW_WAYLAND_EVENT_SURFACE_FAILED_TO_SET_DECORATIONS
#define WAYLAND_EVENT_SURFACE_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK SW_WAYLAND_EVENT_SURFACE_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK
#define WAYLAND_EVENT_SURFACE_LAYOUT_FAILED SW_WAYLAND_EVENT_SURFACE_LAYOUT_FAILED
#define WAYLAND_EVENT_SURFACE_ERROR_MISSING_PROTOCOL SW_WAYLAND_EVENT_SURFACE_ERROR_MISSING_PROTOCOL
#define WAYLAND_EVENT_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER SW_WAYLAND_EVENT_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER
#define WAYLAND_EVENT_POINTER_ENTER SW_WAYLAND_EVENT_POINTER_ENTER
#define WAYLAND_EVENT_POINTER_LEAVE SW_WAYLAND_EVENT_POINTER_LEAVE
#define WAYLAND_EVENT_POINTER_MOTION SW_WAYLAND_EVENT_POINTER_MOTION
#define WAYLAND_EVENT_POINTER_BUTTON SW_WAYLAND_EVENT_POINTER_BUTTON
#define WAYLAND_EVENT_POINTER_SCROLL SW_WAYLAND_EVENT_POINTER_SCROLL

typedef sw_wayland_surfaces_t wayland_surfaces_t;
typedef sw_layout_blocks_t layout_blocks_t;
typedef sw_wayland_outputs_t wayland_outputs_t;
typedef sw_wayland_seats_t wayland_seats_t;

typedef sw_wayland_output_in_t wayland_output_in_t;
typedef sw_wayland_output_out_t wayland_output_out_t;
typedef sw_wayland_output_t wayland_output_t;
typedef sw_wayland_output_transform_t wayland_output_transform_t;
#define WAYLAND_OUTPUT_TRANSFORM_NORMAL SW_WAYLAND_OUTPUT_TRANSFORM_NORMAL
#define WAYLAND_OUTPUT_TRANSFORM_90 SW_WAYLAND_OUTPUT_TRANSFORM_90
#define WAYLAND_OUTPUT_TRANSFORM_180 SW_WAYLAND_OUTPUT_TRANSFORM_180
#define WAYLAND_OUTPUT_TRANSFORM_270 SW_WAYLAND_OUTPUT_TRANSFORM_270
#define WAYLAND_OUTPUT_TRANSFORM_FLIPPED SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED
#define WAYLAND_OUTPUT_TRANSFORM_FLIPPED_90 SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED_90
#define WAYLAND_OUTPUT_TRANSFORM_FLIPPED_180 SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED_180
#define WAYLAND_OUTPUT_TRANSFORM_FLIPPED_270 SW_WAYLAND_OUTPUT_TRANSFORM_FLIPPED_270
typedef sw_wayland_output_subpixel_t wayland_output_subpixel_t;
#define WAYLAND_OUTPUT_SUBPIXEL_UNKNOWN SW_WAYLAND_OUTPUT_SUBPIXEL_UNKNOWN
#define WAYLAND_OUTPUT_SUBPIXEL_NONE SW_WAYLAND_OUTPUT_SUBPIXEL_NONE
#define WAYLAND_OUTPUT_SUBPIXEL_HORIZONTAL_RGB SW_WAYLAND_OUTPUT_SUBPIXEL_HORIZONTAL_RGB
#define WAYLAND_OUTPUT_SUBPIXEL_HORIZONTAL_BGR SW_WAYLAND_OUTPUT_SUBPIXEL_HORIZONTAL_BGR
#define WAYLAND_OUTPUT_SUBPIXEL_VERTICAL_RGB SW_WAYLAND_OUTPUT_SUBPIXEL_VERTICAL_RGB
#define WAYLAND_OUTPUT_SUBPIXEL_VERTICAL_BGR SW_WAYLAND_OUTPUT_SUBPIXEL_VERTICAL_BGR
typedef sw_wayland_output_destroy_func_t sw_wayland_output_destroy_func_t;
typedef sw_wayland_output_create_func_t wayland_output_create_func_t;
typedef sw_wayland_pointer_button_state_t wayland_pointer_button_state_t;
#define WAYLAND_POINTER_BUTTON_STATE_RELEASED SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED
#define WAYLAND_POINTER_BUTTON_STATE_PRESSED SW_WAYLAND_POINTER_BUTTON_STATE_PRESSED
typedef sw_wayland_pointer_scroll_axis_t wayland_pointer_scroll_axis_t;
#define WAYLAND_POINTER_AXIS_VERTICAL_SCROLL SW_WAYLAND_POINTER_AXIS_VERTICAL_SCROLL
#define WAYLAND_POINTER_AXIS_HORIZONTAL_SCROLL SW_WAYLAND_POINTER_AXIS_HORIZONTAL_SCROLL
typedef sw_wayland_pointer_in_t wayland_pointer_in_t;
typedef sw_wayland_pointer_out_t wayland_pointer_out_t;
typedef sw_wayland_pointer_t wayland_pointer_t;
typedef sw_wayland_pointer_func_t wayland_pointer_func_t;
typedef sw_wayland_pointer_create_func_t wayland_pointer_create_func_t;
typedef sw_wayland_seat_in_t wayland_seat_in_t;
typedef sw_wayland_seat_out_t wayland_seat_out_t;
typedef sw_wayland_seat_t wayland_seat_t;
typedef sw_wayland_seat_destroy_func_t wayland_seat_destroy_func_t;
typedef sw_wayland_seat_create_func_t wayland_seat_create_func_t;
typedef sw_wayland_surface_layer_anchor_t wayland_surface_layer_anchor_t;
#define WAYLAND_SURFACE_LAYER_ANCHOR_NONE SW_WAYLAND_SURFACE_LAYER_ANCHOR_NONE
#define WAYLAND_SURFACE_LAYER_ANCHOR_TOP SW_WAYLAND_SURFACE_LAYER_ANCHOR_TOP
#define WAYLAND_SURFACE_LAYER_ANCHOR_BOTTOM SW_WAYLAND_SURFACE_LAYER_ANCHOR_BOTTOM
#define WAYLAND_SURFACE_LAYER_ANCHOR_LEFT SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT
#define WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT
#define WAYLAND_SURFACE_LAYER_ANCHOR_ALL SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL
typedef sw_wayland_surface_layer_layer_t wayland_surface_layer_layer_t;
#define WAYLAND_SURFACE_LAYER_LAYER_BACKGROUND SW_WAYLAND_SURFACE_LAYER_LAYER_BACKGROUND
#define WAYLAND_SURFACE_LAYER_LAYER_BOTTOM SW_WAYLAND_SURFACE_LAYER_LAYER_BOTTOM
#define WAYLAND_SURFACE_LAYER_LAYER_TOP SW_WAYLAND_SURFACE_LAYER_LAYER_TOP
#define WAYLAND_SURFACE_LAYER_LAYER_OVERLAY SW_WAYLAND_SURFACE_LAYER_LAYER_OVERLAY
typedef sw_wayland_cursor_shape_t wayland_cursor_shape_t;
#define WAYLAND_CURSOR_SHAPE_DEFAULT W_WAYLAND_CURSOR_SHAPE_DEFAULT
#define WAYLAND_CURSOR_SHAPE_DEFAULT_ W_WAYLAND_CURSOR_SHAPE_DEFAULT_
#define WAYLAND_CURSOR_SHAPE_CONTEXT_MENU W_WAYLAND_CURSOR_SHAPE_CONTEXT_MENU
#define WAYLAND_CURSOR_SHAPE_HELP W_WAYLAND_CURSOR_SHAPE_HELP
#define WAYLAND_CURSOR_SHAPE_POINTER W_WAYLAND_CURSOR_SHAPE_POINTER
#define WAYLAND_CURSOR_SHAPE_PROGRESS W_WAYLAND_CURSOR_SHAPE_PROGRESS
#define WAYLAND_CURSOR_SHAPE_WAIT W_WAYLAND_CURSOR_SHAPE_WAIT
#define WAYLAND_CURSOR_SHAPE_CELL W_WAYLAND_CURSOR_SHAPE_CELL
#define WAYLAND_CURSOR_SHAPE_CROSSHAIR W_WAYLAND_CURSOR_SHAPE_CROSSHAIR
#define WAYLAND_CURSOR_SHAPE_TEXT W_WAYLAND_CURSOR_SHAPE_TEXT
#define WAYLAND_CURSOR_SHAPE_VERTICAL_TEXT SW_WAYLAND_CURSOR_SHAPE_VERTICAL_TEXT 
#define WAYLAND_CURSOR_SHAPE_ALIAS SW_WAYLAND_CURSOR_SHAPE_ALIAS 
#define WAYLAND_CURSOR_SHAPE_COPY SW_WAYLAND_CURSOR_SHAPE_COPY 
#define WAYLAND_CURSOR_SHAPE_MOVE SW_WAYLAND_CURSOR_SHAPE_MOVE 
#define WAYLAND_CURSOR_SHAPE_NO_DROP SW_WAYLAND_CURSOR_SHAPE_NO_DROP 
#define WAYLAND_CURSOR_SHAPE_NOT_ALLOWED SW_WAYLAND_CURSOR_SHAPE_NOT_ALLOWED 
#define WAYLAND_CURSOR_SHAPE_GRAB SW_WAYLAND_CURSOR_SHAPE_GRAB 
#define WAYLAND_CURSOR_SHAPE_GRABBING SW_WAYLAND_CURSOR_SHAPE_GRABBING 
#define WAYLAND_CURSOR_SHAPE_E_RESIZE SW_WAYLAND_CURSOR_SHAPE_E_RESIZE 
#define WAYLAND_CURSOR_SHAPE_N_RESIZE SW_WAYLAND_CURSOR_SHAPE_N_RESIZE 
#define WAYLAND_CURSOR_SHAPE_NE_RESIZE SW_WAYLAND_CURSOR_SHAPE_NE_RESIZE 
#define WAYLAND_CURSOR_SHAPE_NW_RESIZE SW_WAYLAND_CURSOR_SHAPE_NW_RESIZE 
#define WAYLAND_CURSOR_SHAPE_S_RESIZE SW_WAYLAND_CURSOR_SHAPE_S_RESIZE 
#define WAYLAND_CURSOR_SHAPE_SE_RESIZE SW_WAYLAND_CURSOR_SHAPE_SE_RESIZE 
#define WAYLAND_CURSOR_SHAPE_SW_RESIZE SW_WAYLAND_CURSOR_SHAPE_SW_RESIZE 
#define WAYLAND_CURSOR_SHAPE_W_RESIZE SW_WAYLAND_CURSOR_SHAPE_W_RESIZE 
#define WAYLAND_CURSOR_SHAPE_EW_RESIZE SW_WAYLAND_CURSOR_SHAPE_EW_RESIZE 
#define WAYLAND_CURSOR_SHAPE_NS_RESIZE SW_WAYLAND_CURSOR_SHAPE_NS_RESIZE 
#define WAYLAND_CURSOR_SHAPE_NESW_RESIZE SW_WAYLAND_CURSOR_SHAPE_NESW_RESIZE 
#define WAYLAND_CURSOR_SHAPE_NWSE_RESIZE SW_WAYLAND_CURSOR_SHAPE_NWSE_RESIZE 
#define WAYLAND_CURSOR_SHAPE_COL_RESIZE SW_WAYLAND_CURSOR_SHAPE_COL_RESIZE 
#define WAYLAND_CURSOR_SHAPE_ROW_RESIZE SW_WAYLAND_CURSOR_SHAPE_ROW_RESIZE 
#define WAYLAND_CURSOR_SHAPE_ALL_SCROLL SW_WAYLAND_CURSOR_SHAPE_ALL_SCROLL 
#define WAYLAND_CURSOR_SHAPE_ZOOM_IN SW_WAYLAND_CURSOR_SHAPE_ZOOM_IN 
#define WAYLAND_CURSOR_SHAPE_ZOOM_OUT SW_WAYLAND_CURSOR_SHAPE_ZOOM_OUT
/*#define WAYLAND_CURSOR_SHAPE_DND_ASK SW_WAYLAND_CURSOR_SHAPE_DND_ASK */
/*#define WAYLAND_CURSOR_SHAPE_ALL_RESIZE SW_WAYLAND_CURSOR_SHAPE_ALL_RESIZE */
typedef sw_wayland_region_t wayland_region_t;
typedef sw_wayland_surface_popup_gravity_t wayland_surface_popup_gravity_t;
#define WAYLAND_SURFACE_POPUP_GRAVITY_NONE SW_WAYLAND_SURFACE_POPUP_GRAVITY_NONE
#define WAYLAND_SURFACE_POPUP_GRAVITY_TOP SW_WAYLAND_SURFACE_POPUP_GRAVITY_TOP
#define WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM SW_WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM
#define WAYLAND_SURFACE_POPUP_GRAVITY_LEFT SW_WAYLAND_SURFACE_POPUP_GRAVITY_LEFT
#define WAYLAND_SURFACE_POPUP_GRAVITY_RIGHT SW_WAYLAND_SURFACE_POPUP_GRAVITY_RIGHT
#define WAYLAND_SURFACE_POPUP_GRAVITY_TOP_LEFT SW_WAYLAND_SURFACE_POPUP_GRAVITY_TOP_LEFT
#define WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM_LEFT SW_WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM_LEFT
#define WAYLAND_SURFACE_POPUP_GRAVITY_TOP_RIGHT SW_WAYLAND_SURFACE_POPUP_GRAVITY_TOP_RIGHT
#define WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM_RIGHT SW_WAYLAND_SURFACE_POPUP_GRAVITY_BOTTOM_RIGHT
typedef sw_wayland_surface_popup_constraint_adjustment_t wayland_surface_popup_constraint_adjustment_t;
#define WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_NONE SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_NONE
#define WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_SLIDE_X SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_SLIDE_X
#define WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_SLIDE_Y SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_SLIDE_Y
#define WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_X SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_X
#define WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_Y SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_Y
#define WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_RESIZE_X SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_RESIZE_X
#define WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_RESIZE_Y SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_RESIZE_Y
typedef sw_wayland_surface_type_t wayland_surface_type_t;
#define WAYLAND_SURFACE_TYPE_TOPLEVEL SW_WAYLAND_SURFACE_TYPE_TOPLEVEL
#define WAYLAND_SURFACE_TYPE_LAYER SW_WAYLAND_SURFACE_TYPE_LAYER
#define WAYLAND_SURFACE_TYPE_POPUP SW_WAYLAND_SURFACE_TYPE_POPUP
typedef sw_wayland_surface_toplevel_state_t wayland_surface_toplevel_state_t;
#define WAYLAND_SURFACE_TOPLEVEL_STATE_MAXIMIZED SW_WAYLAND_SURFACE_TOPLEVEL_STATE_MAXIMIZED
#define WAYLAND_SURFACE_TOPLEVEL_STATE_FULLSCREEN SW_WAYLAND_SURFACE_TOPLEVEL_STATE_FULLSCREEN
#define WAYLAND_SURFACE_TOPLEVEL_STATE_RESIZING SW_WAYLAND_SURFACE_TOPLEVEL_STATE_RESIZING
#define WAYLAND_SURFACE_TOPLEVEL_STATE_ACTIVATED SW_WAYLAND_SURFACE_TOPLEVEL_STATE_ACTIVATED
#define WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_LEFT SW_WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_LEFT
#define WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_RIGHT SW_WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_RIGHT
#define WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_TOP SW_WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_TOP
#define WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_BOTTOM SW_WAYLAND_SURFACE_TOPLEVEL_STATE_TILED_BOTTOM
typedef sw_wayland_toplevel_decoration_mode_t wayland_toplevel_decoration_mode_t;
#define WAYLAND_TOPLEVEL_DECORATION_MODE_COMPOSITOR_DEFAULT SW_WAYLAND_TOPLEVEL_DECORATION_MODE_COMPOSITOR_DEFAULT
#define WAYLAND_TOPLEVEL_DECORATION_MODE_CLIENT_SIDE SW_WAYLAND_TOPLEVEL_DECORATION_MODE_CLIENT_SIDE
#define WAYLAND_TOPLEVEL_DECORATION_MODE_SERVER_SIDE SW_WAYLAND_TOPLEVEL_DECORATION_MODE_SERVER_SIDE
typedef sw_wayland_surface_toplevel_t wayland_surface_toplevel_t;
typedef sw_wayland_surface_layer_t wayland_surface_layer_t;
typedef sw_wayland_surface_popup_t wayland_surface_popup_t;
typedef sw_wayland_surface_in__t wayland_surface_in__t;
typedef sw_wayland_surface_in_t wayland_surface_in_t;
typedef sw_wayland_surface_out_toplevel_t wayland_surface_out_toplevel_t;
typedef sw_wayland_surface_out__t wayland_surface_out__t;
typedef sw_wayland_surface_out_t wayland_surface_out_t;
typedef sw_wayland_surface_t wayland_surface_t;
typedef sw_wayland_event_source_t wayland_event_source_t;
typedef sw_wayland_notify_func_t wayland_notify_func_t;
typedef sw_wayland_surface_fini_func_t wayland_surface_fini_func_t;

#endif /* SW_WITH_WAYLAND_BACKEND */

typedef sw_pixmap_t pixmap_t;
typedef sw_point_t point_t;

typedef sw_color_argb32_t color_argb32_t;
typedef sw_color_gradient_stop_t color_gradient_stop_t;
typedef sw_color_linear_gradient_t color_linear_gradient_t;
typedef sw_color_conical_gradient_t color_conical_gradient_t;
typedef sw_color_radial_gradient_t color_radial_gradient_t;
typedef sw_color__t color__t;
typedef sw_color_type_t color_type_t;
#define COLOR_TYPE_ARGB32 SW_COLOR_TYPE_ARGB32
#define COLOR_TYPE_LINEAR_GRADIENT SW_COLOR_TYPE_LINEAR_GRADIENT
#define COLOR_TYPE_CONICAL_GRADIENT SW_COLOR_TYPE_CONICAL_GRADIENT
#define COLOR_TYPE_RADIAL_GRADIENT SW_COLOR_TYPE_RADIAL_GRADIENT
typedef sw_color_t color_t;

typedef sw_layout_block_type_t layout_block_type_t;
#define LAYOUT_BLOCK_TYPE_SPACER SW_LAYOUT_BLOCK_TYPE_SPACER
#if SW_WITH_TEXT
#define LAYOUT_BLOCK_TYPE_TEXT SW_LAYOUT_BLOCK_TYPE_TEXT
#endif /* SW_WITH_TEXT */ 
#define LAYOUT_BLOCK_TYPE_IMAGE SW_LAYOUT_BLOCK_TYPE_IMAGE
#define LAYOUT_BLOCK_TYPE_COMPOSITE SW_LAYOUT_BLOCK_TYPE_COMPOSITE
typedef sw_layout_block_anchor_t layout_block_anchor_t;
#define LAYOUT_BLOCK_ANCHOR_LEFT SW_LAYOUT_BLOCK_ANCHOR_LEFT
#define LAYOUT_BLOCK_ANCHOR_TOP SW_LAYOUT_BLOCK_ANCHOR_TOP
#define LAYOUT_BLOCK_ANCHOR_RIGHT SW_LAYOUT_BLOCK_ANCHOR_RIGHT
#define LAYOUT_BLOCK_ANCHOR_BOTTOM SW_LAYOUT_BLOCK_ANCHOR_BOTTOM
#define LAYOUT_BLOCK_ANCHOR_CENTER SW_LAYOUT_BLOCK_ANCHOR_CENTER
#define LAYOUT_BLOCK_ANCHOR_NONE SW_LAYOUT_BLOCK_ANCHOR_NONE
typedef sw_layout_block_content_anchor_t layout_block_content_anchor_t;
#define LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_CENTER SW_LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_CENTER
#define LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_TOP SW_LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_TOP
#define LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_BOTTOM SW_LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_BOTTOM
#define LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_TOP SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_TOP
#define LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER
#define LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_BOTTOM SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_BOTTOM
#define LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_TOP SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_TOP
#define LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_CENTER SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_CENTER
#define LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_BOTTOM SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_BOTTOM
typedef sw_layout_block_content_transform_t layout_block_content_transform_t;
#define LAYOUT_BLOCK_CONTENT_TRANSFORM_NORMAL SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_NORMAL
#define LAYOUT_BLOCK_CONTENT_TRANSFORM_90 SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_90
#define LAYOUT_BLOCK_CONTENT_TRANSFORM_180 SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_180
#define LAYOUT_BLOCK_CONTENT_TRANSFORM_270 SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_270
#define LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED
#define LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_90 SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_90
#define LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_180 SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_180
#define LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_270 SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_270
typedef sw_layout_block_fill_t layout_block_fill_t;
#define LAYOUT_BLOCK_FILL_NONE SW_LAYOUT_BLOCK_FILL_NONE
#define LAYOUT_BLOCK_FILL_LEFT SW_LAYOUT_BLOCK_FILL_LEFT
#define LAYOUT_BLOCK_FILL_RIGHT SW_LAYOUT_BLOCK_FILL_RIGHT
#define LAYOUT_BLOCK_FILL_BOTTOM SW_LAYOUT_BLOCK_FILL_BOTTOM
#define LAYOUT_BLOCK_FILL_TOP SW_LAYOUT_BLOCK_FILL_TOP
#define LAYOUT_BLOCK_FILL_CONTENT SW_LAYOUT_BLOCK_FILL_CONTENT
#define LAYOUT_BLOCK_FILL_ALL_SIDES SW_LAYOUT_BLOCK_FILL_ALL_SIDES
#define LAYOUT_BLOCK_FILL_ALL_SIDES_CONTENT SW_LAYOUT_BLOCK_FILL_ALL_SIDES_CONTENT
typedef sw_layout_block_border_t layout_block_border_t;
typedef sw_layout_block_composite_children_layout_t layout_block_composite_children_layout_t;
#define LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_HORIZONTAL SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_HORIZONTAL
#define LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_VERTICAL SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_VERTICAL
typedef sw_layout_block_content_repeat_t layout_block_content_repeat_t;
#define LAYOUT_BLOCK_CONTENT_REPEAT_NONE SW_LAYOUT_BLOCK_CONTENT_REPEAT_NONE
#define LAYOUT_BLOCK_CONTENT_REPEAT_NORMAL SW_LAYOUT_BLOCK_CONTENT_REPEAT_NORMAL
#define LAYOUT_BLOCK_CONTENT_REPEAT_PAD SW_LAYOUT_BLOCK_CONTENT_REPEAT_PAD
#define LAYOUT_BLOCK_CONTENT_REPEAT_REFLECT SW_LAYOUT_BLOCK_CONTENT_REPEAT_REFLECT
typedef sw_layout_block_dimensions_t layout_block_dimensions_t;
typedef sw_layout_block_text_t layout_block_text_t;
typedef sw_layout_block_image_image_type_t layout_block_image_image_type_t;
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_AUTO SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_AUTO
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SW_PIXMAP SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SW_PIXMAP
#if SW_WITH_PNG
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNG SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNG
#endif /* SW_WITH_PNG */
#if SW_WITH_JPG
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_JPG SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_JPG
#endif /* SW_WITH_JPG */
#if SW_WITH_SVG
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SVG SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SVG
#endif /* SW_WITH_SVG */
#if SW_WITH_TGA
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_TGA SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_TGA
#endif /* SW_WITH_TGA */
#if SW_WITH_BMP
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_BMP SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_BMP
#endif /* SW_WITH_BMP */
#if SW_WITH_PSD
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PSD SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PSD
#endif /* SW_WITH_PSD */
#if SW_WITH_GIF
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_GIF SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_GIF
#endif /* SW_WITH_GIF */
#if SW_WITH_HDR
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_HDR SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_HDR
#endif /* SW_WITH_HDR */
#if SW_WITH_PIC
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PIC SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PIC
#endif /* SW_WITH_PIC */
#if SW_WITH_PNM
#define LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNM SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNM
#endif /* SW_WITH_PNM */
typedef sw_layout_block_image_t layout_block_image_t;
typedef sw_layout_block_composite_t layout_block_composite_t;
typedef sw_layout_block__t layout_block__t;
typedef sw_layout_block_in_t layout_block_in_t;
typedef sw_layout_block_out_t layout_block_out_t;
#if SW_WITH_TEXT
typedef sw_layout_block_out_text_t layout_block_out_text_t;
typedef sw_glyph_t glyph_t;
#endif /* SW_WITH_TEXT */
#if SW_WITH_GIF
typedef sw_layout_block_out_gif_t layout_block_out_gif_t;
#endif /* SW_WITH_GIF */
#if SW_WITH_TEXT || SW_WITH_GIF
typedef sw_layout_block_out__t layout_block_out__t;
#endif /* SW_WITH_TEXT || SW_WITH_GIF */
typedef sw_layout_block_t layout_block_t;
typedef sw_layout_block_notify_func_t layout_block_notify_func_t;
typedef sw_layout_block_fini_func_t layout_block_fini_func_t;

typedef sw_backend_type_t backend_type_t;
#define BACKEND_TYPE_INVALID SW_BACKEND_TYPE_INVALID
#if SW_WITH_MEMORY_BACKEND
#define BACKEND_TYPE_MEMORY SW_BACKEND_TYPE_MEMORY
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
#define BACKEND_TYPE_WAYLAND SW_BACKEND_TYPE_WAYLAND
#endif /* SW_WITH_WAYLAND_BACKEND */
typedef sw_backend_in_t backend_in_t;
typedef sw_backend_out_t backend_out_t;

typedef sw_context_in_t context_in_t;
typedef sw_context_out_t context_out_t;
typedef sw_context_t context_t;

/*#define set sw_set */
/*#define cleanup sw_cleanup */
/*#define flush sw_flush */
/*#define process sw_process */

#endif /* defined(SW_STRIP_PREFIXES) */

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* SW_HEADER */


#if defined(SW_IMPLEMENTATION) && !defined(SW__REIMPLEMENTATION_GUARD)
#define SW__REIMPLEMENTATION_GUARD

#if !defined(SU_IMPLEMENTATION)
#define SU_IMPLEMENTATION
#endif /* !defined(SU_IMPLEMENTATION) */
#include <sutil.h>

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

SU_IGNORE_WARNINGS_START

SU_IGNORE_WARNING("-Wc99-extensions")
SU_IGNORE_WARNING("-Wcomment")
SU_IGNORE_WARNING("-Wc++98-compat-pedantic")

SU_IGNORE_WARNING("-Wpedantic")

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
#include <resvg.h>
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

#define memcpy SU_MEMCPY
#define memset SU_MEMSET
#define strcmp SU_STRCMP
#define strncmp SU_STRNCMP
#include <stb_image.h>
#undef memcpy
#undef memset
#undef strcmp
#undef strncmp

#endif /* SW_WITH_PNG || SW_WITH_JPG || SW_WITH_TGA || SW_WITH_BMP || SW_WITH_PSD || SW_WITH_GIF || SW_WITH_HDR || SW_WITH_PIC || SW_WITH_PNM */

#if SW_WITH_WAYLAND_BACKEND

#include <sys/mman.h>

SU_IGNORE_WARNINGS_START

SU_IGNORE_WARNING("-Wc99-extensions")
SU_IGNORE_WARNING("-Wc++98-compat-pedantic")
SU_IGNORE_WARNING("-Wcast-qual")

#if SU_HAS_INCLUDE(<wayland-client/wayland-client.h>)
#include <wayland-client/wayland-client.h>
#else
#include <wayland-client.h>
#endif

#include <wlr-layer-shell-unstable-v1.h>
#include <wlr-layer-shell-unstable-v1.c>
#include <xdg-shell.h>
#include <xdg-shell.c>
#include <tablet-unstable-v2.c>
#include <cursor-shape-v1.h>
#include <cursor-shape-v1.c>
#include <xdg-decoration-unstable-v1.h>
#include <xdg-decoration-unstable-v1.c>

SU_IGNORE_WARNINGS_END

typedef struct sw__wayland_output {
	su_bool32_t initialized;
	uint32_t wl_name;
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
	su_bool32_t initialized;
} sw__wayland_seat_t;

typedef struct sw__wayland_surface_buffer {
	struct wl_buffer *wl_buffer;
	pixman_image_t *image; /* bits */
	uint32_t *pixels;
	uint32_t size;
	su_bool32_t busy;
} sw__wayland_surface_buffer_t;

typedef struct sw__wayland_surface_toplevel {
	struct xdg_surface *xdg_surface; /* must be first */
	struct xdg_toplevel *xdg_toplevel;
	struct zxdg_toplevel_decoration_v1 *decoration;
	su_string_t title, app_id;
	sw_wayland_toplevel_decoration_mode_t decoration_mode;
	int32_t min_width, min_height;
	int32_t max_width, max_height;
	SU_PAD32;
} sw__wayland_surface_toplevel_t;

typedef struct sw__wayland_surface_layer {
	struct zwlr_layer_surface_v1 *layer_surface;
	int32_t exclusive_zone;
	uint32_t anchor; /* sw_wayland_surface_layer_anchor_t | */
	sw_wayland_surface_layer_layer_t layer;
	int32_t margins[4]; /* top right bottom left */
	SU_PAD32;
	sw_wayland_output_t *output;
} sw__wayland_surface_layer_t;

typedef struct sw__wayland_surface_popup {
	struct xdg_surface *xdg_surface; /* must be first */
	sw_wayland_surface_t *parent;
	sw_wayland_pointer_t *grab;
	struct xdg_popup *xdg_popup;
	struct xdg_positioner *xdg_positioner;
	int32_t x, y;
    sw_wayland_surface_popup_gravity_t gravity;
    uint32_t constraint_adjustment; /* sw_wayland_surface_popup_constraint_adjustment_t | */
} sw__wayland_surface_popup_t;

typedef union sw__wayland_surface_ {
	sw__wayland_surface_toplevel_t toplevel;
	sw__wayland_surface_layer_t layer;
	sw__wayland_surface_popup_t popup;
} sw__wayland_surface__t;

typedef struct sw__wayland_surface {
	sw__wayland_surface__t _;
	struct wl_surface *wl_surface;
	sw__wayland_surface_buffer_t buffer;
	su_bool32_t dirty;
	sw_wayland_cursor_shape_t cursor_shape;
    sw_wayland_region_t *input_regions;
	size_t input_regions_count;
} sw__wayland_surface_t;

#endif /* SW_WITH_WAYLAND_BACKEND */

typedef struct sw__layout_block {
	pixman_image_t *content_image;
	su_bool32_t valid;
	SU_PAD32;
} sw__layout_block_t;

#if SW_WITH_GIF
typedef struct sw__image_gif_frame {
	pixman_image_t *image; /* bits */
	uint16_t delay; /* ms */
	SU_PAD16;
	SU_PAD32;
} sw__image_gif_frame_t;

typedef struct sw__image_multiframe_gif {
	sw__image_gif_frame_t *frames;
	size_t frames_count;
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

typedef struct sw__text_run_cache {
	SU_HASH_TABLE_FIELDS(su_string_t);
	sw__text_run_cache_entry_t items[8]; /* ? TODO: dynamic */
	size_t items_count;
} sw__text_run_cache_t;

/* TODO: better hash function */
SU_HASH_TABLE_DECLARE_DEFINE(sw__text_run_cache_t, su_string_t, su_stbds_hash_string, su_string_equal, 16)
#endif /* SW_WITH_TEXT */

#if SW_WITH_MEMORY_BACKEND
typedef struct sw__context_memory {
	pixman_image_t *image;
} sw__context_memory_t;
#endif /* SW_WITH_MEMORY_BACKEND */

#if SW_WITH_WAYLAND_BACKEND
typedef struct sw__context_wayland {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct xdg_wm_base *wm_base;
	struct wp_cursor_shape_manager_v1 *cursor_shape_manager;
	struct zxdg_decoration_manager_v1 *decoration_manager;
} sw__context_wayland_t;
#endif /* SW_WITH_WAYLAND_BACKEND */

typedef union sw__context_ {
#if SW_WITH_MEMORY_BACKEND
	sw__context_memory_t memory;
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
	sw__context_wayland_t wayland;
#endif /* SW_WITH_WAYLAND_BACKEND */
} sw__context__t;

typedef struct sw__context {
	sw__context__t _;

	/* ? TODO: arena for content: */
	su_hash_table__sw__image_cache_t__t image_cache;
#if SW_WITH_TEXT
	su_hash_table__sw__text_run_cache_t__t text_run_cache;
#endif /* SW_WITH_TEXT */
} sw__context_t;


static SU_THREAD_LOCAL sw_context_t *sw__context;

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

SU_STATIC_ASSERT(sizeof(sw__context->sw__private) >= sizeof(sw__context_t));

#if SW_WITH_WAYLAND_BACKEND
SU_STATIC_ASSERT(sizeof(sw__context->out.backend.wayland.outputs.head->sw__private) >= sizeof(sw__wayland_output_t));
SU_STATIC_ASSERT(sizeof(sw__context->out.backend.wayland.seats.head->sw__private) >= sizeof(sw__wayland_seat_t));
SU_STATIC_ASSERT(sizeof(sw__context->out.backend.wayland.seats.head->out.pointer->sw__private) >= sizeof(sw__wayland_pointer_t));
SU_STATIC_ASSERT(sizeof(sw__context->in.backend.wayland.layers.head->sw__private) >= sizeof(sw__wayland_surface_t));
SU_STATIC_ASSERT(sizeof(sw__context->in.backend.wayland.layers.head->in.root->sw__private) >= sizeof(sw__layout_block_t));
#endif /* SW_WITH_WAYLAND_BACKEND */

#if SW_WITH_MEMORY_BACKEND
SU_STATIC_ASSERT(sizeof(sw__context->in.backend.memory.root->sw__private) >= sizeof(sw__layout_block_t));
#endif /* SW_WITH_MEMORY_BACKEND */

#if SW_WITH_PNG || SW_WITH_JPG || SW_WITH_TGA || SW_WITH_BMP || SW_WITH_PSD || SW_WITH_GIF || SW_WITH_HDR || SW_WITH_PIC || SW_WITH_PNM
static void *sw__malloc_stbi(size_t size) {
	void *ret;
	SU_ALLOCTSA(ret, sw__context->in.scratch_alloc, size, 32);
	return ret;
}

static void sw__free_stbi(void *ptr) {
	SU_FREE(sw__context->in.scratch_alloc, ptr);
}

static void *sw__realloc_sized_stbi(void *ptr, size_t old_size, size_t new_size) {
	su_allocator_t *scratch_alloc = sw__context->in.scratch_alloc;
	void *ret;
	SU_ALLOCTSA(ret, scratch_alloc, new_size, 32);
	if (ptr) {
		SU_MEMCPY(ret, ptr, SU_MIN(old_size, new_size));
		SU_FREE(scratch_alloc, ptr);
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
	c.red = (uint16_t)((((uint32_t)color.c.r * (uint32_t)color.c.a + 127) / 255) * 257);
	c.green = (uint16_t)((((uint32_t)color.c.g * (uint32_t)color.c.a + 127) / 255) * 257);
	c.blue = (uint16_t)((((uint32_t)color.c.b * (uint32_t)color.c.a + 127) / 255) * 257);

	return c;
}

static pixman_image_t *sw__color_to_pixman_image(sw_color_t color, su_allocator_t *scratch_alloc) {
	switch (color.type) {
	case SW_COLOR_TYPE_ARGB32: {
		pixman_color_t c = sw__color_argb32_to_pixman_color(color._.argb32);
		return pixman_image_create_solid_fill(&c);
	}
	case SW_COLOR_TYPE_LINEAR_GRADIENT: {
		sw_color_linear_gradient_t linear_gradient = color._.linear_gradient;
		pixman_gradient_stop_t *stops;
		sw_color_gradient_stop_t prev_stop = linear_gradient.stops[0];
		pixman_point_fixed_t p1, p2;
		pixman_image_t *image;
		size_t i = 0;

		SU_NOTUSED(prev_stop);
		SU_ASSERT(linear_gradient.stops_count >= 2);

		SU_ARRAY_ALLOC(stops, scratch_alloc, linear_gradient.stops_count);

		for ( ; i < linear_gradient.stops_count; ++i) {
			sw_color_gradient_stop_t stop = linear_gradient.stops[i];
			SU_ASSERT(stop.pos >= prev_stop.pos);
			stops[i].color = sw__color_argb32_to_pixman_color(stop.color);
			stops[i].x = pixman_double_to_fixed(stop.pos);
			prev_stop = stop;
		}

		p1.x = pixman_int_to_fixed(linear_gradient.p1.x);
		p1.y = pixman_int_to_fixed(linear_gradient.p1.y);
		p2.x = pixman_int_to_fixed(linear_gradient.p2.x);
		p2.y = pixman_int_to_fixed(linear_gradient.p2.y);
		image = pixman_image_create_linear_gradient(
			&p1, &p2, stops, (int)linear_gradient.stops_count);

		SU_FREE(scratch_alloc, stops);
		return image;
	}
	case SW_COLOR_TYPE_CONICAL_GRADIENT: {
		sw_color_conical_gradient_t conical_gradient = color._.conical_gradient;
		pixman_gradient_stop_t *stops;
		sw_color_gradient_stop_t prev_stop = conical_gradient.stops[0];
		size_t i = 0;
		pixman_point_fixed_t center;
		pixman_image_t *image;

		SU_NOTUSED(prev_stop);
		SU_ASSERT(conical_gradient.stops_count >= 2);

		SU_ARRAY_ALLOC(stops, scratch_alloc, conical_gradient.stops_count);

		for ( ; i < conical_gradient.stops_count; ++i) {
			sw_color_gradient_stop_t stop = conical_gradient.stops[i];
			SU_ASSERT(stop.pos >= prev_stop.pos);
			stops[i].color = sw__color_argb32_to_pixman_color(stop.color);
			stops[i].x = pixman_double_to_fixed(stop.pos);
			prev_stop = stop;
		}
		center.x = pixman_int_to_fixed(conical_gradient.center.x);
		center.y = pixman_int_to_fixed(conical_gradient.center.y);
		image = pixman_image_create_conical_gradient(
			&center, pixman_double_to_fixed(conical_gradient.angle),
			stops, (int)conical_gradient.stops_count);

		SU_FREE(scratch_alloc, stops);
		return image;
	}
	case SW_COLOR_TYPE_RADIAL_GRADIENT: {
		sw_color_radial_gradient_t radial_gradient = color._.radial_gradient;
		pixman_gradient_stop_t *stops;
		sw_color_gradient_stop_t prev_stop = radial_gradient.stops[0];
		size_t i = 0;
		pixman_point_fixed_t p_inner, p_outer;
		pixman_image_t *image;

		SU_NOTUSED(prev_stop);
		SU_ASSERT(radial_gradient.stops_count >= 2);

		SU_ARRAY_ALLOC(stops, scratch_alloc, radial_gradient.stops_count);

		for ( ; i < radial_gradient.stops_count; ++i) {
			sw_color_gradient_stop_t stop = radial_gradient.stops[i];
			SU_ASSERT(stop.pos >= prev_stop.pos);
			stops[i].color = sw__color_argb32_to_pixman_color(stop.color);
			stops[i].x = pixman_double_to_fixed(stop.pos);
			prev_stop = stop;
		}
		p_inner.x = pixman_int_to_fixed(radial_gradient.inner_p.x);
		p_inner.y = pixman_int_to_fixed(radial_gradient.inner_p.y);
		p_outer.x = pixman_int_to_fixed(radial_gradient.outer_p.x);
		p_outer.y = pixman_int_to_fixed(radial_gradient.outer_p.y);
		image = pixman_image_create_radial_gradient(
			&p_inner, &p_outer,
			pixman_int_to_fixed(radial_gradient.inner_r),
			pixman_int_to_fixed(radial_gradient.outer_r),
			stops, (int)radial_gradient.stops_count);

		SU_FREE(scratch_alloc, stops);
		return image;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}
}

static void sw__image_handle_destroy(pixman_image_t *image, void *data) {
	su_allocator_t *gp_alloc = sw__context->in.gp_alloc;
	sw__image_data_t *image_data = (sw__image_data_t *)data;

	SU_NOTUSED(image);

#if SW_WITH_SVG || SW_WITH_GIF
	switch (image_data->type) {
	case SW__IMAGE_DATA_TYPE_NONE:
		break;
#if SW_WITH_SVG
	case SW__IMAGE_DATA_TYPE_SVG:
		resvg_tree_destroy((resvg_render_tree *)image_data->data);
		break;
#endif /* SW_WITH_SVG */
#if SW_WITH_GIF
	case SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF: {
		sw__image_multiframe_gif_t *gif = (sw__image_multiframe_gif_t *)image_data->data;
		size_t i = 0;
		for ( ; i < gif->frames_count; ++i) {
			pixman_image_t *frame_image = gif->frames[i].image;
			sw__image_data_t *frame_data = (sw__image_data_t *)pixman_image_get_destroy_data(frame_image);
			frame_data->type = SW__IMAGE_DATA_TYPE_NONE;
			pixman_image_unref(frame_image);
		}
		SU_FREE(gp_alloc, gif->frames);
		SU_FREE(gp_alloc, gif);
		break;
	}
#endif /* SW_WITH_GIF */
	default:
		SU_ASSERT_UNREACHABLE;
	}
#endif /* SW_WITH_SVG || SW_WITH_GIF */

	SU_FREE(gp_alloc, image_data->pixels);
	SU_FREE(gp_alloc, image_data);
}

static pixman_image_t *sw__image_create( su_allocator_t *gp_alloc,
		int width, int height, sw__image_data_t **data_out) {
	sw__image_data_t *data;
	int stride;
	size_t size;
	pixman_image_t *image;

	SU_ASSERT(width > 0);
	SU_ASSERT(height > 0);

	SU_ALLOCT(data, gp_alloc);
#if SW_WITH_SVG || SW_WITH_GIF
	data->type = SW__IMAGE_DATA_TYPE_NONE;
#endif /* SW_WITH_SVG || SW_WITH_GIF */

	stride = (width * 4);
	size = ((size_t)height * (size_t)stride);

	/* ? TODO: preallocate (size % 32) == 0 */
	SU_ALLOCTSA(data->pixels, gp_alloc, size, 32);

	image = pixman_image_create_bits_no_clear(PIXMAN_a8r8g8b8, width, height, data->pixels, stride);
	pixman_image_set_destroy_function(image, sw__image_handle_destroy, data);

	if (data_out) {
		*data_out = data;
	} else {
		SU_MEMSET(data->pixels, 0, size);
	}
	return image;
}

static pixman_image_t *sw__load_pixmap(su_allocator_t *gp_alloc, su_fat_ptr_t data) {
	sw_pixmap_t *pixmap;
	pixman_image_t *image;
	sw__image_data_t *image_data;
	size_t pixels_count;

	if (data.len <= (sizeof(*pixmap) - sizeof(pixmap->pixels))) {
		return NULL;
	}

	pixmap = (sw_pixmap_t *)data.ptr;

	pixels_count = (pixmap->width * pixmap->height * 4);
	if ((data.len - (sizeof(*pixmap) - sizeof(pixmap->pixels))) != pixels_count) {
		return NULL;
	}

	image = sw__image_create(gp_alloc, (int)pixmap->width, (int)pixmap->height, &image_data);
	SU_MEMCPY(image_data->pixels, pixmap->pixels, pixels_count);

    return image;
}

#if SW_WITH_SVG
static pixman_image_t *sw__render_svg(su_allocator_t *gp_alloc, resvg_render_tree *tree,
		sw__image_data_t **image_data_out, int32_t target_width, int32_t target_height) {
	resvg_size image_size = resvg_get_image_size(tree); /* ? TODO: resvg_get_image_bbox, etc */
	int32_t width = (int32_t)image_size.width;
	int32_t height = (int32_t)image_size.width;
	resvg_transform transform = { 1, 0, 0, 1, 0, 0 };
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
	SU_MEMSET(image_data->pixels, 0, (size_t)width * (size_t)height * 4);

	resvg_render(tree, transform, (uint32_t)width, (uint32_t)height, (char *)image_data->pixels);

	su_abgr_to_argb(image_data->pixels, image_data->pixels, (size_t)width * (size_t)height);

	if (image_data_out) {
		*image_data_out = image_data;
	}
	return image;
}

static pixman_image_t *sw__load_svg(su_allocator_t *gp_alloc, su_fat_ptr_t data) {
	pixman_image_t *image;
	resvg_render_tree *tree = NULL;
	resvg_options *opt = resvg_options_create();
	sw__image_data_t *image_data;
	resvg_size size;
	float w, h;
	int32_t c;

	c = resvg_parse_tree_from_data((char *)data.ptr, (uintptr_t)data.len, opt, &tree);
	if (opt) {
		resvg_options_destroy(opt);
	}
    if (c != RESVG_OK) {
		goto error;
	}

	size = resvg_get_image_size(tree);
	w = size.width;
	h = size.height;
	if ((w <= 0) || (h <= 0)) {
		goto error;
	}

	image = sw__render_svg(gp_alloc, tree, &image_data, -1, -1);
	image_data->type = SW__IMAGE_DATA_TYPE_SVG;
	image_data->data = tree;

	return image;
error:
	if (tree) {
		resvg_tree_destroy(tree);
	}
	return NULL;
}
#endif /* SW_WITH_SVG */

#if SW_WITH_PNG
static pixman_image_t *sw__load_png(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data) {
	pixman_image_t *image = NULL;
	stbi__result_info ri;
	stbi__context ctx;
	SU_MEMSET(&ri, 0, sizeof(ri));
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
		SU_FREE(scratch_alloc, src);
	}

	return image;
}
#endif /* SW_WITH_PNG */

#if SW_WITH_JPG
static pixman_image_t *sw__load_jpg(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data) {
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
		SU_FREE(scratch_alloc, src);
	}

	return image;
}
#endif /* SW_WITH_JPG */

#if SW_WITH_TGA
static pixman_image_t *sw__load_tga(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data) {
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
		SU_FREE(scratch_alloc, src);
	}

	return image;
}
#endif /* SW_WITH_TGA */

#if SW_WITH_BMP
static pixman_image_t *sw__load_bmp(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data) {
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
		SU_FREE(scratch_alloc, src);
	}

	return image;
}
#endif /* SW_WITH_BMP */

#if SW_WITH_PSD
static pixman_image_t *sw__load_psd(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data) {
	pixman_image_t *image = NULL;
	stbi__result_info ri;
	stbi__context ctx;
	SU_MEMSET(&ri, 0, sizeof(ri));
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
		SU_FREE(scratch_alloc, src);
	}

	return image;
}
#endif /* SW_WITH_PSD */

#if SW_WITH_GIF
static pixman_image_t *sw__load_gif(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		sw_layout_block_image_t *data) {
	pixman_image_t *image = NULL;
	int width, height, unused, *frame_delays, frame_count;
	uint32_t *src;
	stbi__context ctx;
	stbi__start_mem(&ctx, (stbi_uc *)data->data.ptr, (int)data->data.len);

	if ((src = (uint32_t *)stbi__load_gif_main(&ctx, &frame_delays, &width, &height, &frame_count, &unused, 4))
			&& (width > 0) && (height > 0) && (frame_count > 0)) {
		if (frame_count > 1) {
			int i = 0;
			sw__image_multiframe_gif_t *gif;
			sw__image_gif_frame_t frame;

			SU_ALLOCT(gif, gp_alloc);
			gif->frames_count = (size_t)frame_count;
			SU_ARRAY_ALLOC(gif->frames, gp_alloc, gif->frames_count);

			for ( ; i < frame_count; ++i) {
				sw__image_data_t *frame_data;
				size_t size = (size_t)width * (size_t)height * 4;
				frame.image = sw__image_create(gp_alloc, width, height, &frame_data);
				frame.delay = (uint16_t)frame_delays[i];
				gif->frames[i] = frame;

				su_abgr_to_argb_premultiply_alpha(frame_data->pixels,
					(uint32_t *)(void *)&((uint8_t *)src)[size * (size_t)i],
					(size_t)width * (size_t)height);
				frame_data->type = SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF;
				frame_data->data = gif;
			}

			SU_ASSERT(data->gif_frame_idx < gif->frames_count);
			gif->frame_idx = data->gif_frame_idx;
			frame = gif->frames[gif->frame_idx];
			gif->frame_end = (data->gif_static ? INT64_MAX : (su_now_ms(CLOCK_MONOTONIC) + frame.delay));
			image = frame.image;
		} else {
			sw__image_data_t *image_data;
			image = sw__image_create(gp_alloc, width, height, &image_data);
			su_abgr_to_argb_premultiply_alpha(image_data->pixels, src, (size_t)width * (size_t)height);
		}
	}
	SU_FREE(scratch_alloc, src);
	SU_FREE(scratch_alloc, frame_delays);

	return image;
}
#endif /* SW_WITH_GIF */

#if SW_WITH_HDR
static pixman_image_t *sw__load_hdr(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data) {
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
		SU_FREE(scratch_alloc, src);
	}

	return image;
}
#endif /* SW_WITH_HDR */

#if SW_WITH_PIC
static pixman_image_t *sw__load_pic(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data) {
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
		SU_FREE(scratch_alloc, src);
	}

	return image;
}
#endif /* SW_WITH_PIC */

#if SW_WITH_PNM
static pixman_image_t *sw__load_pnm(su_allocator_t *gp_alloc, su_allocator_t *scratch_alloc,
		su_fat_ptr_t data) {
	pixman_image_t *image = NULL;
	stbi__result_info ri;
	stbi__context ctx;
	SU_MEMSET(&ri, 0, sizeof(ri));
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
		SU_FREE(scratch_alloc, src);
	}

	return image;
}
#endif /* SW_WITH_PNM */

static void sw__layout_block_fini(sw_layout_block_t *block, sw_context_t *sw) {
	/* TODO: remove recursion */

	sw__layout_block_t *block_priv = (sw__layout_block_t *)&block->sw__private;
	sw_context_t *old_context = sw__context;

	if (SU_UNLIKELY(!block_priv->valid)) {
		return;
	}

	sw__context = sw;

	if (block->in.type == SW_LAYOUT_BLOCK_TYPE_COMPOSITE) {
		sw_layout_block_t *block_;
		for ( block_ = block->in._.composite.children.head; block_; ) {
			sw_layout_block_t *next = block_->next;
			sw__layout_block_fini(block_, sw__context);
			block_->in.notify(block_, sw__context, SW_LAYOUT_BLOCK_EVENT_DESTROY);
			block_ = next;
		}
	}
#if SW_WITH_TEXT
	else if (block->in.type == SW_LAYOUT_BLOCK_TYPE_TEXT) {
		SU_FREE(sw__context->in.gp_alloc, block->out._.text.glyphs);
	}
#endif /* SW_WITH_TEXT */

	if (block_priv->content_image) {
		pixman_image_unref(block_priv->content_image);
	}

	SU_MEMSET(block_priv, 0 , sizeof(*block_priv));
	SU_MEMSET(&block->out, 0, sizeof(block->out));
	block->out.fini = sw__layout_block_fini;

	sw__context = old_context;
}

static su_bool32_t sw__layout_block_handle_event_fallback( sw_layout_block_t *block,
		sw_context_t *sw, sw_layout_block_event_t event) {
	SU_NOTUSED(block); SU_NOTUSED(sw); SU_NOTUSED(event);
	/* TODO: log warnings,errors */
	return SU_TRUE;
}

static su_bool32_t sw__layout_block_init(sw_layout_block_t *block) {
	/* TODO: remove recursion */

	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw__layout_block_t *block_priv = (sw__layout_block_t *)&block->sw__private;
	su_allocator_t *gp_alloc = sw__context->in.gp_alloc;
	su_allocator_t *scratch_alloc = sw__context->in.scratch_alloc;
	sw_layout_block_event_t error;
	sw_layout_block_type_t block_type = block->in.type;

	SU_NOTUSED(scratch_alloc);

	/* TODO: rework */
	if (block_type == SW_LAYOUT_BLOCK_TYPE_COMPOSITE) {
		block->in.type = SW_LAYOUT_BLOCK_TYPE_SPACER;
	}
	sw__layout_block_fini(block, sw__context);
	block->in.type = block_type;
	if (!block->in.notify) {
		block->in.notify = sw__layout_block_handle_event_fallback;
	}

	block_priv->valid = SU_TRUE;
	block->out.fini = sw__layout_block_fini;

	switch (block->in.type) {
	case SW_LAYOUT_BLOCK_TYPE_SPACER:
		break;
#if SW_WITH_TEXT
	case SW_LAYOUT_BLOCK_TYPE_TEXT: {
		sw_layout_block_text_t text = block->in._.text;
		const char **font_names;
		struct fcft_font *font;
		struct fcft_text_run *text_run = NULL;
		sw__text_run_cache_t *cache;
		sw__text_run_cache_entry_t entry;
		int image_width = 0, image_height;
		int x = 0, y;
		pixman_image_t *text_color;
		size_t i = 0;

		SU_ARRAY_ALLOC(font_names, scratch_alloc, (text.font_names_count + 1));
		for ( ; i < text.font_names_count; ++i) {
			su_string_t *s = &text.font_names[i];
			SU_ASSERT(s->nul_terminated == SU_TRUE); /* TODO: handle properly */
			font_names[i] = s->s;
		}
		font_names[text.font_names_count] = "monospace:size=16";

		font = fcft_from_name(text.font_names_count + 1, font_names, NULL);
		SU_FREE(scratch_alloc, font_names);
		if (font == NULL) {
			error = SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_FONT;
			goto error;
		}

		if (su_hash_table__sw__text_run_cache_t__get(&sw_priv->text_run_cache,
				text.text, &cache)) {
			size_t j = 0;
			for ( ; j < cache->items_count; ++j) {
				entry = cache->items[j];
				if (entry.font == font) {
					text_run = entry.text_run;
					break;
				}
			}
		}

		if (text_run == NULL) {
			su_c32_t *c32;
			size_t c32_count = 0;
			size_t consumed = 0;
			mbstate_t s;

			SU_MEMSET(&s, 0, sizeof(s));
			SU_ARRAY_ALLOC(c32, scratch_alloc, (text.text.len + 1));

			while (consumed < text.text.len) {
				size_t ret = mbrtoc32(&c32[c32_count++], &text.text.s[consumed],
						text.text.len - consumed, &s);
				switch (ret) {
				case 0: /* ? TODO: do not treat as error */
				case (size_t)-1:
				case (size_t)-2:
				case (size_t)-3:
					SU_FREE(scratch_alloc, c32);
					error = SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_TEXT;
					goto error;
				default:
					consumed += ret;
				}
			}

			text_run = fcft_rasterize_text_run_utf32(font, c32_count, (uint32_t *)c32, FCFT_SUBPIXEL_NONE);
			SU_FREE(scratch_alloc, c32);
			if ((text_run == NULL) || (text_run->count == 0)) {
				fcft_text_run_destroy(text_run);
				error = SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_TEXT;
				goto error;
			}

			if (su_hash_table__sw__text_run_cache_t__add(&sw_priv->text_run_cache,
					gp_alloc, text.text, &cache)) {
				su_string_init_string(&cache->key, gp_alloc, cache->key);
			}

			entry.font = font;
			entry.text_run = text_run;
			if (cache->items_count < SU_LENGTH(cache->items)) {
				cache->items[cache->items_count++] = entry;
			}
		}

		image_height = font->height;
		for ( i = 0; i < text_run->count; ++i) {
			image_width += text_run->glyphs[i]->advance.x;
		}
		if ((image_width <= 0) || (image_height <= 0)) {
			error = SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_TEXT;
			goto error;
		}

		block->out._.text.glyphs_count = text_run->count;
		SU_ARRAY_ALLOC(block->out._.text.glyphs, gp_alloc, text_run->count);

		block_priv->content_image = sw__image_create(gp_alloc, image_width, image_height, NULL);

		text_color = sw__color_to_pixman_image(text.color, scratch_alloc);
		y = font->height - font->descent;
		for ( i = 0; i < text_run->count; ++i) {
			const struct fcft_glyph *fcft_glyph = text_run->glyphs[i];
			sw_glyph_t *glyph = &block->out._.text.glyphs[i];
			glyph->x = (x + fcft_glyph->x);
			glyph->y = (y - fcft_glyph->y);
			glyph->width = fcft_glyph->width;
			glyph->height = fcft_glyph->height;
			glyph->codepoint = fcft_glyph->cp;
			glyph->cluster = text_run->cluster[i];
			if (pixman_image_get_format(fcft_glyph->pix) == PIXMAN_a8r8g8b8) {
				pixman_image_composite32(PIXMAN_OP_OVER, fcft_glyph->pix, NULL, block_priv->content_image,
						0, 0, 0, 0, glyph->x, glyph->y, glyph->width, glyph->height);
			} else {
				pixman_image_composite32(PIXMAN_OP_OVER, text_color, fcft_glyph->pix, block_priv->content_image,
						0, 0, 0, 0, glyph->x, glyph->y, glyph->width, glyph->height);
			}
			x += fcft_glyph->advance.x;
		}

		pixman_image_unref(text_color);
		break;
	}
#endif /* SW_WITH_TEXT */
	case SW_LAYOUT_BLOCK_TYPE_IMAGE: {
		sw_layout_block_image_t image = block->in._.image;
		su_fat_ptr_t data = image.data;
		sw__image_cache_t *cache;

		if (su_hash_table__sw__image_cache_t__get(&sw_priv->image_cache, data, &cache)) {
#if SW_WITH_GIF
			sw__image_data_t *d = (sw__image_data_t *)pixman_image_get_destroy_data(cache->image);
			if (d->type == SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF) {
				sw__image_multiframe_gif_t *gif = (sw__image_multiframe_gif_t *)d->data;
				SU_ASSERT(image.gif_frame_idx < gif->frames_count);
				if (image.gif_static) {
					gif->frame_end = INT64_MAX;
					gif->frame_idx = image.gif_frame_idx;
				} else if (gif->frame_end == INT64_MAX) {
					gif->frame_idx = image.gif_frame_idx;
					gif->frame_end = (su_now_ms(CLOCK_MONOTONIC) + gif->frames[gif->frame_idx].delay);
				}
				block_priv->content_image = pixman_image_ref(gif->frames[gif->frame_idx].image);
			} else
#endif /* SW_WITH_GIF */
				block_priv->content_image = pixman_image_ref(cache->image);
		} else {
			if (image.type == SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_AUTO) {
#if SW_WITH_SVG
				if ((block_priv->content_image = sw__load_svg(gp_alloc, data))) {
					goto process_content_image;
				}
#endif /* SW_WITH_SVG */
#if SW_WITH_PNG
				if ((block_priv->content_image = sw__load_png(gp_alloc, scratch_alloc, data))) {
					goto process_content_image;
				}
#endif /* SW_WITH_PNG */
#if SW_WITH_JPG
				if ((block_priv->content_image = sw__load_jpg(gp_alloc, scratch_alloc, data))) {
					goto process_content_image;
				}
#endif /* SW_WITH_JPG */
#if SW_WITH_TGA
				if ((block_priv->content_image = sw__load_tga(gp_alloc, scratch_alloc, data))) {
					goto process_content_image;
				}
#endif /* SW_WITH_TGA */
#if SW_WITH_BMP
				if ((block_priv->content_image = sw__load_bmp(gp_alloc, scratch_alloc, data))) {
					goto process_content_image;
				}
#endif /* SW_WITH_BMP */
#if SW_WITH_PSD
				if ((block_priv->content_image = sw__load_psd(gp_alloc, scratch_alloc, data))) {
					goto process_content_image;
				}
#endif /* SW_WITH_PSD */
#if SW_WITH_GIF
				if ((block_priv->content_image = sw__load_gif(gp_alloc, scratch_alloc, &image))) {
					goto process_content_image;
				}
#endif /* SW_WITH_GIF */
#if SW_WITH_HDR
				if ((block_priv->content_image = sw__load_hdr(gp_alloc, scratch_alloc, data))) {
					goto process_content_image;
				}
#endif /* SW_WITH_HDR */
#if SW_WITH_PIC
				if ((block_priv->content_image = sw__load_pic(gp_alloc, scratch_alloc, data))) {
					goto process_content_image;
				}
#endif /* SW_WITH_PIC */
#if SW_WITH_PNM
				if ((block_priv->content_image = sw__load_pnm(gp_alloc, scratch_alloc, data))) {
					goto process_content_image;
				}
#endif /* SW_WITH_PNM */
				if ((block_priv->content_image = sw__load_pixmap(gp_alloc, data))) {
					goto process_content_image;
				}
			} else {
				switch (image.type) {
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SW_PIXMAP:
					block_priv->content_image = sw__load_pixmap(gp_alloc, data);
					break;
#if SW_WITH_SVG
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SVG:
					block_priv->content_image = sw__load_svg(gp_alloc, data);
					break;
#endif /* SW_WITH_SVG */
#if SW_WITH_PNG
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNG:
					block_priv->content_image = sw__load_png(gp_alloc, scratch_alloc, data);
					break;
#endif /* SW_WITH_PNG */
#if SW_WITH_JPG
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_JPG:
					block_priv->content_image = sw__load_jpg(gp_alloc, scratch_alloc, data);
					break;
#endif /* SW_WITH_JPG */
#if SW_WITH_TGA
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_TGA:
					block_priv->content_image = sw__load_tga(gp_alloc, scratch_alloc, data);
					break;
#endif /* SW_WITH_TGA */
#if SW_WITH_BMP
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_BMP:
					block_priv->content_image = sw__load_bmp(gp_alloc, scratch_alloc, data);
					break;
#endif /* SW_WITH_BMP */
#if SW_WITH_PSD
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PSD:
					block_priv->content_image = sw__load_psd(gp_alloc, scratch_alloc, data);
					break;
#endif /* SW_WITH_PSD */
#if SW_WITH_GIF
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_GIF:
					block_priv->content_image = sw__load_gif(gp_alloc, scratch_alloc, &image);
					break;
#endif /* SW_WITH_GIF */
#if SW_WITH_HDR
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_HDR:
					block_priv->content_image = sw__load_hdr(gp_alloc, scratch_alloc, data);
					break;
#endif /* SW_WITH_HDR */
#if SW_WITH_PIC
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PIC:
					block_priv->content_image = sw__load_pic(gp_alloc, scratch_alloc, data);
					break;
#endif /* SW_WITH_PIC */
#if SW_WITH_PNM
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNM:
					block_priv->content_image = sw__load_pnm(gp_alloc, scratch_alloc, data);
					break;
#endif /* SW_WITH_PNM */
				case SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_AUTO:
				default:
					SU_ASSERT_UNREACHABLE;
				}
			}

process_content_image:
			if (!block_priv->content_image) {
				error = SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_IMAGE;
				goto error;
			}

			pixman_image_set_filter(block_priv->content_image, PIXMAN_FILTER_BEST, NULL, 0);

			su_hash_table__sw__image_cache_t__add(&sw_priv->image_cache, gp_alloc, data, &cache);

			SU_ALLOCTSA(cache->key.ptr, gp_alloc, data.len, 32);
			SU_MEMCPY(cache->key.ptr, data.ptr, data.len);
			cache->key.len = data.len;
			cache->image = pixman_image_ref(block_priv->content_image);
		}
		break;
	}
	case SW_LAYOUT_BLOCK_TYPE_COMPOSITE: {
		sw_layout_block_composite_t composite = block->in._.composite;
		sw_layout_block_t *b = composite.children.head;
		SU_ASSERT(composite.children.count > 0);
		for ( ; b; b = b->next) {
			sw__layout_block_init(b);
		}
		break;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}

	if (block_priv->content_image) {
		pixman_image_set_repeat(block_priv->content_image, (pixman_repeat_t)block->in.content_repeat);
	}

	return SU_TRUE;
error:
	sw__layout_block_fini(block, sw__context);
	block->in.notify(block, sw__context, error);
	return SU_FALSE;
}

static su_bool32_t sw__layout_block_prepare(sw_layout_block_t *, sw_layout_block_dimensions_t *overrides);

static su_bool32_t sw__layout_block_fill(sw_layout_block_t *block, int32_t available_width, int32_t available_height) {
	sw__layout_block_t *block_priv = (sw__layout_block_t *)&block->sw__private;
	uint32_t fill = block->in.fill;
	sw_layout_block_dimensions_t *dim = &block->out.dim;
	int32_t x = dim->x;
	int32_t y = dim->y;
	int32_t width = dim->width;
	int32_t height = dim->height;

	if (SU_UNLIKELY(!block_priv->valid) || (fill == SW_LAYOUT_BLOCK_FILL_NONE)) {
		return SU_TRUE;
	}

	if (fill & SW_LAYOUT_BLOCK_FILL_LEFT) {
		width += x;
		x = 0;
	}

	if (fill & SW_LAYOUT_BLOCK_FILL_RIGHT) {
		width = (available_width - x);
	}

	if (fill & SW_LAYOUT_BLOCK_FILL_TOP) {
		height += y;
		y = 0;
	}

	if (fill & SW_LAYOUT_BLOCK_FILL_BOTTOM) {
		height = (available_height - y);
	}

	if (fill & SW_LAYOUT_BLOCK_FILL_CONTENT) {
		sw_layout_block_dimensions_t dims;
		SU_MEMCPY(dims.borders, dim->borders, sizeof(dim->borders));
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

	sw__layout_block_t *block_priv = (sw__layout_block_t *)&block->sw__private;
	sw_layout_block_dimensions_t *dim;

	if (SU_UNLIKELY(!block_priv->valid)) {
		return SU_TRUE;
	}

	/* ? TODO: overrides */
	if (!block->in.notify(block, sw__context, SW_LAYOUT_BLOCK_EVENT_PREPARE)) {
		return SU_FALSE;
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
			sw__layout_block_t *b_priv = (sw__layout_block_t *)&b->sw__private;
			if (SU_UNLIKELY(!b_priv->valid)) {
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
			sw__layout_block_t *b_priv = (sw__layout_block_t *)&b->sw__private;
			if (SU_UNLIKELY(!b_priv->valid)) {
				continue;
			}
			dim = &b->out.dim;
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
			if (!sw__layout_block_fill(b, content_width, content_height)) {
				return SU_FALSE;
			}
		}

		if (!block_priv->content_image || (content_width != pixman_image_get_width(block_priv->content_image))
				|| (content_height != pixman_image_get_height(block_priv->content_image))) {
			if (block_priv->content_image) {
				pixman_image_unref(block_priv->content_image);
			}
			block_priv->content_image = (((content_width > 0) && (content_height > 0))
				? sw__image_create(sw__context->in.gp_alloc, content_width, content_height, NULL)
				: NULL);
		} else {
			SU_MEMSET(pixman_image_get_data(block_priv->content_image), 0, (size_t)content_width * 4 * (size_t)content_height);
		}
	}

	dim = &block->out.dim;
	if (overrides) {
		*dim = *overrides;
	} else {
		int32_t content_width = (((block->in.content_width <= 0) && block_priv->content_image)
			? pixman_image_get_width(block_priv->content_image) : block->in.content_width);
		int32_t content_height = (((block->in.content_height <= 0) && block_priv->content_image)
			? pixman_image_get_height(block_priv->content_image) : block->in.content_height);
		int32_t min_width = block->in.min_width;
		int32_t max_width = block->in.max_width;
		int32_t min_height = block->in.min_height;
		int32_t max_height = block->in.max_height;
		int32_t border_left = block->in.borders[0].width;
		int32_t border_right = block->in.borders[1].width;
		int32_t border_bottom = block->in.borders[2].width;
		int32_t border_top = block->in.borders[3].width;
		int32_t width, height;


		if (block_priv->content_image && block->in.content_transform && ((block->in.content_transform % 2) == 0)) {
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

#if SW_WITH_GIF
	if (block_priv->content_image) {
		sw__image_data_t *image_data = (sw__image_data_t *)pixman_image_get_destroy_data(block_priv->content_image);
		if (image_data->type == SW__IMAGE_DATA_TYPE_MULTIFRAME_GIF) {
			int64_t now_msec = su_now_ms(CLOCK_MONOTONIC);
			sw__image_multiframe_gif_t *gif = (sw__image_multiframe_gif_t *)image_data->data;
			if (now_msec >= gif->frame_end) {
				sw__image_gif_frame_t frame;
				if (++gif->frame_idx >= gif->frames_count) {
					gif->frame_idx = 0;
				}
				frame = gif->frames[gif->frame_idx];
				pixman_image_unref(block_priv->content_image);
				block_priv->content_image = pixman_image_ref(frame.image);
				gif->frame_end = (now_msec + frame.delay);
			}
			sw__update_t(gif->frame_end);
			block->out._.gif.frame_idx = gif->frame_idx;
		}
	}
#endif /* SW_WITH_GIF */

	return block->in.notify(block, sw__context, SW_LAYOUT_BLOCK_EVENT_PREPARED);
}

static void sw__layout_block_render(sw_layout_block_t *block, pixman_image_t *dest) {
	/* TODO: remove recursion */

	sw__layout_block_t *block_priv = (sw__layout_block_t *)&block->sw__private;
	sw_layout_block_dimensions_t dim = block->out.dim;
	su_allocator_t *scratch_alloc = sw__context->in.scratch_alloc;
	pixman_image_t *color;

	if (SU_UNLIKELY(!block_priv->valid)) {
		return;
	}

	if (block->in.type == SW_LAYOUT_BLOCK_TYPE_COMPOSITE) {
		sw_layout_block_t *b = block->in._.composite.children.head;
		for ( ; b; b = b->next) {
			sw__layout_block_render(b, block_priv->content_image);
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

	if (block_priv->content_image) {
		int content_image_width = pixman_image_get_width(block_priv->content_image);
		int content_image_height = pixman_image_get_height(block_priv->content_image);
		pixman_transform_t transform;
		pixman_region32_t clip_region;
		int32_t available_width = (dim.width - dim.borders[0] - dim.borders[1]);
		int32_t available_height = (dim.height - dim.borders[2] - dim.borders[3]);
		int32_t content_x = (dim.x + dim.borders[0]);
		int32_t content_y = (dim.y + dim.borders[3]);

		if (block->in.content_transform && ((block->in.content_transform % 2) == 0)) {
			int tmp = content_image_width;
			content_image_width = content_image_height;
			content_image_height = tmp;
		}

		pixman_transform_init_identity(&transform);

		if ((block->in.content_repeat == SW_LAYOUT_BLOCK_CONTENT_REPEAT_NONE) &&
				((dim.content_width != content_image_width) ||
				(dim.content_height != content_image_height))) {
#if SW_WITH_SVG
			sw__image_data_t *image_data = (sw__image_data_t *)pixman_image_get_destroy_data(
				block_priv->content_image);
			if (image_data->type == SW__IMAGE_DATA_TYPE_SVG) {
				resvg_render_tree *render_tree = (resvg_render_tree *)image_data->data;
				pixman_image_unref(block_priv->content_image);
				block_priv->content_image = sw__render_svg(sw__context->in.gp_alloc,
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
				pixman_int_to_fixed(pixman_image_get_width(block_priv->content_image)), 0);
			break;
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_180:
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_180:
			pixman_transform_rotate(&transform, NULL, pixman_fixed_minus_1, 0);
			pixman_transform_translate(&transform, NULL,
				pixman_int_to_fixed(pixman_image_get_width(block_priv->content_image)),
				pixman_int_to_fixed(pixman_image_get_height(block_priv->content_image)));
			break;
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_270:
		case SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED_270:
			pixman_transform_rotate(&transform, NULL, 0, pixman_fixed_minus_1);
			pixman_transform_translate(&transform, NULL, 0,
				pixman_int_to_fixed(pixman_image_get_height(block_priv->content_image)));
			break;
		default:
			SU_ASSERT_UNREACHABLE;
		}

		if (block->in.content_transform >= SW_LAYOUT_BLOCK_CONTENT_TRANSFORM_FLIPPED) {
			pixman_transform_translate(&transform, NULL,
				-pixman_int_to_fixed(pixman_image_get_width(block_priv->content_image)), 0);
			pixman_transform_scale(&transform, NULL, pixman_fixed_minus_1, pixman_fixed_1);
		}

		pixman_image_set_transform(block_priv->content_image, &transform);

		pixman_region32_init_rect(&clip_region, dim.x, dim.y, (unsigned int)dim.width, (unsigned int)dim.height);
		pixman_image_set_clip_region32(dest, &clip_region);

		/* TODO: move to sw__layout_block_prepare */
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

		pixman_image_composite32(PIXMAN_OP_OVER, block_priv->content_image, NULL, dest,
			0, 0, 0, 0, content_x, content_y, dim.content_width, dim.content_height);

		pixman_image_set_transform(block_priv->content_image, NULL);
		pixman_image_set_clip_region32(dest, NULL);
	}
}

#if SW_WITH_WAYLAND_BACKEND
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
	/* TODO: remove recursion */

	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw_context_t *old_context = sw__context;
	sw_wayland_seat_t *seat;
	sw_wayland_surface_t *p;

	if (!surface_priv->wl_surface) {
		return;
	}

	sw__context = sw;

	for ( p = surface->in.popups.head; p; ) {
		sw_wayland_surface_t *next = p->next;
		sw__wayland_surface_fini(p, sw__context);
		p->in.notify((sw_wayland_event_source_t *)p, sw__context, SW_WAYLAND_EVENT_SURFACE_CLOSE);
		p = next;
	}

	switch (surface->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_TOPLEVEL: {
		sw__wayland_surface_toplevel_t *toplevel_priv = &surface_priv->_.toplevel;
		if (toplevel_priv->decoration) {
			zxdg_toplevel_decoration_v1_destroy(toplevel_priv->decoration);
		}
		if (toplevel_priv->xdg_toplevel) {
			xdg_toplevel_destroy(toplevel_priv->xdg_toplevel);
		}
		if (toplevel_priv->xdg_surface) {
			xdg_surface_destroy(toplevel_priv->xdg_surface);
		}
		su_string_fini(&toplevel_priv->app_id, sw__context->in.gp_alloc);
		su_string_fini(&toplevel_priv->title, sw__context->in.gp_alloc);
		break;
	}
	case SW_WAYLAND_SURFACE_TYPE_LAYER: {
		sw__wayland_surface_layer_t *layer_priv = &surface_priv->_.layer;
		if (layer_priv->layer_surface) {
			zwlr_layer_surface_v1_destroy(layer_priv->layer_surface);
		}
		break;
	}
	case SW_WAYLAND_SURFACE_TYPE_POPUP: {
		sw__wayland_surface_popup_t *popup_priv = &surface_priv->_.popup;
		if (popup_priv->xdg_positioner) {
			xdg_positioner_destroy(popup_priv->xdg_positioner);
		}
		if (popup_priv->xdg_popup) {
			xdg_popup_destroy(popup_priv->xdg_popup);
		}
		if (popup_priv->xdg_surface) {
			xdg_surface_destroy(popup_priv->xdg_surface);
		}
		break;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}
	sw__wayland_surface_buffer_fini(&surface_priv->buffer);
	wl_surface_destroy(surface_priv->wl_surface);

	sw__layout_block_fini(surface->in.root, sw__context);
	surface->in.root->in.notify(surface->in.root, sw__context, SW_LAYOUT_BLOCK_EVENT_DESTROY);

	SU_FREE(sw->in.gp_alloc, surface_priv->input_regions);

	for ( seat = sw->out.backend.wayland.seats.head; seat; seat = seat->next) {
		if (seat->out.pointer && (seat->out.pointer->out.focused_surface == surface)) {
			seat->out.pointer->out.focused_surface = NULL;
		}
	}

	SU_MEMSET(surface_priv, 0, sizeof(*surface_priv));
	SU_MEMSET(&surface->out, 0, sizeof(surface->out));
	surface->out.fini = sw__wayland_surface_fini;

	sw__context = old_context;
}

static void sw__wayland_surface_popup_init_stage2(sw_wayland_surface_t *);
static su_bool32_t sw__wayland_surface_buffer_init(sw__wayland_surface_buffer_t *,
		sw_wayland_surface_t *, int32_t width, int32_t height);

static su_bool32_t sw__wayland_surface_render(sw_wayland_surface_t *surface) {
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	int32_t surface_width, surface_height;
	sw_layout_block_t *root = surface->in.root;

	if (surface_priv->buffer.busy) {
		surface_priv->dirty = SU_TRUE;
		return SU_TRUE;
	}

	if (SU_UNLIKELY(!surface_priv->wl_surface) || !sw__layout_block_prepare(root, NULL)) {
		return SU_FALSE;
	}
	if ((root->out.dim.width <= 0) || (root->out.dim.height <= 0)) {
		surface->in.notify( (sw_wayland_event_source_t *)surface,
			sw__context, SW_WAYLAND_EVENT_SURFACE_LAYOUT_FAILED);
		return SU_FALSE;
	}

	switch (surface->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_TOPLEVEL:
		if (SU_UNLIKELY(!surface_priv->buffer.wl_buffer)) {
			goto commit;
		}
		surface_width = surface->out.width;
		surface_height = surface->out.height;
		if (!(surface->out._.toplevel.states & SW_WAYLAND_SURFACE_TOPLEVEL_STATE_MAXIMIZED)) {
			if (surface->in.width > 0) {
				surface_width = surface->in.width;
			} else if ((surface->in.width < 0) || (surface_width == 0)) {
				surface_width = root->out.dim.width;
			}
			if (surface->in.height > 0) {
				surface_height = surface->in.height;
			} else if ((surface->in.height < 0) || (surface_height == 0)) {
				surface_height = root->out.dim.height;
			}
			if ((surface_width != surface->out.width) || (surface_height != surface->out.height)) {
				sw__wayland_surface_buffer_fini(&surface_priv->buffer);
				if (SU_UNLIKELY(!sw__wayland_surface_buffer_init(&surface_priv->buffer,
						surface, surface_width, surface_height))) {
					return SU_FALSE;
				}
				surface->out.width = surface_width;
				surface->out.height = surface_height;
			}
		}
		/* ? TODO: handle SW_WAYLAND_SURFACE_TOPLEVEL_STATE_FULLSCREEN | SW_WAYLAND_SURFACE_TOPLEVEL_STATE_RESIZING */
		break;
	case SW_WAYLAND_SURFACE_TYPE_LAYER: {
		sw__wayland_surface_layer_t *layer_priv = &surface_priv->_.layer;
		int32_t exclusive_zone = surface->in._.layer.exclusive_zone;
		int32_t w = ((surface->in.width < 0) ? root->out.dim.width : surface->in.width);
		int32_t h = ((surface->in.height < 0) ? root->out.dim.height : surface->in.height);
		if (!surface_priv->buffer.wl_buffer ||
				((w != 0) && (surface->out.width != w)) ||
				((h != 0) && (surface->out.height != h))) {
			zwlr_layer_surface_v1_set_size(layer_priv->layer_surface,
				(uint32_t)(w / surface->out.scale),
				(uint32_t)(h / surface->out.scale));
			goto commit;
		}
		surface_width = surface->out.width;
		surface_height = surface->out.height;
		if (exclusive_zone < -1) {
			switch (layer_priv->anchor) {
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
		if (layer_priv->exclusive_zone != exclusive_zone) {
			zwlr_layer_surface_v1_set_exclusive_zone(layer_priv->layer_surface,
				exclusive_zone / surface->out.scale);
			layer_priv->exclusive_zone = exclusive_zone;
		}
		break;
	}
	case SW_WAYLAND_SURFACE_TYPE_POPUP: {
		sw__wayland_surface_popup_t *popup_priv = &surface_priv->_.popup;
		surface_width = (surface->in.width <= 0) ? root->out.dim.width : surface->in.width;
		surface_height = (surface->in.height <= 0) ? root->out.dim.height : surface->in.height;
		if ((surface->out.width != surface_width) || (surface->out.height != surface_height)) {
			xdg_positioner_set_size(popup_priv->xdg_positioner,
				surface_width / surface->out.scale, surface_height / surface->out.scale);
			if (SU_UNLIKELY(!popup_priv->xdg_popup)) {
				sw__wayland_surface_popup_init_stage2(surface);
			} else {
				xdg_popup_reposition(popup_priv->xdg_popup, popup_priv->xdg_positioner, 0);
			}
			goto commit;
		}
		break;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}

	SU_ASSERT(surface_width > 0);
	SU_ASSERT(surface_height > 0);

	if (!sw__layout_block_fill(root, surface_width, surface_height)) {
		return SU_FALSE;
	}

	SU_MEMSET( surface_priv->buffer.pixels, 0, surface_priv->buffer.size);
	sw__layout_block_render(root, surface_priv->buffer.image);

	/* TODO: wl_surface_set_opaque_region, wl_surface_set_buffer_transform */
	wl_surface_set_buffer_scale(surface_priv->wl_surface, surface->out.scale);
	wl_surface_attach(surface_priv->wl_surface, surface_priv->buffer.wl_buffer, 0, 0);
	wl_surface_damage_buffer(surface_priv->wl_surface, 0, 0, surface->out.width, surface->out.height);

	surface_priv->buffer.busy = SU_TRUE;
	surface_priv->dirty = SU_FALSE;
commit:
	wl_surface_commit(surface_priv->wl_surface);
	return SU_TRUE;
}

static void sw__wayland_surface_buffer_handle_release(void *data, struct wl_buffer *wl_buffer) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;

	SU_NOTUSED(wl_buffer);

	surface_priv->buffer.busy = SU_FALSE;
	if (surface_priv->dirty) {
		sw__wayland_surface_render(surface);
	}
}

static su_bool32_t sw__wayland_surface_buffer_init(sw__wayland_surface_buffer_t *buffer,
		sw_wayland_surface_t *surface, int32_t width, int32_t height) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	struct timespec ts;
	pid_t pid = getpid();
	char shm_name[NAME_MAX];
	int shm_fd, c;
	int32_t stride = (width * 4);
	struct wl_shm_pool *wl_shm_pool;

	static struct wl_buffer_listener wl_buffer_listener = { sw__wayland_surface_buffer_handle_release };

	SU_ASSERT(width > 0);
	SU_ASSERT(height > 0);

	SU_MEMSET(buffer, 0, sizeof(*buffer));

/* TODO: limit retry count */
generate_shm_name:
	c = clock_gettime(CLOCK_MONOTONIC, &ts);
	SU_NOTUSED(c);
	SU_ASSERT(c == 0);
	su_snprintf(shm_name, sizeof(shm_name),"/sw-%d-%ld-%ld", pid, ts.tv_sec, ts.tv_nsec);

	shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0600);
	if (SU_UNLIKELY(shm_fd == -1)) {
		if (errno == EEXIST) {
			goto generate_shm_name;
		} else {
			goto error;
		}
	}
	shm_unlink(shm_name);

	buffer->size = ((uint32_t)stride * (uint32_t)height);
	while (SU_UNLIKELY(ftruncate(shm_fd, buffer->size) == -1)) {
		if (errno == EINTR) {
			continue;
		} else {
			goto error;
		}
	}

	buffer->pixels = (uint32_t *)mmap( NULL,
			buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (SU_UNLIKELY(buffer->pixels == MAP_FAILED)) {
		goto error;
	}

	buffer->image = pixman_image_create_bits_no_clear(
		PIXMAN_a8r8g8b8, width, height, buffer->pixels, stride);

	wl_shm_pool = wl_shm_create_pool(sw_priv->_.wayland.shm, shm_fd, (int32_t)buffer->size);
	buffer->wl_buffer = wl_shm_pool_create_buffer(
		wl_shm_pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_buffer_add_listener(buffer->wl_buffer, &wl_buffer_listener, surface);
	wl_shm_pool_destroy(wl_shm_pool);
	close(shm_fd);

	buffer->busy = SU_FALSE;

	return SU_TRUE;
error:
	sw__wayland_surface_fini(surface, sw__context);
	surface->in.notify( (sw_wayland_event_source_t *)surface,
		sw__context, SW_WAYLAND_EVENT_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER);
	return SU_FALSE;
}

static void sw__wayland_surface_handle_enter(void *data, struct wl_surface *wl_surface, struct wl_output *output) {
	SU_NOTUSED(data); SU_NOTUSED(wl_surface); SU_NOTUSED(output);
	/* TODO: set in surface->out */
}

static void sw__wayland_surface_handle_leave(void *data, struct wl_surface *wl_surface, struct wl_output *output) {
	SU_NOTUSED(data); SU_NOTUSED(wl_surface); SU_NOTUSED(output);
	/* TODO: set in surface->out */
}

static void sw__wayland_surface_handle_preferred_buffer_transform(void *data,
		struct wl_surface *wl_surface, uint32_t transform) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;

	SU_NOTUSED(wl_surface);

	surface->out.transform = (sw_wayland_output_transform_t)transform;
}

static void sw__wayland_surface_layer_handle_preferred_buffer_scale(void *data,
		struct wl_surface *wl_surface, int32_t factor) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_layer_t *layer_priv = &surface_priv->_.layer;

	SU_NOTUSED(wl_surface);

	if (surface->out.scale != factor) {
		zwlr_layer_surface_v1_set_size(layer_priv->layer_surface,
			((surface->in.width == 0) ? 0 : (uint32_t)(surface->out.width / factor)),
			((surface->in.height == 0) ? 0 : (uint32_t)(surface->out.height / factor)));
		if (layer_priv->exclusive_zone > 0) {
			zwlr_layer_surface_v1_set_exclusive_zone(layer_priv->layer_surface,
				(layer_priv->exclusive_zone / factor));
		}
		zwlr_layer_surface_v1_set_margin(layer_priv->layer_surface,
			(layer_priv->margins[0] / factor),
			(layer_priv->margins[1] / factor),
			(layer_priv->margins[2] / factor),
			(layer_priv->margins[3] / factor));
		wl_surface_commit(surface_priv->wl_surface);
		surface->out.scale = factor;
		surface_priv->dirty = SU_TRUE;
	}
}

static void sw__wayland_surface_layer_handle_configure(void *data, struct zwlr_layer_surface_v1 *layer_surface,
		uint32_t serial, uint32_t width_, uint32_t height_) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	int32_t width = (int32_t)width_;
	int32_t height = (int32_t)height_;

	SU_NOTUSED(layer_surface);

	zwlr_layer_surface_v1_ack_configure(surface_priv->_.layer.layer_surface, serial);

	if (width <= 0) {
		width = ((surface->in.width > 0) ? surface->in.width : surface->in.root->out.dim.width);
	}
	if (height <= 0) {
		height = ((surface->in.height > 0) ? surface->in.height : surface->in.root->out.dim.height);
	}

	width *= surface->out.scale;
	height *= surface->out.scale;

	SU_ASSERT(width > 0);
	SU_ASSERT(height > 0);

	if (((surface->out.height != height) || (surface->out.width != width))) {
		sw__wayland_surface_buffer_fini(&surface_priv->buffer);
		if (SU_LIKELY(sw__wayland_surface_buffer_init(&surface_priv->buffer, surface, width, height))) {
			surface->out.width = width;
			surface->out.height = height;
			surface_priv->dirty = SU_TRUE;
		}
	}

	if (surface_priv->dirty) {
		sw__wayland_surface_render(surface);
	}
}

static void sw__wayland_surface_layer_handle_closed(void *data,
		struct zwlr_layer_surface_v1 *layer_surface) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	SU_NOTUSED(layer_surface);
	sw__wayland_surface_fini(surface, sw__context);
	surface->in.notify((sw_wayland_event_source_t *)surface, sw__context, SW_WAYLAND_EVENT_SURFACE_CLOSE);
}

static void sw__wayland_surface_popup_handle_preferred_buffer_scale(void *data,
		struct wl_surface *wl_surface, int32_t factor) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_popup_t *popup_priv = &surface_priv->_.popup;

	SU_NOTUSED(wl_surface);

	if (surface->out.scale != factor) {
		surface->out.scale = factor;
		surface_priv->dirty = SU_TRUE;

		xdg_positioner_set_size( popup_priv->xdg_positioner,
			surface->out.width / factor, surface->out.height / factor);
		xdg_positioner_set_anchor_rect(popup_priv->xdg_positioner,
			popup_priv->x / factor, popup_priv->y / factor, 1, 1);
		xdg_popup_reposition(popup_priv->xdg_popup, popup_priv->xdg_positioner, 0);

		wl_surface_commit(surface_priv->wl_surface);
	}
}

static void sw__wayland_xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface,
		uint32_t serial) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;

	SU_NOTUSED(xdg_surface);

	xdg_surface_ack_configure(surface_priv->_.toplevel.xdg_surface, serial);

	if (surface_priv->dirty) {
		sw__wayland_surface_render(surface);
	}
}

static void sw__wayland_surface_toplevel_handle_preferred_buffer_scale(void *data,
		struct wl_surface *wl_surface, int32_t factor) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	SU_NOTUSED(wl_surface);
	surface->out.scale = factor;
}

static void sw__wayland_surface_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel,
		int32_t width, int32_t height, struct wl_array *states) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	uint32_t *state;

	SU_NOTUSED(xdg_toplevel);

	surface->out._.toplevel.states = 0;
	for (state = (uint32_t *)states->data;
			(uint8_t *)state < ((uint8_t *)states->data + states->size);
			++state) {
		surface->out._.toplevel.states |= (1 << *state);
	}
	
	if (!(surface->out._.toplevel.states & SW_WAYLAND_SURFACE_TOPLEVEL_STATE_MAXIMIZED)) {
		if (surface->in.width > 0) {
			width = surface->in.width;
		} else if ((surface->in.width < 0) || (width <= 0)) {
			width = surface->in.root->out.dim.width;
		}
		if (surface->in.height > 0) {
			height = surface->in.height;
		} else if ((surface->in.height < 0) || (height <= 0)) {
			height = surface->in.root->out.dim.height;
		}
	}

	/* ? TODO: handle SW_WAYLAND_SURFACE_TOPLEVEL_STATE_FULLSCREEN | SW_WAYLAND_SURFACE_TOPLEVEL_STATE_RESIZING */

	width *= surface->out.scale;
	height *= surface->out.scale;

	SU_ASSERT(width > 0);
	SU_ASSERT(height > 0);

	if ((surface->out.width != width) || (surface->out.height != height)) {
		sw__wayland_surface_buffer_fini(&surface_priv->buffer);
		if (SU_LIKELY(sw__wayland_surface_buffer_init(&surface_priv->buffer, surface, width, height))) {
			surface->out.width = width;
			surface->out.height = height;
			surface_priv->dirty = SU_TRUE;
		}
	}
}

static void sw__wayland_surface_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	SU_NOTUSED(xdg_toplevel);
	surface->in.notify((sw_wayland_event_source_t *)surface, sw__context, SW_WAYLAND_EVENT_SURFACE_CLOSE);
}

static void sw__wayland_surface_toplevel_handle_decoration_configure( void *data,
		struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1, uint32_t mode) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_toplevel_t *toplevel_priv = (sw__wayland_surface_toplevel_t *)&surface->sw__private;

	SU_NOTUSED(zxdg_toplevel_decoration_v1);

	if (SU_UNLIKELY((mode != toplevel_priv->decoration_mode) &&
			(toplevel_priv->decoration_mode != SW_WAYLAND_TOPLEVEL_DECORATION_MODE_COMPOSITOR_DEFAULT))) {
		surface->in.notify( (sw_wayland_event_source_t *)surface,
			sw__context, SW_WAYLAND_EVENT_SURFACE_FAILED_TO_SET_DECORATIONS);
	}
}

static su_bool32_t sw__wayland_surface_toplevel_init(sw_wayland_surface_t *surface) {
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_toplevel_t *toplevel_priv = &surface_priv->_.toplevel;
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	
	static struct wl_surface_listener wl_surface_listener = {
		sw__wayland_surface_handle_enter,
		sw__wayland_surface_handle_leave,
		sw__wayland_surface_toplevel_handle_preferred_buffer_scale,
		sw__wayland_surface_handle_preferred_buffer_transform
	};
	static struct xdg_surface_listener xdg_surface_listener = {
		sw__wayland_xdg_surface_handle_configure
	};
	static struct xdg_toplevel_listener xdg_toplevel_listener = {
		sw__wayland_surface_toplevel_handle_configure,
		sw__wayland_surface_toplevel_handle_close,
		NULL,
		NULL
	};
	static struct zxdg_toplevel_decoration_v1_listener decoration_listener = {
		sw__wayland_surface_toplevel_handle_decoration_configure
	};

	if (SU_UNLIKELY(!sw_priv->_.wayland.wm_base)) {
		surface->in.notify((sw_wayland_event_source_t *)surface,
			sw__context, SW_WAYLAND_EVENT_SURFACE_ERROR_MISSING_PROTOCOL);
		return SU_FALSE;
	}

	surface->out.scale = 1;
	surface->out.fini = sw__wayland_surface_fini;
	
	surface_priv->wl_surface = wl_compositor_create_surface(sw_priv->_.wayland.compositor);
	wl_surface_add_listener(surface_priv->wl_surface, &wl_surface_listener, surface);

	toplevel_priv->xdg_surface = xdg_wm_base_get_xdg_surface(
		sw_priv->_.wayland.wm_base, surface_priv->wl_surface);
	xdg_surface_add_listener(toplevel_priv->xdg_surface, &xdg_surface_listener, surface);

	toplevel_priv->xdg_toplevel = xdg_surface_get_toplevel(toplevel_priv->xdg_surface);
	xdg_toplevel_add_listener(toplevel_priv->xdg_toplevel, &xdg_toplevel_listener, surface);

	if (sw_priv->_.wayland.decoration_manager) {
		toplevel_priv->decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
			sw_priv->_.wayland.decoration_manager, toplevel_priv->xdg_toplevel);
		zxdg_toplevel_decoration_v1_add_listener(
			toplevel_priv->decoration, &decoration_listener, surface);
	}

	return SU_TRUE;
}

static su_bool32_t sw__wayland_surface_layer_init(sw_wayland_surface_t *surface, sw_wayland_surface_layer_layer_t l) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_output_t *output_priv = (sw__wayland_output_t *)&surface->in._.layer.output->sw__private;
	sw__wayland_surface_layer_t *layer_priv = &surface_priv->_.layer;

	static struct wl_surface_listener wl_surface_listener = {
		sw__wayland_surface_handle_enter,
		sw__wayland_surface_handle_leave,
		sw__wayland_surface_layer_handle_preferred_buffer_scale,
		sw__wayland_surface_handle_preferred_buffer_transform
	};
	static struct zwlr_layer_surface_v1_listener layer_surface_listener = {
		sw__wayland_surface_layer_handle_configure,
		sw__wayland_surface_layer_handle_closed
	};

	if (SU_UNLIKELY(!sw_priv->_.wayland.layer_shell)) {
		surface->in.notify((sw_wayland_event_source_t *)surface,
			sw__context, SW_WAYLAND_EVENT_SURFACE_ERROR_MISSING_PROTOCOL);
		return SU_FALSE;
	}

	surface->out.fini = sw__wayland_surface_fini;
	surface->out.scale = surface->in._.layer.output->out.scale;
	layer_priv->exclusive_zone = INT32_MIN;
	layer_priv->anchor = UINT32_MAX;
	layer_priv->layer = l;
	layer_priv->margins[0] = INT32_MIN;
	layer_priv->margins[1] = INT32_MIN;
	layer_priv->margins[2] = INT32_MIN;
	layer_priv->margins[3] = INT32_MIN;
	layer_priv->output = surface->in._.layer.output;

	surface_priv->wl_surface = wl_compositor_create_surface(sw_priv->_.wayland.compositor);
	wl_surface_add_listener(surface_priv->wl_surface, &wl_surface_listener, surface);
	layer_priv->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
		sw_priv->_.wayland.layer_shell, surface_priv->wl_surface,
		output_priv->wl_output, l, "sw");
	zwlr_layer_surface_v1_add_listener(layer_priv->layer_surface, &layer_surface_listener, surface);

	return SU_TRUE;
}

static su_bool32_t sw__wayland_surface_popup_init_stage1( sw_wayland_surface_t *surface, sw_wayland_surface_t *parent) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_popup_t *popup_priv = &surface_priv->_.popup;

	static struct wl_surface_listener wl_surface_listener = {
		sw__wayland_surface_handle_enter,
		sw__wayland_surface_handle_leave,
		sw__wayland_surface_popup_handle_preferred_buffer_scale,
		sw__wayland_surface_handle_preferred_buffer_transform,
	};
	static struct xdg_surface_listener xdg_surface_listener = {
		sw__wayland_xdg_surface_handle_configure
	};

	SU_ASSERT(parent != NULL);

	if (!sw_priv->_.wayland.wm_base) {
		surface->in.notify((sw_wayland_event_source_t *)surface,
			sw__context, SW_WAYLAND_EVENT_SURFACE_ERROR_MISSING_PROTOCOL);
		return SU_FALSE;
	}

	popup_priv->parent = parent;
	surface->out.fini = sw__wayland_surface_fini;
	surface->out.scale = 1;
	popup_priv->x = INT32_MIN;
	popup_priv->y = INT32_MIN;
	popup_priv->gravity = (sw_wayland_surface_popup_gravity_t)UINT32_MAX;
	popup_priv->constraint_adjustment = UINT32_MAX;

	surface_priv->wl_surface = wl_compositor_create_surface(sw_priv->_.wayland.compositor);
	wl_surface_add_listener(surface_priv->wl_surface, &wl_surface_listener, surface);

	popup_priv->xdg_surface = xdg_wm_base_get_xdg_surface(
		sw_priv->_.wayland.wm_base, surface_priv->wl_surface);
	xdg_surface_add_listener(popup_priv->xdg_surface, &xdg_surface_listener, surface);

	popup_priv->xdg_positioner = xdg_wm_base_create_positioner(sw_priv->_.wayland.wm_base);

	return SU_TRUE;
}

static void sw__wayland_surface_popup_handle_configure(void *data, struct xdg_popup *xdg_popup,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;

	SU_NOTUSED(xdg_popup); SU_NOTUSED(x); SU_NOTUSED(y);

	width *= surface->out.scale;
	height *= surface->out.scale;

	SU_ASSERT(width > 0);
	SU_ASSERT(height > 0);

	if ((surface->out.width != width) || (surface->out.height != height)) {
		sw__wayland_surface_buffer_fini(&surface_priv->buffer);
		if (SU_LIKELY(sw__wayland_surface_buffer_init(&surface_priv->buffer, surface, width, height))) {
			surface->out.width = width;
			surface->out.height = height;
			surface_priv->dirty = SU_TRUE;
		}
	}
}

static void sw__wayland_surface_popup_handle_done(void *data, struct xdg_popup *xdg_popup) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	SU_NOTUSED(xdg_popup);
	sw__wayland_surface_fini(surface, sw__context);
	surface->in.notify((sw_wayland_event_source_t *)surface,
		sw__context, SW_WAYLAND_EVENT_SURFACE_CLOSE);
}

static void sw__wayland_surface_popup_handle_repositioned(void *data, struct xdg_popup *xdg_popup,
		uint32_t token) {
	SU_NOTUSED(data); SU_NOTUSED(xdg_popup); SU_NOTUSED(token);
}

static su_bool32_t sw__wayland_surface_handle_event_fallback(sw_wayland_event_source_t *source,
		sw_context_t *sw, sw_wayland_event_t event) {
	SU_NOTUSED(source); SU_NOTUSED(sw); SU_NOTUSED(event);
	/* TODO: log warnings,errors */
	return SU_TRUE;
}

static void sw__wayland_surface_prepare(sw_wayland_surface_t *surface, sw_wayland_surface_t *parent) {
	/* TODO: remove recursion */
	
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw_wayland_cursor_shape_t cursor_shape = ((surface->in.cursor_shape == SW_WAYLAND_CURSOR_SHAPE_DEFAULT)
		? SW_WAYLAND_CURSOR_SHAPE_DEFAULT_ : surface->in.cursor_shape);
	su_allocator_t *gp_alloc = sw__context->in.gp_alloc;

	SU_ASSERT(surface->in.root != NULL);

	if (!surface->in.notify) {
		surface->in.notify = sw__wayland_surface_handle_event_fallback;
	}

	if (SU_UNLIKELY(!sw__layout_block_init(surface->in.root))) {
		surface->in.notify((sw_wayland_event_source_t *)surface,
			sw__context, SW_WAYLAND_EVENT_SURFACE_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK);
		return;
	}

	switch (surface->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_TOPLEVEL: {
		sw_wayland_surface_toplevel_t *toplevel = &surface->in._.toplevel;
		sw__wayland_surface_toplevel_t *toplevel_priv = &surface_priv->_.toplevel;

		if (SU_UNLIKELY(!surface_priv->wl_surface && !sw__wayland_surface_toplevel_init(surface))) {
			return;
		}

		if (toplevel_priv->decoration_mode != toplevel->decoration_mode) {
			if (SU_LIKELY(toplevel_priv->decoration)) {
				if (toplevel->decoration_mode == SW_WAYLAND_TOPLEVEL_DECORATION_MODE_COMPOSITOR_DEFAULT) {
					zxdg_toplevel_decoration_v1_unset_mode(toplevel_priv->decoration);
				} else {
					zxdg_toplevel_decoration_v1_set_mode(toplevel_priv->decoration, toplevel->decoration_mode);
				}
			} else {
				surface->in.notify((sw_wayland_event_source_t *)surface,
					sw__context, SW_WAYLAND_EVENT_SURFACE_FAILED_TO_SET_DECORATIONS);
			}
			toplevel_priv->decoration_mode = toplevel->decoration_mode;
		}

		if (!su_string_equal(toplevel->title, toplevel_priv->title)) {
			SU_ASSERT(toplevel->title.nul_terminated); /* TODO: handle properly */
			xdg_toplevel_set_title(toplevel_priv->xdg_toplevel, toplevel->title.s);
			su_string_fini(&toplevel_priv->title, gp_alloc);
			su_string_init_string(&toplevel_priv->title, gp_alloc, toplevel->title);
		}

		if (!su_string_equal(toplevel->app_id, toplevel_priv->app_id)) {
			SU_ASSERT(toplevel->app_id.nul_terminated); /* TODO: handle properly */
			xdg_toplevel_set_app_id(toplevel_priv->xdg_toplevel, toplevel->app_id.s);
			su_string_fini(&toplevel_priv->app_id, gp_alloc);
			su_string_init_string(&toplevel_priv->app_id, gp_alloc, toplevel->app_id);
		}

		SU_ASSERT(toplevel->min_width >= 0);
		SU_ASSERT(toplevel->min_height >= 0);
		SU_ASSERT(toplevel->max_width >= 0);
		SU_ASSERT(toplevel->max_height >= 0);
		SU_ASSERT(toplevel->min_width <= toplevel->max_width);
		SU_ASSERT(toplevel->min_height <= toplevel->max_height);

		if ((toplevel->min_width != toplevel_priv->min_width) ||
				(toplevel->min_height != toplevel_priv->min_height)) {
			xdg_toplevel_set_min_size(toplevel_priv->xdg_toplevel, toplevel->min_width, toplevel->min_height);
			toplevel_priv->min_width = toplevel->min_width;
			toplevel_priv->min_height = toplevel->min_height;
		}

		if ((toplevel->max_width != toplevel_priv->max_width) ||
				(toplevel->max_height != toplevel_priv->max_height)) {
			xdg_toplevel_set_max_size(toplevel_priv->xdg_toplevel, toplevel->max_width, toplevel->max_height);
			toplevel_priv->max_width = toplevel->max_width;
			toplevel_priv->max_height = toplevel->max_height;
		}

		/* ? TODO: use internal state */
		if ((surface->out._.toplevel.states & SW_WAYLAND_SURFACE_TOPLEVEL_STATE_MAXIMIZED) != toplevel->maximized) {
			if (toplevel->maximized) {
				xdg_toplevel_set_maximized(toplevel_priv->xdg_toplevel);
			} else {
				xdg_toplevel_unset_maximized(toplevel_priv->xdg_toplevel);
			}
		}
		if ((surface->out._.toplevel.states & SW_WAYLAND_SURFACE_TOPLEVEL_STATE_FULLSCREEN) != toplevel->fullscreen) {
			if (toplevel->fullscreen) {
				xdg_toplevel_set_fullscreen(toplevel_priv->xdg_toplevel,
					toplevel->fullscreen_output
					? ((sw__wayland_output_t *)&toplevel->fullscreen_output->sw__private)->wl_output
					: NULL);
			} else {
				xdg_toplevel_unset_fullscreen(toplevel_priv->xdg_toplevel);
			}
		}

		if (toplevel->minimized) {
			xdg_toplevel_set_minimized(toplevel_priv->xdg_toplevel);
		}

		/* TODO: xdg_surface_set_window_geometry , set_parent,show_window_menu,move,resize */
		break;
	}
	case SW_WAYLAND_SURFACE_TYPE_LAYER: {
		sw_wayland_surface_layer_t *layer = &surface->in._.layer;
		sw__wayland_surface_layer_t *layer_priv = &surface_priv->_.layer;
	
		static uint32_t horiz = (ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
		static uint32_t vert = (ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
		SU_NOTUSED(horiz); SU_NOTUSED(vert);

		SU_ASSERT(layer->output != NULL);
		SU_ASSERT((layer->anchor == SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL) ||
			(((surface->in.width != 0) || ((layer->anchor & horiz) == horiz)) &&
			((surface->in.height != 0) || ((layer->anchor & vert) == vert))));

		if (SU_UNLIKELY(layer_priv->output != layer->output)) {
			sw__wayland_surface_fini(surface, sw__context);
		}
		
		if (SU_UNLIKELY(!surface_priv->wl_surface && !sw__wayland_surface_layer_init(surface, layer->layer))) {
			return;
		}

		if (layer_priv->anchor != layer->anchor) {
			zwlr_layer_surface_v1_set_anchor(layer_priv->layer_surface, layer->anchor);
			layer_priv->anchor = layer->anchor;
		}

		if (layer_priv->layer != layer->layer) {
			zwlr_layer_surface_v1_set_layer(layer_priv->layer_surface, layer->layer);
			layer_priv->layer = layer->layer;
		}

		if (SU_MEMCMP(layer_priv->margins, layer->margins, sizeof(layer->margins)) != 0) {
			zwlr_layer_surface_v1_set_margin(layer_priv->layer_surface,
				layer->margins[0] / surface->out.scale,
				layer->margins[1] / surface->out.scale,
				layer->margins[2] / surface->out.scale,
				layer->margins[3] / surface->out.scale);
			SU_MEMCPY(layer_priv->margins, layer->margins, sizeof(layer->margins));
		}

		/* TODO: set_keyboard_interactivity,set_exclusive_edge  */
		break;
	}
	case SW_WAYLAND_SURFACE_TYPE_POPUP: {
		sw_wayland_surface_popup_t *popup = &surface->in._.popup;
		sw__wayland_surface_popup_t *popup_priv = &surface_priv->_.popup;
		su_bool32_t reposition = SU_FALSE;

		SU_ASSERT(parent != NULL);

		if (SU_UNLIKELY(!surface_priv->wl_surface && !sw__wayland_surface_popup_init_stage1(surface, parent))) {
			return;
		}

		if ((popup_priv->x != popup->x) || (popup_priv->y != popup->y)) {
			xdg_positioner_set_anchor_rect(popup_priv->xdg_positioner,
				popup->x / surface->out.scale, popup->y / surface->out.scale, 1, 1);
			popup_priv->x = popup->x;
			popup_priv->y = popup->y;
			reposition = SU_TRUE;
		}

		if (popup_priv->gravity != popup->gravity) {
			xdg_positioner_set_gravity(popup_priv->xdg_positioner, popup->gravity);
			popup_priv->gravity = popup->gravity;
			reposition = SU_TRUE;
		}

		if (popup_priv->constraint_adjustment != popup->constraint_adjustment) {
			xdg_positioner_set_constraint_adjustment(
				popup_priv->xdg_positioner, popup->constraint_adjustment);
			popup_priv->constraint_adjustment = popup->constraint_adjustment;
			reposition = SU_TRUE;
		}

		/* TODO: xdg_surface_set_window_geometry , set_anchor,set_offset,set_reactive,set_parent_size,set_parent_configure */

		if (popup_priv->xdg_popup && reposition) {
			xdg_popup_reposition(popup_priv->xdg_popup, popup_priv->xdg_positioner, 0);
		}
		break;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}

	if (surface_priv->cursor_shape != cursor_shape) {
		sw_wayland_seat_t *seat = sw__context->out.backend.wayland.seats.head;
		for ( ; seat; seat = seat->next) {
			sw__wayland_pointer_t *pointer_priv = (sw__wayland_pointer_t *)&seat->out.pointer->sw__private;
			if (seat->out.pointer && (seat->out.pointer->out.focused_surface == surface)) {
				if (SU_LIKELY(pointer_priv->cursor_shape_device)) {
					wp_cursor_shape_device_v1_set_shape(pointer_priv->cursor_shape_device,
						pointer_priv->enter_serial, cursor_shape);
				} else {
					surface->in.notify((sw_wayland_event_source_t *)surface,
						sw__context, SW_WAYLAND_EVENT_SURFACE_FAILED_TO_SET_CURSOR_SHAPE);
				} 
			}
		}
		surface_priv->cursor_shape = cursor_shape;
	}

	if ((surface_priv->input_regions_count != surface->in.input_regions_count) ||
			(SU_MEMCMP(surface_priv->input_regions, surface->in.input_regions,
				sizeof(*surface->in.input_regions) * surface->in.input_regions_count) != 0)) {
		size_t i;
		struct wl_region *input_region = NULL;
		if (surface->in.input_regions_count > 0) {
			input_region = wl_compositor_create_region(sw_priv->_.wayland.compositor);
			for ( i = 0; i < surface->in.input_regions_count; ++i) {
				sw_wayland_region_t region = surface->in.input_regions[i];
				wl_region_add(input_region, region.x, region.y, region.width, region.height);
			}
		}
		wl_surface_set_input_region(surface_priv->wl_surface, input_region);

		SU_FREE(gp_alloc, surface_priv->input_regions);
		surface_priv->input_regions_count = surface->in.input_regions_count;
		if (input_region) {
			wl_region_destroy(input_region);
			SU_ARRAY_ALLOC(surface_priv->input_regions, gp_alloc, surface_priv->input_regions_count);
			SU_MEMCPY(surface_priv->input_regions, surface->in.input_regions,
				sizeof(surface_priv->input_regions[0]) * surface->in.input_regions_count);
		} else {
			surface_priv->input_regions = NULL;
		}
	}

	if (sw__wayland_surface_render(surface)) {
		sw_wayland_surface_t *p;
		for ( p = surface->in.popups.head; p; p = p->next) {
			SU_ASSERT(p->in.type == SW_WAYLAND_SURFACE_TYPE_POPUP);
			sw__wayland_surface_prepare(p, surface);
		}
	}
}

static void sw__wayland_surface_popup_init_stage2(sw_wayland_surface_t *surface) {
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_popup_t *popup_priv = &surface_priv->_.popup;
	sw__wayland_surface_t *parent_priv = (sw__wayland_surface_t *)&popup_priv->parent->sw__private;

	static struct xdg_popup_listener xdg_popup_listener = {
		sw__wayland_surface_popup_handle_configure,
		sw__wayland_surface_popup_handle_done,
		sw__wayland_surface_popup_handle_repositioned,
	};

	switch (popup_priv->parent->in.type) {
	case SW_WAYLAND_SURFACE_TYPE_TOPLEVEL:
		popup_priv->xdg_popup = xdg_surface_get_popup( popup_priv->xdg_surface,
			parent_priv->_.toplevel.xdg_surface, popup_priv->xdg_positioner);
		popup_priv->grab = surface->in._.popup.grab;
		break;
	case SW_WAYLAND_SURFACE_TYPE_POPUP:
		popup_priv->xdg_popup = xdg_surface_get_popup( popup_priv->xdg_surface,
			parent_priv->_.popup.xdg_surface, popup_priv->xdg_positioner);
		popup_priv->grab = parent_priv->_.popup.grab;
		break;
	case SW_WAYLAND_SURFACE_TYPE_LAYER:
		popup_priv->xdg_popup = xdg_surface_get_popup(
			popup_priv->xdg_surface, NULL, popup_priv->xdg_positioner);
		zwlr_layer_surface_v1_get_popup(parent_priv->_.layer.layer_surface, popup_priv->xdg_popup);
		popup_priv->grab = surface->in._.popup.grab;
		break;
	default:
		SU_ASSERT_UNREACHABLE;
	}

	xdg_popup_add_listener(popup_priv->xdg_popup, &xdg_popup_listener, surface);

	/* TODO: handle grab with invalid serial, touch serial */
	if (popup_priv->grab) {
		sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&popup_priv->grab->out.seat->sw__private;
		sw__wayland_pointer_t *pointer_priv = (sw__wayland_pointer_t *)&popup_priv->grab->sw__private;
		xdg_popup_grab( popup_priv->xdg_popup,
			seat_priv->wl_seat, pointer_priv->button_serial);
	}
}

static void sw__wayland_output_destroy(sw_wayland_output_t *output) {
	sw__wayland_output_t *output_priv = (sw__wayland_output_t *)&output->sw__private;
	su_allocator_t *gp_alloc = sw__context->in.gp_alloc;
	sw_wayland_surface_t *surface = sw__context->in.backend.wayland.layers.head;
	for ( ; surface; ) {
		sw_wayland_surface_t *next = surface->next;
		if (surface->in._.layer.output == output) {
			sw__wayland_surface_fini(surface, sw__context);
			surface->in.notify( (sw_wayland_event_source_t *)surface,
				sw__context, SW_WAYLAND_EVENT_SURFACE_CLOSE);
		}
		surface = next;
	}
	if (output_priv->wl_output) {
		wl_output_destroy(output_priv->wl_output);
	}
	su_string_fini(&output->out.name, gp_alloc);
	su_string_fini(&output->out.description, gp_alloc);
	su_string_fini(&output->out.make, gp_alloc);
	su_string_fini(&output->out.model, gp_alloc);
	if (output->in.destroy) {
		output->in.destroy(output, sw__context);
	}
}

static void sw__wayland_output_handle_geometry(void *data, struct wl_output *wl_output,
		int32_t x, int32_t y, int32_t physical_width, int32_t physical_height,
		int32_t subpixel, const char *make, const char *model, int32_t transform) {
	sw_wayland_output_t *output = (sw_wayland_output_t *)data;
	su_allocator_t *gp_alloc = sw__context->in.gp_alloc;
	size_t len;

	SU_NOTUSED(wl_output);

	su_string_fini(&output->out.make, gp_alloc);
	su_string_fini(&output->out.model, gp_alloc);

	if ((len = SU_STRLEN(make)) > 0) {
		su_string_init_len(&output->out.make, gp_alloc, make, len, SU_TRUE);
	} else {
		SU_MEMSET(&output->out.make, 0, sizeof(output->out.make));
	}
	if ((len = SU_STRLEN(model)) > 0) {
		su_string_init_len(&output->out.model, gp_alloc, model, len, SU_TRUE);
	} else {
		SU_MEMSET(&output->out.model, 0, sizeof(output->out.model));
	}

	output->out.x = x;
	output->out.y = y;
	output->out.transform = (sw_wayland_output_transform_t)transform;
	output->out.subpixel = (sw_wayland_output_subpixel_t)subpixel;

	output->out.physical_width = physical_width;
	output->out.physical_height = physical_height;
}

static void sw__wayland_output_handle_mode(void *data, struct wl_output *wl_output,
		uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
	sw_wayland_output_t *output = (sw_wayland_output_t *)data;

	SU_NOTUSED(wl_output); SU_NOTUSED(flags);

	output->out.width = width;
	output->out.height = height;
	output->out.refresh = refresh;
}

static void sw__wayland_output_handle_done(void *data, struct wl_output *wl_output) {
	sw_wayland_output_t *output = (sw_wayland_output_t *)data;
	sw__wayland_output_t *output_priv = (sw__wayland_output_t *)&output->sw__private;

	SU_NOTUSED(wl_output);

	if (!output_priv->initialized) {
		sw_wayland_output_t *new_output = NULL;
		output_priv->initialized = SU_TRUE;
		if (sw__context->in.backend.wayland.output_create) {
			new_output = sw__context->in.backend.wayland.output_create(output, sw__context);
		}
		if (!new_output) {
			sw__wayland_output_destroy(output);
		} else if (new_output != output) {
			SU_MEMCPY(&new_output->out, &output->out, sizeof(output->out));
			SU_MEMCPY(&new_output->sw__private, &output->sw__private, sizeof(output->sw__private));
			SU_FREE(sw__context->in.gp_alloc, output);
			wl_output_set_user_data(((sw__wayland_output_t *)&new_output->sw__private)->wl_output, new_output);
		}
		if (new_output) {
			SU_LLIST_APPEND_TAIL(&sw__context->out.backend.wayland.outputs, new_output);
		}
	}
}

static void sw__wayland_output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	sw_wayland_output_t *output = (sw_wayland_output_t *)data;
	SU_NOTUSED(wl_output);
	output->out.scale = factor;
}

static void sw__wayland_output_handle_name(void *data, struct wl_output *wl_output, const char *name) {
	sw_wayland_output_t *output = (sw_wayland_output_t *)data;
	size_t len = SU_STRLEN(name);

	SU_NOTUSED(wl_output);

	if (len > 0) {
		su_string_init_len(&output->out.name, sw__context->in.gp_alloc, name, len, SU_TRUE);
	}
}

static void sw__wayland_output_handle_description(void *data, struct wl_output *wl_output, const char *description) {
	sw_wayland_output_t *output = (sw_wayland_output_t *)data;
	size_t len = SU_STRLEN(description);

	SU_NOTUSED(wl_output);

	su_string_fini(&output->out.description, sw__context->in.gp_alloc);
	if (len > 0) {
		su_string_init_len(&output->out.description, sw__context->in.gp_alloc, description, len, SU_TRUE);
	} else {
		SU_MEMSET(&output->out.description, 0, sizeof(output->out.description));
	}
}

static void sw__wayland_output_handle_destroy(sw_wayland_output_t *output, sw_context_t *sw) {
	SU_FREE(sw->in.gp_alloc, output);
}

static sw_wayland_output_t *sw__wayland_output_create(uint32_t wl_name) {
	static struct wl_output_listener output_listener = {
		sw__wayland_output_handle_geometry,
		sw__wayland_output_handle_mode,
		sw__wayland_output_handle_done,
		sw__wayland_output_handle_scale,
		sw__wayland_output_handle_name,
		sw__wayland_output_handle_description,
	};

	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw_wayland_output_t *output;
	sw__wayland_output_t *output_priv;

	SU_ALLOCCT(output, sw__context->in.gp_alloc);
	output_priv = (sw__wayland_output_t *)&output->sw__private;
	
	output->in.destroy = sw__wayland_output_handle_destroy;
	output->out.scale = 1;
	output_priv->wl_output = (struct wl_output *)wl_registry_bind(sw_priv->_.wayland.registry,
		wl_name, &wl_output_interface, 4);
	output_priv->wl_name = wl_name;
	wl_output_add_listener(output_priv->wl_output, &output_listener, output);

	return output;
}

static void sw__wayland_pointer_handle_enter(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *wl_surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;
	sw__wayland_pointer_t *pointer_priv = (sw__wayland_pointer_t *)&pointer->sw__private;
	sw_wayland_surface_t *surface;

	SU_NOTUSED(wl_pointer);

	if (SU_UNLIKELY(!wl_surface)) {
		return;
	}

	surface = (sw_wayland_surface_t *)wl_surface_get_user_data(wl_surface);

	if (SU_LIKELY(pointer_priv->cursor_shape_device)) {
		sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
		wp_cursor_shape_device_v1_set_shape(pointer_priv->cursor_shape_device,
				serial, surface_priv->cursor_shape);
	}

	pointer_priv->enter_serial = serial;

	pointer->out.focused_surface = surface;
	pointer->out.pos_x = (int32_t)(wl_fixed_to_double(surface_x) * (double)surface->out.scale);
	pointer->out.pos_y = (int32_t)(wl_fixed_to_double(surface_y) * (double)surface->out.scale);
	surface->in.notify((sw_wayland_event_source_t *)pointer, sw__context, SW_WAYLAND_EVENT_POINTER_ENTER);
}

static void sw__wayland_pointer_handle_leave(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;

	SU_NOTUSED(wl_pointer); SU_NOTUSED(serial); SU_NOTUSED(surface);

	if (SU_LIKELY(pointer->out.focused_surface)) {
		pointer->out.focused_surface->in.notify((sw_wayland_event_source_t *)pointer,
			sw__context, SW_WAYLAND_EVENT_POINTER_LEAVE);
		pointer->out.focused_surface = NULL;
	}
}

static void sw__wayland_pointer_handle_motion(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;
	int32_t x, y;

	SU_NOTUSED(wl_pointer); SU_NOTUSED(time);

	if (SU_UNLIKELY(!pointer->out.focused_surface)) {
		return;
	}

	x = (int32_t)(wl_fixed_to_double(surface_x) * (double)pointer->out.focused_surface->out.scale);
	y = (int32_t)(wl_fixed_to_double(surface_y) * (double)pointer->out.focused_surface->out.scale);
	if ((x != pointer->out.pos_x) || (y != pointer->out.pos_y)) {
		pointer->out.pos_x = x;
		pointer->out.pos_y = y;
		pointer->out.focused_surface->in.notify((sw_wayland_event_source_t *)pointer,
			sw__context, SW_WAYLAND_EVENT_POINTER_MOTION);
	}
}

static void sw__wayland_pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t st) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;
	sw__wayland_pointer_t *pointer_priv = (sw__wayland_pointer_t *)&pointer->sw__private;

	SU_NOTUSED(wl_pointer); SU_NOTUSED(time);

	if (SU_UNLIKELY(!pointer->out.focused_surface)) {
		return;
	}

	pointer_priv->button_serial = serial;

	pointer->out.btn_code = button;
	pointer->out.btn_state = (sw_wayland_pointer_button_state_t)st;
	pointer->out.focused_surface->in.notify((sw_wayland_event_source_t *)pointer,
		sw__context, SW_WAYLAND_EVENT_POINTER_BUTTON);
}

static void sw__wayland_pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;

	SU_NOTUSED(wl_pointer); SU_NOTUSED(data); SU_NOTUSED(time);

	if (SU_UNLIKELY(!pointer->out.focused_surface)) {
		return;
	}

	pointer->out.scroll_axis = (sw_wayland_pointer_scroll_axis_t)axis;
	pointer->out.scroll_vector_length = wl_fixed_to_double(value);
	pointer->out.focused_surface->in.notify((sw_wayland_event_source_t *)pointer,
		sw__context, SW_WAYLAND_EVENT_POINTER_SCROLL);
}

static sw_wayland_pointer_t *sw__wayland_pointer_create(sw_wayland_seat_t *seat) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw_wayland_pointer_t *pointer = NULL;
	if (seat->in.pointer_create && (pointer = seat->in.pointer_create(seat, sw__context))) {
		sw__wayland_pointer_t *pointer_priv = (sw__wayland_pointer_t *)&pointer->sw__private;
		sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&seat->sw__private;

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
		pointer_priv->wl_pointer = wl_seat_get_pointer(seat_priv->wl_seat);
		wl_pointer_add_listener(pointer_priv->wl_pointer, &pointer_listener, pointer);
		if (sw_priv->_.wayland.cursor_shape_manager) {
			pointer_priv->cursor_shape_device = wp_cursor_shape_manager_v1_get_pointer(
				sw_priv->_.wayland.cursor_shape_manager, pointer_priv->wl_pointer);
		}
	}

	return pointer;
}

static void sw__wayland_pointer_destroy(sw_wayland_pointer_t *pointer) {
	sw__wayland_pointer_t *pointer_priv = (sw__wayland_pointer_t *)&pointer->sw__private;
	if (pointer_priv->cursor_shape_device) {
		wp_cursor_shape_device_v1_destroy(pointer_priv->cursor_shape_device);
	}
	if (pointer_priv->wl_pointer) {
		wl_pointer_destroy(pointer_priv->wl_pointer);
	}
	if (pointer->in.destroy) {
		pointer->in.destroy(pointer, sw__context);
	}
}

static void sw__wayland_seat_destroy(sw_wayland_seat_t *seat) {
	sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&seat->sw__private;
	if (seat->out.pointer) {
		sw__wayland_pointer_destroy(seat->out.pointer);
	}
	if (seat_priv->wl_seat) {
		wl_seat_destroy(seat_priv->wl_seat);
	}
	su_string_fini(&seat->out.name, sw__context->in.gp_alloc);
	if (seat->in.destroy) {
		seat->in.destroy(seat, sw__context);
	}
}

static void sw__wayland_seat_handle_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
	sw_wayland_seat_t *seat = (sw_wayland_seat_t *)data;

	su_bool32_t cap_pointer = (capabilities & WL_SEAT_CAPABILITY_POINTER);
	/* TODO: touch, keyboard */

	SU_NOTUSED(wl_seat);

	if (cap_pointer && !seat->out.pointer) {
		seat->out.pointer = sw__wayland_pointer_create(seat);
	} else if (!cap_pointer && seat->out.pointer) {
		sw__wayland_pointer_destroy(seat->out.pointer);
		seat->out.pointer = NULL;
	}
}

static void sw__wayland_seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name) {
	sw_wayland_seat_t *seat = (sw_wayland_seat_t *)data;
	sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&seat->sw__private;
	size_t len = SU_STRLEN(name);

	SU_NOTUSED(wl_seat);

	if (len > 0) {
		su_string_init_len(&seat->out.name, sw__context->in.gp_alloc, name, len, SU_TRUE);
	}

	if (!seat_priv->initialized) {
		sw_wayland_seat_t *new_seat = NULL;
		seat_priv->initialized = SU_TRUE;
		if (sw__context->in.backend.wayland.seat_create) {
			new_seat = sw__context->in.backend.wayland.seat_create(seat, sw__context);
		}
		if (!new_seat) {
			sw__wayland_seat_destroy(seat);
		} else if (new_seat != seat) {
			SU_MEMCPY(&new_seat->out, &seat->out, sizeof(seat->out));
			SU_MEMCPY(&new_seat->sw__private, &seat->sw__private, sizeof(seat->sw__private));
			SU_FREE(sw__context->in.gp_alloc, seat);
			wl_seat_set_user_data(((sw__wayland_seat_t *)&new_seat->sw__private)->wl_seat, new_seat);
		}
		if (new_seat) {
			SU_LLIST_APPEND_TAIL(&sw__context->out.backend.wayland.seats, new_seat);
		}
	}
}

static void sw__wayland_seat_handle_destroy(sw_wayland_seat_t *seat, sw_context_t *sw) {
	SU_FREE(sw->in.gp_alloc, seat);
}

static sw_wayland_seat_t *sw__wayland_seat_create(uint32_t wl_name) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw_wayland_seat_t *seat;
	sw__wayland_seat_t *seat_priv;

	static struct wl_seat_listener seat_listener = {
		sw__wayland_seat_handle_capabilities,
		sw__wayland_seat_handle_name,
	};

	SU_ALLOCCT(seat, sw__context->in.gp_alloc);
	seat_priv = (sw__wayland_seat_t *)&seat->sw__private;

	seat->in.destroy = sw__wayland_seat_handle_destroy;
	seat_priv->wl_seat = (struct wl_seat *)wl_registry_bind(
		sw_priv->_.wayland.registry, wl_name, &wl_seat_interface, 2);
	seat_priv->wl_name = wl_name;
	wl_seat_add_listener(seat_priv->wl_seat, &seat_listener, seat);

	return seat;
}

static void sw__wayland_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	SU_NOTUSED(data); SU_NOTUSED(xdg_wm_base);
	xdg_wm_base_pong(sw_priv->_.wayland.wm_base, serial);
}

static void sw__wayland_registry_handle_global(void *data, struct wl_registry *wl_registry,
		uint32_t wl_name, const char *interface, uint32_t version) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw__context_wayland_t *wayland_priv = &sw_priv->_.wayland;

	SU_NOTUSED(data); SU_NOTUSED(wl_registry); SU_NOTUSED(version);
	
	if (SU_STRCMP(interface, wl_output_interface.name) == 0) {
		sw__wayland_output_create(wl_name);
	} else if (SU_STRCMP(interface, wl_seat_interface.name) == 0) {
		sw__wayland_seat_create(wl_name);
    } else if (SU_STRCMP(interface, wl_compositor_interface.name) == 0) {
		wayland_priv->compositor = (struct wl_compositor *)wl_registry_bind(
			wayland_priv->registry, wl_name, &wl_compositor_interface, 6);
	} else if (SU_STRCMP(interface, wl_shm_interface.name) == 0) {
		wayland_priv->shm = (struct wl_shm *)wl_registry_bind(
			wayland_priv->registry, wl_name, &wl_shm_interface, 1);
		/* ? TODO: wl_shm_add_listener (check for ARGB32) */
	} else if (SU_STRCMP(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		wayland_priv->layer_shell = (struct zwlr_layer_shell_v1 *)wl_registry_bind(
			wayland_priv->registry, wl_name, &zwlr_layer_shell_v1_interface, 2);
	} else if (SU_STRCMP(interface, xdg_wm_base_interface.name) == 0) {
		static struct xdg_wm_base_listener wm_base_listener = { sw__wayland_wm_base_handle_ping };
		wayland_priv->wm_base = (struct xdg_wm_base *)wl_registry_bind(
			wayland_priv->registry,wl_name, &xdg_wm_base_interface, 3);
		xdg_wm_base_add_listener(wayland_priv->wm_base, &wm_base_listener, NULL);
	} else if (SU_STRCMP(interface, wp_cursor_shape_manager_v1_interface.name) == 0) {
		wayland_priv->cursor_shape_manager = (struct wp_cursor_shape_manager_v1 *)wl_registry_bind(
			wayland_priv->registry, wl_name, &wp_cursor_shape_manager_v1_interface, 1);
	} else if (SU_STRCMP(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
		wayland_priv->decoration_manager = (struct zxdg_decoration_manager_v1 *)wl_registry_bind(
			wayland_priv->registry, wl_name, &zxdg_decoration_manager_v1_interface, 1);
	}
}

static void sw__wayland_registry_handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
	sw_backend_wayland_out_t *wayland = &sw__context->out.backend.wayland;
	sw_wayland_output_t *output;
	sw_wayland_seat_t *seat;

	SU_NOTUSED(data); SU_NOTUSED(wl_registry);

	for ( output = wayland->outputs.head; output; output = output->next) {
		sw__wayland_output_t *output_priv = (sw__wayland_output_t *)&output->sw__private;
		if (output_priv->wl_name == name) {
			SU_LLIST_POP(&wayland->outputs, output);
			sw__wayland_output_destroy(output);
			return;
		}
	}

	for ( seat = wayland->seats.head; seat; seat = seat->next) {
		sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&seat->sw__private;
		if (seat_priv->wl_name == name) {
			SU_LLIST_POP(&wayland->seats, seat);
			sw__wayland_seat_destroy(seat);
			return;
		}
	}
}

#endif /* SW_WITH_WAYLAND_BACKEND */

SW_FUNC_DEF void sw_cleanup(sw_context_t *sw) {
	sw__context_t *sw_priv = (sw__context_t *)&sw->sw__private;
	sw_context_t *old_context = sw__context;
	su_allocator_t *gp_alloc = sw->in.gp_alloc;
	size_t i;

	sw__context = sw;

	switch (sw->in.backend_type) {
#if SW_WITH_MEMORY_BACKEND
	case SW_BACKEND_TYPE_MEMORY: {
		sw__context_memory_t *memory = &sw_priv->_.memory;
		pixman_image_unref(memory->image);
		break;
	}
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
	case SW_BACKEND_TYPE_WAYLAND: {
		sw__context_wayland_t *wayland = &sw_priv->_.wayland;
		sw_wayland_surface_t *surface;
		sw_wayland_output_t *output;
		sw_wayland_seat_t *seat;

		for ( surface = sw->in.backend.wayland.toplevels.head; surface; ) {
			sw_wayland_surface_t *next = surface->next;
			sw__wayland_surface_fini(surface, sw__context);
			surface->in.notify( (sw_wayland_event_source_t *)surface,
				sw__context, SW_WAYLAND_EVENT_SURFACE_CLOSE);
			surface = next;
		}

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

		if (wayland->decoration_manager) {
			zxdg_decoration_manager_v1_destroy(wayland->decoration_manager);
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
#endif /* SW_WITH_WAYLAND_BACKEND */
	case SW_BACKEND_TYPE_INVALID:
	default:
		SU_ASSERT_UNREACHABLE;
	}

	for ( i = 0; i < sw_priv->image_cache.capacity; ++i) {
		sw__image_cache_t *cache = &sw_priv->image_cache.items[i];
		SU_FREE(gp_alloc, cache->key.ptr);
		if (cache->image) {
			pixman_image_unref(cache->image);
		}
	}
	su_hash_table__sw__image_cache_t__fini(&sw_priv->image_cache, gp_alloc);

#if SW_WITH_TEXT
	for ( i = 0; i < sw_priv->text_run_cache.capacity; ++i) {
		size_t j = 0;
		sw__text_run_cache_t *c = &sw_priv->text_run_cache.items[i];
		su_string_fini(&c->key, gp_alloc);
		for ( ; j < c->items_count; ++j) {
			sw__text_run_cache_entry_t entry = c->items[j];
			fcft_text_run_destroy(entry.text_run);
			fcft_destroy(entry.font);
		}
	}
	su_hash_table__sw__text_run_cache_t__fini(&sw_priv->text_run_cache, gp_alloc);

	fcft_fini();
#endif /* SW_WITH_TEXT */

	SU_MEMSET(&sw->out, 0, sizeof(sw->out));
	SU_MEMSET(sw_priv, 0, sizeof(*sw_priv));

	sw__context = old_context;
}

SW_FUNC_DEF su_bool32_t sw_flush(sw_context_t *sw) {
	su_bool32_t ret = SU_TRUE;
	sw__context_t *sw_priv = (sw__context_t *)&sw->sw__private;
	sw_context_t *old_context = sw__context;

	SU_NOTUSED(sw_priv);

	sw__context = sw;

	switch (sw->in.backend_type) {
#if SW_WITH_MEMORY_BACKEND
	case SW_BACKEND_TYPE_MEMORY: {
		break;
	}
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
	case SW_BACKEND_TYPE_WAYLAND:
		sw->out.backend.wayland.pfd.events = POLLIN;
		if (wl_display_flush(sw_priv->_.wayland.display) == -1) {
			if (errno == EAGAIN) {
				sw->out.backend.wayland.pfd.events = (POLLIN | POLLOUT);
			} else {
				ret = SU_FALSE;
			}
		}
		break;
#endif /* SW_WITH_WAYLAND_BACKEND */
	case SW_BACKEND_TYPE_INVALID:
	default:
		SU_ASSERT_UNREACHABLE;
	}

	sw__context = old_context;
	return ret;
}

SW_FUNC_DEF su_bool32_t sw_process(sw_context_t *sw) {
	sw__context_t *sw_priv = (sw__context_t *)&sw->sw__private;
	sw_context_t *old_context = sw__context;

	SU_NOTUSED(sw_priv);

	sw__context = sw;

	switch (sw->in.backend_type) {
#if SW_WITH_MEMORY_BACKEND
	case SW_BACKEND_TYPE_MEMORY:
		break;
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
	case SW_BACKEND_TYPE_WAYLAND:
		if (wl_display_prepare_read(sw_priv->_.wayland.display) != -1) {
			if (wl_display_read_events(sw_priv->_.wayland.display) == -1) {
				sw__context = old_context;
				return SU_FALSE;
			}
		}
		wl_display_dispatch_pending(sw_priv->_.wayland.display);
		break;
#endif /* SW_WITH_WAYLAND_BACKEND */
	case SW_BACKEND_TYPE_INVALID:
	default:
		SU_ASSERT_UNREACHABLE;
	}

	sw__context = old_context;
	return SU_TRUE;
}

SW_FUNC_DEF su_bool32_t sw_set(sw_context_t *sw) {
	static su_allocator_t fallback_alloc = { su_libc_alloc, su_libc_free };
	su_bool32_t ret = SU_TRUE;
	sw__context_t *sw_priv = (sw__context_t *)&sw->sw__private;
	sw_context_t *old_context = sw__context;

	SU_ASSERT(sw->in.backend_type != SW_BACKEND_TYPE_INVALID);
	SU_ASSERT(su_locale_is_utf8());

	sw__context = sw;

	if (SU_UNLIKELY(!sw->in.gp_alloc)) {
		sw->in.gp_alloc = &fallback_alloc;
	}
	if (SU_UNLIKELY(!sw->in.scratch_alloc)) {
		/* ? TODO: internal arena alloc */
		sw->in.scratch_alloc = &fallback_alloc;
	}

	if (su_now_ms(CLOCK_MONOTONIC) >= sw->out.t) {
		sw->out.t = -1;
	}

	if (SU_UNLIKELY(!sw_priv->image_cache.items)) {
		/* ? TODO: resvg_init_log(); */

		su_hash_table__sw__image_cache_t__init(&sw_priv->image_cache, sw->in.gp_alloc, 512);

#if SW_WITH_TEXT
		if (!fcft_init(FCFT_LOG_COLORIZE_NEVER, SU_FALSE, FCFT_LOG_CLASS_ERROR)) {
			ret = SU_FALSE;
			goto out;
		}

		su_hash_table__sw__text_run_cache_t__init(&sw_priv->text_run_cache, sw->in.gp_alloc, 1024);
#endif /* SW_WITH_TEXT */
	}

	switch (sw->in.backend_type) {
#if SW_WITH_MEMORY_BACKEND
	case SW_BACKEND_TYPE_MEMORY: {
		sw_backend_memory_in_t *memory = &sw->in.backend.memory;
		sw__context_memory_t *memory_priv = &sw_priv->_.memory;

		SU_ASSERT(memory->root != NULL);
		SU_ASSERT((memory->width > 0) && (memory->height > 0));
		SU_ASSERT(memory->memory != NULL);

		if (SU_UNLIKELY(!memory_priv->image)) {
			memory_priv->image = pixman_image_create_bits_no_clear(PIXMAN_a8r8g8b8,
				(int)memory->width, (int)memory->height,
				(uint32_t *)memory->memory, (int)(memory->width * 4));
		} else if (((uint32_t)pixman_image_get_width(memory_priv->image) != memory->width) ||
				((uint32_t)pixman_image_get_height(memory_priv->image) != memory->height)) {
			pixman_image_unref(memory_priv->image);
			memory_priv->image = pixman_image_create_bits_no_clear(PIXMAN_a8r8g8b8,
				(int)memory->width, (int)memory->height,
				(uint32_t *)memory->memory, (int)(memory->width * 4));
		}

		if (!sw__layout_block_init(memory->root)) {
			goto out;
		}
		if (!sw__layout_block_prepare(memory->root, NULL)) {
			goto out;
		}
		if (!sw__layout_block_fill(memory->root, (int32_t)memory->width, (int32_t)memory->height)) {
			goto out;
		}

		SU_MEMSET(memory->memory, 0, (memory->width * memory->height * 4));
		sw__layout_block_render(memory->root, memory_priv->image);
		break;
	}
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
	case SW_BACKEND_TYPE_WAYLAND: {
		sw__context_wayland_t *wayland_priv = &sw_priv->_.wayland;
		sw_wayland_surface_t *s;

		if (SU_UNLIKELY(!wayland_priv->display)) {
			static struct wl_registry_listener registry_listener = {
				sw__wayland_registry_handle_global,
				sw__wayland_registry_handle_global_remove,
			};

			wayland_priv->display = wl_display_connect(NULL);
			if (!wayland_priv->display) {
				ret = SU_FALSE;
				goto out;
			}

			wayland_priv->registry = wl_display_get_registry(wayland_priv->display);
			wl_registry_add_listener(wayland_priv->registry, &registry_listener, NULL);
			if (wl_display_roundtrip(wayland_priv->display) == -1) {
				ret = SU_FALSE;
				goto out;
			}
			if (wl_display_roundtrip(wayland_priv->display) == -1) {
				ret = SU_FALSE;
				goto out;
			}

			sw->out.backend.wayland.pfd.fd = wl_display_get_fd(wayland_priv->display);
			sw->out.backend.wayland.pfd.events = POLLIN;
		}

		for ( s = sw->in.backend.wayland.toplevels.head; s; s = s->next) {
			SU_ASSERT(s->in.type == SW_WAYLAND_SURFACE_TYPE_TOPLEVEL);
			sw__wayland_surface_prepare(s, NULL);
		}

		for ( s = sw->in.backend.wayland.layers.head; s; s = s->next) {
			SU_ASSERT(s->in.type == SW_WAYLAND_SURFACE_TYPE_LAYER);
			sw__wayland_surface_prepare(s, NULL);
		}
		break;
	}
#endif /* SW_WITH_WAYLAND_BACKEND */
	case SW_BACKEND_TYPE_INVALID:
	default:
		SU_ASSERT_UNREACHABLE;
	}

out:
	sw__context = old_context;
	return ret;
}

#endif /* defined(SW_IMPLEMENTATION) && !defined(SW__REIMPLEMENTATION_GUARD) */
