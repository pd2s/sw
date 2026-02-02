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
#include <sutil.h>

STATIC_ASSERT(WITH_SVG || WITH_PNG);

typedef struct xdg_icon_theme__icon {
	HASH_TABLE_FIELDS(string_t);
#if WITH_SVG
	string_t *svg_paths;
	size_t svg_paths_count, svg_paths_capacity;
#endif /* WITH_SVG */
#if WITH_PNG
	string_t *png_paths;
	size_t png_paths_count, png_paths_capacity;
#endif /* WITH_PNG */
} xdg_icon_theme__icon_t;

/* TODO: better hash function */
HASH_TABLE_DECLARE_DEFINE(xdg_icon_theme__icon_t, string_t, stbds_hash_string, string_equal, MEMCPY, 16)

typedef struct xdg_icon_theme__theme xdg_icon_theme__theme_t;

struct xdg_icon_theme__theme {
	allocator_t alloc; /* must be first */
	const allocator_t *user_alloc;
	string_t path, name;
	su_hash_table__xdg_icon_theme__icon_t__t icons;
	pthread_mutex_t lock;
	arena_t arena;
	xdg_icon_theme__theme_t *next; /* singly linked list */
};

typedef struct xdg_icon_theme_cache {
	xdg_icon_theme__theme_t unthemed;
	string_t *basedirs;
	size_t basedirs_count, basedirs_capacity;
	pthread_t threads[64]; /* ? TODO: dynamic */
	size_t threads_count;
	xdg_icon_theme__theme_t *themes; /* singly linked list */
} xdg_icon_theme_cache_t;

static void xdg_icon_theme_cache_init(xdg_icon_theme_cache_t *, const allocator_t *);
static void xdg_icon_theme_cache_fini(xdg_icon_theme_cache_t *, const allocator_t *);
static bool32_t xdg_icon_theme_cache_add_basedir(xdg_icon_theme_cache_t *, const allocator_t *, string_t path);
static bool32_t xdg_icon_theme_cache_find_icon(xdg_icon_theme_cache_t *,
		string_t **svgs_out, size_t *svgs_count_out,
		string_t **pngs_out, size_t *pngs_count_out,
		string_t icon_name, string_t *theme_name);


