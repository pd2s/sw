//#define _XOPEN_SOURCE 700 // realpath
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <sys/types.h>
#include <poll.h>
#include <uchar.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <locale.h>
#include <time.h>
#include <limits.h>
#include <sys/stat.h>
#include <getopt.h>
#include <signal.h>

#include <wayland-client.h>
#include <wayland-util.h>
#include <pixman.h>
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "xdg-shell-protocol.h"
#include "cursor-shape-v1-protocol.h"

#include "macros.h"

#if HAVE_TEXT
#include <fcft/fcft.h>
#endif // HAVE_TEXT

#if HAVE_SVG
#if HAS_INCLUDE(<resvg.h>)
#include <resvg.h>
#else
#include <resvg/resvg.h>
#endif // HAS_INCLUDE
#endif // HAVE_SVG

#include "sw.h"
#include "json.h"

#include "util.h"

#if HAS_INCLUDE(<tinyexpr.h>)
#include <tinyexpr.h>
#else
#include <tinyexpr/<tinyexpr.h>>
#endif // HAS_INCLUDE

ARRAY_DECLARE_DEFINE(te_variable)

static void *malloc_stbi(size_t size);
static void *realloc_sized_stbi(void *ptr, size_t old_size, size_t new_size);

IGNORE_WARNINGS_START

IGNORE_WARNING("-Wduplicated-branches")
IGNORE_WARNING("-Warith-conversion")
IGNORE_WARNING("-Wconversion")
IGNORE_WARNING("-Wsuggest-attribute=pure")
IGNORE_WARNING("-Walloc-zero")
IGNORE_WARNING("-Wpadded")
IGNORE_WARNING("-Wsign-conversion")
IGNORE_WARNING("-Wcast-align")
IGNORE_WARNING("-Wdouble-promotion")
IGNORE_WARNING("-Wextra-semi-stmt")
IGNORE_WARNING("-Wswitch-default")
IGNORE_WARNING("-Wimplicit-int-conversion")
IGNORE_WARNING("-Wimplicit-fallthrough")
IGNORE_WARNING("-Wmissing-prototypes")

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_FAILURE_STRINGS

#define STBI_MALLOC malloc_stbi
#define STBI_FREE free
#define STBI_REALLOC_SIZED realloc_sized_stbi

#if !HAVE_PNG
#define STBI_NO_PNG
#endif // !HAVE_PNG

#if !HAVE_JPG
#define STBI_NO_JPEG
#endif // !HAVE_JPG

#if !HAVE_TGA
#define STBI_NO_TGA
#endif // !HAVE_TGA

#if !HAVE_BMP
#define STBI_NO_BMP
#endif // !HAVE_BMP

#if !HAVE_PSD
#define STBI_NO_PSD
#endif // !HAVE_PSD

#if !HAVE_GIF
#define STBI_NO_GIF
#endif //  !HAVE_GIF

#if !HAVE_HDR
#define STBI_NO_HDR
#endif // !HAVE_HDR

#if !HAVE_PIC
#define STBI_NO_PIC
#endif // !HAVE_PIC

#if !HAVE_PNM
#define STBI_NO_PNM
#endif // !HAVE_PNM

#include <stb_image.h>

IGNORE_WARNINGS_END

ARRAY_DECLARE_DEFINE(uint32_t)

typedef struct surface* struct_surface_ptr;
ARRAY_DECLARE_DEFINE(struct_surface_ptr)

struct output {
	uint32_t wl_name;
	int32_t scale, width, height;
	enum wl_output_transform transform;
	uint32_t pad;
	struct wl_output *wl_output;
	string_t name;

	array_struct_surface_ptr_t layers;
};

typedef struct output* struct_output_ptr;
ARRAY_DECLARE_DEFINE(struct_output_ptr)

struct box {
	int32_t x, y;
	int32_t width, height;
};

typedef struct box struct_box;
ARRAY_DECLARE_DEFINE(struct_box)

struct surface_block_border {
	string_t width;
	pixman_image_t *color; // solid fill
};

struct surface_block_box {
	int32_t x, y;
	int32_t width, height;
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
};

typedef struct surface_block struct_surface_block;
ARRAY_DECLARE(struct_surface_block)

struct surface_block {
	enum sw_surface_block_type type;
	enum sw_surface_block_type_composite_block_layout composite_layout;
	array_struct_surface_block_t composite_children;

	bool32_t destroy_content_image; // TODO: remove
	uint32_t pad;
	pixman_image_t *content_image;
	union {
		struct {
			struct surface_block_border border_left;
			struct surface_block_border border_right;
			struct surface_block_border border_bottom;
			struct surface_block_border border_top;
		};
		struct {
			struct surface_block_border borders[4]; // left, right, bottom, top
		};
	};
	pixman_image_t *color; // solid fill
	enum sw_surface_block_content_anchor content_anchor;
	enum sw_surface_block_anchor anchor;
	string_t x, y;
	string_t min_width, max_width;
	string_t min_height, max_height;
	string_t content_width, content_height;
	uint32_t expand; // enum sw_surface_block_expand |
	enum sw_surface_block_content_transform content_transform;
	uint64_t id;

	struct surface_block_box box;
	struct surface *surface;
};

ARRAY_DEFINE(struct_surface_block)

struct surface_type_layer {
	struct zwlr_layer_surface_v1 *layer_surface;
	int32_t desired_exclusive_zone;
	int32_t exclusive_zone;
	int32_t margins[4]; // top, right, bottom, left
	uint32_t anchor; // enum zwlr_layer_surface_v1_anchor |
	enum zwlr_layer_shell_v1_layer layer;
};

struct surface_type_popup {
	struct xdg_surface *xdg_surface;
	struct xdg_popup *xdg_popup;
	struct xdg_positioner *xdg_positioner;
	enum xdg_positioner_gravity gravity;
	uint32_t constraint_adjustment; // enum xdg_positioner_constraint_adjustment |
	int32_t desired_x, desired_y;
	struct surface *parent;
	struct {
		uint32_t serial;
		uint32_t pad;
		struct wl_seat *seat;
	} grab;
};

struct surface {
	bool32_t dirty;

	enum sw_surface_type type;
	union {
		struct surface_type_layer layer;
		struct surface_type_popup popup;
	};

	struct output *output;
	struct wl_surface *wl_surface;
	struct surface_buffer *buffer;
	int32_t width, height;
	int32_t desired_width, desired_height;
	enum wp_cursor_shape_device_v1_shape cursor_shape;
	int32_t scale;
	array_struct_box_t input_regions;

	struct surface_block layout_root;

	array_struct_surface_ptr_t popups;

	string_t userdata;
};

struct pointer {
	struct seat *seat;
	struct wl_pointer *wl_pointer;
	struct wp_cursor_shape_device_v1 *cursor_shape_device;

	struct surface *surface;
	uint32_t wl_pointer_enter_serial;

	int32_t pos_x, pos_y;
	bool32_t pos_dirty;

	uint32_t btn_code;
	enum wl_pointer_button_state btn_state;
	uint32_t btn_serial;
	bool32_t btn_dirty;

	double scroll_vector_length;
	enum wl_pointer_axis scroll_axis;
	bool32_t scroll_dirty;
};

struct seat {
	struct wl_seat *wl_seat;
	string_t name;
	uint32_t wl_name;
	uint32_t pad;
	struct pointer *pointer;
	struct {
		array_uint32_t_t serials; // wl_pointer_button, TODO: wl_touch_down/up,
		uint8_t index;
		uint8_t pad1;
		uint16_t pad2;
		uint32_t pad3;
	} popup_grab;
};

typedef struct seat* struct_seat_ptr;
ARRAY_DECLARE_DEFINE(struct_seat_ptr)

struct surface_buffer {
	struct surface *surface;
	struct wl_buffer *wl_buffer;
	pixman_image_t *image;
	uint32_t *pixels;
	uint32_t size;
	bool32_t busy;
};

struct gif_frame {
	pixman_image_t *image;
	uint16_t delay; // msec
	uint16_t pad1;
	uint16_t pad2;
	uint16_t pad3;
};

typedef struct gif_frame struct_gif_frame;
ARRAY_DECLARE_DEFINE(struct_gif_frame)

struct gif_frame_data {
	array_struct_gif_frame_t frames;
	size_t frame_idx;
	int64_t frame_end; // ms
	pixman_image_t *image;
};

enum image_data_type {
	IMAGE_DATA_TYPE_NONE,
#if HAVE_SVG
	IMAGE_DATA_TYPE_SVG_TREE,
#endif // HAVE_SVG
#if HAVE_GIF
	IMAGE_DATA_TYPE_MULTIFRAME_GIF,
#endif // HAVE_GIF
};

struct image_data {
	uint32_t pad;
	enum image_data_type type;
	union {
#if HAVE_SVG
		struct {
			resvg_render_tree *tree;
		} svg;
#endif // HAVE_SVG
		struct gif_frame_data *gif;
	};

	uint32_t *pixels;
};

#if HAVE_TEXT
struct text_run_cache_entry {
	struct fcft_font *font;
	struct fcft_text_run *text_run;
};

typedef struct text_run_cache_entry struct_text_run_cache_entry;
ARRAY_DECLARE_DEFINE(struct_text_run_cache_entry)

struct text_run_cache {
	HASH_TABLE_STRUCT_FIELDS(string_t)
	array_struct_text_run_cache_entry_t items;
};

typedef struct text_run_cache struct_text_run_cache;
HASH_TABLE_DECLARE_DEFINE(struct_text_run_cache, stbds_hash_string, string_equal, 16)
#endif // HAVE_TEXT

struct image_cache {
	HASH_TABLE_STRUCT_FIELDS(string_t)
	enum sw_surface_block_type_image_image_type type;
	uint32_t pad;
	struct timespec mtim_ts;
	pixman_image_t *image;
};

typedef struct image_cache struct_image_cache;
HASH_TABLE_DECLARE_DEFINE(struct_image_cache, stbds_hash_string, string_equal, 16)

struct buffer {
	uint8_t *data;
	size_t size, index;
};

struct poll_timer {
	int64_t expiry; // msec
	void *data;
	void (*callback)(void *data);
};

typedef struct poll_timer struct_poll_timer;
ARRAY_DECLARE_DEFINE(struct_poll_timer)

enum poll_fd {
	POLL_FD_STDIN,
	POLL_FD_STDOUT,
	POLL_FD_WAYLAND,
	POLL_FD_LAST,
};

static struct {
	struct pollfd poll_fds[POLL_FD_LAST];
	array_struct_poll_timer_t poll_timers;

	struct buffer stdin_buffer;

	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct xdg_wm_base *wm_base;
	struct wp_cursor_shape_manager_v1 *cursor_shape_manager;

	array_struct_output_ptr_t outputs;
	array_struct_seat_ptr_t seats;

#if HAVE_TEXT
	hash_table_struct_text_run_cache_t text_run_cache;
#endif // HAVE_TEXT
	hash_table_struct_image_cache_t image_cache;

	bool32_t events;
	bool32_t dirty;
	string_t userdata;
	struct json_writer writer;
	struct json_writer userdata_writer;
	struct json_tokener tokener;
	struct json_ast json_ast;

	bool32_t running;
	uint32_t pad;
} state;

static void *malloc_stbi(size_t size) {
	return aalloc(64, size);
}

static void *realloc_sized_stbi(void *ptr, size_t old_size, size_t new_size) {
	void *ret = aalloc(64, new_size);
	if (ptr) {
		memcpy(ret, ptr, MIN(old_size, new_size));
		free(ptr);
	}

	return ret;
}

static void buffer_init(struct buffer *buffer, size_t size) {
	buffer->data = malloc(size);
	buffer->size = size;
	buffer->index = 0;
}

static void buffer_fini(struct buffer *buffer) {
	free(buffer->data);
}

static bool32_t surface_render(struct surface *surface);
#if HAVE_SVG
static pixman_image_t *render_svg(resvg_render_tree *tree, int32_t target_width, int32_t target_height);
#endif // HAVE_SVG

static void surface_handle_render(void *data) {
	surface_render((struct surface *)data);
}

static void surface_block_render(struct surface_block *block, pixman_image_t *dest) {
	if (!block->surface) {
		return;
	}

	if (block->type == SW_SURFACE_BLOCK_TYPE_COMPOSITE) {
		for (size_t i = 0; i < block->composite_children.len; ++i) {
			surface_block_render(array_struct_surface_block_get_ptr(&block->composite_children, i),
				block->content_image);
		}
	}

	struct surface_block_box box = block->box;

	if (block->color) {
		pixman_image_composite32(PIXMAN_OP_OVER, block->color, NULL, dest,
			0, 0, 0, 0,
			box.x + box.border_left,
			box.y + box.border_top,
			box.width - box.border_right - box.border_left,
			box.height - box.border_bottom - box.border_top);
	}

	if ((box.border_left > 0) && block->border_left.color) {
		pixman_image_composite32(PIXMAN_OP_OVER, block->border_left.color, NULL, dest,
			0, 0, 0, 0, box.x, box.y, box.border_left, box.height);
	}

	if ((box.border_right > 0) && block->border_right.color) {
		pixman_image_composite32(PIXMAN_OP_OVER, block->border_right.color, NULL, dest,
			0, 0, 0, 0, box.x + box.width - box.border_right, box.y,
			box.border_right, box.height);
	}

	if ((box.border_bottom > 0) && block->border_bottom.color) {
		pixman_image_composite32(PIXMAN_OP_OVER, block->border_bottom.color, NULL, dest,
			0, 0, 0, 0, box.x + box.border_left,
			box.y + box.height - box.border_bottom,
			box.width - box.border_left - box.border_right,
			box.border_bottom);
	}

	if ((box.border_top > 0) && block->border_top.color) {
		pixman_image_composite32(PIXMAN_OP_OVER, block->border_top.color, NULL, dest,
			0, 0, 0, 0, box.x + box.border_left, box.y,
			box.width - box.border_left - box.border_right,
			box.border_top);
	}

	if (block->content_image) {
		struct image_data *image_data = pixman_image_get_destroy_data(block->content_image);
		(void)image_data;
#if HAVE_GIF
		if (image_data && (image_data->type == IMAGE_DATA_TYPE_MULTIFRAME_GIF)) {
			int64_t now_msec = now_ms();
			struct gif_frame_data *data = image_data->gif;
			if (now_msec >= data->frame_end) {
				struct gif_frame frame = array_struct_gif_frame_get(
					&data->frames, data->frame_idx);
				if (++data->frame_idx >= data->frames.len) {
					data->frame_idx = 0;
				}
				data->frame_end = now_msec + frame.delay;
				array_struct_poll_timer_add(&state.poll_timers, (struct poll_timer){
					.expiry = data->frame_end,
					.data = block->surface,
					.callback = surface_handle_render,
				});
				data->image = frame.image;
			}
			block->content_image = data->image;
			block->destroy_content_image = false;
		}
#endif // HAVE_GIF

		int content_image_width = pixman_image_get_width(block->content_image);
		int content_image_height = pixman_image_get_height(block->content_image);
		if ((block->content_transform % 2) == 0) {
			int tmp = content_image_width;
			content_image_width = content_image_height;
			content_image_height = tmp;
		}

		pixman_transform_t transform;
		pixman_transform_init_identity(&transform);

		if ((box.content_width != content_image_width)
				|| (box.content_height != content_image_height)) {
#if HAVE_SVG
			if (image_data && (image_data->type == IMAGE_DATA_TYPE_SVG_TREE)) {
				pixman_image_unref(block->content_image);
				block->content_image = render_svg(image_data->svg.tree, box.content_width, box.content_height);
			} else
#endif // HAVE_SVG
				pixman_transform_scale(&transform, NULL,
					pixman_int_to_fixed(content_image_width) / box.content_width,
					pixman_int_to_fixed(content_image_height) / box.content_height);
		}

		switch (block->content_transform) {
		case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_NORMAL:
		case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_FLIPPED:
			break;
		case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_90:
		case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_FLIPPED_90:
			pixman_transform_rotate(&transform, NULL, 0, pixman_fixed_1);
			pixman_transform_translate(&transform, NULL,
				pixman_int_to_fixed(pixman_image_get_width(block->content_image)), 0);
			break;
		case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_180:
		case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_FLIPPED_180:
			pixman_transform_rotate(&transform, NULL, pixman_fixed_minus_1, 0);
			pixman_transform_translate(&transform, NULL,
				pixman_int_to_fixed(pixman_image_get_width(block->content_image)),
				pixman_int_to_fixed(pixman_image_get_height(block->content_image)));
			break;
		case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_270:
		case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_FLIPPED_270:
			pixman_transform_rotate(&transform, NULL, 0, pixman_fixed_minus_1);
			pixman_transform_translate(&transform, NULL, 0,
				pixman_int_to_fixed(pixman_image_get_height(block->content_image)));
			break;
		case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_DEFAULT:
		default:
			ASSERT_UNREACHABLE;
		}

		if (block->content_transform >= SW_SURFACE_BLOCK_CONTENT_TRANSFORM_FLIPPED) {
			pixman_transform_translate(&transform, NULL,
				-pixman_int_to_fixed(pixman_image_get_width(block->content_image)), 0);
			pixman_transform_scale(&transform, NULL, pixman_fixed_minus_1, pixman_fixed_1);
		}

		pixman_image_set_transform(block->content_image, &transform);

		pixman_region32_t clip_region;
		pixman_region32_init_rect(&clip_region, box.x, box.y, (unsigned int)box.width, (unsigned int)box.height);
		pixman_image_set_clip_region32(dest, &clip_region);

		// ? TODO: move to surface_block_prepare
		int32_t available_width = box.width - box.border_left - box.border_right;
		int32_t available_height = box.height - box.border_bottom - box.border_top;
		int32_t content_x = box.x + box.border_left;
		int32_t content_y = box.y + box.border_top;
		switch (block->content_anchor) {
		case SW_SURFACE_BLOCK_CONTENT_ANCHOR_LEFT_TOP:
			break;
		case SW_SURFACE_BLOCK_CONTENT_ANCHOR_LEFT_CENTER:
			content_y += ((available_height - box.content_height) / 2);
			break;
		case SW_SURFACE_BLOCK_CONTENT_ANCHOR_LEFT_BOTTOM:
			content_y += (available_height - box.content_height);
			break;
		case SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_TOP:
			content_x += ((available_width - box.content_width) / 2);
			break;
		case SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_CENTER:
			content_x += ((available_width - box.content_width) / 2);
			content_y += ((available_height - box.content_height) / 2);
			break;
		case SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_BOTTOM:
			content_x += ((available_width - box.content_width) / 2);
			content_y += (available_height - box.content_height);
			break;
		case SW_SURFACE_BLOCK_CONTENT_ANCHOR_RIGHT_TOP:
			content_x += (available_width - box.content_width);
			break;
		case SW_SURFACE_BLOCK_CONTENT_ANCHOR_RIGHT_CENTER:
			content_x += (available_width - box.content_width);
			content_y += ((available_height - box.content_height) / 2);
			break;
		case SW_SURFACE_BLOCK_CONTENT_ANCHOR_RIGHT_BOTTOM:
			content_x += (available_width - box.content_width);
			content_y += (available_height - box.content_height);
			break;
		case SW_SURFACE_BLOCK_CONTENT_ANCHOR_DEFAULT:
		default:
			ASSERT_UNREACHABLE;
		}

		pixman_image_composite32(PIXMAN_OP_OVER, block->content_image, NULL, dest,
			0, 0, 0, 0, content_x, content_y, box.content_width, box.content_height);

		pixman_image_set_transform(block->content_image, NULL);
		pixman_image_set_clip_region32(dest, NULL);
	}
}

