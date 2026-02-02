#if !defined(SW_HEADER)
#define SW_HEADER

#define SW_IMPLEMENTATION

#if !defined(SW_WITH_DEBUG)
	#define SW_WITH_DEBUG 1
#endif /* !defined(SW_WITH_DEBUG) */
#if !defined(SW_WITH_MEMORY_BACKEND)
	#define SW_WITH_MEMORY_BACKEND 1
#endif /* !defined(SW_WITH_MEMORY_BACKEND) */
#if !defined(SW_WITH_WAYLAND_BACKEND)
	#define SW_WITH_WAYLAND_BACKEND 1
#endif /* !defined(SW_WITH_WAYLAND_BACKEND) */
#if !defined(SW_WITH_WAYLAND_KEYBOARD)
	#define SW_WITH_WAYLAND_KEYBOARD 1
#endif /* !defined(SW_WITH_WAYLAND_KEYBOARD) */
#if !defined(SW_WITH_WAYLAND_CLIPBOARD)
	#define SW_WITH_WAYLAND_CLIPBOARD 1
#endif /* !defined(SW_WITH_WAYLAND_CLIPBOARD) */
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



typedef struct sw_layout_block sw_layout_block_t;
#define SW__PRIVATE_FIELDS(size) size_t sw__private[size / sizeof(size_t)]

#if SW_WITH_WAYLAND_BACKEND

#include <poll.h>

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

typedef struct sw_wayland_output_out {
	su_string_t name, description;
	su_string_t make, model;
	int32_t width, height, scale;
	sw_wayland_output_subpixel_t subpixel;
	int32_t x, y, refresh, physical_width, physical_height;
	sw_wayland_output_transform_t transform;
} sw_wayland_output_out_t;

typedef struct sw_wayland_output sw_wayland_output_t;

struct sw_wayland_output {
	sw_wayland_output_out_t out;
	SW__PRIVATE_FIELDS(16);
	SU_LLIST_NODE_FIELDS(sw_wayland_output_t);
};

typedef struct sw_wayland_seat sw_wayland_seat_t;
typedef struct sw_wayland_surface sw_wayland_surface_t;

typedef struct sw_wayland_pointer_out {
	sw_wayland_seat_t *seat;
	sw_wayland_surface_t *focused_surface;
	uint32_t time;
	int32_t pos_x, pos_y;
	uint32_t btn_code;
	sw_wayland_pointer_button_state_t btn_state;
	sw_wayland_pointer_scroll_axis_t scroll_axis;
	double scroll_vector_length;
} sw_wayland_pointer_out_t;

typedef struct sw_wayland_pointer {
	sw_wayland_pointer_out_t out;
	SW__PRIVATE_FIELDS(24);
} sw_wayland_pointer_t;

typedef struct sw_event_wayland_pointer_out {
	sw_wayland_pointer_t *pointer; /* must be first */
	sw_wayland_pointer_out_t state; /* at the time of event */
} sw_event_wayland_pointer_out_t;

#if SW_WITH_WAYLAND_KEYBOARD
typedef struct sw_wayland_keyboard sw_wayland_keyboard_t;

typedef enum sw_wayland_keyboard_key_state {
	SW_WAYLAND_KEYBOARD_KEY_STATE_RELEASED = 0,
	SW_WAYLAND_KEYBOARD_KEY_STATE_PRESSED = 1,
	SW_WAYLAND_KEYBOARD_KEY_STATE_REPEATED = 2
} sw_wayland_keyboard_key_state_t;

typedef enum sw_wayland_keyboard_mod {
	SW_WAYLAND_KEYBOARD_MOD_SHIFT = (1 << 1),
	SW_WAYLAND_KEYBOARD_MOD_CAPS = (1 << 2),
	SW_WAYLAND_KEYBOARD_MOD_CTRL = (1 << 3),
	SW_WAYLAND_KEYBOARD_MOD_1 = (1 << 4),
	SW_WAYLAND_KEYBOARD_MOD_2 = (1 << 5),
	SW_WAYLAND_KEYBOARD_MOD_3 = (1 << 6),
	SW_WAYLAND_KEYBOARD_MOD_4 = (1 << 7),
	SW_WAYLAND_KEYBOARD_MOD_5 = (1 << 8),
	SW_WAYLAND_KEYBOARD_MOD_VALT = (1 << 9),
	SW_WAYLAND_KEYBOARD_MOD_VHYPER = (1 << 10),
	SW_WAYLAND_KEYBOARD_MOD_VLEVEL3 = (1 << 11),
	SW_WAYLAND_KEYBOARD_MOD_VLEVEL5 = (1 << 12),
	SW_WAYLAND_KEYBOARD_MOD_VMETA = (1 << 13),
	SW_WAYLAND_KEYBOARD_MOD_VNUM = (1 << 14),
	SW_WAYLAND_KEYBOARD_MOD_VSCROLL = (1 << 15),
	SW_WAYLAND_KEYBOARD_MOD_VSUPER = (1 << 16)
} sw_wayland_keyboard_mod_t;

typedef uint32_t sw_wayland_keyboard_mod_mask_t;

typedef struct sw_wayland_keyboard_key {
	uint32_t cp;
	sw_wayland_keyboard_key_state_t state;
	int64_t time;
} sw_wayland_keyboard_key_t;

typedef struct sw_wayland_keyboard_out {
	sw_wayland_seat_t *seat;
	sw_wayland_surface_t *focused_surface;
	sw_wayland_keyboard_key_t key;
	sw_wayland_keyboard_mod_mask_t mods;
	SU_PAD32;
	int32_t repeat_rate;
	int32_t repeat_delay;
} sw_wayland_keyboard_out_t;

struct sw_wayland_keyboard {
	sw_wayland_keyboard_out_t out;
	SW__PRIVATE_FIELDS(48);
};

typedef struct sw_event_wayland_keyboard_out {
	sw_wayland_keyboard_t *keyboard; /* must be first */
	sw_wayland_keyboard_out_t state; /* at the time of event */
} sw_event_wayland_keyboard_out_t;

#endif /* SW_WITH_WAYLAND_KEYBOARD */

#if SW_WITH_WAYLAND_CLIPBOARD

typedef enum sw_wayland_data_device_dnd_action {
	SW_WAYLAND_DATA_DEVICE_DND_ACTION_NONE = 0,
	SW_WAYLAND_DATA_DEVICE_DND_ACTION_COPY = 1,
	SW_WAYLAND_DATA_DEVICE_DND_ACTION_MOVE = 2,
	SW_WAYLAND_DATA_DEVICE_DND_ACTION_ASK = 4
} sw_wayland_data_device_dnd_action_t;

typedef uint32_t sw_wayland_data_device_dnd_action_mask_t;

typedef struct sw_wayland_data_device_in_copy {
	su_string_t mime_type;
	su_fat_ptr_t data;
	su_bool32_t dnd;
	sw_wayland_data_device_dnd_action_mask_t dnd_actions;
	sw_wayland_surface_t *dnd_cursor_image; /* may be NULL */
} sw_wayland_data_device_in_copy_t;

typedef struct sw_wayland_data_device_in_paste {
	su_string_t mime_type;
	sw_wayland_data_device_dnd_action_mask_t dnd_actions;
	sw_wayland_data_device_dnd_action_t dnd_preferred_action;
} sw_wayland_data_device_in_paste_t;

typedef struct sw_wayland_data_device_in {
	sw_wayland_data_device_in_copy_t copy;
	sw_wayland_data_device_in_paste_t paste;
} sw_wayland_data_device_in_t;

typedef struct sw_wayland_data_device_out_copy {
	sw_wayland_data_device_dnd_action_t dnd_action;
	SU_PAD32;
} sw_wayland_data_device_out_copy_t;

typedef struct sw_wayland_data_device_out_paste {
	su_string_t *offered_mime_types; /* ? TODO: arena */
	size_t offered_mime_types_count;
	su_fat_ptr_t data;
	su_bool32_t dnd;
	sw_wayland_data_device_dnd_action_mask_t dnd_source_actions;
	sw_wayland_data_device_dnd_action_t dnd_action;
	int32_t dnd_x, dnd_y;
	uint32_t dnd_time; /* last motion event */
	sw_wayland_surface_t *dnd_surface;
} sw_wayland_data_device_out_paste_t;

typedef struct sw_wayland_data_device_out {
	sw_wayland_seat_t *seat;
	sw_wayland_data_device_out_copy_t copy;
	sw_wayland_data_device_out_paste_t paste;
} sw_wayland_data_device_out_t;

typedef struct sw_wayland_data_device {
	sw_wayland_data_device_in_t in;
	sw_wayland_data_device_out_t out;
	SW__PRIVATE_FIELDS(152);
} sw_wayland_data_device_t;

typedef struct sw_event_wayland_data_device_out {
	sw_wayland_data_device_t *data_device;
	sw_wayland_data_device_out_paste_t paste_state; /* at the time of event */
} sw_event_wayland_data_device_out_t;

#endif /* SW_WITH_WAYLAND_CLIPBOARD */

typedef struct sw_wayland_seat_out {
	su_string_t name;
	sw_wayland_pointer_t *pointer;
#if SW_WITH_WAYLAND_KEYBOARD
	sw_wayland_keyboard_t *keyboard;
#endif /* SW_WITH_WAYLAND_KEYBOARD */
#if SW_WITH_WAYLAND_CLIPBOARD
	sw_wayland_data_device_t *data_device;
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
} sw_wayland_seat_out_t;