static bool32_t xdg_icon_theme__theme_add_icon(xdg_icon_theme__theme_t *theme,
		string_t path, string_t name) {
	xdg_icon_theme__icon_t *icon;
	string_t xdg_name;

	if (UNLIKELY(name.len < 5)) {
		return FALSE;
	}

	xdg_name.s = name.s;
	xdg_name.len = (name.len - 4);
	xdg_name.free_contents = FALSE;
	xdg_name.nul_terminated = FALSE;

	pthread_mutex_lock(&theme->lock);

	if (su_hash_table__xdg_icon_theme__icon_t__add(&theme->icons, theme->user_alloc, xdg_name, &icon)) {
		string_init_string(&icon->key, &theme->alloc, xdg_name);
#if WITH_SVG
		icon->svg_paths_capacity = 48;
		ARRAY_ALLOC(icon->svg_paths, &theme->alloc, icon->svg_paths_capacity);
		icon->svg_paths_count = 0;
#endif /* WITH_SVG */
#if WITH_PNG
		icon->png_paths_capacity = 48;
		ARRAY_ALLOC(icon->png_paths, &theme->alloc, icon->png_paths_capacity);
		icon->png_paths_count = 0;
#endif /* WITH_PNG */
	}

#if WITH_SVG
	if (string_ends_with(name, string(".svg"))) {
    	if (UNLIKELY(icon->svg_paths_capacity == icon->svg_paths_count)) {
			string_t *new_svg_paths;
			icon->svg_paths_capacity *= 2;
			ARRAY_ALLOC(new_svg_paths, &theme->alloc, icon->svg_paths_capacity);
			MEMCPY(new_svg_paths, icon->svg_paths,
				sizeof(new_svg_paths[0]) * icon->svg_paths_count);
			FREE(&theme->alloc, icon->svg_paths);
			icon->svg_paths = new_svg_paths;
    	}
		string_init_string(&icon->svg_paths[icon->svg_paths_count++], &theme->alloc, path);
	}
#endif /* WITH_SVG */
#if WITH_SVG && WITH_PNG
	else
#endif /* WITH_SVG && WITH_PNG */
#if WITH_PNG
	if (string_ends_with(name, string(".png"))) {
    	if (UNLIKELY(icon->png_paths_capacity == icon->png_paths_count)) {
			string_t *new_png_paths;
			icon->png_paths_capacity *= 2;
			ARRAY_ALLOC(new_png_paths, &theme->alloc, icon->png_paths_capacity);
			MEMCPY(new_png_paths, icon->png_paths,
				sizeof(new_png_paths[0]) * icon->png_paths_count);
			FREE(&theme->alloc, icon->png_paths);
			icon->png_paths = new_png_paths;
    	}
		string_init_string(&icon->png_paths[icon->png_paths_count++], &theme->alloc, path);
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

		d_name_len = STRLEN(e->d_name);
		if (UNLIKELY((path.len + d_name_len + 2) > PATH_MAX)) {
			continue;
		}

		MEMCPY(buf, path.s, path.len);
		buf[path.len] = '/';
		MEMCPY(&buf[path.len + 1], e->d_name, d_name_len + 1);

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

static void *xdg_icon_theme__theme_alloc_alloc(const allocator_t *alloc, size_t size, size_t alignment) {
	xdg_icon_theme__theme_t *theme = (xdg_icon_theme__theme_t *)(uintptr_t)alloc;
	void *ret = arena_alloc(&theme->arena, theme->user_alloc, size, alignment);
	return ret;
}

static void *xdg_icon_theme__theme_populate_thread(void *data) {
	xdg_icon_theme__theme_t *theme = (xdg_icon_theme__theme_t *)data;
	string_t path;

	/* TODO: rework */
	pthread_mutex_lock(&theme->lock);
	path = theme->path;
	pthread_mutex_unlock(&theme->lock);

	xdg_icon_theme__theme_populate(theme, path);
	return NULL;
}

static bool32_t xdg_icon_theme__cache_add_theme(xdg_icon_theme_cache_t *cache,
		const allocator_t *alloc, string_t path, string_t name) {
	xdg_icon_theme__theme_t *theme = NULL;
	pthread_t th;
	xdg_icon_theme__theme_t *t = cache->themes;
	for ( ; t; t = t->next) {
		if (string_equal(t->name, name)) {
			theme = t;
			break;
		}
	}

	if (!theme) {
		ALLOCT(theme, alloc);
		pthread_mutex_init(&theme->lock, NULL);
		theme->user_alloc = alloc;
		theme->alloc.alloc = xdg_icon_theme__theme_alloc_alloc;
		arena_init(&theme->arena, alloc, 67108864);
		su_hash_table__xdg_icon_theme__icon_t__init(&theme->icons, alloc, 65536);
		string_init_string(&theme->name, &theme->alloc, name);
		theme->next = cache->themes;

		cache->themes = theme;
	}

	/* TODO: rework */
	pthread_mutex_lock(&theme->lock);
	string_init_string(&theme->path, &theme->alloc, path);
	pthread_mutex_unlock(&theme->lock);

	if ((cache->threads_count < LENGTH(cache->threads)) &&
			(0 == pthread_create(&th, NULL, xdg_icon_theme__theme_populate_thread, theme))) {
		cache->threads[cache->threads_count++] = th;
	} else {
		DEBUG_LOG("warning: failed to create thread / thread pool is full. threads_count = %lu", cache->threads_count);
		xdg_icon_theme__theme_populate(theme, path);
	}

	return TRUE;
}

static bool32_t xdg_icon_theme_cache_add_basedir(xdg_icon_theme_cache_t *cache,
		const allocator_t *alloc, string_t path) {
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

	for ( i = (cache->basedirs_count - 1); i != SIZE_MAX; --i) {
		if (string_equal(path, cache->basedirs[i])) {
			return FALSE;
		}
	}

	if (!(dir = opendir(path.s))) {
		return FALSE;
	}

    if (UNLIKELY(cache->basedirs_capacity == cache->basedirs_count)) {
		string_t *new_basedirs;
		cache->basedirs_capacity *= 2;
		ARRAY_ALLOC(new_basedirs, alloc, cache->basedirs_capacity);
		MEMCPY(new_basedirs, cache->basedirs,
			sizeof(new_basedirs[0]) * cache->basedirs_count);
		FREE(alloc, cache->basedirs);
		cache->basedirs = new_basedirs;
    }
	string_init_string(&cache->basedirs[cache->basedirs_count++], alloc, path);

	dir_fd = dirfd(dir);
	while ((e = readdir(dir))) {
		char buf[PATH_MAX];
		string_t new_path, name;
		size_t d_name_len;

		if ((e->d_name[0] == '.') && ((e->d_name[1] == '\0') ||
				((e->d_name[1] == '.') && (e->d_name[2] == '\0')))) {
			continue;
		}

		d_name_len = STRLEN(e->d_name);
		if (UNLIKELY((path.len + d_name_len + 2) > PATH_MAX)) {
			continue;
		}

		MEMCPY(buf, path.s, path.len);
		buf[path.len] = '/';
		MEMCPY(&buf[path.len + 1], e->d_name, d_name_len + 1);

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

static void xdg_icon_theme_cache_init(xdg_icon_theme_cache_t *cache, const allocator_t *alloc) {
	char *data_home = getenv("XDG_DATA_HOME");
	char *data_dirs = getenv("XDG_DATA_DIRS");
	static char buf[PATH_MAX];

	MEMSET(cache, 0, sizeof(*cache));

	arena_init(&cache->unthemed.arena, alloc, 67108864);
	cache->unthemed.alloc.alloc = xdg_icon_theme__theme_alloc_alloc;
	cache->unthemed.user_alloc = alloc;
	su_hash_table__xdg_icon_theme__icon_t__init(&cache->unthemed.icons, alloc, 65536);
	pthread_mutex_init(&cache->unthemed.lock, NULL);
	cache->basedirs_capacity = 32;
	ARRAY_ALLOC(cache->basedirs, alloc, cache->basedirs_capacity);

	if (data_home && *data_home) {
		size_t len = STRLEN(data_home);
		if ((len + STRING_LITERAL_LENGTH("/icons") + 1) <= sizeof(buf)) {
			string_t s;
			s.s = buf;
			s.len = len + STRING_LITERAL_LENGTH("/icons");
			s.nul_terminated = TRUE;
			s.free_contents = FALSE;
			MEMCPY(buf, data_home, len);
			MEMCPY(&buf[len], "/icons", STRING_LITERAL_LENGTH("/icons") + 1);
			xdg_icon_theme_cache_add_basedir(cache, alloc, s);
		}
	} else {
		char *home = getenv("HOME");
		if (home && *home) {
			size_t len = STRLEN(home);
			if ((len + STRING_LITERAL_LENGTH("/.local/share/icons") + 1) <= sizeof(buf)) {
				string_t s;
				s.s = buf;
				s.len = len + STRING_LITERAL_LENGTH("/.local/share/icons");
				s.nul_terminated = TRUE;
				s.free_contents = FALSE;
				MEMCPY(buf, home, len);
				MEMCPY(&buf[len], "/.local/share/icons", STRING_LITERAL_LENGTH("/.local/share/icons") + 1);
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
				MEMCPY(buf, dir.s, dir.len);
				MEMCPY(&buf[dir.len], "/icons", STRING_LITERAL_LENGTH("/icons") + 1);
				xdg_icon_theme_cache_add_basedir(cache, alloc, s);
			}
		}
	} else {
		xdg_icon_theme_cache_add_basedir(cache, alloc, string("/usr/local/share/icons"));
		xdg_icon_theme_cache_add_basedir(cache, alloc, string("/usr/share/icons"));
	}

	xdg_icon_theme_cache_add_basedir(cache, alloc, string("/usr/share/pixmaps"));
}

static void xdg_icon_theme_cache_fini(xdg_icon_theme_cache_t *cache, const allocator_t *alloc) {
	size_t i;
	xdg_icon_theme__theme_t *theme;

	for ( i = 0; i < cache->threads_count; ++i) {
		pthread_join(cache->threads[i], NULL); /* ? TODO: pthread_cancel */
	}

	for ( i = 0; i < cache->basedirs_count; ++i) {
		string_fini(&cache->basedirs[i], alloc);
	}
	FREE(alloc, cache->basedirs);

	arena_fini(&cache->unthemed.arena, alloc);
	pthread_mutex_destroy(&cache->unthemed.lock);
	su_hash_table__xdg_icon_theme__icon_t__fini(&cache->unthemed.icons, alloc);

	for ( theme = cache->themes; theme; ) {
		xdg_icon_theme__theme_t *next = theme->next;
		arena_fini(&theme->arena, alloc);
		pthread_mutex_destroy(&theme->lock);
		su_hash_table__xdg_icon_theme__icon_t__fini(&theme->icons, alloc);
		FREE(alloc, theme);
		theme = next;
	}
}

static bool32_t xdg_icon_theme__find_icon(
		string_t **svgs_out, size_t *svgs_count_out,
		string_t **pngs_out, size_t *pngs_count_out,
		xdg_icon_theme__theme_t *theme, string_t icon_name) {
	bool32_t ret = FALSE;
	xdg_icon_theme__icon_t *c;
	
	NOTUSED(svgs_out); NOTUSED(svgs_count_out);
	NOTUSED(pngs_out); NOTUSED(pngs_count_out);

	if (su_hash_table__xdg_icon_theme__icon_t__get(&theme->icons, icon_name , &c)) {
#if WITH_SVG
		if (c->svg_paths_count > 0) {
			*svgs_out = c->svg_paths;
			*svgs_count_out = c->svg_paths_count;
			ret = TRUE;
		}
#endif /* WITH_SVG */
#if WITH_PNG
		if (c->png_paths_count > 0) {
			ret = TRUE;
			*pngs_out = c->png_paths;
			*pngs_count_out = c->png_paths_count;
		}
#endif /* WITH_PNG */
	}

	return ret;
}

static bool32_t xdg_icon_theme__cache_find_icon_in_theme(xdg_icon_theme_cache_t *cache,
		string_t **svgs_out, size_t *svgs_count_out,
		string_t **pngs_out, size_t *pngs_count_out,
		string_t icon_name, string_t theme_name) {
	xdg_icon_theme__theme_t *theme = cache->themes;
	for ( ; theme; theme = theme->next) {
		if (string_equal(theme->name, theme_name)) {
			return xdg_icon_theme__find_icon(
				svgs_out, svgs_count_out, pngs_out, pngs_count_out, theme, icon_name);
		}
	}

	return FALSE;
}

static bool32_t xdg_icon_theme_cache_find_icon(xdg_icon_theme_cache_t *cache,
		string_t **svgs_out, size_t *svgs_count_out,
		string_t **pngs_out, size_t *pngs_count_out,
		string_t icon_name, string_t *theme_name) {
	xdg_icon_theme__theme_t *theme;

	size_t i = 0;
	for ( ; i < cache->threads_count; ++i) {
		pthread_join(cache->threads[i], NULL);
	}
	cache->threads_count = 0;

	/* TODO: check mtime / inotify/alternative */

	if (theme_name && xdg_icon_theme__cache_find_icon_in_theme(
			cache, svgs_out, svgs_count_out, pngs_out, pngs_count_out, icon_name, *theme_name)) {
		return TRUE;
	}
	if (!theme_name || !string_equal(*theme_name, string("hicolor"))) {
		if (xdg_icon_theme__cache_find_icon_in_theme(
				cache, svgs_out, svgs_count_out, pngs_out, pngs_count_out, icon_name, string("hicolor"))) {
			return TRUE;
		}
	}
	if (xdg_icon_theme__find_icon(
			svgs_out, svgs_count_out, pngs_out, pngs_count_out, &cache->unthemed, icon_name)) {
		return TRUE;
	}
	for ( theme = cache->themes; theme; theme = theme->next) {
		if (xdg_icon_theme__find_icon(
				svgs_out, svgs_count_out, pngs_out, pngs_count_out, theme, icon_name)) {
			return TRUE;
		}
	}

	return FALSE;
}

#endif /* XDG_ICON_THEME_H */
