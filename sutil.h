#if !defined(SU_HEADER)
#define SU_HEADER

#if !defined(_GNU_SOURCE) && !defined(_DEFAULT_SOURCE) && !(defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 700))
    #error "_XOPEN_SOURCE >= 700 or _GNU_SOURCE or _DEFAULT_SOURCE must be defined"
#endif

#define SU_IMPLEMENTATION

#if !defined(SU_WITH_SIMD)
    #define SU_WITH_SIMD 1
#endif /* !defined(SU_WITH_SIMD) */
#if !defined(SU_WITH_DEBUG)
    #define SU_WITH_DEBUG 1
#endif /* !defined(SU_WITH_DEBUG) */

#include <time.h>

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define SU__CONCAT(a, b) a##b
#define SU_CONCAT(a, b) SU__CONCAT(a, b)

#define SU_PRAGMA(x) _Pragma(#x)

#if defined(__GNUC__)
#define SU_IGNORE_WARNING(w) SU_PRAGMA(GCC diagnostic ignored w)
#define SU_IGNORE_WARNINGS_START \
    SU_PRAGMA(GCC diagnostic push) \
    SU_IGNORE_WARNING("-Wpragmas") \
    SU_IGNORE_WARNING("-Wunknown-warning-option")
#define SU_IGNORE_WARNINGS_END SU_PRAGMA(GCC diagnostic pop)

#define inline __inline
#define restrict __restrict

#define SU_ALIGNOF __alignof
#define SU_TYPEOF __typeof
#define SU_THREAD_LOCAL __thread

#if defined(_Static_assert)
    #undef _Static_assert
#endif
/* ? TODO: with message */
#define SU_STATIC_ASSERT(x) __extension__ _Static_assert(x, "")

#if !defined(SU_MEMCPY)
    #define SU_MEMCPY __builtin_memcpy
#endif /* SU_MEMCPY */
#if !defined(SU_MEMMOVE)
    #define SU_MEMMOVE __builtin_memmove
#endif /* SU_MEMMOVE */
#if !defined(SU_MEMSET)
    #define SU_MEMSET __builtin_memset
#endif /* SU_MEMSET */
#if !defined(SU_MEMCMP)
    #define SU_MEMCMP __builtin_memcmp
#endif /* SU_MEMCMP */
#if !defined(SU_MEMCHR)
    #define SU_MEMCHR __builtin_memchr
#endif /* SU_MEMCHR */
#if !defined(SU_STRCPY)
    #define SU_STRCPY __builtin_strcpy
#endif /* SU_STRCPY */
#if !defined(SU_STRNCPY)
    #define SU_STRNCPY __builtin_strncpy
#endif /* SU_STRNCPY */
#if !defined(SU_STRCMP)
    #define SU_STRCMP __builtin_strcmp
#endif /* SU_STRCMP */
#if !defined(SU_STRNCMP)
    #define SU_STRNCMP __builtin_strncmp
#endif /* SU_STRNCMP */
#if !defined(SU_STRLEN)
    #define SU_STRLEN __builtin_strlen
#endif /* SU_STRLEN */
#if !defined(SU_ABS)
    #define SU_ABS __builtin_abs
#endif /* SU_ABS */
#if !defined(SU_FABS)
    #define SU_FABS __builtin_fabs
#endif /* SU_FABS */
#if !defined(SU_FABSF)
    #define SU_FABSF __builtin_fabsf
#endif /* SU_FABSF */
#if !defined(SU_OFFSETOF)
    #define SU_OFFSETOF __builtin_offsetof
#endif /* SU_OFFSETOF */

#else

#define SU_IGNORE_WARNING(w)
#define SU_IGNORE_WARNINGS_START
#define SU_IGNORE_WARNINGS_END

#if defined(__TINYC__)
    #define SU_ALIGNOF __alignof__
#else
    #define SU_ALIGNOF alignof
#endif
#define SU_THREAD_LOCAL thread_local
#define SU_TYPEOF typeof
#define SU_STATIC_ASSERT(x) typedef char SU_CONCAT(static_assertion_, __LINE__)[(x) ? 1 : -1]

/* ? TODO: default simd impls instead of libc ones */
#if !defined(SU_MEMCPY)
    #define SU_MEMCPY memcpy
#endif /* SU_MEMCPY */
#if !defined(SU_MEMMOVE)
    #define SU_MEMMOVE memmove
#endif /* SU_MEMMOVE */
#if !defined(SU_MEMSET)
    #define SU_MEMSET memset
#endif /* SU_MEMSET */
#if !defined(SU_MEMCMP)
    #define SU_MEMCMP memcmp
#endif /* SU_MEMCMP */
#if !defined(SU_MEMCHR)
    #define SU_MEMCHR memchr
#endif /* SU_MEMCHR */
#if !defined(SU_STRCPY)
    #define SU_STRCPY strcpy
#endif /* SU_STRCPY */
#if !defined(SU_STRNCPY)
    #define SU_STRNCPY strncpy
#endif /* SU_STRNCPY */
#if !defined(SU_STRCMP)
    #define SU_STRCMP strcmp
#endif /* SU_STRCMP */
#if !defined(SU_STRNCMP)
    #define SU_STRNCMP strncmp
#endif /* SU_STRNCMP */
#if !defined(SU_STRLEN)
    #define SU_STRLEN strlen
#endif /* SU_STRLEN */
#if !defined(SU_ABS)
    #define SU_ABS abs
#endif /* SU_ABS */
#if !defined(SU_OFFSETOF)
    #define SU_OFFSETOF offsetof
#endif /* SU_OFFSETOF */

#include <math.h>
#if !defined(SU_FABS)
    #define SU_FABS fabs
#endif /* SU_FABS */
#if !defined(SU_FABSF)
    #define SU_FABSF fabsf
#endif /* SU_FABSF */
#endif

#define SU_UNREACHABLE 0

#if defined(__has_feature)
    #define SU_HAS_FEATURE(x) __has_feature(x)
#else
    #define SU_HAS_FEATURE(x) 0
#endif

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

#if SU_HAS_BUILTIN(__builtin_clzg)
    #define SU_COUNT_LEADING_ZEROS(x, fallback) __builtin_clzg(x, fallback)
#else
    #error "TODO: implement __builtin_clzg"
#endif

#if SU_HAS_BUILTIN(__builtin_bswap16)
    #define SU_BSWAP16(x) __builtin_bswap16(x)
#else
    #error "TODO: implement __builtin_bswap16"
#endif

#if SU_HAS_BUILTIN(__builtin_bswap32)
    #define SU_BSWAP32(x) __builtin_bswap32(x)
#else
    #error "TODO: implement __builtin_bswap32"
#endif

#if SU_HAS_BUILTIN(__builtin_bswap64)
    #define SU_BSWAP64(x) __builtin_bswap64(x)
#else
    #error "TODO: implement __builtin_bswap64"
#endif

#if SU_HAS_BUILTIN(__builtin_unreachable)
    #define SU_ASSERT_UNREACHABLE SU_STMT_START { SU_ASSERT(SU_UNREACHABLE); __builtin_unreachable(); } SU_STMT_END
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
#define SU_SWAP(a, b) SU_STMT_START { SU_TYPEOF(a) su__tmp = a; a = b; b = su__tmp; } SU_STMT_END

#define SU_STMT_START do
#define SU_STMT_END while (0)

#define SU__STRINGIFY(x) #x
#define SU_STRINGIFY(x) SU__STRINGIFY(x)

#define SU_NOTUSED(x) (void)x

#if SU_WITH_DEBUG
    #define SU_ASSERT(expr) \
        SU_STMT_START { \
            if (SU_UNLIKELY(!(expr))) { \
                su_log_stderr("%s:%d: assertion '%s' failed", __FILE__, __LINE__, #expr); \
                abort(); \
            } \
        } SU_STMT_END
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

#define SU_LLIST_FIELDS(type) type *head; type *tail; size_t count
#define SU_LLIST_NODE_FIELDS(type) type *next; type *prev

#define SU_LLIST_APPEND_HEAD(llist_ptr, node_ptr) \
SU_STMT_START { \
    (node_ptr)->prev = NULL; \
    (node_ptr)->next = (llist_ptr)->head; \
    if ((llist_ptr)->head) { \
        (llist_ptr)->head->prev = (node_ptr); \
    } else { \
        (llist_ptr)->tail = (node_ptr); \
    } \
    (llist_ptr)->head = (node_ptr); \
    (llist_ptr)->count++; \
} SU_STMT_END

#define SU_LLIST_APPEND_TAIL(llist_ptr, node_ptr) \
SU_STMT_START { \
    (node_ptr)->next = NULL; \
    (node_ptr)->prev = (llist_ptr)->tail; \
    if ((llist_ptr)->tail) { \
        (llist_ptr)->tail->next = (node_ptr); \
    } else { \
        (llist_ptr)->head = (node_ptr); \
    } \
    (llist_ptr)->tail = (node_ptr); \
    (llist_ptr)->count++; \
} SU_STMT_END

#define SU_LLIST_POP(llist_ptr, node_ptr) \
SU_STMT_START { \
    if ((node_ptr)->prev) { \
        (node_ptr)->prev->next = (node_ptr)->next; \
    } else { \
        (llist_ptr)->head = (node_ptr)->next; \
    } \
    if ((node_ptr)->next) { \
        (node_ptr)->next->prev = (node_ptr)->prev; \
    } else { \
        (llist_ptr)->tail = (node_ptr)->prev; \
    } \
    SU_ASSERT((llist_ptr)->count > 0); \
    (llist_ptr)->count--; \
} SU_STMT_END
/* TODO: llist_insert/append_list/after/before */

/* TODO: rework */
#define SU_HASH_TABLE_DECLARE(type, key_type, hash_key_func, keys_equal_func, move_item_func, collisions_to_resize) \
\
typedef struct su_hash_table__##type { \
    type *items; \
    size_t capacity; \
} su_hash_table__##type##__t; \
\
static void su_hash_table__##type##__init(su_hash_table__##type##__t *, const su_allocator_t *, size_t initial_size); \
static void su_hash_table__##type##__fini(su_hash_table__##type##__t *, const su_allocator_t *); \
static void su_hash_table__##type##__resize(su_hash_table__##type##__t *, const su_allocator_t *, size_t new_size); \
static su_bool32_t su_hash_table__##type##__add(su_hash_table__##type##__t *, const su_allocator_t *, key_type, type **out); \
static su_bool32_t su_hash_table__##type##__get(su_hash_table__##type##__t *, key_type, type **out); \
static su_bool32_t su_hash_table__##type##__del(su_hash_table__##type##__t *, key_type, type *out)

#define SU_HASH_TABLE_FIELDS(key_type) \
    key_type key; \
    su_bool32_t occupied; \
    su_bool32_t tombstone

/* ? TODO: argc, argv as params */
#define SU_ARGPARSE_LOOP_BEGIN \
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
#define SU_ARGPARSE_LOOP_END \
        } \
    }

#define SU_ARGPARSE_KEY argc_

#define SU_ARGPARSE_VALUE \
    (((argv[0][1] == '\0') && (argv[1] == NULL)) ? \
        NULL : \
        (brk_ = 1, (argv[0][1] != '\0') ? \
            (&argv[0][1]) : \
            (argc--, argv++, argv[0])))

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

/* TODO: remove */
#if defined(__cplusplus) && (__cplusplus > 199711L)
    #undef SU_STATIC_ASSERT
    #define SU_STATIC_ASSERT(x) static_assert(x, "")
    typedef char32_t su_c32_t;
#else
    typedef uint32_t su_c32_t;
#endif

#if defined(__BYTE_ORDER__)
    #define SU_BYTE_ORDER_IS_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#else
    /* TODO */
    #define SU_BYTE_ORDER_IS_LITTLE_ENDIAN 1
#endif

SU_STATIC_ASSERT(SU_BYTE_ORDER_IS_LITTLE_ENDIAN); /* TODO: support SU_BYTE_ORDER_BIG_ENDIAN */

typedef uint32_t su_bool32_t;

typedef struct su_fat_ptr {
    size_t len;
    void *ptr;
} su_fat_ptr_t;

typedef struct su_allocator su_allocator_t;
typedef void *(*su_allocator_alloc_func_t)(const su_allocator_t *, size_t size, size_t alignment);
typedef void (*su_allocator_free_func_t)(const su_allocator_t *, void *ptr);

struct su_allocator {
    su_allocator_alloc_func_t alloc;
    su_allocator_free_func_t free;
};

typedef struct su_page_allocator_header {
    void *ptr;
    size_t size;
} su_page_allocator_header_t;

#define SU_ALLOCT(dst, alloc_)                    (dst) = (SU_TYPEOF(dst))(alloc_)->alloc((alloc_), sizeof(*(dst)), SU_ALIGNOF(*(dst)))
#define SU_ALLOCTS(dst, alloc_, size)             (dst) = (SU_TYPEOF(dst))(alloc_)->alloc((alloc_), (size), SU_ALIGNOF(*(dst)))
#define SU_ALLOCTSA(dst, alloc_, size, alignment) (dst) = (SU_TYPEOF(dst))(alloc_)->alloc((alloc_), (size), (alignment))

#define SU_ALLOCCT(dst, alloc_)                    SU_STMT_START { (dst) = (SU_TYPEOF(dst))(alloc_)->alloc((alloc_), sizeof(*(dst)), SU_ALIGNOF(*(dst))); SU_MEMSET((dst), 0, sizeof(*(dst))); } SU_STMT_END
#define SU_ALLOCCTS(dst, alloc_, size)             SU_STMT_START { (dst) = (SU_TYPEOF(dst))(alloc_)->alloc((alloc_), (size), SU_ALIGNOF(*(dst))); SU_MEMSET((dst), 0, (size)); } SU_STMT_END
#define SU_ALLOCCTSA(dst, alloc_, size, alignment) SU_STMT_START { (dst) = (SU_TYPEOF(dst))(alloc_)->alloc((alloc_), (size), (alignment)); SU_MEMSET((dst), 0, (size)); } SU_STMT_END

#define SU_FREE(alloc_, ptr)                       (alloc_)->free((alloc_), (ptr))

#define SU_ARRAY_ALLOC(dst, alloc_, capacity)  SU_ALLOCTS(dst, alloc_, (sizeof(dst[0]) * capacity))
#define SU_ARRAY_ALLOCC(dst, alloc_, capacity) SU_ALLOCCTS(dst, alloc_, (sizeof(dst[0]) * capacity))
/* ? TODO: ARRAY_CPY, ARRAY_RESIZE */

typedef struct su_string {
    su_bool32_t free_contents;
    su_bool32_t nul_terminated;
    size_t len; /* excluding '\0' */
    char *s;
} su_string_t;

#define SU_STRING_FMT "%.*s"
#define SU_STRING_ARG(str) (int)(str).len, (str).s

typedef struct su_arena_block {
    size_t ptr;
    size_t size;
    uint8_t *data;
} su_arena_block_t;

typedef struct su_arena {
    su_arena_block_t *blocks;
    size_t blocks_count, blocks_capacity;
} su_arena_t;

typedef struct su_file_cache {
    SU_HASH_TABLE_FIELDS(su_string_t);
    struct timespec st_mtim;
    su_fat_ptr_t data;
} su_file_cache_t;

/* TODO: better hash function */
SU_HASH_TABLE_DECLARE(su_file_cache_t, su_string_t, su_stbds_hash_string, su_string_equal, SU_MEMCPY, 16);

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

typedef struct su_json_writer {
    su_json_buffer_t buf;

    /* ? TODO: preallocate */
    su__json_writer_state_t *state;
    size_t state_count, state_capacity;
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

typedef struct su_json_tokener {
    su_string_t str;
    size_t pos;
    su_json_buffer_t buf;
    size_t last_escape_idx; /* in buf */
    size_t depth;
    su__json_tokener_state_t *state;
    size_t state_count;
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

typedef struct su_json_ast_key_value su_json_ast_key_value_t;

typedef struct su_json_ast_node_value_object {
    su_json_ast_key_value_t *kvs;
    size_t count, capacity;
} su_json_ast_node_value_object_t;

typedef struct su_json_ast_node_value_array {
    su_json_ast_node_t *nodes; 
    size_t count, capacity;
} su_json_ast_node_value_array_t;

typedef union su_json_ast_node_value {
    su_json_ast_node_value_object_t o; /* SU_JSON_AST_NODE_TYPE_OBJECT */ /* ? TODO: hash table */
    su_json_ast_node_value_array_t a; /* SU_JSON_AST_NODE_TYPE_ARRAY */
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

static inline SU_ATTRIBUTE_ALWAYS_INLINE su_string_t su_string_(const char *literal);
static SU_ATTRIBUTE_FORMAT_PRINTF(3, 4) void su_string_init_format(
    su_string_t *, const su_allocator_t *, const char *fmt, ...);
static void su_string_init_len(su_string_t *, const su_allocator_t *,
        const char *s, size_t len, su_bool32_t nul_terminate);
static void su_string_init_string(su_string_t *, const su_allocator_t *, su_string_t src);
static void su_string_init(su_string_t *, const su_allocator_t *, const char *src);
static void su_string_fini(su_string_t *, const su_allocator_t *);
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

/* ? TODO: variants without su_allocator_t */
static void *su_libc_alloc(const su_allocator_t *, size_t size, size_t alignment);
static void su_libc_free(const su_allocator_t *, void *ptr);
static void *su_page_alloc(const su_allocator_t *, size_t size, size_t alignment);
static void su_page_free(const su_allocator_t *, void *ptr);

static void su_arena_init(su_arena_t *, const su_allocator_t *, size_t initial_block_size);
static void su_arena_fini(su_arena_t *, const su_allocator_t *);
static su_arena_block_t *su_arena_add_block(su_arena_t *, const su_allocator_t *, size_t size);
static void *su_arena_alloc(su_arena_t *, const su_allocator_t *, size_t size, size_t alignment);
static size_t su_arena_alloc_get_size(void *ptr);
static void su_arena_reset(su_arena_t *, const su_allocator_t *);

static su_bool32_t su_fat_ptr_equal(su_fat_ptr_t a, su_fat_ptr_t b);

/* static SU_ATTRIBUTE_PURE size_t su_sdbm_hash(su_string_t); */
/* static SU_ATTRIBUTE_PURE size_t su_djb2_hash(su_string_t); */
static SU_ATTRIBUTE_PURE size_t su_stbds_hash_string(su_string_t);
static SU_ATTRIBUTE_PURE size_t su_stbds_hash(su_fat_ptr_t);

/* ? TODO: pitch for all surfaces */
static void su_argb32_premultiply_alpha(uint32_t *dst, uint32_t *src, size_t count);
static void su_bswap32_argb32_premultiply_alpha(uint32_t *dst, uint32_t *src, size_t count);
static void su_abgr32_convert_argb32_premultiply_alpha(uint32_t *dst, uint32_t *src, size_t count);
static void su_abgr32_convert_argb32(uint32_t *dst, uint32_t *src, size_t count);
static void su_argb32_rect_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color, int32_t x, int32_t y, uint32_t w, uint32_t h);
static void su_argb32_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t *src, uint32_t src_w, uint32_t src_h,
        int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y,
        uint32_t w, uint32_t h);
static void su_argb32_mask8_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color,
        uint8_t *mask, uint32_t mask_w, uint32_t mask_h, uint32_t mask_pitch,
        int32_t dst_x, int32_t dst_y, int32_t mask_x, int32_t mask_y,
        uint32_t w, uint32_t h);
static void su_argb32_mask1_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color,
        uint8_t *mask, uint32_t mask_w, uint32_t mask_h, uint32_t mask_pitch,
        int32_t dst_x, int32_t dst_y, int32_t mask_x, int32_t mask_y,
        uint32_t w, uint32_t h);
static void su_argb32_mask24_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color,
        uint8_t *mask, uint32_t mask_w, uint32_t mask_h, uint32_t mask_pitch,
        int32_t dst_x, int32_t dst_y, int32_t mask_x, int32_t mask_y,
        uint32_t w, uint32_t h);
static void su_argb32_mask24v_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color,
        uint8_t *mask, uint32_t mask_w, uint32_t mask_h, uint32_t mask_pitch,
        int32_t dst_x, int32_t dst_y, int32_t mask_x, int32_t mask_y,
        uint32_t w, uint32_t h);
static void su_argb32_mask8_bilinear_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color,
        uint8_t *mask, uint32_t mask_w, uint32_t mask_h, uint32_t mask_pitch,
        int32_t dst_x, int32_t dst_y,
        uint32_t w, uint32_t h);
static void su_argb32_bilinear_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t *src, uint32_t src_w, uint32_t src_h,
        int32_t dst_x, int32_t dst_y,
        uint32_t w, uint32_t h);

static su_bool32_t su_read_entire_file(su_string_t path, su_fat_ptr_t *out, const su_allocator_t *);
static su_bool32_t su_read_entire_file_with_cache(su_string_t path, su_fat_ptr_t *out,
    const su_allocator_t *, su_hash_table__su_file_cache_t__t *cache);

static su_bool32_t su_fd_set_nonblock(int);
static su_bool32_t su_fd_set_cloexec(int);

static int64_t su_timespec_to_ms(struct timespec);
static int64_t su_now_ms(clockid_t);

static su_bool32_t su_locale_is_utf8(void);
static void su_nop(void *notused, ...);

