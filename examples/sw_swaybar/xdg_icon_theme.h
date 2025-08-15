#if !defined(XDG_ICON_THEME_H)
#define XDG_ICON_THEME_H

#if !defined(WITH_SVG)
#define WITH_SVG 1
#endif /* !defined(WITH_SVG) */
#if !defined(WITH_PNG)
#define WITH_PNG 1
#endif /* !defined(WITH_PNG) */

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
#endif /* !defined(SU_IMPLEMENTATION) */
#if !defined(SU_STRIP_PREFIXES)
#define SU_STRIP_PREFIXES
#endif /* !defined(SU_STRIP_PREFIXES) */
#include "sutil.h"

STATIC_ASSERT(WITH_SVG || WITH_PNG);

ARRAY_DECLARE_DEFINE(pthread_t)

typedef struct xdg_icon_theme__icon {
	HASH_TABLE_FIELDS(string_t);
#if WITH_SVG
	su_array__su_string_t__t svg_paths;
#endif /* WITH_SVG */
#if WITH_PNG
	su_array__su_string_t__t png_paths;
#endif /* WITH_PNG */
} xdg_icon_theme__icon_t;

HASH_TABLE_DECLARE_DEFINE(xdg_icon_theme__icon_t, string_t, stbds_hash_string, string_equal, 16)

typedef struct xdg_icon_theme__theme xdg_icon_theme__theme_t;

struct xdg_icon_theme__theme {
	allocator_t alloc; /* must be first */
	allocator_t *user_alloc;
	string_t path, name;
	su_hash_table__xdg_icon_theme__icon_t__t icons;
	pthread_mutex_t lock;
	arena_t arena;
	LLIST_FIELDS(xdg_icon_theme__theme_t);
};

LLIST_DECLARE_DEFINE(xdg_icon_theme__theme_t)

typedef struct xdg_icon_theme_cache {
	su_llist__xdg_icon_theme__theme_t__t themes;
	xdg_icon_theme__theme_t unthemed;
	su_array__su_string_t__t basedirs;
	su_array__pthread_t__t threads; /* ? TODO: use scratch allocator */
} xdg_icon_theme_cache_t;

static void xdg_icon_theme_cache_init(xdg_icon_theme_cache_t *, allocator_t *);
static void xdg_icon_theme_cache_fini(xdg_icon_theme_cache_t *, allocator_t *);
static bool32_t xdg_icon_theme_cache_add_basedir(xdg_icon_theme_cache_t *, allocator_t *, string_t path);
static bool32_t xdg_icon_theme_cache_find_icon(xdg_icon_theme_cache_t *,
		su_array__su_string_t__t **svgs_out, su_array__su_string_t__t **pngs_out,
		string_t icon_name, string_t *theme_name);


static bool32_t xdg_icon_theme__theme_add_icon(xdg_icon_theme__theme_t *theme,
		string_t path, string_t name) {
	xdg_icon_theme__icon_t *icon;
	string_t xdg_name;

	if (UNLIKELY(name.len < 5)) {
		return FALSE;
	}

	xdg_name.s = name.s;
	xdg_name.len = name.len - 4;
	xdg_name.free_contents = FALSE;
	xdg_name.nul_terminated = FALSE;

	pthread_mutex_lock(&theme->lock);

	if (su_hash_table__xdg_icon_theme__icon_t__add(&theme->icons, theme->user_alloc, xdg_name, &icon)) {
		string_init_string(&icon->key, &theme->alloc, xdg_name);
#if WITH_SVG
		su_array__su_string_t__init(&icon->svg_paths, &theme->alloc, 48);
#endif /* WITH_SVG */
#if WITH_PNG
		su_array__su_string_t__init(&icon->png_paths, &theme->alloc, 48);
#endif /* WITH_PNG */
	}

#if WITH_SVG
	if (string_ends_with(name, string(".svg"))) {
		string_init_string(
			su_array__su_string_t__add_uninitialized(&icon->svg_paths, &theme->alloc),\
			&theme->alloc, path);
	}
#endif /* WITH_SVG */
#if WITH_SVG && WITH_PNG
	else
#endif /* WITH_SVG && WITH_PNG */
#if WITH_PNG
	if (string_ends_with(name, string(".png"))) {
		string_init_string(
			su_array__su_string_t__add_uninitialized(&icon->png_paths, &theme->alloc),
			&theme->alloc, path);
	}
#endif /* WITH_PNG */

    pthread_mutex_unlock(&theme->lock);

	return TRUE;
}

