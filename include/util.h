#if !defined(UTIL_H)
#define UTIL_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include <immintrin.h>

#include "macros.h"

IGNORE_WARNINGS_START

IGNORE_WARNING("-Wcast-align")
IGNORE_WARNING("-Wextra-semi-stmt")
IGNORE_WARNING("-Wsign-conversion")
IGNORE_WARNING("-Wdouble-promotion")
IGNORE_WARNING("-Wpadded")
IGNORE_WARNING("-Wimplicit-fallthrough")
IGNORE_WARNING("-Wconditional-uninitialized")
IGNORE_WARNING("-Wsign-compare")
IGNORE_WARNING("-Warith-conversion")
IGNORE_WARNING("-Wconversion")

#define STB_SPRINTF_NOUNALIGNED
#define STB_SPRINTF_MIN 128
#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>

IGNORE_WARNINGS_END

typedef uint32_t bool32_t;
#define true 1
#define false 0

static char *log_stderr_va_stbsp_vsprintfcb_callback(const char *buf, void *data, int len) {
	(void)data;
	ssize_t total = 0;
	while (total < len) {
		ssize_t written_bytes = write(STDERR_FILENO, &buf[total], (size_t)(len - total));
		if (written_bytes == -1) {
			return NULL;
		}
		total += written_bytes;
	}

	return (char *)buf;
}

static ATTRIB_FORMAT_PRINTF(1, 0) void log_stderr_va(const char *fmt, va_list args) {
	ssize_t suppress_warn_unused_result = write(STDERR_FILENO, PREFIX ": ", STRING_LITERAL_LENGTH(PREFIX ": "));
	char buf[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(log_stderr_va_stbsp_vsprintfcb_callback, NULL, buf, fmt, args);
	suppress_warn_unused_result = write(STDERR_FILENO, "\n", 1);
	(void)suppress_warn_unused_result;
}

static ATTRIB_FORMAT_PRINTF(1, 2) void log_stderr(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_stderr_va(fmt, args);
	va_end(args);
}

static ATTRIB_NORETURN ATTRIB_FORMAT_PRINTF(2, 3) void abort_(int code, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
    log_stderr_va(fmt, args);
	va_end(args);
    exit(code);
}

#define TIMER(name) \
	struct timespec name##_ts; \
	clock_gettime(CLOCK_MONOTONIC, &name##_ts); \
	double name = (double)name##_ts.tv_sec + (double)name##_ts.tv_nsec * 1e-9

#if DEBUG
#define DEBUG_LOG log_stderr
#else
#define DEBUG_LOG(...)
#endif // DEBUG

#define ARRAY_DECLARE(type) \
typedef struct { \
    size_t size, len; \
    type *items; \
} array_##type##_t; \
\
static void array_##type##_init(array_##type##_t *, size_t initial_size); \
static void array_##type##_fini(array_##type##_t *); \
static void array_##type##_resize(array_##type##_t *, size_t new_size); \
static type *array_##type##_add(array_##type##_t *, type item); \
static type *array_##type##_add_uninitialized(array_##type##_t *); \
static void array_##type##_remove(array_##type##_t *, size_t n); \
static void array_##type##_swap(array_##type##_t *, size_t idx1, size_t idx2); \
static void array_##type##_insert(array_##type##_t *, type item, size_t idx); \
static void array_##type##_pop(array_##type##_t *, size_t idx); \
static void array_##type##_pop_swapback(array_##type##_t *, size_t idx); \
static type *array_##type##_put(array_##type##_t *, type item, size_t idx); \
static type array_##type##_get(array_##type##_t *, size_t idx); \
static type *array_##type##_get_ptr(array_##type##_t *, size_t idx); \
static void array_##type##_clear(array_##type##_t *); \
static void array_##type##_qsort(array_##type##_t *, int compare(const void *, const void *));

#define ARRAY_DEFINE(type) \
static void array_##type##_init(array_##type##_t *array, size_t initial_size) { \
    assert(initial_size > 0); \
    array->items = malloc(sizeof(type) * initial_size); \
	array->size = initial_size; \
	array->len = 0; \
} \
\
static void array_##type##_fini(array_##type##_t *array) { \
	if (array) { \
		free(array->items); \
	} \
} \
\
static void array_##type##_resize(array_##type##_t *array, size_t new_size) { \
    assert(new_size > 0); \
    array->items = realloc(array->items, sizeof(type) * new_size); \
    array->size = new_size; \
} \
\
static type *array_##type##_add(array_##type##_t *array, type item) { \
    if (array->size == array->len) { \
        array_##type##_resize(array, 8 + array->size * 2); \
    } \
    array->items[array->len] = item; \
	return &array->items[array->len++];\
} \
static type *array_##type##_add_uninitialized(array_##type##_t *array) { \
    if (array->size == array->len) { \
        array_##type##_resize(array, 8 + array->size * 2); \
    } \
	return &array->items[array->len++];\
} \
\
static void array_##type##_remove(array_##type##_t *array, size_t n) { \
	assert(array->len > 0); \
    array->len -= n; \
} \
\
static void array_##type##_swap(array_##type##_t *array, size_t idx1, size_t idx2) { \
	assert(idx1 < array->len); \
	assert(idx2 < array->len); \
	type tmp = array->items[idx1]; \
	array->items[idx1] = array->items[idx2]; \
	array->items[idx2] = tmp; \
} \
\
static void array_##type##_insert(array_##type##_t *array, type item, size_t idx) { \
    assert(idx <= array->len); \
    if (array->size == array->len) { \
        array_##type##_resize(array, array->size * 2); \
    } \
	memmove(&array->items[idx + 1], &array->items[idx], sizeof(type) * (array->len - idx)); \
	array->len++; \
	array->items[idx] = item; \
} \
\
static void array_##type##_pop(array_##type##_t *array, size_t idx) { \
    assert(idx < array->len); \
	array->len--; \
	memmove(&array->items[idx], &array->items[idx + 1], sizeof(type) * (array->len - idx)); \
} \
\
static void array_##type##_pop_swapback(array_##type##_t *array, size_t idx) { \
    assert(idx < array->len); \
	array->len--; \
	array->items[idx] = array->items[array->len]; \
} \
\
static type *array_##type##_put(array_##type##_t *array, type item, size_t idx) { \
    assert(idx <= array->len); \
    if (idx == array->len) { \
        if (array->size == array->len) { \
            array_##type##_resize(array, array->size * 2); \
        } \
        array->len++; \
    } \
	array->items[idx] = item; \
	return &array->items[idx]; \
} \
\
static ATTRIB_PURE type array_##type##_get(array_##type##_t *array, size_t idx) { \
    assert(idx < array->len); \
	return array->items[idx]; \
} \
\
static ATTRIB_PURE type *array_##type##_get_ptr(array_##type##_t *array, size_t idx) { \
    assert(idx < array->len); \
	return &array->items[idx]; \
} \
\
static void array_##type##_clear(array_##type##_t *array) { \
	memset(array->items, 0, array->len * sizeof(type)); \
} \
\
static void array_##type##_qsort(array_##type##_t *array, int compare(const void *, const void *)) { \
    qsort(array->items, array->len, sizeof(type), compare); \
}