/*static void *su_base64_decode(su_string_t, const su_allocator_t *);*/

static void su_json_writer_init(su_json_writer_t *, const su_allocator_t *, size_t initial_bufsize);
static void su_json_writer_fini(su_json_writer_t *, const su_allocator_t *);
static void su_json_writer_reset(su_json_writer_t *);

static void su_json_writer_object_begin(su_json_writer_t *, const su_allocator_t *);
static void su_json_writer_object_end(su_json_writer_t *, const su_allocator_t *);
static void su_json_writer_object_key_escape(su_json_writer_t *, const su_allocator_t *, su_string_t);
static void su_json_writer_object_key(su_json_writer_t *, const su_allocator_t *, su_string_t);

static void su_json_writer_array_begin(su_json_writer_t *, const su_allocator_t *);
static void su_json_writer_array_end(su_json_writer_t *, const su_allocator_t *);

static void su_json_writer_null(su_json_writer_t *, const su_allocator_t *);
static void su_json_writer_bool(su_json_writer_t *, const su_allocator_t *, su_bool32_t);
static void su_json_writer_int(su_json_writer_t *, const su_allocator_t *, int64_t);
static void su_json_writer_uint(su_json_writer_t *, const su_allocator_t *, uint64_t);
static void su_json_writer_double(su_json_writer_t *, const su_allocator_t *, double);
static void su_json_writer_string_escape(su_json_writer_t *, const su_allocator_t *, su_string_t);
static void su_json_writer_string(su_json_writer_t *, const su_allocator_t *, su_string_t);

static void su_json_writer_raw(su_json_writer_t *, const su_allocator_t *, void *data, size_t len);
static void su_json_writer_token(su_json_writer_t *, const su_allocator_t *, su_json_token_t);
static void su_json_writer_ast_node(su_json_writer_t *, const su_allocator_t *, su_json_ast_node_t *);

static void su_json_tokener_set_string(su_json_tokener_t *, const su_allocator_t *, su_string_t);
static su_json_tokener_state_t su_json_tokener_next(su_json_tokener_t *,
    const su_allocator_t *, su_json_token_t *out);
static su_json_tokener_state_t su_json_tokener_ast(su_json_tokener_t *, const su_allocator_t *,
    su_json_ast_t *, uint32_t stop_depth, su_bool32_t check_for_repeating_keys);
static void su_json_ast_reset(su_json_ast_t *);

static su_json_ast_node_t *su_json_ast_node_object_get(su_json_ast_node_t *, su_string_t key);
static SU_ATTRIBUTE_ALWAYS_INLINE void su_json_tokener_advance_assert(su_json_tokener_t *,
    const su_allocator_t *, su_json_token_t *token_out);
static SU_ATTRIBUTE_ALWAYS_INLINE void su_json_tokener_advance_assert_type(
    su_json_tokener_t *, const su_allocator_t *,
    su_json_token_t *token_out, su_json_token_type_t expected_type);

/* ? TODO: strip by default, flag to enable */
#if defined(SU_STRIP_PREFIXES)

#define CONCAT SU_CONCAT

#define PRAGMA SU_PRAGMA

#define IGNORE_WARNING SU_IGNORE_WARNING
#define IGNORE_WARNINGS_START SU_IGNORE_WARNINGS_START
#define IGNORE_WARNINGS_END SU_IGNORE_WARNINGS_END

#define ALIGNOF SU_ALIGNOF
#define TYPEOF SU_TYPEOF
#define THREAD_LOCAL SU_THREAD_LOCAL
#define STATIC_ASSERT SU_STATIC_ASSERT

#define MEMCPY SU_MEMCPY
#define MEMMOVE SU_MEMMOVE
#define MEMSET SU_MEMSET
#define MEMCMP SU_MEMCMP
#define MEMCHR SU_MEMCHR

#define STRCPY SU_STRCPY
#define STRNCPY SU_STRNCPY
#define STRCMP SU_STRCMP
#define STRNCMP SU_STRNCMP
#define STRLEN SU_STRLEN
#define STRNLEN SU_STRNLEN
#define ABS SU_ABS
#define FABS SU_FABS
#define FABSF SU_FABSF

#define OFFSETOF SU_OFFSETOF

#define HAS_FEATURE SU_HAS_FEATURE
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
#define COUNT_LEADING_ZEROS SU_COUNT_LEADING_ZEROS
#define BSWAP16 SU_BSWAP16
#define BSWAP32 SU_BSWAP32
#define BSWAP64 SU_BSWAP64

#define BYTE_ORDER_IS_LITTLE_ENDIAN SU_BYTE_ORDER_IS_LITTLE_ENDIAN
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
#define SWAP SU_SWAP

#define STRINGIFY SU_STRINGIFY

#define PAD8 SU_PAD8
#define PAD16 SU_PAD16
#define PAD32 SU_PAD32
#define TRUE SU_TRUE
#define FALSE SU_FALSE

#define DEBUG_LOG SU_DEBUG_LOG

#define STRING_FMT SU_STRING_FMT
#define STRING_ARG SU_STRING_ARG

#define LLIST_FIELDS SU_LLIST_FIELDS
#define LLIST_NODE_FIELDS SU_LLIST_NODE_FIELDS
#define LLIST_APPEND_HEAD SU_LLIST_APPEND_HEAD
#define LLIST_APPEND_TAIL SU_LLIST_APPEND_TAIL
#define LLIST_POP SU_LLIST_POP

#define HASH_TABLE_DECLARE SU_HASH_TABLE_DECLARE
#define HASH_TABLE_DEFINE SU_HASH_TABLE_DEFINE
#define HASH_TABLE_DECLARE_DEFINE SU_HASH_TABLE_DECLARE_DEFINE
#define HASH_TABLE_FIELDS SU_HASH_TABLE_FIELDS

#define ARGPARSE_LOOP_BEGIN SU_ARGPARSE_LOOP_BEGIN
#define ARGPARSE_LOOP_END SU_ARGPARSE_LOOP_END
#define ARGPARSE_KEY SU_ARGPARSE_KEY
#define ARGPARSE_VALUE SU_ARGPARSE_VALUE

typedef su_bool32_t bool32_t;
typedef su_c32_t c32_t;
typedef su_fat_ptr_t fat_ptr_t;

typedef su_string_t string_t;

typedef su_allocator_t allocator_t;
typedef su_allocator_alloc_func_t alloc_func_t;
typedef su_allocator_free_func_t free_func_t;
typedef su_page_allocator_header_t page_allocator_header_t;

typedef su_arena_block_t arena_block_t;
typedef su_arena_t arena_t;

typedef su_file_cache_t file_cache_t;

typedef su_json_buffer_t json_buffer_t;
typedef su_json_writer_t json_writer_t;
typedef su_json_tokener_t json_tokener_t;
typedef su_json_tokener_state_t json_tokener_state_t;
#define JSON_TOKENER_STATE_SUCCESS SU_JSON_TOKENER_STATE_SUCCESS
#define JSON_TOKENER_STATE_ERROR SU_JSON_TOKENER_STATE_ERROR
#define JSON_TOKENER_STATE_EOF SU_JSON_TOKENER_STATE_EOF
#define JSON_TOKENER_STATE_MORE_DATA_EXPECTED SU_JSON_TOKENER_STATE_MORE_DATA_EXPECTED
typedef su_json_token_type_t json_token_type_t;
#define JSON_TOKEN_TYPE_OBJECT_START SU_JSON_TOKEN_TYPE_OBJECT_START
#define JSON_TOKEN_TYPE_OBJECT_END SU_JSON_TOKEN_TYPE_OBJECT_END
#define JSON_TOKEN_TYPE_KEY SU_JSON_TOKEN_TYPE_KEY
#define JSON_TOKEN_TYPE_ARRAY_START SU_JSON_TOKEN_TYPE_ARRAY_START
#define JSON_TOKEN_TYPE_ARRAY_END SU_JSON_TOKEN_TYPE_ARRAY_END
#define JSON_TOKEN_TYPE_STRING SU_JSON_TOKEN_TYPE_STRING
#define JSON_TOKEN_TYPE_BOOL SU_JSON_TOKEN_TYPE_BOOL
#define JSON_TOKEN_TYPE_NULL SU_JSON_TOKEN_TYPE_NULL
#define JSON_TOKEN_TYPE_DOUBLE SU_JSON_TOKEN_TYPE_DOUBLE
#define JSON_TOKEN_TYPE_INT SU_JSON_TOKEN_TYPE_INT
#define JSON_TOKEN_TYPE_UINT SU_JSON_TOKEN_TYPE_UINT
typedef su_json_ast_node_type_t json_ast_node_type_t; 
#define JSON_AST_NODE_TYPE_NONE SU_JSON_AST_NODE_TYPE_NONE
#define JSON_AST_NODE_TYPE_OBJECT SU_JSON_AST_NODE_TYPE_OBJECT
#define JSON_AST_NODE_TYPE_ARRAY SU_JSON_AST_NODE_TYPE_ARRAY
#define JSON_AST_NODE_TYPE_STRING SU_JSON_AST_NODE_TYPE_STRING
#define JSON_AST_NODE_TYPE_BOOL SU_JSON_AST_NODE_TYPE_BOOL
#define JSON_AST_NODE_TYPE_NULL SU_JSON_AST_NODE_TYPE_NULL
#define JSON_AST_NODE_TYPE_DOUBLE SU_JSON_AST_NODE_TYPE_DOUBLE
#define JSON_AST_NODE_TYPE_INT SU_JSON_AST_NODE_TYPE_INT
#define JSON_AST_NODE_TYPE_UINT SU_JSON_AST_NODE_TYPE_UINT
typedef su_json_token_value_t json_token_value_t;
typedef su_json_token_t json_token_t;
typedef su_json_ast_node_value_object_t json_ast_node_value_object_t;
typedef su_json_ast_node_value_array_t json_ast_node_value_array_t;
typedef su_json_ast_node_value_t json_ast_node_value_t;
typedef su_json_ast_node_t json_ast_node_t;
typedef su_json_ast_key_value_t json_ast_key_value_t;
typedef su_json_ast_t json_ast_t;

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

#define string su_string_
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

#define ALLOCT SU_ALLOCT
#define ALLOCTS SU_ALLOCTS
#define ALLOCTSA SU_ALLOCTSA
#define ALLOCCT SU_ALLOCCT
#define ALLOCCTS SU_ALLOCCTS
#define ALLOCCTSA SU_ALLOCCTSA
#define FREE SU_FREE
#define ARRAY_ALLOC SU_ARRAY_ALLOC
#define ARRAY_ALLOCC SU_ARRAY_ALLOCC

#define libc_alloc su_libc_alloc
#define libc_free su_libc_free

#define page_alloc su_page_alloc
#define page_free su_page_free

#define arena_init su_arena_init
#define arena_fini su_arena_fini
#define arena_add_block su_arena_add_block
#define arena_alloc su_arena_alloc
#define arena_alloc_get_size su_arena_alloc_get_size
#define arena_reset su_arena_reset

#define fat_ptr_equal su_fat_ptr_equal

/*#define sdbm_hash su_sdbm_hash*/
/*#define djb2_hash su_djb2_hash*/
#define stbds_hash_string su_stbds_hash_string
#define stbds_hash su_stbds_hash

#define argb32_premultiply_alpha su_argb32_premultiply_alpha
#define bswap32_argb32_premultiply_alpha su_bswap32_argb32_premultiply_alpha
#define abgr32_convert_argb32_premultiply_alpha su_abgr32_convert_argb32_premultiply_alpha
#define abgr32_convert_argb32 su_abgr32_convert_argb32
#define argb32_rect_blend_argb32 su_argb32_rect_blend_argb32
#define argb32_blend_argb32 su_argb32_blend_argb32
#define argb32_mask8_blend_argb32 su_argb32_mask8_blend_argb32
#define argb32_mask1_blend_argb32 su_argb32_mask1_blend_argb32
#define argb32_mask24_blend_argb32 su_argb32_mask24_blend_argb32
#define argb32_mask24v_blend_argb32 su_argb32_mask24v_blend_argb32
#define argb32_mask8_bilinear_blend_argb32 su_argb32_mask8_bilinear_blend_argb32

#define read_entire_file su_read_entire_file
#define read_entire_file_with_cache su_read_entire_file_with_cache

#define fd_set_nonblock su_fd_set_nonblock
#define fd_set_cloexec su_fd_set_cloexec

#define timespec_to_ms su_timespec_to_ms
#define now_ms su_now_ms

/*#define base64_decode su_base64_decode */

#define locale_is_utf8 su_locale_is_utf8

#define nop su_nop

#endif /* defined(SU_STRIP_PREFIXES) */

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* !defined(SU_HEADER) */

#if defined(SU_IMPLEMENTATION) && !defined(SU__REIMPLEMENTATION_GUARD)
#define SU__REIMPLEMENTATION_GUARD

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <uchar.h>
#include <sys/mman.h>

#include <limits.h>
#include <float.h>

#if SU_HAS_FEATURE(address_sanitizer)
    #include <sanitizer/asan_interface.h>
#endif

#if SU_WITH_SIMD && defined(__x86_64__)
    #include <immintrin.h>
#endif /* SU_WITH_SIMD && defined(__x86_64__) */

#define STB_SPRINTF_MIN 128
#define STB_SPRINTF_STATIC
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOUNALIGNED

#include <stb_sprintf.h>

