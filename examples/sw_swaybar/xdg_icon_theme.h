#if !defined(XDG_ICON_THEME_H)
#define XDG_ICON_THEME_H

#if !defined(WITH_SVG)
#define WITH_SVG 1
#endif // !defined(WITH_SVG)
#if !defined(WITH_PNG)
#define WITH_PNG 1
#endif // !defined(WITH_PNG)

#if !defined(DEBUG)
#define DEBUG 0
#endif // !defined(DEBUG)

// TODO: #define _DEFAULT_SOURCE

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>

#include <stddef.h>

#if !defined(SU_IMPLEMENTATION)
#define SU_IMPLEMENTATION
#endif // !defined(SU_IMPLEMENTATION)
#if !defined(SU_STRIP_PREFIXES)
#define SU_STRIP_PREFIXES
#endif // !defined(SU_STRIP_PREFIXES)
#include "su.h"

STATIC_ASSERT(WITH_SVG || WITH_PNG);

ARRAY_DECLARE_DEFINE(pthread_t)

enum xdg_icon_theme_icon_type {
	XDG_ICON_THEME_ICON_TYPE_XPM = 1, // deprecated, not implemented
#if WITH_PNG
	XDG_ICON_THEME_ICON_TYPE_PNG = 2,
#endif // WITH_PNG
#if WITH_SVG
	XDG_ICON_THEME_ICON_TYPE_SVG = 4,
#endif // WITH_SVG
};

struct xdg_icon_theme_icon {
	string_t path;
	enum xdg_icon_theme_icon_type type;
	PAD32;
};

struct xdg_icon_theme__icon_cache {
	HASH_TABLE_STRUCT_FIELDS(string_t)

#if WITH_SVG
	su_array__su_string_t__t svg_paths;
#endif // WITH_SVG
#if WITH_PNG
	su_array__su_string_t__t png_paths;
#endif // WITH_PNG
};

typedef struct xdg_icon_theme__icon_cache struct_xdg_icon_theme__icon_cache;
HASH_TABLE_DECLARE_DEFINE(struct_xdg_icon_theme__icon_cache, stbds_hash, string_equal, 16)

typedef struct xdg_icon_theme__theme_cache struct_xdg_icon_theme__theme_cache;

struct xdg_icon_theme__theme_cache {
	string_t name;
	su_hash_table__struct_xdg_icon_theme__icon_cache__t icons;
	pthread_mutex_t lock;
	arena_t arena;
	LLIST_STRUCT_FIELDS(struct_xdg_icon_theme__theme_cache);
};

LLIST_DECLARE_DEFINE(struct_xdg_icon_theme__theme_cache)

struct xdg_icon_theme_cache {
	su_llist__struct_xdg_icon_theme__theme_cache__t themes;
	su_hash_table__struct_xdg_icon_theme__icon_cache__t unthemed;
	su_array__su_string_t__t basedirs;
	su_array__pthread_t__t threads; // ? TODO: use scratch allocator
};

struct xdg_icon_theme__theme_cache_alloc {
	allocator_t _; // must be first
	allocator_t *user_alloc;
	struct xdg_icon_theme__theme_cache *theme;
	string_t path;
};


static void xdg_icon_theme_cache_init(struct xdg_icon_theme_cache *, allocator_t *);
static void xdg_icon_theme_cache_fini(struct xdg_icon_theme_cache *, allocator_t *);
static bool32_t xdg_icon_theme_cache_add_basedir(struct xdg_icon_theme_cache *,
	allocator_t *, string_t path);
static bool32_t xdg_icon_theme_cache_find_icon(struct xdg_icon_theme_cache *,
	struct xdg_icon_theme_icon *dest, string_t icon_name, string_t *theme_name);

