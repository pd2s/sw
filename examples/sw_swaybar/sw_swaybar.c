#define _DEFAULT_SOURCE

#if !defined(DEBUG)
#define DEBUG 1
#endif
#if !defined(WITH_TRAY)
#define WITH_TRAY 1
#endif /* !defined(WITH_TRAY) */

#if !defined(SU_WITH_DEBUG)
#define SU_WITH_DEBUG DEBUG
#endif /* !defined(SU_WITH_DEBUG) */
#define SU_LOG_PREFIX "sw_swaybar: "
#define SU_IMPLEMENTATION
#define SU_STRIP_PREFIXES
#include "../../sutil.h"

#define SW_WITH_MEMORY_BACKEND 0
#define SW_WITH_WAYLAND_BACKEND 1
#define SW_WITH_TEXT 1
#if !defined(SW_WITH_SVG)
#define SW_WITH_SVG 1
#endif /* !defined(SW_WITH_SVG) */
#if !defined(SW_WITH_PNG)
#define SW_WITH_PNG 1
#endif /* !defined(SW_WITH_PNG) */
#define SW_WITH_JPG 0
#define SW_WITH_TGA 0
#define SW_WITH_BMP 0
#define SW_WITH_PSD 0
#define SW_WITH_GIF 0
#define SW_WITH_HDR 0
#define SW_WITH_PIC 0
#define SW_WITH_PNM 0

#if !defined(SW_WITH_DEBUG)
#define SW_WITH_DEBUG DEBUG
#endif /* !defined(SW_WITH_DEBUG) */
#define SW_IMPLEMENTATION
#include "../../swidgets.h"

#include <linux/input-event-codes.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <locale.h>

#include "sway_ipc.h"

#if WITH_TRAY
#if SW_WITH_SVG || SW_WITH_PNG
#define WITH_SVG SW_WITH_SVG
#define WITH_PNG SW_WITH_PNG
#include "xdg_icon_theme.h"
#endif /* SW_WITH_SVG || SW_WITH_PNG */
#include "sni_server.h"
#endif /* WITH_TRAY */

#define KEY_SCROLL_UP KEY_MAX + 1
#define KEY_SCROLL_DOWN KEY_MAX + 2
#define KEY_SCROLL_LEFT KEY_MAX + 3
#define KEY_SCROLL_RIGHT KEY_MAX + 4

typedef struct workspace {
	string_t name;
	int32_t num;
	bool32_t visible;
	bool32_t focused;
	bool32_t urgent;
} workspace_t;

typedef struct output {
	sw_wayland_output_t _; /* must be first */
	workspace_t *workspaces;
	size_t workspaces_count;
	bool32_t focused;
	PAD32;
} output_t;

typedef struct bar {
	sw_wayland_surface_t _; /* must be first */
	bool32_t dirty;
	PAD32;
	/* ? TODO: block arena alloc */
} bar_t;

typedef enum layout_block_type {
	LAYOUT_BLOCK_TYPE_DUMMY,
	LAYOUT_BLOCK_TYPE_WORKSPACE,
	LAYOUT_BLOCK_TYPE_BINDING_MODE_INDICATOR,
	LAYOUT_BLOCK_TYPE_STATUS_LINE_I3BAR
#if WITH_TRAY
	,LAYOUT_BLOCK_TYPE_TRAY_SNI_ITEM
	,LAYOUT_BLOCK_TYPE_TRAY_SNI_ITEM_IMAGE
	,LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM
	,LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM_IMAGE
	,LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM_DECORATOR
#endif /* WITH_TRAY */
} layout_block_type_t;

typedef struct layout_block layout_block_t;

struct layout_block {
	sw_layout_block_t _; /* must be first */
	PAD32;
	layout_block_type_t type;
	void *data;
	layout_block_t *block; /* TODO: rename */
};

typedef struct binding {
	uint32_t event_code;
	bool32_t release;
	string_t command;
} bar_binding_t;

typedef struct box_colors {
	sw_color_argb32_t border;
	sw_color_argb32_t background;
	sw_color_argb32_t text;
} bar_box_colors_t;

#if WITH_TRAY
typedef struct tray_dbusmenu_menu_popup {
	sw_wayland_surface_t _; /* must be first */
	sni_dbusmenu_menu_t *menu;
	sw_wayland_surface_t *parent;
	sw_wayland_seat_t *seat;

	layout_block_t *focused_block;
} tray_dbusmenu_menu_popup_t;

typedef struct tray {
	bool32_t active;
	PAD32;
	tray_dbusmenu_menu_popup_t *popup;
#if SW_WITH_SVG || SW_WITH_PNG
	xdg_icon_theme_cache_t cache;
#endif /* SW_WITH_SVG || SW_WITH_PNG */
} tray_t;

typedef enum tray_binding_command {
	TRAY_BINDING_COMMAND_NONE,
	TRAY_BINDING_COMMAND_CONTEXT_MENU,
	TRAY_BINDING_COMMAND_ACTIVATE,
	TRAY_BINDING_COMMAND_SECONDARY_ACTIVATE,
	TRAY_BINDING_COMMAND_SCROLL_DOWN,
	TRAY_BINDING_COMMAND_SCROLL_LEFT,
	TRAY_BINDING_COMMAND_SCROLL_RIGHT,
	TRAY_BINDING_COMMAND_SCROLL_UP,
	TRAY_BINDING_COMMAND_NOP
} tray_binding_command_t;

typedef struct tray_binding {
	uint32_t event_code;
	tray_binding_command_t command;
} tray_binding_t;

#endif /* WITH_TRAY */

typedef enum status_protocol {
	STATUS_PROTOCOL_UNDEF,
	STATUS_PROTOCOL_ERROR,
	STATUS_PROTOCOL_TEXT,
	STATUS_PROTOCOL_I3BAR
} status_protocol_t;

typedef struct status_i3bar_block {
	string_t name;
	string_t instance;
	string_t full_text;
	string_t short_text;
	bool32_t text_color_set;
	bool32_t urgent;
	bool32_t separator;
	bool32_t border_color_set;
	int32_t min_width;
	sw_color_argb32_t text_color;
	sw_color_argb32_t background_color;
	sw_color_argb32_t border_color;
	string_t min_width_str;
	sw_layout_block_content_anchor_t content_anchor;
	int32_t separator_block_width;
	int32_t border_widths[4]; /* left right bottom top */
} status_i3bar_block_t;

typedef struct status_buffer {
	uint8_t *data;
	size_t size;
	size_t idx;
} status_buffer_t;

typedef struct status {
	bool32_t active;
	int read_fd, write_fd;
	pid_t pid;
	int stop_signal;
	int cont_signal;
	status_buffer_t buf;
	status_protocol_t protocol;

	/* i3bar */
	bool32_t block_clicked;
	PAD32;
	bool32_t click_events;
	bool32_t float_event_coords;
	su__json_tokener_state_t tokener_state;
	status_i3bar_block_t *blocks;
	size_t blocks_count;
} status_t;

typedef enum bar_hidden_state {
	BAR_HIDDEN_STATE_HIDE,
	BAR_HIDDEN_STATE_SHOW
} bar_hidden_state_t;

typedef enum bar_mode {
	BAR_MODE_DOCK,
	BAR_MODE_HIDE,
	BAR_MODE_INVISIBLE,
	BAR_MODE_OVERLAY
} bar_mode_t;

enum {
	POLL_FD_SW,
	POLL_FD_SWAY_IPC,
	POLL_FD_STATUS,
	POLL_FD_SNI_SERVER,
	POLL_FD_LAST
};

typedef struct config_colors {
	sw_color_argb32_t background;
	sw_color_argb32_t statusline;
	sw_color_argb32_t separator;
	sw_color_argb32_t focused_background;
	sw_color_argb32_t focused_statusline;
	sw_color_argb32_t focused_separator;
	bar_box_colors_t focused_workspace;
	bar_box_colors_t inactive_workspace;
	bar_box_colors_t active_workspace;
	bar_box_colors_t urgent_workspace;
	bar_box_colors_t binding_mode;
} config_colors_t;

typedef struct config {
	bar_mode_t mode;
	bar_hidden_state_t hidden_state;
	string_t status_command;
	string_t font;
	int32_t gaps[4]; /* top right bottom left */
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
	uint32_t position; /* sw_wayland_surface_layer_anchor_t | */
	config_colors_t colors;
#if WITH_TRAY
	int32_t tray_padding;
	PAD32;
	string_t *tray_outputs;
	size_t tray_outputs_count;
	tray_binding_t *tray_bindings;
	size_t tray_bindings_count;
	string_t tray_icon_theme;
#endif /* WITH_TRAY */
	bar_binding_t *bindings;
	size_t bindings_count;
	string_t *outputs;
	size_t outputs_count;
} config_t;

typedef struct state {
	sw_context_t sw;
	arena_t scratch_arena;
	struct pollfd poll_fds[POLL_FD_LAST];
	config_t config;
	string_t binding_mode_indicator_text;
	status_t status;
#if WITH_TRAY
	tray_t tray;
#endif /* WITH_TRAY */
	string_t bar_id;
	bool32_t visible_by_urgency;
	bool32_t visible_by_modifier;
	bool32_t visible_by_mode;
	bool32_t update;
	bool32_t running;
	PAD32;
	su_hash_table__su_file_cache_t__t icon_cache;
} state_t;


static state_t state;

static allocator_t gp_alloc = { libc_alloc, libc_free };

static void *scratch_alloc_alloc(allocator_t *alloc, size_t size, size_t alignment) {
	void *ret = arena_alloc(&state.scratch_arena, &gp_alloc, size, alignment);
	NOTUSED(alloc);
	return ret;
}

static void scratch_alloc_free(allocator_t *alloc, void *ptr) {
	NOTUSED(alloc); NOTUSED(ptr);
}

static allocator_t scratch_alloc = { scratch_alloc_alloc, scratch_alloc_free };

static void layout_block_destroy(layout_block_t *block) {
	block->_.out.fini(&block->_, &state.sw);
	FREE(&gp_alloc, block);
}

static bool32_t layout_block_handle_event(sw_layout_block_t *sw_block, sw_context_t *sw, sw_layout_block_event_t event) {
	layout_block_t *block = (layout_block_t *)sw_block;

	NOTUSED(sw);

	switch (event) {
	case SW_LAYOUT_BLOCK_EVENT_DESTROY:
		layout_block_destroy(block);
		break;
	case SW_LAYOUT_BLOCK_EVENT_PREPARE:
		switch (block->type) {
		case LAYOUT_BLOCK_TYPE_DUMMY:
		case LAYOUT_BLOCK_TYPE_WORKSPACE:
		case LAYOUT_BLOCK_TYPE_BINDING_MODE_INDICATOR:
		case LAYOUT_BLOCK_TYPE_STATUS_LINE_I3BAR:
			if (block->block) {
				block->_.in.min_width = block->block->_.out.dim.content_width;	
			}
			break;
#if WITH_TRAY
		case LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM:
			break;
		case LAYOUT_BLOCK_TYPE_TRAY_SNI_ITEM:
			block->_.in.min_width = block->_.in.min_height = block->block->_.out.dim.content_height;
			break;
		case LAYOUT_BLOCK_TYPE_TRAY_SNI_ITEM_IMAGE:
			block->_.in.content_width = block->_.in.content_height =
				(block->block->_.out.dim.content_height - (state.config.tray_padding * 2));
			break;
		case LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM_IMAGE:
			block->_.in.content_width = block->_.in.content_height =
				block->block->_.out.dim.content_height;
			break;
		case LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM_DECORATOR:
			block->_.in.min_width = block->_.in.max_width = block->_.in.min_height = block->_.in.max_height =
				(block->block->_.out.dim.content_height + (state.config.tray_padding * 2));
			break;
#endif /* WITH_TRAY */
		default:
			ASSERT_UNREACHABLE;
		}
		break;
	case SW_LAYOUT_BLOCK_EVENT_PREPARED:
		break;
	case SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_IMAGE:
	case SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_FONT:
	case SW_LAYOUT_BLOCK_EVENT_ERROR_INVALID_TEXT:

		break;
	default:
		ASSERT_UNREACHABLE;
	}

	return TRUE;
}

static layout_block_t *layout_block_create(void) {
	layout_block_t *block;
	ALLOCCT(block, &gp_alloc);
	block->_.in.notify = layout_block_handle_event;

	return block;
}

static void layout_block_init_text(sw_layout_block_t *block, string_t *text) {
	block->in.type = SW_LAYOUT_BLOCK_TYPE_TEXT;
	block->in._.text.font_names = &state.config.font;
	block->in._.text.font_names_count = 1;
	if (text) {
		block->in._.text.text = string_view(*text);
	}
}

static void bars_set_dirty(void) {
	sw_wayland_surface_t *bar = state.sw.in.backend.wayland.layers.head;
	for ( ; bar; bar = bar->next) {
		((bar_t *)bar)->dirty = TRUE;
	}
} 