/* TODO: rework */
#define SU_HASH_TABLE_DEFINE(type, key_type, hash_key_func, keys_equal_func, move_item_func, collisions_to_resize) \
\
static void su_hash_table__##type##__init(su_hash_table__##type##__t *ht, const su_allocator_t *alloc, size_t initial_size) { \
    SU_ASSERT(initial_size > 0); \
    SU_ARRAY_ALLOCC(ht->items, alloc, initial_size); \
    ht->capacity = initial_size; \
} \
\
static void su_hash_table__##type##__fini(su_hash_table__##type##__t *ht, const su_allocator_t *alloc) { \
    SU_FREE(alloc, ht->items); \
} \
\
static void su_hash_table__##type##__resize(su_hash_table__##type##__t *ht, const su_allocator_t *alloc, size_t new_size) { \
    size_t i; \
    su_hash_table__##type##__t new_ht; \
    su_hash_table__##type##__init(&new_ht, alloc, new_size); \
    for ( i = 0; i < ht->capacity; ++i) { \
        type *it = &ht->items[i]; \
        if (it->occupied && !it->tombstone) { \
            type *new_it; \
            su_hash_table__##type##__add(&new_ht, alloc, it->key, &new_it); \
            move_item_func(new_it, it, sizeof(type)); \
        } \
    } \
    su_hash_table__##type##__fini(ht, alloc); \
    *ht = new_ht; \
} \
\
static su_bool32_t su_hash_table__##type##__add(su_hash_table__##type##__t *ht, const su_allocator_t *alloc, key_type key, type **out) { \
    size_t h = (hash_key_func(key) % ht->capacity); \
    type *it = &ht->items[h]; \
    size_t c = 0; \
    for ( ; \
            it->occupied && !keys_equal_func(it->key, key) && (c < ht->capacity); \
            ++c) { \
        it = &ht->items[++h % ht->capacity]; \
    } \
    if (c >= collisions_to_resize) { \
        su_hash_table__##type##__resize(ht, alloc, ht->capacity * 2); \
        return su_hash_table__##type##__add(ht, alloc, key, out); \
    } else if (it->occupied) { \
        if (keys_equal_func(it->key, key)) { \
            if (out) { \
                *out = it; \
            } \
            return SU_FALSE; \
        } \
        su_hash_table__##type##__resize(ht, alloc, ht->capacity * 2); \
        return su_hash_table__##type##__add(ht, alloc, key, out); \
    } else { \
        it->key = key; \
        it->occupied = SU_TRUE; \
        /*it->tombstone = SU_FALSE;*/ \
        if (out) { \
            *out = it; \
        } \
        return SU_TRUE; \
    } \
} \
\
static su_bool32_t su_hash_table__##type##__get(su_hash_table__##type##__t *ht, key_type key, type **out) { \
    size_t h = (hash_key_func(key) % ht->capacity); \
    type *it = &ht->items[h]; \
    size_t c = 0; \
    for ( ; \
            it->occupied && !keys_equal_func(it->key, key) && (c < ht->capacity); \
            ++c) { \
        it = &ht->items[++h % ht->capacity]; \
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
    size_t h = (hash_key_func(key) % ht->capacity); \
    type *it = &ht->items[h]; \
    size_t c = 0; \
    for ( ; \
            it->occupied && !keys_equal_func(it->key, key) && (c < ht->capacity); \
            ++c) { \
        it = &ht->items[++h % ht->capacity]; \
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

/* TODO: tombstone threshold, shrink */
#define SU_HASH_TABLE_DECLARE_DEFINE(type, key_type, hash_key_func, keys_equal_func, move_item_func, collisions_to_resize) \
    SU_HASH_TABLE_DECLARE(type, key_type, hash_key_func, keys_equal_func, move_item_func, collisions_to_resize); \
    SU_HASH_TABLE_DEFINE(type, key_type, hash_key_func, keys_equal_func, move_item_func, collisions_to_resize)


static void *su_libc_alloc(const su_allocator_t *alloc, size_t size, size_t alignment) {
    void *ptr;
    int s;

    SU_NOTUSED(alloc);

    SU_ASSERT(size > 0);
    SU_ASSERT((alignment > 0) && ((alignment == 1) || ((alignment & (alignment - 1)) == 0)));

    alignment = SU_MAX(alignment, sizeof(void *));

    s = posix_memalign(&ptr, alignment, (size + alignment - 1) & ~(alignment - 1));
    if ( SU_UNLIKELY(s != 0)) {
        su_abort(s, "posix_memalign: %s", strerror(s));
    }

    SU_ASSERT(((uintptr_t)ptr % alignment) == 0);
    return ptr;
}

static void su_libc_free(const su_allocator_t *alloc, void *ptr) {
    SU_NOTUSED(alloc);
    free(ptr);
}

static void *su_page_alloc(const su_allocator_t *alloc, size_t size, size_t alignment) {
    void *ret;
    su_page_allocator_header_t *hdr;
    size_t real_size;
    void *ptr;

    SU_ASSERT(size > 0);
    SU_ASSERT((alignment > 0) && ((alignment == 1) || ((alignment & (alignment - 1)) == 0)));
    SU_NOTUSED(alloc);

    real_size = (size + alignment + sizeof(*hdr));
    ptr = mmap(NULL, real_size, (PROT_READ | PROT_WRITE), (MAP_ANONYMOUS | MAP_PRIVATE), -1, 0);
    if (SU_UNLIKELY(ptr == MAP_FAILED)) {
        su_abort(errno, "mmap: %s", strerror(errno));
    }

    ret = (void *)((((uintptr_t)ptr + sizeof(*hdr)) + (alignment - 1)) & ~(alignment - 1));
    hdr = (su_page_allocator_header_t *)(void *)((uint8_t *)ret - sizeof(*hdr));
    hdr->size = real_size;
    hdr->ptr = ptr;

    SU_ASSERT(((uintptr_t)ret % alignment) == 0);
    return ret;
}

static void su_page_free(const su_allocator_t *alloc, void *ptr) {
    if (ptr) {
        su_page_allocator_header_t *hdr = (su_page_allocator_header_t *)(void *)((uint8_t *)ptr - sizeof(*hdr));
        int r = munmap(hdr->ptr, hdr->size);
        SU_ASSERT(r == 0);
        SU_NOTUSED(alloc); SU_NOTUSED(r);
    }
}

static su_bool32_t su_fat_ptr_equal(su_fat_ptr_t a, su_fat_ptr_t b) {
    if (a.len != b.len) {
        return SU_FALSE;
    }

    return (SU_MEMCMP(a.ptr, b.ptr, a.len) == 0);
}

static char *su__log_va_stbsp_vsprintfcb(const char *buf, void *data, int len) {
    int fd = (int)(intptr_t)data;
    ssize_t total = 0;
    while (total < len) {
        ssize_t written_bytes = write(fd, &buf[total], (size_t)(len - total));
        if (written_bytes <= 0) {
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

static inline SU_ATTRIBUTE_ALWAYS_INLINE su_string_t su_string_(const char *literal) {
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
        const su_allocator_t *alloc;
    } *_data = (struct d *)data;
    su_string_t *str = _data->str;
    const su_allocator_t *alloc = _data->alloc;
    
    SU_NOTUSED(buf);

    str->len += (size_t)len;

    {
        /* TODO: optimize size */
        char *new_s;
        SU_ALLOCTS(new_s, alloc, str->len + STB_SPRINTF_MIN);
        SU_MEMCPY(new_s, str->s, str->len);
        if (str->len != (size_t)len) {
            SU_FREE(alloc, str->s);
        }
        str->s = new_s;
    }

    str->s[str->len] = '\0';

    return &str->s[str->len];
}

static SU_ATTRIBUTE_FORMAT_PRINTF(3, 4) void su_string_init_format(su_string_t *str,
        const su_allocator_t *alloc, const char *fmt, ...) {
    struct data {
        su_string_t *str;
        const su_allocator_t *alloc;
    } data;
    char buf[STB_SPRINTF_MIN];

    va_list args;
    va_start(args, fmt);

    data.str = str;
    data.alloc = alloc;

    str->s = buf;
    str->len = 0;
    stbsp_vsprintfcb(su__string_init_format_stbsp_vsprintfcb_callback, &data, str->s, fmt, args);
    str->free_contents = SU_TRUE;
    str->nul_terminated = SU_TRUE;

    va_end(args);
}

static void su_string_init_len(su_string_t *str, const su_allocator_t *alloc,
        const char *s, size_t len, su_bool32_t nul_terminate) {
    SU_ASSERT((len > 0) || nul_terminate);
    SU_ALLOCTS(str->s, alloc, len + nul_terminate);
    SU_MEMCPY(str->s, s, len);
    if (nul_terminate) {
        str->s[len] = '\0';
    }
    str->len = len;
    str->free_contents = SU_TRUE;
    str->nul_terminated = nul_terminate;
}

static void su_string_init_string(su_string_t *str, const su_allocator_t *alloc, su_string_t src) {
    su_string_init_len(str, alloc, src.s, src.len, src.nul_terminated);
}

static void su_string_init(su_string_t *str, const su_allocator_t *alloc, const char *src) {
    str->len = SU_STRLEN(src);
    SU_ALLOCTS(str->s, alloc, str->len + 1);
    SU_MEMCPY(str->s, src, str->len + 1);
    str->free_contents = SU_TRUE;
    str->nul_terminated = SU_TRUE;
}

static void su_string_fini(su_string_t *str, const su_allocator_t *alloc) {
    if (str->free_contents) {
        SU_FREE(alloc, str->s);
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

    return (SU_MEMCMP(str1.s, str2.s, str1.len) == 0);
}

static int su_string_compare(su_string_t str1, su_string_t str2, size_t max) {
    size_t len = ((max < str1.len) && (max < str2.len)) ? max : SU_MIN(str1.len, str2.len);

    int result = SU_MEMCMP(str1.s, str2.s, len);
    if ((result != 0) || (len == max)) {
        return result;
    }

    return ((str1.len < str2.len) ? -1 : ((str1.len > str2.len) ? 1 : 0));
}

static su_bool32_t su_string_find_char(su_string_t str, char c, su_string_t *view_out) {
    char *s = (char *)SU_MEMCHR(str.s, c, str.len);
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
    static uint32_t lut[] = {
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

static void su_arena_init(su_arena_t *arena, const su_allocator_t *alloc, size_t initial_block_size) {
    arena->blocks_capacity = 8;
    SU_ARRAY_ALLOC(arena->blocks, alloc, arena->blocks_capacity);
    arena->blocks_count = 0;
    su_arena_add_block(arena, alloc, initial_block_size);
}

static void su_arena_fini(su_arena_t *arena, const su_allocator_t *alloc) {
    size_t i = 0;
    for ( ; i < arena->blocks_count; ++i) {
        su_arena_block_t *block = &arena->blocks[i];
#if SU_HAS_FEATURE(address_sanitizer)
        ASAN_UNPOISON_MEMORY_REGION(block->data, block->size);
#endif
        SU_FREE(alloc, block->data);
    }
    SU_FREE(alloc, arena->blocks);
}

static su_arena_block_t *su_arena_add_block(su_arena_t *arena, const su_allocator_t *alloc, size_t size) {
    su_arena_block_t block;

    size = (size + (4096 - 1)) & (size_t)~(4096 - 1);

    SU_ALLOCTSA(block.data, alloc, size, 4096);
    block.size = size;
    block.ptr = 0;

#if SU_HAS_FEATURE(address_sanitizer)
    ASAN_POISON_MEMORY_REGION(block.data, block.size);
#endif

    if (SU_UNLIKELY(arena->blocks_count == arena->blocks_capacity)) {
        su_arena_block_t *new_blocks;
        arena->blocks_capacity *= 2;
        SU_ARRAY_ALLOC(new_blocks, alloc, arena->blocks_capacity);
        SU_MEMCPY(new_blocks, arena->blocks, arena->blocks_count * sizeof(arena->blocks[0]));
        SU_FREE(alloc, arena->blocks);
        arena->blocks = new_blocks;
    }
    arena->blocks[arena->blocks_count++] = block;
    return &arena->blocks[arena->blocks_count - 1];
}

static void *su_arena_alloc(su_arena_t *arena, const su_allocator_t *alloc, size_t size, size_t alignment) {
    su_arena_block_t *block;
    size_t new_size = size;
    size_t new_ptr;
    void *ret;
    size_t i = 0;

    SU_ASSERT(size > 0);
    SU_ASSERT((alignment > 0) && ((alignment == 1) || ((alignment & (alignment - 1)) == 0)));

    alignment = SU_MAX(alignment, sizeof(size_t));

    do {
        block = &arena->blocks[i];
        new_ptr = (sizeof(size_t) + block->ptr + (alignment - 1)) & ~(alignment - 1);
        if (SU_LIKELY((new_ptr < block->size) && (size <= (block->size - new_ptr)))) {
            goto out;
        }
        new_size += block->size;
    } while (SU_UNLIKELY(++i < arena->blocks_count));

    block = su_arena_add_block(arena, alloc, (new_size + alignment) * 2);
    new_ptr = (sizeof(size_t) + (alignment - 1)) & ~(alignment - 1);

out:
#if SU_HAS_FEATURE(address_sanitizer)
    ASAN_UNPOISON_MEMORY_REGION(&block->data[new_ptr - sizeof(size_t)], size + sizeof(size_t));
#endif

    SU_MEMCPY(&block->data[new_ptr - sizeof(size_t)], &size, sizeof(size));
    ret = &block->data[new_ptr];
    block->ptr = (new_ptr + size);

    SU_ASSERT(((uintptr_t)ret % alignment) == 0);
    return ret;
}

static size_t su_arena_alloc_get_size(void *ptr) {
    size_t ret;
    SU_MEMCPY(&ret, (uint8_t *)ptr - sizeof(size_t), sizeof(ret));
    return ret;
}

static void su_arena_reset(su_arena_t *arena, const su_allocator_t *alloc) {
    su_arena_block_t *first_block = &arena->blocks[0];
    size_t size = first_block->size;
    size_t i = 1;

    SU_ASSERT(arena->blocks_count > 0);

    for ( ; i < arena->blocks_count; ++i) {
        su_arena_block_t *block = &arena->blocks[i];
#if SU_HAS_FEATURE(address_sanitizer)
        ASAN_UNPOISON_MEMORY_REGION(block->data, block->size);
#endif
        size += block->size;
        SU_FREE(alloc, block->data);
    }
    arena->blocks_count = 1;

    SU_ASSERT((size % 4096) == 0);

#if SU_HAS_FEATURE(address_sanitizer)
    ASAN_UNPOISON_MEMORY_REGION(first_block->data, first_block->size);
#endif

    if (size > first_block->size) {
        SU_FREE(alloc, first_block->data);
        SU_ALLOCTSA(first_block->data, alloc, size, 4096);
        first_block->size = size;
    }
    first_block->ptr = 0;

#if SU_HAS_FEATURE(address_sanitizer)
    ASAN_POISON_MEMORY_REGION(first_block->data, first_block->size);
#endif
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

    hash = ((~hash) + (hash << 18));
    hash ^= (hash ^ ROTATE_RIGHT(hash, 31));
    hash = (hash * 21);
    hash ^= (hash ^ ROTATE_RIGHT(hash, 11));
    hash += (hash << 6);
    hash ^= ROTATE_RIGHT(hash, 22);

    return hash;

#undef ROTATE_LEFT
#undef ROTATE_RIGHT
}

static SU_ATTRIBUTE_PURE size_t su_stbds_hash(su_fat_ptr_t data) {
    su_string_t s;
    s.s = (char *)data.ptr;
    s.len = data.len;
    return su_stbds_hash_string(s);
}

static void su_argb32_premultiply_alpha(uint32_t *dst, uint32_t *src, size_t count) {
    size_t i = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256i const_0 = _mm256_setzero_si256();
    __m256i const_128_16x16 = _mm256_set1_epi16(128);
    __m256i const_257_16x16 = _mm256_set1_epi16(257);

    __m256i extract_alpha_mask = _mm256_set_epi8(
#define ZERO 0x80
        ZERO, 14, ZERO, 14, ZERO, 14, ZERO, 14,
        ZERO, 6, ZERO, 6, ZERO, 6, ZERO, 6,
        ZERO, 30, ZERO, 30, ZERO, 30, ZERO, 30,
        ZERO, 22, ZERO, 22, ZERO, 22, ZERO, 22
#undef ZERO
    );

    __m256i blend_mask = _mm256_set_epi8(
#define FROM1 0
#define FROM2 0x80
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
        __m256i src_8x32, src_lo_16x16, src_hi_16x16;
        __m256i alpha_lo_16x16, alpha_hi_16x16;
        __m256i result_8x32;

        src_8x32 = _mm256_loadu_si256((__m256i_u *)&src[i]);
        src_lo_16x16 = _mm256_unpacklo_epi8(src_8x32, const_0);
        src_hi_16x16 = _mm256_unpackhi_epi8(src_8x32, const_0);

        alpha_lo_16x16 = _mm256_shuffle_epi8(src_lo_16x16, extract_alpha_mask);
        alpha_hi_16x16 = _mm256_shuffle_epi8(src_hi_16x16, extract_alpha_mask);

        src_lo_16x16 = _mm256_mullo_epi16(src_lo_16x16, alpha_lo_16x16);
        src_hi_16x16 = _mm256_mullo_epi16(src_hi_16x16, alpha_hi_16x16);
        src_lo_16x16 = _mm256_add_epi16(src_lo_16x16, const_128_16x16);
        src_hi_16x16 = _mm256_add_epi16(src_hi_16x16, const_128_16x16);
        src_lo_16x16 = _mm256_mulhi_epu16(src_lo_16x16, const_257_16x16);
        src_hi_16x16 = _mm256_mulhi_epu16(src_hi_16x16, const_257_16x16);

        result_8x32 = _mm256_packus_epi16(src_lo_16x16, src_hi_16x16);
        result_8x32 = _mm256_blendv_epi8(result_8x32, src_8x32, blend_mask);

        _mm256_storeu_si256((__m256i_u *)&dst[i], result_8x32);
    }
#endif /* SU_WITH_SIMD && __AVX2__ */
/* TODO: AVX512, WASM, ARM, ?SSE */

    for ( ; i < count; ++i) {
        uint32_t p = src[i];

        uint32_t a = ((p >> 24) & 0xFF);
        uint32_t r = ((p >> 16) & 0xFF);
        uint32_t g = ((p >> 8) & 0xFF);
        uint32_t b = ((p >> 0) & 0xFF);

        r = (((r * a + 128) * 257) >> 16);
        g = (((g * a + 128) * 257) >> 16);
        b = (((b * a + 128) * 257) >> 16);

        dst[i] = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
    }
}

static void su_bswap32_argb32_premultiply_alpha(uint32_t *dst, uint32_t *src, size_t count) {
    size_t i = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256i bswap32_mask = _mm256_set_epi8(
        28,29,30,31,24,25,26,27,
        20,21,22,23,16,17,18,19,
        12,13,14,15,8, 9, 10,11,
        4, 5, 6, 7, 0, 1, 2, 3
    );

    __m256i const_0 = _mm256_setzero_si256();
    __m256i const_128_16x16 = _mm256_set1_epi16(128);
    __m256i const_257_16x16 = _mm256_set1_epi16(257);

    __m256i extract_alpha_mask = _mm256_set_epi8(
#define ZERO 0x80
        ZERO, 14, ZERO, 14, ZERO, 14, ZERO, 14,
        ZERO, 6, ZERO, 6, ZERO, 6, ZERO, 6,
        ZERO, 30, ZERO, 30, ZERO, 30, ZERO, 30,
        ZERO, 22, ZERO, 22, ZERO, 22, ZERO, 22
#undef ZERO
    );

    __m256i blend_mask = _mm256_set_epi8(
#define FROM1 0
#define FROM2 0x80
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
        __m256i src_8x32, src_lo_16x16, src_hi_16x16;
        __m256i alpha_lo_16x16, alpha_hi_16x16;
        __m256i result_8x32;

        src_8x32 = _mm256_loadu_si256((__m256i_u *)&src[i]);
        src_8x32 = _mm256_shuffle_epi8(src_8x32, bswap32_mask);
        src_lo_16x16 = _mm256_unpacklo_epi8(src_8x32, const_0);
        src_hi_16x16 = _mm256_unpackhi_epi8(src_8x32, const_0);

        alpha_lo_16x16 = _mm256_shuffle_epi8(src_lo_16x16, extract_alpha_mask);
        alpha_hi_16x16 = _mm256_shuffle_epi8(src_hi_16x16, extract_alpha_mask);

        src_lo_16x16 = _mm256_mullo_epi16(src_lo_16x16, alpha_lo_16x16);
        src_hi_16x16 = _mm256_mullo_epi16(src_hi_16x16, alpha_hi_16x16);
        src_lo_16x16 = _mm256_add_epi16(src_lo_16x16, const_128_16x16);
        src_hi_16x16 = _mm256_add_epi16(src_hi_16x16, const_128_16x16);
        src_lo_16x16 = _mm256_mulhi_epu16(src_lo_16x16, const_257_16x16);
        src_hi_16x16 = _mm256_mulhi_epu16(src_hi_16x16, const_257_16x16);

        result_8x32 = _mm256_packus_epi16(src_lo_16x16, src_hi_16x16);
        result_8x32 = _mm256_blendv_epi8(result_8x32, src_8x32, blend_mask);

        _mm256_storeu_si256((__m256i_u *)&dst[i], result_8x32);
    }
#endif /* SU_WITH_SIMD && __AVX2__ */
/* TODO: AVX512, WASM, ARM, ?SSE */

    for ( ; i < count; ++i) {
        uint32_t p = src[i];

        uint32_t a = ((p >> 0) & 0xFF);
        uint32_t r = ((p >> 8) & 0xFF);
        uint32_t g = ((p >> 16) & 0xFF);
        uint32_t b = ((p >> 24) & 0xFF);

        r = (((r * a + 128) * 257) >> 16);
        g = (((g * a + 128) * 257) >> 16);
        b = (((b * a + 128) * 257) >> 16);

        dst[i] = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
    }
}

static void su_abgr32_convert_argb32_premultiply_alpha(uint32_t *dst, uint32_t *src, size_t count) {
    size_t i = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256i abgr32_to_argb32_mask = _mm256_set_epi8(
        31,28,29,30,
        27,24,25,26,
        23,20,21,22,
        19,16,17,18,
        15,12,13,14,
        11,8, 9, 10,
        7, 4, 5, 6,
        3, 0, 1, 2
    );

    __m256i const_0 = _mm256_setzero_si256();
    __m256i const_128_16x16 = _mm256_set1_epi16(128);
    __m256i const_257_16x16 = _mm256_set1_epi16(257);

    __m256i extract_alpha_mask = _mm256_set_epi8(
#define ZERO 0x80
        ZERO, 14, ZERO, 14, ZERO, 14, ZERO, 14,
        ZERO, 6, ZERO, 6, ZERO, 6, ZERO, 6,
        ZERO, 30, ZERO, 30, ZERO, 30, ZERO, 30,
        ZERO, 22, ZERO, 22, ZERO, 22, ZERO, 22
#undef ZERO
    );

    __m256i blend_mask = _mm256_set_epi8(
#define FROM1 0
#define FROM2 0x80
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
        __m256i src_8x32, src_lo_16x16, src_hi_16x16;
        __m256i alpha_lo_16x16, alpha_hi_16x16;
        __m256i result_8x32;

        src_8x32 = _mm256_loadu_si256((__m256i_u *)&src[i]);
        src_lo_16x16 = _mm256_unpacklo_epi8(src_8x32, const_0);
        src_hi_16x16 = _mm256_unpackhi_epi8(src_8x32, const_0);

        alpha_lo_16x16 = _mm256_shuffle_epi8(src_lo_16x16, extract_alpha_mask);
        alpha_hi_16x16 = _mm256_shuffle_epi8(src_hi_16x16, extract_alpha_mask);

        src_lo_16x16 = _mm256_mullo_epi16(src_lo_16x16, alpha_lo_16x16);
        src_hi_16x16 = _mm256_mullo_epi16(src_hi_16x16, alpha_hi_16x16);
        src_lo_16x16 = _mm256_add_epi16(src_lo_16x16, const_128_16x16);
        src_hi_16x16 = _mm256_add_epi16(src_hi_16x16, const_128_16x16);
        src_lo_16x16 = _mm256_mulhi_epu16(src_lo_16x16, const_257_16x16);
        src_hi_16x16 = _mm256_mulhi_epu16(src_hi_16x16, const_257_16x16);

        result_8x32 = _mm256_packus_epi16(src_lo_16x16, src_hi_16x16);
        result_8x32 = _mm256_blendv_epi8(result_8x32, src_8x32, blend_mask);
        result_8x32 = _mm256_shuffle_epi8(result_8x32, abgr32_to_argb32_mask);

        _mm256_storeu_si256((__m256i_u *)&dst[i], result_8x32);
    }
#endif /* SU_WITH_SIMD && __AVX2__ */
/* TODO: AVX512, WASM, ARM, ?SSE */

    for ( ; i < count; ++i) {
        uint32_t p = src[i];

        uint32_t a = ((p >> 24) & 0xFF);
        uint32_t b = ((p >> 16) & 0xFF);
        uint32_t g = ((p >> 8) & 0xFF);
        uint32_t r = ((p >> 0) & 0xFF);

        r = (((r * a + 128) * 257) >> 16);
        g = (((g * a + 128) * 257) >> 16);
        b = (((b * a + 128) * 257) >> 16);

        dst[i] = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
    }
}

static void su_abgr32_convert_argb32(uint32_t *dst, uint32_t *src, size_t count) {
    size_t i = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256i abgr32_to_argb32_mask = _mm256_set_epi8(
        31,28,29,30,
        27,24,25,26,
        23,20,21,22,
        19,16,17,18,
        15,12,13,14,
        11,8, 9, 10,
        7, 4, 5, 6,
        3, 0, 1, 2
    );

    for ( ; (i + 8) <= count; i += 8) {
        __m256i abgr32_8x32 = _mm256_loadu_si256((__m256i_u *)&src[i]);
        __m256i argb32_8x32 = _mm256_shuffle_epi8(abgr32_8x32, abgr32_to_argb32_mask);
        _mm256_storeu_si256((__m256i_u *)&dst[i], argb32_8x32);
    }
#endif /* SU_WITH_SIMD && __AVX2__ */
/* TODO: AVX512, WASM, ARM, ?SSE */

    for ( ; i < count; ++i) {
        uint32_t p = src[i];
        uint32_t a = ((p >> 24) & 0xFF);
        uint32_t b = ((p >> 16) & 0xFF);
        uint32_t g = ((p >> 8) & 0xFF);
        uint32_t r = ((p >> 0) & 0xFF);
        dst[i] = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
    }
}

static void su_argb32_rect_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color, int32_t sx, int32_t sy, uint32_t w, uint32_t h) {
    uint32_t x, y;
    uint32_t sa = ((color >> 24) & 0xFF);
    uint32_t sr = ((color >> 16) & 0xFF);
    uint32_t sg = ((color >> 8) & 0xFF);
    uint32_t sb = ((color >> 0) & 0xFF);
    uint32_t inv_sa = (255 - sa);

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256i const_0 = _mm256_setzero_si256();
    __m256i inv_sa_16x16 = _mm256_set1_epi16((short)inv_sa);
    __m256i src_8x32 = _mm256_set1_epi32((int)color);
    __m256i const_128_16x16 = _mm256_set1_epi16(128);
    __m256i const_257_16x16 = _mm256_set1_epi16(257);
#endif /* SU_WITH_SIMD && __AVX2__ */

    if (sx < 0) {
        uint32_t shift = (uint32_t)(-sx);
        if (shift >= w) {
            return;
        }
        sx = 0;
        w -= shift;
    }
    if (sy < 0) {
        uint32_t shift = (uint32_t)(-sy);
        if (shift >= h) {
            return;
        }
        sy = 0;
        h -= shift;
    }

    if ((sx >= (int32_t)dst_w) || (sy >= (int32_t)dst_h)) {
        return;
    }
    if (w > (dst_w - (uint32_t)sx)) {
        w = (dst_w - (uint32_t)sx);
    }
    if (h > (dst_h - (uint32_t)sy)) {
        h = (dst_h - (uint32_t)sy);
    }

    w += (uint32_t)sx;
    h += (uint32_t)sy;

    if (sa == 0) {
        return;
    } else if (sa == 255) {
        for ( y = (uint32_t)sy; y < h; ++y) {
            uint32_t *row = &dst[y * dst_w];
            x = (uint32_t)sx;
#if SU_WITH_SIMD && defined(__AVX2__)
            for ( ; (x + 8) <= w; x += 8) {
                _mm256_storeu_si256((__m256i_u *)&row[x], src_8x32);
            }
#endif /* SU_WITH_SIMD && __AVX2__ */
            for ( ; x < w; ++x) {
                row[x] = color;
            }
        }
    } else {
        for ( y = (uint32_t)sy; y < h; ++y) {
            uint32_t *row = &dst[y * dst_w];

            x = (uint32_t)sx;

#if SU_WITH_SIMD && defined(__AVX2__)
            for ( ; (x + 8) <= w; x += 8) {
                __m256i dst_8x32, dst_lo_16x16, dst_hi_16x16, result_8x32;

                dst_8x32 = _mm256_loadu_si256((__m256i_u *)&row[x]);
                dst_lo_16x16 = _mm256_unpacklo_epi8(dst_8x32, const_0);
                dst_hi_16x16 = _mm256_unpackhi_epi8(dst_8x32, const_0);

                dst_lo_16x16 = _mm256_mullo_epi16(dst_lo_16x16, inv_sa_16x16);
                dst_hi_16x16 = _mm256_mullo_epi16(dst_hi_16x16, inv_sa_16x16);
                dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, const_128_16x16);
                dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, const_128_16x16);
                dst_lo_16x16 = _mm256_mulhi_epu16(dst_lo_16x16, const_257_16x16);
                dst_hi_16x16 = _mm256_mulhi_epu16(dst_hi_16x16, const_257_16x16);

                result_8x32 = _mm256_packus_epi16(dst_lo_16x16, dst_hi_16x16);
                result_8x32 = _mm256_adds_epu8(result_8x32, src_8x32);

                _mm256_storeu_si256((__m256i_u *)&row[x], result_8x32);
            }
#endif /* SU_WITH_SIMD && __AVX2__ */

            for ( ; x < w; ++x) {
                uint32_t p = row[x];

                uint32_t da = ((p >> 24) & 0xFF);
                uint32_t dr = ((p >> 16) & 0xFF);
                uint32_t dg = ((p >> 8) & 0xFF);
                uint32_t db = ((p >> 0) & 0xFF);

                uint32_t a = (sa + (((da * inv_sa + 128) * 257) >> 16));
                uint32_t r = (sr + (((dr * inv_sa + 128) * 257) >> 16));
                uint32_t g = (sg + (((dg * inv_sa + 128) * 257) >> 16));
                uint32_t b = (sb + (((db * inv_sa + 128) * 257) >> 16));

                row[x] = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
            }
        }
    }
}

static void su_argb32_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t *src, uint32_t src_w, uint32_t src_h,
        int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y,
        uint32_t w, uint32_t h) {
    uint32_t x, y;

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256i const_0  = _mm256_setzero_si256();
    __m256i const_128_16x16 = _mm256_set1_epi16(128);
    __m256i const_257_16x16 = _mm256_set1_epi16(257);
    __m256i const_255_32x8 = _mm256_set1_epi8((char)255);
    __m256i a_mask = _mm256_setr_epi8(
#define ZERO 0x80
        3,  3,  3,  3,  7,  7,  7,  7,
        11, 11, 11, 11, 15, 15, 15, 15,
        19, 19, 19, 19, 23, 23, 23, 23,
        27, 27, 27, 27, 31, 31, 31, 31
#undef ZERO
    );
#endif /* SU_WITH_SIMD && __AVX2__ */

    if (dst_x < 0) {
        uint32_t shift = (uint32_t)(-dst_x);
        if (shift >= w) {
            return;
        }
        dst_x = 0;
        src_x += (int32_t)shift;
        w -= shift;
    }
    if (dst_y < 0) {
        uint32_t shift = (uint32_t)(-dst_y);
        if (shift >= h) {
            return;
        }
        dst_y = 0;
        src_y += (int32_t)shift;
        h -= shift;
    }

    if (src_x < 0) {
        uint32_t shift = (uint32_t)(-src_x);
        if (shift >= w) {
            return;
        }
        src_x = 0;
        dst_x += (int32_t)shift;
        w -= shift;
    }
    if (src_y < 0) {
        uint32_t shift = (uint32_t)(-src_y);
        if (shift >= h) {
            return;
        }
        src_y = 0;
        dst_y += (int32_t)shift;
        h -= shift;
    }

    if ((dst_x >= (int32_t)dst_w) || (dst_y >= (int32_t)dst_h)) {
        return;
    }
    if (w > (dst_w - (uint32_t)dst_x)) {
        w = (dst_w - (uint32_t)dst_x);
    }
    if (h > (dst_h - (uint32_t)dst_y)) {
        h = (dst_h - (uint32_t)dst_y);
    }

    if ((src_x >= (int32_t)src_w) || (src_y >= (int32_t)src_h)) {
        return;
    }
    if (w > (src_w - (uint32_t)src_x)) {
        w = (src_w - (uint32_t)src_x);
    }
    if (h > (src_h - (uint32_t)src_y)) {
        h = (src_h - (uint32_t)src_y);
    }

    for ( y = 0; y < h; ++y) {
        uint32_t *dst_row = &dst[((uint32_t)dst_y + y) * dst_w + (uint32_t)dst_x];
        uint32_t *src_row = &src[((uint32_t)src_y + y) * src_w + (uint32_t)src_x];

        x = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
        for ( ; (x + 8) <= w; x += 8) {
            __m256i src_8x32;
            uint32_t src_amask_8x32;

            src_8x32 = _mm256_loadu_si256((__m256i_u *)&src_row[x]);
            src_amask_8x32 = (uint32_t)_mm256_movemask_epi8(src_8x32);
            if (src_amask_8x32 == 0) {
                continue;
            } else if (src_amask_8x32 == UINT32_MAX) {
                _mm256_storeu_si256((__m256i_u *)&dst_row[x], src_8x32);
            } else {
                __m256i dst_8x32, dst_16x16_lo, dst_16x16_hi;
                __m256i src_a_8x32, inv_src_a_8x32, inv_src_a_16x16_lo, inv_src_a_16x16_hi;
                __m256i result_8x32;

                dst_8x32 = _mm256_loadu_si256((__m256i_u *)&dst_row[x]);
                dst_16x16_lo = _mm256_unpacklo_epi8(dst_8x32, const_0);
                dst_16x16_hi = _mm256_unpackhi_epi8(dst_8x32, const_0);

                src_a_8x32 = _mm256_shuffle_epi8(src_8x32, a_mask);
                inv_src_a_8x32 = _mm256_sub_epi32(const_255_32x8, src_a_8x32);
                inv_src_a_16x16_lo = _mm256_unpacklo_epi8(inv_src_a_8x32, const_0);
                inv_src_a_16x16_hi = _mm256_unpackhi_epi8(inv_src_a_8x32, const_0);

                dst_16x16_lo = _mm256_mullo_epi16(dst_16x16_lo, inv_src_a_16x16_lo);
                dst_16x16_hi = _mm256_mullo_epi16(dst_16x16_hi, inv_src_a_16x16_hi);
                dst_16x16_lo = _mm256_add_epi16(dst_16x16_lo, const_128_16x16);
                dst_16x16_hi = _mm256_add_epi16(dst_16x16_hi, const_128_16x16);
                dst_16x16_lo = _mm256_mulhi_epu16(dst_16x16_lo, const_257_16x16);
                dst_16x16_hi = _mm256_mulhi_epu16(dst_16x16_hi, const_257_16x16);

                result_8x32 = _mm256_packus_epi16(dst_16x16_lo, dst_16x16_hi);
                result_8x32 = _mm256_adds_epu8(result_8x32, src_8x32);

                _mm256_storeu_si256((__m256i_u *)&dst_row[x], result_8x32);
            }
        }
#endif /* SU_WITH_SIMD && __AVX2__ */

        for ( ; x < w; ++x) {
            uint32_t s = src_row[x];

            uint32_t sa = ((s >> 24) & 0xFF);
            if (sa == 0) {
                continue;
            } else if (sa == 255) {
                dst_row[x] = s;
            } else {
                uint32_t sr = ((s >> 16) & 0xFF);
                uint32_t sg = ((s >> 8) & 0xFF);
                uint32_t sb = ((s >> 0) & 0xFF);

                uint32_t inv_sa = (255 - sa);

                uint32_t d = dst_row[x];

                uint32_t da = ((d >> 24) & 0xFF);
                uint32_t dr = ((d >> 16) & 0xFF);
                uint32_t dg = ((d >> 8) & 0xFF);
                uint32_t db = ((d >> 0) & 0xFF);

                da = (sa + (((da * inv_sa + 128) * 257) >> 16));
                dr = (sr + (((dr * inv_sa + 128) * 257) >> 16));
                dg = (sg + (((dg * inv_sa + 128) * 257) >> 16));
                db = (sb + (((db * inv_sa + 128) * 257) >> 16));

                dst_row[x] = ((da << 24) | (dr << 16) | (dg << 8) | (db << 0));
            }
        }
    }
}

static void su_argb32_mask8_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color,
        uint8_t *mask, uint32_t mask_w, uint32_t mask_h, uint32_t mask_pitch,
        int32_t dst_x, int32_t dst_y, int32_t mask_x, int32_t mask_y,
        uint32_t w, uint32_t h) {
    uint32_t x, y;

    uint32_t color_a = ((color >> 24) & 0xFF);
    uint32_t color_r = ((color >> 16) & 0xFF);
    uint32_t color_g = ((color >> 8) & 0xFF);
    uint32_t color_b = ((color >> 0) & 0xFF);

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256i const_0 = _mm256_setzero_si256();
    __m256i const_128_16x16 = _mm256_set1_epi16(128);
    __m256i const_257_16x16 = _mm256_set1_epi16(257);
    __m256i const_255_16x16 = _mm256_set1_epi16(255);

    __m256i src_8x32 = _mm256_set1_epi32((int)color);
    __m256i src_lo_16x16_ = _mm256_unpacklo_epi8(src_8x32, const_0);
    __m256i src_hi_16x16_ = _mm256_unpackhi_epi8(src_8x32, const_0);

    __m256i src_a_16x16_ = _mm256_set1_epi16((int16_t)color_a);

    __m256i mask_mask = _mm256_setr_epi8(
        0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
        4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7);
#endif /* SU_WITH_SIMD && defined(__AVX2__) */

    if (dst_x < 0) {
        uint32_t shift = (uint32_t)(-dst_x);
        if (shift >= w) {
            return;
        }
        dst_x = 0;
        mask_x += (int32_t)shift;
        w -= shift;
    }
    if (dst_y < 0) {
        uint32_t shift = (uint32_t)(-dst_y);
        if (shift >= h) {
            return;
        }
        dst_y = 0;
        mask_y += (int32_t)shift;
        h -= shift;
    }

    if (mask_x < 0) {
        uint32_t shift = (uint32_t)(-mask_x);
        if (shift >= w) {
            return;
        }
        mask_x = 0;
        dst_x += (int32_t)shift;
        w -= shift;
    }
    if (mask_y < 0) {
        uint32_t shift = (uint32_t)(-mask_y);
        if (shift >= h) {
            return;
        }
        mask_y = 0;
        dst_y += (int32_t)shift;
        h -= shift;
    }

    if ((dst_x >= (int32_t)dst_w) || (dst_y >= (int32_t)dst_h)) {
        return;
    }
    if (w > (dst_w - (uint32_t)dst_x)) {
        w = (dst_w - (uint32_t)dst_x);
    }
    if (h > (dst_h - (uint32_t)dst_y)) {
        h = (dst_h - (uint32_t)dst_y);
    }

    if ((mask_x >= (int32_t)mask_w) || (mask_y >= (int32_t)mask_h)) {
        return;
    }
    if (w > (mask_w - (uint32_t)mask_x)) {
        w = (mask_w - (uint32_t)mask_x);
    }
    if (h > (mask_h - (uint32_t)mask_y)) {
        h = (mask_h - (uint32_t)mask_y);
    }

    if (color_a == 0) {
        return;
    } else if (color_a == 255) {
        for ( y = 0; y < h; ++y) {
            uint32_t *dst_row = &dst[((uint32_t)dst_y + y) * dst_w + (uint32_t)dst_x];
            uint8_t *mask_row = &mask[((uint32_t)mask_y + y) * mask_pitch + (uint32_t)mask_x];

            x = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
            for ( ; (x + 8) <= w; x += 8) {
                __m256i dst_8x32, dst_lo_16x16, dst_hi_16x16;
                __m256i src_lo_16x16, src_hi_16x16;
                __m256i inv_mask_lo_16x16, inv_mask_hi_16x16;
                __m256i mask_4x64, mask_8x32, mask_lo_16x16, mask_hi_16x16;
                __m256i result_lo_16x16, result_hi_16x16, result_8x32;

                uint64_t m;
                SU_MEMCPY(&m, &mask_row[x], sizeof(m));

                if (m == 0) {
                    continue;
                } else if (m == UINT64_MAX) {
                    _mm256_storeu_si256((__m256i_u *)&dst_row[x], src_8x32);
                    continue;
                }

                mask_4x64 = _mm256_setr_epi64x((int64_t)m, 0, (int64_t)m, 0);
                mask_8x32 = _mm256_shuffle_epi8(mask_4x64, mask_mask);

                mask_lo_16x16 = _mm256_unpacklo_epi8(mask_8x32, const_0);
                mask_hi_16x16 = _mm256_unpackhi_epi8(mask_8x32, const_0);

                src_lo_16x16 = _mm256_mullo_epi16(src_lo_16x16_, mask_lo_16x16);
                src_hi_16x16 = _mm256_mullo_epi16(src_hi_16x16_, mask_hi_16x16);
                src_lo_16x16 = _mm256_add_epi16(src_lo_16x16, const_128_16x16);
                src_hi_16x16 = _mm256_add_epi16(src_hi_16x16, const_128_16x16);
                src_lo_16x16 = _mm256_mulhi_epu16(src_lo_16x16, const_257_16x16);
                src_hi_16x16 = _mm256_mulhi_epu16(src_hi_16x16, const_257_16x16);

                inv_mask_lo_16x16 = _mm256_subs_epu16(const_255_16x16, mask_lo_16x16);
                inv_mask_hi_16x16 = _mm256_subs_epu16(const_255_16x16, mask_hi_16x16);

                dst_8x32 = _mm256_loadu_si256((__m256i_u *)&dst_row[x]);
                dst_lo_16x16  = _mm256_unpacklo_epi8(dst_8x32, const_0);
                dst_hi_16x16  = _mm256_unpackhi_epi8(dst_8x32, const_0);

                dst_lo_16x16 = _mm256_mullo_epi16(dst_lo_16x16, inv_mask_lo_16x16);
                dst_hi_16x16 = _mm256_mullo_epi16(dst_hi_16x16, inv_mask_hi_16x16);
                dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, const_128_16x16);
                dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, const_128_16x16);
                dst_lo_16x16 = _mm256_mulhi_epu16(dst_lo_16x16, const_257_16x16);
                dst_hi_16x16 = _mm256_mulhi_epu16(dst_hi_16x16, const_257_16x16);

                result_lo_16x16 = _mm256_adds_epu16(src_lo_16x16, dst_lo_16x16);
                result_hi_16x16 = _mm256_adds_epu16(src_hi_16x16, dst_hi_16x16);
                result_8x32 = _mm256_packus_epi16(result_lo_16x16, result_hi_16x16);

                _mm256_storeu_si256((__m256i_u *)&dst_row[x], result_8x32);
            }
#endif /* SU_WITH_SIMD && defined(__AVX2__) */

            for ( ; x < w; ++x) {
                uint32_t m = mask_row[x];
                uint32_t sa, sr, sg, sb;
                uint32_t d, da, dr, dg, db;
                uint32_t inv_sa;
                uint32_t a, r, g, b;

                if (m == 0) {
                    continue;
                } else if (m == 255) {
                    dst_row[x] = color;
                    continue;
                }

                sa = m;
                sr = (((color_r * m + 128) * 257) >> 16);
                sg = (((color_g * m + 128) * 257) >> 16);
                sb = (((color_b * m + 128) * 257) >> 16);

                inv_sa = (255 - sa);

                d = dst_row[x];
                da = ((d >> 24) & 0xFF);
                dr = ((d >> 16) & 0xFF);
                dg = ((d >> 8) & 0xFF);
                db = ((d >> 0) & 0xFF);

                a = (sa + (((da * inv_sa + 128) * 257) >> 16));
                r = (sr + (((dr * inv_sa + 128) * 257) >> 16));
                g = (sg + (((dg * inv_sa + 128) * 257) >> 16));
                b = (sb + (((db * inv_sa + 128) * 257) >> 16));

                dst_row[x] = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
            }
        }
    } else {
        for ( y = 0; y < h; ++y) {
            uint32_t *dst_row = &dst[((uint32_t)dst_y + y) * dst_w + (uint32_t)dst_x];
            uint8_t *mask_row = &mask[((uint32_t)mask_y + y) * mask_pitch + (uint32_t)mask_x];

            x = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
            for ( ; (x + 8) <= w; x += 8) {
                __m256i dst_8x32, dst_lo_16x16, dst_hi_16x16;
                __m256i src_lo_16x16, src_hi_16x16, src_a_lo_16x16, src_a_hi_16x16;
                __m256i inv_src_a_lo_16x16, inv_src_a_hi_16x16;
                __m256i result_lo_16x16, result_hi_16x16, result_8x32;

                uint64_t m;
                SU_MEMCPY(&m, &mask_row[x], sizeof(m));

                if (m == 0) {
                    continue;
                } else if (m == UINT64_MAX) {
                    src_lo_16x16 = src_lo_16x16_;
                    src_hi_16x16 = src_hi_16x16_;
                    src_a_lo_16x16 = src_a_16x16_;
                    src_a_hi_16x16 = src_a_16x16_;
                } else {
                    __m256i mask_4x64, mask_8x32, mask_lo_16x16, mask_hi_16x16;

                    mask_4x64 = _mm256_setr_epi64x((int64_t)m, 0, (int64_t)m, 0);
                    mask_8x32 = _mm256_shuffle_epi8(mask_4x64, mask_mask);

                    mask_lo_16x16 = _mm256_unpacklo_epi8(mask_8x32, const_0);
                    mask_hi_16x16 = _mm256_unpackhi_epi8(mask_8x32, const_0);

                    src_lo_16x16 = _mm256_mullo_epi16(src_lo_16x16_, mask_lo_16x16);
                    src_hi_16x16 = _mm256_mullo_epi16(src_hi_16x16_, mask_hi_16x16);
                    src_lo_16x16 = _mm256_add_epi16(src_lo_16x16, const_128_16x16);
                    src_hi_16x16 = _mm256_add_epi16(src_hi_16x16, const_128_16x16);
                    src_lo_16x16 = _mm256_mulhi_epu16(src_lo_16x16, const_257_16x16);
                    src_hi_16x16 = _mm256_mulhi_epu16(src_hi_16x16, const_257_16x16);

                    /* TODO: optimize */
                    src_a_lo_16x16 = _mm256_mullo_epi16(src_a_16x16_, mask_lo_16x16);
                    src_a_hi_16x16 = _mm256_mullo_epi16(src_a_16x16_, mask_hi_16x16);
                    src_a_lo_16x16 = _mm256_add_epi16(src_a_lo_16x16, const_128_16x16);
                    src_a_hi_16x16 = _mm256_add_epi16(src_a_hi_16x16, const_128_16x16);
                    src_a_lo_16x16 = _mm256_mulhi_epu16(src_a_lo_16x16, const_257_16x16);
                    src_a_hi_16x16 = _mm256_mulhi_epu16(src_a_hi_16x16, const_257_16x16);
                }

                inv_src_a_lo_16x16 = _mm256_subs_epu16(const_255_16x16, src_a_lo_16x16);
                inv_src_a_hi_16x16 = _mm256_subs_epu16(const_255_16x16, src_a_hi_16x16);

                dst_8x32 = _mm256_loadu_si256((__m256i_u *)&dst_row[x]);
                dst_lo_16x16  = _mm256_unpacklo_epi8(dst_8x32, const_0);
                dst_hi_16x16  = _mm256_unpackhi_epi8(dst_8x32, const_0);

                dst_lo_16x16 = _mm256_mullo_epi16(dst_lo_16x16, inv_src_a_lo_16x16);
                dst_hi_16x16 = _mm256_mullo_epi16(dst_hi_16x16, inv_src_a_hi_16x16);
                dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, const_128_16x16);
                dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, const_128_16x16);
                dst_lo_16x16 = _mm256_mulhi_epu16(dst_lo_16x16, const_257_16x16);
                dst_hi_16x16 = _mm256_mulhi_epu16(dst_hi_16x16, const_257_16x16);

                result_lo_16x16 = _mm256_adds_epu16(src_lo_16x16, dst_lo_16x16);
                result_hi_16x16 = _mm256_adds_epu16(src_hi_16x16, dst_hi_16x16);
                result_8x32 = _mm256_packus_epi16(result_lo_16x16, result_hi_16x16);

                _mm256_storeu_si256((__m256i_u *)&dst_row[x], result_8x32);
            }
#endif /* SU_WITH_SIMD && defined(__AVX2__) */

            for ( ; x < w; ++x) {
                uint32_t m = mask_row[x];
                uint32_t sa, sr, sg, sb;
                uint32_t d, da, dr, dg, db;
                uint32_t inv_sa;
                uint32_t a, r, g, b;

                if (m == 0) {
                    continue;
                } else if (m == 255) {
                    sa = color_a;
                    sr = color_r;
                    sg = color_g;
                    sb = color_b;
                } else {
                    sa = (((color_a * m + 128) * 257) >> 16);
                    sr = (((color_r * m + 128) * 257) >> 16);
                    sg = (((color_g * m + 128) * 257) >> 16);
                    sb = (((color_b * m + 128) * 257) >> 16);
                }

                inv_sa = (255 - sa);

                d = dst_row[x];
                da = ((d >> 24) & 0xFF);
                dr = ((d >> 16) & 0xFF);
                dg = ((d >> 8) & 0xFF);
                db = ((d >> 0) & 0xFF);

                a = (sa + (((da * inv_sa + 128) * 257) >> 16));
                r = (sr + (((dr * inv_sa + 128) * 257) >> 16));
                g = (sg + (((dg * inv_sa + 128) * 257) >> 16));
                b = (sb + (((db * inv_sa + 128) * 257) >> 16));

                dst_row[x] = ((a << 24) | (r << 16) | (g << 8) | (b << 0));
            }
        }
    }
}