#define ARRAY_DECLARE_DEFINE(type) \
	ARRAY_DECLARE(type) \
	ARRAY_DEFINE(type)

typedef char* char_ptr;
ARRAY_DECLARE_DEFINE(char_ptr)

#define STACK_DECLARE(type) \
\
ARRAY_DECLARE(type) \
\
typedef struct { \
	array_##type##_t data; \
} stack_##type##_t; \
\
static void stack_##type##_init(stack_##type##_t *stack, size_t initial_size); \
static void stack_##type##_fini(stack_##type##_t *stack); \
static type *stack_##type##_push(stack_##type##_t *stack, type item); \
static type stack_##type##_pop(stack_##type##_t *stack); \
static type stack_##type##_get(stack_##type##_t *stack);

#define STACK_DEFINE(type) \
\
ARRAY_DEFINE(type) \
\
static void stack_##type##_init(stack_##type##_t *stack, size_t initial_size) { \
	array_##type##_init(&stack->data, initial_size); \
} \
\
static void stack_##type##_fini(stack_##type##_t *stack) { \
	array_##type##_fini(&stack->data); \
} \
\
static type *stack_##type##_push(stack_##type##_t *stack, type item) { \
	return array_##type##_add(&stack->data, item); \
} \
\
static type stack_##type##_pop(stack_##type##_t *stack) { \
	type ret = array_##type##_get(&stack->data, stack->data.len - 1); \
	array_##type##_remove(&stack->data, 1); \
	return ret; \
} \
\
static ATTRIB_PURE type stack_##type##_get(stack_##type##_t *stack) { \
	return array_##type##_get(&stack->data, stack->data.len - 1); \
}

#define STACK_DECLARE_DEFINE(type) \
	STACK_DECLARE(type) \
	STACK_DEFINE(type)

#define ARENA_STACK_DECLARE(type) \
\
ARENA_ARRAY_DECLARE(type) \
\
typedef struct { \
	arena_array_##type##_t data; \
} arena_stack_##type##_t; \
\
static void arena_stack_##type##_init(arena_stack_##type##_t *stack, arena_t *arena, size_t initial_size); \
static void arena_stack_##type##_fini(arena_stack_##type##_t *stack); \
static type *arena_stack_##type##_push(arena_stack_##type##_t *stack, arena_t *arena, type item); \
static type arena_stack_##type##_pop(arena_stack_##type##_t *stack); \
static type arena_stack_##type##_get(arena_stack_##type##_t *stack);

#define ARENA_STACK_DEFINE(type) \
\
ARENA_ARRAY_DEFINE(type) \
\
static void arena_stack_##type##_init(arena_stack_##type##_t *stack, arena_t *arena, size_t initial_size) { \
	arena_array_##type##_init(&stack->data, arena, initial_size); \
} \
\
static void arena_stack_##type##_fini(arena_stack_##type##_t *stack) { \
	arena_array_##type##_fini(&stack->data); \
} \
\
static type *arena_stack_##type##_push(arena_stack_##type##_t *stack, arena_t *arena, type item) { \
	return arena_array_##type##_add(&stack->data, arena, item); \
} \
\
static type arena_stack_##type##_pop(arena_stack_##type##_t *stack) { \
	type ret = arena_array_##type##_get(&stack->data, stack->data.len - 1); \
	arena_array_##type##_remove(&stack->data, 1); \
	return ret; \
} \
\
static ATTRIB_PURE type arena_stack_##type##_get(arena_stack_##type##_t *stack) { \
	return arena_array_##type##_get(&stack->data, stack->data.len - 1); \
}

#define ARENA_STACK_DECLARE_DEFINE(type) \
	ARENA_STACK_DECLARE(type) \
	ARENA_STACK_DEFINE(type)

typedef struct {
	bool32_t free_contents;
	bool32_t nul_terminated;
	size_t len; // excluding '\0'
	char *s;
} string_t;

ARRAY_DECLARE_DEFINE(string_t)
#define STRING_LITERAL(str) ((string_t){ .s = (str), .len = STRING_LITERAL_LENGTH((str)), .free_contents = false, .nul_terminated = true })

#define STRING_FMT "%.*s"
#define STRING_ARGS(str) (int)(str).len, (str).s

static char *string_init_format_stbsp_vsprintfcb_callback(const char *buf, void *data, int len) {
	(void)buf;
	string_t *str = data;
	str->len += (size_t)len;

	if (len == STB_SPRINTF_MIN) {
		// TODO: handle last cb when len == STB_SPRINTF_MIN
		str->s = realloc(str->s, str->len + STB_SPRINTF_MIN);
	}

	str->s[str->len] = '\0';
	return &str->s[str->len];
}

