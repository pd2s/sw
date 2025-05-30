#if !defined(SNI_SERVER_H)
#define SNI_SERVER_H

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#include "util.h"

#if HAS_INCLUDE(<sd-bus.h>)
#include <sd-bus.h>
#else
#include <basu/sd-bus.h>
#endif // HAS_INCLUDE

typedef struct sni_item *sni_item_create_func_t(string_t id);
typedef void sni_item_destroy_func_t(struct sni_item *);
typedef void sni_item_properties_updated_func_t(struct sni_item *);
typedef void sni_item_dbusmenu_menu_updated_func_t(struct sni_item *);

STATIC_ASSERT(TYPES_COMPATIBLE(TYPEOF(SNI_ITEM_CREATE_FUNC), sni_item_create_func_t));
STATIC_ASSERT(TYPES_COMPATIBLE(TYPEOF(SNI_ITEM_DESTROY_FUNC), sni_item_destroy_func_t));
STATIC_ASSERT(TYPES_COMPATIBLE(TYPEOF(SNI_ITEM_PROPERTIES_UPDATED_FUNC), sni_item_properties_updated_func_t));
STATIC_ASSERT(TYPES_COMPATIBLE(TYPEOF(SNI_ITEM_DBUSMENU_MENU_UPDATED_FUNC), sni_item_dbusmenu_menu_updated_func_t));

enum sni_dbusmenu_menu_item_type {
    SNI_DBUSMENU_MENU_ITEM_TYPE_STANDARD,
    SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR,
};

enum sni_dbusmenu_menu_item_toggle_type {
    SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_NONE,
    SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_CHECKMARK,
    SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_RADIO,
};

enum sni_dbusmenu_menu_item_disposition {
    SNI_DBUSMENU_MENU_ITEM_DISPOSITION_NORNAL,
    SNI_DBUSMENU_MENU_ITEM_DISPOSITION_INFORMATIVE,
    SNI_DBUSMENU_MENU_ITEM_DISPOSITION_WARNING,
    SNI_DBUSMENU_MENU_ITEM_DISPOSITION_ALERT,
};

enum sni_dbusmenu_menu_item_event_type {
    SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLICKED,
    SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_HOVERED,
    SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_OPENED,
    SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLOSED,
};

struct sni_dbusmenu_menu_item {
    struct sni_dbusmenu_menu *parent_menu;
    struct sni_dbusmenu_menu *submenu; // may be NULL
    int id;
    enum sni_dbusmenu_menu_item_type type;
    string_t label;
    int enabled; // bool
    int visible; // bool
    string_t icon_name;
    struct { // png data of the icon
        void *bytes; // may be NULL
        size_t nbytes;
    } icon_data;
    void *shortcut; // NOT IMPLEMENTED
    enum sni_dbusmenu_menu_item_toggle_type toggle_type;
    int toggle_state; // 0 - off, 1 - on, else - indeterminate
    enum sni_dbusmenu_menu_item_disposition disposition;
    int activation_requested; // bool  NOT IMPLEMENTED
};

typedef struct sni_dbusmenu_menu_item struct_sni_dbusmenu_menu_item;
ARRAY_DECLARE_DEFINE(struct_sni_dbusmenu_menu_item)

struct sni_dbusmenu_menu {
    struct sni_dbusmenu *dbusmenu;
    struct sni_dbusmenu_menu_item *parent_menu_item; // NULL when root
	array_struct_sni_dbusmenu_menu_item_t menu_items;
	size_t depth;
};

enum sni_dbusmenu_status {
    SNI_DBUSMENU_STATUS_INVALID,
    SNI_DBUSMENU_STATUS_NORMAL,
    SNI_DBUSMENU_STATUS_NOTICE,
};

enum sni_dbusmenu_text_direction {
    SNI_DBUSMENU_TEXT_DIRECTION_INVALID,
    SNI_DBUSMENU_TEXT_DIRECTION_LEFT_TO_RIGHT,
    SNI_DBUSMENU_TEXT_DIRECTION_RIGHT_TO_LEFT,
};

struct sni_dbusmenu_properties {
    array_string_t_t icon_theme_path;
    enum sni_dbusmenu_status status;
    enum sni_dbusmenu_text_direction text_direction;
};

struct sni_dbusmenu {
	struct sni_item *item;
    struct sni_dbusmenu_properties *properties; // may be NULL
    struct sni_dbusmenu_menu *menu; // may be NULL
};

struct sni_item_pixmap {
    int width;
    int height;
    uint32_t pixels[]; // ARGB32, native byte order
	// width * height * 4
};

typedef struct sni_item_pixmap* struct_sni_item_pixmap_ptr;
ARRAY_DECLARE_DEFINE(struct_sni_item_pixmap_ptr)

enum sni_item_status {
    SNI_ITEM_STATUS_INVALID,
    SNI_ITEM_STATUS_PASSIVE,
    SNI_ITEM_STATUS_ACTIVE,
    SNI_ITEM_STATUS_NEEDS_ATTENTION,
};

enum sni_item_category {
    SNI_ITEM_CATEGORY_INVALID,
    SNI_ITEM_CATEGORY_APPLICATION_STATUS,
    SNI_ITEM_CATEGORY_COMMUNICATIONS,
    SNI_ITEM_CATEGORY_SYSTEM_SERVICES,
    SNI_ITEM_CATEGORY_HARDWARE,
};

struct sni_item_properties {
    // every field may be NULL
    string_t icon_name;
    string_t icon_theme_path;
    array_struct_sni_item_pixmap_ptr_t icon_pixmap; // qsorted size descending
    enum sni_item_status status;
    enum sni_item_category category;
    string_t menu;
    string_t attention_icon_name;
    array_struct_sni_item_pixmap_ptr_t attention_icon_pixmap; // qsorted size descending
    int item_is_menu; // bool
    int window_id;
    string_t id;
    string_t title;
    string_t attention_movie_name;
    string_t overlay_icon_name;
    array_struct_sni_item_pixmap_ptr_t overlay_icon_pixmap; // qsorted size descending
    struct {
        string_t icon_name;
        array_struct_sni_item_pixmap_ptr_t icon_pixmap; // qsorted size descending
        string_t title;
        string_t text;
    } tooltip;
};

enum sni_item_scroll_orientation {
    SNI_ITEM_SCROLL_ORIENTATION_VERTICAL,
    SNI_ITEM_SCROLL_ORIENTATION_HORIZONTAL,
};

typedef struct sni__slot struct_sni__slot;
struct sni__slot {
	struct sni_item *item;
	sd_bus_slot *slot;
	LIST_STRUCT_FIELDS(struct_sni__slot);
};

LIST_DECLARE_DEFINE(struct_sni__slot)

struct sni_item {
    struct sni_item_properties *properties; // may be NULL
    struct sni_dbusmenu *dbusmenu; // may be NULL

    // dbus
    string_t watcher_id;
    string_t service;
    string_t path;

	list_struct_sni__slot_t slots;
};

typedef struct sni_item* struct_sni_item_ptr;
ARRAY_DECLARE_DEFINE(struct_sni_item_ptr)

struct sni_server {
    sd_bus *bus;
    struct {
        string_t interface;
        array_struct_sni_item_ptr_t items;
    } host;
    struct {
        array_string_t_t items;
        array_string_t_t hosts;
    } watcher;
};

static int sni_server_init(void);
static void sni_server_fini(void);
static int sni_server_get_poll_info(int *fd, int *events, uint64_t *timeout);
static int sni_server_process(void);

