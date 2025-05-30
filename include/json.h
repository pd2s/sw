#if !defined(JSON_H)
#define JSON_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <errno.h>

#include "util.h"
#include "macros.h"

struct json_buffer {
	char *data;
	size_t size;
	size_t idx;
};

enum json_writer_state {
	JSON_WRITER_STATE_ROOT,
	JSON_WRITER_STATE_OBJECT,
	JSON_WRITER_STATE_ARRAY,
	JSON_WRITER_STATE_KEY,
	JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA,
	JSON_WRITER_STATE_ARRAY_EXPECTING_COMMA,
};

typedef enum json_writer_state enum_json_writer_state;

STACK_DECLARE_DEFINE(enum_json_writer_state)

struct json_writer {
	struct json_buffer buf;
	stack_enum_json_writer_state_t state;
};

enum json_tokener_state {
	JSON_TOKENER_STATE_ROOT,
	JSON_TOKENER_STATE_OBJECT,
	JSON_TOKENER_STATE_OBJECT_EMPTY,
	JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA,
	JSON_TOKENER_STATE_KEY,
	JSON_TOKENER_STATE_VALUE,
	JSON_TOKENER_STATE_ARRAY,
	JSON_TOKENER_STATE_ARRAY_EMPTY,
	JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA,
	JSON_TOKENER_STATE_STRING,
	JSON_TOKENER_STATE_NUMBER,
	JSON_TOKENER_STATE_NULL,
	JSON_TOKENER_STATE_FALSE,
	JSON_TOKENER_STATE_TRUE,
};

typedef enum json_tokener_state enum_json_tokener_state;

ARENA_STACK_DECLARE_DEFINE(enum_json_tokener_state)

struct json_tokener {
	string_t str;
	size_t pos;

	struct json_buffer buf;
	size_t last_escape_idx; // in buf

	size_t depth;

	arena_stack_enum_json_tokener_state_t state;
	arena_t allocator;
};

enum json_tokener_state_ { // TODO: rename
	JSON_TOKENER_STATE_SUCCESS,
	JSON_TOKENER_STATE_ERROR,
	JSON_TOKENER_STATE_EOF,
	JSON_TOKENER_STATE_MORE_DATA_EXPECTED,
};

enum json_token_type {
	JSON_TOKEN_TYPE_OBJECT_START,
	JSON_TOKEN_TYPE_OBJECT_END,
	JSON_TOKEN_TYPE_KEY,
	JSON_TOKEN_TYPE_ARRAY_START,
	JSON_TOKEN_TYPE_ARRAY_END,
	JSON_TOKEN_TYPE_STRING,
	JSON_TOKEN_TYPE_BOOL,
	JSON_TOKEN_TYPE_NULL,
	JSON_TOKEN_TYPE_DOUBLE,
	JSON_TOKEN_TYPE_INT,
	JSON_TOKEN_TYPE_UINT,
};

struct json_token {
	size_t depth;
	uint32_t pad;
	enum json_token_type type;
	union {
		string_t s; // JSON_TOKEN_TYPE_STRING, JSON_TOKEN_TYPE_KEY
		bool32_t b; // JSON_TOKEN_TYPE_BOOL
		double d; // JSON_TOKEN_TYPE_DOUBLE
		int64_t i; // JSON_TOKEN_TYPE_INT
		uint64_t u; // JSON_TOKEN_TYPE_UINT
	};
};

#define JSON_TOKENER_ADVANCE_ASSERT(tokener, token) \
	do { \
		enum json_tokener_state_ json_tokener_state__ = json_tokener_next(&(tokener), &(token)); \
		(void)json_tokener_state__; \
		ASSERT(json_tokener_state__ == JSON_TOKENER_STATE_SUCCESS); \
	} while (0)

#define JSON_TOKENER_ADVANCE_ASSERT_TYPE(tokener, token, expected_type) \
	do { \
		JSON_TOKENER_ADVANCE_ASSERT(tokener, token); \
		ASSERT(token.type == (expected_type)); \
	} while (0)

enum json_ast_node_type {
	JSON_AST_NODE_TYPE_NONE,
	JSON_AST_NODE_TYPE_OBJECT,
	JSON_AST_NODE_TYPE_ARRAY,
	JSON_AST_NODE_TYPE_STRING,
	JSON_AST_NODE_TYPE_BOOL,
	JSON_AST_NODE_TYPE_NULL,
	JSON_AST_NODE_TYPE_DOUBLE,
	JSON_AST_NODE_TYPE_INT,
	JSON_AST_NODE_TYPE_UINT,
};

typedef struct json_ast_node struct_json_ast_node;
ARENA_ARRAY_DECLARE(struct_json_ast_node);

typedef struct json_ast_key_value struct_json_ast_key_value;
ARENA_ARRAY_DECLARE(struct_json_ast_key_value);

struct json_ast_node {
	struct json_ast_node *parent;
	uint32_t pad;
	enum json_ast_node_type type;
	union {
		arena_array_struct_json_ast_key_value_t object; // JSON_AST_NODE_TYPE_OBJECT  ? TODO: hash table
		arena_array_struct_json_ast_node_t array; // JSON_AST_NODE_TYPE_ARRAY
		string_t s; // JSON_AST_NODE_TYPE_STRING
		bool32_t b; // JSON_AST_NODE_TYPE_BOOL
		double d; // JSON_AST_NODE_TYPE_DOUBLE
		int64_t i; // JSON_AST_NODE_TYPE_INT
		uint64_t u; // JSON_AST_NODE_TYPE_UINT
	};
};

struct json_ast_key_value {
	string_t key;
	struct json_ast_node value;
};

ARENA_ARRAY_DEFINE(struct_json_ast_key_value)

struct json_ast {
	struct json_ast_node *current;
	struct json_ast_node root;
};

ARENA_ARRAY_DEFINE(struct_json_ast_node)

static void json_writer_init(struct json_writer *, size_t initial_bufsize);
static void json_writer_fini(struct json_writer *);
static void json_writer_reset(struct json_writer *);

// ? TODO: JSON_WRITER_OBJECT() { ... } macro
static void json_writer_object_begin(struct json_writer *);
static void json_writer_object_end(struct json_writer *);
// ? TODO: JSON_WRITER_OBJECT_KEY_VALUE(key, value) macro
static void json_writer_object_key_escape(struct json_writer *, string_t);
static void json_writer_object_key(struct json_writer *, string_t);

// ? TODO: JSON_WRITER_ARRAY() { ... } macro
static void json_writer_array_begin(struct json_writer *);
static void json_writer_array_end(struct json_writer *);

static void json_writer_null(struct json_writer *);
static void json_writer_bool(struct json_writer *, bool32_t);
static void json_writer_int(struct json_writer *, int64_t);
static void json_writer_uint(struct json_writer *, uint64_t);
static void json_writer_double(struct json_writer *, double);
static void json_writer_string_escape(struct json_writer *, string_t);
static void json_writer_string(struct json_writer *, string_t);

