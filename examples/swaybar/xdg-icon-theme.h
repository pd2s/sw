#if !defined(XDG_ICON_THEME_H)
#define XDG_ICON_THEME_H

#if !WITH_SVG && !WITH_PNG
#error "png or svg support must be enabled"
#endif // !WITH_SVG && !WITH_PNG

//#define _XOPEN_SOURCE 700 // realpath
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>

#include "util.h"

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
	uint32_t pad;
};

struct xdg_icon_theme__icon_cache {
	HASH_TABLE_STRUCT_FIELDS(string_t)

#if WITH_SVG
	array_string_t_t svg_paths;
#endif // WITH_SVG
#if WITH_PNG
	array_string_t_t png_paths;
#endif // WITH_PNG
};

typedef struct xdg_icon_theme__icon_cache struct_xdg_icon_theme__icon_cache;
HASH_TABLE_DECLARE_DEFINE(struct_xdg_icon_theme__icon_cache, stbds_hash_string, string_equal, 16)

struct xdg_icon_theme__theme_cache {
	string_t name;
	hash_table_struct_xdg_icon_theme__icon_cache_t icons;
	pthread_mutex_t lock;
};

typedef struct xdg_icon_theme__theme_cache* struct_xdg_icon_theme__theme_cache_ptr;
ARRAY_DECLARE_DEFINE(struct_xdg_icon_theme__theme_cache_ptr)

struct xdg_icon_theme_cache {
	array_struct_xdg_icon_theme__theme_cache_ptr_t themes;
	hash_table_struct_xdg_icon_theme__icon_cache_t unthemed;
	array_string_t_t basedirs;
	array_pthread_t_t threads;
};

static void xdg_icon_theme_cache_init(struct xdg_icon_theme_cache *);
static void xdg_icon_theme_cache_fini(struct xdg_icon_theme_cache *);
static bool32_t xdg_icon_theme_cache_add_basedir(struct xdg_icon_theme_cache *, string_t path, bool32_t join_threads);
static void xdg_icon_theme_cache_join_threads(struct xdg_icon_theme_cache *);

static bool32_t xdg_icon_theme_cache_find_icon(struct xdg_icon_theme_cache *,
	struct xdg_icon_theme_icon *dest, string_t icon_name, string_t *theme_name);
static bool32_t xdg_icon_theme_cache_find_icon_in_theme(struct xdg_icon_theme_cache *,
	struct xdg_icon_theme_icon *dest, string_t icon_name, string_t theme_name);


static void xdg_icon_theme__icon_cache_init(struct xdg_icon_theme__icon_cache *icon, string_t name) {
	string_init_string(&icon->key, name);
#if WITH_SVG
	array_string_t_init(&icon->svg_paths, 16);
#endif // WITH_SVG
#if WITH_PNG
	array_string_t_init(&icon->png_paths, 16);
#endif // WITH_PNG
}

static void xdg_icon_theme__icon_cache_fini(struct xdg_icon_theme__icon_cache *icon) {
	string_fini(&icon->key);
#if WITH_SVG
	for (size_t i = 0; i < icon->svg_paths.len; ++i) {
		string_fini(array_string_t_get_ptr(&icon->svg_paths, i));
	}
	array_string_t_fini(&icon->svg_paths);
#endif // WITH_SVG
#if WITH_PNG
	for (size_t i = 0; i < icon->png_paths.len; ++i) {
		string_fini(array_string_t_get_ptr(&icon->png_paths, i));
	}
	array_string_t_fini(&icon->png_paths);
#endif // WITH_PNG
}

static struct xdg_icon_theme__theme_cache *xdg_icon_theme__theme_cache_create(string_t name) {
	struct xdg_icon_theme__theme_cache *theme = malloc(SIZEOF(struct xdg_icon_theme__theme_cache));
	string_init_string(&theme->name, name);
	hash_table_struct_xdg_icon_theme__icon_cache_init(&theme->icons, 65536);
	pthread_mutex_init(&theme->lock, NULL);

	return theme;
}