static ATTRIB_FORMAT_PRINTF(2, 3) void string_init_format(string_t *str, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	str->len = 0;
	str->s = malloc(STB_SPRINTF_MIN);
	stbsp_vsprintfcb(string_init_format_stbsp_vsprintfcb_callback, str, str->s, fmt, args);
	str->free_contents = true;
	str->nul_terminated = true;

	va_end(args);
}

static void string_init_len(string_t *str, const char *s, size_t len, bool32_t nul_terminate) {
	assert((len > 0) || nul_terminate);
	str->s = malloc(len + nul_terminate);
	memcpy(str->s, s, len);
	if (nul_terminate) {
		str->s[len] = '\0';
	}
	str->len = len;
	str->free_contents = true;
	str->nul_terminated = nul_terminate;
}

static void string_init_string(string_t *str, string_t src) {
	string_init_len(str, src.s, src.len, src.nul_terminated);
}

static void string_init(string_t *str, const char *src) {
	str->len = strlen(src);
	str->s = malloc(str->len + 1);
	memcpy(str->s, src, str->len);
	str->free_contents = true;
	str->nul_terminated = true;
}

static void string_fini(string_t *str) {
	if (str && str->free_contents) {
		free(str->s);
	}
}

// TODO: rename
static string_t string_copy(string_t str) {
	return (string_t){
		.s = str.s,
		.len = str.len,
		.nul_terminated = str.nul_terminated,
		.free_contents = false,
	};
}

static bool32_t string_equal(string_t str1, string_t str2) {
	if (str1.len != str2.len) {
		return false;
	}

	return (memcmp(str1.s, str2.s, str1.len) == 0);
}

static int string_compare(string_t str1, string_t str2, size_t max) {
    size_t len = ((max < str1.len) && (max < str2.len)) ? max : MIN(str1.len, str2.len);

    int result = memcmp(str1.s, str2.s, len);
    if ((result != 0) || (len == max)) {
		return result;
	}

    return ((str1.len < str2.len) ? -1 : ((str1.len > str2.len) ? 1 : 0));
}

static bool32_t string_find_char(string_t str, char c, string_t *slice_out) {
	char *s = memchr(str.s, c, str.len);
	if (!s) {
		return false;
	}

	*slice_out = (string_t){
		.s = s,
		.len = (size_t)((uintptr_t)&str.s[str.len] - (uintptr_t)s),
		.free_contents = false,
		.nul_terminated = str.nul_terminated,
	};

	return true;
}

static bool32_t string_tok(string_t *str, char delim, string_t *token_out, string_t *saveptr) {
	// TODO: simd

    if (str) {
		assert(str->s != NULL);
		assert(str->len > 0);
		*saveptr = (string_t){
			.s = str->s,
			.len = str->len,
			.free_contents = false,
			.nul_terminated = false, // TODO: return str->nul_terminated on last token
		};
	}

    while ((saveptr->len > 0) && (*saveptr->s == delim)) {
        saveptr->s++;
        saveptr->len--;
    }
    if (saveptr->len == 0) {
		return false;
	}

    *token_out = *saveptr;
    while ((saveptr->len > 0) && (*saveptr->s != delim)) {
        saveptr->s++;
        saveptr->len--;
    }

	token_out->len -= saveptr->len;
	return true;
}

static bool32_t string_starts_with(string_t str, string_t prefix) {
	if (prefix.len <= str.len) {
		return string_equal(prefix, (string_t){
			.s = str.s,
			.len = prefix.len,
			.free_contents = false,
			.nul_terminated = false,
		});
	}

	return false;
}

static bool32_t string_ends_with(string_t str, string_t suffix) {
	if (suffix.len <= str.len) {
		return string_equal(suffix, (string_t){
			.s = &str.s[str.len - suffix.len],
			.len = suffix.len,
			.free_contents = false,
			.nul_terminated = str.nul_terminated,
		});
	}

	return false;
}

// TODO: remove
static bool32_t string_hex_to_uint32(uint32_t *out, string_t str) {
	// TODO: handle overflows/underflows

	uint32_t result = 0;

	for (size_t i = 0; i < str.len; ++i) {
		char c = str.s[i];
		if ((c >= 'a') && (c <= 'f')) {
			result *= 16;
			result += (uint32_t)(10 + c - 'a');
		} else if ((c >= 'A') && (c <= 'F')) {
			result *= 16;
			result += (uint32_t)(10 + c - 'A');
		} else if ((c >= '0') && (c <= '9')) {
			result *= 16;
			result += (uint32_t)(c - '0');
		} else {
			return false;
		}
	}

	*out = result;
	return true;
}

// on success, returns u16 in lower bits of u32 result
// on error, returns u32 with high bits set
static ATTRIB_PURE uint32_t string_hex_16_to_uint16(string_t str) {
	assert(str.len == 4);

	static const uint32_t table[886] = {
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0x0,        0x1,        0x2,        0x3,        0x4,        0x5,
		0x6,        0x7,        0x8,        0x9,        0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa,
		0xb,        0xc,        0xd,        0xe,        0xf,        0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xa,        0xb,        0xc,        0xd,        0xe,
		0xf,        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0x0,        0x10,       0x20,       0x30,       0x40,       0x50,
		0x60,       0x70,       0x80,       0x90,       0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa0,
		0xb0,       0xc0,       0xd0,       0xe0,       0xf0,       0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xa0,       0xb0,       0xc0,       0xd0,       0xe0,
		0xf0,       0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0x0,        0x100,      0x200,      0x300,      0x400,      0x500,
		0x600,      0x700,      0x800,      0x900,      0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa00,
		0xb00,      0xc00,      0xd00,      0xe00,      0xf00,      0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xa00,      0xb00,      0xc00,      0xd00,      0xe00,
		0xf00,      0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0x0,        0x1000,     0x2000,     0x3000,     0x4000,     0x5000,
		0x6000,     0x7000,     0x8000,     0x9000,     0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xa000,
		0xb000,     0xc000,     0xd000,     0xe000,     0xf000,     0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xa000,     0xb000,     0xc000,     0xd000,     0xe000,
		0xf000,     0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
	};

	return  table[630 + (uint8_t)str.s[0]] |
			table[420 + (uint8_t)str.s[1]] |
			table[210 + (uint8_t)str.s[2]] |
			table[0   + (uint8_t)str.s[3]] ;
}

