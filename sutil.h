#if !defined(SU_HEADER)
#define SU_HEADER

/* su - simple utils */

#if !defined(SU_WITH_SIMD)
#define SU_WITH_SIMD 1
#endif /* !defined(SU_WITH_SIMD) */
#if !defined(SU_WITH_DEBUG)
#define SU_WITH_DEBUG 0
#endif /* !defined(SU_WITH_DEBUG) */

/* TODO: remove */
#define SU_IMPLEMENTATION
/*#define SU_STRIP_PREFIXES */
#if defined(_XOPEN_SOURCE)
#define SU__USER_XOPEN_SOURCE _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif /* defined(_XOPEN_SOURCE) */
#define _XOPEN_SOURCE 700
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <uchar.h>

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>

#if SU_WITH_SIMD && defined(__x86_64__)
#include <immintrin.h>
#endif /* SU_WITH_SIMD && defined(__x86_64__) */

#define SU_PRAGMA(x) _Pragma(#x)

#define SU_IGNORE_WARNING(w) SU_PRAGMA(GCC diagnostic ignored w)
#define SU_IGNORE_WARNINGS_START \
	SU_PRAGMA(GCC diagnostic push) \
	SU_IGNORE_WARNING("-Wpragmas") \
	SU_IGNORE_WARNING("-Wunknown-warning-option")
#define SU_IGNORE_WARNINGS_END SU_PRAGMA(GCC diagnostic pop)

#if defined(__GNUC__)
/* TODO: remove */
#define inline __inline
#define restrict __restrict

#define SU_ALIGNOF __alignof
#define SU_TYPEOF __typeof
#define SU_THREAD_LOCAL __thread

#if defined(_Static_assert)
#undef _Static_assert
#endif
/* ? TODO: with message */
#if !defined(__cplusplus)
#define SU_STATIC_ASSERT(x) __extension__ _Static_assert(x, "")
#endif /* !defined(__cplusplus) */

#define SU_STRLEN __builtin_strlen

#else
#define SU_ALIGNOF alignof
#define SU_TYPEOF typeof
#define SU_THREAD_LOCAL thread_local
#define SU_STRLEN strlen
#endif

#if !defined(SU_STATIC_ASSERT)
#define SU_STATIC_ASSERT(x) static_assert(x, "")
#endif /* !defined(SU_STATIC_ASSERT) */

#define SU_UNREACHABLE 0

#if defined(__has_include)
#define SU_HAS_INCLUDE(x) __has_include(x)
#else
#define SU_HAS_INCLUDE(x) 0
#endif

#if defined(__has_attribute)
#define SU_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define SU_HAS_ATTRIBUTE(x) 0
#endif

#if SU_HAS_ATTRIBUTE(always_inline)
#define SU_ATTRIBUTE_ALWAYS_INLINE __attribute__((__always_inline__))
#else
#define SU_ATTRIBUTE_ALWAYS_INLINE
#endif

#if SU_HAS_ATTRIBUTE(format)
#define SU_ATTRIBUTE_FORMAT_PRINTF(start, end) __attribute__((__format__(__printf__, (start), (end))))
#else
#define SU_ATTRIBUTE_FORMAT_PRINTF(start, end)
#endif

#if SU_HAS_ATTRIBUTE(const)
#define SU_ATTRIBUTE_CONST __attribute__((__const__))
#else
#define SU_ATTRIBUTE_CONST
#endif

#if SU_HAS_ATTRIBUTE(fallthrough)
#define SU_ATTRIBUTE_FALLTHROUGH __attribute__((__fallthrough__))
#else
#define SU_ATTRIBUTE_FALLTHROUGH
#endif

#if SU_HAS_ATTRIBUTE(pure)
#define SU_ATTRIBUTE_PURE __attribute__((__pure__))
#else
#define SU_ATTRIBUTE_PURE
#endif

#if SU_HAS_ATTRIBUTE(noreturn)
#define SU_ATTRIBUTE_NORETURN __attribute__((__noreturn__))
#else
#define SU_ATTRIBUTE_NORETURN
#endif

#if defined(__has_builtin)
#define SU_HAS_BUILTIN(x) __has_builtin(x)
#else
#define SU_HAS_BUILTIN(x) 0
#endif

#if SU_HAS_BUILTIN(__builtin_ctzg)
#define SU_COUNT_TRAILING_ZEROS(x, fallback) __builtin_ctzg(x, fallback)
#else
#error "TODO: implement __builtin_ctzg"
#endif

#if SU_HAS_BUILTIN(__builtin_unreachable)
#define SU_ASSERT_UNREACHABLE SU_ASSERT(SU_UNREACHABLE); __builtin_unreachable()
#else
#define SU_ASSERT_UNREACHABLE SU_ASSERT(SU_UNREACHABLE)
#endif

#if SU_HAS_BUILTIN(__builtin_expect)
#define SU_LIKELY(expr) __builtin_expect(!!(expr), 1)
#define SU_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#else
#define SU_LIKELY(expr) (expr)
#define SU_UNLIKELY(expr) (expr)
#endif

#define SU_LENGTH(x) (sizeof((x)) / sizeof((x)[0]))
#define SU_STRING_LITERAL_LENGTH(str) (SU_LENGTH((str)) - 1)

#define SU_MIN(a, b) (((a) > (b)) ? (b) : (a))
#define SU_MAX(a, b) (((a) > (b)) ? (a) : (b))

#define SU__STRINGIFY(x) #x
#define SU_STRINGIFY(x) SU__STRINGIFY(x)

#define SU_NOTUSED(x) (void)x

#if SU_WITH_DEBUG
#define SU_ASSERT(expr) \
	do { \
		if (SU_UNLIKELY(!(expr))) { \
			size_t *su__n = NULL; \
			su_log_stderr("%s:%d: assertion '%s' failed", __FILE__, __LINE__, #expr); \
			*su__n = 0; \
		} \
	} while(0)
#else
#define SU_ASSERT(expr)
#endif

#define SU_PAD8 uint8_t su__pad8
#define SU_PAD16 uint16_t su__pad16
#define SU_PAD32 uint32_t su__pad32

#define SU_TRUE 1
#define SU_FALSE 0

#if SU_WITH_DEBUG
#define SU_DEBUG_LOG su_log_stderr
#else
#define SU_DEBUG_LOG su_nop
#endif /* SU_WITH_DEBUG */

#define SU_ARRAY_DECLARE(type) \
typedef struct su_array__##type { \
    size_t size, len; \
    type *items; \
} su_array__##type##__t; \
\
static void su_array__##type##__init(su_array__##type##__t *, su_allocator_t *, size_t initial_size); \
static void su_array__##type##__init0(su_array__##type##__t *, su_allocator_t *, size_t initial_size); \
static void su_array__##type##__fini(su_array__##type##__t *, su_allocator_t *); \
static void su_array__##type##__resize(su_array__##type##__t *, su_allocator_t *, size_t new_size); \
static type *su_array__##type##__add(su_array__##type##__t *, su_allocator_t *, type item); \
static type *su_array__##type##__add_nocheck(su_array__##type##__t *, type item); \
static type *su_array__##type##__add_uninitialized(su_array__##type##__t *, su_allocator_t *); \
static type *su_array__##type##__add_nocheck_uninitialized(su_array__##type##__t *); \
static void su_array__##type##__remove(su_array__##type##__t *, size_t n); \
static void su_array__##type##__swap(su_array__##type##__t *, size_t idx1, size_t idx2); \
static void su_array__##type##__insert(su_array__##type##__t *, su_allocator_t *, type item, size_t idx); \
static void su_array__##type##__pop(su_array__##type##__t *, size_t idx); \
static void su_array__##type##__pop_swapback(su_array__##type##__t *, size_t idx); \
static type *su_array__##type##__put(su_array__##type##__t *, su_allocator_t *, type item, size_t idx); \
static type su_array__##type##__get(su_array__##type##__t *, size_t idx); \
static type *su_array__##type##__get_ptr(su_array__##type##__t *, size_t idx); \
static void su_array__##type##__qsort(su_array__##type##__t *, int compare(const void *, const void *))

/* TODO: rework */
#define SU_STACK_DECLARE(type) \
\
SU_ARRAY_DECLARE(type); \
\
typedef struct su_stack__##type { \
	su_array__##type##__t data; \
} su_stack__##type##__t; \
\
static void su_stack__##type##__init(su_stack__##type##__t *, su_allocator_t *, size_t initial_size); \
static void su_stack__##type##__fini(su_stack__##type##__t *, su_allocator_t *); \
static void su_stack__##type##__resize(su_stack__##type##__t *, su_allocator_t *, size_t new_size); \
static type *su_stack__##type##__push(su_stack__##type##__t *, su_allocator_t *, type item); \
static type *su_stack__##type##__push_nocheck(su_stack__##type##__t *, type item); \
static type su_stack__##type##__pop(su_stack__##type##__t *); \
static type su_stack__##type##__get(su_stack__##type##__t *)

#define SU_LLIST_DECLARE(type) \
typedef struct su_llist__##type { \
    type *head; \
    type *tail; \
    size_t len; \
} su_llist__##type##__t; \
\
static void su_llist__##type##__insert_head(su_llist__##type##__t *, type *node); \
static void su_llist__##type##__insert_tail(su_llist__##type##__t *, type *node); \
static void su_llist__##type##__pop(su_llist__##type##__t *, type *node)

#define SU_LLIST_FIELDS(type) \
	type *next; \
	type *prev

#define SU_HASH_TABLE_DECLARE(type, key_type, hash_key_func, keys_equal_func, collisions_to_resize) \
\
SU_ARRAY_DECLARE(type); \
\
typedef struct su_hash_table__##type { \
	su_array__##type##__t items; \
} su_hash_table__##type##__t; \
\
static void su_hash_table__##type##__init(su_hash_table__##type##__t *, su_allocator_t *, size_t initial_size); \
static void su_hash_table__##type##__fini(su_hash_table__##type##__t *, su_allocator_t *); \
static void su_hash_table__##type##__resize(su_hash_table__##type##__t *, su_allocator_t *, size_t new_size); \
static su_bool32_t su_hash_table__##type##__add(su_hash_table__##type##__t *, su_allocator_t *, key_type, type **out); \
static su_bool32_t su_hash_table__##type##__get(su_hash_table__##type##__t *, key_type, type **out); \
static su_bool32_t su_hash_table__##type##__del(su_hash_table__##type##__t *, key_type, type *out)

#define SU_HASH_TABLE_FIELDS(key_type) \
	key_type key; \
	su_bool32_t occupied; \
	su_bool32_t tombstone

/* ? TODO: argc, argv as params */
#define SU_ARGPARSE_BEGIN \
    for (argv++, argc--; \
            argv[0] && (argv[0][0] == '-') && argv[0][1]; \
            argc--, argv++) { \
		char argc_; \
		char **argv_; \
		int brk_; \
		if ((argv[0][1] == '-') && (argv[0][2] == '\0')) { \
			argv++; \
			argc--; \
			break; \
		} \
		for (brk_ = 0, argv[0]++, argv_ = argv; \
				argv[0][0] && !brk_; \
				argv[0]++) { \
			if (argv_ != argv) { \
                break; \
            } \
			argc_ = argv[0][0];
#define SU_ARGPARSE_END \
        } \
    }

#define SU_ARGPARSE_KEY argc_

#define SU_ARGPARSE_VALUE \
    (((argv[0][1] == '\0') && (argv[1] == NULL)) ? \
        (char *)0 : \
        (brk_ = 1, (argv[0][1] != '\0') ? \
            (&argv[0][1]) : \
            (argc--, argv++, argv[0])))

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

typedef uint32_t su_bool32_t;

typedef struct su_fat_ptr {
	void *ptr;
	size_t len;
} su_fat_ptr_t;

typedef struct su_allocator su_allocator_t;
typedef void *(*su_allocator_alloc_func_t)(su_allocator_t *, size_t size, size_t alignment);
typedef void *(*su_allocator_realloc_func_t)(su_allocator_t *, void *ptr, size_t new_size, size_t new_alignment);
typedef void (*su_allocator_free_func_t)(su_allocator_t *, void *ptr);

struct su_allocator {
	su_allocator_alloc_func_t alloc;
	su_allocator_realloc_func_t realloc;
	su_allocator_free_func_t free;
};

typedef struct su_string {
	su_bool32_t free_contents;
	su_bool32_t nul_terminated;
	size_t len; /* excluding '\0' */
	char *s;
} su_string_t;

#define SU_STRING_PF_FMT "%.*s"
#define SU_STRING_PF_ARGS(str) (int)(str).len, (str).s

SU_ARRAY_DECLARE(su_string_t);

typedef struct su_arena_block {
	size_t ptr;
	size_t size;
	uint8_t *data;
} su_arena_block_t;

SU_ARRAY_DECLARE(su_arena_block_t);

typedef struct su_arena {
	su_array__su_arena_block_t__t blocks;
} su_arena_t;

typedef struct su_file_cache {
	SU_HASH_TABLE_FIELDS(su_string_t);
	struct timespec st_mtim;
	su_fat_ptr_t data;
} su_file_cache_t;

/* TODO: better hash function */
SU_HASH_TABLE_DECLARE(su_file_cache_t, su_string_t, su_stbds_hash_string, su_string_equal, 16);


typedef struct su_json_buffer {
	char *data;
	size_t size;
	size_t idx;
} su_json_buffer_t;

typedef enum su__json_writer_state {
	SU__JSON_WRITER_STATE_ROOT,
	SU__JSON_WRITER_STATE_OBJECT,
	SU__JSON_WRITER_STATE_ARRAY,
	SU__JSON_WRITER_STATE_KEY,
	SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA,
	SU__JSON_WRITER_STATE_ARRAY_EXPECTING_COMMA
} su__json_writer_state_t;

SU_STACK_DECLARE(su__json_writer_state_t);

typedef struct su_json_writer {
	su_json_buffer_t buf;
	su_stack__su__json_writer_state_t__t state;
} su_json_writer_t;

typedef enum su__json_tokener_state {
	SU__JSON_TOKENER_STATE_ROOT,
	SU__JSON_TOKENER_STATE_OBJECT,
	SU__JSON_TOKENER_STATE_OBJECT_EMPTY,
	SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA,
	SU__JSON_TOKENER_STATE_KEY,
	SU__JSON_TOKENER_STATE_VALUE,
	SU__JSON_TOKENER_STATE_ARRAY,
	SU__JSON_TOKENER_STATE_ARRAY_EMPTY,
	SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA,
	SU__JSON_TOKENER_STATE_STRING,
	SU__JSON_TOKENER_STATE_NUMBER,
	SU__JSON_TOKENER_STATE_NULL,
	SU__JSON_TOKENER_STATE_FALSE,
	SU__JSON_TOKENER_STATE_TRUE
} su__json_tokener_state_t;

SU_STACK_DECLARE(su__json_tokener_state_t);

typedef struct su_json_tokener {
	su_string_t str;
	size_t pos;
	su_json_buffer_t buf;
	size_t last_escape_idx; /* in buf */
	size_t depth;
	su_stack__su__json_tokener_state_t__t state;
} su_json_tokener_t;

typedef enum su_json_tokener_state {
	SU_JSON_TOKENER_STATE_SUCCESS,
	SU_JSON_TOKENER_STATE_ERROR,
	SU_JSON_TOKENER_STATE_EOF,
	SU_JSON_TOKENER_STATE_MORE_DATA_EXPECTED
} su_json_tokener_state_t;

typedef enum su_json_token_type {
	SU_JSON_TOKEN_TYPE_OBJECT_START,
	SU_JSON_TOKEN_TYPE_OBJECT_END,
	SU_JSON_TOKEN_TYPE_KEY,
	SU_JSON_TOKEN_TYPE_ARRAY_START,
	SU_JSON_TOKEN_TYPE_ARRAY_END,
	SU_JSON_TOKEN_TYPE_STRING,
	SU_JSON_TOKEN_TYPE_BOOL,
	SU_JSON_TOKEN_TYPE_NULL,
	SU_JSON_TOKEN_TYPE_DOUBLE,
	SU_JSON_TOKEN_TYPE_INT,
	SU_JSON_TOKEN_TYPE_UINT
} su_json_token_type_t;

