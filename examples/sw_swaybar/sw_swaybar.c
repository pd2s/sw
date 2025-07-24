#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <time.h>
#include <locale.h>
#include <linux/input-event-codes.h>

#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>

#if !defined(DEBUG)
#define DEBUG 0
#endif

#if !defined(WITH_TRAY)
#define WITH_TRAY 1
#endif // !defined(WITH_TRAY)

#define SU_DEBUG DEBUG
#define SU_LOG_PREFIX "sw_swaybar: "

#define SU_IMPLEMENTATION
#define SU_STRIP_PREFIXES
#include "../../su.h"

#if !defined(SW_WITH_WAYLAND)
#define SW_WITH_WAYLAND 1
#endif // !defined(SW_WITH_WAYLAND)
#if !defined(SW_WITH_TEXT)
#define SW_WITH_TEXT 1
#endif // !defined(SW_WITH_TEXT)
#if !defined(SW_WITH_SVG)
#define SW_WITH_SVG 1
#endif // !defined(SW_WITH_SVG)
#if !defined(SW_WITH_PNG)
#define SW_WITH_PNG 1
#endif // !defined(SW_WITH_PNG)
#if !defined(SW_WITH_JPG)
#define SW_WITH_JPG 0
#endif // !defined(SW_WITH_JPG)
#if !defined(SW_WITH_TGA)
#define SW_WITH_TGA 0
#endif // !defined(SW_WITH_TGA)
#if !defined(SW_WITH_BMP)
#define SW_WITH_BMP 0
#endif // !defined(SW_WITH_BMP)
#if !defined(SW_WITH_PSD)
#define SW_WITH_PSD 0
#endif // !defined(SW_WITH_PSD)
#if !defined(SW_WITH_GIF)
#define SW_WITH_GIF 0
#endif // !defined(SW_WITH_GIF)
#if !defined(SW_WITH_HDR)
#define SW_WITH_HDR 0
#endif // !defined(SW_WITH_HDR)
#if !defined(SW_WITH_PIC)
#define SW_WITH_PIC 0
#endif // !defined(SW_WITH_PIC)
#if !defined(SW_WITH_PNM)
#define SW_WITH_PNM 0
#endif // !defined(SW_WITH_PNM)

#define SW_DEBUG DEBUG
#define SW_IMPLEMENTATION
#include "../../sw.h"

STATIC_ASSERT(SW_WITH_WAYLAND && SW_WITH_TEXT);

#include "sway_ipc.h"

#if WITH_TRAY
#if SW_WITH_SVG || SW_WITH_PNG
#define WITH_SVG SW_WITH_SVG
#define WITH_PNG SW_WITH_PNG
#include "xdg_icon_theme.h"
#endif // SW_WITH_SVG || SW_WITH_PNG
#include "sni_server.h"
#endif // WITH_TRAY

#include "config.h"

#define KEY_SCROLL_UP KEY_MAX + 1
#define KEY_SCROLL_DOWN KEY_MAX + 2
#define KEY_SCROLL_LEFT KEY_MAX + 3
#define KEY_SCROLL_RIGHT KEY_MAX + 4

struct workspace {
	string_t name;
	int32_t num;
	bool32_t visible;
	bool32_t focused;
	bool32_t urgent;
};

typedef struct workspace struct_workspace;
ARRAY_DECLARE_DEFINE(struct_workspace)

struct output {
	sw_wayland_output_t _; // must be first
	su_array__struct_workspace__t workspaces;
	bool32_t focused;
	PAD32;
};

struct bar {
	sw_wayland_surface_t _; // must be first
	struct output *output;
};

enum layout_block_type {
	LAYOUT_BLOCK_TYPE_DUMMY,
	LAYOUT_BLOCK_TYPE_WORKSPACE,
	LAYOUT_BLOCK_TYPE_BINDING_MODE_INDICATOR,
	LAYOUT_BLOCK_TYPE_STATUS_LINE_I3BAR,
#if WITH_TRAY
	LAYOUT_BLOCK_TYPE_TRAY_SNI_ITEM,
	LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM,
#endif // WITH_TRAY
};

struct layout_block {
	sw_layout_block_t _; // must be first
	PAD32;

	enum layout_block_type type;
	union {
		void *data;
		struct workspace *workspace;
		struct status_line_i3bar_block *i3bar_block;
#if WITH_TRAY
		struct sni_item *tray_sni_item;
		struct sni_dbusmenu_menu_item *tray_menu_item;
#endif // WITH_TRAY
	};

	struct layout_block *block; // TODO: rename
};

struct binding {
	uint32_t event_code;
	bool32_t release;
	string_t command;
};

typedef struct binding struct_binding;
ARRAY_DECLARE_DEFINE(struct_binding)

struct config_box_colors {
	sw_color_t border;
	sw_color_t background;
	sw_color_t text;
};

#if WITH_TRAY
struct tray_dbusmenu_menu_popup {
	sw_wayland_surface_t _; // must be first
	struct sni_dbusmenu_menu *menu;
	sw_wayland_surface_t *parent;
	sw_wayland_seat_t *seat;

	struct layout_block *focused_block;
};

struct tray {
	bool32_t active;
	PAD32;
	struct tray_dbusmenu_menu_popup *popup;
#if SW_WITH_SVG || SW_WITH_PNG
	struct xdg_icon_theme_cache cache;
#endif // SW_WITH_SVG || SW_WITH_PNG
};

enum tray_binding_command {
	TRAY_BINDING_COMMAND_NONE,
	TRAY_BINDING_COMMAND_CONTEXT_MENU,
	TRAY_BINDING_COMMAND_ACTIVATE,
	TRAY_BINDING_COMMAND_SECONDARY_ACTIVATE,
	TRAY_BINDING_COMMAND_SCROLL_DOWN,
	TRAY_BINDING_COMMAND_SCROLL_LEFT,
	TRAY_BINDING_COMMAND_SCROLL_RIGHT,
	TRAY_BINDING_COMMAND_SCROLL_UP,
	TRAY_BINDING_COMMAND_NOP,
};

struct tray_binding {
	uint32_t event_code;
	enum tray_binding_command command;
};

typedef struct tray_binding struct_tray_binding;
ARRAY_DECLARE_DEFINE(struct_tray_binding)

#endif // WITH_TRAY

enum status_line_protocol {
	STATUS_LINE_PROTOCOL_UNDEF,
	STATUS_LINE_PROTOCOL_ERROR,
	STATUS_LINE_PROTOCOL_TEXT,
	STATUS_LINE_PROTOCOL_I3BAR,
};

struct status_line_i3bar_block {
	string_t name;
	string_t instance;
	string_t full_text;
	string_t short_text;
	bool32_t text_color_set;
	bool32_t urgent;
	bool32_t separator;
	bool32_t border_color_set;
	int32_t min_width;
	sw_color_t text_color;
	sw_color_t background_color;
	sw_color_t border_color;
	string_t min_width_str;
	sw_layout_block_content_anchor_t content_anchor;
	int32_t separator_block_width;
	union {
		struct {
			int32_t border_widths[4]; // left, right, bottom, top
		};
		struct {
			int32_t border_left;
			int32_t border_right;
			int32_t border_bottom;
			int32_t border_top;
		};
	};
};

typedef struct status_line_i3bar_block struct_status_line_i3bar_block;
ARRAY_DECLARE_DEFINE(struct_status_line_i3bar_block)

struct status_line {
	bool32_t active;

	int read_fd, write_fd;
	pid_t pid;

	enum status_line_protocol protocol;
	bool32_t click_events;
	bool32_t float_event_coords;

	enum su__json_tokener_state tokener_state;
	su_array__struct_status_line_i3bar_block__t blocks;

	int stop_signal;
	int cont_signal;

	struct {
		uint8_t *data;
		size_t size;
		size_t idx;
	} buf;

	FILE *read;
};

enum config_hidden_state {
	CONFIG_HIDDEN_STATE_HIDE,
	CONFIG_HIDDEN_STATE_SHOW,
};

enum config_mode {
	CONFIG_MODE_DOCK,
	CONFIG_MODE_HIDE,
	CONFIG_MODE_INVISIBLE,
	CONFIG_MODE_OVERLAY,
};

enum poll_fd {
	POLL_FD_SW,
	POLL_FD_SWAY_IPC,
	POLL_FD_STATUS,
	POLL_FD_SNI_SERVER,
	POLL_FD_LAST,
};

static struct {
	sw_state_t sw;

	arena_t scratch_arena;
	struct pollfd poll_fds[POLL_FD_LAST];

	struct {
		enum config_mode mode;
		enum config_hidden_state hidden_state;
		string_t status_command;
		string_t font;
		int32_t gaps[4]; // top, right, bottom, left
		string_t separator_symbol;
		int32_t height;
		int32_t status_padding;
		int32_t status_edge_padding;
		bool32_t wrap_scroll;
		bool32_t workspace_buttons;
		bool32_t strip_workspace_numbers;
		bool32_t strip_workspace_name;
		int32_t workspace_min_width;
		bool32_t binding_mode_indicator;
		bool32_t pango_markup;
		uint32_t position; // enum sw_surface_layer_anchor |
		struct {
			sw_color_t background;
			sw_color_t statusline;
			sw_color_t separator;
			sw_color_t focused_background;
			sw_color_t focused_statusline;
			sw_color_t focused_separator;
			struct config_box_colors focused_workspace;
			struct config_box_colors inactive_workspace;
			struct config_box_colors active_workspace;
			struct config_box_colors urgent_workspace;
			struct config_box_colors binding_mode;
		} colors;
#if WITH_TRAY
		int32_t tray_padding;
		PAD32;
		su_array__su_string_t__t tray_outputs;
		su_array__struct_tray_binding__t tray_bindings;
		string_t tray_icon_theme;
#endif // WITH_TRAY
		su_array__struct_binding__t bindings;
		su_array__su_string_t__t outputs;
	} config;

	string_t binding_mode_indicator_text;
	struct status_line status;
#if WITH_TRAY
	struct tray tray;
#endif // WITH_TRAY

	string_t bar_id;
	bool32_t visible_by_urgency;
	bool32_t visible_by_modifier;
	bool32_t visible_by_mode;

	PAD32;

	bool32_t update;
	bool32_t running;
} state;

static void *gp_alloc_alloc(allocator_t *alloc, size_t size, size_t alignment) {
	ASSERT(size > 0);
	ASSERT((alignment != 0) && ((alignment == 1) || ((alignment % 2) == 0)));
	NOTUSED(alloc);
	void *ret = aligned_alloc(alignment, (size + alignment - 1) & ~(alignment - 1));
	if (UNLIKELY(!ret)) {
		abort(errno, "aligned_alloc: %s", strerror(errno));
	}
	ASSERT(((uintptr_t)ret % alignment) == 0);
	return ret;
}

static void gp_alloc_free(allocator_t *alloc, void *ptr) {
	NOTUSED(alloc);
	free(ptr);
}

static void *gp_alloc_realloc(allocator_t *alloc, void *ptr, size_t new_size, size_t new_alignment) {
	NOTUSED(alloc);
	NOTUSED(new_alignment);
	void *ret = realloc(ptr, new_size);// TODO: alignment
	if (UNLIKELY(!ret)) {
		abort(errno, "realloc: %s", strerror(errno));
	}
	return ret;
}

static allocator_t gp_alloc = {
	.alloc = gp_alloc_alloc,
	.free = gp_alloc_free,
	.realloc = gp_alloc_realloc,
};

static void *scratch_alloc_alloc(allocator_t *alloc, size_t size, size_t alignment) {
	NOTUSED(alloc);
//#if DEBUG
//	size_t c = state.scratch_arena.blocks.len;
//#endif // DEBUG
	void *ret = arena_alloc(&state.scratch_arena, &gp_alloc, size, alignment);
//#if DEBUG
//	if (c < state.scratch_arena.blocks.len) {
//		DEBUG_LOG("%s: new block size %zu", __func__,
//			su_array__su_arena_block_t__get(&state.scratch_arena.blocks, state.scratch_arena.blocks.len - 1).size);
//	}
//#endif // DEBUG
	return ret;
}

static void scratch_alloc_free(allocator_t *alloc, void *ptr) {
	NOTUSED(alloc); NOTUSED(ptr);
	//DEBUG_LOG("%s: %p (%zu)", __func__, ptr, arena_alloc_get_size(ptr));
}

static void *scratch_alloc_realloc(allocator_t *alloc, void *ptr, size_t new_size, size_t new_alignment) {
	NOTUSED(alloc);
//#if DEBUG
//	size_t c = state.scratch_arena.blocks.len;
//#endif // DEBUG
	void *ret = arena_alloc(&state.scratch_arena, &gp_alloc, new_size, new_alignment);
//#if DEBUG
//	if (c < state.scratch_arena.blocks.len) {
//		DEBUG_LOG("%s: new block size %zu", __func__,
//			su_array__su_arena_block_t__get(&state.scratch_arena.blocks, state.scratch_arena.blocks.len - 1).size);
//	}
//	//DEBUG_LOG("%s: %p -> %p (%zu -> %zu)", __func__, ptr, ret, ptr ? arena_alloc_get_size(ptr) : 0, new_size);
//#endif // DEBUG
	if (ptr) {
		memcpy(ret, ptr, MIN(new_size, arena_alloc_get_size(ptr)));
	}
	return ret;
}

static allocator_t scratch_alloc = {
	.alloc = scratch_alloc_alloc,
	.free = scratch_alloc_free,
	.realloc = scratch_alloc_realloc,
};

static void layout_block_destroy(struct layout_block *block) {
	// TODO: remove recursion

	switch (block->_.in.type) {
	case SW_LAYOUT_BLOCK_TYPE_TEXT:
		string_fini(&block->_.in.text.text, &gp_alloc);
		for (size_t i = 0; i < block->_.in.text.font_names.len; ++i) {
			string_fini(
				su_array__su_string_t__get_ptr(&block->_.in.text.font_names, i), &gp_alloc);
		}
		su_array__su_string_t__fini(&block->_.in.text.font_names, &gp_alloc);
		break;
	case SW_LAYOUT_BLOCK_TYPE_IMAGE:
		string_fini(&block->_.in.image.path, &gp_alloc);
		break;
	case SW_LAYOUT_BLOCK_TYPE_COMPOSITE:
		for (sw_layout_block_t *b = block->_.in.composite.children.head; b; ) {
			sw_layout_block_t *next = b->next;
			layout_block_destroy((struct layout_block *)b);
			b = next;
		}
		break;
	case SW_LAYOUT_BLOCK_TYPE_SPACER:
	default:
		break;
	}

	if (block->_.out.fini) {
		block->_.out.fini(&block->_, &state.sw);
	}

	gp_alloc.free(&gp_alloc, block);
}

#if DEBUG
static void ATTRIBUTE_NORETURN layout_block_handle_error(sw_layout_block_t *block, sw_state_t *sw) {
	NOTUSED(block); NOTUSED(sw);
	ASSERT_UNREACHABLE;
}
#endif // DEBUG

static struct layout_block *layout_block_create(void) {
	struct layout_block *block = gp_alloc.alloc(&gp_alloc, sizeof(*block), ALIGNOF(*block));
	*block = (struct layout_block){
		.type = LAYOUT_BLOCK_TYPE_DUMMY,
#if DEBUG
		._.in.error = layout_block_handle_error,
#endif // DEBUG
	};

	return block;
}

static void layout_block_init_text(sw_layout_block_t *block, string_t *text) {
	block->in.type = SW_LAYOUT_BLOCK_TYPE_TEXT;
	su_array__su_string_t__init(&block->in.text.font_names, &gp_alloc, 1);
	string_init_string(su_array__su_string_t__add_uninitialized(&block->in.text.font_names, &gp_alloc),
		&gp_alloc, state.config.font);
	if (text) {
		block->in.text.text = string_copy(*text);
	}
}

static void bar_update(struct bar *);

static void update_bars(void) {
	for (sw_wayland_surface_t *bar = state.sw.in.wayland.layers.head; bar; bar = bar->next) {
		bar_update((struct bar *)bar);
	}
} 