static void su_argb32_mask1_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color,
        uint8_t *mask, uint32_t mask_w, uint32_t mask_h, uint32_t mask_pitch,
        int32_t dst_x, int32_t dst_y, int32_t mask_x, int32_t mask_y,
        uint32_t w, uint32_t h) {
    uint32_t x, y;
    uint32_t sa = ((color >> 24) & 0xFF);
    uint32_t sr = ((color >> 16) & 0xFF);
    uint32_t sg = ((color >> 8) & 0xFF);
    uint32_t sb = ((color >> 0) & 0xFF);
    uint32_t inv_sa = (255 - sa);
    uint32_t start_bit = (mask_x & 7);

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256i src_8x32 = _mm256_set1_epi32((int)color);
    __m256i const_0 = _mm256_setzero_si256();
    __m256i const_1_8x32 = _mm256_set1_epi32(1);
    __m256i const_128_16x16 = _mm256_set1_epi16(128);
    __m256i const_257_16x16 = _mm256_set1_epi16(257);
    __m256i const_255_16x16 = _mm256_set1_epi16(255);
    __m256i src_a_16x16 = _mm256_set1_epi16((short)sa);
    __m256i inv_src_a_16x16 = _mm256_subs_epu16(const_255_16x16, src_a_16x16);
    __m256i shift_8x32 = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
#endif /* SU_WITH_SIMD && __AVX2__ */

    if (sa == 0) {
        return;
    }

    if (dst_x < 0) {
        uint32_t shift = (uint32_t)(-dst_x);
        if (shift >= w) {
            return;
        }
        dst_x = 0;
        mask_x += (int32_t)shift;
        w -= shift;
    }
    if (dst_y < 0) {
        uint32_t shift = (uint32_t)(-dst_y);
        if (shift >= h) {
            return;
        }
        dst_y = 0;
        mask_y += (int32_t)shift;
        h -= shift;
    }

    if (mask_x < 0) {
        uint32_t shift = (uint32_t)(-mask_x);
        if (shift >= w) {
            return;
        }
        mask_x = 0;
        dst_x += (int32_t)shift;
        w -= shift;
    }
    if (mask_y < 0) {
        uint32_t shift = (uint32_t)(-mask_y);
        if (shift >= h) {
            return;
        }
        mask_y = 0;
        dst_y += (int32_t)shift;
        h -= shift;
    }

    if ((dst_x >= (int32_t)dst_w) || (dst_y >= (int32_t)dst_h)) {
        return;
    }
    if (w > (dst_w - (uint32_t)dst_x)) {
        w = (dst_w - (uint32_t)dst_x);
    }
    if (h > (dst_h - (uint32_t)dst_y)) {
        h = (dst_h - (uint32_t)dst_y);
    }

    if ((mask_x >= (int32_t)mask_w) || (mask_y >= (int32_t)mask_h)) {
        return;
    }
    if (w > (mask_w - (uint32_t)mask_x)) {
        w = (mask_w - (uint32_t)mask_x);
    }
    if (h > (mask_h - (uint32_t)mask_y)) {
        h = (mask_h - (uint32_t)mask_y);
    }

    for ( y = 0; y < h; ++y) {
        uint32_t *dst_row = &dst[((uint32_t)dst_y + y) * dst_w + (uint32_t)dst_x];
        uint8_t *mask_row = &mask[((uint32_t)mask_y + y) * mask_pitch];

        x = 0;

        for ( ; ((start_bit + x) & 7) && (x < w); ++x) {
            uint32_t p = ((uint32_t)mask_x + x);
            if (mask_row[p >> 3] & (0x80 >> (p & 7))) {
                uint32_t d = dst_row[x];

                uint32_t da = ((d >> 24) & 0xFF);
                uint32_t dr = ((d >> 16) & 0xFF);
                uint32_t dg = ((d >> 8) & 0xFF);
                uint32_t db = ((d >> 0) & 0xFF);

                da = (sa + (((da * inv_sa + 128) * 257) >> 16));
                dr = (sr + (((dr * inv_sa + 128) * 257) >> 16));
                dg = (sg + (((dg * inv_sa + 128) * 257) >> 16));
                db = (sb + (((db * inv_sa + 128) * 257) >> 16));

                dst_row[x] = ((da << 24) | (dr << 16) | (dg << 8) | (db << 0));
            }
        }

#if SU_WITH_SIMD && defined(__AVX2__)
        for ( ; (x + 8) <= w; x += 8) {
            uint8_t m = mask_row[((uint32_t)mask_x + x) >> 3];
            __m256i m_8x32;
            __m256i dst_8x32, dst_lo_16x16, dst_hi_16x16;
            __m256i result_8x32;

            m_8x32 = _mm256_set1_epi32((int)m);
            m_8x32 = _mm256_srlv_epi32(m_8x32, shift_8x32);
            m_8x32 = _mm256_and_si256(m_8x32, const_1_8x32);
            m_8x32 = _mm256_sub_epi32(const_0, m_8x32);

            dst_8x32 = _mm256_loadu_si256((__m256i_u *)&dst_row[x]);

            dst_lo_16x16 = _mm256_unpacklo_epi8(dst_8x32, const_0);
            dst_hi_16x16 = _mm256_unpackhi_epi8(dst_8x32, const_0);

            dst_lo_16x16 = _mm256_mullo_epi16(dst_lo_16x16, inv_src_a_16x16);
            dst_hi_16x16 = _mm256_mullo_epi16(dst_hi_16x16, inv_src_a_16x16);
            dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, const_128_16x16);
            dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, const_128_16x16);
            dst_lo_16x16 = _mm256_mulhi_epu16(dst_lo_16x16, const_257_16x16);
            dst_hi_16x16 = _mm256_mulhi_epu16(dst_hi_16x16, const_257_16x16);

            result_8x32 = _mm256_packus_epi16(dst_lo_16x16, dst_hi_16x16);
            result_8x32 = _mm256_adds_epu8(src_8x32, result_8x32);
            result_8x32 = _mm256_blendv_epi8(dst_8x32, result_8x32, m_8x32);

            _mm256_storeu_si256((__m256i_u *)&dst_row[x], result_8x32);
        }
