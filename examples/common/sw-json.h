#if !defined(SW_JSON_H)
#define SW_JSON_H

//#define TEST

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>

#include "sw.h"
#include "macros.h"
#include "util.h"

#include "json.h"

typedef struct sw_json_output *sw_json_output_create_func_t(string_t name,
	int32_t width, int32_t height, int32_t scale, enum sw_output_transform transform);
typedef void sw_json_output_destroy_func_t(struct sw_json_output *);

// TODO: remove
struct sw_json_surface_block;
typedef void sw_json_surface_block_func_t(struct sw_json_surface_block *);

STATIC_ASSERT(TYPES_COMPATIBLE(TYPEOF(SW_JSON_OUTPUT_CREATE_FUNC), sw_json_output_create_func_t));
STATIC_ASSERT(TYPES_COMPATIBLE(TYPEOF(SW_JSON_OUTPUT_DESTROY_FUNC), sw_json_output_destroy_func_t));
STATIC_ASSERT(TYPES_COMPATIBLE(TYPEOF(SW_JSON_SURFACE_BLOCK_DESTROY_FUNC), sw_json_surface_block_func_t));

struct sw_json_surface_type_layer {
	int32_t exclusive_zone;
	uint32_t anchor; // enum sw_surface_layer_anchor |
	enum sw_surface_layer_layer layer;
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
};

struct sw_json_surface_type_popup {
	int32_t x, y;
	enum sw_surface_popup_gravity gravity;
	uint32_t constraint_adjustment; // enum sw_surface_popup_constraint_adjustment |
	uint32_t grab_serial;
	bool32_t grab;
};

// TODO: make macro
struct sw_json_surface;
typedef void (*sw_json_surface_func_t)(struct sw_json_surface *);
struct sw_json_pointer;
typedef void (*sw_json_pointer_func_t)(struct sw_json_pointer *);


struct sw_json_surface_block_box {
	int32_t x, y;
	int32_t width, height;
};

struct sw_json_region {
	int32_t x, y;
	int32_t width, height;
};

typedef struct sw_json_region struct_sw_json_region;
ARRAY_DECLARE_DEFINE(struct_sw_json_region)

typedef struct sw_json_surface_block* struct_sw_json_surface_block_ptr;
ARRAY_DECLARE(struct_sw_json_surface_block_ptr);

struct sw_json_surface_block_type_text {
	string_t text;
	array_string_t_t font_names;
	union sw_color color;
	uint32_t pad;
};

struct sw_json_surface_block_type_image {
	string_t path;
	enum sw_surface_block_type_image_image_type type;
	uint32_t pad;
};

struct sw_json_surface_block_type_composite {
	enum sw_surface_block_type_composite_block_layout layout;
	uint32_t pad;
	array_struct_sw_json_surface_block_ptr_t children;
};

struct sw_json_surface_block_border {
	string_t width;
	union sw_color color;
	uint32_t pad;
};

struct sw_json_surface_block {
	struct sw_json_surface_block_box box;

	//
	bool32_t raw;

	enum sw_surface_block_type type;
	union {
		//struct  spacer;
		struct sw_json_surface_block_type_text text;
		struct sw_json_surface_block_type_image image;
		struct sw_json_surface_block_type_composite composite;
		string_t json_raw;
	};

	uint64_t id;
	union sw_color color;
	enum sw_surface_block_anchor anchor;
	string_t x, y;
	string_t min_width, max_width;
	string_t min_height, max_height;
	enum sw_surface_block_content_anchor content_anchor;
	enum sw_surface_block_content_transform content_transform;
	uint32_t expand; // enum sw_surface_block_expand |
	uint32_t pad;
	string_t content_width, content_height;
	union {
		struct {
			struct sw_json_surface_block_border border_left;
			struct sw_json_surface_block_border border_right;
			struct sw_json_surface_block_border border_bottom;
			struct sw_json_surface_block_border border_top;
		};
		struct {
			struct sw_json_surface_block_border borders[4]; // left, right, bottom, top
		};
	};
};

typedef struct sw_json_surface* struct_sw_json_surface_ptr;
ARRAY_DECLARE(struct_sw_json_surface_ptr);

struct sw_json_surface {
	enum sw_surface_type type;
	union {
		struct sw_json_surface_type_layer layer;
		struct sw_json_surface_type_popup popup;
	};

	int32_t desired_width, desired_height;
	array_struct_sw_json_region_t input_regions;
	array_struct_sw_json_surface_ptr_t popups;

	sw_json_surface_func_t updated_callback;

	sw_json_pointer_func_t pointer_enter_callback;
	sw_json_pointer_func_t pointer_motion_callback;
	sw_json_pointer_func_t pointer_button_callback;
	sw_json_pointer_func_t pointer_scroll_callback;
	sw_json_pointer_func_t pointer_leave_callback;

	enum sw_surface_cursor_shape cursor_shape;


	int32_t width, height;
	int32_t scale;

	sw_json_surface_func_t destroy;
	uint64_t id;

	struct sw_json_surface_block *layout_root;
};

ARRAY_DEFINE(struct_sw_json_surface_ptr)
ARRAY_DEFINE(struct_sw_json_surface_block_ptr)

typedef struct sw_json_output struct_sw_json_output;
LIST_DECLARE(struct_sw_json_output);

struct sw_json_output {
	string_t name;
	int32_t scale, width, height;
	enum sw_output_transform transform;
	LIST_STRUCT_FIELDS(struct_sw_json_output);

	//
	array_struct_sw_json_surface_ptr_t layers;
};

LIST_DEFINE(struct_sw_json_output)

struct sw_json_pointer {
	struct sw_json_seat *seat;

	struct sw_json_surface *focused_surface;

	int32_t pos_x, pos_y;

	uint32_t btn_code;
	enum sw_pointer_button_state btn_state;
	uint32_t btn_serial;

	enum sw_pointer_scroll_axis scroll_axis;
	double scroll_vector_length;
};

typedef struct sw_json_seat struct_sw_json_seat;
LIST_DECLARE(struct_sw_json_seat);

struct sw_json_seat {
	string_t name;
	struct sw_json_pointer *pointer;
	LIST_STRUCT_FIELDS(struct sw_json_seat);
};

LIST_DEFINE(struct_sw_json_seat)

struct sw_json_connection_buffer {
	size_t size, index;
	uint8_t *data;
};

struct sw_json_connection {
	pid_t pid;
	int read_fd;
	int write_fd;
	uint32_t pad;
	struct sw_json_connection_buffer read_buffer;
	struct json_writer writer;

	list_struct_sw_json_output_t outputs;
	list_struct_sw_json_seat_t seats;