static int32_t eval_surface_block_te_expr(string_t expr, array_te_variable_t *vars) {
	int32_t ret = 0;
	if (expr.len > 0) {
		assert(expr.nul_terminated);
		te_expr *e = te_compile(expr.s, vars->items, (int)vars->len, NULL);
		if (e) {
			double result = te_eval(e);
			if (result > 0) {
				ret = (int32_t)result;
			}
			te_free(e);
		}
	}

	return ret;
}

static void surface_block_prepare(struct surface_block *block, array_te_variable_t *te_vars,
	struct surface_block_box *overrides);

static void surface_block_expand(struct surface_block *block, array_te_variable_t *te_vars,
		int32_t available_width, int32_t available_height) {
	if ((block->surface == NULL) || (block->expand == SW_SURFACE_BLOCK_EXPAND_NONE)) {
		return;
	}

	int32_t x = block->box.x;
	int32_t y = block->box.y;
	int32_t width = block->box.width;
	int32_t height = block->box.height;

	if (block->expand & SW_SURFACE_BLOCK_EXPAND_LEFT) {
		width += x;
		x = 0;
	}

	if (block->expand & SW_SURFACE_BLOCK_EXPAND_RIGHT) {
		width = (available_width - x);
	}

	if (block->expand & SW_SURFACE_BLOCK_EXPAND_TOP) {
		height += y;
		y = 0;
	}

	if (block->expand & SW_SURFACE_BLOCK_EXPAND_BOTTOM) {
		height = (available_height - y);
	}

	if (block->expand & SW_SURFACE_BLOCK_EXPAND_CONTENT) {
		surface_block_prepare(block, te_vars, &(struct surface_block_box){
			.border_left = block->box.border_left,
			.border_bottom = block->box.border_bottom,
			.border_right = block->box.border_right,
			.border_top = block->box.border_top,
			.x = x,
			.y = y,
			.content_width = width - block->box.border_left - block->box.border_right,
			.width = width,
			.content_height = height - block->box.border_top - block->box.border_bottom,
			.height = height,
		});
	} else {
		block->box.x = x;
		block->box.y = y;
		block->box.width = width;
		block->box.height = height;
	}
}

static void image_handle_destroy(pixman_image_t *image, void *data) {
	(void)image;
	struct image_data *image_data = data;
	switch (image_data->type) {
	case IMAGE_DATA_TYPE_NONE:
		break;
#if HAVE_SVG
	case IMAGE_DATA_TYPE_SVG_TREE:
		if (image_data->svg.tree) {
			resvg_tree_destroy(image_data->svg.tree);
		}
		break;
#endif // HAVE_SVG
#if HAVE_GIF
	case IMAGE_DATA_TYPE_MULTIFRAME_GIF: {
		struct gif_frame_data *gif = image_data->gif;
		for (size_t i = 0; i < gif->frames.len; ++i) {
			pixman_image_t *frame_image = array_struct_gif_frame_get(&gif->frames, i).image;
			struct image_data *frame_data = pixman_image_get_destroy_data(frame_image);
			frame_data->type = IMAGE_DATA_TYPE_NONE;
			pixman_image_unref(frame_image);
		}
		array_struct_gif_frame_fini(&gif->frames);
		free(gif);
		break;
	}
#endif // HAVE_GIF
	default:
		ASSERT_UNREACHABLE;
	}

	free(image_data->pixels);

	free(image_data);
}

static pixman_image_t *image_create(int width, int height, struct image_data **data_out) {
	assert(width > 0);
	assert(height > 0);

	struct image_data *data = malloc(sizeof(struct image_data));
	data->type = IMAGE_DATA_TYPE_NONE;

	int stride = width * 4;
	size_t size = (size_t)height * (size_t)stride;
	data->pixels = aalloc(64, size);
	memset(data->pixels, 0, size);
	pixman_image_t *image = pixman_image_create_bits(PIXMAN_a8r8g8b8, width, height, data->pixels, stride);
	pixman_image_set_destroy_function(image, image_handle_destroy, data);

	if (data_out) {
		*data_out = data;
	}
	return image;
}