static bool32_t workspace_block_pointer_button(struct layout_block *block,
		struct bar *bar, uint32_t code, sw_wayland_pointer_button_state_t state_) {
	if (state_ == SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED) {
		return TRUE;
	}

	struct workspace *workspace = NULL;

	switch (code) {
	case BTN_LEFT:
		workspace = block->workspace;
		break;
	case KEY_SCROLL_DOWN:
	case KEY_SCROLL_RIGHT:
	case KEY_SCROLL_LEFT:
	case KEY_SCROLL_UP: {
		struct output *output = bar->output;
		for (size_t i = 0; i < output->workspaces.len; ++i) {
			workspace = su_array__struct_workspace__get_ptr(&output->workspaces, i);
			if (workspace->visible) {
				struct workspace *first_ws = su_array__struct_workspace__get_ptr(&output->workspaces, 0);
				struct workspace *last_ws = su_array__struct_workspace__get_ptr(
					&output->workspaces, output->workspaces.len - 1);
					bool32_t left = ((code == KEY_SCROLL_UP) || (code == KEY_SCROLL_LEFT));
				if (left) {
					if (workspace == first_ws) {
						workspace = state.config.wrap_scroll ? last_ws : NULL;
					} else {
						workspace = su_array__struct_workspace__get_ptr(&output->workspaces, i - 1);
					}
				} else {
					if (workspace == last_ws) {
						workspace = state.config.wrap_scroll ? first_ws : NULL;
					} else {
						workspace = su_array__struct_workspace__get_ptr(&output->workspaces, i + 1);
					}
				}
				break;
			}
		}
		break;
	}
	default:
		return FALSE;
	}

	if (!workspace || workspace->focused) {
		return TRUE;
	}

	string_t name = workspace->name;
	size_t len = STRING_LITERAL_LENGTH("workspace \"\"") + name.len;
	for (size_t i = 0; i < name.len; ++i) {
		if ((name.s[i] == '"') || (name.s[i] == '\\')) {
			len++;
		}
	}

	string_t payload = {
		.s = scratch_alloc.alloc(&scratch_alloc, len + 1, ALIGNOF(*payload.s)),
		.len = len,
		.free_contents = TRUE,
		.nul_terminated = FALSE,
	};

	memcpy(payload.s, "workspace \"", STRING_LITERAL_LENGTH("workspace \""));
	payload.s[len - 1] = '"';
	for (size_t i = 0, d = STRING_LITERAL_LENGTH("workspace \""); i < name.len; ++i) {
		if ((name.s[i] == '"') || (name.s[i] == '\\')) {
			payload.s[d++] = '\\';
		}
		payload.s[d++] = name.s[i];
	}

	if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_COMMAND, &payload) == -1) {
		abort(errno, "sway_ipc_send: write: %s", strerror(errno));
	}

	return TRUE;
}

static void workspace_fini(struct workspace *workspace) {
	string_fini(&workspace->name, &gp_alloc);
}