struct sw_wayland_seat {
	sw_wayland_seat_out_t out;
	SW__PRIVATE_FIELDS(24);
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

typedef uint32_t sw_wayland_surface_layer_anchor_mask_t;

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
	SW_WAYLAND_CURSOR_SHAPE_ZOOM_OUT = 34,
	/*SW_WAYLAND_CURSOR_SHAPE_DND_ASK = 35, */
	/*SW_WAYLAND_CURSOR_SHAPE_ALL_RESIZE = 36 */
	SW_WAYLAND_CURSOR_SHAPE_INVISIBLE = 1000
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

typedef uint32_t sw_wayland_surface_popup_constraint_adjustment_mask_t;

typedef enum sw_wayland_surface_type {
	SW_WAYLAND_SURFACE_TYPE_TOPLEVEL,
	SW_WAYLAND_SURFACE_TYPE_LAYER,
	SW_WAYLAND_SURFACE_TYPE_POPUP,
	SW_WAYLAND_SURFACE_TYPE_CURSOR_IMAGE
} sw_wayland_surface_type_t;

typedef enum sw_wayland_surface_layer_keyboard_interactivity {
	SW_WAYLAND_SURFACE_LAYER_KEYBOARD_INTERACTIVITY_NONE,
	SW_WAYLAND_SURFACE_LAYER_KEYBOARD_INTERACTIVITY_EXCLUSIVE,
	SW_WAYLAND_SURFACE_LAYER_KEYBOARD_INTERACTIVITY_ON_DEMAND
} sw_wayland_surface_layer_keyboard_interactivity_t;

typedef struct sw_wayland_surface_layer {
	sw_wayland_output_t *output;
	int32_t exclusive_zone;
	sw_wayland_surface_layer_anchor_mask_t anchor;
	sw_wayland_surface_layer_layer_t layer;
	int32_t margins[4]; /* top right bottom left */
	sw_wayland_surface_layer_keyboard_interactivity_t keyboard_interactivity;
} sw_wayland_surface_layer_t;

typedef struct sw_wayland_surface_popup {
	int32_t x, y;
	sw_wayland_surface_popup_gravity_t gravity;
	sw_wayland_surface_popup_constraint_adjustment_mask_t constraint_adjustment;
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

typedef struct sw_wayland_surface_cursor_image {
	int32_t offset_x, offset_y;
} sw_wayland_surface_cursor_image_t;

typedef union sw_wayland_surface_in_ {
	sw_wayland_surface_toplevel_t toplevel;
	sw_wayland_surface_layer_t layer;
	sw_wayland_surface_popup_t popup;
	sw_wayland_surface_cursor_image_t cursor_image;
} sw_wayland_surface_in__t;

typedef struct sw_wayland_surfaces {
	SU_LLIST_FIELDS(sw_wayland_surface_t);
} sw_wayland_surfaces_t;

typedef struct sw_wayland_surface_in {
	sw_wayland_surface_in__t _;
	sw_wayland_surface_type_t type;

	/* ? TODO: since this is ignored for cursor_shape surfaces, move this to sw_wayland_surface_layer,toplevel,popup_t */
	sw_wayland_cursor_shape_t cursor_shape;
	sw_wayland_surface_t *cursor_image; /* may be NULL, takes precedence over cursor_shape */
    sw_wayland_region_t *input_regions; /* may be NULL */
	size_t input_regions_count;
    sw_wayland_surfaces_t popups;

	int32_t width, height;
    sw_layout_block_t *root;
	/* ? TODO: prepare, prepared callbacks */
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

typedef uint32_t sw_wayland_surface_toplevel_state_mask_t;

typedef struct sw_wayland_surface_out_toplevel {
	sw_wayland_surface_toplevel_state_mask_t states;
	SU_PAD32;
} sw_wayland_surface_out_toplevel_t;

typedef union sw_wayland_surface_out_ {
	sw_wayland_surface_out_toplevel_t toplevel;
} sw_wayland_surface_out__t;

typedef struct sw_wayland_surface_out {
	sw_wayland_surface_out__t _;
	int32_t width, height, scale;
	sw_wayland_output_transform_t transform;
} sw_wayland_surface_out_t;

struct sw_wayland_surface {
    sw_wayland_surface_in_t in;
	sw_wayland_surface_out_t out;
	SW__PRIVATE_FIELDS(160);
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

typedef uint32_t sw_layout_block_fill_mask_t;

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

typedef su_bool32_t (*sw_layout_block_prepare_func_t)(sw_layout_block_t *, sw_context_t *);

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
	sw_layout_block_fill_mask_t fill;
	int32_t x, y;
	int32_t min_width, max_width;
	int32_t min_height, max_height;
	int32_t content_width, content_height;
	sw_layout_block_content_repeat_t content_repeat;
	sw_layout_block_content_anchor_t content_anchor;
	sw_layout_block_content_transform_t content_transform;
	sw_layout_block_border_t borders[4]; /* left right bottom top */
	sw_layout_block_prepare_func_t prepare, prepared; /* ? TODO: macros */
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
} sw_layout_block_out_t;

struct sw_layout_block {
	sw_layout_block_in_t in;
	sw_layout_block_out_t out;
	SW__PRIVATE_FIELDS(16);
	SU_LLIST_NODE_FIELDS(sw_layout_block_t);
};

typedef enum sw_backend_type {
	SW_BACKEND_TYPE_NONE = 0 /* cleanup everythign except event queue(TODO) */
#if SW_WITH_MEMORY_BACKEND
	,SW_BACKEND_TYPE_MEMORY = 1
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
	,SW_BACKEND_TYPE_WAYLAND = 2
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
	sw_wayland_surfaces_t toplevels;
	sw_wayland_surfaces_t layers;
	sw_wayland_surfaces_t surfaces_to_destroy;
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
	struct pollfd *fds; /* TODO: generic structure to support select, epoll, kqueue etc */
	size_t fds_count;
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

/* TODO: strings */
typedef enum sw_event_type {
#if SW_WITH_WAYLAND_BACKEND
	SW_EVENT_WAYLAND_OUTPUT_CREATE = 0,
	SW_EVENT_WAYLAND_OUTPUT_DESTROY = 1,
	SW_EVENT_WAYLAND_SEAT_CREATE = 2,
	SW_EVENT_WAYLAND_SEAT_DESTROY = 3,
	SW_EVENT_WAYLAND_POINTER_CREATE = 4,
	SW_EVENT_WAYLAND_POINTER_DESTROY = 5,
	SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK = 6, /*  ? TODO: merge */
	SW_EVENT_WAYLAND_SURFACE_ERROR_LAYOUT_FAILED = 7, /*  ? TODO: merge */
	SW_EVENT_WAYLAND_SURFACE_ERROR_MISSING_PROTOCOL = 8,
	SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER = 9,
	SW_EVENT_WAYLAND_SURFACE_FAILED_TO_SET_CURSOR_SHAPE = 10,
	SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_FAILED_TO_SET_DECORATIONS = 11,
	SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_CLOSE = 12,
	SW_EVENT_WAYLAND_SURFACE_DESTROY = 13,
	SW_EVENT_WAYLAND_POINTER_ENTER = 14,
	SW_EVENT_WAYLAND_POINTER_LEAVE = 15,
	SW_EVENT_WAYLAND_POINTER_MOTION = 16,
	SW_EVENT_WAYLAND_POINTER_BUTTON = 17,
	SW_EVENT_WAYLAND_POINTER_SCROLL = 18,
#if SW_WITH_WAYLAND_KEYBOARD
	SW_EVENT_WAYLAND_KEYBOARD_CREATE = 19,
	SW_EVENT_WAYLAND_KEYBOARD_DESTROY = 20,
	SW_EVENT_WAYLAND_KEYBOARD_ENTER = 21,
	SW_EVENT_WAYLAND_KEYBOARD_LEAVE = 22,
	SW_EVENT_WAYLAND_KEYBOARD_KEY = 23,
	SW_EVENT_WAYLAND_KEYBOARD_KEY_REPEAT = 24,
	SW_EVENT_WAYLAND_KEYBOARD_MOD = 25,
#endif /* SW_WITH_WAYLAND_KEYBOARD */
#if SW_WITH_WAYLAND_CLIPBOARD
	SW_EVENT_WAYLAND_DATA_DEVICE_CREATE = 26,
	SW_EVENT_WAYLAND_DATA_DEVICE_DESTROY = 27,
	SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_MIME_OFFERS = 28,
	SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_DATA = 29,
	SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ACTION = 30,
	SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_SOURCE_ACTIONS = 31,
	SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ENTER = 32,
	SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_LEAVE = 33,
	SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_MOTION = 34,
	SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_DROP = 35,
	SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_FINISHED = 36,
	SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_CANCELLED = 37,
	SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_DROP_PERFORMED = 38,
	SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_ACTION = 39,
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
#endif /* SW_WITH_WAYLAND_BACKEND */
#if SW_WITH_TEXT
	SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_FONT = 40,
	SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_TEXT = 41,
#endif /* SW_WITH_TEXT */
	SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_IMAGE = 42,
	SW_EVENT_LAYOUT_BLOCK_DESTROY = 43
} sw_event_type_t;

typedef union sw_event_in {
#if SW_WITH_WAYLAND_BACKEND
	sw_wayland_output_t *wayland_output;
	sw_wayland_seat_t *wayland_seat;
	sw_wayland_pointer_t *wayland_pointer;
#if SW_WITH_WAYLAND_KEYBOARD
	sw_wayland_keyboard_t *wayland_keyboard;
#endif /* SW_WITH_WAYLAND_KEYBOARD */
#if SW_WITH_WAYLAND_CLIPBOARD
	sw_wayland_data_device_t *wayland_data_device;
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
#endif /* SW_WITH_WAYLAND_BACKEND */
	void *data;
} sw_event_in_t;

typedef union sw_event_out_ {
	sw_layout_block_t *layout_block;
#if SW_WITH_WAYLAND_BACKEND
	sw_wayland_output_t *wayland_output;
	sw_wayland_seat_t *wayland_seat;
	sw_wayland_surface_t *wayland_surface;
	sw_event_wayland_pointer_out_t wayland_pointer;
#if SW_WITH_WAYLAND_KEYBOARD
	sw_event_wayland_keyboard_out_t wayland_keyboard;
#endif /* SW_WITH_WAYLAND_KEYBOARD */
#if SW_WITH_WAYLAND_CLIPBOARD
	sw_event_wayland_data_device_out_t wayland_data_device;
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
#endif /* SW_WITH_WAYLAND_BACKEND */
	void *data;
} sw_event_out__t;

typedef struct sw_event_out {
	sw_event_out__t _; /* must be first */
	sw_event_type_t type;
	SU_PAD32;
} sw_event_out_t;

typedef struct sw_event {
	sw_event_out_t out; /* must be first */
	sw_event_in_t in;
} sw_event_t;

typedef struct sw_context_in {
	const su_allocator_t *gp_alloc;
	const su_allocator_t *scratch_alloc;
	su_bool32_t update_and_render;
	sw_backend_type_t backend_type;
	sw_backend_in_t backend;
	sw_layout_blocks_t blocks_to_destroy;
} sw_context_in_t;

typedef struct sw_context_out {
	int64_t t; /* absolute timeout in ms at which to call sw_set */
	sw_event_t *events; /* TODO: rework ownership */
	size_t events_count;
	sw_backend_out_t backend;
} sw_context_out_t;

struct sw_context {
	sw_context_in_t in;
	sw_context_out_t out;
	SW__PRIVATE_FIELDS(128);
};

/* ? TODO: strip by default, flag to enable */
#if defined(SW_STRIP_PREFIXES)
#if SW_WITH_MEMORY_BACKEND

typedef sw_backend_memory_in_t backend_memory_in_t;
typedef sw_backend_memory_out_t backend_memory_out_t;

#endif /* SW_WITH_MEMORY_BACKEND */

#if SW_WITH_WAYLAND_BACKEND

typedef sw_backend_wayland_in_t backend_wayland_in_t;
typedef sw_backend_wayland_out_t backend_wayland_out_t;

typedef sw_wayland_surfaces_t wayland_surfaces_t;
typedef sw_layout_blocks_t layout_blocks_t;
typedef sw_wayland_outputs_t wayland_outputs_t;
typedef sw_wayland_seats_t wayland_seats_t;

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
typedef sw_wayland_pointer_button_state_t wayland_pointer_button_state_t;
#define WAYLAND_POINTER_BUTTON_STATE_RELEASED SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED
#define WAYLAND_POINTER_BUTTON_STATE_PRESSED SW_WAYLAND_POINTER_BUTTON_STATE_PRESSED
typedef sw_wayland_pointer_scroll_axis_t wayland_pointer_scroll_axis_t;
#define WAYLAND_POINTER_AXIS_VERTICAL_SCROLL SW_WAYLAND_POINTER_AXIS_VERTICAL_SCROLL
#define WAYLAND_POINTER_AXIS_HORIZONTAL_SCROLL SW_WAYLAND_POINTER_AXIS_HORIZONTAL_SCROLL
typedef sw_wayland_pointer_out_t wayland_pointer_out_t;
typedef sw_wayland_pointer_t wayland_pointer_t;
#if SW_WITH_WAYLAND_KEYBOARD
typedef sw_wayland_keyboard_t wayland_keyboard_t;
typedef sw_wayland_keyboard_key_t wayland_keyboard_key_t;
typedef sw_wayland_keyboard_out_t wayland_keyboard_out_t;
typedef sw_wayland_keyboard_mod_t wayland_keyboard_mod_t;
typedef sw_wayland_keyboard_mod_mask_t wayland_keyboard_mod_mask_t;
#define WAYLAND_KEYBOARD_MOD_SHIFT SW_WAYLAND_KEYBOARD_MOD_SHIFT
#define WAYLAND_KEYBOARD_MOD_CAPS SW_WAYLAND_KEYBOARD_MOD_CAPS
#define WAYLAND_KEYBOARD_MOD_CTRL SW_WAYLAND_KEYBOARD_MOD_CTRL
#define WAYLAND_KEYBOARD_MOD_1 SW_WAYLAND_KEYBOARD_MOD_1
#define WAYLAND_KEYBOARD_MOD_2 SW_WAYLAND_KEYBOARD_MOD_2
#define WAYLAND_KEYBOARD_MOD_3 SW_WAYLAND_KEYBOARD_MOD_3
#define WAYLAND_KEYBOARD_MOD_4 SW_WAYLAND_KEYBOARD_MOD_4
#define WAYLAND_KEYBOARD_MOD_5 SW_WAYLAND_KEYBOARD_MOD_5
#define WAYLAND_KEYBOARD_MOD_VALT SW_WAYLAND_KEYBOARD_MOD_VALT
#define WAYLAND_KEYBOARD_MOD_VHYPER SW_WAYLAND_KEYBOARD_MOD_VHYPER
#define WAYLAND_KEYBOARD_MOD_VLEVEL3 SW_WAYLAND_KEYBOARD_MOD_VLEVEL3
#define WAYLAND_KEYBOARD_MOD_VLEVEL5 SW_WAYLAND_KEYBOARD_MOD_VLEVEL5
#define WAYLAND_KEYBOARD_MOD_VMETA SW_WAYLAND_KEYBOARD_MOD_VMETA
#define WAYLAND_KEYBOARD_MOD_VNUM SW_WAYLAND_KEYBOARD_MOD_VNUM
#define WAYLAND_KEYBOARD_MOD_VSCROLL SW_WAYLAND_KEYBOARD_MOD_VSCROLL
#define WAYLAND_KEYBOARD_MOD_VSUPER SW_WAYLAND_KEYBOARD_MOD_VSUPER
typedef sw_wayland_keyboard_key_state_t wayland_keyboard_key_state_t;
#define WAYLAND_KEYBOARD_KEY_STATE_RELEASED SW_WAYLAND_KEYBOARD_KEY_STATE_RELEASED
#define WAYLAND_KEYBOARD_KEY_STATE_PRESSED SW_WAYLAND_KEYBOARD_KEY_STATE_PRESSED
#define WAYLAND_KEYBOARD_KEY_STATE_REPEATED SW_WAYLAND_KEYBOARD_KEY_STATE_REPEATED
#endif /* SW_WITH_WAYLAND_KEYBOARD */
typedef sw_wayland_seat_out_t wayland_seat_out_t;
typedef sw_wayland_seat_t wayland_seat_t;
typedef sw_wayland_surface_layer_keyboard_interactivity_t wayland_surface_layer_keyboard_interactivity_t;
#define WAYLAND_SURFACE_LAYER_KEYBOARD_INTERACTIVITY_NONE SW_WAYLAND_SURFACE_LAYER_KEYBOARD_INTERACTIVITY_NONE
#define WAYLAND_SURFACE_LAYER_KEYBOARD_INTERACTIVITY_EXCLUSIVE SW_WAYLAND_SURFACE_LAYER_KEYBOARD_INTERACTIVITY_EXCLUSIVE
#define WAYLAND_SURFACE_LAYER_KEYBOARD_INTERACTIVITY_ON_DEMAND SW_WAYLAND_SURFACE_LAYER_KEYBOARD_INTERACTIVITY_ON_DEMAND
typedef sw_wayland_surface_layer_anchor_t wayland_surface_layer_anchor_t;
typedef sw_wayland_surface_layer_anchor_mask_t wayland_surface_layer_anchor_mask_t;
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
typedef sw_wayland_surface_popup_constraint_adjustment_mask_t wayland_surface_popup_constraint_adjustment_mask_t;
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
#define WAYLAND_SURFACE_TYPE_CURSOR_IMAGE SW_WAYLAND_SURFACE_TYPE_CURSOR_IMAGE
typedef sw_wayland_surface_toplevel_state_t wayland_surface_toplevel_state_t;
typedef sw_wayland_surface_toplevel_state_mask_t wayland_surface_toplevel_state_mask_t;
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
typedef sw_wayland_surface_cursor_image_t wayland_surface_cursor_image_t;
typedef sw_wayland_surface_in__t wayland_surface_in__t;
typedef sw_wayland_surface_in_t wayland_surface_in_t;
typedef sw_wayland_surface_out_toplevel_t wayland_surface_out_toplevel_t;
typedef sw_wayland_surface_out__t wayland_surface_out__t;
typedef sw_wayland_surface_out_t wayland_surface_out_t;
typedef sw_wayland_surface_t wayland_surface_t;

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
typedef sw_layout_block_fill_mask_t layout_block_fill_mask_t;
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
typedef sw_layout_block_prepare_func_t layout_block_prepare_func_t;

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

typedef sw_event_in_t event_in_t;
typedef sw_event_out_t event_out_t;
typedef sw_event_type_t event_type_t;
typedef sw_event_out__t event_out__t;
#define EVENT_LAYOUT_BLOCK_DESTROY SW_EVENT_LAYOUT_BLOCK_DESTROY
#define EVENT_LAYOUT_BLOCK_ERROR_INVALID_IMAGE SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_IMAGE
#if SW_WITH_TEXT
#define EVENT_LAYOUT_BLOCK_ERROR_INVALID_FONT SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_FONT
#define EVENT_LAYOUT_BLOCK_ERROR_INVALID_TEXT SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_TEXT
#endif /* SW_WITH_TEXT */
#if SW_WITH_WAYLAND_BACKEND
#define EVENT_WAYLAND_OUTPUT_CREATE SW_EVENT_WAYLAND_OUTPUT_CREATE
#define EVENT_WAYLAND_OUTPUT_DESTROY SW_EVENT_WAYLAND_OUTPUT_DESTROY
#define EVENT_WAYLAND_SEAT_CREATE SW_EVENT_WAYLAND_SEAT_CREATE
#define EVENT_WAYLAND_SEAT_DESTROY SW_EVENT_WAYLAND_SEAT_DESTROY
#define EVENT_WAYLAND_POINTER_CREATE SW_EVENT_WAYLAND_POINTER_CREATE
#define EVENT_WAYLAND_POINTER_DESTROY SW_EVENT_WAYLAND_POINTER_DESTROY
#define EVENT_WAYLAND_SURFACE_DESTROY SW_EVENT_WAYLAND_SURFACE_DESTROY
#define EVENT_WAYLAND_SURFACE_TOPLEVEL_CLOSE SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_CLOSE
#define EVENT_WAYLAND_SURFACE_FAILED_TO_SET_CURSOR_SHAPE SW_EVENT_WAYLAND_SURFACE_FAILED_TO_SET_CURSOR_SHAPE
#define EVENT_WAYLAND_SURFACE_TOPLEVEL_FAILED_TO_SET_DECORATIONS SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_FAILED_TO_SET_DECORATIONS
#define EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK
#define EVENT_WAYLAND_SURFACE_ERROR_LAYOUT_FAILED SW_EVENT_WAYLAND_SURFACE_ERROR_LAYOUT_FAILED
#define EVENT_WAYLAND_SURFACE_ERROR_MISSING_PROTOCOL SW_EVENT_WAYLAND_SURFACE_ERROR_MISSING_PROTOCOL
#define EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER
#define EVENT_WAYLAND_POINTER_ENTER SW_EVENT_WAYLAND_POINTER_ENTER
#define EVENT_WAYLAND_POINTER_LEAVE SW_EVENT_WAYLAND_POINTER_LEAVE
#define EVENT_WAYLAND_POINTER_MOTION SW_EVENT_WAYLAND_POINTER_MOTION
#define EVENT_WAYLAND_POINTER_BUTTON SW_EVENT_WAYLAND_POINTER_BUTTON
#define EVENT_WAYLAND_POINTER_SCROLL SW_EVENT_WAYLAND_POINTER_SCROLL
typedef sw_event_wayland_pointer_out_t event_wayland_pointer_out_t;
#if SW_WITH_WAYLAND_KEYBOARD
#define EVENT_WAYLAND_KEYBOARD_CREATE SW_EVENT_WAYLAND_KEYBOARD_CREATE
#define EVENT_WAYLAND_KEYBOARD_DESTROY SW_EVENT_WAYLAND_KEYBOARD_DESTROY
#define EVENT_WAYLAND_KEYBOARD_ENTER SW_EVENT_WAYLAND_KEYBOARD_ENTER
#define EVENT_WAYLAND_KEYBOARD_LEAVE SW_EVENT_WAYLAND_KEYBOARD_LEAVE
#define EVENT_WAYLAND_KEYBOARD_KEY SW_EVENT_WAYLAND_KEYBOARD_KEY
#define EVENT_WAYLAND_KEYBOARD_KEY_REPEAT SW_EVENT_WAYLAND_KEYBOARD_KEY_REPEAT
#define EVENT_WAYLAND_KEYBOARD_MOD SW_EVENT_WAYLAND_KEYBOARD_MOD
typedef sw_event_wayland_keyboard_out_t event_wayland_keyboard_out_t;
#endif /* SW_WITH_WAYLAND_KEYBOARD */
#if SW_WITH_WAYLAND_CLIPBOARD
#define EVENT_WAYLAND_DATA_DEVICE_CREATE SW_EVENT_WAYLAND_DATA_DEVICE_CREATE
#define EVENT_WAYLAND_DATA_DEVICE_DESTROY SW_EVENT_WAYLAND_DATA_DEVICE_DESTROY
#define EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_MIME_OFFERS SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_MIME_OFFERS
#define EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_DATA SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_DATA
#define EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ACTION SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ACTION
#define EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_SOURCE_ACTIONS SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_SOURCE_ACTIONS
#define EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ENTER SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ENTER
#define EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_LEAVE SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_LEAVE
#define EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_MOTION SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_MOTION
#define EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_DROP SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_DROP
#define EVENT_WAYLAND_DATA_DEVICE_COPY_DND_FINISHED SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_FINISHED
#define EVENT_WAYLAND_DATA_DEVICE_COPY_DND_CANCELLED SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_CANCELLED
#define EVENT_WAYLAND_DATA_DEVICE_COPY_DND_DROP_PERFORMED SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_DROP_PERFORMED
#define EVENT_WAYLAND_DATA_DEVICE_COPY_DND_ACTION SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_ACTION
typedef sw_event_wayland_data_device_out_t event_wayland_data_device_out_t;
typedef sw_wayland_data_device_t wayland_data_device_t;
typedef sw_wayland_data_device_dnd_action_t wayland_data_device_dnd_action_t;
#define WAYLAND_DATA_DEVICE_DND_ACTION_NONE SW_WAYLAND_DATA_DEVICE_DND_ACTION_NONE
#define WAYLAND_DATA_DEVICE_DND_ACTION_COPY SW_WAYLAND_DATA_DEVICE_DND_ACTION_COPY
#define WAYLAND_DATA_DEVICE_DND_ACTION_MOVE SW_WAYLAND_DATA_DEVICE_DND_ACTION_MOVE
#define WAYLAND_DATA_DEVICE_DND_ACTION_ASK SW_WAYLAND_DATA_DEVICE_DND_ACTION_ASK
typedef sw_wayland_data_device_dnd_action_mask_t wayland_data_device_dnd_action_mask_t;
typedef sw_wayland_data_device_out_paste_t wayland_data_device_out_paste_t;
typedef sw_wayland_data_device_out_copy_t wayland_data_device_out_copy_t;
typedef sw_wayland_data_device_out_t wayland_data_device_out_t;
typedef sw_wayland_data_device_in_copy_t wayland_data_device_in_copy_t;
typedef sw_wayland_data_device_in_paste_t wayland_data_device_in_paste_t;
typedef sw_wayland_data_device_in_t wayland_data_device_in_t;
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
#endif /* SW_WITH_WAYLAND_BACKEND */

/*#define set sw_set */

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

#if SW_WITH_WAYLAND_KEYBOARD
	#if SU_HAS_INCLUDE(<xkbcommon/xkbcommon.h>)
		#include <xkbcommon/xkbcommon.h>
	#else
		#include <xkbcommon.h>
	#endif
#endif /* SW_WITH_WAYLAND_KEYBOARD */

SU_IGNORE_WARNINGS_END

typedef enum sw__wayland_object_state {
	SW__WAYLAND_OBJECT_STATE_UNINITIALIZED,
	SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER,
	SW__WAYLAND_OBJECT_STATE_OWNED_BY_SW
} sw__wayland_object_state_t;

typedef struct sw__wayland_output {
	sw__wayland_object_state_t state;
	uint32_t wl_name;
	struct wl_output *wl_output;
} sw__wayland_output_t;

typedef struct sw__wayland_pointer {
	struct wl_pointer *wl_pointer;
	struct wp_cursor_shape_device_v1 *cursor_shape_device;
	uint32_t enter_serial;
	sw__wayland_object_state_t state;
} sw__wayland_pointer_t;

#if SW_WITH_WAYLAND_KEYBOARD
typedef struct sw__wayland_keyboard {
	struct wl_keyboard *wl_keyboard;
	struct xkb_context *xkb_context;
	struct xkb_keymap *xkb_keymap;
	struct xkb_state *xkb_state;
	uint32_t repeat_cp;
	sw__wayland_object_state_t state;
	int64_t repeat_next; /* absolute ms */
} sw__wayland_keyboard_t;
#endif /* SW_WITH_WAYLAND_KEYBOARD */

#if SW_WITH_WAYLAND_CLIPBOARD
typedef struct sw__wayland_data_device_paste {
	struct wl_data_offer *wl_data_offer;
	struct pollfd pfd;
	su_string_t mime_type;
	uint32_t dnd_enter_serial;
	sw_wayland_data_device_dnd_action_mask_t dnd_actions;
	sw_wayland_data_device_dnd_action_t dnd_preferred_action;
	su_bool32_t dnd_got_action; /* TODO: rework/remove */
	size_t data_capacity;
	size_t offered_mime_types_capacity;
} sw__wayland_data_device_paste_t;

typedef struct sw__wayland_data_device_copy {
	struct wl_data_source *wl_data_source;
	su_string_t mime_type;
	struct pollfd pfd;
	size_t buf_idx;
	su_fat_ptr_t data;
} sw__wayland_data_device_copy_t;

typedef struct sw__wayland_data_device {
	sw__wayland_data_device_paste_t paste;
	sw__wayland_data_device_copy_t copy;
	SU_PAD32;
	sw__wayland_object_state_t state;
	struct wl_data_device *wl_data_device;
} sw__wayland_data_device_t;
#endif /* SW_WITH_WAYLAND_CLIPBOARD */

typedef struct sw__wayland_seat {
	struct wl_seat *wl_seat;
	uint32_t wl_name;
	sw__wayland_object_state_t state;
	uint32_t pointer_serial;
	uint32_t keyboard_serial;
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
	sw_wayland_surface_layer_anchor_mask_t anchor;
	sw_wayland_surface_layer_layer_t layer;
	int32_t margins[4]; /* top right bottom left */
	sw_wayland_surface_layer_keyboard_interactivity_t keyboard_interactivity;
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
    sw_wayland_surface_popup_constraint_adjustment_mask_t constraint_adjustment;
} sw__wayland_surface_popup_t;

typedef struct sw__wayland_surface_cursor_image {
	int32_t offset_x, offset_y;
	su_bool32_t set_offset;
	SU_PAD32;
} sw__wayland_surface_cursor_image_t;

typedef union sw__wayland_surface_ {
	sw__wayland_surface_toplevel_t toplevel;
	sw__wayland_surface_layer_t layer;
	sw__wayland_surface_popup_t popup;
	sw__wayland_surface_cursor_image_t cursor_image;
} sw__wayland_surface__t;

typedef enum sw__wayland_surface_state {
	SW__WAYLAND_SURFACE_STATE_ALIVE,
	SW__WAYLAND_SURFACE_STATE_DESTROYED,
	SW__WAYLAND_SURFACE_STATE_DIRTY
} sw__wayland_surface_state_t;

typedef struct sw__wayland_surface {
	sw__wayland_surface__t _;

	/* ? TODO: since this is ignored for cursor_shape surfaces, move this to sw__wayland_surface_layer,toplevel,popup_t */
    sw_wayland_region_t *input_regions;
	size_t input_regions_count;
	sw_wayland_cursor_shape_t cursor_shape;

	sw__wayland_surface_state_t state;
	struct wl_surface *wl_surface;
	sw__wayland_surface_buffer_t buffer;
} sw__wayland_surface_t;

#endif /* SW_WITH_WAYLAND_BACKEND */

typedef enum sw__layout_block_state {
	SW__LAYOUT_BLOCK_STATE_ALIVE,
	SW__LAYOUT_BLOCK_STATE_DESTROYED
} sw__layout_block_state_t;

typedef struct sw__layout_block {
	pixman_image_t *content_image;
	sw__layout_block_state_t state;
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
SU_HASH_TABLE_DECLARE_DEFINE(sw__image_cache_t, su_fat_ptr_t, su_stbds_hash, su_fat_ptr_equal, SU_MEMCPY, 16)

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
SU_HASH_TABLE_DECLARE_DEFINE(sw__text_run_cache_t, su_string_t, su_stbds_hash_string, su_string_equal, SU_MEMCPY, 16)
#endif /* SW_WITH_TEXT */

#if SW_WITH_MEMORY_BACKEND
typedef struct sw__backend_memory {
	pixman_image_t *image;
} sw__backend_memory_t;
#endif /* SW_WITH_MEMORY_BACKEND */

#if SW_WITH_WAYLAND_BACKEND
typedef struct sw__backend_wayland {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct xdg_wm_base *wm_base;
	struct wp_cursor_shape_manager_v1 *cursor_shape_manager;
	struct zxdg_decoration_manager_v1 *decoration_manager;
#if SW_WITH_WAYLAND_CLIPBOARD
	struct wl_data_device_manager *data_device_manager;
	size_t fds_capacity;
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
} sw__backend_wayland_t;
#endif /* SW_WITH_WAYLAND_BACKEND */

typedef union sw__backend {
#if SW_WITH_MEMORY_BACKEND
	sw__backend_memory_t memory;
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
	sw__backend_wayland_t wayland;
#endif /* SW_WITH_WAYLAND_BACKEND */
} sw__backend_t;

typedef struct sw__context {
	sw__backend_t backend;
	sw_backend_type_t backend_type;
	su_bool32_t check_events;
	size_t events_capacity;
	/* ? TODO: arena for content: */
	su_hash_table__sw__image_cache_t__t image_cache;
#if SW_WITH_TEXT
	su_hash_table__sw__text_run_cache_t__t text_run_cache;
#endif /* SW_WITH_TEXT */
} sw__context_t;

/* ? TODO: option to remove tls */
static SU_THREAD_LOCAL sw_context_t *sw__context;

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

SU_STATIC_ASSERT(sizeof(sw__context->sw__private) >= sizeof(sw__context_t));

#if SW_WITH_WAYLAND_BACKEND
SU_STATIC_ASSERT(sizeof(sw__context->out.backend.wayland.outputs.head->sw__private) >= sizeof(sw__wayland_output_t));
SU_STATIC_ASSERT(sizeof(sw__context->out.backend.wayland.seats.head->sw__private) >= sizeof(sw__wayland_seat_t));
SU_STATIC_ASSERT(sizeof(sw__context->out.backend.wayland.seats.head->out.pointer->sw__private) >= sizeof(sw__wayland_pointer_t));
#if SW_WITH_WAYLAND_KEYBOARD
SU_STATIC_ASSERT(sizeof(sw__context->out.backend.wayland.seats.head->out.keyboard->sw__private) >= sizeof(sw__wayland_keyboard_t));
#endif /* SW_WITH_WAYLAND_KEYBOARD */
#if SW_WITH_WAYLAND_CLIPBOARD
SU_STATIC_ASSERT(sizeof(sw__context->out.backend.wayland.seats.head->out.data_device->sw__private) >= sizeof(sw__wayland_data_device_t));
#endif /* SW_WITH_WAYLAND_CLIPBOARD */  
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
	const su_allocator_t *scratch_alloc = sw__context->in.scratch_alloc;
	void *ret;
	SU_ALLOCTSA(ret, scratch_alloc, new_size, 32);
	if (ptr) {
		SU_MEMCPY(ret, ptr, SU_MIN(old_size, new_size));
		SU_FREE(scratch_alloc, ptr);
	}
	return ret;
}
#endif /* SW_WITH_PNG || SW_WITH_JPG || SW_WITH_TGA || SW_WITH_BMP || SW_WITH_PSD || SW_WITH_GIF || SW_WITH_HDR || SW_WITH_PIC || SW_WITH_PNM */

static sw_event_out__t *sw__event(sw_event_type_t type, void *data) { /* TODO: replace void* */
	sw_event_t *event;

	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	const su_allocator_t *gp_alloc = sw__context->in.gp_alloc;

	if (SU_UNLIKELY(sw_priv->events_capacity == sw__context->out.events_count)) {
		sw_event_t *new_events;
		sw_priv->events_capacity *= 2;
		SU_ARRAY_ALLOC(new_events, gp_alloc, sw_priv->events_capacity);
		SU_MEMCPY(new_events, sw__context->out.events, sw__context->out.events_count * sizeof(sw__context->out.events[0]));
		SU_FREE(gp_alloc, sw__context->out.events);
		sw__context->out.events = new_events;
	}

	event = &sw__context->out.events[sw__context->out.events_count++];
	SU_MEMSET(event, 0, sizeof(*event));
	event->out.type = type;
	event->out._.data = data;

	return &event->out._;
}

static void sw__update_t(int64_t t) {
	SU_ASSERT(t > 0);
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

static pixman_image_t *sw__color_to_pixman_image(sw_color_t color, const su_allocator_t *scratch_alloc) {
	pixman_image_t *image;

	switch (color.type) {
	case SW_COLOR_TYPE_ARGB32: {
		pixman_color_t c = sw__color_argb32_to_pixman_color(color._.argb32);
		image = pixman_image_create_solid_fill(&c);
		break;
	}
	case SW_COLOR_TYPE_LINEAR_GRADIENT: {
		sw_color_linear_gradient_t linear_gradient = color._.linear_gradient;
		pixman_gradient_stop_t *stops;
		sw_color_gradient_stop_t prev_stop = linear_gradient.stops[0];
		pixman_point_fixed_t p1, p2;
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
		break;
	}
	case SW_COLOR_TYPE_CONICAL_GRADIENT: {
		sw_color_conical_gradient_t conical_gradient = color._.conical_gradient;
		pixman_gradient_stop_t *stops;
		sw_color_gradient_stop_t prev_stop = conical_gradient.stops[0];
		size_t i = 0;
		pixman_point_fixed_t center;

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
		break;
	}
	case SW_COLOR_TYPE_RADIAL_GRADIENT: {
		sw_color_radial_gradient_t radial_gradient = color._.radial_gradient;
		pixman_gradient_stop_t *stops;
		sw_color_gradient_stop_t prev_stop = radial_gradient.stops[0];
		size_t i = 0;
		pixman_point_fixed_t p_inner, p_outer;

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
		break;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}

	return image;
}

static void sw__image_handle_destroy(pixman_image_t *image, void *data) {
	const su_allocator_t *gp_alloc = sw__context->in.gp_alloc;
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

static pixman_image_t *sw__image_create( const su_allocator_t *gp_alloc,
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

static pixman_image_t *sw__load_pixmap(const su_allocator_t *gp_alloc, su_fat_ptr_t data) {
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
static pixman_image_t *sw__render_svg(const su_allocator_t *gp_alloc, resvg_render_tree *tree,
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

static pixman_image_t *sw__load_svg(const su_allocator_t *gp_alloc, su_fat_ptr_t data) {
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
static pixman_image_t *sw__load_png(const su_allocator_t *gp_alloc, const su_allocator_t *scratch_alloc,
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
static pixman_image_t *sw__load_jpg(const su_allocator_t *gp_alloc, const su_allocator_t *scratch_alloc,
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
static pixman_image_t *sw__load_tga(const su_allocator_t *gp_alloc, const su_allocator_t *scratch_alloc,
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
static pixman_image_t *sw__load_bmp(const su_allocator_t *gp_alloc, const su_allocator_t *scratch_alloc,
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
static pixman_image_t *sw__load_psd(const su_allocator_t *gp_alloc, const su_allocator_t *scratch_alloc,
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
static pixman_image_t *sw__load_gif(const su_allocator_t *gp_alloc, const su_allocator_t *scratch_alloc,
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
static pixman_image_t *sw__load_hdr(const su_allocator_t *gp_alloc, const su_allocator_t *scratch_alloc,
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
static pixman_image_t *sw__load_pic(const su_allocator_t *gp_alloc, const su_allocator_t *scratch_alloc,
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
static pixman_image_t *sw__load_pnm(const su_allocator_t *gp_alloc, const su_allocator_t *scratch_alloc,
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

static su_bool32_t sw__layout_block_fini(sw_layout_block_t *block, su_bool32_t destroy_children) {
	/* TODO: remove recursion */

	sw__layout_block_t *block_priv = (sw__layout_block_t *)&block->sw__private;
	su_bool32_t ret = (block_priv->state != SW__LAYOUT_BLOCK_STATE_DESTROYED);

	if ((block->in.type == SW_LAYOUT_BLOCK_TYPE_COMPOSITE) && destroy_children) {
		sw_layout_block_t *block_;
		for ( block_ = block->in._.composite.children.head; block_; block_ = block_->next) {
			if (sw__layout_block_fini(block_, SU_TRUE)) {
				sw__event(SW_EVENT_LAYOUT_BLOCK_DESTROY, block_);
			}
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

	block_priv->state = SW__LAYOUT_BLOCK_STATE_DESTROYED;
	return ret;
}

static su_bool32_t sw__layout_block_init(sw_layout_block_t *block) {
	/* TODO: remove recursion */

	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw__layout_block_t *block_priv = (sw__layout_block_t *)&block->sw__private;
	const su_allocator_t *gp_alloc = sw__context->in.gp_alloc;
	const su_allocator_t *scratch_alloc = sw__context->in.scratch_alloc;
	sw_event_type_t error;

	SU_NOTUSED(scratch_alloc);

	sw__layout_block_fini(block, SU_FALSE);

	block_priv->state = SW__LAYOUT_BLOCK_STATE_ALIVE;

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
			error = SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_FONT;
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
					error = SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_TEXT;
					goto error;
				default:
					consumed += ret;
				}
			}

			text_run = fcft_rasterize_text_run_utf32(font, c32_count, (uint32_t *)c32, FCFT_SUBPIXEL_NONE);
			SU_FREE(scratch_alloc, c32);
			if ((text_run == NULL) || (text_run->count == 0)) {
				fcft_text_run_destroy(text_run);
				error = SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_TEXT;
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
			error = SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_TEXT;
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
				error = SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_IMAGE;
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
	sw__layout_block_fini(block, SU_TRUE);
	sw__event(error, block);
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

	if (SU_UNLIKELY(block_priv->state != SW__LAYOUT_BLOCK_STATE_ALIVE) 
			|| (fill == SW_LAYOUT_BLOCK_FILL_NONE)) {
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

	if (SU_UNLIKELY(block_priv->state != SW__LAYOUT_BLOCK_STATE_ALIVE)) {
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
			sw__layout_block_t *b_priv = (sw__layout_block_t *)&b->sw__private;
			if (SU_UNLIKELY(b_priv->state != SW__LAYOUT_BLOCK_STATE_ALIVE)) {
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
			if (SU_UNLIKELY(b_priv->state != SW__LAYOUT_BLOCK_STATE_ALIVE)) {
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

	if (block->in.prepared) {
		return block->in.prepared(block, sw__context);
	}

	return SU_TRUE;
}

static void sw__layout_block_render(sw_layout_block_t *block, pixman_image_t *dest) {
	/* TODO: remove recursion */

	sw__layout_block_t *block_priv = (sw__layout_block_t *)&block->sw__private;
	sw_layout_block_dimensions_t dim = block->out.dim;
	const su_allocator_t *scratch_alloc = sw__context->in.scratch_alloc;
	pixman_image_t *color;

	if (SU_UNLIKELY(block_priv->state != SW__LAYOUT_BLOCK_STATE_ALIVE)) {
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

	SU_MEMSET(buffer, 0, sizeof(*buffer));
}

static su_bool32_t sw__wayland_surface_fini(sw_wayland_surface_t *surface, su_bool32_t destroy_root_block) {
	/* TODO: remove recursion */

	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	su_bool32_t ret = (surface_priv->state != SW__WAYLAND_SURFACE_STATE_DESTROYED);
	sw_wayland_seat_t *seat;
	sw_wayland_surface_t *p;

	if (surface->in.type != SW_WAYLAND_SURFACE_TYPE_CURSOR_IMAGE) {
		for ( p = surface->in.popups.head; p; p = p->next) {
			if (sw__wayland_surface_fini(p, SU_TRUE)) {
				sw__event(SW_EVENT_WAYLAND_SURFACE_DESTROY, p);
			}
		}
		SU_FREE(sw__context->in.gp_alloc, surface_priv->input_regions);
		for ( seat = sw__context->out.backend.wayland.seats.head; seat; seat = seat->next) {
			if (seat->out.pointer && (seat->out.pointer->out.focused_surface == surface)) {
				seat->out.pointer->out.focused_surface = NULL;
			}
		}
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
	case SW_WAYLAND_SURFACE_TYPE_CURSOR_IMAGE:
		break;
	default:
		SU_ASSERT_UNREACHABLE;
	}

	sw__wayland_surface_buffer_fini(&surface_priv->buffer);

	if (surface_priv->wl_surface) {
		wl_surface_destroy(surface_priv->wl_surface);
	}

	if (destroy_root_block) {
		if (sw__layout_block_fini(surface->in.root, SU_TRUE)) {
			sw__event(SW_EVENT_LAYOUT_BLOCK_DESTROY, surface->in.root);
		}
	}

	SU_MEMSET(surface_priv, 0, sizeof(*surface_priv));
	SU_MEMSET(&surface->out, 0, sizeof(surface->out));

	surface_priv->state = SW__WAYLAND_SURFACE_STATE_DESTROYED;
	return ret;
}

static void sw__wayland_surface_popup_init_stage2(sw_wayland_surface_t *);
static su_bool32_t sw__wayland_surface_buffer_init(sw__wayland_surface_buffer_t *,
		sw_wayland_surface_t *, int32_t width, int32_t height);

static su_bool32_t sw__wayland_surface_render(sw_wayland_surface_t *surface) {
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	int32_t surface_width, surface_height;
	sw_layout_block_t *root = surface->in.root;

	if (surface_priv->buffer.busy) {
		surface_priv->state = SW__WAYLAND_SURFACE_STATE_DIRTY;
		return SU_TRUE;
	}

	if (SU_UNLIKELY(!surface_priv->wl_surface) || !sw__layout_block_prepare(root, NULL)) {
		return SU_FALSE;
	}
	if ((root->out.dim.width <= 0) || (root->out.dim.height <= 0)) {
		sw__event(SW_EVENT_WAYLAND_SURFACE_ERROR_LAYOUT_FAILED, surface);
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
	case SW_WAYLAND_SURFACE_TYPE_CURSOR_IMAGE:
		surface_width = (surface->in.width <= 0) ? root->out.dim.width : surface->in.width;
		surface_height = (surface->in.height <= 0) ? root->out.dim.height : surface->in.height;
		if ((surface_width != surface->out.width) || (surface_height != surface->out.height)) {
			sw__wayland_surface_buffer_fini(&surface_priv->buffer);
			if (SU_UNLIKELY(!sw__wayland_surface_buffer_init(&surface_priv->buffer,
					surface, surface_width, surface_height))) {
				return SU_FALSE;
			}
			surface->out.width = surface_width;
			surface->out.height = surface_height;
		}
		break;
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
	surface_priv->state = SW__WAYLAND_SURFACE_STATE_ALIVE;
commit:
	wl_surface_commit(surface_priv->wl_surface);
	return SU_TRUE;
}

static void sw__wayland_surface_buffer_handle_release(void *data, struct wl_buffer *wl_buffer) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;

	SU_NOTUSED(wl_buffer);

	surface_priv->buffer.busy = SU_FALSE;
	if (surface_priv->state == SW__WAYLAND_SURFACE_STATE_DIRTY) {
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

	static const struct wl_buffer_listener wl_buffer_listener = {
		sw__wayland_surface_buffer_handle_release
	};

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

	wl_shm_pool = wl_shm_create_pool(sw_priv->backend.wayland.shm, shm_fd, (int32_t)buffer->size);
	buffer->wl_buffer = wl_shm_pool_create_buffer(
		wl_shm_pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_buffer_add_listener(buffer->wl_buffer, &wl_buffer_listener, surface);
	wl_shm_pool_destroy(wl_shm_pool);
	close(shm_fd);

	buffer->busy = SU_FALSE;

	return SU_TRUE;
error:
	sw__wayland_surface_fini(surface, SU_TRUE);
	sw__event(SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER, surface);
	return SU_FALSE;
}

static void sw__wayland_surface_handle_enter(void *data, struct wl_surface *wl_surface, struct wl_output *output) {
	SU_NOTUSED(data); SU_NOTUSED(wl_surface); SU_NOTUSED(output);
	/* TODO: set in surface->out, event */
}

static void sw__wayland_surface_handle_leave(void *data, struct wl_surface *wl_surface, struct wl_output *output) {
	SU_NOTUSED(data); SU_NOTUSED(wl_surface); SU_NOTUSED(output);
	/* TODO: set in surface->out, event */
}

static void sw__wayland_surface_handle_preferred_buffer_transform(void *data,
		struct wl_surface *wl_surface, uint32_t transform) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;

	SU_NOTUSED(wl_surface);

	surface->out.transform = (sw_wayland_output_transform_t)transform;

	/* ? TODO: event */
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
		surface_priv->state = SW__WAYLAND_SURFACE_STATE_DIRTY;
	}
	/* ? TODO: event */
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
			surface_priv->state = SW__WAYLAND_SURFACE_STATE_DIRTY;
		}
	}

	if (surface_priv->state == SW__WAYLAND_SURFACE_STATE_DIRTY) {
		sw__wayland_surface_render(surface);
	}
}

static void sw__wayland_surface_layer_handle_closed(void *data,
		struct zwlr_layer_surface_v1 *layer_surface) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	SU_NOTUSED(layer_surface);
	if (sw__wayland_surface_fini(surface, SU_TRUE)) {
		sw__event(SW_EVENT_WAYLAND_SURFACE_DESTROY, surface);
	}
}

static void sw__wayland_surface_popup_handle_preferred_buffer_scale(void *data,
		struct wl_surface *wl_surface, int32_t factor) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_popup_t *popup_priv = &surface_priv->_.popup;

	SU_NOTUSED(wl_surface);

	if (surface->out.scale != factor) {
		surface->out.scale = factor;
		surface_priv->state = SW__WAYLAND_SURFACE_STATE_DIRTY;

		xdg_positioner_set_size( popup_priv->xdg_positioner,
			surface->out.width / factor, surface->out.height / factor);
		xdg_positioner_set_anchor_rect(popup_priv->xdg_positioner,
			popup_priv->x / factor, popup_priv->y / factor, 1, 1);
		xdg_popup_reposition(popup_priv->xdg_popup, popup_priv->xdg_positioner, 0);

		wl_surface_commit(surface_priv->wl_surface);
	}

	/* ? TODO: event */
}

static void sw__wayland_xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface,
		uint32_t serial) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;

	SU_NOTUSED(xdg_surface);

	xdg_surface_ack_configure(surface_priv->_.toplevel.xdg_surface, serial);

	if (surface_priv->state == SW__WAYLAND_SURFACE_STATE_DIRTY) {
		sw__wayland_surface_render(surface);
	}
}

static void sw__wayland_surface_handle_preferred_buffer_scale(void *data,
		struct wl_surface *wl_surface, int32_t factor) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	SU_NOTUSED(wl_surface);
	surface->out.scale = factor;
	/* ? TODO: event */
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
		surface->out._.toplevel.states |= (1u << *state);
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
			surface_priv->state = SW__WAYLAND_SURFACE_STATE_DIRTY;
		}
	}
}

static void sw__wayland_surface_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	SU_NOTUSED(xdg_toplevel);
	sw__event(SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_CLOSE, surface);
}

static void sw__wayland_surface_toplevel_handle_decoration_configure( void *data,
		struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1, uint32_t mode) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	sw__wayland_surface_toplevel_t *toplevel_priv = (sw__wayland_surface_toplevel_t *)&surface->sw__private;

	SU_NOTUSED(zxdg_toplevel_decoration_v1);

	if (SU_UNLIKELY((mode != toplevel_priv->decoration_mode) &&
			(toplevel_priv->decoration_mode != SW_WAYLAND_TOPLEVEL_DECORATION_MODE_COMPOSITOR_DEFAULT))) {
		sw__event(SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_FAILED_TO_SET_DECORATIONS, surface);
	}
}

static su_bool32_t sw__wayland_surface_toplevel_init(sw_wayland_surface_t *surface) {
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_toplevel_t *toplevel_priv = &surface_priv->_.toplevel;
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	
	static const struct wl_surface_listener wl_surface_listener = {
		sw__wayland_surface_handle_enter,
		sw__wayland_surface_handle_leave,
		sw__wayland_surface_handle_preferred_buffer_scale,
		sw__wayland_surface_handle_preferred_buffer_transform
	};
	static const struct xdg_surface_listener xdg_surface_listener = {
		sw__wayland_xdg_surface_handle_configure
	};
	static const struct xdg_toplevel_listener xdg_toplevel_listener = {
		sw__wayland_surface_toplevel_handle_configure,
		sw__wayland_surface_toplevel_handle_close,
		NULL,
		NULL
	};
	static const struct zxdg_toplevel_decoration_v1_listener decoration_listener = {
		sw__wayland_surface_toplevel_handle_decoration_configure
	};

	if (SU_UNLIKELY(!sw_priv->backend.wayland.wm_base)) {
		sw__event(SW_EVENT_WAYLAND_SURFACE_ERROR_MISSING_PROTOCOL, surface);
		return SU_FALSE;
	}

	surface->out.scale = 1;
	
	surface_priv->wl_surface = wl_compositor_create_surface(sw_priv->backend.wayland.compositor);
	wl_surface_add_listener(surface_priv->wl_surface, &wl_surface_listener, surface);

	toplevel_priv->xdg_surface = xdg_wm_base_get_xdg_surface(
		sw_priv->backend.wayland.wm_base, surface_priv->wl_surface);
	xdg_surface_add_listener(toplevel_priv->xdg_surface, &xdg_surface_listener, surface);

	toplevel_priv->xdg_toplevel = xdg_surface_get_toplevel(toplevel_priv->xdg_surface);
	xdg_toplevel_add_listener(toplevel_priv->xdg_toplevel, &xdg_toplevel_listener, surface);

	if (sw_priv->backend.wayland.decoration_manager) {
		toplevel_priv->decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
			sw_priv->backend.wayland.decoration_manager, toplevel_priv->xdg_toplevel);
		zxdg_toplevel_decoration_v1_add_listener(
			toplevel_priv->decoration, &decoration_listener, surface);
	}

	surface_priv->state = SW__WAYLAND_SURFACE_STATE_ALIVE;
	return SU_TRUE;
}

static su_bool32_t sw__wayland_surface_layer_init(sw_wayland_surface_t *surface, sw_wayland_surface_layer_layer_t l) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_output_t *output_priv = (sw__wayland_output_t *)&surface->in._.layer.output->sw__private;
	sw__wayland_surface_layer_t *layer_priv = &surface_priv->_.layer;

	static const struct wl_surface_listener wl_surface_listener = {
		sw__wayland_surface_handle_enter,
		sw__wayland_surface_handle_leave,
		sw__wayland_surface_layer_handle_preferred_buffer_scale,
		sw__wayland_surface_handle_preferred_buffer_transform
	};
	static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
		sw__wayland_surface_layer_handle_configure,
		sw__wayland_surface_layer_handle_closed
	};

	if (SU_UNLIKELY(!sw_priv->backend.wayland.layer_shell)) {
		sw__event(SW_EVENT_WAYLAND_SURFACE_ERROR_MISSING_PROTOCOL, surface);
		return SU_FALSE;
	}

	surface->out.scale = surface->in._.layer.output->out.scale;
	layer_priv->exclusive_zone = INT32_MIN;
	layer_priv->anchor = UINT32_MAX;
	layer_priv->layer = l;
	layer_priv->margins[0] = INT32_MIN;
	layer_priv->margins[1] = INT32_MIN;
	layer_priv->margins[2] = INT32_MIN;
	layer_priv->margins[3] = INT32_MIN;
	layer_priv->output = surface->in._.layer.output;

	surface_priv->wl_surface = wl_compositor_create_surface(sw_priv->backend.wayland.compositor);
	wl_surface_add_listener(surface_priv->wl_surface, &wl_surface_listener, surface);
	layer_priv->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
		sw_priv->backend.wayland.layer_shell, surface_priv->wl_surface,
		output_priv->wl_output, l, "sw");
	zwlr_layer_surface_v1_add_listener(layer_priv->layer_surface, &layer_surface_listener, surface);

	surface_priv->state = SW__WAYLAND_SURFACE_STATE_ALIVE;
	return SU_TRUE;
}

static su_bool32_t sw__wayland_surface_popup_init_stage1( sw_wayland_surface_t *surface, sw_wayland_surface_t *parent) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw__wayland_surface_popup_t *popup_priv = &surface_priv->_.popup;

	static const struct wl_surface_listener wl_surface_listener = {
		sw__wayland_surface_handle_enter,
		sw__wayland_surface_handle_leave,
		sw__wayland_surface_popup_handle_preferred_buffer_scale,
		sw__wayland_surface_handle_preferred_buffer_transform,
	};
	static const struct xdg_surface_listener xdg_surface_listener = {
		sw__wayland_xdg_surface_handle_configure
	};

	SU_ASSERT(parent != NULL);

	if (SU_UNLIKELY(!sw_priv->backend.wayland.wm_base)) {
		sw__event(SW_EVENT_WAYLAND_SURFACE_ERROR_MISSING_PROTOCOL, surface);
		return SU_FALSE;
	}

	popup_priv->parent = parent;
	surface->out.scale = 1;
	popup_priv->x = INT32_MIN;
	popup_priv->y = INT32_MIN;
	popup_priv->gravity = (sw_wayland_surface_popup_gravity_t)UINT32_MAX;
	popup_priv->constraint_adjustment = UINT32_MAX;

	surface_priv->wl_surface = wl_compositor_create_surface(sw_priv->backend.wayland.compositor);
	wl_surface_add_listener(surface_priv->wl_surface, &wl_surface_listener, surface);

	popup_priv->xdg_surface = xdg_wm_base_get_xdg_surface(
		sw_priv->backend.wayland.wm_base, surface_priv->wl_surface);
	xdg_surface_add_listener(popup_priv->xdg_surface, &xdg_surface_listener, surface);

	popup_priv->xdg_positioner = xdg_wm_base_create_positioner(sw_priv->backend.wayland.wm_base);

	surface_priv->state = SW__WAYLAND_SURFACE_STATE_ALIVE;
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
			surface_priv->state = SW__WAYLAND_SURFACE_STATE_DIRTY;
		}
	}
}

static void sw__wayland_surface_popup_handle_done(void *data, struct xdg_popup *xdg_popup) {
	sw_wayland_surface_t *surface = (sw_wayland_surface_t *)data;
	SU_NOTUSED(xdg_popup);
	if (sw__wayland_surface_fini(surface, SU_TRUE)) {
		sw__event(SW_EVENT_WAYLAND_SURFACE_DESTROY, surface);
	}
}

static void sw__wayland_surface_popup_handle_repositioned(void *data, struct xdg_popup *xdg_popup,
		uint32_t token) {
	SU_NOTUSED(data); SU_NOTUSED(xdg_popup); SU_NOTUSED(token);
}

static void sw__wayland_surface_cursor_image_init(sw_wayland_surface_t *surface) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;

	static const struct wl_surface_listener wl_surface_listener = {
		sw__wayland_surface_handle_enter,
		sw__wayland_surface_handle_leave,
		sw__wayland_surface_handle_preferred_buffer_scale,
		sw__wayland_surface_handle_preferred_buffer_transform
	};

	surface_priv->wl_surface = wl_compositor_create_surface(sw_priv->backend.wayland.compositor);
	wl_surface_add_listener(surface_priv->wl_surface, &wl_surface_listener, surface);

	surface->out.scale = 1;
	surface_priv->state = SW__WAYLAND_SURFACE_STATE_ALIVE;
}

static void sw__wayland_surface_prepare(sw_wayland_surface_t *surface, sw_wayland_surface_t *parent) {
	/* TODO: remove recursion */
	
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw__wayland_surface_t *surface_priv = (sw__wayland_surface_t *)&surface->sw__private;
	sw_wayland_cursor_shape_t cursor_shape = ((surface->in.cursor_shape == SW_WAYLAND_CURSOR_SHAPE_DEFAULT)
		? SW_WAYLAND_CURSOR_SHAPE_DEFAULT_ : surface->in.cursor_shape);
	const su_allocator_t *gp_alloc = sw__context->in.gp_alloc;

	SU_ASSERT(surface->in.root != NULL);

	if (SU_UNLIKELY(!sw__layout_block_init(surface->in.root))) {
		sw__event(SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK, surface);
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
				sw__event(SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_FAILED_TO_SET_DECORATIONS, surface);
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
			sw__wayland_surface_fini(surface, SU_FALSE);
		}
		
		if (SU_UNLIKELY(!surface_priv->wl_surface && !sw__wayland_surface_layer_init(surface, layer->layer))) {
			return;
		}

		if (layer_priv->keyboard_interactivity != layer->keyboard_interactivity) {
			zwlr_layer_surface_v1_set_keyboard_interactivity(
				layer_priv->layer_surface, layer->keyboard_interactivity);
			layer_priv->keyboard_interactivity = layer->keyboard_interactivity;
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
	case SW_WAYLAND_SURFACE_TYPE_CURSOR_IMAGE: {
		sw_wayland_surface_cursor_image_t *cursor_image = &surface->in._.cursor_image;
		sw__wayland_surface_cursor_image_t *cursor_image_priv = &surface_priv->_.cursor_image;

		SU_ASSERT(surface->in.input_regions_count == 0);
		SU_ASSERT(surface->in.popups.count == 0);

		if (SU_UNLIKELY(!surface_priv->wl_surface)) {
			sw__wayland_surface_cursor_image_init(surface);
		}

		if (cursor_image_priv->set_offset) {
			wl_surface_offset(surface_priv->wl_surface,
				cursor_image->offset_x - cursor_image_priv->offset_x,
				cursor_image->offset_y - cursor_image_priv->offset_y);
			cursor_image_priv->offset_x = cursor_image->offset_x;
			cursor_image_priv->offset_y = cursor_image->offset_y;
		}

		sw__wayland_surface_render(surface);
		return;
	}
	default:
		SU_ASSERT_UNREACHABLE;
	}
	
	if (surface->in.cursor_image) {
		SU_ASSERT(surface->in.cursor_image->in.type == SW_WAYLAND_SURFACE_TYPE_CURSOR_IMAGE);
		sw__wayland_surface_prepare(surface->in.cursor_image, NULL);
	}

	{
		/* TODO: cmp old and new */
		sw_wayland_seat_t *seat = sw__context->out.backend.wayland.seats.head;
		for ( ; seat; seat = seat->next) {
			if (seat->out.pointer && (seat->out.pointer->out.focused_surface == surface)) {
				sw__wayland_pointer_t *pointer_priv = (sw__wayland_pointer_t *)&seat->out.pointer->sw__private;
				if (surface->in.cursor_image || (surface_priv->cursor_shape == SW_WAYLAND_CURSOR_SHAPE_INVISIBLE)) {
					struct wl_surface *wl_surface = NULL;
					int32_t surface_x = 0, surface_y = 0;
					if (surface->in.cursor_image) {
						wl_surface = ((sw__wayland_surface_t *)surface->in.cursor_image->sw__private)->wl_surface;
						surface_x = surface->in.cursor_image->in._.cursor_image.offset_x;
						surface_y = surface->in.cursor_image->in._.cursor_image.offset_y;
					}
					wl_pointer_set_cursor(
						pointer_priv->wl_pointer, pointer_priv->enter_serial,
						wl_surface, surface_x, surface_y);
				} else if (pointer_priv->cursor_shape_device) {
					wp_cursor_shape_device_v1_set_shape(pointer_priv->cursor_shape_device,
						pointer_priv->enter_serial, cursor_shape);
				} else {
					sw__event(SW_EVENT_WAYLAND_SURFACE_FAILED_TO_SET_CURSOR_SHAPE, surface);
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
			input_region = wl_compositor_create_region(sw_priv->backend.wayland.compositor);
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

	static const struct xdg_popup_listener xdg_popup_listener = {
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
	case SW_WAYLAND_SURFACE_TYPE_CURSOR_IMAGE:
	default:
		SU_ASSERT_UNREACHABLE;
	}

	xdg_popup_add_listener(popup_priv->xdg_popup, &xdg_popup_listener, surface);

	if (popup_priv->grab) {
		sw__wayland_seat_t *grab_seat_priv = (sw__wayland_seat_t *)&popup_priv->grab->out.seat->sw__private;
		if ((grab_seat_priv->pointer_serial > 0) || (grab_seat_priv->keyboard_serial > 0)) {
			/* TODO: more robust serial check, use latest one */
			xdg_popup_grab( popup_priv->xdg_popup, grab_seat_priv->wl_seat,
				SU_MAX(grab_seat_priv->pointer_serial, grab_seat_priv->keyboard_serial));
		}
	}
}

static void sw__wayland_output_fini(sw_wayland_output_t *output) {
	sw__wayland_output_t *output_priv = (sw__wayland_output_t *)&output->sw__private;
	const su_allocator_t *gp_alloc = sw__context->in.gp_alloc;
	sw_wayland_surface_t *surface = sw__context->in.backend.wayland.layers.head;
	for ( ; surface; surface = surface->next) {
		if (surface->in._.layer.output == output) {
			if (sw__wayland_surface_fini(surface, SU_TRUE)) {
				sw__event(SW_EVENT_WAYLAND_SURFACE_DESTROY, surface);
			}
		}
	}
	if (output_priv->wl_output) {
		wl_output_destroy(output_priv->wl_output);
	}
	su_string_fini(&output->out.name, gp_alloc);
	su_string_fini(&output->out.description, gp_alloc);
	su_string_fini(&output->out.make, gp_alloc);
	su_string_fini(&output->out.model, gp_alloc);

	SU_MEMSET(&output->out, 0, sizeof(output->out));
	SU_MEMSET(output_priv, 0, sizeof(*output_priv));
}

static void sw__wayland_output_init(sw_event_t *event) {
	sw_wayland_output_t *new_output = event->in.wayland_output;
	sw_wayland_output_t *output = event->out._.wayland_output;
	SU_ASSERT(event->out.type == SW_EVENT_WAYLAND_OUTPUT_CREATE);
	if (new_output) {
		SU_LLIST_APPEND_TAIL(&sw__context->out.backend.wayland.outputs, new_output);
	}
	if (new_output == output) {
		return;
	}

	if (!new_output) {
		sw__wayland_output_fini(output);
	} else {
		sw__wayland_output_t *new_output_priv = (sw__wayland_output_t *)&new_output->sw__private;
		SU_MEMCPY(&new_output->out, &output->out, sizeof(output->out));
		SU_MEMCPY(new_output_priv, &output->sw__private, sizeof(*new_output_priv));
		wl_output_set_user_data(new_output_priv->wl_output, new_output);
		new_output_priv->state = SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER;
	}
	SU_FREE(sw__context->in.gp_alloc, output);
}

static void sw__wayland_output_handle_geometry(void *data, struct wl_output *wl_output,
		int32_t x, int32_t y, int32_t physical_width, int32_t physical_height,
		int32_t subpixel, const char *make, const char *model, int32_t transform) {
	sw_wayland_output_t *output = (sw_wayland_output_t *)data;
	const su_allocator_t *gp_alloc = sw__context->in.gp_alloc;
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
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;

	SU_NOTUSED(wl_output);

	if (output_priv->state == SW__WAYLAND_OBJECT_STATE_UNINITIALIZED) {
		sw_event_t *event = (sw_event_t *)sw__event(SW_EVENT_WAYLAND_OUTPUT_CREATE, output);
		event->in.wayland_output = output;
		sw_priv->check_events = SU_TRUE;
		output_priv->state = SW__WAYLAND_OBJECT_STATE_OWNED_BY_SW;
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

static void sw__wayland_pointer_handle_enter(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *wl_surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;
	sw__wayland_pointer_t *pointer_priv = (sw__wayland_pointer_t *)&pointer->sw__private;
	sw_wayland_surface_t *surface;
	sw__wayland_surface_t *surface_priv;
	sw_event_out__t *event;

	SU_NOTUSED(wl_pointer);

	if (SU_UNLIKELY(!wl_surface)) {
		return;
	}

	surface = (sw_wayland_surface_t *)wl_surface_get_user_data(wl_surface);
	surface_priv = (sw__wayland_surface_t *)&surface->sw__private;

	pointer_priv->enter_serial = serial;

	pointer->out.focused_surface = surface;
	pointer->out.pos_x = (int32_t)(wl_fixed_to_double(surface_x) * (double)surface->out.scale);
	pointer->out.pos_y = (int32_t)(wl_fixed_to_double(surface_y) * (double)surface->out.scale);

	if (surface->in.cursor_image || (surface_priv->cursor_shape == SW_WAYLAND_CURSOR_SHAPE_INVISIBLE)) {
		wl_surface = NULL;
		surface_x = surface_y = 0;
		if (surface->in.cursor_image) {
			wl_surface = ((sw__wayland_surface_t *)surface->in.cursor_image->sw__private)->wl_surface;
			surface_x = surface->in.cursor_image->in._.cursor_image.offset_x;
			surface_y = surface->in.cursor_image->in._.cursor_image.offset_y;
		}
		wl_pointer_set_cursor(
			pointer_priv->wl_pointer, pointer_priv->enter_serial,
			wl_surface, surface_x, surface_y);
	} else if (pointer_priv->cursor_shape_device) {
		wp_cursor_shape_device_v1_set_shape(pointer_priv->cursor_shape_device,
			serial, surface_priv->cursor_shape);
	} else {
		sw__event(SW_EVENT_WAYLAND_SURFACE_FAILED_TO_SET_CURSOR_SHAPE, surface);
	}

	event = sw__event(SW_EVENT_WAYLAND_POINTER_ENTER, pointer);
	SU_MEMCPY(&event->wayland_pointer.state, &pointer->out, sizeof(pointer->out));
}

static void sw__wayland_pointer_handle_leave(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;

	SU_NOTUSED(wl_pointer); SU_NOTUSED(serial); SU_NOTUSED(surface);

	if (SU_LIKELY(pointer->out.focused_surface)) {
		sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_POINTER_LEAVE, pointer);
		SU_MEMCPY(&event->wayland_pointer.state, &pointer->out, sizeof(pointer->out));
		pointer->out.focused_surface = NULL;
	}
}

static void sw__wayland_pointer_handle_motion(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;
	int32_t x, y;

	SU_NOTUSED(wl_pointer);

	if (SU_UNLIKELY(!pointer->out.focused_surface)) {
		return;
	}

	x = (int32_t)(wl_fixed_to_double(surface_x) * (double)pointer->out.focused_surface->out.scale);
	y = (int32_t)(wl_fixed_to_double(surface_y) * (double)pointer->out.focused_surface->out.scale);
	if ((x != pointer->out.pos_x) || (y != pointer->out.pos_y)) {
		sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_POINTER_MOTION, pointer);
		pointer->out.pos_x = x;
		pointer->out.pos_y = y;
		pointer->out.time = time;
		SU_MEMCPY(&event->wayland_pointer.state, &pointer->out, sizeof(pointer->out));
	}
}

static void sw__wayland_pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t st) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;
	sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&pointer->out.seat->sw__private;
	sw_event_out__t *event;

	SU_NOTUSED(wl_pointer);

	if (SU_UNLIKELY(!pointer->out.focused_surface)) {
		return;
	}

	seat_priv->pointer_serial = serial;

	pointer->out.time = time;
	pointer->out.btn_code = button;
	pointer->out.btn_state = (sw_wayland_pointer_button_state_t)st;

	event = sw__event(SW_EVENT_WAYLAND_POINTER_BUTTON, pointer);
	SU_MEMCPY(&event->wayland_pointer.state, &pointer->out, sizeof(pointer->out));
}

static void sw__wayland_pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)data;
	sw_event_out__t *event;

	SU_NOTUSED(wl_pointer); SU_NOTUSED(data);

	if (SU_UNLIKELY(!pointer->out.focused_surface)) {
		return;
	}

	pointer->out.time = time;
	pointer->out.scroll_axis = (sw_wayland_pointer_scroll_axis_t)axis;
	pointer->out.scroll_vector_length = wl_fixed_to_double(value);

	event = sw__event(SW_EVENT_WAYLAND_POINTER_SCROLL, pointer);
	SU_MEMCPY(&event->wayland_pointer.state, &pointer->out, sizeof(pointer->out));
}

static void sw__wayland_pointer_fini(sw_wayland_pointer_t *pointer) {
	sw__wayland_pointer_t *pointer_priv = (sw__wayland_pointer_t *)&pointer->sw__private;
	if (pointer_priv->cursor_shape_device) {
		wp_cursor_shape_device_v1_destroy(pointer_priv->cursor_shape_device);
	}
	if (pointer_priv->wl_pointer) {
		wl_pointer_destroy(pointer_priv->wl_pointer);
	}

	SU_MEMSET(&pointer->out, 0, sizeof(pointer->out));
	SU_MEMSET(pointer_priv, 0, sizeof(*pointer_priv));
}

static void sw__wayland_pointer_init(sw_event_t *event) {
	sw_wayland_pointer_t *new_pointer = event->in.wayland_pointer;
	sw_wayland_pointer_t *pointer = event->out._.wayland_pointer.pointer;
	sw_wayland_seat_t *seat = pointer->out.seat;

	SU_ASSERT(event->out.type == SW_EVENT_WAYLAND_POINTER_CREATE);

	seat->out.pointer = new_pointer;
	if (new_pointer == pointer) {
		return;
	}

	if (!new_pointer) {
		sw__wayland_pointer_fini(pointer);
	} else {
		sw__wayland_pointer_t *new_pointer_priv = (sw__wayland_pointer_t *)&new_pointer->sw__private;
		SU_MEMCPY(&new_pointer->out, &pointer->out, sizeof(pointer->out));
		SU_MEMCPY(&new_pointer->sw__private, &pointer->sw__private, sizeof(pointer->sw__private));
		wl_pointer_set_user_data(((sw__wayland_pointer_t *)&new_pointer->sw__private)->wl_pointer, new_pointer);
		new_pointer_priv->state = SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER;
	}
	SU_FREE(sw__context->in.gp_alloc, pointer);
}

#if SW_WITH_WAYLAND_KEYBOARD
static void sw__wayland_keyboard_handle_keymap(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t format, int32_t fd, uint32_t size) {
	sw_wayland_keyboard_t *keyboard = (sw_wayland_keyboard_t *)data;
	sw__wayland_keyboard_t *keyboard_priv = (sw__wayland_keyboard_t *)&keyboard->sw__private;

	SU_NOTUSED(wl_keyboard);

	xkb_state_unref(keyboard_priv->xkb_state);
	xkb_keymap_unref(keyboard_priv->xkb_keymap);
	keyboard_priv->xkb_state = NULL;
	keyboard_priv->xkb_keymap = NULL;
	SU_MEMSET(&keyboard->out.key, 0, sizeof(keyboard->out.key));
	keyboard->out.mods = 0;
	/* TODO: reset t */
	keyboard_priv->repeat_cp = 0;
	keyboard_priv->repeat_next = -1;

	switch ((enum wl_keyboard_keymap_format)format) {
	case WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1: {
		/* ? TODO: warn on errors */
		char *buffer = (char *)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (SU_UNLIKELY(buffer == MAP_FAILED)) {
			goto out;
		}
		keyboard_priv->xkb_keymap = xkb_keymap_new_from_buffer(
			keyboard_priv->xkb_context, buffer, size,
			XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
		munmap(buffer, size);
		if (SU_UNLIKELY(!keyboard_priv->xkb_keymap)) {
			goto out;
		}
		keyboard_priv->xkb_state = xkb_state_new(keyboard_priv->xkb_keymap); /* ? TODO: error check */
		break;
	}
	case WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP:
		/* TODO */
	default:
		break;
	}

out:
	/* TODO: event */
	close(fd);
}

static void sw__wayland_keyboard_handle_enter(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *wl_surface, struct wl_array *keys) {
	sw_wayland_keyboard_t *keyboard = (sw_wayland_keyboard_t *)data;
	sw_event_out__t *event;

	SU_NOTUSED(wl_keyboard); SU_NOTUSED(serial);

	if (SU_UNLIKELY(!wl_surface)) {
		return;
	}

	SU_NOTUSED(keys); /* TODO */

	keyboard->out.focused_surface = (sw_wayland_surface_t *)wl_surface_get_user_data(wl_surface);

	event = sw__event(SW_EVENT_WAYLAND_KEYBOARD_ENTER, keyboard);
	SU_MEMCPY(&event->wayland_keyboard.state, &keyboard->out, sizeof(keyboard->out));
}

static void sw__wayland_keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *surface) {
	sw_wayland_keyboard_t *keyboard = (sw_wayland_keyboard_t *)data;

	SU_NOTUSED(wl_keyboard); SU_NOTUSED(serial); SU_NOTUSED(surface);

	if (SU_LIKELY(keyboard->out.focused_surface)) {
		sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_KEYBOARD_LEAVE, keyboard);
		SU_MEMCPY(&event->wayland_keyboard.state, &keyboard->out, sizeof(keyboard->out));
		keyboard->out.focused_surface = NULL;
		/* ??? TODO: sw__wayland_data_device_fini_paste */
	}
}

static void sw__wayland_keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	sw_wayland_keyboard_t *keyboard = (sw_wayland_keyboard_t *)data;
	sw__wayland_keyboard_t *keyboard_priv = (sw__wayland_keyboard_t *)&keyboard->sw__private;
	sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&keyboard->out.seat->sw__private;
	uint32_t code = (key + 8);
	sw_event_out__t *event;

	SU_NOTUSED(wl_keyboard);

	if (SU_UNLIKELY(!keyboard_priv->xkb_state || !keyboard->out.focused_surface)) {
		return;
	}

	seat_priv->keyboard_serial = serial;

	keyboard->out.key.time = time;
	keyboard->out.key.state = (sw_wayland_keyboard_key_state_t)state;
	keyboard->out.key.cp = xkb_state_key_get_utf32(keyboard_priv->xkb_state, code);

	/* TODO: compose */

	if (keyboard->out.key.cp == 0) {
		return;
	}

	event = sw__event(SW_EVENT_WAYLAND_KEYBOARD_KEY, keyboard);
	SU_MEMCPY(&event->wayland_keyboard.state, &keyboard->out, sizeof(keyboard->out));

	if (state == SW_WAYLAND_KEYBOARD_KEY_STATE_RELEASED) {
		/* ? TODO: handle multi-key repeat */
		if (keyboard_priv->repeat_cp == keyboard->out.key.cp) {
			/* TODO: reset t */
			keyboard_priv->repeat_cp = 0;
			keyboard_priv->repeat_next = -1;
		}
	} else if ((keyboard->out.repeat_rate > 0) &&	
			xkb_keymap_key_repeats(keyboard_priv->xkb_keymap, code)) {
		keyboard_priv->repeat_cp = keyboard->out.key.cp;
		keyboard_priv->repeat_next =
			(su_now_ms(CLOCK_MONOTONIC) + keyboard->out.repeat_delay);
		sw__update_t(keyboard_priv->repeat_next);
	}
}

static void sw__wayland_keyboard_handle_modifiers(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
		uint32_t mods_locked, uint32_t group) {
	sw_wayland_keyboard_t *keyboard = (sw_wayland_keyboard_t *)data;
	sw__wayland_keyboard_t *keyboard_priv = (sw__wayland_keyboard_t *)&keyboard->sw__private;
	sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&keyboard->out.seat->sw__private;

	SU_NOTUSED(wl_keyboard);

	if (SU_UNLIKELY(!keyboard_priv->xkb_state || !keyboard->out.focused_surface)) {
		return;
	}

	seat_priv->keyboard_serial = serial;

	if (0 != xkb_state_update_mask( keyboard_priv->xkb_state,
			mods_depressed, mods_latched, mods_locked, 0, 0, group)) {
		sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_KEYBOARD_MOD, keyboard);
		static char *mods[] = {
			XKB_MOD_NAME_SHIFT,
			XKB_MOD_NAME_CAPS,
			XKB_MOD_NAME_CTRL,
			XKB_MOD_NAME_MOD1,
			XKB_MOD_NAME_MOD2,
			XKB_MOD_NAME_MOD3,
			XKB_MOD_NAME_MOD4,
			XKB_MOD_NAME_MOD5,
			XKB_VMOD_NAME_ALT,
			XKB_VMOD_NAME_HYPER,
			XKB_VMOD_NAME_LEVEL3,
			XKB_VMOD_NAME_LEVEL5,
			XKB_VMOD_NAME_META,
			XKB_VMOD_NAME_NUM,
			XKB_VMOD_NAME_SCROLL,
			XKB_VMOD_NAME_SUPER
		};
		size_t i;
		keyboard->out.mods = 0;
		for ( i = 1; i < (SU_LENGTH(mods) + 1); ++i) {
			if (xkb_state_mod_name_is_active( keyboard_priv->xkb_state,
					mods[i - 1], XKB_STATE_MODS_EFFECTIVE) == 1) {
				keyboard->out.mods |= (1 << i);
			}
		}
		/* ? TODO: layouts, leds, consumed */
		SU_MEMCPY(&event->wayland_keyboard.state, &keyboard->out, sizeof(keyboard->out));
	}
}

static void sw__wayland_keyboard_handle_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
		int32_t rate, int32_t delay) {
	sw_wayland_keyboard_t *keyboard = (sw_wayland_keyboard_t *)data;

	SU_NOTUSED(wl_keyboard);

	keyboard->out.repeat_rate = rate;
	keyboard->out.repeat_delay = delay;
	/* ? TODO: SW_EVENT_WAYLAND_KEYBOARD_MOD event */
}

static void sw__wayland_keyboard_fini(sw_wayland_keyboard_t *keyboard) {
	sw__wayland_keyboard_t *keyboard_priv = (sw__wayland_keyboard_t *)&keyboard->sw__private;
	if (keyboard_priv->wl_keyboard) {
		wl_keyboard_destroy(keyboard_priv->wl_keyboard);
	}
	xkb_state_unref(keyboard_priv->xkb_state);
	xkb_keymap_unref(keyboard_priv->xkb_keymap);
	xkb_context_unref(keyboard_priv->xkb_context);

	SU_MEMSET(&keyboard->out, 0, sizeof(keyboard->out));
	SU_MEMSET(keyboard_priv, 0, sizeof(*keyboard_priv));
}

static void sw__wayland_keyboard_init(sw_event_t *event) {
	sw_wayland_keyboard_t *new_keyboard = event->in.wayland_keyboard;
	sw_wayland_keyboard_t *keyboard = event->out._.wayland_keyboard.keyboard;
	sw_wayland_seat_t *seat = keyboard->out.seat;

	SU_ASSERT(event->out.type == SW_EVENT_WAYLAND_KEYBOARD_CREATE);

	seat->out.keyboard = new_keyboard;
	if (new_keyboard == keyboard) {
		return;
	}

	if (!new_keyboard) {
		sw__wayland_keyboard_fini(keyboard);
	} else {
		sw__wayland_keyboard_t *new_keyboard_priv = (sw__wayland_keyboard_t *)&new_keyboard->sw__private;
		SU_MEMCPY(&new_keyboard->out, &keyboard->out, sizeof(keyboard->out));
		SU_MEMCPY(&new_keyboard->sw__private, &keyboard->sw__private, sizeof(keyboard->sw__private));
		wl_keyboard_set_user_data(((sw__wayland_keyboard_t *)&new_keyboard->sw__private)->wl_keyboard, new_keyboard);
		new_keyboard_priv->state = SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER;
	}
	SU_FREE(sw__context->in.gp_alloc, keyboard);
}

#endif /* SW_WITH_WAYLAND_KEYBOARD */

#if SW_WITH_WAYLAND_CLIPBOARD

static void sw__wayland_data_device_fini_paste(sw_wayland_data_device_t *data_device) {
	sw__wayland_data_device_t *data_device_priv = (sw__wayland_data_device_t *)data_device->sw__private;
	size_t i;
	if (data_device_priv->paste.pfd.fd > 0) {
		close(data_device_priv->paste.pfd.fd);
	}
	if (data_device_priv->paste.wl_data_offer) {
		if (data_device->out.paste.dnd && data_device_priv->paste.dnd_got_action) {
			wl_data_offer_finish(data_device_priv->paste.wl_data_offer); /* TODO: rework/remove */
		}
		wl_data_offer_destroy(data_device_priv->paste.wl_data_offer);
	}
	su_string_fini(&data_device_priv->paste.mime_type, sw__context->in.gp_alloc);
	for ( i = 0; i < data_device->out.paste.offered_mime_types_count; ++i) {
		su_string_fini(&data_device->out.paste.offered_mime_types[i], sw__context->in.gp_alloc);
	}
	SU_FREE(sw__context->in.gp_alloc, data_device->out.paste.offered_mime_types);

	SU_FREE(sw__context->in.gp_alloc, data_device->out.paste.data.ptr);

	SU_MEMSET(&data_device->out.paste, 0, sizeof(data_device->out.paste));
	SU_MEMSET(&data_device_priv->paste, 0, sizeof(data_device_priv->paste));
}

static void sw__wayland_data_device_fini_copy(sw_wayland_data_device_t *data_device) {
	sw__wayland_data_device_t *data_device_priv = (sw__wayland_data_device_t *)data_device->sw__private;
	if (data_device_priv->copy.pfd.fd > 0) {
		close(data_device_priv->copy.pfd.fd);
	}
	if (data_device_priv->copy.wl_data_source) {
		wl_data_source_destroy(data_device_priv->copy.wl_data_source);
	}
	su_string_fini(&data_device_priv->copy.mime_type, sw__context->in.gp_alloc);
	SU_FREE(sw__context->in.gp_alloc, data_device_priv->copy.data.ptr);

	SU_MEMSET(&data_device_priv->copy, 0, sizeof(data_device_priv->copy));
	SU_MEMSET(&data_device->out.copy, 0, sizeof(data_device->out.copy));
}

static void sw__wayland_data_device_fini(sw_wayland_data_device_t *data_device) {
	sw__wayland_data_device_t *data_device_priv = (sw__wayland_data_device_t *)data_device->sw__private;
	sw__wayland_data_device_fini_paste(data_device);
	sw__wayland_data_device_fini_copy(data_device);
	if (data_device_priv->wl_data_device) {
		wl_data_device_destroy(data_device_priv->wl_data_device);
	}

	data_device->out.seat = NULL;
	SU_MEMSET(data_device_priv, 0, sizeof(*data_device_priv));
}

static void sw__wayland_data_device_init(sw_event_t *event) {
	sw_wayland_data_device_t *new_data_device = event->in.wayland_data_device;
	sw_wayland_data_device_t *data_device = event->out._.wayland_data_device.data_device;
	sw_wayland_seat_t *seat = data_device->out.seat;

	SU_ASSERT(event->out.type == SW_EVENT_WAYLAND_DATA_DEVICE_CREATE);

	seat->out.data_device = new_data_device;
	if (new_data_device == data_device) {
		return;
	}

	if (!new_data_device) {
		sw__wayland_data_device_fini(data_device);
	} else {
		sw__wayland_data_device_t *new_data_device_priv = (sw__wayland_data_device_t *)&new_data_device->sw__private;
		SU_MEMCPY(&new_data_device->out, &data_device->out, sizeof(data_device->out));
		SU_MEMCPY(&new_data_device->sw__private, &data_device->sw__private, sizeof(data_device->sw__private));
		wl_data_device_set_user_data(((sw__wayland_data_device_t *)&new_data_device->sw__private)->wl_data_device, new_data_device);
		new_data_device_priv->state = SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER;
	}
	SU_FREE(sw__context->in.gp_alloc, data_device);
}


#endif /* SW_WITH_WAYLAND_CLIPBOARD */

static void sw__wayland_seat_fini(sw_wayland_seat_t *seat) {
	sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&seat->sw__private;
#if SW_WITH_WAYLAND_KEYBOARD
	if (seat->out.keyboard) {
		sw__wayland_keyboard_fini(seat->out.keyboard);
		if (((sw__wayland_keyboard_t *)seat->out.keyboard)->state == SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER) {
			sw__event(SW_EVENT_WAYLAND_KEYBOARD_DESTROY, seat->out.keyboard);
		} else {
			SU_FREE(sw__context->in.gp_alloc, seat->out.keyboard);
		}
	}
#endif /* SW_WITH_WAYLAND_KEYBOARD */
	if (seat->out.pointer) {
		sw__wayland_pointer_fini(seat->out.pointer);
		if (((sw__wayland_pointer_t *)seat->out.pointer)->state == SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER) {
			sw__event(SW_EVENT_WAYLAND_POINTER_DESTROY, seat->out.pointer);
		} else {
			SU_FREE(sw__context->in.gp_alloc, seat->out.pointer);
		}
	}
#if SW_WITH_WAYLAND_CLIPBOARD
	if (seat->out.data_device) {
		sw__wayland_data_device_fini(seat->out.data_device);
		if (((sw__wayland_data_device_t *)seat->out.data_device)->state == SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER) {
			sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_DESTROY, seat->out.data_device);
		} else {
			SU_FREE(sw__context->in.gp_alloc, seat->out.data_device);
		}
	}
#endif /* SW_WITH_WAYLAND_CLIPBOARD */

	if (seat_priv->wl_seat) {
		wl_seat_destroy(seat_priv->wl_seat);
	}
	su_string_fini(&seat->out.name, sw__context->in.gp_alloc);

	SU_MEMSET(&seat->out, 0, sizeof(seat->out));
	SU_MEMSET(seat_priv, 0, sizeof(*seat_priv));
}

static void sw__wayland_seat_init(sw_event_t *event) {
	sw_wayland_seat_t *new_seat = event->in.wayland_seat;
	sw_wayland_seat_t *seat = event->out._.wayland_seat;
	SU_ASSERT(event->out.type == SW_EVENT_WAYLAND_SEAT_CREATE);
	if (new_seat) {
		SU_LLIST_APPEND_TAIL(&sw__context->out.backend.wayland.seats, new_seat);
	}
	if (new_seat == seat) {
		return;
	}

	if (!new_seat) {
		sw__wayland_seat_fini(seat);
	} else {
		sw__wayland_seat_t *new_seat_priv = (sw__wayland_seat_t *)&new_seat->sw__private;
		SU_MEMCPY(&new_seat->out, &seat->out, sizeof(seat->out));
		SU_MEMCPY(&new_seat->sw__private, &seat->sw__private, sizeof(seat->sw__private));
		wl_seat_set_user_data(((sw__wayland_seat_t *)&new_seat->sw__private)->wl_seat, new_seat);
		new_seat_priv->state = SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER;
	}
	SU_FREE(sw__context->in.gp_alloc, seat);
}

static void sw__wayland_seat_handle_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw_wayland_seat_t *seat = (sw_wayland_seat_t *)data;
	sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&seat->sw__private;

	/* TODO: touch */
	su_bool32_t cap_pointer = (capabilities & WL_SEAT_CAPABILITY_POINTER);

#if SW_WITH_WAYLAND_KEYBOARD
	su_bool32_t cap_keyboard = (capabilities & WL_SEAT_CAPABILITY_KEYBOARD);
	if (cap_keyboard && !seat->out.keyboard) {
		sw_event_t *event;
		sw_wayland_keyboard_t *keyboard;
		sw__wayland_keyboard_t *keyboard_priv;
		static const struct wl_keyboard_listener keyboard_listener = {
			sw__wayland_keyboard_handle_keymap,
			sw__wayland_keyboard_handle_enter,
			sw__wayland_keyboard_handle_leave,
			sw__wayland_keyboard_handle_key,
			sw__wayland_keyboard_handle_modifiers,
			sw__wayland_keyboard_handle_repeat_info
		};
		
		SU_ALLOCCT(keyboard, sw__context->in.gp_alloc);
		keyboard_priv = (sw__wayland_keyboard_t *)&keyboard->sw__private;
		
		keyboard->out.seat = seat;
		keyboard_priv->wl_keyboard = wl_seat_get_keyboard(seat_priv->wl_seat);
		wl_keyboard_add_listener(keyboard_priv->wl_keyboard, &keyboard_listener, keyboard);
		keyboard_priv->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS); /* ? TODO: error check */
		/* ? TODO: xkb_context_set_log_level, xkb_context_set_log_verbosity, xkb_context_set_log_fn */
		seat->out.keyboard = keyboard;
		event = (sw_event_t *)sw__event(SW_EVENT_WAYLAND_KEYBOARD_CREATE, keyboard);
		event->in.wayland_keyboard = keyboard;
		sw_priv->check_events = SU_TRUE;
	} else if (!cap_keyboard && seat->out.keyboard) {
		sw__wayland_keyboard_fini(seat->out.keyboard);
		if (((sw__wayland_keyboard_t *)seat->out.keyboard)->state == SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER) {
			sw__event(SW_EVENT_WAYLAND_KEYBOARD_DESTROY, seat->out.keyboard);
		} else {
			SU_FREE(sw__context->in.gp_alloc, seat->out.keyboard);
		}
		seat->out.keyboard = NULL;
	}
#endif /* SW_WITH_WAYLAND_KEYBOARD */