	struct json_tokener tokener;
	struct json_ast state_ast;

	uint64_t sync;
	bool32_t dirty;

	//
	bool32_t state_events;
};

static bool32_t sw_json_init(char * const *cmd);
static void sw_json_disconnect(void);
static int sw_json_flush(void);
static int sw_json_process(void);
static void sw_json_set_dirty(void);

static void sw_json_output_init(struct sw_json_output *, string_t name,
	int32_t width, int32_t height, int32_t scale, enum sw_output_transform);
static void sw_json_output_fini(struct sw_json_output *);

static void sw_json_surface_layer_init(struct sw_json_surface *layer,
	struct sw_json_surface_block *layout_root, sw_json_surface_func_t destroy);
static void sw_json_surface_popup_init(struct sw_json_surface *popup,
	struct sw_json_surface_block *layout_root, int32_t x, int32_t y, sw_json_surface_func_t destroy);
static void sw_json_surface_fini(struct sw_json_surface *);

// TODO: sw_json_surface_block_<type>_init
static void sw_json_surface_block_init(struct sw_json_surface_block *);
static void sw_json_surface_block_fini(struct sw_json_surface_block *);

struct sw_json_connection sw_json_conn;

static void sw_json_set_dirty(void) {
	if (!sw_json_conn.dirty) {
		sw_json_conn.sync++;
		sw_json_conn.dirty = TRUE;
	}
}

static void sw_json_surface_block_init(struct sw_json_surface_block *block) {
	*block = (struct sw_json_surface_block){ 0 };
}

static void sw_json_surface_block_fini(struct sw_json_surface_block *block) {
	if (block->raw) {
		string_fini(&block->json_raw);
		return;
	}

	switch (block->type) {
	case SW_SURFACE_BLOCK_TYPE_TEXT:
		string_fini(&block->text.text);
		for (size_t i = 0; i < block->text.font_names.len; ++i) {
			string_fini(array_string_t_get_ptr(&block->text.font_names, i));
		}
		array_string_t_fini(&block->text.font_names);
		break;
	case SW_SURFACE_BLOCK_TYPE_IMAGE:
		string_fini(&block->image.path);
		break;
	case SW_SURFACE_BLOCK_TYPE_COMPOSITE:
		for (size_t i = 0; i < block->composite.children.len; ++i) {
			struct sw_json_surface_block *b = array_struct_sw_json_surface_block_ptr_get(
				&block->composite.children, i);
			SW_JSON_SURFACE_BLOCK_DESTROY_FUNC(b);
		}
		array_struct_sw_json_surface_block_ptr_fini(&block->composite.children);
		break;
	case SW_SURFACE_BLOCK_TYPE_DEFAULT:
	case SW_SURFACE_BLOCK_TYPE_SPACER:
	default:
		break;
	}

	string_fini(&block->min_width);
	string_fini(&block->max_width);
	string_fini(&block->min_height);
	string_fini(&block->max_height);
	string_fini(&block->content_width);
	string_fini(&block->content_height);
	string_fini(&block->x);
	string_fini(&block->y);

	for (size_t i = 0; i < LENGTH(block->borders); ++i) {
		string_fini(&block->borders[i].width);
	}
}

static void sw_json_surface_init(struct sw_json_surface *surface,
		struct sw_json_surface_block *layout_root, sw_json_surface_func_t destroy) {
	*surface = (struct sw_json_surface){ 0 };

	array_struct_sw_json_surface_ptr_init(&surface->popups, 4);
	array_struct_sw_json_region_init(&surface->input_regions, 4);

	static uint64_t next_id = 0;
	surface->id = next_id++;

	surface->desired_height = -1;
	surface->desired_width = -1;

	surface->layout_root = layout_root;
	surface->destroy = destroy;
}

static void sw_json_surface_layer_init(struct sw_json_surface *layer,
		struct sw_json_surface_block *layout_root, sw_json_surface_func_t destroy) {
	sw_json_surface_init(layer, layout_root, destroy);
	layer->type = SW_SURFACE_TYPE_LAYER;
	layer->layer.exclusive_zone = INT_MIN;
	for (size_t i = 0; i < LENGTH(layer->layer.margins); ++i) {
		layer->layer.margins[i] = -1;
	}
}

static void sw_json_surface_popup_init(struct sw_json_surface *popup,
		struct sw_json_surface_block *layout_root, int32_t x, int32_t y, sw_json_surface_func_t destroy) {
	sw_json_surface_init(popup, layout_root, destroy);
	popup->type = SW_SURFACE_TYPE_POPUP;
	popup->popup.x = x;
	popup->popup.y = y;
}

static void sw_json_surface_fini(struct sw_json_surface *surface) {
	for (size_t i = 0; i < surface->popups.len; ++i) {
		struct sw_json_surface *popup = array_struct_sw_json_surface_ptr_get(&surface->popups, i);
		popup->destroy(popup);
	}
	array_struct_sw_json_surface_ptr_fini(&surface->popups);

	SW_JSON_SURFACE_BLOCK_DESTROY_FUNC(surface->layout_root);

	array_struct_sw_json_region_fini(&surface->input_regions);

	for (struct sw_json_seat *seat = sw_json_conn.seats.head; seat; seat = seat->next) {
		struct sw_json_pointer *pointer = seat->pointer;
		if (pointer && (pointer->focused_surface == surface)) {
			pointer->focused_surface = NULL;
		}
	}
}

static void sw_json_output_init(struct sw_json_output *output, string_t name,
		int32_t width, int32_t height, int32_t scale, enum sw_output_transform transform) {
	*output = (struct sw_json_output){ 0 };

	string_init_string(&output->name, name);
	output->width = width;
	output->height = height;
	output->scale = scale;
	output->transform = transform;
	array_struct_sw_json_surface_ptr_init(&output->layers, 4);
}

static void sw_json_output_fini(struct sw_json_output *output) {
	for (size_t i = 0; i < output->layers.len; ++i) {
		struct sw_json_surface *layer = array_struct_sw_json_surface_ptr_get(&output->layers, i);
		layer->destroy(layer);
	}
	array_struct_sw_json_surface_ptr_fini(&output->layers);

	string_fini(&output->name);
}

static struct sw_json_pointer *sw_json_pointer_create(struct sw_json_seat *seat) {
	struct sw_json_pointer *pointer = calloc(1, SIZEOF(struct sw_json_pointer));
	pointer->seat = seat;

	return pointer;
}

static void sw_json_pointer_destroy(struct sw_json_pointer *pointer) {
	free(pointer);
}

