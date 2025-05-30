#if !defined(MACROS_H)
#define MACROS_H

#include <assert.h>

// NULL

#define UNREACHABLE 0

#define SIZEOF sizeof
#define ASSERT assert

#define PRAGMA(x) _Pragma(#x)

#define IGNORE_WARNINGS_START \
	PRAGMA(GCC diagnostic push) \
	IGNORE_WARNING("-Wpragmas") \
	IGNORE_WARNING("-Wunknown-warning-option")
#define IGNORE_WARNING(w) PRAGMA(GCC diagnostic ignored w)
#define IGNORE_WARNINGS_END PRAGMA(GCC diagnostic pop)

#if defined(__GNUC__)
#define ALIGNOF __alignof__
#define TYPEOF __typeof__
#define STATIC_ASSERT_ _Static_assert
#else
#define ALIGNOF alignof
#define TYPEOF typeof
#define STATIC_ASSERT_ static_assert
#endif

#define STATIC_ASSERT_WITH_MSG_(x, msg) STATIC_ASSERT_(x, msg)
#define STATIC_ASSERT_WITHOUT_MSG_(x) STATIC_ASSERT_(x, "")
#define STATIC_ASSERT_CHOOSE_MACRO_(a, b, macro, ...) macro

#define STATIC_ASSERT(...) \
	STATIC_ASSERT_CHOOSE_MACRO_(__VA_ARGS__, STATIC_ASSERT_WITH_MSG_, STATIC_ASSERT_WITHOUT_MSG_, UNREACHABLE)(__VA_ARGS__)


#if defined(__has_include) && __has_include(<stddef.h>)
#define HAS_INCLUDE(x) __has_include(x)
#else
#define HAS_INCLUDE(x) 0
#endif

#if defined(__has_attribute)
#define HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define HAS_ATTRIBUTE(x) 0
#endif

#if HAS_ATTRIBUTE(format)
#define ATTRIB_FORMAT_PRINTF(start, end) __attribute__((__format__(__printf__, (start), (end))))
#else
#define ATTRIB_FORMAT_PRINTF(start, end)
#endif

#if HAS_ATTRIBUTE(const)
#define ATTRIB_CONST __attribute__((__const__))
#else
#define ATTRIB_CONST
#endif

#if HAS_ATTRIBUTE(fallthrough)
#define ATTRIB_FALLTHROUGH __attribute__((__fallthrough__))
#else
#define ATTRIB_FALLTHROUGH
#endif

#if HAS_ATTRIBUTE(pure)
#define ATTRIB_PURE __attribute__((__pure__))
#else
#define ATTRIB_PURE
#endif

#if HAS_ATTRIBUTE(noreturn)
#define ATTRIB_NORETURN __attribute__((__noreturn__))
#else
#define ATTRIB_NORETURN
#endif


#if defined(__has_builtin)
#define HAS_BUILTIN(x) __has_builtin(x)
#else
#define HAS_BUILTIN(x) 0
#endif

#if HAS_BUILTIN(__builtin_unreachable)
#define ASSERT_UNREACHABLE \
	ASSERT(UNREACHABLE); \
	__builtin_unreachable()
#else
#define ASSERT_UNREACHABLE ASSERT(UNREACHABLE)
#endif

#if HAS_BUILTIN(__builtin_expect)
#define LIKELY(expr) __builtin_expect(!!(expr), 1)
#define UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#else
#define LIKELY(expr) (expr)
#define UNLIKELY(expr) (expr)
#endif

#if HAS_BUILTIN(__builtin_types_compatible_p)
#define TYPES_COMPATIBLE(a, b) __builtin_types_compatible_p(a, b)
#else
#define TYPES_COMPATIBLE(a, b) 0
#endif

#define LENGTH(x) (SIZEOF((x)) / SIZEOF((x)[0]))
#define STRING_LITERAL_LENGTH(string) (LENGTH((string)) - 1)

#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

//#define FILENAME (strrchr("/" __FILE__, '/') + 1)

#endif // MACROS_H