static bool32_t xdg_icon_theme__icon_cache_add_icon(su_hash_table__struct_xdg_icon_theme__icon_cache__t *dest,
		allocator_t *alloc, pthread_mutex_t *lock, string_t path, string_t name) {
	uint32_t type = 0;
#if WITH_SVG
		if (string_ends_with(name, STRING(".svg"))) {
			type = XDG_ICON_THEME_ICON_TYPE_SVG;
		}
#endif // WITH_SVG
#if WITH_SVG && WITH_PNG
		else
#endif // WITH_SVG && WITH_PNG
#if WITH_PNG
		if (string_ends_with(name, STRING(".png"))) {
			type = XDG_ICON_THEME_ICON_TYPE_PNG;
		}
#endif // WITH_PNG
	if (!type) {
		return FALSE;
	}

	string_t xdg_name = {
		.s = name.s,
		.len = name.len - 4,
		.free_contents = FALSE,
		.nul_terminated = FALSE,
	};

	struct xdg_icon_theme__icon_cache *icon;

	if (lock) {
		pthread_mutex_lock(lock);
	}

	if (su_hash_table__struct_xdg_icon_theme__icon_cache__add(dest, alloc,
			(struct xdg_icon_theme__icon_cache){ .key = xdg_name }, &icon)) {
		string_init_string(&icon->key, alloc, xdg_name);
#if WITH_SVG
		su_array__su_string_t__init(&icon->svg_paths, alloc, 48);
#endif // WITH_SVG
#if WITH_PNG
		su_array__su_string_t__init(&icon->png_paths, alloc, 48);
#endif // WITH_PNG
	}

	string_t *p;
	switch (type) {
#if WITH_SVG
	case XDG_ICON_THEME_ICON_TYPE_SVG:
		p = su_array__su_string_t__add_uninitialized(&icon->svg_paths, alloc);
		break;
#endif // WITH_SVG
#if WITH_PNG
	case XDG_ICON_THEME_ICON_TYPE_PNG:
		p = su_array__su_string_t__add_uninitialized(&icon->png_paths, alloc);
		break;
#endif // WITH_PNG
	default:
		ASSERT_UNREACHABLE;
	}

	string_init_string(p, alloc, path);

	if (lock) {
		pthread_mutex_unlock(lock);
	}

	return TRUE;
}

static void xdg_icon_theme__theme_cache_populate(struct xdg_icon_theme__theme_cache *theme,
		allocator_t *alloc, string_t path) {
	ASSERT(path.nul_terminated);
	DIR *dir = opendir(path.s);
	if (!dir) {
		return;
	}

	int dir_fd = dirfd(dir);
	struct dirent *e;
	while ((e = readdir(dir))) {
		if ((e->d_name[0] == '.') && ((e->d_name[1] == '\0') ||
				((e->d_name[1] == '.') && (e->d_name[2] == '\0')))) {
			continue;
		}

		size_t d_name_len = strlen(e->d_name);
		if ((path.len + d_name_len + 2) > PATH_MAX) {
			continue;
		}

		char buf[PATH_MAX];
		memcpy(buf, path.s, path.len);
		buf[path.len] = '/';
		memcpy(&buf[path.len + 1], e->d_name, d_name_len + 1);
		string_t new_path = {
			.s = buf,
			.len = path.len + d_name_len + 1,
			.free_contents = FALSE,
			.nul_terminated = TRUE,
		};

		string_t name = {
			.s = e->d_name,
			.len = d_name_len,
			.free_contents = FALSE,
			.nul_terminated = TRUE,
		};

		switch (e->d_type) {
		case DT_REG:
			xdg_icon_theme__icon_cache_add_icon(&theme->icons, alloc, &theme->lock, new_path, name);
			break;
		case DT_DIR:
			xdg_icon_theme__theme_cache_populate(theme, alloc, new_path);
			break;
		case DT_LNK:
		case DT_UNKNOWN: {
			struct stat sb;
			if (fstatat(dir_fd, e->d_name, &sb, 0) == 0) {
				if (S_ISREG(sb.st_mode)) {
					xdg_icon_theme__icon_cache_add_icon( &theme->icons, alloc, &theme->lock, new_path, name);
				} else if (S_ISDIR(sb.st_mode)) {
					xdg_icon_theme__theme_cache_populate(theme, alloc, new_path);
				}
			}
			break;
		}
		default:
			break;
		}
	}

	closedir(dir);
}

