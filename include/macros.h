#if !defined(MACROS_H)
#define MACROS_H

#include <assert.h>

#define PRAGMA(x) _Pragma(#x)

#define IGNORE_WARNINGS_START \
	PRAGMA(GCC diagnostic push) \
	IGNORE_WARNING("-Wpragmas") \
	IGNORE_WARNING("-Wunknown-warning-option")
#define IGNORE_WARNING(w) PRAGMA(GCC diagnostic ignored w)
#define IGNORE_WARNINGS_END PRAGMA(GCC diagnostic pop)

#if defined(__GNUC__)
#define ALIGNOF __alignof__
#else
#define ALIGNOF _Alignof
#endif

#if !defined(__has_attribute)
#define __has_attribute(attr) 0
#endif

#if __has_attribute(format)
#define ATTRIB_FORMAT_PRINTF(start, end) __attribute__((__format__(__printf__, (start), (end))))
#else
#define ATTRIB_FORMAT_PRINTF(start, end)
#endif

#if __has_attribute(const)
#define ATTRIB_CONST __attribute__((__const__))
#else
#define ATTRIB_CONST
#endif

#if __has_attribute(fallthrough)
#define ATTRIB_FALLTHROUGH __attribute__((__fallthrough__))
#else
#define ATTRIB_FALLTHROUGH
#endif

#if __has_attribute(pure)
#define ATTRIB_PURE __attribute__((__pure__))
#else
#define ATTRIB_PURE
#endif

#if __has_attribute(noreturn)
#define ATTRIB_NORETURN __attribute__((__noreturn__))
#else
#define ATTRIB_NORETURN
#endif


#if !defined(__has_builtin)
#define __has_builtin(attr) 0
#endif

#define UNREACHABLE 0

#if __has_builtin(__builtin_unreachable)
#define ASSERT_UNREACHABLE \
	assert(UNREACHABLE); \
	__builtin_unreachable()
#else
#define ASSERT_UNREACHABLE assert(UNREACHABLE)
#endif

#if __has_builtin(__builtin_expect)
#define LIKELY(expr) __builtin_expect(!!(expr), 1)
#define UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#else
#define LIKELY(expr) (expr)
#define UNLIKELY(expr) (expr)
#endif

#define LENGTH(x) (sizeof((x)) / sizeof((x)[0]))
#define STRING_LITERAL_LENGTH(string) (LENGTH((string)) - 1)

#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

//#define FILENAME (strrchr("/" __FILE__, '/') + 1)

#endif // MACROS_H