typedef union su_json_token_value {
	su_string_t s; /* SU_JSON_TOKEN_TYPE_STRING, SU_JSON_TOKEN_TYPE_KEY */
	su_bool32_t b; /* SU_JSON_TOKEN_TYPE_BOOL */
	double d; /* SU_JSON_TOKEN_TYPE_DOUBLE */
	int64_t i; /* SU_JSON_TOKEN_TYPE_INT */
	uint64_t u; /* SU_JSON_TOKEN_TYPE_UINT */
} su_json_token_value_t;

typedef struct su_json_token {
	size_t depth;
	SU_PAD32;
	su_json_token_type_t type;
	su_json_token_value_t value;
} su_json_token_t;

typedef enum su_json_ast_node_type {
	SU_JSON_AST_NODE_TYPE_NONE,
	SU_JSON_AST_NODE_TYPE_OBJECT,
	SU_JSON_AST_NODE_TYPE_ARRAY,
	SU_JSON_AST_NODE_TYPE_STRING,
	SU_JSON_AST_NODE_TYPE_BOOL,
	SU_JSON_AST_NODE_TYPE_NULL,
	SU_JSON_AST_NODE_TYPE_DOUBLE,
	SU_JSON_AST_NODE_TYPE_INT,
	SU_JSON_AST_NODE_TYPE_UINT
} su_json_ast_node_type_t;

typedef struct su_json_ast_node su_json_ast_node_t;
SU_ARRAY_DECLARE(su_json_ast_node_t);

typedef struct su_json_ast_key_value su_json_ast_key_value_t;
SU_ARRAY_DECLARE(su_json_ast_key_value_t);

typedef union su_json_ast_node_value {
	su_array__su_json_ast_key_value_t__t object; /* SU_JSON_AST_NODE_TYPE_OBJECT  ? TODO: hash table */
	su_array__su_json_ast_node_t__t array; /* SU_JSON_AST_NODE_TYPE_ARRAY */
	su_string_t s; /* SU_JSON_AST_NODE_TYPE_STRING */
	su_bool32_t b; /* SU_JSON_AST_NODE_TYPE_BOOL */
	double d; /* SU_JSON_AST_NODE_TYPE_DOUBLE */
	int64_t i; /* SU_JSON_AST_NODE_TYPE_INT */
	uint64_t u; /* SU_JSON_AST_NODE_TYPE_UINT */
} su_json_ast_node_value_t;

struct su_json_ast_node {
	su_json_ast_node_t *parent;
	SU_PAD32;
	su_json_ast_node_type_t type;
	su_json_ast_node_value_t value;
};

struct su_json_ast_key_value {
	su_string_t key;
	su_json_ast_node_t value;
};

typedef struct su_json_ast {
	su_json_ast_node_t *current;
	su_json_ast_node_t root;
} su_json_ast_t;

#define su_vsprintf stbsp_vsprintf
#define su_vsnprintf stbsp_vsnprintf
#define su_sprintf stbsp_sprintf
#define su_snprintf stbsp_snprintf
#define su_vsprintfcb stbsp_vsprintfcb
#define su_printf_set_separators stbsp_set_separators

static SU_ATTRIBUTE_FORMAT_PRINTF(2, 0) void su_log_va(int fd, const char *fmt, va_list args);
static SU_ATTRIBUTE_FORMAT_PRINTF(1, 2) void su_log_stdout(const char *fmt, ...);
static SU_ATTRIBUTE_FORMAT_PRINTF(1, 2) void su_log_stderr(const char *fmt, ...);
static SU_ATTRIBUTE_NORETURN SU_ATTRIBUTE_FORMAT_PRINTF(2, 3) void su_abort(int code, const char *fmt, ...);

static inline SU_ATTRIBUTE_ALWAYS_INLINE su_string_t su_string(const char *literal);
static SU_ATTRIBUTE_FORMAT_PRINTF(3, 4) void su_string_init_format(
	su_string_t *, su_allocator_t *, const char *fmt, ...);
static void su_string_init_len(su_string_t *, su_allocator_t *,
		const char *s, size_t len, su_bool32_t nul_terminate);
static void su_string_init_string(su_string_t *, su_allocator_t *, su_string_t src);
static void su_string_init(su_string_t *, su_allocator_t *, const char *src);
static void su_string_fini(su_string_t *, su_allocator_t *);
static su_string_t su_string_view(su_string_t);
static su_bool32_t su_string_equal(su_string_t str1, su_string_t str2);
static int su_string_compare(su_string_t str1, su_string_t str2, size_t max);
static su_bool32_t su_string_find_char(su_string_t, char c, su_string_t *view_out);
static su_bool32_t su_string_tok(su_string_t *, char delim, su_string_t *token_out, su_string_t *saveptr);
static su_bool32_t su_string_starts_with(su_string_t, su_string_t prefix);
static su_bool32_t su_string_ends_with(su_string_t, su_string_t suffix);
static SU_ATTRIBUTE_PURE uint32_t su_string_hex_16_to_uint16(su_string_t);
static su_bool32_t su_string_to_uint64(su_string_t, uint64_t *out);
static su_bool32_t su_string_to_int64(su_string_t, int64_t *out);

static void su_arena_init(su_arena_t *, su_allocator_t *, size_t initial_block_size);
static void su_arena_fini(su_arena_t *, su_allocator_t *);
static su_arena_block_t *su_arena_add_block(su_arena_t *, su_allocator_t *, size_t size);
static void *su_arena_alloc(su_arena_t *, su_allocator_t *, size_t size, size_t alignment);
static size_t su_arena_alloc_get_size(void *ptr);
static void su_arena_reset(su_arena_t *, su_allocator_t *);

/* static SU_ATTRIBUTE_PURE size_t su_sdbm_hash(su_string_t); */
/* static SU_ATTRIBUTE_PURE size_t su_djb2_hash(su_string_t); */
static SU_ATTRIBUTE_PURE size_t su_stbds_hash_string(su_string_t);
static SU_ATTRIBUTE_PURE size_t su_stbds_hash(su_fat_ptr_t);

static void su_argb_premultiply_alpha(uint32_t *dest, uint32_t *src, size_t count);
static void su_abgr_to_argb_premultiply_alpha(uint32_t *dest, uint32_t *src, size_t count);
static void su_abgr_to_argb(uint32_t *dest, uint32_t *src, size_t count);

static su_bool32_t su_read_entire_file(su_string_t path, su_fat_ptr_t *out, su_allocator_t *);
static su_bool32_t su_read_entire_file_with_cache(su_string_t path, su_fat_ptr_t *out,
	su_allocator_t *, su_hash_table__su_file_cache_t__t *cache);

static su_bool32_t su_fd_set_nonblock(int);
static su_bool32_t su_fd_set_cloexec(int);

static int64_t su_timespec_to_ms(struct timespec);
static int64_t su_now_ms(clockid_t);

static su_bool32_t su_locale_is_utf8(void);
static void su_nop(void *notused, ...);

/*static void *su_base64_decode(su_string_t, su_allocator_t *);*/

static void su_json_writer_init(su_json_writer_t *, su_allocator_t *, size_t initial_bufsize);
static void su_json_writer_fini(su_json_writer_t *, su_allocator_t *);
static void su_json_writer_reset(su_json_writer_t *);

static void su_json_writer_object_begin(su_json_writer_t *, su_allocator_t *);
static void su_json_writer_object_end(su_json_writer_t *, su_allocator_t *);
static void su_json_writer_object_key_escape(su_json_writer_t *, su_allocator_t *, su_string_t);
static void su_json_writer_object_key(su_json_writer_t *, su_allocator_t *, su_string_t);

static void su_json_writer_array_begin(su_json_writer_t *, su_allocator_t *);
static void su_json_writer_array_end(su_json_writer_t *, su_allocator_t *);

static void su_json_writer_null(su_json_writer_t *, su_allocator_t *);
static void su_json_writer_bool(su_json_writer_t *, su_allocator_t *, su_bool32_t);
static void su_json_writer_int(su_json_writer_t *, su_allocator_t *, int64_t);
static void su_json_writer_uint(su_json_writer_t *, su_allocator_t *, uint64_t);
static void su_json_writer_double(su_json_writer_t *, su_allocator_t *, double);
static void su_json_writer_string_escape(su_json_writer_t *, su_allocator_t *, su_string_t);
static void su_json_writer_string(su_json_writer_t *, su_allocator_t *, su_string_t);

static void su_json_writer_raw(su_json_writer_t *, su_allocator_t *, void *data, size_t len);
static void su_json_writer_token(su_json_writer_t *, su_allocator_t *, su_json_token_t);
static void su_json_writer_ast_node(su_json_writer_t *, su_allocator_t *, su_json_ast_node_t *);

static void su_json_tokener_set_string(su_json_tokener_t *, su_allocator_t *, su_string_t);
static su_json_tokener_state_t su_json_tokener_next(su_json_tokener_t *,
	su_allocator_t *, su_json_token_t *out);
static su_json_tokener_state_t su_json_tokener_ast(su_json_tokener_t *, su_allocator_t *,
	su_json_ast_t *, uint32_t stop_depth, su_bool32_t check_for_repeating_keys);
static void su_json_ast_reset(su_json_ast_t *);

static su_json_ast_node_t *su_json_ast_node_object_get(su_json_ast_node_t *, su_string_t key);
static SU_ATTRIBUTE_ALWAYS_INLINE void su_json_tokener_advance_assert(su_json_tokener_t *, su_allocator_t *,
	su_json_token_t *token_out);
static SU_ATTRIBUTE_ALWAYS_INLINE void su_json_tokener_advance_assert_type(su_json_tokener_t *, su_allocator_t *,
	su_json_token_t *token_out, su_json_token_type_t expected_type);

#if defined(SU_STRIP_PREFIXES)

#define PRAGMA SU_PRAGMA
#define IGNORE_WARNING SU_IGNORE_WARNING
#define IGNORE_WARNINGS_START SU_IGNORE_WARNINGS_START
#define IGNORE_WARNINGS_END SU_IGNORE_WARNINGS_END
#define ALIGNOF SU_ALIGNOF
#define TYPEOF SU_TYPEOF
#define THREAD_LOCAL SU_THREAD_LOCAL
#define STRLEN SU_STRLEN
#define STATIC_ASSERT SU_STATIC_ASSERT
#define HAS_INCLUDE SU_HAS_INCLUDE
#define HAS_ATTRIBUTE SU_HAS_ATTRIBUTE
#define HAS_BUILTIN SU_HAS_BUILTIN
#define ATTRIBUTE_ALWAYS_INLINE SU_ATTRIBUTE_ALWAYS_INLINE
#define ATTRIBUTE_FORMAT_PRINTF SU_ATTRIBUTE_FORMAT_PRINTF
#define ATTRIBUTE_CONST SU_ATTRIBUTE_CONST
#define ATTRIBUTE_FALLTHROUGH SU_ATTRIBUTE_FALLTHROUGH
#define ATTRIBUTE_PURE SU_ATTRIBUTE_PURE
#define ATTRIBUTE_NORETURN SU_ATTRIBUTE_NORETURN
#define COUNT_TRAILING_ZEROS SU_COUNT_TRAILING_ZEROS
#define UNREACHABLE SU_UNREACHABLE
#define ASSERT SU_ASSERT
#define ASSERT_UNREACHABLE SU_ASSERT_UNREACHABLE
#define NOTUSED SU_NOTUSED
#define LIKELY SU_LIKELY
#define UNLIKELY SU_UNLIKELY
#define LENGTH SU_LENGTH
#define STRING_LITERAL_LENGTH SU_STRING_LITERAL_LENGTH
#define MIN SU_MIN
#define MAX SU_MAX
#define STRINGIFY SU_STRINGIFY
#define PAD8 SU_PAD8
#define PAD16 SU_PAD16
#define PAD32 SU_PAD32
#define TRUE SU_TRUE
#define FALSE SU_FALSE
#define DEBUG_LOG SU_DEBUG_LOG
#define STRING_PF_FMT SU_STRING_PF_FMT
#define STRING_PF_ARGS SU_STRING_PF_ARGS
#define ARRAY_DECLARE SU_ARRAY_DECLARE
#define ARRAY_DEFINE SU_ARRAY_DEFINE
#define ARRAY_DECLARE_DEFINE SU_ARRAY_DECLARE_DEFINE
#define STACK_DECLARE SU_STACK_DECLARE
#define STACK_DEFINE SU_STACK_DEFINE
#define STACK_DECLARE_DEFINE SU_STACK_DECLARE_DEFINE
#define LLIST_DECLARE SU_LLIST_DECLARE
#define LLIST_DEFINE SU_LLIST_DEFINE
#define LLIST_DECLARE_DEFINE SU_LLIST_DECLARE_DEFINE
#define LLIST_FIELDS SU_LLIST_FIELDS
#define HASH_TABLE_DECLARE SU_HASH_TABLE_DECLARE
#define HASH_TABLE_DEFINE SU_HASH_TABLE_DEFINE
#define HASH_TABLE_DECLARE_DEFINE SU_HASH_TABLE_DECLARE_DEFINE
#define HASH_TABLE_FIELDS SU_HASH_TABLE_FIELDS
#define ARGPARSE_BEGIN SU_ARGPARSE_BEGIN
#define ARGPARSE_END SU_ARGPARSE_END
#define ARGPARSE_KEY SU_ARGPARSE_KEY
#define ARGPARSE_VALUE SU_ARGPARSE_VALUE
typedef su_bool32_t bool32_t;
typedef su_fat_ptr_t fat_ptr_t;
typedef su_string_t string_t;
typedef su_allocator_t allocator_t;
typedef su_allocator_alloc_func_t alloc_func_t;
typedef su_allocator_realloc_func_t realloc_func_t;
typedef su_allocator_free_func_t free_func_t;
typedef su_arena_block_t arena_block_t;
typedef su_arena_t arena_t;
typedef su_file_cache_t file_cache_t;
typedef su_json_buffer_t json_buffer_t;
typedef su_json_writer_t json_writer_t;
typedef su_json_tokener_t json_tokener_t;
typedef su_json_tokener_state_t json_tokener_state_t;
typedef su_json_token_type_t json_token_type_t;
typedef su_json_token_value_t json_token_value_t;
typedef su_json_token_t json_token_t;
typedef su_json_ast_node_type_t json_ast_node_type_t;
typedef su_json_ast_node_value_t json_ast_node_value_t;
typedef su_json_ast_node_t json_ast_node_t;
typedef su_json_ast_key_value_t json_ast_key_value_t;
typedef su_json_ast_t json_ast_t;
/*#define vsprintf su_vsprintf  */
/*#define vsnprintf su_vsnprintf  */
/*#define sprintf su_sprintf  */
/*#define snprintf su_snprintf  */
/*#define vsprintfcb su_vsprintfcb  */
#define printf_set_separators su_printf_set_separators
#define log_va su_log_va
#define log_stdout su_log_stdout
#define log_stderr su_log_stderr
/*#define abort su_abort*/
#define string su_string
#define string_init_format su_string_init_format
#define string_init_len su_string_init_len
#define string_init_string su_string_init_string
#define string_init su_string_init
#define string_fini su_string_fini
#define string_view su_string_view
#define string_equal su_string_equal
#define string_compare su_string_compare
#define string_find_char su_string_find_char
#define string_tok su_string_tok
#define string_starts_with su_string_starts_with
#define string_ends_with su_string_ends_with
#define string_hex_16_to_uint16 su_string_hex_16_to_uint16
#define string_to_uint64 su_string_to_uint64
#define string_to_int64 su_string_to_int64
#define arena_init su_arena_init
#define arena_fini su_arena_fini
#define arena_add_block su_arena_add_block
#define arena_alloc su_arena_alloc
#define arena_alloc_get_size su_arena_alloc_get_size
#define arena_reset su_arena_reset
/*#define sdbm_hash su_sdbm_hash*/
/*#define djb2_hash su_djb2_hash*/
#define stbds_hash_string su_stbds_hash_string
#define stbds_hash su_stbds_hash
#define argb_premultiply_alpha su_argb_premultiply_alpha
#define abgr_to_argb_premultiply_alpha su_abgr_to_argb_premultiply_alpha
#define abgr_to_argb su_abgr_to_argb
#define read_entire_file su_read_entire_file
#define read_entire_file_with_cache su_read_entire_file_with_cache
#define fd_set_nonblock su_fd_set_nonblock
#define fd_set_cloexec su_fd_set_cloexec
#define timespec_to_ms su_timespec_to_ms
#define now_ms su_now_ms
/*#define base64_decode su_base64_decode */
#define locale_is_utf8 su_locale_is_utf8
#define nop su_nop
#define json_writer_init su_json_writer_init
#define json_writer_fini su_json_writer_fini
#define json_writer_reset su_json_writer_reset
#define json_writer_object_begin su_json_writer_object_begin
#define json_writer_object_end su_json_writer_object_end
#define json_writer_object_key_escape su_json_writer_object_key_escape
#define json_writer_object_key su_json_writer_object_key
#define json_writer_array_begin su_json_writer_array_begin
#define json_writer_array_end su_json_writer_array_end
#define json_writer_null su_json_writer_null
#define json_writer_bool su_json_writer_bool
#define json_writer_int su_json_writer_int
#define json_writer_uint su_json_writer_uint
#define json_writer_double su_json_writer_double
#define json_writer_string_escape su_json_writer_string_escape
#define json_writer_string su_json_writer_string
#define json_writer_raw su_json_writer_raw
#define json_writer_token su_json_writer_token
#define json_writer_ast_node su_json_writer_ast_node
#define json_tokener_set_string su_json_tokener_set_string
#define json_tokener_next su_json_tokener_next
#define json_tokener_ast su_json_tokener_ast
#define json_ast_reset su_json_ast_reset
#define json_ast_node_object_get su_json_ast_node_object_get
#define json_tokener_advance_assert su_json_tokener_advance_assert
#define json_tokener_advance_assert_type su_json_tokener_advance_assert_type