static bool32_t sni_item_init(struct sni_item *, string_t id);
static void sni_item_fini(struct sni_item *);
static int sni_item_context_menu(struct sni_item *, int x, int y);
static int sni_item_context_menu_async(struct sni_item *, int x, int y);
static int sni_item_activate(struct sni_item *, int x, int y);
static int sni_item_activate_async(struct sni_item *, int x, int y);
static int sni_item_secondary_activate(struct sni_item *, int x, int y);
static int sni_item_secondary_activate_async(struct sni_item *, int x, int y);
static int sni_item_scroll(struct sni_item *, int delta, enum sni_item_scroll_orientation);
static int sni_item_scroll_async(struct sni_item *, int delta, enum sni_item_scroll_orientation);

static int sni_dbusmenu_menu_about_to_show(struct sni_dbusmenu_menu *, bool32_t async);

static int sni_dbusmenu_menu_item_event(struct sni_dbusmenu_menu_item *,
	enum sni_dbusmenu_menu_item_event_type, bool32_t async);

static struct sni_server *sni_server;

static const char sni__watcher_interface[] = "org.kde.StatusNotifierWatcher";
static const char sni__watcher_obj_path[] = "/StatusNotifierWatcher";
static int sni__watcher_protocol_version = 0;

static const char sni__item_interface[] = "org.kde.StatusNotifierItem";

static const char sni__dbusmenu_interface[] = "com.canonical.dbusmenu";

static int sni__item_pixmap_size_descending_qsort(const void *a, const void *b) {
	struct sni_item_pixmap *p1 = *(struct sni_item_pixmap **)a;
	struct sni_item_pixmap *p2 = *(struct sni_item_pixmap **)b;
	return (p2->width * p2->height) - (p1->width * p1->height);
}

static void sni__item_read_pixmap(sd_bus_message *msg, array_struct_sni_item_pixmap_ptr_t *dest) {
	array_struct_sni_item_pixmap_ptr_init(dest, 10);

	sd_bus_message_enter_container(msg, 'a', "(iiay)");
	while (sd_bus_message_enter_container(msg, 'r', "iiay") == 1) {
		int width, height;
		const void *bytes;
		size_t nbytes;
		sd_bus_message_read_basic(msg, 'i', &width);
		sd_bus_message_read_basic(msg, 'i', &height);
		sd_bus_message_read_array(msg, 'y', &bytes, &nbytes);
		if (((size_t)width * (size_t)height * 4) == nbytes) {
			struct sni_item_pixmap *pixmap = malloc(
				SIZEOF(struct sni_item_pixmap) + nbytes);
			pixmap->width = width;
			pixmap->height = height;
			for (int i = 0; i < (width * height); ++i) {
				pixmap->pixels[i] = ntohl(((const uint32_t *)bytes)[i]);
			}
			array_struct_sni_item_pixmap_ptr_add(dest, pixmap);
		}
		sd_bus_message_exit_container(msg);
	}
	sd_bus_message_exit_container(msg);

	array_struct_sni_item_pixmap_ptr_qsort(dest, sni__item_pixmap_size_descending_qsort);
}

static void sni__item_properties_destroy(struct sni_item_properties *properties) {
	if (properties == NULL) {
		return;
	}

	string_fini(&properties->icon_name);
	string_fini(&properties->icon_theme_path);
	for (size_t i = 0; i < properties->icon_pixmap.len; ++i) {
		free(array_struct_sni_item_pixmap_ptr_get(&properties->icon_pixmap, i));
	}
	array_struct_sni_item_pixmap_ptr_fini(&properties->icon_pixmap);
	string_fini(&properties->menu);
	string_fini(&properties->attention_icon_name);
	for (size_t i = 0; i < properties->attention_icon_pixmap.len; ++i) {
		free(array_struct_sni_item_pixmap_ptr_get(&properties->attention_icon_pixmap, i));
	}
	array_struct_sni_item_pixmap_ptr_fini(&properties->attention_icon_pixmap);
	string_fini(&properties->id);
	string_fini(&properties->title);
	string_fini(&properties->attention_movie_name);
	string_fini(&properties->overlay_icon_name);
	for (size_t i = 0; i < properties->overlay_icon_pixmap.len; ++i) {
		free(array_struct_sni_item_pixmap_ptr_get(&properties->overlay_icon_pixmap, i));
	}
	array_struct_sni_item_pixmap_ptr_fini(&properties->overlay_icon_pixmap);
	for (size_t i = 0; i < properties->tooltip.icon_pixmap.len; ++i) {
		free(array_struct_sni_item_pixmap_ptr_get(&properties->tooltip.icon_pixmap, i));
	}
	array_struct_sni_item_pixmap_ptr_fini(&properties->tooltip.icon_pixmap);
	string_fini(&properties->tooltip.icon_name);
	string_fini(&properties->tooltip.title);
	string_fini(&properties->tooltip.text);

	free(properties);
}

static void sni__slot_free(struct sni__slot *slot) {
	sd_bus_slot_unref(slot->slot);
	list_struct_sni__slot_pop(&slot->item->slots, slot);
	free(slot);
}

static void sni__dbusmenu_menu_destroy(struct sni_dbusmenu_menu *menu) {
	if (menu == NULL) {
		return;
	}

	for (size_t i = 0; i < menu->menu_items.len; ++i) {
		struct sni_dbusmenu_menu_item menu_item = array_struct_sni_dbusmenu_menu_item_get(
			&menu->menu_items, i);
		sni__dbusmenu_menu_destroy(menu_item.submenu);
		string_fini(&menu_item.label);
		string_fini(&menu_item.icon_name);
		free(menu_item.icon_data.bytes);
	}
	array_struct_sni_dbusmenu_menu_item_fini(&menu->menu_items);

	free(menu);
}

static void sni__dbusmenu_properties_destroy(struct sni_dbusmenu_properties *properties) {
	if (properties == NULL) {
		return;
	}

	for (size_t i = 0; i < properties->icon_theme_path.len; ++i) {
		string_fini(array_string_t_get_ptr(&properties->icon_theme_path, i));
	}
	array_string_t_fini(&properties->icon_theme_path);

	free(properties);
}

static void sni__dbusmenu_destroy(struct sni_dbusmenu *dbusmenu) {
	if (dbusmenu == NULL) {
		return;
	}

	sni__dbusmenu_menu_destroy(dbusmenu->menu);
	sni__dbusmenu_properties_destroy(dbusmenu->properties);

	free(dbusmenu);
}

