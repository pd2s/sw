#if !HAVE_TEXT
#error "swaybar example requires text blocks support to be enabled"
#endif /* HAVE_TEXT */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <time.h>
#include <assert.h>
#include <locale.h>

#include <linux/input-event-codes.h>
#define KEY_SCROLL_UP KEY_MAX + 1
#define KEY_SCROLL_DOWN KEY_MAX + 2
#define KEY_SCROLL_LEFT KEY_MAX + 3
#define KEY_SCROLL_RIGHT KEY_MAX + 4

#include "sw.h"
#include "sway-ipc.h"
#include "sw-json.h"
#include "json.h"

#include "macros.h"
#include "util.h"

#include "config.h"

#if HAVE_TRAY
#if HAVE_SVG || HAVE_PNG
#include "xdg-icon-theme.h"
#endif // HAVE_SVG || HAVE_PNG
#include "sni-server.h"
#endif // HAVE_TRAY

typedef struct json_token struct_json_token;
ARRAY_DECLARE_DEFINE(struct_json_token)

struct workspace {
	string_t name;
	int32_t num;
	bool32_t visible;
	bool32_t focused;
	bool32_t urgent;
	struct surface_block *block;
};

typedef struct workspace struct_workspace;
ARRAY_DECLARE_DEFINE(struct_workspace)

struct output {
	struct sw_json_output _; // must be first
	array_struct_workspace_t workspaces;
	bool32_t focused;
	uint32_t pad;
};

struct bar {
	struct sw_json_surface _; // must be first
	struct output *output;
	bool32_t dirty;
	bool32_t status_line_i3bar_use_short_text;
	bool32_t status_line_i3bar_test_short_text;
	uint32_t pad;
};

enum surface_block_type {
	SURFACE_BLOCK_TYPE_DUMMY,
	SURFACE_BLOCK_TYPE_WORKSPACE,
	SURFACE_BLOCK_TYPE_BINDING_MODE_INDICATOR,
	SURFACE_BLOCK_TYPE_STATUS_LINE_I3BAR,
#if HAVE_TRAY
	SURFACE_BLOCK_TYPE_TRAY_ITEM,
	SURFACE_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM,
#endif // HAVE_TRAY
};

struct surface_block {
	struct sw_json_surface_block _; // must be first
	uint32_t pad;

	enum surface_block_type type;
	union {
		void *data;
		struct workspace *workspace;
		struct status_line_i3bar_block *i3bar_block;
#if HAVE_TRAY
		struct tray_item *tray_item;
#endif // HAVE_TRAY
		struct sni_dbusmenu_menu_item *menu_item;
	};

	int cleanup_file;
	uint32_t ref_count;
};

struct binding {
	uint32_t event_code;
	bool32_t release;
	string_t command;
};

typedef struct binding struct_binding;
ARRAY_DECLARE_DEFINE(struct_binding)

struct config_box_colors {
	union sw_color border;
	union sw_color background;
	union sw_color text;
};

#if HAVE_TRAY
struct tray_dbusmenu_menu_popup {
	struct sw_json_surface _; // must be first
	struct sni_dbusmenu_menu *menu;
	struct bar *bar;
	struct sw_json_seat *seat;

	struct surface_block *focused_block;
};

struct tray {
	struct sni_server *sni;
	struct tray_dbusmenu_menu_popup *popup;

#if HAVE_SVG || HAVE_PNG
	struct xdg_icon_theme_cache cache;
#endif // HAVE_SVG || HAVE_PNG
};

struct tray_item {
	struct sni_item _; // must be first
	struct surface_block *block;
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
#endif // HAVE_TRAY

enum status_line_protocol {
	STATUS_LINE_PROTOCOL_UNDEF,
	STATUS_LINE_PROTOCOL_ERROR,
	STATUS_LINE_PROTOCOL_TEXT,
	STATUS_LINE_PROTOCOL_I3BAR,
};

enum status_line_i3bar_block_type {
	STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR,
	STATUS_LINE_I3BAR_BLOCK_TYPE_SW,
};

struct status_line_i3bar_block {
	string_t name;
	string_t instance;
	uint32_t pad;
	enum status_line_i3bar_block_type type;
	union {
		string_t json_raw; // sw

		struct { // i3bar
			string_t full_text;
			string_t short_text;
			bool32_t text_color_set;
			bool32_t urgent;
			bool32_t separator;
			bool32_t border_color_set;
			int32_t min_width;
			union sw_color text_color;
			union sw_color background_color;
			union sw_color border_color;
			string_t min_width_str;
			enum sw_surface_block_content_anchor content_anchor;
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
	};
};

typedef struct status_line_i3bar_block struct_status_line_i3bar_block;
ARRAY_DECLARE_DEFINE(struct_status_line_i3bar_block)

struct status_line {
	int read_fd, write_fd;
	pid_t pid;

	enum status_line_protocol protocol;
	bool32_t click_events;
	bool32_t float_event_coords;

	struct json_writer writer;
	struct json_tokener tokener;
	array_struct_status_line_i3bar_block_t blocks;

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
	POLL_FD_SW_READ,
	POLL_FD_SW_WRITE,
	POLL_FD_SWAY_IPC,
	POLL_FD_STATUS,
	POLL_FD_SNI_SERVER,
	POLL_FD_LAST,
};

static struct {
	struct pollfd poll_fds[POLL_FD_LAST];

	struct {
		enum config_mode mode;
		enum config_hidden_state hidden_state;
		uint32_t position; // enum sw_surface_layer_anchor |
		uint32_t pad;
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
		struct {
			union sw_color background;
			union sw_color statusline;
			union sw_color separator;
			union sw_color focused_background;
			union sw_color focused_statusline;
			union sw_color focused_separator;
			struct config_box_colors focused_workspace;
			struct config_box_colors inactive_workspace;
			struct config_box_colors active_workspace;
			struct config_box_colors urgent_workspace;
			struct config_box_colors binding_mode;
		} colors;
#if HAVE_TRAY
		int32_t tray_padding;
		array_string_t_t tray_outputs;
		array_struct_tray_binding_t tray_bindings;
		string_t tray_icon_theme;
#else
		uint32_t pad2;
#endif // HAVE_TRAY
		array_struct_binding_t bindings;
		array_string_t_t outputs;
	} config;

	struct {
		string_t text;
		struct surface_block *block;
	} binding_mode_indicator;
	struct status_line *status;
#if HAVE_TRAY
	struct tray *tray;
#endif // HAVE_TRAY

	struct sw_json_connection *sw;

	struct json_tokener tokener;

	string_t bar_id;
	bool32_t visible_by_urgency;
	bool32_t visible_by_modifier;
	bool32_t visible_by_mode;

	bool32_t running;
} state;

static struct surface_block *surface_block_ref(struct surface_block *block) {
	block->ref_count++;
	return block;
}

static void surface_block_unref(struct surface_block *block) {
	if (!block || (--block->ref_count > 0)) {
		return;
	}

	if (block->cleanup_file >= 0) {
		assert(block->_.type == SW_SURFACE_BLOCK_TYPE_IMAGE);
		assert(block->_.image.path.len > 0);
		remove(block->_.image.path.s);
		close(block->cleanup_file);
	}

	sw_json_surface_block_fini(&block->_);

	free(block);
}

static void surface_block_destroy_sw_json(struct sw_json_surface_block *block) {
	surface_block_unref((struct surface_block *)block);
}

static struct surface_block *surface_block_create(void) {
	struct surface_block *block = malloc(sizeof(struct surface_block));
	block->cleanup_file = -1;
	block->ref_count = 1;
	block->data = NULL;
	block->type = SURFACE_BLOCK_TYPE_DUMMY;

	sw_json_surface_block_init(&block->_, surface_block_destroy_sw_json);

	return block;
}

static void surface_block_init_text(struct sw_json_surface_block *block, string_t *text) {
	block->type = SW_SURFACE_BLOCK_TYPE_TEXT;
	array_string_t_init(&block->text.font_names, 1);
	array_string_t_add(&block->text.font_names, string_copy(state.config.font));
	if (text) {
		block->text.text = string_copy(*text);
	}
}

static bool32_t workspace_block_pointer_button(struct surface_block *block,
		struct bar *bar, uint32_t code, enum sw_pointer_button_state state_) {
	if (state_ == SW_POINTER_BUTTON_STATE_RELEASED) {
		return true;
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
			workspace = array_struct_workspace_get_ptr(&output->workspaces, i);
			if (workspace->visible) {
				struct workspace *first_ws = array_struct_workspace_get_ptr(&output->workspaces, 0);
				struct workspace *last_ws = array_struct_workspace_get_ptr(
					&output->workspaces, output->workspaces.len - 1);
					bool32_t left = ((code == KEY_SCROLL_UP) || (code == KEY_SCROLL_LEFT));
				if (left) {
					if (workspace == first_ws) {
						workspace = state.config.wrap_scroll ? last_ws : NULL;
					} else {
						workspace = array_struct_workspace_get_ptr(&output->workspaces, i - 1);
					}
				} else {
					if (workspace == last_ws) {
						workspace = state.config.wrap_scroll ? first_ws : NULL;
					} else {
						workspace = array_struct_workspace_get_ptr(&output->workspaces, i + 1);
					}
				}
				break;
			}
		}
		break;
	}
	default:
		return false;
	}

	if (!workspace || workspace->focused) {
		return true;
	}

	string_t name = workspace->name;
	size_t len = STRING_LITERAL_LENGTH("workspace \"\"") + name.len;
	for (size_t i = 0; i < name.len; ++i) {
		if ((name.s[i] == '"') || (name.s[i] == '\\')) {
			len++;
		}
	}

	string_t payload = {
		.s = malloc(len + 1),
		.len = len,
		.free_contents = true,
		.nul_terminated = false,
	};

	memcpy(payload.s, "workspace \"", STRING_LITERAL_LENGTH("workspace \""));
	payload.s[len - 1] = '"';
	for (size_t i = 0, d = STRING_LITERAL_LENGTH("workspace \""); i < name.len; ++i) {
		if ((name.s[i] == '"') || (name.s[i] == '\\')) {
			payload.s[d++] = '\\';
		}
		payload.s[d++] = name.s[i];
	}

	sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_COMMAND, &payload);
	string_fini(&payload);

	return true;
}

static void workspace_fini(struct workspace *workspace) {
	string_fini(&workspace->name);
	surface_block_unref(workspace->block);
}

static void workspace_init(struct workspace *workspace, struct output **output_out) {
	assert(arena_stack_enum_json_tokener_state_get(&state.tokener.state) == JSON_TOKENER_STATE_OBJECT_EMPTY);
	assert(state.tokener.depth == 2);

	struct json_token token;
	enum json_tokener_state_ state_;
	while (((state_ = json_tokener_next(&state.tokener, &token)) == JSON_TOKENER_STATE_SUCCESS)
			&& (state.tokener.depth > 1)) {
		if ((state.tokener.depth == 2) && (token.type == JSON_TOKEN_TYPE_KEY)) {
			if (string_equal(token.s, STRING_LITERAL("urgent"))) {
				JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_BOOL);
				workspace->urgent = token.b;
			} else if (string_equal(token.s, STRING_LITERAL("name"))) {
				JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
				string_init_string(&workspace->name, token.s);
			} else if (string_equal(token.s, STRING_LITERAL("num"))) {
				JSON_TOKENER_ADVANCE_ASSERT(state.tokener, token);
				assert((token.type == JSON_TOKEN_TYPE_INT) || (token.type == JSON_TOKEN_TYPE_UINT));
				workspace->num = (int32_t)token.i;
			} else if (string_equal(token.s, STRING_LITERAL("output"))) {
				JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
				for (struct output *output = (struct output *)state.sw->outputs.head;
						output;
						output = (struct output *)output->_.next) {
					if (string_equal(output->_.name, token.s)) {
						*output_out = output;
						break;
					}
				}
			} else if (string_equal(token.s, STRING_LITERAL("focused"))) {
				JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_BOOL);
				workspace->focused = token.b;
			} else if (string_equal(token.s, STRING_LITERAL("visible"))) {
				JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_BOOL);
				workspace->visible = token.b;
			}
		}
	}
	assert(state_ == JSON_TOKENER_STATE_SUCCESS);

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

	struct surface_block *block = surface_block_create();
	block->type = SURFACE_BLOCK_TYPE_WORKSPACE;
	block->_.type = SW_SURFACE_BLOCK_TYPE_COMPOSITE;
	array_struct_sw_json_surface_block_ptr_init(&block->_.composite.children, 1);
	block->_.expand = SW_SURFACE_BLOCK_EXPAND_TOP | SW_SURFACE_BLOCK_EXPAND_BOTTOM;
	if (state.config.workspace_min_width > 0) {
		string_init_format(&block->_.min_width, "%d", state.config.workspace_min_width);
		block->_.content_anchor = SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
	}
	block->_.color = colors.background;
	for (size_t j = 0; j < LENGTH(block->_.borders); ++j) {
		block->_.borders[j].width = STRING_LITERAL(STRINGIFY(WORKSPACE_BORDER_WIDTH));
		block->_.borders[j].color = colors.border;
	}

	struct surface_block *text = surface_block_create();
	surface_block_init_text(&text->_, NULL);
	text->_.text.color = colors.text;
	text->_.border_left.width = STRING_LITERAL(STRINGIFY(WORKSPACE_MARGIN_LEFT));
	text->_.border_right.width = STRING_LITERAL(STRINGIFY(WORKSPACE_MARGIN_RIGHT));
	text->_.border_bottom.width = STRING_LITERAL(STRINGIFY(WORKSPACE_MARGIN_BOTTOM));
	text->_.border_top.width = STRING_LITERAL(STRINGIFY(WORKSPACE_MARGIN_TOP));
	if (workspace->num != -1) {
		if (state.config.strip_workspace_name) {
			string_init_format(&text->_.text.text, "%d", workspace->num);
		} else if (state.config.strip_workspace_numbers) {
			int num_len = stbsp_snprintf(NULL, 0, "%d", workspace->num);
			num_len += (workspace->name.s[num_len] == ':');
			if (workspace->name.s[num_len] != '\0') {
				text->_.text.text = (string_t){
					.s = workspace->name.s + num_len,
					.len = workspace->name.len - (size_t)num_len,
					.nul_terminated = true,
					.free_contents = false,
				};
			}
		}
	}
	if (text->_.text.text.len == 0) {
		text->_.text.text = string_copy(workspace->name);
	}

	array_struct_sw_json_surface_block_ptr_add(&block->_.composite.children, &text->_);

	workspace->block = block;
}

static void output_destroy(struct output *output) {
	if (!output) {
		return;
	}

	for (size_t i = 0; i < output->workspaces.len; ++i) {
		workspace_fini(
			array_struct_workspace_get_ptr(&output->workspaces, i));
	}
	array_struct_workspace_fini(&output->workspaces);

	free(output);
}