#endif /* SU_WITH_SIMD && __AVX2__ */

        for ( ; x < w; ++x) {
            uint32_t p = ((uint32_t)mask_x + x);
            if (mask_row[p >> 3] & (0x80 >> (p & 7))) {
                uint32_t d = dst_row[x];

                uint32_t da = ((d >> 24) & 0xFF);
                uint32_t dr = ((d >> 16) & 0xFF);
                uint32_t dg = ((d >> 8) & 0xFF);
                uint32_t db = ((d >> 0) & 0xFF);

                da = (sa + (((da * inv_sa + 128) * 257) >> 16));
                dr = (sr + (((dr * inv_sa + 128) * 257) >> 16));
                dg = (sg + (((dg * inv_sa + 128) * 257) >> 16));
                db = (sb + (((db * inv_sa + 128) * 257) >> 16));

                dst_row[x] = ((da << 24) | (dr << 16) | (dg << 8) | (db << 0));
            }
        }
    }
}

static void su_argb32_mask24_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color,
        uint8_t *mask, uint32_t mask_w, uint32_t mask_h, uint32_t mask_pitch,
        int32_t dst_x, int32_t dst_y, int32_t mask_x, int32_t mask_y,
        uint32_t w, uint32_t h) {
    uint32_t x, y;

    uint32_t color_a = ((color >> 24) & 0xFF);
    uint32_t color_r = ((color >> 16) & 0xFF);
    uint32_t color_g = ((color >> 8) & 0xFF);
    uint32_t color_b = ((color >> 0) & 0xFF);

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256i const_0 = _mm256_setzero_si256();
    __m256i const_128_16x16 = _mm256_set1_epi16(128);
    __m256i const_255_16x16 = _mm256_set1_epi16(255);
    __m256i src_8x32 = _mm256_set1_epi32((int)color);
    __m256i src_lo_16x16_ = _mm256_unpacklo_epi8(src_8x32, const_0);
    __m256i src_hi_16x16_ = _mm256_unpackhi_epi8(src_8x32, const_0);
    __m128i raw_mask = _mm_setr_epi8(
#define ZERO 0x80
        2, 1, 0, ZERO, 5, 4, 3, ZERO,
        8, 7, 6, ZERO, 11, 10, 9, ZERO
    );
    __m256i a_mask = _mm256_setr_epi8(
        6, ZERO, 6, ZERO, 6, ZERO, 6, ZERO,
        14, ZERO, 14, ZERO, 14, ZERO, 14, ZERO,
        22, ZERO, 22, ZERO, 22, ZERO, 22, ZERO,
        30, ZERO, 30, ZERO, 30, ZERO, 30, ZERO
#undef ZERO
    );
    __m256i blend_mask = _mm256_setr_epi8(
#define FROM1 0
#define FROM2 0x80
        FROM1, FROM1, FROM1, FROM2,
        FROM1, FROM1, FROM1, FROM2,
        FROM1, FROM1, FROM1, FROM2,
        FROM1, FROM1, FROM1, FROM2,
        FROM1, FROM1, FROM1, FROM2,
        FROM1, FROM1, FROM1, FROM2,
        FROM1, FROM1, FROM1, FROM2,
        FROM1, FROM1, FROM1, FROM2
#undef FROM1
#undef FROM2
    );
#endif /* SU_WITH_SIMD && defined(__AVX2__) */

    if (color_a == 0) {
        return;
    }

    if (dst_x < 0) {
        uint32_t shift = (uint32_t)(-dst_x);
        if (shift >= w) {
            return;
        }
        dst_x = 0;
        mask_x += (int32_t)shift;
        w -= shift;
    }
    if (dst_y < 0) {
        uint32_t shift = (uint32_t)(-dst_y);
        if (shift >= h) {
            return;
        }
        dst_y = 0;
        mask_y += (int32_t)shift;
        h -= shift;
    }

    if (mask_x < 0) {
        uint32_t shift = (uint32_t)(-mask_x);
        if (shift >= w) {
            return;
        }
        mask_x = 0;
        dst_x += (int32_t)shift;
        w -= shift;
    }
    if (mask_y < 0) {
        uint32_t shift = (uint32_t)(-mask_y);
        if (shift >= h) {
            return;
        }
        mask_y = 0;
        dst_y += (int32_t)shift;
        h -= shift;
    }

    if ((dst_x >= (int32_t)dst_w) || (dst_y >= (int32_t)dst_h)) {
        return;
    }
    if (w > (dst_w - (uint32_t)dst_x)) {
        w = (dst_w - (uint32_t)dst_x);
    }
    if (h > (dst_h - (uint32_t)dst_y)) {
        h = (dst_h - (uint32_t)dst_y);
    }

    if ((mask_x >= (int32_t)mask_w) || (mask_y >= (int32_t)mask_h)) {
        return;
    }
    if (w > (mask_w - (uint32_t)mask_x)) {
        w = (mask_w - (uint32_t)mask_x);
    }
    if (h > (mask_h - (uint32_t)mask_y)) {
        h = (mask_h - (uint32_t)mask_y);
    }

    for ( y = 0; y < h; ++y) {
        uint32_t *dst_row = &dst[((uint32_t)dst_y + y) * dst_w + (uint32_t)dst_x];
        uint8_t *mask_row = &mask[((uint32_t)mask_y + y) * mask_pitch + (uint32_t)mask_x * 3];

        x = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
        for ( ; (x + 8) <= w; x += 8) {
            __m256i dst_8x32, dst_lo_16x16, dst_hi_16x16;
            __m128i mask_1_raw, mask_2_raw;
            __m256i mask_8x32, mask_lo_16x16, mask_hi_16x16;
            __m256i src_lo_16x16, src_hi_16x16;
            __m256i a_8x32, r_8x32, g_8x32, b_8x32;
            __m256i a_lo_16x16, a_hi_16x16, inv_a_lo_16x16, inv_a_hi_16x16;
            __m256i result_8x32;

            dst_8x32 = _mm256_loadu_si256((__m256i_u *)&dst_row[x]);
            dst_lo_16x16 = _mm256_unpacklo_epi8(dst_8x32, const_0);
            dst_hi_16x16 = _mm256_unpackhi_epi8(dst_8x32, const_0);

            mask_1_raw = _mm_loadu_si128((__m128i_u *)&mask_row[x * 3]);
            mask_2_raw = _mm_loadu_si128((__m128i_u *)&mask_row[x * 3 + 12]);
            mask_1_raw = _mm_shuffle_epi8(mask_1_raw, raw_mask);
            mask_2_raw = _mm_shuffle_epi8(mask_2_raw, raw_mask);
            mask_8x32 = _mm256_setr_m128i(mask_1_raw, mask_2_raw);

            r_8x32 = _mm256_slli_epi32(mask_8x32, 8);
            g_8x32 = _mm256_slli_epi32(mask_8x32, 16);
            b_8x32 = _mm256_slli_epi32(mask_8x32, 24);
            a_8x32 = _mm256_max_epu8(r_8x32, _mm256_max_epu8(g_8x32, b_8x32));
            mask_8x32 = _mm256_blendv_epi8(mask_8x32, a_8x32, blend_mask);

            mask_lo_16x16 = _mm256_unpacklo_epi8(mask_8x32, const_0);
            mask_hi_16x16 = _mm256_unpackhi_epi8(mask_8x32, const_0);

            src_lo_16x16 = _mm256_mullo_epi16(src_lo_16x16_, mask_lo_16x16);
            src_hi_16x16 = _mm256_mullo_epi16(src_hi_16x16_, mask_hi_16x16);
            src_lo_16x16 = _mm256_add_epi16(src_lo_16x16, const_128_16x16);
            src_hi_16x16 = _mm256_add_epi16(src_hi_16x16, const_128_16x16);
            src_lo_16x16 = _mm256_srli_epi16(src_lo_16x16, 8);
            src_hi_16x16 = _mm256_srli_epi16(src_hi_16x16, 8);

            a_lo_16x16 = _mm256_shuffle_epi8(src_lo_16x16, a_mask);
            a_hi_16x16 = _mm256_shuffle_epi8(src_hi_16x16, a_mask);
            inv_a_lo_16x16 = _mm256_subs_epu16(const_255_16x16, a_lo_16x16);
            inv_a_hi_16x16 = _mm256_subs_epu16(const_255_16x16, a_hi_16x16);

            dst_lo_16x16 = _mm256_mullo_epi16(dst_lo_16x16, inv_a_lo_16x16);
            dst_hi_16x16 = _mm256_mullo_epi16(dst_hi_16x16, inv_a_hi_16x16);
            dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, const_128_16x16);
            dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, const_128_16x16);
            dst_lo_16x16 = _mm256_srli_epi16(dst_lo_16x16, 8);
            dst_hi_16x16 = _mm256_srli_epi16(dst_hi_16x16, 8);
            dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, src_lo_16x16);
            dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, src_hi_16x16);

            result_8x32 = _mm256_packus_epi16(dst_lo_16x16, dst_hi_16x16);
            _mm256_storeu_si256((__m256i_u *)&dst_row[x], result_8x32);
        }
#endif /* SU_WITH_SIMD && defined(__AVX2__) */

        for ( ; x < w; ++x) {
            uint32_t d = dst_row[x];
            uint32_t da = ((d >> 24) & 0xFF);
            uint32_t dr = ((d >> 16) & 0xFF);
            uint32_t dg = ((d >> 8) & 0xFF);
            uint32_t db = ((d >> 0) & 0xFF);

            uint32_t mr = mask_row[x * 3 + 0];
            uint32_t mg = mask_row[x * 3 + 1];
            uint32_t mb = mask_row[x * 3 + 2];

            uint32_t sa = ((color_a * SU_MAX(SU_MAX(mr, mg), mb) + 128) >> 8);
            uint32_t sr = ((color_r * mr + 128) >> 8);
            uint32_t sg = ((color_g * mg + 128) >> 8);
            uint32_t sb = ((color_b * mb + 128) >> 8);

            uint32_t inv_sa = (255 - sa);

            da = (sa + ((da * inv_sa + 128) >> 8));
            dr = (sr + ((dr * inv_sa + 128) >> 8));
            dg = (sg + ((dg * inv_sa + 128) >> 8));
            db = (sb + ((db * inv_sa + 128) >> 8));

            dst_row[x] = ((da << 24) | (dr << 16) | (dg << 8) | (db << 0));
        }
    }
}

static void su_argb32_bilinear_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t *src, uint32_t src_w, uint32_t src_h,
        int32_t dst_x, int32_t dst_y,
        uint32_t w, uint32_t h) {
    /* ? TODO: src_x/y, fast paths (a == 0/255) */
    int32_t x, y;

    float inv_scale = (1.f / (SU_MIN((float)w / (float)src_w, (float)h / (float)src_h)));

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256 inv_scale_8x32 = _mm256_set1_ps(inv_scale);
    __m256i const_1_8x32 = _mm256_set1_epi32(1);
    __m256i src_w_minus_1_8x32 = _mm256_set1_epi32((int)src_w - 1);
    __m256 const_1f_8x32 = _mm256_set1_ps(1.f);
    __m256i const_0 = _mm256_setzero_si256();
    __m256i const_255_32x8 = _mm256_set1_epi8((char)255);
    __m256i const_128_16x16 = _mm256_set1_epi16(128);
    __m256i const_257_16x16 = _mm256_set1_epi16(257);
    __m256i w_mask0 = _mm256_setr_epi32(0, 0, 0, 0, 1, 1, 1, 1);
    __m256i w_mask1 = _mm256_setr_epi32(2, 2, 2, 2, 3, 3, 3, 3);
    __m256i w_mask2 = _mm256_setr_epi32(4, 4, 4, 4, 5, 5, 5, 5);
    __m256i w_mask3 = _mm256_setr_epi32(6, 6, 6, 6, 7, 7, 7, 7);
    __m256i a_mask = _mm256_setr_epi8(
        3,  3,  3,  3,  7,  7,  7,  7,
        11, 11, 11, 11, 15, 15, 15, 15,
        19, 19, 19, 19, 23, 23, 23, 23,
        27, 27, 27, 27, 31, 31, 31, 31);
    __m256i s_mask = _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7);
    __m256 wy_8x32;