static void *xdg_icon_theme__theme_cache_alloc_alloc(allocator_t *alloc, size_t size, size_t alignment) {
	struct xdg_icon_theme__theme_cache_alloc *a = (struct xdg_icon_theme__theme_cache_alloc *)alloc;
//#if DEBUG
//	size_t c = a->theme->arena.blocks.len;
//#endif // DEBUG
	void *ret = arena_alloc(&a->theme->arena, a->user_alloc, size, alignment);
//#if DEBUG
//	if (c < a->theme->arena.blocks.len) {
//		DEBUG_LOG("%s: new block size %zu", __func__,
//			su_array__su_arena_block_t__get(&a->theme->arena.blocks, a->theme->arena.blocks.len - 1).size);
//	}
//#endif // DEBUG
	return ret;
}

static void xdg_icon_theme__theme_cache_alloc_free(allocator_t *alloc, void *ptr) {
	NOTUSED(alloc); NOTUSED(ptr);
	//DEBUG_LOG("%s: %p (%zu)", __func__, ptr, arena_alloc_get_size(ptr));
}

static void *xdg_icon_theme__theme_cache_alloc_realloc(allocator_t *alloc,
		void *ptr, size_t new_size, size_t new_alignment) {
	struct xdg_icon_theme__theme_cache_alloc *a = (struct xdg_icon_theme__theme_cache_alloc *)alloc;
//#if DEBUG
//	size_t c = a->theme->arena.blocks.len;
//#endif // DEBUG
	void *ret = arena_alloc(&a->theme->arena, a->user_alloc, new_size, new_alignment);
//#if DEBUG
//	if (c < a->theme->arena.blocks.len) {
//		DEBUG_LOG("%s: new block size %zu", __func__,
//			su_array__su_arena_block_t__get(&a->theme->arena.blocks, a->theme->arena.blocks.len - 1).size);
//	}
//	//DEBUG_LOG("%s: %p -> %p (%zu -> %zu)", __func__, ptr, ret, ptr ? arena_alloc_get_size(ptr) : 0, new_size);
//#endif // DEBUG
	if (ptr) {
		memcpy(ret, ptr, MIN(new_size, arena_alloc_get_size(ptr)));
	}
	return ret;
}

static void *xdg_icon_theme__theme_cache_populate_thread(void *arg) {
	struct xdg_icon_theme__theme_cache_alloc *alloc = arg;
	xdg_icon_theme__theme_cache_populate(alloc->theme, &alloc->_, alloc->path);

	string_fini(&alloc->path, alloc->user_alloc);
	alloc->user_alloc->free(alloc->user_alloc, alloc);
	return NULL;
}

static bool32_t xdg_icon_theme__cache_add_theme(struct xdg_icon_theme_cache *cache,
		allocator_t *alloc, string_t path, string_t name) {
	struct xdg_icon_theme__theme_cache *theme = NULL;
	for (struct xdg_icon_theme__theme_cache *t = cache->themes.head; t; t = t->next) {
		if (string_equal(t->name, name)) {
			theme = t;
			break;
		}
	}

	// TODO: scratch allocator
	struct xdg_icon_theme__theme_cache_alloc *thread_data = alloc->alloc(
		alloc, sizeof(*thread_data), ALIGNOF(*thread_data));
	string_init_string(&thread_data->path, alloc, path);
	thread_data->user_alloc = alloc;
	thread_data->_.alloc = xdg_icon_theme__theme_cache_alloc_alloc;
	thread_data->_.free = xdg_icon_theme__theme_cache_alloc_free;
	thread_data->_.realloc = xdg_icon_theme__theme_cache_alloc_realloc;

	if (!theme) {
		theme = alloc->alloc(alloc, sizeof(*theme), ALIGNOF(*theme));
		thread_data->theme = theme;
		arena_init(&theme->arena, alloc, 8388608);
		pthread_mutex_init(&theme->lock, NULL);
		string_init_string(&theme->name, &thread_data->_, name);
		su_hash_table__struct_xdg_icon_theme__icon_cache__init(&theme->icons, &thread_data->_, 65536);
		su_llist__struct_xdg_icon_theme__theme_cache__insert_tail(&cache->themes, theme);
	} else {
		thread_data->theme = theme;
	}

	pthread_t th;
	if (0 == pthread_create(&th, NULL, xdg_icon_theme__theme_cache_populate_thread, thread_data)) {
		su_array__pthread_t__add(&cache->threads, alloc, th);
	} else {
		// TODO: warn
		xdg_icon_theme__theme_cache_populate(theme, &thread_data->_, path);
		string_fini(&thread_data->path, alloc);
		alloc->free(alloc, thread_data);
	}

	return TRUE;
}