static void output_destroy_sw_json(struct sw_json_output *output) {
	sw_json_output_fini(output);
	output_destroy((struct output *)output);
}

static struct output *output_create(string_t name, int32_t width, int32_t height,
		int32_t scale, enum sw_output_transform transform) {
	struct output *output = malloc(sizeof(struct output));
	output->focused = false;
	array_struct_workspace_init(&output->workspaces, 20);

	sw_json_output_init(&output->_, name, width, height, scale, transform, output_destroy_sw_json);

	return output;
}

static struct sw_json_output *output_create_sw_json(string_t name, int32_t width, int32_t height,
		int32_t scale, enum sw_output_transform transform) {
	sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_GET_WORKSPACES, NULL);

	struct output *output = output_create(name, width, height, scale, transform);
	return &output->_;
}

static void bars_set_dirty(void) {
	for (struct sw_json_output *output = state.sw->outputs.head; output; output = output->next) {
		if (output->layers.len > 0) {
			assert(output->layers.len == 1);
			struct bar *bar = (struct bar *)array_struct_sw_json_surface_ptr_get(
				&output->layers, 0);
			bar->dirty = true;
		}
	}
}

#if HAVE_TRAY
static void tray_dbusmenu_menu_popup_destroy(struct tray_dbusmenu_menu_popup *popup) {
	if (!popup) {
		return;
	}

	sni_dbusmenu_menu_item_event(popup->menu->parent_menu_item,
		SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLOSED, true);

	sw_json_surface_fini(&popup->_);
	sw_json_set_dirty(state.sw);

	if (popup == state.tray->popup) {
		state.tray->popup = NULL;
	}

	free(popup);
}

static struct tray_dbusmenu_menu_popup *tray_dbusmenu_menu_popup_create(struct sni_dbusmenu_menu *menu,
	int32_t x, int32_t y, uint32_t grab_serial, struct bar *bar, struct sw_json_seat *seat);

static void tray_dbusmenu_menu_item_pointer_enter(struct sni_dbusmenu_menu_item *menu_item,
		struct sw_json_surface_block *block, struct tray_dbusmenu_menu_popup *popup) {
	sni_dbusmenu_menu_item_event(menu_item,
		SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_HOVERED, true);

	if (popup->_.popups.len > 0) {
		assert(popup->_.popups.len == 1);
		tray_dbusmenu_menu_popup_destroy((struct tray_dbusmenu_menu_popup *)
			array_struct_sw_json_surface_ptr_get(&popup->_.popups, 0));
		popup->_.popups.len = 0;
	}

	if (menu_item->enabled && (menu_item->type != SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR)) {
		if (menu_item->submenu && (menu_item->submenu->menu_items.len > 0)) {
			struct tray_dbusmenu_menu_popup *submenu_popup = tray_dbusmenu_menu_popup_create(
				menu_item->submenu, block->box.x, block->box.y + block->box.height, 0,
				popup->bar, popup->seat);
			array_struct_sw_json_surface_ptr_add(&popup->_.popups, &submenu_popup->_);
		}

		block->color = state.config.colors.focused_separator;

		sw_json_set_dirty(state.sw);
	}
}

static void tray_dbusmenu_menu_item_pointer_button(struct sni_dbusmenu_menu_item *menu_item,
		uint32_t code, enum sw_pointer_button_state state_) {
	if ((code != BTN_LEFT) || (state_ == SW_POINTER_BUTTON_STATE_RELEASED)) {
		return;
	}

	if ((menu_item->type != SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR)
			&& menu_item->enabled) {
		sni_dbusmenu_menu_item_event(menu_item,
			SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLICKED, true);

#if TRAY_DBUSMENU_CLOSE_AFTER_SELECTION
		assert(state.tray->popup->bar->_.popups.len == 1);
		state.tray->popup->bar->_.popups.len = 0;
		tray_dbusmenu_menu_popup_destroy(state.tray->popup);
#endif
	}
}

static void tray_dbusmenu_menu_item_pointer_leave(struct sni_dbusmenu_menu_item *menu_item,
		struct sw_json_surface_block *block) {
	if ((menu_item->type != SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR) && menu_item->enabled) {
		block->color = state.config.colors.focused_background;
		sw_json_set_dirty(state.sw);
	}
}

static union sw_color tray_dbusmenu_menu_item_get_text_color(struct sni_dbusmenu_menu_item *menu_item) {
	if (menu_item->enabled) {
		return state.config.colors.focused_statusline;
	} else {
		union sw_color color = state.config.colors.focused_statusline;
		color.a >>= 1;
		return color;
	}
}

static void tray_dbusmenu_menu_popup_process_pointer_pos(struct tray_dbusmenu_menu_popup *popup,
	int32_t x, int32_t y) {
	struct sw_json_surface_block *test = array_struct_sw_json_surface_block_ptr_get(
		&popup->_.layout_root->composite.children, 0);
	if ((test->box.width == 0) || (test->box.height == 0)) {
		return;
	}

	for (size_t i = 0; i < popup->_.layout_root->composite.children.len; ++i) {
		struct surface_block *block = (struct surface_block *)
			array_struct_sw_json_surface_block_ptr_get(&popup->_.layout_root->composite.children, i);
		struct sw_json_surface_block_box box = block->_.box;
		if ((x >= box.x) && (y >= box.y)
				&& (x < (box.x + box.width))
				&& (y < (box.y + box.height))) {
			if (popup->focused_block != block) {
				if (popup->focused_block) {
					struct sni_dbusmenu_menu_item *menu_item = popup->focused_block->menu_item;
					tray_dbusmenu_menu_item_pointer_leave(menu_item, &popup->focused_block->_);
				}
				struct sni_dbusmenu_menu_item *menu_item = block->menu_item;
				tray_dbusmenu_menu_item_pointer_enter(menu_item, &block->_, popup);
				popup->focused_block = block;
			}
			return;
		}
	}

	if (popup->focused_block) {
		struct sni_dbusmenu_menu_item *menu_item = popup->focused_block->menu_item;
		tray_dbusmenu_menu_item_pointer_leave(menu_item, &popup->focused_block->_);
		popup->focused_block = NULL;
	}
}

static void tray_dbusmenu_menu_popup_handle_pointer_enter(struct sw_json_pointer *pointer) {
	struct tray_dbusmenu_menu_popup *popup = (struct tray_dbusmenu_menu_popup *)pointer->focused_surface;
	if (popup->seat != pointer->seat) {
		return;
	}

	popup->focused_block = NULL;
	tray_dbusmenu_menu_popup_process_pointer_pos(popup, pointer->pos_x, pointer->pos_y);
}

static void tray_dbusmenu_menu_popup_handle_pointer_motion(struct sw_json_pointer *pointer) {
	struct tray_dbusmenu_menu_popup *popup = (struct tray_dbusmenu_menu_popup *)pointer->focused_surface;
	if (popup->seat != pointer->seat) {
		return;
	}

	tray_dbusmenu_menu_popup_process_pointer_pos(popup, pointer->pos_x, pointer->pos_y);
}

static void tray_dbusmenu_menu_popup_handle_pointer_leave(struct sw_json_pointer *pointer) {
	struct tray_dbusmenu_menu_popup *popup = (struct tray_dbusmenu_menu_popup *)pointer->focused_surface;
	if (popup->seat != pointer->seat) {
		return;
	}

	if (popup->focused_block) {
		struct sni_dbusmenu_menu_item *menu_item = popup->focused_block->menu_item;
		tray_dbusmenu_menu_item_pointer_leave(menu_item, &popup->focused_block->_);
		popup->focused_block = NULL;
	}
}

static void tray_dbusmenu_menu_popup_handle_pointer_button(struct sw_json_pointer *pointer) {
	struct tray_dbusmenu_menu_popup *popup = (struct tray_dbusmenu_menu_popup *)pointer->focused_surface;
	if (popup->seat != pointer->seat) {
		return;
	}

	if (popup->focused_block) {
		struct sni_dbusmenu_menu_item *menu_item = popup->focused_block->menu_item;
		tray_dbusmenu_menu_item_pointer_button(menu_item, pointer->btn_code, pointer->btn_state);
	}
}

static void tray_dbusmenu_menu_popup_update(struct tray_dbusmenu_menu_popup *popup,
		struct sni_dbusmenu_menu *menu) {
	for (size_t i = 0; i < popup->_.layout_root->composite.children.len; ++i) {
		surface_block_unref((struct surface_block *)
			array_struct_sw_json_surface_block_ptr_get(&popup->_.layout_root->composite.children, i));
	}
	popup->_.layout_root->composite.children.len = 0;
	popup->focused_block = NULL;

	popup->menu = menu;

	bool32_t needs_spacer = false;

	for (size_t i = 0; i < menu->menu_items.len; ++i) {
		struct sni_dbusmenu_menu_item *menu_item = array_struct_sni_dbusmenu_menu_item_get_ptr(
			&menu->menu_items, i);
		if (!menu_item->visible) {
			continue;
		}

		struct surface_block *block = surface_block_create();
		block->menu_item = menu_item;
		block->type = SURFACE_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM;
		block->_.expand = SW_SURFACE_BLOCK_EXPAND_LEFT | SW_SURFACE_BLOCK_EXPAND_RIGHT;
		if (menu_item->type == SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR) {
			//block->_.type = SW_SURFACE_BLOCK_TYPE_SPACER;
			block->_.min_height = STRING_LITERAL(STRINGIFY(TRAY_DBUSMENU_SEPARATOR_WIDTH));
			block->_.color = state.config.colors.focused_separator;
		} else {
			block->_.type = SW_SURFACE_BLOCK_TYPE_COMPOSITE;
			array_struct_sw_json_surface_block_ptr_init(&block->_.composite.children, 6);

			uint64_t id = 0;
			if (menu_item->label.len > 0) {
				id = i + 1;
				struct surface_block *label = surface_block_create();
				label->_.id = id;
				surface_block_init_text(&label->_, &menu_item->label);
				label->_.text.color = tray_dbusmenu_menu_item_get_text_color(menu_item);
				label->_.anchor = SW_SURFACE_BLOCK_ANCHOR_RIGHT;
				if (state.config.tray_padding > 0) {
					string_t s;
					string_init_format(&s, "%d", state.config.tray_padding);
					label->_.border_top.width = s;
					label->_.border_bottom.width = string_copy(s);
					label->_.border_left.width = string_copy(s);
					label->_.border_right.width = string_copy(s);
				}
				array_struct_sw_json_surface_block_ptr_add(&block->_.composite.children, &label->_);
			}

#if HAVE_SVG || HAVE_PNG
			if (menu_item->icon_name.len > 0) {
				struct sni_dbusmenu *dbusmenu = menu_item->parent_menu->dbusmenu;
				if (dbusmenu->item->properties && (dbusmenu->item->properties->icon_theme_path.len > 0)) {
					xdg_icon_theme_cache_add_basedir(&state.tray->cache, dbusmenu->item->properties->icon_theme_path, false);
				}
				if (dbusmenu->properties) {
					for (size_t j = 0; j < dbusmenu->properties->icon_theme_path.len; ++j) {
						xdg_icon_theme_cache_add_basedir(&state.tray->cache,
							array_string_t_get(&dbusmenu->properties->icon_theme_path, j), false);
					}
				}
				xdg_icon_theme_cache_join_threads(&state.tray->cache);
				struct xdg_icon_theme_icon xdg_icon;
				if (xdg_icon_theme_cache_find_icon(&state.tray->cache, &xdg_icon,
						menu_item->icon_name, state.config.tray_icon_theme.s ? &state.config.tray_icon_theme : NULL)) {
					struct surface_block *icon = surface_block_create();
					if (state.config.tray_padding > 0) {
						string_t s;
						string_init_format(&s, "%d", state.config.tray_padding);
						icon->_.border_top.width = s;
						icon->_.border_bottom.width = string_copy(s);
						icon->_.border_left.width = string_copy(s);
						icon->_.border_right.width = string_copy(s);
					}
					icon->_.type = SW_SURFACE_BLOCK_TYPE_IMAGE;
					string_init_string(&icon->_.image.path, xdg_icon.path);
					icon->_.image.type = (enum sw_surface_block_type_image_image_type)xdg_icon.type;
					icon->_.anchor = SW_SURFACE_BLOCK_ANCHOR_RIGHT;
					if (id > 0) {
						string_t s;
						string_init_format(&s, "block_%lu_content_height", id);
						icon->_.content_width = s;
						icon->_.content_height = string_copy(s);
					}
					array_struct_sw_json_surface_block_ptr_add(&block->_.composite.children, &icon->_);
				}
			}
#endif // HAVE_SVG || HAVE_PNG

#if HAVE_PNG
			if (menu_item->icon_data.nbytes > 0) {
				string_t path;
				int fd = data_to_shm(&path, menu_item->icon_data.bytes, menu_item->icon_data.nbytes);
				if (fd != -1) {
					struct surface_block *icon = surface_block_create();
					icon->cleanup_file = fd;
					if (state.config.tray_padding > 0) {
						string_t s;
						string_init_format(&s, "%d", state.config.tray_padding);
						icon->_.border_top.width = s;
						icon->_.border_bottom.width = string_copy(s);
						icon->_.border_left.width = string_copy(s);
						icon->_.border_right.width = string_copy(s);
					}
					icon->_.type = SW_SURFACE_BLOCK_TYPE_IMAGE;
					icon->_.image.path = path;
					icon->_.image.type = SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PNG;
					icon->_.anchor = SW_SURFACE_BLOCK_ANCHOR_RIGHT;
					if (id > 0) {
						string_t s;
						string_init_format(&s, "block_%lu_content_height", id);
						icon->_.content_width = s;
						icon->_.content_height = string_copy(s);
					}
					array_struct_sw_json_surface_block_ptr_add(&block->_.composite.children, &icon->_);
				}
			}
#endif // HAVE_PNG

			switch (menu_item->toggle_type) {
			case SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_CHECKMARK:
			case SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_RADIO: {
				struct surface_block *toggle = surface_block_create();
				surface_block_init_text(&toggle->_, (menu_item->toggle_type == SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_RADIO)
						? ((menu_item->toggle_state == 1) ? &STRING_LITERAL("󰐾") : &STRING_LITERAL("󰄯"))
						: ((menu_item->toggle_state == 1) ? &STRING_LITERAL("󰄲") : &STRING_LITERAL("󰄮")));
				toggle->_.text.color = tray_dbusmenu_menu_item_get_text_color(menu_item);
				toggle->_.anchor = SW_SURFACE_BLOCK_ANCHOR_RIGHT;
				if (state.config.tray_padding > 0) {
					string_t s;
					string_init_format(&s, "%d", state.config.tray_padding);
					toggle->_.border_top.width = s;
					toggle->_.border_bottom.width = string_copy(s);
					toggle->_.border_left.width = string_copy(s);
					toggle->_.border_right.width = string_copy(s);
				}
				if (id > 0) {
					toggle->_.content_anchor = SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
					string_t s;
					string_init_format(&s, "block_%lu_content_height+%d", id, state.config.tray_padding * 2);
					toggle->_.min_width = s;
					toggle->_.max_width = 	string_copy(s);
					toggle->_.min_height = string_copy(s);
					toggle->_.max_height = string_copy(s);
				}
				array_struct_sw_json_surface_block_ptr_add(&block->_.composite.children, &toggle->_);
				needs_spacer = true;
				break;
			}
			case SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_NONE:
			default:
				break;
			}

			if (menu_item->submenu) {
				struct surface_block *submenu = surface_block_create();
				surface_block_init_text(&submenu->_, &STRING_LITERAL("󰍞"));
				submenu->_.text.color = tray_dbusmenu_menu_item_get_text_color(menu_item);
				submenu->_.anchor = SW_SURFACE_BLOCK_ANCHOR_RIGHT;
				if (state.config.tray_padding > 0) {
					string_t s;
					string_init_format(&s, "%d", state.config.tray_padding);
					submenu->_.border_top.width = s;
					submenu->_.border_bottom.width = string_copy(s);
					submenu->_.border_left.width = string_copy(s);
					submenu->_.border_right.width = string_copy(s);
				}
				if (id > 0) {
					submenu->_.content_anchor = SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
					string_t s;
					string_init_format(&s, "block_%lu_content_height+%d", id, state.config.tray_padding * 2);
					submenu->_.min_width = s;
					submenu->_.max_width = string_copy(s);
					submenu->_.min_height = string_copy(s);
					submenu->_.max_height = string_copy(s);
				}
				array_struct_sw_json_surface_block_ptr_add(&block->_.composite.children, &submenu->_);
				needs_spacer = true;
			}
		}
		array_struct_sw_json_surface_block_ptr_add(&popup->_.layout_root->composite.children, &block->_);
	}

	if (needs_spacer) {
		for (size_t i = 0; i < popup->_.layout_root->composite.children.len; ++i) {
			struct surface_block *block = (struct surface_block *)
				array_struct_sw_json_surface_block_ptr_get(&popup->_.layout_root->composite.children, i);
			struct sni_dbusmenu_menu_item *menu_item = block->data;
			if ((block->_.type == SW_SURFACE_BLOCK_TYPE_COMPOSITE)
					&& (menu_item->toggle_type == SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_NONE)
					&& (menu_item->submenu == NULL)) {
				struct sw_json_surface_block *label = array_struct_sw_json_surface_block_ptr_get(
					&block->_.composite.children, 0);
				if (label->id > 0) {
					struct surface_block *spacer = surface_block_create();
					//spacer->_.type = SW_SURFACE_BLOCK_TYPE_SPACER;
					string_t s;
					string_init_format(&s, "block_%lu_content_height+%d", label->id, state.config.tray_padding * 2);
					spacer->_.min_width = s;
					spacer->_.min_height = string_copy(s);
					spacer->_.anchor = SW_SURFACE_BLOCK_ANCHOR_RIGHT;
					array_struct_sw_json_surface_block_ptr_add(&block->_.composite.children, &spacer->_);
				}
			}
		}
	}

	// TODO: update child popups somehow

	sw_json_set_dirty(state.sw);
}