	if (cap_pointer && !seat->out.pointer) {
		sw_event_t *event;
		sw_wayland_pointer_t *pointer;
		sw__wayland_pointer_t *pointer_priv;
		static const struct wl_pointer_listener pointer_listener = {
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

		SU_ALLOCCT(pointer, sw__context->in.gp_alloc);
		pointer_priv = (sw__wayland_pointer_t *)&pointer->sw__private;

		pointer->out.seat = seat;
		/*pointer_priv->state = SW__WAYLAND_OBJECT_STATE_OWNED_BY_SW;*/
		pointer_priv->wl_pointer = wl_seat_get_pointer(seat_priv->wl_seat);
		wl_pointer_add_listener(pointer_priv->wl_pointer, &pointer_listener, pointer);
		if (sw_priv->backend.wayland.cursor_shape_manager) {
			pointer_priv->cursor_shape_device = wp_cursor_shape_manager_v1_get_pointer(
				sw_priv->backend.wayland.cursor_shape_manager, pointer_priv->wl_pointer);
		}
		seat->out.pointer = pointer;
		event = (sw_event_t *)sw__event(SW_EVENT_WAYLAND_POINTER_CREATE, pointer);
		event->in.wayland_pointer = pointer;
		sw_priv->check_events = SU_TRUE;
	} else if (!cap_pointer && seat->out.pointer) {
		sw__wayland_pointer_fini(seat->out.pointer);
		if (((sw__wayland_pointer_t *)seat->out.pointer)->state == SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER) {
			sw__event(SW_EVENT_WAYLAND_POINTER_DESTROY, seat->out.pointer);
		} else {
			SU_FREE(sw__context->in.gp_alloc, seat->out.pointer);
		}
		seat->out.pointer = NULL;
	}

	SU_NOTUSED(wl_seat);
}

#if SW_WITH_WAYLAND_CLIPBOARD

static void sw__wayland_data_source_handle_target(void *data, struct wl_data_source *wl_data_source,
		const char *mime_type) {
	SU_NOTUSED(data); SU_NOTUSED(wl_data_source); SU_NOTUSED(mime_type);
	/* ? TODO: multi-mime */
}

static void sw__wayland_data_source_handle_send(void *data, struct wl_data_source *wl_data_source,
		const char *mime_type, int32_t fd) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	sw__wayland_data_device_t *data_device_priv = (sw__wayland_data_device_t *)data_device->sw__private;