static void workspace_init(struct workspace *workspace, json_tokener_t *tok, struct output **output_out) {
	json_token_t token;
	json_tokener_state_t s;
	while (((s = json_tokener_next(tok, &scratch_alloc, &token))
			== SU_JSON_TOKENER_STATE_SUCCESS) && (tok->depth > 1)) {
		if ((tok->depth == 2) && (token.type == SU_JSON_TOKEN_TYPE_KEY)) {
			if (string_equal(token.s, STRING("urgent"))) {
				json_tokener_advance_assert_type(tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_BOOL);
				workspace->urgent = token.b;
			} else if (string_equal(token.s, STRING("name"))) {
				json_tokener_advance_assert_type(tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
				string_init_string(&workspace->name, &gp_alloc, token.s);
			} else if (string_equal(token.s, STRING("num"))) {
				json_tokener_advance_assert(tok, &scratch_alloc, &token);
				ASSERT((token.type == SU_JSON_TOKEN_TYPE_INT) || (token.type == SU_JSON_TOKEN_TYPE_UINT));
				workspace->num = (int32_t)token.i;
			} else if (string_equal(token.s, STRING("output"))) {
				json_tokener_advance_assert_type(tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
				for (struct output *output = (struct output *)state.sw.out.wayland.outputs.head;
						output;
						output = (struct output *)output->_.next) {
					if (string_equal(output->_.out.name, token.s)) {
						*output_out = output;
						break;
					}
				}
			} else if (string_equal(token.s, STRING("focused"))) {
				json_tokener_advance_assert_type(tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_BOOL);
				workspace->focused = token.b;
			} else if (string_equal(token.s, STRING("visible"))) {
				json_tokener_advance_assert_type(tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_BOOL);
				workspace->visible = token.b;
			}
		}
	}
	ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
}

#if DEBUG
static void ATTRIBUTE_NORETURN surface_handle_error(sw_wayland_surface_t *surface, sw_state_t *sw) {
	NOTUSED(surface); NOTUSED(sw);
	ASSERT_UNREACHABLE;
}
#endif // DEBUG

#if WITH_TRAY
static bool32_t tray_sni_item_block_handle_prepare(sw_layout_block_t *block, sw_state_t *sw) {
	NOTUSED(sw);
	int32_t v = ((struct layout_block *)block)->block->_.out.dim.content_height;
	block->in.min_width = v;
	block->in.min_height = v;
	return TRUE;
}

static bool32_t tray_sni_item_image_block_handle_prepare(sw_layout_block_t *block, sw_state_t *sw) {
	NOTUSED(sw);
	int32_t v = (((struct layout_block *)block)->block->_.out.dim.content_height - (state.config.tray_padding * 2));
	block->in.content_width = v;
	block->in.content_height = v;
	return TRUE;
}

static void tray_describe_sni_items(struct bar *bar) {
	bool32_t visible = (state.config.tray_outputs.len > 0) ? FALSE : TRUE;
	for (size_t i = 0; i < state.config.tray_outputs.len; ++i) {
		if (string_equal(bar->output->_.out.name, su_array__su_string_t__get(&state.config.tray_outputs, i))) {
			visible = TRUE;
			break;
		}
	}
	if (visible) {
		for (size_t i = 0; i < sni_server.out.host.items.len; ++i) {
			struct sni_item *item = su_array__struct_sni_item_ptr__get(&sni_server.out.host.items, i);

			struct layout_block *block = layout_block_create();
			block->type = LAYOUT_BLOCK_TYPE_TRAY_SNI_ITEM;
			block->tray_sni_item = item;
			//block->_.in.type = SW_LAYOUT_BLOCK_TYPE_SPACER;
			block->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
			block->block = (struct layout_block *)bar->_.in.root->in.composite.children.head;
			block->_.in.prepare = tray_sni_item_block_handle_prepare;

			su_llist__sw_layout_block_t__insert_tail(&bar->_.in.root->in.composite.children, &block->_);

			struct sni_item_properties *props = item->out.properties;
			if (!props) {
				return;
			}

			sw_layout_block_image_t icon = { 0 };
			string_t icon_name = { 0 };
			NOTUSED(icon_name);
			struct sni_item_pixmap *icon_pixmap = NULL;
			switch(props->status) {
			case SNI_ITEM_STATUS_ACTIVE:
				icon_name = props->icon_name;
				if (props->icon_pixmap.len > 0) {
					icon_pixmap = su_array__struct_sni_item_pixmap_ptr__get(&props->icon_pixmap, 0);
				}
				break;
			case SNI_ITEM_STATUS_NEEDS_ATTENTION:
				icon_name = props->attention_icon_name;
				if (props->attention_icon_pixmap.len > 0) {
					icon_pixmap = su_array__struct_sni_item_pixmap_ptr__get(&props->attention_icon_pixmap, 0);
				}
				break;
			case SNI_ITEM_STATUS_PASSIVE:
			case SNI_ITEM_STATUS_INVALID:
			default:
				break;
			}

#if SW_WITH_SVG || SW_WITH_PNG
			if (icon_name.len > 0) {
				if (props->icon_theme_path.len > 0) {
					xdg_icon_theme_cache_add_basedir(&state.tray.cache, &gp_alloc, props->icon_theme_path);
				}
				struct xdg_icon_theme_icon xdg_icon;
				if (xdg_icon_theme_cache_find_icon(&state.tray.cache, &xdg_icon,
						icon_name, state.config.tray_icon_theme.s ? &state.config.tray_icon_theme : NULL)) {
					string_init_string(&icon.path, &gp_alloc, xdg_icon.path);
					icon.type = (sw_layout_block_image_image_type_t)xdg_icon.type;
				}
			}
#endif // SW_WITH_SVG || SW_WITH_PNG

			if ((icon.path.len == 0) && icon_pixmap) {
				icon.format = SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_MEMORY;
				icon.type = SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SW_PIXMAP;
				icon.data = icon_pixmap;
				icon.len = (size_t)icon_pixmap->width * (size_t)icon_pixmap->height * 4 + sizeof(*icon_pixmap);
			}

			if (icon.type) {
				block->_.in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
				block->_.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
				block->_.in.expand = (SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM);

				struct layout_block *image = layout_block_create();
				image->_.in.type = SW_LAYOUT_BLOCK_TYPE_IMAGE;
				image->_.in.image = icon;
				image->block = (struct layout_block *)bar->_.in.root->in.composite.children.head;
				image->_.in.prepare = tray_sni_item_image_block_handle_prepare;

				su_llist__sw_layout_block_t__insert_tail(&block->_.in.composite.children, &image->_);
			}
		}
	}
}

static void tray_dbusmenu_menu_popup_destroy(struct tray_dbusmenu_menu_popup *popup) {
	// TODO: remove recursion

	sni_dbusmenu_menu_item_event(popup->menu->parent_menu_item,
		SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLOSED, TRUE);

	for (sw_wayland_surface_t *popup_ = popup->_.in.popups.head; popup_; ) {
		sw_wayland_surface_t *next = popup_->next;
		tray_dbusmenu_menu_popup_destroy((struct tray_dbusmenu_menu_popup *)popup_);
		popup_ = next;
	}

	if (popup->_.out.fini) {
		popup->_.out.fini(&popup->_, &state.sw);
	}

	layout_block_destroy((struct layout_block *)popup->_.in.root);

	state.update = TRUE;

	if (popup == state.tray.popup) {
		state.tray.popup = NULL;
	}

	gp_alloc.free(&gp_alloc, popup);
}

static struct tray_dbusmenu_menu_popup *tray_dbusmenu_menu_popup_create(struct sni_dbusmenu_menu *menu,
	int32_t x, int32_t y, sw_wayland_surface_t *parent, sw_wayland_seat_t *seat);

static void tray_dbusmenu_menu_item_pointer_enter(struct sni_dbusmenu_menu_item *menu_item,
		sw_layout_block_t *block, struct tray_dbusmenu_menu_popup *popup) {
	sni_dbusmenu_menu_item_event(menu_item,
		SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_HOVERED, TRUE);

	if (popup->_.in.popups.len > 0) {
		ASSERT(popup->_.in.popups.len == 1);
		tray_dbusmenu_menu_popup_destroy((struct tray_dbusmenu_menu_popup *)popup->_.in.popups.head);
		popup->_.in.popups = (su_llist__sw_wayland_surface_t__t){ 0 };
	}

	if (menu_item->enabled && (menu_item->type != SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR)) {
		if (menu_item->submenu && (menu_item->submenu->menu_items.len > 0)) {
			struct tray_dbusmenu_menu_popup *submenu_popup = tray_dbusmenu_menu_popup_create(
				menu_item->submenu, block->out.dim.x, block->out.dim.y + block->out.dim.height,
				&popup->_, popup->seat);
			su_llist__sw_wayland_surface_t__insert_tail(&popup->_.in.popups, &submenu_popup->_);
		}

		block->in.color = state.config.colors.focused_separator;

		state.update = TRUE;
	}
}

static void tray_dbusmenu_menu_item_pointer_button(struct sni_dbusmenu_menu_item *menu_item,
		uint32_t code, sw_wayland_pointer_button_state_t state_) {
	if ((code != BTN_LEFT) || (state_ == SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED)) {
		return;
	}

	if ((menu_item->type != SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR)
			&& menu_item->enabled) {
		sni_dbusmenu_menu_item_event(menu_item,
			SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLICKED, TRUE);

#if 1
		ASSERT(state.tray.popup->parent->in.popups.len == 1);
		state.tray.popup->parent->in.popups = (su_llist__sw_wayland_surface_t__t){ 0 };
		tray_dbusmenu_menu_popup_destroy(state.tray.popup);
#endif
	}
}

static void tray_dbusmenu_menu_item_pointer_leave(struct sni_dbusmenu_menu_item *menu_item,
		sw_layout_block_t *block) {
	if ((menu_item->type != SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR) && menu_item->enabled) {
		block->in.color = state.config.colors.focused_background;
		state.update = TRUE;
	}
}

static sw_color_t tray_dbusmenu_menu_item_get_text_color(struct sni_dbusmenu_menu_item *menu_item) {
	if (menu_item->enabled) {
		return state.config.colors.focused_statusline;
	} else {
		sw_color_t color = state.config.colors.focused_statusline;
		color.a >>= 1;
		return color;
	}
}

static void tray_dbusmenu_menu_popup_process_pointer_pos(struct tray_dbusmenu_menu_popup *popup,
		int32_t x, int32_t y) {
	sw_layout_block_t *test = popup->_.in.root->in.composite.children.head;
	if ((test->out.dim.width == 0) || (test->out.dim.height == 0)) {
		return;
	}

	for (sw_layout_block_t *block = popup->_.in.root->in.composite.children.head; block; block = block->next) {
		if ((x >= block->out.dim.x) && (y >= block->out.dim.y)
				&& (x < (block->out.dim.x + block->out.dim.width))
				&& (y < (block->out.dim.y + block->out.dim.height))) {
			if (popup->focused_block != (struct layout_block *)block) {
				if (popup->focused_block) {
					tray_dbusmenu_menu_item_pointer_leave(popup->focused_block->tray_menu_item, &popup->focused_block->_);
				}
				tray_dbusmenu_menu_item_pointer_enter(((struct layout_block *)block)->tray_menu_item, block, popup);
				popup->focused_block = (struct layout_block *)block;
			}
			return;
		}
	}

	if (popup->focused_block) {
		struct sni_dbusmenu_menu_item *menu_item = popup->focused_block->tray_menu_item;
		tray_dbusmenu_menu_item_pointer_leave(menu_item, &popup->focused_block->_);
		popup->focused_block = NULL;
	}
}

static void tray_dbusmenu_menu_popup_handle_pointer_enter(sw_wayland_pointer_t *pointer, sw_state_t *sw) {
	NOTUSED(sw);
	struct tray_dbusmenu_menu_popup *popup = (struct tray_dbusmenu_menu_popup *)pointer->out.focused_surface;
	if (popup->seat != pointer->out.seat) {
		return;
	}

	popup->focused_block = NULL;
	tray_dbusmenu_menu_popup_process_pointer_pos(popup, pointer->out.pos_x, pointer->out.pos_y);
}

static void tray_dbusmenu_menu_popup_handle_pointer_motion(sw_wayland_pointer_t *pointer, sw_state_t *sw) {
	NOTUSED(sw);
	struct tray_dbusmenu_menu_popup *popup = (struct tray_dbusmenu_menu_popup *)pointer->out.focused_surface;
	if (popup->seat != pointer->out.seat) {
		return;
	}

	tray_dbusmenu_menu_popup_process_pointer_pos(popup, pointer->out.pos_x, pointer->out.pos_y);
}

static void tray_dbusmenu_menu_popup_handle_pointer_leave(sw_wayland_pointer_t *pointer, sw_state_t *sw) {
	NOTUSED(sw);
	struct tray_dbusmenu_menu_popup *popup = (struct tray_dbusmenu_menu_popup *)pointer->out.focused_surface;
	if (popup->seat != pointer->out.seat) {
		return;
	}

	if (popup->focused_block) {
		struct sni_dbusmenu_menu_item *menu_item = popup->focused_block->tray_menu_item;
		tray_dbusmenu_menu_item_pointer_leave(menu_item, &popup->focused_block->_);
		popup->focused_block = NULL;
	}
}

static void tray_dbusmenu_menu_popup_handle_pointer_button(sw_wayland_pointer_t *pointer, sw_state_t *sw) {
	NOTUSED(sw);
	struct tray_dbusmenu_menu_popup *popup = (struct tray_dbusmenu_menu_popup *)pointer->out.focused_surface;
	if (popup->seat != pointer->out.seat) {
		return;
	}

	if (popup->focused_block) {
		struct sni_dbusmenu_menu_item *menu_item = popup->focused_block->tray_menu_item;
		tray_dbusmenu_menu_item_pointer_button(menu_item, pointer->out.btn_code, pointer->out.btn_state);
	}
}

static bool32_t tray_dbusmenu_menu_popup_icon_block_handle_prepare(sw_layout_block_t *block, sw_state_t *sw) {
	NOTUSED(sw);
	int32_t v = ((struct layout_block *)block)->block->_.out.dim.content_height;
	block->in.content_width = v;
	block->in.content_height = v;
	return TRUE;
}

static bool32_t tray_dbusmenu_menu_popup_decorator_block_handle_prepare(sw_layout_block_t *block, sw_state_t *sw) {
	NOTUSED(sw);
	int32_t v = (((struct layout_block *)block)->block->_.out.dim.content_height + (state.config.tray_padding * 2));
	block->in.min_width = v;
	block->in.max_width = v;
	block->in.min_height = v;
	block->in.max_height = v;
	return TRUE;
}

static void tray_dbusmenu_menu_popup_update(struct tray_dbusmenu_menu_popup *popup,
		struct sni_dbusmenu_menu *menu) {
	// TODO: update child popups
	if (popup->_.in.popups.len > 0) {
		ASSERT(popup->_.in.popups.len == 1);
		tray_dbusmenu_menu_popup_destroy((struct tray_dbusmenu_menu_popup *)popup->_.in.popups.head);
		popup->_.in.popups = (su_llist__sw_wayland_surface_t__t){ 0 };
	}

	for (sw_layout_block_t *block_ = popup->_.in.root->in.composite.children.head; block_;) {
		sw_layout_block_t *next = block_->next;
		layout_block_destroy((struct layout_block *)block_);
		block_ = next;
	}
	popup->_.in.root->in.composite.children = (su_llist__sw_layout_block_t__t){ 0 };
	popup->focused_block = NULL;
	popup->menu = menu;

	bool32_t needs_spacer = FALSE;

	for (size_t i = 0; i < menu->menu_items.len; ++i) {
		struct sni_dbusmenu_menu_item *menu_item = su_array__struct_sni_dbusmenu_menu_item__get_ptr(
			&menu->menu_items, i);
		if (!menu_item->visible) {
			continue;
		}

		struct layout_block *block = layout_block_create();
		block->tray_menu_item = menu_item;
		block->type = LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM;
		block->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_LEFT | SW_LAYOUT_BLOCK_EXPAND_RIGHT;
		if (menu_item->type == SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR) {
			//block->_.in.type = SW_LAYOUT_BLOCK_TYPE_SPACER;
			block->_.in.min_height = TRAY_DBUSMENU_SEPARATOR_WIDTH;
			block->_.in.color = state.config.colors.focused_separator;
		} else {
			block->_.in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;

			struct layout_block *label = NULL;
			if (menu_item->label.len > 0) {
				label = layout_block_create();
				layout_block_init_text(&label->_, &menu_item->label);
				label->_.in.text.color = tray_dbusmenu_menu_item_get_text_color(menu_item);
				label->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				if (state.config.tray_padding > 0) {
					label->_.in.border_top.in.width = state.config.tray_padding;
					label->_.in.border_bottom.in.width = state.config.tray_padding;
					label->_.in.border_left.in.width = state.config.tray_padding;
					label->_.in.border_right.in.width = state.config.tray_padding;
				}
				su_llist__sw_layout_block_t__insert_tail(&block->_.in.composite.children, &label->_);
			}

#if SW_WITH_SVG || SW_WITH_PNG
			if (menu_item->icon_name.len > 0) {
				struct sni_dbusmenu *dbusmenu = menu_item->parent_menu->dbusmenu;
				if (dbusmenu->item->out.properties && (dbusmenu->item->out.properties->icon_theme_path.len > 0)) {
					xdg_icon_theme_cache_add_basedir(&state.tray.cache, &gp_alloc,
						dbusmenu->item->out.properties->icon_theme_path);
				}
				if (dbusmenu->properties) {
					for (size_t j = 0; j < dbusmenu->properties->icon_theme_path.len; ++j) {
						xdg_icon_theme_cache_add_basedir(&state.tray.cache, &gp_alloc,
							su_array__su_string_t__get(&dbusmenu->properties->icon_theme_path, j));
					}
				}
				struct xdg_icon_theme_icon xdg_icon;
				if (xdg_icon_theme_cache_find_icon(&state.tray.cache, &xdg_icon, menu_item->icon_name,
						state.config.tray_icon_theme.s ? &state.config.tray_icon_theme : NULL)) {
					struct layout_block *icon = layout_block_create();
					if (state.config.tray_padding > 0) {
						icon->_.in.border_top.in.width = state.config.tray_padding;
						icon->_.in.border_bottom.in.width = state.config.tray_padding;
						icon->_.in.border_left.in.width = state.config.tray_padding;
						icon->_.in.border_right.in.width = state.config.tray_padding;
					}
					icon->_.in.type = SW_LAYOUT_BLOCK_TYPE_IMAGE;
					string_init_string(&icon->_.in.image.path, &gp_alloc, xdg_icon.path);
					icon->_.in.image.type = (sw_layout_block_image_image_type_t)xdg_icon.type;
					icon->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
					if (label) {
						icon->block = label;
						icon->_.in.prepare = tray_dbusmenu_menu_popup_icon_block_handle_prepare;
					}
					su_llist__sw_layout_block_t__insert_tail(&block->_.in.composite.children, &icon->_);
				}
			}
#endif // SW_WITH_SVG || SW_WITH_PNG

#if SW_WITH_PNG
			if (menu_item->icon_data.nbytes > 0) {
				struct layout_block *icon = layout_block_create();
				if (state.config.tray_padding > 0) {
					icon->_.in.border_top.in.width = state.config.tray_padding;
					icon->_.in.border_bottom.in.width = state.config.tray_padding;
					icon->_.in.border_left.in.width = state.config.tray_padding;
					icon->_.in.border_right.in.width = state.config.tray_padding;
				}
				icon->_.in.type = SW_LAYOUT_BLOCK_TYPE_IMAGE;
				icon->_.in.image.type = SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNG;
				icon->_.in.image.format = SW_LAYOUT_BLOCK_IMAGE_IMAGE_FORMAT_MEMORY;
				icon->_.in.image.data = menu_item->icon_data.bytes;
				icon->_.in.image.len = menu_item->icon_data.nbytes;
				icon->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				if (label) {
					icon->block = label;
					icon->_.in.prepare = tray_dbusmenu_menu_popup_icon_block_handle_prepare;
				}
				su_llist__sw_layout_block_t__insert_tail(&block->_.in.composite.children, &icon->_);
			}
#endif // SW_WITH_PNG

			switch (menu_item->toggle_type) {
			case SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_CHECKMARK:
			case SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_RADIO: {
				struct layout_block *toggle = layout_block_create();
				layout_block_init_text(&toggle->_, (menu_item->toggle_type == SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_RADIO)
						? ((menu_item->toggle_state == 1) ? &STRING("󰐾") : &STRING("󰄯"))
						: ((menu_item->toggle_state == 1) ? &STRING("󰄲") : &STRING("󰄮")));
				toggle->_.in.text.color = tray_dbusmenu_menu_item_get_text_color(menu_item);
				toggle->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				if (state.config.tray_padding > 0) {
					toggle->_.in.border_top.in.width = state.config.tray_padding;
					toggle->_.in.border_bottom.in.width = state.config.tray_padding;
					toggle->_.in.border_left.in.width = state.config.tray_padding;
					toggle->_.in.border_right.in.width = state.config.tray_padding;
				}
				if (label) {
					toggle->_.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
					toggle->block = label;
					toggle->_.in.prepare = tray_dbusmenu_menu_popup_decorator_block_handle_prepare;
				}
				su_llist__sw_layout_block_t__insert_tail(&block->_.in.composite.children, &toggle->_);
				needs_spacer = TRUE;
				break;
			}
			case SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_NONE:
			default:
				break;
			}

			if (menu_item->submenu) {
				struct layout_block *submenu = layout_block_create();
				layout_block_init_text(&submenu->_, &STRING("󰍞"));
				submenu->_.in.text.color = tray_dbusmenu_menu_item_get_text_color(menu_item);
				submenu->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				if (state.config.tray_padding > 0) {
					submenu->_.in.border_top.in.width = state.config.tray_padding;
					submenu->_.in.border_bottom.in.width = state.config.tray_padding;
					submenu->_.in.border_left.in.width = state.config.tray_padding;
					submenu->_.in.border_right.in.width = state.config.tray_padding;
				}
				if (label) {
					submenu->_.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
					submenu->block = label;
					submenu->_.in.prepare = tray_dbusmenu_menu_popup_decorator_block_handle_prepare;
				}
				su_llist__sw_layout_block_t__insert_tail(&block->_.in.composite.children, &submenu->_);
				needs_spacer = TRUE;
			}
		}
		su_llist__sw_layout_block_t__insert_tail(&popup->_.in.root->in.composite.children, &block->_);
	}

	if (needs_spacer) {
		for (sw_layout_block_t *block = popup->_.in.root->in.composite.children.head; block; block = block->next) {
			struct sni_dbusmenu_menu_item *menu_item = ((struct layout_block *)block)->data;
			if ((block->in.type == SW_LAYOUT_BLOCK_TYPE_COMPOSITE)
					&& (menu_item->toggle_type == SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_NONE)
					&& (menu_item->submenu == NULL)) {
				struct layout_block *b = (struct layout_block *)block->in.composite.children.head;
				if (b) {
					struct layout_block *spacer = layout_block_create();
					//spacer->_.in.type = SW_LAYOUT_BLOCK_TYPE_SPACER;
					spacer->block = b;
					spacer->_.in.prepare = tray_dbusmenu_menu_popup_decorator_block_handle_prepare;
					spacer->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
					su_llist__sw_layout_block_t__insert_tail(&block->in.composite.children, &spacer->_);
				}
			}
		}
	}

	state.update = TRUE;
}

static void tray_dbusmenu_menu_popup_destroy_sw(sw_wayland_surface_t *popup, sw_state_t *sw) {
	NOTUSED(sw);
	struct tray_dbusmenu_menu_popup *p = (struct tray_dbusmenu_menu_popup *)popup;
	su_llist__sw_wayland_surface_t__pop(&p->parent->in.popups, &p->_);
	tray_dbusmenu_menu_popup_destroy(p);
}

static struct tray_dbusmenu_menu_popup *tray_dbusmenu_menu_popup_create(struct sni_dbusmenu_menu *menu,
		int32_t x, int32_t y, sw_wayland_surface_t *parent, sw_wayland_seat_t *seat) {
	ASSERT(seat->out.pointer != NULL);

	sni_dbusmenu_menu_about_to_show(menu, TRUE);

	struct tray_dbusmenu_menu_popup *popup = gp_alloc.alloc(
		&gp_alloc, sizeof(*popup), ALIGNOF(*popup));
	*popup = (struct tray_dbusmenu_menu_popup){
		.parent = parent,
		.seat = seat,
		._.in = {
			.type = SW_WAYLAND_SURFACE_TYPE_POPUP,
			.popup.x = x,
			.popup.y = y,
			.height = -1,
			.width = -1,
			.destroy = tray_dbusmenu_menu_popup_destroy_sw,
			.root = (sw_layout_block_t *)layout_block_create(),
			.popup.grab = seat->out.pointer,
			.popup.gravity = SW_WAYLAND_SURFACE_POPUP_GRAVITY_TOP_LEFT,
			.popup.constraint_adjustment =
				SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_X
				| SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_Y,
			.enter = tray_dbusmenu_menu_popup_handle_pointer_enter,
			.motion = tray_dbusmenu_menu_popup_handle_pointer_motion,
			.leave = tray_dbusmenu_menu_popup_handle_pointer_leave,
			.button = tray_dbusmenu_menu_popup_handle_pointer_button,
#if DEBUG
			.error = surface_handle_error,
#endif // DEBUG
		},
	};

	popup->_.in.root->in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
	popup->_.in.root->in.composite.layout = SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_VERTICAL;
	popup->_.in.root->in.color = state.config.colors.focused_background;

	tray_dbusmenu_menu_popup_update(popup, menu);

	sni_dbusmenu_menu_item_event(menu->parent_menu_item,
		SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_OPENED, TRUE);

	return popup;
}

static void tray_sni_item_block_pointer_button(struct layout_block *block,
		struct bar *bar, uint32_t code, sw_wayland_pointer_button_state_t state_,
		int32_t x, int32_t y, sw_wayland_seat_t *seat) {
	if (state_ == SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED) {
		return;
	}

	struct sni_item *item = block->tray_sni_item;

	enum tray_binding_command command = TRAY_BINDING_COMMAND_NONE;
	for (size_t i = 0; i < state.config.tray_bindings.len; ++i) {
		struct tray_binding binding = su_array__struct_tray_binding__get(&state.config.tray_bindings, i);
		if (binding.event_code == code) {
			command = binding.command;
			break;
		}
	}
	if (command == TRAY_BINDING_COMMAND_NONE) {
		switch (code) {
		case BTN_LEFT:
			command = TRAY_BINDING_COMMAND_ACTIVATE;
			break;
		case BTN_MIDDLE:
			command = TRAY_BINDING_COMMAND_SECONDARY_ACTIVATE;
			break;
		case BTN_RIGHT:
			command = TRAY_BINDING_COMMAND_CONTEXT_MENU;
			break;
		case KEY_SCROLL_UP:
			command = TRAY_BINDING_COMMAND_SCROLL_UP;
			break;
		case KEY_SCROLL_DOWN:
			command = TRAY_BINDING_COMMAND_SCROLL_DOWN;
			break;
		case KEY_SCROLL_LEFT:
			command = TRAY_BINDING_COMMAND_SCROLL_LEFT;
			break;
		case KEY_SCROLL_RIGHT:
			command = TRAY_BINDING_COMMAND_SCROLL_RIGHT;
			break;
		default:
			command = TRAY_BINDING_COMMAND_NOP;
		}
	}

	if ((command == TRAY_BINDING_COMMAND_ACTIVATE) && item->out.properties
			&& item->out.properties->item_is_menu) {
		command = TRAY_BINDING_COMMAND_CONTEXT_MENU;
	}

	switch (command) {
	case TRAY_BINDING_COMMAND_CONTEXT_MENU: {
		struct sni_dbusmenu *dbusmenu = item->out.dbusmenu;
#define MENU su_array__struct_sni_dbusmenu_menu_item__get_ptr(&dbusmenu->menu->menu_items, 0)->submenu
		if (dbusmenu && dbusmenu->menu && (dbusmenu->menu->menu_items.len > 0)
				&& MENU && (MENU->menu_items.len > 0)) {
			state.tray.popup = tray_dbusmenu_menu_popup_create(MENU, x, y, &bar->_, seat);
			su_llist__sw_wayland_surface_t__insert_tail( &bar->_.in.popups, &state.tray.popup->_);
		} else {
			sni_item_context_menu_async(item, 0, 0);
		}
#undef MENU
		break;
	}
	case TRAY_BINDING_COMMAND_ACTIVATE:
		sni_item_activate_async(item, 0, 0);
		break;
	case TRAY_BINDING_COMMAND_SECONDARY_ACTIVATE:
		sni_item_secondary_activate_async(item, 0, 0);
		break;
	case TRAY_BINDING_COMMAND_SCROLL_DOWN:
		sni_item_scroll_async(item, 1, SNI_ITEM_SCROLL_ORIENTATION_VERTICAL);
		break;
	case TRAY_BINDING_COMMAND_SCROLL_LEFT:
		sni_item_scroll_async(item, -1, SNI_ITEM_SCROLL_ORIENTATION_HORIZONTAL);
		break;
	case TRAY_BINDING_COMMAND_SCROLL_RIGHT:
		sni_item_scroll_async(item, 1, SNI_ITEM_SCROLL_ORIENTATION_HORIZONTAL);
		break;
	case TRAY_BINDING_COMMAND_SCROLL_UP:
		sni_item_scroll_async(item, -1, SNI_ITEM_SCROLL_ORIENTATION_VERTICAL);
		break;
	case TRAY_BINDING_COMMAND_NONE:
	case TRAY_BINDING_COMMAND_NOP:
	default:
		break;
	}
}

static void tray_sni_item_properties_updated_sni_server(struct sni_item *item) {
	NOTUSED(item);
	update_bars();
}

static void tray_sni_item_dbusmenu_menu_updated_sni_server(struct sni_item *item) {
	if ((state.tray.popup == NULL) || (state.tray.popup->menu->dbusmenu->item != item)) {
		return;
	}

	struct sni_dbusmenu_menu *menu = item->out.dbusmenu->menu;
#define MENU su_array__struct_sni_dbusmenu_menu_item__get_ptr(&menu->menu_items, 0)->submenu
	if (menu && (menu->menu_items.len > 0) && MENU && (MENU->menu_items.len > 0)) {
		tray_dbusmenu_menu_popup_update(state.tray.popup, MENU);
	} else {
		ASSERT(state.tray.popup->parent->in.popups.len == 1);
		state.tray.popup->parent->in.popups = (su_llist__sw_wayland_surface_t__t){ 0 };
		tray_dbusmenu_menu_popup_destroy(state.tray.popup);
	}
#undef MENU
}

static void tray_sni_item_destroy_sni_server(struct sni_item *item) {
	if (state.tray.popup && (state.tray.popup->menu->dbusmenu->item == item)) {
		ASSERT(state.tray.popup->parent->in.popups.len == 1);
		state.tray.popup->parent->in.popups = (su_llist__sw_wayland_surface_t__t){ 0 };
		tray_dbusmenu_menu_popup_destroy(state.tray.popup);
	}

	gp_alloc.free(&gp_alloc, item);

	update_bars();
}

static struct sni_item *tray_sni_item_create_sni_server(void) {
	struct sni_item *item = gp_alloc.alloc(&gp_alloc, sizeof(*item), ALIGNOF(*item));
	*item = (struct sni_item){
		.in = {
			.destroy = tray_sni_item_destroy_sni_server,
			.properties_updated = tray_sni_item_properties_updated_sni_server,
			.dbusmenu_menu_updated = tray_sni_item_dbusmenu_menu_updated_sni_server,
		},
	};

	return item;
}

static void tray_init(void) {
	sni_server.in.alloc = &gp_alloc;
	sni_server.in.item_create = tray_sni_item_create_sni_server;

	int ret = sni_server_init();
	if (ret < 0) {
		abort(-ret, "sni_server_init: %s", strerror(-ret));
	}

#if SW_WITH_SVG || SW_WITH_PNG
	xdg_icon_theme_cache_init(&state.tray.cache, &gp_alloc);
#endif // SW_WITH_SVG || SW_WITH_PNG

	state.tray.popup = NULL;
	state.tray.active = TRUE;
}

static void tray_fini(void) {
	sni_server_fini();

#if SW_WITH_SVG || SW_WITH_PNG
	xdg_icon_theme_cache_fini(&state.tray.cache, &gp_alloc);
#endif // SW_WITH_SVG || SW_WITH_PNG

	state.tray = (struct tray){ 0 };
	state.poll_fds[POLL_FD_SNI_SERVER].fd = -1;
}

static void tray_update(void) {
#if SW_WITH_SVG || SW_WITH_PNG
	xdg_icon_theme_cache_fini(&state.tray.cache, &gp_alloc);
	xdg_icon_theme_cache_init(&state.tray.cache, &gp_alloc);
#endif // SW_WITH_SVG || SW_WITH_PNG
}
#endif // WITH_TRAY

static void bar_destroy(struct bar *bar) {
	if (state.status.active && (state.sw.in.wayland.layers.len == 0)) {
		kill(-state.status.pid, state.status.stop_signal);
	}

#if WITH_TRAY
	for (sw_wayland_surface_t *popup_ = bar->_.in.popups.head; popup_; ) {
		sw_wayland_surface_t *next = popup_->next;
		tray_dbusmenu_menu_popup_destroy((struct tray_dbusmenu_menu_popup *)popup_);
		popup_ = next;
	}
#endif // WITH_TRAY

	if (bar->_.out.fini) {
		bar->_.out.fini(&bar->_, &state.sw);
	}

	layout_block_destroy((struct layout_block *)bar->_.in.root);

	state.update = TRUE;

	gp_alloc.free(&gp_alloc, bar);
}

static void output_destroy(struct output *output) {
	for (size_t i = 0; i < output->workspaces.len; ++i) {
		workspace_fini( su_array__struct_workspace__get_ptr(&output->workspaces, i));
	}
	su_array__struct_workspace__fini(&output->workspaces, &gp_alloc);

	gp_alloc.free(&gp_alloc, output);
}

static void output_destroy_sw(sw_wayland_output_t *output, sw_state_t *sw) {
	NOTUSED(sw);
	output_destroy((struct output *)output);
}

static struct output *output_create(void) {
	if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_GET_WORKSPACES, NULL) == -1) {
		abort(errno, "sway_ipc_send: write: %s", strerror(errno));
	}

	struct output *output = gp_alloc.alloc(&gp_alloc, sizeof(*output), ALIGNOF(*output));
	*output = (struct output) {
		._.in.destroy = output_destroy_sw,
		.focused = FALSE,
	};

	su_array__struct_workspace__init(&output->workspaces, &gp_alloc, 32);

	return output;
}