static void json_writer_raw(struct json_writer *, void *data, size_t len);
static void json_writer_token(struct json_writer *, struct json_token);
static void json_writer_ast_node(struct json_writer *, struct json_ast_node *);


static void json_tokener_init(struct json_tokener *);
static void json_tokener_fini(struct json_tokener *);
static void json_tokener_reset(struct json_tokener *);
static void json_tokener_set_string(struct json_tokener *, string_t);
static enum json_tokener_state_ json_tokener_next(struct json_tokener *, struct json_token *out);

static void json_ast_reset(struct json_ast *);
static enum json_tokener_state_ json_tokener_ast(struct json_tokener *, struct json_ast *,
	uint32_t stop_depth, bool32_t check_for_repeating_keys);


static void json_buffer_add_char(struct json_buffer *buffer, char c) {
	if (buffer->size == buffer->idx) {
		buffer->size = (buffer->size + 1) * 2;
		buffer->data = realloc(buffer->data, buffer->size);
	}
	buffer->data[buffer->idx++] = c;
}

static void json_buffer_add_char_unchecked(struct json_buffer *buffer, char c) {
	buffer->data[buffer->idx++] = c;
}

static void json_buffer_put_char_unchecked(struct json_buffer *buffer, size_t *idx, char c) {
	buffer->data[*idx] = c;
	*idx += 1;
}

static void json_buffer_put_string_unchecked(struct json_buffer *buffer, size_t *idx, string_t str) {
	memcpy(&buffer->data[*idx], str.s, str.len);
	*idx += str.len;
}

static void json_buffer_add_string(struct json_buffer *buffer, string_t str) {
	if ((buffer->idx + str.len) > buffer->size) {
		buffer->size = (buffer->size + str.len) * 2;
		buffer->data = realloc(buffer->data, buffer->size);
	}
	memcpy(&buffer->data[buffer->idx], str.s, str.len);
	buffer->idx += str.len;
}

static void json_buffer_add_string_unchecked(struct json_buffer *buffer, string_t str) {
	memcpy(&buffer->data[buffer->idx], str.s, str.len);
	buffer->idx += str.len;
}

static ATTRIB_FORMAT_PRINTF(2, 3) void json_buffer_add_format(struct json_buffer *buffer, const char *fmt, ...) {
	va_list args;
    va_start(args, fmt);

	char buf[512];
	size_t len = (size_t)stbsp_vsnprintf(buf, SIZEOF(buf), fmt, args);
	if ((buffer->idx + len) > buffer->size) {
		buffer->size = (buffer->size + len) * 2;
		buffer->data = realloc(buffer->data, buffer->size);
	}
	memcpy(&buffer->data[buffer->idx], buf, len);
	buffer->idx += len;

    va_end(args);
}

static void json_buffer_add_string_escaped(struct json_buffer *buf, string_t str) {
	// TODO: simd

	for (size_t i = 0; i < str.len; ++i) {
		unsigned char c = (unsigned char)str.s[i];
		switch (c) {
		case '\b':
		case '\t':
		case '\n':
		case '\f':
		case '\r':
			json_buffer_add_char(buf, '\\');
			static const char specials[] = { 'b', 't', 'n', '_'/* unreachable */, 'f', 'r' };
			json_buffer_add_char(buf, specials[c - '\b']);
			break;
		case '"':
		case '\\':
			json_buffer_add_char(buf, '\\');
			json_buffer_add_char(buf, (char)c);
			break;
		default:
			if (c <= 0x1F) {
				static const char hex_chars[] = "0123456789abcdef";
				json_buffer_add_string(buf, STRING_LITERAL("\\u00"));
				json_buffer_add_char(buf, hex_chars[c >> 4]);
				json_buffer_add_char(buf, hex_chars[c & 0xF]);
			} else {
				json_buffer_add_char(buf, (char)c);
			}
			break;
		}
	}
}

static enum json_writer_state json_writer_get_current_state(struct json_writer *writer) {
	if (writer->state.data.len > 0) {
		return stack_enum_json_writer_state_get(&writer->state);
	}

	return JSON_WRITER_STATE_ROOT;
}

static void json_writer_element(struct json_writer *writer) {
	switch (json_writer_get_current_state(writer)) {
	case JSON_WRITER_STATE_ROOT:
		break;
	case JSON_WRITER_STATE_OBJECT:
		stack_enum_json_writer_state_pop(&writer->state);
		stack_enum_json_writer_state_push(&writer->state, JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA);
		break;
	case JSON_WRITER_STATE_ARRAY:
		stack_enum_json_writer_state_pop(&writer->state);
		stack_enum_json_writer_state_push(&writer->state, JSON_WRITER_STATE_ARRAY_EXPECTING_COMMA);
		break;
	case JSON_WRITER_STATE_KEY:
		stack_enum_json_writer_state_pop(&writer->state);
		break;
	case JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA:
	case JSON_WRITER_STATE_ARRAY_EXPECTING_COMMA:
		json_buffer_add_char(&writer->buf, ',');
		break;
	default:
		ASSERT_UNREACHABLE;
	}
}

static void json_buffer_init(struct json_buffer *buffer, size_t initial_size) {
	buffer->data = malloc(initial_size);
	buffer->size = initial_size;
	buffer->idx = 0;
}

static void json_buffer_fini(struct json_buffer *buffer) {
	free(buffer->data);
}

static void json_writer_init(struct json_writer *writer, size_t initial_bufsize) {
	stack_enum_json_writer_state_init(&writer->state, 128);
	json_buffer_init(&writer->buf, initial_bufsize);
}

static void json_writer_fini(struct json_writer *writer) {
	json_buffer_fini(&writer->buf);
	stack_enum_json_writer_state_fini(&writer->state);
}

static void json_writer_reset(struct json_writer *writer) {
	writer->buf.idx = 0;
	writer->state.data.len = 0;
}

static void json_writer_object_begin(struct json_writer *writer) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state != JSON_WRITER_STATE_OBJECT) && (state != JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	json_buffer_add_char(&writer->buf, '{');
	stack_enum_json_writer_state_push(&writer->state, JSON_WRITER_STATE_OBJECT);
}

static void json_writer_object_end(struct json_writer *writer) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state == JSON_WRITER_STATE_OBJECT) || (state == JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_buffer_add_char(&writer->buf, '}');
	stack_enum_json_writer_state_pop(&writer->state);
}

static void json_writer_object_key_escape(struct json_writer *writer, string_t key) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state == JSON_WRITER_STATE_OBJECT) || (state == JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	json_buffer_add_char(&writer->buf, '"');
	json_buffer_add_string_escaped(&writer->buf, key);
	json_buffer_add_char(&writer->buf, '"');
	json_buffer_add_char(&writer->buf, ':');
	stack_enum_json_writer_state_push(&writer->state, JSON_WRITER_STATE_KEY);
}

static void json_writer_object_key(struct json_writer *writer, string_t key) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state == JSON_WRITER_STATE_OBJECT) || (state == JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	json_buffer_add_char(&writer->buf, '"');
	json_buffer_add_string(&writer->buf, key);
	json_buffer_add_char(&writer->buf, '"');
	json_buffer_add_char(&writer->buf, ':');
	stack_enum_json_writer_state_push(&writer->state, JSON_WRITER_STATE_KEY);
}

