#if !defined(SNI_SERVER_H)
#define SNI_SERVER_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
#include <sutil.h>

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
	fat_ptr_t icon_data; /* png data of the icon. may be NULL */
    sni_dbusmenu_menu_item_toggle_type_t toggle_type;
    int toggle_state; /* 0 - off, 1 - on, else - indeterminate */
    sni_dbusmenu_menu_item_disposition_t disposition;
    int activation_requested; /* bool  NOT IMPLEMENTED */
} sni_dbusmenu_menu_item_t;

typedef struct sni_dbusmenu sni_dbusmenu_t;

struct sni_dbusmenu_menu {
    sni_dbusmenu_t *dbusmenu;
    sni_dbusmenu_menu_item_t *parent_menu_item; /* NULL when root */
	sni_dbusmenu_menu_item_t menu_items[64]; /* TODO: preallocate based on dbus array count */
	size_t menu_items_count;
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
    string_t *icon_theme_path;
	size_t icon_theme_path_count;
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
    uint32_t pixels[1]; /* ARGB32, native byte order, premultipled alpha */
	/* width * height * 4 */
} sni_pixmap_t;

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
	sni_pixmap_t *icon_pixmaps[16]; /* qsorted size descending, premultiplied alpha */ /* TODO: preallocate based on dbus array count */
	size_t icon_pixmaps_count;
	string_t title;
	string_t text;
} sni_item_properties_tooltip_t;

typedef struct sni_item_properties {
    /* every field may be NULL */
    string_t icon_name;
    string_t icon_theme_path;
	sni_pixmap_t *icon_pixmaps[16]; /* qsorted size descending, premultiplied alpha */ /* TODO: preallocate based on dbus array count */
	size_t icon_pixmaps_count;
    sni_item_status_t status;
    sni_item_category_t category;
    string_t menu;
    string_t attention_icon_name;
	sni_pixmap_t *attention_icon_pixmaps[16]; /* qsorted size descending, premultiplied alpha */ /* TODO: preallocate based on dbus array count */
	size_t attention_icon_pixmaps_count;
    int item_is_menu; /* bool */
    int window_id;
    string_t id;
    string_t title;
    string_t attention_movie_name;
    string_t overlay_icon_name;
	sni_pixmap_t *overlay_icon_pixmaps[16]; /* qsorted size descending, premultiplied alpha */ /* TODO: preallocate based on dbus array count */
	size_t overlay_icon_pixmaps_count;
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
	LLIST_NODE_FIELDS(sni__slot_t);
};

typedef struct sni_item_out {
	sni_item_properties_t *properties; /* may be NULL */
	sni_dbusmenu_t *dbusmenu; /* may be NULL */
} sni_item_out_t;

typedef struct sni__slots {
	LLIST_FIELDS(sni__slot_t);
} sni__slots_t;

typedef struct sni__item {
	string_t watcher_id;
	string_t service;
	string_t path;
	sni__slots_t slots;
} sni__item_t;

struct sni_item {
	/*sni_item_in_t in;*/
	sni_item_out_t out;
	sni__item_t priv;
	LLIST_NODE_FIELDS(sni_item_t);
};

typedef struct sni_server_in {
	const allocator_t *alloc;
} sni_server_in_t;

typedef struct sni_items {
	LLIST_FIELDS(sni_item_t);
} sni_items_t;

typedef enum sni_server_event_type {
	SNI_SERVER_EVENT_TYPE_ITEM_CREATE,
	SNI_SERVER_EVENT_TYPE_ITEM_DESTROY,
	SNI_SERVER_EVENT_TYPE_ITEM_PROPERTIES_UPDATED,
	SNI_SERVER_EVENT_TYPE_ITEM_DBUSMENU_MENU_UPDATED
} sni_server_event_type_t;

typedef union sni__server_event {
	string_t id;
	sni_dbusmenu_menu_t *menu;
} sni__server_event_t;

typedef struct sni_server_event {
	sni_server_event_type_t type;
	PAD32;
	sni_item_t *item;
	sni__server_event_t priv;
} sni_server_event_t;

typedef struct sni_server_out {
	sni_items_t items;
	sni_server_event_t *events; /* TODO: rework ownership */
	size_t events_count;
} sni_server_out_t;

typedef struct sni__server {
	sd_bus *bus;
	string_t host_interface;
	string_t *watcher_items;
	size_t watcher_items_count, watcher_items_capacity;
	string_t *watcher_hosts;
	size_t watcher_hosts_count, watcher_hosts_capacity;
	size_t events_capacity;
	bool32_t check_events;
	PAD32;
} sni__server_t;