static struct sni_dbusmenu_menu *sni__dbusmenu_menu_create(sd_bus_message *msg,
		struct sni_dbusmenu *dbusmenu, struct sni_dbusmenu_menu_item *parent_menu_item) {
	struct sni_dbusmenu_menu *menu = calloc(1, SIZEOF(struct sni_dbusmenu_menu));
	menu->dbusmenu = dbusmenu;
	menu->parent_menu_item = parent_menu_item;
	if (parent_menu_item) {
		menu->depth = parent_menu_item->parent_menu->depth + 1;
	}

	array_struct_sni_dbusmenu_menu_item_init(&menu->menu_items, 30);

	while (sd_bus_message_at_end(msg, 0) == 0) {
		sd_bus_message_enter_container(msg, 'v', "(ia{sv}av)");
		sd_bus_message_enter_container(msg, 'r', "ia{sv}av");

		struct sni_dbusmenu_menu_item menu_item = { 0 };
		menu_item.parent_menu = menu;
		//menu_item.type = SNI_DBUSMENU_MENU_ITEM_TYPE_STANDARD;
		menu_item.enabled = 1;
		menu_item.visible = 1;
		//menu_item.toggle_type = SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_NONE;
		menu_item.toggle_state = -1;
		//menu_item.disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_NORNAL;
		sd_bus_message_read_basic(msg, 'i', &menu_item.id);

		bool32_t children = FALSE;
		sd_bus_message_enter_container(msg, 'a', "{sv}");
		while (sd_bus_message_enter_container(msg, 'e', "sv") == 1) {
			const char *key;
			sd_bus_message_read_basic(msg, 's', &key);
			size_t len = strlen(key);
			if (len == 0) {
				goto exit_con;
			}
			string_t key_str = {
				.s = (char *)key,
				.len = len,
				.free_contents = FALSE,
				.nul_terminated = TRUE,
			};
			sd_bus_message_enter_container(msg, 'v', NULL);
			if (string_equal(key_str, STRING_LITERAL("type"))) {
				const char *type;
				sd_bus_message_read_basic(msg, 's', &type);
				if (((len = strlen(type)) > 0) && string_equal(STRING_LITERAL("separator"), (string_t){
						.s = (char *)type,
						.len = len,
						.free_contents = FALSE,
						.nul_terminated = TRUE,
				})) {
					menu_item.type = SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR;
				}
			} else if (string_equal(key_str, STRING_LITERAL("label"))) {
				const char *label;
				sd_bus_message_read_basic(msg, 's', &label);
				menu_item.label.s = malloc(strlen(label) + 1);
				size_t l = 0;
				for (const char *c = label; *c; ++c) {
					if ((*c == '_') && (!*++c)) {
						break;
					}
					menu_item.label.s[l++] = *c;
				}
				menu_item.label.s[l] = '\0';
				menu_item.label.len = l;
				menu_item.label.free_contents = TRUE;
				menu_item.label.nul_terminated = TRUE;
				// TODO: handle '_', '__' properly
			} else if (string_equal(key_str, STRING_LITERAL("enabled"))) {
				sd_bus_message_read_basic(msg, 'b', &menu_item.enabled);
			} else if (string_equal(key_str, STRING_LITERAL("visible"))) {
				sd_bus_message_read_basic(msg, 'b', &menu_item.visible);
			} else if (string_equal(key_str, STRING_LITERAL("icon-name"))) {
				const char *icon_name;
				sd_bus_message_read_basic(msg, 's', &icon_name);
				if ((len = strlen(icon_name)) > 0) {
					string_init_len(&menu_item.icon_name, icon_name, len, TRUE);
				}
			} else if (string_equal(key_str, STRING_LITERAL("icon-data"))) {
				const void *bytes;
				sd_bus_message_read_array(msg, 'y', &bytes, &menu_item.icon_data.nbytes);
				menu_item.icon_data.bytes = malloc(menu_item.icon_data.nbytes);
				memcpy(menu_item.icon_data.bytes, bytes, menu_item.icon_data.nbytes);
			//} else if (string_equal(key_str, STRING_LITERAL("shortcut"))) {
			} else if (string_equal(key_str, STRING_LITERAL("toggle-type"))) {
				const char *toggle_type;
				sd_bus_message_read_basic(msg, 's', &toggle_type);
				if ((len = strlen(toggle_type)) > 0) {
					string_t toggle_type_str = {
						.s = (char *)toggle_type,
						.len = len,
						.free_contents = FALSE,
						.nul_terminated = TRUE,
					};
					if (string_equal(toggle_type_str, STRING_LITERAL("checkmark"))) {
						menu_item.toggle_type = SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_CHECKMARK;
					} else if (string_equal(toggle_type_str, STRING_LITERAL("radio"))) {
						menu_item.toggle_type = SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_RADIO;
					}
				}
			} else if (string_equal(key_str, STRING_LITERAL("toggle-state"))) {
				sd_bus_message_read_basic(msg, 'i', &menu_item.toggle_state);
			} else if (string_equal(key_str, STRING_LITERAL("children-display"))) {
				const char *children_display;
				sd_bus_message_read_basic(msg, 's', &children_display);
				if (((len = strlen(children_display)) > 0) && string_equal(STRING_LITERAL("submenu"), (string_t){
						.s = (char *)children_display,
						.len = len,
						.free_contents = FALSE,
						.nul_terminated = TRUE,
				})) {
					children = TRUE;
				}
			} else if (string_equal(key_str, STRING_LITERAL("disposition"))) {
				const char *disposition;
				sd_bus_message_read_basic(msg, 's', &disposition);
				if ((len = strlen(disposition)) > 0) {
					string_t disposition_str = {
						.s = (char *)disposition,
						.len = len,
						.free_contents = FALSE,
						.nul_terminated = TRUE,
					};
					if (string_equal(disposition_str, STRING_LITERAL("normal"))) {
						menu_item.disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_NORNAL;
					} else if (string_equal(disposition_str, STRING_LITERAL("informative"))) {
						menu_item.disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_INFORMATIVE;
					} else if (string_equal(disposition_str, STRING_LITERAL("warning"))) {
						menu_item.disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_WARNING;
					} else if (string_equal(disposition_str, STRING_LITERAL("alert"))) {
						menu_item.disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_ALERT;
					}
				}
			} else {
				// TODO: "shortcut"
				sd_bus_message_skip(msg, NULL);
			}
			sd_bus_message_exit_container(msg);
exit_con:
			sd_bus_message_exit_container(msg);
		}
		sd_bus_message_exit_container(msg);

		struct sni_dbusmenu_menu_item *menu_item_ = array_struct_sni_dbusmenu_menu_item_add(
			&menu->menu_items, menu_item);

		sd_bus_message_enter_container(msg, 'a', "v");
		if (children) { // && (menu_item->id != 0)) {
			menu_item_->submenu = sni__dbusmenu_menu_create(msg, dbusmenu, menu_item_);
		}
		sd_bus_message_exit_container(msg);
		sd_bus_message_exit_container(msg);
		sd_bus_message_exit_container(msg);
	}

	return menu;
}

static int sni__dbusmenu_handle_get_layout(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)ret_error;
	struct sni__slot *slot = data;
	struct sni_item *item = slot->item;
	struct sni_dbusmenu *dbusmenu = item->dbusmenu;

	sni__slot_free(slot);

	struct sni_dbusmenu_menu *old_menu = dbusmenu->menu;

	int ret = sd_bus_message_skip(msg, "u");
	if (ret < 0) {
		dbusmenu->menu = NULL;
	} else {
		dbusmenu->menu = sni__dbusmenu_menu_create(msg, dbusmenu, NULL);
		ret = 1;
	}

	SNI_ITEM_DBUSMENU_MENU_UPDATED_FUNC(item);

	sni__dbusmenu_menu_destroy(old_menu);

	return ret;
}

static int sni__dbusmenu_get_layout(struct sni_dbusmenu *dbusmenu) {
	struct sni_item *item = dbusmenu->item;
	struct sni__slot *slot = malloc(SIZEOF(struct sni__slot));
	int ret = sd_bus_call_method_async(sni_server->bus, &slot->slot, item->service.s,
			item->properties->menu.s, sni__dbusmenu_interface, "GetLayout",
			sni__dbusmenu_handle_get_layout, slot, "iias", 0, -1, NULL);
	if (ret < 0) {
		free(slot);
		return ret;
	}

	slot->item = item;
	list_struct_sni__slot_insert_head(&item->slots, slot);

	return 1;
}

static int sni__dbusmenu_handle_signal(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)msg;
	(void)ret_error;
	// ? TODO: error check
	struct sni__slot *slot = data;

	// TODO: ItemActivationRequested

	sni__dbusmenu_get_layout(slot->item->dbusmenu);
	return 1;
}