static void json_writer_array_begin(struct json_writer *writer) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state != JSON_WRITER_STATE_OBJECT) && (state != JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	json_buffer_add_char(&writer->buf, '[');
	stack_enum_json_writer_state_push(&writer->state, JSON_WRITER_STATE_ARRAY);
}

static void json_writer_array_end(struct json_writer *writer) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state == JSON_WRITER_STATE_ARRAY) || (state == JSON_WRITER_STATE_ARRAY_EXPECTING_COMMA));
	json_buffer_add_char(&writer->buf, ']');
	stack_enum_json_writer_state_pop(&writer->state);
}

static void json_writer_null(struct json_writer *writer) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state != JSON_WRITER_STATE_OBJECT) && (state != JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	json_buffer_add_string(&writer->buf, STRING_LITERAL("null"));
}

static void json_writer_bool(struct json_writer *writer, bool32_t b) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state != JSON_WRITER_STATE_OBJECT) && (state != JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	if (b) {
		json_buffer_add_string(&writer->buf, STRING_LITERAL("true"));
	} else {
		json_buffer_add_string(&writer->buf, STRING_LITERAL("false"));
	}
}

static void json_writer_int(struct json_writer *writer, int64_t i) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state != JSON_WRITER_STATE_OBJECT) && (state != JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	json_buffer_add_format(&writer->buf, "%"PRId64, i);
}

static void json_writer_uint(struct json_writer *writer, uint64_t u) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state != JSON_WRITER_STATE_OBJECT) && (state != JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	json_buffer_add_format(&writer->buf, "%"PRIu64, u);
}

static void json_writer_double(struct json_writer *writer, double d) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state != JSON_WRITER_STATE_OBJECT) && (state != JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	// TODO: isnan(d) || isinf(d) -> null
	if ((d > 0) && (d < 0)) {
		json_buffer_add_format(&writer->buf, "%.17g", d);
	} else {
		json_buffer_add_format(&writer->buf, "%.1f", d);
	}
}

static void json_writer_string_escape(struct json_writer *writer, string_t str) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state != JSON_WRITER_STATE_OBJECT) && (state != JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	json_buffer_add_char(&writer->buf, '"');
	json_buffer_add_string_escaped(&writer->buf, str);
	json_buffer_add_char(&writer->buf, '"');
}

static void json_writer_string(struct json_writer *writer, string_t str) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state != JSON_WRITER_STATE_OBJECT) && (state != JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	json_buffer_add_char(&writer->buf, '"');
	json_buffer_add_string(&writer->buf, str);
	json_buffer_add_char(&writer->buf, '"');
}

static void json_writer_raw(struct json_writer *writer, void *data, size_t len) {
	enum json_writer_state state = json_writer_get_current_state(writer);
	(void)state;
	ASSERT((state != JSON_WRITER_STATE_OBJECT) && (state != JSON_WRITER_STATE_OBJECT_EXPECTING_COMMA));
	json_writer_element(writer);
	json_buffer_add_string(&writer->buf, (string_t){
		.s = data,
		.len = len,
		.free_contents = FALSE,
		.nul_terminated = FALSE,
	});
}

static void json_writer_token(struct json_writer *writer, struct json_token token) {
	switch (token.type) {
	case JSON_TOKEN_TYPE_OBJECT_START:
		json_writer_object_begin(writer);
		break;
	case JSON_TOKEN_TYPE_OBJECT_END:
		json_writer_object_end(writer);
		break;
	case JSON_TOKEN_TYPE_KEY:
		json_writer_object_key_escape(writer, token.s); // TODO: escape only when necessary
		break;
	case JSON_TOKEN_TYPE_ARRAY_START:
		json_writer_array_begin(writer);
		break;
	case JSON_TOKEN_TYPE_ARRAY_END:
		json_writer_array_end(writer);
		break;
	case JSON_TOKEN_TYPE_STRING:
		json_writer_string_escape(writer, token.s); // TODO: escape only when necessary
		break;
	case JSON_TOKEN_TYPE_BOOL:
		json_writer_bool(writer, token.b);
		break;
	case JSON_TOKEN_TYPE_NULL:
		json_writer_null(writer);
		break;
	case JSON_TOKEN_TYPE_DOUBLE:
		json_writer_double(writer, token.d);
		break;
	case JSON_TOKEN_TYPE_INT:
		json_writer_int(writer, token.i);
		break;
	case JSON_TOKEN_TYPE_UINT:
		json_writer_uint(writer, token.u);
		break;
	default:
		ASSERT_UNREACHABLE;
	}
}

static void json_writer_ast_node(struct json_writer *writer, struct json_ast_node *node) {
	switch (node->type) {
	case JSON_AST_NODE_TYPE_OBJECT:
		json_writer_object_begin(writer);
		for (size_t i = 0; i < node->object.len; ++i) {
			struct json_ast_key_value *key_value = arena_array_struct_json_ast_key_value_get_ptr(&node->object, i);
			json_writer_object_key_escape(writer, key_value->key); // TODO: escape only when necessary
			json_writer_ast_node(writer, &key_value->value);
		}
		json_writer_object_end(writer);
		break;
	case JSON_AST_NODE_TYPE_ARRAY:
		json_writer_array_begin(writer);
		for (size_t i = 0; i < node->array.len; ++i) {
			json_writer_ast_node(writer, arena_array_struct_json_ast_node_get_ptr(&node->array, i));
		}
		json_writer_array_end(writer);
		break;
	case JSON_AST_NODE_TYPE_NONE:
		break;
	case JSON_AST_NODE_TYPE_STRING:
		json_writer_string_escape(writer, node->s); // TODO: escape only when necessary
		break;
	case JSON_AST_NODE_TYPE_BOOL:
		json_writer_bool(writer, node->b);
		break;
	case JSON_AST_NODE_TYPE_NULL:
		json_writer_null(writer);
		break;
	case JSON_AST_NODE_TYPE_DOUBLE:
		json_writer_double(writer, node->d);
		break;
	case JSON_AST_NODE_TYPE_INT:
		json_writer_int(writer, node->i);
		break;
	case JSON_AST_NODE_TYPE_UINT:
		json_writer_uint(writer, node->u);
		break;
	default:
		ASSERT_UNREACHABLE;
	}
}


static enum json_tokener_state_ json_tokener_buffer_to_string(struct json_tokener *tokener, string_t *out) {
	*out = (string_t){
		.s = arena_alloc(&tokener->allocator, tokener->buf.idx + 32),
		.len = tokener->buf.idx,
		.free_contents = FALSE,
		.nul_terminated = TRUE, // TODO: remove
	};