static bool32_t string_to_uint64(string_t str, uint64_t *out) {
	if (UNLIKELY((str.len == 0) || (str.len > 20) || ((str.s[0] == '0') && (str.len > 1)))) {
		return false;
	}

	uint64_t result = 0;
	for (size_t i = 0; i < str.len; ++i) {
		uint8_t d = (uint8_t)(str.s[i] - '0');
		if (UNLIKELY(d > 9)) {
			return false;
		}
		result = 10 * result + d;
	}

	if (UNLIKELY((str.len == 20) && (str.s[0] == '1') && (result <= (uint64_t)INT64_MAX))) {
		return false;
	}

	*out = result;
	return true;
}

static bool32_t string_to_int64(string_t str, int64_t *out) {
	if (UNLIKELY((str.len == 0) || (str.len > 20) || ((str.s[0] == '0') && (str.len > 1)))) {
		return false;
	}

	bool32_t negative = (str.s[0] == '-');
	uint64_t result = 0;
	for (size_t i = negative; i < str.len; ++i) {
		uint8_t d = (uint8_t)(str.s[i] - '0');
		if (UNLIKELY(d > 9)) {
			return false;
		}
		result = 10 * result + d;
	}

	if (UNLIKELY(result > ((uint64_t)INT64_MAX + negative))) {
		return false;
	}

	*out = negative ? (int64_t)(~result + 1) : (int64_t)result;
	return true;
}

#define LIST_DECLARE(type) \
typedef struct { \
    type *head; \
    type *tail; \
    size_t len; \
} list_##type##_t; \
\
static void list_##type##_insert_head(list_##type##_t *, type *node); \
static void list_##type##_insert_tail(list_##type##_t *, type *node); \
static void list_##type##_pop(list_##type##_t *, type *node);

#define LIST_DEFINE(type) \
static void list_##type##_insert_head(list_##type##_t *list, type *node) { \
    node->prev = NULL; \
    node->next = list->head; \
    if (list->head) { \
        list->head->prev = node; \
    } else { \
        list->tail = node; \
    } \
    list->head = node; \
    list->len++; \
} \
\
static void list_##type##_insert_tail(list_##type##_t *list, type *node) { \
    node->next = NULL; \
    node->prev = list->tail; \
    if (list->tail) { \
        list->tail->next = node; \
    } else { \
        list->head = node; \
    } \
    list->tail = node; \
    list->len++; \
} \
\
static void list_##type##_pop(list_##type##_t *list, type *node) { \
    if (node->prev) { \
        node->prev->next = node->next; \
    } else { \
        list->head = node->next; \
    } \
    if (node->next) { \
        node->next->prev = node->prev; \
    } else { \
        list->tail = node->prev; \
    } \
    list->len--; \
}

// ? TODO: for_each_ safe, reverse  macro
// TODO: insert_ list, after, before

#define LIST_DECLARE_DEFINE(type) \
	LIST_DECLARE(type) \
	LIST_DEFINE(type)

#define LIST_STRUCT_FIELDS(type) \
	type *next; \
	type *prev;

static void *aalloc(size_t alignment, size_t size) {
	uint32_t *ptr = aligned_alloc(alignment, (size + alignment - 1) & ~(alignment - 1));
	assert(ptr != NULL);
	assert(((uintptr_t)ptr % alignment) == 0);
	return ptr;
}

typedef struct {
	size_t ptr;
	size_t size;
	uint8_t *data;
} arena_block_t;

ARRAY_DECLARE_DEFINE(arena_block_t)

typedef struct {
	array_arena_block_t_t blocks;
} arena_t;

static void arena_init(arena_t *arena) {
	array_arena_block_t_init(&arena->blocks, 8);
}

static void arena_fini(arena_t *arena) {
	for (size_t i = 0; i < arena->blocks.len; ++i) {
		free(array_arena_block_t_get(&arena->blocks, i).data);
	}
	array_arena_block_t_fini(&arena->blocks);
}

static void *arena_alloc(arena_t *arena, size_t size) {
	if (size == 0) { // ? TODO: assert
		return NULL;
	}

	// ? TODO: specify alignment for each alloc
	size = (size + ALIGNOF(max_align_t) - 1) & ~(ALIGNOF(max_align_t) - 1);

	arena_block_t *block;
	size_t new_size = size;
	for (size_t i = 0; i < arena->blocks.len; ++i) {
		block = array_arena_block_t_get_ptr(&arena->blocks, i);
		if (size <= (block->size - block->ptr)) {
			goto out;
		}
		new_size += block->size;
	}

	block = array_arena_block_t_add(&arena->blocks, (arena_block_t){
		.data = aalloc(4096, new_size),
		.size = new_size,
		.ptr = 0,
	});

out: {
		void *ret = &block->data[block->ptr];
		block->ptr += size;
		return ret;
	}
}

static void arena_reset(arena_t *arena) {
	if (arena->blocks.len == 0) { // ? TODO: assert
		return;
	}

	arena_block_t *first_block = array_arena_block_t_get_ptr(&arena->blocks, 0);
	size_t size = first_block->size;
	for (size_t i = 1; i < arena->blocks.len; ++i) {
		arena_block_t block = array_arena_block_t_get(&arena->blocks, i);
		size += block.size;
		free(block.data);
	}
	arena->blocks.len = 1;

	if (size > first_block->size) {
		free(first_block->data);
		first_block->data = aalloc(4096, size);
		first_block->size = size;
	}
	first_block->ptr = 0;
}