#endif /* SU_WITH_SIMD && defined(__AVX2__) */

    int32_t start_x = 0;
    int32_t start_y = 0;
    int32_t end_x = (int32_t)w;
    int32_t end_y = (int32_t)h;

    if (dst_x < 0) {
        start_x = -dst_x;
    }
    if (dst_y < 0) {
        start_y = -dst_y;
    }
    if ((dst_x + end_x) > (int32_t)dst_w) {
        end_x = ((int32_t)dst_w - dst_x);
    }
    if ((dst_y + end_y) > (int32_t)dst_h) {
        end_y = ((int32_t)dst_h - dst_y);
    }

    if (start_x >= end_x /* || (start_y >= end_y) */) {
        return;
    }

    for ( y = start_y; y < end_y; ++y) {
        uint32_t *dst_row = &dst[(dst_y + (int32_t)y) * (int32_t)dst_w + dst_x];
        uint32_t *row0, *row1;
        float fy = ((float)y * inv_scale);
        int32_t y0 = SU_MIN((int32_t)fy, (int32_t)src_h - 1);
        float wy = (fy - (float)y0);
        int32_t y1 = SU_MIN(y0 + 1, (int32_t)src_h - 1);
        row0 = &src[y0 * (int32_t)src_w];
        row1 = &src[y1 * (int32_t)src_w];

        x = start_x;

#if SU_WITH_SIMD && defined(__AVX2__)
        wy_8x32 = _mm256_set1_ps(wy);
        for ( ; (x + 8) <= end_x; x += 8) {
            __m256i src_8x32, src_0123_16x16, src_4567_16x16;
            __m256i ix0_8x32, ix1_8x32;
            __m256 fx0_8x32, wx_8x32;
            __m256 w00_8x32, w00_01_8x32, w00_23_8x32, w00_45_8x32, w00_67_8x32;
            __m256 w10_8x32, w10_01_8x32, w10_23_8x32, w10_45_8x32, w10_67_8x32;
            __m256 w01_8x32, w01_01_8x32, w01_23_8x32, w01_45_8x32, w01_67_8x32;
            __m256 w11_8x32, w11_01_8x32, w11_23_8x32, w11_45_8x32, w11_67_8x32;
            __m256i c00_8x32, c10_8x32, c01_8x32, c11_8x32;
            __m128i c00_lo_4x32, c00_hi_4x32, c10_lo_4x32, c10_hi_4x32;
            __m128i c01_lo_4x32, c01_hi_4x32, c11_lo_4x32, c11_hi_4x32;
            __m256 c00_01_8x32, c00_23_8x32, c00_45_8x32, c00_67_8x32;
            __m256 c10_01_8x32, c10_23_8x32, c10_45_8x32, c10_67_8x32;
            __m256 c01_01_8x32, c01_23_8x32, c01_45_8x32, c01_67_8x32;
            __m256 c11_01_8x32, c11_23_8x32, c11_45_8x32, c11_67_8x32;
            __m256 cw00_01_8x32, cw00_23_8x32, cw00_45_8x32, cw00_67_8x32;
            __m256 cw10_01_8x32, cw10_23_8x32, cw10_45_8x32, cw10_67_8x32;
            __m256 cw01_01_8x32, cw01_23_8x32, cw01_45_8x32, cw01_67_8x32;
            __m256 cw11_01_8x32, cw11_23_8x32, cw11_45_8x32, cw11_67_8x32;
            __m256 cw0010_01_8x32, cw0010_23_8x32, cw0010_45_8x32, cw0010_67_8x32;
            __m256 cw0111_01_8x32, cw0111_23_8x32, cw0111_45_8x32, cw0111_67_8x32;
            __m256 cw00100111_01_8x32, cw00100111_23_8x32, cw00100111_45_8x32, cw00100111_67_8x32;
            __m256i src_01_8x32, src_23_8x32, src_45_8x32, src_67_8x32;
            __m256i sa_8x32;
            __m256i inv_sa_8x32, inv_sa_lo_16x16, inv_sa_hi_16x16;
            __m256i src_lo_16x16, src_hi_16x16;
            __m256i dst_8x32, dst_lo_16x16, dst_hi_16x16;
            __m256i result_8x32;

            ix0_8x32 = _mm256_setr_epi32(
                x + 0, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
            fx0_8x32 =_mm256_cvtepi32_ps(ix0_8x32);
            fx0_8x32 = _mm256_mul_ps(fx0_8x32, inv_scale_8x32);
            ix0_8x32 = _mm256_cvttps_epi32(fx0_8x32);
            ix0_8x32 = _mm256_min_epi32(ix0_8x32, src_w_minus_1_8x32);
            ix1_8x32 = _mm256_add_epi32(ix0_8x32, const_1_8x32);
            ix1_8x32 = _mm256_min_epi32(ix1_8x32, src_w_minus_1_8x32);

            wx_8x32 = _mm256_sub_ps(fx0_8x32, _mm256_cvtepi32_ps(ix0_8x32));

            c00_8x32 = _mm256_i32gather_epi32((void *)row0, ix0_8x32, 4);
            c10_8x32 = _mm256_i32gather_epi32((void *)row0, ix1_8x32, 4);
            c01_8x32 = _mm256_i32gather_epi32((void *)row1, ix0_8x32, 4);
            c11_8x32 = _mm256_i32gather_epi32((void *)row1, ix1_8x32, 4);

            /* ? TODO: unpack+permute */
            c00_lo_4x32 = _mm256_extracti128_si256(c00_8x32, 0);
            c00_hi_4x32 = _mm256_extracti128_si256(c00_8x32, 1);
            c00_01_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(c00_lo_4x32));
            c00_23_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(c00_lo_4x32, 8)));
            c00_45_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(c00_hi_4x32));
            c00_67_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(c00_hi_4x32, 8)));

            c10_lo_4x32 = _mm256_extracti128_si256(c10_8x32, 0);
            c10_hi_4x32 = _mm256_extracti128_si256(c10_8x32, 1);
            c10_01_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(c10_lo_4x32));
            c10_23_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(c10_lo_4x32, 8)));
            c10_45_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(c10_hi_4x32));
            c10_67_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(c10_hi_4x32, 8)));

            c01_lo_4x32 = _mm256_extracti128_si256(c01_8x32, 0);
            c01_hi_4x32 = _mm256_extracti128_si256(c01_8x32, 1);
            c01_01_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(c01_lo_4x32));
            c01_23_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(c01_lo_4x32, 8)));
            c01_45_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(c01_hi_4x32));
            c01_67_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(c01_hi_4x32, 8)));

            c11_lo_4x32 = _mm256_extracti128_si256(c11_8x32, 0);
            c11_hi_4x32 = _mm256_extracti128_si256(c11_8x32, 1);
            c11_01_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(c11_lo_4x32));
            c11_23_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(c11_lo_4x32, 8)));
            c11_45_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(c11_hi_4x32));
            c11_67_8x32 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(c11_hi_4x32, 8)));


            w00_8x32 = _mm256_mul_ps(
                _mm256_sub_ps(const_1f_8x32, wx_8x32),
                _mm256_sub_ps(const_1f_8x32, wy_8x32));
            w10_8x32 = _mm256_mul_ps(wx_8x32, _mm256_sub_ps(const_1f_8x32, wy_8x32));
            w01_8x32 = _mm256_mul_ps(_mm256_sub_ps(const_1f_8x32, wx_8x32), wy_8x32);
            w11_8x32 = _mm256_mul_ps(wx_8x32, wy_8x32);

            w00_01_8x32 = _mm256_permutevar8x32_ps(w00_8x32, w_mask0);
            w00_23_8x32 = _mm256_permutevar8x32_ps(w00_8x32, w_mask1);
            w00_45_8x32 = _mm256_permutevar8x32_ps(w00_8x32, w_mask2);
            w00_67_8x32 = _mm256_permutevar8x32_ps(w00_8x32, w_mask3);

            w10_01_8x32 = _mm256_permutevar8x32_ps(w10_8x32, w_mask0);
            w10_23_8x32 = _mm256_permutevar8x32_ps(w10_8x32, w_mask1);
            w10_45_8x32 = _mm256_permutevar8x32_ps(w10_8x32, w_mask2);
            w10_67_8x32 = _mm256_permutevar8x32_ps(w10_8x32, w_mask3);

            w01_01_8x32 = _mm256_permutevar8x32_ps(w01_8x32, w_mask0);
            w01_23_8x32 = _mm256_permutevar8x32_ps(w01_8x32, w_mask1);
            w01_45_8x32 = _mm256_permutevar8x32_ps(w01_8x32, w_mask2);
            w01_67_8x32 = _mm256_permutevar8x32_ps(w01_8x32, w_mask3);

            w11_01_8x32 = _mm256_permutevar8x32_ps(w11_8x32, w_mask0);
            w11_23_8x32 = _mm256_permutevar8x32_ps(w11_8x32, w_mask1);
            w11_45_8x32 = _mm256_permutevar8x32_ps(w11_8x32, w_mask2);
            w11_67_8x32 = _mm256_permutevar8x32_ps(w11_8x32, w_mask3);
            
            cw00_01_8x32 = _mm256_mul_ps(c00_01_8x32, w00_01_8x32);
            cw00_23_8x32 = _mm256_mul_ps(c00_23_8x32, w00_23_8x32);
            cw00_45_8x32 = _mm256_mul_ps(c00_45_8x32, w00_45_8x32);
            cw00_67_8x32 = _mm256_mul_ps(c00_67_8x32, w00_67_8x32);

            cw10_01_8x32 = _mm256_mul_ps(c10_01_8x32, w10_01_8x32);
            cw10_23_8x32 = _mm256_mul_ps(c10_23_8x32, w10_23_8x32);
            cw10_45_8x32 = _mm256_mul_ps(c10_45_8x32, w10_45_8x32);
            cw10_67_8x32 = _mm256_mul_ps(c10_67_8x32, w10_67_8x32);

            cw01_01_8x32 = _mm256_mul_ps(c01_01_8x32, w01_01_8x32);
            cw01_23_8x32 = _mm256_mul_ps(c01_23_8x32, w01_23_8x32);
            cw01_45_8x32 = _mm256_mul_ps(c01_45_8x32, w01_45_8x32);
            cw01_67_8x32 = _mm256_mul_ps(c01_67_8x32, w01_67_8x32);

            cw11_01_8x32 = _mm256_mul_ps(c11_01_8x32, w11_01_8x32);
            cw11_23_8x32 = _mm256_mul_ps(c11_23_8x32, w11_23_8x32);
            cw11_45_8x32 = _mm256_mul_ps(c11_45_8x32, w11_45_8x32);
            cw11_67_8x32 = _mm256_mul_ps(c11_67_8x32, w11_67_8x32);

            cw0010_01_8x32 = _mm256_add_ps(cw00_01_8x32, cw10_01_8x32);
            cw0010_23_8x32 = _mm256_add_ps(cw00_23_8x32, cw10_23_8x32);
            cw0010_45_8x32 = _mm256_add_ps(cw00_45_8x32, cw10_45_8x32);
            cw0010_67_8x32 = _mm256_add_ps(cw00_67_8x32, cw10_67_8x32);

            cw0111_01_8x32 = _mm256_add_ps(cw01_01_8x32, cw11_01_8x32);
            cw0111_23_8x32 = _mm256_add_ps(cw01_23_8x32, cw11_23_8x32);
            cw0111_45_8x32 = _mm256_add_ps(cw01_45_8x32, cw11_45_8x32);
            cw0111_67_8x32 = _mm256_add_ps(cw01_67_8x32, cw11_67_8x32);

            cw00100111_01_8x32 = _mm256_add_ps(cw0010_01_8x32, cw0111_01_8x32);
            cw00100111_23_8x32 = _mm256_add_ps(cw0010_23_8x32, cw0111_23_8x32);
            cw00100111_45_8x32 = _mm256_add_ps(cw0010_45_8x32, cw0111_45_8x32);
            cw00100111_67_8x32 = _mm256_add_ps(cw0010_67_8x32, cw0111_67_8x32);

            src_01_8x32 = _mm256_cvtps_epi32(cw00100111_01_8x32);
            src_23_8x32 = _mm256_cvtps_epi32(cw00100111_23_8x32);
            src_45_8x32 = _mm256_cvtps_epi32(cw00100111_45_8x32);
            src_67_8x32 = _mm256_cvtps_epi32(cw00100111_67_8x32);

            src_0123_16x16 = _mm256_packus_epi32(src_01_8x32, src_23_8x32);
            src_4567_16x16 = _mm256_packus_epi32(src_45_8x32, src_67_8x32);
            src_8x32 = _mm256_packus_epi16(src_0123_16x16, src_4567_16x16);
            src_8x32 = _mm256_permutevar8x32_epi32(src_8x32, s_mask);

            src_lo_16x16 = _mm256_unpacklo_epi8(src_8x32, const_0);
            src_hi_16x16 = _mm256_unpackhi_epi8(src_8x32, const_0);

            sa_8x32 = _mm256_shuffle_epi8(src_8x32, a_mask);

            inv_sa_8x32 = _mm256_subs_epu8(const_255_32x8, sa_8x32);
            inv_sa_lo_16x16 = _mm256_unpacklo_epi8(inv_sa_8x32, const_0);
            inv_sa_hi_16x16 = _mm256_unpackhi_epi8(inv_sa_8x32, const_0);

            dst_8x32 = _mm256_loadu_si256((__m256i_u *)&dst_row[x]);
            dst_lo_16x16 = _mm256_unpacklo_epi8(dst_8x32, const_0);
            dst_hi_16x16 = _mm256_unpackhi_epi8(dst_8x32, const_0);

            dst_lo_16x16 = _mm256_mullo_epi16(dst_lo_16x16, inv_sa_lo_16x16);
            dst_hi_16x16 = _mm256_mullo_epi16(dst_hi_16x16, inv_sa_hi_16x16);
            dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, const_128_16x16);
            dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, const_128_16x16);
            dst_lo_16x16 = _mm256_mulhi_epu16(dst_lo_16x16, const_257_16x16);
            dst_hi_16x16 = _mm256_mulhi_epu16(dst_hi_16x16, const_257_16x16);
            dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, src_lo_16x16);
            dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, src_hi_16x16);

            result_8x32 = _mm256_packus_epi16(dst_lo_16x16, dst_hi_16x16);

            _mm256_storeu_si256((__m256i_u *)&dst_row[x], result_8x32);
        }
#endif /* SU_WITH_SIMD && defined(__AVX2__) */

        for ( ; x < end_x; ++x) {
            float fx = ((float)x * inv_scale);
            int32_t x0 = SU_MIN((int32_t)fx, (int32_t)src_w - 1);
            float wx = (fx - (float)x0);
            int32_t x1 = SU_MIN(x0 + 1, (int32_t)src_w - 1);

            uint32_t c00 = row0[x0];
            uint32_t c10 = row0[x1];
            uint32_t c01 = row1[x0];
            uint32_t c11 = row1[x1];

            float w00 = ((1.0f - wx) * (1.0f - wy));
            float w10 = (wx * (1.0f - wy));
            float w01 = ((1.0f - wx) * wy);
            float w11 = (wx * wy);

            uint32_t c00a = ((c00 >> 24) & 0xFF);
            uint32_t c00r = ((c00 >> 16) & 0xFF);
            uint32_t c00g = ((c00 >> 8) & 0xFF);
            uint32_t c00b = ((c00 >> 0) & 0xFF);

            uint32_t c10a = ((c10 >> 24) & 0xFF);
            uint32_t c10r = ((c10 >> 16) & 0xFF);
            uint32_t c10g = ((c10 >> 8) & 0xFF);
            uint32_t c10b = ((c10 >> 0) & 0xFF);

            uint32_t c01a = ((c01 >> 24) & 0xFF);
            uint32_t c01r = ((c01 >> 16) & 0xFF);
            uint32_t c01g = ((c01 >> 8) & 0xFF);
            uint32_t c01b = ((c01 >> 0) & 0xFF);

            uint32_t c11a = ((c11 >> 24) & 0xFF);
            uint32_t c11r = ((c11 >> 16) & 0xFF);
            uint32_t c11g = ((c11 >> 8) & 0xFF);
            uint32_t c11b = ((c11 >> 0) & 0xFF);

            uint32_t sa = (uint32_t)((float)c00a * w00 + (float)c10a * w10 + (float)c01a * w01 + (float)c11a * w11 + 0.5f);
            uint32_t sr = (uint32_t)((float)c00r * w00 + (float)c10r * w10 + (float)c01r * w01 + (float)c11r * w11 + 0.5f);
            uint32_t sg = (uint32_t)((float)c00g * w00 + (float)c10g * w10 + (float)c01g * w01 + (float)c11g * w11 + 0.5f);
            uint32_t sb = (uint32_t)((float)c00b * w00 + (float)c10b * w10 + (float)c01b * w01 + (float)c11b * w11 + 0.5f);

            uint32_t inv_sa = (255 - sa);

            uint32_t d = dst_row[x];
            uint32_t da = ((d >> 24) & 0xFF);
            uint32_t dr = ((d >> 16) & 0xFF);
            uint32_t dg = ((d >> 8) & 0xFF);
            uint32_t db = ((d >> 0) & 0xFF);

            da = (sa + (((da * inv_sa + 128) * 257) >> 16));
            dr = (sr + (((dr * inv_sa + 128) * 257) >> 16));
            dg = (sg + (((dg * inv_sa + 128) * 257) >> 16));
            db = (sb + (((db * inv_sa + 128) * 257) >> 16));

            dst_row[x] = ((da << 24) | (dr << 16) | (dg << 8) | (db << 0));
        }
    }
}

static void su_argb32_mask8_bilinear_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color,
        uint8_t *mask, uint32_t mask_w, uint32_t mask_h, uint32_t mask_pitch,
        int32_t dst_x, int32_t dst_y,
        uint32_t w, uint32_t h) {
    /* ? TODO: mask_x/y */
    int32_t x, y;

    float inv_scale = (1.f / (SU_MIN((float)w / (float)mask_w, (float)h / (float)mask_h)));

    uint32_t color_a = ((color >> 24) & 0xFF);
    uint32_t color_r = ((color >> 16) & 0xFF);
    uint32_t color_g = ((color >> 8) & 0xFF);
    uint32_t color_b = ((color >> 0) & 0xFF);

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256 inv_scale_8x32 = _mm256_set1_ps(inv_scale);
    __m256i const_1_8x32 = _mm256_set1_epi32(1);
    __m256i mask_w_minus_1_8x32 = _mm256_set1_epi32((int)mask_w - 1);
    __m256i c_mask_8x32 = _mm256_set1_epi32(0xFF);
    __m256 const_1f_8x32 = _mm256_set1_ps(1.f);
    __m256i const_0 = _mm256_setzero_si256();
    __m256i src_8x32 = _mm256_set1_epi32((int)color);
    __m256i src_lo_16x16_ = _mm256_unpacklo_epi8(src_8x32, const_0);
    __m256i src_hi_16x16_ = _mm256_unpackhi_epi8(src_8x32, const_0);
    __m256 sa_8x32_ = _mm256_set1_ps((float)color_a);
    __m256 const_128f_8x32 = _mm256_set1_ps(128.f);
    __m256i const_257_8x32 = _mm256_set1_epi32(257);
    __m256i const_255_32x8 = _mm256_set1_epi8((char)255);
    __m256i const_128_16x16 = _mm256_set1_epi16(128);
    __m256i const_257_16x16 = _mm256_set1_epi16(257);
    __m256i a_mask = _mm256_setr_epi8(
        0, 0, 0, 0, 4, 4, 4, 4,
        8, 8, 8, 8, 12, 12, 12, 12,
        16, 16, 16, 16, 20, 20, 20, 20,
        24, 24, 24, 24, 28, 28, 28, 28);
#define FROM1 0
#define FROM2 0x80
    __m256i blend_mask = _mm256_setr_epi8(
        FROM1, FROM1, FROM1, FROM1, FROM1, FROM1, FROM2, FROM2,
        FROM1, FROM1, FROM1, FROM1, FROM1, FROM1, FROM2, FROM2,
        FROM1, FROM1, FROM1, FROM1, FROM1, FROM1, FROM2, FROM2,
        FROM1, FROM1, FROM1, FROM1, FROM1, FROM1, FROM2, FROM2);
#undef FROM1
#undef FROM2
    __m256 wy_8x32;
#endif /* SU_WITH_SIMD && defined(__AVX2__) */

    int32_t start_x = 0;
    int32_t start_y = 0;
    int32_t end_x = (int32_t)w;
    int32_t end_y = (int32_t)h;

    if (color_a == 0) {
        return;
    }

    if (dst_x < 0) {
        start_x = -dst_x;
    }
    if (dst_y < 0) {
        start_y = -dst_y;
    }
    if ((dst_x + end_x) > (int32_t)dst_w) {
        end_x = ((int32_t)dst_w - dst_x);
    }
    if ((dst_y + end_y) > (int32_t)dst_h) {
        end_y = ((int32_t)dst_h - dst_y);
    }

    if (start_x >= end_x /* || (start_y >= end_y) */) {
        return;
    }

    for ( y = start_y; y < end_y; ++y) {
        uint32_t *dst_row = &dst[(dst_y + (int32_t)y) * (int32_t)dst_w + dst_x];
        uint8_t *row0, *row1;
        float fy = ((float)y * inv_scale);
        int32_t y0 = SU_MIN((int32_t)fy, (int32_t)mask_h - 1);
        float wy = (fy - (float)y0);
        int32_t y1 = SU_MIN(y0 + 1, (int32_t)mask_h - 1);
        row0 = &mask[y0 * (int32_t)mask_pitch];
        row1 = &mask[y1 * (int32_t)mask_pitch];

        x = start_x;

#if SU_WITH_SIMD && defined(__AVX2__)
        wy_8x32 = _mm256_set1_ps(wy);
        for ( ; (x + 8) <= end_x; x += 8) {
            __m256i ix0_8x32, ix1_8x32;
            __m256 fx0_8x32, wx_8x32;
            __m256i c00i_8x32, c10i_8x32, c01i_8x32, c11i_8x32;
            __m256 c00f_8x32, c10f_8x32, c01f_8x32, c11f_8x32;
            __m256 w00_8x32, w10_8x32, w01_8x32, w11_8x32;
            __m256 cov_8x32;
            __m256 saf_8x32;
            __m256i sai_8x32, sai_lo_16x16, sai_hi_16x16;
            __m256i inv_sai_8x32, inv_sai_lo_16x16, inv_sai_hi_16x16;
            __m256i src_lo_16x16, src_hi_16x16;
            __m256i dst_8x32, dst_lo_16x16, dst_hi_16x16;
            __m256i result_8x32;

            ix0_8x32 = _mm256_setr_epi32(
                (int)x + 0, (int)x + 1, (int)x + 2, (int)x + 3,
                (int)x + 4, (int)x + 5, (int)x + 6, (int)x + 7);
            fx0_8x32 =_mm256_cvtepi32_ps(ix0_8x32);
            fx0_8x32 = _mm256_mul_ps(fx0_8x32, inv_scale_8x32);
            ix0_8x32 = _mm256_cvttps_epi32(fx0_8x32);
            ix0_8x32 = _mm256_min_epi32(ix0_8x32, mask_w_minus_1_8x32);
            ix1_8x32 = _mm256_add_epi32(ix0_8x32, const_1_8x32);
            ix1_8x32 = _mm256_min_epi32(ix1_8x32, mask_w_minus_1_8x32);

            wx_8x32 = _mm256_sub_ps(fx0_8x32, _mm256_cvtepi32_ps(ix0_8x32));

            c00i_8x32 = _mm256_i32gather_epi32((void *)row0, ix0_8x32, 1);
            c10i_8x32 = _mm256_i32gather_epi32((void *)row0, ix1_8x32, 1);
            c01i_8x32 = _mm256_i32gather_epi32((void *)row1, ix0_8x32, 1);
            c11i_8x32 = _mm256_i32gather_epi32((void *)row1, ix1_8x32, 1);
            c00i_8x32 = _mm256_and_si256(c00i_8x32, c_mask_8x32);
            c10i_8x32 = _mm256_and_si256(c10i_8x32, c_mask_8x32);
            c01i_8x32 = _mm256_and_si256(c01i_8x32, c_mask_8x32);
            c11i_8x32 = _mm256_and_si256(c11i_8x32, c_mask_8x32);

            c00f_8x32 = _mm256_cvtepi32_ps(c00i_8x32);
            c10f_8x32 = _mm256_cvtepi32_ps(c10i_8x32);
            c01f_8x32 = _mm256_cvtepi32_ps(c01i_8x32);
            c11f_8x32 = _mm256_cvtepi32_ps(c11i_8x32);

            w00_8x32 = _mm256_mul_ps(
                _mm256_sub_ps(const_1f_8x32, wx_8x32),
                _mm256_sub_ps(const_1f_8x32, wy_8x32));
            w10_8x32 = _mm256_mul_ps(wx_8x32, _mm256_sub_ps(const_1f_8x32, wy_8x32));
            w01_8x32 = _mm256_mul_ps(_mm256_sub_ps(const_1f_8x32, wx_8x32), wy_8x32);
            w11_8x32 = _mm256_mul_ps(wx_8x32, wy_8x32);

            cov_8x32 = _mm256_add_ps(
                _mm256_add_ps(_mm256_mul_ps(c00f_8x32, w00_8x32), _mm256_mul_ps(c10f_8x32, w10_8x32)),
                _mm256_add_ps(_mm256_mul_ps(c01f_8x32, w01_8x32), _mm256_mul_ps(c11f_8x32, w11_8x32)));

            saf_8x32 = _mm256_mul_ps(sa_8x32_, cov_8x32);
            saf_8x32 = _mm256_add_ps(saf_8x32, const_128f_8x32);
            sai_8x32 = _mm256_cvtps_epi32(saf_8x32);
            sai_8x32 = _mm256_mullo_epi32(sai_8x32, const_257_8x32);
            sai_8x32 = _mm256_srli_epi32(sai_8x32, 16);
            sai_8x32 = _mm256_shuffle_epi8(sai_8x32, a_mask);
            sai_lo_16x16 = _mm256_unpacklo_epi8(sai_8x32, const_0);
            sai_hi_16x16 = _mm256_unpackhi_epi8(sai_8x32, const_0);

            src_lo_16x16 = _mm256_mullo_epi16(src_lo_16x16_, sai_lo_16x16);
            src_hi_16x16 = _mm256_mullo_epi16(src_hi_16x16_, sai_hi_16x16);
            src_lo_16x16 = _mm256_add_epi16(src_lo_16x16, const_128_16x16);
            src_hi_16x16 = _mm256_add_epi16(src_hi_16x16, const_128_16x16);
            src_lo_16x16 = _mm256_mulhi_epu16(src_lo_16x16, const_257_16x16);
            src_hi_16x16 = _mm256_mulhi_epu16(src_hi_16x16, const_257_16x16);
            src_lo_16x16 = _mm256_blendv_epi8(src_lo_16x16, sai_lo_16x16, blend_mask);
            src_hi_16x16 = _mm256_blendv_epi8(src_hi_16x16, sai_hi_16x16, blend_mask);

            inv_sai_8x32 = _mm256_subs_epu8(const_255_32x8, sai_8x32);
            inv_sai_lo_16x16 = _mm256_unpacklo_epi8(inv_sai_8x32, const_0);
            inv_sai_hi_16x16 = _mm256_unpackhi_epi8(inv_sai_8x32, const_0);

            dst_8x32 = _mm256_loadu_si256((__m256i_u *)&dst_row[x]);
            dst_lo_16x16 = _mm256_unpacklo_epi8(dst_8x32, const_0);
            dst_hi_16x16 = _mm256_unpackhi_epi8(dst_8x32, const_0);

            dst_lo_16x16 = _mm256_mullo_epi16(dst_lo_16x16, inv_sai_lo_16x16);
            dst_hi_16x16 = _mm256_mullo_epi16(dst_hi_16x16, inv_sai_hi_16x16);
            dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, const_128_16x16);
            dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, const_128_16x16);
            dst_lo_16x16 = _mm256_mulhi_epu16(dst_lo_16x16, const_257_16x16);
            dst_hi_16x16 = _mm256_mulhi_epu16(dst_hi_16x16, const_257_16x16);
            dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, src_lo_16x16);
            dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, src_hi_16x16);

            result_8x32 = _mm256_packus_epi16(dst_lo_16x16, dst_hi_16x16);

            _mm256_storeu_si256((__m256i_u *)&dst_row[x], result_8x32);
        }