static void xdg_icon_theme__theme_cache_destroy(struct xdg_icon_theme__theme_cache *theme) {
	if (!theme) {
		return;
	}

	string_fini(&theme->name);
	for (size_t i = 0; i < theme->icons.items.len; ++i) {
		struct xdg_icon_theme__icon_cache *icon = array_struct_xdg_icon_theme__icon_cache_get_ptr(&theme->icons.items, i);
		if (icon->occupied && !icon->tombstone) {
			xdg_icon_theme__icon_cache_fini(icon);
		}
	}
	hash_table_struct_xdg_icon_theme__icon_cache_fini(&theme->icons);
	pthread_mutex_destroy(&theme->lock);

	free(theme);
}

static bool32_t xdg_icon_theme__icon_cache_add_icon(hash_table_struct_xdg_icon_theme__icon_cache_t *dest,
		pthread_mutex_t *lock, string_t path, string_t name) {
	uint32_t type = 0;
#if WITH_SVG
		if (string_ends_with(name, STRING_LITERAL(".svg"))) {
			type = XDG_ICON_THEME_ICON_TYPE_SVG;
		}
#endif // WITH_SVG
#if WITH_SVG && WITH_PNG
		else
#endif // WITH_SVG && WITH_PNG
#if WITH_PNG
		if (string_ends_with(name, STRING_LITERAL(".png"))) {
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

	if (hash_table_struct_xdg_icon_theme__icon_cache_add(dest,
			(struct xdg_icon_theme__icon_cache){ .key = xdg_name }, &icon)) {
		xdg_icon_theme__icon_cache_init(icon, xdg_name);
	}

	string_t *p;
	switch (type) {
#if WITH_SVG
	case XDG_ICON_THEME_ICON_TYPE_SVG:
		p = array_string_t_add(&icon->svg_paths, path);
		break;
#endif // WITH_SVG
#if WITH_PNG
	case XDG_ICON_THEME_ICON_TYPE_PNG:
		p = array_string_t_add(&icon->png_paths, path);
		break;
#endif // WITH_PNG
	default:
		ASSERT_UNREACHABLE;
		return FALSE;
	}

	if (lock) {
		pthread_mutex_unlock(lock);
	}

	string_init_string(p, path);

	return TRUE;
}

static void xdg_icon_theme__theme_cache_populate(struct xdg_icon_theme__theme_cache *theme, string_t path) {
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
			xdg_icon_theme__icon_cache_add_icon(&theme->icons, &theme->lock, new_path, name);
			break;
		case DT_DIR:
			xdg_icon_theme__theme_cache_populate(theme, new_path);
			break;
		case DT_LNK:
		case DT_UNKNOWN: {
			struct stat sb;
			if (fstatat(dir_fd, e->d_name, &sb, 0) == 0) {
				if (S_ISREG(sb.st_mode)) {
					xdg_icon_theme__icon_cache_add_icon(
						&theme->icons, &theme->lock, new_path, name);
				} else if (S_ISDIR(sb.st_mode)) {
					xdg_icon_theme__theme_cache_populate(theme, new_path);
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

struct xdg_icon_theme__theme_cache_populate_thread_data {
	struct xdg_icon_theme__theme_cache *theme;
	string_t path;
};

static void *xdg_icon_theme__theme_cache_populate_thread(void *arg) {
	struct xdg_icon_theme__theme_cache_populate_thread_data *data = arg;
	xdg_icon_theme__theme_cache_populate(data->theme, data->path);

	string_fini(&data->path);
	free(data);
	return NULL;
}

static bool32_t xdg_icon_theme__cache_add_theme(struct xdg_icon_theme_cache *cache,
		string_t path, string_t name) {
	struct xdg_icon_theme__theme_cache *theme = NULL;
	for (size_t i = 0; i < cache->themes.len; ++i) {
		struct xdg_icon_theme__theme_cache *theme_ =
			array_struct_xdg_icon_theme__theme_cache_ptr_get(&cache->themes, i);
		if (string_equal(theme_->name, name)) {
			theme = theme_;
			break;
		}
	}

	if (!theme) {
		theme = xdg_icon_theme__theme_cache_create(name);
		array_struct_xdg_icon_theme__theme_cache_ptr_add(&cache->themes, theme);
	}

	struct xdg_icon_theme__theme_cache_populate_thread_data *thread_data = malloc(
		SIZEOF(struct xdg_icon_theme__theme_cache_populate_thread_data));
	thread_data->theme = theme;
	string_init_string(&thread_data->path, path);
	pthread_t th;
	if (0 == pthread_create(&th, NULL, xdg_icon_theme__theme_cache_populate_thread, thread_data)) {
		array_pthread_t_add(&cache->threads, th);
	} else {
		string_fini(&thread_data->path);
		free(thread_data);
		xdg_icon_theme__theme_cache_populate(theme, path);
	}

	return TRUE;
}

static void xdg_icon_theme_cache_join_threads(struct xdg_icon_theme_cache *cache) {
	for (size_t i = 0; i < cache->threads.len; ++i) {
		pthread_join(array_pthread_t_get(&cache->threads, i), NULL);
	}
	cache->threads.len = 0;
}

static bool32_t xdg_icon_theme_cache_add_basedir(struct xdg_icon_theme_cache *cache, string_t path, bool32_t join_threads) {
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
		if (string_equal(path, array_string_t_get(&cache->basedirs, i))) {
			return FALSE;
		}
	}

	DIR *dir = opendir(path.s);
	if (!dir) {
		return FALSE;
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
			xdg_icon_theme__icon_cache_add_icon(&cache->unthemed, NULL, new_path, name);
			break;
		case DT_DIR:
			xdg_icon_theme__cache_add_theme(cache, new_path, name);
			break;
		case DT_LNK:
		case DT_UNKNOWN: {
			struct stat sb;
			if (fstatat(dir_fd, e->d_name, &sb, 0) == 0) {
				if (S_ISREG(sb.st_mode)) {
					xdg_icon_theme__icon_cache_add_icon(
						&cache->unthemed, NULL, new_path, name);
				} else if (S_ISDIR(sb.st_mode)) {
					xdg_icon_theme__cache_add_theme(cache, new_path, name);
				}
			}
			break;
		}
		default:
			break;
		}
	}

	// ? TODO: sort icons by size

	if (join_threads) {
		xdg_icon_theme_cache_join_threads(cache);
	}

	closedir(dir);

	string_init_string(&path, path);
	array_string_t_add(&cache->basedirs, path);
	return TRUE;
}

static void xdg_icon_theme_cache_init(struct xdg_icon_theme_cache *cache) {
	array_struct_xdg_icon_theme__theme_cache_ptr_init(&cache->themes, 64);
	hash_table_struct_xdg_icon_theme__icon_cache_init(&cache->unthemed, 65536);
	array_string_t_init(&cache->basedirs, 16);
	array_pthread_t_init(&cache->threads, 64);

	char buf[PATH_MAX];

	char *data_home = getenv("XDG_DATA_HOME");
	if (data_home && *data_home) {
		size_t data_home_len = strlen(data_home);
		if ((data_home_len + STRING_LITERAL_LENGTH("/icons") + 1) <= SIZEOF(buf)) {
			memcpy(buf, data_home, data_home_len);
			memcpy(&buf[data_home_len], "/icons", STRING_LITERAL_LENGTH("/icons") + 1);
			xdg_icon_theme_cache_add_basedir(cache, (string_t){
				.s = buf,
				.len = data_home_len + STRING_LITERAL_LENGTH("/icons"),
				.nul_terminated = TRUE,
				.free_contents = FALSE,
			}, FALSE);
		}
	} else {
		char *home = getenv("HOME");
		if (home && *home) {
			size_t home_len = strlen(home);
			if ((home_len + STRING_LITERAL_LENGTH("/.local/share/icons") + 1) <= SIZEOF(buf)) {
				memcpy(buf, home, home_len);
				memcpy(&buf[home_len], "/.local/share/icons", STRING_LITERAL_LENGTH("/.local/share/icons") + 1);
				xdg_icon_theme_cache_add_basedir(cache, (string_t){
					.s = buf,
					.len = home_len + STRING_LITERAL_LENGTH("/.local/share/icons"),
					.nul_terminated = TRUE,
					.free_contents = FALSE,
				}, FALSE);
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
			if ((dir.len + STRING_LITERAL_LENGTH("/icons") + 1) > SIZEOF(buf)) {
				continue;
			}
			memcpy(buf, dir.s, dir.len);
			memcpy(&buf[dir.len], "/icons", STRING_LITERAL_LENGTH("/icons") + 1);
			xdg_icon_theme_cache_add_basedir(cache, (string_t){
				.s = buf,
				.len = dir.len + STRING_LITERAL_LENGTH("/icons"),
				.nul_terminated = TRUE,
				.free_contents = FALSE,
			}, FALSE);
		}
	} else {
		xdg_icon_theme_cache_add_basedir(cache, STRING_LITERAL("/usr/local/share/icons"), FALSE);
		xdg_icon_theme_cache_add_basedir(cache, STRING_LITERAL("/usr/share/icons"), FALSE);
	}

	xdg_icon_theme_cache_add_basedir(cache, STRING_LITERAL("/usr/share/pixmaps"), FALSE);

	xdg_icon_theme_cache_join_threads(cache);
}

static void xdg_icon_theme_cache_fini(struct xdg_icon_theme_cache *cache) {
	for (size_t i = 0; i < cache->themes.len; ++i) {
		xdg_icon_theme__theme_cache_destroy(
			array_struct_xdg_icon_theme__theme_cache_ptr_get(&cache->themes, i));
	}
	array_struct_xdg_icon_theme__theme_cache_ptr_fini(&cache->themes);
	for (size_t i = 0; i < cache->unthemed.items.len; ++i) {
		struct xdg_icon_theme__icon_cache *icon =
			array_struct_xdg_icon_theme__icon_cache_get_ptr(&cache->unthemed.items, i);
		if (icon->occupied && !icon->tombstone) {
			xdg_icon_theme__icon_cache_fini(icon);
		}
	}
	hash_table_struct_xdg_icon_theme__icon_cache_fini(&cache->unthemed);
	for (size_t i = 0; i < cache->basedirs.len; ++i) {
		string_fini(array_string_t_get_ptr(&cache->basedirs, i));
	}
	array_string_t_fini(&cache->basedirs);
	array_pthread_t_fini(&cache->threads);
}

static bool32_t xdg_icon_theme__find_icon(struct xdg_icon_theme_icon *dest,
		hash_table_struct_xdg_icon_theme__icon_cache_t *ht, string_t icon_name) {
	struct xdg_icon_theme__icon_cache *icon;
	if (hash_table_struct_xdg_icon_theme__icon_cache_get(ht,
			(struct xdg_icon_theme__icon_cache){ .key = icon_name }, &icon)) {
#if WITH_SVG
		for (size_t i = icon->svg_paths.len - 1; i != SIZE_MAX; --i) {
			string_t path = array_string_t_get(&icon->svg_paths, i);
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
			string_t path = array_string_t_get(&icon->png_paths, i);
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

static bool32_t xdg_icon_theme_cache_find_icon_in_theme(struct xdg_icon_theme_cache *cache,
		struct xdg_icon_theme_icon *dest, string_t icon_name, string_t theme_name) {
	ASSERT(cache->threads.len == 0);

	for (size_t i = 0; i < cache->themes.len; ++i) {
		struct xdg_icon_theme__theme_cache *theme =
			array_struct_xdg_icon_theme__theme_cache_ptr_get(&cache->themes, i);
		if (string_equal(theme->name, theme_name)) {
			return xdg_icon_theme__find_icon(dest, &theme->icons, icon_name);
		}
	}

	return FALSE;
}

static bool32_t xdg_icon_theme_cache_find_icon(struct xdg_icon_theme_cache *cache,
		struct xdg_icon_theme_icon *dest, string_t icon_name, string_t *theme_name) {
	ASSERT(cache->threads.len == 0);
	// TODO: check mtime

	if (theme_name && xdg_icon_theme_cache_find_icon_in_theme(cache, dest, icon_name, *theme_name)) {
		return TRUE;
	}
	if (!theme_name || !string_equal(*theme_name, STRING_LITERAL("hicolor"))) {
		if (xdg_icon_theme_cache_find_icon_in_theme(cache, dest, icon_name, STRING_LITERAL("hicolor"))) {
			return TRUE;
		}
	}
	if (xdg_icon_theme__find_icon(dest, &cache->unthemed, icon_name)) {
		return TRUE;
	}
	for (size_t i = 0; i < cache->themes.len; ++i) {
		struct xdg_icon_theme__theme_cache *theme =
			array_struct_xdg_icon_theme__theme_cache_ptr_get(&cache->themes, i);
		if (xdg_icon_theme__find_icon(dest, &theme->icons, icon_name)) {
			return TRUE;
		}
	}

	return FALSE;
}

#endif // XDG_ICON_THEME_H