static int sni__dbusmenu_handle_get_properties(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)ret_error;
	struct sni__slot *slot = data;
	struct sni_dbusmenu *dbusmenu = slot->item->dbusmenu;

	sni__slot_free(slot);

	int ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
	if (ret < 0) {
		return ret;
	}

	struct sni_dbusmenu_properties *props = calloc(1, SIZEOF(struct sni_dbusmenu_properties));
	while (sd_bus_message_enter_container(msg, 'e', "sv") == 1) {
		const char *key;
		sd_bus_message_read_basic(msg, 's', &key);
		size_t len = strlen(key);
		if (len == 0) {
			goto exit_con;
		}
		string_t key_str = {
			.s = (char *)key,
			.len = len,
			.free_contents = FALSE,
			.nul_terminated = TRUE,
		};
		sd_bus_message_enter_container(msg, 'v', NULL);
		if (string_equal(key_str, STRING_LITERAL("IconThemePath"))) {
			char **icon_theme_path;
			sd_bus_message_read_strv(msg, &icon_theme_path);
			if (icon_theme_path) {
				array_string_t_init(&props->icon_theme_path, 4);
				for (char **p = icon_theme_path; *p != NULL; ++p) {
					char *path = *p;
					if ((len = strlen(path)) > 0) {
						string_init_len(array_string_t_add_uninitialized(&props->icon_theme_path), path, len, TRUE);
					} else {
						free(path);
					}
				}
				free(icon_theme_path);
			}
		} else if (string_equal(key_str, STRING_LITERAL("Status"))) {
			const char *status;
			sd_bus_message_read_basic(msg, 's', &status);
			if ((len = strlen(status)) > 0) {
				string_t status_str = {
					.s = (char *)status,
					.len = len,
					.free_contents = FALSE,
					.nul_terminated = TRUE,
				};
				if (string_equal(status_str, STRING_LITERAL("normal"))) {
					props->status = SNI_DBUSMENU_STATUS_NORMAL;
				} else if (string_equal(status_str, STRING_LITERAL("notice"))) {
					props->status = SNI_DBUSMENU_STATUS_NOTICE;
				}
			}
		} else if (string_equal(key_str, STRING_LITERAL("TextDirection"))) {
			const char *text_direction;
			sd_bus_message_read_basic(msg, 's', &text_direction);
			if ((len = strlen(text_direction)) > 0) {
				string_t text_direction_str = {
					.s = (char *)text_direction,
					.len = len,
					.free_contents = FALSE,
					.nul_terminated = TRUE,
				};
				if (string_equal(text_direction_str, STRING_LITERAL("ltr"))) {
					props->text_direction = SNI_DBUSMENU_TEXT_DIRECTION_LEFT_TO_RIGHT;
				} else if (string_equal(text_direction_str, STRING_LITERAL("rtl"))) {
					props->text_direction = SNI_DBUSMENU_TEXT_DIRECTION_RIGHT_TO_LEFT;
				}
			}
		} else {
			// ignored: Version
			sd_bus_message_skip(msg, NULL);
		}
		sd_bus_message_exit_container(msg);
exit_con:
		sd_bus_message_exit_container(msg);
	}
	//sd_bus_message_exit_container(msg);

	dbusmenu->properties = props;
	return 1;
}

static struct sni_dbusmenu *sni__dbusmenu_create(struct sni_item *item) {
	if (item->properties->menu.len == 0) {
		return NULL;
	}

	struct sni_dbusmenu *dbusmenu = calloc(1, SIZEOF(struct sni_dbusmenu));

	struct sni__slot *slot1 = malloc(SIZEOF(struct sni__slot));
	int ret = sd_bus_call_method_async(sni_server->bus, &slot1->slot, item->service.s,
			item->properties->menu.s, "org.freedesktop.DBus.Properties", "GetAll",
			sni__dbusmenu_handle_get_properties, slot1, "s", sni__dbusmenu_interface);
	if (ret < 0) {
		goto error_1;
	}

	struct sni__slot *slot2 = malloc(SIZEOF(struct sni__slot));
	ret = sd_bus_match_signal_async(sni_server->bus, &slot2->slot, item->service.s,
			item->properties->menu.s, sni__dbusmenu_interface, NULL,
			sni__dbusmenu_handle_signal, sni__dbusmenu_handle_signal, slot2);
	if (ret < 0) {
		goto error_2;
	}

	dbusmenu->item = item;

	slot1->item = item;
	list_struct_sni__slot_insert_head(&item->slots, slot1);
	slot2->item = item;
	list_struct_sni__slot_insert_head(&item->slots, slot2);

	return dbusmenu;
error_2:
	sd_bus_slot_unref(slot1->slot);
	free(slot2);
error_1:
	free(slot1);
	free(dbusmenu);
	return NULL;
}