typedef struct sni_server {
	sni_server_in_t in;
	sni_server_out_t out;
	sni__server_t priv;
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

static void sni__item_read_pixmap( sd_bus_message *msg,
		sni_pixmap_t **dest, size_t *dest_count, size_t dest_capacity) {
	sd_bus_message_enter_container(msg, 'a', "(iiay)");
	while ((sd_bus_message_enter_container(msg, 'r', "iiay") == 1) && (*dest_count < dest_capacity)) {
		int width, height;
		const void *bytes;
		size_t nbytes;
		sd_bus_message_read_basic(msg, 'i', &width);
		sd_bus_message_read_basic(msg, 'i', &height);
		sd_bus_message_read_array(msg, 'y', &bytes, &nbytes);
		if (((size_t)width * (size_t)height * 4) == nbytes) {
			sni_pixmap_t *pixmap;
			ALLOCTS(pixmap, sni_server.in.alloc, ((sizeof(*pixmap) - sizeof(pixmap->pixels)) + nbytes));
			pixmap->width = width;
			pixmap->height = height;

#if (BYTE_ORDER_IS_LITTLE_ENDIAN)
			bswap32_argb_premultiply_alpha(pixmap->pixels, (uint32_t *)(uintptr_t)bytes, ((size_t)width * (size_t)height));
#else
			argb_premultiply_alpha(pixmap->pixels, (uint32_t *)(uintptr_t)bytes, ((size_t)width * (size_t)height));
#endif
			
			dest[(*dest_count)++] = pixmap;
		}
		sd_bus_message_exit_container(msg);
	}
	sd_bus_message_exit_container(msg);

	qsort(dest, *dest_count, sizeof(*dest), sni__item_pixmap_size_descending_qsort);
}

static void sni__item_properties_destroy(sni_item_properties_t *properties) {
	size_t i;
	const allocator_t *alloc;

	if (properties == NULL) {
		return;
	}

	alloc = sni_server.in.alloc;

	string_fini(&properties->icon_name, alloc);
	string_fini(&properties->icon_theme_path, alloc);
	for ( i = 0; i < properties->icon_pixmaps_count; ++i) {
		FREE(alloc, properties->icon_pixmaps[i]);
	}
	string_fini(&properties->menu, alloc);
	string_fini(&properties->attention_icon_name, alloc);
	for ( i = 0; i < properties->attention_icon_pixmaps_count; ++i) {
		FREE(alloc, properties->attention_icon_pixmaps[i]);
	}
	string_fini(&properties->id, alloc);
	string_fini(&properties->title, alloc);
	string_fini(&properties->attention_movie_name, alloc);
	string_fini(&properties->overlay_icon_name, alloc);
	for ( i = 0; i < properties->overlay_icon_pixmaps_count; ++i) {
		FREE(alloc, properties->overlay_icon_pixmaps[i]);
	}
	for ( i = 0; i < properties->tooltip.icon_pixmaps_count; ++i) {
		FREE(alloc, properties->tooltip.icon_pixmaps[i]);
	}
	string_fini(&properties->tooltip.icon_name, alloc);
	string_fini(&properties->tooltip.title, alloc);
	string_fini(&properties->tooltip.text, alloc);

	FREE(alloc, properties);
}

static void sni__slot_free(sni__slot_t *slot) {
	sd_bus_slot_unref(slot->slot);
	LLIST_POP(&slot->item->priv.slots, slot);
	FREE(sni_server.in.alloc, slot);
}

static void sni__dbusmenu_menu_destroy(sni_dbusmenu_menu_t *menu) {
	/* TODO: remove recursion */

	size_t i;
	const allocator_t *alloc;

	if (menu == NULL) {
		return;
	}

	alloc = sni_server.in.alloc;

	for ( i = 0; i < menu->menu_items_count; ++i) {
		sni_dbusmenu_menu_item_t *menu_item = &menu->menu_items[i];
		sni__dbusmenu_menu_destroy(menu_item->submenu);
		string_fini(&menu_item->label, alloc);
		string_fini(&menu_item->icon_name, alloc);
		FREE(alloc, menu_item->icon_data.ptr);
	}

	FREE(alloc, menu);
}

static void sni__dbusmenu_properties_destroy(sni_dbusmenu_properties_t *properties) {
	size_t i;

	if (properties == NULL) {
		return;
	}

	for ( i = 0; i < properties->icon_theme_path_count; ++i) {
		string_fini(&properties->icon_theme_path[i], sni_server.in.alloc);
	}
	FREE(sni_server.in.alloc, properties->icon_theme_path);

	FREE(sni_server.in.alloc, properties);
}

static void sni__dbusmenu_destroy(sni_dbusmenu_t *dbusmenu) {
	if (dbusmenu == NULL) {
		return;
	}

	sni__dbusmenu_menu_destroy(dbusmenu->menu);
	sni__dbusmenu_properties_destroy(dbusmenu->properties);

	FREE(sni_server.in.alloc, dbusmenu);
}

static sni_dbusmenu_menu_t *sni__dbusmenu_menu_create(sd_bus_message *msg,
		sni_dbusmenu_t *dbusmenu, sni_dbusmenu_menu_item_t *parent_menu_item) {
	/* TODO: remove recursion */
	const allocator_t *alloc = sni_server.in.alloc;

	sni_dbusmenu_menu_t *menu;
	ALLOCT(menu, alloc);
	menu->dbusmenu = dbusmenu;
	menu->parent_menu_item = parent_menu_item;
	menu->depth = (parent_menu_item ? (parent_menu_item->parent_menu->depth + 1) : 0);
	menu->menu_items_count = 0;

	while ((sd_bus_message_at_end(msg, 0) == 0) && (menu->menu_items_count < LENGTH(menu->menu_items))) {
		sni_dbusmenu_menu_item_t *menu_item = &menu->menu_items[menu->menu_items_count++];
		bool32_t children = FALSE;

		MEMSET(menu_item, 0, sizeof(*menu_item));
		menu_item->parent_menu = menu;
		/*menu_item->type = SNI_DBUSMENU_MENU_ITEM_TYPE_STANDARD;*/
		menu_item->enabled = 1;
		menu_item->visible = 1;
		/*menu_item->toggle_type = SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_NONE;*/
		menu_item->toggle_state = -1;
		/*menu_item->disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_NORNAL;*/

		sd_bus_message_enter_container(msg, 'v', "(ia{sv}av)");
		sd_bus_message_enter_container(msg, 'r', "ia{sv}av");

		sd_bus_message_read_basic(msg, 'i', &menu_item->id);

		sd_bus_message_enter_container(msg, 'a', "{sv}");
		while (sd_bus_message_enter_container(msg, 'e', "sv") == 1) {
			string_t s;
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) == 0) {
				goto exit_con;
			}
			s.free_contents = FALSE;
			s.nul_terminated = TRUE;
			sd_bus_message_enter_container(msg, 'v', NULL);
			if (string_equal(s, string("type"))) {
				sd_bus_message_read_basic(msg, 's', &s.s);
				s.len = STRLEN(s.s);
				if (string_equal(s, string("separator"))) {
					menu_item->type = SNI_DBUSMENU_MENU_ITEM_TYPE_SEPARATOR;
				}
			} else if (string_equal(s, string("label"))) {
				size_t l = 0;
				char *c;
				char *label;
				sd_bus_message_read_basic(msg, 's', &label);
				ALLOCTS(menu_item->label.s, alloc, STRLEN(label) + 1);
				for ( c = label; *c; ++c) {
					if ((*c == '_') && (!*++c)) {
						break;
					}
					menu_item->label.s[l++] = *c;
				}
				menu_item->label.s[l] = '\0';
				menu_item->label.len = l;
				menu_item->label.free_contents = TRUE;
				menu_item->label.nul_terminated = TRUE;
				/* TODO: handle '_', '__' properly */
			} else if (string_equal(s, string("enabled"))) {
				sd_bus_message_read_basic(msg, 'b', &menu_item->enabled);
			} else if (string_equal(s, string("visible"))) {
				sd_bus_message_read_basic(msg, 'b', &menu_item->visible);
			} else if (string_equal(s, string("icon-name"))) {
				sd_bus_message_read_basic(msg, 's', &s.s);
				if ((s.len = STRLEN(s.s)) > 0) {
					string_init_string(&menu_item->icon_name, alloc, s);
				}
			} else if (string_equal(s, string("icon-data"))) {
				const void *bytes;
				sd_bus_message_read_array(msg, 'y', &bytes, &menu_item->icon_data.len);
				ALLOCTSA(menu_item->icon_data.ptr, alloc, menu_item->icon_data.len, 32);
				MEMCPY(menu_item->icon_data.ptr, bytes, menu_item->icon_data.len);
			/*} else if (string_equal(key_str, string("shortcut"))) { */
			} else if (string_equal(s, string("toggle-type"))) {
				sd_bus_message_read_basic(msg, 's', &s.s);
				if ((s.len = STRLEN(s.s)) > 0) {
					if (string_equal(s, string("checkmark"))) {
						menu_item->toggle_type = SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_CHECKMARK;
					} else if (string_equal(s, string("radio"))) {
						menu_item->toggle_type = SNI_DBUSMENU_MENU_ITEM_TOGGLE_TYPE_RADIO;
					}
				}
			} else if (string_equal(s, string("toggle-state"))) {
				sd_bus_message_read_basic(msg, 'i', &menu_item->toggle_state);
			} else if (string_equal(s, string("children-display"))) {
				sd_bus_message_read_basic(msg, 's', &s.s);
				s.len = STRLEN(s.s);
				if (string_equal(s, string("submenu"))) {
					children = TRUE;
				}
			} else if (string_equal(s, string("disposition"))) {
				sd_bus_message_read_basic(msg, 's', &s.s);
				if ((s.len = STRLEN(s.s)) > 0) {
					if (string_equal(s, string("normal"))) {
						menu_item->disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_NORNAL;
					} else if (string_equal(s, string("informative"))) {
						menu_item->disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_INFORMATIVE;
					} else if (string_equal(s, string("warning"))) {
						menu_item->disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_WARNING;
					} else if (string_equal(s, string("alert"))) {
						menu_item->disposition = SNI_DBUSMENU_MENU_ITEM_DISPOSITION_ALERT;
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

		sd_bus_message_enter_container(msg, 'a', "v");
		if (children) { /* && (menu_item->id != 0)) { */
			menu_item->submenu = sni__dbusmenu_menu_create(msg, dbusmenu, menu_item);
		}

		sd_bus_message_exit_container(msg);
		sd_bus_message_exit_container(msg);
		sd_bus_message_exit_container(msg);
	}

	return menu;
}

static sni_server_event_t *sni__server_event(sni_server_event_type_t type, sni_item_t *item) {
	sni_server_event_t *event;

	const allocator_t *alloc = sni_server.in.alloc;

	if (SU_UNLIKELY(sni_server.priv.events_capacity == sni_server.out.events_count)) {
		sni_server_event_t *new_events;
		sni_server.priv.events_capacity *= 2;
		SU_ARRAY_ALLOC(new_events, alloc, sni_server.priv.events_capacity);
		SU_MEMCPY(new_events, sni_server.out.events, sni_server.out.events_count * sizeof(sni_server.out.events[0]));
		SU_FREE(alloc, sni_server.out.events);
		sni_server.out.events = new_events;
	}

	event = &sni_server.out.events[sni_server.out.events_count++];
	/*SU_MEMSET(event, 0, sizeof(*event));*/
	event->type = type;
	event->item = item;

	return event;
}

static int sni__dbusmenu_handle_get_layout(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	sni__slot_t *slot = (sni__slot_t *)data;
	sni_item_t *item = slot->item;
	sni_dbusmenu_t *dbusmenu = item->out.dbusmenu;
	sni_server_event_t *event;
	int ret;

	NOTUSED(ret_error);

	sni__slot_free(slot);
	event = sni__server_event(SNI_SERVER_EVENT_TYPE_ITEM_DBUSMENU_MENU_UPDATED, item);
	event->priv.menu = dbusmenu->menu;
	sni_server.priv.check_events = TRUE;

	ret = sd_bus_message_skip(msg, "u");
	if (ret < 0) {
		dbusmenu->menu = NULL;
	} else {
		dbusmenu->menu = sni__dbusmenu_menu_create(msg, dbusmenu, NULL);
		ret = 1;
	}

	return ret;
}

static int sni__dbusmenu_get_layout(sni_dbusmenu_t *dbusmenu) {
	int ret;
	const allocator_t *alloc = sni_server.in.alloc;
	sni_item_t *item = dbusmenu->item;
	sni__slot_t *slot;
	ALLOCT(slot, alloc);
	ret = sd_bus_call_method_async(sni_server.priv.bus, &slot->slot, item->priv.service.s,
			item->out.properties->menu.s, sni__dbusmenu_interface, "GetLayout",
			sni__dbusmenu_handle_get_layout, slot, "iias", 0, -1, NULL);
	if (ret < 0) {
		FREE(alloc, slot);
		return ret;
	}

	slot->item = item;
	LLIST_APPEND_HEAD(&item->priv.slots, slot);

	return 1;
}

static int sni__dbusmenu_handle_signal(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	/* ? TODO: error check */
	sni__slot_t *slot = (sni__slot_t *)data;

	NOTUSED(msg); NOTUSED(ret_error);

	/* TODO: ItemActivationRequested */

	sni__dbusmenu_get_layout(slot->item->out.dbusmenu);
	return 1;
}

static int sni__dbusmenu_handle_get_properties(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	sni__slot_t *slot = (sni__slot_t *)data;
	const allocator_t *alloc;
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

	ALLOCCT(props, alloc);

	while (sd_bus_message_enter_container(msg, 'e', "sv") == 1) {
		string_t s;
		sd_bus_message_read_basic(msg, 's', &s.s);
		if ((s.len = STRLEN(s.s)) == 0) {
			goto exit_con;
		}
		s.free_contents = FALSE;
		s.nul_terminated = TRUE;
		sd_bus_message_enter_container(msg, 'v', NULL);
		if (string_equal(s, string("IconThemePath"))) {
			char **icon_theme_path;
			sd_bus_message_read_strv(msg, &icon_theme_path);
			if (icon_theme_path) {
				size_t count = 0;
				char **p = icon_theme_path;
				while (*p != NULL) {
					count++; ++p;
				}
				ARRAY_ALLOC(props->icon_theme_path, alloc, count);
				for ( p = icon_theme_path; *p != NULL; ++p) {
					char *path = *p;
					size_t len;
					if ((len = STRLEN(path)) > 0) {
						string_init_len(&props->icon_theme_path[props->icon_theme_path_count++],
							alloc, path, len, TRUE);
					}
					free(path);
				}
				free(icon_theme_path);
			}
		} else if (string_equal(s, string("Status"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
				if (string_equal(s, string("normal"))) {
					props->status = SNI_DBUSMENU_STATUS_NORMAL;
				} else if (string_equal(s, string("notice"))) {
					props->status = SNI_DBUSMENU_STATUS_NOTICE;
				}
			}
		} else if (string_equal(s, string("TextDirection"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
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
	const allocator_t *alloc;
	int ret;

	if (item->out.properties->menu.len == 0) {
		return NULL;
	}

	alloc = sni_server.in.alloc;

	ALLOCT(slot1, alloc);
	ret = sd_bus_call_method_async(sni_server.priv.bus, &slot1->slot, item->priv.service.s,
			item->out.properties->menu.s, "org.freedesktop.DBus.Properties", "GetAll",
			sni__dbusmenu_handle_get_properties, slot1, "s", sni__dbusmenu_interface);
	if (ret < 0) {
		goto error_1;
	}

	ALLOCT(slot2, alloc);
	ret = sd_bus_match_signal_async(sni_server.priv.bus, &slot2->slot, item->priv.service.s,
			item->out.properties->menu.s, sni__dbusmenu_interface, NULL,
			sni__dbusmenu_handle_signal, sni__dbusmenu_handle_signal, slot2);
	if (ret < 0) {
		goto error_2;
	}

	ALLOCT(dbusmenu, alloc);
	dbusmenu->item = item;
	dbusmenu->properties = NULL;
	dbusmenu->menu = NULL;

	slot1->item = item;
	LLIST_APPEND_HEAD(&item->priv.slots, slot1);
	slot2->item = item;
	LLIST_APPEND_HEAD(&item->priv.slots, slot2);

	return dbusmenu;
error_2:
	sd_bus_slot_unref(slot1->slot);
	FREE(alloc, slot2);
error_1:
	FREE(alloc, slot1);
	return NULL;
}

static int sni__item_handle_get_properties(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	sni__slot_t *slot = (sni__slot_t *)data;
	sni_item_t *item = slot->item;
	sni_item_properties_t *props;
	const allocator_t *alloc = sni_server.in.alloc;
	int ret;

	NOTUSED(ret_error);

	sni__slot_free(slot);
	sni__item_properties_destroy(item->out.properties);

	ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
	if (ret < 0) {
		item->out.properties = NULL;
		goto out;
	} else {
		ALLOCCT(item->out.properties, alloc);
		ret = 1;
	}

	props = item->out.properties;
	while (sd_bus_message_enter_container(msg, 'e', "sv") == 1) {
		string_t s;
		sd_bus_message_read_basic(msg, 's', &s.s);
		if ((s.len = STRLEN(s.s)) == 0) {
			goto exit_con;
		}
		s.free_contents = FALSE;
		s.nul_terminated = TRUE;
		sd_bus_message_enter_container(msg, 'v', NULL);
		if (string_equal(s, string("IconName"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
				string_init_string(&props->icon_name, alloc, s);
			}
		} else if (string_equal(s, string("IconThemePath"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
				string_init_string(&props->icon_theme_path, alloc, s);
			}
		} else if (string_equal(s, string("IconPixmap"))) {
			sni__item_read_pixmap(msg, props->icon_pixmaps,
				&props->icon_pixmaps_count, LENGTH(props->icon_pixmaps));
		} else if (string_equal(s, string("Status"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
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
			if ((s.len = STRLEN(s.s)) > 0) {
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
			if ((s.len = STRLEN(s.s)) > 0) {
				string_init_string(&props->menu, alloc, s);
			}
		} else if (string_equal(s, string("AttentionIconName"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
				string_init_string(&props->attention_icon_name, alloc, s);
			}
		} else if (string_equal(s, string("AttentionIconPixmap"))) {
			sni__item_read_pixmap(msg, props->attention_icon_pixmaps,
				&props->attention_icon_pixmaps_count, LENGTH(props->attention_icon_pixmaps));
		} else if (string_equal(s, string("ItemIsMenu"))) {
			sd_bus_message_read_basic(msg, 'b', &props->item_is_menu);
		} else if (string_equal(s, string("WindowId"))) {
			sd_bus_message_read_basic(msg, 'i', &props->window_id);
		} else if (string_equal(s, string("Id"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
				string_init_string(&props->id, alloc, s);
			}
		} else if (string_equal(s, string("Title"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
				string_init_string(&props->title, alloc, s);
			}
		} else if (string_equal(s, string("AttentionMovieName"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
				string_init_string(&props->attention_movie_name, alloc, s);
			}
		} else if (string_equal(s, string("OverlayIconName"))) {
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
				string_init_string(&props->overlay_icon_name, alloc, s);
			}
		} else if (string_equal(s, string("OverlayIconPixmap"))) {
			sni__item_read_pixmap(msg, props->overlay_icon_pixmaps,
				&props->overlay_icon_pixmaps_count, LENGTH(props->overlay_icon_pixmaps));
		} else if (string_equal(s, string("ToolTip"))) {
			sd_bus_message_enter_container(msg, 'r', "sa(iiay)ss");
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
				string_init_string(&props->tooltip.icon_name, alloc, s);
			}
			sni__item_read_pixmap(msg, props->tooltip.icon_pixmaps,
				&props->tooltip.icon_pixmaps_count, LENGTH(props->tooltip.icon_pixmaps));
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
				string_init_string(&props->tooltip.title, alloc, s);
			}
			sd_bus_message_read_basic(msg, 's', &s.s);
			if ((s.len = STRLEN(s.s)) > 0) {
				string_init_string(&props->tooltip.text, alloc, s);
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
	sni__server_event(SNI_SERVER_EVENT_TYPE_ITEM_PROPERTIES_UPDATED, item);

	return ret;
}

static int sni__item_handle_signal(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	/* ? TODO: error check */
	sni_item_t *item = (sni_item_t *)data;
	const allocator_t *alloc = sni_server.in.alloc;
	sni__slot_t *slot;
	int ret;

	NOTUSED(msg); NOTUSED(ret_error);

	ALLOCT(slot, alloc);

	ret = sd_bus_call_method_async(sni_server.priv.bus, &slot->slot, item->priv.service.s,
			item->priv.path.s, "org.freedesktop.DBus.Properties", "GetAll",
			sni__item_handle_get_properties, slot, "s", sni__item_interface);
	if (ret >= 0) {
		slot->item = item;
		LLIST_APPEND_HEAD(&item->priv.slots, slot);
	} else {
		FREE(alloc, slot);
	}

	return 1;
}

static void sni__item_fini(sni_item_t *item) {
	const allocator_t *alloc = sni_server.in.alloc;
	sni_dbusmenu_t *dbusmenu = item->out.dbusmenu;
	sni_item_properties_t *properties = item->out.properties;

	sni__slot_t *slot = item->priv.slots.head;
	for ( ; slot; ) {
		sni__slot_t *next = slot->next;
        sd_bus_slot_unref(slot->slot);
        FREE(alloc, slot);
        slot = next;
	}

    string_fini(&item->priv.watcher_id, alloc);
	string_fini(&item->priv.service, alloc);
	string_fini(&item->priv.path, alloc);

	sni__dbusmenu_destroy(dbusmenu);

	sni__item_properties_destroy(properties);

	/*MEMSET(item, 0, sizeof(*item));*/

	sni__server_event(SNI_SERVER_EVENT_TYPE_ITEM_DESTROY, item);
}

static bool32_t sni__item_init(sni_item_t *item, string_t id) {
	sni__slot_t *slot;
	sd_bus_slot *slot_;
	string_t path;
	const allocator_t *alloc = sni_server.in.alloc;
	int ret;

	ASSERT(id.len > 0);

	if (!string_find_char(id, '/', &path)) {
		return FALSE;
	}

	MEMSET(item, 0, sizeof(*item));

	string_init_len(&item->priv.service, alloc, id.s, (id.len - path.len), TRUE);

	ret = sd_bus_match_signal_async(sni_server.priv.bus, &slot_, item->priv.service.s,
			path.s, sni__item_interface, NULL,
			sni__item_handle_signal, sni__item_handle_signal, item);
	if (ret < 0) {
		sni__item_fini(item);
		return FALSE;
	}

	ALLOCT(slot, alloc);
	slot->item = item;
	slot->slot = slot_;
	LLIST_APPEND_HEAD(&item->priv.slots, slot);

	item->priv.watcher_id = id;
	item->priv.path = path;

	return TRUE;
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

	/*if (STRLEN(service_or_path) == 0) { */
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
    for ( i = 0; i < sni_server.priv.watcher_items_count; ++i) {
        if (string_equal(id, sni_server.priv.watcher_items[i])) {
            string_fini(&id, sni_server.in.alloc);
            return -EEXIST;
        }
    }

    ret = sd_bus_emit_signal(sni_server.priv.bus, sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierItemRegistered", "s", id.s);
    if (ret < 0) {
        string_fini(&id, sni_server.in.alloc);
        return ret;
    }

    if (UNLIKELY(sni_server.priv.watcher_items_capacity == sni_server.priv.watcher_items_count)) {
		string_t *new_watcher_items;
		sni_server.priv.watcher_items_capacity *= 2;
		ARRAY_ALLOC(new_watcher_items, sni_server.in.alloc, sni_server.priv.watcher_items_capacity);
		MEMCPY(new_watcher_items, sni_server.priv.watcher_items,
			sizeof(new_watcher_items[0]) * sni_server.priv.watcher_items_count);
		FREE(sni_server.in.alloc, sni_server.priv.watcher_items);
		sni_server.priv.watcher_items = new_watcher_items;
    }
    sni_server.priv.watcher_items[sni_server.priv.watcher_items_count++] = id;

	/*ret = sd_bus_emit_properties_changed(sni_server.priv.bus, */
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

	s = string(s.s);

    for ( i = 0; i < sni_server.priv.watcher_hosts_count; ++i) {
        if (string_equal(s, sni_server.priv.watcher_hosts[i])) {
            return -EEXIST;
        }
    }

	/*if (sni_server.priv.watcher_hosts.len == 0) { */
	/*	ret = sd_bus_emit_properties_changed(sni_server.priv.bus, */
	/*		sni__watcher_obj_path, sni__watcher_interface, */
	/*		"IsStatusNotifierHostRegistered", NULL); */
	/*	if (ret < 0) { */
	/*		return ret; */
	/*	} */
	/*} */

    ret = sd_bus_emit_signal(sni_server.priv.bus, sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierHostRegistered", "");
    if (ret < 0) {
        return ret;
    }

	string_init_string(&s, sni_server.in.alloc, s);

    if (UNLIKELY(sni_server.priv.watcher_hosts_capacity == sni_server.priv.watcher_hosts_count)) {
		string_t *new_watcher_hosts;
		sni_server.priv.watcher_hosts_capacity *= 2;
		ARRAY_ALLOC(new_watcher_hosts, sni_server.in.alloc, sni_server.priv.watcher_hosts_capacity);
		MEMCPY(new_watcher_hosts, sni_server.priv.watcher_hosts,
			sizeof(new_watcher_hosts[0]) * sni_server.priv.watcher_hosts_count);
		FREE(sni_server.in.alloc, sni_server.priv.watcher_hosts);
		sni_server.priv.watcher_hosts = new_watcher_hosts;
    }
    sni_server.priv.watcher_hosts[sni_server.priv.watcher_hosts_count++] = s;

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
	const allocator_t *alloc = sni_server.in.alloc;
	char **array;
	size_t i = 0;
	int ret;

	NOTUSED(b); NOTUSED(path); NOTUSED(iface); NOTUSED(prop); NOTUSED(data); NOTUSED(ret_error);

	ARRAY_ALLOC(array, alloc, (sni_server.priv.watcher_items_count + 1));

	for ( ; i < sni_server.priv.watcher_items_count; ++i) {
		string_t s = sni_server.priv.watcher_items[i];
		ASSERT(s.nul_terminated);
		array[i] = s.s;
	}
	array[sni_server.priv.watcher_items_count] = NULL;
	ret = sd_bus_message_append_strv(reply, array);
	FREE(alloc, array);
    if (ret < 0) {
		return ret;
    }

    return 1;
}

static int sni__watcher_handle_is_host_registered(sd_bus *b, const char *path,
		const char *iface, const char *prop, sd_bus_message *reply,
		void *data, sd_bus_error *ret_error) {
	int registered = (sni_server.priv.watcher_hosts_count > 0);
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

	s = string(s.s);

    for ( i = 0; i < sni_server.priv.watcher_items_count; ++i) {
        string_t item = sni_server.priv.watcher_items[i];
        if (string_compare(item, s, s.len) == 0) {
            ret = sd_bus_emit_signal(sni_server.priv.bus, sni__watcher_obj_path, sni__watcher_interface,
					"StatusNotifierItemUnregistered", "s", item.s);
            if (ret < 0) {
                return ret;
            }
            string_fini(&item, sni_server.in.alloc);
			sni_server.priv.watcher_items[i] = sni_server.priv.watcher_items[sni_server.priv.watcher_items_count-- - 1];
			/*ret = sd_bus_emit_properties_changed(sni_server.priv.bus,*/
			/*	sni__watcher_obj_path, sni__watcher_interface,*/
			/*	"RegisteredStatusNotifierItems", NULL);*/
			/*if (ret < 0) {*/
			/*	return ret;*/
			/*}*/
            return 0;
        }
    }

    for ( i = 0; i < sni_server.priv.watcher_hosts_count; ++i) {
        string_t host = sni_server.priv.watcher_hosts[i];
        if (string_equal(s, host)) {
			/*if (sni_server.priv.watcher_hosts_count == 1) { */
			/*	ret = sd_bus_emit_properties_changed(sni_server.priv.bus, */
			/*		sni__watcher_obj_path, sni__watcher_interface, */
			/*		"IsStatusNotifierHostRegistered", NULL); */
			/*	if (ret < 0) { */
			/*		return ret; */
			/*	} */
			/*} */
            ret = sd_bus_emit_signal(sni_server.priv.bus, sni__watcher_obj_path, sni__watcher_interface,
					"StatusNotifierHostUnregistered", "");
			if (ret < 0) {
                return ret;
            }
            string_fini(&host, sni_server.in.alloc);
			sni_server.priv.watcher_hosts[i] = sni_server.priv.watcher_hosts[sni_server.priv.watcher_hosts_count-- - 1];
            return 0;
        }
    }

	return 0;
}

static int sni__host_add_item(string_t id) {
	sni_server_event_t *event;

	sni_item_t *item = sni_server.out.items.head;
	for ( ; item; item = item->next) {
		if (string_equal(id, item->priv.watcher_id)) {
			return -EEXIST;
		}
	}

	event = sni__server_event(SNI_SERVER_EVENT_TYPE_ITEM_CREATE, NULL);
	string_init_string(&event->priv.id, sni_server.in.alloc, id);

	sni_server.priv.check_events = TRUE;

	return 1;
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
			str.len = STRLEN(str.s);
			str.free_contents = TRUE;
			str.nul_terminated = TRUE;
			sni__host_add_item(str);
			free(id);
		}
		free(ids);
	}

	return 1;
}

static int sni__host_register_to_watcher(void) {
	/* ? TODO: slots */
	int ret = sd_bus_call_method_async(sni_server.priv.bus, NULL,
			sni__watcher_interface, sni__watcher_obj_path, sni__watcher_interface,
			"RegisterStatusNotifierHost", NULL, NULL,
			"s", sni_server.priv.host_interface.s);
	if (ret < 0) {
		return ret;
	}

	ret = sd_bus_call_method_async(sni_server.priv.bus, NULL,
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

	s = string(s.s);

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

	s = string(s.s);

	if (s.len > 0) {
		sni_item_t *item = sni_server.out.items.head;
		for ( ; item; item = item->next) {
			if (string_equal(item->priv.watcher_id, s)) {
				LLIST_POP(&sni_server.out.items, item);
				sni__item_fini(item);
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

	if (STRCMP(service, sni__watcher_interface) == 0) {
		sni_item_t *item;
        ret = sni__host_register_to_watcher();
		if (ret < 0) {
			return ret;
		}
		for ( item = sni_server.out.items.tail; item; item = item->prev) {
			LLIST_POP(&sni_server.out.items, item);
			sni__item_fini(item);
		}
	}

	return 0;
}

static void sni__server_process_events(void) {
	if (sni_server.priv.check_events) {
		size_t i = 0;
		for ( ; i < sni_server.out.events_count; ++i) {
			sni_server_event_t event = sni_server.out.events[i];
			switch (event.type) {
			case SNI_SERVER_EVENT_TYPE_ITEM_CREATE: {
				if (event.item) {
					if (sni__item_init(event.item, event.priv.id)) {
						LLIST_APPEND_TAIL(&sni_server.out.items, event.item);
					} else {
						string_fini(&event.priv.id, sni_server.in.alloc);
					}
				}
				break;
			}
			case SNI_SERVER_EVENT_TYPE_ITEM_DBUSMENU_MENU_UPDATED:
				sni__dbusmenu_menu_destroy(event.priv.menu);
				break;
			case SNI_SERVER_EVENT_TYPE_ITEM_DESTROY:
			case SNI_SERVER_EVENT_TYPE_ITEM_PROPERTIES_UPDATED:
				break;
			default:
				ASSERT_UNREACHABLE;
			}
		}
		sni_server.priv.check_events = FALSE;
	}
	sni_server.out.events_count = 0;
}

static void sni_server_fini(void) {
	const allocator_t *alloc = sni_server.in.alloc;
	size_t i;
	sni_server_event_t *events;
	size_t events_count;
	sni_item_t *item;

	sni__server_process_events();

	for ( item = sni_server.out.items.head; item; item = item->next) {
		sni__item_fini(item);
	}

	sd_bus_flush_close_unref(sni_server.priv.bus);

	string_fini(&sni_server.priv.host_interface, alloc);
	for ( i = 0; i < sni_server.priv.watcher_items_count; ++i) {
		string_fini(&sni_server.priv.watcher_items[i], alloc);
	}
	for ( i = 0; i < sni_server.priv.watcher_hosts_count; ++i) {
		string_fini(&sni_server.priv.watcher_hosts[i], alloc);
	}
	FREE(alloc, sni_server.priv.watcher_items);
	FREE(alloc, sni_server.priv.watcher_hosts);

	events = sni_server.out.events;
	events_count = sni_server.out.events_count;

	MEMSET(&sni_server.out, 0, sizeof(sni_server.out));
	MEMSET(&sni_server.priv, 0, sizeof(sni_server.priv));

	sni_server.out.events = events;
	sni_server.out.events_count = events_count;
}

static int sni_server_init(void) {
	int ret;
	const allocator_t *alloc = sni_server.in.alloc;
	static sd_bus_vtable watcher_vtable[11];

	ASSERT(sni_server.in.alloc != NULL);

	MEMSET(&sni_server.out, 0, sizeof(sni_server.out));
	MEMSET(&sni_server.priv, 0, sizeof(sni_server.priv));

	ret = sd_bus_open_user(&sni_server.priv.bus);
    if (ret < 0) {
		return ret;
    }

	sni_server.priv.events_capacity = 32;
	ARRAY_ALLOC(sni_server.out.events, alloc, sni_server.priv.events_capacity);

	ret = sd_bus_request_name(sni_server.priv.bus, sni__watcher_interface, SD_BUS_NAME_QUEUE);
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

    ret = sd_bus_add_object_vtable(sni_server.priv.bus, NULL, sni__watcher_obj_path,
            sni__watcher_interface, watcher_vtable,
			&sni__watcher_protocol_version);
    if (ret < 0) {
        return ret;
    }

    ret = sd_bus_match_signal(sni_server.priv.bus, NULL, "org.freedesktop.DBus",
			"/org/freedesktop/DBus", "org.freedesktop.DBus",
			"NameOwnerChanged", sni__watcher_handle_lost_service, NULL);
    if (ret < 0) {
        return ret;
    }

	sni_server.priv.watcher_items_capacity = 16;
    ARRAY_ALLOC(sni_server.priv.watcher_items, alloc, sni_server.priv.watcher_items_capacity);
	sni_server.priv.watcher_hosts_capacity = 8;
    ARRAY_ALLOC(sni_server.priv.watcher_hosts, alloc, sni_server.priv.watcher_hosts_capacity);


	string_init_format(&sni_server.priv.host_interface, alloc,
		"org.kde.StatusNotifierHost-%d", getpid());
    ret = sd_bus_request_name(sni_server.priv.bus, sni_server.priv.host_interface.s, 0); /* ? TODO: SD_BUS_NAME_QUEUE */
    if (ret < 0) {
        return ret;
    }

    ret = sni__host_register_to_watcher();
    if (ret < 0) {
        return ret;
    }

    ret = sd_bus_match_signal(sni_server.priv.bus, NULL, sni__watcher_interface,
			sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierItemRegistered", sni__host_handle_item_registered, NULL);
	if (ret < 0) {
		return ret;
	}
	ret = sd_bus_match_signal(sni_server.priv.bus, NULL, sni__watcher_interface,
			sni__watcher_obj_path, sni__watcher_interface,
			"StatusNotifierItemUnregistered", sni__host_handle_item_unregistered, NULL);
	if (ret < 0) {
		return ret;
	}

	ret = sd_bus_match_signal(sni_server.priv.bus, NULL, "org.freedesktop.DBus",
			"/org/freedesktop/DBus", "org.freedesktop.DBus", "NameOwnerChanged",
			sni__host_handle_new_watcher, NULL);
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_server_get_poll_info(struct pollfd *pollfd_out, int64_t *absolute_timeout_ms) {
	uint64_t usec;
	int fd, events, ret;

	fd = sd_bus_get_fd(sni_server.priv.bus);
	if (fd < 0) {
		return fd;
	}

	events = sd_bus_get_events(sni_server.priv.bus);
	if (events < 0) {
		return events;
	}

	ret = sd_bus_get_timeout(sni_server.priv.bus, &usec);
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

	sni__server_process_events();

    while ((ret = sd_bus_process(sni_server.priv.bus, NULL)) > 0)
	{
	}
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni__item_handle_method(sd_bus_message *msg, void *data,
        sd_bus_error *ret_error) {
	sni__slot_t *slot = (sni__slot_t *)data;
	NOTUSED(msg); NOTUSED(ret_error);
	sni__slot_free(slot);
	return 1;
}

static int sni_item_context_menu(sni_item_t *item, int x, int y) {
	int ret = sd_bus_call_method(sni_server.priv.bus, item->priv.service.s,
		item->priv.path.s, sni__item_interface, "ContextMenu",
		NULL, NULL, "ii", x, y);
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_context_menu_async(sni_item_t *item, int x, int y) {
	int ret;
	const allocator_t *alloc = sni_server.in.alloc;
	sni__slot_t *slot;
	ALLOCT(slot, alloc);
	ret = sd_bus_call_method_async(sni_server.priv.bus, &slot->slot, item->priv.service.s,
			item->priv.path.s, sni__item_interface, "ContextMenu",
			sni__item_handle_method, slot, "ii", x, y);
	if (ret < 0) {
		FREE(alloc, slot);
		return ret;
	} else {
		slot->item = item;
	}

	LLIST_APPEND_HEAD(&item->priv.slots, slot);

	return 1;
}

static int sni_item_activate(sni_item_t *item, int x, int y) {
	int ret = sd_bus_call_method(sni_server.priv.bus, item->priv.service.s,
		item->priv.path.s, sni__item_interface, "Activate",
		NULL, NULL, "ii", x, y);
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_activate_async(sni_item_t *item, int x, int y) {
	int ret;
	const allocator_t *alloc = sni_server.in.alloc;
	sni__slot_t *slot;
	ALLOCT(slot, alloc);
	ret = sd_bus_call_method_async(sni_server.priv.bus, &slot->slot, item->priv.service.s,
			item->priv.path.s, sni__item_interface, "Activate",
			sni__item_handle_method, slot, "ii", x, y);
	if (ret < 0) {
		FREE(alloc, slot);
		return ret;
	} else {
		slot->item = item;
	}

	LLIST_APPEND_HEAD(&item->priv.slots, slot);

	return 1;
}

static int sni_item_secondary_activate(sni_item_t *item, int x, int y) {
	int ret = sd_bus_call_method(sni_server.priv.bus, item->priv.service.s,
		item->priv.path.s, sni__item_interface, "SecondaryActivate",
		NULL, NULL, "ii", x, y);
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_secondary_activate_async(sni_item_t *item, int x, int y) {
	int ret;
	const allocator_t *alloc = sni_server.in.alloc;
	sni__slot_t *slot;
	ALLOCT(slot, alloc);
	ret = sd_bus_call_method_async(sni_server.priv.bus, &slot->slot, item->priv.service.s,
			item->priv.path.s, sni__item_interface, "SecondaryActivate",
			sni__item_handle_method, slot, "ii", x, y);
	if (ret < 0) {
		FREE(alloc, slot);
		return ret;
	} else {
		slot->item = item;
	}

	LLIST_APPEND_HEAD(&item->priv.slots, slot);

	return 1;
}

static int sni_item_scroll(sni_item_t *item, int delta, sni_item_scroll_orientation_t orientation) {
	int ret = sd_bus_call_method(sni_server.priv.bus, item->priv.service.s,
		item->priv.path.s, sni__item_interface, "Scroll", NULL, NULL,
		"is", delta, (orientation == SNI_ITEM_SCROLL_ORIENTATION_VERTICAL) ? "vertical" : "horizontal");
	if (ret < 0) {
		return ret;
	}

	return 1;
}

static int sni_item_scroll_async(sni_item_t *item, int delta, sni_item_scroll_orientation_t orientation) {
	int ret;
	const allocator_t *alloc = sni_server.in.alloc;
	sni__slot_t *slot;
	ALLOCT(slot, alloc);
	ret = sd_bus_call_method_async(sni_server.priv.bus, &slot->slot, item->priv.service.s,
			item->priv.path.s, sni__item_interface, "Scroll", sni__item_handle_method, slot,
			"is", delta, (orientation == SNI_ITEM_SCROLL_ORIENTATION_VERTICAL) ? "vertical" : "horizontal");
	if (ret < 0) {
		FREE(alloc, slot);
		return ret;
	} else {
		slot->item = item;
	}

	LLIST_APPEND_HEAD(&item->priv.slots, slot);

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
		int ret;
		const allocator_t *alloc = sni_server.in.alloc;
		sni__slot_t *slot;
		ALLOCT(slot, alloc);
		ret = sd_bus_call_method_async(sni_server.priv.bus, &slot->slot, item->priv.service.s,
				item->out.properties->menu.s, sni__dbusmenu_interface, "Event",
				sni__item_handle_method, slot, "isvu",
				menu_item->id, event_types[type], "y", 0, time(NULL));
		if (ret < 0) {
			FREE(alloc, slot);
			return ret;
		} else {
			slot->item = item;
			LLIST_APPEND_HEAD(&item->priv.slots, slot);
		}
	} else {
		int ret = sd_bus_call_method(sni_server.priv.bus, item->priv.service.s,
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
	sni__slot_t *slot = (sni__slot_t *)data;
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
		int ret;
		const allocator_t *alloc = sni_server.in.alloc;
		sni__slot_t *slot;
		ALLOCT(slot, alloc);
		ret = sd_bus_call_method_async(sni_server.priv.bus, &slot->slot, item->priv.service.s,
				item->out.properties->menu.s, sni__dbusmenu_interface, "AboutToShow",
				sni__dbusmenu_menu_handle_about_to_show, slot, "i", menu->parent_menu_item->id);
		if (ret < 0) {
			FREE(alloc, slot);
			return ret;
		} else {
			slot->item = item;
			LLIST_APPEND_HEAD(&item->priv.slots, slot);
		}
	} else {
		int ret = sd_bus_call_method(sni_server.priv.bus, item->priv.service.s,
				item->out.properties->menu.s, sni__dbusmenu_interface, "AboutToShow",
				NULL, NULL, "i", menu->parent_menu_item->id);
		if (ret < 0) {
			return ret;
		}
	}

	return 1;
}

#endif /* SNI_SERVER_H */