	size_t buf_idx = 0, str_idx = 0; // ? TODO: idx -> ptr

#if defined(__AVX2__)
	__m256i backslash = _mm256_set1_epi8('\\');
	for (;;) {
		__m256i v = _mm256_loadu_si256((__m256i_u *)&tokener->buf.data[buf_idx]);
		__m256i backslash_cmp = _mm256_cmpeq_epi8(v, backslash);
		uint32_t backslash_mask = (uint32_t)_mm256_movemask_epi8(backslash_cmp);
		_mm256_storeu_si256((__m256i_u *)&out->s[str_idx], v);
		size_t backslash_idx = (size_t)trailing_zeros(backslash_mask, 32);
		str_idx += backslash_idx;
		buf_idx += backslash_idx;
		if (LIKELY(buf_idx >= tokener->buf.idx)) {
			break;
		} else if (UNLIKELY(backslash_idx < 32)) {
			uint8_t escape_char = (uint8_t)tokener->buf.data[buf_idx + 1];
			if (UNLIKELY(escape_char == 'u')) {
				if (UNLIKELY((buf_idx + 6) > tokener->buf.idx)) {
					return JSON_TOKENER_STATE_ERROR;
				}
				uint32_t c = string_hex_16_to_uint16((string_t){
					.s = &tokener->buf.data[buf_idx + 2],
					.len = 4,
				});
				buf_idx += 6;

				if ((c >= 0xD800) && (c < 0xDC00)) { // utf16 suggorage pair
					if (UNLIKELY(((buf_idx + 6) > tokener->buf.idx) ||
							(tokener->buf.data[buf_idx] != '\\') ||
							(tokener->buf.data[buf_idx + 1] != 'u'))) {
						return JSON_TOKENER_STATE_ERROR;
					}
					uint32_t c2 = string_hex_16_to_uint16((string_t){
						.s = &tokener->buf.data[buf_idx + 2],
						.len = 4,
					});
					uint32_t low = c2 - 0xDC00;
					if (UNLIKELY(low >> 10)) {
						return JSON_TOKENER_STATE_ERROR;
					} else {
						c = (((c - 0xD800) << 10) | low) + 0x10000;
						buf_idx += 6;
					}
				} else if (UNLIKELY((c >= 0xDC00) && (c <= 0xDFFF))) {
					return JSON_TOKENER_STATE_ERROR;
				}

				if (UNLIKELY(c > 0x10FFFF)) {
					return JSON_TOKENER_STATE_ERROR;
				}

				uint8_t *dest = (uint8_t *)&out->s[str_idx];
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
				static const uint8_t map[256] = {
					['\"'] = '\"',
					['/'] = '/',
					['\\'] = '\\',
					['b'] = '\b',
					['f'] = '\f',
					['n'] = '\n',
					['r'] = '\r',
					['t'] = '\t',
				};
				if (UNLIKELY((out->s[str_idx] = (char)map[escape_char]) == 0)) {
					return JSON_TOKENER_STATE_ERROR;
				}

				str_idx += 1;
				buf_idx += 2;
			}
		}
	}
// TODO: ARM SIMD
#else
	memcpy(out->s, tokener->buf.data, tokener->buf.idx);
	for (;;) {
		char *backslash = memchr(&tokener->buf.data[buf_idx], '\\', tokener->buf.idx - buf_idx);
		if (LIKELY(backslash == NULL)) {
			break;
		} else {
			size_t backslash_idx = (size_t)(backslash - &tokener->buf.data[buf_idx]);
			buf_idx += backslash_idx;
			str_idx += backslash_idx;
			uint8_t escape_char = (uint8_t)tokener->buf.data[buf_idx + 1];
			if (UNLIKELY(escape_char == 'u')) {
				if (UNLIKELY((buf_idx + 6) > tokener->buf.idx)) {
					return JSON_TOKENER_STATE_ERROR;
				}
				uint32_t c = string_hex_16_to_uint16((string_t){
					.s = &tokener->buf.data[buf_idx + 2],
					.len = 4,
				});
				buf_idx += 6;

				if ((c >= 0xD800) && (c < 0xDC00)) { // utf16 suggorage pair
					if (UNLIKELY(((buf_idx + 6) > tokener->buf.idx) ||
							(tokener->buf.data[buf_idx] != '\\') ||
							(tokener->buf.data[buf_idx + 1] != 'u'))) {
						return JSON_TOKENER_STATE_ERROR;
					}
					uint32_t c2 = string_hex_16_to_uint16((string_t){
						.s = &tokener->buf.data[buf_idx + 2],
						.len = 4,
					});
					uint32_t low = c2 - 0xDC00;
					if (UNLIKELY(low >> 10)) {
						return JSON_TOKENER_STATE_ERROR;
					} else {
						c = (((c - 0xD800) << 10) | low) + 0x10000;
						buf_idx += 6;
					}
				} else if (UNLIKELY((c >= 0xDC00) && (c <= 0xDFFF))) {
					return JSON_TOKENER_STATE_ERROR;
				}

				if (UNLIKELY(c > 0x10FFFF)) {
					return JSON_TOKENER_STATE_ERROR;
				}

				uint8_t *dest = (uint8_t *)&out->s[str_idx];
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
				static const uint8_t map[256] = {
					['\"'] = '\"',
					['/'] = '/',
					['\\'] = '\\',
					['b'] = '\b',
					['f'] = '\f',
					['n'] = '\n',
					['r'] = '\r',
					['t'] = '\t',
				};
				if (UNLIKELY((out->s[str_idx] = (char)map[escape_char]) == 0)) {
					return JSON_TOKENER_STATE_ERROR;
				}

				str_idx += 1;
				buf_idx += 2;
			}
			memmove(&out->s[str_idx], &tokener->buf.data[buf_idx], tokener->buf.idx - buf_idx);
		}
	}
#endif

	out->len -= (buf_idx - str_idx);
	out->s[out->len] = '\0'; // TODO: remove
	tokener->buf.idx = 0;
	return JSON_TOKENER_STATE_SUCCESS;
}

static void json_tokener_init(struct json_tokener *tokener) {
	*tokener = (struct json_tokener){ 0 };
	arena_init(&tokener->allocator);
}

static void json_tokener_fini(struct json_tokener *tokener) {
	arena_fini(&tokener->allocator);
}

static void json_tokener_reset(struct json_tokener *tokener) {
	*tokener = (struct json_tokener){
		.allocator = tokener->allocator,
	};
	arena_reset(&tokener->allocator);
}

static void json_tokener_set_string(struct json_tokener *tokener, string_t str) {
	if (str.len >= (tokener->buf.size - tokener->buf.idx)) {
		char *old_data = tokener->buf.data;
		tokener->buf.size = (str.len + tokener->buf.size) * 2;
		tokener->buf.data = arena_alloc(&tokener->allocator, tokener->buf.size);
		if (old_data && tokener->buf.data) {
			memcpy(tokener->buf.data, old_data, tokener->buf.idx);
		}
	}

	tokener->pos = 0;
	tokener->str = str;
}

static enum json_tokener_state json_tokener_get_current_state(struct json_tokener *tokener) {
	if (tokener->state.data.len > 0) {
		return arena_stack_enum_json_tokener_state_get(&tokener->state);
	}