static int sni__item_handle_get_properties(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)ret_error;
	struct sni__slot *slot = data;
	struct sni_item *item = slot->item;

	sni__slot_free(slot);
	struct sni_item_properties *old_properties = item->properties;

	int ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
	if (ret < 0) {
		item->properties = NULL;
		goto out;
	} else {
		item->properties = calloc(1, SIZEOF(struct sni_item_properties));
		ret = 1;
	}

	struct sni_item_properties *props = item->properties;
	while (sd_bus_message_enter_container(msg, 'e', "sv") == 1) {
		const char *key;
		sd_bus_message_read_basic(msg, 's', &key);
		size_t len = strlen(key);
		if (len == 0) {
			goto exit_con;
		}
		string_t key_str = {
			.s = (char *)key,
			.len = len,
			.free_contents = FALSE,
			.nul_terminated = TRUE,
		};
		sd_bus_message_enter_container(msg, 'v', NULL);
		if (string_equal(key_str, STRING_LITERAL("IconName"))) {
			const char *icon_name;
			sd_bus_message_read_basic(msg, 's', &icon_name);
			if ((len = strlen(icon_name)) > 0) {
				string_init_len(&props->icon_name, icon_name, len, TRUE);
			}
		} else if (string_equal(key_str, STRING_LITERAL("IconThemePath"))) {
			const char *icon_theme_path;
			sd_bus_message_read_basic(msg, 's', &icon_theme_path);
			if ((len = strlen(icon_theme_path)) > 0) {
				string_init_len(&props->icon_theme_path, icon_theme_path, len, TRUE);
			}
		} else if (string_equal(key_str, STRING_LITERAL("IconPixmap"))) {
			sni__item_read_pixmap(msg, &props->icon_pixmap);
		} else if (string_equal(key_str, STRING_LITERAL("Status"))) {
			const char *status;
			sd_bus_message_read_basic(msg, 's', &status);
			if ((len = strlen(status)) > 0) {
				string_t status_str = {
					.s = (char *)status,
					.len = len,
					.free_contents = FALSE,
					.nul_terminated = TRUE,
				};
				if (string_equal(status_str, STRING_LITERAL("Active"))) {
					props->status = SNI_ITEM_STATUS_ACTIVE;
				} else if (string_equal(status_str, STRING_LITERAL("Passive"))) {
					props->status = SNI_ITEM_STATUS_PASSIVE;
				} else if (string_equal(status_str, STRING_LITERAL("NeedsAttention"))) {
					props->status = SNI_ITEM_STATUS_NEEDS_ATTENTION;
				}
			}
		} else if (string_equal(key_str, STRING_LITERAL("Category"))) {
			const char *category;
			sd_bus_message_read_basic(msg, 's', &category);
			if ((len = strlen(category)) > 0) {
				string_t category_str = {
					.s = (char *)category,
					.len = len,
					.free_contents = FALSE,
					.nul_terminated = TRUE,
				};
				if (string_equal(category_str, STRING_LITERAL("ApplicationStatus"))) {
					props->category = SNI_ITEM_CATEGORY_APPLICATION_STATUS;
				} else if (string_equal(category_str, STRING_LITERAL("Communications"))) {
					props->category = SNI_ITEM_CATEGORY_COMMUNICATIONS;
				} else if (string_equal(category_str, STRING_LITERAL("SystemServices"))) {
					props->category = SNI_ITEM_CATEGORY_SYSTEM_SERVICES;
				} else if (string_equal(category_str, STRING_LITERAL("Hardware"))) {
					props->category = SNI_ITEM_CATEGORY_HARDWARE;
				}
			}
		} else if (string_equal(key_str, STRING_LITERAL("Menu"))) {
			const char *menu;
			sd_bus_message_read_basic(msg, 'o', &menu);
			if ((len = strlen(menu)) > 0) {
				string_init_len(&props->menu, menu, len, TRUE);
			}
		} else if (string_equal(key_str, STRING_LITERAL("AttentionIconName"))) {
			const char *attention_icon_name;
			sd_bus_message_read_basic(msg, 's', &attention_icon_name);
			if ((len = strlen(attention_icon_name)) > 0) {
				string_init_len(&props->attention_icon_name, attention_icon_name, len, TRUE);
			}
		} else if (string_equal(key_str, STRING_LITERAL("AttentionIconPixmap"))) {
			sni__item_read_pixmap(msg, &props->attention_icon_pixmap);
		} else if (string_equal(key_str, STRING_LITERAL("ItemIsMenu"))) {
			sd_bus_message_read_basic(msg, 'b', &props->item_is_menu);
		} else if (string_equal(key_str, STRING_LITERAL("WindowId"))) {
			sd_bus_message_read_basic(msg, 'i', &props->window_id);
		} else if (string_equal(key_str, STRING_LITERAL("Id"))) {
			const char *id;
			sd_bus_message_read_basic(msg, 's', &id);
			if ((len = strlen(id)) > 0) {
				string_init_len(&props->id, id, len, TRUE);
			}
		} else if (string_equal(key_str, STRING_LITERAL("Title"))) {
			const char *title;
			sd_bus_message_read_basic(msg, 's', &title);
			if ((len = strlen(title)) > 0) {
				string_init_len(&props->title, title, len, TRUE);
			}
		} else if (string_equal(key_str, STRING_LITERAL("AttentionMovieName"))) {
			const char *attention_movie_name;
			sd_bus_message_read_basic(msg, 's', &attention_movie_name);
			if ((len = strlen(attention_movie_name)) > 0) {
				string_init_len(&props->attention_movie_name, attention_movie_name, len, TRUE);
			}
		} else if (string_equal(key_str, STRING_LITERAL("OverlayIconName"))) {
			const char *overlay_icon_name;
			sd_bus_message_read_basic(msg, 's', &overlay_icon_name);
			if ((len = strlen(overlay_icon_name)) > 0) {
				string_init_len(&props->overlay_icon_name, overlay_icon_name, len, TRUE);
			}
		} else if (string_equal(key_str, STRING_LITERAL("OverlayIconPixmap"))) {
			sni__item_read_pixmap(msg, &props->overlay_icon_pixmap);
		} else if (string_equal(key_str, STRING_LITERAL("ToolTip"))) {
			sd_bus_message_enter_container(msg, 'r', "sa(iiay)ss");
			const char *icon_name;
			sd_bus_message_read_basic(msg, 's', &icon_name);
			if ((len = strlen(icon_name)) > 0) {
				string_init_len(&props->tooltip.icon_name, icon_name, len, TRUE);
			}
			sni__item_read_pixmap(msg, &props->tooltip.icon_pixmap);
			const char *title;
			sd_bus_message_read_basic(msg, 's', &title);
			if ((len = strlen(title)) > 0) {
				string_init_len(&props->tooltip.title, title, len, TRUE);
			}
			const char *text;
			sd_bus_message_read_basic(msg, 's', &text);
			if ((len = strlen(text)) > 0) {
				string_init_len(&props->tooltip.text, text, len, TRUE);
			}
			sd_bus_message_exit_container(msg);
		} else {
			sd_bus_message_skip(msg, NULL);
		}
		sd_bus_message_exit_container(msg);
exit_con:
		sd_bus_message_exit_container(msg);
	}
	//sd_bus_message_exit_container(msg);

	if (item->dbusmenu == NULL) {
		item->dbusmenu = sni__dbusmenu_create(item);
	}

out:
	SNI_ITEM_PROPERTIES_UPDATED_FUNC(item);

	sni__item_properties_destroy(old_properties);

	return ret;
}

static int sni__item_handle_signal(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)msg;
	(void)ret_error;
	// ? TODO: error check
	struct sni_item *item = data;

	struct sni__slot *slot = malloc(SIZEOF(struct sni__slot));
	int ret = sd_bus_call_method_async(sni_server->bus, &slot->slot, item->service.s,
			item->path.s, "org.freedesktop.DBus.Properties", "GetAll",
			sni__item_handle_get_properties, slot, "s", sni__item_interface);
	if (ret >= 0) {
		slot->item = item;
		list_struct_sni__slot_insert_head(&item->slots, slot);
	} else {
		free(slot);
	}

	return 1;
}

static bool32_t sni_item_init(struct sni_item *item, string_t id) {
	string_t path;
	if (!string_find_char(id, '/', &path)) {
		return FALSE;
	}

	string_t service = {
		.s = strndup(id.s, (id.len - path.len)),
		.len = (id.len - path.len),
		.free_contents = TRUE,
		.nul_terminated = TRUE,
	};

	sd_bus_slot *slot_;
	int ret = sd_bus_match_signal_async(sni_server->bus, &slot_, service.s,
			path.s, sni__item_interface, NULL,
			sni__item_handle_signal, sni__item_handle_signal, item);
	if (ret < 0) {
		string_fini(&service);
		return FALSE;
	}

	*item = (struct sni_item){ 0 };

	struct sni__slot *slot = malloc(SIZEOF(struct sni__slot));
	slot->item = item;
	slot->slot = slot_;
	list_struct_sni__slot_insert_head(&item->slots, slot);

	if (id.free_contents) {
		item->watcher_id = id;
		item->path = path;
	} else {
		string_init_string(&item->watcher_id, id);
		string_init_string(&item->path, path);
	}
	item->service = service;

    return TRUE;
}

static void sni_item_fini(struct sni_item *item) {
	for (struct sni__slot *slot = item->slots.head; slot; ) {
		struct sni__slot *next = slot->next;
        sd_bus_slot_unref(slot->slot);
        free(slot);
        slot = next;
	}

	sni__dbusmenu_destroy(item->dbusmenu);

	sni__item_properties_destroy(item->properties);

    string_fini(&item->watcher_id);
	string_fini(&item->service);
	string_fini(&item->path);
}