#define ARENA_ARRAY_DECLARE(type) \
typedef struct { \
    size_t size, len; \
    type *items; \
} arena_array_##type##_t; \
\
static void arena_array_##type##_init(arena_array_##type##_t *, arena_t *arena, size_t initial_size); \
static void arena_array_##type##_fini(arena_array_##type##_t *); \
static void arena_array_##type##_resize(arena_array_##type##_t *, arena_t *arena, size_t new_size); \
static type *arena_array_##type##_add(arena_array_##type##_t *, arena_t *arena, type item); \
static type *arena_array_##type##_add_uninitialized(arena_array_##type##_t *, arena_t *arena); \
static void arena_array_##type##_remove(arena_array_##type##_t *, size_t n); \
static void arena_array_##type##_swap(arena_array_##type##_t *, size_t idx1, size_t idx2); \
static void arena_array_##type##_insert(arena_array_##type##_t *, arena_t *arena, type item, size_t idx); \
static void arena_array_##type##_pop(arena_array_##type##_t *, size_t idx); \
static void arena_array_##type##_pop_swapback(arena_array_##type##_t *, size_t idx); \
static type *arena_array_##type##_put(arena_array_##type##_t *, arena_t *arena, type item, size_t idx); \
static type arena_array_##type##_get(arena_array_##type##_t *, size_t idx); \
static type *arena_array_##type##_get_ptr(arena_array_##type##_t *, size_t idx); \
static void arena_array_##type##_clear(arena_array_##type##_t *); \
static void arena_array_##type##_qsort(arena_array_##type##_t *, int compare(const void *, const void *));

#define ARENA_ARRAY_DEFINE(type) \
static void arena_array_##type##_init(arena_array_##type##_t *array, arena_t *arena, size_t initial_size) { \
    array->items = arena_alloc(arena, sizeof(type) * initial_size); \
	array->size = initial_size; \
	array->len = 0; \
} \
\
static void arena_array_##type##_fini(arena_array_##type##_t *array) { \
	(void)array; \
} \
\
static void arena_array_##type##_resize(arena_array_##type##_t *array, arena_t *arena, size_t new_size) { \
	type *new_items = arena_alloc(arena, sizeof(type) * new_size); \
	if (new_items && array->items) { \
		memcpy(new_items, array->items, sizeof(type) * array->len); \
	} \
    array->items = new_items; \
    array->size = new_size; \
} \
\
static type *arena_array_##type##_add(arena_array_##type##_t *array, arena_t *arena, type item) { \
    if (array->size == array->len) { \
        arena_array_##type##_resize(array, arena, 8 + array->size * 2); \
    } \
    array->items[array->len] = item; \
	return &array->items[array->len++];\
} \
static type *arena_array_##type##_add_uninitialized(arena_array_##type##_t *array, arena_t *arena) { \
    if (array->size == array->len) { \
        arena_array_##type##_resize(array, arena, 8 + array->size * 2); \
    } \
	return &array->items[array->len++];\
} \
\
static void arena_array_##type##_remove(arena_array_##type##_t *array, size_t n) { \
	assert(array->len > 0); \
    array->len -= n; \
} \
\
static void arena_array_##type##_swap(arena_array_##type##_t *array, size_t idx1, size_t idx2) { \
	assert(idx1 < array->len); \
	assert(idx2 < array->len); \
	type tmp = array->items[idx1]; \
	array->items[idx1] = array->items[idx2]; \
	array->items[idx2] = tmp; \
} \
\
static void arena_array_##type##_insert(arena_array_##type##_t *array, arena_t *arena, type item, size_t idx) { \
    assert(idx <= array->len); \
    if (array->size == array->len) { \
        arena_array_##type##_resize(array, arena, array->size * 2); \
    } \
	memmove(&array->items[idx + 1], &array->items[idx], sizeof(type) * (array->len - idx)); \
	array->len++; \
	array->items[idx] = item; \
} \
\
static void arena_array_##type##_pop(arena_array_##type##_t *array, size_t idx) { \
    assert(idx < array->len); \
	array->len--; \
	memmove(&array->items[idx], &array->items[idx + 1], sizeof(type) * (array->len - idx)); \
} \
\
static void arena_array_##type##_pop_swapback(arena_array_##type##_t *array, size_t idx) { \
    assert(idx < array->len); \
	array->len--; \
	array->items[idx] = array->items[array->len]; \
} \
\
static type *arena_array_##type##_put(arena_array_##type##_t *array, arena_t *arena, type item, size_t idx) { \
    assert(idx <= array->len); \
    if (idx == array->len) { \
        if (array->size == array->len) { \
            arena_array_##type##_resize(array, arena, array->size * 2); \
        } \
        array->len++; \
    } \
	array->items[idx] = item; \
	return &array->items[idx]; \
} \
\
static ATTRIB_PURE type arena_array_##type##_get(arena_array_##type##_t *array, size_t idx) { \
    assert(idx < array->len); \
	return array->items[idx]; \
} \
\
static ATTRIB_PURE type *arena_array_##type##_get_ptr(arena_array_##type##_t *array, size_t idx) { \
    assert(idx < array->len); \
	return &array->items[idx]; \
} \
\
static void arena_array_##type##_clear(arena_array_##type##_t *array) { \
	memset(array->items, 0, array->len * sizeof(type)); \
} \
\
static void arena_array_##type##_qsort(arena_array_##type##_t *array, int compare(const void *, const void *)) { \
    qsort(array->items, array->len, sizeof(type), compare); \
}

#define ARENA_ARRAY_DECLARE_DEFINE(type) \
	ARENA_ARRAY_DECLARE(type) \
	ARENA_ARRAY_DEFINE(type)