	if (!su_string_equal(su_string_(mime_type), data_device->in.copy.mime_type)
			|| !su_fd_set_cloexec((int)fd) || !su_fd_set_nonblock((int)fd)) {
		/* TODO: error/log */
		return;
	}

	data_device_priv->copy.pfd.fd = fd;
	data_device_priv->copy.pfd.events = POLLOUT;

	/* ? TODO: event */

	SU_NOTUSED(wl_data_source);
}

static void sw__wayland_data_source_handle_cancelled(void *data, struct wl_data_source *wl_data_source) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	sw__wayland_data_device_fini_copy(data_device);
	SU_NOTUSED(wl_data_source);

	sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_CANCELLED, data_device);
}

static void sw__wayland_data_source_handle_dnd_drop_performed(void *data, struct wl_data_source *wl_data_source) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	SU_NOTUSED(wl_data_source);

	sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_DROP_PERFORMED, data_device);
}

static void sw__wayland_data_source_handle_dnd_finished(void *data, struct wl_data_source *wl_data_source) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	sw__wayland_data_device_fini_copy(data_device);
	
	SU_NOTUSED(wl_data_source);

	sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_FINISHED, data_device);
}

static void sw__wayland_data_source_handle_action(void *data, struct wl_data_source *wl_data_source, uint32_t dnd_action) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	
	SU_NOTUSED(wl_data_source);

	data_device->out.copy.dnd_action = (sw_wayland_data_device_dnd_action_t)dnd_action;
	sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_ACTION, data_device);
}