static int sni__watcher_handle_register_item(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)data;
	(void)ret_error;
	const char *service_or_path;
	int ret = sd_bus_message_read_basic(msg, 's', &service_or_path);
	if (ret < 0) {
		return ret;
	}

	//if (strlen(service_or_path) == 0) {
	//	return -EINVAL;
	//}

	const char *service, *path;
	if (service_or_path[0] == '/') {
		service = sd_bus_message_get_sender(msg);
		path = service_or_path;
	} else {
		service = service_or_path;
		path = "/StatusNotifierItem";
	}

	string_t id;
	string_init_format(&id, "%s%s", service, path);
    for (size_t i = 0; i < sni_server->watcher.items.len; ++i) {
        if (string_equal(id, array_string_t_get(&sni_server->watcher.items, i))) {
            string_fini(&id);
            return -EEXIST;
        }
    }

    ret = sd_bus_emit_signal(sni_server->bus, sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierItemRegistered", "s", id.s);
    if (ret < 0) {
        string_fini(&id);
        return ret;
    }

    array_string_t_add(&sni_server->watcher.items, id);

	//ret = sd_bus_emit_properties_changed(sni_server->bus,
	//	sni__watcher_obj_path, sni__watcher_interface,
	//	"RegisteredStatusNotifierItems", NULL);
	//if (ret < 0) {
	//	string_fini(&id);
	//	return ret;
	//}

    ret = sd_bus_reply_method_return(msg, "");
    if (ret < 0) {
        return ret;
    }

	return 1;
}

static int sni__watcher_handle_register_host(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)data;
	(void)ret_error;
	const char *service;
	int ret = sd_bus_message_read_basic(msg, 's', &service);
	if (ret < 0) {
		return ret;
	}

	string_t service_str = {
		.s = (char *)service,
		.len = strlen(service),
		.nul_terminated = TRUE,
		.free_contents = FALSE,
	};

    for (size_t i = 0; i < sni_server->watcher.hosts.len; ++i) {
        if (string_equal(service_str, array_string_t_get(&sni_server->watcher.hosts, i))) {
            return -EEXIST;
        }
    }

	//if (sni_server->watcher.hosts.len == 0) {
	//	ret = sd_bus_emit_properties_changed(sni_server->bus,
	//		sni__watcher_obj_path, sni__watcher_interface,
	//		"IsStatusNotifierHostRegistered", NULL);
	//	if (ret < 0) {
	//		return ret;
	//	}
	//}

    ret = sd_bus_emit_signal(sni_server->bus, sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierHostRegistered", "");
    if (ret < 0) {
        return ret;
    }

	string_init_string(&service_str, service_str);
    array_string_t_add(&sni_server->watcher.hosts, service_str);

    ret = sd_bus_reply_method_return(msg, "");
    if (ret < 0) {
        return ret;
    }

	return 1;
}

static int sni__watcher_handle_get_registered_items(sd_bus *b, const char *path,
		const char *iface, const char *prop, sd_bus_message *reply,
		void *data, sd_bus_error *ret_error) {
	(void)b;
	(void)path;
	(void)iface;
	(void)prop;
	(void)data;
	(void)ret_error;
	array_char_ptr_t array;
	array_char_ptr_init(&array, sni_server->watcher.items.len + 1);
	for (size_t i = 0; i < sni_server->watcher.items.len; ++i) {
		array_char_ptr_add(&array, array_string_t_get(&sni_server->watcher.items, i).s);
	}
	array_char_ptr_add(&array, NULL);
    int ret = sd_bus_message_append_strv(reply, array.items);
	array_char_ptr_fini(&array);
    if (ret < 0) {
		return ret;
    }

    return 1;
}

static int sni__watcher_handle_is_host_registered(sd_bus *b, const char *path,
		const char *iface, const char *prop, sd_bus_message *reply,
		void *data, sd_bus_error *ret_error) {
	(void)b;
	(void)path;
	(void)iface;
	(void)prop;
	(void)data;
	(void)ret_error;
	int registered = sni_server->watcher.hosts.len > 0;
	int ret = sd_bus_message_append_basic(reply, 'b', &registered);
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static const sd_bus_vtable sni__watcher_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("RegisterStatusNotifierItem", "s", "", sni__watcher_handle_register_item,
			SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("RegisterStatusNotifierHost", "s", "", sni__watcher_handle_register_host,
			SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_PROPERTY("RegisteredStatusNotifierItems", "as", sni__watcher_handle_get_registered_items,
			0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_PROPERTY("IsStatusNotifierHostRegistered", "b", sni__watcher_handle_is_host_registered,
			0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_PROPERTY("ProtocolVersion", "i", NULL, 0,
			SD_BUS_VTABLE_PROPERTY_CONST),
	SD_BUS_SIGNAL("StatusNotifierItemRegistered", "s", 0),
	SD_BUS_SIGNAL("StatusNotifierItemUnregistered", "s", 0),
	SD_BUS_SIGNAL("StatusNotifierHostRegistered", NULL, 0),
	SD_BUS_SIGNAL("StatusNotifierHostUnregistered", NULL, 0),
	SD_BUS_VTABLE_END,
};

static int sni__watcher_handle_lost_service(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)data;
	(void)ret_error;
	const char *service, *old_owner, *new_owner;
	int ret = sd_bus_message_read(msg, "sss", &service, &old_owner, &new_owner);
	if (ret < 0) {
		return ret;
	}
    if (*new_owner) {
        return 0;
    }

    string_t service_str = {
		.s = (char *)service,
		.len = strlen(service),
		.free_contents = FALSE,
		.nul_terminated = TRUE,
	};

    for (size_t i = 0; i < sni_server->watcher.items.len; ++i) {
        string_t item = array_string_t_get(&sni_server->watcher.items, i);
        if (string_compare(item, service_str, service_str.len) == 0) {
            ret = sd_bus_emit_signal(sni_server->bus, sni__watcher_obj_path, sni__watcher_interface,
					"StatusNotifierItemUnregistered", "s", item.s);
            if (ret < 0) {
                return ret;
            }
            string_fini(&item);
            array_string_t_pop_swapback(&sni_server->watcher.items, i);
			//ret = sd_bus_emit_properties_changed(sni_server->bus,
			//	sni__watcher_obj_path, sni__watcher_interface,
			//	"RegisteredStatusNotifierItems", NULL);
			//if (ret < 0) {
			//	return ret;
			//}
            return 0;
        }
    }

    for (size_t i = 0; i < sni_server->watcher.hosts.len; ++i) {
        string_t host = array_string_t_get(&sni_server->watcher.hosts, i);
        if (string_equal(service_str, host)) {
			//if (sni_server->watcher.hosts.len == 1) {
			//	ret = sd_bus_emit_properties_changed(sni_server->bus,
			//		sni__watcher_obj_path, sni__watcher_interface,
			//		"IsStatusNotifierHostRegistered", NULL);
			//	if (ret < 0) {
			//		return ret;
			//	}
			//}
            ret = sd_bus_emit_signal(sni_server->bus, sni__watcher_obj_path, sni__watcher_interface,
					"StatusNotifierHostUnregistered", "");
			if (ret < 0) {
                return ret;
            }
            string_fini(&host);
            array_string_t_pop_swapback(&sni_server->watcher.hosts, i);
            return 0;
        }
    }

	return 0;
}

static int sni__host_add_item(string_t id) {
	for (size_t i = 0; i < sni_server->host.items.len; ++i) {
		struct sni_item *item = array_struct_sni_item_ptr_get(
			&sni_server->host.items, i);
		if (string_equal(id, item->watcher_id)) {
			return -EEXIST;
		}
	}

	struct sni_item *item = SNI_ITEM_CREATE_FUNC(id);
	if (item) {
		ASSERT(string_equal(item->watcher_id, id));
		array_struct_sni_item_ptr_add(&sni_server->host.items, item);
		return 1;
	} else {
		return -EINVAL;
	}
}

static int sni__host_handle_get_registered_items(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)data;
	(void)ret_error;
	int ret = sd_bus_message_enter_container(msg, 'v', "as");
	if (ret < 0) {
		return ret;
	}

	char **ids;
	ret = sd_bus_message_read_strv(msg, &ids);
	//sd_bus_message_exit_container(msg);
	if (ret < 0) {
		return ret;
	}

	if (ids) {
		for (char **s = ids; *s != NULL; ++s) {
			char *id = *s;
			size_t len = strlen(id);
			if ((len == 0) || (0 > sni__host_add_item((string_t){
					.s = id,
					.len = len,
					.free_contents = TRUE,
					.nul_terminated = TRUE,
			}))) {
				free(id);
			}
		}
		free(ids);
	}

	return 1;
}

static int sni__host_register_to_watcher(void) {
	// ? TODO: slots
	int ret = sd_bus_call_method_async(sni_server->bus, NULL,
			sni__watcher_interface, sni__watcher_obj_path, sni__watcher_interface,
			"RegisterStatusNotifierHost", NULL, NULL,
			"s", sni_server->host.interface.s);
	if (ret < 0) {
		return ret;
	}

	ret = sd_bus_call_method_async(sni_server->bus, NULL,
			sni__watcher_interface, sni__watcher_obj_path,
			"org.freedesktop.DBus.Properties", "Get",
			sni__host_handle_get_registered_items, NULL, "ss",
			sni__watcher_interface, "RegisteredStatusNotifierItems");
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni__host_handle_item_registered(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)data;
	(void)ret_error;
	const char *id;
	int ret = sd_bus_message_read_basic(msg, 's', &id);
	if (ret < 0) {
		return ret;
	}

	size_t len = strlen(id);
	if (len > 0) {
		return sni__host_add_item((string_t){
			.s = (char *)id,
			.len = len,
			.free_contents = FALSE,
			.nul_terminated = TRUE,
		});
	} else {
		return -EINVAL;
	}
}

static int sni__host_handle_item_unregistered(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)data;
	(void)ret_error;
	const char *id;
	int ret = sd_bus_message_read_basic(msg, 's', &id);
	if (ret < 0) {
		return ret;
	}

	size_t len = strlen(id);
	if (len > 0) {
		for (size_t i = 0; i < sni_server->host.items.len; ++i) {
			struct sni_item *item = array_struct_sni_item_ptr_get(&sni_server->host.items, i);
			if (string_equal(item->watcher_id, (string_t){
					.s = (char *)id,
					.len = len,
					.free_contents = FALSE,
					.nul_terminated = TRUE,
			})) {
				array_struct_sni_item_ptr_pop(&sni_server->host.items, i);
				SNI_ITEM_DESTROY_FUNC(item);
				break;
			}
		}
	}

	return 1;
}

static int sni__host_handle_new_watcher(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)data;
	(void)ret_error;
	const char *service, *old_owner, *new_owner;
	int ret = sd_bus_message_read(msg, "sss", &service, &old_owner, &new_owner);
	if (ret < 0) {
		return ret;
	}

	if (strcmp(service, sni__watcher_interface) == 0) {
        ret = sni__host_register_to_watcher();
		if (ret < 0) {
			return ret;
		}
		for (size_t i = sni_server->host.items.len - 1; i != SIZE_MAX; --i) {
			struct sni_item *item = array_struct_sni_item_ptr_get(&sni_server->host.items, i);
			sni_server->host.items.len--;
			SNI_ITEM_DESTROY_FUNC(item);
		}
	}

	return 0;
}