static void tray_dbusmenu_menu_popup_destroy_sw_json(struct sw_json_surface *popup) {
	tray_dbusmenu_menu_popup_destroy((struct tray_dbusmenu_menu_popup *)popup);
}

static struct tray_dbusmenu_menu_popup *tray_dbusmenu_menu_popup_create(struct sni_dbusmenu_menu *menu,
		int32_t x, int32_t y, uint32_t grab_serial, struct bar *bar, struct sw_json_seat *seat) {
	sni_dbusmenu_menu_about_to_show(menu, true);

	struct tray_dbusmenu_menu_popup *popup = malloc(sizeof(struct tray_dbusmenu_menu_popup));
	popup->bar = bar;
	popup->seat = seat;

	struct surface_block *layout_root = surface_block_create();
	layout_root->_.type = SW_SURFACE_BLOCK_TYPE_COMPOSITE;
	layout_root->_.composite.layout = SW_SURFACE_BLOCK_TYPE_COMPOSITE_BLOCK_LAYOUT_VERTICAL;
	array_struct_sw_json_surface_block_ptr_init(&layout_root->_.composite.children, 64);
	layout_root->_.color = state.config.colors.focused_background;

	sw_json_surface_popup_init(&popup->_, &layout_root->_, state.sw, x, y, tray_dbusmenu_menu_popup_destroy_sw_json);
	popup->_.popup.grab = true;
	popup->_.popup.grab_serial = grab_serial;
	popup->_.popup.gravity = SW_SURFACE_POPUP_GRAVITY_TOP_LEFT;
	popup->_.popup.constraint_adjustment =
		SW_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_X
		| SW_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_Y;
	popup->_.pointer_enter_callback = tray_dbusmenu_menu_popup_handle_pointer_enter;
	popup->_.pointer_motion_callback = tray_dbusmenu_menu_popup_handle_pointer_motion;
	popup->_.pointer_leave_callback = tray_dbusmenu_menu_popup_handle_pointer_leave;
	popup->_.pointer_button_callback = tray_dbusmenu_menu_popup_handle_pointer_button;
	sw_json_set_dirty(state.sw);

	tray_dbusmenu_menu_popup_update(popup, menu);

	sni_dbusmenu_menu_item_event(menu->parent_menu_item,
		SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_OPENED, true);

	return popup;
}

static void tray_item_update(struct tray_item *item) {
	struct surface_block *block = item->block;
	if (block->_.type == SW_SURFACE_BLOCK_TYPE_COMPOSITE) {
		assert(block->_.composite.children.len == 1);
		surface_block_unref(
			(struct surface_block *)array_struct_sw_json_surface_block_ptr_get(
				&block->_.composite.children, 0));
		array_struct_sw_json_surface_block_ptr_fini(&block->_.composite.children);
		block->_.type = SW_SURFACE_BLOCK_TYPE_SPACER;
	}

	struct sni_item_properties *props = item->_.properties;
	if (!props) {
		return;
	}

	struct sw_json_surface_block_type_image icon = { 0 };
	string_t icon_name = { 0 };
	(void)icon_name;
	struct sni_item_pixmap *icon_pixmap = NULL;
	switch(props->status) {
	case SNI_ITEM_STATUS_ACTIVE:
		icon_name = props->icon_name;
		if (props->icon_pixmap.len > 0) {
			icon_pixmap = array_struct_sni_item_pixmap_ptr_get(&props->icon_pixmap, 0);
		}
		break;
	case SNI_ITEM_STATUS_NEEDS_ATTENTION:
		icon_name = props->attention_icon_name;
		if (props->attention_icon_pixmap.len > 0) {
			icon_pixmap = array_struct_sni_item_pixmap_ptr_get(&props->attention_icon_pixmap, 0);
		}
		break;
	case SNI_ITEM_STATUS_PASSIVE:
	case SNI_ITEM_STATUS_INVALID:
	default:
		break;
	}

#if HAVE_SVG || HAVE_PNG
	if (icon_name.len > 0) {
		if (props->icon_theme_path.len > 0) {
			xdg_icon_theme_cache_add_basedir(&state.tray->cache, props->icon_theme_path, true);
		}
		struct xdg_icon_theme_icon xdg_icon;
		if (xdg_icon_theme_cache_find_icon(&state.tray->cache, &xdg_icon,
				icon_name, state.config.tray_icon_theme.s ? &state.config.tray_icon_theme : NULL)) {
			string_init_string(&icon.path, xdg_icon.path);
			icon.type = (enum sw_surface_block_type_image_image_type)xdg_icon.type;
		}
	}
#endif // HAVE_SVG || HAVE_PNG

	int cleanup_file = -1;
	if ((icon.path.len == 0) && icon_pixmap) {
		int fd = data_to_shm(&icon.path, icon_pixmap,
					(size_t)icon_pixmap->width * (size_t)icon_pixmap->height * 4
					+ sizeof(struct sni_item_pixmap));
		if (fd != -1) {
			icon.type = SW_SURFACE_BLOCK_TYPE_IMAGE_IMAGE_TYPE_PIXMAP;
			cleanup_file = fd;
		}
	}

	if (icon.path.len > 0) {
		block->_.type = SW_SURFACE_BLOCK_TYPE_COMPOSITE;
		array_struct_sw_json_surface_block_ptr_init(&block->_.composite.children, 1);
		block->_.content_anchor = SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;

		struct surface_block *image = surface_block_create();
		image->_.type = SW_SURFACE_BLOCK_TYPE_IMAGE;
		image->_.image = icon;
		image->_.expand = SW_SURFACE_BLOCK_EXPAND_BOTTOM | SW_SURFACE_BLOCK_EXPAND_TOP | SW_SURFACE_BLOCK_EXPAND_CONTENT;
		if (state.config.tray_padding > 0) {
			string_t s;
			string_init_format(&s, "block_1_content_height-%d", state.config.tray_padding * 2);
			image->_.content_width = s;
			image->_.content_height = string_copy(s);
		} else {
			image->_.content_width = STRING_LITERAL("block_1_content_height");
			image->_.content_height = STRING_LITERAL("block_1_content_height");
		}
		image->cleanup_file = cleanup_file;

		array_struct_sw_json_surface_block_ptr_add(&block->_.composite.children, &image->_);
	}
}