#endif /* SU_WITH_SIMD && defined(__AVX2__) */

        for ( ; x < end_x; ++x) {
            float fx = ((float)x * inv_scale);
            int32_t x0 = SU_MIN((int32_t)fx, (int32_t)mask_w - 1);
            float wx = (fx - (float)x0);
            int32_t x1 = SU_MIN(x0 + 1, (int32_t)mask_w - 1);

            float c00 = (float)row0[x0];
            float c10 = (float)row0[x1];
            float c01 = (float)row1[x0];
            float c11 = (float)row1[x1];

            float w00 = ((1.0f - wx) * (1.0f - wy));
            float w10 = (wx * (1.0f - wy));
            float w01 = ((1.0f - wx) * wy);
            float w11 = (wx * wy);

            uint32_t sa = (((uint32_t)((float)color_a *
                (c00 * w00 + c10 * w10 + c01 * w01 + c11 * w11) + 128.f) * 257) >> 16);
            uint32_t sr = ((((uint32_t)color_r * sa + 128) * 257) >> 16);
            uint32_t sg = ((((uint32_t)color_g * sa + 128) * 257) >> 16);
            uint32_t sb = ((((uint32_t)color_b * sa + 128) * 257) >> 16);

            uint32_t inv_sa = (255 - sa);

            uint32_t d = dst_row[x];
            uint32_t da = ((d >> 24) & 0xFF);
            uint32_t dr = ((d >> 16) & 0xFF);
            uint32_t dg = ((d >> 8) & 0xFF);
            uint32_t db = ((d >> 0) & 0xFF);

            da = (sa + (((da * inv_sa + 128) * 257) >> 16));
            dr = (sr + (((dr * inv_sa + 128) * 257) >> 16));
            dg = (sg + (((dg * inv_sa + 128) * 257) >> 16));
            db = (sb + (((db * inv_sa + 128) * 257) >> 16));

            dst_row[x] = ((da << 24) | (dr << 16) | (dg << 8) | (db << 0));
        }
    }
}

static void su_argb32_mask24v_blend_argb32( uint32_t *dst, uint32_t dst_w, uint32_t dst_h,
        uint32_t color,
        uint8_t *mask, uint32_t mask_w, uint32_t mask_h, uint32_t mask_pitch,
        int32_t dst_x, int32_t dst_y, int32_t mask_x, int32_t mask_y,
        uint32_t w, uint32_t h) {
    uint32_t x, y;

    uint32_t color_a = ((color >> 24) & 0xFF);
    uint32_t color_r = ((color >> 16) & 0xFF);
    uint32_t color_g = ((color >> 8) & 0xFF);
    uint32_t color_b = ((color >> 0) & 0xFF);

#if SU_WITH_SIMD && defined(__AVX2__)
    __m256i const_0 = _mm256_setzero_si256();
    __m256i const_128_16x16 = _mm256_set1_epi16(128);
    __m256i const_255_16x16 = _mm256_set1_epi16(255);
    __m256i src_8x32 = _mm256_set1_epi32((int)color);
    __m256i src_lo_16x16_ = _mm256_unpacklo_epi8(src_8x32, const_0);
    __m256i src_hi_16x16_ = _mm256_unpackhi_epi8(src_8x32, const_0);
#define ZERO 0x80
    __m256i a_mask = _mm256_setr_epi8(
        6, ZERO, 6, ZERO, 6, ZERO, 6, ZERO,
        14, ZERO, 14, ZERO, 14, ZERO, 14, ZERO,
        22, ZERO, 22, ZERO, 22, ZERO, 22, ZERO,
        30, ZERO, 30, ZERO, 30, ZERO, 30, ZERO
#undef ZERO
    );
#endif /* SU_WITH_SIMD && defined(__AVX2__) */

    if (color_a == 0) {
        return;
    }

    if (dst_x < 0) {
        uint32_t shift = (uint32_t)(-dst_x);
        if (shift >= w) {
            return;
        }
        dst_x = 0;
        mask_x += (int32_t)shift;
        w -= shift;
    }
    if (dst_y < 0) {
        uint32_t shift = (uint32_t)(-dst_y);
        if (shift >= h) {
            return;
        }
        dst_y = 0;
        mask_y += (int32_t)shift;
        h -= shift;
    }

    if (mask_x < 0) {
        uint32_t shift = (uint32_t)(-mask_x);
        if (shift >= w) {
            return;
        }
        mask_x = 0;
        dst_x += (int32_t)shift;
        w -= shift;
    }
    if (mask_y < 0) {
        uint32_t shift = (uint32_t)(-mask_y);
        if (shift >= h) {
            return;
        }
        mask_y = 0;
        dst_y += (int32_t)shift;
        h -= shift;
    }

    if ((dst_x >= (int32_t)dst_w) || (dst_y >= (int32_t)dst_h)) {
        return;
    }
    if (w > (dst_w - (uint32_t)dst_x)) {
        w = (dst_w - (uint32_t)dst_x);
    }
    if (h > (dst_h - (uint32_t)dst_y)) {
        h = (dst_h - (uint32_t)dst_y);
    }

    if ((mask_x >= (int32_t)mask_w) || (mask_y >= (int32_t)mask_h)) {
        return;
    }
    if (w > (mask_w - (uint32_t)mask_x)) {
        w = (mask_w - (uint32_t)mask_x);
    }
    if (h > (mask_h - (uint32_t)mask_y)) {
        h = (mask_h - (uint32_t)mask_y);
    }

    for ( y = 0; y < h; ++y) {
        uint32_t *dst_row = &dst[((uint32_t)dst_y + y) * dst_w + (uint32_t)dst_x];
        uint8_t *mask_row0 = &mask[(((uint32_t)mask_y + y) * 3 + 0) * mask_pitch + (uint32_t)mask_x];
        uint8_t *mask_row1 = &mask[(((uint32_t)mask_y + y) * 3 + 1) * mask_pitch + (uint32_t)mask_x];
        uint8_t *mask_row2 = &mask[(((uint32_t)mask_y + y) * 3 + 2) * mask_pitch + (uint32_t)mask_x];

        x = 0;

#if SU_WITH_SIMD && defined(__AVX2__)
        for ( ; (x + 8) <= w; x += 8) {
            __m256i dst_8x32, dst_lo_16x16, dst_hi_16x16;
            __m128i mask_1_raw, mask_2_raw, mask_3_raw, mask_lo_8x16, mask_hi_8x16, a_raw;
            __m256i mask_8x32, mask_lo_16x16, mask_hi_16x16;
            __m256i src_lo_16x16, src_hi_16x16;
            __m256i a_lo_16x16, a_hi_16x16, inv_a_lo_16x16, inv_a_hi_16x16;
            __m256i result_8x32;

            dst_8x32 = _mm256_loadu_si256((__m256i_u *)&dst_row[x]);
            dst_lo_16x16 = _mm256_unpacklo_epi8(dst_8x32, const_0);
            dst_hi_16x16 = _mm256_unpackhi_epi8(dst_8x32, const_0);

            mask_1_raw = _mm_loadl_epi64((__m128i_u *)&mask_row0[x]);
            mask_2_raw = _mm_loadl_epi64((__m128i_u *)&mask_row1[x]);
            mask_3_raw = _mm_loadl_epi64((__m128i_u *)&mask_row2[x]);
            a_raw = _mm_max_epu8(mask_1_raw, _mm_max_epu8(mask_2_raw, mask_3_raw));
            mask_lo_8x16 = _mm_unpacklo_epi8(mask_3_raw, mask_2_raw);
            mask_hi_8x16 = _mm_unpacklo_epi8(mask_1_raw, a_raw);
            mask_8x32 = _mm256_setr_m128i(
                _mm_unpacklo_epi16(mask_lo_8x16, mask_hi_8x16),
                _mm_unpackhi_epi16(mask_lo_8x16, mask_hi_8x16));

            mask_lo_16x16 = _mm256_unpacklo_epi8(mask_8x32, const_0);
            mask_hi_16x16 = _mm256_unpackhi_epi8(mask_8x32, const_0);

            src_lo_16x16 = _mm256_mullo_epi16(src_lo_16x16_, mask_lo_16x16);
            src_hi_16x16 = _mm256_mullo_epi16(src_hi_16x16_, mask_hi_16x16);
            src_lo_16x16 = _mm256_add_epi16(src_lo_16x16, const_128_16x16);
            src_hi_16x16 = _mm256_add_epi16(src_hi_16x16, const_128_16x16);
            src_lo_16x16 = _mm256_srli_epi16(src_lo_16x16, 8);
            src_hi_16x16 = _mm256_srli_epi16(src_hi_16x16, 8);

            a_lo_16x16 = _mm256_shuffle_epi8(src_lo_16x16, a_mask);
            a_hi_16x16 = _mm256_shuffle_epi8(src_hi_16x16, a_mask);
            inv_a_lo_16x16 = _mm256_subs_epu16(const_255_16x16, a_lo_16x16);
            inv_a_hi_16x16 = _mm256_subs_epu16(const_255_16x16, a_hi_16x16);

            dst_lo_16x16 = _mm256_mullo_epi16(dst_lo_16x16, inv_a_lo_16x16);
            dst_hi_16x16 = _mm256_mullo_epi16(dst_hi_16x16, inv_a_hi_16x16);
            dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, const_128_16x16);
            dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, const_128_16x16);
            dst_lo_16x16 = _mm256_srli_epi16(dst_lo_16x16, 8);
            dst_hi_16x16 = _mm256_srli_epi16(dst_hi_16x16, 8);
            dst_lo_16x16 = _mm256_add_epi16(dst_lo_16x16, src_lo_16x16);
            dst_hi_16x16 = _mm256_add_epi16(dst_hi_16x16, src_hi_16x16);

            result_8x32 = _mm256_packus_epi16(dst_lo_16x16, dst_hi_16x16);
            _mm256_storeu_si256((__m256i_u *)&dst_row[x], result_8x32);
        }
#endif /* SU_WITH_SIMD && defined(__AVX2__) */

        for ( ; x < w; ++x) {
            uint32_t d = dst_row[x];
            uint32_t da = ((d >> 24) & 0xFF);
            uint32_t dr = ((d >> 16) & 0xFF);
            uint32_t dg = ((d >> 8) & 0xFF);
            uint32_t db = ((d >> 0) & 0xFF);

            uint32_t mr = mask_row0[x];
            uint32_t mg = mask_row1[x];
            uint32_t mb = mask_row2[x];

            uint32_t sa = ((color_a * SU_MAX(SU_MAX(mr, mg), mb) + 128) >> 8);
            uint32_t sr = ((color_r * mr + 128) >> 8);
            uint32_t sg = ((color_g * mg + 128) >> 8);
            uint32_t sb = ((color_b * mb + 128) >> 8);

            uint32_t inv_sa = (255 - sa);

            da = (sa + ((da * inv_sa + 128) >> 8));
            dr = (sr + ((dr * inv_sa + 128) >> 8));
            dg = (sg + ((dg * inv_sa + 128) >> 8));
            db = (sb + ((db * inv_sa + 128) >> 8));

            dst_row[x] = ((da << 24) | (dr << 16) | (dg << 8) | (db << 0));
        }
    }
}

static su_bool32_t su_read_entire_file(su_string_t path, su_fat_ptr_t *out, const su_allocator_t *alloc) {
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
    SU_ALLOCTSA(data.ptr, alloc, data.len + 1, 32);

    bytes_read = 0;
    while (bytes_read < data.len) {
        ssize_t r = read(fd, (uint8_t *)data.ptr + bytes_read, data.len - bytes_read);
        if (r == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                SU_FREE(alloc, data.ptr);
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
SU_HASH_TABLE_DEFINE(su_file_cache_t, su_string_t, su_stbds_hash_string, su_string_equal, SU_MEMCPY, 16)

static su_bool32_t su_read_entire_file_with_cache(su_string_t path, su_fat_ptr_t *out,
        const su_allocator_t *alloc, su_hash_table__su_file_cache_t__t *cache) {
    su_file_cache_t *e = NULL;
    static char buf[PATH_MAX];
    struct stat sb;

    SU_ASSERT(path.nul_terminated); /* TODO: handle properly */

    if (realpath(path.s, buf) == NULL) {
        goto error;
    }

    path.s = buf;
    path.len = SU_STRLEN(buf);
    path.nul_terminated = SU_TRUE;
    path.free_contents = SU_FALSE;

    if (stat(path.s, &sb) == -1) {
        goto error;
    }

    if (su_hash_table__su_file_cache_t__get(cache, path, &e)) {
        if (SU_MEMCMP(&sb.st_mtim, &e->st_mtim, sizeof(sb.st_mtim)) == 0) {
            goto out;
        }
        SU_FREE(alloc, e->data.ptr);
        SU_MEMSET(&e->data, 0, sizeof(e->data));
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
            SU_FREE(alloc, del.data.ptr);
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
    int c = clock_gettime(clock_id, &ts);
    SU_NOTUSED(c);
    SU_ASSERT(c == 0);
    return su_timespec_to_ms(ts);
}

static su_bool32_t su_locale_is_utf8(void) {
    su_c32_t w;
    mbstate_t s;
    
    SU_MEMSET(&s, 0, sizeof(s));

    if (mbrtoc32(&w, "\xC3\xB6", 2, &s) != 2) {
        return SU_FALSE;
    }

    return (w == 0xF6);
}

static void su_nop(void *notused, ...) {
    SU_NOTUSED(notused);
}

/*static void *su_base64_decode(su_string_t text, const su_allocator_t *alloc) {
    uint8_t *ret;
    size_t i = 0, o = 0;

    if (text.len % 4 != 0) {
        return NULL;
    }

    SU_ALLOCTS(ret, alloc, text.len / 4 * 3);

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
    SU_FREE(alloc, ret);
    return NULL;
}*/

static void su__json_buffer_add_char(su_json_buffer_t *buffer, const su_allocator_t *alloc, char c) {
    if (SU_UNLIKELY(buffer->size == buffer->idx)) {
        char *new_data;
        buffer->size = ((buffer->size + 1) * 2);
        SU_ALLOCTS(new_data, alloc, buffer->size);
        SU_MEMCPY(new_data, buffer->data, buffer->idx);
        SU_FREE(alloc, buffer->data);
        buffer->data = new_data;
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
    SU_MEMCPY(&buffer->data[*idx], str.s, str.len);
    *idx += str.len;
}

static void su__json_buffer_add_string(su_json_buffer_t *buffer, const su_allocator_t *alloc, su_string_t str) {
    if (SU_UNLIKELY((buffer->idx + str.len) > buffer->size)) {
        char *new_data;
        buffer->size = ((buffer->size + str.len) * 2);
        SU_ALLOCTS(new_data, alloc, buffer->size);
        SU_MEMCPY(new_data, buffer->data, buffer->idx);
        SU_FREE(alloc, buffer->data);
        buffer->data = new_data;
    }
    SU_MEMCPY(&buffer->data[buffer->idx], str.s, str.len);
    buffer->idx += str.len;
}

static void su__json_buffer_add_string_nocheck(su_json_buffer_t *buffer, su_string_t str) {
    SU_MEMCPY(&buffer->data[buffer->idx], str.s, str.len);
    buffer->idx += str.len;
}

static SU_ATTRIBUTE_FORMAT_PRINTF(3, 4) void su__json_buffer_add_format(su_json_buffer_t *buffer,
        const su_allocator_t *alloc, const char *fmt, ...) {
    char buf[512];
    size_t len;

    va_list args;
    va_start(args, fmt);

    len = (size_t)stbsp_vsnprintf(buf, sizeof(buf), fmt, args);
    if (SU_UNLIKELY((buffer->idx + len) > buffer->size)) {
        char *new_data;
        buffer->size = ((buffer->size + len) * 2);
        SU_ALLOCTS(new_data, alloc, buffer->size);
        SU_MEMCPY(new_data, buffer->data, buffer->idx);
        SU_FREE(alloc, buffer->data);
        buffer->data = new_data;
    }
    SU_MEMCPY(&buffer->data[buffer->idx], buf, len);
    buffer->idx += len;

    va_end(args);
}

static void su__json_buffer_add_string_escape(su_json_buffer_t *buf, const su_allocator_t *alloc, su_string_t str) {
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
                su__json_buffer_add_string(buf, alloc, su_string_("\\u00"));
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
    if (writer->state_count > 0) {
        return writer->state[writer->state_count - 1];
    }

    return SU__JSON_WRITER_STATE_ROOT;
}

static void su__json_writer_add_state(su_json_writer_t *writer, const su_allocator_t *alloc, su__json_writer_state_t state) {
    if (SU_UNLIKELY(writer->state_capacity == writer->state_count)) {
        su__json_writer_state_t *new_state;
        writer->state_capacity *= 2;
        SU_ARRAY_ALLOC(new_state, alloc, writer->state_capacity);
        SU_MEMCPY(new_state, writer->state, sizeof(writer->state[0]) * writer->state_count);
        SU_FREE(alloc, writer->state);
        writer->state = new_state;
    }
    writer->state[writer->state_count++] = state;
}

static void su__json_writer_element(su_json_writer_t *writer, const su_allocator_t *alloc) {
    switch (su__json_writer_get_state(writer)) {
    case SU__JSON_WRITER_STATE_ROOT:
        break;
    case SU__JSON_WRITER_STATE_OBJECT:
        writer->state[writer->state_count - 1] = SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA;
        break;
    case SU__JSON_WRITER_STATE_ARRAY:
        writer->state[writer->state_count - 1] = SU__JSON_WRITER_STATE_ARRAY_EXPECTING_COMMA;
        break;
    case SU__JSON_WRITER_STATE_KEY:
        writer->state_count--;
        break;
    case SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA:
    case SU__JSON_WRITER_STATE_ARRAY_EXPECTING_COMMA:
        su__json_buffer_add_char(&writer->buf, alloc, ',');
        break;
    default:
        SU_ASSERT_UNREACHABLE;
    }
}

static void su_json_writer_init(su_json_writer_t *writer, const su_allocator_t *alloc, size_t initial_bufsize) {
    writer->state_capacity = initial_bufsize;
    SU_ARRAY_ALLOC(writer->state, alloc, writer->state_capacity);
    SU_ALLOCTS(writer->buf.data, alloc, initial_bufsize);
    writer->state_count = 0;
    writer->buf.size = initial_bufsize;
    writer->buf.idx = 0;
}

static void su_json_writer_fini(su_json_writer_t *writer, const su_allocator_t *alloc) {
    SU_FREE(alloc, writer->buf.data);
    SU_FREE(alloc, writer->state);
}

static void su_json_writer_reset(su_json_writer_t *writer) {
    writer->buf.idx = 0;
    writer->state_count = 0;
}

static void su_json_writer_object_begin(su_json_writer_t *writer, const su_allocator_t *alloc) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    su__json_buffer_add_char(&writer->buf, alloc, '{');
    su__json_writer_add_state(writer, alloc, SU__JSON_WRITER_STATE_OBJECT);
}

static void su_json_writer_object_end(su_json_writer_t *writer, const su_allocator_t *alloc) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state == SU__JSON_WRITER_STATE_OBJECT) || (state == SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_buffer_add_char(&writer->buf, alloc, '}');
    writer->state_count--;
}

static void su_json_writer_object_key_escape(su_json_writer_t *writer, const su_allocator_t *alloc, su_string_t key) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state == SU__JSON_WRITER_STATE_OBJECT) || (state == SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    su__json_buffer_add_char(&writer->buf, alloc, '"');
    su__json_buffer_add_string_escape(&writer->buf, alloc, key);
    su__json_buffer_add_char(&writer->buf, alloc, '"');
    su__json_buffer_add_char(&writer->buf, alloc, ':');
    su__json_writer_add_state(writer, alloc, SU__JSON_WRITER_STATE_KEY);
}

static void su_json_writer_object_key(su_json_writer_t *writer, const su_allocator_t *alloc, su_string_t key) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state == SU__JSON_WRITER_STATE_OBJECT) || (state == SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    su__json_buffer_add_char(&writer->buf, alloc, '"');
    su__json_buffer_add_string(&writer->buf, alloc, key);
    su__json_buffer_add_char(&writer->buf, alloc, '"');
    su__json_buffer_add_char(&writer->buf, alloc, ':');
    su__json_writer_add_state(writer, alloc, SU__JSON_WRITER_STATE_KEY);
}

static void su_json_writer_array_begin(su_json_writer_t *writer, const su_allocator_t *alloc) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    su__json_buffer_add_char(&writer->buf, alloc, '[');
    su__json_writer_add_state(writer, alloc, SU__JSON_WRITER_STATE_ARRAY);
}

