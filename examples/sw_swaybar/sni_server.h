#if !defined(SNI_SERVER_H)
#define SNI_SERVER_H

/* TODO: #define _DEFAULT_SOURCE */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <poll.h>

#include <stddef.h>
#include <stdint.h>

#if !defined(SU_IMPLEMENTATION)
#define SU_IMPLEMENTATION
#endif /* !defined(SU_IMPLEMENTATION) */
#if !defined(SU_STRIP_PREFIXES)
#define SU_STRIP_PREFIXES
#endif /* !defined(SU_STRIP_PREFIXES) */
#include "su.h"

#if HAS_INCLUDE(<basu/sd-bus.h>)
#include <basu/sd-bus.h>
#elif HAS_INCLUDE(<elogind/sd-bus.h>)
#include <elogind/sd-bus.h>
#elif HAS_INCLUDE(<systemd/sd-bus.h>)
#include <systemd/sd-bus.h>
#else
#include <sd-bus.h>
#endif

typedef enum sni_dbusmenu_menu_item_type {
    SNI_DBUSMENU_MENU_ITEM_TYPE_STANDARD,
    SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR
} sni_dbusmenu_menu_item_type_t;

typedef enum sni_dbusmenu_menu_item_toggle_type {
    SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_NONE,
    SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_CHECKMARK,
    SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_RADIO
} sni_dbusmenu_menu_item_toggle_type_t;

typedef enum sni_dbusmenu_menu_item_disposition {
    SNI_DBUSMENU_MENU_ITEM_DISPOSITION_NORNAL,
    SNI_DBUSMENU_MENU_ITEM_DISPOSITION_INFORMATIVE,
    SNI_DBUSMENU_MENU_ITEM_DISPOSITION_WARNING,
    SNI_DBUSMENU_MENU_ITEM_DISPOSITION_ALERT
} sni_dbusmenu_menu_item_disposition_t;

typedef enum sni_dbusmenu_menu_item_event_type {
    SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLICKED,
    SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_HOVERED,
    SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_OPENED,
    SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLOSED
} sni_dbusmenu_menu_item_event_type_t;

typedef struct sni_dbusmenu_menu sni_dbusmenu_menu_t;

typedef struct sni_dbusmenu_menu_item {
    sni_dbusmenu_menu_t *parent_menu;
    sni_dbusmenu_menu_t *submenu; /* may be NULL */
    int id;
    sni_dbusmenu_menu_item_type_t type;
    string_t label;
    int enabled; /* bool */
    int visible; /* bool */
    string_t icon_name;
    struct { /* png data of the icon */
        void *bytes; /* may be NULL */
        size_t nbytes;
    } icon_data;
    sni_dbusmenu_menu_item_toggle_type_t toggle_type;
    int toggle_state; /* 0 - off, 1 - on, else - indeterminate */
    sni_dbusmenu_menu_item_disposition_t disposition;
    int activation_requested; /* bool  NOT IMPLEMENTED */
} sni_dbusmenu_menu_item_t;

ARRAY_DECLARE_DEFINE(sni_dbusmenu_menu_item_t)

typedef struct sni_dbusmenu sni_dbusmenu_t;

struct sni_dbusmenu_menu {
    sni_dbusmenu_t *dbusmenu;
    sni_dbusmenu_menu_item_t *parent_menu_item; /* NULL when root */
	su_array__sni_dbusmenu_menu_item_t__t menu_items;
	size_t depth;
};

typedef enum sni_dbusmenu_status {
    SNI_DBUSMENU_STATUS_INVALID,
    SNI_DBUSMENU_STATUS_NORMAL,
    SNI_DBUSMENU_STATUS_NOTICE
} sni_dbusmenu_status_t;

typedef enum sni_dbusmenu_text_direction {
    SNI_DBUSMENU_TEXT_DIRECTION_INVALID,
    SNI_DBUSMENU_TEXT_DIRECTION_LEFT_TO_RIGHT,
    SNI_DBUSMENU_TEXT_DIRECTION_RIGHT_TO_LEFT
} sni_dbusmenu_text_direction_t;

typedef struct sni_dbusmenu_properties {
    su_array__su_string_t__t icon_theme_path;
    sni_dbusmenu_status_t status;
    sni_dbusmenu_text_direction_t text_direction;
} sni_dbusmenu_properties_t;

typedef struct sni_item sni_item_t;

struct sni_dbusmenu {
	sni_item_t *item;
    sni_dbusmenu_properties_t *properties; /* may be NULL */
    sni_dbusmenu_menu_t *menu; /* may be NULL */
};

typedef struct sni_pixmap {
    int width;
    int height;
    uint32_t pixels[1]; /* flexible array, ARGB32, native byte order */
	/* width * height * 4 */
} sni_pixmap_t, *sni_pixmap_t_ptr;

ARRAY_DECLARE_DEFINE(sni_pixmap_t_ptr)

typedef enum sni_item_status {
    SNI_ITEM_STATUS_INVALID,
    SNI_ITEM_STATUS_PASSIVE,
    SNI_ITEM_STATUS_ACTIVE,
    SNI_ITEM_STATUS_NEEDS_ATTENTION
} sni_item_status_t;

typedef enum sni_item_category {
    SNI_ITEM_CATEGORY_INVALID,
    SNI_ITEM_CATEGORY_APPLICATION_STATUS,
    SNI_ITEM_CATEGORY_COMMUNICATIONS,
    SNI_ITEM_CATEGORY_SYSTEM_SERVICES,
    SNI_ITEM_CATEGORY_HARDWARE
} sni_item_category_t;

typedef struct sni_item_properties_tooltip {
	string_t icon_name;
	su_array__sni_pixmap_t_ptr__t icon_pixmap; /* qsorted size descending */
	string_t title;
	string_t text;
} sni_item_properties_tooltip_t;

typedef struct sni_item_properties {
    /* every field may be NULL */
    string_t icon_name;
    string_t icon_theme_path;
    su_array__sni_pixmap_t_ptr__t icon_pixmap; /* qsorted size descending */
    sni_item_status_t status;
    sni_item_category_t category;
    string_t menu;
    string_t attention_icon_name;
    su_array__sni_pixmap_t_ptr__t attention_icon_pixmap; /* qsorted size descending */
    int item_is_menu; /* bool */
    int window_id;
    string_t id;
    string_t title;
    string_t attention_movie_name;
    string_t overlay_icon_name;
    su_array__sni_pixmap_t_ptr__t overlay_icon_pixmap; /* qsorted size descending */
    sni_item_properties_tooltip_t tooltip;
} sni_item_properties_t;

typedef enum sni_item_scroll_orientation {
    SNI_ITEM_SCROLL_ORIENTATION_VERTICAL,
    SNI_ITEM_SCROLL_ORIENTATION_HORIZONTAL
} sni_item_scroll_orientation_t;

typedef struct sni__slot sni__slot_t;
struct sni__slot {
	sni_item_t *item;
	sd_bus_slot *slot;
	LLIST_FIELDS(sni__slot_t);
};

LLIST_DECLARE_DEFINE(sni__slot_t)

typedef void (*sni_item_func_t)(sni_item_t *);
typedef sni_item_t *(*sni_item_create_func_t)(void);

typedef struct sni_item_in {
	sni_item_func_t destroy; /* may be NULL */
	sni_item_func_t properties_updated; /* may be NULL */
	sni_item_func_t dbusmenu_menu_updated; /* may be NULL */
} sni_item_in_t;

typedef struct sni_item_out {
	sni_item_properties_t *properties; /* may be NULL */
	sni_dbusmenu_t *dbusmenu; /* may be NULL */
} sni_item_out_t;

typedef struct sni__item {
	string_t watcher_id;
	string_t service;
	string_t path;
	su_llist__sni__slot_t__t slots;
} sni__item_t;

struct sni_item {
	sni_item_in_t in;
	sni_item_out_t out;
	sni__item_t private;
};

typedef sni_item_t* sni_item_t_ptr;
ARRAY_DECLARE_DEFINE(sni_item_t_ptr)