static bool32_t xdg_icon_theme_cache_add_basedir(struct xdg_icon_theme_cache *cache,
		allocator_t *alloc, string_t path) {
	ASSERT(path.nul_terminated);
	char abspath_buf[PATH_MAX];
	if (realpath(path.s, abspath_buf) == NULL) {
		return FALSE;
	}

	path = (string_t){
		.s = abspath_buf,
		.len = strlen(abspath_buf),
		.free_contents = FALSE,
		.nul_terminated = TRUE,
	};

	for (size_t i = cache->basedirs.len - 1; i != SIZE_MAX; --i) {
		if (string_equal(path, su_array__su_string_t__get(&cache->basedirs, i))) {
			return FALSE;
		}
	}

	DIR *dir = opendir(path.s);
	if (!dir) {
		return FALSE;
	}

	string_init_string(su_array__su_string_t__add_uninitialized(&cache->basedirs, alloc), alloc, path);

	int dir_fd = dirfd(dir);
	struct dirent *e;
	while ((e = readdir(dir))) {
		if ((e->d_name[0] == '.') && ((e->d_name[1] == '\0') ||
				((e->d_name[1] == '.') && (e->d_name[2] == '\0')))) {
			continue;
		}

		size_t d_name_len = strlen(e->d_name);
		if ((path.len + d_name_len + 2) > PATH_MAX) {
			continue;
		}

		char buf[PATH_MAX];
		memcpy(buf, path.s, path.len);
		buf[path.len] = '/';
		memcpy(&buf[path.len + 1], e->d_name, d_name_len + 1);
		string_t new_path = {
			.s = buf,
			.len = path.len + 1 + d_name_len,
			.free_contents = FALSE,
			.nul_terminated = TRUE,
		};

		string_t name = {
			.s = e->d_name,
			.len = d_name_len,
			.free_contents = FALSE,
			.nul_terminated = TRUE,
		};

		switch (e->d_type) {
		case DT_REG:
			xdg_icon_theme__icon_cache_add_icon(&cache->unthemed, alloc, NULL, new_path, name);
			break;
		case DT_DIR:
			xdg_icon_theme__cache_add_theme(cache, alloc, new_path, name);
			break;
		case DT_LNK:
		case DT_UNKNOWN: {
			struct stat sb;
			if (fstatat(dir_fd, e->d_name, &sb, 0) == 0) {
				if (S_ISREG(sb.st_mode)) {
					xdg_icon_theme__icon_cache_add_icon(&cache->unthemed, alloc, NULL, new_path, name);
				} else if (S_ISDIR(sb.st_mode)) {
					xdg_icon_theme__cache_add_theme(cache, alloc, new_path, name);
				}
			}
			break;
		}
		default:
			break;
		}
	}

	// ? TODO: sort icons by size

	closedir(dir);
	return TRUE;
}