#endif /* defined(SU_STRIP_PREFIXES) */

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#undef _XOPEN_SOURCE
#if defined(SU__USER_XOPEN_SOURCE)
#define _XOPEN_SOURCE SU__USER_XOPEN_SOURCE
#endif /* defined(SU__USER_XOPEN_SOURCE) */

#endif /* !defined(SU_HEADER) */

#if defined(SU_IMPLEMENTATION) && !defined(SU__REIMPLEMENTATION_GUARD)
#define SU__REIMPLEMENTATION_GUARD

#define STB_SPRINTF_MIN 128
#define STB_SPRINTF_STATIC
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOUNALIGNED

SU_IGNORE_WARNINGS_START

SU_IGNORE_WARNING("-Wcast-align")
SU_IGNORE_WARNING("-Wextra-semi-stmt")
SU_IGNORE_WARNING("-Wsign-conversion")
SU_IGNORE_WARNING("-Wdouble-promotion")
SU_IGNORE_WARNING("-Wpadded")
SU_IGNORE_WARNING("-Wimplicit-fallthrough")
SU_IGNORE_WARNING("-Wconditional-uninitialized")
SU_IGNORE_WARNING("-Wsign-compare")
SU_IGNORE_WARNING("-Warith-conversion")
SU_IGNORE_WARNING("-Wconversion")
SU_IGNORE_WARNING("-Wcomment")
SU_IGNORE_WARNING("-Wlong-long")
SU_IGNORE_WARNING("-Wc++98-compat-pedantic")
SU_IGNORE_WARNING("-Wzero-as-null-pointer-constant")
SU_IGNORE_WARNING("-Wcast-qual")
SU_IGNORE_WARNING("-Wuseless-cast")

#include "stb_sprintf.h"

SU_IGNORE_WARNINGS_END

#define SU_ARRAY_DEFINE(type) \
static void su_array__##type##__init(su_array__##type##__t *array, su_allocator_t *alloc, size_t initial_size) { \
    SU_ASSERT(initial_size > 0); \
    array->items = (type *)alloc->alloc(alloc, sizeof(type) * initial_size, SU_ALIGNOF(type)); \
	array->size = initial_size; \
	array->len = 0; \
} \
\
static void su_array__##type##__init0(su_array__##type##__t *array, su_allocator_t *alloc, size_t initial_size) { \
    size_t size = initial_size * sizeof(type); \
	SU_ASSERT(initial_size > 0); \
    array->items = (type *)alloc->alloc(alloc, size, SU_ALIGNOF(type)); \
	memset(array->items, 0, size); \
	array->size = initial_size; \
	array->len = 0; \
} \
\
static void su_array__##type##__fini(su_array__##type##__t *array, su_allocator_t *alloc) { \
	alloc->free(alloc, array->items); \
} \
\
static void su_array__##type##__resize(su_array__##type##__t *array, su_allocator_t *alloc, size_t new_size) { \
    SU_ASSERT(new_size > 0); \
    array->items = (type *)alloc->realloc(alloc, array->items, sizeof(type) * new_size, SU_ALIGNOF(type)); \
    array->size = new_size; \
} \
\
static type *su_array__##type##__add(su_array__##type##__t *array, su_allocator_t *alloc, type item) { \
    if (array->size == array->len) { \
        su_array__##type##__resize(array, alloc, 8 + array->size * 2); \
    } \
    array->items[array->len] = item; \
	return &array->items[array->len++]; \
} \
\
static type *su_array__##type##__add_nocheck(su_array__##type##__t *array, type item) { \
    array->items[array->len] = item; \
	return &array->items[array->len++]; \
} \
\
static type *su_array__##type##__add_uninitialized(su_array__##type##__t *array, su_allocator_t *alloc) { \
    if (array->size == array->len) { \
        su_array__##type##__resize(array, alloc, 8 + array->size * 2); \
    } \
	return &array->items[array->len++];\
} \
\
static type *su_array__##type##__add_nocheck_uninitialized(su_array__##type##__t *array) { \
	return &array->items[array->len++]; \
} \
\
static void su_array__##type##__remove(su_array__##type##__t *array, size_t n) { \
	SU_ASSERT(array->len > 0); \
    array->len -= n; \
} \
\
static void su_array__##type##__swap(su_array__##type##__t *array, size_t idx1, size_t idx2) { \
	type tmp = array->items[idx1]; \
	SU_ASSERT(idx1 < array->len); \
	SU_ASSERT(idx2 < array->len); \
	array->items[idx1] = array->items[idx2]; \
	array->items[idx2] = tmp; \
} \
\
static void su_array__##type##__insert(su_array__##type##__t *array, su_allocator_t *alloc, type item, size_t idx) { \
    SU_ASSERT(idx <= array->len); \
    if (array->size == array->len) { \
        su_array__##type##__resize(array, alloc, array->size * 2); \
    } \
	memmove(&array->items[idx + 1], &array->items[idx], sizeof(type) * (array->len - idx)); \
	array->len++; \
	array->items[idx] = item; \
} \
\
static void su_array__##type##__pop(su_array__##type##__t *array, size_t idx) { \
    SU_ASSERT(idx < array->len); \
	array->len--; \
	memmove(&array->items[idx], &array->items[idx + 1], sizeof(type) * (array->len - idx)); \
} \
\
static void su_array__##type##__pop_swapback(su_array__##type##__t *array, size_t idx) { \
    SU_ASSERT(idx < array->len); \
	array->len--; \
	array->items[idx] = array->items[array->len]; \
} \
\
static type *su_array__##type##__put(su_array__##type##__t *array, su_allocator_t *alloc, type item, size_t idx) { \
    SU_ASSERT(idx <= array->len); \
    if (idx == array->len) { \
        if (array->size == array->len) { \
            su_array__##type##__resize(array, alloc, array->size * 2); \
        } \
        array->len++; \
    } \
	array->items[idx] = item; \
	return &array->items[idx]; \
} \
\
static SU_ATTRIBUTE_PURE type su_array__##type##__get(su_array__##type##__t *array, size_t idx) { \
    SU_ASSERT(idx < array->len); \
	return array->items[idx]; \
} \
\
static SU_ATTRIBUTE_PURE type *su_array__##type##__get_ptr(su_array__##type##__t *array, size_t idx) { \
    SU_ASSERT(idx < array->len); \
	return &array->items[idx]; \
} \
\
static void su_array__##type##__qsort(su_array__##type##__t *array, int compare(const void *, const void *)) { \
    qsort(array->items, array->len, sizeof(type), compare); \
}

#define SU_STACK_DEFINE(type) \
\
SU_ARRAY_DEFINE(type) \
\
static void su_stack__##type##__init(su_stack__##type##__t *stack, su_allocator_t *alloc, size_t initial_size) { \
	su_array__##type##__init(&stack->data, alloc, initial_size); \
} \
\
static void su_stack__##type##__fini(su_stack__##type##__t *stack, su_allocator_t *alloc) { \
	su_array__##type##__fini(&stack->data, alloc); \
} \
\
static void su_stack__##type##__resize(su_stack__##type##__t *stack, su_allocator_t *alloc, size_t new_size) { \
	su_array__##type##__resize(&stack->data, alloc, new_size); \
} \
\
static type *su_stack__##type##__push(su_stack__##type##__t *stack, su_allocator_t *alloc, type item) { \
	return su_array__##type##__add(&stack->data, alloc, item); \
} \
\
static type *su_stack__##type##__push_nocheck(su_stack__##type##__t *stack, type item) { \
	return su_array__##type##__add_nocheck(&stack->data, item); \
} \
\
static type su_stack__##type##__pop(su_stack__##type##__t *stack) { \
	type ret = su_array__##type##__get(&stack->data, stack->data.len - 1); \
	su_array__##type##__remove(&stack->data, 1); \
	return ret; \
} \
\
static SU_ATTRIBUTE_PURE type su_stack__##type##__get(su_stack__##type##__t *stack) { \
	return su_array__##type##__get(&stack->data, stack->data.len - 1); \
}

#define SU_LLIST_DEFINE(type) \
static void su_llist__##type##__insert_head(su_llist__##type##__t *list, type *node) { \
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
static void su_llist__##type##__insert_tail(su_llist__##type##__t *list, type *node) { \
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
static void su_llist__##type##__pop(su_llist__##type##__t *list, type *node) { \
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
/* TODO: insert_list|after|before */

#define SU_HASH_TABLE_DEFINE(type, key_type, hash_key_func, keys_equal_func, collisions_to_resize) \
\
SU_ARRAY_DEFINE(type) \
\
static void su_hash_table__##type##__init(su_hash_table__##type##__t *ht, su_allocator_t *alloc, size_t initial_size) { \
	su_array__##type##__init0(&ht->items, alloc, initial_size); \
	ht->items.len = initial_size; \
} \
\
static void su_hash_table__##type##__fini(su_hash_table__##type##__t *ht, su_allocator_t *alloc) { \
	su_array__##type##__fini(&ht->items, alloc); \
} \
\
static void su_hash_table__##type##__resize(su_hash_table__##type##__t *ht, su_allocator_t *alloc, size_t new_size) { \
	size_t i; \
	su_hash_table__##type##__t new_ht; \
	su_hash_table__##type##__init(&new_ht, alloc, new_size); \
	for ( i = 0; i < ht->items.len; ++i) { \
		type it = su_array__##type##__get(&ht->items, i); \
		if (it.occupied && !it.tombstone) { \
			su_hash_table__##type##__add(&new_ht, alloc, it.key, NULL); \
		} \
	} \
	su_hash_table__##type##__fini(ht, alloc); \
	*ht = new_ht; \
} \
\
static su_bool32_t su_hash_table__##type##__add(su_hash_table__##type##__t *ht, su_allocator_t *alloc, key_type key, type **out) { \
	size_t h = hash_key_func(key) % ht->items.len; \
	type *it = su_array__##type##__get_ptr(&ht->items, h); \
	size_t c = 0; \
	for ( ; \
			it->occupied && !keys_equal_func(it->key, key) && (c < ht->items.len); \
			++c) { \
		it = su_array__##type##__get_ptr(&ht->items, (++h % ht->items.len)); \
	} \
	if (c >= collisions_to_resize) { \
		su_hash_table__##type##__resize(ht, alloc, ht->items.size * 2); \
		su_hash_table__##type##__add(ht, alloc, key, out); \
	} else if (it->occupied) { \
		if (keys_equal_func(it->key, key)) { \
			if (out) { \
				*out = it; \
			} \
			return SU_FALSE; \
		} \
		su_hash_table__##type##__resize(ht, alloc, ht->items.size * 2); \
		su_hash_table__##type##__add(ht, alloc, key, out); \
	} else { \
		it->key = key; \
		it->occupied = SU_TRUE; \
		if (out) { \
			*out = it; \
		} \
	} \
	return SU_TRUE; \
} \
\
static su_bool32_t su_hash_table__##type##__get(su_hash_table__##type##__t *ht, key_type key, type **out) { \
	size_t h = hash_key_func(key) % ht->items.len; \
	type *it = su_array__##type##__get_ptr(&ht->items, h); \
	size_t c = 0; \
	for ( ; \
			it->occupied && !keys_equal_func(it->key, key) && (c < ht->items.len); \
			++c) { \
		it = su_array__##type##__get_ptr(&ht->items, (++h % ht->items.len)); \
	} \
	if (it->occupied && keys_equal_func(it->key, key)) { \
		*out = it; \
		return SU_TRUE; \
	} else { \
		return SU_FALSE; \
	} \
} \
\
static su_bool32_t su_hash_table__##type##__del(su_hash_table__##type##__t *ht, key_type key, type *out) { \
	size_t h = hash_key_func(key) % ht->items.len; \
	type *it = su_array__##type##__get_ptr(&ht->items, h); \
	size_t c = 0; \
	for ( ; \
			it->occupied && !keys_equal_func(it->key, key) && (c < ht->items.len); \
			++c) { \
		it = su_array__##type##__get_ptr(&ht->items, (++h % ht->items.len)); \
	} \
	if (it->occupied && keys_equal_func(it->key, key)) { \
		if (out) { \
			*out = *it; \
		} \
		it->occupied = SU_TRUE; \
		it->tombstone = SU_TRUE; \
		return SU_TRUE; \
	} else { \
		return SU_FALSE; \
	} \
}
/* ? TODO: tombstone threshold, shrink */

#define SU_ARRAY_DECLARE_DEFINE(type) \
	SU_ARRAY_DECLARE(type); \
	SU_ARRAY_DEFINE(type)

#define SU_STACK_DECLARE_DEFINE(type) \
	SU_STACK_DECLARE(type); \
	SU_STACK_DEFINE(type)

#define SU_LLIST_DECLARE_DEFINE(type) \
	SU_LLIST_DECLARE(type); \
	SU_LLIST_DEFINE(type)

#define SU_HASH_TABLE_DECLARE_DEFINE(type, key_type, hash_key_func, keys_equal_func, collisions_to_resize) \
	SU_HASH_TABLE_DECLARE(type, key_type, hash_key_func, keys_equal_func, collisions_to_resize); \
	SU_HASH_TABLE_DEFINE(type, key_type, hash_key_func, keys_equal_func, collisions_to_resize)

SU_ARRAY_DEFINE(su_string_t)
SU_ARRAY_DEFINE(su_arena_block_t)
SU_STACK_DEFINE(su__json_writer_state_t)
SU_STACK_DEFINE(su__json_tokener_state_t)
SU_ARRAY_DEFINE(su_json_ast_node_t)
SU_ARRAY_DEFINE(su_json_ast_key_value_t)