static void surface_block_prepare(struct surface_block *block, array_te_variable_t *te_vars,
		struct surface_block_box *overrides) {
	if (!block->surface) {
		return;
	}

	if (block->type == SW_SURFACE_BLOCK_TYPE_COMPOSITE) {
		int32_t content_width, content_height;
		if (overrides) {
			content_width = overrides->content_width;
			content_height = overrides->content_height;
		} else {
			content_width = eval_surface_block_te_expr(block->content_width, te_vars);
			content_height = eval_surface_block_te_expr(block->content_height, te_vars);
		}

		bool32_t auto_content_width = (content_width == 0);
		bool32_t auto_content_height = (content_height == 0);
		bool32_t vertical = (block->composite_layout == SW_SURFACE_BLOCK_TYPE_COMPOSITE_BLOCK_LAYOUT_VERTICAL);
		int32_t l = 0, c = 0, r;

		for (size_t i = 0; i < block->composite_children.len; ++i) {
			struct surface_block *b = array_struct_surface_block_get_ptr(&block->composite_children, i);
			if (!b->surface) {
				continue;
			}
			surface_block_prepare(b, te_vars, NULL);
			if (b->anchor != SW_SURFACE_BLOCK_ANCHOR_NONE) {
				if (vertical) {
					if (b->anchor == SW_SURFACE_BLOCK_ANCHOR_CENTER) {
						c += b->box.height;
					} else if (auto_content_height) {
						content_height += b->box.height;
					}
					if (auto_content_width && (b->box.width > content_width)) {
						content_width = b->box.width;
					}
				} else {
					if (b->anchor == SW_SURFACE_BLOCK_ANCHOR_CENTER) {
						c += b->box.width;
					} else if (auto_content_width) {
						content_width += b->box.width;
					}
					if (auto_content_height && (b->box.height > content_height)) {
						content_height = b->box.height;
					}
				}
			}
			if (b->id > 0) {
				te_variable *v = array_te_variable_add(te_vars, (te_variable){
					.address = (double[]){ b->content_image ? pixman_image_get_width(b->content_image) : 0.0 },
				});
				stbsp_snprintf(v->name, sizeof(v->name), "block_%lu_content_width", b->id);
				v = array_te_variable_add(te_vars, (te_variable){
					.address = (double[]){ b->content_image ? pixman_image_get_height(b->content_image) : 0.0 },
				});
				stbsp_snprintf(v->name, sizeof(v->name), "block_%lu_content_height", b->id);
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

		for (size_t i = 0; i < block->composite_children.len; ++i) {
			struct surface_block *b = array_struct_surface_block_get_ptr(&block->composite_children, i);
			if (!b->surface) {
				continue;
			}
			if (b->anchor != SW_SURFACE_BLOCK_ANCHOR_NONE) {
				if (vertical) {
					switch (b->anchor) {
					case SW_SURFACE_BLOCK_ANCHOR_TOP:
					case SW_SURFACE_BLOCK_ANCHOR_LEFT:
						b->box.x = 0;
						b->box.y = l;
						l += b->box.height;
						break;
					case SW_SURFACE_BLOCK_ANCHOR_CENTER:
						b->box.y = c;
						c += b->box.height;
						b->box.x = 0;
						break;
					case SW_SURFACE_BLOCK_ANCHOR_BOTTOM:
					case SW_SURFACE_BLOCK_ANCHOR_RIGHT:
						b->box.x = 0;
						r -= b->box.height;
						b->box.y = r;
						break;
					case SW_SURFACE_BLOCK_ANCHOR_NONE:
						break;
					case SW_SURFACE_BLOCK_ANCHOR_DEFAULT:
					default:
						ASSERT_UNREACHABLE;
					}
				} else {
					switch (b->anchor) {
					case SW_SURFACE_BLOCK_ANCHOR_TOP:
					case SW_SURFACE_BLOCK_ANCHOR_LEFT:
						b->box.y = 0;
						b->box.x = l;
						l += b->box.width;
						break;
					case SW_SURFACE_BLOCK_ANCHOR_CENTER:
						b->box.x = c;
						c += b->box.width;
						b->box.y = 0;
						break;
					case SW_SURFACE_BLOCK_ANCHOR_BOTTOM:
					case SW_SURFACE_BLOCK_ANCHOR_RIGHT:
						b->box.y = 0;
						r -= b->box.width;
						b->box.x = r;
						break;
					case SW_SURFACE_BLOCK_ANCHOR_NONE:
						break;
					case SW_SURFACE_BLOCK_ANCHOR_DEFAULT:
					default:
						ASSERT_UNREACHABLE;
					}
				}
			}
			surface_block_expand(b, te_vars, content_width, content_height);
		}

		if (!block->content_image || (content_width != pixman_image_get_width(block->content_image))
				|| (content_height != pixman_image_get_height(block->content_image))) {
			if (block->content_image) {
				pixman_image_unref(block->content_image);
			}
			block->content_image = ((content_width > 0) && (content_height > 0))
				? image_create(content_width, content_height, NULL)
				: NULL;
		} else {
			memset(pixman_image_get_data(block->content_image),
				0,
				(size_t)content_width * 4 * (size_t)content_height);
		}
	}

	if (overrides) {
		block->box = *overrides;
	} else {
		int32_t content_width = block->content_image ? pixman_image_get_width(block->content_image) : 0;
		int32_t content_height = block->content_image ? pixman_image_get_height(block->content_image) : 0;

		array_te_variable_add(te_vars, (te_variable){
			.name = "content_width",
			.address = (double[]){ content_width },
		});
		array_te_variable_add(te_vars, (te_variable){
			.name = "content_height",
			.address = (double[]){ content_height },
		});

		if ((block->type != SW_SURFACE_BLOCK_TYPE_SPACER) && (block->type != SW_SURFACE_BLOCK_TYPE_COMPOSITE)) {
			int32_t tmp;
			if ((tmp = eval_surface_block_te_expr(block->content_width, te_vars)) > 0) {
				content_width = tmp;
			}
			if ((tmp = eval_surface_block_te_expr(block->content_height, te_vars)) > 0) {
				content_height = tmp;
			}
		}

		int32_t x = 0, y = 0;
		if (block->anchor == SW_SURFACE_BLOCK_ANCHOR_NONE) {
			x = eval_surface_block_te_expr(block->x, te_vars);
			y = eval_surface_block_te_expr(block->y, te_vars);
		}

		int32_t min_width = eval_surface_block_te_expr(block->min_width, te_vars);
		int32_t max_width = eval_surface_block_te_expr(block->max_width, te_vars);
		int32_t min_height = eval_surface_block_te_expr(block->min_height, te_vars);
		int32_t max_height = eval_surface_block_te_expr(block->max_height, te_vars);
		int32_t border_left = eval_surface_block_te_expr(block->border_left.width, te_vars);
		int32_t border_right = eval_surface_block_te_expr(block->border_right.width, te_vars);
		int32_t border_bottom = eval_surface_block_te_expr(block->border_bottom.width, te_vars);
		int32_t border_top = eval_surface_block_te_expr(block->border_top.width, te_vars);

		array_te_variable_remove(te_vars, 2);

		if ((block->content_image) && ((block->content_transform % 2) == 0)) {
			int32_t tmp = content_width;
			content_width = content_height;
			content_height = tmp;
		}

		int32_t width = content_width + border_left + border_right;
		int32_t height = content_height + border_bottom + border_top;

		if ((min_width > 0) && (max_width > 0) && (max_width < min_width)) {
			min_width = max_width = 0;
		}
		if (width < min_width) {
			width = min_width;
		} else if ((max_width > 0) && (width > max_width)) {
			width = max_width;
		}

		if ((min_height > 0) && (max_height > 0) && (max_height < min_height)) {
			min_height = max_height = 0;
		}
		if (height < min_height) {
			height = min_height;
		} else if ((max_height > 0) && (height > max_height)) {
			height = max_height;
		}

		block->box.x = x;
		block->box.y = y;
		block->box.width = width;
		block->box.height = height;
		block->box.content_width = content_width;
		block->box.content_height = content_height;
		block->box.border_left = border_left;
		block->box.border_right = border_right;
		block->box.border_bottom = border_bottom;
		block->box.border_top = border_top;
	}
}

static bool32_t surface_render(struct surface *surface) {
	struct surface_buffer *buffer = surface->buffer;
	if (buffer && buffer->busy) {
		surface->dirty = true;
		return true;
	}

	// TODO: rework

	array_te_variable_t te_vars;
	array_te_variable_init(&te_vars, 32);
	array_te_variable_add(&te_vars, (te_variable){
		.name = "output_width",
		.address = (double[]){ surface->output->width },
	});
	array_te_variable_add(&te_vars, (te_variable){
		.name = "output_height",
		.address = (double[]){ surface->output->height },
	});
	array_te_variable_add(&te_vars, (te_variable){
		.name = "surface_width",
		.address = (double[]){ surface->width },
	});
	array_te_variable_add(&te_vars, (te_variable){
		.name = "surface_height",
		.address = (double[]){ surface->height },
	});

	surface_block_prepare(&surface->layout_root, &te_vars, NULL);

	te_vars.len = 4;

	bool32_t ret = true;
	int32_t surface_width, surface_height;
	switch (surface->type) {
	case SW_SURFACE_TYPE_LAYER: {
		// ? TODO: allow surface sizing with SW_SURFACE_LAYER_ANCHOR_ALL (the same can be achieved with 0 anchor)
		if (surface->layer.anchor != SW_SURFACE_LAYER_ANCHOR_ALL) {
			if (surface->desired_width < 0) {
				if (surface->layout_root.box.width <= 0) {
					ret = false;
					goto cleanup;
				}
				surface_width = surface->layout_root.box.width;
			} else {
				surface_width = surface->desired_width;
			}
			if (surface->desired_height < 0) {
				if (surface->layout_root.box.height <= 0) {
					ret = false;
					goto cleanup;
				}
				surface_height = surface->layout_root.box.height;
			} else {
				surface_height = surface->desired_height;
			}
			if (!buffer ||
						((surface_width != 0) && (surface->width != surface_width)) ||
						((surface_height != 0) && (surface->height != surface_height))) {
				zwlr_layer_surface_v1_set_size(surface->layer.layer_surface,
					(uint32_t)(surface_width / surface->scale),
					(uint32_t)(surface_height / surface->scale));
			}
		} else {
			surface_width = surface->width;
			surface_height = surface->height;
		}
		int32_t exclusive_zone = surface->layer.desired_exclusive_zone;
		if (exclusive_zone < -1) {
			switch (surface->layer.anchor) {
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
		if (surface->layer.exclusive_zone != exclusive_zone) {
			zwlr_layer_surface_v1_set_exclusive_zone(surface->layer.layer_surface,
				exclusive_zone / surface->scale);
			surface->layer.exclusive_zone = exclusive_zone;
		}
		break;
	}
	case SW_SURFACE_TYPE_POPUP: {
		surface_width = (surface->desired_width <= 0) ? surface->layout_root.box.width : surface->desired_width;
		surface_height = (surface->desired_height <= 0) ? surface->layout_root.box.height : surface->desired_height;
		if ((surface_width <= 0) || (surface_height <= 0)) {
			ret = false;
			goto cleanup;
		}
		if (!buffer || (surface->width != surface_width) || (surface->height != surface_height)) {
			xdg_positioner_set_size(surface->popup.xdg_positioner,
				surface_width / surface->scale, surface_height / surface->scale);
			if (!surface->popup.xdg_popup) {
				surface_block_expand(&surface->layout_root, &te_vars, surface_width, surface_height);
				goto cleanup;
			}
			xdg_popup_reposition(surface->popup.xdg_popup, surface->popup.xdg_positioner, 0);
		}
		break;
	}
	default:
		ASSERT_UNREACHABLE;
		return false;
	}

	surface_block_expand(&surface->layout_root, &te_vars,
		(surface_width <= 0) ? surface->width : surface_width,
		(surface_height <= 0) ? surface->height : surface_height);

	if (buffer) {
		memset(buffer->pixels, 0, buffer->size);

		surface_block_render(&surface->layout_root, buffer->image);

		wl_surface_set_buffer_scale(surface->wl_surface, surface->scale);
		wl_surface_attach(surface->wl_surface, buffer->wl_buffer, 0, 0);
		wl_surface_damage_buffer(surface->wl_surface, 0, 0, surface->width, surface->height);

		buffer->busy = true;
		surface->dirty = false;
	}

	wl_surface_commit(surface->wl_surface);

cleanup:
	array_te_variable_fini(&te_vars);

	return ret;
}

static void surface_block_fini(struct surface_block *block) {
	if  (block->type == SW_SURFACE_BLOCK_TYPE_COMPOSITE) {
		for (size_t i = 0; i < block->composite_children.len; ++i) {
			surface_block_fini(array_struct_surface_block_get_ptr(&block->composite_children, i));
		}
		array_struct_surface_block_fini(&block->composite_children);
	}

	// TODO: remove multiframe gif timer

	if (block->destroy_content_image && block->content_image) {
		pixman_image_unref(block->content_image);
	}
	if (block->color) {
		pixman_image_unref(block->color);
	}

	for (size_t i = 0; i < LENGTH(block->borders); ++i) {
		struct surface_block_border border = block->borders[i];
		string_fini(&border.width);
		if (border.color) {
			pixman_image_unref(border.color);
		}
	}

	string_fini(&block->content_width);
	string_fini(&block->content_height);
	string_fini(&block->min_width);
	string_fini(&block->max_width);
	string_fini(&block->min_height);
	string_fini(&block->max_height);
	string_fini(&block->x);
	string_fini(&block->y);
}

static pixman_color_t to_pixman_color(union sw_color color) {
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

static pixman_image_t *load_pixmap(string_t path) {
	assert(path.nul_terminated);

	FILE *f = fopen(path.s, "r");
	if (f == NULL) {
		return NULL;
	}

	struct {
		uint32_t width;
		uint32_t height;
		uint32_t pixels[];
	} pixmap;

	pixman_image_t *image = NULL;
	if (fread(&pixmap, 1, sizeof(pixmap), f) != sizeof(pixmap)) {
		goto cleanup;
	}

	if ((pixmap.width == 0) || (pixmap.height == 0)) {
		goto cleanup;
	}

	struct image_data *image_data;
	image = image_create((int)pixmap.width, (int)pixmap.height, &image_data);

	size_t size = pixmap.width * pixmap.height * 4;
	if (fread(image_data->pixels, 1, size, f) != size) {
		pixman_image_unref(image);
		image = NULL;
		goto cleanup;
	}

	argb_premultiply_alpha(image_data->pixels, image_data->pixels, pixmap.width * pixmap.height);

cleanup:
	fclose(f);
    return image;
}

#if HAVE_SVG
static pixman_image_t *render_svg(resvg_render_tree *tree, int32_t target_width, int32_t target_height) {
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

	assert(width > 0);
	assert(height > 0);

	struct image_data *image_data;
	pixman_image_t *image = image_create(width, height, &image_data);

	resvg_render(tree, transform, (uint32_t)width, (uint32_t)height, (char *)image_data->pixels);

	abgr_to_argb(image_data->pixels, image_data->pixels, (size_t)width * (size_t)height);
	pixman_image_set_filter(image, PIXMAN_FILTER_BEST, NULL, 0);

	image_data->type = IMAGE_DATA_TYPE_SVG_TREE;
	image_data->svg.tree = NULL;

	return image;
}

static pixman_image_t *load_svg(string_t path) {
	assert(path.nul_terminated);

	pixman_image_t *image = NULL;
	resvg_render_tree *tree = NULL;
	resvg_options *opt = resvg_options_create();
	int32_t ret = resvg_parse_tree_from_file(path.s, opt, &tree);
    if (ret == RESVG_OK) {
		image = render_svg(tree, -1, -1);
    } else {
		log_stderr(STRING_FMT": resvg_parse_tree_from_file failed. code = %d", STRING_ARGS(path), ret);
	}

	if (opt) {
		resvg_options_destroy(opt);
	}
	if (tree) {
		if (image) {
			struct image_data *data = pixman_image_get_destroy_data(image);
			data->type = IMAGE_DATA_TYPE_SVG_TREE;
			data->svg.tree = tree;
		} else {
			resvg_tree_destroy(tree);
		}
	}

	return image;
}
#endif // HAVE_SVG

#if HAVE_PNG
static pixman_image_t *load_png(string_t path) {
	assert(path.nul_terminated);

	FILE *file = fopen(path.s, "rb");
	if (!file) {
		return NULL;
	}

	stbi__context ctx;
	stbi__start_file(&ctx, file);

	stbi__result_info ri = {
		.bits_per_channel = 8,
	};

	pixman_image_t *image = NULL;
	if (stbi__png_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__png_load(&ctx, &width, &height, &unused, 4, &ri);
		if (src && (width > 0) && (height > 0)) {
			if (ri.bits_per_channel != 8) {
				assert(ri.bits_per_channel == 16);
				src = (uint32_t *)(void *)stbi__convert_16_to_8((stbi__uint16 *)src, width, height, 4);
			}
			struct image_data *image_data;
			image = image_create(width, height, &image_data);
			abgr_to_argb_premultiply_alpha(src, image_data->pixels, (size_t)width * (size_t)height);
		}
		free(src);
	}

	fclose(file);
	return image;
}
#endif // HAVE_PNG

#if HAVE_JPG
static pixman_image_t *load_jpg(string_t path) {
	assert(path.nul_terminated);

	FILE *file = fopen(path.s, "rb");
	if (!file) {
		return NULL;
	}

	stbi__context ctx;
	stbi__start_file(&ctx, file);

	pixman_image_t *image = NULL;
	if (stbi__jpeg_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__jpeg_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			struct image_data *image_data;
			image = image_create(width, height, &image_data);
			abgr_to_argb_premultiply_alpha(src, image_data->pixels, (size_t)width * (size_t)height);
		}
		free(src);
	}

	fclose(file);
	return image;
}
#endif // HAVE_JPG

#if HAVE_TGA
static pixman_image_t *load_tga(string_t path) {
	assert(path.nul_terminated);

	FILE *file = fopen(path.s, "rb");
	if (!file) {
		return NULL;
	}

	stbi__context ctx;
	stbi__start_file(&ctx, file);

	pixman_image_t *image = NULL;
	if (stbi__tga_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__tga_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			struct image_data *image_data;
			image = image_create(width, height, &image_data);
			abgr_to_argb_premultiply_alpha(src, image_data->pixels, (size_t)width * (size_t)height);
		}
		free(src);
	}

	fclose(file);
	return image;
}
#endif // HAVE_TGA

#if HAVE_BMP
static pixman_image_t *load_bmp(string_t path) {
	assert(path.nul_terminated);

	FILE *file = fopen(path.s, "rb");
	if (!file) {
		return NULL;
	}

	stbi__context ctx;
	stbi__start_file(&ctx, file);

	pixman_image_t *image = NULL;
	if (stbi__bmp_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__bmp_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			struct image_data *image_data;
			image = image_create(width, height, &image_data);
			abgr_to_argb_premultiply_alpha(src, image_data->pixels, (size_t)width * (size_t)height);
		}
		free(src);
	}

	fclose(file);
	return image;
}
#endif // HAVE_BMP

#if HAVE_PSD
static pixman_image_t *load_psd(string_t path) {
	assert(path.nul_terminated);

	FILE *file = fopen(path.s, "rb");
	if (!file) {
		return NULL;
	}

	stbi__context ctx;
	stbi__start_file(&ctx, file);

	stbi__result_info ri = {
		.bits_per_channel = 8,
	};

	pixman_image_t *image = NULL;
	if (stbi__psd_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__psd_load(&ctx, &width, &height, &unused, 4, &ri, 8);
		if (src && (width > 0) && (height > 0)) {
			if (ri.bits_per_channel != 8) {
				assert(ri.bits_per_channel == 16);
				src = (uint32_t *)(void *)stbi__convert_16_to_8((stbi__uint16 *)src, width, height, 4);
			}
			struct image_data *image_data;
			image = image_create(width, height, &image_data);
			abgr_to_argb_premultiply_alpha(src, image_data->pixels, (size_t)width * (size_t)height);
		}
		free(src);
	}

	fclose(file);
	return image;
}
#endif // HAVE_PSD

#if HAVE_GIF
static pixman_image_t *load_gif(string_t path) {
	assert(path.nul_terminated);

	FILE *file = fopen(path.s, "rb");
	if (!file) {
		return NULL;
	}

	stbi__context ctx;
	stbi__start_file(&ctx, file);

	pixman_image_t *image = NULL;
	if (stbi__gif_test(&ctx)) {
		int width, height, unused, *frame_delays, frame_count;
		uint32_t *src = stbi__load_gif_main(&ctx, &frame_delays, &width, &height, &frame_count, &unused, 4);
		if (src && (width > 0) && (height > 0)) {
			if (frame_count > 1) {
				struct gif_frame_data *gif_data = malloc(sizeof(struct gif_frame_data));
				gif_data->frame_idx = 0;
				gif_data->frame_end = 0;
				gif_data->image = NULL;
				array_struct_gif_frame_init(&gif_data->frames, (size_t)frame_count);

				for (int i = 0; i < frame_count; ++i) {
					struct image_data *frame_data;
					array_struct_gif_frame_add(&gif_data->frames, (struct gif_frame){
						.image = image_create(width, height, &frame_data),
						.delay = (uint16_t)frame_delays[i],
					});

					size_t size = (size_t)width * (size_t)height * 4;
					memcpy( frame_data->pixels,
						&((uint8_t *)src)[size * (size_t)i],
						size);
					abgr_to_argb_premultiply_alpha(frame_data->pixels, frame_data->pixels, (size_t)width * (size_t)height);
					frame_data->type = IMAGE_DATA_TYPE_MULTIFRAME_GIF;
					frame_data->gif = gif_data;
				}

				image = array_struct_gif_frame_get(&gif_data->frames, 0).image;
			} else if (frame_count == 1) {
				struct image_data *image_data;
				image = image_create(width, height, &image_data);
				abgr_to_argb_premultiply_alpha(src, image_data->pixels, (size_t)width * (size_t)height);
			}
		}
		free(src);
		free(frame_delays);
	}

	fclose(file);
	return image;
}
#endif // HAVE_GIF

#if HAVE_HDR
static pixman_image_t *load_hdr(string_t path) {
	assert(path.nul_terminated);

	FILE *file = fopen(path.s, "rb");
	if (!file) {
		return NULL;
	}

	stbi__context ctx;
	stbi__start_file(&ctx, file);

	pixman_image_t *image = NULL;
	if (stbi__hdr_test(&ctx)) {
		int width = 0, height = 0, unused;
		uint32_t *src = (uint32_t *)(void *)stbi__hdr_to_ldr(
			stbi__hdr_load(&ctx, &width, &height, &unused, 4, NULL),
			width,  height,  4
		);
		if (src && (width > 0) && (height > 0)) {
			struct image_data *image_data;
			image = image_create(width, height, &image_data);
			abgr_to_argb_premultiply_alpha(src, image_data->pixels, (size_t)width * (size_t)height);
		}
		free(src);
	}

	fclose(file);
	return image;
}
#endif // HAVE_HDR

#if HAVE_PIC
static pixman_image_t *load_pic(string_t path) {
	assert(path.nul_terminated);

	FILE *file = fopen(path.s, "rb");
	if (!file) {
		return NULL;
	}

	stbi__context ctx;
	stbi__start_file(&ctx, file);

	pixman_image_t *image = NULL;
	if (stbi__pic_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__pic_load(&ctx, &width, &height, &unused, 4, NULL);
		if (src && (width > 0) && (height > 0)) {
			struct image_data *image_data;
			image = image_create(width, height, &image_data);
			abgr_to_argb_premultiply_alpha(src, image_data->pixels, (size_t)width * (size_t)height);
		}
		free(src);
	}

	fclose(file);
	return image;
}
#endif // HAVE_PIC

#if HAVE_PNM
static pixman_image_t *load_pnm(string_t path) {
	assert(path.nul_terminated);

	FILE *file = fopen(path.s, "rb");
	if (!file) {
		return NULL;
	}

	stbi__context ctx;
	stbi__start_file(&ctx, file);

	stbi__result_info ri = {
		.bits_per_channel = 8,
	};

	pixman_image_t *image = NULL;
	if (stbi__pnm_test(&ctx)) {
		int width, height, unused;
		uint32_t *src = stbi__pnm_load(&ctx, &width, &height, &unused, 4, &ri);
		if (src && (width > 0) && (height > 0)) {
			if (ri.bits_per_channel != 8) {
				assert(ri.bits_per_channel == 16);
				src = (uint32_t *)(void *)stbi__convert_16_to_8((stbi__uint16 *)src, width, height, 4);
			}
			struct image_data *image_data;
			image = image_create(width, height, &image_data);
			abgr_to_argb_premultiply_alpha(src, image_data->pixels, (size_t)width * (size_t)height);
		}
		free(src);
	}

	fclose(file);
	return image;
}
#endif // HAVE_PNM

static void process_border(struct surface_block_border *border, struct json_ast_node *json) {
	if (json->type != JSON_AST_NODE_TYPE_OBJECT) {
		return;
	}

	for (size_t i = 0; i < json->object.len; ++i) {
		struct json_ast_key_value *key_value = arena_array_struct_json_ast_key_value_get_ptr(&json->object, i);
		if (string_equal(key_value->key, STRING_LITERAL("width"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&border->width, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("color"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				pixman_color_t border_color = to_pixman_color((union sw_color){
					(uint32_t)key_value->value.u
				});
				border->color = pixman_image_create_solid_fill(&border_color);
			}
		}
	}
}

static bool32_t surface_block_init(struct surface_block *block, struct surface *surface, struct json_ast_node *json) {
	*block = (struct surface_block){ 0 };

	if (!json || (json->type != JSON_AST_NODE_TYPE_OBJECT)) {
		return false;
	}

	block->type = SW_SURFACE_BLOCK_TYPE_SPACER;
	block->anchor = SW_SURFACE_BLOCK_ANCHOR_LEFT;
	block->content_anchor = SW_SURFACE_BLOCK_CONTENT_ANCHOR_LEFT_CENTER;
	block->content_transform = SW_SURFACE_BLOCK_CONTENT_TRANSFORM_NORMAL;
	block->destroy_content_image = true;

	struct json_ast_node *composite_layout = NULL, *composite_children = NULL,
#if HAVE_TEXT
			*text_text = NULL, *text_font_names = NULL, *text_text_color = NULL,
#endif // HAVE_TEXT
			*image_path = NULL, *image_image_type = NULL;

	for (size_t i = 0; i < json->object.len; ++i) {
		struct json_ast_key_value *key_value = arena_array_struct_json_ast_key_value_get_ptr(&json->object, i);
		if (string_equal(key_value->key, STRING_LITERAL("type"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				switch ((enum sw_surface_block_type)key_value->value.u) {
				case SW_SURFACE_BLOCK_TYPE_TEXT:
#if !HAVE_TEXT
					goto error;
#endif // HAVE_TEXT
				case SW_SURFACE_BLOCK_TYPE_IMAGE:
				case SW_SURFACE_BLOCK_TYPE_COMPOSITE:
					block->type = (enum sw_surface_block_type)key_value->value.u;
					break;
				case SW_SURFACE_BLOCK_TYPE_SPACER:
				case SW_SURFACE_BLOCK_TYPE_DEFAULT:
				default:
					break;
				}
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("id"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				block->id = key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("anchor"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				switch ((enum sw_surface_block_anchor)key_value->value.u) {
				case SW_SURFACE_BLOCK_ANCHOR_TOP:
				case SW_SURFACE_BLOCK_ANCHOR_RIGHT:
				case SW_SURFACE_BLOCK_ANCHOR_BOTTOM:
				case SW_SURFACE_BLOCK_ANCHOR_CENTER:
				case SW_SURFACE_BLOCK_ANCHOR_NONE:
					block->anchor = (enum sw_surface_block_anchor)key_value->value.u;
					break;
				case SW_SURFACE_BLOCK_ANCHOR_LEFT:
				case SW_SURFACE_BLOCK_ANCHOR_DEFAULT:
				default:
					break;
				}
			} else if (key_value->value.type == JSON_AST_NODE_TYPE_OBJECT) {
				for (size_t j = 0; j < key_value->value.object.len; ++j) {
					key_value = arena_array_struct_json_ast_key_value_get_ptr(&key_value->value.object, j);
					if (string_equal(key_value->key, STRING_LITERAL("x"))) {
						if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
							string_init_string(&block->x, key_value->value.s);
						}
					} else if (string_equal(key_value->key, STRING_LITERAL("y"))) {
						if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
							string_init_string(&block->y, key_value->value.s);
						}
					}
				}
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("color"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				pixman_color_t color = to_pixman_color((union sw_color){
					(uint32_t)key_value->value.u
				});
				block->color = pixman_image_create_solid_fill(&color);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("min_width"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->min_width, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("max_width"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->max_width, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("min_height"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->min_height, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("max_height"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->max_height, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("content_anchor"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				switch ((enum sw_surface_block_content_anchor)key_value->value.u) {
				case SW_SURFACE_BLOCK_CONTENT_ANCHOR_LEFT_TOP:
				case SW_SURFACE_BLOCK_CONTENT_ANCHOR_LEFT_BOTTOM:
				case SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_TOP:
				case SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_CENTER:
				case SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_BOTTOM:
				case SW_SURFACE_BLOCK_CONTENT_ANCHOR_RIGHT_TOP:
				case SW_SURFACE_BLOCK_CONTENT_ANCHOR_RIGHT_CENTER:
				case SW_SURFACE_BLOCK_CONTENT_ANCHOR_RIGHT_BOTTOM:
					block->content_anchor = (enum sw_surface_block_content_anchor)key_value->value.u;
					break;
				case SW_SURFACE_BLOCK_CONTENT_ANCHOR_LEFT_CENTER:
				case SW_SURFACE_BLOCK_CONTENT_ANCHOR_DEFAULT:
				default:
					break;
				}
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("content_transform"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				switch ((enum sw_surface_block_content_transform)key_value->value.u) {
				case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_90:
				case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_180:
				case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_270:
				case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_FLIPPED:
				case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_FLIPPED_90:
				case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_FLIPPED_180:
				case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_FLIPPED_270:
					block->content_transform = (enum sw_surface_block_content_transform)key_value->value.u;
					break;
				case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_NORMAL:
				case SW_SURFACE_BLOCK_CONTENT_TRANSFORM_DEFAULT:
				default:
					break;
				}
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("expand"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_UINT)
					&& (key_value->value.u <= SW_SURFACE_BLOCK_EXPAND_ALL_SIDES_CONTENT)) {
				block->expand = (uint32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("content_width"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->content_width, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("content_height"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->content_height, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("border_left"))) {
			process_border(&block->border_left, &key_value->value);
		} else if (string_equal(key_value->key, STRING_LITERAL("border_right"))) {
			process_border(&block->border_right, &key_value->value);
		} else if (string_equal(key_value->key, STRING_LITERAL("border_bottom"))) {
			process_border(&block->border_bottom, &key_value->value);
		} else if (string_equal(key_value->key, STRING_LITERAL("border_top"))) {
			process_border(&block->border_top, &key_value->value);
#if HAVE_TEXT
		} else if (string_equal(key_value->key, STRING_LITERAL("text"))) {
			text_text = &key_value->value;
		} else if (string_equal(key_value->key, STRING_LITERAL("font_names"))) {
			text_font_names = &key_value->value;
		} else if (string_equal(key_value->key, STRING_LITERAL("text_color"))) {
			text_text_color = &key_value->value;
#endif // HAVE_TEXT
		} else if (string_equal(key_value->key, STRING_LITERAL("path"))) {
			image_path = &key_value->value;
		} else if (string_equal(key_value->key, STRING_LITERAL("image_type"))) {
			image_image_type = &key_value->value;
		}
		else if (string_equal(key_value->key, STRING_LITERAL("layout"))) {
			composite_layout = &key_value->value;
		} else if (string_equal(key_value->key, STRING_LITERAL("children"))) {
			composite_children = &key_value->value;
		}
	}

	switch (block->type) {
	case SW_SURFACE_BLOCK_TYPE_SPACER:
		break;
	case SW_SURFACE_BLOCK_TYPE_TEXT: {
#if HAVE_TEXT
		if (!text_text || (text_text->type != JSON_AST_NODE_TYPE_STRING) || (text_text->s.len == 0)) {
			goto error;
		}

		array_char_ptr_t font_names;
		array_char_ptr_init(&font_names, 8);
		if (text_font_names && (text_font_names->type == JSON_AST_NODE_TYPE_ARRAY)) {
			for (size_t i = 0; i < text_font_names->array.len; ++i) {
				struct json_ast_node *name = arena_array_struct_json_ast_node_get_ptr(&text_font_names->array, i);
				if ((name->type == JSON_AST_NODE_TYPE_STRING) && (name->s.len > 0)) {
					array_char_ptr_add(&font_names, name->s.s);
				}
			}
		}
		array_char_ptr_add(&font_names, (char *)"monospace:size=16");
		struct fcft_font *font = fcft_from_name(font_names.len,
				(const char **)font_names.items, NULL);
		array_char_ptr_fini(&font_names);
		if (font == NULL) {
			log_stderr("fcft_from_name failed");
			goto error;
		}

		struct fcft_text_run *text_run = NULL;
		struct text_run_cache *cache;
		if (hash_table_struct_text_run_cache_get(&state.text_run_cache,
				(struct text_run_cache){ .key = text_text->s },
				&cache)) {
			for (size_t i = 0; i < cache->items.len; ++i) {
				struct text_run_cache_entry entry =
					array_struct_text_run_cache_entry_get(&cache->items, i);
				if (entry.font == font) {
					text_run = entry.text_run;
					break;
				}
			}
		}

		if (text_run == NULL) {
			char32_t *text = malloc(text_text->s.len * sizeof(char32_t) + 1);
			size_t text_len = 0;
			mbstate_t s = { 0 };
			size_t consumed = 0;
			while (consumed < text_text->s.len) {
				char32_t c32;
				size_t ret = mbrtoc32(&c32, &text_text->s.s[consumed],
						text_text->s.len - consumed, &s);
				switch (ret) {
				case 0: // ? TODO: do not treat as error
				case (size_t)-1:
				case (size_t)-2:
				case (size_t)-3:
					free(text);
					log_stderr("mbrtoc32 failed");
					goto error;
				default:
					text[text_len++] = c32;
					consumed += ret;
				}
			}

			text_run = fcft_rasterize_text_run_utf32(font, text_len, text, FCFT_SUBPIXEL_NONE);
			free(text);
			if ((text_run == NULL) || (text_run->count == 0)) {
				fcft_text_run_destroy(text_run);
				log_stderr("fcft_rasterize_text_run_utf32 failed");
				goto error;
			}

			if (hash_table_struct_text_run_cache_add(&state.text_run_cache,
					(struct text_run_cache){ .key = text_text->s },
					&cache)) {
				string_init_string(&cache->key, cache->key);
				array_struct_text_run_cache_entry_init(&cache->items, 8);
			}

			array_struct_text_run_cache_entry_add(&cache->items, (struct text_run_cache_entry){
				.font = font,
				.text_run = text_run,
			});
		}

		int image_width = 0, image_height = font->height;
		for (size_t i = 0; i < text_run->count; ++i) {
			image_width += text_run->glyphs[i]->advance.x;
		}
		if ((image_width <= 0) || (image_height <= 0)) {
			goto error;
		}

		block->content_image = image_create(image_width, image_height, NULL);

		pixman_color_t color = to_pixman_color((union sw_color){
			(text_text_color && (text_text_color->type == JSON_AST_NODE_TYPE_UINT))
				? (uint32_t)text_text_color->u
				: 0xFFFFFFFF
		});
		pixman_image_t *text_color = pixman_image_create_solid_fill(&color);

		int x = 0, y = font->height - font->descent;
		for (size_t i = 0; i < text_run->count; ++i) {
			const struct fcft_glyph *glyph = text_run->glyphs[i];
			if (pixman_image_get_format(glyph->pix) == PIXMAN_a8r8g8b8) {
				pixman_image_composite32(PIXMAN_OP_OVER, glyph->pix, NULL, block->content_image,
						0, 0, 0, 0, x + glyph->x, y - glyph->y,
						glyph->width, glyph->height);
			} else {
				pixman_image_composite32(PIXMAN_OP_OVER, text_color, glyph->pix, block->content_image,
						0, 0, 0, 0, x + glyph->x, y - glyph->y,
						glyph->width, glyph->height);
			}
			x += glyph->advance.x;
		}

		pixman_image_unref(text_color);
#endif // HAVE_TEXT
		break;
	}
	case SW_SURFACE_BLOCK_TYPE_IMAGE: {
		if (!image_path || (image_path->type != JSON_AST_NODE_TYPE_STRING) || (image_path->s.len == 0)) {
			goto error;
		}

		string_t path = image_path->s;
		assert(path.nul_terminated);

		char abspath_buf[PATH_MAX];
		if (realpath(path.s, abspath_buf) == NULL) {
			log_stderr(STRING_FMT": realpath: %s", STRING_ARGS(path), strerror(errno));
			goto error;
		}

		string_t abspath = {
			.s = abspath_buf,
			.len = strlen(abspath_buf),
			.free_contents = false,
			.nul_terminated = true,
		};

		struct stat sb;
		if (stat(abspath_buf, &sb) == -1) {
			log_stderr(STRING_FMT": stat: %s", STRING_ARGS(abspath), strerror(errno));
			struct image_cache del;
			if (hash_table_struct_image_cache_del(&state.image_cache,
					(struct image_cache){ .key = abspath }, &del)) {
				string_fini(&del.key);
				pixman_image_unref(del.image);
			}
			goto error;
		}

		enum sw_surface_block_type_image_image_type image_type = SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PIXMAP;
		if (image_image_type && (image_image_type->type == JSON_AST_NODE_TYPE_UINT)) {
			switch ((enum sw_surface_block_type_image_image_type)image_image_type->u) {
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PNG:
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_JPG:
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_SVG:
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_TGA:
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_BMP:
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PSD:
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_GIF:
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_HDR:
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PIC:
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PNM:
				image_type = (enum sw_surface_block_type_image_image_type)image_image_type->u;
				break;
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PIXMAP:
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_DEFAULT:
			default:
				break;
			}
		}

		struct image_cache *cache;
		if (hash_table_struct_image_cache_get(&state.image_cache,
				(struct image_cache){ .key = abspath }, &cache)) {
			if ((memcmp(&sb.st_mtim, &cache->mtim_ts, sizeof(struct timespec)) == 0)
					&& (cache->type == image_type)) {
#if HAVE_GIF
				struct image_data *data = pixman_image_get_destroy_data(cache->image);
				if (data && (data->type == IMAGE_DATA_TYPE_MULTIFRAME_GIF)) {
					block->content_image = cache->image;
					block->destroy_content_image = false;
				} else
#endif // HAVE_GIF
					block->content_image = pixman_image_ref(cache->image);
			} else {
				pixman_image_unref(cache->image);
			}
		}

		if (block->content_image == NULL) {
			switch (image_type) {
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PIXMAP:
				block->content_image = load_pixmap(abspath);
				break;
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_SVG:
#if HAVE_SVG
				block->content_image = load_svg(abspath);
#endif // HAVE_SVG
				break;
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PNG:
#if HAVE_PNG
				block->content_image = load_png(abspath);
#endif // HAVE_PNG
				break;
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_JPG:
#if HAVE_JPG
				block->content_image = load_jpg(abspath);
#endif // HAVE_JPG
				break;
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_TGA:
#if HAVE_TGA
				block->content_image = load_tga(abspath);
#endif // HAVE_TGA
				break;
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_BMP:
#if HAVE_BMP
				block->content_image = load_bmp(abspath);
#endif // HAVE_BMP
				break;
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PSD:
#if HAVE_PSD
				block->content_image = load_psd(abspath);
#endif // HAVE_PSD
				break;
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_GIF:
#if HAVE_GIF
				block->content_image = load_gif(abspath);
#endif // HAVE_GIF
				break;
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_HDR:
#if HAVE_HDR
				block->content_image = load_hdr(abspath);
#endif // HAVE_HDR
				break;
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PIC:
#if HAVE_PIC
				block->content_image = load_pic(abspath);
#endif // HAVE_PIC
				break;
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PNM:
#if HAVE_PNM
				block->content_image = load_pnm(abspath);
#endif // HAVE_PNM
				break;
			default:
			case SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_DEFAULT:
				ASSERT_UNREACHABLE;
			}
			if (block->content_image == NULL) {
				struct image_cache del;
				if (hash_table_struct_image_cache_del(&state.image_cache,
						(struct image_cache){ .key = abspath }, &del)) {
					string_fini(&del.key);
				}
				goto error;
			}

			pixman_image_set_filter(block->content_image, PIXMAN_FILTER_BEST, NULL, 0);

			if (hash_table_struct_image_cache_add(&state.image_cache,
					(struct image_cache){ .key = abspath }, &cache)) {
				string_init_string(&cache->key, cache->key);
			}
#if HAVE_GIF
			struct image_data *data = pixman_image_get_destroy_data(block->content_image);
			if (data && (data->type == IMAGE_DATA_TYPE_MULTIFRAME_GIF)) {
				block->destroy_content_image = false;
				cache->image = block->content_image;
			} else
#endif // HAVE_GIF
				cache->image = pixman_image_ref(block->content_image);

			cache->mtim_ts = sb.st_mtim;
			cache->type = image_type;
		}
		break;
	}
	case SW_SURFACE_BLOCK_TYPE_COMPOSITE: {
		if (!composite_children || (composite_children->type != JSON_AST_NODE_TYPE_ARRAY)
				|| (composite_children->array.len == 0)) {
			goto error;
		}

		block->composite_layout = SW_SURFACE_BLOCK_TYPE_COMPOSITE_BLOCK_LAYOUT_HORIZONTAL;
		if (composite_layout && (composite_layout->type == JSON_AST_NODE_TYPE_UINT)) {
			switch ((enum sw_surface_block_type_composite_block_layout)composite_layout->u) {
			case SW_SURFACE_BLOCK_TYPE_COMPOSITE_BLOCK_LAYOUT_VERTICAL:
				block->composite_layout = (enum sw_surface_block_type_composite_block_layout)composite_layout->u;
				break;
			case SW_SURFACE_BLOCK_TYPE_COMPOSITE_BLOCK_LAYOUT_HORIZONTAL:
			case SW_SURFACE_BLOCK_TYPE_COMPOSITE_BLOCK_LAYOUT_DEFAULT:
			default:
				break;
			}
		}

		array_struct_surface_block_init(&block->composite_children, composite_children->array.len);

		for (size_t i = 0; i < composite_children->array.len; ++i) {
			struct surface_block *b = array_struct_surface_block_add_uninitialized(&block->composite_children);
			surface_block_init(b, surface, arena_array_struct_json_ast_node_get_ptr(&composite_children->array, i));
		}
		break;
	}
	case SW_SURFACE_BLOCK_TYPE_DEFAULT:
	default:
		ASSERT_UNREACHABLE;
	}

	block->surface = surface;
	return true;
error:
	surface_block_fini(block);
	*block = (struct surface_block){ 0 };
	return false;
}

static void describe_surface_block(struct json_writer *writer, struct surface_block *block) {
	if (!block->surface) {
		json_writer_null(writer);
		return;
	}

	json_writer_object_begin(writer);

	json_writer_object_key(writer, STRING_LITERAL("x"));
	json_writer_int(writer, block->box.x);

	json_writer_object_key(writer, STRING_LITERAL("y"));
	json_writer_int(writer, block->box.y);

	json_writer_object_key(writer, STRING_LITERAL("width"));
	json_writer_int(writer, block->box.width);

	json_writer_object_key(writer, STRING_LITERAL("height"));
	json_writer_int(writer, block->box.height);

	if (block->type == SW_SURFACE_BLOCK_TYPE_COMPOSITE) {
		json_writer_object_key(writer, STRING_LITERAL("children"));
		json_writer_array_begin(writer);

		for (size_t i = 0; i < block->composite_children.len; ++i) {
			describe_surface_block( writer,
				array_struct_surface_block_get_ptr(&block->composite_children, i));
		}

		json_writer_array_end(writer);
	}

	json_writer_object_end(writer);
}

static void describe_surfaces(struct json_writer *writer, array_struct_surface_ptr_t *source) {
	json_writer_array_begin(writer);

	for (size_t i = 0; i < source->len; ++i) {
		struct surface *surface = array_struct_surface_ptr_get(source, i);
		if (surface == NULL) {
			json_writer_null(writer);
			continue;
		}

		json_writer_object_begin(writer);

		json_writer_object_key(writer, STRING_LITERAL("width"));
		json_writer_int(writer, surface->width);

		json_writer_object_key(writer, STRING_LITERAL("height"));
		json_writer_int(writer, surface->height);

		json_writer_object_key(writer, STRING_LITERAL("scale"));
		json_writer_int(writer, surface->scale);

		json_writer_object_key(writer, STRING_LITERAL("layout_root"));
		describe_surface_block(writer, &surface->layout_root);

		json_writer_object_key(writer, STRING_LITERAL("popups"));
		describe_surfaces(writer, &surface->popups);

		json_writer_object_end(writer);
	}

	json_writer_array_end(writer);
}

static void describe_outputs(struct json_writer *writer) {
	json_writer_array_begin(writer);

	for (size_t i = 0; i < state.outputs.len; ++i) {
		struct output *output = array_struct_output_ptr_get(&state.outputs, i);
		if (output->name.len == 0) {
			continue;
		}
		json_writer_object_begin(writer);

		json_writer_object_key(writer, STRING_LITERAL("name"));
		json_writer_string_escape(writer, output->name);

		json_writer_object_key(writer, STRING_LITERAL("width"));
		json_writer_int(writer, output->width);

		json_writer_object_key(writer, STRING_LITERAL("height"));
		json_writer_int(writer, output->height);

		json_writer_object_key(writer, STRING_LITERAL("scale"));
		json_writer_int(writer, output->scale);

		json_writer_object_key(writer, STRING_LITERAL("transform"));
		json_writer_uint(writer, output->transform);

		json_writer_object_key(writer, STRING_LITERAL("layers"));
		describe_surfaces(writer, &output->layers);

		json_writer_object_end(writer);
	}

	json_writer_array_end(writer);
}

static void describe_pointer(struct json_writer *writer, struct pointer *pointer) {
	json_writer_object_begin(writer);

	json_writer_object_key(writer, STRING_LITERAL("position"));
	if (pointer->pos_dirty) {
		json_writer_object_begin(writer);

		json_writer_object_key(writer, STRING_LITERAL("x"));
		json_writer_int(writer, pointer->pos_x);

		json_writer_object_key(writer, STRING_LITERAL("y"));
		json_writer_int(writer, pointer->pos_y);

		json_writer_object_end(writer);
		pointer->pos_dirty = false;
	} else {
		json_writer_null(writer);
	}

	json_writer_object_key(writer, STRING_LITERAL("button"));
	if (pointer->btn_dirty) {
		json_writer_object_begin(writer);

		json_writer_object_key(writer, STRING_LITERAL("code"));
		json_writer_uint(writer, pointer->btn_code);

		json_writer_object_key(writer, STRING_LITERAL("state"));
		json_writer_uint(writer, pointer->btn_state);

		json_writer_object_key(writer, STRING_LITERAL("serial"));
		json_writer_uint(writer, pointer->btn_serial);

		json_writer_object_end(writer);
		pointer->btn_dirty = false;
	} else {
		json_writer_null(writer);
	}

	json_writer_object_key(writer, STRING_LITERAL("scroll"));
	if (pointer->scroll_dirty) {
		json_writer_object_begin(writer);

		json_writer_object_key(writer, STRING_LITERAL("axis"));
		json_writer_uint(writer, pointer->scroll_axis);

		json_writer_object_key(writer, STRING_LITERAL("vector_length"));
		json_writer_double(writer, pointer->scroll_vector_length);

		json_writer_object_end(writer);
		pointer->scroll_dirty = false;
	} else {
		json_writer_null(writer);
	}

	json_writer_object_key(writer, STRING_LITERAL("surface"));
	if (pointer->surface) {
		json_writer_object_begin(writer);

		json_writer_object_key(writer, STRING_LITERAL("userdata"));
		json_writer_raw(writer, pointer->surface->userdata.s,
			pointer->surface->userdata.len);

		json_writer_object_end(writer);
	} else {
		json_writer_null(writer);
	}

	json_writer_object_end(writer);
}

static void describe_seats(struct json_writer *writer) {
	json_writer_array_begin(writer);

	for (size_t i = 0; i < state.seats.len; ++i) {
		struct seat *seat = array_struct_seat_ptr_get(&state.seats, i);
		if (seat->name.len == 0) {
			continue;
		}

		json_writer_object_begin(writer);

		json_writer_object_key(writer, STRING_LITERAL("name"));
		json_writer_string_escape(writer, seat->name);

		json_writer_object_key(writer, STRING_LITERAL("pointer"));
		if (seat->pointer) {
			describe_pointer(writer, seat->pointer);
		} else {
			json_writer_null(writer);
		}

		json_writer_object_end(writer);
	}

	json_writer_array_end(writer);
}

static void state_update(bool32_t force) {
	if (!state.events || (!state.dirty && !force)) {
		return;
	}

	json_writer_object_begin(&state.writer);

	if (state.userdata.len > 0) {
		json_writer_object_key(&state.writer, STRING_LITERAL("userdata"));
		json_writer_raw(&state.writer, state.userdata.s, state.userdata.len);
	}

	json_writer_object_key(&state.writer, STRING_LITERAL("outputs"));
	describe_outputs(&state.writer);

	json_writer_object_key(&state.writer, STRING_LITERAL("seats"));
	describe_seats(&state.writer);

	json_writer_object_end(&state.writer);

	json_writer_raw(&state.writer, "\n", 1); // ? TODO: remove

	state.dirty = false;
}

static void layer_destroy(struct surface *layer);
static void popup_destroy(struct surface *popup);

static void surface_buffer_handle_release(void *data, struct wl_buffer *wl_buffer) {
	(void)wl_buffer;
	struct surface_buffer *buffer = data;
	buffer->busy = false;
	if (buffer->surface->dirty) {
		if (!surface_render(buffer->surface)) {
			switch (buffer->surface->type) {
			case SW_SURFACE_TYPE_LAYER:
				layer_destroy(buffer->surface);
				break;
			case SW_SURFACE_TYPE_POPUP:
				popup_destroy(buffer->surface);
				break;
			default:
				ASSERT_UNREACHABLE;
			}
		}
		state.dirty = true;
	}
}

static const struct wl_buffer_listener surface_buffer_listener = {
	.release = surface_buffer_handle_release,
};

static struct surface_buffer *surface_buffer_create(int32_t width, int32_t height,
		struct surface *surface) {
	struct surface_buffer *buffer = malloc(sizeof(struct surface_buffer));

	struct timespec ts = { 0 };
	pid_t pid = getpid();
	char shm_name[NAME_MAX];

generate_shm_name:
	clock_gettime(CLOCK_MONOTONIC, &ts);
	stbsp_snprintf(shm_name, sizeof(shm_name),"/" PREFIX "-%d-%ld-%ld",
			pid, ts.tv_sec, ts.tv_nsec);

	int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0600);
	if (shm_fd == -1) {
		if (errno == EEXIST) {
			goto generate_shm_name;
		} else {
			abort_(errno, "shm_open: %s", strerror(errno));
		}
	} else {
		shm_unlink(shm_name);
	}

	int32_t stride = width * 4;
	buffer->size = (uint32_t)stride * (uint32_t)height;
	while (ftruncate(shm_fd, buffer->size) == -1) {
		if (errno == EINTR) {
			continue;
		} else {
			abort_(errno, "ftruncate: %s", strerror(errno));
		}
	}

	buffer->pixels = mmap(NULL, buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (buffer->pixels == MAP_FAILED) {
		abort_(errno, "mmap: %s", strerror(errno));
	}

	buffer->image = pixman_image_create_bits_no_clear(PIXMAN_a8r8g8b8, width, height,
			buffer->pixels, stride);

	struct wl_shm_pool *wl_shm_pool =
		wl_shm_create_pool(state.shm, shm_fd, (int32_t)buffer->size);
	buffer->wl_buffer = wl_shm_pool_create_buffer(wl_shm_pool, 0, width,
			height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_buffer_add_listener(buffer->wl_buffer, &surface_buffer_listener, buffer);
	wl_shm_pool_destroy(wl_shm_pool);
	close(shm_fd);

	buffer->surface = surface;
	buffer->busy = false;

	return buffer;
}

static void surface_buffer_destroy(struct surface_buffer *buffer) {
	if (!buffer) {
		return;
	}

	if (buffer->image) {
		pixman_image_unref(buffer->image);
	}
	if (buffer->wl_buffer) {
		wl_buffer_destroy(buffer->wl_buffer);
	}
	if (buffer->pixels) {
		munmap(buffer->pixels, buffer->size);
	}

	free(buffer);
}

static void surface_init(struct surface *surface, struct output *output) {
	surface->output = output;
	surface->scale = output->scale;
	surface->wl_surface = wl_compositor_create_surface(state.compositor);
	wl_surface_set_user_data(surface->wl_surface, surface);

	array_struct_surface_ptr_init(&surface->popups, 4);
	array_struct_box_init(&surface->input_regions, 4);
}

static void surface_fini(struct surface *surface) {
	for (size_t i = 0; i < surface->popups.len; ++i) {
		popup_destroy(array_struct_surface_ptr_get(&surface->popups, i));
	}
	array_struct_surface_ptr_fini(&surface->popups);

	surface_buffer_destroy(surface->buffer);

	surface_block_fini(&surface->layout_root);

	array_struct_box_fini(&surface->input_regions);

	string_fini(&surface->userdata);

	for (size_t i = 0; i < state.seats.len; ++i) {
		struct seat *seat = array_struct_seat_ptr_get(&state.seats, i);
		if (seat->pointer && (seat->pointer->surface == surface)) {
			seat->pointer->surface = NULL;
		}
	}
}

static void popup_destroy(struct surface *popup) {
	if (popup == NULL) {
		return;
	}

	surface_fini(popup);

	if (popup->popup.xdg_positioner) {
		xdg_positioner_destroy(popup->popup.xdg_positioner);
	}
	if (popup->popup.xdg_popup) {
		xdg_popup_destroy(popup->popup.xdg_popup);
	}
	if (popup->popup.xdg_surface) {
		xdg_surface_destroy(popup->popup.xdg_surface);
	}
	if (popup->wl_surface) {
		wl_surface_destroy(popup->wl_surface);
	}

	free(popup);
}

static void popup_handle_configure(void *data, struct xdg_popup *xdg_popup,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	(void)xdg_popup;
	(void)x;
	(void)y;
	struct surface *popup = data;
	assert(width > 0);
	assert(height > 0);

	width *= popup->scale;
	height *= popup->scale;
	if ((popup->width != width) || (popup->height != height)) {
		surface_buffer_destroy(popup->buffer);
		popup->buffer = surface_buffer_create(width, height, popup);
		popup->width = width;
		popup->height = height;
		popup->dirty = true;
	}
}

static void popup_handle_done(void *data, struct xdg_popup *xdg_popup) {
	(void)xdg_popup;
	struct surface *popup = data;
	for (size_t i = 0; i < popup->popup.parent->popups.len; ++i) {
		if (array_struct_surface_ptr_get(&popup->popup.parent->popups, i) == popup) {
			array_struct_surface_ptr_put(&popup->popup.parent->popups, NULL, i);
			popup_destroy(popup);
			state.dirty = true;
			return;
		}
	}
}

static void popup_handle_repositioned(void *data, struct xdg_popup *xdg_popup,
		uint32_t token) {
	(void)data;
	(void)xdg_popup;
	(void)token;
}

static const struct xdg_popup_listener xdg_popup_listener = {
	.configure = popup_handle_configure,
	.popup_done = popup_handle_done,
	.repositioned = popup_handle_repositioned,
};

static void popup_xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface,
		uint32_t serial) {
	(void)xdg_surface;
	struct surface *popup = data;

	xdg_surface_ack_configure(popup->popup.xdg_surface, serial);

	if (popup->dirty) {
		if (!surface_render(popup)) {
			popup_destroy(popup);
		}
		state.dirty = true;
	}
}

static const struct xdg_surface_listener popup_xdg_surface_listener = {
	.configure = popup_xdg_surface_handle_configure,
};

static void process_cursor_shape(struct surface *surface, struct json_ast_node *json) {
	enum wp_cursor_shape_device_v1_shape cursor_shape;
	if (json && (json->type == JSON_AST_NODE_TYPE_UINT)) {
		switch ((enum wp_cursor_shape_device_v1_shape)json->u) {
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CONTEXT_MENU:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_HELP:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_POINTER:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_PROGRESS:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_WAIT:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CELL:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CROSSHAIR:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_TEXT:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_VERTICAL_TEXT:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALIAS:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_COPY:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_MOVE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NO_DROP:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NOT_ALLOWED:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_GRAB:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_GRABBING:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_E_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_N_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NE_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NW_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_S_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SE_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SW_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_W_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_EW_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NS_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NESW_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NWSE_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_COL_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ROW_RESIZE:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALL_SCROLL:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ZOOM_IN:
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ZOOM_OUT:
#if defined(WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DND_ASK_SINCE_VERSION)
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DND_ASK:
#endif // WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DND_ASK_SINCE_VERSION
#if defined(WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALL_RESIZE_SINCE_VERSION)
		case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALL_RESIZE:
#endif // WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALL_RESIZE_SINCE_VERSION
			cursor_shape = (enum wp_cursor_shape_device_v1_shape)json->u;
			break;
		default:
			cursor_shape = WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
			break;
		}
	} else {
		cursor_shape = WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
	}
	if (surface->cursor_shape != cursor_shape) {
		for (size_t i = 0; i < state.seats.len; ++i) {
			struct seat *seat = array_struct_seat_ptr_get(&state.seats, i);
			if (seat->pointer && seat->pointer->cursor_shape_device
					&& (seat->pointer->surface == surface)) {
				wp_cursor_shape_device_v1_set_shape(seat->pointer->cursor_shape_device,
					seat->pointer->wl_pointer_enter_serial, cursor_shape);
			}
		}
		surface->cursor_shape = cursor_shape;
	}
}

static void process_input_regions(struct surface *surface, struct json_ast_node *json) {
	array_struct_box_t new_input_regions = { 0 };
	if (json && (json->type == JSON_AST_NODE_TYPE_ARRAY) && (json->array.len > 0)) {
		array_struct_box_init(&new_input_regions, json->array.len);
		for (size_t i = 0; i < json->array.len; ++i) {
			struct json_ast_node *box_json = arena_array_struct_json_ast_node_get_ptr(&json->array, i);
			struct box box = { 0 };
			if (box_json->type == JSON_AST_NODE_TYPE_OBJECT) {
				for (size_t j = 0; j < box_json->object.len; ++j) {
					struct json_ast_key_value *key_value = arena_array_struct_json_ast_key_value_get_ptr(&box_json->object, j);
					if (string_equal(key_value->key, STRING_LITERAL("x"))) {
						if ((key_value->value.type == JSON_AST_NODE_TYPE_INT)
								|| (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
							box.x = (int32_t)key_value->value.i;
						}
					} else if (string_equal(key_value->key, STRING_LITERAL("y"))) {
						if ((key_value->value.type == JSON_AST_NODE_TYPE_INT)
								|| (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
							box.y = (int32_t)key_value->value.i;
						}
					} else if (string_equal(key_value->key, STRING_LITERAL("width"))) {
						if ((key_value->value.type == JSON_AST_NODE_TYPE_INT)
								|| (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
							box.width = (int32_t)key_value->value.i;
						}
					} else if (string_equal(key_value->key, STRING_LITERAL("height"))) {
						if ((key_value->value.type == JSON_AST_NODE_TYPE_INT)
								|| (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
							box.height = (int32_t)key_value->value.i;
						}
					}
				}
			}
			array_struct_box_add(&new_input_regions, box);
		}
	}

	// TODO: rework
	if ((surface->input_regions.len == new_input_regions.len) &&
			(!new_input_regions.items ||
				(memcmp(surface->input_regions.items, new_input_regions.items, new_input_regions.len) == 0))) {
		array_struct_box_fini(&new_input_regions);
	} else {
		array_struct_box_fini(&surface->input_regions);
		if (new_input_regions.len > 0) {
			struct wl_region *input_region = wl_compositor_create_region(state.compositor);
			for (size_t i = 0; i < new_input_regions.len; ++i) {
				struct box box = array_struct_box_get(&new_input_regions, i);
				wl_region_add(input_region, box.x, box.y, box.width, box.height);
			}
			wl_surface_set_input_region(surface->wl_surface, input_region);
			wl_region_destroy(input_region);
		} else {
			wl_surface_set_input_region(surface->wl_surface, NULL);
		}
		surface->input_regions = new_input_regions;
	}
}

static void process_popups(struct surface *surface, struct json_ast_node *json);

static bool32_t popup_update(struct surface *popup, struct json_ast_node *json,
		struct json_ast_node **grab_out, struct json_ast_node **popups_out) {
	if (json->type != JSON_AST_NODE_TYPE_OBJECT) {
		return false;
	}

	int64_t new_x = INT64_MIN;
	int64_t new_y = INT64_MIN;
	enum xdg_positioner_gravity new_gravity = XDG_POSITIONER_GRAVITY_NONE;
	enum xdg_positioner_constraint_adjustment new_constraint_adjustment = XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE;
	string_fini(&popup->userdata);
	popup->userdata = (string_t){ 0 };
	popup->desired_width = -1;
	popup->desired_height = -1;

	struct json_ast_node *new_grab = NULL, *new_cursor_shape = NULL,
						*new_input_regions = NULL, *new_popups = NULL,
						*new_layout_root = NULL;

	for (size_t i = 0; i < json->object.len; ++i) {
		struct json_ast_key_value *key_value = arena_array_struct_json_ast_key_value_get_ptr(&json->object, i);
		if (string_equal(key_value->key, STRING_LITERAL("x"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_UINT) || (key_value->value.type == JSON_AST_NODE_TYPE_INT)) {
				new_x = (int32_t)key_value->value.i;
			} else {
				return false;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("y"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_UINT) || (key_value->value.type == JSON_AST_NODE_TYPE_INT)) {
				new_y = (int32_t)key_value->value.i;
			} else {
				return false;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("gravity"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				switch ((enum sw_surface_popup_gravity)key_value->value.u) {
				case SW_SURFACE_POPUP_GRAVITY_TOP:
					new_gravity = XDG_POSITIONER_GRAVITY_TOP;
					break;
				case SW_SURFACE_POPUP_GRAVITY_BOTTOM:
					new_gravity = XDG_POSITIONER_GRAVITY_BOTTOM;
					break;
				case SW_SURFACE_POPUP_GRAVITY_LEFT:
					new_gravity = XDG_POSITIONER_GRAVITY_LEFT;
					break;
				case SW_SURFACE_POPUP_GRAVITY_RIGHT:
					new_gravity = XDG_POSITIONER_GRAVITY_RIGHT;
					break;
				case SW_SURFACE_POPUP_GRAVITY_TOP_LEFT:
					new_gravity = XDG_POSITIONER_GRAVITY_TOP_LEFT;
					break;
				case SW_SURFACE_POPUP_GRAVITY_BOTTOM_LEFT:
					new_gravity = XDG_POSITIONER_GRAVITY_BOTTOM_LEFT;
					break;
				case SW_SURFACE_POPUP_GRAVITY_TOP_RIGHT:
					new_gravity = XDG_POSITIONER_GRAVITY_TOP_RIGHT;
					break;
				case SW_SURFACE_POPUP_GRAVITY_BOTTOM_RIGHT:
					new_gravity = XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT;
					break;
				case SW_SURFACE_POPUP_GRAVITY_NONE:
				case SW_SURFACE_POPUP_GRAVITY_DEFAULT:
				default:
					new_gravity = XDG_POSITIONER_GRAVITY_NONE;
					break;
				}
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("constraint_adjustment"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_UINT)
					&& (key_value->value.u <=
						(XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X
						| XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X
						| XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X
						| XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y))) {
				new_constraint_adjustment = (enum xdg_positioner_constraint_adjustment)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("grab"))) {
			new_grab = &key_value->value;
		} else if (string_equal(key_value->key, STRING_LITERAL("userdata"))) {
			json_writer_ast_node(&state.userdata_writer, &key_value->value);
			string_init_len(&popup->userdata, state.userdata_writer.buf.data, state.userdata_writer.buf.idx, false);
			json_writer_reset(&state.userdata_writer);
		} else if (string_equal(key_value->key, STRING_LITERAL("width"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_UINT) || (key_value->value.type == JSON_AST_NODE_TYPE_INT)) {
				popup->desired_width = (int32_t)key_value->value.i;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("height"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_UINT) || (key_value->value.type == JSON_AST_NODE_TYPE_INT)) {
				popup->desired_height = (int32_t)key_value->value.i;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("cursor_shape"))) {
			new_cursor_shape = &key_value->value;
		} else if (string_equal(key_value->key, STRING_LITERAL("input_regions"))) {
			new_input_regions = &key_value->value;
		} else if (string_equal(key_value->key, STRING_LITERAL("popups"))) {
			new_popups = &key_value->value;
		} else if (string_equal(key_value->key, STRING_LITERAL("layout_root"))) {
			new_layout_root = &key_value->value;
		}
	}

	if ((new_x == INT64_MIN) || (new_y == INT64_MIN)) {
		return false;
	}

	surface_block_fini(&popup->layout_root);
	if (!surface_block_init(&popup->layout_root, popup, new_layout_root)) {
		return false;
	}

	bool32_t reposition = false;
	if ((new_x != popup->popup.desired_x) || (new_y != popup->popup.desired_y)) {
		xdg_positioner_set_anchor_rect(popup->popup.xdg_positioner,
			(int32_t)new_x / popup->scale, (int32_t)new_y / popup->scale, 1, 1);
		popup->popup.desired_x = (int32_t)new_x;
		popup->popup.desired_y = (int32_t)new_y;
		reposition = true;
	}

	if (popup->popup.gravity != new_gravity) {
		xdg_positioner_set_gravity(popup->popup.xdg_positioner, new_gravity);
		popup->popup.gravity = new_gravity;
		reposition = true;
	}

	if (popup->popup.constraint_adjustment != new_constraint_adjustment) {
		xdg_positioner_set_constraint_adjustment(
			popup->popup.xdg_positioner, new_constraint_adjustment);
		popup->popup.constraint_adjustment = new_constraint_adjustment;
		reposition = true;
	}

	process_cursor_shape(popup, new_cursor_shape);
	process_input_regions(popup, new_input_regions);

	if (popup->popup.xdg_popup) {
		if (reposition) {
			xdg_popup_reposition(popup->popup.xdg_popup, popup->popup.xdg_positioner, 0);
		}
		process_popups(popup, new_popups);
	}

	if (grab_out) {
		*grab_out = new_grab;
	}
	if (popups_out) {
		*popups_out = new_popups;
	}

	return surface_render(popup);
}

static void surface_handle_enter(void *data, struct wl_surface *wl_surface, struct wl_output *output) {
	(void)data;
	(void)wl_surface;
	(void)output;
}

static void surface_handle_leave(void *data, struct wl_surface *wl_surface, struct wl_output *output) {
	(void)data;
	(void)wl_surface;
	(void)output;
}

static void surface_handle_preferred_buffer_transform(void *data,
		struct wl_surface *wl_surface, uint32_t transform) {
	(void)data;
	(void)wl_surface;
	(void)transform;
}

static void popup_handle_preferred_buffer_scale(void *data, struct wl_surface *wl_surface, int32_t factor) {
	(void)wl_surface;
	assert(factor > 0);
	struct surface *popup = data;
	if (popup->scale != factor) {
		popup->scale = factor;
		popup->dirty = true;
		xdg_positioner_set_size(popup->popup.xdg_positioner,
			popup->width / factor, popup->height / factor);
		xdg_positioner_set_anchor_rect(popup->popup.xdg_positioner,
			popup->popup.desired_x / factor,
			popup->popup.desired_y / factor,
			1, 1);
		xdg_popup_reposition(popup->popup.xdg_popup, popup->popup.xdg_positioner, 0);
		wl_surface_commit(popup->wl_surface);
	}
}

static const struct wl_surface_listener popup_listener = {
	.enter = surface_handle_enter,
	.leave = surface_handle_leave,
	.preferred_buffer_transform = surface_handle_preferred_buffer_transform,
	.preferred_buffer_scale = popup_handle_preferred_buffer_scale,
};

static struct surface *popup_create(struct surface *parent, struct json_ast_node *json) {
	struct surface *popup = calloc(1, sizeof(struct surface));
	popup->type = SW_SURFACE_TYPE_POPUP;
	surface_init(popup, parent->output);
	popup->popup.xdg_surface =
		xdg_wm_base_get_xdg_surface(state.wm_base, popup->wl_surface);
	popup->popup.xdg_positioner = xdg_wm_base_create_positioner(state.wm_base);
	popup->popup.parent = parent;

	struct json_ast_node *grab, *popups;
	if (!popup_update(popup, json, &grab, &popups)) {
		goto error;
	}

	switch (parent->type) {
	case SW_SURFACE_TYPE_LAYER:
		popup->popup.xdg_popup = xdg_surface_get_popup(
			popup->popup.xdg_surface, NULL, popup->popup.xdg_positioner);
		zwlr_layer_surface_v1_get_popup(parent->layer.layer_surface, popup->popup.xdg_popup);
		if (grab && (grab->type == JSON_AST_NODE_TYPE_UINT)) {
			for (size_t i = 0; i < state.seats.len; ++i) {
				struct seat *seat = array_struct_seat_ptr_get(&state.seats, i);
				for (uint8_t j = (uint8_t)(seat->popup_grab.index - 1);
						(j < seat->popup_grab.serials.len) && (j != seat->popup_grab.index);
						--j) {
					if (array_uint32_t_get(&seat->popup_grab.serials, j) == grab->u) {
						popup->popup.grab.seat = seat->wl_seat;
						popup->popup.grab.serial = (uint32_t)grab->u;
						goto grab;
					}
				}
			}
			goto error;
		}
		break;
	case SW_SURFACE_TYPE_POPUP:
		popup->popup.xdg_popup = xdg_surface_get_popup(
			popup->popup.xdg_surface, parent->popup.xdg_surface, popup->popup.xdg_positioner);
		popup->popup.grab = parent->popup.grab;
		break;
	default:
		ASSERT_UNREACHABLE;
	}

	if (popup->popup.grab.seat) {
grab:
		xdg_popup_grab(popup->popup.xdg_popup,
			popup->popup.grab.seat, popup->popup.grab.serial);
	}

	process_popups(popup, popups);

	wl_surface_add_listener(popup->wl_surface, &popup_listener, popup);
	xdg_surface_add_listener(popup->popup.xdg_surface, &popup_xdg_surface_listener, popup);
	xdg_popup_add_listener(popup->popup.xdg_popup, &xdg_popup_listener, popup);

	wl_surface_commit(popup->wl_surface);

	return popup;
error:
	popup_destroy(popup);
	return NULL;
}

static void layer_destroy(struct surface *layer) {
	if (layer == NULL) {
		return;
	}

	surface_fini(layer);

	if (layer->layer.layer_surface) {
		zwlr_layer_surface_v1_destroy(layer->layer.layer_surface);
	}
	if (layer->wl_surface) {
		wl_surface_destroy(layer->wl_surface);
	}

	free(layer);
}

static bool32_t layer_update(struct surface *layer, struct json_ast_node *json) {
	if (json->type != JSON_AST_NODE_TYPE_OBJECT) {
		return false;
	}

	layer->desired_width = -1;
	layer->desired_height = -1;
	layer->layer.desired_exclusive_zone = INT_MIN;
	string_fini(&layer->userdata);
	layer->userdata = (string_t){ 0 };

	uint32_t new_anchor = 0;
	enum zwlr_layer_shell_v1_layer new_layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;
	int32_t new_margins[4] = { 0 }; // top, right, bottom, left
	struct json_ast_node *new_cursor_shape = NULL, *new_input_regions = NULL,
						*new_popups = NULL, *new_layout_root = NULL;

	for (size_t i = 0; i < json->object.len; ++i) {
		struct json_ast_key_value *key_value = arena_array_struct_json_ast_key_value_get_ptr(&json->object, i);
		if (string_equal(key_value->key, STRING_LITERAL("exclusive_zone"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_INT)
					|| (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
				layer->layer.desired_exclusive_zone = (int32_t)key_value->value.i;
			}
		}
		// ? TODO: exclusive_edge
		else if (string_equal(key_value->key, STRING_LITERAL("anchor"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_UINT)
					&& (key_value->value.u <=
						(ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
						| 	ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT))) {
				new_anchor = (uint32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("layer"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				switch ((enum sw_surface_layer_layer)key_value->value.u) {
				case SW_SURFACE_LAYER_LAYER_BACKGROUND:
					new_layer = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;
					break;
				case SW_SURFACE_LAYER_LAYER_BOTTOM:
					new_layer = ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM;
					break;
				case SW_SURFACE_LAYER_LAYER_OVERLAY:
					new_layer = ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY;
					break;
				case SW_SURFACE_LAYER_LAYER_TOP:
				case SW_SURFACE_LAYER_LAYER_DEFAULT:
				default:
					break;
				}
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("margin_top"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				new_margins[0] = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("margin_right"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				new_margins[1] = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("margin_bottom"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				new_margins[2] = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("margin_left"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				new_margins[3] = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("userdata"))) {
			json_writer_ast_node(&state.userdata_writer, &key_value->value);
			string_init_len(&layer->userdata, state.userdata_writer.buf.data, state.userdata_writer.buf.idx, false);
			json_writer_reset(&state.userdata_writer);
		} else if (string_equal(key_value->key, STRING_LITERAL("width"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_INT)
					|| (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
				layer->desired_width = (int32_t)key_value->value.i;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("height"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_INT)
					|| (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
				layer->desired_height = (int32_t)key_value->value.i;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("cursor_shape"))) {
			new_cursor_shape = &key_value->value;
		} else if (string_equal(key_value->key, STRING_LITERAL("input_regions"))) {
			new_input_regions = &key_value->value;
		} else if (string_equal(key_value->key, STRING_LITERAL("popups"))) {
			new_popups = &key_value->value;
		} else if (string_equal(key_value->key, STRING_LITERAL("layout_root"))) {
			new_layout_root = &key_value->value;
		}
	}

	static const uint32_t horiz = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
	static const uint32_t vert = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
	if ((new_anchor != SW_SURFACE_LAYER_ANCHOR_ALL) &&
			(((layer->desired_width == 0) && ((new_anchor & horiz) != horiz)) ||
			((layer->desired_height == 0) && ((new_anchor & vert) != vert)))) {
		return false;
	}

	surface_block_fini(&layer->layout_root);
	if (!surface_block_init(&layer->layout_root, layer, new_layout_root)) {
		return false;
	}

	if (layer->layer.anchor != new_anchor) {
		zwlr_layer_surface_v1_set_anchor(layer->layer.layer_surface, new_anchor);
		layer->layer.anchor = new_anchor;
	}

	if (layer->layer.layer != new_layer) {
		zwlr_layer_surface_v1_set_layer(layer->layer.layer_surface, new_layer);
		layer->layer.layer = new_layer;
	}

	if (memcmp(layer->layer.margins, new_margins, sizeof(new_margins)) != 0) {
		zwlr_layer_surface_v1_set_margin(layer->layer.layer_surface,
			new_margins[0] / layer->scale,
			new_margins[1] / layer->scale,
			new_margins[2] / layer->scale,
			new_margins[3] / layer->scale);
		memcpy(layer->layer.margins, new_margins, sizeof(new_margins));
	}

	process_cursor_shape(layer, new_cursor_shape);
	process_input_regions(layer, new_input_regions);
	process_popups(layer, new_popups);

	return surface_render(layer);
}

static void layer_handle_layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *layer_surface,
		uint32_t serial, uint32_t width_, uint32_t height_) {
	(void)layer_surface;
	struct surface *layer = data;

	zwlr_layer_surface_v1_ack_configure(layer->layer.layer_surface, serial);

	int32_t width = (int32_t)width_ * layer->scale;
	int32_t height = (int32_t)height_ * layer->scale;
	if (((layer->height != height) || (layer->width != width))
			&& (width != 0) && (height != 0)) {
		surface_buffer_destroy(layer->buffer);
		layer->buffer = surface_buffer_create(width, height, layer);
		layer->width = width;
		layer->height = height;
		layer->dirty = true;
	}

	if (layer->dirty) {
		if (!surface_render(layer)) {
			layer_destroy(layer);
		}
		state.dirty = true;
	}
}

static void layer_handle_layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *layer_surface) {
	(void)layer_surface;
	struct surface *layer = data;
	struct output *output = layer->output;
	for (size_t i = 0; i < output->layers.len; ++i) {
		if (array_struct_surface_ptr_get(&output->layers, i) == layer) {
			layer_destroy(layer);
			array_struct_surface_ptr_put(&output->layers, NULL, i);
			state.dirty = true;
			return;
		}
	}
}

static const struct zwlr_layer_surface_v1_listener layer_layer_surface_listener = {
	.configure = layer_handle_layer_surface_configure,
	.closed = layer_handle_layer_surface_closed,
};

static void layer_handle_preferred_buffer_scale(void *data, struct wl_surface *wl_surface, int32_t factor) {
	(void)wl_surface;
	assert(factor > 0);
	struct surface *layer = data;
	if (layer->scale != factor) {
		layer->scale = factor;
		layer->dirty = true;
		if (layer->layer.anchor != SW_SURFACE_LAYER_ANCHOR_ALL) {
			zwlr_layer_surface_v1_set_size(layer->layer.layer_surface,
				(layer->desired_width == 0) ? 0 : (uint32_t)(layer->width / factor),
				(layer->desired_height == 0) ? 0 : (uint32_t)(layer->height / factor));
		}
		if (layer->layer.exclusive_zone > 0) {
			zwlr_layer_surface_v1_set_exclusive_zone(layer->layer.layer_surface,
				layer->layer.exclusive_zone / factor);
		}
		zwlr_layer_surface_v1_set_margin(layer->layer.layer_surface,
			layer->layer.margins[0] / factor,
			layer->layer.margins[1] / factor,
			layer->layer.margins[2] / factor,
			layer->layer.margins[3] / factor);
		wl_surface_commit(layer->wl_surface);
	}
}

static const struct wl_surface_listener layer_listener = {
	.enter = surface_handle_enter,
	.leave = surface_handle_leave,
	.preferred_buffer_transform = surface_handle_preferred_buffer_transform,
	.preferred_buffer_scale = layer_handle_preferred_buffer_scale,
};

static struct surface *layer_create(struct output *output, struct json_ast_node *json) {
	struct surface *layer = calloc(1, sizeof(struct surface));
	layer->type = SW_SURFACE_TYPE_LAYER;
	surface_init(layer, output);
	layer->layer.layer_surface =
		zwlr_layer_shell_v1_get_layer_surface(state.layer_shell, layer->wl_surface,
		output->wl_output, ZWLR_LAYER_SHELL_V1_LAYER_TOP, PREFIX);
	layer->layer.layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;

	wl_surface_add_listener(layer->wl_surface, &layer_listener, layer);
	zwlr_layer_surface_v1_add_listener(
		layer->layer.layer_surface, &layer_layer_surface_listener, layer);

	if (!layer_update(layer, json)) {
		layer_destroy(layer);
		return NULL;
	}

	return layer;
}

static void process_popups(struct surface *surface, struct json_ast_node *json) {
	size_t len = 0;
	if (json && (json->type == JSON_AST_NODE_TYPE_ARRAY)) {
		len = json->array.len;
		for (size_t i = 0; i < len; ++i) {
			struct json_ast_node *popup_json = arena_array_struct_json_ast_node_get_ptr(&json->array, i);
			struct surface *popup = (i < surface->popups.len)
				? array_struct_surface_ptr_get(&surface->popups, i) : NULL;
			if (!popup) {
				array_struct_surface_ptr_put(&surface->popups, popup_create(surface, popup_json), i);
			} else if (!popup_update(popup, popup_json, NULL, NULL)) {
				popup_destroy(popup);
				array_struct_surface_ptr_put(&surface->popups, NULL, i);
			}
		}
	}
	if (surface->popups.len > len) {
		for (size_t i = len; i < surface->popups.len; ++i) {
			popup_destroy(array_struct_surface_ptr_get(&surface->popups, i));
		}
		surface->popups.len = len;
	}
}

static void process_json_ast(void) {
	state.dirty = true;

	string_fini(&state.userdata);
	state.userdata = (string_t){ 0 };
	state.events = false;

	struct json_ast_node *layers = NULL;
	if (state.json_ast.root.type == JSON_AST_NODE_TYPE_OBJECT) {
		for (size_t i = 0; i < state.json_ast.root.object.len; ++i) {
			struct json_ast_key_value *key_value = arena_array_struct_json_ast_key_value_get_ptr(
				&state.json_ast.root.object, i);
			if (string_equal(key_value->key, STRING_LITERAL("userdata"))) {
				json_writer_ast_node(&state.userdata_writer, &key_value->value);
				string_init_len(&state.userdata, state.userdata_writer.buf.data, state.userdata_writer.buf.idx, false);
				json_writer_reset(&state.userdata_writer);
			} else if (string_equal(key_value->key, STRING_LITERAL("state_events"))) {
				if (key_value->value.type == JSON_AST_NODE_TYPE_BOOL) {
					state.events = key_value->value.b;
				}
			} else if (string_equal(key_value->key, STRING_LITERAL("layers"))) {
				if (key_value->value.type == JSON_AST_NODE_TYPE_OBJECT) {
					layers = &key_value->value;
				}
			}
		}
	}

	for (size_t i = 0; i < state.outputs.len; ++i) {
		struct output *output = array_struct_output_ptr_get(&state.outputs, i);
		if (output->name.len == 0) {
			continue;
		}

		struct json_ast_node *layers_array = NULL;
		if (layers) {
			for (size_t j = 0; j < layers->object.len; ++j) {
				struct json_ast_key_value *key_value = arena_array_struct_json_ast_key_value_get_ptr(&layers->object, j);
				if (string_equal(output->name, key_value->key)) {
					if (key_value->value.type == JSON_AST_NODE_TYPE_ARRAY) {
						layers_array = &key_value->value;
					}
					break;
				}
			}
		}
		size_t layers_array_len = 0;
		if (layers_array) {
			layers_array_len = layers_array->array.len;
			for (size_t j = 0; j < layers_array_len; ++j) {
				struct json_ast_node *layer_json = arena_array_struct_json_ast_node_get_ptr(&layers_array->array, j);
				struct surface *layer = (j < output->layers.len) ?
					array_struct_surface_ptr_get(&output->layers, j) : NULL;
				if (!layer) {
					array_struct_surface_ptr_put(&output->layers, layer_create(output, layer_json), j);
				} else if (!layer_update(layer, layer_json)) {
					layer_destroy(layer);
					array_struct_surface_ptr_put(&output->layers, NULL, j);
				}
			}
		}
		if (output->layers.len > layers_array_len) {
			for (size_t j = layers_array_len; j < output->layers.len; ++j) {
				layer_destroy(array_struct_surface_ptr_get(&output->layers, j));
			}
			output->layers.len = layers_array_len;
		}
	}
}

static void stdin_process(void) {
#if !FUZZ_TEST
    for (;;) {
		ssize_t read_bytes = read(STDIN_FILENO, &state.stdin_buffer.data[state.stdin_buffer.index],
			state.stdin_buffer.size - state.stdin_buffer.index);
		if (read_bytes <= 0) {
			if (read_bytes == 0) {
				errno = EPIPE;
			}
			if (errno == EAGAIN) {
				break;
			} else if (errno == EINTR) {
				continue;
			} else {
				abort_(errno, "read: %s", strerror(errno));
			}
		} else {
			state.stdin_buffer.index += (size_t)read_bytes;
			if (state.stdin_buffer.index == state.stdin_buffer.size) {
				state.stdin_buffer.size *= 2;
				state.stdin_buffer.data = realloc(state.stdin_buffer.data, state.stdin_buffer.size);
			}
		}
    }
#endif // FUZZ_TEST

	if (state.stdin_buffer.index > 0) {
		string_t str = {
			.s = (char *)state.stdin_buffer.data,
			.len = state.stdin_buffer.index,
		};
		json_tokener_set_string(&state.tokener, str);

		for (;;) {
			enum json_tokener_state_ s = json_tokener_ast(&state.tokener, &state.json_ast, 0, true);
			switch (s) {
			case JSON_TOKENER_STATE_SUCCESS:
				process_json_ast();
				json_ast_reset(&state.json_ast);
				if (state.tokener.pos == state.stdin_buffer.index) {
					json_tokener_reset(&state.tokener);
				}
				break;
			case JSON_TOKENER_STATE_ERROR:
#if FUZZ_TEST
				json_tokener_reset(&state.tokener);
				json_ast_reset(&state.json_ast);
				return;
#else
				abort_(1, "failed to parse: "STRING_FMT, STRING_ARGS(str));
#endif // FUZZ_TEST
			case JSON_TOKENER_STATE_MORE_DATA_EXPECTED:
			case JSON_TOKENER_STATE_EOF:
			state.stdin_buffer.index = 0;
				return;
			default:
				ASSERT_UNREACHABLE;
			}
		}
	}
}

static void stdout_flush(void) {
	while (state.writer.buf.idx > 0) {
		ssize_t written = write(STDOUT_FILENO, state.writer.buf.data, state.writer.buf.idx);
		if (written == -1) {
			if (errno == EAGAIN) {
				state.poll_fds[POLL_FD_STDOUT].fd = STDOUT_FILENO;
				break;
			} else if (errno == EINTR) {
				continue;
			} else {
				abort_(errno, "write: %s", strerror(errno));
			}
		} else {
			state.writer.buf.idx -= (size_t)written;
			memmove(state.writer.buf.data, &state.writer.buf.data[written], state.writer.buf.idx);
			if (state.poll_fds[POLL_FD_STDOUT].fd != -1) {
				state.poll_fds[POLL_FD_STDOUT].fd = -1;
			}
		}
	}
}

static void pointer_handle_enter(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *wl_surface,
		wl_fixed_t surface_x, wl_fixed_t surface_y) {
	(void)wl_pointer;
	if (!wl_surface) {
		return;
	}

	struct surface *surface = wl_surface_get_user_data(wl_surface);
	struct pointer *pointer = data;

	if (pointer->cursor_shape_device) {
		wp_cursor_shape_device_v1_set_shape(pointer->cursor_shape_device,
				serial, surface->cursor_shape);
	}

	pointer->surface = surface;
	pointer->wl_pointer_enter_serial = serial;

	pointer->pos_x = (int32_t)(wl_fixed_to_double(surface_x) * (double)surface->scale);
	pointer->pos_y = (int32_t)(wl_fixed_to_double(surface_y) * (double)surface->scale);
	pointer->pos_dirty = true;

	state_update(true);
}

static void pointer_handle_leave(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface) {
	(void)wl_pointer;
	(void)serial;
	(void)surface;
	struct pointer *pointer = data;
	pointer->surface = NULL;

	state_update(true);
}

static void pointer_handle_motion(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	(void)wl_pointer;
	(void)time;
	struct pointer *pointer = data;
	int32_t x, y;
	if (pointer->surface) {
		x = (int32_t)(wl_fixed_to_double(surface_x) * (double)pointer->surface->scale);
		y = (int32_t)(wl_fixed_to_double(surface_y) * (double)pointer->surface->scale);
	} else {
		x = wl_fixed_to_int(surface_x);
		y = wl_fixed_to_int(surface_y);
	}
	if ((x == pointer->pos_x) && (y == pointer->pos_y)) {
		return;
	}

	pointer->pos_x = x;
	pointer->pos_y = y;
	pointer->pos_dirty = true;

	state_update(true);
}

static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t state_) {
	(void)wl_pointer;
	(void)time;
	struct pointer *pointer = data;
	pointer->btn_code = button;
	pointer->btn_state = state_;
	pointer->btn_serial = serial;
	pointer->btn_dirty = true;

	state_update(true);

	struct seat *seat = pointer->seat;
	array_uint32_t_put(&seat->popup_grab.serials, serial, seat->popup_grab.index++);
}

static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	(void)wl_pointer;
	(void)time;
	struct pointer *pointer = data;
	pointer->scroll_axis = axis;
	pointer->scroll_vector_length = wl_fixed_to_double(value);
	pointer->scroll_dirty = true;

	state_update(true);
}

static const struct wl_pointer_listener pointer_listener = {
	.enter = pointer_handle_enter,
	.leave = pointer_handle_leave,
	.motion = pointer_handle_motion,
	.button = pointer_handle_button,
	.axis = pointer_handle_axis,
};

static void wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
	(void)data;
	(void)xdg_wm_base;
	xdg_wm_base_pong(state.wm_base, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {
	.ping = wm_base_handle_ping,
};

static void output_handle_geometry(void *data, struct wl_output *wl_output,
		int32_t x, int32_t y,
		int32_t physical_width, int32_t physical_height,
		int32_t subpixel, const char *make,
		const char *model, int32_t transform) {
	(void)wl_output;
	(void)x;
	(void)y;
	(void)physical_width;
	(void)physical_height;
	(void)subpixel;
	(void)make;
	(void)model;
	assert((transform >= 0) && (transform <= 7));
	struct output *output = data;
	if (output->transform != (enum wl_output_transform)transform) {
		output->transform = (enum wl_output_transform)transform;
		state.dirty = true;
	}
}

static void output_handle_mode(void *data, struct wl_output *wl_output,
		uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
	(void)wl_output;
	(void)flags;
	(void)refresh;
	struct output *output = data;
	if ((output->width != width) || (output->height != height)) {
		output->width = width;
		output->height = height;
		state.dirty = true;
	}
}

static void output_handle_done(void *data, struct wl_output *wl_output) {
	(void)data;
	(void)wl_output;
}

static void output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	(void)wl_output;
	assert(factor > 0);
	struct output *output = data;
	if (output->scale != factor) {
		output->scale = factor;
		state.dirty = true;
	}
}

static void output_handle_name(void *data, struct wl_output *wl_output, const char *name) {
	(void)wl_output;
	struct output *output = data;
	assert(output->name.s == NULL);

	size_t len = strlen(name);
	if (len > 0) {
		string_init_len(&output->name, name, len, true);
	}

	state.dirty = true;
}

static void output_handle_description(void *data, struct wl_output *wl_output, const char *description) {
	(void)data;
	(void)wl_output;
	(void)description;
}

static const struct wl_output_listener output_listener = {
	.geometry = output_handle_geometry,
	.mode = output_handle_mode,
	.done = output_handle_done,
	.scale = output_handle_scale,
	.name = output_handle_name,
	.description = output_handle_description,
};

static struct pointer *pointer_create(struct seat *seat) {
	struct pointer *pointer = calloc(1, sizeof(struct pointer));
	pointer->seat = seat;
	pointer->wl_pointer = wl_seat_get_pointer(seat->wl_seat);

	if (state.cursor_shape_manager) {
		pointer->cursor_shape_device = wp_cursor_shape_manager_v1_get_pointer(
			state.cursor_shape_manager, pointer->wl_pointer);
	}

	wl_pointer_add_listener(pointer->wl_pointer, &pointer_listener, pointer);

	return pointer;
}

static void pointer_destroy(struct pointer *pointer) {
	if (!pointer) {
		return;
	}

	if (pointer->wl_pointer) {
		wl_pointer_destroy(pointer->wl_pointer);
	}

	if (pointer->cursor_shape_device) {
		wp_cursor_shape_device_v1_destroy(pointer->cursor_shape_device);
	}

	free(pointer);
}

static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
	(void)wl_seat;
	struct seat *seat = data;

	bool32_t have_pointer = (capabilities & WL_SEAT_CAPABILITY_POINTER);
	// TODO: touch

	if (have_pointer && !seat->pointer) {
		seat->pointer = pointer_create(seat);
		state.dirty = true;
	} else if (!have_pointer && seat->pointer) {
		pointer_destroy(seat->pointer);
		seat->pointer = NULL;
		state.dirty = true;
	}
}

static void seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name) {
	(void)wl_seat;
	struct seat *seat = data;
	assert(seat->name.s == NULL);

	size_t len = strlen(name);
	if (len > 0) {
		string_init_len(&seat->name, name, len, true);
	}

	state.dirty = true;
}

static const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

static struct output *output_create(uint32_t wl_name) {
	struct output *output = calloc(1, sizeof(struct output));
	output->wl_output = wl_registry_bind(state.registry, wl_name, &wl_output_interface, 4);
	output->wl_name = wl_name;
	output->scale = 1;
	array_struct_surface_ptr_init(&output->layers, 4);
	wl_output_add_listener(output->wl_output, &output_listener, output);

	return output;
}

static void output_destroy(struct output *output) {
	if (!output) {
		return;
	}

	for (size_t i = 0; i < output->layers.len; ++i) {
		layer_destroy(array_struct_surface_ptr_get(&output->layers, i));
	}
	array_struct_surface_ptr_fini(&output->layers);

	if (output->wl_output) {
		wl_output_destroy(output->wl_output);
	}
	string_fini(&output->name);

	free(output);
}

static struct seat *seat_create(uint32_t wl_name) {
	struct seat *seat = calloc(1, sizeof(struct seat));
	seat->wl_seat = wl_registry_bind(state.registry, wl_name, &wl_seat_interface, 2);
	seat->wl_name = wl_name;
	array_uint32_t_init(&seat->popup_grab.serials, 256);
	wl_seat_add_listener(seat->wl_seat, &seat_listener, seat);

	return seat;
}

static void seat_destroy(struct seat *seat) {
	if (!seat) {
		return;
	}

	pointer_destroy(seat->pointer);
	string_fini(&seat->name);
	if (seat->wl_seat) {
		wl_seat_destroy(seat->wl_seat);
	}

	array_uint32_t_fini(&seat->popup_grab.serials);

	free(seat);
}

static void registry_handle_global(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *interface, uint32_t version) {
	(void)data;
	(void)wl_registry;
	(void)version;
	if (strcmp(interface, wl_output_interface.name) == 0) {
		struct output *output = output_create(name);
		array_struct_output_ptr_add(&state.outputs, output);
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		struct seat *seat = seat_create(name);
		array_struct_seat_ptr_add(&state.seats, seat);
    } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
		state.compositor = wl_registry_bind(state.registry, name,
			&wl_compositor_interface, 6);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		state.shm = wl_registry_bind(state.registry, name, &wl_shm_interface, 1);
		// ? TODO: wl_shm_add_listener (check for ARGB32)
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		state.layer_shell = wl_registry_bind(state.registry, name,
			&zwlr_layer_shell_v1_interface, 2);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		state.wm_base = wl_registry_bind(state.registry,
			name, &xdg_wm_base_interface, 3);
		xdg_wm_base_add_listener(state.wm_base, &wm_base_listener, NULL);
	} else if (strcmp(interface, wp_cursor_shape_manager_v1_interface.name) == 0) {
		state.cursor_shape_manager = wl_registry_bind(state.registry, name,
			&wp_cursor_shape_manager_v1_interface, 1);
	}
}

static void registry_handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
	(void)data;
	(void)wl_registry;
	for (size_t i = 0; i < state.outputs.len; ++i) {
		struct output *output = array_struct_output_ptr_get(&state.outputs, i);
		if (output->wl_name == name) {
			output_destroy(output);
			array_struct_output_ptr_pop_swapback(&state.outputs, i);
			state.dirty = true;
			return;
		}
	}

	for (size_t i = 0; i < state.seats.len; ++i) {
		struct seat *seat = array_struct_seat_ptr_get(&state.seats, i);
		if (seat->wl_name == name) {
			seat_destroy(seat);
			array_struct_seat_ptr_pop_swapback(&state.seats, i);
			state.dirty = true;
			return;
		}
	}
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

static void handle_signal(int sig) {
	(void)sig;
	state.running = false;
}

static const struct sigaction sigact = {
	.sa_handler = &handle_signal
};

static void setup(int argc, char **argv) {
#if FUZZ_TEST
	argc = 1;
	argv = (char*[]){ "sw", NULL };
#endif // FUZZ_TEST
	char *locale = setlocale(LC_ALL, "");
	if ((locale == NULL) && (strstr(locale, "utf-8") || strstr(locale, "UTF-8"))) {
		abort_(1, "failed to set utf-8 locale");
	}

	static const struct option long_options[] = {
		{"version", no_argument, NULL, 'v'},
		{ 0 },
	};
	int c;
	while ((c = getopt_long(argc, argv, "v", long_options, NULL)) != -1) {
		switch (c) {
		case 'v':
			abort_(0, "sw version " VERSION);
		default:
			break;
		}
	}

	state.display = wl_display_connect(NULL);
	if (state.display == NULL) {
		abort_(1, "wl_display_connect: %s", strerror(errno));
	}

	array_struct_output_ptr_init(&state.outputs, 4);
	array_struct_seat_ptr_init(&state.seats, 4);

	state.registry = wl_display_get_registry(state.display);
	wl_registry_add_listener(state.registry, &registry_listener, NULL);
	if (wl_display_roundtrip(state.display) == -1) {
		abort_(1, "wl_display_roundtrip: %s", strerror(errno));
	}

	assert(state.compositor != NULL);
	assert(state.shm != NULL);
	if (state.layer_shell == NULL) {
		abort_(EPROTONOSUPPORT, "zwlr_layer_shell_v1: %s", strerror(EPROTONOSUPPORT));
	}
	if (state.wm_base == NULL) {
		abort_(EPROTONOSUPPORT, "xdg_wm_base: %s", strerror(EPROTONOSUPPORT));
	}
	if (state.cursor_shape_manager == NULL) {
		log_stderr("Missing wayland protocol: wp_cursor_shape_manager_v1."
			"Cursor may be rendered incorrectly");
	}

	if (wl_display_roundtrip(state.display) == -1) {
		abort_(1, "wl_display_roundtrip: %s", strerror(errno));
	}

	if (!fd_set_nonblock(STDOUT_FILENO)) {
		abort_(errno, "STDOUT_FILENO O_NONBLOCK fcntl: %s", strerror(errno));
	}
	if (!fd_set_nonblock(STDIN_FILENO)) {
		abort_(errno, "STDIN_FILENO O_NONBLOCK fcntl: %s", strerror(errno));
	}

#if HAVE_TEXT
	if (!fcft_init(FCFT_LOG_COLORIZE_NEVER, false, FCFT_LOG_CLASS_ERROR)) {
		abort_(1, "fcft_init failed");
	}

	hash_table_struct_text_run_cache_init(&state.text_run_cache, 1024);
#endif // HAVE_TEXT

	hash_table_struct_image_cache_init(&state.image_cache, 512);

#if HAVE_SVG
	resvg_init_log();
#endif // HAVE_SVG

	json_tokener_init(&state.tokener);
	json_ast_reset(&state.json_ast);

	buffer_init(&state.stdin_buffer, 4096);

	json_writer_init(&state.writer, 4096);
	json_writer_init(&state.userdata_writer, 256);

	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGPIPE, &sigact, NULL);\

	state.poll_fds[POLL_FD_STDIN] = (struct pollfd){ .fd = STDIN_FILENO, .events = POLLIN };
	state.poll_fds[POLL_FD_STDOUT] = (struct pollfd){ .fd = -1, .events = POLLOUT };
	state.poll_fds[POLL_FD_WAYLAND] = (struct pollfd){ .fd = wl_display_get_fd(state.display), .events = POLLIN };

	state.running = true;
}

static void cleanup(void);

static void run(void) {
	while (state.running) {
		state_update(false);
		stdout_flush();

		if (wl_display_flush(state.display) == -1) {
			if (errno == EAGAIN) {
				state.poll_fds[POLL_FD_WAYLAND].events = (POLLIN | POLLOUT);
			} else {
#if FUZZ_TEST
				if ((errno == ECONNRESET) || (errno == EPIPE)) {
					cleanup();
					setup(0, NULL);
				} else
#endif // FUZZ_TEST
				abort_(errno, "wl_display_flush: %s", strerror(errno));
			}
		} else if (state.poll_fds[POLL_FD_WAYLAND].events & POLLOUT) {
			state.poll_fds[POLL_FD_WAYLAND].events = POLLIN;
		}

		int ms;
		if (state.poll_timers.len > 0) {
			ms = INT_MAX;
			int64_t now_msec = now_ms();
			for (size_t i = 0; i < state.poll_timers.len; ++i) {
				struct poll_timer timer = array_struct_poll_timer_get(&state.poll_timers, i);
				int timer_ms = (int)(timer.expiry - now_msec);
				if (timer_ms < 0) {
					ms = 0;
					break;
				} else if (timer_ms < ms) {
					ms = timer_ms;
				}
			}
		} else {
			ms = -1;
		}

#if FUZZ_TEST
		ms = 0;
#endif // FUZZ_TEST

		if ((poll(state.poll_fds, LENGTH(state.poll_fds), ms) == -1) && (errno != EINTR)) {
			abort_(errno, "poll: %s", strerror(errno));
		}

		static short int err = POLLHUP | POLLERR | POLLNVAL;

#if !FUZZ_TEST
		if (state.poll_fds[POLL_FD_STDIN].revents & (state.poll_fds[POLL_FD_STDIN].events | err)) {
			stdin_process();
		}
#endif // FUZZ_TEST

		if (state.poll_fds[POLL_FD_WAYLAND].revents & (state.poll_fds[POLL_FD_WAYLAND].events | err)) {
			if (wl_display_dispatch(state.display) == -1) {
#if FUZZ_TEST
				if ((errno == ECONNRESET) || (errno == EPIPE)) {
					cleanup();
					setup(0, NULL);
				} else
#endif // FUZZ_TEST
				abort_(errno, "wl_display_dispatch: %s", strerror(errno));
			}
		}

		if (state.poll_timers.len > 0) {
			int64_t now_msec = now_ms();
			for (size_t i = state.poll_timers.len - 1; i != SIZE_MAX; --i) {
				struct poll_timer timer = array_struct_poll_timer_get(&state.poll_timers, i);
				if (now_msec >= timer.expiry) {
					timer.callback(timer.data);
					array_struct_poll_timer_pop_swapback(&state.poll_timers, i);
					now_msec = now_ms();
				}
			}
		}

#if FUZZ_TEST
		return;
#endif // FUZZ_TEST
	}
}

static void cleanup(void) {
	for (size_t i = 0; i < state.outputs.len; ++i) {
		output_destroy(array_struct_output_ptr_get(&state.outputs, i));
	}
	array_struct_output_ptr_fini(&state.outputs);

	for (size_t i = 0; i < state.seats.len; ++i) {
		seat_destroy(array_struct_seat_ptr_get(&state.seats, i));
	}
	array_struct_seat_ptr_fini(&state.seats);

	if (state.cursor_shape_manager) {
		wp_cursor_shape_manager_v1_destroy(state.cursor_shape_manager);
	}
	if (state.layer_shell) {
		zwlr_layer_shell_v1_destroy(state.layer_shell);
	}
	if (state.wm_base) {
		xdg_wm_base_destroy(state.wm_base);
	}
	if (state.compositor) {
		wl_compositor_destroy(state.compositor);
	}
	if (state.shm) {
		wl_shm_destroy(state.shm);
	}
	if (state.registry) {
		wl_registry_destroy(state.registry);
	}

	if (state.display) {
		wl_display_flush(state.display);
		wl_display_disconnect(state.display);
	}


#if DEBUG
#if HAVE_TEXT
	if (state.text_run_cache.items.len > 0) {
		for (size_t i = 0; i < state.text_run_cache.items.len; ++i) {
			struct text_run_cache cache = array_struct_text_run_cache_get(&state.text_run_cache.items, i);
			string_fini(&cache.key);
			for (size_t j = 0; j < cache.items.len; ++j) {
				struct text_run_cache_entry entry = array_struct_text_run_cache_entry_get(&cache.items, j);
				fcft_text_run_destroy(entry.text_run);
				fcft_destroy(entry.font);
			}
			array_struct_text_run_cache_entry_fini(&cache.items);
		}
		hash_table_struct_text_run_cache_fini(&state.text_run_cache);

		fcft_fini();
	}
#endif // HAVE_TEXT

	for (size_t i = 0; i < state.image_cache.items.len; ++i) {
		struct image_cache cache = array_struct_image_cache_get(&state.image_cache.items, i);
		string_fini(&cache.key);
		if (cache.image) {
			pixman_image_unref(cache.image);
		}
	}
	hash_table_struct_image_cache_fini(&state.image_cache);

	string_fini(&state.userdata);

	json_tokener_fini(&state.tokener);
	buffer_fini(&state.stdin_buffer);

	json_writer_fini(&state.writer);
	json_writer_fini(&state.userdata_writer);

	array_struct_poll_timer_fini(&state.poll_timers);

#if FUZZ_TEST
	memset(&state, 0, sizeof(state));
#endif // FUZZ_TEST
#endif // DEBUG
}

#if FUZZ_TEST
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
#if 0
	static bool32_t initialized = false;
	if (!initialized) {
		setup(0, NULL);
		initialized = true;
	}

	if (!state.running) {
		cleanup();
		raise(SIGUSR1);
		return -1;
	}

	state.stdin_buffer.index = size;
	if (state.stdin_buffer.index >= state.stdin_buffer.size) {
		state.stdin_buffer.size = state.stdin_buffer.index * 2;
		free(state.stdin_buffer.data);
		state.stdin_buffer.data = malloc(state.stdin_buffer.size);
	}
	memcpy(state.stdin_buffer.data, data, state.stdin_buffer.index);

	// ? TODO: return -1 on error or when input json is incomplete
	stdin_process();
	run();

	return 0;
#else
	static bool32_t initialized = false;
	if (!initialized) {
		json_tokener_init(&state.tokener);
		json_ast_reset(&state.json_ast);
		sigaction(SIGINT, &sigact, NULL);
		sigaction(SIGTERM, &sigact, NULL);
		state.running = true;
		initialized = true;
	}

	if (!state.running) {
		json_tokener_fini(&state.tokener);
		state.tokener = (struct json_tokener){ 0 };
		raise(SIGUSR1);
		return -1;
	}

	json_tokener_set_string(&state.tokener, (string_t){
		.s = (char *)data,
		.len = size,
	});

	for (;;) {
		switch (json_tokener_ast(&state.tokener, &state.json_ast, 0, true)) {
		case JSON_TOKENER_STATE_SUCCESS:
			json_ast_reset(&state.json_ast);
			if (state.tokener.pos == size) {
				json_tokener_reset(&state.tokener);
			}
			break;
		case JSON_TOKENER_STATE_MORE_DATA_EXPECTED:
		case JSON_TOKENER_STATE_ERROR:
			json_tokener_reset(&state.tokener);
			json_ast_reset(&state.json_ast);
			return -1;
		case JSON_TOKENER_STATE_EOF:
			return 0;
		default:
			ASSERT_UNREACHABLE;
		}
	}
#endif
}
#else
int main(int argc, char **argv) {
	setup(argc, argv);
	run();
	cleanup();

	return EXIT_SUCCESS;
}
#endif // FUZZ_TEST