static void sw__wayland_data_offer_handle_offer(void *data,
		struct wl_data_offer *wl_data_offer, const char *mime_type) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	size_t len = SU_STRLEN(mime_type);
	if (len > 0) {
		sw__wayland_data_device_t *data_device_priv = (sw__wayland_data_device_t *)data_device->sw__private;
		if (SU_UNLIKELY(data_device_priv->paste.offered_mime_types_capacity == data_device->out.paste.offered_mime_types_count)) {
			su_string_t *new_;
			data_device_priv->paste.offered_mime_types_capacity = ((data_device_priv->paste.offered_mime_types_capacity * 2) + 8);
			SU_ARRAY_ALLOC(new_, sw__context->in.gp_alloc, data_device_priv->paste.offered_mime_types_capacity);
			SU_MEMCPY(new_, data_device->out.paste.offered_mime_types,
				data_device->out.paste.offered_mime_types_count * sizeof(data_device->out.paste.offered_mime_types[0]));
			SU_FREE(sw__context->in.gp_alloc, data_device->out.paste.offered_mime_types);
			data_device->out.paste.offered_mime_types = new_;
		}
		su_string_init_len(&data_device->out.paste.offered_mime_types[data_device->out.paste.offered_mime_types_count++],
				sw__context->in.gp_alloc, mime_type, len, SU_TRUE);
	}

	SU_NOTUSED(wl_data_offer);
}