typedef struct sni_server_in {
	allocator_t *alloc;
	sni_item_create_func_t item_create; /* may be NULL */
} sni_server_in_t;

typedef struct sni_server_out {
	su_array__sni_item_t_ptr__t host_items;
} sni_server_out_t;

typedef struct sni__server {
	sd_bus *bus;
	string_t host_interface;
	su_array__su_string_t__t watcher_items;
	su_array__su_string_t__t watcher_hosts;
} sni__server_t;

typedef struct sni_server {
	sni_server_in_t in;
	sni_server_out_t out;
	sni__server_t private;
} sni_server_t;

static sni_server_t sni_server;

static int sni_server_init(void);
static void sni_server_fini(void);
static int sni_server_get_poll_info(struct pollfd *pollfd_out, int64_t *absolute_timeout_ms);
static int sni_server_process(void);

static int sni_item_context_menu(sni_item_t *, int x, int y);
static int sni_item_context_menu_async(sni_item_t *, int x, int y);
static int sni_item_activate(sni_item_t *, int x, int y);
static int sni_item_activate_async(sni_item_t *, int x, int y);
static int sni_item_secondary_activate(sni_item_t *, int x, int y);
static int sni_item_secondary_activate_async(sni_item_t *, int x, int y);
static int sni_item_scroll(sni_item_t *, int delta, sni_item_scroll_orientation_t);
static int sni_item_scroll_async(sni_item_t *, int delta, sni_item_scroll_orientation_t);

static int sni_dbusmenu_menu_about_to_show(sni_dbusmenu_menu_t *, bool32_t async);

static int sni_dbusmenu_menu_item_event(sni_dbusmenu_menu_item_t *,
	sni_dbusmenu_menu_item_event_type_t, bool32_t async);

static char sni__watcher_interface[] = "org.kde.StatusNotifierWatcher";
static char sni__watcher_obj_path[] = "/StatusNotifierWatcher";
static int sni__watcher_protocol_version = 0;

static char sni__item_interface[] = "org.kde.StatusNotifierItem";

static char sni__dbusmenu_interface[] = "com.canonical.dbusmenu";

static int sni__item_pixmap_size_descending_qsort(const void *a, const void *b) {
	sni_pixmap_t *p1 = *(sni_pixmap_t **)(uintptr_t)a;
	sni_pixmap_t *p2 = *(sni_pixmap_t **)(uintptr_t)b;
	return (p2->width * p2->height) - (p1->width * p1->height);
}

static void sni__item_read_pixmap(sd_bus_message *msg, su_array__sni_pixmap_t_ptr__t *dest) {
	su_array__sni_pixmap_t_ptr__init(dest, sni_server.in.alloc, 16);

	sd_bus_message_enter_container(msg, 'a', "(iiay)");
	while (sd_bus_message_enter_container(msg, 'r', "iiay") == 1) {
		int width, height;
		const void *bytes;
		size_t nbytes;
		sd_bus_message_read_basic(msg, 'i', &width);
		sd_bus_message_read_basic(msg, 'i', &height);
		sd_bus_message_read_array(msg, 'y', &bytes, &nbytes);
		if (((size_t)width * (size_t)height * 4) == nbytes) {
			int i;
			sni_pixmap_t *pixmap = sni_server.in.alloc->alloc(sni_server.in.alloc,
				sizeof(*pixmap) - sizeof(pixmap->pixels) + nbytes, 64);
			pixmap->width = width;
			pixmap->height = height;
			for ( i = 0; i < (width * height); ++i) {
				pixmap->pixels[i] = ntohl(((uint32_t *)(uintptr_t)bytes)[i]);
			}
			su_array__sni_pixmap_t_ptr__add(dest, sni_server.in.alloc, pixmap);
		}
		sd_bus_message_exit_container(msg);
	}
	sd_bus_message_exit_container(msg);

	su_array__sni_pixmap_t_ptr__qsort(dest, sni__item_pixmap_size_descending_qsort);
}

static void sni__item_properties_destroy(sni_item_properties_t *properties) {
	size_t i;
	su_allocator_t *alloc;

	if (properties == NULL) {
		return;
	}

	alloc = sni_server.in.alloc;

	string_fini(&properties->icon_name, alloc);
	string_fini(&properties->icon_theme_path, alloc);
	for ( i = 0; i < properties->icon_pixmap.len; ++i) {
		alloc->free(alloc,
			su_array__sni_pixmap_t_ptr__get(&properties->icon_pixmap, i));
	}
	su_array__sni_pixmap_t_ptr__fini(&properties->icon_pixmap, alloc);
	string_fini(&properties->menu, alloc);
	string_fini(&properties->attention_icon_name, alloc);
	for ( i = 0; i < properties->attention_icon_pixmap.len; ++i) {
		alloc->free(alloc,
			su_array__sni_pixmap_t_ptr__get(&properties->attention_icon_pixmap, i));
	}
	su_array__sni_pixmap_t_ptr__fini(&properties->attention_icon_pixmap, alloc);
	string_fini(&properties->id, alloc);
	string_fini(&properties->title, alloc);
	string_fini(&properties->attention_movie_name, alloc);
	string_fini(&properties->overlay_icon_name, alloc);
	for ( i = 0; i < properties->overlay_icon_pixmap.len; ++i) {
		alloc->free(alloc,
			su_array__sni_pixmap_t_ptr__get(&properties->overlay_icon_pixmap, i));
	}
	su_array__sni_pixmap_t_ptr__fini(&properties->overlay_icon_pixmap, alloc);
	for ( i = 0; i < properties->tooltip.icon_pixmap.len; ++i) {
		alloc->free(alloc,
			su_array__sni_pixmap_t_ptr__get(&properties->tooltip.icon_pixmap, i));
	}
	su_array__sni_pixmap_t_ptr__fini(&properties->tooltip.icon_pixmap, alloc);
	string_fini(&properties->tooltip.icon_name, alloc);
	string_fini(&properties->tooltip.title, alloc);
	string_fini(&properties->tooltip.text, alloc);

	alloc->free(alloc, properties);
}

static void sni__slot_free(sni__slot_t *slot) {
	sd_bus_slot_unref(slot->slot);
	su_llist__sni__slot_t__pop(&slot->item->private.slots, slot);
	sni_server.in.alloc->free(sni_server.in.alloc, slot);
}

static void sni__dbusmenu_menu_destroy(sni_dbusmenu_menu_t *menu) {
	/* TODO: remove recursion */

	size_t i;
	su_allocator_t *alloc;

	if (menu == NULL) {
		return;
	}

	alloc = sni_server.in.alloc;

	for ( i = 0; i < menu->menu_items.len; ++i) {
		sni_dbusmenu_menu_item_t menu_item = su_array__sni_dbusmenu_menu_item_t__get(
			&menu->menu_items, i);
		sni__dbusmenu_menu_destroy(menu_item.submenu);
		string_fini(&menu_item.label, alloc);
		string_fini(&menu_item.icon_name, alloc);
		alloc->free(alloc, menu_item.icon_data.bytes);
	}
	su_array__sni_dbusmenu_menu_item_t__fini(&menu->menu_items, alloc);

	alloc->free(alloc, menu);
}

static void sni__dbusmenu_properties_destroy(sni_dbusmenu_properties_t *properties) {
	size_t i;

	if (properties == NULL) {
		return;
	}

	for ( i = 0; i < properties->icon_theme_path.len; ++i) {
		string_fini(su_array__su_string_t__get_ptr(&properties->icon_theme_path, i), sni_server.in.alloc);
	}
	su_array__su_string_t__fini(&properties->icon_theme_path, sni_server.in.alloc);

	sni_server.in.alloc->free(sni_server.in.alloc, properties);
}

static void sni__dbusmenu_destroy(sni_dbusmenu_t *dbusmenu) {
	if (dbusmenu == NULL) {
		return;
	}

	sni__dbusmenu_menu_destroy(dbusmenu->menu);
	sni__dbusmenu_properties_destroy(dbusmenu->properties);

	sni_server.in.alloc->free(sni_server.in.alloc, dbusmenu);
}