static sw_wayland_output_t *output_create_sw(sw_state_t *sw) {
	NOTUSED(sw);
	struct output *output = output_create();
	return &output->_;
}

static void pointer_destroy_sw(sw_wayland_pointer_t *pointer, sw_state_t *sw) {
	NOTUSED(sw);
	gp_alloc.free(&gp_alloc, pointer);
}

static sw_wayland_pointer_t *pointer_create_sw(sw_state_t *sw) {
	NOTUSED(sw);
	sw_wayland_pointer_t *pointer = gp_alloc.alloc(&gp_alloc, sizeof(*pointer), ALIGNOF(*pointer));
	*pointer = (sw_wayland_pointer_t){
		.in.destroy = pointer_destroy_sw,
	};
	return pointer;
}

static void seat_destroy_sw(sw_wayland_seat_t *seat, sw_state_t *sw) {
	NOTUSED(sw);
	gp_alloc.free(&gp_alloc, seat);
}

static sw_wayland_seat_t *seat_create_sw(sw_state_t *sw) {
	NOTUSED(sw);
	sw_wayland_seat_t *seat = gp_alloc.alloc(&gp_alloc, sizeof(*seat), ALIGNOF(*seat));
	*seat = (sw_wayland_seat_t){
		.in = {
			.destroy = seat_destroy_sw,
			.pointer_create = pointer_create_sw,
		},
	};
	return seat;
}

static void status_line_init(void) {
	ASSERT(state.config.status_command.nul_terminated);

	int pipe_read_fd[2];
	int pipe_write_fd[2];
	if ((pipe(pipe_read_fd) == -1) || (pipe(pipe_write_fd) == -1)) {
		abort(errno, "pipe: %s", strerror(errno));
	}

	pid_t pid = fork();
	if (pid == -1) {
		abort(errno, "fork: %s", strerror(errno));
	} else if (pid == 0) {
		setpgid(0, 0);

		dup2(pipe_read_fd[1], STDOUT_FILENO);
		close(pipe_read_fd[0]);
		close(pipe_read_fd[1]);

		dup2(pipe_write_fd[0], STDIN_FILENO);
		close(pipe_write_fd[0]);
		close(pipe_write_fd[1]);

		char *cmd_[] = { "sh", "-c", state.config.status_command.s, NULL };
		execvp(cmd_[0], (char **)cmd_);
		exit(1);
	}

	if (!fd_set_nonblock(pipe_read_fd[0])) {
		abort(errno, "fcntl: %s", strerror(errno));
	}

	close(pipe_read_fd[1]);
	close(pipe_write_fd[0]);

	fd_set_cloexec(pipe_read_fd[0]);
	fd_set_cloexec(pipe_write_fd[1]);

	state.status = (struct status_line){
		.buf.size = 8192,
		.buf.data = gp_alloc.alloc(&gp_alloc, 8192, ALIGNOF(*state.status.buf.data)),
		.stop_signal = SIGSTOP,
		.cont_signal = SIGCONT,
		.pid = pid,
		.read_fd = pipe_read_fd[0],
		.write_fd = pipe_write_fd[1],
		.active = TRUE,
	};

	state.poll_fds[POLL_FD_STATUS].fd = pipe_read_fd[0];
}

static void status_line_close_pipes(void) {
	if (state.status.read_fd != -1) {
		close(state.status.read_fd);
		state.status.read_fd = -1;

		state.poll_fds[POLL_FD_STATUS].fd = -1;
	}
	if (state.status.write_fd != -1) {
		close(state.status.write_fd);
		state.status.write_fd = -1;
	}

	if (state.status.read) {
		fclose(state.status.read);
		state.status.read = NULL;
	}
}

static void status_line_i3bar_block_fini(struct status_line_i3bar_block *block);

static void status_line_fini(void) {
	status_line_close_pipes();
	kill(-state.status.pid, state.status.cont_signal);
	kill(-state.status.pid, SIGTERM);
	waitpid(state.status.pid, NULL, 0);

	gp_alloc.free(&gp_alloc, state.status.buf.data);

	if (state.status.protocol == STATUS_LINE_PROTOCOL_I3BAR) {
		for (size_t i = 0; i < state.status.blocks.len; ++i) {
			status_line_i3bar_block_fini(
				su_array__struct_status_line_i3bar_block__get_ptr(&state.status.blocks, i));
		}
		su_array__struct_status_line_i3bar_block__fini(&state.status.blocks, &gp_alloc);
	}

	state.status = (struct status_line){ 0 };
}

static void status_line_set_error(string_t text) {
	status_line_close_pipes();

	if (state.status.protocol == STATUS_LINE_PROTOCOL_I3BAR) {
		for (size_t i = 0; i < state.status.blocks.len; ++i) {
			status_line_i3bar_block_fini(
				su_array__struct_status_line_i3bar_block__get_ptr(&state.status.blocks, i));
		}
		su_array__struct_status_line_i3bar_block__fini(&state.status.blocks, &gp_alloc);
	}

	state.status.buf.data = gp_alloc.realloc(&gp_alloc,
		state.status.buf.data, text.len, ALIGNOF(*state.status.buf.data));
	memcpy(state.status.buf.data, text.s, text.len);
	state.status.buf.size = text.len;
	state.status.buf.idx = text.len;

	state.status.protocol = STATUS_LINE_PROTOCOL_ERROR;
}

static uint32_t to_x11_button(uint32_t code) {
	switch (code) {
	case BTN_LEFT:
		return 1;
	case BTN_MIDDLE:
		return 2;
	case BTN_RIGHT:
		return 3;
	case KEY_SCROLL_UP:
		return 4;
	case KEY_SCROLL_DOWN:
		return 5;
	case KEY_SCROLL_LEFT:
		return 6;
	case KEY_SCROLL_RIGHT:
		return 7;
	case BTN_SIDE:
		return 8;
	case BTN_EXTRA:
		return 9;
	default:
		return 0;
	}
}

static void status_line_i3bar_block_pointer_button(struct layout_block *block,
		struct bar *bar, uint32_t code, sw_wayland_pointer_button_state_t state_,
		int32_t x, int32_t y) {
	if ((state_ == SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED) || (state.status.write_fd == -1)) {
		return;
	}

	// TODO: scale

	json_writer_t writer;
	json_writer_init(&writer, &scratch_alloc, 1024);

	double rx = x - block->_.out.dim.x;
	double ry = y - block->_.out.dim.y;

	struct status_line_i3bar_block *i3bar_block = block->i3bar_block;

	json_writer_object_begin(&writer, &scratch_alloc);

	json_writer_object_key(&writer, &scratch_alloc, STRING("name"));
	json_writer_string_escape(&writer, &scratch_alloc, i3bar_block->name);

	if (i3bar_block->instance.len > 0) {
		json_writer_object_key(&writer, &scratch_alloc, STRING("instance"));
		json_writer_string_escape(&writer, &scratch_alloc, i3bar_block->instance);
	}

	json_writer_object_key(&writer, &scratch_alloc, STRING("button"));
	json_writer_uint(&writer, &scratch_alloc, to_x11_button(code));
	json_writer_object_key(&writer, &scratch_alloc, STRING("event"));
	json_writer_uint(&writer, &scratch_alloc, code);
	if (state.status.float_event_coords) {
		json_writer_object_key(&writer, &scratch_alloc, STRING("x"));
		json_writer_double(&writer, &scratch_alloc, x);
		json_writer_object_key(&writer, &scratch_alloc, STRING("y"));
		json_writer_double(&writer, &scratch_alloc, y);
		json_writer_object_key(&writer, &scratch_alloc, STRING("relative_x"));
		json_writer_double(&writer, &scratch_alloc, rx);
		json_writer_object_key(&writer, &scratch_alloc, STRING("relative_y"));
		json_writer_double(&writer, &scratch_alloc, ry);
		json_writer_object_key(&writer, &scratch_alloc, STRING("width"));
		json_writer_double(&writer, &scratch_alloc, (double)block->_.out.dim.width);
		json_writer_object_key(&writer, &scratch_alloc, STRING("height"));
		json_writer_double(&writer, &scratch_alloc, (double)block->_.out.dim.height);
	} else {
		json_writer_object_key(&writer, &scratch_alloc, STRING("x"));
		json_writer_int(&writer, &scratch_alloc, x);
		json_writer_object_key(&writer, &scratch_alloc, STRING("y"));
		json_writer_int(&writer, &scratch_alloc, y);
		json_writer_object_key(&writer, &scratch_alloc, STRING("relative_x"));
		json_writer_int(&writer, &scratch_alloc, (int64_t)rx);
		json_writer_object_key(&writer, &scratch_alloc, STRING("relative_y"));
		json_writer_int(&writer, &scratch_alloc, (int64_t)ry);
		json_writer_object_key(&writer, &scratch_alloc, STRING("width"));
		json_writer_int(&writer, &scratch_alloc, block->_.out.dim.width);
		json_writer_object_key(&writer, &scratch_alloc, STRING("height"));
		json_writer_int(&writer, &scratch_alloc, block->_.out.dim.height);
	}
	json_writer_object_key(&writer, &scratch_alloc, STRING("scale"));
	json_writer_int(&writer, &scratch_alloc, bar->_.out.scale);

	json_writer_object_end(&writer, &scratch_alloc);

	su__json_buffer_add_string(&writer.buf, &scratch_alloc, STRING(",\n"));

	if (write(state.status.write_fd, writer.buf.data, writer.buf.idx) == -1) {
		status_line_set_error(STRING("[failed to write click event]"));
		update_bars();
	}
}

static bool32_t status_line_i3bar_block_handle_prepare(sw_layout_block_t *block, sw_state_t *sw) {
	NOTUSED(sw);
	struct layout_block *b = (struct layout_block *)block;
	b->_.in.min_width = b->block->_.out.dim.content_width;
	return TRUE;
}