	return JSON_TOKENER_STATE_ROOT;
}

static void json_tokener_value_end(struct json_tokener *tokener) {
	arena_stack_enum_json_tokener_state_pop(&tokener->state);
	switch (json_tokener_get_current_state(tokener)) {
	case JSON_TOKENER_STATE_VALUE:
		arena_stack_enum_json_tokener_state_pop(&tokener->state);
		break;
	case JSON_TOKENER_STATE_ARRAY_EMPTY:
	case JSON_TOKENER_STATE_ARRAY:
		arena_stack_enum_json_tokener_state_pop(&tokener->state);
		arena_stack_enum_json_tokener_state_push(&tokener->state,
			&tokener->allocator, JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA);
		break;
	case JSON_TOKENER_STATE_ROOT:
		break;
	case JSON_TOKENER_STATE_NULL:
	case JSON_TOKENER_STATE_TRUE:
	case JSON_TOKENER_STATE_FALSE:
	case JSON_TOKENER_STATE_OBJECT_EMPTY:
	case JSON_TOKENER_STATE_OBJECT:
	case JSON_TOKENER_STATE_STRING:
	case JSON_TOKENER_STATE_NUMBER:
	case JSON_TOKENER_STATE_KEY:
	case JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
	case JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
	default:
		ASSERT_UNREACHABLE;
	}

	tokener->buf.idx = 0;
}