static sni_dbusmenu_menu_t *sni__dbusmenu_menu_create(sd_bus_message *msg,
		sni_dbusmenu_t *dbusmenu, sni_dbusmenu_menu_item_t *parent_menu_item) {
	/* TODO: remove recursion */
	su_allocator_t *alloc = sni_server.in.alloc;

	sni_dbusmenu_menu_t *menu = alloc->alloc(
		alloc, sizeof(*menu), ALIGNOF(*menu));
	menu->dbusmenu = dbusmenu;
	menu->parent_menu_item = parent_menu_item;
	menu->depth = (parent_menu_item ? (parent_menu_item->parent_menu->depth + 1) : 0);
	memset(&menu->menu_items, 0, sizeof(menu->menu_items));

	su_array__sni_dbusmenu_menu_item_t__init(&menu->menu_items, alloc, 32);

	while (sd_bus_message_at_end(msg, 0) == 0) {
		sni_dbusmenu_menu_item_t *menu_item_;
		bool32_t children = FALSE;

		sni_dbusmenu_menu_item_t menu_item;
		memset(&menu_item, 0, sizeof(menu_item));
		menu_item.parent_menu = menu;
		/*menu_item.type = SNI_DBUSMENU_MENU_ITEM_TYPE_STANDARD;*/
		menu_item.enabled = 1;
		menu_item.visible = 1;
		/*menu_item.toggle_type = SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_NONE;*/
		menu_item.toggle_state = -1;
		/*menu_item.disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_NORNAL;*/

		sd_bus_message_enter_container(msg, 'v', "(ia{sv}av)");
		sd_bus_message_enter_container(msg, 'r', "ia{sv}av");

		sd_bus_message_read_basic(msg, 'i', &menu_item.id);

		sd_bus_message_enter_container(msg, 'a', "{sv}");
		while (sd_bus_message_enter_container(msg, 'e', "sv") == 1) {
			string_t s;
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) == 0) {
				goto exit_con;
			}
			s.free_contents = FALSE;
			s.nul_terminated = TRUE;
			sd_bus_message_enter_container(msg, 'v', NULL);
			if (string_equal(s, string("type"))) {
				sd_bus_message_read_basic(msg, 's', &s.s);
				s.len = strlen(s.s);
				if (string_equal(s, string("separator"))) {
					menu_item.type = SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR;
				}
			} else if (string_equal(s, string("label"))) {
				size_t l = 0;
				char *c;
				char *label;
				sd_bus_message_read_basic(msg, 's', &label);
				c = label;
				menu_item.label.s = alloc->alloc(alloc,
					strlen(label) + 1, ALIGNOF(*menu_item.label.s));
				for ( ; *c; ++c) {
					if ((*c == '_') && (!*++c)) {
						break;
					}
					menu_item.label.s[l++] = *c;
				}
				menu_item.label.s[l] = '\0';
				menu_item.label.len = l;
				menu_item.label.free_contents = TRUE;
				menu_item.label.nul_terminated = TRUE;
				/* TODO: handle '_', '__' properly */
			} else if (string_equal(s, string("enabled"))) {
				sd_bus_message_read_basic(msg, 'b', &menu_item.enabled);
			} else if (string_equal(s, string("visible"))) {
				sd_bus_message_read_basic(msg, 'b', &menu_item.visible);
			} else if (string_equal(s, string("icon-name"))) {
				sd_bus_message_read_basic(msg, 's', &s.s);
				if ((s.len = strlen(s.s)) > 0) {
					string_init_len(&menu_item.icon_name, alloc, s.s, s.len, TRUE);
				}
			} else if (string_equal(s, string("icon-data"))) {
				const void *bytes;
				sd_bus_message_read_array(msg, 'y', &bytes, &menu_item.icon_data.nbytes);
				menu_item.icon_data.bytes = alloc->alloc( alloc, menu_item.icon_data.nbytes, 64);
				memcpy(menu_item.icon_data.bytes, bytes, menu_item.icon_data.nbytes);
			/*} else if (string_equal(key_str, string("shortcut"))) { */
			} else if (string_equal(s, string("toggle-type"))) {
				sd_bus_message_read_basic(msg, 's', &s.s);
				if ((s.len = strlen(s.s)) > 0) {
					if (string_equal(s, string("checkmark"))) {
						menu_item.toggle_type = SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_CHECKMARK;
					} else if (string_equal(s, string("radio"))) {
						menu_item.toggle_type = SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_RADIO;
					}
				}
			} else if (string_equal(s, string("toggle-state"))) {
				sd_bus_message_read_basic(msg, 'i', &menu_item.toggle_state);
			} else if (string_equal(s, string("children-display"))) {
				sd_bus_message_read_basic(msg, 's', &s.s);
				s.len = strlen(s.s);
				if (string_equal(s, string("submenu"))) {
					children = TRUE;
				}
			} else if (string_equal(s, string("disposition"))) {
				sd_bus_message_read_basic(msg, 's', &s.s);
				if ((s.len = strlen(s.s)) > 0) {
					if (string_equal(s, string("normal"))) {
						menu_item.disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_NORNAL;
					} else if (string_equal(s, string("informative"))) {
						menu_item.disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_INFORMATIVE;
					} else if (string_equal(s, string("warning"))) {
						menu_item.disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_WARNING;
					} else if (string_equal(s, string("alert"))) {
						menu_item.disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_ALERT;
					}
				}
			} else {
				/* TODO: "shortcut" */
				sd_bus_message_skip(msg, NULL);
			}
			sd_bus_message_exit_container(msg);
exit_con:
			sd_bus_message_exit_container(msg);
		}
		sd_bus_message_exit_container(msg);

		menu_item_ = su_array__sni_dbusmenu_menu_item_t__add(
			&menu->menu_items, alloc, menu_item);

		sd_bus_message_enter_container(msg, 'a', "v");
		if (children) { /* && (menu_item->id != 0)) { */
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
	sni__slot_t *slot = data;
	sni_item_t *item = slot->item;
	sni_dbusmenu_t *dbusmenu = item->out.dbusmenu;
	struct sni_dbusmenu_menu *old_menu = dbusmenu->menu;
	int ret;

	NOTUSED(ret_error);

	sni__slot_free(slot);

	ret = sd_bus_message_skip(msg, "u");
	if (ret < 0) {
		dbusmenu->menu = NULL;
	} else {
		dbusmenu->menu = sni__dbusmenu_menu_create(msg, dbusmenu, NULL);
		ret = 1;
	}

	if (item->in.dbusmenu_menu_updated) {
		item->in.dbusmenu_menu_updated(item);
	}

	sni__dbusmenu_menu_destroy(old_menu);

	return ret;
}

static int sni__dbusmenu_get_layout(sni_dbusmenu_t *dbusmenu) {
	su_allocator_t *alloc = sni_server.in.alloc;
	sni_item_t *item = dbusmenu->item;
	sni__slot_t *slot = alloc->alloc(alloc, sizeof(*slot), ALIGNOF(*slot));
	int ret = sd_bus_call_method_async(sni_server.private.bus, &slot->slot, item->private.service.s,
			item->out.properties->menu.s, sni__dbusmenu_interface, "GetLayout",
			sni__dbusmenu_handle_get_layout, slot, "iias", 0, -1, NULL);
	if (ret < 0) {
		alloc->free(alloc, slot);
		return ret;
	}

	slot->item = item;
	su_llist__sni__slot_t__insert_head(&item->private.slots, slot);

	return 1;
}

static int sni__dbusmenu_handle_signal(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	/* ? TODO: error check */
	sni__slot_t *slot = data;

	NOTUSED(msg); NOTUSED(ret_error);

	/* TODO: ItemActivationRequested */

	sni__dbusmenu_get_layout(slot->item->out.dbusmenu);
	return 1;
}