static void status_line_describe(struct bar *bar) {
	switch (state.status.protocol) {
	case STATUS_LINE_PROTOCOL_ERROR:
	case STATUS_LINE_PROTOCOL_TEXT:
		if (state.status.buf.idx > 0) {
			struct layout_block *block = layout_block_create();
			layout_block_init_text(&block->_, &(string_t){
				.s = (char *)state.status.buf.data,
				.len = state.status.buf.idx,
				.free_contents = FALSE,
				.nul_terminated = FALSE,
			});
			block->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
			block->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
			if (state.status.protocol == STATUS_LINE_PROTOCOL_TEXT) {
				block->_.in.text.color = bar->output->focused ?
					state.config.colors.focused_statusline : state.config.colors.statusline;
			} else {
				block->_.in.text.color = STATUS_ERROR_TEXT_COLOR;
			}
			block->_.in.border_left.in.width = STATUS_MARGIN_LEFT;
			block->_.in.border_right.in.width = STATUS_MARGIN_RIGHT;
			if (state.config.status_padding > 0) {
				block->_.in.border_top.in.width = state.config.status_padding;
				block->_.in.border_bottom.in.width = state.config.status_padding;
			}
			su_llist__sw_layout_block_t__insert_tail(&bar->_.in.root->in.composite.children, &block->_);
		}
		break;
	case STATUS_LINE_PROTOCOL_I3BAR: {
		bool32_t edge = (bar->_.in.root->in.composite.children.len == 1);

		// TODO: short_text

		for (size_t i = state.status.blocks.len - 1; i != SIZE_MAX; --i) {
			struct status_line_i3bar_block *i3bar_block = su_array__struct_status_line_i3bar_block__get_ptr(
				&state.status.blocks, i);
			struct layout_block *block = layout_block_create();
			if (state.status.click_events && (i3bar_block->name.len > 0)) {
				block->i3bar_block = i3bar_block;
				block->type = LAYOUT_BLOCK_TYPE_STATUS_LINE_I3BAR;
			}

			if (edge && (state.config.status_edge_padding > 0)) {
				struct layout_block *spacer = layout_block_create();
				//spacer->_.in.type = SW_LAYOUT_BLOCK_TYPE_SPACER;
				spacer->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				spacer->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
				spacer->_.in.min_width = state.config.status_edge_padding;
				su_llist__sw_layout_block_t__insert_tail(&bar->_.in.root->in.composite.children, &spacer->_);
			} else if (!edge && ((i3bar_block->separator_block_width > 0) || i3bar_block->separator)) {
				struct layout_block *separator = layout_block_create();
				separator->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				if (i3bar_block->separator) {
					if (state.config.separator_symbol.len > 0) {
						layout_block_init_text(&separator->_, &state.config.separator_symbol);
						separator->_.in.text.color = bar->output->focused ?
							state.config.colors.focused_separator : state.config.colors.separator;
						if (i3bar_block->separator_block_width > 0) {
							separator->_.in.min_width = i3bar_block->separator_block_width;
							separator->_.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
						}
					} else {
						//separator->_.in.type = SW_LAYOUT_BLOCK_TYPE_SPACER;
						int32_t width = MAX(i3bar_block->separator_block_width, STATUS_SEPARATOR_WIDTH);
						if (width > 0) {
							separator->_.in.min_width = width;
							separator->_.in.max_width = width;
						}
						separator->_.in.color = bar->output->focused ?
							state.config.colors.focused_separator : state.config.colors.separator;
						int32_t border = (i3bar_block->separator_block_width - STATUS_SEPARATOR_WIDTH) / 2;
						separator->_.in.border_left.in.width = border;
						if ((i3bar_block->separator_block_width - STATUS_SEPARATOR_WIDTH - border - border) == 1) {
							separator->_.in.border_right.in.width = border + 1;
						} else {
							separator->_.in.border_right.in.width = border;
						}
					}
				} else {
					//separator->_.type = SW_LAYOUT_BLOCK_TYPE_SPACER;
					if (i3bar_block->separator_block_width > 0) {
						separator->_.in.min_width = i3bar_block->separator_block_width;
					}
				}
				separator->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
				su_llist__sw_layout_block_t__insert_tail(&bar->_.in.root->in.composite.children, &separator->_);
			}

			block->_.in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
			block->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
			if (i3bar_block->min_width_str.len > 0) {
				struct layout_block *min_width = layout_block_create();
				layout_block_init_text(&min_width->_, &i3bar_block->min_width_str);
				min_width->_.in.text.color.u32 = 0;
				min_width->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_NONE;
				su_llist__sw_layout_block_t__insert_tail(&bar->_.in.root->in.composite.children, &min_width->_);

				block->block = min_width;
				block->_.in.prepare = status_line_i3bar_block_handle_prepare;
			} else if (i3bar_block->min_width > 0) {
				block->_.in.min_width = i3bar_block->min_width;
			}
			block->_.in.color = i3bar_block->urgent ?
				state.config.colors.urgent_workspace.background : i3bar_block->background_color;
			block->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
			block->_.in.content_anchor = i3bar_block->content_anchor;
			if (i3bar_block->border_color_set || i3bar_block->urgent) {
				for (size_t j = 0; j < LENGTH(block->_.in.borders); ++j) {
					if (i3bar_block->border_widths[j] > 0) {
						block->_.in.borders[j].in.width = i3bar_block->border_widths[j];
						block->_.in.borders[j].in.color = i3bar_block->urgent ?
							state.config.colors.urgent_workspace.border : i3bar_block->border_color;
					}
				}
			}

			struct layout_block *text = layout_block_create();
			layout_block_init_text(&text->_, &i3bar_block->full_text);
			text->_.in.text.color = i3bar_block->urgent ? state.config.colors.urgent_workspace.text :
				(i3bar_block->text_color_set ? i3bar_block->text_color :
				(bar->output->focused ? state.config.colors.focused_statusline : state.config.colors.statusline));
			text->_.in.border_left.in.width = STATUS_MARGIN_LEFT;
			text->_.in.border_right.in.width = STATUS_MARGIN_RIGHT;
			if (state.config.status_padding > 0) {
				text->_.in.border_top.in.width = state.config.status_padding;
				text->_.in.border_bottom.in.width = state.config.status_padding;
			}

			su_llist__sw_layout_block_t__insert_tail(&block->_.in.composite.children, &text->_);

			su_llist__sw_layout_block_t__insert_tail(&bar->_.in.root->in.composite.children, &block->_);

			edge = FALSE;
		}
		break;
	}
	case STATUS_LINE_PROTOCOL_UNDEF:
	default:
		break;
	}
}

static bool32_t parse_sway_color(string_t str, sw_color_t *dest) {
	ASSERT(str.nul_terminated); // TODO: remove

	if (str.len == 0) {
		return FALSE;
	}

	if (str.s[0] == '#') {
		str.s++;
		str.len--;
	}

	if ((str.len != 8) && (str.len != 6)) {
		return FALSE;
	}

	char *p;
	uint32_t rgba = (uint32_t)strtoul(str.s, &p, 16);
	if (*p != '\0') {
		return FALSE;
	}

	uint8_t a, r ,g ,b;
	if (str.len == 8) {
		a = (uint8_t)(rgba & 0xFF);
		b = (uint8_t)((rgba >> 8) & 0xFF);
		g = (uint8_t)((rgba >> 16) & 0xFF);
		r = (uint8_t)((rgba >> 24) & 0xFF);
	} else {
		a = 0xFF;
		b = (uint8_t)(rgba & 0xFF);
		g = (uint8_t)((rgba >> 8) & 0xFF);
		r = (uint8_t)((rgba >> 16) & 0xFF);
	}

	dest->u32 = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g<< 8) | ((uint32_t)b << 0);

	return TRUE;
}