static su_bool32_t su_fat_ptr_equal(su_fat_ptr_t a, su_fat_ptr_t b) {
	if (a.len != b.len) {
		return SU_FALSE;
	}

	return (memcmp(a.ptr, b.ptr, a.len) == 0);
}

static char *su__log_va_stbsp_vsprintfcb(const char *buf, void *data, int len) {
	int fd = (int)(intptr_t)data;
	ssize_t total = 0;
	while (total < len) {
		ssize_t written_bytes = write(fd, &buf[total], (size_t)(len - total));
		if (written_bytes == -1) {
			return NULL;
		}
		total += written_bytes;
	}

	return (char *)(uintptr_t)buf;
}

static SU_ATTRIBUTE_FORMAT_PRINTF(2, 0) void su_log_va(int fd, const char *fmt, va_list args) {
	/* TODO: format -> buf (scratch alloc) -> write */
	char buf[STB_SPRINTF_MIN];
	ssize_t suppress_warn_unused_result;
	SU_NOTUSED(suppress_warn_unused_result);
#if defined(SU_LOG_PREFIX)
	suppress_warn_unused_result = write(fd, SU_LOG_PREFIX, SU_STRING_LITERAL_LENGTH(SU_LOG_PREFIX));
#endif /* defined(SU_LOG_PREFIX) */
	stbsp_vsprintfcb(su__log_va_stbsp_vsprintfcb, (void *)(intptr_t)fd, buf, fmt, args);
	suppress_warn_unused_result = write(fd, "\n", 1);
}

static SU_ATTRIBUTE_FORMAT_PRINTF(1, 2) void su_log_stdout(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	su_log_va(STDOUT_FILENO, fmt, args);
	va_end(args);
}

static SU_ATTRIBUTE_FORMAT_PRINTF(1, 2) void su_log_stderr(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	su_log_va(STDERR_FILENO, fmt, args);
	va_end(args);
}

static SU_ATTRIBUTE_NORETURN SU_ATTRIBUTE_FORMAT_PRINTF(2, 3) void su_abort(int code, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
    su_log_va(STDERR_FILENO, fmt, args);
	va_end(args);
    exit(code);
}

static inline SU_ATTRIBUTE_ALWAYS_INLINE su_string_t su_string(const char *literal) {
	su_string_t s;
	s.s = (char *)(uintptr_t)literal;
	s.len = SU_STRLEN(literal);
	s.free_contents = SU_FALSE;
	s.nul_terminated = SU_TRUE;

	return s;
}

static char *su__string_init_format_stbsp_vsprintfcb_callback(const char *buf, void *data, int len) {
	struct d {
		su_string_t *str;
		su_allocator_t *alloc;
	} *_data = (struct d *)data;
	su_string_t *str = _data->str;
	su_allocator_t *alloc = _data->alloc;
	
	SU_NOTUSED(buf);

	str->len += (size_t)len;

	if (len == STB_SPRINTF_MIN) {
		/* TODO: handle last cb when len == STB_SPRINTF_MIN */
		str->s = (SU_TYPEOF(str->s))alloc->realloc(
			alloc, str->s, str->len + STB_SPRINTF_MIN, SU_ALIGNOF(*str->s));
	}

	str->s[str->len] = '\0';
	return &str->s[str->len];
}

static SU_ATTRIBUTE_FORMAT_PRINTF(3, 4) void su_string_init_format(su_string_t *str,
		su_allocator_t *alloc, const char *fmt, ...) {
	struct data {
		su_string_t *str;
		su_allocator_t *alloc;
	} data;

	va_list args;
	va_start(args, fmt);

	data.str = str;
	data.alloc = alloc;

	str->len = 0;
	str->s = (SU_TYPEOF(str->s))alloc->alloc(alloc, STB_SPRINTF_MIN, SU_ALIGNOF(*str->s));
	stbsp_vsprintfcb(su__string_init_format_stbsp_vsprintfcb_callback, &data, str->s, fmt, args);
	str->free_contents = SU_TRUE;
	str->nul_terminated = SU_TRUE;

	va_end(args);
}

static void su_string_init_len(su_string_t *str, su_allocator_t *alloc,
		const char *s, size_t len, su_bool32_t nul_terminate) {
	SU_ASSERT((len > 0) || nul_terminate);
	str->s = (SU_TYPEOF(str->s))alloc->alloc(alloc, len + nul_terminate, SU_ALIGNOF(*str->s));
	memcpy(str->s, s, len);
	if (nul_terminate) {
		str->s[len] = '\0';
	}
	str->len = len;
	str->free_contents = SU_TRUE;
	str->nul_terminated = nul_terminate;
}

static void su_string_init_string(su_string_t *str, su_allocator_t *alloc, su_string_t src) {
	su_string_init_len(str, alloc, src.s, src.len, src.nul_terminated);
}

static void su_string_init(su_string_t *str, su_allocator_t *alloc, const char *src) {
	str->len = strlen(src);
	str->s = (SU_TYPEOF(str->s))alloc->alloc(alloc, str->len + 1, SU_ALIGNOF(*str->s));
	memcpy(str->s, src, str->len + 1);
	str->free_contents = SU_TRUE;
	str->nul_terminated = SU_TRUE;
}

static void su_string_fini(su_string_t *str, su_allocator_t *alloc) {
	if (str->free_contents) {
		alloc->free(alloc, str->s);
	}
}

static su_string_t su_string_view(su_string_t str) {
	str.free_contents = SU_FALSE;
	return str;
}

static su_bool32_t su_string_equal(su_string_t str1, su_string_t str2) {
	if (str1.len != str2.len) {
		return SU_FALSE;
	}

	return (memcmp(str1.s, str2.s, str1.len) == 0);
}

static int su_string_compare(su_string_t str1, su_string_t str2, size_t max) {
    size_t len = ((max < str1.len) && (max < str2.len)) ? max : SU_MIN(str1.len, str2.len);

    int result = memcmp(str1.s, str2.s, len);
    if ((result != 0) || (len == max)) {
		return result;
	}

    return ((str1.len < str2.len) ? -1 : ((str1.len > str2.len) ? 1 : 0));
}

static su_bool32_t su_string_find_char(su_string_t str, char c, su_string_t *view_out) {
	char *s = (char *)memchr(str.s, c, str.len);
	if (!s) {
		return SU_FALSE;
	}

	view_out->s = s;
	view_out->len = ((size_t)&str.s[str.len] - (size_t)s);
	view_out->free_contents = SU_FALSE;
	view_out->nul_terminated = str.nul_terminated;

	return SU_TRUE;
}

static su_bool32_t su_string_tok(su_string_t *str, char delim, su_string_t *token_out, su_string_t *saveptr) {
	/* TODO: simd */

    if (str) {
		SU_ASSERT(str->s != NULL);
		SU_ASSERT(str->len > 0);
		saveptr->s = str->s;
		saveptr->len = str->len;
		saveptr->free_contents = SU_FALSE;
		saveptr->nul_terminated = SU_FALSE; /* TODO: return str->nul_terminated on last token */
	}

    while ((saveptr->len > 0) && (*saveptr->s == delim)) {
        saveptr->s++;
        saveptr->len--;
    }
    if (saveptr->len == 0) {
		return SU_FALSE;
	}

    *token_out = *saveptr;
    while ((saveptr->len > 0) && (*saveptr->s != delim)) {
        saveptr->s++;
        saveptr->len--;
    }

	token_out->len -= saveptr->len;
	return SU_TRUE;
}

static su_bool32_t su_string_starts_with(su_string_t str, su_string_t prefix) {
	if (prefix.len <= str.len) {
		su_string_t s;
		s.s = str.s;
		s.len = prefix.len;
		s.free_contents = SU_FALSE;
		s.nul_terminated = SU_FALSE;
		return su_string_equal(prefix, s);
	}

	return SU_FALSE;
}

static su_bool32_t su_string_ends_with(su_string_t str, su_string_t suffix) {
	if (suffix.len <= str.len) {
		su_string_t s;
		s.s = &str.s[str.len - suffix.len];
		s.len = suffix.len;
		s.free_contents = SU_FALSE;
		s.nul_terminated = str.nul_terminated;
		return su_string_equal(suffix, s);
	}

	return SU_FALSE;
}

/* on success, returns u16 in lower bits of u32 result */
/* on error, returns u32 with high bits set */
static SU_ATTRIBUTE_PURE uint32_t su_string_hex_16_to_uint16(su_string_t str) {
	static uint32_t lut[886] = {
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

	SU_ASSERT(str.len == 4);

	return  lut[630 + (uint8_t)str.s[0]] |
			lut[420 + (uint8_t)str.s[1]] |
			lut[210 + (uint8_t)str.s[2]] |
			lut[0   + (uint8_t)str.s[3]];
}

static su_bool32_t su_string_to_uint64(su_string_t str, uint64_t *out) {
	uint64_t result = 0;
	size_t i = 0;

	if (SU_UNLIKELY((str.len == 0) || (str.len > 20) || ((str.s[0] == '0') && (str.len > 1)))) {
		return SU_FALSE;
	}

	for ( ; i < str.len; ++i) {
		uint8_t d = (uint8_t)(str.s[i] - '0');
		if (SU_UNLIKELY(d > 9)) {
			return SU_FALSE;
		}
		result = 10 * result + d;
	}

	if (SU_UNLIKELY((str.len == 20) && (str.s[0] == '1') && (result <= (uint64_t)INT64_MAX))) {
		return SU_FALSE;
	}

	*out = result;
	return SU_TRUE;
}

static su_bool32_t su_string_to_int64(su_string_t str, int64_t *out) {
	su_bool32_t negative = (str.s[0] == '-');
	uint64_t result = 0;
	size_t i = negative;

	if (SU_UNLIKELY((str.len == 0) || (str.len > 20) || ((str.s[0] == '0') && (str.len > 1)))) {
		return SU_FALSE;
	}

	for ( ; i < str.len; ++i) {
		uint8_t d = (uint8_t)(str.s[i] - '0');
		if (SU_UNLIKELY(d > 9)) {
			return SU_FALSE;
		}
		result = 10 * result + d;
	}

	if (SU_UNLIKELY(result > ((uint64_t)INT64_MAX + negative))) {
		return SU_FALSE;
	}

	*out = negative ? (int64_t)(~result + 1) : (int64_t)result;
	return SU_TRUE;
}

static void su_arena_init(su_arena_t *arena, su_allocator_t *alloc, size_t initial_block_size) {
	su_array__su_arena_block_t__init(&arena->blocks, alloc, 8);
	su_arena_add_block(arena, alloc, initial_block_size);
}

static void su_arena_fini(su_arena_t *arena, su_allocator_t *alloc) {
	size_t i = 0;
	for ( ; i < arena->blocks.len; ++i) {
		alloc->free(alloc, su_array__su_arena_block_t__get(&arena->blocks, i).data);
	}
	su_array__su_arena_block_t__fini(&arena->blocks, alloc);
}

static su_arena_block_t *su_arena_add_block(su_arena_t *arena, su_allocator_t *alloc, size_t size) {
	su_arena_block_t block;
	
	size = (size + (4096 - 1)) & (size_t)~(4096 - 1);
	
	block.data = (SU_TYPEOF(block.data))alloc->alloc(alloc, size, 4096);
	block.size = size;
	block.ptr = 0;

	return su_array__su_arena_block_t__add(&arena->blocks, alloc, block);
}

static void *su_arena_alloc(su_arena_t *arena, su_allocator_t *alloc, size_t size, size_t alignment) {
	su_arena_block_t *block;
	size_t new_size = size;
	size_t new_ptr;
	void *ret;
	size_t i = 0;

	SU_ASSERT(size > 0);
	SU_ASSERT((alignment > 0) && ((alignment == 1) || ((alignment & (alignment - 1)) == 0)));

	alignment = SU_MAX(alignment, sizeof(size_t));

	do {
		block = su_array__su_arena_block_t__get_ptr(&arena->blocks, i);
		new_ptr = (sizeof(size_t) + block->ptr + (alignment - 1)) & ~(alignment - 1);
		if (SU_LIKELY((new_ptr < block->size) && (size <= (block->size - new_ptr)))) {
			goto out;
		}
		new_size += block->size;
	} while (SU_UNLIKELY(++i < arena->blocks.len));

	block = su_arena_add_block(arena, alloc, (new_size + alignment) * 2);
	new_ptr = (sizeof(size_t) + (alignment - 1)) & ~(alignment - 1);

out:
	memcpy(&block->data[new_ptr - sizeof(size_t)], &size, sizeof(size));
	ret = &block->data[new_ptr];
	block->ptr = new_ptr + size;
	SU_ASSERT(((uintptr_t)ret % alignment) == 0);
	return ret;
}

static size_t su_arena_alloc_get_size(void *ptr) {
	size_t ret;
	memcpy(&ret, (uint8_t *)ptr - sizeof(size_t), sizeof(ret));
	return ret;
}

static void su_arena_reset(su_arena_t *arena, su_allocator_t *alloc) {
	su_arena_block_t *first_block = su_array__su_arena_block_t__get_ptr(&arena->blocks, 0);
	size_t size = first_block->size;
	size_t i = 1;

	SU_ASSERT(arena->blocks.len > 0);

	for ( ; i < arena->blocks.len; ++i) {
		su_arena_block_t block = su_array__su_arena_block_t__get(&arena->blocks, i);
		size += block.size;
		alloc->free(alloc, block.data);
	}
	arena->blocks.len = 1;

	SU_ASSERT((size % 4096) == 0);

	if (size > first_block->size) {
		alloc->free(alloc, first_block->data);
		first_block->data = (SU_TYPEOF(first_block->data))alloc->alloc(alloc, size, 4096);
		first_block->size = size;
	}
	first_block->ptr = 0;
}

/*static SU_ATTRIBUTE_PURE size_t su_sdbm_hash(su_string_t s) {
    size_t hash = 0;
	size_t i = 0;
    for ( ; i < s.len; ++i) {
        hash = (size_t)s.s[i] + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}*/

/*static SU_ATTRIBUTE_PURE size_t su_djb2_hash(su_string_t s) {
	size_t hash = 5381;
	size_t i = 0;

	for ( ; i < s.len; ++i) {
		hash = ((hash << 5) + hash) + (size_t)s.s[i];
	}

	return hash;
}*/

static SU_ATTRIBUTE_PURE size_t su_stbds_hash_string(su_string_t s) {
#define ROTATE_LEFT(val, n)  (((val) << (n)) | ((val) >> (((sizeof(size_t)) * 8) - (n))))
#define ROTATE_RIGHT(val, n) (((val) >> (n)) | ((val) << (((sizeof(size_t)) * 8) - (n))))

	/* TODO: simd */

	size_t hash = 0;
	size_t i = 0;
	for ( ; i < s.len; ++i) {
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

static SU_ATTRIBUTE_PURE size_t su_stbds_hash(su_fat_ptr_t data) {
	su_string_t s;
	s.s = data.ptr;
	s.len = data.len;
	return su_stbds_hash_string(s);
}

static void su_argb_premultiply_alpha(uint32_t *dest, uint32_t *src, size_t count) {
	size_t i = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
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

	SU_ASSERT(((uintptr_t)dest % 32) == 0);

	for ( ; (i + 8) <= count; i += 8) {
		__m256i px = _mm256_loadu_si256((__m256i_u *)&src[i]);

		__m256i lo = _mm256_unpacklo_epi8(px, zero);
		__m256i hi = _mm256_unpackhi_epi8(px, zero);

		__m256i alpha_lo = _mm256_shuffle_epi8(lo, extract_alpha_mask);
		__m256i alpha_hi = _mm256_shuffle_epi8(hi, extract_alpha_mask);

		__m256i packed, result;

		lo = _mm256_mullo_epi16(lo, alpha_lo);
		hi = _mm256_mullo_epi16(hi, alpha_hi);

		lo = _mm256_add_epi16(lo, const_127);
		hi = _mm256_add_epi16(hi, const_127);

		lo = _mm256_mulhi_epu16(lo, const_257);
		hi = _mm256_mulhi_epu16(hi, const_257);

		packed = _mm256_packus_epi16(lo, hi);
		result = _mm256_blendv_epi8(packed, px, blend_mask);

		_mm256_store_si256((__m256i *)(void *)&dest[i], result);
	}
#endif /* SU_WITH_SIMD && __AVX2__ */
/* TODO: ARM SIMD */

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

static void su_abgr_to_argb_premultiply_alpha(uint32_t *dest, uint32_t *src, size_t count) {
	size_t i = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
	__m256i abgr_to_argb_mask = _mm256_set_epi8(
		31,28,29,30,
		27,24,25,26,
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

	SU_ASSERT(((uintptr_t)dest % 32) == 0);

    for ( ; (i + 8) <= count; i += 8) {
        __m256i abgr = _mm256_loadu_si256((__m256i_u *)&src[i]);

		__m256i abgr_lo = _mm256_unpacklo_epi8(abgr, zero);
		__m256i abgr_hi = _mm256_unpackhi_epi8(abgr, zero);

		__m256i alpha_lo = _mm256_shuffle_epi8(abgr_lo, extract_alpha_mask);
		__m256i alpha_hi = _mm256_shuffle_epi8(abgr_hi, extract_alpha_mask);

		__m256i abgr_packed, argb;

		abgr_lo = _mm256_mullo_epi16(abgr_lo, alpha_lo);
		abgr_hi = _mm256_mullo_epi16(abgr_hi, alpha_hi);

		abgr_lo = _mm256_add_epi16(abgr_lo, const_127);
		abgr_hi = _mm256_add_epi16(abgr_hi, const_127);

		abgr_lo = _mm256_mulhi_epu16(abgr_lo, const_257);
		abgr_hi = _mm256_mulhi_epu16(abgr_hi, const_257);

		abgr_packed = _mm256_packus_epi16(abgr_lo, abgr_hi);
		abgr = _mm256_blendv_epi8(abgr_packed, abgr, blend_mask);

		argb = _mm256_shuffle_epi8(abgr, abgr_to_argb_mask);

        _mm256_store_si256((__m256i *)(void *)&dest[i], argb);
    }
#endif /* SU_WITH_SIMD && __AVX2__ */
/* TODO: ARM SIMD */

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

static void su_abgr_to_argb(uint32_t *dest, uint32_t *src, size_t count) {
	size_t i = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
	__m256i mask = _mm256_set_epi8(
		31,28,29,30,
		27,24,25,26,
		23,20,21,22,
		19,16,17,18,
		15,12,13,14,
		11,8, 9, 10,
		7, 4, 5, 6,
		3, 0, 1, 2
	);

	SU_ASSERT(((uintptr_t)dest % 32) == 0);

    for ( ; (i + 8) <= count; i += 8) {
        __m256i abgr = _mm256_loadu_si256((__m256i_u *)&src[i]);
        __m256i argb = _mm256_shuffle_epi8(abgr, mask);
        _mm256_store_si256((__m256i *)(void *)&dest[i], argb);
    }
#endif /* SU_WITH_SIMD && __AVX2__ */
/* TODO: ARM SIMD */

	for ( ; i < count; ++i) {
		uint32_t p = src[i];
		uint8_t a = (uint8_t)((p >> 24) & 0xFF);
		uint8_t b = (uint8_t)((p >> 16) & 0xFF);
		uint8_t g = (uint8_t)((p >> 8) & 0xFF);
		uint8_t r = (uint8_t)((p >> 0) & 0xFF);
		dest[i] = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b << 0);
	}
}

static su_bool32_t su_read_entire_file(su_string_t path, su_fat_ptr_t *out, su_allocator_t *alloc) {\
	int fd;
	struct stat sb;
	su_fat_ptr_t data;
	size_t bytes_read;

	SU_ASSERT(path.nul_terminated); /* TODO: handle properly */

	if ((fd = open(path.s, O_RDONLY)) == -1) {
		return SU_FALSE;
	}

	if (fstat(fd, &sb) == -1) {
		goto error;
	}

	data.len = (size_t)sb.st_size;
	data.ptr = alloc->alloc(alloc, data.len + 1, 64);

	bytes_read = 0;
	while (bytes_read < data.len) {
		ssize_t r = read(fd, (uint8_t *)data.ptr + bytes_read, data.len - bytes_read);
		if (r == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				alloc->free(alloc, data.ptr);
				goto error;
			}
		}
		SU_ASSERT(r != 0); /* TODO */
		bytes_read += (size_t)r;
	}

	close(fd);
	*out = data;

	return SU_TRUE;
error:
	close(fd);
	return SU_FALSE;
}

/* TODO: better hash function */
SU_HASH_TABLE_DEFINE(su_file_cache_t, su_string_t, su_stbds_hash_string, su_string_equal, 16)

static su_bool32_t su_read_entire_file_with_cache(su_string_t path, su_fat_ptr_t *out,
		su_allocator_t *alloc, su_hash_table__su_file_cache_t__t *cache) {
	su_file_cache_t *e = NULL;
	static char buf[PATH_MAX];
	struct stat sb;

	SU_ASSERT(path.nul_terminated); /* TODO: handle properly */

	if (realpath(path.s, buf) == NULL) {
		goto error;
	}

	path.s = buf;
	path.len = strlen(buf);
	path.nul_terminated = SU_TRUE;
	path.free_contents = SU_FALSE;

	if (stat(path.s, &sb) == -1) {
		goto error;
	}

	if (su_hash_table__su_file_cache_t__get(cache, path, &e)) {
		if (memcmp(&sb.st_mtim, &e->st_mtim, sizeof(sb.st_mtim)) == 0) {
			goto out;
		}
		alloc->free(alloc, e->data.ptr);
		memset(&e->data, 0, sizeof(e->data));
	} else {
		su_hash_table__su_file_cache_t__add(cache, alloc, path, &e);
		su_string_init_string(&e->key, alloc, path);
	}

	if (!su_read_entire_file(path, &e->data, alloc)) {
		goto error;
	}

	e->st_mtim = sb.st_mtim;

out:
	*out = e->data;
	return SU_TRUE;
error: {
		su_file_cache_t del;
		if (su_hash_table__su_file_cache_t__del(cache, path, &del)) {
			su_string_fini(&del.key, alloc);
			alloc->free(alloc, del.data.ptr);
		}
		return SU_FALSE;
	}
}

static su_bool32_t su_fd_set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        return SU_FALSE;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return SU_FALSE;
    }

	return SU_TRUE;
}

static su_bool32_t su_fd_set_cloexec(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1) {
        return SU_FALSE;
    }

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
        return SU_FALSE;
    }

	return SU_TRUE;
}