static struct sw_json_seat *sw_json_seat_create(string_t name) {
	struct sw_json_seat *seat = malloc(SIZEOF(struct sw_json_seat));
	string_init_string(&seat->name, name);
	seat->pointer = NULL;

	return seat;
}

static void sw_json_seat_destroy(struct sw_json_seat *seat) {
	if (!seat) {
		return;
	}

	sw_json_pointer_destroy(seat->pointer);
	string_fini(&seat->name);

	free(seat);
}

static bool32_t sw_json_init(char * const *cmd) {
	int pipe_read_fd[2], pipe_write_fd[2];
	if (pipe(pipe_read_fd) == -1) {
		return FALSE;
	}
	if (pipe(pipe_write_fd) == -1) {
		goto error_1;
	}

	if (!fd_set_nonblock(pipe_read_fd[0])) {
		goto error_2;
	}
	if (!fd_set_nonblock(pipe_write_fd[1])) {
		goto error_2;
	}

    pid_t pid = fork();
	if (pid == -1) {
		goto error_2;
	} else if (pid == 0) {
		dup2(pipe_write_fd[0], STDIN_FILENO);
		close(pipe_write_fd[0]);
		close(pipe_write_fd[1]);

		dup2(pipe_read_fd[1], STDOUT_FILENO);
		close(pipe_read_fd[0]);
		close(pipe_read_fd[1]);

		if (cmd[0]) {
			execvp(cmd[0], cmd);
		} else {
			static const char *default_cmd[] = { "sw", NULL };
			execvp(default_cmd[0], (char * const *)default_cmd);
		}
		exit(1);
	}

	close(pipe_read_fd[1]);
    close(pipe_write_fd[0]);

	fd_set_cloexec(pipe_read_fd[0]);
	fd_set_cloexec(pipe_write_fd[1]);

	sw_json_conn.pid = pid;
	sw_json_conn.read_fd = pipe_read_fd[0];
	sw_json_conn.write_fd = pipe_write_fd[1];

	static const size_t initial_buffer_size = 4096;
	sw_json_conn.read_buffer.size = initial_buffer_size;
	sw_json_conn.read_buffer.data = malloc(initial_buffer_size);

	json_writer_init(&sw_json_conn.writer, initial_buffer_size);

	json_tokener_init(&sw_json_conn.tokener);
	json_ast_reset(&sw_json_conn.state_ast);

	return TRUE;
error_2:
	close(pipe_write_fd[0]);
	close(pipe_write_fd[1]);
error_1:
	close(pipe_read_fd[0]);
	close(pipe_read_fd[1]);
	return FALSE;
}

static void sw_json_disconnect(void) {
	kill(sw_json_conn.pid, SIGTERM);
	waitpid(sw_json_conn.pid, NULL, 0);

	close(sw_json_conn.read_fd);
	close(sw_json_conn.write_fd);

	json_writer_fini(&sw_json_conn.writer);

	free(sw_json_conn.read_buffer.data);

	for (struct sw_json_output *output = sw_json_conn.outputs.head; output; ) {
		struct sw_json_output *next = output->next;
		SW_JSON_OUTPUT_DESTROY_FUNC(output);
        output = next;
	}

	for (struct sw_json_seat *seat = sw_json_conn.seats.head; seat; ) {
		struct sw_json_seat *next = seat->next;
		sw_json_seat_destroy(seat);
        seat = next;
	}

	json_tokener_fini(&sw_json_conn.tokener);
}

static void sw_json_describe_surface_block(struct json_writer *writer, struct sw_json_surface_block *block) {
	if (block->raw) {
		ASSERT(block->json_raw.len > 0);
		json_writer_raw(writer, block->json_raw.s, block->json_raw.len);
		return;
	}

	json_writer_object_begin(writer);

	if (block->type != SW_SURFACE_BLOCK_TYPE_DEFAULT) {
		json_writer_object_key(writer, STRING_LITERAL("type"));
		json_writer_uint(writer, block->type);
	}
	if (block->id > 0) {
		json_writer_object_key(writer, STRING_LITERAL("id"));
		json_writer_uint(writer, block->id);
	}
	if (block->anchor != SW_SURFACE_BLOCK_ANCHOR_DEFAULT) {
		json_writer_object_key(writer, STRING_LITERAL("anchor"));
		if ((block->anchor == SW_SURFACE_BLOCK_ANCHOR_NONE) && ((block->x.len > 0) || (block->y.len > 0))) {
			json_writer_object_begin(writer);
			if (block->x.len > 0) {
				json_writer_object_key(writer, STRING_LITERAL("x"));
				json_writer_string(writer, block->x);
			}
			if (block->y.len > 0) {
				json_writer_object_key(writer, STRING_LITERAL("y"));
				json_writer_string(writer, block->y);
			}
			json_writer_object_end(writer);
		} else {
			json_writer_uint(writer, block->anchor);
		}
	}
	if (block->color.a > 0) {
		json_writer_object_key(writer, STRING_LITERAL("color"));
		json_writer_uint(writer, block->color.u32);
	}
	if (block->min_width.len > 0) {
		json_writer_object_key(writer, STRING_LITERAL("min_width"));
		json_writer_string(writer, block->min_width);
	}
	if (block->max_width.len > 0) {
		json_writer_object_key(writer, STRING_LITERAL("max_width"));
		json_writer_string(writer, block->max_width);
	}
	if (block->min_height.len > 0) {
		json_writer_object_key(writer, STRING_LITERAL("min_height"));
		json_writer_string(writer, block->min_height);
	}
	if (block->max_height.len > 0) {
		json_writer_object_key(writer, STRING_LITERAL("max_height"));
		json_writer_string(writer, block->max_height);
	}
	if (block->content_anchor != SW_SURFACE_BLOCK_CONTENT_ANCHOR_DEFAULT) {
		json_writer_object_key(writer, STRING_LITERAL("content_anchor"));
		json_writer_uint(writer, block->content_anchor);
	}
	if (block->content_transform != SW_SURFACE_BLOCK_CONTENT_TRANSFORM_DEFAULT) {
		json_writer_object_key(writer, STRING_LITERAL("content_transform"));
		json_writer_uint(writer, block->content_transform);
	}
	if (block->expand != SW_SURFACE_BLOCK_EXPAND_NONE) {
		json_writer_object_key(writer, STRING_LITERAL("expand"));
		json_writer_uint(writer, block->expand);
	}
	if (block->content_width.len > 0) {
		json_writer_object_key(writer, STRING_LITERAL("content_width"));
		json_writer_string(writer, block->content_width);
	}
	if (block->content_height.len > 0) {
		json_writer_object_key(writer, STRING_LITERAL("content_height"));
		json_writer_string(writer, block->content_height);
	}
	for (size_t j = 0; j < LENGTH(block->borders); ++j) {
		string_t width = block->borders[j].width;
		if (width.len > 0) {
			string_t keys[] = {
				STRING_LITERAL("border_left"),
				STRING_LITERAL("border_right"),
				STRING_LITERAL("border_bottom"),
				STRING_LITERAL("border_top"),
			};
			json_writer_object_key(writer, keys[j]);
			json_writer_object_begin(writer);

			json_writer_object_key(writer, STRING_LITERAL("width"));
			json_writer_string(writer, width);

			if (block->borders[j].color.a > 0) {
				json_writer_object_key(writer, STRING_LITERAL("color"));
				json_writer_uint(writer, block->borders[j].color.u32);
			}

			json_writer_object_end(writer);
		}
	}
	switch (block->type) {
	case SW_SURFACE_BLOCK_TYPE_DEFAULT:
	case SW_SURFACE_BLOCK_TYPE_SPACER:
		break;
	case SW_SURFACE_BLOCK_TYPE_TEXT:
		ASSERT(block->text.text.len > 0);
		json_writer_object_key(writer, STRING_LITERAL("text"));
		json_writer_string_escape(writer, block->text.text);
		if (block->text.font_names.len > 0) {
			json_writer_object_key(writer, STRING_LITERAL("font_names"));
			json_writer_array_begin(writer);

			for (size_t i = 0; i < block->text.font_names.len; ++i) {
				json_writer_string_escape(writer,
					array_string_t_get(&block->text.font_names, i));
			}

			json_writer_array_end(writer);
		}
		if (block->text.color.u32 != 0xFFFFFFFF) {
			json_writer_object_key(writer, STRING_LITERAL("text_color"));
			json_writer_uint(writer, block->text.color.u32);
		}
		break;
	case SW_SURFACE_BLOCK_TYPE_IMAGE:
		ASSERT(block->image.path.len > 0);
		json_writer_object_key(writer, STRING_LITERAL("path"));
		json_writer_string_escape(writer, block->image.path);
		if (block->image.type != SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_DEFAULT) {
			json_writer_object_key(writer, STRING_LITERAL("image_type"));
			json_writer_uint(writer, block->image.type);
		}
		break;
	case SW_SURFACE_BLOCK_TYPE_COMPOSITE: {
		ASSERT(block->composite.children.len > 0);
		if (block->composite.layout != SW_SURFACE_BLOCK_TYPE_COMPOSITE_BLOCK_LAYOUT_DEFAULT) {
			json_writer_object_key(writer, STRING_LITERAL("layout"));
			json_writer_uint(writer, block->composite.layout);
		}

		json_writer_object_key(writer, STRING_LITERAL("children"));
		json_writer_array_begin(writer);

		for (size_t i = 0; i < block->composite.children.len; ++i) {
			sw_json_describe_surface_block(writer,
				array_struct_sw_json_surface_block_ptr_get(&block->composite.children, i));
		}

		json_writer_array_end(writer);
		break;
	}
	default:
		ASSERT_UNREACHABLE;
	}

	json_writer_object_end(writer);
}