//static ATTRIB_PURE size_t sdbm_hash(string_t s) {
//    size_t hash = 0;
//    for (size_t i = 0; i < s.len; ++i) {
//        hash = (size_t)s.s[i] + (hash << 6) + (hash << 16) - hash;
//    }
//
//    return hash;
//}

//static ATTRIB_PURE size_t djb2_hash(string_t s) {
//	size_t hash = 5381;
//
//	for (size_t i = 0; i < s.len; ++i) {
//		hash = ((hash << 5) + hash) + (size_t)s.s[i]; // hash * 33 + c
//	}
//
//	return hash;
//}

static ATTRIB_PURE size_t stbds_hash_string(string_t s) {
#define ROTATE_LEFT(val, n)  (((val) << (n)) | ((val) >> (((sizeof(size_t)) * 8) - (n))))
#define ROTATE_RIGHT(val, n) (((val) >> (n)) | ((val) << (((sizeof(size_t)) * 8) - (n))))

	size_t hash = 0;
	for (size_t i = 0; i < s.len; ++i) {
		hash = ROTATE_LEFT(hash, 9) + (unsigned char)s.s[i];
	}

	hash = (~hash) + (hash << 18);
	hash ^= hash ^ ROTATE_RIGHT(hash, 31);
	hash = hash * 21;
	hash ^= hash ^ ROTATE_RIGHT(hash, 11);
	hash += (hash << 6);
	hash ^= ROTATE_RIGHT(hash, 22);

	return hash;

#undef ROTATE_LEFT
#undef ROTATE_RIGHT
}

#define HASH_TABLE_DECLARE(type, hash_key_func, keys_equal_func, collisions_to_resize) \
\
ARRAY_DECLARE(type) \
\
typedef struct { \
	array_##type##_t items; \
} hash_table_##type##_t; \
\
static void hash_table_##type##_init(hash_table_##type##_t *, size_t initial_size); \
static void hash_table_##type##_fini(hash_table_##type##_t *); \
static void hash_table_##type##_resize(hash_table_##type##_t *, size_t new_size); \
static bool32_t hash_table_##type##_add(hash_table_##type##_t *, type in, type **out); \
static bool32_t hash_table_##type##_get(hash_table_##type##_t *, type in, type **out); \
static bool32_t hash_table_##type##_del(hash_table_##type##_t *, type in, type *out);

#define HASH_TABLE_DEFINE(type, hash_key_func, keys_equal_func, collisions_to_resize) \
\
ARRAY_DEFINE(type) \
\
static void hash_table_##type##_init(hash_table_##type##_t *ht, size_t initial_size) { \
	array_##type##_init(&ht->items, initial_size); \
	ht->items.len = initial_size; \
	array_##type##_clear(&ht->items); \
} \
\
static void hash_table_##type##_fini(hash_table_##type##_t *ht) { \
	array_##type##_fini(&ht->items); \
} \
\
static void hash_table_##type##_resize(hash_table_##type##_t *ht, size_t new_size) { \
	hash_table_##type##_t new_ht; \
	hash_table_##type##_init(&new_ht, new_size); \
	for (size_t i = 0; i < ht->items.len; ++i) { \
		type it = array_##type##_get(&ht->items, i); \
		if (it.occupied && !it.tombstone) { \
			hash_table_##type##_add(&new_ht, it, NULL); \
		} \
	} \
	hash_table_##type##_fini(ht); \
	*ht = new_ht; \
} \
\
static bool32_t hash_table_##type##_add(hash_table_##type##_t *ht, type in, type **out) { \
	size_t h = hash_key_func(in.key) % ht->items.len; \
	type *it = array_##type##_get_ptr(&ht->items, h); \
	size_t c = 0; \
	for ( ; \
			it->occupied && !keys_equal_func(it->key, in.key) && (c < ht->items.len); \
			++c) { \
		it = array_##type##_get_ptr(&ht->items, (++h % ht->items.len)); \
	} \
	if (c >= collisions_to_resize) { \
		hash_table_##type##_resize(ht, ht->items.size * 2); \
		hash_table_##type##_add(ht, in, out); \
	} else if (it->occupied) { \
		if (keys_equal_func(it->key, in.key)) { \
			if (out) { \
				*out = it; \
			} \
			return false; \
		} \
		hash_table_##type##_resize(ht, ht->items.size * 2); \
		hash_table_##type##_add(ht, in, out); \
	} else { \
		*it = in; \
		it->occupied = true; \
		if (out) { \
			*out = it; \
		} \
	} \
	return true; \
} \
\
static bool32_t hash_table_##type##_get(hash_table_##type##_t *ht, type in, type **out) { \
	size_t h = hash_key_func(in.key) % ht->items.len; \
	type *it = array_##type##_get_ptr(&ht->items, h); \
	size_t c = 0; \
	for ( ; \
			it->occupied && !keys_equal_func(it->key, in.key) && (c < ht->items.len); \
			++c) { \
		it = array_##type##_get_ptr(&ht->items, (++h % ht->items.len)); \
	} \
	if (it->occupied && keys_equal_func(it->key, in.key)) { \
		*out = it; \
		return true; \
	} else { \
		return false; \
	} \
} \
\
static bool32_t hash_table_##type##_del(hash_table_##type##_t *ht, type in, type *out) { \
	size_t h = hash_key_func(in.key) % ht->items.len; \
	type *it = array_##type##_get_ptr(&ht->items, h); \
	size_t c = 0; \
	for ( ; \
			it->occupied && !keys_equal_func(it->key, in.key) && (c < ht->items.len); \
			++c) { \
		it = array_##type##_get_ptr(&ht->items, (++h % ht->items.len)); \
	} \
	if (it->occupied && keys_equal_func(it->key, in.key)) { \
		if (out) { \
			*out = *it; \
		} \
		*it = (type){ \
			.occupied = true, \
			.tombstone = true, \
		}; \
		return true; \
	} else { \
		return false; \
	} \
}