static int64_t su_timespec_to_ms(struct timespec timespec) {
	return (timespec.tv_sec * 1000) + (timespec.tv_nsec / 1000000);
}

static int64_t su_now_ms(clockid_t clock_id) {
	struct timespec ts;
	int ret = clock_gettime(clock_id, &ts);
	SU_NOTUSED(ret);
	SU_ASSERT(ret == 0);
	return su_timespec_to_ms(ts);
}

static su_bool32_t su_locale_is_utf8(void) {
	char32_t w;
	mbstate_t s;
	
	memset(&s, 0, sizeof(s));

	if (mbrtoc32(&w, "\xC3\xB6", 2, &s) != 2) {
		return SU_FALSE;
	}

	return (w == 0xF6);
}

static void su_nop(void *notused, ...) {
	SU_NOTUSED(notused);
}

/*static void *su_base64_decode(su_string_t text, su_allocator_t *alloc) {
	uint8_t *ret;
	size_t i = 0, o = 0;

    if (text.len % 4 != 0) {
        return NULL;
    }

	ret = alloc->alloc(alloc, text.len / 4 * 3, SU_ALIGNOF(*ret));

    for ( ; i < text.len; i += 4, o += 3) {
		static uint8_t rlut[] = {
		    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		    255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
		     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
		    255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
		    255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
		    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		};

        uint32_t a = rlut[(uint8_t)text.s[i + 0]];
        uint32_t b = rlut[(uint8_t)text.s[i + 1]];
        uint32_t c = rlut[(uint8_t)text.s[i + 2]];
        uint32_t d = rlut[(uint8_t)text.s[i + 3]];

        uint32_t u = a | b | c | d;
		uint32_t v;
        if (u & 128) {
            goto error;
        }

        if (u & 64) {
            if (i + 4 != text.len || (a | b) & 64 || (c & 64 && !(d & 64))) {
                goto error;
            }
            c &= 63;
            d &= 63;
        }

		v = a << 18 | b << 12 | c << 6 | d << 0;
        ret[o + 0] = (uint8_t)((v >> 16) & 0xFF);
        ret[o + 1] = (uint8_t)((v >> 8) & 0xFF);
        ret[o + 2] = (uint8_t)((v >> 0) & 0xFF);
    }

    return ret;
error:
    alloc->free(alloc, ret);
    return NULL;
}*/

static void su__json_buffer_add_char(su_json_buffer_t *buffer, su_allocator_t *alloc, char c) {
	if (buffer->size == buffer->idx) {
		buffer->size = (buffer->size + 1) * 2;
		buffer->data = (SU_TYPEOF(buffer->data))alloc->realloc(
			alloc, buffer->data, buffer->size, SU_ALIGNOF(*buffer->data));
	}
	buffer->data[buffer->idx++] = c;
}

static void su__json_buffer_add_char_nocheck(su_json_buffer_t *buffer, char c) {
	buffer->data[buffer->idx++] = c;
}

static void su__json_buffer_put_char_nocheck(su_json_buffer_t *buffer, size_t *idx, char c) {
	buffer->data[*idx] = c;
	*idx += 1;
}

static void su__json_buffer_put_string_nocheck(su_json_buffer_t *buffer, size_t *idx, su_string_t str) {
	memcpy(&buffer->data[*idx], str.s, str.len);
	*idx += str.len;
}

static void su__json_buffer_add_string(su_json_buffer_t *buffer, su_allocator_t *alloc, su_string_t str) {
	if ((buffer->idx + str.len) > buffer->size) {
		buffer->size = (buffer->size + str.len) * 2;
		buffer->data = (SU_TYPEOF(buffer->data))alloc->realloc(
			alloc, buffer->data, buffer->size, SU_ALIGNOF(*buffer->data));
	}
	memcpy(&buffer->data[buffer->idx], str.s, str.len);
	buffer->idx += str.len;
}

static void su__json_buffer_add_string_nocheck(su_json_buffer_t *buffer, su_string_t str) {
	memcpy(&buffer->data[buffer->idx], str.s, str.len);
	buffer->idx += str.len;
}

static SU_ATTRIBUTE_FORMAT_PRINTF(3, 4) void su__json_buffer_add_format(su_json_buffer_t *buffer,
		su_allocator_t *alloc, const char *fmt, ...) {
	char buf[512];
	size_t len;

	va_list args;
    va_start(args, fmt);

	len = (size_t)stbsp_vsnprintf(buf, sizeof(buf), fmt, args);
	if ((buffer->idx + len) > buffer->size) {
		buffer->size = (buffer->size + len) * 2;
		buffer->data = (SU_TYPEOF(buffer->data))alloc->realloc(
			alloc, buffer->data, buffer->size, SU_ALIGNOF(*buffer->data));
	}
	memcpy(&buffer->data[buffer->idx], buf, len);
	buffer->idx += len;

    va_end(args);
}

static void su__json_buffer_add_string_escape(su_json_buffer_t *buf, su_allocator_t *alloc, su_string_t str) {
	/* TODO: simd */

	size_t i = 0;
	for ( ; i < str.len; ++i) {
		unsigned char c = (unsigned char)str.s[i];
		switch (c) {
		case '\b':
		case '\t':
		case '\n':
		case '\f':
		case '\r': {
			static char specials[] = { 'b', 't', 'n', '_'/* unreachable */, 'f', 'r' };
			su__json_buffer_add_char(buf, alloc, '\\');
			su__json_buffer_add_char(buf, alloc, specials[c - '\b']);
			break;
		}
		case '"':
		case '\\':
			su__json_buffer_add_char(buf, alloc, '\\');
			su__json_buffer_add_char(buf, alloc, (char)c);
			break;
		default:
			if (c <= 0x1F) {
				static char hex_chars[] = "0123456789abcdef";
				su__json_buffer_add_string(buf, alloc, su_string("\\u00"));
				su__json_buffer_add_char(buf, alloc, hex_chars[c >> 4]);
				su__json_buffer_add_char(buf, alloc, hex_chars[c & 0xF]);
			} else {
				su__json_buffer_add_char(buf, alloc, (char)c);
			}
			break;
		}
	}
}

static su__json_writer_state_t su__json_writer_get_state(su_json_writer_t *writer) {
	if (writer->state.data.len > 0) {
		return su_stack__su__json_writer_state_t__get(&writer->state);
	}

	return SU__JSON_WRITER_STATE_ROOT;
}

static void su__json_writer_element(su_json_writer_t *writer, su_allocator_t *alloc) {
	switch (su__json_writer_get_state(writer)) {
	case SU__JSON_WRITER_STATE_ROOT:
		break;
	case SU__JSON_WRITER_STATE_OBJECT:
		su_stack__su__json_writer_state_t__pop(&writer->state);
		su_stack__su__json_writer_state_t__push(&writer->state, alloc,
				SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA);
		break;
	case SU__JSON_WRITER_STATE_ARRAY:
		su_stack__su__json_writer_state_t__pop(&writer->state);
		su_stack__su__json_writer_state_t__push(&writer->state, alloc,
				SU__JSON_WRITER_STATE_ARRAY_EXPECTING_COMMA);
		break;
	case SU__JSON_WRITER_STATE_KEY:
		su_stack__su__json_writer_state_t__pop(&writer->state);
		break;
	case SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA:
	case SU__JSON_WRITER_STATE_ARRAY_EXPECTING_COMMA:
		su__json_buffer_add_char(&writer->buf, alloc, ',');
		break;
	default:
		SU_ASSERT_UNREACHABLE;
	}
}

static void su_json_writer_init(su_json_writer_t *writer, su_allocator_t *alloc, size_t initial_bufsize) {
	su_stack__su__json_writer_state_t__init(&writer->state, alloc, 128);
	writer->buf.data = (SU_TYPEOF(writer->buf.data))alloc->alloc(
		alloc, initial_bufsize, SU_ALIGNOF(*writer->buf.data));
	writer->buf.size = initial_bufsize;
	writer->buf.idx = 0;
}

static void su_json_writer_fini(su_json_writer_t *writer, su_allocator_t *alloc) {
	alloc->free(alloc, writer->buf.data);
	su_stack__su__json_writer_state_t__fini(&writer->state, alloc);
}

static void su_json_writer_reset(su_json_writer_t *writer) {
	writer->buf.idx = 0;
	writer->state.data.len = 0;
}

static void su_json_writer_object_begin(su_json_writer_t *writer, su_allocator_t *alloc) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	su__json_buffer_add_char(&writer->buf, alloc, '{');
	su_stack__su__json_writer_state_t__push(&writer->state, alloc, SU__JSON_WRITER_STATE_OBJECT);
}

static void su_json_writer_object_end(su_json_writer_t *writer, su_allocator_t *alloc) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state == SU__JSON_WRITER_STATE_OBJECT) || (state == SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_buffer_add_char(&writer->buf, alloc, '}');
	su_stack__su__json_writer_state_t__pop(&writer->state);
}