static bool32_t status_line_i3bar_block_init(struct status_line_i3bar_block *block, json_ast_node_t *json) {
	if (json->type != SU_JSON_AST_NODE_TYPE_OBJECT) {
		return FALSE;
	}

	block->full_text = (string_t){ 0 };
	block->short_text = (string_t){ 0 };
	block->text_color_set = FALSE;
	block->text_color.u32 = 0;
	block->background_color.u32 = 0;
	block->border_color_set = FALSE;
	block->text_color.u32 = 0;
	block->border_top = 1;
	block->border_bottom = 1;
	block->border_left = 1;
	block->border_right = 1;
	block->min_width = 0;
	block->min_width_str = (string_t){ 0 };
	block->content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_LEFT_CENTER;
	block->name = (string_t){ 0 };
	block->instance = (string_t){ 0 };
	block->urgent = FALSE;
	block->separator = TRUE;
	block->separator_block_width = 9;

	// ? TODO: "_sw"

	for (size_t i = 0; i < json->object.len; ++i) {
		json_ast_key_value_t *key_value = su_array__su_json_ast_key_value_t__get_ptr(&json->object, i);
		if (string_equal(key_value->key, STRING("full_text"))) {
			if ((key_value->value.type == SU_JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->full_text, &gp_alloc, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING("short_text"))) {
			if ((key_value->value.type == SU_JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->short_text, &gp_alloc, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING("color"))) {
			if (key_value->value.type == SU_JSON_AST_NODE_TYPE_STRING) {
				block->text_color_set = parse_sway_color(
					key_value->value.s, &block->text_color);
			}
		} else if (string_equal(key_value->key, STRING("background"))) {
			if (key_value->value.type == SU_JSON_AST_NODE_TYPE_STRING) {
				parse_sway_color(
					key_value->value.s, &block->background_color);
			}
		} else if (string_equal(key_value->key, STRING("border"))) {
			if (key_value->value.type == SU_JSON_AST_NODE_TYPE_STRING) {
				block->border_color_set = parse_sway_color(
					key_value->value.s, &block->border_color);
			}
		} else if (string_equal(key_value->key, STRING("border_top"))) {
			if (key_value->value.type == SU_JSON_AST_NODE_TYPE_UINT) {
				block->border_top = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING("border_bottom"))) {
			if (key_value->value.type == SU_JSON_AST_NODE_TYPE_UINT) {
				block->border_bottom = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING("border_left"))) {
			if (key_value->value.type == SU_JSON_AST_NODE_TYPE_UINT) {
				block->border_left = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING("border_right"))) {
			if (key_value->value.type == SU_JSON_AST_NODE_TYPE_UINT) {
				block->border_right = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING("min_width"))) {
			if (key_value->value.type == SU_JSON_AST_NODE_TYPE_UINT) {
				block->min_width = (int32_t)key_value->value.u;
			} else if ((key_value->value.type == SU_JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(
					&block->min_width_str, &gp_alloc, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING("align"))) {
			if ((key_value->value.type == SU_JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				if (string_equal(key_value->value.s, STRING("center"))) {
					block->content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
				} else if (string_equal(key_value->value.s, STRING("right"))) {
					block->content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_CENTER;
				}
			}
		} else if (string_equal(key_value->key, STRING("name"))) {
			if ((key_value->value.type == SU_JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->name, &gp_alloc, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING("instance"))) {
			if ((key_value->value.type == SU_JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->instance, &gp_alloc, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING("urgent"))) {
			if (key_value->value.type == SU_JSON_AST_NODE_TYPE_BOOL) {
				block->urgent = key_value->value.b;
			}
		} else if (string_equal(key_value->key, STRING("separator"))) {
			if (key_value->value.type == SU_JSON_AST_NODE_TYPE_BOOL) {
				block->separator = key_value->value.b;
			}
		} else if (string_equal(key_value->key, STRING("separator_block_width"))) {
			if (key_value->value.type == SU_JSON_AST_NODE_TYPE_UINT) {
				block->separator_block_width = (int32_t)key_value->value.u;
			}
		}
		// TODO: markup
	}

	if (block->full_text.len == 0) {
		status_line_i3bar_block_fini(block);
		return FALSE;
	}

	return TRUE;
}

static void status_line_i3bar_block_fini(struct status_line_i3bar_block *block) {
	string_fini(&block->full_text, &gp_alloc);
	string_fini(&block->short_text, &gp_alloc);
	string_fini(&block->min_width_str, &gp_alloc);
	string_fini(&block->name, &gp_alloc);
	string_fini(&block->instance, &gp_alloc);
}

static void status_line_i3bar_parse_json(json_ast_node_t *json) {
	if (json->type != SU_JSON_AST_NODE_TYPE_ARRAY) {
		return;
	}

	for (size_t i = 0; i < state.status.blocks.len; ++i) {
		status_line_i3bar_block_fini(
			su_array__struct_status_line_i3bar_block__get_ptr(&state.status.blocks, i));
	}
	state.status.blocks.len = 0;

	struct status_line_i3bar_block block;
	for (size_t i = 0; i < json->array.len; ++i) {
		if (status_line_i3bar_block_init(&block, su_array__su_json_ast_node_t__get_ptr(&json->array, i))) {
			su_array__struct_status_line_i3bar_block__add(
				&state.status.blocks, &gp_alloc, block);
		}
	}
}

static bool32_t status_line_i3bar_process(void) {
    for (;;) {
		ssize_t read_bytes = read(state.status.read_fd, &state.status.buf.data[state.status.buf.idx],
			state.status.buf.size - state.status.buf.idx);
		if (read_bytes <= 0) {
			if (read_bytes == 0) {
				errno = EPIPE;
			}
			if (errno == EAGAIN) {
				break;
			} else if (errno == EINTR) {
				continue;
			} else {
				status_line_set_error(STRING("[error reading from status command]"));
				return TRUE;
			}
		} else {
			state.status.buf.idx += (size_t)read_bytes;
			if (state.status.buf.idx == state.status.buf.size) {
				state.status.buf.size *= 2;
				state.status.buf.data = gp_alloc.realloc( &gp_alloc,
					state.status.buf.data, state.status.buf.size, ALIGNOF(*state.status.buf.data));
			}
		}
    }

	if (state.status.buf.idx == 0) {
		return FALSE;
	}

	json_tokener_t tok = { 0 };

	string_t str = {
		.s = (char *)state.status.buf.data,
		.len = state.status.buf.idx,
	};
	json_tokener_set_string(&tok, &scratch_alloc, str);

	if (state.status.tokener_state) {
		su_stack__enum_su__json_tokener_state__push(&tok.state,
			&scratch_alloc, state.status.tokener_state);
		tok.depth = 1;
	} else {
		json_token_t token;
		json_tokener_state_t s = json_tokener_next(&tok, &scratch_alloc, &token);
		if (s == SU_JSON_TOKENER_STATE_MORE_DATA_EXPECTED) {
			state.status.buf.idx = 0;
			return FALSE;
		} else if ((s != SU_JSON_TOKENER_STATE_SUCCESS) || (token.type != SU_JSON_TOKEN_TYPE_ARRAY_START)) {
			status_line_set_error(STRING("[invalid i3bar json]"));
			return TRUE;
		}
	}

	json_ast_t ast;
	json_ast_reset(&ast);
	for (;;) {
		json_tokener_state_t s = json_tokener_ast(&tok, &scratch_alloc, &ast, 1, TRUE);
		if ((tok.depth != 1) || (s == SU_JSON_TOKENER_STATE_ERROR) || (s == SU_JSON_TOKENER_STATE_EOF)) {
			status_line_set_error(STRING("[failed to parse i3bar json]"));
			return TRUE;
		} else if (s == SU_JSON_TOKENER_STATE_SUCCESS) {
			status_line_i3bar_parse_json(&ast.root);
			json_ast_reset(&ast);
		} else if (s == SU_JSON_TOKENER_STATE_MORE_DATA_EXPECTED) {
			break;
		}
	}

	state.status.tokener_state = su_stack__enum_su__json_tokener_state__get(&tok.state);

	state.status.buf.idx = 0;
	return TRUE;
}

static bool32_t status_line_process(void) {
	ssize_t read_bytes = 1;
	switch (state.status.protocol) {
	case STATUS_LINE_PROTOCOL_UNDEF:
		errno = 0;
		int available_bytes;
		if (ioctl(state.status.read_fd, FIONREAD, &available_bytes) == -1) {
			status_line_set_error(STRING("[error reading from status command]"));
			return TRUE;
		}

		if (((size_t)available_bytes + 1) > state.status.buf.size) {
			state.status.buf.size = (size_t)available_bytes + 1;
			state.status.buf.data = gp_alloc.realloc(&gp_alloc,
				state.status.buf.data, state.status.buf.size, ALIGNOF(*state.status.buf.data));
		}

		read_bytes = read(state.status.read_fd, state.status.buf.data, (size_t)available_bytes);
		if (read_bytes != available_bytes) {
			status_line_set_error(STRING("[error reading from status command]"));
			return TRUE;
		}

		string_t str = {
			.s = (char *)state.status.buf.data,
			.len = (size_t)read_bytes,
		};

		// TODO: handle properly
		string_t newline;
		if ((str.len > 0) && string_find_char(str, '\n', &newline)) {
			json_tokener_t tok = { 0 };
			json_tokener_set_string(&tok, &scratch_alloc, str);

			json_ast_t header;
			json_ast_reset(&header);

			bool32_t valid = FALSE;
			json_tokener_state_t s = json_tokener_ast(&tok, &scratch_alloc, &header, 0, TRUE);
			if ((s != SU_JSON_TOKENER_STATE_SUCCESS) || (header.root.type != SU_JSON_AST_NODE_TYPE_OBJECT)) {
				goto protocol_text;
			}

			for (size_t i = 0; i < header.root.object.len; ++i) {
				json_ast_key_value_t *key_value =
					su_array__su_json_ast_key_value_t__get_ptr(&header.root.object, i);
				if (string_equal(key_value->key, STRING("version"))) {
					if ((key_value->value.type != SU_JSON_AST_NODE_TYPE_UINT) || (key_value->value.u != 1)) {
						goto protocol_text;
					}
					valid = TRUE;
				} else if (string_equal(key_value->key, STRING("click_events"))) {
					if (key_value->value.type == SU_JSON_AST_NODE_TYPE_BOOL) {
						state.status.click_events = key_value->value.b;
					}
				} else if (string_equal(key_value->key, STRING("float_event_coords"))) {
					if (key_value->value.type == SU_JSON_AST_NODE_TYPE_BOOL) {
						state.status.float_event_coords = key_value->value.b;
					}
				} else if (string_equal(key_value->key, STRING("stop_signal"))) {
					if (key_value->value.type == SU_JSON_AST_NODE_TYPE_UINT) {
						state.status.stop_signal = (int)key_value->value.u;
					}
				} else if (string_equal(key_value->key, STRING("cont_signal"))) {
					if (key_value->value.type == SU_JSON_AST_NODE_TYPE_UINT) {
						state.status.cont_signal = (int)key_value->value.u;
					}
				}
			}
			if (!valid) {
				goto protocol_text;
			}

			state.status.protocol = STATUS_LINE_PROTOCOL_I3BAR;
			su_array__struct_status_line_i3bar_block__init(&state.status.blocks, &gp_alloc, 32);
			state.status.buf.idx = newline.len - 1;
			memmove(state.status.buf.data, newline.s + 1, state.status.buf.idx);

			if (state.status.click_events) {
				if (write(state.status.write_fd, "[\n", 2) != 2) {
					status_line_set_error(STRING("[failed to write to status command]"));
					return TRUE;
				}
			}

			return status_line_i3bar_process();
		}

protocol_text:

		state.status.read = fdopen(state.status.read_fd, "r");
		if (!state.status.read) {
			abort(errno, "fdopen: %s", strerror(errno));
		}

		state.status.protocol = STATUS_LINE_PROTOCOL_TEXT;
		state.status.buf.idx = (size_t)read_bytes;

		ATTRIBUTE_FALLTHROUGH;
	case STATUS_LINE_PROTOCOL_TEXT:
		for (;;) {
			if ((read_bytes > 0) && (state.status.buf.idx > 0) && (state.status.buf.data[read_bytes - 1] == '\n')) {
				state.status.buf.idx--;
			}
			errno = 0;
			read_bytes = getline((char **)&state.status.buf.data,
					&state.status.buf.size, state.status.read);
			if (read_bytes == -1) {
				if (errno && (errno != EAGAIN)) {
					status_line_set_error(STRING("[error reading from status command]"));
				} else {
					clearerr(state.status.read);
				}
				return TRUE;
			}
			state.status.buf.idx = (size_t)read_bytes;
		}
	case STATUS_LINE_PROTOCOL_I3BAR:
		return status_line_i3bar_process();
	case STATUS_LINE_PROTOCOL_ERROR:
	default:
		ASSERT_UNREACHABLE;
	}

	return FALSE;
}

static void workspaces_describe(struct bar *bar) {
	for (size_t i = 0; i < bar->output->workspaces.len; ++i) {
		struct workspace *workspace = su_array__struct_workspace__get_ptr(&bar->output->workspaces, i);
		struct config_box_colors colors;
		if (workspace->urgent) {
			colors = state.config.colors.urgent_workspace;
		} else if (workspace->focused) {
			colors = state.config.colors.focused_workspace;
		} else if (workspace->visible) {
			colors = state.config.colors.active_workspace;
		} else {
			colors = state.config.colors.inactive_workspace;
		}

		struct layout_block *block = layout_block_create();
		block->type = LAYOUT_BLOCK_TYPE_WORKSPACE;
		block->workspace = workspace;
		block->_.in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
		block->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
		if (state.config.workspace_min_width > 0) {
			block->_.in.min_width = state.config.workspace_min_width;
			block->_.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
		}
		block->_.in.color = colors.background;
		for (size_t j = 0; j < LENGTH(block->_.in.borders); ++j) {
			block->_.in.borders[j].in.width = WORKSPACE_BORDER_WIDTH;
			block->_.in.borders[j].in.color = colors.border;
		}

		struct layout_block *text = layout_block_create();
		layout_block_init_text(&text->_, NULL);
		text->_.in.text.color = colors.text;
		text->_.in.border_left.in.width = WORKSPACE_MARGIN_LEFT;
		text->_.in.border_right.in.width = WORKSPACE_MARGIN_RIGHT;
		text->_.in.border_bottom.in.width = WORKSPACE_MARGIN_BOTTOM;
		text->_.in.border_top.in.width = WORKSPACE_MARGIN_TOP;
		if (workspace->num != -1) {
			if (state.config.strip_workspace_name) {
				string_init_format(&text->_.in.text.text, &gp_alloc, "%d", workspace->num);
			} else if (state.config.strip_workspace_numbers) {
				int num_len = su_snprintf(NULL, 0, "%d", workspace->num);
				num_len += (workspace->name.s[num_len] == ':');
				if (workspace->name.s[num_len] != '\0') {
					text->_.in.text.text = (string_t){
						.s = workspace->name.s + num_len,
						.len = workspace->name.len - (size_t)num_len,
						.nul_terminated = TRUE,
						.free_contents = FALSE,
					};
				}
			}
		}
		if (text->_.in.text.text.len == 0) {
			text->_.in.text.text = string_copy(workspace->name);
		}

		su_llist__sw_layout_block_t__insert_tail(&block->_.in.composite.children, &text->_);

		su_llist__sw_layout_block_t__insert_tail(&bar->_.in.root->in.composite.children, &block->_);
	}
}

static void binding_mode_indicator_describe(struct bar *bar) {
	if (state.binding_mode_indicator_text.len == 0) {
		return;
	}

	struct layout_block *block = layout_block_create();
	block->type = LAYOUT_BLOCK_TYPE_BINDING_MODE_INDICATOR;
	block->_.in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
	block->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
	if (state.config.workspace_min_width > 0) {
		block->_.in.min_width = state.config.workspace_min_width;
	}
	block->_.in.color = state.config.colors.binding_mode.background;
	for (size_t i = 0; i < LENGTH(block->_.in.borders); ++i) {
		block->_.in.borders[i].in.width = BINDING_MODE_INDICATOR_BORDER_WIDTH;
		block->_.in.borders[i].in.color = state.config.colors.binding_mode.border;
	}

	struct layout_block *text = layout_block_create();
	layout_block_init_text(&text->_, &state.binding_mode_indicator_text);
	text->_.in.text.color = state.config.colors.binding_mode.text;
	text->_.in.border_left.in.width = BINDING_MODE_INDICATOR_MARGIN_LEFT;
	text->_.in.border_right.in.width = BINDING_MODE_INDICATOR_MARGIN_RIGHT;
	text->_.in.border_bottom.in.width = BINDING_MODE_INDICATOR_MARGIN_BOTTOM;
	text->_.in.border_top.in.width = BINDING_MODE_INDICATOR_MARGIN_TOP;

	su_llist__sw_layout_block_t__insert_tail(&block->_.in.composite.children, &text->_);

	su_llist__sw_layout_block_t__insert_tail(&bar->_.in.root->in.composite.children, &block->_);
}

static void bar_update(struct bar *bar) {
	for (sw_layout_block_t *block = bar->_.in.root->in.composite.children.head->next; block; ) {
		sw_layout_block_t *next = block->next;
		su_llist__sw_layout_block_t__pop(&bar->_.in.root->in.composite.children, block);
		layout_block_destroy((struct layout_block *)block);
		block = next;
	}

	bar->_.in.input_regions = (su_llist__sw_wayland_region_t__t){ 0 };
	bar->_.in.height = state.config.height;
	bar->_.in.layer.anchor = state.config.position;
	memcpy(bar->_.in.layer.margins, state.config.gaps, sizeof(bar->_.in.layer.margins));

	if ((state.config.mode == CONFIG_MODE_OVERLAY) || (state.config.mode == CONFIG_MODE_HIDE)) {
		bar->_.in.layer.layer = SW_WAYLAND_SURFACE_LAYER_LAYER_OVERLAY;
		bar->_.in.layer.exclusive_zone = -1;
		if (state.config.mode == CONFIG_MODE_OVERLAY) {
			static sw_wayland_region_t empty_input_region = { 0 };
			su_llist__sw_wayland_region_t__insert_tail(&bar->_.in.input_regions, &empty_input_region);
		}
	} else {
		bar->_.in.layer.layer = SW_WAYLAND_SURFACE_LAYER_LAYER_BOTTOM;
		bar->_.in.layer.exclusive_zone = INT_MIN;
	}

	bar->_.in.root->in.color = bar->output->focused ? state.config.colors.focused_background : state.config.colors.background;

#if WITH_TRAY
	if (state.tray.active) {
		tray_describe_sni_items(bar);
	}
#endif // WITH_TRAY

	if (state.status.active) {
		status_line_describe(bar);
	}
	if (state.config.workspace_buttons) {
		workspaces_describe(bar);
	}
	if (state.config.binding_mode_indicator) {
		binding_mode_indicator_describe(bar);
	}

	state.update = TRUE;
}

static void update_config(string_t str) {
	json_tokener_t tok = { 0 };
	json_tokener_set_string(&tok, &scratch_alloc, str);

	json_token_t token;
	json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_OBJECT_START);

	// ? TODO: handle different key order

	json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_KEY);
	if (string_equal(token.s, STRING("success"))) {
		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_BOOL);
		ASSERT(token.b == FALSE);
		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
		ASSERT(string_equal(token.s, STRING("error")));
		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
		abort(1, STRING_FMT, STRING_ARGS(token.s));
	}
	ASSERT(string_equal(token.s, STRING("id")));

	json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);

	if (!string_equal(state.bar_id, token.s)) {
		return;
	}

	string_t old_status_command = state.config.status_command, new_status_command = { 0 };

	state.config.mode = CONFIG_MODE_DOCK;
	state.config.hidden_state = CONFIG_HIDDEN_STATE_HIDE;
	state.config.position = (SW_WAYLAND_SURFACE_LAYER_ANCHOR_BOTTOM |
		SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT | SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT);
	state.config.status_command = (string_t){ 0 };
	string_fini(&state.config.font, &gp_alloc);
	state.config.font = (string_t){ 0 };
	memset(state.config.gaps, 0, sizeof(state.config.gaps));
	string_fini(&state.config.separator_symbol, &gp_alloc);
	state.config.separator_symbol = (string_t){ 0 };
	state.config.height = -1;
	state.config.status_padding = 1;
	state.config.status_edge_padding = 3;
	state.config.wrap_scroll = FALSE;
	state.config.workspace_buttons = TRUE;
	state.config.strip_workspace_numbers = FALSE;
	state.config.strip_workspace_name = FALSE;
	state.config.workspace_min_width = 0;
	state.config.binding_mode_indicator = TRUE;
	state.config.pango_markup = FALSE;
	state.config.colors.background.u32 = 0xFF000000;
	state.config.colors.statusline.u32 = 0xFFFFFFFF;
	state.config.colors.separator.u32 = 0xFF666666;
	state.config.colors.focused_background.u32 = 0xFF000000;
	state.config.colors.focused_statusline.u32 = 0xFFFFFFFF;
	state.config.colors.focused_separator.u32 = 0; // TODO: default
	state.config.colors.focused_workspace = (struct config_box_colors){
		.border.u32 = 0xFF4C7899,
		.background.u32 = 0xFF285577,
		.text.u32 = 0xFFFFFFFF,
	};
	state.config.colors.inactive_workspace = (struct config_box_colors){
		.border.u32 = 0xFF333333,
		.background.u32 = 0xFF222222,
		.text.u32 = 0xFF888888,
	};
	state.config.colors.active_workspace = (struct config_box_colors){
		.border.u32 = 0xFF333333,
		.background.u32 = 0xFF5F676A,
		.text.u32 = 0xFFFFFFFF,
	};
	state.config.colors.urgent_workspace = (struct config_box_colors){
		.border.u32 = 0xFF2F343A,
		.background.u32 = 0xFF900000,
		.text.u32 = 0xFFFFFFFF,
	};
	state.config.colors.binding_mode = (struct config_box_colors){
		.border.u32 = 0xFF2F343A,
		.background.u32 = 0xFF900000,
		.text.u32 = 0xFFFFFFFF,
	};
	for (size_t i = 0; i < state.config.bindings.len; ++i) {
		struct binding binding = su_array__struct_binding__get(&state.config.bindings, i);
		string_fini(&binding.command, &gp_alloc);
	}
	state.config.bindings.len = 0;
	for (size_t i = 0; i < state.config.outputs.len; ++i) {
		string_fini(su_array__su_string_t__get_ptr(&state.config.outputs, i), &gp_alloc);
	}
	state.config.outputs.len = 0;

#if WITH_TRAY
	bool32_t tray_enabled = TRUE;
	for (size_t i = 0; i < state.config.tray_outputs.len; ++i) {
		string_fini(su_array__su_string_t__get_ptr(&state.config.tray_outputs, i), &gp_alloc);
	}
	state.config.tray_outputs.len = 0;
	state.config.tray_bindings.len = 0;
	string_fini(&state.config.tray_icon_theme, &gp_alloc);
	state.config.tray_icon_theme = (string_t){ 0 };
	state.config.tray_padding = 2;
#endif // WITH_TRAY

	json_tokener_state_t s;
	while (((s = json_tokener_next(&tok, &scratch_alloc, &token))
			== SU_JSON_TOKENER_STATE_SUCCESS) && (tok.depth > 0)) {
		if ((tok.depth != 1) || (token.type != SU_JSON_TOKEN_TYPE_KEY)) {
			continue;
		}
		if (string_equal(token.s, STRING("mode"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
			if (string_equal(token.s, STRING("hide"))) {
				state.config.mode = CONFIG_MODE_HIDE;
			} else if (string_equal(token.s, STRING("invisible"))) {
				state.config.mode = CONFIG_MODE_INVISIBLE;
			} else if (string_equal(token.s, STRING("overlay"))) {
				state.config.mode = CONFIG_MODE_OVERLAY;
			}
		} else if (string_equal(token.s, STRING("hidden_state"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
			if (string_equal(token.s, STRING("show"))) {
				state.config.hidden_state = CONFIG_HIDDEN_STATE_SHOW;
			}
		} else if (string_equal(token.s, STRING("position"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
			if (string_equal(token.s, STRING("top"))) {
				state.config.position = (SW_WAYLAND_SURFACE_LAYER_ANCHOR_TOP |
					SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT | SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT);
			}
		} else if (string_equal(token.s, STRING("status_command"))) {
			json_tokener_advance_assert(&tok, &scratch_alloc, &token);
			if ((token.type == SU_JSON_TOKEN_TYPE_STRING) && (token.s.len > 0)) {
				string_init_len(&new_status_command, &gp_alloc, token.s.s, token.s.len, TRUE);
			}
		} else if (string_equal(token.s, STRING("font"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
			string_init_string(&state.config.font, &gp_alloc, token.s);
		} else if (string_equal(token.s, STRING("gaps"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_OBJECT_START);
			while (((s = json_tokener_next(&tok, &scratch_alloc, &token))
					== SU_JSON_TOKENER_STATE_SUCCESS) && (tok.depth > 1)) {
				if ((tok.depth == 2) && (token.type == SU_JSON_TOKEN_TYPE_KEY)) {
					if (string_equal(token.s, STRING("top"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_UINT);
						state.config.gaps[0] = (int32_t)token.i;
					} else if (string_equal(token.s, STRING("right"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_UINT);
						state.config.gaps[1] = (int32_t)token.i;
					} else if (string_equal(token.s, STRING("bottom"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_UINT);
						state.config.gaps[2] = (int32_t)token.i;
					} else if (string_equal(token.s, STRING("left"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_UINT);
						state.config.gaps[3] = (int32_t)token.i;
					}
				}
			}
			ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
		} else if (string_equal(token.s, STRING("separator_symbol"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
			string_init_string(&state.config.separator_symbol, &gp_alloc, token.s);
		} else if (string_equal(token.s, STRING("bar_height"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_UINT);
			state.config.height = (token.u == 0) ? -1 : (int32_t)token.u;
		} else if (string_equal(token.s, STRING("status_padding"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_UINT);
			state.config.status_padding = (int32_t)token.u;
		} else if (string_equal(token.s, STRING("status_edge_padding"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_UINT);
			state.config.status_edge_padding = (int32_t)token.u;
		} else if (string_equal(token.s, STRING("wrap_scroll"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_BOOL);
			state.config.wrap_scroll = token.b;
		} else if (string_equal(token.s, STRING("workspace_buttons"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_BOOL);
			state.config.workspace_buttons = token.b;
		} else if (string_equal(token.s, STRING("strip_workspace_numbers"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_BOOL);
			state.config.strip_workspace_numbers = token.b;
		} else if (string_equal(token.s, STRING("strip_workspace_name"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_BOOL);
			state.config.strip_workspace_name = token.b;
		} else if (string_equal(token.s, STRING("workspace_min_width"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_UINT);
			state.config.workspace_min_width = (int32_t)token.u;
		} else if (string_equal(token.s, STRING("binding_mode_indicator"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_BOOL);
			state.config.binding_mode_indicator = token.b;
		}
		// TODO: pango_markup
		else if (string_equal(token.s, STRING("colors"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_OBJECT_START);
			while (((s = json_tokener_next(&tok, &scratch_alloc, &token))
					== SU_JSON_TOKENER_STATE_SUCCESS) && (tok.depth > 1)) {
				if ((tok.depth == 2) && (token.type == SU_JSON_TOKEN_TYPE_KEY)) {
					if (string_equal(token.s, STRING("background"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.background);
					} else if (string_equal(token.s, STRING("statusline"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.statusline);
					} else if (string_equal(token.s, STRING("separator"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.separator);
					} else if (string_equal(token.s, STRING("focused_background"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.focused_background);
					} else if (string_equal(token.s, STRING("focused_statusline"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.focused_statusline);
					} else if (string_equal(token.s, STRING("focused_separator"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.focused_separator);
					} else if (string_equal(token.s, STRING("focused_workspace_text"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.focused_workspace.text);
					} else if (string_equal(token.s, STRING("focused_workspace_bg"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.focused_workspace.background);
					} else if (string_equal(token.s, STRING("focused_workspace_border"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.focused_workspace.border);
					} else if (string_equal(token.s, STRING("inactive_workspace_text"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.inactive_workspace.text);
					} else if (string_equal(token.s, STRING("inactive_workspace_bg"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.inactive_workspace.background);
					} else if (string_equal(token.s, STRING("inactive_workspace_border"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.inactive_workspace.border);
					} else if (string_equal(token.s, STRING("active_workspace_text"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.active_workspace.text);
					} else if (string_equal(token.s, STRING("active_workspace_bg"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.active_workspace.background);
					} else if (string_equal(token.s, STRING("active_workspace_border"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.active_workspace.border);
					} else if (string_equal(token.s, STRING("urgent_workspace_text"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.urgent_workspace.text);
					} else if (string_equal(token.s, STRING("urgent_workspace_bg"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.urgent_workspace.background);
					} else if (string_equal(token.s, STRING("urgent_workspace_border"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.urgent_workspace.border);
					} else if (string_equal(token.s, STRING("binding_mode_text"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.binding_mode.text);
					} else if (string_equal(token.s, STRING("binding_mode_bg"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.binding_mode.background);
					} else if (string_equal(token.s, STRING("binding_mode_border"))) {
						json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
						parse_sway_color(token.s, &state.config.colors.binding_mode.border);
					}
				}
			}
			ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
		} else if (string_equal(token.s, STRING("bindings"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_ARRAY_START);
			if (state.config.bindings.size == 0) {
				su_array__struct_binding__init(&state.config.bindings, &gp_alloc, 16);
			}
			while (((s = json_tokener_next(&tok, &scratch_alloc, &token))
					== SU_JSON_TOKENER_STATE_SUCCESS) && (tok.depth == 3)) {
				ASSERT(token.type == SU_JSON_TOKEN_TYPE_OBJECT_START);
				struct binding binding = { 0 };
				while (((s = json_tokener_next(&tok, &scratch_alloc, &token))
						== SU_JSON_TOKENER_STATE_SUCCESS) && (tok.depth > 2)) {
					if ((tok.depth == 3) && (token.type == SU_JSON_TOKEN_TYPE_KEY)) {
						if (string_equal(token.s, STRING("event_code"))) {
							json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_UINT);
							binding.event_code = (uint32_t)token.u;
						} else if (string_equal(token.s, STRING("command"))) {
							json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
							string_init_string(&binding.command, &gp_alloc, token.s);
						} else if (string_equal(token.s, STRING("release"))) {
							json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_BOOL);
							binding.release = token.b;
						}
					}
				}
				ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
				su_array__struct_binding__add(&state.config.bindings, &gp_alloc, binding);
			}
			ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
		} else if (string_equal(token.s, STRING("outputs"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_ARRAY_START);
			if (state.config.outputs.size == 0) {
				su_array__su_string_t__init(&state.config.outputs, &gp_alloc, 8);
			}
			bool32_t all_outputs = FALSE;
			while (((s = json_tokener_next(&tok, &scratch_alloc, &token))
					== SU_JSON_TOKENER_STATE_SUCCESS) && (tok.depth == 2)) {
				ASSERT(token.type == SU_JSON_TOKEN_TYPE_STRING);
				if (string_equal(token.s, STRING("*"))) {
					all_outputs = TRUE;
				} else if (!all_outputs) {
					string_init_string(
						su_array__su_string_t__add_uninitialized(&state.config.outputs, &gp_alloc),
						&gp_alloc, token.s);
				}
			}
			ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
			if (all_outputs) {
				for (size_t j = 0; j < state.config.outputs.len; ++j) {
					string_fini(
						su_array__su_string_t__get_ptr(&state.config.outputs, j), &gp_alloc);
				}
				state.config.outputs.len = 0;
			}
		}
#if WITH_TRAY
		else if (string_equal(token.s, STRING("tray_outputs"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_ARRAY_START);
			if (state.config.tray_outputs.size == 0) {
				su_array__su_string_t__init(&state.config.tray_outputs, &gp_alloc, 8);
			}
			while (((s = json_tokener_next(&tok, &scratch_alloc, &token)) == SU_JSON_TOKENER_STATE_SUCCESS)
					&& (tok.depth == 2)) {
				ASSERT(token.type == SU_JSON_TOKEN_TYPE_STRING);
				if (string_equal(token.s, STRING("none"))) {
					tray_enabled = FALSE;
				} else if (tray_enabled) {
					string_init_string(
						su_array__su_string_t__add_uninitialized(&state.config.tray_outputs, &gp_alloc),
						&gp_alloc, token.s);
				}
			}
			ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
			if (!tray_enabled) {
				for (size_t j = 0; j < state.config.tray_outputs.len; ++j) {
					string_fini(
						su_array__su_string_t__get_ptr(&state.config.tray_outputs, j), &gp_alloc);
				}
				state.config.tray_outputs.len = 0;
			}
		} else if (string_equal(token.s, STRING("tray_bindings"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_ARRAY_START);
			if (state.config.tray_bindings.size == 0) {
				su_array__struct_tray_binding__init(
					&state.config.tray_bindings, &gp_alloc, 16);
			}
			while (((s = json_tokener_next(&tok, &scratch_alloc, &token))
					== SU_JSON_TOKENER_STATE_SUCCESS) && (tok.depth == 3)) {
				ASSERT(token.type == SU_JSON_TOKEN_TYPE_OBJECT_START);
				struct tray_binding tray_binding = { 0 };
				while (((s = json_tokener_next(&tok, &scratch_alloc, &token))
						== SU_JSON_TOKENER_STATE_SUCCESS) && (tok.depth > 2)) {
					if ((tok.depth == 3) && (token.type == SU_JSON_TOKEN_TYPE_KEY)) {
						if (string_equal(token.s, STRING("event_code"))) {
							json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_UINT);
							tray_binding.event_code = (uint32_t)token.u;
						} else if (string_equal(token.s, STRING("command"))) {
							json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
							if (string_equal(token.s, STRING("ContextMenu"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_CONTEXT_MENU;
							} else if (string_equal(token.s, STRING("Activate"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_ACTIVATE;
							} else if (string_equal(token.s, STRING("SecondaryActivate"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_SECONDARY_ACTIVATE;
							} else if (string_equal(token.s, STRING("ScrollDown"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_SCROLL_DOWN;
							} else if (string_equal(token.s, STRING("ScrollLeft"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_SCROLL_LEFT;
							} else if (string_equal(token.s, STRING("ScrollRight"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_SCROLL_RIGHT;
							} else if (string_equal(token.s, STRING("ScrollUp"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_SCROLL_UP;
							} else {
								tray_binding.command = TRAY_BINDING_COMMAND_NOP;
							}
						}
					}
				}
				ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
				su_array__struct_tray_binding__add(&state.config.tray_bindings,
					&gp_alloc, tray_binding);
			}
			ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
		} else if (string_equal(token.s, STRING("icon_theme"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
			if (token.s.len > 0) {
				string_init_string(&state.config.tray_icon_theme, &gp_alloc, token.s);
			}
		} else if (string_equal(token.s, STRING("tray_padding"))) {
			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_UINT);
			state.config.tray_padding = (int32_t)token.u;
		}
#endif // WITH_TRAY
	}
	ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);

	if (state.status.active && ((new_status_command.len == 0) ||
			!string_equal(new_status_command, old_status_command))) {
		status_line_fini();
	}
	string_fini(&old_status_command, &gp_alloc);
	state.config.status_command = new_status_command;
	if (!state.status.active && (new_status_command.len > 0)) {
		status_line_init();
	}

#if WITH_TRAY
	if (tray_enabled && !state.tray.active) {
		tray_init();
	} else if (!tray_enabled && state.tray.active) {
		tray_fini();
	} else if (state.tray.active) {
		tray_update();
	}
#endif // WITH_TRAY

	if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_GET_BINDING_STATE, NULL) == -1) {
		abort(errno, "sway_ipc_send: write: %s", strerror(errno));
	}
}

static bool32_t bar_visible_on_output(sw_wayland_output_t *output) {
	bool32_t visible = !((state.config.mode == CONFIG_MODE_INVISIBLE)
		|| ((state.config.hidden_state == CONFIG_HIDDEN_STATE_HIDE) && (state.config.mode == CONFIG_MODE_HIDE)
		&& !state.visible_by_modifier && !state.visible_by_urgency && !state.visible_by_mode));
	if (visible && (state.config.outputs.len > 0)) {
		visible = FALSE;
		for (size_t i = 0; i < state.config.outputs.len; ++i) {
			if (string_equal(output->out.name, su_array__su_string_t__get(&state.config.outputs, i))) {
				visible = TRUE;
				break;
			}
		}
	}

	return visible;
}

static bool32_t bar_process_button_event(struct bar *bar,
		uint32_t code, sw_wayland_pointer_button_state_t state_,
		int32_t x, int32_t y, sw_wayland_seat_t *seat) {
	NOTUSED(seat);
#if WITH_TRAY
	if (state.tray.active && state.tray.popup && (state.tray.popup->seat == seat)) {
		if (state_ == SW_WAYLAND_POINTER_BUTTON_STATE_PRESSED) {
			ASSERT(state.tray.popup->parent->in.popups.len == 1);
			state.tray.popup->parent->in.popups = (su_llist__sw_wayland_surface_t__t){ 0 };
			tray_dbusmenu_menu_popup_destroy(state.tray.popup);
		}
		return TRUE;
	}
#endif // WITH_TRAY

	for (sw_layout_block_t *block = bar->_.in.root->in.composite.children.head; block; block = block->next) {
		if ((((struct layout_block *)block)->data)
				&& (x >= block->out.dim.x) && (y >= block->out.dim.y)
				&& (x < (block->out.dim.x + block->out.dim.width))
				&& (y < (block->out.dim.y + block->out.dim.height))) {
			switch (((struct layout_block *)block)->type) {
			case LAYOUT_BLOCK_TYPE_WORKSPACE:
				if (workspace_block_pointer_button((struct layout_block *)block, bar, code, state_)) {
					return TRUE;
				}
				break;
			case LAYOUT_BLOCK_TYPE_STATUS_LINE_I3BAR:
				status_line_i3bar_block_pointer_button((struct layout_block *)block, bar, code, state_, x, y);
				return TRUE;
#if WITH_TRAY
			case LAYOUT_BLOCK_TYPE_TRAY_SNI_ITEM:
				tray_sni_item_block_pointer_button((struct layout_block *)block, bar, code, state_, x, y, seat);
				return TRUE;
			case LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM:
#endif // WITH_TRAY
			case LAYOUT_BLOCK_TYPE_BINDING_MODE_INDICATOR:
			case LAYOUT_BLOCK_TYPE_DUMMY:
			default:
				ASSERT_UNREACHABLE;
			}
		}
	}

	bool32_t released = (state_ == SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED);
	for (size_t i = 0; i < state.config.bindings.len; ++i) {
		struct binding binding = su_array__struct_binding__get(&state.config.bindings, i);
		if ((binding.event_code == code) && (binding.release == released)) {
			if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_COMMAND, &binding.command) == -1) {
				abort(errno, "sway_ipc_send: write: %s", strerror(errno));
			}
			return TRUE;
		}
	}

	return FALSE;
}

static void bar_handle_pointer_button(sw_wayland_pointer_t *pointer, sw_state_t *sw) {
	NOTUSED(sw);
	bar_process_button_event((struct bar *)pointer->out.focused_surface,
		pointer->out.btn_code, pointer->out.btn_state,
		pointer->out.pos_x, pointer->out.pos_y, pointer->out.seat);
}

static void bar_handle_pointer_scroll(sw_wayland_pointer_t *pointer, sw_state_t *sw) {
	NOTUSED(sw);
	uint32_t button_code;
	bool32_t negative = pointer->out.scroll_vector_length < 0;
	switch (pointer->out.scroll_axis) {
	case SW_WAYLAND_POINTER_AXIS_VERTICAL_SCROLL:
		button_code = negative ? KEY_SCROLL_UP : KEY_SCROLL_DOWN;
		break;
	case SW_WAYLAND_POINTER_AXIS_HORIZONTAL_SCROLL:
		button_code = negative ? KEY_SCROLL_LEFT : KEY_SCROLL_RIGHT;
		break;
	default:
		ASSERT_UNREACHABLE;
	}

	struct bar *bar = (struct bar *)pointer->out.focused_surface;
	if (!bar_process_button_event(bar, button_code, SW_WAYLAND_POINTER_BUTTON_STATE_PRESSED,
				pointer->out.pos_x, pointer->out.pos_y, pointer->out.seat)) {
		bar_process_button_event(bar, button_code, SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED,
			pointer->out.pos_x, pointer->out.pos_y, pointer->out.seat);
	}
}

static void bar_destroy_sw(sw_wayland_surface_t *bar, sw_state_t *sw) {
	NOTUSED(sw);
	su_llist__sw_wayland_surface_t__pop(&state.sw.in.wayland.layers, bar);
	bar_destroy((struct bar *)bar);
}

static struct bar *bar_create(struct output *output) {
	if (state.status.active && (state.sw.in.wayland.layers.len == 0)) {
		kill(-state.status.pid, state.status.cont_signal);
	}

	struct bar *bar = gp_alloc.alloc(&gp_alloc, sizeof(*bar), ALIGNOF(*bar));
	*bar = (struct bar){
		.output = output,
		._.in = {
			.type = SW_WAYLAND_SURFACE_TYPE_LAYER,
			.layer.output = &output->_,
			.layer.exclusive_zone = INT_MIN,
			.layer.margin_top = -1,
			.layer.margin_right = -1,
			.layer.margin_bottom = -1,
			.layer.margin_left = -1,
			.root = (sw_layout_block_t *)layout_block_create(),
			.height = -1,
			.destroy = bar_destroy_sw,
			.button = bar_handle_pointer_button,
			.scroll = bar_handle_pointer_scroll,
#if DEBUG
			.error = surface_handle_error,
#endif // DEBUG
		},
	};

	bar->_.in.root->in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
	bar->_.in.root->in.expand = SW_LAYOUT_BLOCK_EXPAND_ALL_SIDES_CONTENT;

	struct layout_block *min_height = layout_block_create();
	layout_block_init_text(&min_height->_, &STRING(" "));
	min_height->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_NONE;
	su_llist__sw_layout_block_t__insert_tail(&bar->_.in.root->in.composite.children, &min_height->_);

	bar_update(bar);

	return bar;
}

static void process_ipc(void) {
	struct sway_ipc_response *response = sway_ipc_receive(
			state.poll_fds[POLL_FD_SWAY_IPC].fd, &scratch_alloc);
	if (response == NULL) {
		abort(errno, "sway_ipc_receive: read: %s", strerror(errno));
	}

	bool32_t update = FALSE;

	switch (response->type) {
	case SWAY_IPC_MESSAGE_TYPE_EVENT_WORKSPACE:
		if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_GET_WORKSPACES, NULL) == -1) {
			abort(errno, "sway_ipc_send: write: %s", strerror(errno));
		}
		break;
	case SWAY_IPC_MESSAGE_TYPE_GET_WORKSPACES: {
		for (struct output *output = (struct output *)state.sw.out.wayland.outputs.head;
				output;
				output = (struct output *)output->_.next) {
			output->focused = FALSE;
			for (size_t i = 0; i < output->workspaces.len; ++i) {
				workspace_fini(su_array__struct_workspace__get_ptr(&output->workspaces, i));
			}
			output->workspaces.len = 0;
		}

		state.visible_by_urgency = FALSE;
		update = TRUE;

		json_tokener_t tok = { 0 };
		json_tokener_set_string(&tok, &scratch_alloc, response->payload);

		json_token_t token;
		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_ARRAY_START);

		json_tokener_state_t s;
		while (((s = json_tokener_next(&tok, &scratch_alloc, &token))
				== SU_JSON_TOKENER_STATE_SUCCESS) && (tok.depth > 0)) {
			struct output *output = NULL;
			struct workspace workspace = { 0 };
			workspace_init(&workspace, &tok, &output);
			if (output) {
				su_array__struct_workspace__add(&output->workspaces, &gp_alloc, workspace);
				if (workspace.focused) {
					output->focused = TRUE;
				}
				if (workspace.urgent) {
					state.visible_by_urgency = TRUE;
				}
			} else {
				workspace_fini(&workspace);
			}
		}
		ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
		break;
	}
	case SWAY_IPC_MESSAGE_TYPE_EVENT_BAR_STATE_UPDATE: {
		json_tokener_t tok = { 0 };
		json_tokener_set_string(&tok, &scratch_alloc, response->payload);

		// ? TODO: handle different key order

		json_token_t token;
		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_OBJECT_START);

		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_KEY);
		ASSERT(string_equal(token.s, STRING("id")));

		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);
		if (string_equal(token.s, state.bar_id)) {

			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_KEY);
			ASSERT(string_equal(token.s, STRING("visible_by_modifier")));

			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_BOOL);
			state.visible_by_modifier = token.b;

			if (state.visible_by_modifier) {
				state.visible_by_mode = FALSE;
				state.visible_by_urgency = FALSE;
			}
			update = TRUE;
		}
		break;
	}
	case SWAY_IPC_MESSAGE_TYPE_EVENT_MODE:
	case SWAY_IPC_MESSAGE_TYPE_GET_BINDING_STATE: {
		json_tokener_t tok = { 0 };
		json_tokener_set_string(&tok, &scratch_alloc, response->payload);

		// ? TODO: handle different key order

		json_token_t token;
		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_OBJECT_START);

		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_KEY);
		ASSERT(string_equal(token.s, (response->type == SWAY_IPC_MESSAGE_TYPE_EVENT_MODE)
				? STRING("change") : STRING("name")));

		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, SU_JSON_TOKEN_TYPE_STRING);

		state.visible_by_mode = TRUE;
		update = TRUE;

		if ((token.s.len == 0) || string_equal(token.s, STRING("default"))) {
			string_fini(&state.binding_mode_indicator_text, &gp_alloc);
			state.binding_mode_indicator_text = (string_t){ 0 };
			state.visible_by_mode = FALSE;
		} else if (!string_equal(token.s, state.binding_mode_indicator_text)) {
			string_fini(&state.binding_mode_indicator_text, &gp_alloc);
			string_init_string(&state.binding_mode_indicator_text, &gp_alloc, token.s);
		}
		// TODO: pango_markup
		break;
	}
	case SWAY_IPC_MESSAGE_TYPE_EVENT_BARCONFIG_UPDATE:
		update_config(response->payload);
		update = TRUE;
		break;
	case SWAY_IPC_MESSAGE_TYPE_SUBSCRIBE:
	case SWAY_IPC_MESSAGE_TYPE_COMMAND:
	case SWAY_IPC_MESSAGE_TYPE_GET_OUTPUTS:
	case SWAY_IPC_MESSAGE_TYPE_GET_TREE:
	case SWAY_IPC_MESSAGE_TYPE_GET_MARKS:
	case SWAY_IPC_MESSAGE_TYPE_GET_BAR_CONFIG:
	case SWAY_IPC_MESSAGE_TYPE_GET_VERSION:
	case SWAY_IPC_MESSAGE_TYPE_GET_BINDING_MODES:
	case SWAY_IPC_MESSAGE_TYPE_GET_CONFIG:
	case SWAY_IPC_MESSAGE_TYPE_SEND_TICK:
	case SWAY_IPC_MESSAGE_TYPE_SYNC:
	case SWAY_IPC_MESSAGE_TYPE_GET_INPUTS:
	case SWAY_IPC_MESSAGE_TYPE_GET_SEATS:
	case SWAY_IPC_MESSAGE_TYPE_EVENT_OUTPUT:
	case SWAY_IPC_MESSAGE_TYPE_EVENT_WINDOW:
	case SWAY_IPC_MESSAGE_TYPE_EVENT_BINDING:
	case SWAY_IPC_MESSAGE_TYPE_EVENT_SHUTDOWN:
	case SWAY_IPC_MESSAGE_TYPE_EVENT_TICK:
	case SWAY_IPC_MESSAGE_TYPE_EVENT_INPUT:
	default:
		break;
	}

	if (update) {
		for (sw_wayland_output_t *output = state.sw.out.wayland.outputs.head; output; output = output->next) {
			if (bar_visible_on_output(output)) {
				struct bar *bar = NULL;
				for (sw_wayland_surface_t *bar_ = state.sw.in.wayland.layers.head; bar_; bar_ = bar_->next) {
					if (bar_->in.layer.output == output) {
						bar = (struct bar *)bar_;
						break;
					}
				}
				if (!bar) {
					bar = bar_create((struct output *)output);
					su_llist__sw_wayland_surface_t__insert_tail(&state.sw.in.wayland.layers, &bar->_);
				} else {
					bar_update(bar);
				}
			} else {
				for (sw_wayland_surface_t *bar = state.sw.in.wayland.layers.head; bar; bar = bar->next) {
					if (bar->in.layer.output == output) {
						su_llist__sw_wayland_surface_t__pop(&state.sw.in.wayland.layers, bar);
						bar_destroy((struct bar *)bar);
						break;
					}
				}
			}
		}
	}
}

static bool32_t init_sway_ipc(int fd) {
	state.poll_fds[POLL_FD_SWAY_IPC] = (struct pollfd){ .fd = fd, .events = POLLIN, };

	if (sway_ipc_send(fd, SWAY_IPC_MESSAGE_TYPE_GET_BAR_CONFIG, &state.bar_id) == -1) {
		return FALSE;
	}
	struct sway_ipc_response *response = sway_ipc_receive(fd, &scratch_alloc);
	if (response == NULL) {
		return FALSE;
	}
	ASSERT(response->type == SWAY_IPC_MESSAGE_TYPE_GET_BAR_CONFIG);
	update_config(response->payload);

	if (sway_ipc_send(fd, SWAY_IPC_MESSAGE_TYPE_SUBSCRIBE,
			&STRING("[\"barconfig_update\",\"bar_state_update\",\"mode\",\"workspace\"]")) == -1) {
		return FALSE;
	}

	return TRUE;
}

static void handle_signal(int sig) {
	NOTUSED(sig);
	state.running = FALSE;
}

static void setup(int argc, char **argv) {
	setlocale(LC_ALL, "");
	if (!locale_is_utf8()) {
		abort(1, "failed to set UTF-8 locale");
	}

	char sway_ipc_socket_path[PATH_MAX] = { 0 };

	static struct option long_options[] = {
		{ "help",    no_argument,       NULL, 'h' },
		{ "version", no_argument,       NULL, 'v' },
		{ "socket",  required_argument, NULL, 's' },
		{ "bar_id",  required_argument, NULL, 'b' },
		{ 0 },
	};
	
	int c;
	while ((c = getopt_long(argc, argv, "hvs:b:p:", long_options, NULL)) != -1) {
		switch (c) {
		case 's':
			strncpy(sway_ipc_socket_path, optarg, sizeof(sway_ipc_socket_path));
			break;
		case 'b':
			state.bar_id = (string_t){
				.s = optarg,
				.len = strlen(optarg),
				.free_contents = FALSE,
				.nul_terminated = TRUE,
			};
			break;
		case 'v':
			abort(0, "%s version", argv[0] ); // TODO
		default:
			abort(1, "Usage: %s [options...]\n"
				"\n"
				"  -h, --help             Show this help message and quit.\n"
				"  -v, --version          Show the version and quit.\n"
				"  -s, --socket <path>    Connect to sway via socket specified in <path>.\n"
				"  -b, --bar_id <id>      Bar ID for which to get the configuration.\n"
				"\n"
				" PLEASE NOTE that you can use swaybar_command field in your\n"
				" configuration file to let sway start %s automatically.\n"
				" You should never need to start it manually.\n", argv[0], argv[0]);
		}
	}

	if (state.bar_id.len == 0) {
		abort(1, "No bar_id passed. Provide --bar_id or use swaybar_command in sway config file");
	}

	arena_init(&state.scratch_arena, &gp_alloc, 16384);

	if (!*sway_ipc_socket_path) {
		if (!sway_ipc_get_socket_path(sway_ipc_socket_path)) {
			abort(ESOCKTNOSUPPORT, "Failed to get sway ipc socket path");
		}
	}
	int sway_ipc_fd = sway_ipc_connect(sway_ipc_socket_path);
	if (sway_ipc_fd == -1) {
		abort(errno, "Failed to connect to sway ipc socket '%s': %s", sway_ipc_socket_path, strerror(errno));
	}
    state.poll_fds[POLL_FD_STATUS] = (struct pollfd){ .fd = -1, .events = POLLIN, };
    state.poll_fds[POLL_FD_SNI_SERVER] = (struct pollfd){ .fd = -1, .events = 0, };
	if (!init_sway_ipc(sway_ipc_fd)) {
		abort(errno, "Failed to initialize sway ipc: %s", strerror(errno));
	}

	state.sw.in.gp_alloc = &gp_alloc;
	state.sw.in.scratch_alloc = &scratch_alloc;
	state.sw.in.wayland.output_create = output_create_sw;
	state.sw.in.wayland.seat_create = seat_create_sw;

	if (!sw_init(&state.sw)) {
		abort(errno, "sw_init: %s", strerror(errno));
	}

	state.poll_fds[POLL_FD_SW] = state.sw.out.wayland.pfd;

	static struct sigaction sigact = {
		.sa_handler = handle_signal,
	};
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGPIPE, &sigact, NULL);

	state.running = TRUE;
	state.update = TRUE;
}

static void run(void) {
	while (state.running) {
		if (state.update) {
			sw_set(&state.sw);
			state.update = FALSE;
		}
		if (!sw_flush(&state.sw)) {
			abort(errno, "sw_flush: %s", strerror(errno));
		}

		arena_reset(&state.scratch_arena, &gp_alloc);

		int tray_timeout = -1;
		bool32_t process_tray = FALSE;
#if WITH_TRAY
		if (state.tray.active) {
			int64_t absolute_timeout_ms;
			int ret = sni_server_get_poll_info(&state.poll_fds[POLL_FD_SNI_SERVER], &absolute_timeout_ms);
			if (ret < 0) {
				abort(-ret, "sni_server_get_poll_info: %s", strerror(-ret));
			}
			tray_timeout = (absolute_timeout_ms > 0) ? (int)(absolute_timeout_ms - now_ms()) : (int)absolute_timeout_ms;
		}
#endif // WITH_TRAY

		int sw_timeout = (state.sw.out.t > 0) ? (int)(state.sw.out.t - now_ms()) : (int)state.sw.out.t;

		int timeout = ((sw_timeout > 0) && (tray_timeout > 0))
			? MIN(sw_timeout, tray_timeout) : MAX(sw_timeout, tray_timeout);
		switch (poll(state.poll_fds, LENGTH(state.poll_fds), timeout)) {
		case -1:
			if (errno != EINTR) {
				abort(errno, "poll: %s", strerror(errno));
			}
			break;
		case 0:
			NOTUSED(process_tray);
			process_tray = (timeout == tray_timeout);
			state.update = (timeout == sw_timeout);
			break;
		default:
			break;
		}

		static short err = (POLLHUP | POLLERR | POLLNVAL);

		if (state.poll_fds[POLL_FD_SW].revents & (state.poll_fds[POLL_FD_SW].events | err)) {
			if (!sw_process(&state.sw)) {
				abort(errno, "sw_process: %s", strerror(errno));
			}
		}

		if (state.poll_fds[POLL_FD_SWAY_IPC].revents & (state.poll_fds[POLL_FD_SWAY_IPC].events | err)) {
			process_ipc();
		}

		if (state.status.active) {
			if (state.poll_fds[POLL_FD_STATUS].revents & (state.poll_fds[POLL_FD_STATUS].events | err)) {
				if (status_line_process()) {
					update_bars();
				}
			}
		}

#if WITH_TRAY
		if (state.tray.active && (process_tray || (state.poll_fds[POLL_FD_SNI_SERVER].revents
				& (state.poll_fds[POLL_FD_SNI_SERVER].events | err)))) {
			int ret = sni_server_process();
			if (ret < 0) {
				abort(-ret, "sni_server_process: %s", strerror(-ret));
			}
		}
#endif // WITH_TRAY
	}
}

static void cleanup(void) {
	if (state.status.active) {
		status_line_fini();
	}

#if WITH_TRAY
	if (state.tray.active) {
		tray_fini();
	}
#endif // WITH_TRAY

	close(state.poll_fds[POLL_FD_SWAY_IPC].fd);

#if DEBUG
	arena_fini(&state.scratch_arena, &gp_alloc);

	for (size_t i = 0; i < state.config.bindings.len; ++i) {
		struct binding binding = su_array__struct_binding__get(&state.config.bindings, i);
		string_fini(&binding.command, &gp_alloc);
	}
	su_array__struct_binding__fini(&state.config.bindings, &gp_alloc);
	string_fini(&state.config.font, &gp_alloc);
	for (size_t i = 0; i < state.config.outputs.len; ++i) {
		string_fini(
			su_array__su_string_t__get_ptr(&state.config.outputs, i), &gp_alloc);
	}
	su_array__su_string_t__fini(&state.config.outputs, &gp_alloc);

#if WITH_TRAY
	for (size_t i = 0; i < state.config.tray_outputs.len; ++i) {
		string_fini(
			su_array__su_string_t__get_ptr(&state.config.tray_outputs, i), &gp_alloc);
	}
	su_array__su_string_t__fini(&state.config.tray_outputs, &gp_alloc);
	su_array__struct_tray_binding__fini(&state.config.tray_bindings, &gp_alloc);
	string_fini(&state.config.tray_icon_theme, &gp_alloc);
#endif // WITH_TRAY

	string_fini(&state.config.status_command, &gp_alloc);
	string_fini(&state.config.separator_symbol, &gp_alloc);

	string_fini(&state.binding_mode_indicator_text, &gp_alloc);
	string_fini(&state.bar_id, &gp_alloc);
#endif // DEBUG

	sw_fini(&state.sw);
}

int main(int argc, char **argv) {
	setup(argc, argv);
	run();
	cleanup();

	return EXIT_SUCCESS;
}