static void sw__wayland_data_offer_handle_source_actions(void *data,
		struct wl_data_offer *wl_data_offer, uint32_t source_actions) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_SOURCE_ACTIONS, data_device);

	SU_NOTUSED(wl_data_offer);

	data_device->out.paste.dnd_source_actions = (sw_wayland_data_device_dnd_action_mask_t)source_actions;
	SU_MEMCPY(&event->wayland_data_device.paste_state, &data_device->out.paste, sizeof(data_device->out.paste));
}

static void sw__wayland_data_offer_handle_action(void *data,
		struct wl_data_offer *wl_data_offer, uint32_t dnd_action) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	sw__wayland_data_device_t *data_device_priv = (sw__wayland_data_device_t *)data_device->sw__private;
	sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ACTION, data_device);

	SU_NOTUSED(wl_data_offer);

	data_device->out.paste.dnd_action = (sw_wayland_data_device_dnd_action_t)dnd_action;
	data_device_priv->paste.dnd_got_action = SU_TRUE;
	SU_MEMCPY(&event->wayland_data_device.paste_state, &data_device->out.paste, sizeof(data_device->out.paste));
}

static void sw__wayland_data_device_handle_data_offer(void *data,
		struct wl_data_device *wl_data_device, struct wl_data_offer *id) {
	static const struct wl_data_offer_listener data_offer_listener = {
		sw__wayland_data_offer_handle_offer,
		sw__wayland_data_offer_handle_source_actions,
		sw__wayland_data_offer_handle_action,
	};
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	sw__wayland_data_device_t *data_device_priv = (sw__wayland_data_device_t *)data_device->sw__private;

	data_device->out.copy.dnd_action = (sw_wayland_data_device_dnd_action_t)0;
	sw__wayland_data_device_fini_paste(data_device);
	data_device_priv->paste.wl_data_offer = id;
	wl_data_offer_add_listener(data_device_priv->paste.wl_data_offer, &data_offer_listener, data_device);

	SU_NOTUSED(wl_data_device);
}