// ? TODO: tombstone threshold, shrink

#define HASH_TABLE_DECLARE_DEFINE(type, hash_key_func, keys_equal_func, collisions_to_resize) \
	HASH_TABLE_DECLARE(type, hash_key_func, keys_equal_func, collisions_to_resize) \
	HASH_TABLE_DEFINE(type, hash_key_func, keys_equal_func, collisions_to_resize)

#define HASH_TABLE_STRUCT_FIELDS(key_type) \
	key_type key; \
	bool32_t occupied; \
	bool32_t tombstone;

static void argb_premultiply_alpha(uint32_t *src, uint32_t *dest, size_t count) {
	assert(((uintptr_t)src % 32) == 0);
	assert(((uintptr_t)dest % 32) == 0);

	size_t i = 0;

#if defined(__AVX2__)
    __m256i zero = _mm256_setzero_si256();
    __m256i const_127 = _mm256_set1_epi16(127);
    __m256i const_257 = _mm256_set1_epi16(257);

	__m256i extract_alpha_mask = _mm256_set_epi8(
#define ZERO CHAR_MIN
		ZERO, 14, ZERO, 14, ZERO, 14, ZERO, 14,
		ZERO, 6, ZERO, 6, ZERO, 6, ZERO, 6,
		ZERO, 30, ZERO, 30, ZERO, 30, ZERO, 30,
		ZERO, 22, ZERO, 22, ZERO, 22, ZERO, 22
#undef ZERO
	);

	__m256i blend_mask = _mm256_set_epi8(
#define FROM1 0
#define FROM2 CHAR_MIN
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1
#undef FROM1
#undef FROM2
	);

	for ( ; (i + 8) <= count; i += 8) {
		__m256i px = _mm256_load_si256((__m256i *)(void *)&src[i]);

		__m256i lo = _mm256_unpacklo_epi8(px, zero);
		__m256i hi = _mm256_unpackhi_epi8(px, zero);

		__m256i alpha_lo = _mm256_shuffle_epi8(lo, extract_alpha_mask);
		__m256i alpha_hi = _mm256_shuffle_epi8(hi, extract_alpha_mask);

		lo = _mm256_mullo_epi16(lo, alpha_lo);
		hi = _mm256_mullo_epi16(hi, alpha_hi);

		lo = _mm256_add_epi16(lo, const_127);
		hi = _mm256_add_epi16(hi, const_127);

		lo = _mm256_mulhi_epu16(lo, const_257);
		hi = _mm256_mulhi_epu16(hi, const_257);

		__m256i packed = _mm256_packus_epi16(lo, hi);
		__m256i result = _mm256_blendv_epi8(packed, px, blend_mask);

		_mm256_store_si256((__m256i *)(void *)&dest[i], result);
	}
#endif // __AVX2__
// TODO: ARM SIMD

	for ( ; i < count; ++i) {
		uint32_t p = src[i];

		uint8_t a = (uint8_t)((p >> 24) & 0xFF);
		uint8_t r = (uint8_t)((p >> 16) & 0xFF);
		uint8_t g = (uint8_t)((p >> 8) & 0xFF);
		uint8_t b = (uint8_t)((p >> 0) & 0xFF);

		r = (uint8_t)((((uint32_t)r * (uint32_t)a + 127) * 257) >> 16);
		g = (uint8_t)((((uint32_t)g * (uint32_t)a + 127) * 257) >> 16);
		b = (uint8_t)((((uint32_t)b * (uint32_t)a + 127) * 257) >> 16);

		dest[i] = (((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b << 0));
	}
}

static void abgr_to_argb_premultiply_alpha(uint32_t *src, uint32_t *dest, size_t count) {
	assert(((uintptr_t)src % 32) == 0);
	assert(((uintptr_t)dest % 32) == 0);

	size_t i = 0;

#if defined(__AVX2__)
	__m256i abgr_to_argb_mask = _mm256_set_epi8(
		32,28,29,30,
		28,24,25,26,
		23,20,21,22,
		19,16,17,18,
		15,12,13,14,
		11,8, 9, 10,
		7, 4, 5, 6,
		3, 0, 1, 2
	);

    __m256i zero = _mm256_setzero_si256();
    __m256i const_127 = _mm256_set1_epi16(127);
    __m256i const_257 = _mm256_set1_epi16(257);

	__m256i extract_alpha_mask = _mm256_set_epi8(
#define ZERO CHAR_MIN
		ZERO, 14, ZERO, 14, ZERO, 14, ZERO, 14,
		ZERO, 6, ZERO, 6, ZERO, 6, ZERO, 6,
		ZERO, 30, ZERO, 30, ZERO, 30, ZERO, 30,
		ZERO, 22, ZERO, 22, ZERO, 22, ZERO, 22
#undef ZERO
	);

	__m256i blend_mask = _mm256_set_epi8(
#define FROM1 0
#define FROM2 CHAR_MIN
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1,
		FROM2, FROM1, FROM1, FROM1
#undef FROM1
#undef FROM2
	);

    for ( ; (i + 8) <= count; i += 8) {
        __m256i abgr = _mm256_load_si256((__m256i *)(void *)&src[i]);

		__m256i abgr_lo = _mm256_unpacklo_epi8(abgr, zero);
		__m256i abgr_hi = _mm256_unpackhi_epi8(abgr, zero);

		__m256i alpha_lo = _mm256_shuffle_epi8(abgr_lo, extract_alpha_mask);
		__m256i alpha_hi = _mm256_shuffle_epi8(abgr_hi, extract_alpha_mask);

		abgr_lo = _mm256_mullo_epi16(abgr_lo, alpha_lo);
		abgr_hi = _mm256_mullo_epi16(abgr_hi, alpha_hi);

		abgr_lo = _mm256_add_epi16(abgr_lo, const_127);
		abgr_hi = _mm256_add_epi16(abgr_hi, const_127);

		abgr_lo = _mm256_mulhi_epu16(abgr_lo, const_257);
		abgr_hi = _mm256_mulhi_epu16(abgr_hi, const_257);

		__m256i abgr_packed = _mm256_packus_epi16(abgr_lo, abgr_hi);
		__m256i abgr_premultiplied = _mm256_blendv_epi8(abgr_packed, abgr, blend_mask);
		__m256i argb = _mm256_shuffle_epi8(abgr_premultiplied, abgr_to_argb_mask); // TODO: rework

        _mm256_store_si256((__m256i *)(void *)&dest[i], argb);
    }
#endif // __AVX2__
// TODO: ARM SIMD

	for ( ; i < count; ++i) {
		uint32_t p = src[i];
		uint8_t a = (uint8_t)((p >> 24) & 0xFF);
		uint8_t b = (uint8_t)((p >> 16) & 0xFF);
		uint8_t g = (uint8_t)((p >> 8) & 0xFF);
		uint8_t r = (uint8_t)((p >> 0) & 0xFF);

		r = (uint8_t)((((uint32_t)r * (uint32_t)a + 127) * 257) >> 16);
		g = (uint8_t)((((uint32_t)g * (uint32_t)a + 127) * 257) >> 16);
		b = (uint8_t)((((uint32_t)b * (uint32_t)a + 127) * 257) >> 16);

		dest[i] = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b << 0);
	}
}