static void sw_json_describe_surfaces(struct json_writer *writer,
		array_struct_sw_json_surface_ptr_t *surfaces, enum sw_surface_type expected_type) {
	(void)expected_type;
	for (size_t i = 0; i < surfaces->len; ++i) {
		struct sw_json_surface *surface = array_struct_sw_json_surface_ptr_get(surfaces, i);
		ASSERT(surface->type == expected_type);

		json_writer_object_begin(writer);

		switch (surface->type) {
		case SW_SURFACE_TYPE_LAYER: {
			struct sw_json_surface_type_layer *layer = &surface->layer;
			if (layer->exclusive_zone >= -1) {
				json_writer_object_key(writer, STRING_LITERAL("exclusive_zone"));
				json_writer_int(writer, layer->exclusive_zone);
			}
			if (layer->anchor <= (SW_SURFACE_LAYER_ANCHOR_NONE | SW_SURFACE_LAYER_ANCHOR_TOP
						| SW_SURFACE_LAYER_ANCHOR_BOTTOM | SW_SURFACE_LAYER_ANCHOR_LEFT
						| SW_SURFACE_LAYER_ANCHOR_RIGHT)) {
				json_writer_object_key(writer, STRING_LITERAL("anchor"));
				json_writer_uint(writer, layer->anchor);
			}
			if (layer->layer != SW_SURFACE_LAYER_LAYER_DEFAULT) {
				json_writer_object_key(writer, STRING_LITERAL("layer"));
				json_writer_uint(writer, layer->layer);
			}
			for (size_t j = 0; j < LENGTH(layer->margins); ++j) {
				if (layer->margins[j] >= 0) {
					string_t keys[] = {
						STRING_LITERAL("margin_top"),
						STRING_LITERAL("margin_right"),
						STRING_LITERAL("margin_bottom"),
						STRING_LITERAL("margin_left"),
					};
					json_writer_object_key(writer, keys[j]);
					json_writer_int(writer, layer->margins[j]);
				}
			}
			break;
		}
		case SW_SURFACE_TYPE_POPUP: {
			struct sw_json_surface_type_popup *popup = &surface->popup;
			json_writer_object_key(writer, STRING_LITERAL("x"));
			json_writer_int(writer, popup->x);
			json_writer_object_key(writer, STRING_LITERAL("y"));
			json_writer_int(writer, popup->y);
			if (popup->gravity != SW_SURFACE_POPUP_GRAVITY_DEFAULT) {
				json_writer_object_key(writer, STRING_LITERAL("gravity"));
				json_writer_uint(writer, popup->gravity);
			}
			if (popup->constraint_adjustment <= (SW_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_NONE
						| SW_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_SLIDE_X
						| SW_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_SLIDE_Y
						| SW_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_X
						| SW_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_Y
						| SW_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_RESIZE_X
						| SW_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_RESIZE_Y)) {
				json_writer_object_key(writer, STRING_LITERAL("constraint_adjustment"));
				json_writer_uint(writer, popup->constraint_adjustment);
			}
			if (popup->grab) {
				json_writer_object_key(writer, STRING_LITERAL("grab"));
				json_writer_uint(writer, popup->grab_serial);
			}
			break;
		}
		default:
			ASSERT_UNREACHABLE;
		}

		json_writer_object_key(writer, STRING_LITERAL("userdata"));
		json_writer_uint(writer, surface->id);
		if (surface->desired_width >= 0) {
			json_writer_object_key(writer, STRING_LITERAL("width"));
			json_writer_int(writer, surface->desired_width);
		}
		if (surface->desired_height >= 0) {
			json_writer_object_key(writer, STRING_LITERAL("height"));
			json_writer_int(writer, surface->desired_height);
		}
		if (surface->cursor_shape != SW_SURFACE_CURSOR_SHAPE_DEFAULT_) {
			json_writer_object_key(writer, STRING_LITERAL("cursor_shape"));
			json_writer_uint(writer, surface->cursor_shape);
		}
		if (surface->input_regions.len > 0) {
			json_writer_object_key(writer, STRING_LITERAL("input_regions"));
			json_writer_array_begin(writer);

			for (size_t j = 0; j < surface->input_regions.len; ++j) {
				struct sw_json_region region = array_struct_sw_json_region_get(
					&surface->input_regions, j);
				json_writer_object_begin(writer);
				if (region.x != 0) {
					json_writer_object_key(writer, STRING_LITERAL("x"));
					json_writer_int(writer, region.x);
				}
				if (region.y != 0) {
					json_writer_object_key(writer, STRING_LITERAL("y"));
					json_writer_int(writer, region.y);
				}
				if (region.width != 0) {
					json_writer_object_key(writer, STRING_LITERAL("width"));
					json_writer_int(writer, region.width);
				}
				if (region.height != 0) {
					json_writer_object_key(writer, STRING_LITERAL("height"));
					json_writer_int(writer, region.height);
				}
				json_writer_object_end(writer);
			}

			json_writer_array_end(writer);
		}
		if (surface->popups.len > 0) {
			json_writer_object_key(writer, STRING_LITERAL("popups"));
			json_writer_array_begin(writer);

			sw_json_describe_surfaces(writer, &surface->popups, SW_SURFACE_TYPE_POPUP);

			json_writer_array_end(writer);
		}
		json_writer_object_key(writer, STRING_LITERAL("layout_root"));
		sw_json_describe_surface_block(writer, surface->layout_root);

		json_writer_object_end(writer);
	}
}