static void sni_server_fini(void) {
	ASSERT(sni_server != NULL);

	for (size_t i = sni_server->host.items.len - 1; i != SIZE_MAX; --i) {
		struct sni_item *item = array_struct_sni_item_ptr_get(&sni_server->host.items, i);
		sni_server->host.items.len--;
		SNI_ITEM_DESTROY_FUNC(item);
	}
	array_struct_sni_item_ptr_fini(&sni_server->host.items);

	sd_bus_flush_close_unref(sni_server->bus);

	string_fini(&sni_server->host.interface);
	for (size_t i = 0; i < sni_server->watcher.items.len; ++i) {
		string_fini(array_string_t_get_ptr(&sni_server->watcher.items, i));
	}
	for (size_t i = 0; i < sni_server->watcher.hosts.len; ++i) {
		string_fini(array_string_t_get_ptr(&sni_server->watcher.hosts, i));
	}
	array_string_t_fini(&sni_server->watcher.items);
	array_string_t_fini(&sni_server->watcher.hosts);

    free(sni_server);
	sni_server = NULL;
}

static int sni_server_init(void) {
	ASSERT(sni_server == NULL);

	sd_bus *bus;
    int ret = sd_bus_open_user(&bus);
    if (ret < 0) {
		return ret;
    }

	sni_server = malloc(SIZEOF(struct sni_server));
	sni_server->bus = bus;

	ret = sd_bus_request_name(sni_server->bus, sni__watcher_interface, SD_BUS_NAME_QUEUE);
	if (ret < 0) {
        goto error;
    }

    ret = sd_bus_add_object_vtable(sni_server->bus, NULL, sni__watcher_obj_path,
            sni__watcher_interface, sni__watcher_vtable,
			&sni__watcher_protocol_version);
    if (ret < 0) {
        goto error;
    }

    ret = sd_bus_match_signal(sni_server->bus, NULL, "org.freedesktop.DBus",
			"/org/freedesktop/DBus", "org.freedesktop.DBus",
			"NameOwnerChanged", sni__watcher_handle_lost_service, NULL);
    if (ret < 0) {
        goto error;
    }

    array_string_t_init(&sni_server->watcher.items, 10);
    array_string_t_init(&sni_server->watcher.hosts, 4);


	string_init_format(&sni_server->host.interface, "org.kde.StatusNotifierHost-%d", getpid());
    ret = sd_bus_request_name(sni_server->bus, sni_server->host.interface.s, 0); // ? SD_BUS_NAME_QUEUE
    if (ret < 0) {
        goto error;
    }

    ret = sni__host_register_to_watcher();
    if (ret < 0) {
        goto error;
    }

    ret = sd_bus_match_signal(sni_server->bus, NULL, sni__watcher_interface,
			sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierItemRegistered", sni__host_handle_item_registered, NULL);
	if (ret < 0) {
		goto error;
	}
	ret = sd_bus_match_signal(sni_server->bus, NULL, sni__watcher_interface,
			sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierItemUnregistered", sni__host_handle_item_unregistered, NULL);
	if (ret < 0) {
		goto error;
	}

	ret = sd_bus_match_signal(sni_server->bus, NULL, "org.freedesktop.DBus",
			"/org/freedesktop/DBus", "org.freedesktop.DBus", "NameOwnerChanged",
			sni__host_handle_new_watcher, NULL);
	if (ret < 0) {
		goto error;
	}

    array_struct_sni_item_ptr_init(&sni_server->host.items, 10);

	return 1;
error:
	sni_server_fini();
	return ret;
}

static int sni_server_get_poll_info(int *fd_, int *events_, uint64_t *timeout_) {
	ASSERT(sni_server != NULL);

	int fd = sd_bus_get_fd(sni_server->bus);
	if (fd < 0) {
		return fd;
	}

	int events = sd_bus_get_events(sni_server->bus);
	if (events < 0) {
		return events;
	}

	uint64_t timeout;
	int ret = sd_bus_get_timeout(sni_server->bus, &timeout);
	if (ret < 0) {
		return ret;
	}

	*fd_ = fd;
	*events_ = events;
	*timeout_ = timeout;
	return 1;
}

static int sni_server_process(void) {
	ASSERT(sni_server != NULL);

	int ret;
    while ((ret = sd_bus_process(sni_server->bus, NULL)) > 0)
	{
	}
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni__item_handle_method(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)msg;
	(void)ret_error;
	struct sni__slot *slot = data;
	sni__slot_free(slot);
	return 1;
}

static int sni_item_context_menu(struct sni_item *item, int x, int y) {
	ASSERT(sni_server != NULL);

	int ret = sd_bus_call_method(sni_server->bus, item->service.s,
		item->path.s, sni__item_interface, "ContextMenu",
		NULL, NULL, "ii", x, y);
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_context_menu_async(struct sni_item *item, int x, int y) {
	ASSERT(sni_server != NULL);

	struct sni__slot *slot = malloc(SIZEOF(struct sni__slot));
	int ret = sd_bus_call_method_async(sni_server->bus, &slot->slot, item->service.s,
			item->path.s, sni__item_interface, "ContextMenu",
			sni__item_handle_method, slot, "ii", x, y);
	if (ret < 0) {
		free(slot);
		return ret;
	} else {
		slot->item = item;
	}

	list_struct_sni__slot_insert_head(&item->slots, slot);

	return 1;
}

static int sni_item_activate(struct sni_item *item, int x, int y) {
	ASSERT(sni_server != NULL);

	int ret = sd_bus_call_method(sni_server->bus, item->service.s,
		item->path.s, sni__item_interface, "Activate",
		NULL, NULL, "ii", x, y);
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_activate_async(struct sni_item *item, int x, int y) {
	ASSERT(sni_server != NULL);

	struct sni__slot *slot = malloc(SIZEOF(struct sni__slot));
	int ret = sd_bus_call_method_async(sni_server->bus, &slot->slot, item->service.s,
			item->path.s, sni__item_interface, "Activate",
			sni__item_handle_method, slot, "ii", x, y);
	if (ret < 0) {
		free(slot);
		return ret;
	} else {
		slot->item = item;
	}

	list_struct_sni__slot_insert_head(&item->slots, slot);

	return 1;
}

static int sni_item_secondary_activate(struct sni_item *item, int x, int y) {
	ASSERT(sni_server != NULL);

	int ret = sd_bus_call_method(sni_server->bus, item->service.s,
		item->path.s, sni__item_interface, "SecondaryActivate",
		NULL, NULL, "ii", x, y);
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_secondary_activate_async(struct sni_item *item, int x, int y) {
	ASSERT(sni_server != NULL);

	struct sni__slot *slot = malloc(SIZEOF(struct sni__slot));
	int ret = sd_bus_call_method_async(sni_server->bus, &slot->slot, item->service.s,
			item->path.s, sni__item_interface, "SecondaryActivate",
			sni__item_handle_method, slot, "ii", x, y);
	if (ret < 0) {
		free(slot);
		return ret;
	} else {
		slot->item = item;
	}

	list_struct_sni__slot_insert_head(&item->slots, slot);

	return 1;
}

static int sni_item_scroll(struct sni_item *item, int delta, enum sni_item_scroll_orientation orientation) {
	ASSERT(sni_server != NULL);

	int ret = sd_bus_call_method(sni_server->bus, item->service.s,
		item->path.s, sni__item_interface, "Scroll", NULL, NULL,
		"is", delta, (orientation == SNI_ITEM_SCROLL_ORIENTATION_VERTICAL) ? "vertical" : "horizontal");
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_scroll_async(struct sni_item *item, int delta, enum sni_item_scroll_orientation orientation) {
	ASSERT(sni_server != NULL);

	struct sni__slot *slot = malloc(SIZEOF(struct sni__slot));
	int ret = sd_bus_call_method_async(sni_server->bus, &slot->slot, item->service.s,
			item->path.s, sni__item_interface, "Scroll", sni__item_handle_method, slot,
			"is", delta, (orientation == SNI_ITEM_SCROLL_ORIENTATION_VERTICAL) ? "vertical" : "horizontal");
	if (ret < 0) {
		free(slot);
		return ret;
	} else {
		slot->item = item;
	}

	list_struct_sni__slot_insert_head(&item->slots, slot);

	return 1;
}

static int sni_dbusmenu_menu_item_event(struct sni_dbusmenu_menu_item *menu_item,
		enum sni_dbusmenu_menu_item_event_type type, bool32_t async) {
	ASSERT(sni_server != NULL);
	ASSERT(((type == SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_OPENED)
				|| (type == SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLOSED))
			? (menu_item->submenu != NULL) : 1);

	struct sni_dbusmenu *dbusmenu = menu_item->parent_menu->dbusmenu;
	struct sni_item *item = dbusmenu->item;
	if ((dbusmenu->item->properties == NULL) || (dbusmenu->item->properties->menu.len == 0)) {
		return -ENOENT;
	}

	static const char *event_types[] = {
		"clicked",
		"hovered",
		"opened",
		"closed",
	};

	if (async) {
		struct sni__slot *slot = malloc(SIZEOF(struct sni__slot));
		int ret = sd_bus_call_method_async(sni_server->bus, &slot->slot, item->service.s,
				item->properties->menu.s, sni__dbusmenu_interface, "Event",
				sni__item_handle_method, slot, "isvu",
				menu_item->id, event_types[type], "y", 0, time(NULL));
		if (ret < 0) {
			free(slot);
			return ret;
		} else {
			slot->item = item;
			list_struct_sni__slot_insert_head(&item->slots, slot);
		}
	} else {
		int ret = sd_bus_call_method(sni_server->bus, item->service.s,
				item->properties->menu.s, sni__dbusmenu_interface, "Event",
				NULL, NULL, "isvu", menu_item->id, event_types[type], "y", 0, time(NULL));
		if (ret < 0) {
			return ret;
		}
	}

	return 1;
}

static int sni__dbusmenu_menu_handle_about_to_show(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	(void)ret_error;
	struct sni__slot *slot = data;
	struct sni_dbusmenu *dbusmenu = slot->item->dbusmenu;

	sni__slot_free(slot);

	int need_update;
	int ret = sd_bus_message_read_basic(msg, 'b', &need_update);
	if (ret >= 0) {
		if (need_update) {
			sni__dbusmenu_get_layout(dbusmenu);
		}
		ret = 1;
	}

	return ret;
}

static int sni_dbusmenu_menu_about_to_show(struct sni_dbusmenu_menu *menu, bool32_t async) {
	ASSERT(sni_server != NULL);
	ASSERT(menu->parent_menu_item != NULL);

	struct sni_item *item = menu->dbusmenu->item;
	if ((item->properties == NULL) || (item->properties->menu.len == 0)) {
		return -ENOENT;
	}

	if (async) {
		struct sni__slot *slot = malloc(SIZEOF(struct sni__slot));
		int ret = sd_bus_call_method_async(sni_server->bus, &slot->slot, item->service.s,
				item->properties->menu.s, sni__dbusmenu_interface, "AboutToShow",
				sni__dbusmenu_menu_handle_about_to_show, slot, "i", menu->parent_menu_item->id);
		if (ret < 0) {
			free(slot);
			return ret;
		} else {
			slot->item = item;
			list_struct_sni__slot_insert_head(&item->slots, slot);
		}
	} else {
		int ret = sd_bus_call_method(sni_server->bus, item->service.s,
				item->properties->menu.s, sni__dbusmenu_interface, "AboutToShow",
				NULL, NULL, "i", menu->parent_menu_item->id);
		if (ret < 0) {
			return ret;
		}
	}

	return 1;
}

#endif // SNI_SERVER_H