static enum json_tokener_state_ json_tokener_next(struct json_tokener *tokener, struct json_token *out) {
	enum json_tokener_state state = json_tokener_get_current_state(tokener);
	switch (state) {
	case JSON_TOKENER_STATE_STRING:
_string: {
		if (UNLIKELY(tokener->pos == tokener->str.len)) {
			return JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
		}
		if (LIKELY(tokener->buf.idx == 0)) {
			char c = tokener->str.s[tokener->pos++];
			if (UNLIKELY(c == '\"')) {
				goto _string_end;
			} else if (UNLIKELY(c == '\\')) {
				tokener->last_escape_idx = 0;
				json_buffer_add_char_unchecked(&tokener->buf, '\\');
			} else {
				tokener->last_escape_idx = SIZE_MAX;
				json_buffer_add_char_unchecked(&tokener->buf, c);
			}
		}

#if defined(__AVX2__)
		if (LIKELY((tokener->pos + 32) <= tokener->str.len)) {
			__m256i quote = _mm256_set1_epi8('"');
			__m256i backslash = _mm256_set1_epi8('\\');
			do {
				__m256i v = _mm256_loadu_si256((__m256i_u *)&tokener->str.s[tokener->pos]);
				__m256i quote_cmp = _mm256_cmpeq_epi8(v, quote);
				__m256i backslash_cmp = _mm256_cmpeq_epi8(v, backslash);
				uint32_t quote_mask = (uint32_t)_mm256_movemask_epi8(quote_cmp);
				uint32_t backslash_mask = (uint32_t)_mm256_movemask_epi8(backslash_cmp);
				_mm256_storeu_si256((__m256i_u *)&tokener->buf.data[tokener->buf.idx], v);
				if (LIKELY((backslash_mask - 1) & quote_mask)) {
					size_t quote_idx = (size_t)trailing_zeros(quote_mask, 0);
					tokener->buf.idx += quote_idx;
					tokener->pos += quote_idx + 1;
					if (LIKELY(tokener->last_escape_idx != (tokener->buf.idx - 1))) {
						goto _string_end;
					}
					tokener->buf.idx += 1;
				} else if (UNLIKELY((quote_mask - 1) & backslash_mask)) {
					size_t backslash_idx = (size_t)trailing_zeros(backslash_mask, 0);
					tokener->buf.idx += backslash_idx;
					tokener->pos += backslash_idx + 1;
					if (LIKELY(tokener->last_escape_idx != (tokener->buf.idx - 1))) {
						tokener->last_escape_idx = tokener->buf.idx;
					}
					tokener->buf.idx += 1;
				} else {
					tokener->buf.idx += 32;
					tokener->pos += 32;
				}
			} while (LIKELY((tokener->pos + 32) <= tokener->str.len));
		}
#endif // __AVX2__
// TODO: ARM SIMD

		for (;;) {
			if (UNLIKELY(tokener->pos == tokener->str.len)) {
				return JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
			}

			char c = tokener->str.s[tokener->pos++];
			if (UNLIKELY((c == '\"') && (tokener->last_escape_idx != (tokener->buf.idx - 1)))) {
				goto _string_end;
			} else if (UNLIKELY((c == '\\') && (tokener->last_escape_idx != (tokener->buf.idx - 1)))) {
				tokener->last_escape_idx = tokener->buf.idx;
			}

			json_buffer_add_char_unchecked(&tokener->buf, c);
		}

_string_end:
		arena_stack_enum_json_tokener_state_pop(&tokener->state);
		switch (json_tokener_get_current_state(tokener)) {
		case JSON_TOKENER_STATE_OBJECT_EMPTY:
		case JSON_TOKENER_STATE_OBJECT:
			arena_stack_enum_json_tokener_state_pop(&tokener->state);
			arena_stack_enum_json_tokener_state_push(&tokener->state,
				&tokener->allocator, JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA);
			arena_stack_enum_json_tokener_state_push(&tokener->state,
				&tokener->allocator, JSON_TOKENER_STATE_KEY);
			out->type = JSON_TOKEN_TYPE_KEY;
			break;
		case JSON_TOKENER_STATE_VALUE:
			arena_stack_enum_json_tokener_state_pop(&tokener->state);
			out->type = JSON_TOKEN_TYPE_STRING;
			break;
		case JSON_TOKENER_STATE_ARRAY_EMPTY:
		case JSON_TOKENER_STATE_ARRAY:
			arena_stack_enum_json_tokener_state_pop(&tokener->state);
			arena_stack_enum_json_tokener_state_push(&tokener->state,
				&tokener->allocator, JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA);
			out->type = JSON_TOKEN_TYPE_STRING;
			break;
		case JSON_TOKENER_STATE_ROOT:
			break;
		case JSON_TOKENER_STATE_NULL:
		case JSON_TOKENER_STATE_TRUE:
		case JSON_TOKENER_STATE_FALSE:
		case JSON_TOKENER_STATE_STRING:
		case JSON_TOKENER_STATE_NUMBER:
		case JSON_TOKENER_STATE_KEY:
		case JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
		case JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
		default:
			ASSERT_UNREACHABLE;
		}

		out->depth = tokener->depth;
		return json_tokener_buffer_to_string(tokener, &out->s);
	}
	case JSON_TOKENER_STATE_NUMBER:
_number: {
		string_t str;

#if defined(__AVX2__)
		if (LIKELY((tokener->pos + 32) <= tokener->str.len)) {
			__m256i v = _mm256_loadu_si256((__m256i_u *)&tokener->str.s[tokener->pos]);
			// TODO: lut
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
			if (LIKELY(mask)) {
				size_t idx = (size_t)trailing_zeros(mask, 0);
				str.s = tokener->buf.data;
				str.len = idx;
				tokener->pos += idx;
				goto _parse_number;
			} else {
				return JSON_TOKENER_STATE_ERROR;
			}
		}
#endif // __AVX2__
// TODO: ARM SIMD

		for (;;) {
			if (UNLIKELY(tokener->pos == tokener->str.len)) {
				return JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
			}
			char c = tokener->str.s[tokener->pos];
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
				json_buffer_add_char_unchecked(&tokener->buf, c);
				tokener->pos++;
			}
		}

_parse_number:
		if (string_to_uint64(str, &out->u)) {
			out->type = JSON_TOKEN_TYPE_UINT;
		} else if (string_to_int64(str, &out->i)) {
			out->type = JSON_TOKEN_TYPE_INT;
		} else {
			// TODO: string to double conversion function with length
			json_buffer_add_char_unchecked(&tokener->buf, '\0');
			char *end = NULL;
			errno = 0;
			out->d = strtod(str.s, &end);
			if (LIKELY((end == &tokener->buf.data[tokener->buf.idx - 1]) && (errno == 0))) {
				out->type = JSON_TOKEN_TYPE_DOUBLE;
			} else {
				return JSON_TOKENER_STATE_ERROR;
			}
		}
		out->depth = tokener->depth;

		json_tokener_value_end(tokener);

		return JSON_TOKENER_STATE_SUCCESS;
	}
	case JSON_TOKENER_STATE_NULL:
_null: {
		size_t bytes_available = (tokener->str.len - tokener->pos);
		size_t bytes_needed = (4 - tokener->buf.idx);
		if (LIKELY(bytes_available >= bytes_needed)) {
			memcpy(&tokener->buf.data[tokener->buf.idx], &tokener->str.s[tokener->pos], bytes_needed);
			tokener->pos += bytes_needed;
		} else {
			json_buffer_add_string_unchecked(&tokener->buf, (string_t){
				.s = &tokener->str.s[tokener->pos],
				.len = bytes_available,
			});
			//tokener->pos += bytes_available;
			return JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
		}

		if (UNLIKELY(!string_equal(STRING_LITERAL("null"), (string_t){ .s = tokener->buf.data, .len = 4 }))) {
			return JSON_TOKENER_STATE_ERROR;
		}

		json_tokener_value_end(tokener);

		*out = (struct json_token){
			.type = JSON_TOKEN_TYPE_NULL,
			.depth = tokener->depth,
		};
		return JSON_TOKENER_STATE_SUCCESS;
	}
	case JSON_TOKENER_STATE_TRUE:
_true: {
		size_t bytes_available = (tokener->str.len - tokener->pos);
		size_t bytes_needed = (4 - tokener->buf.idx);
		if (LIKELY(bytes_available >= bytes_needed)) {
			memcpy(&tokener->buf.data[tokener->buf.idx], &tokener->str.s[tokener->pos], bytes_needed);
			tokener->pos += bytes_needed;
		} else {
			json_buffer_add_string_unchecked(&tokener->buf, (string_t){
				.s = &tokener->str.s[tokener->pos],
				.len = bytes_available,
			});
			//tokener->pos += bytes_available;
			return JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
		}

		if (UNLIKELY(!string_equal(STRING_LITERAL("true"), (string_t){ .s = tokener->buf.data, .len = 4 }))) {
			return JSON_TOKENER_STATE_ERROR;
		}

		json_tokener_value_end(tokener);

		*out = (struct json_token){
			.type = JSON_TOKEN_TYPE_BOOL,
			.b = TRUE,
			.depth = tokener->depth,
		};
		return JSON_TOKENER_STATE_SUCCESS;
	}
	case JSON_TOKENER_STATE_FALSE:
_false: {
		size_t bytes_available = (tokener->str.len - tokener->pos);
		size_t bytes_needed = (5 - tokener->buf.idx);
		if (LIKELY(bytes_available >= bytes_needed)) {
			memcpy(&tokener->buf.data[tokener->buf.idx], &tokener->str.s[tokener->pos], bytes_needed);
			tokener->pos += bytes_needed;
		} else {
			json_buffer_add_string_unchecked(&tokener->buf, (string_t){
				.s = &tokener->str.s[tokener->pos],
				.len = bytes_available,
			});
			//tokener->pos += bytes_available;
			return JSON_TOKENER_STATE_MORE_DATA_EXPECTED;
		}

		if (UNLIKELY(!string_equal(STRING_LITERAL("false"), (string_t){ .s = tokener->buf.data, .len = 5 }))) {
			return JSON_TOKENER_STATE_ERROR;
		}

		json_tokener_value_end(tokener);

		*out = (struct json_token){
			.type = JSON_TOKEN_TYPE_BOOL,
			.b = FALSE,
			.depth = tokener->depth,
		};
		return JSON_TOKENER_STATE_SUCCESS;
	}
	case JSON_TOKENER_STATE_ROOT:
	case JSON_TOKENER_STATE_OBJECT:
	case JSON_TOKENER_STATE_OBJECT_EMPTY:
	case JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
	case JSON_TOKENER_STATE_KEY:
	case JSON_TOKENER_STATE_VALUE:
	case JSON_TOKENER_STATE_ARRAY:
	case JSON_TOKENER_STATE_ARRAY_EMPTY:
	case JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
		break;
	default:
		ASSERT_UNREACHABLE;
	}

	for ( ; tokener->pos < tokener->str.len; ++tokener->pos) {
		char c = tokener->str.s[tokener->pos];
		switch (c) {
		case '{':
			switch (state) {
			case JSON_TOKENER_STATE_OBJECT:
			case JSON_TOKENER_STATE_OBJECT_EMPTY:
			case JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case JSON_TOKENER_STATE_KEY:
			case JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return JSON_TOKENER_STATE_ERROR;
			case JSON_TOKENER_STATE_VALUE:
				arena_stack_enum_json_tokener_state_pop(&tokener->state);
				arena_stack_enum_json_tokener_state_push(&tokener->state, &tokener->allocator,
					JSON_TOKENER_STATE_OBJECT_EMPTY);
				break;
			case JSON_TOKENER_STATE_ARRAY_EMPTY:
			case JSON_TOKENER_STATE_ARRAY:
				arena_stack_enum_json_tokener_state_pop(&tokener->state);
				arena_stack_enum_json_tokener_state_push(&tokener->state,
					&tokener->allocator, JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA);
				ATTRIB_FALLTHROUGH;
			case JSON_TOKENER_STATE_ROOT:
				arena_stack_enum_json_tokener_state_push(&tokener->state, &tokener->allocator,
					JSON_TOKENER_STATE_OBJECT_EMPTY);
				break;
			case JSON_TOKENER_STATE_NULL:
			case JSON_TOKENER_STATE_TRUE:
			case JSON_TOKENER_STATE_FALSE:
			case JSON_TOKENER_STATE_STRING:
			case JSON_TOKENER_STATE_NUMBER:
			default:
				ASSERT_UNREACHABLE;
			}
			tokener->pos++;
			tokener->depth++;
			*out = (struct json_token){
				.depth = tokener->depth,
				.type = JSON_TOKEN_TYPE_OBJECT_START,
			};
			return JSON_TOKENER_STATE_SUCCESS;
		case '[':
			switch (state) {
			case JSON_TOKENER_STATE_OBJECT:
			case JSON_TOKENER_STATE_OBJECT_EMPTY:
			case JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case JSON_TOKENER_STATE_KEY:
			case JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return JSON_TOKENER_STATE_ERROR;
			case JSON_TOKENER_STATE_VALUE:
				arena_stack_enum_json_tokener_state_pop(&tokener->state);
				arena_stack_enum_json_tokener_state_push(&tokener->state, &tokener->allocator,
					JSON_TOKENER_STATE_ARRAY_EMPTY);
				break;
			case JSON_TOKENER_STATE_ARRAY_EMPTY:
			case JSON_TOKENER_STATE_ARRAY:
				arena_stack_enum_json_tokener_state_pop(&tokener->state);
				arena_stack_enum_json_tokener_state_push(&tokener->state,
					&tokener->allocator, JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA);
				ATTRIB_FALLTHROUGH;
			case JSON_TOKENER_STATE_ROOT:
				arena_stack_enum_json_tokener_state_push(&tokener->state, &tokener->allocator,
					JSON_TOKENER_STATE_ARRAY_EMPTY);
				break;
			case JSON_TOKENER_STATE_NULL:
			case JSON_TOKENER_STATE_TRUE:
			case JSON_TOKENER_STATE_FALSE:
			case JSON_TOKENER_STATE_STRING:
			case JSON_TOKENER_STATE_NUMBER:
			default:
				ASSERT_UNREACHABLE;
			}
			tokener->pos++;
			tokener->depth++;
			*out = (struct json_token){
				.depth = tokener->depth,
				.type = JSON_TOKEN_TYPE_ARRAY_START,
			};
			return JSON_TOKENER_STATE_SUCCESS;
		case '}':
			if (UNLIKELY((state != JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA) && (state != JSON_TOKENER_STATE_OBJECT_EMPTY))) {
				return JSON_TOKENER_STATE_ERROR;
			}
			arena_stack_enum_json_tokener_state_pop(&tokener->state);
			tokener->pos++;
			tokener->depth--;
			*out = (struct json_token){
				.depth = tokener->depth,
				.type = JSON_TOKEN_TYPE_OBJECT_END,
			};
			return JSON_TOKENER_STATE_SUCCESS;
		case ']':
			if (UNLIKELY((state != JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA) && (state != JSON_TOKENER_STATE_ARRAY_EMPTY))) {
				return JSON_TOKENER_STATE_ERROR;
			}
			arena_stack_enum_json_tokener_state_pop(&tokener->state);
			tokener->pos++;
			tokener->depth--;
			*out = (struct json_token){
				.depth = tokener->depth,
				.type = JSON_TOKEN_TYPE_ARRAY_END,
			};
			return JSON_TOKENER_STATE_SUCCESS;
		case '\"': {
			if (UNLIKELY((state == JSON_TOKENER_STATE_KEY) || (state == JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA)
					|| (state == JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA))) {
				return JSON_TOKENER_STATE_ERROR;
			}
			arena_stack_enum_json_tokener_state_push(
				&tokener->state, &tokener->allocator, JSON_TOKENER_STATE_STRING);
			tokener->pos++;
			goto _string;
		}
		case ':':
			if (UNLIKELY(state != JSON_TOKENER_STATE_KEY)) {
				return JSON_TOKENER_STATE_ERROR;
			}
			arena_stack_enum_json_tokener_state_pop(&tokener->state);
			arena_stack_enum_json_tokener_state_push(&tokener->state,
				&tokener->allocator, JSON_TOKENER_STATE_VALUE);
			state = JSON_TOKENER_STATE_VALUE;
			continue;
		case ',':
			switch (state) {
			case JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
				arena_stack_enum_json_tokener_state_pop(&tokener->state);
				arena_stack_enum_json_tokener_state_push(&tokener->state,
					&tokener->allocator, JSON_TOKENER_STATE_OBJECT);
				state = JSON_TOKENER_STATE_OBJECT;
				continue;
			case JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				arena_stack_enum_json_tokener_state_pop(&tokener->state);
				arena_stack_enum_json_tokener_state_push(&tokener->state,
					&tokener->allocator, JSON_TOKENER_STATE_ARRAY);
				state = JSON_TOKENER_STATE_ARRAY;
				continue;
			case JSON_TOKENER_STATE_ROOT:
			case JSON_TOKENER_STATE_OBJECT:
			case JSON_TOKENER_STATE_OBJECT_EMPTY:
			case JSON_TOKENER_STATE_KEY:
			case JSON_TOKENER_STATE_VALUE:
			case JSON_TOKENER_STATE_ARRAY:
			case JSON_TOKENER_STATE_ARRAY_EMPTY:
				return JSON_TOKENER_STATE_ERROR;
			case JSON_TOKENER_STATE_STRING:
			case JSON_TOKENER_STATE_NUMBER:
			case JSON_TOKENER_STATE_NULL:
			case JSON_TOKENER_STATE_TRUE:
			case JSON_TOKENER_STATE_FALSE:
			default:
				ASSERT_UNREACHABLE;
			}
		case 'n':
			switch (state) {
			case JSON_TOKENER_STATE_ROOT:
			case JSON_TOKENER_STATE_VALUE:
			case JSON_TOKENER_STATE_ARRAY:
			case JSON_TOKENER_STATE_ARRAY_EMPTY:
				arena_stack_enum_json_tokener_state_push(&tokener->state,
					&tokener->allocator, JSON_TOKENER_STATE_NULL);
				goto _null;
			case JSON_TOKENER_STATE_OBJECT:
			case JSON_TOKENER_STATE_OBJECT_EMPTY:
			case JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case JSON_TOKENER_STATE_KEY:
			case JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return JSON_TOKENER_STATE_ERROR;
			case JSON_TOKENER_STATE_STRING:
			case JSON_TOKENER_STATE_NUMBER:
			case JSON_TOKENER_STATE_NULL:
			case JSON_TOKENER_STATE_TRUE:
			case JSON_TOKENER_STATE_FALSE:
			default:
				ASSERT_UNREACHABLE;
			}
		case 't':
			switch (state) {
			case JSON_TOKENER_STATE_ROOT:
			case JSON_TOKENER_STATE_VALUE:
			case JSON_TOKENER_STATE_ARRAY:
			case JSON_TOKENER_STATE_ARRAY_EMPTY:
				arena_stack_enum_json_tokener_state_push(&tokener->state,
					&tokener->allocator, JSON_TOKENER_STATE_TRUE);
				goto _true;
			case JSON_TOKENER_STATE_OBJECT:
			case JSON_TOKENER_STATE_OBJECT_EMPTY:
			case JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case JSON_TOKENER_STATE_KEY:
			case JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return JSON_TOKENER_STATE_ERROR;
			case JSON_TOKENER_STATE_STRING:
			case JSON_TOKENER_STATE_NUMBER:
			case JSON_TOKENER_STATE_NULL:
			case JSON_TOKENER_STATE_TRUE:
			case JSON_TOKENER_STATE_FALSE:
			default:
				ASSERT_UNREACHABLE;
			}
		case 'f':
			switch (state) {
			case JSON_TOKENER_STATE_ROOT:
			case JSON_TOKENER_STATE_VALUE:
			case JSON_TOKENER_STATE_ARRAY:
			case JSON_TOKENER_STATE_ARRAY_EMPTY:
				arena_stack_enum_json_tokener_state_push(&tokener->state,
					&tokener->allocator, JSON_TOKENER_STATE_FALSE);
				goto _false;
			case JSON_TOKENER_STATE_OBJECT:
			case JSON_TOKENER_STATE_OBJECT_EMPTY:
			case JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case JSON_TOKENER_STATE_KEY:
			case JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return JSON_TOKENER_STATE_ERROR;
			case JSON_TOKENER_STATE_STRING:
			case JSON_TOKENER_STATE_NUMBER:
			case JSON_TOKENER_STATE_NULL:
			case JSON_TOKENER_STATE_TRUE:
			case JSON_TOKENER_STATE_FALSE:
			default:
				ASSERT_UNREACHABLE;
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
			case JSON_TOKENER_STATE_ROOT:
			case JSON_TOKENER_STATE_VALUE:
			case JSON_TOKENER_STATE_ARRAY:
			case JSON_TOKENER_STATE_ARRAY_EMPTY:
				arena_stack_enum_json_tokener_state_push(&tokener->state,
					&tokener->allocator, JSON_TOKENER_STATE_NUMBER);
				goto _number;
			case JSON_TOKENER_STATE_OBJECT:
			case JSON_TOKENER_STATE_OBJECT_EMPTY:
			case JSON_TOKENER_STATE_OBJECT_EXPECTING_COMMA:
			case JSON_TOKENER_STATE_KEY:
			case JSON_TOKENER_STATE_ARRAY_EXPECTING_COMMA:
				return JSON_TOKENER_STATE_ERROR;
			case JSON_TOKENER_STATE_STRING:
			case JSON_TOKENER_STATE_NUMBER:
			case JSON_TOKENER_STATE_NULL:
			case JSON_TOKENER_STATE_TRUE:
			case JSON_TOKENER_STATE_FALSE:
			default:
				ASSERT_UNREACHABLE;
			}
		case '\t':
		case '\r':
		case '\n':
		case ' ':
			continue;
		default:
			return JSON_TOKENER_STATE_ERROR;
		}
	}

	return ((json_tokener_get_current_state(tokener) == JSON_TOKENER_STATE_ROOT)
		? JSON_TOKENER_STATE_EOF
		: JSON_TOKENER_STATE_MORE_DATA_EXPECTED);
}