static void su_json_writer_object_key_escape(su_json_writer_t *writer, su_allocator_t *alloc, su_string_t key) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state == SU__JSON_WRITER_STATE_OBJECT) || (state == SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	su__json_buffer_add_char(&writer->buf, alloc, '"');
	su__json_buffer_add_string_escape(&writer->buf, alloc, key);
	su__json_buffer_add_char(&writer->buf, alloc, '"');
	su__json_buffer_add_char(&writer->buf, alloc, ':');
	su_stack__su__json_writer_state_t__push(&writer->state, alloc, SU__JSON_WRITER_STATE_KEY);
}

static void su_json_writer_object_key(su_json_writer_t *writer, su_allocator_t *alloc, su_string_t key) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state == SU__JSON_WRITER_STATE_OBJECT) || (state == SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	su__json_buffer_add_char(&writer->buf, alloc, '"');
	su__json_buffer_add_string(&writer->buf, alloc, key);
	su__json_buffer_add_char(&writer->buf, alloc, '"');
	su__json_buffer_add_char(&writer->buf, alloc, ':');
	su_stack__su__json_writer_state_t__push(&writer->state, alloc, SU__JSON_WRITER_STATE_KEY);
}

static void su_json_writer_array_begin(su_json_writer_t *writer, su_allocator_t *alloc) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	su__json_buffer_add_char(&writer->buf, alloc, '[');
	su_stack__su__json_writer_state_t__push(&writer->state, alloc, SU__JSON_WRITER_STATE_ARRAY);
}

static void su_json_writer_array_end(su_json_writer_t *writer, su_allocator_t *alloc) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state == SU__JSON_WRITER_STATE_ARRAY) || (state == SU__JSON_WRITER_STATE_ARRAY_EXPECTING_COMMA));
	su__json_buffer_add_char(&writer->buf, alloc, ']');
	su_stack__su__json_writer_state_t__pop(&writer->state);
}

static void su_json_writer_null(su_json_writer_t *writer, su_allocator_t *alloc) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	su__json_buffer_add_string(&writer->buf, alloc, su_string("null"));
}

static void su_json_writer_bool(su_json_writer_t *writer, su_allocator_t *alloc, su_bool32_t b) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	if (b) {
		su__json_buffer_add_string(&writer->buf, alloc, su_string("true"));
	} else {
		su__json_buffer_add_string(&writer->buf, alloc, su_string("false"));
	}
}

static void su_json_writer_int(su_json_writer_t *writer, su_allocator_t *alloc, int64_t i) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	su__json_buffer_add_format(&writer->buf, alloc, "%ld", i);
}

static void su_json_writer_uint(su_json_writer_t *writer, su_allocator_t *alloc, uint64_t u) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	su__json_buffer_add_format(&writer->buf, alloc, "%lu", u);
}

static void su_json_writer_double(su_json_writer_t *writer, su_allocator_t *alloc, double d) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	/* TODO: isnan(d) || isinf(d) -> null */
	if ((d > 0) && (d < 0)) {
		su__json_buffer_add_format(&writer->buf, alloc, "%.17g", d);
	} else {
		su__json_buffer_add_format(&writer->buf, alloc, "%.1f", d);
	}
}

static void su_json_writer_string_escape(su_json_writer_t *writer, su_allocator_t *alloc, su_string_t str) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	su__json_buffer_add_char(&writer->buf, alloc, '"');
	su__json_buffer_add_string_escape(&writer->buf, alloc, str);
	su__json_buffer_add_char(&writer->buf, alloc, '"');
}

static void su_json_writer_string(su_json_writer_t *writer, su_allocator_t *alloc, su_string_t str) {
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	su__json_buffer_add_char(&writer->buf, alloc, '"');
	su__json_buffer_add_string(&writer->buf, alloc, str);
	su__json_buffer_add_char(&writer->buf, alloc, '"');
}

static void su_json_writer_raw(su_json_writer_t *writer, su_allocator_t *alloc, void *data, size_t len) {
	su_string_t s;
	su__json_writer_state_t state = su__json_writer_get_state(writer);
	SU_NOTUSED(state);
	SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	su__json_writer_element(writer, alloc);
	s.s = (SU_TYPEOF(s.s))data;
	s.len = len;
	s.free_contents = SU_FALSE;
	s.nul_terminated = SU_FALSE;
	su__json_buffer_add_string(&writer->buf, alloc, s);
}

static void su_json_writer_token(su_json_writer_t *writer, su_allocator_t *alloc, su_json_token_t token) {
	switch (token.type) {
	case SU_JSON_TOKEN_TYPE_OBJECT_START:
		su_json_writer_object_begin(writer, alloc);
		break;
	case SU_JSON_TOKEN_TYPE_OBJECT_END:
		su_json_writer_object_end(writer, alloc);
		break;
	case SU_JSON_TOKEN_TYPE_KEY:
		su_json_writer_object_key_escape(writer, alloc, token.value.s); /* TODO: escape only when necessary */
		break;
	case SU_JSON_TOKEN_TYPE_ARRAY_START:
		su_json_writer_array_begin(writer, alloc);
		break;
	case SU_JSON_TOKEN_TYPE_ARRAY_END:
		su_json_writer_array_end(writer, alloc);
		break;
	case SU_JSON_TOKEN_TYPE_STRING:
		su_json_writer_string_escape(writer, alloc, token.value.s); /* TODO: escape only when necessary */
		break;
	case SU_JSON_TOKEN_TYPE_BOOL:
		su_json_writer_bool(writer, alloc, token.value.b);
		break;
	case SU_JSON_TOKEN_TYPE_NULL:
		su_json_writer_null(writer, alloc);
		break;
	case SU_JSON_TOKEN_TYPE_DOUBLE:
		su_json_writer_double(writer, alloc, token.value.d);
		break;
	case SU_JSON_TOKEN_TYPE_INT:
		su_json_writer_int(writer, alloc, token.value.i);
		break;
	case SU_JSON_TOKEN_TYPE_UINT:
		su_json_writer_uint(writer, alloc, token.value.u);
		break;
	default:
		SU_ASSERT_UNREACHABLE;
	}
}

static void su_json_writer_ast_node(su_json_writer_t *writer, su_allocator_t *alloc, su_json_ast_node_t *node) {
	/* TODO: remove recursion */
	switch (node->type) {
	case SU_JSON_AST_NODE_TYPE_OBJECT: {
		size_t i = 0;
		su_json_writer_object_begin(writer, alloc);
		for ( ; i < node->value.object.len; ++i) {
			su_json_ast_key_value_t *key_value = su_array__su_json_ast_key_value_t__get_ptr(&node->value.object, i);
			su_json_writer_object_key_escape(writer, alloc, key_value->key); /* TODO: escape only when necessary */
			su_json_writer_ast_node(writer, alloc, &key_value->value);
		}
		su_json_writer_object_end(writer, alloc);
		break;
	}
	case SU_JSON_AST_NODE_TYPE_ARRAY: {
		size_t i = 0;
		su_json_writer_array_begin(writer, alloc);
		for ( ; i < node->value.array.len; ++i) {
			su_json_writer_ast_node(writer, alloc, su_array__su_json_ast_node_t__get_ptr(&node->value.array, i));
		}
		su_json_writer_array_end(writer, alloc);
		break;
	}
	case SU_JSON_AST_NODE_TYPE_STRING:
		su_json_writer_string_escape(writer, alloc, node->value.s); /* TODO: escape only when necessary */
		break;
	case SU_JSON_AST_NODE_TYPE_BOOL:
		su_json_writer_bool(writer, alloc, node->value.b);
		break;
	case SU_JSON_AST_NODE_TYPE_NULL:
		su_json_writer_null(writer, alloc);
		break;
	case SU_JSON_AST_NODE_TYPE_DOUBLE:
		su_json_writer_double(writer, alloc, node->value.d);
		break;
	case SU_JSON_AST_NODE_TYPE_INT:
		su_json_writer_int(writer, alloc, node->value.i);
		break;
	case SU_JSON_AST_NODE_TYPE_UINT:
		su_json_writer_uint(writer, alloc, node->value.u);
		break;
	case SU_JSON_AST_NODE_TYPE_NONE:
		break;
	default:
		SU_ASSERT_UNREACHABLE;
	}
}

static su_json_tokener_state_t su__json_tokener_buffer_to_string(su_json_tokener_t *tokener,
		su_allocator_t *alloc, su_string_t *out) {
	static uint8_t escape_lut[256] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,'\"',0,0,0,0,0,0,0,0,0,0,0,0,'/',
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,'\\',0,0,0,
		0,0,'\b',0,0,0,'\f',0,0,0,0,0,0,0,'\n',0,
		0,0,'\r',0,'\t',0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};

	size_t buf_idx = 0, str_idx = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
	__m256i backslash = _mm256_set1_epi8('\\');
	out->s = (SU_TYPEOF(out->s))alloc->alloc(alloc, tokener->buf.idx + 32, SU_ALIGNOF(*out->s));
	out->len = tokener->buf.idx;
	out->free_contents = SU_FALSE;
	out->nul_terminated = SU_TRUE; /* TODO: remove */
	for (;;) {
		__m256i v = _mm256_loadu_si256((__m256i_u *)&tokener->buf.data[buf_idx]);
		__m256i backslash_cmp = _mm256_cmpeq_epi8(v, backslash);
		uint32_t backslash_mask = (uint32_t)_mm256_movemask_epi8(backslash_cmp);
		size_t backslash_idx = (size_t)SU_COUNT_TRAILING_ZEROS(backslash_mask, 32);
		_mm256_storeu_si256((__m256i_u *)&out->s[str_idx], v);
		str_idx += backslash_idx;
		buf_idx += backslash_idx;
		if (SU_LIKELY(buf_idx >= tokener->buf.idx)) {
			break;
		} else if (SU_UNLIKELY(backslash_idx < 32)) {
			uint8_t escape_char = (uint8_t)tokener->buf.data[buf_idx + 1];
			if (SU_UNLIKELY(escape_char == 'u')) {
				su_string_t s;
				uint32_t c;
				uint8_t *dest;

				if (SU_UNLIKELY((buf_idx + 6) > tokener->buf.idx)) {
					return SU_JSON_TOKENER_STATE_ERROR;
				}

				s.s = &tokener->buf.data[buf_idx + 2];
				s.len = 4;
				s.free_contents = SU_FALSE;
				s.nul_terminated = SU_FALSE;
				c = su_string_hex_16_to_uint16(s);
				buf_idx += 6;

				if ((c >= 0xD800) && (c < 0xDC00)) { /* utf16 suggorage pair */
					uint32_t low;
					if (SU_UNLIKELY(((buf_idx + 6) > tokener->buf.idx) ||
							(tokener->buf.data[buf_idx] != '\\') ||
							(tokener->buf.data[buf_idx + 1] != 'u'))) {
						return SU_JSON_TOKENER_STATE_ERROR;
					}
					s.s = &tokener->buf.data[buf_idx + 2];
					low = su_string_hex_16_to_uint16(s) - 0xDC00;
					if (SU_UNLIKELY(low >> 10)) {
						return SU_JSON_TOKENER_STATE_ERROR;
					} else {
						c = (((c - 0xD800) << 10) | low) + 0x10000;
						buf_idx += 6;
					}
				} else if (SU_UNLIKELY((c >= 0xDC00) && (c <= 0xDFFF))) {
					return SU_JSON_TOKENER_STATE_ERROR;
				}

				if (SU_UNLIKELY(c > 0x10FFFF)) {
					return SU_JSON_TOKENER_STATE_ERROR;
				}

				dest = (uint8_t *)&out->s[str_idx];
				if (c <= 0x7F) {
					dest[0] = (uint8_t)c;
					str_idx += 1;
				} else if (c <= 0x7FF) {
					dest[0] = (uint8_t)((c >> 6) + 192);
					dest[1] = (uint8_t)((c & 63) + 128);
					str_idx += 2;
				} else if (c <= 0xFFFF) {
					dest[0] = (uint8_t)((c >> 12) + 224);
					dest[1] = (uint8_t)(((c >> 6) & 63) + 128);
					dest[2] = (uint8_t)((c & 63) + 128);
					str_idx += 3;
				} else if (c <= 0x10FFFF) {
					dest[0] = (uint8_t)((c >> 18) + 240);
					dest[1] = (uint8_t)(((c >> 12) & 63) + 128);
					dest[2] = (uint8_t)(((c >> 6) & 63) + 128);
					dest[3] = (uint8_t)((c & 63) + 128);
					str_idx += 4;
				}
			} else {
				if (SU_UNLIKELY((out->s[str_idx] = (char)escape_lut[escape_char]) == 0)) {
					return SU_JSON_TOKENER_STATE_ERROR;
				}

				str_idx += 1;
				buf_idx += 2;
			}
		}
	}
/* TODO: ARM SIMD */
#else
	out->s = (SU_TYPEOF(out->s))alloc->alloc(alloc,
		tokener->buf.idx + 32, SU_ALIGNOF(*out->s));
	out->len = tokener->buf.idx;
	out->free_contents = SU_FALSE;
	out->nul_terminated = SU_TRUE; /* TODO: remove */
	memcpy(out->s, tokener->buf.data, tokener->buf.idx);
	for (;;) {
		char *backslash = (char *)memchr(&tokener->buf.data[buf_idx], '\\', tokener->buf.idx - buf_idx);
		if (SU_LIKELY(backslash == NULL)) {
			break;
		} else {
			size_t backslash_idx = (size_t)(backslash - &tokener->buf.data[buf_idx]);
			uint8_t escape_char;
			buf_idx += backslash_idx;
			str_idx += backslash_idx;
			escape_char = (uint8_t)tokener->buf.data[buf_idx + 1];
			if (SU_UNLIKELY(escape_char == 'u')) {
				su_string_t s;
				uint32_t c;
				uint8_t *dest;
				if (SU_UNLIKELY((buf_idx + 6) > tokener->buf.idx)) {
					return SU_JSON_TOKENER_STATE_ERROR;
				}

				s.s = &tokener->buf.data[buf_idx + 2];
				s.len = 4;
				s.free_contents = SU_FALSE;
				s.nul_terminated = SU_FALSE;
				c = su_string_hex_16_to_uint16(s);
				buf_idx += 6;

				if ((c >= 0xD800) && (c < 0xDC00)) { /* utf16 suggorage pair */
					uint32_t low;
					if (SU_UNLIKELY(((buf_idx + 6) > tokener->buf.idx) ||
							(tokener->buf.data[buf_idx] != '\\') ||
							(tokener->buf.data[buf_idx + 1] != 'u'))) {
						return SU_JSON_TOKENER_STATE_ERROR;
					}
					s.s = &tokener->buf.data[buf_idx + 2];
					low = su_string_hex_16_to_uint16(s) - 0xDC00;
					if (SU_UNLIKELY(low >> 10)) {
						return SU_JSON_TOKENER_STATE_ERROR;
					} else {
						c = (((c - 0xD800) << 10) | low) + 0x10000;
						buf_idx += 6;
					}
				} else if (SU_UNLIKELY((c >= 0xDC00) && (c <= 0xDFFF))) {
					return SU_JSON_TOKENER_STATE_ERROR;
				}

				if (SU_UNLIKELY(c > 0x10FFFF)) {
					return SU_JSON_TOKENER_STATE_ERROR;
				}

				dest = (uint8_t *)&out->s[str_idx];
				if (c <= 0x7F) {
					dest[0] = (uint8_t)c;
					str_idx += 1;
				} else if (c <= 0x7FF) {
					dest[0] = (uint8_t)((c >> 6) + 192);
					dest[1] = (uint8_t)((c & 63) + 128);
					str_idx += 2;
				} else if (c <= 0xFFFF) {
					dest[0] = (uint8_t)((c >> 12) + 224);
					dest[1] = (uint8_t)(((c >> 6) & 63) + 128);
					dest[2] = (uint8_t)((c & 63) + 128);
					str_idx += 3;
				} else if (c <= 0x10FFFF) {
					dest[0] = (uint8_t)((c >> 18) + 240);
					dest[1] = (uint8_t)(((c >> 12) & 63) + 128);
					dest[2] = (uint8_t)(((c >> 6) & 63) + 128);
					dest[3] = (uint8_t)((c & 63) + 128);
					str_idx += 4;
				}
			} else {
				if (SU_UNLIKELY((out->s[str_idx] = (char)escape_lut[escape_char]) == 0)) {
					return SU_JSON_TOKENER_STATE_ERROR;
				}

				str_idx += 1;
				buf_idx += 2;
			}
			memmove(&out->s[str_idx], &tokener->buf.data[buf_idx], tokener->buf.idx - buf_idx);
		}
	}