static void su_json_writer_array_end(su_json_writer_t *writer, const su_allocator_t *alloc) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state == SU__JSON_WRITER_STATE_ARRAY) || (state == SU__JSON_WRITER_STATE_ARRAY_EXPECTING_COMMA));
    su__json_buffer_add_char(&writer->buf, alloc, ']');
    writer->state_count--;
}

static void su_json_writer_null(su_json_writer_t *writer, const su_allocator_t *alloc) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    su__json_buffer_add_string(&writer->buf, alloc, su_string_("null"));
}

static void su_json_writer_bool(su_json_writer_t *writer, const su_allocator_t *alloc, su_bool32_t b) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    if (b) {
        su__json_buffer_add_string(&writer->buf, alloc, su_string_("true"));
    } else {
        su__json_buffer_add_string(&writer->buf, alloc, su_string_("false"));
    }
}

static void su_json_writer_int(su_json_writer_t *writer, const su_allocator_t *alloc, int64_t i) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    su__json_buffer_add_format(&writer->buf, alloc, "%ld", i);
}

static void su_json_writer_uint(su_json_writer_t *writer, const su_allocator_t *alloc, uint64_t u) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    su__json_buffer_add_format(&writer->buf, alloc, "%lu", u);
}

static void su_json_writer_double(su_json_writer_t *writer, const su_allocator_t *alloc, double d) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    /* TODO: isnan(d) || isinf(d) -> null */
    if (SU_FABS(d) > DBL_EPSILON) {
        su__json_buffer_add_format(&writer->buf, alloc, "%.17g", d);
    } else {
        su__json_buffer_add_format(&writer->buf, alloc, "%.1f", d);
    }
}

static void su_json_writer_string_escape(su_json_writer_t *writer, const su_allocator_t *alloc, su_string_t str) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    su__json_buffer_add_char(&writer->buf, alloc, '"');
    su__json_buffer_add_string_escape(&writer->buf, alloc, str);
    su__json_buffer_add_char(&writer->buf, alloc, '"');
}

static void su_json_writer_string(su_json_writer_t *writer, const su_allocator_t *alloc, su_string_t str) {
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    su__json_buffer_add_char(&writer->buf, alloc, '"');
    su__json_buffer_add_string(&writer->buf, alloc, str);
    su__json_buffer_add_char(&writer->buf, alloc, '"');
}

static void su_json_writer_raw(su_json_writer_t *writer, const su_allocator_t *alloc, void *data, size_t len) {
    su_string_t s;
    su__json_writer_state_t state = su__json_writer_get_state(writer);
    SU_NOTUSED(state);
    SU_ASSERT((state != SU__JSON_WRITER_STATE_OBJECT) && (state != SU__JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
    su__json_writer_element(writer, alloc);
    s.s = (char *)data;
    s.len = len;
    s.free_contents = SU_FALSE;
    s.nul_terminated = SU_FALSE;
    su__json_buffer_add_string(&writer->buf, alloc, s);
}

static void su_json_writer_token(su_json_writer_t *writer, const su_allocator_t *alloc, su_json_token_t token) {
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

static void su_json_writer_ast_node(su_json_writer_t *writer, const su_allocator_t *alloc, su_json_ast_node_t *node) {
    /* TODO: remove recursion */
    switch (node->type) {
    case SU_JSON_AST_NODE_TYPE_OBJECT: {
        size_t i = 0;
        su_json_writer_object_begin(writer, alloc);
        for ( ; i < node->value.o.count; ++i) {
            su_json_ast_key_value_t *key_value = &node->value.o.kvs[i];
            su_json_writer_object_key_escape(writer, alloc, key_value->key); /* TODO: escape only when necessary */
            su_json_writer_ast_node(writer, alloc, &key_value->value);
        }
        su_json_writer_object_end(writer, alloc);
        break;
    }
    case SU_JSON_AST_NODE_TYPE_ARRAY: {
        size_t i = 0;
        su_json_writer_array_begin(writer, alloc);
        for ( ; i < node->value.a.count; ++i) {
            su_json_writer_ast_node(writer, alloc, &node->value.a.nodes[i]);
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
        const su_allocator_t *alloc, su_string_t *out) {
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
    SU_ALLOCTS(out->s, alloc, tokener->buf.idx + 32);
    out->len = tokener->buf.idx;
    out->free_contents = SU_FALSE;
    out->nul_terminated = SU_TRUE; /* TODO: remove */
    for ( ;;) {
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
                uint8_t *dst;

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

                dst = (uint8_t *)&out->s[str_idx];
                if (c <= 0x7F) {
                    dst[0] = (uint8_t)c;
                    str_idx += 1;
                } else if (c <= 0x7FF) {
                    dst[0] = (uint8_t)((c >> 6) + 192);
                    dst[1] = (uint8_t)((c & 63) + 128);
                    str_idx += 2;
                } else if (c <= 0xFFFF) {
                    dst[0] = (uint8_t)((c >> 12) + 224);
                    dst[1] = (uint8_t)(((c >> 6) & 63) + 128);
                    dst[2] = (uint8_t)((c & 63) + 128);
                    str_idx += 3;
                } else if (c <= 0x10FFFF) {
                    dst[0] = (uint8_t)((c >> 18) + 240);
                    dst[1] = (uint8_t)(((c >> 12) & 63) + 128);
                    dst[2] = (uint8_t)(((c >> 6) & 63) + 128);
                    dst[3] = (uint8_t)((c & 63) + 128);
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
/* TODO: AVX512, WASM, ARM, ?SSE */
#else
    SU_ALLOCTS(out->s, alloc, tokener->buf.idx + 32);
    out->len = tokener->buf.idx;
    out->free_contents = SU_FALSE;
    out->nul_terminated = SU_TRUE; /* TODO: remove */
    SU_MEMCPY(out->s, tokener->buf.data, tokener->buf.idx);
    for ( ;;) {
        char *backslash = (char *)SU_MEMCHR(&tokener->buf.data[buf_idx], '\\', tokener->buf.idx - buf_idx);
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
                uint8_t *dst;
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

                dst = (uint8_t *)&out->s[str_idx];
                if (c <= 0x7F) {
                    dst[0] = (uint8_t)c;
                    str_idx += 1;
                } else if (c <= 0x7FF) {
                    dst[0] = (uint8_t)((c >> 6) + 192);
                    dst[1] = (uint8_t)((c & 63) + 128);
                    str_idx += 2;
                } else if (c <= 0xFFFF) {
                    dst[0] = (uint8_t)((c >> 12) + 224);
                    dst[1] = (uint8_t)(((c >> 6) & 63) + 128);
                    dst[2] = (uint8_t)((c & 63) + 128);
                    str_idx += 3;
                } else if (c <= 0x10FFFF) {
                    dst[0] = (uint8_t)((c >> 18) + 240);
                    dst[1] = (uint8_t)(((c >> 12) & 63) + 128);
                    dst[2] = (uint8_t)(((c >> 6) & 63) + 128);
                    dst[3] = (uint8_t)((c & 63) + 128);
                    str_idx += 4;
                }
            } else {
                if (SU_UNLIKELY((out->s[str_idx] = (char)escape_lut[escape_char]) == 0)) {
                    return SU_JSON_TOKENER_STATE_ERROR;
                }

                str_idx += 1;
                buf_idx += 2;
            }
            SU_MEMMOVE(&out->s[str_idx], &tokener->buf.data[buf_idx], tokener->buf.idx - buf_idx);
        }
    }
#endif

    out->len -= (buf_idx - str_idx);
    out->s[out->len] = '\0'; /* TODO: remove */
    tokener->buf.idx = 0;
    return SU_JSON_TOKENER_STATE_SUCCESS;
}

static void su_json_tokener_set_string(su_json_tokener_t *tokener, const su_allocator_t *alloc, su_string_t str) {
    if (str.len >= (tokener->buf.size - tokener->buf.idx)) {
        char *new_data;
        tokener->buf.size = ((str.len + tokener->buf.size) * 2);
        SU_ALLOCTS(new_data, alloc, tokener->buf.size);
        SU_MEMCPY(new_data, tokener->buf.data, tokener->buf.idx);
        SU_FREE(alloc, tokener->buf.data);
        tokener->buf.data = new_data;
    }

    {
        su__json_tokener_state_t *new_state;
        SU_ARRAY_ALLOC(new_state, alloc, (tokener->state_count + str.len + 1));
        SU_MEMCPY(new_state, tokener->state, sizeof(tokener->state[0]) * tokener->state_count);
        SU_FREE(alloc, tokener->state);
        tokener->state = new_state;
    }

    tokener->pos = 0;
    tokener->str = str;
}

static su__json_tokener_state_t su__json_tokener_get_state(su_json_tokener_t *tokener) {
    if (tokener->state_count > 0) {
        return tokener->state[tokener->state_count - 1];
    }

    return SU__JSON_TOKENER_STATE_ROOT;
}

static void su__json_tokener_value_end(su_json_tokener_t *tokener) {
    tokener->state_count--;
    switch (su__json_tokener_get_state(tokener)) {
    case SU__JSON_TOKENER_STATE_VALUE:
        tokener->state_count--;
        break;
    case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
    case SU__JSON_TOKENER_STATE_ARRAY:
        tokener->state[tokener->state_count - 1] = SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA;
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
        const su_allocator_t *alloc, su_json_token_t *out) {
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
/* TODO: AVX512, WASM, ARM, ?SSE */

        for ( ;;) {
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
        tokener->state_count--;
        switch (su__json_tokener_get_state(tokener)) {
        case SU__JSON_TOKENER_STATE_OBJECT_EMPTY:
        case SU__JSON_TOKENER_STATE_OBJECT:
            tokener->state[tokener->state_count - 1] = SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA;
            tokener->state[tokener->state_count++] = SU__JSON_TOKENER_STATE_KEY;
            out->type = SU_JSON_TOKEN_TYPE_KEY;
            break;
        case SU__JSON_TOKENER_STATE_VALUE:
            tokener->state_count--;
            out->type = SU_JSON_TOKEN_TYPE_STRING;
            break;
        case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
        case SU__JSON_TOKENER_STATE_ARRAY:
            tokener->state[tokener->state_count - 1] = SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA;
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
/* TODO: AVX512, WASM, ARM, ?SSE */

        for ( ;;) {
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
            SU_MEMCPY(&tokener->buf.data[tokener->buf.idx], &tokener->str.s[tokener->pos], bytes_needed);
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
        if (SU_UNLIKELY(!su_string_equal(su_string_("null"), s))) {
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
            SU_MEMCPY(&tokener->buf.data[tokener->buf.idx], &tokener->str.s[tokener->pos], bytes_needed);
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
        if (SU_UNLIKELY(!su_string_equal(su_string_("true"), s))) {
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
            SU_MEMCPY(&tokener->buf.data[tokener->buf.idx], &tokener->str.s[tokener->pos], bytes_needed);
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
        if (SU_UNLIKELY(!su_string_equal(su_string_("false"), s))) {
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
                tokener->state[tokener->state_count - 1] = SU__JSON_TOKENER_STATE_OBJECT_EMPTY;
                break;
            case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
            case SU__JSON_TOKENER_STATE_ARRAY:
                tokener->state[tokener->state_count - 1] = SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA;
                SU_ATTRIBUTE_FALLTHROUGH;
            case SU__JSON_TOKENER_STATE_ROOT:
                tokener->state[tokener->state_count++] = SU__JSON_TOKENER_STATE_OBJECT_EMPTY;
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
                tokener->state[tokener->state_count - 1] = SU__JSON_TOKENER_STATE_ARRAY_EMPTY;
                break;
            case SU__JSON_TOKENER_STATE_ARRAY_EMPTY:
            case SU__JSON_TOKENER_STATE_ARRAY:
                tokener->state[tokener->state_count - 1] = SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA;
                SU_ATTRIBUTE_FALLTHROUGH;
            case SU__JSON_TOKENER_STATE_ROOT:
                tokener->state[tokener->state_count++] = SU__JSON_TOKENER_STATE_ARRAY_EMPTY;
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
            tokener->state_count--;
            tokener->pos++;
            tokener->depth--;
            out->depth = tokener->depth;
            out->type = SU_JSON_TOKEN_TYPE_OBJECT_END;
            return SU_JSON_TOKENER_STATE_SUCCESS;
        case ']':
            if (SU_UNLIKELY((state != SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA) && (state != SU__JSON_TOKENER_STATE_ARRAY_EMPTY))) {
                return SU_JSON_TOKENER_STATE_ERROR;
            }
            tokener->state_count--;
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
            tokener->state[tokener->state_count++] = SU__JSON_TOKENER_STATE_STRING;
            tokener->pos++;
            goto _string;
        }
        case ':':
            if (SU_UNLIKELY(state != SU__JSON_TOKENER_STATE_KEY)) {
                return SU_JSON_TOKENER_STATE_ERROR;
            }
            tokener->state[tokener->state_count - 1] = SU__JSON_TOKENER_STATE_VALUE;
            state = SU__JSON_TOKENER_STATE_VALUE;
            continue;
        case ',':
            switch (state) {
            case SU__JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
                tokener->state[tokener->state_count - 1] = SU__JSON_TOKENER_STATE_OBJECT;
                state = SU__JSON_TOKENER_STATE_OBJECT;
                continue;
            case SU__JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
                tokener->state[tokener->state_count - 1] = SU__JSON_TOKENER_STATE_ARRAY;
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
                tokener->state[tokener->state_count++] = SU__JSON_TOKENER_STATE_NULL;
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
                tokener->state[tokener->state_count++] = SU__JSON_TOKENER_STATE_TRUE;
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
                tokener->state[tokener->state_count++] = SU__JSON_TOKENER_STATE_FALSE;
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
                tokener->state[tokener->state_count++] = SU__JSON_TOKENER_STATE_NUMBER;
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
    SU_MEMSET(&ast->root, 0, sizeof(ast->root));
    ast->current = &ast->root;
}

static su_json_tokener_state_t su_json_tokener_ast(su_json_tokener_t *tokener, const su_allocator_t *alloc,
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
            SU_MEMSET(&node.value.o, 0, sizeof(node.value.o));
            break;
        case SU_JSON_TOKEN_TYPE_ARRAY_START:
            node.type = SU_JSON_AST_NODE_TYPE_ARRAY;
            SU_MEMSET(&node.value.a, 0, sizeof(node.value.a));
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
                for ( ; i < ast->current->value.o.count; ++i) {
                    su_json_ast_key_value_t *key_value = &ast->current->value.o.kvs[i];
                    if (SU_UNLIKELY(su_string_equal(token.value.s, key_value->key))) {
                        return SU_JSON_TOKENER_STATE_ERROR;
                    }
                }
            }
            kv.key = token.value.s;
            if (ast->current->value.o.capacity == ast->current->value.o.count) {
                su_json_ast_key_value_t *new_kvs;
                ast->current->value.o.capacity = ((ast->current->value.o.capacity * 2) + 8);
                SU_ARRAY_ALLOC(new_kvs, alloc, ast->current->value.o.capacity);
                SU_MEMCPY(new_kvs, ast->current->value.o.kvs, sizeof(new_kvs[0]) * ast->current->value.o.count);
                SU_FREE(alloc, ast->current->value.o.kvs);
                ast->current->value.o.kvs = new_kvs;
            }
            ast->current->value.o.kvs[ast->current->value.o.count++] = kv;
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
            current = &ast->current->value.o.kvs[ast->current->value.o.count - 1].value;
            *current = node;
        } else if (ast->current->type == SU_JSON_AST_NODE_TYPE_ARRAY) {
            if (ast->current->value.a.capacity == ast->current->value.a.count) {
                su_json_ast_node_t *new_nodes;
                ast->current->value.a.capacity = ((ast->current->value.a.capacity * 2) + 8);
                SU_ARRAY_ALLOC(new_nodes, alloc, ast->current->value.a.capacity);
                SU_MEMCPY(new_nodes, ast->current->value.a.nodes, sizeof(new_nodes[0]) * ast->current->value.a.count);
                SU_FREE(alloc, ast->current->value.a.nodes);
                ast->current->value.a.nodes = new_nodes;
            }
            ast->current->value.a.nodes[ast->current->value.a.count++] = node;
            current = &ast->current->value.a.nodes[ast->current->value.a.count - 1];
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

    for ( ; i < node->value.o.count; ++i) {
        su_json_ast_key_value_t *key_value = &node->value.o.kvs[i];
        if (su_string_equal(key_value->key, key)) {
            return &key_value->value;
        }
    }

    return NULL;
}

static SU_ATTRIBUTE_ALWAYS_INLINE inline void su_json_tokener_advance_assert(su_json_tokener_t *tokener,
        const su_allocator_t *alloc, su_json_token_t *token_out) {
    su_json_tokener_state_t s = su_json_tokener_next(tokener, alloc, token_out);
    SU_NOTUSED(s);
    SU_ASSERT(s == SU_JSON_TOKENER_STATE_SUCCESS);
}

static SU_ATTRIBUTE_ALWAYS_INLINE inline void su_json_tokener_advance_assert_type(
        su_json_tokener_t *tokener, const su_allocator_t *alloc,
        su_json_token_t *token_out, su_json_token_type_t expected_type) {
    SU_NOTUSED(expected_type);
    su_json_tokener_advance_assert(tokener, alloc, token_out);
    SU_ASSERT(token_out->type == expected_type);
}

#endif /* defined(SU_IMPLEMENTATION) && !defined(SU__REIMPLEMENTATION_GUARD) */