static bool32_t workspace_block_pointer_button(layout_block_t *block,
		bar_t *bar, uint32_t code, sw_wayland_pointer_button_state_t state_) {
	workspace_t *workspace = NULL;
	size_t len, i, d;
	string_t name, payload;

	if (state_ == SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED) {
		return TRUE;
	}

	switch (code) {
	case BTN_LEFT:
		workspace = (workspace_t *)block->data;
		break;
	case KEY_SCROLL_DOWN:
	case KEY_SCROLL_RIGHT:
	case KEY_SCROLL_LEFT:
	case KEY_SCROLL_UP: {
		output_t *output = (output_t *)bar->_.in._.layer.output;
		for ( i = 0; i < output->workspaces_count; ++i) {
			workspace = &output->workspaces[i];
			if (workspace->visible) {
				workspace_t *first_ws = &output->workspaces[0];
				workspace_t *last_ws = &output->workspaces[output->workspaces_count - 1];
					bool32_t left = ((code == KEY_SCROLL_UP) || (code == KEY_SCROLL_LEFT));
				if (left) {
					if (workspace == first_ws) {
						workspace = state.config.wrap_scroll ? last_ws : NULL;
					} else {
						workspace = &output->workspaces[i - 1];
					}
				} else {
					if (workspace == last_ws) {
						workspace = state.config.wrap_scroll ? first_ws : NULL;
					} else {
						workspace = &output->workspaces[i + 1];
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

	name = workspace->name;
	len = STRING_LITERAL_LENGTH("workspace \"\"") + workspace->name.len;
	for ( i = 0; i < name.len; ++i) {
		if ((name.s[i] == '"') || (name.s[i] == '\\')) {
			len++;
		}
	}

	ALLOCTS(payload.s, &scratch_alloc, len + 1);
	MEMCPY(payload.s, "workspace \"", STRING_LITERAL_LENGTH("workspace \""));
	payload.s[len - 1] = '"';
	payload.len = len;
	payload.free_contents = TRUE;
	payload.nul_terminated = FALSE;

	d = STRING_LITERAL_LENGTH("workspace \"");
	for ( i = 0; i < name.len; ++i) {
		if ((name.s[i] == '"') || (name.s[i] == '\\')) {
			payload.s[d++] = '\\';
		}
		payload.s[d++] = name.s[i];
	}

	if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_COMMAND, &payload) == -1) {
		su_abort(errno, "sway_ipc_send: write: %s", strerror(errno));
	}

	return TRUE;
}

#if WITH_TRAY
#if SW_WITH_SVG || SW_WITH_PNG
static bool32_t tray_find_icon(string_t name, sw_layout_block_image_t *out) {
	config_t *config = &state.config;
	string_t *svgs = NULL, *pngs = NULL;
	size_t svgs_count = 0, pngs_count = 0;
	if (xdg_icon_theme_cache_find_icon(&state.tray.cache, &svgs, &svgs_count, &pngs, &pngs_count,
			name, config->tray_icon_theme.s ? &config->tray_icon_theme : NULL)) {
#if SW_WITH_SVG
		if (svgs) {
			size_t j = (svgs_count - 1);
			for ( ; j != SIZE_MAX; --j) {
				if (read_entire_file_with_cache(svgs[j],
						&out->data, &gp_alloc, &state.icon_cache)) {
					out->type = SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SVG;
					return TRUE;
				}
			}
		}
#endif /* SW_WITH_SVG */
#if SW_WITH_SVG && SW_WITH_PNG
		else
#endif /* SW_WITH_SVG && SW_WITH_PNG */
#if SW_WITH_PNG
		if (pngs) {
			size_t j = (pngs_count - 1);
			for ( ; j != SIZE_MAX; --j) {
				if (read_entire_file_with_cache(pngs[j],
						&out->data, &gp_alloc, &state.icon_cache)) {
					out->type = SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNG;
					return TRUE;
				}
			}
		}
#endif /* SW_WITH_PNG */
	}

	return FALSE;
}
#endif /* SW_WITH_SVG || SW_WITH_PNG */

static bool32_t tray_visible_on_bar(bar_t *bar) {
	config_t *config = &state.config;
	bool32_t visible = TRUE;
	if (config->tray_outputs_count > 0) {
		size_t i;
		visible = FALSE;
		for ( i = 0; i < config->tray_outputs_count; ++i) {
			if (string_equal(bar->_.in._.layer.output->out.name, config->tray_outputs[i])) {
				visible = TRUE;
				break;
			}
		}
	}
	return visible;
}

static void tray_describe_sni_items(bar_t *bar) {
	if (tray_visible_on_bar(bar)) {
		sni_item_t *item = sni_server.out.items.head;
		for ( ; item; item = item->next) {
			sni_item_properties_t *props = item->out.properties;
			sni_pixmap_t *icon_pixmap = NULL;
			sw_layout_block_image_t icon;
			string_t icon_name;

			layout_block_t *block = layout_block_create();
			block->type = LAYOUT_BLOCK_TYPE_TRAY_SNI_ITEM;
			block->data = item;
			/*block->_.in.type = SW_LAYOUT_BLOCK_TYPE_SPACER;*/
			block->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
			block->block = (layout_block_t *)bar->_.in.root->in._.composite.children.head;

			LLIST_APPEND_TAIL(&bar->_.in.root->in._.composite.children, &block->_);

			NOTUSED(icon_name);
			MEMSET(&icon_name, 0, sizeof(icon_name));
			MEMSET(&icon, 0, sizeof(icon));

			if (!props) {
				return;
			}

			switch (props->status) {
			case SNI_ITEM_STATUS_ACTIVE:
				icon_name = props->icon_name;
				if (props->icon_pixmaps_count > 0) {
					icon_pixmap = props->icon_pixmaps[0];
				}
				break;
			case SNI_ITEM_STATUS_NEEDS_ATTENTION:
				icon_name = props->attention_icon_name;
				if (props->attention_icon_pixmaps_count > 0) {
					icon_pixmap = props->attention_icon_pixmaps[0];
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
				tray_find_icon(icon_name, &icon);
			}
#endif /* SW_WITH_SVG || SW_WITH_PNG */

			if ((icon.data.len == 0) && icon_pixmap) {
				icon.type = SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SW_PIXMAP;
				icon.data.ptr = icon_pixmap;
				icon.data.len = (((size_t)icon_pixmap->width * (size_t)icon_pixmap->height * 4) +
					sizeof(*icon_pixmap) - sizeof(icon_pixmap->pixels));
			}

			if (icon.type) {
				layout_block_t *image = layout_block_create();
				image->type = LAYOUT_BLOCK_TYPE_TRAY_SNI_ITEM_IMAGE;
				image->_.in.type = SW_LAYOUT_BLOCK_TYPE_IMAGE;
				image->_.in._.image = icon;
				image->block = (layout_block_t *)bar->_.in.root->in._.composite.children.head;

				block->_.in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
				block->_.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
				block->_.in.expand = (SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM);

				LLIST_APPEND_TAIL(&block->_.in._.composite.children, &image->_);
			}
		}
	}
}

static void tray_dbusmenu_menu_popup_destroy(tray_dbusmenu_menu_popup_t *popup) {
	sni_dbusmenu_menu_item_event(popup->menu->parent_menu_item,
		SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLOSED, TRUE);

	popup->_.out.fini(&popup->_, &state.sw);

	if (popup == state.tray.popup) {
		state.tray.popup = NULL;
	}

	FREE(&gp_alloc, popup);

	state.update = TRUE;
}

static tray_dbusmenu_menu_popup_t *tray_dbusmenu_menu_popup_create(sni_dbusmenu_menu_t *menu,
	int32_t x, int32_t y, sw_wayland_surface_t *parent, sw_wayland_seat_t *seat);

static void tray_dbusmenu_menu_item_pointer_enter(sni_dbusmenu_menu_item_t *menu_item,
		sw_layout_block_t *block, tray_dbusmenu_menu_popup_t *popup) {
	sni_dbusmenu_menu_item_event(menu_item,
		SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_HOVERED, TRUE);

	if (popup->_.in.popups.count > 0) {
		ASSERT(popup->_.in.popups.count == 1);
		tray_dbusmenu_menu_popup_destroy((tray_dbusmenu_menu_popup_t *)popup->_.in.popups.head);
		MEMSET(&popup->_.in.popups, 0, sizeof(popup->_.in.popups));
	}

	if (menu_item->enabled && (menu_item->type != SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR)) {
		if (menu_item->submenu && (menu_item->submenu->menu_items_count > 0)) {
			tray_dbusmenu_menu_popup_t *submenu_popup = tray_dbusmenu_menu_popup_create(
				menu_item->submenu, block->out.dim.x, block->out.dim.y + block->out.dim.height,
				&popup->_, popup->seat);
			LLIST_APPEND_TAIL(&popup->_.in.popups, &submenu_popup->_);
		}

		block->in.color._.argb32 = state.config.colors.focused_separator;

		state.update = TRUE;
	}
}

static void tray_dbusmenu_menu_item_pointer_button(sni_dbusmenu_menu_item_t *menu_item,
		uint32_t code, sw_wayland_pointer_button_state_t state_) {
	if ((code != BTN_LEFT) || (state_ == SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED)) {
		return;
	}

	if ((menu_item->type != SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR) && menu_item->enabled) {
		sni_dbusmenu_menu_item_event(menu_item,
			SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLICKED, TRUE);
#if 1
		ASSERT(state.tray.popup->parent->in.popups.count == 1);
		MEMSET(&state.tray.popup->parent->in.popups, 0, sizeof(state.tray.popup->parent->in.popups));
		tray_dbusmenu_menu_popup_destroy(state.tray.popup);
#endif
	}
}

static void tray_dbusmenu_menu_item_pointer_leave(sni_dbusmenu_menu_item_t *menu_item,
		sw_layout_block_t *block) {
	if ((menu_item->type != SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR) && menu_item->enabled) {
		block->in.color._.argb32 = state.config.colors.focused_background;
		state.update = TRUE;
	}
}

static sw_color_argb32_t tray_dbusmenu_menu_item_get_text_color(sni_dbusmenu_menu_item_t *menu_item) {
	if (menu_item->enabled) {
		return state.config.colors.focused_statusline;
	} else {
		sw_color_argb32_t color = state.config.colors.focused_statusline;
		color.c.a >>= 1;
		return color;
	}
}

static void tray_dbusmenu_menu_popup_pointer_pos(tray_dbusmenu_menu_popup_t *popup,
		int32_t x, int32_t y) {
	sw_layout_block_t *test = popup->_.in.root->in._.composite.children.head;
	sw_layout_block_t *block;
	sni_dbusmenu_menu_item_t *menu_item;

	if ((test->out.dim.width == 0) || (test->out.dim.height == 0)) {
		return;
	}

	for ( block = popup->_.in.root->in._.composite.children.head; block; block = block->next) {
		sw_layout_block_dimensions_t *dim = &block->out.dim;
		if ((x >= dim->x) && (y >= dim->y)
				&& (x < (dim->x + dim->width))
				&& (y < (dim->y + dim->height))) {
			if (popup->focused_block != (layout_block_t *)block) {
				if (popup->focused_block) {
					menu_item = (sni_dbusmenu_menu_item_t *)popup->focused_block->data;
					tray_dbusmenu_menu_item_pointer_leave(menu_item, &popup->focused_block->_);
				}
				menu_item = (sni_dbusmenu_menu_item_t *)((layout_block_t *)block)->data;
				tray_dbusmenu_menu_item_pointer_enter(menu_item, block, popup);
				popup->focused_block = (layout_block_t *)block;
			}
			return;
		}
	}

	if (popup->focused_block) {
		menu_item = (sni_dbusmenu_menu_item_t *)popup->focused_block->data;
		tray_dbusmenu_menu_item_pointer_leave(menu_item, &popup->focused_block->_);
		popup->focused_block = NULL;
	}
}

static void tray_dbusmenu_menu_popup_update(tray_dbusmenu_menu_popup_t *popup, sni_dbusmenu_menu_t *menu) {
	/* TODO: update child popups */

	config_t *config = &state.config;

	bool32_t needs_spacer = FALSE;
	sw_layout_block_t *block_;
	size_t i;

	if (popup->_.in.popups.count > 0) {
		ASSERT(popup->_.in.popups.count == 1);
		tray_dbusmenu_menu_popup_destroy((tray_dbusmenu_menu_popup_t *)popup->_.in.popups.head);
		MEMSET(&popup->_.in.popups, 0, sizeof(popup->_.in.popups));
	}

	for ( block_ = popup->_.in.root->in._.composite.children.head; block_;) {
		sw_layout_block_t *next = block_->next;
		layout_block_destroy((layout_block_t *)block_);
		block_ = next;
	}

	MEMSET(&popup->_.in.root->in._.composite.children, 0, sizeof(popup->_.in.root->in._.composite.children));
	popup->focused_block = NULL;
	popup->menu = menu;

	for ( i = 0; i < menu->menu_items_count; ++i) {
		layout_block_t *block;
		sni_dbusmenu_menu_item_t *menu_item = &menu->menu_items[i];
		if (!menu_item->visible) {
			continue;
		}

		block = layout_block_create();
		block->data = menu_item;
		block->type = LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM;
		block->_.in.expand = (SW_LAYOUT_BLOCK_EXPAND_LEFT | SW_LAYOUT_BLOCK_EXPAND_RIGHT);
		if (menu_item->type == SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR) {
			/*block->_.in.type = SW_LAYOUT_BLOCK_TYPE_SPACER;*/
			block->_.in.min_height = 2;
			block->_.in.color._.argb32 = config->colors.focused_separator;
		} else {
			layout_block_t *label = NULL;

			block->_.in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;

			if (menu_item->label.len > 0) {
				label = layout_block_create();
				layout_block_init_text(&label->_, &menu_item->label);
				label->_.in._.text.color._.argb32 = tray_dbusmenu_menu_item_get_text_color(menu_item);
				label->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				if (config->tray_padding > 0) {
					label->_.in.borders[0].width = config->tray_padding;
					label->_.in.borders[1].width = config->tray_padding;
					label->_.in.borders[2].width = config->tray_padding;
					label->_.in.borders[3].width = config->tray_padding;
				}
				LLIST_APPEND_TAIL(&block->_.in._.composite.children, &label->_);
			}

#if SW_WITH_SVG || SW_WITH_PNG
			if (menu_item->icon_name.len > 0) {
				layout_block_t *icon;
				sni_dbusmenu_t *dbusmenu = menu_item->parent_menu->dbusmenu;
				if (dbusmenu->item->out.properties && (dbusmenu->item->out.properties->icon_theme_path.len > 0)) {
					xdg_icon_theme_cache_add_basedir(&state.tray.cache, &gp_alloc,
						dbusmenu->item->out.properties->icon_theme_path);
				}
				if (dbusmenu->properties) {
					size_t j = 0;
					for ( ; j < dbusmenu->properties->icon_theme_path_count; ++j) {
						xdg_icon_theme_cache_add_basedir( &state.tray.cache,
							&gp_alloc, dbusmenu->properties->icon_theme_path[j]);
					}
				}
				icon = layout_block_create();
				if (config->tray_padding > 0) {
					icon->_.in.borders[0].width = config->tray_padding;
					icon->_.in.borders[1].width = config->tray_padding;
					icon->_.in.borders[2].width = config->tray_padding;
					icon->_.in.borders[3].width = config->tray_padding;
				}
				icon->_.in.type = SW_LAYOUT_BLOCK_TYPE_IMAGE;
				icon->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				if (label) {
					icon->type = LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM_IMAGE;
					icon->block = label;
				}
				tray_find_icon(menu_item->icon_name, &icon->_.in._.image);
				LLIST_APPEND_TAIL(&block->_.in._.composite.children, &icon->_);
			}
#endif /* SW_WITH_SVG || SW_WITH_PNG */

#if SW_WITH_PNG
			if (menu_item->icon_data.nbytes > 0) {
				layout_block_t *icon = layout_block_create();
				if (config->tray_padding > 0) {
					icon->_.in.borders[0].width = config->tray_padding;
					icon->_.in.borders[1].width = config->tray_padding;
					icon->_.in.borders[2].width = config->tray_padding;
					icon->_.in.borders[3].width = config->tray_padding;
				}
				icon->_.in.type = SW_LAYOUT_BLOCK_TYPE_IMAGE;
				icon->_.in._.image.type = SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_PNG;
				icon->_.in._.image.data.ptr = menu_item->icon_data.bytes;
				icon->_.in._.image.data.len = menu_item->icon_data.nbytes;
				icon->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				if (label) {
					icon->type = LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM_IMAGE;
					icon->block = label;
				}
				LLIST_APPEND_TAIL(&block->_.in._.composite.children, &icon->_);
			}
#endif /* SW_WITH_PNG */

			switch (menu_item->toggle_type) {
			case SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_CHECKMARK:
			case SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_RADIO: {
				layout_block_t *toggle = layout_block_create();
				string_t s = ((menu_item->toggle_type == SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_RADIO)
					? ((menu_item->toggle_state == 1) ? string("\xF3\xB0\x90\xBE") : string("\xF3\xB0\x84\xAF"))
					: ((menu_item->toggle_state == 1) ? string("\xF3\xB0\x84\xB2") : string("\xF3\xB0\x84\xAE")));
				layout_block_init_text(&toggle->_, &s);
				toggle->_.in._.text.color._.argb32 = tray_dbusmenu_menu_item_get_text_color(menu_item);
				toggle->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				if (config->tray_padding > 0) {
					toggle->_.in.borders[0].width = config->tray_padding;
					toggle->_.in.borders[1].width = config->tray_padding;
					toggle->_.in.borders[2].width = config->tray_padding;
					toggle->_.in.borders[3].width = config->tray_padding;
				}
				if (label) {
					toggle->type = LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM_DECORATOR;
					toggle->block = label;
					toggle->_.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
				}
				LLIST_APPEND_TAIL(&block->_.in._.composite.children, &toggle->_);
				needs_spacer = TRUE;
				break;
			}
			case SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_NONE:
			default:
				break;
			}

			if (menu_item->submenu) {
				layout_block_t *submenu = layout_block_create();
				string_t s = string("\xF3\xB0\x8D\x9E");
				layout_block_init_text(&submenu->_, &s);
				submenu->_.in._.text.color._.argb32 = tray_dbusmenu_menu_item_get_text_color(menu_item);
				submenu->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				if (config->tray_padding > 0) {
					submenu->_.in.borders[0].width = config->tray_padding;
					submenu->_.in.borders[1].width = config->tray_padding;
					submenu->_.in.borders[2].width = config->tray_padding;
					submenu->_.in.borders[3].width = config->tray_padding;
				}
				if (label) {
					submenu->type = LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM_DECORATOR;
					submenu->block = label;
					submenu->_.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
				}
				LLIST_APPEND_TAIL(&block->_.in._.composite.children, &submenu->_);
				needs_spacer = TRUE;
			}
		}
		LLIST_APPEND_TAIL(&popup->_.in.root->in._.composite.children, &block->_);
	}

	if (needs_spacer) {
		sw_layout_block_t *block = popup->_.in.root->in._.composite.children.head;
		for ( ; block; block = block->next) {
			sni_dbusmenu_menu_item_t *menu_item = (sni_dbusmenu_menu_item_t *)((layout_block_t *)block)->data;
			if ((block->in.type == SW_LAYOUT_BLOCK_TYPE_COMPOSITE)
					&& (menu_item->toggle_type == SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_NONE)
					&& (menu_item->submenu == NULL)) {
				layout_block_t *b = (layout_block_t *)block->in._.composite.children.head;
				if (b) {
					layout_block_t *spacer = layout_block_create();
					spacer->type = LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM_DECORATOR;
					spacer->block = b;
					/*spacer->_.in.type = SW_LAYOUT_BLOCK_TYPE_SPACER; */
					spacer->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
					LLIST_APPEND_TAIL(&block->in._.composite.children, &spacer->_);
				}
			}
		}
	}

	state.update = TRUE;
}

static bool32_t tray_dbusmenu_menu_popup_handle_event(sw_wayland_notify_source_t *source,
		sw_context_t *ctx, sw_wayland_event_t event) {
	sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)source;
	tray_dbusmenu_menu_popup_t *popup = (tray_dbusmenu_menu_popup_t *)pointer->out.focused_surface;

	NOTUSED(ctx);

	switch (event) {
	case SW_WAYLAND_EVENT_SURFACE_CLOSED: {
		popup = (tray_dbusmenu_menu_popup_t *)source;
		MEMSET(&popup->parent->in.popups, 0, sizeof(popup->parent->in.popups));
		tray_dbusmenu_menu_popup_destroy(popup);
		break;
	}
	case SW_WAYLAND_EVENT_POINTER_ENTER:
		if (popup->seat == pointer->out.seat) {
			popup->focused_block = NULL;
			tray_dbusmenu_menu_popup_pointer_pos(popup, pointer->out.pos_x, pointer->out.pos_y);
		}
		break;
	case SW_WAYLAND_EVENT_POINTER_MOTION:
		if (popup->seat == pointer->out.seat) {
			tray_dbusmenu_menu_popup_pointer_pos(popup, pointer->out.pos_x, pointer->out.pos_y);
		}
		break;
	case SW_WAYLAND_EVENT_POINTER_LEAVE:
		if ((popup->seat == pointer->out.seat) && popup->focused_block) {
			sni_dbusmenu_menu_item_t *menu_item = (sni_dbusmenu_menu_item_t *)popup->focused_block->data;
			tray_dbusmenu_menu_item_pointer_leave(menu_item, &popup->focused_block->_);
			popup->focused_block = NULL;
		}
		break;
	case SW_WAYLAND_EVENT_POINTER_BUTTON:
		if ((popup->seat == pointer->out.seat) && popup->focused_block) {
			sni_dbusmenu_menu_item_t *menu_item = (sni_dbusmenu_menu_item_t *)popup->focused_block->data;
			tray_dbusmenu_menu_item_pointer_button(menu_item, pointer->out.btn_code, pointer->out.btn_state);
		}
		break;
	case SW_WAYLAND_EVENT_POINTER_SCROLL:
		break;
	case SW_WAYLAND_EVENT_SURFACE_FAILED_TO_SET_CURSOR_SHAPE:
	case SW_WAYLAND_EVENT_SURFACE_FAILED_TO_SET_DECORATIONS:
	case SW_WAYLAND_EVENT_SURFACE_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK:
	case SW_WAYLAND_EVENT_SURFACE_LAYOUT_FAILED:
	case SW_WAYLAND_EVENT_SURFACE_ERROR_MISSING_PROTOCOL:
	case SW_WAYLAND_EVENT_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER:
		/* TODO: handle/log */
		break;
	default:
		ASSERT_UNREACHABLE;
	}
	
	return TRUE;
}

static tray_dbusmenu_menu_popup_t *tray_dbusmenu_menu_popup_create(sni_dbusmenu_menu_t *menu,
		int32_t x, int32_t y, sw_wayland_surface_t *parent, sw_wayland_seat_t *seat) {
	tray_dbusmenu_menu_popup_t *popup;

	ASSERT(seat->out.pointer != NULL);

	sni_dbusmenu_menu_about_to_show(menu, TRUE);

	ALLOCCT(popup, &gp_alloc);

	popup->parent = parent;
	popup->seat = seat;
	popup->_.in.type = SW_WAYLAND_SURFACE_TYPE_POPUP;
	popup->_.in.notify = tray_dbusmenu_menu_popup_handle_event;
	popup->_.in.width = -1;
	popup->_.in.height = -1;
	popup->_.in._.popup.x = x;
	popup->_.in._.popup.y = y;
	popup->_.in._.popup.grab = seat->out.pointer;
	popup->_.in._.popup.gravity = SW_WAYLAND_SURFACE_POPUP_GRAVITY_TOP_LEFT;
	popup->_.in._.popup.constraint_adjustment = (SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_X
		| SW_WAYLAND_SURFACE_POPUP_CONSTRAINT_ADJUSTMENT_FLIP_Y);

	popup->_.in.root = (sw_layout_block_t *)layout_block_create();
	popup->_.in.root->in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
	popup->_.in.root->in._.composite.layout = SW_LAYOUT_BLOCK_COMPOSITE_CHILDREN_LAYOUT_VERTICAL;
	popup->_.in.root->in.color._.argb32 = state.config.colors.focused_background;

	tray_dbusmenu_menu_popup_update(popup, menu);

	sni_dbusmenu_menu_item_event(menu->parent_menu_item,
		SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_OPENED, TRUE);

	return popup;
}

static void tray_sni_item_block_pointer_button(layout_block_t *block,
		bar_t *bar, uint32_t code, sw_wayland_pointer_button_state_t state_,
		int32_t x, int32_t y, sw_wayland_seat_t *seat) {
	sni_item_t *item;
	tray_binding_command_t command;
	size_t i;

	if (state_ == SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED) {
		return;
	}

	command = TRAY_BINDING_COMMAND_NONE;
	for ( i = 0; i < state.config.tray_bindings_count; ++i) {
		tray_binding_t *binding = &state.config.tray_bindings[i];
		if (binding->event_code == code) {
			command = binding->command;
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

	item = (sni_item_t *)block->data;
	if ((command == TRAY_BINDING_COMMAND_ACTIVATE) && item->out.properties
			&& item->out.properties->item_is_menu) {
		command = TRAY_BINDING_COMMAND_CONTEXT_MENU;
	}

	switch (command) {
	case TRAY_BINDING_COMMAND_CONTEXT_MENU: {
		sni_dbusmenu_t *dbusmenu = item->out.dbusmenu;
#define MENU dbusmenu->menu->menu_items[0].submenu
		if (dbusmenu && dbusmenu->menu && (dbusmenu->menu->menu_items_count > 0)
				&& MENU && (MENU->menu_items_count > 0)) {
			state.tray.popup = tray_dbusmenu_menu_popup_create(MENU, x, y, &bar->_, seat);
			LLIST_APPEND_TAIL( &bar->_.in.popups, &state.tray.popup->_);
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

static void tray_sni_item_properties_updated_sni_server(sni_item_t *item) {
	bar_t *bar;

	NOTUSED(item);

	for ( bar = (bar_t *)state.sw.in.backend.wayland.layers.head; bar; bar = (bar_t *)bar->_.next) {
		if (tray_visible_on_bar(bar)) {
			bar->dirty = TRUE;
		}
	}
}

static void tray_sni_item_dbusmenu_menu_updated_sni_server(sni_item_t *item) {
	sni_dbusmenu_menu_t *menu;

	if ((state.tray.popup == NULL) || (state.tray.popup->menu->dbusmenu->item != item)) {
		return;
	}

	menu = item->out.dbusmenu->menu;
#define MENU menu->menu_items[0].submenu
	if (menu && (menu->menu_items_count > 0) && MENU && (MENU->menu_items_count > 0)) {
		tray_dbusmenu_menu_popup_update(state.tray.popup, MENU);
	} else {
		ASSERT(state.tray.popup->parent->in.popups.count == 1);
		MEMSET(&state.tray.popup->parent->in.popups, 0, sizeof(state.tray.popup->parent->in.popups));
		tray_dbusmenu_menu_popup_destroy(state.tray.popup);
	}
#undef MENU
}

static void tray_sni_item_destroy_sni_server(sni_item_t *item) {
	if (state.tray.popup && (state.tray.popup->menu->dbusmenu->item == item)) {
		ASSERT(state.tray.popup->parent->in.popups.count == 1);
		MEMSET(&state.tray.popup->parent->in.popups, 0, sizeof(state.tray.popup->parent->in.popups));
		tray_dbusmenu_menu_popup_destroy(state.tray.popup);
	}

	FREE(&gp_alloc, item);

	{
		bar_t *bar = (bar_t *)state.sw.in.backend.wayland.layers.head;
		for ( ; bar; bar = (bar_t *)bar->_.next) {
			if (tray_visible_on_bar(bar)) {
				bar->dirty = TRUE;
			}
		}
	}
}

static sni_item_t *tray_sni_item_create_sni_server(void) {
	sni_item_t *item;
	ALLOCCT(item, &gp_alloc);
	item->in.destroy = tray_sni_item_destroy_sni_server;
	item->in.properties_updated = tray_sni_item_properties_updated_sni_server;
	item->in.dbusmenu_menu_updated = tray_sni_item_dbusmenu_menu_updated_sni_server;

	return item;
}

static void tray_init(void) {
	int ret;

#if SW_WITH_SVG || SW_WITH_PNG
	xdg_icon_theme_cache_init(&state.tray.cache, &gp_alloc);
#endif /* SW_WITH_SVG || SW_WITH_PNG */

	sni_server.in.alloc = &gp_alloc;
	sni_server.in.item_create = tray_sni_item_create_sni_server;

	ret = sni_server_init();
	if (ret < 0) {
		su_abort(-ret, "sni_server_init: %s", strerror(-ret));
	}

	state.tray.popup = NULL;
	state.tray.active = TRUE;
}

static void tray_fini(void) {
	sni_server_fini();

#if SW_WITH_SVG || SW_WITH_PNG
	xdg_icon_theme_cache_fini(&state.tray.cache, &gp_alloc);
#endif /* SW_WITH_SVG || SW_WITH_PNG */

	MEMSET(&state.tray, 0, sizeof(state.tray));
	state.poll_fds[POLL_FD_SNI_SERVER].fd = -1;
}

static void tray_update(void) {
#if SW_WITH_SVG || SW_WITH_PNG
	xdg_icon_theme_cache_fini(&state.tray.cache, &gp_alloc);
	xdg_icon_theme_cache_init(&state.tray.cache, &gp_alloc);
#endif /* SW_WITH_SVG || SW_WITH_PNG */
}
#endif /* WITH_TRAY */

static void bar_destroy(bar_t *bar) {
	if (state.status.active && (state.sw.in.backend.wayland.layers.count == 0)) {
		kill(-state.status.pid, state.status.stop_signal);
	}

	bar->_.out.fini(&bar->_, &state.sw);
	FREE(&gp_alloc, bar);

	state.update = TRUE;
}

static void output_destroy(output_t *output) {
	size_t i = 0;
	for ( ; i < output->workspaces_count; ++i) {
		string_fini(&output->workspaces[i].name, &gp_alloc);
	}
	FREE(&gp_alloc, output->workspaces);

	FREE(&gp_alloc, output);
}

static void output_destroy_sw(sw_wayland_output_t *output, sw_context_t *sw) {
	NOTUSED(sw);
	output_destroy((output_t *)output);
}

static output_t *output_create(void) {
	output_t *output;

	if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_GET_WORKSPACES, NULL) == -1) {
		su_abort(errno, "sway_ipc_send: write: %s", strerror(errno));
	}

	ALLOCCT(output, &gp_alloc);
	output->_.in.destroy = output_destroy_sw;

	return output;
}

static sw_wayland_output_t *output_create_sw(sw_wayland_output_t *output, sw_context_t *sw) {
	output_t *out = output_create();
	NOTUSED(output); NOTUSED(sw);
	return &out->_;
}

static void pointer_destroy_sw(sw_wayland_pointer_t *pointer, sw_context_t *sw) {
	NOTUSED(sw);
	FREE(&gp_alloc, pointer);
}

static sw_wayland_pointer_t *pointer_create_sw(sw_wayland_seat_t *seat, sw_context_t *sw) {
	sw_wayland_pointer_t *pointer;
	ALLOCCT(pointer, &gp_alloc);

	NOTUSED(seat); NOTUSED(sw);

	pointer->in.destroy = pointer_destroy_sw;
	return pointer;
}

static sw_wayland_seat_t *seat_create_sw(sw_wayland_seat_t *seat, sw_context_t *sw) {
	NOTUSED(sw);

	seat->in.pointer_create = pointer_create_sw;

	return seat;
}

static void status_init(void) {
	status_t *status = &state.status;
	int pipe_read_fd[2], pipe_write_fd[2];
	pid_t pid;

	ASSERT(state.config.status_command.nul_terminated);

	if ((pipe(pipe_read_fd) == -1) || (pipe(pipe_write_fd) == -1)) {
		su_abort(errno, "pipe: %s", strerror(errno));
	}

	pid = fork();
	if (pid == -1) {
		su_abort(errno, "fork: %s", strerror(errno));
	} else if (pid == 0) {
		const char *cmd_[4];

		setpgid(0, 0);

		dup2(pipe_read_fd[1], STDOUT_FILENO);
		close(pipe_read_fd[0]);
		close(pipe_read_fd[1]);

		dup2(pipe_write_fd[0], STDIN_FILENO);
		close(pipe_write_fd[0]);
		close(pipe_write_fd[1]);

		cmd_[0] = "sh";
		cmd_[1] = "-c";
		cmd_[2] = state.config.status_command.s;
		cmd_[3] = NULL;

		execvp(cmd_[0], (char * const *)(uintptr_t)cmd_);
		exit(1);
	}

	if (!fd_set_nonblock(pipe_read_fd[0])) {
		su_abort(errno, "fcntl: %s", strerror(errno));
	}

	close(pipe_read_fd[1]);
	close(pipe_write_fd[0]);

	fd_set_cloexec(pipe_read_fd[0]);
	fd_set_cloexec(pipe_write_fd[1]);

	MEMSET(status, 0, sizeof(*status));
	status->buf.size = 8192;
	ALLOCTS(status->buf.data, &gp_alloc, 8192);
	status->stop_signal = SIGSTOP;
	status->cont_signal = SIGCONT;
	status->pid = pid;
	status->read_fd = pipe_read_fd[0];
	status->write_fd = pipe_write_fd[1];
	status->active = TRUE;

	state.poll_fds[POLL_FD_STATUS].fd = pipe_read_fd[0];
}

static void status_close_pipes(void) {
	status_t *status = &state.status;

	if (status->read_fd != -1) {
		close(status->read_fd);
		status->read_fd = -1;

		state.poll_fds[POLL_FD_STATUS].fd = -1;
	}
	if (status->write_fd != -1) {
		close(status->write_fd);
		status->write_fd = -1;
	}
}

static void status_i3bar_block_fini(status_i3bar_block_t *block);

static void status_fini(void) {
	status_t *status = &state.status;

	kill(-status->pid, status->cont_signal);
	kill(-status->pid, SIGTERM);
	waitpid(status->pid, NULL, 0);

	status_close_pipes();

	FREE(&gp_alloc, status->buf.data);

	if (status->protocol == STATUS_PROTOCOL_I3BAR) {
		size_t i = 0;
		for ( ; i < status->blocks_count; ++i) {
			status_i3bar_block_fini(&status->blocks[i]);
		}
		FREE(&gp_alloc, status->blocks);
	}

	MEMSET(status, 0, sizeof(*status));
}

static void status_set_error(string_t text) {
	status_t *status = &state.status;

	status_close_pipes();

	if (status->protocol == STATUS_PROTOCOL_I3BAR) {
		size_t i = 0;
		for ( ; i < status->blocks_count; ++i) {
			status_i3bar_block_fini(&status->blocks[i]);
		}
		FREE(&gp_alloc, status->blocks);
	}

	FREE(&gp_alloc, status->buf.data);
	ALLOCTS(status->buf.data, &gp_alloc, text.len);
	MEMCPY(status->buf.data, text.s, text.len);
	status->buf.size = text.len;
	status->buf.idx = text.len;

	status->protocol = STATUS_PROTOCOL_ERROR;
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

static void status_i3bar_block_pointer_button(layout_block_t *block,
		bar_t *bar, uint32_t code, sw_wayland_pointer_button_state_t state_,
		int32_t x, int32_t y) {
	/* TODO: scale */

	json_writer_t writer;
	double rx, ry;
	status_i3bar_block_t *i3bar_block;

	if ((state_ == SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED) || (state.status.write_fd == -1)) {
		return;
	}

	json_writer_init(&writer, &scratch_alloc, 1024);
	if (state.status.block_clicked) {
		su__json_buffer_add_string(&writer.buf, &scratch_alloc, string(","));
	} else {
		state.status.block_clicked = TRUE;
	}

	rx = x - block->_.out.dim.x;
	ry = y - block->_.out.dim.y;

	i3bar_block = (status_i3bar_block_t *)block->data;

	json_writer_object_begin(&writer, &scratch_alloc);

	json_writer_object_key(&writer, &scratch_alloc, string("name"));
	json_writer_string_escape(&writer, &scratch_alloc, i3bar_block->name);

	if (i3bar_block->instance.len > 0) {
		json_writer_object_key(&writer, &scratch_alloc, string("instance"));
		json_writer_string_escape(&writer, &scratch_alloc, i3bar_block->instance);
	}

	json_writer_object_key(&writer, &scratch_alloc, string("button"));
	json_writer_uint(&writer, &scratch_alloc, to_x11_button(code));
	json_writer_object_key(&writer, &scratch_alloc, string("event"));
	json_writer_uint(&writer, &scratch_alloc, code);
	if (state.status.float_event_coords) {
		json_writer_object_key(&writer, &scratch_alloc, string("x"));
		json_writer_double(&writer, &scratch_alloc, x);
		json_writer_object_key(&writer, &scratch_alloc, string("y"));
		json_writer_double(&writer, &scratch_alloc, y);
		json_writer_object_key(&writer, &scratch_alloc, string("relative_x"));
		json_writer_double(&writer, &scratch_alloc, rx);
		json_writer_object_key(&writer, &scratch_alloc, string("relative_y"));
		json_writer_double(&writer, &scratch_alloc, ry);
		json_writer_object_key(&writer, &scratch_alloc, string("width"));
		json_writer_double(&writer, &scratch_alloc, (double)block->_.out.dim.width);
		json_writer_object_key(&writer, &scratch_alloc, string("height"));
		json_writer_double(&writer, &scratch_alloc, (double)block->_.out.dim.height);
	} else {
		json_writer_object_key(&writer, &scratch_alloc, string("x"));
		json_writer_int(&writer, &scratch_alloc, x);
		json_writer_object_key(&writer, &scratch_alloc, string("y"));
		json_writer_int(&writer, &scratch_alloc, y);
		json_writer_object_key(&writer, &scratch_alloc, string("relative_x"));
		json_writer_int(&writer, &scratch_alloc, (int64_t)rx);
		json_writer_object_key(&writer, &scratch_alloc, string("relative_y"));
		json_writer_int(&writer, &scratch_alloc, (int64_t)ry);
		json_writer_object_key(&writer, &scratch_alloc, string("width"));
		json_writer_int(&writer, &scratch_alloc, block->_.out.dim.width);
		json_writer_object_key(&writer, &scratch_alloc, string("height"));
		json_writer_int(&writer, &scratch_alloc, block->_.out.dim.height);
	}
	json_writer_object_key(&writer, &scratch_alloc, string("scale"));
	json_writer_int(&writer, &scratch_alloc, bar->_.out.scale);

	json_writer_object_end(&writer, &scratch_alloc);

	su__json_buffer_add_string(&writer.buf, &scratch_alloc, string("\n"));

	if (write(state.status.write_fd, writer.buf.data, writer.buf.idx) == -1) {
		status_set_error(string("[failed to write click event]"));
		bars_set_dirty();
	}
}

static void status_describe(bar_t *bar) {
	status_t *status = &state.status;
	config_t *config = &state.config;
	output_t *output = (output_t *)bar->_.in._.layer.output;

	switch (status->protocol) {
	case STATUS_PROTOCOL_ERROR:
	case STATUS_PROTOCOL_TEXT:
		if (status->buf.idx > 0) {
			layout_block_t *block = layout_block_create();
			string_t s;
			s.s = (char *)status->buf.data;
			s.len = status->buf.idx;
			s.free_contents = FALSE;
			s.nul_terminated = FALSE;
			layout_block_init_text(&block->_, &s);
			block->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
			block->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
			if (status->protocol == STATUS_PROTOCOL_TEXT) {
				block->_.in._.text.color._.argb32 = output->focused ?
					config->colors.focused_statusline : config->colors.statusline;
			} else {
				block->_.in._.text.color._.argb32.u32 = 0xFFFF0000;
			}
			block->_.in.borders[0].width = 5;
			block->_.in.borders[1].width = 5;
			if (config->status_padding > 0) {
				block->_.in.borders[3].width = config->status_padding;
				block->_.in.borders[2].width = config->status_padding;
			}
			LLIST_APPEND_TAIL(&bar->_.in.root->in._.composite.children, &block->_);
		}
		break;
	case STATUS_PROTOCOL_I3BAR: {
		/* TODO: short_text */

		bool32_t edge = (bar->_.in.root->in._.composite.children.count == 1);
		size_t i = (status->blocks_count - 1);
		for ( ; i != SIZE_MAX; --i) {
			status_i3bar_block_t *i3bar_block = &status->blocks[i];
			layout_block_t *block = layout_block_create();
			layout_block_t *text = layout_block_create();
			if (status->click_events && (i3bar_block->name.len > 0)) {
				block->data = i3bar_block;
				block->type = LAYOUT_BLOCK_TYPE_STATUS_LINE_I3BAR;
			}

			if (edge && (config->status_edge_padding > 0)) {
				layout_block_t *spacer = layout_block_create();
				/*spacer->_.in.type = SW_LAYOUT_BLOCK_TYPE_SPACER;*/
				spacer->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				spacer->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
				spacer->_.in.min_width = config->status_edge_padding;
				LLIST_APPEND_TAIL(&bar->_.in.root->in._.composite.children, &spacer->_);
			} else if (!edge && ((i3bar_block->separator_block_width > 0) || i3bar_block->separator)) {
				layout_block_t *separator = layout_block_create();
				separator->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
				if (i3bar_block->separator) {
					if (config->separator_symbol.len > 0) {
						layout_block_init_text(&separator->_, &config->separator_symbol);
						separator->_.in._.text.color._.argb32 = output->focused ?
							config->colors.focused_separator : config->colors.separator;
						if (i3bar_block->separator_block_width > 0) {
							separator->_.in.min_width = i3bar_block->separator_block_width;
							separator->_.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
						}
					} else {
						/*separator->_.in.type = SW_LAYOUT_BLOCK_TYPE_SPACER;*/
						int32_t width = MAX(i3bar_block->separator_block_width, 2);
						int32_t border = (i3bar_block->separator_block_width - 2) / 2;
						if (width > 0) {
							separator->_.in.min_width = width;
							separator->_.in.max_width = width;
						}
						separator->_.in.color._.argb32 = output->focused ?
							config->colors.focused_separator : config->colors.separator;
						separator->_.in.borders[0].width = border;
						if ((i3bar_block->separator_block_width - 2 - border - border) == 1) {
							separator->_.in.borders[1].width = border + 1;
						} else {
							separator->_.in.borders[1].width = border;
						}
					}
				} else {
					/*separator->_.type = SW_LAYOUT_BLOCK_TYPE_SPACER;*/
					if (i3bar_block->separator_block_width > 0) {
						separator->_.in.min_width = i3bar_block->separator_block_width;
					}
				}
				separator->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
				LLIST_APPEND_TAIL(&bar->_.in.root->in._.composite.children, &separator->_);
			}

			block->_.in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
			block->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_RIGHT;
			if (i3bar_block->min_width_str.len > 0) {
				layout_block_t *min_width = layout_block_create();
				layout_block_init_text(&min_width->_, &i3bar_block->min_width_str);
				min_width->_.in._.text.color._.argb32.u32 = 0;
				min_width->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_NONE;
				LLIST_APPEND_TAIL(&bar->_.in.root->in._.composite.children, &min_width->_);

				block->block = min_width;
			} else if (i3bar_block->min_width > 0) {
				block->_.in.min_width = i3bar_block->min_width;
			}
			block->_.in.color._.argb32 = i3bar_block->urgent ?
				config->colors.urgent_workspace.background : i3bar_block->background_color;
			block->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
			block->_.in.content_anchor = i3bar_block->content_anchor;
			if (i3bar_block->border_color_set || i3bar_block->urgent) {
				size_t j = 0;
				for ( ; j < LENGTH(block->_.in.borders); ++j) {
					if (i3bar_block->border_widths[j] > 0) {
						block->_.in.borders[j].width = i3bar_block->border_widths[j];
						block->_.in.borders[j].color._.argb32 = i3bar_block->urgent ?
							config->colors.urgent_workspace.border : i3bar_block->border_color;
					}
				}
			}

			layout_block_init_text(&text->_, &i3bar_block->full_text);
			text->_.in._.text.color._.argb32 = i3bar_block->urgent ? config->colors.urgent_workspace.text :
				(i3bar_block->text_color_set ? i3bar_block->text_color :
				(output->focused ? config->colors.focused_statusline : config->colors.statusline));
			text->_.in.borders[0].width = 5;
			text->_.in.borders[1].width = 5;
			if (config->status_padding > 0) {
				text->_.in.borders[3].width = config->status_padding;
				text->_.in.borders[2].width = config->status_padding;
			}

			LLIST_APPEND_TAIL(&block->_.in._.composite.children, &text->_);

			LLIST_APPEND_TAIL(&bar->_.in.root->in._.composite.children, &block->_);

			edge = FALSE;
		}
		break;
	}
	case STATUS_PROTOCOL_UNDEF:
	default:
		break;
	}
}

static bool32_t parse_sway_color(string_t str, sw_color_argb32_t *dest) {
	char *p;
	uint32_t rgba;
	uint8_t a, r ,g ,b;

	ASSERT(str.nul_terminated); /* TODO: remove */

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

	rgba = (uint32_t)strtoul(str.s, &p, 16);
	if (*p != '\0') {
		return FALSE;
	}

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

static bool32_t status_i3bar_block_init(status_i3bar_block_t *block, json_ast_node_t *json) {
	size_t i;

	if (json->type != JSON_AST_NODE_TYPE_OBJECT) {
		return FALSE;
	}

	MEMSET(block, 0, sizeof(*block));

	block->border_widths[0] = 1;
	block->border_widths[1] = 1;
	block->border_widths[2] = 1;
	block->border_widths[3] = 1;
	block->separator = TRUE;
	block->separator_block_width = 9;

	/* ? TODO: "_sw" */

	for ( i = 0; i < json->value.o.count; ++i) {
		json_ast_key_value_t *key_value = &json->value.o.kvs[i];
		if (string_equal(key_value->key, string("full_text"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.value.s.len > 0)) {
				string_init_string(&block->full_text, &gp_alloc, key_value->value.value.s);
			}
		} else if (string_equal(key_value->key, string("short_text"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.value.s.len > 0)) {
				string_init_string(&block->short_text, &gp_alloc, key_value->value.value.s);
			}
		} else if (string_equal(key_value->key, string("color"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_STRING) {
				block->text_color_set = parse_sway_color(
					key_value->value.value.s, &block->text_color);
			}
		} else if (string_equal(key_value->key, string("background"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_STRING) {
				parse_sway_color(
					key_value->value.value.s, &block->background_color);
			}
		} else if (string_equal(key_value->key, string("border"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_STRING) {
				block->border_color_set = parse_sway_color(
					key_value->value.value.s, &block->border_color);
			}
		} else if (string_equal(key_value->key, string("border_top"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				block->border_widths[3] = (int32_t)key_value->value.value.u;
			}
		} else if (string_equal(key_value->key, string("border_bottom"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				block->border_widths[2] = (int32_t)key_value->value.value.u;
			}
		} else if (string_equal(key_value->key, string("border_left"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				block->border_widths[0] = (int32_t)key_value->value.value.u;
			}
		} else if (string_equal(key_value->key, string("border_right"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				block->border_widths[1] = (int32_t)key_value->value.value.u;
			}
		} else if (string_equal(key_value->key, string("min_width"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				block->min_width = (int32_t)key_value->value.value.u;
			} else if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.value.s.len > 0)) {
				string_init_string(
					&block->min_width_str, &gp_alloc, key_value->value.value.s);
			}
		} else if (string_equal(key_value->key, string("align"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.value.s.len > 0)) {
				if (string_equal(key_value->value.value.s, string("center"))) {
					block->content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
				} else if (string_equal(key_value->value.value.s, string("right"))) {
					block->content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_RIGHT_CENTER;
				}
			}
		} else if (string_equal(key_value->key, string("name"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.value.s.len > 0)) {
				string_init_string(&block->name, &gp_alloc, key_value->value.value.s);
			}
		} else if (string_equal(key_value->key, string("instance"))) {
			if ((key_value->value.type == JSON_AST_NODE_TYPE_STRING) && (key_value->value.value.s.len > 0)) {
				string_init_string(&block->instance, &gp_alloc, key_value->value.value.s);
			}
		} else if (string_equal(key_value->key, string("urgent"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_BOOL) {
				block->urgent = key_value->value.value.b;
			}
		} else if (string_equal(key_value->key, string("separator"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_BOOL) {
				block->separator = key_value->value.value.b;
			}
		} else if (string_equal(key_value->key, string("separator_block_width"))) {
			if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
				block->separator_block_width = (int32_t)key_value->value.value.u;
			}
		}
		/* TODO: markup */
	}

	if (block->full_text.len == 0) {
		status_i3bar_block_fini(block);
		return FALSE;
	}

	return TRUE;
}

static void status_i3bar_block_fini(status_i3bar_block_t *block) {
	string_fini(&block->full_text, &gp_alloc);
	string_fini(&block->short_text, &gp_alloc);
	string_fini(&block->min_width_str, &gp_alloc);
	string_fini(&block->name, &gp_alloc);
	string_fini(&block->instance, &gp_alloc);
}

static void status_i3bar_parse_json(json_ast_node_t *json) {
	status_t *status = &state.status;
	size_t i;

	if (json->type != JSON_AST_NODE_TYPE_ARRAY) {
		return;
	}

	for ( i = 0; i < status->blocks_count; ++i) {
		status_i3bar_block_fini(&status->blocks[i]);
	}
	FREE(&gp_alloc, status->blocks);

	ARRAY_ALLOC(status->blocks, &gp_alloc, json->value.a.count);
	status->blocks_count = 0;

	for ( i = 0; i < json->value.a.count; ++i) {
		if (status_i3bar_block_init(&status->blocks[status->blocks_count], &json->value.a.nodes[i])) {
			status->blocks_count++;
		}
	}
}

static bool32_t status_i3bar_process(void) {
	status_t *status = &state.status;
	json_tokener_t tok;
	string_t str;
	json_ast_t ast;

    for (;;) {
		ssize_t read_bytes = read(status->read_fd, &status->buf.data[status->buf.idx],
			status->buf.size - status->buf.idx);
		if (read_bytes <= 0) {
			if (read_bytes == 0) {
				errno = EPIPE;
			}
			if (errno == EAGAIN) {
				break;
			} else if (errno == EINTR) {
				continue;
			} else {
				status_set_error(string("[error reading from status command]"));
				return TRUE;
			}
		} else {
			status->buf.idx += (size_t)read_bytes;
			if (status->buf.idx == status->buf.size) {
				size_t new_size = (status->buf.size * 2);
				uint8_t *new_data;
				ALLOCTS(new_data, &gp_alloc, new_size);
				MEMCPY(new_data, status->buf.data, status->buf.idx);
				FREE(&gp_alloc, status->buf.data);
				status->buf.data = new_data;
				status->buf.size = new_size;
			}
		}
    }

	if (status->buf.idx == 0) {
		return FALSE;
	}

	MEMSET(&tok, 0, sizeof(tok));

	str.s = (char *)status->buf.data;
	str.len = status->buf.idx;
	json_tokener_set_string(&tok, &scratch_alloc, str);

	if (status->tokener_state) {
		tok.state[tok.state_count++] = status->tokener_state;
		tok.depth = 1;
	} else {
		json_token_t token;
		json_tokener_state_t s = json_tokener_next(&tok, &scratch_alloc, &token);
		if (s == JSON_TOKENER_STATE_MORE_DATA_EXPECTED) {
			return FALSE;
		} else if ((s != JSON_TOKENER_STATE_SUCCESS) || (token.type != JSON_TOKEN_TYPE_ARRAY_START)) {
			status_set_error(string("[invalid i3bar json]"));
			return TRUE;
		}
	}

	json_ast_reset(&ast);

	for (;;) {
		json_tokener_state_t s = json_tokener_ast(&tok, &scratch_alloc, &ast, 1, TRUE);
		if ((tok.depth != 1) || (s == JSON_TOKENER_STATE_ERROR) || (s == JSON_TOKENER_STATE_EOF)) {
			status_set_error(string("[failed to parse i3bar json]"));
			return TRUE;
		} else if (s == JSON_TOKENER_STATE_SUCCESS) {
			status_i3bar_parse_json(&ast.root);
			json_ast_reset(&ast);
		} else if (s == JSON_TOKENER_STATE_MORE_DATA_EXPECTED) {
			break;
		}
	}

	status->tokener_state = tok.state[tok.state_count - 1];

	return TRUE;
}

static bool32_t status_process(void) {
	status_t *status = &state.status;
	ssize_t read_bytes;

	status->buf.idx = 0;
	
	switch (status->protocol) {
	case STATUS_PROTOCOL_UNDEF: {
		int available_bytes;
		string_t str, newline;

		errno = 0;

		if (ioctl(status->read_fd, FIONREAD, &available_bytes) == -1) {
			status_set_error(string("[error reading from status command]"));
			return TRUE;
		}

		if (((size_t)available_bytes + 1) > status->buf.size) {
			size_t new_size = ((size_t)available_bytes + 1);
			uint8_t *new_data;
			ALLOCTS(new_data, &gp_alloc, new_size);
			MEMCPY(new_data, status->buf.data, status->buf.idx);
			FREE(&gp_alloc, status->buf.data);
			status->buf.data = new_data;
			status->buf.size = new_size;
		}

		read_bytes = read(status->read_fd, status->buf.data, (size_t)available_bytes);
		if (read_bytes != available_bytes) {
			status_set_error(string("[error reading from status command]"));
			return TRUE;
		}

		str.s = (char *)status->buf.data;
		str.len = (size_t)read_bytes;

		if ((str.len > 0) && string_find_char(str, '\n', &newline)) {
			json_tokener_t tok;
			json_ast_t header;
			bool32_t valid = FALSE;
			json_tokener_state_t s;
			size_t i;

			MEMSET(&tok, 0, sizeof(tok));
			json_tokener_set_string(&tok, &scratch_alloc, str);
			json_ast_reset(&header);

			s = json_tokener_ast(&tok, &scratch_alloc, &header, 0, TRUE);
			if ((s != JSON_TOKENER_STATE_SUCCESS) || (header.root.type != JSON_AST_NODE_TYPE_OBJECT)) {
				goto protocol_text;
			}

			for ( i = 0; i < header.root.value.o.count; ++i) {
				json_ast_key_value_t *key_value = &header.root.value.o.kvs[i];
				if (string_equal(key_value->key, string("version"))) {
					if ((key_value->value.type != JSON_AST_NODE_TYPE_UINT) || (key_value->value.value.u != 1)) {
						goto protocol_text;
					}
					valid = TRUE;
				} else if (string_equal(key_value->key, string("click_events"))) {
					if (key_value->value.type == JSON_AST_NODE_TYPE_BOOL) {
						status->click_events = key_value->value.value.b;
					}
				} else if (string_equal(key_value->key, string("float_event_coords"))) {
					if (key_value->value.type == JSON_AST_NODE_TYPE_BOOL) {
						status->float_event_coords = key_value->value.value.b;
					}
				} else if (string_equal(key_value->key, string("stop_signal"))) {
					if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
						status->stop_signal = (int)key_value->value.value.u;
					}
				} else if (string_equal(key_value->key, string("cont_signal"))) {
					if (key_value->value.type == JSON_AST_NODE_TYPE_UINT) {
						status->cont_signal = (int)key_value->value.value.u;
					}
				}
			}
			if (!valid) {
				goto protocol_text;
			}

			status->protocol = STATUS_PROTOCOL_I3BAR;
			status->buf.idx = newline.len - 1;
			MEMMOVE(status->buf.data, newline.s + 1, status->buf.idx);

			if (status->click_events) {
				if (write(status->write_fd, "[\n", 2) != 2) {
					status_set_error(string("[failed to write to status command]"));
					return TRUE;
				}
			}

			return status_i3bar_process();
		}

protocol_text:
		status->protocol = STATUS_PROTOCOL_TEXT;
		status->buf.idx = (size_t)read_bytes;
		ATTRIBUTE_FALLTHROUGH;
	}
	case STATUS_PROTOCOL_TEXT:
		for (;;) {
			read_bytes = read(status->read_fd, &status->buf.data[status->buf.idx],
				status->buf.size - status->buf.idx);
			if (read_bytes <= 0) {
				if (read_bytes == 0) {
					errno = EPIPE;
				}
				if (errno == EAGAIN) {
					if ((status->buf.idx > 0) && (status->buf.data[status->buf.idx - 1] == '\n')) {
						status->buf.idx--;
					}
					return TRUE;
				} else if (errno == EINTR) {
					continue;
				} else {
					status_set_error(string("[error reading from status command]"));
					return TRUE;
				}
			} else {
				status->buf.idx += (size_t)read_bytes;
				if (status->buf.idx == status->buf.size) {
					size_t new_size = (status->buf.size * 2);
					uint8_t *new_data;
					ALLOCTS(new_data, &gp_alloc, new_size);
					MEMCPY(new_data, status->buf.data, status->buf.idx);
					FREE(&gp_alloc, status->buf.data);
					status->buf.data = new_data;
					status->buf.size = new_size;
				}
			}
		}
	case STATUS_PROTOCOL_I3BAR:
		return status_i3bar_process();
	case STATUS_PROTOCOL_ERROR:
	default:
		ASSERT_UNREACHABLE;
	}

	return FALSE;
}

static void workspaces_describe(bar_t *bar) {
	config_t *config = &state.config;
	output_t *output = (output_t *)bar->_.in._.layer.output;
	size_t i = 0;
	for ( ; i < output->workspaces_count; ++i) {
		workspace_t *workspace = &output->workspaces[i];
		layout_block_t *block = layout_block_create();
		layout_block_t *text = layout_block_create();
		bar_box_colors_t colors;
		
		if (workspace->urgent) {
			colors = config->colors.urgent_workspace;
		} else if (workspace->focused) {
			colors = config->colors.focused_workspace;
		} else if (workspace->visible) {
			colors = config->colors.active_workspace;
		} else {
			colors = config->colors.inactive_workspace;
		}

		block->type = LAYOUT_BLOCK_TYPE_WORKSPACE;
		block->data = workspace;
		block->_.in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
		block->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
		if (config->workspace_min_width > 0) {
			block->_.in.min_width = config->workspace_min_width;
			block->_.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
		}
		block->_.in.color._.argb32 = colors.background;
		block->_.in.borders[0].width = 1;
		block->_.in.borders[0].color._.argb32 = colors.border;
		block->_.in.borders[1].width = 1;
		block->_.in.borders[1].color._.argb32 = colors.border;
		block->_.in.borders[2].width = 1;
		block->_.in.borders[2].color._.argb32 = colors.border;
		block->_.in.borders[3].width = 1;
		block->_.in.borders[3].color._.argb32 = colors.border;

		layout_block_init_text(&text->_, NULL);
		text->_.in._.text.color._.argb32 = colors.text;
		text->_.in.borders[0].width = 5;
		text->_.in.borders[1].width = 5;
		text->_.in.borders[2].width = 1;
		text->_.in.borders[3].width = 1;
		if (workspace->num != -1) {
			if (config->strip_workspace_name) {
				string_init_format(&text->_.in._.text.text, &gp_alloc, "%d", workspace->num);
			} else if (config->strip_workspace_numbers) {
				int num_len = su_snprintf(NULL, 0, "%d", workspace->num);
				num_len += (workspace->name.s[num_len] == ':');
				if (workspace->name.s[num_len] != '\0') {
					text->_.in._.text.text.s = workspace->name.s + num_len;
					text->_.in._.text.text.len = workspace->name.len - (size_t)num_len;
					text->_.in._.text.text.nul_terminated = TRUE;
					text->_.in._.text.text.free_contents = FALSE;
				}
			}
		}
		if (text->_.in._.text.text.len == 0) {
			text->_.in._.text.text = string_view(workspace->name);
		}

		LLIST_APPEND_TAIL(&block->_.in._.composite.children, &text->_);

		LLIST_APPEND_TAIL(&bar->_.in.root->in._.composite.children, &block->_);
	}
}

static void binding_mode_indicator_describe(bar_t *bar) {
	config_t *config = &state.config;
	layout_block_t *block, *text;

	if (state.binding_mode_indicator_text.len == 0) {
		return;
	}

	block = layout_block_create();
	block->type = LAYOUT_BLOCK_TYPE_BINDING_MODE_INDICATOR;
	block->_.in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
	block->_.in.expand = SW_LAYOUT_BLOCK_EXPAND_TOP | SW_LAYOUT_BLOCK_EXPAND_BOTTOM;
	if (config->workspace_min_width > 0) {
		block->_.in.min_width = config->workspace_min_width;
	}
	block->_.in.color._.argb32 = config->colors.binding_mode.background;
	block->_.in.borders[0].width = 1;
	block->_.in.borders[0].color._.argb32 = config->colors.binding_mode.border;
	block->_.in.borders[1].width = 1;
	block->_.in.borders[1].color._.argb32 = config->colors.binding_mode.border;
	block->_.in.borders[2].width = 1;
	block->_.in.borders[2].color._.argb32 = config->colors.binding_mode.border;
	block->_.in.borders[3].width = 1;
	block->_.in.borders[3].color._.argb32 = config->colors.binding_mode.border;

	text = layout_block_create();
	layout_block_init_text(&text->_, &state.binding_mode_indicator_text);
	text->_.in._.text.color._.argb32 = config->colors.binding_mode.text;
	text->_.in.borders[0].width = 5;
	text->_.in.borders[1].width = 5;
	text->_.in.borders[2].width = 1;
	text->_.in.borders[3].width = 1;

	LLIST_APPEND_TAIL(&block->_.in._.composite.children, &text->_);

	LLIST_APPEND_TAIL(&bar->_.in.root->in._.composite.children, &block->_);
}

static void bar_update(bar_t *bar) {
	output_t *output = (output_t *)bar->_.in._.layer.output;
	config_t *config = &state.config;
	layout_block_t *min_height = layout_block_create();
	string_t min_height_str = string(" ");

	sw_layout_block_t *block = bar->_.in.root->in._.composite.children.head;
	for ( ; block; ) {
		sw_layout_block_t *next = block->next;
		layout_block_destroy((layout_block_t *)block);
		block = next;
	}
	MEMSET(&bar->_.in.root->in._.composite.children, 0, sizeof(bar->_.in.root->in._.composite.children));

	layout_block_init_text(&min_height->_, &min_height_str);
	min_height->_.in.anchor = SW_LAYOUT_BLOCK_ANCHOR_NONE;
	LLIST_APPEND_TAIL(&bar->_.in.root->in._.composite.children, &min_height->_);

	bar->_.in.input_regions_count = 0;
	bar->_.in.height = config->height;
	bar->_.in._.layer.anchor = config->position;
	MEMCPY(bar->_.in._.layer.margins, config->gaps, sizeof(bar->_.in._.layer.margins));

	if ((config->mode == BAR_MODE_OVERLAY) || (config->mode == BAR_MODE_HIDE)) {
		bar->_.in._.layer.layer = SW_WAYLAND_SURFACE_LAYER_LAYER_OVERLAY;
		bar->_.in._.layer.exclusive_zone = -1;
		if (config->mode == BAR_MODE_OVERLAY) {
			static sw_wayland_region_t empty_region = { 0, 0, 0, 0 };
			bar->_.in.input_regions = &empty_region;
			bar->_.in.input_regions_count = 1;
		}
	} else {
		bar->_.in._.layer.layer = SW_WAYLAND_SURFACE_LAYER_LAYER_BOTTOM;
		bar->_.in._.layer.exclusive_zone = INT_MIN;
	}

	bar->_.in.root->in.color._.argb32 = output->focused
		? config->colors.focused_background : config->colors.background;

#if WITH_TRAY
	if (state.tray.active) {
		tray_describe_sni_items(bar);
	}
#endif /* WITH_TRAY */

	if (state.status.active) {
		status_describe(bar);
	}
	if (config->workspace_buttons) {
		workspaces_describe(bar);
	}
	if (config->binding_mode_indicator) {
		binding_mode_indicator_describe(bar);
	}

	state.update = TRUE;
}

static void config_update(string_t str) {
	config_t *config = &state.config;
	json_tokener_t tok;
	json_ast_t ast;
	json_ast_node_t *node;
	json_token_t token;
	string_t old_status_command, new_status_command;
	size_t i;
	json_tokener_state_t s;

	MEMSET(&tok, 0, sizeof(tok));
	json_tokener_set_string(&tok, &scratch_alloc, str);

	json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_OBJECT_START);

	json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_KEY);
	if (UNLIKELY(string_equal(token.value.s, string("success")))) {
		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_BOOL);
		ASSERT(token.value.b == FALSE);
		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_KEY);
		ASSERT(string_equal(token.value.s, string("error")));
		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_STRING);
		su_abort(1, STRING_PF_FMT, STRING_PF_ARGS(token.value.s));
	} else {
		ASSERT(string_equal(token.value.s, string("id")));
	}

	json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_STRING);
	if (!string_equal(state.bar_id, token.value.s)) {
		return;
	}

	if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_GET_BINDING_STATE, NULL) == -1) {
		su_abort(errno, "sway_ipc_send: write: %s", strerror(errno));
	}

	old_status_command = config->status_command;
	MEMSET(&new_status_command, 0, sizeof(new_status_command));

	string_fini(&config->font, &gp_alloc);
	string_fini(&config->separator_symbol, &gp_alloc);
	for ( i = 0; i < config->bindings_count; ++i) {
		string_fini(&config->bindings[i].command, &gp_alloc);
	}
	FREE(&gp_alloc, config->bindings);
	for ( i = 0; i < config->outputs_count; ++i) {
		string_fini(&config->outputs[i], &gp_alloc);
	}
	FREE(&gp_alloc, config->outputs);
#if WITH_TRAY
	for ( i = 0; i < config->tray_outputs_count; ++i) {
		string_fini(&config->tray_outputs[i], &gp_alloc);
	}
	FREE(&gp_alloc, config->tray_outputs);
	FREE(&gp_alloc, config->tray_bindings);
	string_fini(&config->tray_icon_theme, &gp_alloc);
#endif /* WITH_TRAY */

	MEMSET(&state.config, 0, sizeof(state.config));
	config->font = string("monospace:size=16");
	config->position = (SW_WAYLAND_SURFACE_LAYER_ANCHOR_BOTTOM |
		SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT | SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT);
	config->height = -1;
	config->status_padding = 1;
	config->status_edge_padding = 3;
	config->workspace_buttons = TRUE;
	config->binding_mode_indicator = TRUE;
	config->colors.background.u32 = 0xFF000000;
	config->colors.statusline.u32 = 0xFFFFFFFF;
	config->colors.separator.u32 = 0xFF666666;
	config->colors.focused_background.u32 = 0xFF000000;
	config->colors.focused_statusline.u32 = 0xFFFFFFFF;
	/* TODO: default  config->colors.focused_separator.u32 */
	config->colors.focused_workspace.border.u32 = 0xFF4C7899;
	config->colors.focused_workspace.background.u32 = 0xFF285577;
	config->colors.focused_workspace.text.u32 = 0xFFFFFFFF;
	config->colors.inactive_workspace.border.u32 = 0xFF333333;
	config->colors.inactive_workspace.background.u32 = 0xFF222222;
	config->colors.inactive_workspace.text.u32 = 0xFF888888;
	config->colors.active_workspace.border.u32 = 0xFF333333;
	config->colors.active_workspace.background.u32 = 0xFF5F676A;
	config->colors.active_workspace.text.u32 = 0xFFFFFFFF;
	config->colors.urgent_workspace.border.u32 = 0xFF2F343A;
	config->colors.urgent_workspace.background.u32 = 0xFF900000;
	config->colors.urgent_workspace.text.u32 = 0xFFFFFFFF;
	config->colors.binding_mode.border.u32 = 0xFF2F343A;
	config->colors.binding_mode.background.u32 = 0xFF900000;
	config->colors.binding_mode.text.u32 = 0xFFFFFFFF;
#if WITH_TRAY
	config->tray_padding = 2;
#endif /* WITH_TRAY */

	json_ast_reset(&ast);
	MEMSET(&tok, 0, sizeof(tok));
	json_tokener_set_string(&tok, &scratch_alloc, str);
	s = json_tokener_ast(&tok, &scratch_alloc, &ast, 0, FALSE);
	ASSERT(s == JSON_TOKENER_STATE_SUCCESS);
	ASSERT(ast.root.type == JSON_AST_NODE_TYPE_OBJECT);

#if WITH_TRAY
	{
		bool32_t tray_enabled = TRUE;
		if ((node = json_ast_node_object_get(&ast.root, string("tray_outputs")))) {
			ASSERT(node->type == JSON_AST_NODE_TYPE_ARRAY);
			if (node->value.a.count > 0) {
				for ( i = 0; i < node->value.a.count; ++i) {
					json_ast_node_t *v = &node->value.a.nodes[i];
					ASSERT(v->type == JSON_AST_NODE_TYPE_STRING);
					if (string_equal(v->value.s, string("none"))) {
						tray_enabled = FALSE;
						break;
					}
				}
				if (tray_enabled) {
					ARRAY_ALLOC(config->tray_outputs, &gp_alloc, node->value.a.count);
					for ( i = 0; i < node->value.a.count; ++i) {
						json_ast_node_t *v = &node->value.a.nodes[i];
						if (v->value.s.len > 0) {
							string_init_string( &config->tray_outputs[config->tray_outputs_count++],
								&gp_alloc, v->value.s);
						}
					}
				}
			}
		}
		if (tray_enabled && !state.tray.active) {
			tray_init();
		} else if (!tray_enabled && state.tray.active) {
			tray_fini();
		} else if (state.tray.active) {
			tray_update();
		}

		if ((node = json_ast_node_object_get(&ast.root, string("tray_bindings")))) {
			ASSERT(node->type == JSON_AST_NODE_TYPE_ARRAY);
			if (node->value.a.count > 0) {
				ARRAY_ALLOC(config->tray_bindings, &gp_alloc, node->value.a.count);
				for ( i = 0; i < node->value.a.count; ++i) {
					json_ast_node_t *obj = &node->value.a.nodes[i];
					json_ast_node_t *v;
					tray_binding_t *tray_binding = &config->tray_bindings[config->tray_bindings_count++];
					/* ??? input_code */
					{
						v = json_ast_node_object_get(obj, string("event_code"));
						ASSERT(v->type == JSON_AST_NODE_TYPE_UINT);
						tray_binding->event_code = (uint32_t)v->value.u;
					}
					{
						v = json_ast_node_object_get(obj, string("command"));
						ASSERT((v->type == JSON_AST_NODE_TYPE_STRING) && (v->value.s.len > 0));
						if (string_equal(v->value.s, string("ContextMenu"))) {
							tray_binding->command = TRAY_BINDING_COMMAND_CONTEXT_MENU;
						} else if (string_equal(v->value.s, string("Activate"))) {
							tray_binding->command = TRAY_BINDING_COMMAND_ACTIVATE;
						} else if (string_equal(v->value.s, string("SecondaryActivate"))) {
							tray_binding->command = TRAY_BINDING_COMMAND_SECONDARY_ACTIVATE;
						} else if (string_equal(v->value.s, string("ScrollDown"))) {
							tray_binding->command = TRAY_BINDING_COMMAND_SCROLL_DOWN;
						} else if (string_equal(v->value.s, string("ScrollLeft"))) {
							tray_binding->command = TRAY_BINDING_COMMAND_SCROLL_LEFT;
						} else if (string_equal(v->value.s, string("ScrollRight"))) {
							tray_binding->command = TRAY_BINDING_COMMAND_SCROLL_RIGHT;
						} else if (string_equal(v->value.s, string("ScrollUp"))) {
							tray_binding->command = TRAY_BINDING_COMMAND_SCROLL_UP;
						} else {
							tray_binding->command = TRAY_BINDING_COMMAND_NOP;
						}
					}
				}
			}
		}

		if ((node = json_ast_node_object_get(&ast.root, string("icon_theme")))) {
			ASSERT(node->type == JSON_AST_NODE_TYPE_STRING);
			if (node->value.s.len > 0) {
				string_init_string(&config->tray_icon_theme, &gp_alloc, node->value.s);
			}
		}

		if ((node = json_ast_node_object_get(&ast.root, string("tray_padding")))) {
			ASSERT(node->type == JSON_AST_NODE_TYPE_UINT);
			config->tray_padding = (int32_t)node->value.u;
		}
	}
#endif /* WITH_TRAY */

	if ((node = json_ast_node_object_get(&ast.root, string("status_command")))) {
		if ((node->type == JSON_AST_NODE_TYPE_STRING) && (node->value.s.len > 0)) {
			string_init_string(&new_status_command, &gp_alloc, node->value.s);
		}
	}

	if (state.status.active && ((new_status_command.len == 0) ||
			!string_equal(new_status_command, old_status_command))) {
		status_fini();
	}
	string_fini(&old_status_command, &gp_alloc);
	config->status_command = new_status_command;
	if (!state.status.active && (new_status_command.len > 0)) {
		status_init();
	}

	if ((node = json_ast_node_object_get(&ast.root, string("mode")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_STRING);
		if (string_equal(node->value.s, string("hide"))) {
			config->mode = BAR_MODE_HIDE;
		} else if (string_equal(node->value.s, string("invisible"))) {
			config->mode = BAR_MODE_INVISIBLE;
		} else if (string_equal(node->value.s, string("overlay"))) {
			config->mode = BAR_MODE_OVERLAY;
		}
	}

	if ((node = json_ast_node_object_get(&ast.root, string("hidden_state")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_STRING);
		if (string_equal(node->value.s, string("show"))) {
			config->hidden_state = BAR_HIDDEN_STATE_SHOW;
		}
	}

	if ((node = json_ast_node_object_get(&ast.root, string("position")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_STRING);
		if (string_equal(node->value.s, string("top"))) {
			config->position = (SW_WAYLAND_SURFACE_LAYER_ANCHOR_TOP |
				SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT | SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT);
		}
	}

	if ((node = json_ast_node_object_get(&ast.root, string("font")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_STRING);
		if (node->value.s.len > 0) {
			string_init_string(&config->font, &gp_alloc, node->value.s);
		}
	}

	if ((node = json_ast_node_object_get(&ast.root, string("gaps")))) {
		json_ast_node_t *top, *right, *bottom, *left;
		ASSERT(node->type == JSON_AST_NODE_TYPE_OBJECT);
		if ((top = json_ast_node_object_get(node, string("top")))) {
			ASSERT(top->type == JSON_AST_NODE_TYPE_UINT);
			config->gaps[0] = (int32_t)top->value.i;
		}
		if ((right = json_ast_node_object_get(node, string("right")))) {
			ASSERT(right->type == JSON_AST_NODE_TYPE_UINT);
			config->gaps[1] = (int32_t)right->value.i;
		}
		if ((bottom = json_ast_node_object_get(node, string("bottom")))) {
			ASSERT(bottom->type == JSON_AST_NODE_TYPE_UINT);
			config->gaps[2] = (int32_t)bottom->value.i;
		}
		if ((left = json_ast_node_object_get(node, string("left")))) {
			ASSERT(left->type == JSON_AST_NODE_TYPE_UINT);
			config->gaps[3] = (int32_t)left->value.i;
		}
	}

	if ((node = json_ast_node_object_get(&ast.root, string("separator_symbol")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_STRING);
		if (node->value.s.len > 0) {
			string_init_string(&config->separator_symbol, &gp_alloc, node->value.s);
		}
	}

	if ((node = json_ast_node_object_get(&ast.root, string("bar_height")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_UINT);
		config->height = ((node->value.u == 0) ? -1 : (int32_t)node->value.u);
	}

	if ((node = json_ast_node_object_get(&ast.root, string("status_padding")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_UINT);
		config->status_padding = (int32_t)node->value.u;
	}

	if ((node = json_ast_node_object_get(&ast.root, string("status_edge_padding")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_UINT);
		config->status_edge_padding = (int32_t)node->value.u;
	}

	if ((node = json_ast_node_object_get(&ast.root, string("wrap_scroll")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_BOOL);
		config->wrap_scroll = node->value.b;
	}

	if ((node = json_ast_node_object_get(&ast.root, string("workspace_buttons")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_BOOL);
		config->workspace_buttons = node->value.b;
	}

	if ((node = json_ast_node_object_get(&ast.root, string("strip_workspace_numbers")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_BOOL);
		config->strip_workspace_numbers = node->value.b;
	}

	if ((node = json_ast_node_object_get(&ast.root, string("strip_workspace_name")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_BOOL);
		config->strip_workspace_name = node->value.b;
	}

	if ((node = json_ast_node_object_get(&ast.root, string("workspace_min_width")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_UINT);
		config->workspace_min_width = (int32_t)node->value.u;
	}

	if ((node = json_ast_node_object_get(&ast.root, string("binding_mode_indicator")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_BOOL);
		config->binding_mode_indicator = node->value.b;
	}

	/* ??? verbose */

	/* TODO: pango_markup */

		
	if ((node = json_ast_node_object_get(&ast.root, string("colors")))) {
		json_ast_node_t *color;
		ASSERT(node->type == JSON_AST_NODE_TYPE_OBJECT);
		if ((color = json_ast_node_object_get(node, string("background")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.background);
		}
		if ((color = json_ast_node_object_get(node, string("statusline")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.statusline);
		}
		if ((color = json_ast_node_object_get(node, string("background")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.background);
		}

		if ((color = json_ast_node_object_get(node, string("focused_background")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.focused_background);
		}
		if ((color = json_ast_node_object_get(node, string("focused_statusline")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.focused_statusline);
		}
		if ((color = json_ast_node_object_get(node, string("focused_separator")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.focused_separator);
		}

		if ((color = json_ast_node_object_get(node, string("focused_workspace_border")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.focused_workspace.border);
		}
		if ((color = json_ast_node_object_get(node, string("focused_workspace_bg")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.focused_workspace.background);
		}
		if ((color = json_ast_node_object_get(node, string("focused_workspace_text")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.focused_workspace.text);
		}

		if ((color = json_ast_node_object_get(node, string("inactive_workspace_border")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.inactive_workspace.border);
		}
		if ((color = json_ast_node_object_get(node, string("inactive_workspace_bg")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.inactive_workspace.background);
		}
		if ((color = json_ast_node_object_get(node, string("inactive_workspace_text")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.inactive_workspace.text);
		}

		if ((color = json_ast_node_object_get(node, string("active_workspace_border")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.active_workspace.border);
		}
		if ((color = json_ast_node_object_get(node, string("active_workspace_bg")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.active_workspace.background);
		}
		if ((color = json_ast_node_object_get(node, string("active_workspace_text")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.active_workspace.text);
		}

		if ((color = json_ast_node_object_get(node, string("urgent_workspace_border")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.urgent_workspace.border);
		}
		if ((color = json_ast_node_object_get(node, string("urgent_workspace_bg")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.urgent_workspace.background);
		}
		if ((color = json_ast_node_object_get(node, string("urgent_workspace_text")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.urgent_workspace.text);
		}

		if ((color = json_ast_node_object_get(node, string("binding_mode_border")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.binding_mode.border);
		}
		if ((color = json_ast_node_object_get(node, string("binding_mode_bg")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.binding_mode.background);
		}
		if ((color = json_ast_node_object_get(node, string("binding_mode_text")))) {
			ASSERT(color->type == JSON_AST_NODE_TYPE_STRING);
			parse_sway_color(color->value.s, &config->colors.binding_mode.text);
		}
	}

	if ((node = json_ast_node_object_get(&ast.root, string("bindings")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_ARRAY);
		if (node->value.a.count > 0) {
			ARRAY_ALLOC(config->bindings, &gp_alloc, node->value.a.count);
			for ( i = 0; i < node->value.a.count; ++i) {
				json_ast_node_t *obj = &node->value.a.nodes[i];
				json_ast_node_t *v;
				bar_binding_t *binding = &config->bindings[config->bindings_count++];
				/* ??? input_code */
				{
					v = json_ast_node_object_get(obj, string("event_code"));
					ASSERT(v->type == JSON_AST_NODE_TYPE_UINT);
					binding->event_code = (uint32_t)v->value.u;
				}
				{
					v = json_ast_node_object_get(obj, string("command"));
					ASSERT((v->type == JSON_AST_NODE_TYPE_STRING) && (v->value.s.len > 0));
					string_init_string(&binding->command, &gp_alloc, v->value.s);
				}
				{
					v = json_ast_node_object_get(obj, string("release"));
					ASSERT(v->type == JSON_AST_NODE_TYPE_BOOL);
					binding->release = v->value.b;
				}
			}
		}
	}

	if ((node = json_ast_node_object_get(&ast.root, string("outputs")))) {
		ASSERT(node->type == JSON_AST_NODE_TYPE_ARRAY);
		if (node->value.a.count > 0) {
			bool32_t all = FALSE;
			for ( i = 0; i < node->value.a.count; ++i) {
				json_ast_node_t *v = &node->value.a.nodes[i];
				ASSERT(v->type == JSON_AST_NODE_TYPE_STRING);
				if (string_equal(v->value.s, string("*"))) {
					all = TRUE;
					break;
				}
			}
			if (!all) {
				ARRAY_ALLOC(config->outputs, &gp_alloc, node->value.a.count);
				for ( i = 0; i < node->value.a.count; ++i) {
					json_ast_node_t *v = &node->value.a.nodes[i];
					if (v->value.s.len > 0) {
						string_init_string( &config->outputs[config->outputs_count++],
							&gp_alloc, v->value.s);
					}
				}
			}
		}
	}
}

static bool32_t bar_visible_on_output(sw_wayland_output_t *output) {
	config_t *config = &state.config;
	bool32_t visible = !((config->mode == BAR_MODE_INVISIBLE)
		|| ((config->hidden_state == BAR_HIDDEN_STATE_HIDE) && (config->mode == BAR_MODE_HIDE)
		&& !state.visible_by_modifier && !state.visible_by_urgency && !state.visible_by_mode));
	if (visible && (config->outputs_count > 0)) {
		size_t i = 0;
		visible = FALSE;
		for ( ; i < config->outputs_count; ++i) {
			if (string_equal(output->out.name, config->outputs[i])) {
				visible = TRUE;
				break;
			}
		}
	}

	return visible;
}

static bool32_t bar_process_button_event(bar_t *bar,
		uint32_t code, sw_wayland_pointer_button_state_t state_,
		int32_t x, int32_t y, sw_wayland_seat_t *seat) {
	sw_layout_block_t *block;
	size_t i;
	bool32_t released;

	NOTUSED(seat);

#if WITH_TRAY
	if (state.tray.active && state.tray.popup && (state.tray.popup->seat == seat)) {
		if (state_ == SW_WAYLAND_POINTER_BUTTON_STATE_PRESSED) {
			ASSERT(state.tray.popup->parent->in.popups.count == 1);
			MEMSET(&state.tray.popup->parent->in.popups, 0, sizeof(state.tray.popup->parent->in.popups));
			tray_dbusmenu_menu_popup_destroy(state.tray.popup);
		}
		return TRUE;
	}
#endif /* WITH_TRAY */

	for ( block = bar->_.in.root->in._.composite.children.head; block; block = block->next) {
		if ((((layout_block_t *)block)->data)
				&& (x >= block->out.dim.x) && (y >= block->out.dim.y)
				&& (x < (block->out.dim.x + block->out.dim.width))
				&& (y < (block->out.dim.y + block->out.dim.height))) {
			switch (((layout_block_t *)block)->type) {
			case LAYOUT_BLOCK_TYPE_WORKSPACE:
				if (workspace_block_pointer_button((layout_block_t *)block, bar, code, state_)) {
					return TRUE;
				}
				break;
			case LAYOUT_BLOCK_TYPE_STATUS_LINE_I3BAR:
				status_i3bar_block_pointer_button((layout_block_t *)block, bar, code, state_, x, y);
				return TRUE;
#if WITH_TRAY
			case LAYOUT_BLOCK_TYPE_TRAY_SNI_ITEM:
				tray_sni_item_block_pointer_button((layout_block_t *)block, bar, code, state_, x, y, seat);
				return TRUE;
			case LAYOUT_BLOCK_TYPE_TRAY_SNI_ITEM_IMAGE:
			case LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM:
			case LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM_IMAGE:
			case LAYOUT_BLOCK_TYPE_TRAY_DBUSMENU_MENU_ITEM_DECORATOR:
#endif /* WITH_TRAY */
			case LAYOUT_BLOCK_TYPE_BINDING_MODE_INDICATOR:
			case LAYOUT_BLOCK_TYPE_DUMMY:
			default:
				ASSERT_UNREACHABLE;
			}
		}
	}

	released = (state_ == SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED);
	for ( i = 0; i < state.config.bindings_count; ++i) {
		bar_binding_t *binding = &state.config.bindings[i];
		if ((binding->event_code == code) && (binding->release == released)) {
			if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_COMMAND, &binding->command) == -1) {
				su_abort(errno, "sway_ipc_send: write: %s", strerror(errno));
			}
			return TRUE;
		}
	}

	return FALSE;
}

static bool32_t bar_handle_event(sw_wayland_notify_source_t *source, sw_context_t *ctx, sw_wayland_event_t event) {
	NOTUSED(ctx);

	switch (event) {
	case SW_WAYLAND_EVENT_SURFACE_CLOSED: {
		sw_wayland_surface_t *surface = (sw_wayland_surface_t *)source;
		LLIST_POP(&state.sw.in.backend.wayland.layers, surface);
		bar_destroy((bar_t *)source);
		break;
	}
	case SW_WAYLAND_EVENT_POINTER_BUTTON: {
		sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)source;
		bar_process_button_event( (bar_t *)pointer->out.focused_surface,
				pointer->out.btn_code, pointer->out.btn_state,
				pointer->out.pos_x, pointer->out.pos_y, pointer->out.seat);
		break;
	}
	case SW_WAYLAND_EVENT_POINTER_SCROLL: {
		sw_wayland_pointer_t *pointer = (sw_wayland_pointer_t *)source;
		bar_t *bar = (bar_t *)pointer->out.focused_surface;
		bool32_t negative = (pointer->out.scroll_vector_length < 0);
		uint32_t button_code;

		switch (pointer->out.scroll_axis) {
		case SW_WAYLAND_POINTER_AXIS_VERTICAL_SCROLL:
			button_code = (negative ? KEY_SCROLL_UP : KEY_SCROLL_DOWN);
			break;
		case SW_WAYLAND_POINTER_AXIS_HORIZONTAL_SCROLL:
			button_code = (negative ? KEY_SCROLL_LEFT : KEY_SCROLL_RIGHT);
			break;
		default:
			ASSERT_UNREACHABLE;
		}

		if (!bar_process_button_event(bar, button_code, SW_WAYLAND_POINTER_BUTTON_STATE_PRESSED,
					pointer->out.pos_x, pointer->out.pos_y, pointer->out.seat)) {
			bar_process_button_event(bar, button_code, SW_WAYLAND_POINTER_BUTTON_STATE_RELEASED,
				pointer->out.pos_x, pointer->out.pos_y, pointer->out.seat);
		}
		break;
	}
	case SW_WAYLAND_EVENT_POINTER_ENTER:
	case SW_WAYLAND_EVENT_POINTER_LEAVE:
	case SW_WAYLAND_EVENT_POINTER_MOTION:
		break;
	case SW_WAYLAND_EVENT_SURFACE_FAILED_TO_SET_CURSOR_SHAPE:
	case SW_WAYLAND_EVENT_SURFACE_FAILED_TO_SET_DECORATIONS:
	case SW_WAYLAND_EVENT_SURFACE_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK:
	case SW_WAYLAND_EVENT_SURFACE_LAYOUT_FAILED:
	case SW_WAYLAND_EVENT_SURFACE_ERROR_MISSING_PROTOCOL:
	case SW_WAYLAND_EVENT_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER:
		/* TODO: handle/log */
		break;
	default:
		ASSERT_UNREACHABLE;
	}

	return TRUE;
}

static bar_t *bar_create(output_t *output) {
	bar_t *bar;

	if (state.status.active && (state.sw.in.backend.wayland.layers.count == 0)) {
		kill(-state.status.pid, state.status.cont_signal);
	}

	ALLOCCT(bar, &gp_alloc);
	bar->dirty = TRUE;
	bar->_.in.type = SW_WAYLAND_SURFACE_TYPE_LAYER;
	bar->_.in.notify = bar_handle_event;
	bar->_.in.height = -1;
	bar->_.in._.layer.output = &output->_;
	bar->_.in._.layer.exclusive_zone = INT_MIN;

	bar->_.in.root = (sw_layout_block_t *)layout_block_create();
	bar->_.in.root->in.type = SW_LAYOUT_BLOCK_TYPE_COMPOSITE;
	bar->_.in.root->in.expand = SW_LAYOUT_BLOCK_EXPAND_ALL_SIDES_CONTENT;

	return bar;
}

static void process_ipc(void) {
	bool32_t update = FALSE;

	sway_ipc_response_t *response = sway_ipc_receive(
			state.poll_fds[POLL_FD_SWAY_IPC].fd, &scratch_alloc);
	if (response == NULL) {
		su_abort(errno, "sway_ipc_receive: read: %s", strerror(errno));
	}

	switch (response->type) {
	case SWAY_IPC_MESSAGE_TYPE_EVENT_WORKSPACE:
		if (sway_ipc_send(state.poll_fds[POLL_FD_SWAY_IPC].fd, SWAY_IPC_MESSAGE_TYPE_GET_WORKSPACES, NULL) == -1) {
			su_abort(errno, "sway_ipc_send: write: %s", strerror(errno));
		}
		break;
	case SWAY_IPC_MESSAGE_TYPE_GET_WORKSPACES: {
		json_tokener_t tok;
		json_ast_t ast;
		json_tokener_state_t s;
		size_t i;
		output_t *output;

		state.visible_by_urgency = FALSE;
		update = TRUE;

		MEMSET(&tok, 0, sizeof(tok));
		json_ast_reset(&ast);
		json_tokener_set_string(&tok, &scratch_alloc, response->payload);

		s = json_tokener_ast(&tok, &scratch_alloc, &ast, 0, FALSE);
		ASSERT(s == JSON_TOKENER_STATE_SUCCESS);

		ASSERT(ast.root.type == JSON_AST_NODE_TYPE_ARRAY);
		for ( output = (output_t *)state.sw.out.backend.wayland.outputs.head; output; output = (output_t *)output->_.next) {
			for ( i = 0; i < output->workspaces_count; ++i) {
				string_fini(&output->workspaces[i].name, &gp_alloc);
			}
			FREE(&gp_alloc, output->workspaces);
			ARRAY_ALLOC(output->workspaces, &gp_alloc, ast.root.value.a.count);
			output->workspaces_count = 0;
			output->focused = FALSE;
		}

		for ( i = 0; i < ast.root.value.a.count; ++i) {
			workspace_t *workspace = NULL;
			json_ast_node_t *node = &ast.root.value.a.nodes[i];
			json_ast_node_t *output_, *urgent, *name, *num, *focused, *visible;
			ASSERT(node->type == JSON_AST_NODE_TYPE_OBJECT);

			output_ = json_ast_node_object_get(node, string("output"));
			ASSERT((output_ != NULL) && (output_->type == JSON_AST_NODE_TYPE_STRING));
			for ( output = (output_t *)state.sw.out.backend.wayland.outputs.head; output; output = (output_t *)output->_.next) {
				if (string_equal(output->_.out.name, output_->value.s)) {
					workspace = &output->workspaces[output->workspaces_count++];
					break;
				}
			}
			if (UNLIKELY(!workspace)) {
				DEBUG_LOG("warning: workspace on non-existent output: " STRING_PF_FMT, STRING_PF_ARGS(output_->value.s));
				continue;
			}

			urgent = json_ast_node_object_get(node, string("urgent"));
			ASSERT((urgent != NULL) && (urgent->type == JSON_AST_NODE_TYPE_BOOL));
			workspace->urgent = urgent->value.b;

			name = json_ast_node_object_get(node, string("name"));
			ASSERT((name != NULL) && (name->type == JSON_AST_NODE_TYPE_STRING));
			string_init_string(&workspace->name, &gp_alloc, name->value.s);

			num = json_ast_node_object_get(node, string("num"));
			ASSERT((num != NULL) && ((num->type == JSON_AST_NODE_TYPE_INT) || (num->type == JSON_AST_NODE_TYPE_UINT)));
			workspace->num = (int32_t)num->value.i;

			focused = json_ast_node_object_get(node, string("focused"));
			ASSERT((focused != NULL) && (focused->type == JSON_AST_NODE_TYPE_BOOL));
			workspace->focused = focused->value.b;

			visible = json_ast_node_object_get(node, string("visible"));
			ASSERT((visible != NULL) && (visible->type == JSON_AST_NODE_TYPE_BOOL));
			workspace->visible = visible->value.b;

			if (workspace->focused) {
				output->focused = TRUE;
			}
			if (workspace->urgent) {
				state.visible_by_urgency = TRUE;
			}
		}
		break;
	}
	case SWAY_IPC_MESSAGE_TYPE_EVENT_BAR_STATE_UPDATE: {
		json_tokener_t tok;
		json_token_t token;

		MEMSET(&tok, 0, sizeof(tok));
		json_tokener_set_string(&tok, &scratch_alloc, response->payload);

		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_OBJECT_START);

		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_KEY);
		ASSERT(string_equal(token.value.s, string("id")));

		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_STRING);
		if (string_equal(token.value.s, state.bar_id)) {

			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_KEY);
			ASSERT(string_equal(token.value.s, string("visible_by_modifier")));

			json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_BOOL);
			state.visible_by_modifier = token.value.b;

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
		string_t *text = &state.binding_mode_indicator_text;
		json_tokener_t tok;
		json_token_t token;

		MEMSET(&tok, 0, sizeof(tok));
		json_tokener_set_string(&tok, &scratch_alloc, response->payload);

		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_OBJECT_START);

		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_KEY);
		ASSERT(string_equal(token.value.s, (response->type == SWAY_IPC_MESSAGE_TYPE_EVENT_MODE) ? string("change") : string("name")));

		json_tokener_advance_assert_type(&tok, &scratch_alloc, &token, JSON_TOKEN_TYPE_STRING);

		/* TODO: pango_markup */

		state.visible_by_mode = TRUE;
		update = TRUE;

		if ((token.value.s.len == 0) || string_equal(token.value.s, string("default"))) {
			string_fini(text, &gp_alloc);
			MEMSET(text, 0, sizeof(*text));
			state.visible_by_mode = FALSE;
		} else if (!string_equal(token.value.s, *text)) {
			string_fini(text, &gp_alloc);
			string_init_string(text, &gp_alloc, token.value.s);
		}
		break;
	}
	case SWAY_IPC_MESSAGE_TYPE_EVENT_BARCONFIG_UPDATE:
		config_update(response->payload);
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
		sw_wayland_output_t *output = state.sw.out.backend.wayland.outputs.head;
		for ( ; output; output = output->next) {
			if (bar_visible_on_output(output)) {
				bar_t *bar = NULL;
				sw_wayland_surface_t *b = state.sw.in.backend.wayland.layers.head;
				for ( ; b; b = b->next) {
					if (b->in._.layer.output == output) {
						bar = (bar_t *)b;
						break;
					}
				}
				if (!bar) {
					bar = bar_create((output_t *)output);
					LLIST_APPEND_TAIL(&state.sw.in.backend.wayland.layers, &bar->_);
				} else {
					bar->dirty = TRUE;
				}
			} else {
				sw_wayland_surface_t *bar = state.sw.in.backend.wayland.layers.head;
				for ( ; bar; bar = bar->next) {
					if (bar->in._.layer.output == output) {
						LLIST_POP(&state.sw.in.backend.wayland.layers, bar);
						bar_destroy((bar_t *)bar);
						break;
					}
				}
			}
		}
	}
}

static bool32_t init_sway_ipc(int fd) {
	sway_ipc_response_t *response;
	string_t subscribe_payload;

	state.poll_fds[POLL_FD_SWAY_IPC].fd = fd;
	state.poll_fds[POLL_FD_SWAY_IPC].events = POLLIN;

	if (sway_ipc_send(fd, SWAY_IPC_MESSAGE_TYPE_GET_BAR_CONFIG, &state.bar_id) == -1) {
		return FALSE;
	}
	response = sway_ipc_receive(fd, &scratch_alloc);
	if (!response) {
		return FALSE;
	}
	ASSERT(response->type == SWAY_IPC_MESSAGE_TYPE_GET_BAR_CONFIG);
	config_update(response->payload);

	subscribe_payload = string("[\"barconfig_update\",\"bar_state_update\",\"mode\",\"workspace\"]");
	if (sway_ipc_send(fd, SWAY_IPC_MESSAGE_TYPE_SUBSCRIBE, &subscribe_payload) == -1) {
		return FALSE;
	}

	return TRUE;
}

static void handle_signal(int sig) {
	NOTUSED(sig);
	DEBUG_LOG("%d: %s", sig, strsignal(sig));
	state.running = FALSE;
}

static void setup(int argc, char *argv[]) {
	static struct sigaction sigact;

	int sway_ipc_fd;
	static char sway_ipc_socket_path[PATH_MAX];

	setlocale(LC_ALL, "");
	if (!locale_is_utf8()) {
		su_abort(1, "failed to set UTF-8 locale");
	}

	ARGPARSE_BEGIN {
		switch (ARGPARSE_KEY) {
		case 's': {
			char *s = ARGPARSE_VALUE;
			if (s) {
				STRNCPY(sway_ipc_socket_path, s, sizeof(sway_ipc_socket_path));
			}
			break;
		}
		case 'b': {
			char *s = ARGPARSE_VALUE;
			if (s) {
				state.bar_id = string(s);
			}
			break;
		}
		case 'v':
			su_abort(0, "sw_swaybar version" ); /* TODO */
		default:
			su_abort( (ARGPARSE_KEY != 'h'),
				"Usage: sw_swaybar [options...]\n"
				"\n"
				"  -h,         Show this help message and quit.\n"
				"  -v,         Show the version and quit.\n"
				"  -s, <path>  Connect to sway via socket specified in <path>.\n"
				"  -b, <id>    Bar ID for which to get the configuration.\n"
				"\n"
				" PLEASE NOTE that you can use swaybar_command field in your\n"
				" configuration file to let sway start sw_swaybar automatically.\n"
				" You should never need to start it manually.\n");
		}
	} ARGPARSE_END

	if (state.bar_id.len == 0) {
		su_abort(1, "No bar id passed. Provide -b or use swaybar_command in sway config file");
	}

	arena_init(&state.scratch_arena, &gp_alloc, 16384);
	su_hash_table__su_file_cache_t__init(&state.icon_cache, &gp_alloc, 512);

    state.poll_fds[POLL_FD_STATUS].fd = -1;
	state.poll_fds[POLL_FD_STATUS].events = POLLIN;
    state.poll_fds[POLL_FD_SNI_SERVER].fd = -1;
	state.poll_fds[POLL_FD_SNI_SERVER].events = 0;

	if (!*sway_ipc_socket_path) {
		if (!sway_ipc_get_socket_path(sway_ipc_socket_path)) {
			su_abort(ESOCKTNOSUPPORT, "Failed to get sway ipc socket path");
		}
	}
	sway_ipc_fd = sway_ipc_connect(sway_ipc_socket_path);
	if (sway_ipc_fd == -1) {
		su_abort(errno, "Failed to connect to sway ipc socket '%s': %s", sway_ipc_socket_path, strerror(errno));
	}
	if (!init_sway_ipc(sway_ipc_fd)) {
		su_abort(errno, "Failed to initialize sway ipc: %s", strerror(errno));
	}

	sigact.sa_handler = handle_signal;
	/* ? TODO: error check */
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGPIPE, &sigact, NULL);

	state.sw.in.backend_type = SW_BACKEND_TYPE_WAYLAND;
	state.sw.in.gp_alloc = &gp_alloc;
	state.sw.in.scratch_alloc = &scratch_alloc;
	state.sw.in.backend.wayland.output_create = output_create_sw;
	state.sw.in.backend.wayland.seat_create = seat_create_sw;
	state.running = TRUE;
	state.update = TRUE;
}

static void run(void) {
	while (state.running) {
		static short err = (POLLHUP | POLLERR | POLLNVAL);

		int timeout, sw_timeout, tray_timeout = -1;
		bool32_t process_tray = FALSE;

		if (state.update) {
			if (!sw_set(&state.sw)) {
				su_abort(errno, "sw_set: %s", strerror(errno));
			}
			state.poll_fds[POLL_FD_SW] = state.sw.out.backend.wayland.pfd;
			state.update = FALSE;
		}
		if (!sw_flush(&state.sw)) {
			su_abort(errno, "sw_flush: %s", strerror(errno));
		}

		arena_reset(&state.scratch_arena, &gp_alloc);

#if WITH_TRAY
		if (state.tray.active) {
			int64_t absolute_timeout_ms;
			int ret = sni_server_get_poll_info(&state.poll_fds[POLL_FD_SNI_SERVER], &absolute_timeout_ms);
			if (ret < 0) {
				su_abort(-ret, "sni_server_get_poll_info: %s", strerror(-ret));
			}
			tray_timeout = (absolute_timeout_ms > 0) ? (int)(absolute_timeout_ms - now_ms(CLOCK_MONOTONIC)) : (int)absolute_timeout_ms;
		}
#endif /* WITH_TRAY */

		sw_timeout = (state.sw.out.t > 0) ? (int)(state.sw.out.t - now_ms(CLOCK_MONOTONIC)) : (int)state.sw.out.t;

		timeout = ((sw_timeout > 0) && (tray_timeout > 0))
			? MIN(sw_timeout, tray_timeout) : MAX(sw_timeout, tray_timeout);
		switch (poll(state.poll_fds, LENGTH(state.poll_fds), timeout)) {
		case -1:
			if (errno != EINTR) {
				su_abort(errno, "poll: %s", strerror(errno));
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

		if (state.poll_fds[POLL_FD_SW].revents & (state.poll_fds[POLL_FD_SW].events | err)) {
			if (!sw_process(&state.sw)) {
				su_abort(errno, "sw_process: %s", strerror(errno));
			}
		}

		if (state.poll_fds[POLL_FD_SWAY_IPC].revents & (state.poll_fds[POLL_FD_SWAY_IPC].events | err)) {
			process_ipc();
		}

		if (state.status.active) {
			if (state.poll_fds[POLL_FD_STATUS].revents & (state.poll_fds[POLL_FD_STATUS].events | err)) {
				if (status_process()) {
					bars_set_dirty();
				}
			}
		}

#if WITH_TRAY
		if (state.tray.active && (process_tray || (state.poll_fds[POLL_FD_SNI_SERVER].revents
				& (state.poll_fds[POLL_FD_SNI_SERVER].events | err)))) {
			int ret = sni_server_process();
			if (ret < 0) {
				su_abort(-ret, "sni_server_process: %s", strerror(-ret));
			}
		}
#endif /* WITH_TRAY */

		{
			bar_t *bar = (bar_t *)state.sw.in.backend.wayland.layers.head;
			for ( ; bar; bar = (bar_t *)bar->_.next) {
				if (bar->dirty) {
					bar_update(bar);
					bar->dirty = FALSE;
				}
			}
		}
	}
}

static void cleanup(void) {
	if (state.status.active) {
		status_fini();
	}

#if WITH_TRAY
	if (state.tray.active) {
		tray_fini();
	}
#endif /* WITH_TRAY */

	close(state.poll_fds[POLL_FD_SWAY_IPC].fd);

#if DEBUG
	{
		size_t i;
		config_t *config = &state.config;

		for ( i = 0; i < state.icon_cache.capacity; ++i) {
			file_cache_t *c = &state.icon_cache.items[i];
			FREE(&gp_alloc, c->data.ptr);
			string_fini(&c->key, &gp_alloc);
		}
		su_hash_table__su_file_cache_t__fini(&state.icon_cache, &gp_alloc);

		for ( i = 0; i < config->bindings_count; ++i) {
			string_fini(&config->bindings[i].command, &gp_alloc);
		}
		FREE(&gp_alloc, config->bindings);
		string_fini(&config->font, &gp_alloc);
		for ( i = 0; i < config->outputs_count; ++i) {
			string_fini(&config->outputs[i], &gp_alloc);
		}
		FREE(&gp_alloc, config->outputs);

#if WITH_TRAY
		for ( i = 0; i < config->tray_outputs_count; ++i) {
			string_fini(&config->tray_outputs[i], &gp_alloc);
		}
		FREE(&gp_alloc, config->tray_outputs);
		FREE(&gp_alloc, config->tray_bindings);
		string_fini(&config->tray_icon_theme, &gp_alloc);
#endif /* WITH_TRAY */

		string_fini(&config->status_command, &gp_alloc);
		string_fini(&config->separator_symbol, &gp_alloc);

		string_fini(&state.binding_mode_indicator_text, &gp_alloc);
		string_fini(&state.bar_id, &gp_alloc);

		arena_fini(&state.scratch_arena, &gp_alloc);
	}
#endif /* DEBUG */

	sw_cleanup(&state.sw);
}

int main(int argc, char *argv[]) {
	setup(argc, argv);
	run();
	cleanup();

	return 0;
}