static void sw__wayland_data_device_handle_enter(void *data, struct wl_data_device *wl_data_device,
		uint32_t serial, struct wl_surface *surface,
		wl_fixed_t x, wl_fixed_t y, struct wl_data_offer *id) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	sw__wayland_data_device_t *data_device_priv = (sw__wayland_data_device_t *)data_device->sw__private;
	sw_event_out__t *event;

	SU_NOTUSED(wl_data_device); SU_NOTUSED(id);

	if (SU_UNLIKELY(!surface)) {
		return;
	}

	SU_ASSERT(id == data_device_priv->paste.wl_data_offer);

	data_device->out.paste.dnd = SU_TRUE;
	data_device_priv->paste.dnd_enter_serial = serial;
	data_device->out.paste.dnd_surface = (sw_wayland_surface_t *)wl_surface_get_user_data(surface);
	data_device->out.paste.dnd_x = (int32_t)(wl_fixed_to_double(x) * (double)data_device->out.paste.dnd_surface->out.scale);
	data_device->out.paste.dnd_y = (int32_t)(wl_fixed_to_double(y) * (double)data_device->out.paste.dnd_surface->out.scale);

	event = sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ENTER, data_device);
	SU_MEMCPY(&event->wayland_data_device.paste_state, &data_device->out.paste, sizeof(data_device->out.paste));
}

static void sw__wayland_data_device_handle_leave(void *data, struct wl_data_device *wl_data_device) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;

	SU_NOTUSED(wl_data_device);

	if (SU_LIKELY(data_device->out.paste.dnd_surface)) {
		sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_LEAVE, data_device);
		SU_MEMCPY(&event->wayland_data_device.paste_state, &data_device->out.paste, sizeof(data_device->out.paste));
		sw__wayland_data_device_fini_paste(data_device);
	}
}

static void sw__wayland_data_device_handle_motion(void *data, struct wl_data_device *wl_data_device,
		uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	int32_t x, y;

	SU_NOTUSED(wl_data_device);

	if (SU_UNLIKELY(!data_device->out.paste.dnd_surface)) {
		return;
	}

	x = (int32_t)(wl_fixed_to_double(surface_x) * (double)data_device->out.paste.dnd_surface->out.scale);
	y = (int32_t)(wl_fixed_to_double(surface_y) * (double)data_device->out.paste.dnd_surface->out.scale);
	if ((x != data_device->out.paste.dnd_x) || (data_device->out.paste.dnd_y)) {
		sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_MOTION, data_device);
		event->wayland_data_device.paste_state.dnd_x = x;
		event->wayland_data_device.paste_state.dnd_y = y;
		data_device->out.paste.dnd_time = time;
		SU_MEMCPY(&event->wayland_data_device.paste_state, &data_device->out.paste, sizeof(data_device->out.paste));
	}
}

static void sw__wayland_data_device_handle_drop(void *data, struct wl_data_device *wl_data_device) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	if (SU_LIKELY(data_device->out.paste.dnd_surface)) {
		sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_DROP, data_device);
		SU_MEMCPY(&event->wayland_data_device.paste_state, &data_device->out.paste, sizeof(data_device->out.paste));
	}

	SU_NOTUSED(wl_data_device);
}

static void sw__wayland_data_device_handle_selection(void *data, struct wl_data_device *wl_data_device,
		struct wl_data_offer *id) {
	sw_wayland_data_device_t *data_device = (sw_wayland_data_device_t *)data;
	if (!id) {
		sw__wayland_data_device_fini_paste(data_device);
		data_device->out.copy.dnd_action = (sw_wayland_data_device_dnd_action_t)0;
	} else {
		sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_MIME_OFFERS, data_device);
		SU_MEMCPY(&event->wayland_data_device.paste_state, &data_device->out.paste, sizeof(data_device->out.paste));
	}

	SU_NOTUSED(wl_data_device);
}

#endif /* SW_WITH_WAYLAND_CLIPBOARD */

static void sw__wayland_seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name) {
	sw_wayland_seat_t *seat = (sw_wayland_seat_t *)data;
	sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&seat->sw__private;
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	size_t len = SU_STRLEN(name);

	SU_NOTUSED(wl_seat);

	if (len > 0) {
		su_string_init_len(&seat->out.name, sw__context->in.gp_alloc, name, len, SU_TRUE);
	}

	if (seat_priv->state == SW__WAYLAND_OBJECT_STATE_UNINITIALIZED) {
		sw_event_t *event;
		event = (sw_event_t *)sw__event(SW_EVENT_WAYLAND_SEAT_CREATE, seat);
		event->in.wayland_seat = seat;
		sw_priv->check_events = SU_TRUE;
		seat_priv->state = SW__WAYLAND_OBJECT_STATE_OWNED_BY_SW;

#if SW_WITH_WAYLAND_CLIPBOARD
		{
			sw_wayland_data_device_t *data_device;
			sw__wayland_data_device_t *data_device_priv;
			static const struct wl_data_device_listener data_device_listener = {
				sw__wayland_data_device_handle_data_offer,
				sw__wayland_data_device_handle_enter,
				sw__wayland_data_device_handle_leave,
				sw__wayland_data_device_handle_motion,
				sw__wayland_data_device_handle_drop,
				sw__wayland_data_device_handle_selection,
			};
		
			SU_ALLOCCT(data_device, sw__context->in.gp_alloc);
			data_device_priv = (sw__wayland_data_device_t *)&data_device->sw__private;
		
			data_device->out.seat = seat;
			data_device_priv->wl_data_device = wl_data_device_manager_get_data_device(
				sw_priv->backend.wayland.data_device_manager, seat_priv->wl_seat);
			wl_data_device_add_listener(data_device_priv->wl_data_device, &data_device_listener, data_device);
			seat->out.data_device = data_device;
			event = (sw_event_t *)sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_CREATE, data_device);
			event->in.wayland_data_device = data_device;
		}
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
	}
}

static void sw__wayland_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	SU_NOTUSED(data); SU_NOTUSED(xdg_wm_base);
	xdg_wm_base_pong(sw_priv->backend.wayland.wm_base, serial);
}

static void sw__wayland_registry_handle_global(void *data, struct wl_registry *wl_registry,
		uint32_t wl_name, const char *interface, uint32_t version) {
	sw__context_t *sw_priv = (sw__context_t *)&sw__context->sw__private;
	sw__backend_wayland_t *wayland_priv = &sw_priv->backend.wayland;

	SU_NOTUSED(data); SU_NOTUSED(wl_registry); SU_NOTUSED(version);
	
	if (SU_STRCMP(interface, wl_output_interface.name) == 0) {
		static const struct wl_output_listener output_listener = {
			sw__wayland_output_handle_geometry,
			sw__wayland_output_handle_mode,
			sw__wayland_output_handle_done,
			sw__wayland_output_handle_scale,
			sw__wayland_output_handle_name,
			sw__wayland_output_handle_description,
		};

		sw_wayland_output_t *output;
		sw__wayland_output_t *output_priv;

		SU_ALLOCCT(output, sw__context->in.gp_alloc);
		output_priv = (sw__wayland_output_t *)&output->sw__private;
	
		output->out.scale = 1;
		output_priv->wl_output = (struct wl_output *)wl_registry_bind(sw_priv->backend.wayland.registry,
			wl_name, &wl_output_interface, 4);
		output_priv->wl_name = wl_name;
		wl_output_add_listener(output_priv->wl_output, &output_listener, output);
	} else if (SU_STRCMP(interface, wl_seat_interface.name) == 0) {
		static const struct wl_seat_listener seat_listener = {
			sw__wayland_seat_handle_capabilities,
			sw__wayland_seat_handle_name,
		};

		sw_wayland_seat_t *seat;
		sw__wayland_seat_t *seat_priv;

		SU_ALLOCCT(seat, sw__context->in.gp_alloc);
		seat_priv = (sw__wayland_seat_t *)&seat->sw__private;

		seat_priv->wl_seat = (struct wl_seat *)wl_registry_bind(
			sw_priv->backend.wayland.registry, wl_name, &wl_seat_interface, 4);
		seat_priv->wl_name = wl_name;
		wl_seat_add_listener(seat_priv->wl_seat, &seat_listener, seat);
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
		static const struct xdg_wm_base_listener wm_base_listener = {
			sw__wayland_wm_base_handle_ping
		};
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
#if SW_WITH_WAYLAND_CLIPBOARD
	else if (SU_STRCMP(interface, wl_data_device_manager_interface.name) == 0) {
		wayland_priv->data_device_manager = (struct wl_data_device_manager *)wl_registry_bind(
			wayland_priv->registry, wl_name, &wl_data_device_manager_interface, 3);
	}
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
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
			sw__wayland_output_fini(output);
			if (output_priv->state == SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER) {
				sw__event(SW_EVENT_WAYLAND_OUTPUT_DESTROY, output);
			} else {
				SU_FREE(sw__context->in.gp_alloc, output);
			}
			return;
		}
	}

	for ( seat = wayland->seats.head; seat; seat = seat->next) {
		sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)&seat->sw__private;
		if (seat_priv->wl_name == name) {
			SU_LLIST_POP(&wayland->seats, seat);
			sw__wayland_seat_fini(seat);
			if (seat_priv->state == SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER) {
				sw__event(SW_EVENT_WAYLAND_SEAT_DESTROY, seat);
			} else {
				SU_FREE(sw__context->in.gp_alloc, seat);
			}
			return;
		}
	}
}

#endif /* SW_WITH_WAYLAND_BACKEND */