static int sni__dbusmenu_handle_get_properties(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	sni__slot_t *slot = data;
	su_allocator_t *alloc;
	sni_dbusmenu_t *dbusmenu = slot->item->out.dbusmenu;
	sni_dbusmenu_properties_t *props;
	int ret;

	NOTUSED(ret_error);

	sni__slot_free(slot);

	ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
	if (ret < 0) {
		return ret;
	}

	alloc = sni_server.in.alloc;

	props = alloc->alloc(alloc, sizeof(*props), ALIGNOF(*props));
	memset(props, 0, sizeof(*props));
	while (sd_bus_message_enter_container(msg, 'e', "sv") == 1) {
		string_t s;
		sd_bus_message_read_basic(msg, 's', &s.s);
		if ((s.len = strlen(s.s)) == 0) {
			goto exit_con;
		}
		s.free_contents = FALSE;
		s.nul_terminated = TRUE;
		sd_bus_message_enter_container(msg, 'v', NULL);
		if (string_equal(s, string("IconThemePath"))) {
			char **icon_theme_path;
			sd_bus_message_read_strv(msg, &icon_theme_path);
			if (icon_theme_path) {
				char **p = icon_theme_path;
				su_array__su_string_t__init(&props->icon_theme_path, alloc, 4);
				for ( ; *p != NULL; ++p) {
					char *path = *p;
					size_t len;
					if ((len = strlen(path)) > 0) {
						string_init_len(
							su_array__su_string_t__add_uninitialized(&props->icon_theme_path, alloc),
							alloc, path, len, TRUE);
					} else {
						free(path);
					}
				}
				free(icon_theme_path);
			}
		} else if (string_equal(s, string("Status"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				if (string_equal(s, string("normal"))) {
					props->status = SNI_DBUSMENU_STATUS_NORMAL;
				} else if (string_equal(s, string("notice"))) {
					props->status = SNI_DBUSMENU_STATUS_NOTICE;
				}
			}
		} else if (string_equal(s, string("TextDirection"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				if (string_equal(s, string("ltr"))) {
					props->text_direction = SNI_DBUSMENU_TEXT_DIRECTION_LEFT_TO_RIGHT;
				} else if (string_equal(s, string("rtl"))) {
					props->text_direction = SNI_DBUSMENU_TEXT_DIRECTION_RIGHT_TO_LEFT;
				}
			}
		} else {
			/* ignored: Version */
			sd_bus_message_skip(msg, NULL);
		}
		sd_bus_message_exit_container(msg);
exit_con:
		sd_bus_message_exit_container(msg);
	}
	/*sd_bus_message_exit_container(msg); */

	dbusmenu->properties = props;
	return 1;
}

static sni_dbusmenu_t *sni__dbusmenu_create(sni_item_t *item) {
	sni__slot_t *slot1, *slot2;
	sni_dbusmenu_t *dbusmenu;
	su_allocator_t *alloc;
	int ret;

	if (item->out.properties->menu.len == 0) {
		return NULL;
	}

	alloc = sni_server.in.alloc;

	slot1 = alloc->alloc(alloc, sizeof(*slot1), ALIGNOF(*slot1));
	ret = sd_bus_call_method_async(sni_server.private.bus, &slot1->slot, item->private.service.s,
			item->out.properties->menu.s, "org.freedesktop.DBus.Properties", "GetAll",
			sni__dbusmenu_handle_get_properties, slot1, "s", sni__dbusmenu_interface);
	if (ret < 0) {
		goto error_1;
	}

	slot2 = alloc->alloc(alloc, sizeof(*slot2), ALIGNOF(*slot2));
	ret = sd_bus_match_signal_async(sni_server.private.bus, &slot2->slot, item->private.service.s,
			item->out.properties->menu.s, sni__dbusmenu_interface, NULL,
			sni__dbusmenu_handle_signal, sni__dbusmenu_handle_signal, slot2);
	if (ret < 0) {
		goto error_2;
	}

	dbusmenu = alloc->alloc(alloc, sizeof(*dbusmenu), ALIGNOF(*dbusmenu));
	dbusmenu->item = item;
	dbusmenu->properties = NULL;
	dbusmenu->menu = NULL;

	slot1->item = item;
	su_llist__sni__slot_t__insert_head(&item->private.slots, slot1);
	slot2->item = item;
	su_llist__sni__slot_t__insert_head(&item->private.slots, slot2);

	return dbusmenu;
error_2:
	sd_bus_slot_unref(slot1->slot);
	alloc->free(alloc, slot2);
error_1:
	alloc->free(alloc, slot1);
	return NULL;
}

static int sni__item_handle_get_properties(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	sni__slot_t *slot = data;
	sni_item_t *item = slot->item;
	sni_item_properties_t *props;
	su_allocator_t *alloc = sni_server.in.alloc;
	int ret;

	NOTUSED(ret_error);

	sni__slot_free(slot);
	sni__item_properties_destroy(item->out.properties);

	ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
	if (ret < 0) {
		item->out.properties = NULL;
		goto out;
	} else {
		item->out.properties = alloc->alloc(alloc,
			sizeof(*item->out.properties), ALIGNOF(*item->out.properties));
		memset(item->out.properties, 0, sizeof(*item->out.properties));
		ret = 1;
	}

	props = item->out.properties;
	while (sd_bus_message_enter_container(msg, 'e', "sv") == 1) {
		string_t s;
		sd_bus_message_read_basic(msg, 's', &s.s);
		if ((s.len = strlen(s.s)) == 0) {
			goto exit_con;
		}
		s.free_contents = FALSE;
		s.nul_terminated = TRUE;
		sd_bus_message_enter_container(msg, 'v', NULL);
		if (string_equal(s, string("IconName"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				string_init_len(&props->icon_name, alloc, s.s, s.len, TRUE);
			}
		} else if (string_equal(s, string("IconThemePath"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				string_init_len(&props->icon_theme_path, alloc, s.s, s.len, TRUE);
			}
		} else if (string_equal(s, string("IconPixmap"))) {
			sni__item_read_pixmap(msg, &props->icon_pixmap);
		} else if (string_equal(s, string("Status"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				if (string_equal(s, string("Active"))) {
					props->status = SNI_ITEM_STATUS_ACTIVE;
				} else if (string_equal(s, string("Passive"))) {
					props->status = SNI_ITEM_STATUS_PASSIVE;
				} else if (string_equal(s, string("NeedsAttention"))) {
					props->status = SNI_ITEM_STATUS_NEEDS_ATTENTION;
				}
			}
		} else if (string_equal(s, string("Category"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				if (string_equal(s, string("ApplicationStatus"))) {
					props->category = SNI_ITEM_CATEGORY_APPLICATION_STATUS;
				} else if (string_equal(s, string("Communications"))) {
					props->category = SNI_ITEM_CATEGORY_COMMUNICATIONS;
				} else if (string_equal(s, string("SystemServices"))) {
					props->category = SNI_ITEM_CATEGORY_SYSTEM_SERVICES;
				} else if (string_equal(s, string("Hardware"))) {
					props->category = SNI_ITEM_CATEGORY_HARDWARE;
				}
			}
		} else if (string_equal(s, string("Menu"))) {
			sd_bus_message_read_basic(msg, 'o', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				string_init_len(&props->menu, alloc, s.s, s.len, TRUE);
			}
		} else if (string_equal(s, string("AttentionIconName"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				string_init_len(&props->attention_icon_name, alloc, s.s, s.len, TRUE);
			}
		} else if (string_equal(s, string("AttentionIconPixmap"))) {
			sni__item_read_pixmap(msg, &props->attention_icon_pixmap);
		} else if (string_equal(s, string("ItemIsMenu"))) {
			sd_bus_message_read_basic(msg, 'b', &props->item_is_menu);
		} else if (string_equal(s, string("WindowId"))) {
			sd_bus_message_read_basic(msg, 'i', &props->window_id);
		} else if (string_equal(s, string("Id"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				string_init_len(&props->id, alloc, s.s, s.len, TRUE);
			}
		} else if (string_equal(s, string("Title"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				string_init_len(&props->title, alloc, s.s, s.len, TRUE);
			}
		} else if (string_equal(s, string("AttentionMovieName"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				string_init_len(&props->attention_movie_name, alloc, s.s, s.len, TRUE);
			}
		} else if (string_equal(s, string("OverlayIconName"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				string_init_len(&props->overlay_icon_name, alloc, s.s, s.len, TRUE);
			}
		} else if (string_equal(s, string("OverlayIconPixmap"))) {
			sni__item_read_pixmap(msg, &props->overlay_icon_pixmap);
		} else if (string_equal(s, string("ToolTip"))) {
			sd_bus_message_enter_container(msg, 'r', "sa(iiay)ss");
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				string_init_len(&props->tooltip.icon_name, alloc, s.s, s.len, TRUE);
			}
			sni__item_read_pixmap(msg, &props->tooltip.icon_pixmap);
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				string_init_len(&props->tooltip.title, alloc, s.s, s.len, TRUE);
			}
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = strlen(s.s)) > 0) {
				string_init_len(&props->tooltip.text, alloc, s.s, s.len, TRUE);
			}
			sd_bus_message_exit_container(msg);
		} else {
			sd_bus_message_skip(msg, NULL);
		}
		sd_bus_message_exit_container(msg);
exit_con:
		sd_bus_message_exit_container(msg);
	}
	/*sd_bus_message_exit_container(msg); */

	if (item->out.dbusmenu == NULL) {
		item->out.dbusmenu = sni__dbusmenu_create(item);
	}

out:
	if (item->in.properties_updated) {
		item->in.properties_updated(item);
	}

	return ret;
}

static int sni__item_handle_signal(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	/* ? TODO: error check */
	sni_item_t *item = data;
	su_allocator_t *alloc = sni_server.in.alloc;
	sni__slot_t *slot = alloc->alloc(
		alloc, sizeof(*slot), ALIGNOF(*slot));
	int ret;

	NOTUSED(msg); NOTUSED(ret_error);

	ret = sd_bus_call_method_async(sni_server.private.bus, &slot->slot, item->private.service.s,
			item->private.path.s, "org.freedesktop.DBus.Properties", "GetAll",
			sni__item_handle_get_properties, slot, "s", sni__item_interface);
	if (ret >= 0) {
		slot->item = item;
		su_llist__sni__slot_t__insert_head(&item->private.slots, slot);
	} else {
		alloc->free(alloc, slot);
	}

	return 1;
}

static void sni__item_destroy(sni_item_t *item) {
	su_allocator_t *alloc = sni_server.in.alloc;
	sni__slot_t *slot = item->private.slots.head;
	for ( ; slot; ) {
		sni__slot_t *next = slot->next;
        sd_bus_slot_unref(slot->slot);
        alloc->free(alloc, slot);
        slot = next;
	}

	sni__dbusmenu_destroy(item->out.dbusmenu);

	sni__item_properties_destroy(item->out.properties);

    string_fini(&item->private.watcher_id, alloc);
	string_fini(&item->private.service, alloc);
	string_fini(&item->private.path, alloc);

	if (item->in.destroy) {
		item->in.destroy(item);
	}
}

static sni_item_t *sni__item_create(string_t id) {
	sni_item_t *item;
	sni__slot_t *slot;
	sd_bus_slot *slot_;
	string_t path;
	su_allocator_t *alloc;
	int ret;

	if (!string_find_char(id, '/', &path)) {
		return NULL;
	}

	if (!sni_server.in.item_create || !(item = sni_server.in.item_create())) {
		return NULL;
	}

	alloc = sni_server.in.alloc;
	string_init_len(&item->private.service, alloc, id.s, id.len - path.len, TRUE);

	ret = sd_bus_match_signal_async(sni_server.private.bus, &slot_, item->private.service.s,
			path.s, sni__item_interface, NULL,
			sni__item_handle_signal, sni__item_handle_signal, item);
	if (ret < 0) {
		sni__item_destroy(item);
		return NULL;
	}

	slot = alloc->alloc( alloc, sizeof(*slot), ALIGNOF(*slot));
	slot->item = item;
	slot->slot = slot_;
	su_llist__sni__slot_t__insert_head(&item->private.slots, slot);

	if (id.free_contents) {
		item->private.watcher_id = id;
		item->private.path = path;
	} else {
		string_init_string(&item->private.watcher_id, alloc, id);
		string_init_string(&item->private.path, alloc, path);
	}

	return item;
}

static int sni__watcher_handle_register_item(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	const char *service, *path;
	string_t id;
	char *service_or_path;
	int ret;
	size_t i;
	
	NOTUSED(data); NOTUSED(ret_error);

	ret= sd_bus_message_read_basic(msg, 's', &service_or_path);
	if (ret < 0) {
		return ret;
	}

	/*if (strlen(service_or_path) == 0) { */
	/*	return -EINVAL; */
	/*} */

	if (service_or_path[0] == '/') {
		service = sd_bus_message_get_sender(msg);
		path = service_or_path;
	} else {
		service = service_or_path;
		path = "/StatusNotifierItem";
	}

	string_init_format(&id, sni_server.in.alloc, "%s%s", service, path);
    for ( i = 0; i < sni_server.private.watcher_items.len; ++i) {
        if (string_equal(id, su_array__su_string_t__get(&sni_server.private.watcher_items, i))) {
            string_fini(&id, sni_server.in.alloc);
            return -EEXIST;
        }
    }

    ret = sd_bus_emit_signal(sni_server.private.bus, sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierItemRegistered", "s", id.s);
    if (ret < 0) {
        string_fini(&id, sni_server.in.alloc);
        return ret;
    }

    su_array__su_string_t__add(&sni_server.private.watcher_items, sni_server.in.alloc, id);

	/*ret = sd_bus_emit_properties_changed(sni_server.private.bus, */
	/*	sni__watcher_obj_path, sni__watcher_interface, */
	/*	"RegisteredStatusNotifierItems", NULL); */
	/*if (ret < 0) { */
	/*	string_fini(&id, sni_server.in.alloc); */
	/*	return ret; */
	/*} */

    ret = sd_bus_reply_method_return(msg, "");
    if (ret < 0) {
        return ret;
    }

	return 1;
}

static int sni__watcher_handle_register_host(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	size_t i;
	string_t s;
	int ret;

	NOTUSED(data); NOTUSED(ret_error);

	ret = sd_bus_message_read_basic(msg, 's', &s.s);
	if (ret < 0) {
		return ret;
	}

	s.len = strlen(s.s);
	s.nul_terminated = TRUE;
	s.free_contents = FALSE;

    for ( i = 0; i < sni_server.private.watcher_hosts.len; ++i) {
        if (string_equal(s, su_array__su_string_t__get(&sni_server.private.watcher_hosts, i))) {
            return -EEXIST;
        }
    }

	/*if (sni_server.private.watcher_hosts.len == 0) { */
	/*	ret = sd_bus_emit_properties_changed(sni_server.private.bus, */
	/*		sni__watcher_obj_path, sni__watcher_interface, */
	/*		"IsStatusNotifierHostRegistered", NULL); */
	/*	if (ret < 0) { */
	/*		return ret; */
	/*	} */
	/*} */

    ret = sd_bus_emit_signal(sni_server.private.bus, sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierHostRegistered", "");
    if (ret < 0) {
        return ret;
    }

	string_init_string(&s, sni_server.in.alloc, s);
    su_array__su_string_t__add(&sni_server.private.watcher_hosts, sni_server.in.alloc, s);

    ret = sd_bus_reply_method_return(msg, "");
    if (ret < 0) {
        return ret;
    }

	return 1;
}

static int sni__watcher_handle_get_registered_items(sd_bus *b, const char *path,
		const char *iface, const char *prop, sd_bus_message *reply,
		void *data, sd_bus_error *ret_error) {
	/* ? TODO: scratch alloc */
	su_allocator_t *alloc = sni_server.in.alloc;
	char **array = alloc->alloc(alloc,
		(sni_server.private.watcher_items.len + 1) * sizeof(*array), ALIGNOF(*array));
	size_t i = 0;
	int ret;

	NOTUSED(b); NOTUSED(path); NOTUSED(iface); NOTUSED(prop); NOTUSED(data); NOTUSED(ret_error);

	for ( ; i < sni_server.private.watcher_items.len; ++i) {
		array[i] = su_array__su_string_t__get(&sni_server.private.watcher_items, i).s;
	}
	array[sni_server.private.watcher_items.len] = NULL;
	ret = sd_bus_message_append_strv(reply, array);
	alloc->free(alloc, array);
    if (ret < 0) {
		return ret;
    }

    return 1;
}

static int sni__watcher_handle_is_host_registered(sd_bus *b, const char *path,
		const char *iface, const char *prop, sd_bus_message *reply,
		void *data, sd_bus_error *ret_error) {
	int registered = (sni_server.private.watcher_hosts.len > 0);
	int ret = sd_bus_message_append_basic(reply, 'b', &registered);

	NOTUSED(b); NOTUSED(path); NOTUSED(iface); NOTUSED(prop); NOTUSED(data); NOTUSED(ret_error);

	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni__watcher_handle_lost_service(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	string_t s;
	size_t i;
	char *old_owner, *new_owner;
	int ret = sd_bus_message_read(msg, "sss", &s.s, &old_owner, &new_owner);

	NOTUSED(data); NOTUSED(ret_error);

	if (ret < 0) {
		return ret;
	}
    if (*new_owner) {
        return 0;
    }

	s.len = strlen(s.s);
	s.free_contents = FALSE;
	s.nul_terminated = TRUE;

    for ( i = 0; i < sni_server.private.watcher_items.len; ++i) {
        string_t item = su_array__su_string_t__get(&sni_server.private.watcher_items, i);
        if (string_compare(item, s, s.len) == 0) {
            ret = sd_bus_emit_signal(sni_server.private.bus, sni__watcher_obj_path, sni__watcher_interface,
					"StatusNotifierItemUnregistered", "s", item.s);
            if (ret < 0) {
                return ret;
            }
            string_fini(&item, sni_server.in.alloc);
            su_array__su_string_t__pop_swapback(&sni_server.private.watcher_items, i);
			/*ret = sd_bus_emit_properties_changed(sni_server.private.bus,*/
			/*	sni__watcher_obj_path, sni__watcher_interface,*/
			/*	"RegisteredStatusNotifierItems", NULL);*/
			/*if (ret < 0) {*/
			/*	return ret;*/
			/*}*/
            return 0;
        }
    }

    for ( i = 0; i < sni_server.private.watcher_hosts.len; ++i) {
        string_t host = su_array__su_string_t__get(&sni_server.private.watcher_hosts, i);
        if (string_equal(s, host)) {
			/*if (sni_server.private.watcher_hosts.len == 1) { */
			/*	ret = sd_bus_emit_properties_changed(sni_server.private.bus, */
			/*		sni__watcher_obj_path, sni__watcher_interface, */
			/*		"IsStatusNotifierHostRegistered", NULL); */
			/*	if (ret < 0) { */
			/*		return ret; */
			/*	} */
			/*} */
            ret = sd_bus_emit_signal(sni_server.private.bus, sni__watcher_obj_path, sni__watcher_interface,
					"StatusNotifierHostUnregistered", "");
			if (ret < 0) {
                return ret;
            }
            string_fini(&host, sni_server.in.alloc);
            su_array__su_string_t__pop_swapback(&sni_server.private.watcher_hosts, i);
            return 0;
        }
    }

	return 0;
}

static int sni__host_add_item(string_t id) {
	sni_item_t *item;
	size_t i = 0;
	for ( ; i < sni_server.out.host_items.len; ++i) {
		item = su_array__sni_item_t_ptr__get(&sni_server.out.host_items, i);
		if (string_equal(id, item->private.watcher_id)) {
			return -EEXIST;
		}
	}

	if ((item = sni__item_create(id))) {
		ASSERT(string_equal(item->private.watcher_id, id));
		su_array__sni_item_t_ptr__add(&sni_server.out.host_items, sni_server.in.alloc, item);
		return 1;
	} else {
		return -EINVAL;
	}
}

static int sni__host_handle_get_registered_items(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	char **ids;
	int ret = sd_bus_message_enter_container(msg, 'v', "as");

	NOTUSED(data); NOTUSED(ret_error);

	if (ret < 0) {
		return ret;
	}

	ret = sd_bus_message_read_strv(msg, &ids);
	/*sd_bus_message_exit_container(msg);*/
	if (ret < 0) {
		return ret;
	}

	if (ids) {
		char **s = ids;
		for ( ; *s != NULL; ++s) {
			char *id = *s;
			string_t str;
			str.s = id;
			str.len = strlen(str.s);
			str.free_contents = TRUE;
			str.nul_terminated = TRUE;
			if ((str.len == 0) || (0 > sni__host_add_item(str))) {
				free(id);
			}
		}
		free(ids);
	}

	return 1;
}

static int sni__host_register_to_watcher(void) {
	/* ? TODO: slots */
	int ret = sd_bus_call_method_async(sni_server.private.bus, NULL,
			sni__watcher_interface, sni__watcher_obj_path, sni__watcher_interface,
			"RegisterStatusNotifierHost", NULL, NULL,
			"s", sni_server.private.host_interface.s);
	if (ret < 0) {
		return ret;
	}

	ret = sd_bus_call_method_async(sni_server.private.bus, NULL,
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
	string_t s;
	int ret = sd_bus_message_read_basic(msg, 's', &s.s);

	NOTUSED(data); NOTUSED(ret_error);

	if (ret < 0) {
		return ret;
	}

	s.len = strlen(s.s);
	s.free_contents = FALSE;
	s.nul_terminated = TRUE;

	if (s.len > 0) {
		return sni__host_add_item(s);
	} else {
		return -EINVAL;
	}
}

static int sni__host_handle_item_unregistered(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	string_t s;
	int ret = sd_bus_message_read_basic(msg, 's', &s.s);

	NOTUSED(data); NOTUSED(ret_error);

	if (ret < 0) {
		return ret;
	}

	s.len = strlen(s.s);
	s.free_contents = FALSE;
	s.nul_terminated = TRUE;

	if (s.len > 0) {
		size_t i = 0;
		for ( ; i < sni_server.out.host_items.len; ++i) {
			sni_item_t *item = su_array__sni_item_t_ptr__get(&sni_server.out.host_items, i);
			if (string_equal(item->private.watcher_id, s)) {
				su_array__sni_item_t_ptr__pop(&sni_server.out.host_items, i);
				sni__item_destroy(item);
				break;
			}
		}
	}

	return 1;
}

static int sni__host_handle_new_watcher(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	char *service, *old_owner, *new_owner;
	int ret = sd_bus_message_read(msg, "sss", &service, &old_owner, &new_owner);

	NOTUSED(data); NOTUSED(ret_error);

	if (ret < 0) {
		return ret;
	}

	if (strcmp(service, sni__watcher_interface) == 0) {
		size_t i;
        ret = sni__host_register_to_watcher();
		if (ret < 0) {
			return ret;
		}
		for ( i = sni_server.out.host_items.len - 1; i != SIZE_MAX; --i) {
			sni_item_t *item = su_array__sni_item_t_ptr__get(&sni_server.out.host_items, i);
			sni_server.out.host_items.len--;
			sni__item_destroy(item);
		}
	}

	return 0;
}

static void sni_server_fini(void) {
	su_allocator_t *alloc = sni_server.in.alloc;
	size_t i;
	for ( i = (sni_server.out.host_items.len - 1); i != SIZE_MAX; --i) {
		sni_item_t *item = su_array__sni_item_t_ptr__get(&sni_server.out.host_items, i);
		sni_server.out.host_items.len--;
		sni__item_destroy(item);
	}
	su_array__sni_item_t_ptr__fini(&sni_server.out.host_items, alloc);

	sd_bus_flush_close_unref(sni_server.private.bus);

	string_fini(&sni_server.private.host_interface, alloc);
	for ( i = 0; i < sni_server.private.watcher_items.len; ++i) {
		string_fini(
			su_array__su_string_t__get_ptr(&sni_server.private.watcher_items, i), alloc);
	}
	for ( i = 0; i < sni_server.private.watcher_hosts.len; ++i) {
		string_fini(
			su_array__su_string_t__get_ptr(&sni_server.private.watcher_hosts, i), alloc);
	}
	su_array__su_string_t__fini(&sni_server.private.watcher_items, alloc);
	su_array__su_string_t__fini(&sni_server.private.watcher_hosts, alloc);

	memset(&sni_server.out, 0, sizeof(sni_server.out));
	memset(&sni_server.private, 0, sizeof(sni_server.private));
}

static int sni_server_init(void) {
	int ret;
	su_allocator_t *alloc = sni_server.in.alloc;
	static sd_bus_vtable watcher_vtable[11];

	memset(&sni_server.out, 0, sizeof(sni_server.out));
	memset(&sni_server.private, 0, sizeof(sni_server.private));

	ret = sd_bus_open_user(&sni_server.private.bus);
    if (ret < 0) {
		return ret;
    }

	ret = sd_bus_request_name(sni_server.private.bus, sni__watcher_interface, SD_BUS_NAME_QUEUE);
	if (ret < 0) {
        return ret;
    }

	watcher_vtable[0].type = _SD_BUS_VTABLE_START;
	watcher_vtable[0].flags = 0;
	watcher_vtable[0].x.start.element_size = sizeof(sd_bus_vtable);
	watcher_vtable[1].type = _SD_BUS_VTABLE_METHOD;
	watcher_vtable[1].flags = SD_BUS_VTABLE_UNPRIVILEGED;
	watcher_vtable[1].x.method.member = "RegisterStatusNotifierItem";
	watcher_vtable[1].x.method.signature = "s";
	watcher_vtable[1].x.method.result = "";
	watcher_vtable[1].x.method.handler = sni__watcher_handle_register_item;
	watcher_vtable[1].x.method.offset = 0;
	watcher_vtable[2].type = _SD_BUS_VTABLE_METHOD;
	watcher_vtable[2].flags = SD_BUS_VTABLE_UNPRIVILEGED;
	watcher_vtable[2].x.method.member = "RegisterStatusNotifierHost";
	watcher_vtable[2].x.method.signature = "s";
	watcher_vtable[2].x.method.result = "";
	watcher_vtable[2].x.method.handler = sni__watcher_handle_register_host;
	watcher_vtable[2].x.method.offset = 0;
	watcher_vtable[3].type = _SD_BUS_VTABLE_PROPERTY;
	watcher_vtable[3].flags = SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE;
	watcher_vtable[3].x.property.member = "RegisteredStatusNotifierItems";
	watcher_vtable[3].x.property.signature = "as";
	watcher_vtable[3].x.property.get = sni__watcher_handle_get_registered_items;
	watcher_vtable[3].x.property.set = NULL;
	watcher_vtable[3].x.property.offset = 0;
	watcher_vtable[4].type = _SD_BUS_VTABLE_PROPERTY;
	watcher_vtable[4].flags = SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE;
	watcher_vtable[4].x.property.member = "IsStatusNotifierHostRegistered";
	watcher_vtable[4].x.property.signature = "b";
	watcher_vtable[4].x.property.get = sni__watcher_handle_is_host_registered;
	watcher_vtable[4].x.property.set = NULL;
	watcher_vtable[4].x.property.offset = 0;
	watcher_vtable[5].type = _SD_BUS_VTABLE_PROPERTY;
	watcher_vtable[5].flags = SD_BUS_VTABLE_PROPERTY_CONST;
	watcher_vtable[5].x.property.member = "ProtocolVersion";
	watcher_vtable[5].x.property.signature = "i";
	watcher_vtable[5].x.property.get = NULL;
	watcher_vtable[5].x.property.set = NULL;
	watcher_vtable[5].x.property.offset = 0;
	watcher_vtable[6].type = _SD_BUS_VTABLE_SIGNAL;
	watcher_vtable[6].flags = 0;
	watcher_vtable[6].x.signal.member = "StatusNotifierItemRegistered";
	watcher_vtable[6].x.signal.signature = "s";
	watcher_vtable[7].type = _SD_BUS_VTABLE_SIGNAL;
	watcher_vtable[7].flags = 0;
	watcher_vtable[7].x.signal.member = "StatusNotifierItemUnregistered";
	watcher_vtable[7].x.signal.signature = "s";
	watcher_vtable[8].type = _SD_BUS_VTABLE_SIGNAL;
	watcher_vtable[8].flags = 0;
	watcher_vtable[8].x.signal.member = "StatusNotifierHostRegistered";
	watcher_vtable[8].x.signal.signature = NULL;
	watcher_vtable[9].type = _SD_BUS_VTABLE_SIGNAL;
	watcher_vtable[9].flags = 0;
	watcher_vtable[9].x.signal.member = "StatusNotifierHostUnregistered";
	watcher_vtable[9].x.signal.signature = NULL;
	watcher_vtable[10].type = _SD_BUS_VTABLE_END;
	watcher_vtable[10].flags = 0;
	watcher_vtable[10].x.start.element_size = 0;

    ret = sd_bus_add_object_vtable(sni_server.private.bus, NULL, sni__watcher_obj_path,
            sni__watcher_interface, watcher_vtable,
			&sni__watcher_protocol_version);
    if (ret < 0) {
        return ret;
    }

    ret = sd_bus_match_signal(sni_server.private.bus, NULL, "org.freedesktop.DBus",
			"/org/freedesktop/DBus", "org.freedesktop.DBus",
			"NameOwnerChanged", sni__watcher_handle_lost_service, NULL);
    if (ret < 0) {
        return ret;
    }

    su_array__su_string_t__init(&sni_server.private.watcher_items, alloc, 16);
    su_array__su_string_t__init(&sni_server.private.watcher_hosts, alloc, 4);


	string_init_format(&sni_server.private.host_interface, alloc,
		"org.kde.StatusNotifierHost-%d", getpid());
    ret = sd_bus_request_name(sni_server.private.bus, sni_server.private.host_interface.s, 0); /* ? TODO: SD_BUS_NAME_QUEUE */
    if (ret < 0) {
        return ret;
    }

    ret = sni__host_register_to_watcher();
    if (ret < 0) {
        return ret;
    }

    ret = sd_bus_match_signal(sni_server.private.bus, NULL, sni__watcher_interface,
			sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierItemRegistered", sni__host_handle_item_registered, NULL);
	if (ret < 0) {
		return ret;
	}
	ret = sd_bus_match_signal(sni_server.private.bus, NULL, sni__watcher_interface,
			sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierItemUnregistered", sni__host_handle_item_unregistered, NULL);
	if (ret < 0) {
		return ret;
	}

	ret = sd_bus_match_signal(sni_server.private.bus, NULL, "org.freedesktop.DBus",
			"/org/freedesktop/DBus", "org.freedesktop.DBus", "NameOwnerChanged",
			sni__host_handle_new_watcher, NULL);
	if (ret < 0) {
		return ret;
	}

    su_array__sni_item_t_ptr__init(&sni_server.out.host_items, alloc, 16);

	return 1;
}

static int sni_server_get_poll_info(struct pollfd *pollfd_out, int64_t *absolute_timeout_ms) {
	uint64_t usec;
	int fd, events, ret;

	fd = sd_bus_get_fd(sni_server.private.bus);
	if (fd < 0) {
		return fd;
	}

	events = sd_bus_get_events(sni_server.private.bus);
	if (events < 0) {
		return events;
	}

	ret = sd_bus_get_timeout(sni_server.private.bus, &usec);
	if (ret < 0) {
		return ret;
	}

	switch (usec) {
	case UINT64_MAX:
		*absolute_timeout_ms = -1;
		break;
	case 0:
		*absolute_timeout_ms = 0;
		break;
	default:
		*absolute_timeout_ms = (int)(usec / 1000);
		break;
	}
	pollfd_out->fd = fd;
	pollfd_out->events = (short)events;

	return 1;
}

static int sni_server_process(void) {
	int ret;
    while ((ret = sd_bus_process(sni_server.private.bus, NULL)) > 0)
	{
	}
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni__item_handle_method(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	sni__slot_t *slot = data;
	NOTUSED(msg); NOTUSED(ret_error);
	sni__slot_free(slot);
	return 1;
}

static int sni_item_context_menu(sni_item_t *item, int x, int y) {
	int ret = sd_bus_call_method(sni_server.private.bus, item->private.service.s,
		item->private.path.s, sni__item_interface, "ContextMenu",
		NULL, NULL, "ii", x, y);
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_context_menu_async(sni_item_t *item, int x, int y) {
	su_allocator_t *alloc = sni_server.in.alloc;
	sni__slot_t *slot = alloc->alloc(
		alloc, sizeof(*slot), ALIGNOF(*slot));
	int ret = sd_bus_call_method_async(sni_server.private.bus, &slot->slot, item->private.service.s,
			item->private.path.s, sni__item_interface, "ContextMenu",
			sni__item_handle_method, slot, "ii", x, y);
	if (ret < 0) {
		alloc->free(alloc, slot);
		return ret;
	} else {
		slot->item = item;
	}

	su_llist__sni__slot_t__insert_head(&item->private.slots, slot);

	return 1;
}

static int sni_item_activate(sni_item_t *item, int x, int y) {
	int ret = sd_bus_call_method(sni_server.private.bus, item->private.service.s,
		item->private.path.s, sni__item_interface, "Activate",
		NULL, NULL, "ii", x, y);
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_activate_async(sni_item_t *item, int x, int y) {
	su_allocator_t *alloc = sni_server.in.alloc;
	sni__slot_t *slot = alloc->alloc(
		alloc, sizeof(*slot), ALIGNOF(*slot));
	int ret = sd_bus_call_method_async(sni_server.private.bus, &slot->slot, item->private.service.s,
			item->private.path.s, sni__item_interface, "Activate",
			sni__item_handle_method, slot, "ii", x, y);
	if (ret < 0) {
		alloc->free(alloc, slot);
		return ret;
	} else {
		slot->item = item;
	}

	su_llist__sni__slot_t__insert_head(&item->private.slots, slot);

	return 1;
}

static int sni_item_secondary_activate(sni_item_t *item, int x, int y) {
	int ret = sd_bus_call_method(sni_server.private.bus, item->private.service.s,
		item->private.path.s, sni__item_interface, "SecondaryActivate",
		NULL, NULL, "ii", x, y);
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_secondary_activate_async(sni_item_t *item, int x, int y) {
	su_allocator_t *alloc = sni_server.in.alloc;
	sni__slot_t *slot = alloc->alloc(
		alloc, sizeof(*slot), ALIGNOF(*slot));
	int ret = sd_bus_call_method_async(sni_server.private.bus, &slot->slot, item->private.service.s,
			item->private.path.s, sni__item_interface, "SecondaryActivate",
			sni__item_handle_method, slot, "ii", x, y);
	if (ret < 0) {
		alloc->free(alloc, slot);
		return ret;
	} else {
		slot->item = item;
	}

	su_llist__sni__slot_t__insert_head(&item->private.slots, slot);

	return 1;
}

static int sni_item_scroll(sni_item_t *item, int delta, sni_item_scroll_orientation_t orientation) {
	int ret = sd_bus_call_method(sni_server.private.bus, item->private.service.s,
		item->private.path.s, sni__item_interface, "Scroll", NULL, NULL,
		"is", delta, (orientation == SNI_ITEM_SCROLL_ORIENTATION_VERTICAL) ? "vertical" : "horizontal");
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_scroll_async(sni_item_t *item, int delta, sni_item_scroll_orientation_t orientation) {
	su_allocator_t *alloc = sni_server.in.alloc;
	sni__slot_t *slot = alloc->alloc(
		alloc, sizeof(*slot), ALIGNOF(*slot));
	int ret = sd_bus_call_method_async(sni_server.private.bus, &slot->slot, item->private.service.s,
			item->private.path.s, sni__item_interface, "Scroll", sni__item_handle_method, slot,
			"is", delta, (orientation == SNI_ITEM_SCROLL_ORIENTATION_VERTICAL) ? "vertical" : "horizontal");
	if (ret < 0) {
		alloc->free(alloc, slot);
		return ret;
	} else {
		slot->item = item;
	}

	su_llist__sni__slot_t__insert_head(&item->private.slots, slot);

	return 1;
}

static int sni_dbusmenu_menu_item_event(sni_dbusmenu_menu_item_t *menu_item,
		sni_dbusmenu_menu_item_event_type_t type, bool32_t async) {
	static const char *event_types[] = {
		"clicked",
		"hovered",
		"opened",
		"closed",
	};
	sni_dbusmenu_t *dbusmenu = menu_item->parent_menu->dbusmenu;
	sni_item_t *item = dbusmenu->item;

	ASSERT(((type == SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_OPENED)
				|| (type == SNI_DBUSMENU_MENU_ITEM_EVENT_TYPE_CLOSED))
			? (menu_item->submenu != NULL) : TRUE);

	if ((dbusmenu->item->out.properties == NULL) || (dbusmenu->item->out.properties->menu.len == 0)) {
		return -ENOENT;
	}

	if (async) {
		su_allocator_t *alloc = sni_server.in.alloc;
		sni__slot_t *slot = alloc->alloc(
			alloc, sizeof(*slot), ALIGNOF(*slot));
		int ret = sd_bus_call_method_async(sni_server.private.bus, &slot->slot, item->private.service.s,
				item->out.properties->menu.s, sni__dbusmenu_interface, "Event",
				sni__item_handle_method, slot, "isvu",
				menu_item->id, event_types[type], "y", 0, time(NULL));
		if (ret < 0) {
			alloc->free(alloc, slot);
			return ret;
		} else {
			slot->item = item;
			su_llist__sni__slot_t__insert_head(&item->private.slots, slot);
		}
	} else {
		int ret = sd_bus_call_method(sni_server.private.bus, item->private.service.s,
				item->out.properties->menu.s, sni__dbusmenu_interface, "Event",
				NULL, NULL, "isvu", menu_item->id, event_types[type], "y", 0, time(NULL));
		if (ret < 0) {
			return ret;
		}
	}

	return 1;
}

static int sni__dbusmenu_menu_handle_about_to_show(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	sni__slot_t *slot = data;
	sni_dbusmenu_t *dbusmenu = slot->item->out.dbusmenu;
	int need_update, ret;	
	
	NOTUSED(ret_error);

	sni__slot_free(slot);

	ret = sd_bus_message_read_basic(msg, 'b', &need_update);
	if (ret >= 0) {
		if (need_update) {
			sni__dbusmenu_get_layout(dbusmenu);
		}
		ret = 1;
	}

	return ret;
}

static int sni_dbusmenu_menu_about_to_show(sni_dbusmenu_menu_t *menu, bool32_t async) {
	sni_item_t *item = menu->dbusmenu->item;
	ASSERT(menu->parent_menu_item != NULL);

	if ((item->out.properties == NULL) || (item->out.properties->menu.len == 0)) {
		return -ENOENT;
	}

	if (async) {
		su_allocator_t *alloc = sni_server.in.alloc;
		sni__slot_t *slot = alloc->alloc(
			alloc, sizeof(*slot), ALIGNOF(*slot));
		int ret = sd_bus_call_method_async(sni_server.private.bus, &slot->slot, item->private.service.s,
				item->out.properties->menu.s, sni__dbusmenu_interface, "AboutToShow",
				sni__dbusmenu_menu_handle_about_to_show, slot, "i", menu->parent_menu_item->id);
		if (ret < 0) {
			alloc->free(alloc, slot);
			return ret;
		} else {
			slot->item = item;
			su_llist__sni__slot_t__insert_head(&item->private.slots, slot);
		}
	} else {
		int ret = sd_bus_call_method(sni_server.private.bus, item->private.service.s,
				item->out.properties->menu.s, sni__dbusmenu_interface, "AboutToShow",
				NULL, NULL, "i", menu->parent_menu_item->id);
		if (ret < 0) {
			return ret;
		}
	}

	return 1;
}

#endif /* SNI_SERVER_H */