static void sw_json_describe_layers(struct json_writer *writer) {
	if (sw_json_conn.outputs.len == 0) {
		return;
	}

	// TODO: do not emit if there are no layers
	json_writer_object_key(writer, STRING_LITERAL("layers"));
	json_writer_object_begin(writer);

	for (struct sw_json_output *output = sw_json_conn.outputs.head; output; output = output->next) {
		if (output->layers.len == 0) {
			continue;
		}

		json_writer_object_key_escape(writer, output->name);
		json_writer_array_begin(writer);

		sw_json_describe_surfaces(writer, &output->layers, SW_SURFACE_TYPE_LAYER);

		json_writer_array_end(writer);
	}

	json_writer_object_end(writer);
}

static int sw_json_flush(void) {
	if (sw_json_conn.dirty) {
		struct json_writer *writer = &sw_json_conn.writer;
		json_writer_object_begin(writer);

		json_writer_object_key(writer, STRING_LITERAL("userdata"));
		json_writer_uint(writer, sw_json_conn.sync);

		if (sw_json_conn.state_events) {
			json_writer_object_key(writer, STRING_LITERAL("state_events"));
			json_writer_bool(writer, TRUE);
		}

		sw_json_describe_layers(writer);

		json_writer_object_end(writer);

		sw_json_conn.dirty = FALSE;
	}

	while (sw_json_conn.writer.buf.idx > 0) {
		ssize_t written_bytes = write(sw_json_conn.write_fd,
			sw_json_conn.writer.buf.data, sw_json_conn.writer.buf.idx);
		if (written_bytes == -1) {
			if (errno == EAGAIN) {
				return 0;
			} else if (errno == EINTR) {
				continue;
			} else {
				return -1;
			}
		} else {
			sw_json_conn.writer.buf.idx -= (size_t)written_bytes;
			memmove(sw_json_conn.writer.buf.data,
				&sw_json_conn.writer.buf.data[written_bytes],
				sw_json_conn.writer.buf.idx);
		}
	}

	return 1;
}

static void sw_json_process_surface_block(struct sw_json_surface_block *block, struct json_ast_node *block_json) {
	if (block_json->type == JSON_AST_NODE_TYPE_NULL) {
		block->box = (struct sw_json_surface_block_box){ 0 };
		return;
	}

	ASSERT(block_json->type == JSON_AST_NODE_TYPE_OBJECT);
	if (block->type == SW_SURFACE_BLOCK_TYPE_COMPOSITE) {
		ASSERT(block_json->object.len == 5);

		struct json_ast_key_value *children = arena_array_struct_json_ast_key_value_get_ptr(&block_json->object, 4);
		ASSERT(string_equal(children->key, STRING_LITERAL("children")));
		ASSERT(children->value.type == JSON_AST_NODE_TYPE_ARRAY);

		ASSERT(block->composite.children.len == children->value.array.len);

		for (size_t i = 0; i < block->composite.children.len; ++i) {
			sw_json_process_surface_block(
				array_struct_sw_json_surface_block_ptr_get(&block->composite.children, i),
				arena_array_struct_json_ast_node_get_ptr(&children->value.array, i));
		}
	} else {
		ASSERT(block_json->object.len == 4);
	}

	struct json_ast_key_value *x = arena_array_struct_json_ast_key_value_get_ptr(&block_json->object, 0);
	ASSERT(string_equal(x->key, STRING_LITERAL("x")));
	ASSERT((x->value.type == JSON_AST_NODE_TYPE_UINT) || (x->value.type == JSON_AST_NODE_TYPE_INT));
	block->box.x = (int32_t)x->value.i;

	struct json_ast_key_value *y = arena_array_struct_json_ast_key_value_get_ptr(&block_json->object, 1);
	ASSERT(string_equal(y->key, STRING_LITERAL("y")));
	ASSERT((y->value.type == JSON_AST_NODE_TYPE_UINT) || (y->value.type == JSON_AST_NODE_TYPE_INT));
	block->box.y = (int32_t)y->value.i;

	struct json_ast_key_value *width = arena_array_struct_json_ast_key_value_get_ptr(&block_json->object, 2);
	ASSERT(string_equal(width->key, STRING_LITERAL("width")));
	ASSERT(width->value.type == JSON_AST_NODE_TYPE_UINT);
	block->box.width = (int32_t)width->value.i;

	struct json_ast_key_value *height = arena_array_struct_json_ast_key_value_get_ptr(&block_json->object, 3);
	ASSERT(string_equal(height->key, STRING_LITERAL("height")));
	ASSERT(height->value.type == JSON_AST_NODE_TYPE_UINT);
	block->box.height = (int32_t)height->value.i;
}