static void xdg_icon_theme__theme_populate(xdg_icon_theme__theme_t *theme, string_t path) {
	/* TODO: remove recursion */

	DIR *dir;
	int dir_fd;
	struct dirent *e;

	ASSERT(path.nul_terminated);

	if (!(dir = opendir(path.s))) {
		return;
	}

	dir_fd = dirfd(dir);
	while ((e = readdir(dir))) {
		char buf[PATH_MAX];
		string_t new_path, name;
		size_t d_name_len;

		if ((e->d_name[0] == '.') && ((e->d_name[1] == '\0') ||
				((e->d_name[1] == '.') && (e->d_name[2] == '\0')))) {
			continue;
		}

		d_name_len = strlen(e->d_name);
		if (UNLIKELY((path.len + d_name_len + 2) > PATH_MAX)) {
			continue;
		}

		memcpy(buf, path.s, path.len);
		buf[path.len] = '/';
		memcpy(&buf[path.len + 1], e->d_name, d_name_len + 1);

		new_path.s = buf;
		new_path.len = path.len + d_name_len + 1;
		new_path.free_contents = FALSE;
		new_path.nul_terminated = TRUE;

		name.s = e->d_name;
		name.len = d_name_len;
		name.free_contents = FALSE;
		name.nul_terminated = TRUE;

		switch (e->d_type) {
		case DT_REG:
			xdg_icon_theme__theme_add_icon(theme, new_path, name);
			break;
		case DT_DIR:
			xdg_icon_theme__theme_populate(theme, new_path);
			break;
		case DT_LNK:
		case DT_UNKNOWN: {
			struct stat sb;
			if (fstatat(dir_fd, e->d_name, &sb, 0) == 0) {
				if (S_ISREG(sb.st_mode)) {
					xdg_icon_theme__theme_add_icon(theme, new_path, name);
				} else if (S_ISDIR(sb.st_mode)) {
					xdg_icon_theme__theme_populate(theme, new_path);
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

static void *xdg_icon_theme__theme_alloc_alloc(allocator_t *alloc, size_t size, size_t alignment) {
	xdg_icon_theme__theme_t *theme = (xdg_icon_theme__theme_t *)alloc;
	void *ret = arena_alloc(&theme->arena, theme->user_alloc, size, alignment);
	return ret;
}

static void *xdg_icon_theme__theme_alloc_realloc(allocator_t *alloc,
		void *ptr, size_t new_size, size_t new_alignment) {
	xdg_icon_theme__theme_t *theme = (xdg_icon_theme__theme_t *)alloc;
	void *ret = arena_alloc(&theme->arena, theme->user_alloc, new_size, new_alignment);
	if (ptr) {
		memcpy(ret, ptr, MIN(new_size, arena_alloc_get_size(ptr)));
	}
	return ret;
}

static void *xdg_icon_theme__theme_populate_thread(void *data) {
	xdg_icon_theme__theme_t *theme = data;
	string_t path;

	/* TODO: rework */
	pthread_mutex_lock(&theme->lock);
	path = theme->path;
	pthread_mutex_unlock(&theme->lock);

	xdg_icon_theme__theme_populate(theme, path);
	return NULL;
}

static bool32_t xdg_icon_theme__cache_add_theme(xdg_icon_theme_cache_t *cache,
		allocator_t *alloc, string_t path, string_t name) {
	xdg_icon_theme__theme_t *theme = NULL;
	xdg_icon_theme__theme_t *t = cache->themes.head;
	pthread_t th;

	for ( ; t; t = t->next) {
		if (string_equal(t->name, name)) {
			theme = t;
			break;
		}
	}

	if (!theme) {
		theme = alloc->alloc(alloc, sizeof(*theme), ALIGNOF(*theme));
		pthread_mutex_init(&theme->lock, NULL);
		theme->user_alloc = alloc;
		theme->alloc.alloc = xdg_icon_theme__theme_alloc_alloc;
		theme->alloc.realloc = xdg_icon_theme__theme_alloc_realloc;
		arena_init(&theme->arena, alloc, 67108864);
		su_hash_table__xdg_icon_theme__icon_t__init(&theme->icons, alloc, 65536);
		string_init_string(&theme->name, &theme->alloc, name);

		su_llist__xdg_icon_theme__theme_t__insert_tail(&cache->themes, theme);
	}

	/* TODO: rework */
	pthread_mutex_lock(&theme->lock);
	string_init_string(&theme->path, &theme->alloc, path);
	pthread_mutex_unlock(&theme->lock);

	if (0 == pthread_create(&th, NULL, xdg_icon_theme__theme_populate_thread, theme)) {
		su_array__pthread_t__add(&cache->threads, alloc, th);
	} else {
		/* TODO: warn */
		xdg_icon_theme__theme_populate(theme, path);
	}

	return TRUE;
}

static bool32_t xdg_icon_theme_cache_add_basedir(xdg_icon_theme_cache_t *cache,
		allocator_t *alloc, string_t path) {
	static char abspath_buf[PATH_MAX];
	DIR *dir;
	int dir_fd;
	struct dirent *e;
	size_t i;

	ASSERT(path.nul_terminated);
	
	if (realpath(path.s, abspath_buf) == NULL) {
		return FALSE;
	}

	path = string(abspath_buf);

	for ( i = cache->basedirs.len - 1; i != SIZE_MAX; --i) {
		if (string_equal(path, su_array__su_string_t__get(&cache->basedirs, i))) {
			return FALSE;
		}
	}

	if (!(dir = opendir(path.s))) {
		return FALSE;
	}

	string_init_string(su_array__su_string_t__add_uninitialized(&cache->basedirs, alloc), alloc, path);

	dir_fd = dirfd(dir);
	while ((e = readdir(dir))) {
		char buf[PATH_MAX];
		string_t new_path, name;
		size_t d_name_len;

		if ((e->d_name[0] == '.') && ((e->d_name[1] == '\0') ||
				((e->d_name[1] == '.') && (e->d_name[2] == '\0')))) {
			continue;
		}

		d_name_len = strlen(e->d_name);
		if (UNLIKELY((path.len + d_name_len + 2) > PATH_MAX)) {
			continue;
		}

		memcpy(buf, path.s, path.len);
		buf[path.len] = '/';
		memcpy(&buf[path.len + 1], e->d_name, d_name_len + 1);

		new_path.s = buf;
		new_path.len = path.len + 1 + d_name_len;
		new_path.free_contents = FALSE;
		new_path.nul_terminated = TRUE;

		name.s = e->d_name;
		name.len = d_name_len;
		name.free_contents = FALSE;
		name.nul_terminated = TRUE;

		switch (e->d_type) {
		case DT_REG:
			xdg_icon_theme__theme_add_icon(&cache->unthemed, new_path, name);
			break;
		case DT_DIR:
			xdg_icon_theme__cache_add_theme(cache, alloc, new_path, name);
			break;
		case DT_LNK:
		case DT_UNKNOWN: {
			struct stat sb;
			if (fstatat(dir_fd, e->d_name, &sb, 0) == 0) {
				if (S_ISREG(sb.st_mode)) {
					xdg_icon_theme__theme_add_icon(&cache->unthemed, new_path, name);
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

	/* ? TODO: sort icons by size */

	closedir(dir);
	return TRUE;
}

static void xdg_icon_theme_cache_init(xdg_icon_theme_cache_t *cache, allocator_t *alloc) {
	char *data_home = getenv("XDG_DATA_HOME");
	char *data_dirs = getenv("XDG_DATA_DIRS");
	static char buf[PATH_MAX];

	memset(cache, 0, sizeof(*cache));

	arena_init(&cache->unthemed.arena, alloc, 67108864);
	cache->unthemed.alloc.alloc = xdg_icon_theme__theme_alloc_alloc;
	cache->unthemed.alloc.realloc = xdg_icon_theme__theme_alloc_realloc;
	cache->unthemed.user_alloc = alloc;
	su_hash_table__xdg_icon_theme__icon_t__init(&cache->unthemed.icons, alloc, 65536);
	pthread_mutex_init(&cache->unthemed.lock, NULL);

	su_array__su_string_t__init(&cache->basedirs, alloc, 32);
	su_array__pthread_t__init(&cache->threads, alloc, 64);

	if (data_home && *data_home) {
		size_t len = strlen(data_home);
		if ((len + STRING_LITERAL_LENGTH("/icons") + 1) <= sizeof(buf)) {
			string_t s;
			s.s = buf;
			s.len = len + STRING_LITERAL_LENGTH("/icons");
			s.nul_terminated = TRUE;
			s.free_contents = FALSE;
			memcpy(buf, data_home, len);
			memcpy(&buf[len], "/icons", STRING_LITERAL_LENGTH("/icons") + 1);
			xdg_icon_theme_cache_add_basedir(cache, alloc, s);
		}
	} else {
		char *home = getenv("HOME");
		if (home && *home) {
			size_t len = strlen(home);
			if ((len + STRING_LITERAL_LENGTH("/.local/share/icons") + 1) <= sizeof(buf)) {
				string_t s;
				s.s = buf;
				s.len = len + STRING_LITERAL_LENGTH("/.local/share/icons");
				s.nul_terminated = TRUE;
				s.free_contents = FALSE;
				memcpy(buf, home, len);
				memcpy(&buf[len], "/.local/share/icons", STRING_LITERAL_LENGTH("/.local/share/icons") + 1);
				xdg_icon_theme_cache_add_basedir(cache, alloc, s);
			}
		}
	}

	if (data_dirs && *data_dirs) {
		bool32_t next;
		string_t data_dirs_str, dir, tmp;
		data_dirs_str = string(data_dirs);
		for (next = string_tok(&data_dirs_str, ':', &dir, &tmp);
				next == TRUE;
				next = string_tok(NULL, ':', &dir, &tmp)) {
			if ((dir.len + STRING_LITERAL_LENGTH("/icons") + 1) <= sizeof(buf)) {
				string_t s;
				s.s = buf;
				s.len = dir.len + STRING_LITERAL_LENGTH("/icons");
				s.nul_terminated = TRUE;
				s.free_contents = FALSE;
				memcpy(buf, dir.s, dir.len);
				memcpy(&buf[dir.len], "/icons", STRING_LITERAL_LENGTH("/icons") + 1);
				xdg_icon_theme_cache_add_basedir(cache, alloc, s);
			}
		}
	} else {
		xdg_icon_theme_cache_add_basedir(cache, alloc, string("/usr/local/share/icons"));
		xdg_icon_theme_cache_add_basedir(cache, alloc, string("/usr/share/icons"));
	}

	xdg_icon_theme_cache_add_basedir(cache, alloc, string("/usr/share/pixmaps"));
}

static void xdg_icon_theme_cache_fini(xdg_icon_theme_cache_t *cache, allocator_t *alloc) {
	size_t i;
	xdg_icon_theme__theme_t *theme;

	for ( i = 0; i < cache->threads.len; ++i) {
		pthread_join(su_array__pthread_t__get(&cache->threads, i), NULL); /* ? TODO: pthread_cancel */
	}
	su_array__pthread_t__fini(&cache->threads, alloc);

	for ( i = 0; i < cache->basedirs.len; ++i) {
		string_fini(su_array__su_string_t__get_ptr(&cache->basedirs, i), alloc);
	}
	su_array__su_string_t__fini(&cache->basedirs, alloc);

	arena_fini(&cache->unthemed.arena, alloc);
	pthread_mutex_destroy(&cache->unthemed.lock);
	su_hash_table__xdg_icon_theme__icon_t__fini(&cache->unthemed.icons, alloc);

	for ( theme = cache->themes.head; theme; ) {
		xdg_icon_theme__theme_t *next = theme->next;
		arena_fini(&theme->arena, alloc);
		pthread_mutex_destroy(&theme->lock);
		su_hash_table__xdg_icon_theme__icon_t__fini(&theme->icons, alloc);
		alloc->free(alloc, theme);
		theme = next;
	}
}

static bool32_t xdg_icon_theme__find_icon(
		su_array__su_string_t__t **svgs_out, su_array__su_string_t__t **pngs_out,
		xdg_icon_theme__theme_t *theme, string_t icon_name) {
	bool32_t ret = FALSE;
	xdg_icon_theme__icon_t *c;
	
	NOTUSED(svgs_out); NOTUSED(pngs_out);

	if (su_hash_table__xdg_icon_theme__icon_t__get(&theme->icons, icon_name , &c)) {
#if WITH_SVG
		if (c->svg_paths.len > 0) {
			*svgs_out = &c->svg_paths;
			ret = TRUE;
		}
#endif /* WITH_SVG */
#if WITH_PNG
		if (c->png_paths.len > 0) {
			ret = TRUE;
			*pngs_out = &c->png_paths;
		}
#endif /* WITH_PNG */
	}

	return ret;
}

static bool32_t xdg_icon_theme__cache_find_icon_in_theme(xdg_icon_theme_cache_t *cache,
		su_array__su_string_t__t **svgs_out, su_array__su_string_t__t **pngs_out,
		string_t icon_name, string_t theme_name) {
	xdg_icon_theme__theme_t *theme = cache->themes.head;
	for ( ; theme; theme = theme->next) {
		if (string_equal(theme->name, theme_name)) {
			return xdg_icon_theme__find_icon(svgs_out, pngs_out, theme, icon_name);
		}
	}

	return FALSE;
}

static bool32_t xdg_icon_theme_cache_find_icon(xdg_icon_theme_cache_t *cache,
		su_array__su_string_t__t **svgs_out, su_array__su_string_t__t **pngs_out,
		string_t icon_name, string_t *theme_name) {
	xdg_icon_theme__theme_t *theme;

	size_t i = 0;
	for ( ; i < cache->threads.len; ++i) {
		pthread_join(su_array__pthread_t__get(&cache->threads, i), NULL);
	}
	cache->threads.len = 0;

	/* TODO: check mtime / inotify/alternative */

	if (theme_name && xdg_icon_theme__cache_find_icon_in_theme(
			cache, svgs_out, pngs_out, icon_name, *theme_name)) {
		return TRUE;
	}
	if (!theme_name || !string_equal(*theme_name, string("hicolor"))) {
		if (xdg_icon_theme__cache_find_icon_in_theme(
				cache, svgs_out, pngs_out, icon_name, string("hicolor"))) {
			return TRUE;
		}
	}
	if (xdg_icon_theme__find_icon(svgs_out, pngs_out, &cache->unthemed, icon_name)) {
		return TRUE;
	}
	for ( theme = cache->themes.head; theme; theme = theme->next) {
		if (xdg_icon_theme__find_icon( svgs_out, pngs_out, theme, icon_name)) {
			return TRUE;
		}
	}

	return FALSE;
}

#endif /* XDG_ICON_THEME_H */