static void json_ast_reset(struct json_ast *ast) {
	ast->root = (struct json_ast_node){ 0 };
	ast->current = &ast->root;
}

static enum json_tokener_state_ json_tokener_ast(struct json_tokener *tokener, struct json_ast *ast,
		uint32_t stop_depth, bool32_t check_for_repeating_keys) {
	// ? TODO: min/max depth
	enum json_tokener_state_ state;
	struct json_token token = { 0 };
	struct json_ast_node node = { 0 };
	while ((state = json_tokener_next(tokener, &token)) == JSON_TOKENER_STATE_SUCCESS) {
		if (UNLIKELY(token.depth == stop_depth)) {
			return JSON_TOKENER_STATE_SUCCESS;
		}
		switch (token.type) {
		case JSON_TOKEN_TYPE_OBJECT_START:
			node.type = JSON_AST_NODE_TYPE_OBJECT;
			node.object = (arena_array_struct_json_ast_key_value_t){ 0 };
			break;
		case JSON_TOKEN_TYPE_ARRAY_START:
			node.type = JSON_AST_NODE_TYPE_ARRAY;
			node.array = (arena_array_struct_json_ast_node_t){ 0 };
			break;
		case JSON_TOKEN_TYPE_OBJECT_END:
		case JSON_TOKEN_TYPE_ARRAY_END:
			ASSERT((ast->current->type == JSON_AST_NODE_TYPE_OBJECT)
				|| (ast->current->type == JSON_AST_NODE_TYPE_ARRAY));
			ast->current = ast->current->parent;
			continue;
		case JSON_TOKEN_TYPE_KEY:
			ASSERT(ast->current->type == JSON_AST_NODE_TYPE_OBJECT);
			if (check_for_repeating_keys) {
				for (size_t i = 0; i < ast->current->object.len; ++i) {
					struct json_ast_key_value *key_value = arena_array_struct_json_ast_key_value_get_ptr(
						&ast->current->object, i);
					if (UNLIKELY(string_equal(token.s, key_value->key))) {
						return JSON_TOKENER_STATE_ERROR;
					}
				}
			}
			arena_array_struct_json_ast_key_value_add(&ast->current->object, &tokener->allocator,
				(struct json_ast_key_value){
					.key = token.s,
				}
			);
			continue;
		case JSON_TOKEN_TYPE_STRING:
			node.type = JSON_AST_NODE_TYPE_STRING;
			node.s = token.s;
			break;
		case JSON_TOKEN_TYPE_BOOL:
			node.type = JSON_AST_NODE_TYPE_BOOL;
			node.b = token.b;
			break;
		case JSON_TOKEN_TYPE_NULL:
			node.type = JSON_AST_NODE_TYPE_NULL;
			break;
		case JSON_TOKEN_TYPE_DOUBLE:
			node.type = JSON_AST_NODE_TYPE_DOUBLE;
			node.d = token.d;
			break;
		case JSON_TOKEN_TYPE_INT:
			node.type = JSON_AST_NODE_TYPE_INT;
			node.i = token.i;
			break;
		case JSON_TOKEN_TYPE_UINT:
			node.type = JSON_AST_NODE_TYPE_UINT;
			node.u = token.u;
			break;
		default:
			ASSERT_UNREACHABLE;
		}
		node.parent = ast->current;

		struct json_ast_node *current;
		if (ast->current->type == JSON_AST_NODE_TYPE_OBJECT) {
			current = &arena_array_struct_json_ast_key_value_get_ptr( &ast->current->object,
				ast->current->object.len - 1)->value;
			*current = node;
		} else if (ast->current->type == JSON_AST_NODE_TYPE_ARRAY) {
			current = arena_array_struct_json_ast_node_add(&ast->current->array, &tokener->allocator, node);
		} else {
			ast->root = node;
			current = &ast->root;
		}

		if ((token.type == JSON_TOKEN_TYPE_OBJECT_START) || (token.type == JSON_TOKEN_TYPE_ARRAY_START)) {
			ast->current = current;
		}
	}
	ASSERT(state != JSON_TOKENER_STATE_SUCCESS);
	return state;
}

#endif // JSON_H