static void xdg_icon_theme_cache_init(struct xdg_icon_theme_cache *cache, allocator_t *alloc) {
	cache->themes = (su_llist__struct_xdg_icon_theme__theme_cache__t){ 0 };
	su_hash_table__struct_xdg_icon_theme__icon_cache__init(&cache->unthemed, alloc, 65536);
	su_array__su_string_t__init(&cache->basedirs, alloc, 16);
	su_array__pthread_t__init(&cache->threads, alloc, 64);

	char buf[PATH_MAX];

	char *data_home = getenv("XDG_DATA_HOME");
	if (data_home && *data_home) {
		size_t data_home_len = strlen(data_home);
		if ((data_home_len + STRING_LITERAL_LENGTH("/icons") + 1) <= sizeof(buf)) {
			memcpy(buf, data_home, data_home_len);
			memcpy(&buf[data_home_len], "/icons", STRING_LITERAL_LENGTH("/icons") + 1);
			xdg_icon_theme_cache_add_basedir(cache, alloc,
				(string_t){
					.s = buf,
					.len = data_home_len + STRING_LITERAL_LENGTH("/icons"),
					.nul_terminated = TRUE,
					.free_contents = FALSE,
				});
		}
	} else {
		char *home = getenv("HOME");
		if (home && *home) {
			size_t home_len = strlen(home);
			if ((home_len + STRING_LITERAL_LENGTH("/.local/share/icons") + 1) <= sizeof(buf)) {
				memcpy(buf, home, home_len);
				memcpy(&buf[home_len], "/.local/share/icons", STRING_LITERAL_LENGTH("/.local/share/icons") + 1);
				xdg_icon_theme_cache_add_basedir(cache, alloc,
					(string_t){
						.s = buf,
						.len = home_len + STRING_LITERAL_LENGTH("/.local/share/icons"),
						.nul_terminated = TRUE,
						.free_contents = FALSE,
					});
			}
		}
	}

	char *data_dirs = getenv("XDG_DATA_DIRS");
	if (data_dirs && *data_dirs) {
		string_t data_dirs_str = {
			.s = data_dirs,
			.len = strlen(data_dirs),
			.free_contents = FALSE,
			.nul_terminated = TRUE,
		};
		string_t dir = { 0 };
		string_t tmp;
		for (bool32_t next = string_tok(&data_dirs_str, ':', &dir, &tmp);
				next == TRUE;
				next = string_tok(NULL, ':', &dir, &tmp)) {
			if ((dir.len + STRING_LITERAL_LENGTH("/icons") + 1) > sizeof(buf)) {
				continue;
			}
			memcpy(buf, dir.s, dir.len);
			memcpy(&buf[dir.len], "/icons", STRING_LITERAL_LENGTH("/icons") + 1);
			xdg_icon_theme_cache_add_basedir(cache, alloc,
				(string_t){
					.s = buf,
					.len = dir.len + STRING_LITERAL_LENGTH("/icons"),
					.nul_terminated = TRUE,
					.free_contents = FALSE,
				});
		}
	} else {
		xdg_icon_theme_cache_add_basedir(cache, alloc, STRING("/usr/local/share/icons"));
		xdg_icon_theme_cache_add_basedir(cache, alloc, STRING("/usr/share/icons"));
	}

	xdg_icon_theme_cache_add_basedir(cache, alloc, STRING("/usr/share/pixmaps"));
}

static void xdg_icon_theme_cache_fini(struct xdg_icon_theme_cache *cache, allocator_t *alloc) {
	for (size_t i = 0; i < cache->threads.len; ++i) {
		pthread_join(su_array__pthread_t__get(&cache->threads, i), NULL); // ? TODO: pthread_cancel
	}
	su_array__pthread_t__fini(&cache->threads, alloc);

	for (size_t i = 0; i < cache->basedirs.len; ++i) {
		string_fini(su_array__su_string_t__get_ptr(&cache->basedirs, i), alloc);
	}
	su_array__su_string_t__fini(&cache->basedirs, alloc);

	for (size_t i = 0; i < cache->unthemed.items.len; ++i) {
		struct xdg_icon_theme__icon_cache icon =
			su_array__struct_xdg_icon_theme__icon_cache__get(&cache->unthemed.items, i);
		string_fini(&icon.key, alloc);
#if WITH_SVG
		for (size_t j = 0; j < icon.svg_paths.len; ++j) {
			string_fini(su_array__su_string_t__get_ptr(&icon.svg_paths, 0), alloc);
		}
		su_array__su_string_t__fini(&icon.svg_paths, alloc);
#endif // WITH_SVG
#if WITH_PNG
		for (size_t j = 0; j < icon.png_paths.len; ++j) {
			string_fini(su_array__su_string_t__get_ptr(&icon.png_paths, 0), alloc);
		}
		su_array__su_string_t__fini(&icon.png_paths, alloc);
#endif // WITH_PNG
	}
	su_hash_table__struct_xdg_icon_theme__icon_cache__fini(&cache->unthemed, alloc);

	for (struct xdg_icon_theme__theme_cache *theme = cache->themes.head; theme; ) {
		struct xdg_icon_theme__theme_cache *next = theme->next;
		arena_fini(&theme->arena, alloc);
		pthread_mutex_destroy(&theme->lock);
		alloc->free(alloc, theme);
		theme = next;
	}
}