static void tray_item_block_pointer_button(struct surface_block *block,
		struct bar *bar, uint32_t code, enum sw_pointer_button_state state_, uint32_t serial,
		int32_t x, int32_t y, struct sw_json_seat *seat) {
	if (state_ == SW_POINTER_BUTTON_STATE_RELEASED) {
		return;
	}

	struct sni_item *item = &block->tray_item->_;

	enum tray_binding_command command = TRAY_BINDING_COMMAND_NONE;
	for (size_t i = 0; i < state.config.tray_bindings.len; ++i) {
		struct tray_binding binding = array_struct_tray_binding_get(&state.config.tray_bindings, i);
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

	if ((command == TRAY_BINDING_COMMAND_ACTIVATE) && item->properties
			&& item->properties->item_is_menu) {
		command = TRAY_BINDING_COMMAND_CONTEXT_MENU;
	}

	switch (command) {
	case TRAY_BINDING_COMMAND_CONTEXT_MENU: {
		struct sni_dbusmenu *dbusmenu = item->dbusmenu;
#define MENU array_struct_sni_dbusmenu_menu_item_get_ptr(&dbusmenu->menu->menu_items, 0)->submenu
		if (dbusmenu && dbusmenu->menu && (dbusmenu->menu->menu_items.len > 0)
				&& MENU && (MENU->menu_items.len > 0)) {
					state.tray->popup = tray_dbusmenu_menu_popup_create(MENU, x, y, serial, bar, seat);
			array_struct_sw_json_surface_ptr_add( &bar->_.popups, &state.tray->popup->_);
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

static void tray_item_properties_updated_sni_item(struct sni_item *item,
		struct sni_item_properties *old_props) {
	(void)old_props;
	tray_item_update((struct tray_item *)item);
	bars_set_dirty();
}

static void tray_item_dbusmenu_menu_updated_sni_item(struct sni_item *item,
		struct sni_dbusmenu_menu *old_menu) {
	(void)old_menu;
	if ((state.tray->popup == NULL) || (state.tray->popup->menu->dbusmenu->item != item)) {
		return;
	}

	struct sni_dbusmenu_menu *menu = item->dbusmenu->menu;
#define MENU array_struct_sni_dbusmenu_menu_item_get_ptr(&menu->menu_items, 0)->submenu
	if (menu && (menu->menu_items.len > 0) && MENU && (MENU->menu_items.len > 0)) {
		if (state.tray->popup->_.popups.len > 0) {
			assert(state.tray->popup->_.popups.len == 1);
			tray_dbusmenu_menu_popup_destroy((struct tray_dbusmenu_menu_popup *)
				array_struct_sw_json_surface_ptr_get(&state.tray->popup->_.popups, 0));
				state.tray->popup->_.popups.len = 0;
		}
		tray_dbusmenu_menu_popup_update(state.tray->popup, MENU);
	} else {
		assert(state.tray->popup->bar->_.popups.len == 1);
		state.tray->popup->bar->_.popups.len = 0;
		tray_dbusmenu_menu_popup_destroy(state.tray->popup);
	}
#undef MENU
}

static void tray_item_destroy(struct tray_item *item) {
	if (!item) {
		return;
	}

	surface_block_unref(item->block);

	if (state.tray->popup && (state.tray->popup->menu->dbusmenu->item == &item->_)) {
		assert(state.tray->popup->bar->_.popups.len == 1);
		state.tray->popup->bar->_.popups.len = 0;
		tray_dbusmenu_menu_popup_destroy(state.tray->popup);
	}

	sni_item_fini(&item->_);

	free(item);
}

static void tray_item_destroy_sni_item(struct sni_item *item) {
	tray_item_destroy((struct tray_item *)item);
	bars_set_dirty();
}

static struct tray_item *tray_item_create(string_t id) {
	struct tray_item *item = malloc(sizeof(struct tray_item));
	if (!sni_item_init(&item->_, id, tray_item_destroy_sni_item)) {
		free(item);
		return NULL;
	}

	struct surface_block *block = surface_block_create();
	block->type = SURFACE_BLOCK_TYPE_TRAY_ITEM;
	block->tray_item = item;
	block->_.anchor = SW_SURFACE_BLOCK_ANCHOR_RIGHT;
	block->_.expand = SW_SURFACE_BLOCK_EXPAND_TOP | SW_SURFACE_BLOCK_EXPAND_BOTTOM;
	block->_.min_width = STRING_LITERAL("block_1_content_height");
	block->_.min_height = STRING_LITERAL("block_1_content_height");

	item->block = block;

	return item;
}

static struct sni_item *tray_item_create_sni_item(string_t id) {
	struct tray_item *item = tray_item_create(id);
	if (!item) {
		return NULL;
	}

	item->_.properties_updated = tray_item_properties_updated_sni_item;
	item->_.dbusmenu_menu_updated = tray_item_dbusmenu_menu_updated_sni_item;

	bars_set_dirty();

	return &item->_;
}

static void tray_init(void) {
	int ret = sni_server_init(tray_item_create_sni_item);
	if (ret < 0) {
		abort_(-ret, "Failed to initialize system tray: %s", strerror(-ret));
	}

	state.tray = malloc(sizeof(struct tray));
	state.tray->sni = sni_server;
	state.tray->popup = NULL;

#if HAVE_SVG || HAVE_PNG
	xdg_icon_theme_cache_init(&state.tray->cache);
#endif // HAVE_SVG || HAVE_PNG
}

static void tray_fini(void) {
	sni_server_fini();

#if HAVE_SVG || HAVE_PNG
	xdg_icon_theme_cache_fini(&state.tray->cache);
#endif // HAVE_SVG || HAVE_PNG

	free(state.tray);

	state.poll_fds[POLL_FD_SNI_SERVER].fd = -1;
	state.tray = NULL;
}

static void tray_update(void) {
	for (size_t i = 0; i < sni_server->host.items.len; ++i) {
		tray_item_update((struct tray_item *)
			array_struct_sni_item_ptr_get(&sni_server->host.items, i));
	}

#if HAVE_SVG || HAVE_PNG
	xdg_icon_theme_cache_fini(&state.tray->cache);
	xdg_icon_theme_cache_init(&state.tray->cache);
#endif // HAVE_SVG || HAVE_PNG
}
#endif // HAVE_TRAY

static void status_line_init(void) {
	assert(state.config.status_command.nul_terminated);

	int pipe_read_fd[2];
	int pipe_write_fd[2];
	if ((pipe(pipe_read_fd) == -1) || (pipe(pipe_write_fd) == -1)) {
		abort_(errno, "pipe: %s", strerror(errno));
	}

	pid_t pid = fork();
	if (pid == -1) {
		abort_(errno, "fork: %s", strerror(errno));
	} else if (pid == 0) {
		setpgid(0, 0);

		dup2(pipe_read_fd[1], STDOUT_FILENO);
		close(pipe_read_fd[0]);
		close(pipe_read_fd[1]);

		dup2(pipe_write_fd[0], STDIN_FILENO);
		close(pipe_write_fd[0]);
		close(pipe_write_fd[1]);

		const char *cmd_[] = { "sh", "-c", state.config.status_command.s, NULL };
		execvp(cmd_[0], (char * const *)cmd_);
		exit(1);
	}

	if (!fd_set_nonblock(pipe_read_fd[0])) {
		abort_(errno, "fcntl: %s", strerror(errno));
	}

	close(pipe_read_fd[1]);
	close(pipe_write_fd[0]);

	fd_set_cloexec(pipe_read_fd[0]);
	fd_set_cloexec(pipe_write_fd[1]);

	state.status = calloc(1, sizeof(struct status_line));

	state.status->buf.size = 8192;
	state.status->buf.data = malloc(state.status->buf.size);

	state.status->stop_signal = SIGSTOP;
	state.status->cont_signal = SIGCONT;

	state.status->pid = pid;
	state.status->read_fd = pipe_read_fd[0];
	state.status->write_fd = pipe_write_fd[1];

	state.poll_fds[POLL_FD_STATUS].fd = pipe_read_fd[0];
}

static void status_line_close_pipes(void) {
	if (state.status->read_fd != -1) {
		close(state.status->read_fd);
		state.status->read_fd = -1;

		state.poll_fds[POLL_FD_STATUS].fd = -1;
	}
	if (state.status->write_fd != -1) {
		close(state.status->write_fd);
		state.status->write_fd = -1;
	}

	if (state.status->read) {
		fclose(state.status->read);
		state.status->read = NULL;
	}
}

static void status_line_i3bar_block_fini(struct status_line_i3bar_block *block);

static void status_line_fini(void) {
	status_line_close_pipes();
	kill(-state.status->pid, state.status->cont_signal);
	kill(-state.status->pid, SIGTERM);
	waitpid(state.status->pid, NULL, 0);

	free(state.status->buf.data);

	if (state.status->protocol == STATUS_LINE_PROTOCOL_I3BAR) {
		json_writer_fini(&state.status->writer);
		json_tokener_fini(&state.status->tokener);
		for (size_t i = 0; i < state.status->blocks.len; ++i) {
			status_line_i3bar_block_fini(
				array_struct_status_line_i3bar_block_get_ptr(&state.status->blocks, i));
		}
		array_struct_status_line_i3bar_block_fini(&state.status->blocks);
	}

	free(state.status);
	state.status = NULL;
}

static void status_line_set_error(string_t text) {
	status_line_close_pipes();

	if (state.status->protocol == STATUS_LINE_PROTOCOL_I3BAR) {
		json_writer_fini(&state.status->writer);
		json_tokener_fini(&state.status->tokener);
		for (size_t i = 0; i < state.status->blocks.len; ++i) {
			status_line_i3bar_block_fini(
				array_struct_status_line_i3bar_block_get_ptr(&state.status->blocks, i));
		}
		array_struct_status_line_i3bar_block_fini(&state.status->blocks);
	}

	state.status->buf.data = realloc(state.status->buf.data, text.len);
	memcpy(state.status->buf.data, text.s, text.len);
	state.status->buf.size = text.len;
	state.status->buf.idx = text.len;

	state.status->protocol = STATUS_LINE_PROTOCOL_ERROR;
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

static void status_line_i3bar_block_pointer_button(struct surface_block *block,
		struct bar *bar, uint32_t code, enum sw_pointer_button_state state_,
		int32_t x, int32_t y) {
	if ((state_ == SW_POINTER_BUTTON_STATE_RELEASED) || (state.status->write_fd == -1)) {
		return;
	}

	// TODO: scale

	double rx = x - block->_.box.x;
	double ry = y - block->_.box.y;

	struct status_line_i3bar_block *i3bar_block = block->i3bar_block;
	struct json_writer *writer = &state.status->writer;

	json_writer_object_begin(writer);

	json_writer_object_key(writer, STRING_LITERAL("name"));
	json_writer_string_escape(writer, i3bar_block->name);

	if (i3bar_block->instance.len > 0) {
		json_writer_object_key(writer, STRING_LITERAL("instance"));
		json_writer_string_escape(writer, i3bar_block->instance);
	}

	json_writer_object_key(writer, STRING_LITERAL("button"));
	json_writer_uint(writer, to_x11_button(code));
	json_writer_object_key(writer, STRING_LITERAL("event"));
	json_writer_uint(writer, code);
	if (state.status->float_event_coords) {
		json_writer_object_key(writer, STRING_LITERAL("x"));
		json_writer_double(writer, x);
		json_writer_object_key(writer, STRING_LITERAL("y"));
		json_writer_double(writer, y);
		json_writer_object_key(writer, STRING_LITERAL("relative_x"));
		json_writer_double(writer, rx);
		json_writer_object_key(writer, STRING_LITERAL("relative_y"));
		json_writer_double(writer, ry);
		json_writer_object_key(writer, STRING_LITERAL("width"));
		json_writer_double(writer, (double)block->_.box.width);
		json_writer_object_key(writer, STRING_LITERAL("height"));
		json_writer_double(writer, (double)block->_.box.height);
	} else {
		json_writer_object_key(writer, STRING_LITERAL("x"));
		json_writer_int(writer, x);
		json_writer_object_key(writer, STRING_LITERAL("y"));
		json_writer_int(writer, y);
		json_writer_object_key(writer, STRING_LITERAL("relative_x"));
		json_writer_int(writer, (int64_t)rx);
		json_writer_object_key(writer, STRING_LITERAL("relative_y"));
		json_writer_int(writer, (int64_t)ry);
		json_writer_object_key(writer, STRING_LITERAL("width"));
		json_writer_int(writer, block->_.box.width);
		json_writer_object_key(writer, STRING_LITERAL("height"));
		json_writer_int(writer, block->_.box.height);
	}
	json_writer_object_key(writer, STRING_LITERAL("scale"));
	json_writer_int(writer, bar->_.scale);

	json_writer_object_end(writer);

	json_buffer_add_char(&writer->buf, '\n');
	if (write(state.status->write_fd, writer->buf.data, writer->buf.idx) == -1) {
		status_line_set_error(STRING_LITERAL("[failed to write click event]"));
		bars_set_dirty();
	}
	json_writer_reset(writer);
}

static void status_line_i3bar_handle_test_short_text(struct sw_json_surface *bar_) {
	struct bar *bar = (struct bar *)bar_;

	struct sw_json_surface_block *test = array_struct_sw_json_surface_block_ptr_get(
		&bar->_.layout_root->composite.children, 0);
	if ((test->box.width == 0) || (test->box.height == 0)) {
		return;
	}

	size_t last_status_block_idx = bar->_.layout_root->composite.children.len - 1
		- (state.binding_mode_indicator.block != NULL)
		- bar->output->workspaces.len;
	int32_t w = 0;
	for (size_t i = bar->_.layout_root->composite.children.len - 1; i > last_status_block_idx; --i) {
		struct sw_json_surface_block *block = array_struct_sw_json_surface_block_ptr_get(
			&bar->_.layout_root->composite.children, i);
		w += block->box.width;
	}

	struct sw_json_surface_block *last_status_block = array_struct_sw_json_surface_block_ptr_get(
		&bar->_.layout_root->composite.children, last_status_block_idx);
	bar->status_line_i3bar_use_short_text = (last_status_block->box.x <= w) ? true : false;
	bar->status_line_i3bar_test_short_text = false;

	bar->_.updated_callback = NULL;
	bar->dirty = true;
}

static void describe_status_line(struct bar *bar) {
	switch (state.status->protocol) {
	case STATUS_LINE_PROTOCOL_ERROR:
	case STATUS_LINE_PROTOCOL_TEXT:
		if (state.status->buf.idx > 0) {
			struct surface_block *block = surface_block_create();
			surface_block_init_text(&block->_, &(string_t){
				.s = (char *)state.status->buf.data,
				.len = state.status->buf.idx,
				.free_contents = false,
				.nul_terminated = false,
			});
			block->_.expand = SW_SURFACE_BLOCK_EXPAND_TOP | SW_SURFACE_BLOCK_EXPAND_BOTTOM;
			block->_.anchor = SW_SURFACE_BLOCK_ANCHOR_RIGHT;
			if (state.status->protocol == STATUS_LINE_PROTOCOL_TEXT) {
				block->_.text.color = bar->output->focused ?
					state.config.colors.focused_statusline : state.config.colors.statusline;
			} else {
				block->_.text.color = STATUS_ERROR_TEXT_COLOR;
			}
			block->_.border_left.width = STRING_LITERAL(STRINGIFY(STATUS_MARGIN_LEFT));
			block->_.border_right.width = STRING_LITERAL(STRINGIFY(STATUS_MARGIN_RIGHT));
			if (state.config.status_padding > 0) {
				string_t s;
				string_init_format(&s, "%d", state.config.status_padding);
				block->_.border_top.width = s;
				block->_.border_bottom.width = string_copy(s);
			}
			array_struct_sw_json_surface_block_ptr_add(&bar->_.layout_root->composite.children, &block->_);
		}
		break;
	case STATUS_LINE_PROTOCOL_I3BAR: {
		bool32_t edge = (bar->_.layout_root->composite.children.len == 1);
		bool32_t invisible = false;
		if (bar->status_line_i3bar_test_short_text) {
			for (size_t i = 0; i < state.status->blocks.len; ++i) {
				struct status_line_i3bar_block *i3bar_block = array_struct_status_line_i3bar_block_get_ptr(
					&state.status->blocks, i);
				if (i3bar_block->short_text.len > 0) {
					bar->_.updated_callback = status_line_i3bar_handle_test_short_text;
					invisible = true;
					break;
				}
			}
		}

		for (size_t i = state.status->blocks.len - 1; i != SIZE_MAX; --i) {
			struct status_line_i3bar_block *i3bar_block = array_struct_status_line_i3bar_block_get_ptr(
				&state.status->blocks, i);
			struct surface_block *block = surface_block_create();
			if (state.status->click_events && (i3bar_block->name.len > 0)) {
				block->i3bar_block = i3bar_block;
				block->type = SURFACE_BLOCK_TYPE_STATUS_LINE_I3BAR;
			}

			switch (i3bar_block->type) {
			case STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR:
				if (edge && (state.config.status_edge_padding > 0)) {
					struct surface_block *spacer = surface_block_create();
					//spacer->_.type = SW_SURFACE_BLOCK_TYPE_SPACER;
					spacer->_.anchor = SW_SURFACE_BLOCK_ANCHOR_RIGHT;
					spacer->_.expand = SW_SURFACE_BLOCK_EXPAND_TOP | SW_SURFACE_BLOCK_EXPAND_BOTTOM;
					string_init_format(&spacer->_.min_width, "%d", state.config.status_edge_padding);
					array_struct_sw_json_surface_block_ptr_add(&bar->_.layout_root->composite.children, &spacer->_);
				} else if (!edge && ((i3bar_block->separator_block_width > 0)
							|| i3bar_block->separator)) {
					struct surface_block *separator = surface_block_create();
					separator->_.anchor = SW_SURFACE_BLOCK_ANCHOR_RIGHT;
					if (i3bar_block->separator) {
						if (state.config.separator_symbol.len > 0) {
							surface_block_init_text(&separator->_, &state.config.separator_symbol);
							if (!invisible) {
								separator->_.text.color = bar->output->focused ?
									state.config.colors.focused_separator : state.config.colors.separator;
							}
							if (i3bar_block->separator_block_width > 0) {
								string_init_format(&separator->_.min_width, "%d", i3bar_block->separator_block_width);
								separator->_.content_anchor = SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
							}
						} else {
							//separator->_.type = SW_SURFACE_BLOCK_TYPE_SPACER;
							int32_t width = MAX(i3bar_block->separator_block_width, STATUS_SEPARATOR_WIDTH);
							if (width > 0) {
								string_t s;
								string_init_format(&s, "%d", width);
								separator->_.min_width = s;
								separator->_.max_width = string_copy(s);
							}
							if (!invisible) {
								separator->_.color = bar->output->focused ?
									state.config.colors.focused_separator : state.config.colors.separator;
							}
							int32_t border = (i3bar_block->separator_block_width - STATUS_SEPARATOR_WIDTH) / 2;
							string_t s;
							string_init_format(&s, "%d", border);
							separator->_.border_left.width = s;
							if ((i3bar_block->separator_block_width - STATUS_SEPARATOR_WIDTH - border - border) == 1) {
								string_init_format(&separator->_.border_right.width, "%d", border + 1);
							} else {
								separator->_.border_right.width = string_copy(s);
							}
						}
					} else {
						//separator->_.type = SW_SURFACE_BLOCK_TYPE_SPACER;
						if (i3bar_block->separator_block_width > 0) {
							string_init_format(&separator->_.min_width, "%d", i3bar_block->separator_block_width);
						}
					}
					separator->_.expand = SW_SURFACE_BLOCK_EXPAND_TOP | SW_SURFACE_BLOCK_EXPAND_BOTTOM;
					array_struct_sw_json_surface_block_ptr_add(&bar->_.layout_root->composite.children, &separator->_);
				}

				block->_.type = SW_SURFACE_BLOCK_TYPE_COMPOSITE;
				array_struct_sw_json_surface_block_ptr_init(&block->_.composite.children, 1);
				block->_.anchor = SW_SURFACE_BLOCK_ANCHOR_RIGHT;
				if (i3bar_block->min_width_str.len > 0) {
					struct surface_block *min_width = surface_block_create();
					min_width->_.id = (uint64_t)i + 2;
					surface_block_init_text(&min_width->_, &i3bar_block->min_width_str);
					min_width->_.text.color.u32 = 0;
					min_width->_.anchor = SW_SURFACE_BLOCK_ANCHOR_NONE;
					array_struct_sw_json_surface_block_ptr_add(&bar->_.layout_root->composite.children, &min_width->_);

					string_init_format(&block->_.min_width, "block_%lu_content_width", min_width->_.id);
				} else if (i3bar_block->min_width > 0) {
					string_init_format(&block->_.min_width, "%d", i3bar_block->min_width);
				}
				if (!invisible) {
					block->_.color = i3bar_block->urgent ?
						state.config.colors.urgent_workspace.background : i3bar_block->background_color;
				}
				block->_.expand = SW_SURFACE_BLOCK_EXPAND_TOP | SW_SURFACE_BLOCK_EXPAND_BOTTOM;
				block->_.content_anchor = i3bar_block->content_anchor;
				if (i3bar_block->border_color_set || i3bar_block->urgent) {
					for (size_t j = 0; j < LENGTH(block->_.borders); ++j) {
						if (i3bar_block->border_widths[j] > 0) {
							string_init_format(&block->_.borders[j].width, "%d", i3bar_block->border_widths[j]);
							if (!invisible) {
								block->_.borders[j].color = i3bar_block->urgent ?
									state.config.colors.urgent_workspace.border : i3bar_block->border_color;
							}
						}
					}
				}

				struct surface_block *text = surface_block_create();
				surface_block_init_text(&text->_,
						(bar->status_line_i3bar_use_short_text && (i3bar_block->short_text.len > 0))
							? &i3bar_block->short_text
							: &i3bar_block->full_text);
				if (!invisible) {
					text->_.text.color = i3bar_block->urgent ? state.config.colors.urgent_workspace.text :
						(i3bar_block->text_color_set ? i3bar_block->text_color :
						(bar->output->focused ? state.config.colors.focused_statusline : state.config.colors.statusline));
				}
				text->_.border_left.width = STRING_LITERAL(STRINGIFY(STATUS_MARGIN_LEFT));
				text->_.border_right.width = STRING_LITERAL(STRINGIFY(STATUS_MARGIN_RIGHT));
				if (state.config.status_padding > 0) {
					string_t s;
					string_init_format(&s, "%d", state.config.status_padding);
					text->_.border_top.width = s;
					text->_.border_bottom.width = string_copy(s);
				}

				array_struct_sw_json_surface_block_ptr_add(&block->_.composite.children, &text->_);

				array_struct_sw_json_surface_block_ptr_add(&bar->_.layout_root->composite.children, &block->_);
				break;
			case STATUS_LINE_I3BAR_BLOCK_TYPE_SW: {
				block->_.raw = true;
				block->_.json_raw = string_copy(i3bar_block->json_raw);
				array_struct_sw_json_surface_block_ptr_add(&bar->_.layout_root->composite.children, &block->_);
				break;
			}
			default:
				ASSERT_UNREACHABLE;
			}

			edge = false;
		}
		bar->status_line_i3bar_test_short_text = true;
		bar->status_line_i3bar_use_short_text = false;
		break;
	}
	case STATUS_LINE_PROTOCOL_UNDEF:
	default:
		break;
	}
}

static bool32_t parse_json_color(string_t str, union sw_color *dest) {
	if (str.len == 0) {
		return false;
	}

	if (str.s[0] == '#') {
		str.s++;
		str.len--;
	}

	if ((str.len != 8) && (str.len != 6)) {
		return false;
	}

	uint32_t rgba;
	if (!string_hex_to_uint32(&rgba, str)) {
		return false;
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

	return true;
}

static bool32_t status_line_i3bar_block_init(struct status_line_i3bar_block *block, struct json_ast_node *json) {
	if (json->type != JSON_AST_NODE_TYPE_OBJECT) {
		return false;
	}


	block->type = STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR;
	block->full_text = (string_t){ 0 };
	block->short_text = (string_t){ 0 };
	block->text_color_set = false;
	block->text_color.u32 = 0;
	block->background_color.u32 = 0;
	block->border_color_set = false;
	block->text_color.u32 = 0;
	block->border_top = 1;
	block->border_bottom = 1;
	block->border_left = 1;
	block->border_right = 1;
	block->min_width = 0;
	block->min_width_str = (string_t){ 0 };
	block->content_anchor = SW_SURFACE_BLOCK_CONTENT_ANCHOR_LEFT_CENTER;
	block->name = (string_t){ 0 };
	block->instance = (string_t){ 0 };
	block->urgent = false;
	block->separator = true;
	block->separator_block_width = 9;

	for (size_t i = 0; i < json->object.len; ++i) {
		struct json_ast_key_value *key_value = arena_array_struct_json_ast_key_value_get_ptr(&json->object, i);
		if (string_equal(key_value->key, STRING_LITERAL("_sw"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_OBJECT) {
				string_fini(&block->full_text);
				string_fini(&block->short_text);
				string_fini(&block->min_width_str);

				enum json_writer_state tmp = JSON_WRITER_STATE_ROOT;
				if (state.status->writer.state.data.len == 1) {
					tmp = stack_enum_json_writer_state_pop(&state.status->writer.state);
				}

				json_writer_ast_node(&state.status->writer, &key_value->value);

				string_init_len(&block->json_raw, state.status->writer.buf.data, state.status->writer.buf.idx, false);
				block->type = STATUS_LINE_I3BAR_BLOCK_TYPE_SW;

				if (tmp != JSON_WRITER_STATE_ROOT) {
					stack_enum_json_writer_state_push(&state.status->writer.state, tmp);
				}
				json_writer_reset(&state.status->writer);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("full_text"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_STRING)
					&& (key_value->value.s.len > 0)) {
				string_init_string(&block->full_text, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("short_text"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_STRING)
					&& (key_value->value.s.len > 0)) {
				string_init_string(&block->short_text, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("color"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_STRING)) {
				block->text_color_set = parse_json_color(
					key_value->value.s, &block->text_color);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("background"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_STRING)) {
				parse_json_color(
					key_value->value.s, &block->background_color);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("border"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_STRING)) {
				block->border_color_set = parse_json_color(
					key_value->value.s, &block->border_color);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("border_top"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
				block->border_top = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("border_bottom"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
				block->border_bottom = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("border_left"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
				block->border_left = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("border_right"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
				block->border_right = (int32_t)key_value->value.u;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("min_width"))) {
			if (block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR) {
				if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
					block->min_width = (int32_t)key_value->value.u;
				} else if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING)
						&& (key_value->value.s.len > 0)) {
					string_init_string( &block->min_width_str,
						key_value->value.s);
				}
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("align"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_STRING)
					&& (key_value->value.s.len > 0)) {
				if (string_equal(key_value->value.s,
						STRING_LITERAL("center"))) {
					block->content_anchor = SW_SURFACE_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
				} else if (string_equal(key_value->value.s,
						STRING_LITERAL("right"))) {
					block->content_anchor = SW_SURFACE_BLOCK_CONTENT_ANCHOR_RIGHT_CENTER;
				}
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("name"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->name, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("instance"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.s.len > 0)) {
				string_init_string(&block->instance, key_value->value.s);
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("urgent"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_BOOL)) {
				block->urgent = key_value->value.b;
			}
		} else if (string_equal(key_value->key, STRING_LITERAL("separator"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_BOOL)) {
				block->separator = key_value->value.b;
			}
		} else if (string_equal(key_value->key,
				STRING_LITERAL("separator_block_width"))) {
			if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR)
					&& (key_value->value.type == JSON_AST_NODE_TYPE_UINT)) {
				block->separator_block_width = (int32_t)key_value->value.u;
			}
		}
		// TODO: markup
	}

	if ((block->type == STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR) && (block->full_text.len == 0)) {
		status_line_i3bar_block_fini(block);
		return false;
	}

	return true;
}

static void status_line_i3bar_block_fini(struct status_line_i3bar_block *block) {
	switch (block->type) {
	case STATUS_LINE_I3BAR_BLOCK_TYPE_I3BAR:
		string_fini(&block->full_text);
		string_fini(&block->short_text);
		string_fini(&block->min_width_str);
		break;
	case STATUS_LINE_I3BAR_BLOCK_TYPE_SW:
		string_fini(&block->json_raw);
		break;
	default:
		ASSERT_UNREACHABLE;
	}

	string_fini(&block->name);
	string_fini(&block->instance);
}

static void status_line_i3bar_parse_json(struct json_ast_node *json) {
	if (json->type != JSON_AST_NODE_TYPE_ARRAY) {
		return;
	}

	for (size_t i = 0; i < state.status->blocks.len; ++i) {
		status_line_i3bar_block_fini(
			array_struct_status_line_i3bar_block_get_ptr(&state.status->blocks, i));
	}
	state.status->blocks.len = 0;

	struct status_line_i3bar_block block;
	for (size_t i = 0; i < json->array.len; ++i) {
		if (status_line_i3bar_block_init(&block,
				arena_array_struct_json_ast_node_get_ptr(&json->array, i))) {
			array_struct_status_line_i3bar_block_add(&state.status->blocks, block);
		}
	}
}

static bool32_t status_line_i3bar_process(void) {
    for (;;) {
		ssize_t read_bytes = read(state.status->read_fd, &state.status->buf.data[state.status->buf.idx],
			state.status->buf.size - state.status->buf.idx);
		if (read_bytes <= 0) {
			if (read_bytes == 0) {
				errno = EPIPE;
			}
			if (errno == EAGAIN) {
				break;
			} else if (errno == EINTR) {
				continue;
			} else {
				status_line_set_error(STRING_LITERAL("[error reading from status command]"));
				return true;
			}
		} else {
			state.status->buf.idx += (size_t)read_bytes;
			if (state.status->buf.idx == state.status->buf.size) {
				state.status->buf.size *= 2;
				state.status->buf.data = realloc(state.status->buf.data, state.status->buf.size);
			}
		}
    }

	if (state.status->buf.idx == 0) {
		return false;
	}

	string_t str = {
		.s = (char *)state.status->buf.data,
		.len = state.status->buf.idx,
	};
	json_tokener_set_string(&state.status->tokener, str);

	struct json_token token;
	enum json_tokener_state_ state_;
	if (state.status->tokener.depth == 0) {
		state_ = json_tokener_next(&state.status->tokener, &token);
		if (state_ == JSON_TOKENER_STATE_MORE_DATA_EXPECTED) {
			state.status->buf.idx = 0;
			return false;
		} else if ((state_ != JSON_TOKENER_STATE_SUCCESS) || (token.type != JSON_TOKEN_TYPE_ARRAY_START)) {
			status_line_set_error(STRING_LITERAL("[invalid i3bar json]"));
			return true;
		}
	}

	struct json_ast ast;
	json_ast_reset(&ast);
	for (;;) {
		state_ = json_tokener_ast(&state.status->tokener, &ast, 1, true);
		if ((state.status->tokener.depth != 1) ||
				(state_ == JSON_TOKENER_STATE_ERROR) || (state_ == JSON_TOKENER_STATE_EOF)) {
			status_line_set_error(STRING_LITERAL("[failed to parse i3bar json]"));
			return true;
		} else if (state_ == JSON_TOKENER_STATE_SUCCESS) {
			status_line_i3bar_parse_json(&ast.root);
			json_ast_reset(&ast);
		} else if (state_ == JSON_TOKENER_STATE_MORE_DATA_EXPECTED) {
			break;
		}
	}

	// TODO: rework
	enum json_tokener_state tmp = arena_stack_enum_json_tokener_state_get(&state.status->tokener.state);
	json_tokener_reset(&state.status->tokener);
	arena_stack_enum_json_tokener_state_push(&state.status->tokener.state, &state.status->tokener.allocator, tmp);
	state.status->tokener.depth = 1;

	state.status->buf.idx = 0;
	return true;
}

static bool32_t status_line_process(void) {
	ssize_t read_bytes = 1;
	switch (state.status->protocol) {
	case STATUS_LINE_PROTOCOL_UNDEF:
		errno = 0;
		int available_bytes;
		if (ioctl(state.status->read_fd, FIONREAD, &available_bytes) == -1) {
			status_line_set_error(STRING_LITERAL("[error reading from status command]"));
			return true;
		}

		if (((size_t)available_bytes + 1) > state.status->buf.size) {
			state.status->buf.size = (size_t)available_bytes + 1;
			state.status->buf.data = realloc(state.status->buf.data, state.status->buf.size);
		}

		read_bytes = read(state.status->read_fd, state.status->buf.data, (size_t)available_bytes);
		if (read_bytes != available_bytes) {
			status_line_set_error(STRING_LITERAL("[error reading from status command]"));
			return true;
		}

		struct json_ast header;
		json_ast_reset(&header);

		string_t str = {
			.s = (char *)state.status->buf.data,
			.len = (size_t)read_bytes,
		};
		string_t newline;
		if ((str.len > 0) && string_find_char(str, '\n', &newline)) {
			json_tokener_reset(&state.tokener);
			json_tokener_set_string(&state.tokener, str);

			bool32_t valid = false;
			if ((json_tokener_ast(&state.tokener, &header, 0, true) != JSON_TOKENER_STATE_SUCCESS)
					|| (header.root.type != JSON_AST_NODE_TYPE_OBJECT)) {
				goto protocol_text;
			}

			for (size_t i = 0; i < header.root.object.len; ++i) {
				struct json_ast_key_value *key_value = arena_array_struct_json_ast_key_value_get_ptr(&header.root.object, i);
				if (string_equal(key_value->key, STRING_LITERAL("version"))) {
					if ((key_value->value.type != JSON_AST_NODE_TYPE_UINT) || (key_value->value.u != 1)) {
						goto protocol_text;
					}
					valid = true;
				} else if (string_equal(key_value->key, STRING_LITERAL("click_events"))) {
					if (key_value->value.type == JSON_AST_NODE_TYPE_BOOL) {
						state.status->click_events = key_value->value.b;
					}
				} else if (string_equal(key_value->key, STRING_LITERAL("float_event_coords"))) {
					if (key_value->value.type == JSON_AST_NODE_TYPE_BOOL) {
						state.status->float_event_coords = key_value->value.b;
					}
				} else if (string_equal(key_value->key, STRING_LITERAL("stop_signal"))) {
					if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
						state.status->stop_signal = (int)key_value->value.u;
					}
				} else if (string_equal(key_value->key, STRING_LITERAL("cont_signal"))) {
					if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
						state.status->cont_signal = (int)key_value->value.u;
					}
				}
			}
			if (!valid) {
				goto protocol_text;
			}

			state.status->protocol = STATUS_LINE_PROTOCOL_I3BAR;
			json_writer_init(&state.status->writer, 1024);
			json_tokener_init(&state.status->tokener);
			array_struct_status_line_i3bar_block_init(&state.status->blocks, 32);
			state.status->buf.idx = newline.len - 1;
			memmove(state.status->buf.data, newline.s + 1, state.status->buf.idx);

			if (state.status->click_events) {
				json_writer_array_begin(&state.status->writer);
				json_buffer_add_char(&state.status->writer.buf, '\n');
				if (write(state.status->write_fd, state.status->writer.buf.data, 2) != 2) {
					status_line_set_error(STRING_LITERAL("[failed to write to status command]"));
					return true;
				}
				json_writer_reset(&state.status->writer);
			}

			return status_line_i3bar_process();
		}

protocol_text:

		state.status->read = fdopen(state.status->read_fd, "r");
		if (!state.status->read) {
			abort_(errno, "fdopen: %s", strerror(errno));
		}

		state.status->protocol = STATUS_LINE_PROTOCOL_TEXT;
		state.status->buf.idx = (size_t)read_bytes;

		ATTRIB_FALLTHROUGH;
	case STATUS_LINE_PROTOCOL_TEXT:
		for (;;) {
			if ((read_bytes > 0) && (state.status->buf.idx > 0) && (state.status->buf.data[read_bytes - 1] == '\n')) {
				state.status->buf.idx--;
			}
			errno = 0;
			read_bytes = getline((char **)&state.status->buf.data,
					&state.status->buf.size, state.status->read);
			if (read_bytes == -1) {
				if (errno && (errno != EAGAIN)) {
					status_line_set_error(STRING_LITERAL("[error reading from status command]"));
				} else {
					clearerr(state.status->read);
				}
				return true;
			}
			state.status->buf.idx = (size_t)read_bytes;
		}
	case STATUS_LINE_PROTOCOL_I3BAR:
		return status_line_i3bar_process();
	case STATUS_LINE_PROTOCOL_ERROR:
	default:
		ASSERT_UNREACHABLE;
	}

	return false;
}

static void bar_update(struct bar *bar) {
	if ((state.config.mode == CONFIG_MODE_OVERLAY) || (state.config.mode == CONFIG_MODE_HIDE)) {
		bar->_.layer.layer = SW_SURFACE_LAYER_LAYER_OVERLAY;
		bar->_.layer.exclusive_zone = -1;
		if (state.config.mode == CONFIG_MODE_OVERLAY) {
			bar->_.input_regions.len = 1;
			array_struct_sw_json_region_clear(&bar->_.input_regions);
		}
	} else {
		bar->_.layer.layer = SW_SURFACE_LAYER_LAYER_BOTTOM;
		bar->_.layer.exclusive_zone = INT_MIN;
		bar->_.input_regions.len = 0;
	}

	bar->_.desired_height = state.config.height;
	bar->_.layer.anchor = state.config.position;
	memcpy(bar->_.layer.margins, state.config.gaps, sizeof(bar->_.layer.margins));

	for (size_t i = 1; i < bar->_.layout_root->composite.children.len; ++i) {
		surface_block_unref((struct surface_block *)
			array_struct_sw_json_surface_block_ptr_get(&bar->_.layout_root->composite.children, i));
	}
	bar->_.layout_root->composite.children.len = 1;

	bar->_.layout_root->color = bar->output->focused ? state.config.colors.focused_background : state.config.colors.background;

#if HAVE_TRAY
	if (state.tray) {
		bool32_t visible = (state.config.tray_outputs.len > 0) ? false : true;
		for (size_t i = 0; i < state.config.tray_outputs.len; ++i) {
			if (string_equal(bar->output->_.name, array_string_t_get(&state.config.tray_outputs, i))) {
				visible = true;
				break;
			}
		}
		if (visible) {
			for (size_t i = 0; i < sni_server->host.items.len; ++i) {
				struct tray_item *item = (struct tray_item *)array_struct_sni_item_ptr_get(
					&sni_server->host.items, i);
				struct surface_block *block = surface_block_ref(item->block);
				array_struct_sw_json_surface_block_ptr_add(&bar->_.layout_root->composite.children, &block->_);
			}
		}
	}
#endif // HAVE_TRAY

	if (state.status) {
		describe_status_line(bar);
	}
	if (state.config.workspace_buttons) {
		for (size_t i = 0; i < bar->output->workspaces.len; ++i) {
			struct workspace ws = array_struct_workspace_get(&bar->output->workspaces, i);
			struct surface_block *block = surface_block_ref(ws.block);
			array_struct_sw_json_surface_block_ptr_add(&bar->_.layout_root->composite.children, &block->_);
		}
	}
	if (state.config.binding_mode_indicator && state.binding_mode_indicator.block) {
		struct surface_block *block = surface_block_ref(state.binding_mode_indicator.block);
		array_struct_sw_json_surface_block_ptr_add(&bar->_.layout_root->composite.children, &block->_);
	}

	sw_json_set_dirty(state.sw);
}

static void update_config(string_t s) {
	json_tokener_reset(&state.tokener);
	json_tokener_set_string(&state.tokener, s);

	struct json_token token;
	JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_OBJECT_START);

	// ? TODO: handle different order

	JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_KEY);
	if (string_equal(token.s, STRING_LITERAL("success"))) {
		JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_BOOL);
		assert(token.b == false);
		JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
		assert(string_equal(token.s, STRING_LITERAL("error")));
		JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
		abort_(1, STRING_FMT, STRING_ARGS(token.s));
	}
	assert(string_equal(token.s, STRING_LITERAL("id")));

	JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);

	if (!string_equal(state.bar_id, token.s)) {
		return;
	}

	string_t old_status_command = state.config.status_command, new_status_command = { 0 };

	state.config.mode = CONFIG_MODE_DOCK;
	state.config.hidden_state = CONFIG_HIDDEN_STATE_HIDE;
	state.config.position = (SW_SURFACE_LAYER_ANCHOR_BOTTOM | SW_SURFACE_LAYER_ANCHOR_LEFT | SW_SURFACE_LAYER_ANCHOR_RIGHT);
	state.config.status_command = (string_t){ 0 };
	//string_fini(&state.config.font);
	state.config.font = STRING_LITERAL(FONT);
	memset(state.config.gaps, 0, sizeof(state.config.gaps));
	string_fini(&state.config.separator_symbol);
	state.config.separator_symbol = (string_t){ 0 };
	state.config.height = -1;
	state.config.status_padding = 1;
	state.config.status_edge_padding = 3;
	state.config.wrap_scroll = false;
	state.config.workspace_buttons = true;
	state.config.strip_workspace_numbers = false;
	state.config.strip_workspace_name = false;
	state.config.workspace_min_width = 0;
	state.config.binding_mode_indicator = true;
	state.config.pango_markup = false;
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
		struct binding binding = array_struct_binding_get(&state.config.bindings, i);
		string_fini(&binding.command);
	}
	state.config.bindings.len = 0;
	for (size_t i = 0; i < state.config.outputs.len; ++i) {
		string_fini(array_string_t_get_ptr(&state.config.outputs, i));
	}
	state.config.outputs.len = 0;

#if HAVE_TRAY
	bool32_t tray_enabled = true;
	for (size_t i = 0; i < state.config.tray_outputs.len; ++i) {
		string_fini(array_string_t_get_ptr(&state.config.tray_outputs, i));
	}
	state.config.tray_outputs.len = 0;
	state.config.tray_bindings.len = 0;
	string_fini(&state.config.tray_icon_theme);
	state.config.tray_icon_theme = (string_t){ 0 };
	state.config.tray_padding = 2;
#endif // HAVE_TRAY

	enum json_tokener_state_ state_;
	while (((state_ = json_tokener_next(&state.tokener, &token)) == JSON_TOKENER_STATE_SUCCESS)
			&& (state.tokener.depth > 0)) {
		if ((state.tokener.depth != 1) || (token.type != JSON_TOKEN_TYPE_KEY)) {
			continue;
		}
		if (string_equal(token.s, STRING_LITERAL("mode"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
			if (string_equal(token.s, STRING_LITERAL("hide"))) {
				state.config.mode = CONFIG_MODE_HIDE;
			} else if (string_equal(token.s, STRING_LITERAL("invisible"))) {
				state.config.mode = CONFIG_MODE_INVISIBLE;
			} else if (string_equal(token.s, STRING_LITERAL("overlay"))) {
				state.config.mode = CONFIG_MODE_OVERLAY;
			}
		} else if (string_equal(token.s, STRING_LITERAL("hidden_state"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
			if (string_equal(token.s, STRING_LITERAL("show"))) {
				state.config.hidden_state = CONFIG_HIDDEN_STATE_SHOW;
			}
		} else if (string_equal(token.s, STRING_LITERAL("position"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
			if (string_equal(token.s, STRING_LITERAL("top"))) {
				state.config.position = (SW_SURFACE_LAYER_ANCHOR_TOP | SW_SURFACE_LAYER_ANCHOR_LEFT | SW_SURFACE_LAYER_ANCHOR_RIGHT);
			}
		} else if (string_equal(token.s, STRING_LITERAL("status_command"))) {
			JSON_TOKENER_ADVANCE_ASSERT(state.tokener, token);
			if ((token.type == JSON_TOKEN_TYPE_STRING) && (token.s.len > 0)) {
				string_init_len(&new_status_command, token.s.s, token.s.len, true);
			}
		}
		// TODO: font
		else if (string_equal(token.s, STRING_LITERAL("gaps"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_OBJECT_START);
			while (((state_ = json_tokener_next(&state.tokener, &token)) == JSON_TOKENER_STATE_SUCCESS)
					&& (state.tokener.depth > 1)) {
				if ((state.tokener.depth == 2) && (token.type == JSON_TOKEN_TYPE_KEY)) {
					if (string_equal(token.s, STRING_LITERAL("top"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_UINT);
						state.config.gaps[0] = (int32_t)token.i;
					} else if (string_equal(token.s, STRING_LITERAL("right"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_UINT);
						state.config.gaps[1] = (int32_t)token.i;
					} else if (string_equal(token.s, STRING_LITERAL("bottom"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_UINT);
						state.config.gaps[2] = (int32_t)token.i;
					} else if (string_equal(token.s, STRING_LITERAL("left"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_UINT);
						state.config.gaps[3] = (int32_t)token.i;
					}
				}
			}
			assert(state_ == JSON_TOKENER_STATE_SUCCESS);
		} else if (string_equal(token.s, STRING_LITERAL("separator_symbol"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
			string_init_string(&state.config.separator_symbol, token.s);
		} else if (string_equal(token.s, STRING_LITERAL("bar_height"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_UINT);
			state.config.height = (token.u == 0) ? -1 : (int32_t)token.u;
		} else if (string_equal(token.s, STRING_LITERAL("status_padding"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_UINT);
			state.config.status_padding = (int32_t)token.u;
		} else if (string_equal(token.s, STRING_LITERAL("status_edge_padding"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_UINT);
			state.config.status_edge_padding = (int32_t)token.u;
		} else if (string_equal(token.s, STRING_LITERAL("wrap_scroll"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_BOOL);
			state.config.wrap_scroll = token.b;
		} else if (string_equal(token.s, STRING_LITERAL("workspace_buttons"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_BOOL);
			state.config.workspace_buttons = token.b;
		} else if (string_equal(token.s, STRING_LITERAL("strip_workspace_numbers"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_BOOL);
			state.config.strip_workspace_numbers = token.b;
		} else if (string_equal(token.s, STRING_LITERAL("strip_workspace_name"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_BOOL);
			state.config.strip_workspace_name = token.b;
		} else if (string_equal(token.s, STRING_LITERAL("workspace_min_width"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_UINT);
			state.config.workspace_min_width = (int32_t)token.u;
		} else if (string_equal(token.s, STRING_LITERAL("binding_mode_indicator"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_BOOL);
			state.config.binding_mode_indicator = token.b;
		}
		// TODO: pango_markup
		else if (string_equal(token.s, STRING_LITERAL("colors"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_OBJECT_START);
			while (((state_ = json_tokener_next(&state.tokener, &token)) == JSON_TOKENER_STATE_SUCCESS)
					&& (state.tokener.depth > 1)) {
				if ((state.tokener.depth == 2) && (token.type == JSON_TOKEN_TYPE_KEY)) {
					if (string_equal(token.s, STRING_LITERAL("background"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.background);
					} else if (string_equal(token.s, STRING_LITERAL("statusline"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.statusline);
					} else if (string_equal(token.s, STRING_LITERAL("separator"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.separator);
					} else if (string_equal(token.s, STRING_LITERAL("focused_background"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.focused_background);
					} else if (string_equal(token.s, STRING_LITERAL("focused_statusline"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.focused_statusline);
					} else if (string_equal(token.s, STRING_LITERAL("focused_separator"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.focused_separator);
					} else if (string_equal(token.s, STRING_LITERAL("focused_workspace_text"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.focused_workspace.text);
					} else if (string_equal(token.s, STRING_LITERAL("focused_workspace_bg"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.focused_workspace.background);
					} else if (string_equal(token.s, STRING_LITERAL("focused_workspace_border"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.focused_workspace.border);
					} else if (string_equal(token.s, STRING_LITERAL("inactive_workspace_text"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.inactive_workspace.text);
					} else if (string_equal(token.s, STRING_LITERAL("inactive_workspace_bg"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.inactive_workspace.background);
					} else if (string_equal(token.s, STRING_LITERAL("inactive_workspace_border"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.inactive_workspace.border);
					} else if (string_equal(token.s, STRING_LITERAL("active_workspace_text"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.active_workspace.text);
					} else if (string_equal(token.s, STRING_LITERAL("active_workspace_bg"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.active_workspace.background);
					} else if (string_equal(token.s, STRING_LITERAL("active_workspace_border"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.active_workspace.border);
					} else if (string_equal(token.s, STRING_LITERAL("urgent_workspace_text"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.urgent_workspace.text);
					} else if (string_equal(token.s, STRING_LITERAL("urgent_workspace_bg"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.urgent_workspace.background);
					} else if (string_equal(token.s, STRING_LITERAL("urgent_workspace_border"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.urgent_workspace.border);
					} else if (string_equal(token.s, STRING_LITERAL("binding_mode_text"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.binding_mode.text);
					} else if (string_equal(token.s, STRING_LITERAL("binding_mode_bg"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.binding_mode.background);
					} else if (string_equal(token.s, STRING_LITERAL("binding_mode_border"))) {
						JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
						parse_json_color(token.s, &state.config.colors.binding_mode.border);
					}
				}
			}
			assert(state_ == JSON_TOKENER_STATE_SUCCESS);
		} else if (string_equal(token.s, STRING_LITERAL("bindings"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_ARRAY_START);
			if (state.config.bindings.size == 0) {
				array_struct_binding_init(&state.config.bindings, 16);
			}
			while (((state_ = json_tokener_next(&state.tokener, &token)) == JSON_TOKENER_STATE_SUCCESS)
					&& (state.tokener.depth == 3)) {
				assert(token.type == JSON_TOKEN_TYPE_OBJECT_START);
				struct binding binding = { 0 };
				while (((state_ = json_tokener_next(&state.tokener, &token)) == JSON_TOKENER_STATE_SUCCESS)
							&& (state.tokener.depth > 2)) {
					if ((state.tokener.depth == 3) && (token.type == JSON_TOKEN_TYPE_KEY)) {
						if (string_equal(token.s, STRING_LITERAL("event_code"))) {
							JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_UINT);
							binding.event_code = (uint32_t)token.u;
						} else if (string_equal(token.s, STRING_LITERAL("command"))) {
							JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
							string_init_string(&binding.command, token.s);
						} else if (string_equal(token.s, STRING_LITERAL("release"))) {
							JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_BOOL);
							binding.release = token.b;
						}
					}
				}
				assert(state_ == JSON_TOKENER_STATE_SUCCESS);
				array_struct_binding_add(&state.config.bindings, binding);
			}
			assert(state_ == JSON_TOKENER_STATE_SUCCESS);
		} else if (string_equal(token.s, STRING_LITERAL("outputs"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_ARRAY_START);
			if (state.config.outputs.size == 0) {
				array_string_t_init(&state.config.outputs, 16);
			}
			bool32_t all_outputs = false;
			while (((state_ = json_tokener_next(&state.tokener, &token)) == JSON_TOKENER_STATE_SUCCESS)
					&& (state.tokener.depth == 2)) {
				assert(token.type == JSON_TOKEN_TYPE_STRING);
				if (string_equal(token.s, STRING_LITERAL("*"))) {
					all_outputs = true;
				} else if (!all_outputs) {
					string_init_string(
						array_string_t_add_uninitialized(&state.config.outputs),
						token.s);
				}
			}
			assert(state_ == JSON_TOKENER_STATE_SUCCESS);
			if (all_outputs) {
				for (size_t j = 0; j < state.config.outputs.len; ++j) {
					string_fini(array_string_t_get_ptr(&state.config.outputs, j));
				}
				state.config.outputs.len = 0;
			}
		}
#if HAVE_TRAY
		else if (string_equal(token.s, STRING_LITERAL("tray_outputs"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_ARRAY_START);
			if (state.config.tray_outputs.size == 0) {
				array_string_t_init(&state.config.tray_outputs, 16);
			}
			while (((state_ = json_tokener_next(&state.tokener, &token)) == JSON_TOKENER_STATE_SUCCESS)
					&& (state.tokener.depth == 2)) {
				assert(token.type == JSON_TOKEN_TYPE_STRING);
				if (string_equal(token.s, STRING_LITERAL("none"))) {
					tray_enabled = false;
				} else if (tray_enabled) {
					string_init_string(
						array_string_t_add_uninitialized(&state.config.tray_outputs),
						token.s);
				}
			}
			assert(state_ == JSON_TOKENER_STATE_SUCCESS);
			if (!tray_enabled) {
				for (size_t j = 0; j < state.config.tray_outputs.len; ++j) {
					string_fini(array_string_t_get_ptr(&state.config.tray_outputs, j));
				}
				state.config.tray_outputs.len = 0;
			}
		} else if (string_equal(token.s, STRING_LITERAL("tray_bindings"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_ARRAY_START);
			if (state.config.tray_bindings.size == 0) {
				array_struct_tray_binding_init(&state.config.tray_bindings, 16);
			}
			while (((state_ = json_tokener_next(&state.tokener, &token)) == JSON_TOKENER_STATE_SUCCESS)
					&& (state.tokener.depth == 3)) {
				assert(token.type == JSON_TOKEN_TYPE_OBJECT_START);
				struct tray_binding tray_binding = { 0 };
				while (((state_ = json_tokener_next(&state.tokener, &token)) == JSON_TOKENER_STATE_SUCCESS)
							&& (state.tokener.depth > 2)) {
					if ((state.tokener.depth == 3) && (token.type == JSON_TOKEN_TYPE_KEY)) {
						if (string_equal(token.s, STRING_LITERAL("event_code"))) {
							JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_UINT);
							tray_binding.event_code = (uint32_t)token.u;
						} else if (string_equal(token.s, STRING_LITERAL("command"))) {
							JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
							if (string_equal(token.s, STRING_LITERAL("ContextMenu"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_CONTEXT_MENU;
							} else if (string_equal(token.s, STRING_LITERAL("Activate"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_ACTIVATE;
							} else if (string_equal(token.s, STRING_LITERAL("SecondaryActivate"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_SECONDARY_ACTIVATE;
							} else if (string_equal(token.s, STRING_LITERAL("ScrollDown"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_SCROLL_DOWN;
							} else if (string_equal(token.s, STRING_LITERAL("ScrollLeft"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_SCROLL_LEFT;
							} else if (string_equal(token.s, STRING_LITERAL("ScrollRight"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_SCROLL_RIGHT;
							} else if (string_equal(token.s, STRING_LITERAL("ScrollUp"))) {
								tray_binding.command = TRAY_BINDING_COMMAND_SCROLL_UP;
							} else {
								tray_binding.command = TRAY_BINDING_COMMAND_NOP;
							}
						}
					}
				}
				assert(state_ == JSON_TOKENER_STATE_SUCCESS);
				array_struct_tray_binding_add(&state.config.tray_bindings, tray_binding);
			}
			assert(state_ == JSON_TOKENER_STATE_SUCCESS);
		} else if (string_equal(token.s, STRING_LITERAL("icon_theme"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
			if (token.s.len > 0) {
				string_init_string(&state.config.tray_icon_theme, token.s);
			}
		} else if (string_equal(token.s, STRING_LITERAL("tray_padding"))) {
			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_UINT);
			state.config.tray_padding = (int32_t)token.u;
		}
#endif // HAVE_TRAY
	}
	assert(state_ == JSON_TOKENER_STATE_SUCCESS);
	assert(json_tokener_next(&state.tokener, &token) == JSON_TOKENER_STATE_EOF);

	if (state.status && ((new_status_command.len == 0) ||
			!string_equal(new_status_command, old_status_command))) {
		status_line_fini();
	}
	string_fini(&old_status_command);
	state.config.status_command = new_status_command;
	if ((state.status == NULL) && (new_status_command.len > 0)) {
		status_line_init();
	}

#if HAVE_TRAY
	if (tray_enabled && (state.tray == NULL)) {
		tray_init();
	} else if (!tray_enabled && state.tray) {
		tray_fini();
	} else if (state.tray) {
		tray_update();
	}
#endif // HAVE_TRAY

	sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_GET_BINDING_STATE, NULL);
}

static bool32_t bar_visible_on_output(struct sw_json_output *output) {
	bool32_t visible = !((state.config.mode == CONFIG_MODE_INVISIBLE)
		|| ((state.config.hidden_state == CONFIG_HIDDEN_STATE_HIDE) && (state.config.mode == CONFIG_MODE_HIDE)
		&& !state.visible_by_modifier && !state.visible_by_urgency && !state.visible_by_mode));
	if (visible && (state.config.outputs.len > 0)) {
		visible = false;
		for (size_t i = 0; i < state.config.outputs.len; ++i) {
			if (string_equal(output->name, array_string_t_get(&state.config.outputs, i))) {
				visible = true;
				break;
			}
		}
	}

	return visible;
}

static bool32_t bar_process_button_event(struct bar *bar,
		uint32_t code, enum sw_pointer_button_state state_,
		uint32_t serial, int32_t x, int32_t y, struct sw_json_seat *seat) {
#if HAVE_TRAY
	if (state.tray && state.tray->popup && (state.tray->popup->seat == seat)) {
		if (state_ == SW_POINTER_BUTTON_STATE_PRESSED) {
			assert(state.tray->popup->bar->_.popups.len == 1);
			state.tray->popup->bar->_.popups.len = 0;
			tray_dbusmenu_menu_popup_destroy(state.tray->popup);
		}
		return true;
	}
#endif // HAVE_TRAY

	for (size_t i = 1; i < bar->_.layout_root->composite.children.len; ++i) {
		struct surface_block *block = (struct surface_block *)
			array_struct_sw_json_surface_block_ptr_get(&bar->_.layout_root->composite.children, i);
		struct sw_json_surface_block_box box = block->_.box;
		if ((block->data)
				&& (x >= box.x) && (y >= box.y)
				&& (x < (box.x + box.width))
				&& (y < (box.y + box.height))) {
			switch (block->type) {
			case SURFACE_BLOCK_TYPE_WORKSPACE:
				if (workspace_block_pointer_button(block, bar, code, state_)) {
					return true;
				}
				break;
			case SURFACE_BLOCK_TYPE_STATUS_LINE_I3BAR:
				status_line_i3bar_block_pointer_button(block, bar, code, state_, x, y);
				return true;
#if HAVE_TRAY
			case SURFACE_BLOCK_TYPE_TRAY_ITEM:
				tray_item_block_pointer_button(block, bar, code, state_, serial, x, y, seat);
				return true;
			case SURFACE_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM:
#endif // HAVE_TRAY
			case SURFACE_BLOCK_TYPE_BINDING_MODE_INDICATOR:
			case SURFACE_BLOCK_TYPE_DUMMY:
			default:
				ASSERT_UNREACHABLE;
			}
		}
	}

	bool32_t released = (state_ == SW_POINTER_BUTTON_STATE_RELEASED);
	for (size_t i = 0; i < state.config.bindings.len; ++i) {
		struct binding binding = array_struct_binding_get(&state.config.bindings, i);
		if ((binding.event_code == code) && (binding.release == released)) {
			sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_COMMAND, &binding.command);
			return true;
		}
	}

	return false;
}

static void bar_handle_pointer_button(struct sw_json_pointer *pointer) {
	bar_process_button_event((struct bar *)pointer->focused_surface,
		pointer->btn_code, pointer->btn_state, pointer->btn_serial,
		pointer->pos_x, pointer->pos_y, pointer->seat);
}

static void bar_handle_pointer_scroll(struct sw_json_pointer *pointer) {
	uint32_t button_code;
	bool32_t negative = pointer->scroll_vector_length < 0;
	switch (pointer->scroll_axis) {
	case SW_POINTER_AXIS_VERTICAL_SCROLL:
		button_code = negative ? KEY_SCROLL_UP : KEY_SCROLL_DOWN;
		break;
	case SW_POINTER_AXIS_HORIZONTAL_SCROLL:
		button_code = negative ? KEY_SCROLL_LEFT : KEY_SCROLL_RIGHT;
		break;
	default:
		ASSERT_UNREACHABLE;
	}

	struct bar *bar = (struct bar *)pointer->focused_surface;
	if (!bar_process_button_event(bar, button_code, SW_POINTER_BUTTON_STATE_PRESSED, 0,
				pointer->pos_x, pointer->pos_y, pointer->seat)) {
		bar_process_button_event(bar, button_code, SW_POINTER_BUTTON_STATE_RELEASED, 0,
			pointer->pos_x, pointer->pos_y, pointer->seat);
	}
}

static void bar_destroy(struct bar *bar) {
	if (!bar) {
		return;
	}

	if (state.status) {
		size_t bar_count = 0;
		for (struct sw_json_output *output = state.sw->outputs.head; output; output = output->next) {
			bar_count += output->layers.len;
		}
		if (bar_count == 0) {
			kill(-state.status->pid, state.status->stop_signal);
		}
	}

	sw_json_surface_fini(&bar->_);
	sw_json_set_dirty(state.sw);

	free(bar);
}

static void bar_destroy_sw_json(struct sw_json_surface *bar) {
	bar_destroy((struct bar *)bar);
}

static struct bar *bar_create(struct output *output) {
	struct bar *bar = malloc(sizeof(struct bar));
	bar->output = output;
	bar->dirty = true;
	bar->status_line_i3bar_use_short_text = false;
	bar->status_line_i3bar_test_short_text = true;

	struct surface_block *layout_root = surface_block_create();
	layout_root->_.type = SW_SURFACE_BLOCK_TYPE_COMPOSITE;
	array_struct_sw_json_surface_block_ptr_init(&layout_root->_.composite.children, 64);
	layout_root->_.expand = SW_SURFACE_BLOCK_EXPAND_ALL_SIDES_CONTENT;

	sw_json_surface_layer_init(&bar->_, &layout_root->_, state.sw, bar_destroy_sw_json);
	bar->_.pointer_button_callback = bar_handle_pointer_button;
	bar->_.pointer_scroll_callback = bar_handle_pointer_scroll;
	bar->_.desired_width = 0;
	sw_json_set_dirty(state.sw);

	struct surface_block *min_height = surface_block_create();
	surface_block_init_text(&min_height->_, &STRING_LITERAL(" "));
	min_height->_.id = 1;
	min_height->_.anchor = SW_SURFACE_BLOCK_ANCHOR_NONE;
	array_struct_sw_json_surface_block_ptr_add(&bar->_.layout_root->composite.children, &min_height->_);

	if (state.status) {
		size_t bar_count = 0;
		for (struct sw_json_output *output_ = state.sw->outputs.head; output_; output_ = output_->next) {
			bar_count += output_->layers.len;
		}
		if (bar_count == 0) {
			kill(-state.status->pid, state.status->cont_signal);
		}
	}

	return bar;
}

static void process_ipc(void) {
	struct sway_ipc_response *response = sway_ipc_receive(state.poll_fds[POLL_FD_SWAY_IPC].fd);
	if (response == NULL) {
		abort_(errno, "sway_ipc_receive: %s", strerror(errno));
	}

	bool32_t update = false;

	switch (response->type) {
	case SWAY_IPC_MESSAGE_TYPE_EVENT_WORKSPACE:
		sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_GET_WORKSPACES, NULL);
		break;
	case SWAY_IPC_MESSAGE_TYPE_GET_WORKSPACES: {
		for (struct output *output = (struct output *)state.sw->outputs.head;
				output;
				output = (struct output *)output->_.next) {
			output->focused = false;
			for (size_t i = 0; i < output->workspaces.len; ++i) {
				workspace_fini(array_struct_workspace_get_ptr(&output->workspaces, i));
			}
			output->workspaces.len = 0;
		}

		state.visible_by_urgency = false;
		update = true;

		json_tokener_reset(&state.tokener);
		json_tokener_set_string(&state.tokener, response->payload);

		struct json_token token;
		JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_ARRAY_START);

		enum json_tokener_state_ state_;
		while (((state_ = json_tokener_next(&state.tokener, &token)) == JSON_TOKENER_STATE_SUCCESS)
				&& (state.tokener.depth > 0)) {
			struct output *output = NULL;
			struct workspace workspace = { 0 };
			workspace_init(&workspace, &output);
			if (output) {
				workspace.block->workspace = array_struct_workspace_add(&output->workspaces, workspace);
				if (workspace.focused) {
					output->focused = true;
				}
				if (workspace.urgent) {
					state.visible_by_urgency = true;
				}
			} else {
				workspace_fini(&workspace);
			}
		}
		assert(state_ == JSON_TOKENER_STATE_SUCCESS);
		assert(json_tokener_next(&state.tokener, &token) == JSON_TOKENER_STATE_EOF);
		break;
	}
	case SWAY_IPC_MESSAGE_TYPE_EVENT_BAR_STATE_UPDATE: {
		json_tokener_reset(&state.tokener);
		json_tokener_set_string(&state.tokener, response->payload);

		// ? TODO: handle different key order

		struct json_token token;
		JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_OBJECT_START);

		JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_KEY);
		assert(string_equal(token.s, STRING_LITERAL("id")));

		JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);
		if (string_equal(token.s, state.bar_id)) {

			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_KEY);
			assert(string_equal(token.s, STRING_LITERAL("visible_by_modifier")));

			JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_BOOL);
			state.visible_by_modifier = token.b;

			if (state.visible_by_modifier) {
				state.visible_by_mode = false;
				state.visible_by_urgency = false;
			}
			update = true;
		}
		break;
	}
	case SWAY_IPC_MESSAGE_TYPE_EVENT_MODE:
	case SWAY_IPC_MESSAGE_TYPE_GET_BINDING_STATE: {
		json_tokener_reset(&state.tokener);
		json_tokener_set_string(&state.tokener, response->payload);

		// ? TODO: handle different key order

		struct json_token token;
		JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_OBJECT_START);

		JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_KEY);
		assert(string_equal(token.s, (response->type == SWAY_IPC_MESSAGE_TYPE_EVENT_MODE)
				? STRING_LITERAL("change") : STRING_LITERAL("name")));

		JSON_TOKENER_ADVANCE_ASSERT_TYPE(state.tokener, token, JSON_TOKEN_TYPE_STRING);

		state.visible_by_mode = true;
		update = true;

		if ((token.s.len == 0) || string_equal(token.s, STRING_LITERAL("default"))) {
			surface_block_unref(state.binding_mode_indicator.block);
			state.binding_mode_indicator.block = NULL;
			string_fini(&state.binding_mode_indicator.text);
			state.binding_mode_indicator.text = (string_t){ 0 };

			state.visible_by_mode = false;
		} else if (!string_equal(token.s, state.binding_mode_indicator.text)) {
			string_fini(&state.binding_mode_indicator.text);
			string_init_string(&state.binding_mode_indicator.text, token.s);

			if (!state.binding_mode_indicator.block) {
				struct surface_block *block = surface_block_create();
				block->type = SURFACE_BLOCK_TYPE_BINDING_MODE_INDICATOR;
				block->_.type = SW_SURFACE_BLOCK_TYPE_COMPOSITE;
				array_struct_sw_json_surface_block_ptr_init(&block->_.composite.children, 1);
				block->_.expand = SW_SURFACE_BLOCK_EXPAND_TOP | SW_SURFACE_BLOCK_EXPAND_BOTTOM;
				if (state.config.workspace_min_width > 0) {
					string_init_format(&block->_.min_width, "%d", state.config.workspace_min_width);
				}
				block->_.color = state.config.colors.binding_mode.background;
				for (size_t i = 0; i < LENGTH(block->_.borders); ++i) {
					block->_.borders[i].width = STRING_LITERAL(STRINGIFY(BINDING_MODE_INDICATOR_BORDER_WIDTH));
					block->_.borders[i].color = state.config.colors.binding_mode.border;
				}

				struct surface_block *text = surface_block_create();
				surface_block_init_text(&text->_, &state.binding_mode_indicator.text);
				text->_.text.color = state.config.colors.binding_mode.text;
				text->_.border_left.width = STRING_LITERAL(STRINGIFY(BINDING_MODE_INDICATOR_MARGIN_LEFT));
				text->_.border_right.width = STRING_LITERAL(STRINGIFY(BINDING_MODE_INDICATOR_MARGIN_RIGHT));
				text->_.border_bottom.width = STRING_LITERAL(STRINGIFY(BINDING_MODE_INDICATOR_MARGIN_BOTTOM));
				text->_.border_top.width = STRING_LITERAL(STRINGIFY(BINDING_MODE_INDICATOR_MARGIN_TOP));

				array_struct_sw_json_surface_block_ptr_add(&block->_.composite.children, &text->_);

				state.binding_mode_indicator.block = block;
			} else {
				assert(state.binding_mode_indicator.block->_.composite.children.len == 1);
				struct sw_json_surface_block *text = array_struct_sw_json_surface_block_ptr_get(
					&state.binding_mode_indicator.block->_.composite.children, 0);
				text->text.text = string_copy(state.binding_mode_indicator.text);
			}
		}
		// TODO: pango_markup
		break;
	}
	case SWAY_IPC_MESSAGE_TYPE_EVENT_BARCONFIG_UPDATE:
		update_config(response->payload);
		update = true;
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
		for (struct sw_json_output *output = state.sw->outputs.head; output; output = output->next) {
			if (bar_visible_on_output(output)) {
				if (output->layers.len == 0) {
					struct bar *bar = bar_create((struct output *)output);
					array_struct_sw_json_surface_ptr_add(&output->layers, &bar->_);
				} else {
					assert(output->layers.len == 1);
					struct bar *bar = (struct bar *)array_struct_sw_json_surface_ptr_get(&output->layers, 0);
					bar->dirty = true;
				}
			} else if (output->layers.len > 0) {
				assert(output->layers.len == 1);
				bar_destroy((struct bar *)
					array_struct_sw_json_surface_ptr_get(&output->layers, 0));
				output->layers.len = 0;
			}
		}
	}

	sway_ipc_response_free(response);
}

static bool32_t init_sway_ipc(void) {
	if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_GET_BAR_CONFIG, &state.bar_id) == -1) {
		return false;
	}
	struct sway_ipc_response *response = sway_ipc_receive(state.poll_fds[POLL_FD_SWAY_IPC].fd);
	if (response == NULL) {
		return false;
	}
	assert(response->type == SWAY_IPC_MESSAGE_TYPE_GET_BAR_CONFIG);
	update_config(response->payload);
	sway_ipc_response_free(response);

	if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_SUBSCRIBE,
			&STRING_LITERAL("[\"barconfig_update\",\"bar_state_update\",\"mode\",\"workspace\"]")) == -1) {
		return false;
	}

	return true;
}

static void handle_signal(int sig) {
	(void)sig;
	state.running = false;
}

static void setup(int argc, char **argv) {
	char *locale = setlocale(LC_ALL, "");
	if ((locale == NULL) && (strstr(locale, "utf-8") || strstr(locale, "UTF-8"))) {
		abort_(1, "failed to set utf-8 locale");
	}

	char *sw_path = NULL;
	string_t sway_ipc_socket_path = { 0 };

	static const struct option long_options[] = {
		{ "help",    no_argument,       NULL, 'h' },
		{ "version", no_argument,       NULL, 'v' },
		{ "socket",  required_argument, NULL, 's' },
		{ "bar_id",  required_argument, NULL, 'b' },
		{ "sw_path", required_argument, NULL, 'p' },
		{ 0 },
	};
	
	int c;
	while ((c = getopt_long(argc, argv, "hvs:b:p:", long_options, NULL)) != -1) {
		switch (c) {
		case 's':
			sway_ipc_socket_path = (string_t){
				.s = optarg,
				.len = strlen(optarg),
				.free_contents = false,
				.nul_terminated = true,
			};
			break;
		case 'b':
		state.bar_id = (string_t){
				.s = optarg,
				.len = strlen(optarg),
				.free_contents = false,
				.nul_terminated = true,
			};
			break;
		case 'p':
			sw_path = optarg;
			break;
		case 'v':
			abort_(0, "sw-swaybar version " VERSION);
		default:
			abort_(1, "Usage: sw-swaybar [options...]\n"
				"\n"
				"  -h, --help             Show this help message and quit.\n"
				"  -v, --version          Show the version and quit.\n"
				"  -s, --socket <path>    Connect to sway via socket specified in <path>.\n"
				"  -b, --bar_id <id>      Bar ID for which to get the configuration.\n"
				"  -p, --sw_path <path>   Custom path to sw.\n"
				"\n"
				" PLEASE NOTE that you can use swaybar_command field in your\n"
				" configuration file to let sway start sw-swaybar automatically.\n"
				" You should never need to start it manually.\n");
		}
	}

	if (state.bar_id.len == 0) {
		abort_(1, "No bar_id passed. Provide --bar_id or use swaybar_command in sway config file");
	}

	char *cmd[] = { sw_path, NULL };
	state.sw = sw_json_connect(cmd, output_create_sw_json);
	if (state.sw == NULL) {
		abort_(errno, "Failed to initialize sw: %s", strerror(errno));
	}

	state.sw->state_events = true;
	sw_json_set_dirty(state.sw);
	
	json_tokener_init(&state.tokener);

	if (sway_ipc_socket_path.len == 0) {
		if (!sway_ipc_get_socket_path(&sway_ipc_socket_path)) {
			abort_(ESOCKTNOSUPPORT, "Failed to get sway ipc socket path");
		}
	}
	int sway_ipc_fd = sway_ipc_connect(sway_ipc_socket_path);
	if (sway_ipc_fd == -1) {
		abort_(errno, "Failed to connect to sway ipc socket '"STRING_FMT"': %s",
			STRING_ARGS(sway_ipc_socket_path), strerror(errno));
	}

	state.poll_fds[POLL_FD_SW_READ] = (struct pollfd){ .fd = state.sw->read_fd, .events = POLLIN, };
    state.poll_fds[POLL_FD_SW_WRITE] = (struct pollfd){ .fd = -1, .events = POLLOUT, };
    state.poll_fds[POLL_FD_SWAY_IPC] = (struct pollfd){ .fd = sway_ipc_fd, .events = POLLIN, };
    state.poll_fds[POLL_FD_STATUS] = (struct pollfd){ .fd = -1, .events = POLLIN, };
    state.poll_fds[POLL_FD_SNI_SERVER] = (struct pollfd){ .fd = -1, .events = 0, };

	if (!init_sway_ipc()) {
		abort_(errno, "Failed to initialize sway ipc: %s", strerror(errno));
	}
	string_fini(&sway_ipc_socket_path);

	static const struct sigaction sigact = {
		.sa_handler = handle_signal,
	};
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGPIPE, &sigact, NULL);

	state.running = true;
}

static void run(void) {
	while (state.running) {
		switch (sw_json_flush(state.sw)) {
		case -1:
			abort_(errno, "sw_json_flush: %s", strerror(errno));
		case 0:
		state.poll_fds[POLL_FD_SW_WRITE].fd = state.sw->write_fd;
			break;
		default:
			if (state.poll_fds[POLL_FD_SW_WRITE].fd != -1) {
				state.poll_fds[POLL_FD_SW_WRITE].fd = -1;
			}
		}

        int timeout_ms = -1;
		bool32_t timed_out = false;
		(void)timed_out;

#if HAVE_TRAY
		if (state.tray) {
			uint64_t usec;
			int ret = sni_server_get_poll_info(&state.poll_fds[POLL_FD_SNI_SERVER].fd,
				(int *)(void *)&state.poll_fds[POLL_FD_SNI_SERVER].events, &usec);
			if (ret < 0) {
				abort_(-ret, "sni_server_get_poll_info: %s", strerror(-ret));
			}
			switch (usec) {
			case UINT64_MAX:
				timeout_ms = -1;
				break;
			case 0:
				timeout_ms = 0;
				break;
			default: {
				struct timespec now;
				clock_gettime(CLOCK_MONOTONIC, &now);
				uint64_t now_usec = (uint64_t)((now.tv_sec * 1000000) + (now.tv_nsec / 1000));
				timeout_ms = (int)((usec - now_usec) / 1000);
				break;
			}
			}
		}
#endif // HAVE_TRAY

		switch (poll(state.poll_fds, LENGTH(state.poll_fds), timeout_ms)) {
		case -1:
			if (errno != EINTR) {
				abort_(errno, "poll: %s", strerror(errno));
			}
			break;
		case 0:
			timed_out = true;
			break;
		default:
			break;
		}

		static short err = POLLHUP | POLLERR | POLLNVAL;

		if (state.poll_fds[POLL_FD_SW_READ].revents & (state.poll_fds[POLL_FD_SW_READ].events | err)) {
			if (sw_json_process(state.sw) == -1) {
				abort_(errno, "sw_json_process: %s", strerror(errno));
			}
		}

		if (state.poll_fds[POLL_FD_SWAY_IPC].revents & (state.poll_fds[POLL_FD_SWAY_IPC].events | err)) {
			process_ipc();
		}

		if (state.status) {
			if (state.poll_fds[POLL_FD_STATUS].revents & (state.poll_fds[POLL_FD_STATUS].events | err)) {
				if (status_line_process()) {
					bars_set_dirty();
				}
			}
		}

#if HAVE_TRAY
		if (state.tray) {
			if (timed_out || (state.poll_fds[POLL_FD_SNI_SERVER].revents
						& (state.poll_fds[POLL_FD_SNI_SERVER].events | err))) {
				int ret = sni_server_process();
				if (ret < 0) {
					abort_(-ret, "sni_server_process: %s", strerror(-ret));
				}
			}
		}
#endif // HAVE_TRAY

		for (struct sw_json_output *output = state.sw->outputs.head; output; output = output->next) {
			if (output->layers.len > 0) {
				assert(output->layers.len == 1);
				struct bar *bar = (struct bar *)array_struct_sw_json_surface_ptr_get(
					&output->layers, 0);
				if (bar->dirty) {
					bar_update(bar);
					bar->dirty = false;
				}
			}
		}
	}
}

static void cleanup(void) {
	if (state.status) {
		status_line_fini();
	}

#if HAVE_TRAY
	if (state.tray) {
		tray_fini();
	}
#endif // HAVE_TRAY

	close(state.poll_fds[POLL_FD_SWAY_IPC].fd);

#if DEBUG
	for (size_t i = 0; i < state.config.bindings.len; ++i) {
		struct binding binding = array_struct_binding_get(&state.config.bindings, i);
		string_fini(&binding.command);
	}
	array_struct_binding_fini(&state.config.bindings);
	//string_fini(&state.config.font);
	for (size_t i = 0; i < state.config.outputs.len; ++i) {
		string_fini(array_string_t_get_ptr(&state.config.outputs, i));
	}
	array_string_t_fini(&state.config.outputs);

#if HAVE_TRAY
	for (size_t i = 0; i < state.config.tray_outputs.len; ++i) {
		string_fini(array_string_t_get_ptr(&state.config.tray_outputs, i));
	}
	array_string_t_fini(&state.config.tray_outputs);
	array_struct_tray_binding_fini(&state.config.tray_bindings);
	string_fini(&state.config.tray_icon_theme);
#endif // HAVE_TRAY

	string_fini(&state.config.status_command);
	string_fini(&state.config.separator_symbol);

	surface_block_unref(state.binding_mode_indicator.block);
	string_fini(&state.binding_mode_indicator.text);
	string_fini(&state.bar_id);

	json_tokener_fini(&state.tokener);
#endif // DEBUG

	sw_json_disconnect(state.sw);
}

int main(int argc, char **argv) {
	setup(argc, argv);
	run();
	cleanup();

	return EXIT_SUCCESS;
}