static void sw_json_process_surfaces(array_struct_sw_json_surface_ptr_t *dest, struct json_ast_node *source) {
	ASSERT(source->type == JSON_AST_NODE_TYPE_ARRAY);
	ASSERT(dest->len == source->array.len);

	for (size_t i = dest->len - 1; i != SIZE_MAX; --i) {
		struct json_ast_node *surface_json = arena_array_struct_json_ast_node_get_ptr(&source->array, i);
		struct sw_json_surface *surface = array_struct_sw_json_surface_ptr_get(dest, i);
		if (surface_json->type == JSON_AST_NODE_TYPE_NULL) {
			array_struct_sw_json_surface_ptr_pop(dest, i);
			surface->destroy(surface);
			continue;
		}

		ASSERT(surface_json->type == JSON_AST_NODE_TYPE_OBJECT);
		ASSERT(surface_json->object.len == 5);

		struct json_ast_key_value *width = arena_array_struct_json_ast_key_value_get_ptr(&surface_json->object, 0);
		ASSERT(string_equal(width->key, STRING_LITERAL("width")));
		ASSERT(width->value.type == JSON_AST_NODE_TYPE_UINT);
		surface->width = (int32_t)width->value.i;

		struct json_ast_key_value *height = arena_array_struct_json_ast_key_value_get_ptr(&surface_json->object, 1);
		ASSERT(string_equal(height->key, STRING_LITERAL("height")));
		ASSERT(height->value.type == JSON_AST_NODE_TYPE_UINT);
		surface->height = (int32_t)height->value.i;

		struct json_ast_key_value *scale = arena_array_struct_json_ast_key_value_get_ptr(&surface_json->object, 2);
		ASSERT(string_equal(scale->key, STRING_LITERAL("scale")));
		ASSERT(scale->value.type == JSON_AST_NODE_TYPE_UINT);
		surface->scale = (int32_t)scale->value.i;

		struct json_ast_key_value *layout_root = arena_array_struct_json_ast_key_value_get_ptr(&surface_json->object, 3);
		ASSERT(string_equal(layout_root->key, STRING_LITERAL("layout_root")));
		sw_json_process_surface_block(surface->layout_root, &layout_root->value);

		if (surface->updated_callback) {
			surface->updated_callback(surface);
		}

		struct json_ast_key_value *popups = arena_array_struct_json_ast_key_value_get_ptr(&surface_json->object, 4);
		ASSERT(string_equal(popups->key, STRING_LITERAL("popups")));
		sw_json_process_surfaces(&surface->popups, &popups->value);
	}
}

static void sw_json_process_outputs(struct json_ast_node *outputs_array, struct json_ast_node *userdata) {
	ASSERT(outputs_array->type == JSON_AST_NODE_TYPE_ARRAY);
	ASSERT(userdata->type == JSON_AST_NODE_TYPE_UINT);

	list_struct_sw_json_output_t new_outputs = { 0 };

	for (size_t i = 0; i < outputs_array->array.len; ++i) {
		struct json_ast_node *output_object = arena_array_struct_json_ast_node_get_ptr(
			&outputs_array->array, i);
		ASSERT(output_object->type == JSON_AST_NODE_TYPE_OBJECT);
		ASSERT(output_object->object.len == 6);

		struct json_ast_key_value *name = arena_array_struct_json_ast_key_value_get_ptr(
			&output_object->object, 0);
		ASSERT(string_equal(name->key, STRING_LITERAL("name")));
		ASSERT(name->value.type == JSON_AST_NODE_TYPE_STRING);

		string_t output_name = name->value.s;
		struct sw_json_output *output = NULL;
		for (struct sw_json_output *output_ = sw_json_conn.outputs.head; output_; output_ = output_->next) {
			if (string_equal(output_->name, output_name)) {
				list_struct_sw_json_output_pop(&sw_json_conn.outputs, output_);
				output = output_;
				break;
			}
		}

		struct json_ast_key_value *width = arena_array_struct_json_ast_key_value_get_ptr(&output_object->object, 1);
		ASSERT(string_equal(width->key, STRING_LITERAL("width")));
		ASSERT(width->value.type == JSON_AST_NODE_TYPE_UINT);

		struct json_ast_key_value *height = arena_array_struct_json_ast_key_value_get_ptr(&output_object->object, 2);
		ASSERT(string_equal(height->key, STRING_LITERAL("height")));
		ASSERT(height->value.type == JSON_AST_NODE_TYPE_UINT);

		struct json_ast_key_value *scale = arena_array_struct_json_ast_key_value_get_ptr(&output_object->object, 3);
		ASSERT(string_equal(scale->key, STRING_LITERAL("scale")));
		ASSERT(scale->value.type == JSON_AST_NODE_TYPE_UINT);

		struct json_ast_key_value *transform = arena_array_struct_json_ast_key_value_get_ptr(&output_object->object, 4);
		ASSERT(string_equal(transform->key, STRING_LITERAL("transform")));
		ASSERT(transform->value.type == JSON_AST_NODE_TYPE_UINT);

		if (!output) {
			output = SW_JSON_OUTPUT_CREATE_FUNC(output_name,
				(int32_t)width->value.i, (int32_t)height->value.i,
				(int32_t)scale->value.i, (enum sw_output_transform)transform->value.u);
			ASSERT(string_equal(output_name, output->name));
		} else {
			struct json_ast_key_value *layers = arena_array_struct_json_ast_key_value_get_ptr(&output_object->object, 5);
			ASSERT(string_equal(layers->key, STRING_LITERAL("layers")));

			output->width = (int32_t)width->value.i;
			output->height = (int32_t)height->value.i;
			output->scale = (int32_t)scale->value.i;
			output->transform = (enum sw_output_transform)scale->value.u;

			if (userdata->u == sw_json_conn.sync) {
				sw_json_process_surfaces(&output->layers, &layers->value);
			}
		}

		list_struct_sw_json_output_insert_tail(&new_outputs, output);
	}

	for (struct sw_json_output *output = sw_json_conn.outputs.head; output; ) {
		struct sw_json_output *next = output->next;
		SW_JSON_OUTPUT_DESTROY_FUNC(output);
        output = next;
	}

	sw_json_conn.outputs = new_outputs;
}

static ATTRIB_PURE struct sw_json_surface *sw_json_find_surface_recursive(
		uint64_t id, array_struct_sw_json_surface_ptr_t *source) {
	for (size_t i = 0; i < source->len; ++i) {
		struct sw_json_surface *surface = array_struct_sw_json_surface_ptr_get(source, i);
		if (surface->id == id) {
			return surface;
		}
		surface = sw_json_find_surface_recursive(id, &surface->popups);
		if (surface) {
			return surface;
		}
	}

	return NULL;
}