SW_FUNC_DEF su_bool32_t sw_set(sw_context_t *sw) {
	static const su_allocator_t fallback_alloc = { su_libc_alloc, su_libc_free };
	su_bool32_t ret = SU_TRUE;
	sw__context_t *sw_priv = (sw__context_t *)&sw->sw__private;
	sw_context_t *old_context = sw__context;
	const su_allocator_t *gp_alloc;
	size_t i;
	sw_layout_block_t *block;
	su_bool32_t update_and_render = sw->in.update_and_render;

	SU_ASSERT(su_locale_is_utf8());

	sw__context = sw;

	if ((sw->out.t > 0) && (su_now_ms(CLOCK_MONOTONIC) >= sw->out.t)) {
		sw->out.t = -1;
		update_and_render = SU_TRUE; /* TODO: rework */
	}

	if (!sw->in.gp_alloc) {
		sw->in.gp_alloc = &fallback_alloc;
	}
	if (!sw->in.scratch_alloc) {
		/* ? TODO: internal arena alloc */
		sw->in.scratch_alloc = &fallback_alloc;
	}

	gp_alloc = sw->in.gp_alloc;

	if (sw_priv->check_events) {
		for ( i = (sw->out.events_count - 1); i != SIZE_MAX; --i) {
			sw_event_t *event = &sw->out.events[i];
			switch (event->out.type) {
#if SW_WITH_WAYLAND_BACKEND
			case SW_EVENT_WAYLAND_OUTPUT_CREATE:
				sw__wayland_output_init(event);
				break;
			case SW_EVENT_WAYLAND_SEAT_CREATE:
				sw__wayland_seat_init(event);
				break;
			case SW_EVENT_WAYLAND_POINTER_CREATE:
				sw__wayland_pointer_init(event);
				break;
#if SW_WITH_WAYLAND_KEYBOARD
			case SW_EVENT_WAYLAND_KEYBOARD_CREATE:
				sw__wayland_keyboard_init(event);
				break;
			case SW_EVENT_WAYLAND_KEYBOARD_ENTER:
			case SW_EVENT_WAYLAND_KEYBOARD_LEAVE:
			case SW_EVENT_WAYLAND_KEYBOARD_KEY:
			case SW_EVENT_WAYLAND_KEYBOARD_KEY_REPEAT:
			case SW_EVENT_WAYLAND_KEYBOARD_MOD:
				break;
			case SW_EVENT_WAYLAND_KEYBOARD_DESTROY:
#endif /* SW_WITH_WAYLAND_KEYBOARD */
#if SW_WITH_WAYLAND_CLIPBOARD
			case SW_EVENT_WAYLAND_DATA_DEVICE_CREATE:
				sw__wayland_data_device_init(event);
				break;
			case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_MIME_OFFERS:
			case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_DATA:
			case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ACTION:
			case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_SOURCE_ACTIONS:
			case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ENTER:
			case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_LEAVE:
			case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_MOTION:
			case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_DROP:
			case SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_FINISHED:
			case SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_CANCELLED:
			case SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_DROP_PERFORMED:
			case SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_ACTION:
			case SW_EVENT_WAYLAND_DATA_DEVICE_DESTROY:
				break;
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
			case SW_EVENT_WAYLAND_OUTPUT_DESTROY:
			case SW_EVENT_WAYLAND_SEAT_DESTROY:
			case SW_EVENT_WAYLAND_POINTER_DESTROY:
			case SW_EVENT_WAYLAND_SURFACE_DESTROY:
			case SW_EVENT_WAYLAND_SURFACE_FAILED_TO_SET_CURSOR_SHAPE:
			case SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_FAILED_TO_SET_DECORATIONS:
			case SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_CLOSE:
			case SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK:
			case SW_EVENT_WAYLAND_SURFACE_ERROR_LAYOUT_FAILED:
			case SW_EVENT_WAYLAND_SURFACE_ERROR_MISSING_PROTOCOL:
			case SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER:
			case SW_EVENT_WAYLAND_POINTER_ENTER:
			case SW_EVENT_WAYLAND_POINTER_LEAVE:
			case SW_EVENT_WAYLAND_POINTER_MOTION:
			case SW_EVENT_WAYLAND_POINTER_BUTTON:
			case SW_EVENT_WAYLAND_POINTER_SCROLL:
#endif /* SW_WITH_WAYLAND_BACKEND */
			case SW_EVENT_LAYOUT_BLOCK_DESTROY:
			case SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_IMAGE:
#if SW_WITH_TEXT
			case SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_FONT:
			case SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_TEXT:
#endif /* SW_WITH_TEXT */
				break;
			default:
				SU_ASSERT_UNREACHABLE;
			}
		}
		sw_priv->check_events = SU_FALSE;
	}
	sw->out.events_count = 0;

	for ( block = sw->in.blocks_to_destroy.head; block; block = block->next) {
		if (sw__layout_block_fini(block, SU_TRUE)) {
			sw__event(SW_EVENT_LAYOUT_BLOCK_DESTROY, block);
		}
	}

	if (sw->in.backend_type != sw_priv->backend_type) {
		sw_event_t *events;
		size_t events_count, events_capacity;

		switch (sw_priv->backend_type) {
#if SW_WITH_MEMORY_BACKEND
		case SW_BACKEND_TYPE_MEMORY: {
			sw__backend_memory_t *memory = &sw_priv->backend.memory;
			pixman_image_unref(memory->image);
			break;
		}
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
		case SW_BACKEND_TYPE_WAYLAND: {
			sw__backend_wayland_t *wayland_priv = &sw_priv->backend.wayland;
			sw_wayland_surface_t *surface;
			sw_wayland_output_t *output;
			sw_wayland_seat_t *seat;

			for ( surface = sw->in.backend.wayland.toplevels.head; surface; surface = surface->next) {
				if (sw__wayland_surface_fini(surface, SU_TRUE)) {
					sw__event(SW_EVENT_WAYLAND_SURFACE_DESTROY, surface);
				}
			}

			for ( output = sw->out.backend.wayland.outputs.head; output; ) {
				sw_wayland_output_t *next = output->next;
				sw__wayland_output_t *output_priv = (sw__wayland_output_t *)output->sw__private;
				su_bool32_t event = (output_priv->state == SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER);
				sw__wayland_output_fini(output);
				if (event) {
					sw__event(SW_EVENT_WAYLAND_OUTPUT_DESTROY, output);
				} else {
					SU_FREE(gp_alloc, output);
				}
				output = next;
			}

			for ( seat = sw->out.backend.wayland.seats.head; seat; ) {
				sw_wayland_seat_t *next = seat->next;
				sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)seat->sw__private;
				su_bool32_t event = (seat_priv->state == SW__WAYLAND_OBJECT_STATE_OWNED_BY_USER);
				sw__wayland_seat_fini(seat);
				if (event) {
					sw__event(SW_EVENT_WAYLAND_SEAT_DESTROY, seat);
				} else {
					SU_FREE(gp_alloc, seat);
				}
				seat = next;
			}

#if SW_WITH_WAYLAND_CLIPBOARD
			SU_FREE(gp_alloc, sw->out.backend.wayland.fds);
			if (wayland_priv->data_device_manager) {
				wl_data_device_manager_destroy(wayland_priv->data_device_manager);
			}
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
			if (wayland_priv->decoration_manager) {
				zxdg_decoration_manager_v1_destroy(wayland_priv->decoration_manager);
			}
			if (wayland_priv->cursor_shape_manager) {
				wp_cursor_shape_manager_v1_destroy(wayland_priv->cursor_shape_manager);
			}
			if (wayland_priv->layer_shell) {
				zwlr_layer_shell_v1_destroy(wayland_priv->layer_shell);
			}
			if (wayland_priv->wm_base) {
				xdg_wm_base_destroy(wayland_priv->wm_base);
			}
			if (wayland_priv->shm) {
				wl_shm_destroy(wayland_priv->shm);
			}
			if (wayland_priv->compositor) {
				wl_compositor_destroy(wayland_priv->compositor);
			}
			if (wayland_priv->registry) {
				wl_registry_destroy(wayland_priv->registry);
			}
			if (wayland_priv->display) {
				wl_display_flush(wayland_priv->display);
				wl_display_disconnect(wayland_priv->display);
			}
			break;
		}
#endif /* SW_WITH_WAYLAND_BACKEND */
		case SW_BACKEND_TYPE_NONE:
			break;
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

		events = sw->out.events;
		events_count = sw->out.events_count;
		events_capacity = sw_priv->events_capacity;

		SU_MEMSET(&sw->out, 0, sizeof(sw->out));
		SU_MEMSET(sw_priv, 0, sizeof(*sw_priv));

		sw->out.events = events;
		sw->out.events_count = events_count;
		sw_priv->events_capacity = events_capacity;
	}

	if (SU_UNLIKELY(!sw_priv->image_cache.items && (sw->in.backend_type != SW_BACKEND_TYPE_NONE))) {
		sw->out.events_count = 0;
		sw_priv->events_capacity = 64;
		SU_ARRAY_ALLOC(sw->out.events, gp_alloc, sw_priv->events_capacity);

		/* ? TODO: resvg_init_log(); */

		su_hash_table__sw__image_cache_t__init(&sw_priv->image_cache, gp_alloc, 512);

#if SW_WITH_TEXT
		if (!fcft_init(FCFT_LOG_COLORIZE_NEVER, SU_FALSE, FCFT_LOG_CLASS_ERROR)) {
			ret = SU_FALSE;
			goto out;
		}

		su_hash_table__sw__text_run_cache_t__init(&sw_priv->text_run_cache, gp_alloc, 1024);
#endif /* SW_WITH_TEXT */
	}

	switch (sw->in.backend_type) {
#if SW_WITH_MEMORY_BACKEND
	case SW_BACKEND_TYPE_MEMORY: {
		sw_backend_memory_in_t *memory = &sw->in.backend.memory;
		sw__backend_memory_t *memory_priv = &sw_priv->backend.memory;

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

		if (SU_LIKELY(update_and_render)) {
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
		}

		break;
	}
#endif /* SW_WITH_MEMORY_BACKEND */
#if SW_WITH_WAYLAND_BACKEND
	case SW_BACKEND_TYPE_WAYLAND: {
		sw_backend_wayland_in_t *wayland_in = &sw->in.backend.wayland;
		sw_backend_wayland_out_t *wayland_out = &sw->out.backend.wayland;
		sw__backend_wayland_t *wayland_priv = &sw_priv->backend.wayland;
		sw_wayland_surface_t *surface;
#if SW_WITH_WAYLAND_KEYBOARD || SW_WITH_WAYLAND_CLIPBOARD
		sw_wayland_seat_t *seat;
#endif /* SW_WITH_WAYLAND_KEYBOARD || SW_WITH_WAYLAND_CLIPBOARD */

		if (SU_UNLIKELY(!wayland_priv->display)) {
			static const struct wl_registry_listener registry_listener = {
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
			/* ? TODO: check core globals */
			if (wl_display_roundtrip(wayland_priv->display) == -1) {
				ret = SU_FALSE;
				goto out;
			}

			{
#if SW_WITH_WAYLAND_CLIPBOARD
				wayland_priv->fds_capacity = ((wayland_out->seats.count + 1) * 2);
				SU_ARRAY_ALLOC(wayland_out->fds, gp_alloc, wayland_priv->fds_capacity);
#else
				static struct pollfd pfd;
				wayland_out->fds = &pfd;
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
				wayland_out->fds_count = 1;
				wayland_out->fds[0].fd = wl_display_get_fd(wayland_priv->display);
				wayland_out->fds[0].events = POLLIN;
			}
		}

		for ( surface = wayland_in->surfaces_to_destroy.head; surface; surface = surface->next) {
			if (sw__wayland_surface_fini(surface, SU_TRUE)) {
				sw__event(SW_EVENT_WAYLAND_SURFACE_DESTROY, surface);
			}
		}

		if (update_and_render) {
			for ( surface = wayland_in->toplevels.head; surface; surface = surface->next) {
				SU_ASSERT(surface->in.type == SW_WAYLAND_SURFACE_TYPE_TOPLEVEL);
				sw__wayland_surface_prepare(surface, NULL);
			}

			for ( surface = wayland_in->layers.head; surface; surface = surface->next) {
				SU_ASSERT(surface->in.type == SW_WAYLAND_SURFACE_TYPE_LAYER);
				sw__wayland_surface_prepare(surface, NULL);
			}
		}

		if (wl_display_prepare_read(sw_priv->backend.wayland.display) != -1) {
			if (wl_display_read_events(sw_priv->backend.wayland.display) == -1) {
				ret = SU_FALSE;
				goto out;
			}
		}
		wl_display_dispatch_pending(sw_priv->backend.wayland.display);

#if SW_WITH_WAYLAND_KEYBOARD || SW_WITH_WAYLAND_CLIPBOARD
		for (seat = wayland_out->seats.head; seat; seat = seat->next) {
#if SW_WITH_WAYLAND_CLIPBOARD
			if (seat->out.data_device) {
				sw__wayland_seat_t *seat_priv = (sw__wayland_seat_t *)seat->sw__private;
				sw_wayland_data_device_t *data_device = seat->out.data_device;
				sw__wayland_data_device_t *data_device_priv = (sw__wayland_data_device_t *)data_device->sw__private;
				if (data_device_priv->paste.wl_data_offer) {
					if (data_device->out.paste.dnd &&
							((data_device->in.paste.dnd_actions != data_device_priv->paste.dnd_actions) ||
							(data_device->in.paste.dnd_preferred_action != data_device_priv->paste.dnd_preferred_action))) {
						if (data_device->out.paste.dnd_source_actions & data_device->in.paste.dnd_preferred_action) {
							wl_data_offer_set_actions(data_device_priv->paste.wl_data_offer,
								data_device->in.paste.dnd_actions, data_device->in.paste.dnd_preferred_action);
						}
						data_device_priv->paste.dnd_actions = data_device->in.paste.dnd_actions;
						data_device_priv->paste.dnd_preferred_action = data_device->in.paste.dnd_preferred_action;
					}
					if (!su_string_equal(data_device_priv->paste.mime_type, data_device->in.paste.mime_type)) {
						if (data_device_priv->paste.pfd.fd > 0) {
							close(data_device_priv->paste.pfd.fd);
							data_device_priv->paste.pfd.fd = 0;
						}
						su_string_fini(&data_device_priv->paste.mime_type, sw__context->in.gp_alloc);
						if (data_device->in.paste.mime_type.len > 0) {
							int pipe_fd[2];
							SU_ASSERT(data_device->in.paste.mime_type.nul_terminated); /* TODO: handle properly */
							if ((pipe(pipe_fd) == 0) && su_fd_set_cloexec(pipe_fd[0]) && su_fd_set_nonblock(pipe_fd[0])) {
								/* TODO: error check mime string */
								wl_data_offer_receive(data_device_priv->paste.wl_data_offer,
									data_device->in.paste.mime_type.s, pipe_fd[1]);
								close(pipe_fd[1]);
								data_device->out.paste.data.len = 0;
								if (SU_UNLIKELY(!data_device->out.paste.data.ptr)) {
									data_device_priv->paste.data_capacity = 1024;
									SU_ALLOCTSA(data_device->out.paste.data.ptr, gp_alloc, data_device_priv->paste.data_capacity, 8);
								}
								data_device_priv->paste.pfd.fd = pipe_fd[0];
								data_device_priv->paste.pfd.events = POLLIN;
								su_string_init_string(&data_device_priv->paste.mime_type, gp_alloc, data_device->in.paste.mime_type);
								if (data_device->out.paste.dnd) {
									wl_data_offer_accept(data_device_priv->paste.wl_data_offer,
										data_device_priv->paste.dnd_enter_serial, data_device->in.paste.mime_type.s);
								}
							}
						} else {
							if (data_device->out.paste.dnd) {
								/* ? TODO: wl_data_offer_accept(, , NULL); */
								sw__wayland_data_device_fini_paste(data_device);
							} else {
								SU_MEMSET(&data_device_priv->paste.mime_type, 0, sizeof(data_device_priv->paste.mime_type));
							}
						}
					}
				}
				if ((data_device->in.copy.data.len != data_device_priv->copy.data.len) ||
						!su_string_equal(data_device->in.copy.mime_type, data_device_priv->copy.mime_type) ||
						SU_MEMCMP(data_device->in.copy.data.ptr, data_device_priv->copy.data.ptr, data_device->in.copy.data.len)) {
					sw__wayland_data_device_fini_copy(data_device);

					if ((data_device->in.copy.mime_type.len > 0) && (data_device->in.copy.data.len > 0)
							&& ((data_device->in.copy.dnd)
								? (seat_priv->pointer_serial > 0)
								: ((seat_priv->pointer_serial > 0) || (seat_priv->keyboard_serial > 0)))) {
						/* TODO: more robust serial check */
						static const struct wl_data_source_listener data_source_listener = {
							sw__wayland_data_source_handle_target,
							sw__wayland_data_source_handle_send,
							sw__wayland_data_source_handle_cancelled,
							sw__wayland_data_source_handle_dnd_drop_performed,
							sw__wayland_data_source_handle_dnd_finished,
							sw__wayland_data_source_handle_action,
						};

						SU_ASSERT(data_device->in.copy.mime_type.nul_terminated); /* TODO: handle properly */

						data_device_priv->copy.wl_data_source = wl_data_device_manager_create_data_source(
							wayland_priv->data_device_manager);
						wl_data_source_add_listener(data_device_priv->copy.wl_data_source, &data_source_listener, data_device);
						wl_data_source_offer(data_device_priv->copy.wl_data_source, data_device->in.copy.mime_type.s);

						if (data_device->in.copy.dnd) {
							struct wl_surface *origin = NULL;
							if (seat->out.pointer && seat->out.pointer->out.focused_surface) {
								origin = ((sw__wayland_surface_t *)seat->out.pointer->out.focused_surface->sw__private)->wl_surface;
							}
							if (origin) {
								struct wl_surface *icon = NULL;
								if (data_device->in.copy.dnd_cursor_image) {
									sw__wayland_surface_t *dnd_cursor_image_priv = 
										(sw__wayland_surface_t *)data_device->in.copy.dnd_cursor_image->sw__private;
									if (!dnd_cursor_image_priv->wl_surface) {
										sw__wayland_surface_cursor_image_init(data_device->in.copy.dnd_cursor_image);
									}
									dnd_cursor_image_priv->_.cursor_image.offset_x =
										dnd_cursor_image_priv->_.cursor_image.offset_y = 0;
									dnd_cursor_image_priv->_.cursor_image.set_offset = SU_TRUE;
									icon = dnd_cursor_image_priv->wl_surface;
								}

								/* ? TODO: cmp old and new */
								wl_data_source_set_actions(
									data_device_priv->copy.wl_data_source, data_device->in.copy.dnd_actions);

								/* ? TODO: internal dnd (NULL wl_data_source) */
								wl_data_device_start_drag( data_device_priv->wl_data_device,
									data_device_priv->copy.wl_data_source, origin, icon, seat_priv->pointer_serial);
							}
						} else {
							wl_data_device_set_selection(data_device_priv->wl_data_device,
								data_device_priv->copy.wl_data_source,
								SU_MAX(seat_priv->keyboard_serial, seat_priv->pointer_serial)); /* TODO: use latest serial */
						}

						su_string_init_string(&data_device_priv->copy.mime_type, gp_alloc, data_device->in.copy.mime_type);
						SU_ALLOCTSA(data_device_priv->copy.data.ptr, gp_alloc, data_device->in.copy.data.len, 8);
						SU_MEMCPY(data_device_priv->copy.data.ptr, data_device->in.copy.data.ptr, data_device->in.copy.data.len);
						data_device_priv->copy.data.len = data_device->in.copy.data.len;
					}
				}
				if (update_and_render && data_device->in.copy.dnd_cursor_image && data_device_priv->copy.wl_data_source) {
					SU_ASSERT(data_device->in.copy.dnd_cursor_image->in.type == SW_WAYLAND_SURFACE_TYPE_CURSOR_IMAGE);
					sw__wayland_surface_prepare(data_device->in.copy.dnd_cursor_image, NULL);
				}
			}
#endif /* SW_WITH_WAYLAND_CLIPBOARD */
#if SW_WITH_WAYLAND_KEYBOARD
			if (seat->out.keyboard) {
				sw_wayland_keyboard_t *keyboard = seat->out.keyboard;
				sw__wayland_keyboard_t *keyboard_priv = (sw__wayland_keyboard_t *)&keyboard->sw__private;
				int64_t ms_now = su_now_ms(CLOCK_MONOTONIC);
				if (ms_now >= keyboard_priv->repeat_next) {
					if (keyboard->out.focused_surface && keyboard_priv->repeat_cp) {
						sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_KEYBOARD_KEY_REPEAT, keyboard);
						keyboard->out.key.time = ms_now;
						keyboard->out.key.state = SW_WAYLAND_KEYBOARD_KEY_STATE_REPEATED;
						keyboard->out.key.cp = keyboard_priv->repeat_cp;
						SU_MEMCPY(&event->wayland_keyboard.state, &keyboard->out, sizeof(keyboard->out));
						keyboard_priv->repeat_next = (ms_now + keyboard->out.repeat_rate);
					} else {
						keyboard_priv->repeat_next = -1;
						keyboard_priv->repeat_cp = 0;
					}
				}
				if (keyboard_priv->repeat_next > 0) {
					sw__update_t(keyboard_priv->repeat_next);
				}
			}
#endif /* SW_WITH_WAYLAND_KEYBOARD */
		}
#endif /* SW_WITH_WAYLAND_KEYBOARD || SW_WITH_WAYLAND_CLIPBOARD */

		wayland_out->fds[0].events = POLLIN;
		if (wl_display_flush(sw_priv->backend.wayland.display) == -1) {
			if (errno == EAGAIN) {
				wayland_out->fds[0].events = (POLLIN | POLLOUT);
			} else {
				ret = SU_FALSE;
				goto out;
			}
		}

#if SW_WITH_WAYLAND_CLIPBOARD
		wayland_out->fds_count = 1;
		for ( seat = wayland_out->seats.head; seat; seat = seat->next) {
			sw_wayland_data_device_t *data_device = seat->out.data_device;
			sw__wayland_data_device_t *data_device_priv;
			if (!data_device) {
				continue;
			}
			data_device_priv = (sw__wayland_data_device_t *)data_device->sw__private;
			if (data_device_priv->paste.pfd.fd > 0) {
				struct pollfd *pfd = &data_device_priv->paste.pfd;
				for (;;) {
					ssize_t read_bytes = read( pfd->fd,
						&((uint8_t *)data_device->out.paste.data.ptr)[data_device->out.paste.data.len],
						data_device_priv->paste.data_capacity - data_device->out.paste.data.len);
					if (read_bytes == 0) {
						if (data_device->out.paste.data.len > 0) {
							sw_event_out__t *event = sw__event(SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_DATA, data_device);
							SU_MEMCPY(&event->wayland_data_device.paste_state, &data_device->out.paste, sizeof(data_device->out.paste));
						}
						close(pfd->fd);
						SU_MEMSET(pfd, 0, sizeof(*pfd));
						break;
					} else if (read_bytes == -1) {
						if (errno == EAGAIN) {
							if (SU_UNLIKELY(wayland_priv->fds_capacity == wayland_out->fds_count)) {
								struct pollfd *new_fds;
								wayland_priv->fds_capacity *= 2;
								SU_ARRAY_ALLOC(new_fds, gp_alloc, wayland_priv->fds_capacity);
								SU_MEMCPY(new_fds, wayland_out->fds,
									wayland_out->fds_count * sizeof(wayland_out->fds[0]));
								SU_FREE(gp_alloc, wayland_out->fds);
								wayland_out->fds = new_fds;
							}
							wayland_out->fds[wayland_out->fds_count++] = *pfd;
							break;
						} else if (errno == EINTR) {
							continue;
						} else {
							break;
						}
					} else {
						data_device->out.paste.data.len += (size_t)read_bytes;
						if (data_device->out.paste.data.len == data_device_priv->paste.data_capacity) {
							uint8_t *new_data;
							data_device_priv->paste.data_capacity *= 2;
							SU_ALLOCTS(new_data, gp_alloc, data_device_priv->paste.data_capacity);
							SU_MEMCPY(new_data, data_device->out.paste.data.ptr, data_device->out.paste.data.len);
							SU_FREE(gp_alloc, data_device->out.paste.data.ptr);
							data_device->out.paste.data.ptr = new_data;
						}
					}
				}
			}
			if (data_device_priv->copy.pfd.fd > 0) {
				struct pollfd *pfd = &data_device_priv->copy.pfd;
				for (;;) {
					ssize_t written_bytes = write( pfd->fd,
							&((uint8_t *)data_device->in.copy.data.ptr)[data_device_priv->copy.buf_idx],
							data_device->in.copy.data.len - data_device_priv->copy.buf_idx);
					if (written_bytes <= 0) {
						if (errno == EAGAIN) {
							if (SU_UNLIKELY(wayland_priv->fds_capacity == wayland_out->fds_count)) {
								struct pollfd *new_fds;
								wayland_priv->fds_capacity *= 2;
								SU_ARRAY_ALLOC(new_fds, gp_alloc, wayland_priv->fds_capacity);
								SU_MEMCPY(new_fds, wayland_out->fds,
									wayland_out->fds_count * sizeof(wayland_out->fds[0]));
								SU_FREE(gp_alloc, wayland_out->fds);
								wayland_out->fds = new_fds;
							}
							wayland_out->fds[wayland_out->fds_count++] = *pfd;
							break;
						} else if (errno == EINTR) {
							continue;
						} else {
							break;
						}
					}
					data_device_priv->copy.buf_idx += (size_t)written_bytes;
					if (data_device->in.copy.data.len == data_device_priv->copy.buf_idx) {
						close(pfd->fd);
						SU_MEMSET(pfd, 0, sizeof(*pfd));
						data_device_priv->copy.buf_idx = 0;
						break;
					}
				}
			}
		}
#endif /* SW_WITH_WAYLAND_CLIPBOARD */

		break;
	}
#endif /* SW_WITH_WAYLAND_BACKEND */
	case SW_BACKEND_TYPE_NONE:
		break;
	default:
		SU_ASSERT_UNREACHABLE;
	}

out:
	sw_priv->backend_type = sw->in.backend_type;
	sw__context = old_context;
	return ret;
}

#endif /* defined(SW_IMPLEMENTATION) && !defined(SW__REIMPLEMENTATION_GUARD) */