static void abgr_to_argb(uint32_t *src, uint32_t *dest, size_t count) {
	assert(((uintptr_t)src % 32) == 0);
	assert(((uintptr_t)dest % 32) == 0);

	size_t i = 0;

#if defined(__AVX2__)
	__m256i mask = _mm256_set_epi8(
		32,28,29,30,
		28,24,25,26,
		23,20,21,22,
		19,16,17,18,
		15,12,13,14,
		11,8, 9, 10,
		7, 4, 5, 6,
		3, 0, 1, 2
	);

    for ( ; (i + 8) <= count; i += 8) {
        __m256i abgr = _mm256_load_si256((__m256i *)(void *)&src[i]);
        __m256i argb = _mm256_shuffle_epi8(abgr, mask);
        _mm256_store_si256((__m256i *)(void *)&dest[i], argb);
    }
#endif // __AVX2__
// TODO: ARM SIMD

	for ( ; i < count; ++i) {
		uint32_t p = src[i];
		uint8_t a = (uint8_t)((p >> 24) & 0xFF);
		uint8_t b = (uint8_t)((p >> 16) & 0xFF);
		uint8_t g = (uint8_t)((p >> 8) & 0xFF);
		uint8_t r = (uint8_t)((p >> 0) & 0xFF);
		dest[i] = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b << 0);
	}
}

static int trailing_zeros(uint32_t x, int fallback) {
#if __has_builtin(__builtin_ctzg)
	return __builtin_ctzg(x, fallback);
#else
// TODO
#error "TODO: implement fallback for __builtin_ctz"
#endif
}

static bool32_t fd_set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        return false;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return false;
    }

	return true;
}

static bool32_t fd_set_cloexec(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1) {
        return false;
    }

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
        return false;
    }

	return true;
}

static int64_t timespec_to_ms(struct timespec timespec) {
	return (timespec.tv_sec * 1000) + (timespec.tv_nsec / 1000000);
}

static int64_t now_ms(void) {
	struct timespec ts;
	int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
	(void)ret;
	assert(ret == 0);
	return timespec_to_ms(ts);
}

static int data_to_shm(string_t *path_out, void *bytes, size_t nbytes) {
	static const char template[] = "/dev/shm/" PREFIX "-XXXXXX";
	string_init_len(path_out, template, STRING_LITERAL_LENGTH(template), true);
	int fd = mkstemp(path_out->s);
	if (fd == -1) {
		goto error_1;
	}

	fd_set_cloexec(fd);

	size_t total = 0;
	while (total < nbytes) {
		ssize_t written_bytes = write(fd, (uint8_t *)bytes + total, nbytes - total);
		if (written_bytes == -1) {
			goto error_2;
		}
		total += (size_t)written_bytes;
	}

	return fd;
error_2:
	close(fd);
error_1:
	string_fini(path_out);
	return -1;
}

//static const uint8_t base64__reverse_lookup[] = {
//    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
//    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
//    255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
//     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
//    255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
//     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
//    255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
//     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
//    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
//    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
//    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
//    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
//    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
//    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
//    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
//    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
//};

//static void *base64_decode(string_t text) {
//    if (text.len % 4 != 0) {
//        return NULL;
//    }
//
//    uint8_t *ret = malloc(text.len / 4 * 3);
//
//    for (size_t i = 0, o = 0; i < text.len; i += 4, o += 3) {
//        uint32_t a = base64__reverse_lookup[(uint8_t)text.s[i + 0]];
//        uint32_t b = base64__reverse_lookup[(uint8_t)text.s[i + 1]];
//        uint32_t c = base64__reverse_lookup[(uint8_t)text.s[i + 2]];
//        uint32_t d = base64__reverse_lookup[(uint8_t)text.s[i + 3]];
//
//        uint32_t u = a | b | c | d;
//        if (u & 128) {
//            goto error;
//        }
//
//        if (u & 64) {
//            if (i + 4 != text.len || (a | b) & 64 || (c & 64 && !(d & 64))) {
//                goto error;
//            }
//            c &= 63;
//            d &= 63;
//        }
//
//        uint32_t v = a << 18 | b << 12 | c << 6 | d << 0;
//        ret[o + 0] = (uint8_t)((v >> 16) & 0xFF);
//        ret[o + 1] = (uint8_t)((v >> 8) & 0xFF);
//        ret[o + 2] = (uint8_t)((v >> 0) & 0xFF);
//    }
//
//    return ret;
//error:
//    free(ret);
//    return NULL;
//}

#endif // UTIL_H