static ATTRIB_PURE struct sw_json_surface *sw_json_find_surface(uint64_t id) {
	for (struct sw_json_output *output = sw_json_conn.outputs.head; output; output = output->next) {
		struct sw_json_surface *surface = sw_json_find_surface_recursive(id, &output->layers);
		if (surface) {
			return surface;
		}
	}

	return NULL;
}

static void sw_json_parse_pointer(struct sw_json_seat *seat,
		struct json_ast_node *pointer_json) {
	if (pointer_json->type == JSON_AST_NODE_TYPE_NULL) {
		if (seat->pointer) {
			sw_json_pointer_destroy(seat->pointer);
			seat->pointer = NULL;
		}
		return;
	}

	ASSERT(pointer_json->type == JSON_AST_NODE_TYPE_OBJECT);
	ASSERT(pointer_json->object.len == 4);

	if (!seat->pointer) {
		seat->pointer = sw_json_pointer_create(seat);
	}

	struct sw_json_pointer *pointer = seat->pointer;

	struct json_ast_key_value *position_key_value = arena_array_struct_json_ast_key_value_get_ptr(&pointer_json->object, 0);
	ASSERT(string_equal(position_key_value->key, STRING_LITERAL("position")));
	bool32_t position_dirty;
	if (position_key_value->value.type == JSON_AST_NODE_TYPE_OBJECT) {
		struct json_ast_node *position = &position_key_value->value;
		ASSERT(position->object.len == 2);

		struct json_ast_key_value *x = arena_array_struct_json_ast_key_value_get_ptr(&position->object, 0);
		ASSERT(string_equal(x->key, STRING_LITERAL("x")));
		ASSERT((x->value.type == JSON_AST_NODE_TYPE_UINT) || (x->value.type == JSON_AST_NODE_TYPE_INT));
		seat->pointer->pos_x = (int32_t)x->value.i;

		struct json_ast_key_value *y = arena_array_struct_json_ast_key_value_get_ptr(&position->object, 1);
		ASSERT(string_equal(y->key, STRING_LITERAL("y")));
		ASSERT((y->value.type == JSON_AST_NODE_TYPE_UINT) || (y->value.type == JSON_AST_NODE_TYPE_INT));
		seat->pointer->pos_y = (int32_t)y->value.i;

		position_dirty = TRUE;
	} else {
		ASSERT(position_key_value->value.type == JSON_AST_NODE_TYPE_NULL);
		position_dirty = FALSE;
	}

	struct json_ast_key_value *button_key_value = arena_array_struct_json_ast_key_value_get_ptr(&pointer_json->object, 1);
	ASSERT(string_equal(button_key_value->key, STRING_LITERAL("button")));
	bool32_t button_dirty;
	if (button_key_value->value.type == JSON_AST_NODE_TYPE_OBJECT) {
		struct json_ast_node *button = &button_key_value->value;
		ASSERT(button->object.len == 3);

		struct json_ast_key_value *code = arena_array_struct_json_ast_key_value_get_ptr(&button->object, 0);
		ASSERT(string_equal(code->key, STRING_LITERAL("code")));
		ASSERT(code->value.type == JSON_AST_NODE_TYPE_UINT);
		seat->pointer->btn_code = (uint32_t)code->value.u;

		struct json_ast_key_value *state = arena_array_struct_json_ast_key_value_get_ptr(&button->object, 1);
		ASSERT(string_equal(state->key, STRING_LITERAL("state")));
		ASSERT(state->value.type == JSON_AST_NODE_TYPE_UINT);
		seat->pointer->btn_state = (uint32_t)state->value.u;

		struct json_ast_key_value *serial = arena_array_struct_json_ast_key_value_get_ptr(&button->object, 2);
		ASSERT(string_equal(serial->key, STRING_LITERAL("serial")));
		ASSERT(serial->value.type == JSON_AST_NODE_TYPE_UINT);
		seat->pointer->btn_serial = (uint32_t)serial->value.u;

		button_dirty = TRUE;
	} else {
		ASSERT(button_key_value->value.type == JSON_AST_NODE_TYPE_NULL);
		button_dirty = FALSE;
	}

	struct json_ast_key_value *scroll_key_value = arena_array_struct_json_ast_key_value_get_ptr(&pointer_json->object, 2);
	ASSERT(string_equal(scroll_key_value->key, STRING_LITERAL("scroll")));
	bool32_t scroll_dirty;
	if (scroll_key_value->value.type == JSON_AST_NODE_TYPE_OBJECT) {
		struct json_ast_node *scroll = &scroll_key_value->value;
		ASSERT(scroll->object.len == 2);

		struct json_ast_key_value *axis = arena_array_struct_json_ast_key_value_get_ptr(&scroll->object, 0);
		ASSERT(string_equal(axis->key, STRING_LITERAL("axis")));
		ASSERT(axis->value.type == JSON_AST_NODE_TYPE_UINT);
		seat->pointer->scroll_axis = (enum sw_pointer_scroll_axis)axis->value.u;

		struct json_ast_key_value *vector_length = arena_array_struct_json_ast_key_value_get_ptr(&scroll->object, 1);
		ASSERT(string_equal(vector_length->key, STRING_LITERAL("vector_length")));
		ASSERT(vector_length->value.type == JSON_AST_NODE_TYPE_DOUBLE);
		seat->pointer->scroll_vector_length = (double)vector_length->value.d;

		scroll_dirty = TRUE;
	} else {
		ASSERT(scroll_key_value->value.type == JSON_AST_NODE_TYPE_NULL);
		scroll_dirty = FALSE;
	}

	struct json_ast_key_value *surface_key_value = arena_array_struct_json_ast_key_value_get_ptr(&pointer_json->object, 3);
	ASSERT(string_equal(surface_key_value->key, STRING_LITERAL("surface")));
	if (surface_key_value->value.type == JSON_AST_NODE_TYPE_OBJECT) {
		struct json_ast_node *surface = &surface_key_value->value;
		ASSERT(surface->object.len == 1);

		struct json_ast_key_value *userdata = arena_array_struct_json_ast_key_value_get_ptr(&surface->object, 0);
		ASSERT(string_equal(userdata->key, STRING_LITERAL("userdata")));
		ASSERT(userdata->value.type == JSON_AST_NODE_TYPE_UINT);

		sw_json_pointer_func_t motion_callback;
		uint64_t focused_surface_id = userdata->value.u;
		if ((pointer->focused_surface == NULL) || (pointer->focused_surface->id != focused_surface_id)) {
			if (pointer->focused_surface && pointer->focused_surface->pointer_leave_callback) {
				pointer->focused_surface->pointer_leave_callback(pointer);
			}
			pointer->focused_surface = sw_json_find_surface(focused_surface_id);
			motion_callback = pointer->focused_surface ? pointer->focused_surface->pointer_enter_callback : NULL;
		} else {
			motion_callback = pointer->focused_surface->pointer_motion_callback;
		}

		if (position_dirty && pointer->focused_surface) {
			if (motion_callback) {
				motion_callback(pointer);
			}
		}

		if (button_dirty && pointer->focused_surface &&
					pointer->focused_surface->pointer_button_callback) {
			pointer->focused_surface->pointer_button_callback(pointer);
		}

		if (scroll_dirty && pointer->focused_surface &&
					pointer->focused_surface->pointer_scroll_callback) {
			pointer->focused_surface->pointer_scroll_callback(pointer);
		}
	} else {
		ASSERT(surface_key_value->value.type == JSON_AST_NODE_TYPE_NULL);
		if (pointer->focused_surface && pointer->focused_surface->pointer_leave_callback) {
			pointer->focused_surface->pointer_leave_callback(pointer);
		}
		pointer->focused_surface = NULL;
	}
}