#endif

	out->len -= (buf_idx - str_idx);
	out->s[out->len] = '\0'; /* TODO: remove */
	tokener->buf.idx = 0;
	return SU_JSON_TOKENER_STATE_SUCCESS;
}

static void su_json_tokener_set_string(su_json_tokener_t *tokener, su_allocator_t *alloc, su_string_t str) {
	if (str.len >= (tokener->buf.size - tokener->buf.idx)) {
		tokener->buf.size = (str.len + tokener->buf.size) * 2;
		tokener->buf.data = (SU_TYPEOF(tokener->buf.data))alloc->realloc(
			alloc, tokener->buf.data, tokener->buf.size, SU_ALIGNOF(*tokener->buf.data));
	}

	su_stack__su__json_tokener_state_t__resize(&tokener->state, alloc, tokener->state.data.size + str.len);

	tokener->pos = 0;
	tokener->str = str;
}

static su__json_tokener_state_t su__json_tokener_get_state(su_json_tokener_t *tokener) {
	if (tokener->state.data.len > 0) {
		return su_stack__su__json_tokener_state_t__get(&tokener->state);
	}

	return SU__JSON_TOKENER_STATE_ROOT;
}

static void su__json_tokener_value_end(su_json_tokener_t *tokener) {
	su_stack__su__json_tokener_state_t__pop(&tokener->state);
	switch (su__json_tokener_get_state(tokener)) {
	case SU__JSON_TOKENER_STATE_VALUE:
		su_stack__su__json_tokener_state_t__pop(&tokener->state);
		break;
	case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
	case SU__JSON_TOKENER_STATE_ARRAY:
		su_stack__su__json_tokener_state_t__pop(&tokener->state);
		su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
			SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA);
		break;
	case SU__JSON_TOKENER_STATE_ROOT:
		break;
	case SU__JSON_TOKENER_STATE_NULL:
	case SU__JSON_TOKENER_STATE_TRUE:
	case SU__JSON_TOKENER_STATE_FALSE:
	case SU__JSON_TOKENER_STATE_OBJECT_EMPTY:
	case SU__JSON_TOKENER_STATE_OBJECT:
	case SU__JSON_TOKENER_STATE_STRING:
	case SU__JSON_TOKENER_STATE_NUMBER:
	case SU__JSON_TOKENER_STATE_KEY:
	case SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
	case SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
	default:
		SU_ASSERT_UNREACHABLE;
	}

	tokener->buf.idx = 0;
}

static su_json_tokener_state_t su_json_tokener_next(su_json_tokener_t *tokener,
		su_allocator_t *alloc, su_json_token_t *out) {
	su__json_tokener_state_t state = su__json_tokener_get_state(tokener);
	switch (state) {
	case SU__JSON_TOKENER_STATE_STRING:
_string: {
		if (SU_UNLIKELY(tokener->pos == tokener->str.len)) {
			return SU_JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
		}
		if (SU_LIKELY(tokener->buf.idx == 0)) {
			char c = tokener->str.s[tokener->pos++];
			if (SU_UNLIKELY(c == '\"')) {
				goto _string_end;
			} else if (SU_UNLIKELY(c == '\\')) {
				tokener->last_escape_idx = 0;
				su__json_buffer_add_char_nocheck(&tokener->buf, '\\');
			} else {
				tokener->last_escape_idx = SIZE_MAX;
				su__json_buffer_add_char_nocheck(&tokener->buf, c);
			}
		}

#if SU_WITH_SIMD && defined(__AVX2__)
		if (SU_LIKELY((tokener->pos + 32) <= tokener->str.len)) {
			__m256i quote = _mm256_set1_epi8('"');
			__m256i backslash = _mm256_set1_epi8('\\');
			do {
				__m256i v = _mm256_loadu_si256((__m256i_u *)&tokener->str.s[tokener->pos]);
				__m256i quote_cmp = _mm256_cmpeq_epi8(v, quote);
				__m256i backslash_cmp = _mm256_cmpeq_epi8(v, backslash);
				uint32_t quote_mask = (uint32_t)_mm256_movemask_epi8(quote_cmp);
				uint32_t backslash_mask = (uint32_t)_mm256_movemask_epi8(backslash_cmp);
				_mm256_storeu_si256((__m256i_u *)&tokener->buf.data[tokener->buf.idx], v);
				if (SU_LIKELY((backslash_mask - 1) & quote_mask)) {
					size_t quote_idx = (size_t)SU_COUNT_TRAILING_ZEROS(quote_mask, 0);
					tokener->buf.idx += quote_idx;
					tokener->pos += quote_idx + 1;
					if (SU_LIKELY(tokener->last_escape_idx != (tokener->buf.idx - 1))) {
						goto _string_end;
					}
					tokener->buf.idx += 1;
				} else if (SU_UNLIKELY((quote_mask - 1) & backslash_mask)) {
					size_t backslash_idx = (size_t)SU_COUNT_TRAILING_ZEROS(backslash_mask, 0);
					tokener->buf.idx += backslash_idx;
					tokener->pos += backslash_idx + 1;
					if (SU_LIKELY(tokener->last_escape_idx != (tokener->buf.idx - 1))) {
						tokener->last_escape_idx = tokener->buf.idx;
					}
					tokener->buf.idx += 1;
				} else {
					tokener->buf.idx += 32;
					tokener->pos += 32;
				}
			} while (SU_LIKELY((tokener->pos + 32) <= tokener->str.len));
		}
#endif /* SU_WITH_SIMD && __AVX2__ */
/* TODO: ARM SIMD */

		for (;;) {
			char c;
			if (SU_UNLIKELY(tokener->pos == tokener->str.len)) {
				return SU_JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
			}

			c = tokener->str.s[tokener->pos++];
			if (SU_UNLIKELY((c == '\"') && (tokener->last_escape_idx != (tokener->buf.idx - 1)))) {
				goto _string_end;
			} else if (SU_UNLIKELY((c == '\\') && (tokener->last_escape_idx != (tokener->buf.idx - 1)))) {
				tokener->last_escape_idx = tokener->buf.idx;
			}

			su__json_buffer_add_char_nocheck(&tokener->buf, c);
		}

_string_end:
		su_stack__su__json_tokener_state_t__pop(&tokener->state);
		switch (su__json_tokener_get_state(tokener)) {
		case SU__JSON_TOKENER_STATE_OBJECT_EMPTY:
		case SU__JSON_TOKENER_STATE_OBJECT:
			su_stack__su__json_tokener_state_t__pop(&tokener->state);
			su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
				SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA);
			su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
				SU__JSON_TOKENER_STATE_KEY);
			out->type = SU_JSON_TOKEN_TYPE_KEY;
			break;
		case SU__JSON_TOKENER_STATE_VALUE:
			su_stack__su__json_tokener_state_t__pop(&tokener->state);
			out->type = SU_JSON_TOKEN_TYPE_STRING;
			break;
		case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
		case SU__JSON_TOKENER_STATE_ARRAY:
			su_stack__su__json_tokener_state_t__pop(&tokener->state);
			su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
				SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA);
			out->type = SU_JSON_TOKEN_TYPE_STRING;
			break;
		case SU__JSON_TOKENER_STATE_ROOT:
			break;
		case SU__JSON_TOKENER_STATE_NULL:
		case SU__JSON_TOKENER_STATE_TRUE:
		case SU__JSON_TOKENER_STATE_FALSE:
		case SU__JSON_TOKENER_STATE_STRING:
		case SU__JSON_TOKENER_STATE_NUMBER:
		case SU__JSON_TOKENER_STATE_KEY:
		case SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
		case SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
		default:
			SU_ASSERT_UNREACHABLE;
		}

		out->depth = tokener->depth;
		return su__json_tokener_buffer_to_string(tokener, alloc, &out->value.s);
	}
	case SU__JSON_TOKENER_STATE_NUMBER:
_number: {
		su_string_t str;

#if SU_WITH_SIMD && defined(__AVX2__)
		if (SU_LIKELY((tokener->pos + 32) <= tokener->str.len)) {
			__m256i v = _mm256_loadu_si256((__m256i_u *)&tokener->str.s[tokener->pos]);
			/* TODO: lut */
			uint32_t mask = (uint32_t)_mm256_movemask_epi8(
				_mm256_or_si256(_mm256_or_si256(_mm256_or_si256(_mm256_or_si256(_mm256_or_si256(_mm256_or_si256(
					_mm256_cmpeq_epi8(v, _mm256_set1_epi8('\t')),
					_mm256_cmpeq_epi8(v, _mm256_set1_epi8('\r'))),
					_mm256_cmpeq_epi8(v, _mm256_set1_epi8('\n'))),
					_mm256_cmpeq_epi8(v, _mm256_set1_epi8(' '))),
					_mm256_cmpeq_epi8(v, _mm256_set1_epi8(','))),
					_mm256_cmpeq_epi8(v, _mm256_set1_epi8('}'))),
					_mm256_cmpeq_epi8(v, _mm256_set1_epi8(']'))
				)
			);
			_mm256_storeu_si256((__m256i_u *)&tokener->buf.data[tokener->buf.idx], v);
			if (SU_LIKELY(mask)) {
				size_t idx = (size_t)SU_COUNT_TRAILING_ZEROS(mask, 0);
				str.s = tokener->buf.data;
				str.len = idx;
				tokener->pos += idx;
				goto _parse_number;
			} else {
				return SU_JSON_TOKENER_STATE_ERROR;
			}
		}
#endif /* SU_WITH_SIMD && __AVX2__ */
/* TODO: ARM SIMD */

		for (;;) {
			char c;
			if (SU_UNLIKELY(tokener->pos == tokener->str.len)) {
				return SU_JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
			}
			c = tokener->str.s[tokener->pos];
			switch (c) {
			case '\t':
			case '\r':
			case '\n':
			case ' ':
			case ',':
			case ']':
			case '}':
				str.s = tokener->buf.data;
				str.len = tokener->buf.idx;
				goto _parse_number;
			default:
				su__json_buffer_add_char_nocheck(&tokener->buf, c);
				tokener->pos++;
			}
		}

_parse_number:
		if (su_string_to_uint64(str, &out->value.u)) {
			out->type = SU_JSON_TOKEN_TYPE_UINT;
		} else if (su_string_to_int64(str, &out->value.i)) {
			out->type = SU_JSON_TOKEN_TYPE_INT;
		} else {
			/* TODO: string to double conversion function with length */
			char *end = NULL;
			su__json_buffer_add_char_nocheck(&tokener->buf, '\0');
			errno = 0;
			out->value.d = strtod(str.s, &end);
			if (SU_LIKELY((end == &tokener->buf.data[tokener->buf.idx - 1]) && (errno == 0))) {
				out->type = SU_JSON_TOKEN_TYPE_DOUBLE;
			} else {
				return SU_JSON_TOKENER_STATE_ERROR;
			}
		}
		out->depth = tokener->depth;

		su__json_tokener_value_end(tokener);

		return SU_JSON_TOKENER_STATE_SUCCESS;
	}
	case SU__JSON_TOKENER_STATE_NULL:
_null: {
		size_t bytes_available = (tokener->str.len - tokener->pos);
		size_t bytes_needed = (4 - tokener->buf.idx);
		su_string_t s;
		s.nul_terminated = SU_FALSE;
		s.free_contents = SU_FALSE;
		if (SU_LIKELY(bytes_available >= bytes_needed)) {
			memcpy(&tokener->buf.data[tokener->buf.idx], &tokener->str.s[tokener->pos], bytes_needed);
			tokener->pos += bytes_needed;
		} else {
			s.s = &tokener->str.s[tokener->pos];
			s.len = bytes_available;
			su__json_buffer_add_string_nocheck(&tokener->buf, s);
			/*tokener->pos += bytes_available;*/
			return SU_JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
		}

		s.s = tokener->buf.data;
		s.len = 4;
		if (SU_UNLIKELY(!su_string_equal(su_string("null"), s))) {
			return SU_JSON_TOKENER_STATE_ERROR;
		}

		su__json_tokener_value_end(tokener);

		out->type = SU_JSON_TOKEN_TYPE_NULL;
		out->depth = tokener->depth;
		return SU_JSON_TOKENER_STATE_SUCCESS;
	}
	case SU__JSON_TOKENER_STATE_TRUE:
_true: {
		size_t bytes_available = (tokener->str.len - tokener->pos);
		size_t bytes_needed = (4 - tokener->buf.idx);
		su_string_t s;
		s.nul_terminated = SU_FALSE;
		s.free_contents = SU_FALSE;
		if (SU_LIKELY(bytes_available >= bytes_needed)) {
			memcpy(&tokener->buf.data[tokener->buf.idx], &tokener->str.s[tokener->pos], bytes_needed);
			tokener->pos += bytes_needed;
		} else {
			s.s = &tokener->str.s[tokener->pos];
			s.len = bytes_available;
			su__json_buffer_add_string_nocheck(&tokener->buf, s);
			/*tokener->pos += bytes_available;*/
			return SU_JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
		}

		s.s = tokener->buf.data;
		s.len = 4;
		if (SU_UNLIKELY(!su_string_equal(su_string("true"), s))) {
			return SU_JSON_TOKENER_STATE_ERROR;
		}

		su__json_tokener_value_end(tokener);

		out->type = SU_JSON_TOKEN_TYPE_BOOL;
		out->value.b = SU_TRUE;
		out->depth = tokener->depth;
		return SU_JSON_TOKENER_STATE_SUCCESS;
	}
	case SU__JSON_TOKENER_STATE_FALSE:
_false: {
		size_t bytes_available = (tokener->str.len - tokener->pos);
		size_t bytes_needed = (5 - tokener->buf.idx);
		su_string_t s;
		s.nul_terminated = SU_FALSE;
		s.free_contents = SU_FALSE;
		if (SU_LIKELY(bytes_available >= bytes_needed)) {
			memcpy(&tokener->buf.data[tokener->buf.idx], &tokener->str.s[tokener->pos], bytes_needed);
			tokener->pos += bytes_needed;
		} else {
			s.s = &tokener->str.s[tokener->pos];
			s.len = bytes_available;
			su__json_buffer_add_string_nocheck(&tokener->buf, s);
			/*tokener->pos += bytes_available;*/
			return SU_JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
		}

		s.s = tokener->buf.data;
		s.len = 5;
		if (SU_UNLIKELY(!su_string_equal(su_string("false"), s))) {
			return SU_JSON_TOKENER_STATE_ERROR;
		}

		su__json_tokener_value_end(tokener);

		out->type = SU_JSON_TOKEN_TYPE_BOOL;
		out->value.b = SU_FALSE;
		out->depth = tokener->depth;
		return SU_JSON_TOKENER_STATE_SUCCESS;
	}
	case SU__JSON_TOKENER_STATE_ROOT:
	case SU__JSON_TOKENER_STATE_OBJECT:
	case SU__JSON_TOKENER_STATE_OBJECT_EMPTY:
	case SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
	case SU__JSON_TOKENER_STATE_KEY:
	case SU__JSON_TOKENER_STATE_VALUE:
	case SU__JSON_TOKENER_STATE_ARRAY:
	case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
	case SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
		break;
	default:
		SU_ASSERT_UNREACHABLE;
	}

	for ( ; tokener->pos < tokener->str.len; ++tokener->pos) {
		char c = tokener->str.s[tokener->pos];
		switch (c) {
		case '{':
			switch (state) {
			case SU__JSON_TOKENER_STATE_OBJECT:
			case SU__JSON_TOKENER_STATE_OBJECT_EMPTY:
			case SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case SU__JSON_TOKENER_STATE_KEY:
			case SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return SU_JSON_TOKENER_STATE_ERROR;
			case SU__JSON_TOKENER_STATE_VALUE:
				su_stack__su__json_tokener_state_t__pop(&tokener->state);
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_OBJECT_EMPTY);
				break;
			case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
			case SU__JSON_TOKENER_STATE_ARRAY:
				su_stack__su__json_tokener_state_t__pop(&tokener->state);
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA);
				SU_ATTRIBUTE_FALLTHROUGH;
			case SU__JSON_TOKENER_STATE_ROOT:
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_OBJECT_EMPTY);
				break;
			case SU__JSON_TOKENER_STATE_NULL:
			case SU__JSON_TOKENER_STATE_TRUE:
			case SU__JSON_TOKENER_STATE_FALSE:
			case SU__JSON_TOKENER_STATE_STRING:
			case SU__JSON_TOKENER_STATE_NUMBER:
			default:
				SU_ASSERT_UNREACHABLE;
			}
			tokener->pos++;
			tokener->depth++;
			out->depth = tokener->depth;
			out->type = SU_JSON_TOKEN_TYPE_OBJECT_START;
			return SU_JSON_TOKENER_STATE_SUCCESS;
		case '[':
			switch (state) {
			case SU__JSON_TOKENER_STATE_OBJECT:
			case SU__JSON_TOKENER_STATE_OBJECT_EMPTY:
			case SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case SU__JSON_TOKENER_STATE_KEY:
			case SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return SU_JSON_TOKENER_STATE_ERROR;
			case SU__JSON_TOKENER_STATE_VALUE:
				su_stack__su__json_tokener_state_t__pop(&tokener->state);
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_ARRAY_EMPTY);
				break;
			case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
			case SU__JSON_TOKENER_STATE_ARRAY:
				su_stack__su__json_tokener_state_t__pop(&tokener->state);
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA);
				SU_ATTRIBUTE_FALLTHROUGH;
			case SU__JSON_TOKENER_STATE_ROOT:
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_ARRAY_EMPTY);
				break;
			case SU__JSON_TOKENER_STATE_NULL:
			case SU__JSON_TOKENER_STATE_TRUE:
			case SU__JSON_TOKENER_STATE_FALSE:
			case SU__JSON_TOKENER_STATE_STRING:
			case SU__JSON_TOKENER_STATE_NUMBER:
			default:
				SU_ASSERT_UNREACHABLE;
			}
			tokener->pos++;
			tokener->depth++;
			out->depth = tokener->depth;
			out->type = SU_JSON_TOKEN_TYPE_ARRAY_START;
			return SU_JSON_TOKENER_STATE_SUCCESS;
		case '}':
			if (SU_UNLIKELY((state != SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA) && (state != SU__JSON_TOKENER_STATE_OBJECT_EMPTY))) {
				return SU_JSON_TOKENER_STATE_ERROR;
			}
			su_stack__su__json_tokener_state_t__pop(&tokener->state);
			tokener->pos++;
			tokener->depth--;
			out->depth = tokener->depth;
			out->type = SU_JSON_TOKEN_TYPE_OBJECT_END;
			return SU_JSON_TOKENER_STATE_SUCCESS;
		case ']':
			if (SU_UNLIKELY((state != SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA) && (state != SU__JSON_TOKENER_STATE_ARRAY_EMPTY))) {
				return SU_JSON_TOKENER_STATE_ERROR;
			}
			su_stack__su__json_tokener_state_t__pop(&tokener->state);
			tokener->pos++;
			tokener->depth--;
			out->depth = tokener->depth;
			out->type = SU_JSON_TOKEN_TYPE_ARRAY_END;
			return SU_JSON_TOKENER_STATE_SUCCESS;
		case '\"': {
			if (SU_UNLIKELY((state == SU__JSON_TOKENER_STATE_KEY)
					|| (state == SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA)
					|| (state == SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA))) {
				return SU_JSON_TOKENER_STATE_ERROR;
			}
			su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
				SU__JSON_TOKENER_STATE_STRING);
			tokener->pos++;
			goto _string;
		}
		case ':':
			if (SU_UNLIKELY(state != SU__JSON_TOKENER_STATE_KEY)) {
				return SU_JSON_TOKENER_STATE_ERROR;
			}
			su_stack__su__json_tokener_state_t__pop(&tokener->state);
			su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
				SU__JSON_TOKENER_STATE_VALUE);
			state = SU__JSON_TOKENER_STATE_VALUE;
			continue;
		case ',':
			switch (state) {
			case SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
				su_stack__su__json_tokener_state_t__pop(&tokener->state);
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_OBJECT);
				state = SU__JSON_TOKENER_STATE_OBJECT;
				continue;
			case SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				su_stack__su__json_tokener_state_t__pop(&tokener->state);
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_ARRAY);
				state = SU__JSON_TOKENER_STATE_ARRAY;
				continue;
			case SU__JSON_TOKENER_STATE_ROOT:
			case SU__JSON_TOKENER_STATE_OBJECT:
			case SU__JSON_TOKENER_STATE_OBJECT_EMPTY:
			case SU__JSON_TOKENER_STATE_KEY:
			case SU__JSON_TOKENER_STATE_VALUE:
			case SU__JSON_TOKENER_STATE_ARRAY:
			case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
				return SU_JSON_TOKENER_STATE_ERROR;
			case SU__JSON_TOKENER_STATE_STRING:
			case SU__JSON_TOKENER_STATE_NUMBER:
			case SU__JSON_TOKENER_STATE_NULL:
			case SU__JSON_TOKENER_STATE_TRUE:
			case SU__JSON_TOKENER_STATE_FALSE:
			default:
				SU_ASSERT_UNREACHABLE;
			}
		case 'n':
			switch (state) {
			case SU__JSON_TOKENER_STATE_ROOT:
			case SU__JSON_TOKENER_STATE_VALUE:
			case SU__JSON_TOKENER_STATE_ARRAY:
			case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_NULL);
				goto _null;
			case SU__JSON_TOKENER_STATE_OBJECT:
			case SU__JSON_TOKENER_STATE_OBJECT_EMPTY:
			case SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case SU__JSON_TOKENER_STATE_KEY:
			case SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return SU_JSON_TOKENER_STATE_ERROR;
			case SU__JSON_TOKENER_STATE_STRING:
			case SU__JSON_TOKENER_STATE_NUMBER:
			case SU__JSON_TOKENER_STATE_NULL:
			case SU__JSON_TOKENER_STATE_TRUE:
			case SU__JSON_TOKENER_STATE_FALSE:
			default:
				SU_ASSERT_UNREACHABLE;
			}
		case 't':
			switch (state) {
			case SU__JSON_TOKENER_STATE_ROOT:
			case SU__JSON_TOKENER_STATE_VALUE:
			case SU__JSON_TOKENER_STATE_ARRAY:
			case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_TRUE);
				goto _true;
			case SU__JSON_TOKENER_STATE_OBJECT:
			case SU__JSON_TOKENER_STATE_OBJECT_EMPTY:
			case SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case SU__JSON_TOKENER_STATE_KEY:
			case SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return SU_JSON_TOKENER_STATE_ERROR;
			case SU__JSON_TOKENER_STATE_STRING:
			case SU__JSON_TOKENER_STATE_NUMBER:
			case SU__JSON_TOKENER_STATE_NULL:
			case SU__JSON_TOKENER_STATE_TRUE:
			case SU__JSON_TOKENER_STATE_FALSE:
			default:
				SU_ASSERT_UNREACHABLE;
			}
		case 'f':
			switch (state) {
			case SU__JSON_TOKENER_STATE_ROOT:
			case SU__JSON_TOKENER_STATE_VALUE:
			case SU__JSON_TOKENER_STATE_ARRAY:
			case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_FALSE);
				goto _false;
			case SU__JSON_TOKENER_STATE_OBJECT:
			case SU__JSON_TOKENER_STATE_OBJECT_EMPTY:
			case SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case SU__JSON_TOKENER_STATE_KEY:
			case SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return SU_JSON_TOKENER_STATE_ERROR;
			case SU__JSON_TOKENER_STATE_STRING:
			case SU__JSON_TOKENER_STATE_NUMBER:
			case SU__JSON_TOKENER_STATE_NULL:
			case SU__JSON_TOKENER_STATE_TRUE:
			case SU__JSON_TOKENER_STATE_FALSE:
			default:
				SU_ASSERT_UNREACHABLE;
			}
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			switch (state) {
			case SU__JSON_TOKENER_STATE_ROOT:
			case SU__JSON_TOKENER_STATE_VALUE:
			case SU__JSON_TOKENER_STATE_ARRAY:
			case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
				su_stack__su__json_tokener_state_t__push_nocheck(&tokener->state,
					SU__JSON_TOKENER_STATE_NUMBER);
				goto _number;
			case SU__JSON_TOKENER_STATE_OBJECT:
			case SU__JSON_TOKENER_STATE_OBJECT_EMPTY:
			case SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case SU__JSON_TOKENER_STATE_KEY:
			case SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return SU_JSON_TOKENER_STATE_ERROR;
			case SU__JSON_TOKENER_STATE_STRING:
			case SU__JSON_TOKENER_STATE_NUMBER:
			case SU__JSON_TOKENER_STATE_NULL:
			case SU__JSON_TOKENER_STATE_TRUE:
			case SU__JSON_TOKENER_STATE_FALSE:
			default:
				SU_ASSERT_UNREACHABLE;
			}
		case '\t':
		case '\r':
		case '\n':
		case ' ':
			continue;
		default:
			return SU_JSON_TOKENER_STATE_ERROR;
		}
	}

	return ((su__json_tokener_get_state(tokener) == SU__JSON_TOKENER_STATE_ROOT)
		? SU_JSON_TOKENER_STATE_EOF
		: SU_JSON_TOKENER_STATE_MORE_DATA_EXPECTED);
}

static void su_json_ast_reset(su_json_ast_t *ast) {
	memset(&ast->root, 0, sizeof(ast->root));
	ast->current = &ast->root;
}

static su_json_tokener_state_t su_json_tokener_ast(su_json_tokener_t *tokener, su_allocator_t *alloc,
		su_json_ast_t *ast, uint32_t stop_depth, su_bool32_t check_for_repeating_keys) {
	/* ? TODO: min/max depth */
	su_json_tokener_state_t state;
	su_json_token_t token;
	su_json_ast_node_t node;
	while ((state = su_json_tokener_next(tokener, alloc, &token)) == SU_JSON_TOKENER_STATE_SUCCESS) {
		su_json_ast_node_t *current;
		if (SU_UNLIKELY(token.depth == stop_depth)) {
			return SU_JSON_TOKENER_STATE_SUCCESS;
		}
		switch (token.type) {
		case SU_JSON_TOKEN_TYPE_OBJECT_START:
			node.type = SU_JSON_AST_NODE_TYPE_OBJECT;
			memset(&node.value.object, 0, sizeof(node.value.object));
			break;
		case SU_JSON_TOKEN_TYPE_ARRAY_START:
			node.type = SU_JSON_AST_NODE_TYPE_ARRAY;
			memset(&node.value.array, 0, sizeof(node.value.array));
			break;
		case SU_JSON_TOKEN_TYPE_OBJECT_END:
		case SU_JSON_TOKEN_TYPE_ARRAY_END:
			SU_ASSERT((ast->current->type == SU_JSON_AST_NODE_TYPE_OBJECT)
				|| (ast->current->type == SU_JSON_AST_NODE_TYPE_ARRAY));
			ast->current = ast->current->parent;
			continue;
		case SU_JSON_TOKEN_TYPE_KEY: {
			su_json_ast_key_value_t kv;
			SU_ASSERT(ast->current->type == SU_JSON_AST_NODE_TYPE_OBJECT);
			if (check_for_repeating_keys) {
				size_t i = 0;
				for ( ; i < ast->current->value.object.len; ++i) {
					su_json_ast_key_value_t *key_value = su_array__su_json_ast_key_value_t__get_ptr(
						&ast->current->value.object, i);
					if (SU_UNLIKELY(su_string_equal(token.value.s, key_value->key))) {
						return SU_JSON_TOKENER_STATE_ERROR;
					}
				}
			}
			kv.key = token.value.s;
			su_array__su_json_ast_key_value_t__add(&ast->current->value.object, alloc, kv);
			continue;
		}
		case SU_JSON_TOKEN_TYPE_STRING:
			node.type = SU_JSON_AST_NODE_TYPE_STRING;
			node.value.s = token.value.s;
			break;
		case SU_JSON_TOKEN_TYPE_BOOL:
			node.type = SU_JSON_AST_NODE_TYPE_BOOL;
			node.value.b = token.value.b;
			break;
		case SU_JSON_TOKEN_TYPE_NULL:
			node.type = SU_JSON_AST_NODE_TYPE_NULL;
			break;
		case SU_JSON_TOKEN_TYPE_DOUBLE:
			node.type = SU_JSON_AST_NODE_TYPE_DOUBLE;
			node.value.d = token.value.d;
			break;
		case SU_JSON_TOKEN_TYPE_INT:
			node.type = SU_JSON_AST_NODE_TYPE_INT;
			node.value.i = token.value.i;
			break;
		case SU_JSON_TOKEN_TYPE_UINT:
			node.type = SU_JSON_AST_NODE_TYPE_UINT;
			node.value.u = token.value.u;
			break;
		default:
			SU_ASSERT_UNREACHABLE;
		}
		node.parent = ast->current;

		if (ast->current->type == SU_JSON_AST_NODE_TYPE_OBJECT) {
			current = &su_array__su_json_ast_key_value_t__get_ptr( &ast->current->value.object,
				ast->current->value.object.len - 1)->value;
			*current = node;
		} else if (ast->current->type == SU_JSON_AST_NODE_TYPE_ARRAY) {
			current = su_array__su_json_ast_node_t__add(&ast->current->value.array, alloc, node);
		} else {
			ast->root = node;
			current = &ast->root;
		}

		if ((token.type == SU_JSON_TOKEN_TYPE_OBJECT_START) || (token.type == SU_JSON_TOKEN_TYPE_ARRAY_START)) {
			ast->current = current;
		}
	}
	SU_ASSERT(state != SU_JSON_TOKENER_STATE_SUCCESS);
	return state;
}

static su_json_ast_node_t *su_json_ast_node_object_get(su_json_ast_node_t *node, su_string_t key) {
	size_t i = 0;
	
	SU_ASSERT(node->type == SU_JSON_AST_NODE_TYPE_OBJECT);

	for ( ; i < node->value.object.len; ++i) {
		su_json_ast_key_value_t *key_value = su_array__su_json_ast_key_value_t__get_ptr(
			&node->value.object, i);
		if (su_string_equal(key_value->key, key)) {
			return &key_value->value;
		}
	}

	return NULL;
}

static SU_ATTRIBUTE_ALWAYS_INLINE void su_json_tokener_advance_assert(su_json_tokener_t *tokener,
		su_allocator_t *alloc, su_json_token_t *token_out) {
	su_json_tokener_state_t s = su_json_tokener_next(tokener, alloc, token_out);
	SU_NOTUSED(s);
	SU_ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
}

static SU_ATTRIBUTE_ALWAYS_INLINE void su_json_tokener_advance_assert_type(su_json_tokener_t *tokener, su_allocator_t *alloc,
		su_json_token_t *token_out, su_json_token_type_t expected_type) {
	SU_NOTUSED(expected_type);
	su_json_tokener_advance_assert(tokener, alloc, token_out);
	SU_ASSERT(token_out->type == expected_type);
}

#endif /* defined(SU_IMPLEMENTATION) && !defined(SU__REIMPLEMENTATION_GUARD) */