static bool32_t xdg_icon_theme__find_icon(struct xdg_icon_theme_icon *dest,
		su_hash_table__struct_xdg_icon_theme__icon_cache__t *ht, string_t icon_name) {
	struct xdg_icon_theme__icon_cache *icon;
	if (su_hash_table__struct_xdg_icon_theme__icon_cache__get(ht,
			(struct xdg_icon_theme__icon_cache){ .key = icon_name }, &icon)) {
#if WITH_SVG
		for (size_t i = icon->svg_paths.len - 1; i != SIZE_MAX; --i) {
			string_t path = su_array__su_string_t__get(&icon->svg_paths, i);
			ASSERT(path.nul_terminated);
			if (access(path.s, R_OK) == 0) {
				*dest = (struct xdg_icon_theme_icon){
					.path = string_copy(path),
					.type = XDG_ICON_THEME_ICON_TYPE_SVG,
				};
				return TRUE;
			}
		}
#endif // WITH_SVG
#if WITH_PNG
		for (size_t i = icon->png_paths.len - 1; i != SIZE_MAX; --i) {
			string_t path = su_array__su_string_t__get(&icon->png_paths, i);
			ASSERT(path.nul_terminated);
			if (access(path.s, R_OK) == 0) {
				*dest = (struct xdg_icon_theme_icon){
					.path = string_copy(path),
					.type = XDG_ICON_THEME_ICON_TYPE_PNG,
				};
				return TRUE;
			}
		}
#endif // WITH_PNG
		ASSERT_UNREACHABLE;
	}

	return FALSE;
}

static bool32_t xdg_icon_theme__cache_find_icon_in_theme(struct xdg_icon_theme_cache *cache,
		struct xdg_icon_theme_icon *dest, string_t icon_name, string_t theme_name) {
	for (struct xdg_icon_theme__theme_cache *theme = cache->themes.head; theme; theme = theme->next) {
		if (string_equal(theme->name, theme_name)) {
			return xdg_icon_theme__find_icon(dest, &theme->icons, icon_name);
		}
	}

	return FALSE;
}

static bool32_t xdg_icon_theme_cache_find_icon(struct xdg_icon_theme_cache *cache,
		struct xdg_icon_theme_icon *dest, string_t icon_name, string_t *theme_name) {
	for (size_t i = 0; i < cache->threads.len; ++i) {
		pthread_join(su_array__pthread_t__get(&cache->threads, i), NULL);
	}
	cache->threads.len = 0;

	// TODO: check mtime / inotify/alternative

	if (theme_name && xdg_icon_theme__cache_find_icon_in_theme(cache, dest, icon_name, *theme_name)) {
		return TRUE;
	}
	if (!theme_name || !string_equal(*theme_name, STRING("hicolor"))) {
		if (xdg_icon_theme__cache_find_icon_in_theme(cache, dest, icon_name, STRING("hicolor"))) {
			return TRUE;
		}
	}
	if (xdg_icon_theme__find_icon(dest, &cache->unthemed, icon_name)) {
		return TRUE;
	}
	for (struct xdg_icon_theme__theme_cache *theme = cache->themes.head; theme; theme = theme->next) {
		if (xdg_icon_theme__find_icon(dest, &theme->icons, icon_name)) {
			return TRUE;
		}
	}

	return FALSE;
}

#endif // XDG_ICON_THEME_H