static void sw_json_process_seats(struct json_ast_node *seats_array) {
	ASSERT(seats_array->type == JSON_AST_NODE_TYPE_ARRAY);

	list_struct_sw_json_seat_t new_seats = { 0 };

	for (size_t i = 0; i < seats_array->array.len; ++i) {
		struct json_ast_node *seat_object = arena_array_struct_json_ast_node_get_ptr(&seats_array->array, i);
		ASSERT(seat_object->type == JSON_AST_NODE_TYPE_OBJECT);
		ASSERT(seat_object->object.len == 2);

		struct json_ast_key_value *name = arena_array_struct_json_ast_key_value_get_ptr(
			&seat_object->object, 0);
		ASSERT(string_equal(name->key, STRING_LITERAL("name")));
		ASSERT(name->value.type == JSON_AST_NODE_TYPE_STRING);
		string_t seat_name = name->value.s;
		struct sw_json_seat *seat = NULL;
		for (struct sw_json_seat *seat_ = sw_json_conn.seats.head; seat_; seat_ = seat_->next) {
			if (string_equal(seat_->name, seat_name)) {
				list_struct_sw_json_seat_pop(&sw_json_conn.seats, seat_);
				seat = seat_;
				break;
			}
		}

		if (seat == NULL) {
			seat = sw_json_seat_create(seat_name);
		}

		list_struct_sw_json_seat_insert_tail(&new_seats, seat);
	}

	for (struct sw_json_seat *seat = sw_json_conn.seats.head; seat; ) {
		struct sw_json_seat *next = seat->next;
		sw_json_seat_destroy(seat);
        seat = next;
	}

	sw_json_conn.seats = new_seats;

	ASSERT(sw_json_conn.seats.len == seats_array->array.len);
	size_t i = 0;
	for (struct sw_json_seat *seat = sw_json_conn.seats.head; seat; seat = seat->next) {
		struct json_ast_node *seat_object = arena_array_struct_json_ast_node_get_ptr(&seats_array->array, i);
		struct json_ast_key_value *pointer = arena_array_struct_json_ast_key_value_get_ptr(
			&seat_object->object, 1);
		ASSERT(string_equal(pointer->key, STRING_LITERAL("pointer")));
		sw_json_parse_pointer(seat, &pointer->value);
	}
}

static void sw_json_process_state_ast(void) {
	// Since the order of keys is constant, avoid using loops or hash tables
	ASSERT(sw_json_conn.state_ast.root.type == JSON_AST_NODE_TYPE_OBJECT);
	ASSERT(sw_json_conn.state_ast.root.object.len == 3);

	struct json_ast_key_value *userdata = arena_array_struct_json_ast_key_value_get_ptr(
		&sw_json_conn.state_ast.root.object, 0);
	ASSERT(string_equal(userdata->key, STRING_LITERAL("userdata")));

	struct json_ast_key_value *outputs = arena_array_struct_json_ast_key_value_get_ptr(
		&sw_json_conn.state_ast.root.object, 1);
	ASSERT(string_equal(outputs->key, STRING_LITERAL("outputs")));
	sw_json_process_outputs(&outputs->value, &userdata->value);

	struct json_ast_key_value *seats = arena_array_struct_json_ast_key_value_get_ptr(
		&sw_json_conn.state_ast.root.object, 2);
	ASSERT(string_equal(seats->key, STRING_LITERAL("seats")));
	sw_json_process_seats(&seats->value);
}

static int sw_json_process(void) {
	struct sw_json_connection_buffer *read_buffer = &sw_json_conn.read_buffer;
    for (;;) {
		ssize_t read_bytes = read(sw_json_conn.read_fd, &read_buffer->data[read_buffer->index],
			read_buffer->size - read_buffer->index);
		if (read_bytes <= 0) {
			if (read_bytes == 0) {
				errno = EPIPE;
			}
			if (errno == EAGAIN) {
				break;
			} else if (errno == EINTR) {
				continue;
			} else {
				return -1;
			}
		} else {
			read_buffer->index += (size_t)read_bytes;
			if (read_buffer->index == read_buffer->size) {
				read_buffer->size *= 2;
				read_buffer->data = realloc(read_buffer->data, read_buffer->size);
			}
		}
    }

	if (read_buffer->index > 0) {
		string_t str = {
			.s = (char *)read_buffer->data,
			.len = read_buffer->index,
		};
		json_tokener_set_string(&sw_json_conn.tokener, str);

		for (;;) {
			enum json_tokener_state_ state = json_tokener_ast(
				&sw_json_conn.tokener, &sw_json_conn.state_ast, 0, TRUE);
			switch (state) {
			case JSON_TOKENER_STATE_SUCCESS:
				sw_json_process_state_ast();
				json_ast_reset(&sw_json_conn.state_ast);
				if (sw_json_conn.tokener.pos == read_buffer->index) {
					json_tokener_reset(&sw_json_conn.tokener);
				}
				break;
			case JSON_TOKENER_STATE_ERROR:
				abort_(1, "failed to parse: "STRING_FMT, STRING_ARGS(str));
			case JSON_TOKENER_STATE_MORE_DATA_EXPECTED:
			case JSON_TOKENER_STATE_EOF:
				read_buffer->index = 0;
				return 0;
			default:
				ASSERT_UNREACHABLE;
			}
		}
	}

	return 0;
}

#endif // SW_JSON_H
