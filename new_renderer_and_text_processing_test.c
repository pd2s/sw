#define _XOPEN_SOURCE 700
/*#define SU_STRIP_PREFIXES*/
#define SU_WITH_SIMD 1
#define SU_IMPLEMENTATION
#include "sutil.h"

#define SW_IMPLEMENTATION
#include "swidgets.h"

#include <locale.h>
#include <signal.h>

#include <ft2build.h>
#include FT_MODULE_H
#include FT_OTSVG_H
#include FT_TRUETYPE_TABLES_H
#include FT_SYNTHESIS_H
#include <harfbuzz/hb-ft.h>

typedef enum sw_glyph_load_flag {
    SW_GLYPH_LOAD_FLAG_DEFAULT = 0,
    SW_GLYPH_LOAD_FLAG_COLOR = (1L << 20),
    SW_GLYPH_LOAD_FLAG_NO_HINTING = (1L << 1),
    SW_GLYPH_LOAD_FLAG_NO_AUTOHINT = (1L << 15),
    SW_GLYPH_LOAD_FLAG_FORCE_AUTOHINT = (1L << 5),
    SW_GLYPH_LOAD_FLAG_NO_BITMAP = (1L << 3),
    SW_GLYPH_LOAD_FLAG_SBITS_ONLY = (1L << 14),
    SW_GLYPH_LOAD_FLAG_NO_SVG = (1L << 24)
    /* ? TODO: VERTICAL_LAYOUT COMPUTE_METRICS BITMAP_METRICS_ONLY */
} sw_glyph_load_flag_t;

typedef uint32_t sw_glyph_load_flag_mask_t;

typedef enum sw_glyph_render_mode {
    SW_GLYPH_RENDER_MODE_NORMAL = 0,
    SW_GLYPH_RENDER_MODE_MONO = 2,
    SW_GLYPH_RENDER_MODE_LCD = 3,
    SW_GLYPH_RENDER_MODE_LCD_V = 4
    /* TODO: SDF */
} sw_glyph_render_mode_t;

typedef enum sw_glyph_hinting_algorithm {
    SW_GLYPH_HINTING_ALGORITHM_NORMAL = 0,
    SW_GLYPH_HINTING_ALGORITHM_LIGHT = 0x10000,
    SW_GLYPH_HINTING_ALGORITHM_MONO = 0x20000,
    SW_GLYPH_HINTING_ALGORITHM_LCD = 0x30000,
    SW_GLYPH_HINTING_ALGORITHM_LCD_V = 0x40000
} sw_glyph_hinting_algorithm_t;

typedef struct sw__cached_glyph {
    uint8_t *data;
    size_t data_size;
    uint32_t width, height, pitch;
    FT_Pixel_Mode pixel_mode;
    int32_t left, top;
    float scale_factor; /* ? TODO: scale FT_PIXEL_MODE_BGRA right away */
    uint32_t pixel_size;
    sw_glyph_render_mode_t render_mode;
    sw_glyph_load_flag_mask_t load_flags;
    /* ? TODO: cache hb_feature_t */
    int64_t synthetic_slant_x, synthetic_slant_y;
    int64_t synthetic_weight_dx, synthetic_weight_dy;
} sw__cached_glyph_t;

typedef struct sw__font {
    su_allocator_t alloc; /* must be first */
    su_arena_t arena;
    su_fat_ptr_t key;
    su_bool32_t occupied;
    SU_PAD32;
    FT_Face face;
    hb_font_t *hb_font;
    size_t idx;
    uint32_t pixel_size;
    sw_glyph_load_flag_mask_t glyph_load_flags;
    hb_feature_t *features;
    size_t features_count;
    /* ? TODO: cache multiple sizes / render modes / load flags */
    sw__cached_glyph_t **glyph_cache; /* size: FT_Face.num_glyphs */

    /* ? TODO: make public */
    int32_t underline_offset, strikeout_offset; /* from baseline */
    uint32_t underline_thickness, strikeout_thickness;
} sw__font_t;

typedef struct sw__font_hash_table {
    sw__font_t *items;
    size_t capacity;
} sw__font_hash_table_t;

typedef struct sw_font {
    su_fat_ptr_t data;
    size_t idx;
    uint32_t pixel_size;
    sw_glyph_render_mode_t glyph_render_mode;
    /* TODO: lcd filter params */
    sw_glyph_load_flag_mask_t glyph_load_flags;
    sw_glyph_hinting_algorithm_t hinting_algorithm;
    su_string_t *features; /* ? TODO: parsed form */
    size_t features_count;
    /* TODO: fontvariations */
    int64_t synthetic_slant_x, synthetic_slant_y;
    int64_t synthetic_weight_dx, synthetic_weight_dy;
    void *priv;
} sw_font_t;

typedef struct sw_text {
    uint32_t *codepoints; /* must be valid utf32, there is no validator at this moment */
    size_t codepoints_count;
} sw_text_t;

typedef struct sw_layout_block_text {
    sw_text_t text; /* ? TODO: utf8 option */
    sw_font_t *fonts; /* fallback order */
    size_t fonts_count;
    su_bool32_t vertical;
    uint32_t letter_spacing;
    SU_PAD32;
    /* ? TODO: sw_color_t */
    sw_color_argb32_t text_color;
    sw_color_argb32_t underline_color;
    sw_color_argb32_t strikeout_color;
} sw_layout_block_text_t;

typedef struct sw__glyph {
    su_bool32_t valid;
    uint32_t idx;
    int32_t x_advance;
    int32_t y_advance;
    int32_t x_offset;
    int32_t y_offset;
} sw__glyph_t;

typedef struct sw_glyph {
    uint32_t codepoint;
    uint32_t cluster;
    int32_t x, y;
    uint32_t width, height;
    sw_font_t *font;
    SW__PRIVATE_FIELDS(sizeof(sw__glyph_t));
} sw_glyph_t;

typedef struct sw__partial_run {
    size_t start_idx, len;
    hb_script_t script;
    SU_PAD32;
} sw__partial_run_t;

typedef struct sw__text_run {
    sw_glyph_t *glyphs;
    size_t glyphs_count;
    uint32_t width, height;
    su_bool32_t vertical;
    uint32_t clusters_count;
} sw__text_run_t;

typedef struct sw__text_run_cache {
    sw_text_t key;
    su_bool32_t occupied;
    SU_PAD32;
    sw__text_run_t text_run; /* ? TODO: cache multiple runs for different font combinations */
    FT_Face *faces;
    size_t faces_count;
} sw__text_run_cache_t;

typedef struct sw__text_run_cache_hash_table {
    sw__text_run_cache_t *items;
    size_t capacity;
} sw__text_run_cache_hash_table_t;

typedef struct state {
    FT_Library ft;
    hb_buffer_t *hb_buf;
    hb_unicode_funcs_t *hb_unicode_funcs;
    sw__font_hash_table_t font_cache;
    sw__text_run_cache_hash_table_t text_cache;
    su_arena_t font_cache_arena;
    su_arena_t text_cache_arena;
    su_arena_t scratch_arena;
    su_bool32_t running;
    SU_PAD32;
} state_t;


typedef struct resvg_render_ft_state {
    resvg_render_tree *tree;
    resvg_transform transform;
    char id[32];
} resvg_render_ft_state_t;

typedef struct  TT_SBit_MetricsRec_
{
  FT_UShort  height;
  FT_UShort  width;
  FT_Short   horiBearingX;
  FT_Short   horiBearingY;
  FT_UShort  horiAdvance;
  FT_Short   vertBearingX;
  FT_Short   vertBearingY;
  FT_UShort  vertAdvance;
} TT_SBit_MetricsRec, *TT_SBit_Metrics;

FT_Error Load_SBit_Png(FT_GlyphSlot slot, FT_Int x_offset, FT_Int y_offset, FT_Int pix_bits,
        TT_SBit_Metrics metrics, FT_Memory memory, FT_Byte *data, FT_UInt png_len,
        FT_Bool populate_map_and_metrics, FT_Bool metrics_only);

void *hb_malloc_impl(size_t size);
void *hb_calloc_impl(size_t nmemb, size_t size);
void *hb_realloc_impl(void *ptr, size_t size);
void hb_free_impl(void *ptr);

static resvg_render_ft_state_t resvg_render_ft_state;

static sw_context_t sw;
static state_t state;

static const su_allocator_t gp_alloc = { su_libc_alloc, su_libc_free };

static size_t sw__stbds_hash_text(sw_text_t text) {
    su_fat_ptr_t data;
    data.ptr = text.codepoints;
    data.len = (sizeof(*text.codepoints) * text.codepoints_count);
    return su_stbds_hash(data);
}

static su_bool32_t sw__text_equal(sw_text_t a, sw_text_t b) {
    if (a.codepoints_count != b.codepoints_count) {
        return SU_FALSE;
    }
    return (SU_MEMCMP(a.codepoints, b.codepoints, a.codepoints_count) == 0);
}

static void *sw__font_cache_alloc_alloc(const su_allocator_t *alloc, size_t size, size_t alignment) {
    SU_NOTUSED(alloc);
    return su_arena_alloc(&state.font_cache_arena, &gp_alloc, size, alignment);
}

static void *sw__text_cache_alloc_alloc(const su_allocator_t *alloc, size_t size, size_t alignment) {
    SU_NOTUSED(alloc);
    return su_arena_alloc(&state.text_cache_arena, &gp_alloc, size, alignment);
}

static void sw__noop_free(const su_allocator_t *alloc, void *ptr) {
    SU_NOTUSED(alloc); SU_NOTUSED(ptr);
}

static const su_allocator_t font_cache_alloc = { sw__font_cache_alloc_alloc, sw__noop_free };
static const su_allocator_t text_cache_alloc = { sw__text_cache_alloc_alloc, sw__noop_free };

static void *scratch_alloc_alloc(const su_allocator_t *alloc, size_t size, size_t alignment) {
    void *ret = su_arena_alloc(&state.scratch_arena, &gp_alloc, size, alignment);
    SU_NOTUSED(alloc);
    return ret;
}

static void scratch_alloc_free(const su_allocator_t *alloc, void *ptr) {
    SU_NOTUSED(alloc); SU_NOTUSED(ptr);
}

static const su_allocator_t scratch_alloc = { scratch_alloc_alloc, scratch_alloc_free };

static void *alloc_alloc_ft(FT_Memory memory, long size) {
    void *ret;

    SU_NOTUSED(memory);
    
    if (size <= 0) {
        return NULL;
    }

    SU_ALLOCTSA(ret, &gp_alloc, (size_t)size, 32);

    return ret;
}

static void alloc_free_ft(FT_Memory memory, void *ptr) {
    SU_NOTUSED(memory);
    SU_FREE(&gp_alloc, ptr);
}

static void *alloc_realloc_ft(FT_Memory memory, long old_size, long new_size, void* ptr) {
    void *ret = NULL;

    SU_NOTUSED(memory);

    if (SU_LIKELY(new_size > 0)) {
        SU_ALLOCTSA(ret, &gp_alloc, (size_t)new_size, 32);
        if (ptr) {
            SU_MEMCPY(ret, ptr, SU_MIN((size_t)old_size, (size_t)new_size));
        }
    }

    SU_FREE(&gp_alloc, ptr);

    return ret;
}

static struct FT_MemoryRec_ alloc_ft = { NULL, alloc_alloc_ft, alloc_free_ft, alloc_realloc_ft };

void *hb_malloc_impl(size_t size) {
    void *ret;

    if (size == 0) {
        return NULL;
    }

    SU_ALLOCTSA(ret, &gp_alloc, size, 32);

    return ret;
}

void *hb_calloc_impl(size_t nmemb, size_t size) {
    void *ret = NULL;
    size_t s = (size * nmemb);
    if (s > 0) {
        SU_ALLOCCTSA(ret, &gp_alloc, s, 32);
    }

    return ret;
}

void *hb_realloc_impl(void *ptr, size_t size) {
    return realloc(ptr, size); /* TODO: rework */
}

void hb_free_impl(void *ptr) {
    SU_FREE(&gp_alloc, ptr);
}

FT_Error Load_SBit_Png(FT_GlyphSlot slot, FT_Int x_offset, FT_Int y_offset, FT_Int pix_bits,
        TT_SBit_Metrics metrics, FT_Memory memory, FT_Byte *data, FT_UInt png_len,
        FT_Bool populate_map_and_metrics, FT_Bool metrics_only) {
    stbi__result_info ri;
    stbi__context ctx;
    int width, height, notused;
    uint32_t *pix;

    SU_NOTUSED(memory);
    
    if ((x_offset < 0) || (y_offset < 0)) {
        return FT_Err_Invalid_Argument;
    }

    if (!populate_map_and_metrics && (
                ((FT_UInt)x_offset + metrics->width > slot->bitmap.width) ||
                ((FT_UInt)y_offset + metrics->height > slot->bitmap.rows) ||
                (pix_bits != 32) ||
                (slot->bitmap.pixel_mode != FT_PIXEL_MODE_BGRA)
            )) {
        return FT_Err_Invalid_Argument;
    }

    SU_MEMSET(&ri, 0, sizeof(ri));
    ri.bits_per_channel = 8;
    stbi__start_mem(&ctx, (stbi_uc *)data, (int)png_len);

    if (!stbi__png_test(&ctx)) {
        return FT_Err_Invalid_File_Format;
    }

    /* ? TODO: stbi__png_info -> check -> if ok, fully decode */

    pix = (uint32_t *)stbi__png_load(&ctx, &width, &height, &notused, 4, &ri);
    if (!pix || (!populate_map_and_metrics &&
            (((FT_Int)width != metrics->width) || ((FT_Int)height != metrics->height)))) {
        return FT_Err_Invalid_File_Format;
    }

    if (populate_map_and_metrics) {
        if ((height > 0x7FFF) || (width > 0x7FFF)) {
            return FT_Err_Array_Too_Large;
        }

        metrics->width = (FT_UShort)width;
        metrics->height = (FT_UShort)height;

        slot->bitmap.width = metrics->width;
        slot->bitmap.rows = metrics->height;
        slot->bitmap.pixel_mode = FT_PIXEL_MODE_BGRA;
        slot->bitmap.pitch = (int)(slot->bitmap.width * 4);
        slot->bitmap.num_grays = 256;
    }

    if (metrics_only) {
        return FT_Err_Ok;
    }

    if (populate_map_and_metrics) {
        /* TODO: slot->internal->flags |= FT_GLYPH_OWN_BITMAP; */
        SU_ALLOCTSA(slot->bitmap.buffer, &gp_alloc, (slot->bitmap.rows * (size_t)slot->bitmap.pitch), 32);
    }

    if (ri.bits_per_channel != 8) {
        SU_ASSERT(ri.bits_per_channel == 16);
        pix = (uint32_t *)(void *)stbi__convert_16_to_8((stbi__uint16 *)pix, width, height, 4);
    }
    su_abgr32_convert_argb32_premultiply_alpha((uint32_t *)(void *)slot->bitmap.buffer, pix, (size_t)width * (size_t)height);

    return FT_Err_Ok;
}

static FT_Error resvg_init_ft(FT_Pointer *data_pointer) {
    SU_NOTUSED(data_pointer);
    return FT_Err_Ok;
}

static void resvg_free_ft(FT_Pointer *data_pointer) {
    SU_NOTUSED(data_pointer);
}

static FT_Error resvg_render_ft(FT_GlyphSlot slot, FT_Pointer *data_pointer) {
    FT_Error ret = FT_Err_Ok;

    SU_NOTUSED(data_pointer);

    if (!resvg_render_ft_state.tree) {
        ret = FT_Err_Invalid_SVG_Document;
        goto out;
    }

    if (resvg_render_ft_state.id[0] == '\0') {
        resvg_render(resvg_render_ft_state.tree, resvg_render_ft_state.transform,
            slot->bitmap.width, slot->bitmap.rows, (char *)slot->bitmap.buffer);
    } else {
        if (!resvg_render_node(resvg_render_ft_state.tree, resvg_render_ft_state.id,
                resvg_render_ft_state.transform, slot->bitmap.width,
                slot->bitmap.rows, (char *)slot->bitmap.buffer)) {
            ret = FT_Err_Invalid_SVG_Document;
            goto out;
        }
    }

    su_abgr32_convert_argb32( (uint32_t *)(void *)slot->bitmap.buffer,
        (uint32_t *)(void *)slot->bitmap.buffer,
        slot->bitmap.width * slot->bitmap.rows);

    slot->bitmap.pixel_mode = FT_PIXEL_MODE_BGRA;
    slot->bitmap.num_grays = 256;
    slot->format = FT_GLYPH_FORMAT_BITMAP;

out:
    if (resvg_render_ft_state.tree) {
        resvg_tree_destroy(resvg_render_ft_state.tree);
    }
    SU_MEMSET(&resvg_render_ft_state, 0, sizeof(resvg_render_ft_state));
    return ret;
}

static FT_Error resvg_preset_slot_ft(FT_GlyphSlot slot, FT_Bool cache, FT_Pointer *data_pointer) {
    FT_Error ret = FT_Err_Ok;
    FT_SVG_Document doc = (FT_SVG_Document)slot->other;
    resvg_options *options = resvg_options_create();
    resvg_rect bbox = { 0, 0, 0, 0 };
    float w, h, s;
    float target_w = (float)doc->metrics.x_ppem, target_h = (float)doc->metrics.y_ppem;
    resvg_render_ft_state_t local;

    float descender = ((float)doc->metrics.descender / 64.f);
    float line_height = ((float)doc->metrics.height / 64.f);
    float baseline = (line_height + descender);
    float line_center;

    resvg_render_ft_state_t *render_state = (cache ? &resvg_render_ft_state : &local);

    SU_NOTUSED(data_pointer);

    if (RESVG_OK != resvg_parse_tree_from_data( (const char *)doc->svg_document,
            (uintptr_t)doc->svg_document_length, options, &render_state->tree)) {
        SU_MEMSET(render_state, 0, sizeof(*render_state));
        ret = FT_Err_Invalid_SVG_Document;
        goto out;
    }

    if (resvg_get_image_bbox(render_state->tree, &bbox)) {
        w = bbox.width;
        h = bbox.height;
    } else {
        resvg_size size = resvg_get_image_size(render_state->tree);
        w = size.width;
        h = size.height;
    }

    if ((w <= 0) || (h <= 0)) {
        w = doc->units_per_EM;
        h = doc->units_per_EM;
    }

    s = SU_MIN((target_w / w), (target_h / h));

    render_state->transform.a = (s * ((float)doc->transform.xx / 65536.f));
    render_state->transform.b = (s * ((float)doc->transform.yx / 65536.f));
    render_state->transform.c = (s * ((float)doc->transform.xy / 65536.f));
    render_state->transform.d = (s * ((float)doc->transform.yy / 65536.f));
    render_state->transform.e = ((-bbox.x * s) + ((target_w - ((bbox.width * s))) / 2) + ((float)doc->delta.x / 64.f));
    render_state->transform.f = ((-bbox.y * s) + ((target_h - ((bbox.height * s))) / 2) + ((float)doc->delta.y / 64.f));

    w = target_w;
    h = target_h;

    line_center = (line_height - h) / 2.f;

    /* ? TODO: match reference implementation */
    slot->bitmap_left = (doc->metrics.x_ppem >= w)
        ? (FT_Int)((target_w - w) / 2.f)
        : 0;
    slot->bitmap_top = ((baseline >= line_center) && (line_center >= 0))
        ? (FT_Int)(baseline - (line_height - h) / 2.f)
        : 0;

    slot->bitmap.rows = (unsigned int)w;
    slot->bitmap.width = (unsigned int)h;

    slot->bitmap.pitch = ((int)slot->bitmap.width * 4);
    slot->bitmap.pixel_mode = FT_PIXEL_MODE_BGRA;

    slot->metrics.width = (FT_Pos)(w * 64.f);
    slot->metrics.height = (FT_Pos)(h * 64.f);

    slot->metrics.horiBearingX = (FT_Pos)((float)slot->bitmap_left * 64.f);
    slot->metrics.horiBearingY = (FT_Pos)((float)slot->bitmap_top * 64.f);

    if (slot->metrics.vertAdvance == 0) {
        slot->metrics.vertAdvance = (FT_Pos)(h * 78.8f);
    }

    slot->metrics.vertBearingX = (FT_Pos)(-w * 32.f);
    slot->metrics.vertBearingY = ((slot->metrics.vertAdvance / 2) - (FT_Pos)(h * 32.f));

    if (doc->start_glyph_id == doc->end_glyph_id) {
        SU_MEMSET(render_state->id, 0, sizeof(render_state->id));
    } else {
        su_snprintf(render_state->id, sizeof(render_state->id), "#glyph%u", slot->glyph_index);
    }

    if (!cache && render_state->tree) {
        resvg_tree_destroy(render_state->tree);
    }

out:
    if (options) {
        resvg_options_destroy(options);
    }
    return ret;
}

static SVG_RendererHooks resvg_hooks_ft = { resvg_init_ft, resvg_free_ft, resvg_render_ft, resvg_preset_slot_ft };

static sw_color_argb32_t color(uint32_t raw) {
    sw_color_argb32_t ret;

    uint32_t a = ((raw >> 24) & 0xFF);
    uint32_t r = ((raw >> 16) & 0xFF);
    uint32_t g = ((raw >> 8) & 0xFF);
    uint32_t b = ((raw >> 0) & 0xFF);

    ret.c.a = (uint8_t)a;
    ret.c.r = (uint8_t)(((r * a + 128) * 257) >> 16);
    ret.c.g = (uint8_t)(((g * a + 128) * 257) >> 16);
    ret.c.b = (uint8_t)(((b * a + 128) * 257) >> 16);

    return ret;
}

static void init(void) {
    /* ? TODO: error check */
    FT_New_Library(&alloc_ft, &state.ft);
    FT_Add_Default_Modules(state.ft);
    FT_Property_Set(state.ft, "ot-svg", "svg-hooks", &resvg_hooks_ft);

    state.hb_unicode_funcs = hb_unicode_funcs_get_default();
    state.hb_buf = hb_buffer_create();

    su_arena_init(&state.font_cache_arena, &gp_alloc, 131072);
    state.font_cache.capacity = 256;
    SU_ARRAY_ALLOCC(state.font_cache.items, &font_cache_alloc, state.font_cache.capacity);

    su_arena_init(&state.text_cache_arena, &gp_alloc, 131072);
    state.text_cache.capacity = 1024;
    SU_ARRAY_ALLOCC(state.text_cache.items, &text_cache_alloc, state.text_cache.capacity);
}

static void face_on_destroy_ft(void *data) {
    FT_Face face = (FT_Face)data;
    sw__font_t *font_priv = (sw__font_t *)face->generic.data;

    hb_font_destroy(font_priv->hb_font);

    su_arena_fini(&font_priv->arena, &gp_alloc);
}

static void fini(void) {
    hb_buffer_destroy(state.hb_buf);
    /*hb_unicode_funcs_destroy(state.hb_unicode_funcs);*/

    FT_Done_Library(state.ft);

    su_arena_fini(&state.text_cache_arena, &gp_alloc);
    su_arena_fini(&state.font_cache_arena, &gp_alloc);
}

static su_bool32_t sw__font_hash_table_add(sw__font_hash_table_t *ht,
        su_fat_ptr_t key, sw__font_t **out);

static void sw__font_hash_table_grow(sw__font_hash_table_t *ht) {
    const size_t max_capacity = 1024;

    if (SU_LIKELY(ht->capacity < max_capacity)) {
        size_t i;
        sw__font_hash_table_t new_ht;

        SU_ASSERT((ht->capacity > 1) && ((ht->capacity & (ht->capacity - 1)) == 0));
        new_ht.capacity = (ht->capacity * 2);
        SU_ARRAY_ALLOCC(new_ht.items, &font_cache_alloc, new_ht.capacity);

        for ( i = 0; i < ht->capacity; ++i) {
            sw__font_t *it = &ht->items[i];
            if (it->occupied) {
                sw__font_t *new_it;
                su_bool32_t r = sw__font_hash_table_add(&new_ht, it->key, &new_it);
                SU_ASSERT(r == SU_TRUE); SU_NOTUSED(r);
                new_it->face = it->face;
                new_it->hb_font = it->hb_font;
                new_it->idx = it->idx;
                new_it->pixel_size = it->pixel_size;
                new_it->glyph_load_flags = it->glyph_load_flags;
                new_it->features = it->features;
                new_it->features_count = it->features_count;
                new_it->glyph_cache = it->glyph_cache;
                new_it->underline_offset = it->underline_offset;
                new_it->underline_thickness = it->underline_thickness;
                new_it->strikeout_offset = it->strikeout_offset;
                new_it->strikeout_thickness = it->strikeout_thickness;
            }
        }

        *ht = new_ht;
    } else {
        /* ? TODO: error check */
        FT_Done_Library(state.ft);
        FT_New_Library(&alloc_ft, &state.ft);
        FT_Add_Default_Modules(state.ft);
        FT_Property_Set(state.ft, "ot-svg", "svg-hooks", &resvg_hooks_ft);
        su_arena_reset(&state.font_cache_arena, &gp_alloc);
        SU_MEMSET(ht->items, 0, sizeof(ht->items[0]) * max_capacity);
    }
}

static su_bool32_t sw__font_hash_table_add(sw__font_hash_table_t *ht,
        su_fat_ptr_t key, sw__font_t **out) {
    size_t h = (su_stbds_hash(key) & (ht->capacity - 1));
    sw__font_t *it = &ht->items[h];
    size_t c = 0;
    const size_t collisions_to_resize = 16;

    SU_ASSERT((ht->capacity > 1) && ((ht->capacity & (ht->capacity - 1)) == 0));
    for ( ;
            it->occupied && !su_fat_ptr_equal(it->key, key) && (c < ht->capacity);
            ++c) {
        it = &ht->items[(++h) & (ht->capacity - 1)];
    }

    if (SU_UNLIKELY((c >= collisions_to_resize) || (c == ht->capacity))) {
        sw__font_hash_table_grow(ht);
        return sw__font_hash_table_add(ht, key, out);
    } else if (it->occupied) {
        *out = it;
        return SU_FALSE;
    } else {
        it->key = key;
        it->occupied = SU_TRUE;
        *out = it;
        return SU_TRUE;
    }
}

static su_bool32_t sw__text_run_cache_hash_table_add(sw__text_run_cache_hash_table_t *ht,
        sw_text_t key, sw__text_run_cache_t **out);

static void sw__text_run_cache_hash_table_grow(sw__text_run_cache_hash_table_t *ht) {
    const size_t max_capacity = 32768;

    if (SU_LIKELY(ht->capacity < max_capacity)) {
        size_t i;
        sw__text_run_cache_hash_table_t new_ht;

        SU_ASSERT((ht->capacity > 1) && ((ht->capacity & (ht->capacity - 1)) == 0));
        new_ht.capacity = (ht->capacity * 2);
        SU_ARRAY_ALLOCC(new_ht.items, &text_cache_alloc, new_ht.capacity);

        for ( i = 0; i < ht->capacity; ++i) {
            sw__text_run_cache_t *it = &ht->items[i];
            if (it->occupied) {
                sw__text_run_cache_t *new_it;
                su_bool32_t r = sw__text_run_cache_hash_table_add(&new_ht, it->key, &new_it);
                SU_ASSERT(r == SU_TRUE); SU_NOTUSED(r);
                new_it->text_run = it->text_run;
                new_it->faces = it->faces;
                new_it->faces_count = it->faces_count;
            }
        }

        *ht = new_ht;
    } else {
        su_arena_reset(&state.text_cache_arena, &gp_alloc);
        SU_MEMSET(ht->items, 0, sizeof(ht->items[0]) * max_capacity);
    }
}

static su_bool32_t sw__text_run_cache_hash_table_add(sw__text_run_cache_hash_table_t *ht,
        sw_text_t key, sw__text_run_cache_t **out) {
    size_t h = (sw__stbds_hash_text(key) & (ht->capacity - 1));
    sw__text_run_cache_t *it = &ht->items[h];
    size_t c = 0;
    const size_t collisions_to_resize = 16;

    SU_ASSERT((ht->capacity > 1) && ((ht->capacity & (ht->capacity - 1)) == 0));
    for ( ;
            it->occupied && !sw__text_equal(it->key, key) && (c < ht->capacity);
            ++c) {
        it = &ht->items[(++h) & (ht->capacity - 1)];
    }

    if (SU_UNLIKELY((c >= collisions_to_resize) || (c == ht->capacity))) {
        sw__text_run_cache_hash_table_grow(ht);
        return sw__text_run_cache_hash_table_add(ht, key, out);
    } else if (it->occupied) {
        *out = it;
        return SU_FALSE;
    } else {
        it->key = key;
        it->occupied = SU_TRUE;
        *out = it;
        return SU_TRUE;
    }
}

static void *sw__font_alloc_alloc(const su_allocator_t *alloc, size_t size, size_t alignment) {
    sw__font_t *font_priv = (sw__font_t *)(uintptr_t)alloc;
    return su_arena_alloc(&font_priv->arena, &gp_alloc, size, alignment);
}

static sw_pixmap_t *render(sw_layout_block_text_t *block) {
    sw_pixmap_t *result;
    uint32_t result_w, result_h;
    FT_Error c;
    size_t i, f = 0, r = 1, g = 0;
    int32_t pen_x = 0, pen_y = 0;
    sw__partial_run_t *pruns;
    size_t pruns_count;
    sw__partial_run_t *prun;
    sw__text_run_t new_text_run;
    sw__text_run_cache_t *cache;
    su_bool32_t fonts_changed = SU_FALSE;
    size_t glyph_advance_field_offset;
    int32_t *axis;
    uint32_t prev_cluster;
    uint32_t next_notdef_cluster = UINT32_MAX;
    hb_script_t prev_script;
    FT_Size_Metrics *primary_font_metrics;

    SU_ASSERT(block->fonts_count > 0);

    for ( f = 0; f < block->fonts_count; ++f) {
        sw_font_t *font = &block->fonts[f];
        sw__font_t *font_priv;

        SU_ASSERT(font->pixel_size > 0);
        SU_ASSERT(font->data.len > 0);

        if (sw__font_hash_table_add(&state.font_cache, font->data, &font_priv)) {
            su_arena_init(&font_priv->arena, &gp_alloc, 131072);
            font_priv->alloc.alloc = sw__font_alloc_alloc;

            font_priv->key.len = font->data.len;
            font_priv->key.ptr = font_priv->alloc.alloc(&font_priv->alloc, font_priv->key.len, 32);
            SU_MEMCPY(font_priv->key.ptr, font->data.ptr, font->data.len);
            font_priv->idx = SIZE_MAX;
        }
        font->priv = (void *)font_priv;

        if (font_priv->idx != font->idx) {
            FT_Done_Face(font_priv->face);
            c = FT_New_Memory_Face(state.ft, (FT_Byte *)font->data.ptr,
                (FT_Long)font->data.len, (FT_Long)font->idx, &font_priv->face);
            SU_ASSERT(c == FT_Err_Ok);
            SU_ASSERT(font_priv->face->num_glyphs > 0);

            SU_ARRAY_ALLOCC(font_priv->glyph_cache, &font_priv->alloc, (size_t)font_priv->face->num_glyphs);

            font_priv->face->generic.finalizer = face_on_destroy_ft;
            font_priv->face->generic.data = font_priv;
            font_priv->idx = font->idx;
            fonts_changed = SU_TRUE;
        }

        if ( (font->pixel_size != font_priv->pixel_size) ||
                ((font->glyph_load_flags | font->hinting_algorithm) != font_priv->glyph_load_flags)) {
            float y_scale, ascent, descent;
            float underline_position, underline_thickness;
            float strikeout_position = 0.f, strikeout_thickness = 0.f;
            TT_OS2 *os2;

            font_priv->glyph_load_flags = (font->glyph_load_flags | font->hinting_algorithm);
            c = FT_Set_Pixel_Sizes(font_priv->face, 0, (FT_UInt)font->pixel_size);
            if (c != FT_Err_Ok) {
                FT_Pos target = ((FT_Pos)font->pixel_size * 64);
                FT_Int best = 0;
                int best_err = INT_MAX;
                SU_ASSERT(c == FT_Err_Invalid_Pixel_Size);
                SU_ASSERT(font_priv->face->num_fixed_sizes >= 0);
                for ( i = 0; i < (size_t)font_priv->face->num_fixed_sizes; ++i) {
                    FT_Bitmap_Size *size = &font_priv->face->available_sizes[i];
                    int err = SU_ABS((int)(target - size->y_ppem));
                    if ((err < best_err) || ((err == best_err) && (size->y_ppem >= target))) {
                        best = (FT_Int)i;
                        best_err = err;
                    }
                }
                c = FT_Select_Size(font_priv->face, best);
                SU_ASSERT(c == FT_Err_Ok);
            }

            if (!font_priv->hb_font) {
                font_priv->hb_font = hb_ft_font_create_referenced(font_priv->face);
                SU_ASSERT(font_priv->hb_font != NULL);
            }
            hb_ft_font_set_load_flags(font_priv->hb_font, (int)font_priv->glyph_load_flags);
            hb_ft_font_changed(font_priv->hb_font);

            y_scale = ((float)font_priv->face->size->metrics.y_scale / 65536.f);
            ascent = ((float)font_priv->face->size->metrics.ascender / 64.f);
            descent = ((float)font_priv->face->size->metrics.descender / 64.f);

            underline_position = ((float)font_priv->face->underline_position * y_scale / 64.f);
            underline_thickness = ((float)font_priv->face->underline_thickness * y_scale / 64.f);
            if ((underline_position == 0.f) || (underline_thickness <= 0.f)) {
                underline_thickness = SU_FABSF(descent / 5.f);
                underline_position = (-2 * underline_thickness);
            }
            
            if ((os2 = (TT_OS2 *)FT_Get_Sfnt_Table(font_priv->face, ft_sfnt_os2))) {
                strikeout_position = (os2->yStrikeoutPosition * y_scale / 64.f);
                strikeout_thickness = (os2->yStrikeoutSize * y_scale / 64.f);
            }
            if ((strikeout_position == 0.f) || (strikeout_thickness <= 0.f)) {
                strikeout_thickness = underline_thickness;
                strikeout_position = (3.f * ascent / 8.f - underline_thickness / 2.f);
            }

            font_priv->underline_offset = (int32_t)(underline_position + underline_thickness / 2.f);
            font_priv->underline_thickness = (uint32_t)(SU_MAX(1.f, underline_thickness) + 0.5f);
            font_priv->strikeout_offset = (int32_t)(strikeout_position + strikeout_thickness / 2.f);
            font_priv->strikeout_thickness = (uint32_t)(SU_MAX(1.f, strikeout_thickness) + 0.5f);

            font_priv->pixel_size = font->pixel_size;
            fonts_changed = SU_TRUE;
        }

        if (font->features_count != font_priv->features_count) {
            if (font->features_count > font_priv->features_count) {
                SU_ARRAY_ALLOCC(font_priv->features, &font_priv->alloc, font->features_count);
            }
            font_priv->features_count = font->features_count;
            fonts_changed = SU_TRUE;
        }
        for ( i = 0; i < font->features_count; ++i) {
            su_string_t str = font->features[i];
            hb_feature_t feat;
            hb_feature_from_string(str.s, (int)str.len, &feat);
            if (SU_MEMCMP(&feat, &font_priv->features[i], sizeof(feat)) != 0) {
                font_priv->features[i] = feat;
                fonts_changed = SU_TRUE;
            }
        }
    }

    primary_font_metrics = &((sw__font_t *)block->fonts[0].priv)->face->size->metrics;

    if (!sw__text_run_cache_hash_table_add(&state.text_cache, block->text, &cache)) {
        if (!fonts_changed && (cache->faces_count == block->fonts_count) && (block->vertical == cache->text_run.vertical)) {
            for ( i = 0; i < block->fonts_count; ++i) {
                if (((sw__font_t *)block->fonts[i].priv)->face != cache->faces[i]) {
                    goto shape;
                }
            }
            goto render;
        }
    } else {
        SU_ARRAY_ALLOC(cache->key.codepoints, &text_cache_alloc, block->text.codepoints_count);
        SU_MEMCPY(cache->key.codepoints, block->text.codepoints, sizeof(block->text.codepoints[0]) * block->text.codepoints_count);
        cache->key.codepoints_count = block->text.codepoints_count;
    }

shape:
    /* ? TODO: validate cps */

    SU_ARRAY_ALLOC(pruns, &scratch_alloc, (block->text.codepoints_count * block->fonts_count) + 1);
    prun = &pruns[0];
    prun->start_idx = 0;
    prun->len = 0;
    prun->script = HB_SCRIPT_INVALID;
    pruns_count = 1;

    prev_script = prun->script;
    new_text_run.glyphs_count = block->text.codepoints_count;
    for ( g = 0; g < new_text_run.glyphs_count; ++g) {
        hb_script_t script = hb_unicode_script(state.hb_unicode_funcs, block->text.codepoints[g]);
        if ((script == HB_SCRIPT_INHERITED) || (script == prev_script)) {
            prun->len++;
        } else {
            prev_script = script;
            prun = &pruns[pruns_count++];
            prun->start_idx = g;
            prun->len = 1;
            prun->script = script;
        }
    }

    new_text_run.vertical = block->vertical;
    new_text_run.clusters_count = 0;
    if (cache->text_run.glyphs &&
            (cache->text_run.glyphs_count >= new_text_run.glyphs_count)) {
        new_text_run.glyphs = cache->text_run.glyphs;
        SU_MEMSET(new_text_run.glyphs, 0, new_text_run.glyphs_count);
    } else {
        SU_ARRAY_ALLOCC(new_text_run.glyphs, &text_cache_alloc, new_text_run.glyphs_count);
    }
    if (block->vertical) {
        new_text_run.width =
            (uint32_t)(((float)primary_font_metrics->max_advance / 64.f) + 0.5f);
        new_text_run.height = 0;
        axis = (int32_t *)&new_text_run.height;
        glyph_advance_field_offset = SU_OFFSETOF(sw__glyph_t, y_advance);
    } else {
        axis = (int32_t *)&new_text_run.width;
        new_text_run.width = 0;
        new_text_run.height =
            (uint32_t)(((float)primary_font_metrics->height / 64.f) + 0.5f);
        glyph_advance_field_offset = SU_OFFSETOF(sw__glyph_t, x_advance);
    }

    for ( f = 0; f < block->fonts_count; ++f) {
        sw_font_t *font = &block->fonts[f];
        sw__font_t *font_priv = (sw__font_t *)font->priv;
        size_t pc = pruns_count;

        /* TODO: properly handle pruns with partial font coverage */
        for ( ; r < pc; ++r) {
            size_t insert_idx;
            unsigned int gc;
            hb_glyph_info_t *glyph_infos;
            hb_glyph_position_t *glyph_pos;

            prun = &pruns[r];
            insert_idx = prun->start_idx;
            prev_cluster = UINT32_MAX;

            hb_buffer_reset(state.hb_buf);
            hb_buffer_add_codepoints( state.hb_buf, block->text.codepoints,
                (int)new_text_run.glyphs_count, (unsigned int)prun->start_idx, (int)prun->len);

            hb_buffer_guess_segment_properties(state.hb_buf);

            SU_ASSERT((hb_buffer_get_direction(state.hb_buf) == HB_DIRECTION_LTR) ||
                    (hb_buffer_get_direction(state.hb_buf) == HB_DIRECTION_RTL));
            if (block->vertical) {
                hb_buffer_set_direction(state.hb_buf, HB_DIRECTION_TTB);
            }

            hb_shape(font_priv->hb_font, state.hb_buf, font_priv->features, (unsigned int)font_priv->features_count);

            glyph_infos = hb_buffer_get_glyph_infos(state.hb_buf, &gc);
            glyph_pos = hb_buffer_get_glyph_positions(state.hb_buf, &gc);

            SU_ASSERT(gc <= new_text_run.glyphs_count);

            for ( g = 0; g < gc; ++g) {
                sw_glyph_t *glyph;
                sw__glyph_t *glyph_priv;
                int32_t *glyph_advance;
                hb_codepoint_t idx = glyph_infos[g].codepoint;
                sw__cached_glyph_t **glyph_cache;
                FT_Face face;
                sw_glyph_load_flag_mask_t load_flags;

                if (idx == 0) {
                    uint32_t start_idx = glyph_infos[g].cluster;
                    hb_script_t script = hb_unicode_script(state.hb_unicode_funcs, block->text.codepoints[start_idx]);
                    sw__partial_run_t *pr = &pruns[pruns_count - 1];
                    size_t len = 1;

                    /* ? TODO: also scan backwards */
                    while (((g + 1) < gc) && (glyph_infos[g + 1].cluster == start_idx)) {
                        g++; len++;
                    }

                    if ((((pr->start_idx + pr->len) == start_idx) || (pr->start_idx == (start_idx + len)))
                            && ((script == HB_SCRIPT_INHERITED) || (pr->script == script))) {
                        pr->len += len;
                        if (start_idx < pr->start_idx) {
                           pr->start_idx = start_idx;
                        }
                    } else {
                        pr = &pruns[pruns_count++];
                        pr->start_idx = start_idx;
                        pr->len = len;
                        pr->script = script;
                    }

                    insert_idx += len;

                    glyph = &new_text_run.glyphs[start_idx];
                    glyph_priv = (sw__glyph_t *)&glyph->sw__private;
                    glyph_advance = (int32_t *)(void *)((uint8_t *)glyph_priv + glyph_advance_field_offset);
                    glyph->font = &block->fonts[0];
                    glyph_cache = ((sw__font_t *)glyph->font->priv)->glyph_cache;
                    face = ((sw__font_t *)glyph->font->priv)->face;
                    load_flags = ((sw__font_t *)glyph->font->priv)->glyph_load_flags;
                    glyph->cluster = --next_notdef_cluster;
                    glyph->codepoint = 0;
                    if (SU_UNLIKELY(glyph_priv->idx != 0)) {
                        new_text_run.clusters_count--;
                        *axis -= *glyph_advance;
                        glyph_priv->valid = SU_FALSE;
                    }
                } else {
                    glyph = &new_text_run.glyphs[insert_idx++];
                    glyph_priv = (sw__glyph_t *)&glyph->sw__private;
                    glyph->font = font;
                    glyph_cache = font_priv->glyph_cache;
                    face = font_priv->face;
                    load_flags = font_priv->glyph_load_flags;
                    glyph_advance = (int32_t *)(void *)((uint8_t *)glyph_priv + glyph_advance_field_offset);
                    glyph->cluster = glyph_infos[g].cluster;
                    glyph->codepoint = block->text.codepoints[glyph->cluster];

                    if (glyph_priv->valid) {
                        new_text_run.clusters_count--;
                        glyph_priv->valid = SU_FALSE;
                        *axis -= *glyph_advance;
                    }
                }

                if (!glyph_priv->valid) {
                    sw__cached_glyph_t *cached_glyph = glyph_cache[idx];
                    if ( !cached_glyph
                            || (cached_glyph->pixel_size != glyph->font->pixel_size)
                            || (cached_glyph->load_flags != load_flags)
                            || (cached_glyph->render_mode != glyph->font->glyph_render_mode)
                            || (cached_glyph->synthetic_weight_dx != glyph->font->synthetic_weight_dx)
                            || (cached_glyph->synthetic_slant_x != glyph->font->synthetic_slant_x)
                            || (cached_glyph->synthetic_weight_dy != glyph->font->synthetic_weight_dy)
                            || (cached_glyph->synthetic_slant_y != glyph->font->synthetic_slant_y)) {
                        FT_Bitmap *ft_bitmap;
                        su_bool32_t scale;

                        c = FT_Load_Glyph(face, idx, (FT_Int32)load_flags);
                        SU_ASSERT(c == FT_Err_Ok);
                        scale = (face->glyph->format == FT_GLYPH_FORMAT_BITMAP);

                        FT_GlyphSlot_AdjustWeight(face->glyph,
                            (FT_Fixed)glyph->font->synthetic_weight_dx,
                            (FT_Fixed)glyph->font->synthetic_weight_dy);
                        FT_GlyphSlot_Slant(face->glyph,
                            (FT_Fixed)glyph->font->synthetic_slant_x,
                            (FT_Fixed)glyph->font->synthetic_slant_y);

                        c = FT_Render_Glyph(face->glyph, (FT_Render_Mode)glyph->font->glyph_render_mode);
                        SU_ASSERT(c == FT_Err_Ok);
                        SU_ASSERT(face->glyph->format == FT_GLYPH_FORMAT_BITMAP);

                        if (!cached_glyph) {
                            SU_ALLOCT(glyph_cache[idx], &font_priv->alloc);
                            cached_glyph = glyph_cache[idx];
                            cached_glyph->data_size = 0;
                        }

                        ft_bitmap = &face->glyph->bitmap;
                        SU_ASSERT(ft_bitmap->pitch >= 0);

                        if ((ft_bitmap->rows * (unsigned int)ft_bitmap->pitch) > cached_glyph->data_size) {
                            cached_glyph->data_size = (ft_bitmap->rows * (unsigned int)ft_bitmap->pitch);
                            SU_ARRAY_ALLOCA(cached_glyph->data, &font_priv->alloc, cached_glyph->data_size, 32);
                        }

                        if (ft_bitmap->buffer) {
                            SU_MEMCPY(cached_glyph->data, ft_bitmap->buffer, cached_glyph->data_size);
                        }

                        cached_glyph->width = ft_bitmap->width;
                        cached_glyph->height = ft_bitmap->rows;
                        cached_glyph->pitch = (uint32_t)ft_bitmap->pitch;
                        cached_glyph->pixel_mode = (FT_Pixel_Mode)ft_bitmap->pixel_mode;
                        cached_glyph->left = face->glyph->bitmap_left;
                        cached_glyph->top = face->glyph->bitmap_top;
                        cached_glyph->pixel_size = glyph->font->pixel_size;
                        cached_glyph->load_flags = load_flags;
                        cached_glyph->render_mode = glyph->font->glyph_render_mode;
                        cached_glyph->scale_factor = ((scale && (ft_bitmap->width > 0) && (ft_bitmap->rows > 0))
                            ? SU_MIN(
                                (float)glyph->font->pixel_size / (float)ft_bitmap->width,
                                (float)glyph->font->pixel_size / (float)ft_bitmap->rows)
                            : 1.f);
                        cached_glyph->synthetic_slant_x = glyph->font->synthetic_slant_x;
                        cached_glyph->synthetic_slant_y = glyph->font->synthetic_slant_y;
                        cached_glyph->synthetic_weight_dx = glyph->font->synthetic_weight_dx;
                        cached_glyph->synthetic_weight_dy = glyph->font->synthetic_weight_dy;
                    }

                    glyph_priv->x_advance = SU_ABS((int)((((float)glyph_pos[g].x_advance / 64.f) + 0.5f) * cached_glyph->scale_factor));
                    glyph_priv->y_advance = SU_ABS((int)((((float)glyph_pos[g].y_advance / 64.f) + 0.5f) * cached_glyph->scale_factor));
                    glyph_priv->x_offset = (int32_t)(((float)glyph_pos[g].x_offset / 64.f) + 0.5f);
                    glyph_priv->y_offset = (int32_t)(((float)glyph_pos[g].y_offset / 64.f) + 0.5f);
                    glyph_priv->idx = idx;
                    glyph_priv->valid = SU_TRUE;
                    
                    if (SU_UNLIKELY((*glyph_advance == 0) && (cached_glyph->scale_factor != 1.f))) {
                        /* TODO: rework */
                        *glyph_advance = (int32_t)glyph->font->pixel_size;
                    }

                    if (glyph->cluster != prev_cluster) {
                        new_text_run.clusters_count++;
                    }

                    prev_cluster = glyph->cluster;
                    *axis += *glyph_advance;
                }
            }
        }
    }

    /* TODO: condition */ {
        size_t write = 0, read = 0;
        for ( ; read < new_text_run.glyphs_count; ++read) {
            sw_glyph_t *glyph = &new_text_run.glyphs[read];
            sw__glyph_t *glyph_priv = (sw__glyph_t *)&glyph->sw__private;
            if (glyph_priv->valid) {
                new_text_run.glyphs[write++] = new_text_run.glyphs[read];
            }
        }
        new_text_run.glyphs_count = write;
    }

    if (block->fonts_count > cache->faces_count) {
        SU_ARRAY_ALLOC(cache->faces, &text_cache_alloc, block->fonts_count);
    }
    cache->text_run = new_text_run;
    for ( i = 0; i < block->fonts_count; ++i) {
        cache->faces[i] = ((sw__font_t *)block->fonts[i].priv)->face;
    }
    cache->faces_count = block->fonts_count;

render:
    if (block->vertical) {
        result_w = cache->text_run.width;
        result_h = (cache->text_run.height + (block->letter_spacing * (cache->text_run.clusters_count - 1)));
        pen_x = (result_w / 2);
        axis = &pen_y;
        glyph_advance_field_offset = SU_OFFSETOF(sw__glyph_t, y_advance);
    } else {
        result_w = (cache->text_run.width + (block->letter_spacing * (cache->text_run.clusters_count - 1)));
        result_h = cache->text_run.height;
        pen_y = (int32_t)((float)result_h + ((float)primary_font_metrics->descender / 64.f) + 0.5f);
        axis = &pen_x;
        glyph_advance_field_offset = SU_OFFSETOF(sw__glyph_t, x_advance);
    }

    SU_ASSERT(result_w > 0);
    SU_ASSERT(result_h > 0);

    SU_ALLOCCTS( result, &gp_alloc,
        (sizeof(result->width) + sizeof(result->height) + result_w * result_h * 4));
    result->width = result_w;
    result->height = result_h;
    /* SU_MEMSET(result->pixels, 0xFF, result_w * result_h * 4); */

    prev_cluster = cache->text_run.glyphs[0].cluster;

    for ( g = 0; g < cache->text_run.glyphs_count; ++g) {
        sw_glyph_t *glyph = &cache->text_run.glyphs[g];
        sw__glyph_t *glyph_priv = (sw__glyph_t *)&glyph->sw__private;
        int32_t *glyph_advance;
        sw_font_t *font;
        sw__font_t *font_priv;
        sw__cached_glyph_t *cached_glyph;

        SU_ASSERT(glyph_priv->valid);

        font = glyph->font;
        glyph_advance = (int32_t *)(void *)((uint8_t *)glyph_priv + glyph_advance_field_offset);

        if (glyph->cluster != prev_cluster) {
            prev_cluster = glyph->cluster;
            *axis += (int32_t)block->letter_spacing;
        }

        font_priv = (sw__font_t *)font->priv;
        SU_ASSERT(glyph_priv->idx < font_priv->face->num_glyphs);

        cached_glyph = font_priv->glyph_cache[glyph_priv->idx];
        SU_ASSERT(cached_glyph != NULL);

        if (cached_glyph->data) {
            uint32_t width, height;
            int32_t dst_x, dst_y;

            if (cached_glyph->scale_factor != 1.f) {
                width = (uint32_t)((float)cached_glyph->width * cached_glyph->scale_factor + 0.5f);
                height = (uint32_t)((float)cached_glyph->height * cached_glyph->scale_factor + 0.5f);
                if (block->vertical) {
                    dst_x = ((result_w - width) / 2);
                    dst_y = pen_y;
                } else {
                    dst_x = pen_x;
                    dst_y = ((result_h - height) / 2);
                }

                /* fow now, only bilinear filter is available TODO: more filters */

                switch (cached_glyph->pixel_mode) {
                case FT_PIXEL_MODE_GRAY:
                    su_argb32_mask8_bilinear_blend_argb32((uint32_t *)result->pixels, result->width, result->height,
                            block->text_color.u32,
                            cached_glyph->data, cached_glyph->width, cached_glyph->height, cached_glyph->pitch,
                            dst_x, dst_y,
                            width, height);
                    break;
                case FT_PIXEL_MODE_BGRA:
                    su_argb32_bilinear_blend_argb32((uint32_t *)result->pixels, result->width, result->height,
                            (uint32_t *)(void *)cached_glyph->data, cached_glyph->width, cached_glyph->height,
                            dst_x, dst_y,
                            width, height);
                    break;
                case FT_PIXEL_MODE_NONE:
                case FT_PIXEL_MODE_MONO:
                case FT_PIXEL_MODE_GRAY2:
                case FT_PIXEL_MODE_GRAY4:
                case FT_PIXEL_MODE_LCD:
                case FT_PIXEL_MODE_LCD_V:
                case FT_PIXEL_MODE_MAX:
                default:
                    SU_ASSERT_UNREACHABLE;
                }
            } else {
                width = cached_glyph->width;
                height = cached_glyph->height;
                dst_x = (pen_x + (cached_glyph->left + glyph_priv->x_offset));
                dst_y = (pen_y - (cached_glyph->top + glyph_priv->y_offset));

                switch (cached_glyph->pixel_mode) {
                case FT_PIXEL_MODE_GRAY:
                    su_argb32_mask8_blend_argb32((uint32_t *)result->pixels, result->width, result->height,
                            block->text_color.u32,
                            cached_glyph->data, cached_glyph->width, cached_glyph->height, cached_glyph->pitch,
                            dst_x, dst_y,
                            width, height);
                    break;
                case FT_PIXEL_MODE_BGRA:
#if 1
                    su_argb32_blend_argb32((uint32_t *)result->pixels, result->width, result->height,
                            (uint32_t *)(void *)cached_glyph->data, cached_glyph->width, cached_glyph->height,
                            dst_x, dst_y, 0, 0,
                            width, height);
#else
                    su_argb32_rotate_blend_argb32( SU_ROTATE_90,
                            (uint32_t *)result->pixels, result->width, result->height,
                            (uint32_t *)(void *)cached_glyph->data, cached_glyph->width, cached_glyph->height,
                            dst_x, dst_y, 0, 0,
                            width, height);
#endif
                    break;
                case FT_PIXEL_MODE_MONO:
                    su_argb32_mask1_blend_argb32((uint32_t *)result->pixels, result->width, result->height,
                            block->text_color.u32,
                            cached_glyph->data, cached_glyph->width, cached_glyph->height, cached_glyph->pitch,
                            dst_x, dst_y,
                            width, height);
                    break;
                case FT_PIXEL_MODE_LCD:
                    SU_ASSERT((cached_glyph->width % 3) == 0);
                    su_argb32_mask24_blend_argb32((uint32_t *)result->pixels, result->width, result->height,
                            block->text_color.u32,
                            cached_glyph->data, cached_glyph->width / 3, cached_glyph->height, cached_glyph->pitch,
                            dst_x, dst_y,
                            width, height);
                    break;
                case FT_PIXEL_MODE_LCD_V:
                    SU_ASSERT((cached_glyph->height % 3) == 0);
                    su_argb32_mask24v_blend_argb32((uint32_t *)result->pixels, result->width, result->height,
                            block->text_color.u32,
                            cached_glyph->data, cached_glyph->width, cached_glyph->height / 3, cached_glyph->pitch,
                            dst_x, dst_y,
                            width, height);
                    break;
                case FT_PIXEL_MODE_NONE:
                case FT_PIXEL_MODE_GRAY2:
                case FT_PIXEL_MODE_GRAY4:
                case FT_PIXEL_MODE_MAX:
                default:
                    SU_ASSERT_UNREACHABLE;
                }
            }
            glyph->x = dst_x;
            glyph->y = dst_y;
            glyph->width = width;
            glyph->height = height;
        } else {
            glyph->x = pen_x;
            glyph->y = pen_y;

            /* TODO */
            glyph->width = (uint32_t)*glyph_advance;
            glyph->height = (uint32_t)*glyph_advance;
        }
        *axis += *glyph_advance;
    }

    /* ? TODO: draw before glyps */ {
        sw_font_t *font = &block->fonts[0];
        sw__font_t *font_priv = (sw__font_t *)font->priv;

        if (block->vertical) {
            /* TODO: rework position */
            su_argb32_rect_blend_argb32((uint32_t *)result->pixels, result->width, result->height,
                    block->underline_color.u32,
                    pen_x - font_priv->underline_offset, 0,
                    font_priv->underline_thickness, result_h);
            su_argb32_rect_blend_argb32((uint32_t *)result->pixels, result->width, result->height,
                    block->strikeout_color.u32,
                    pen_x - font_priv->strikeout_offset, 0,
                    font_priv->strikeout_thickness, result_h);
        } else {
            su_argb32_rect_blend_argb32((uint32_t *)result->pixels, result->width, result->height,
                    block->underline_color.u32, 0, pen_y - font_priv->underline_offset,
                    result_w, font_priv->underline_thickness);

            su_argb32_rect_blend_argb32((uint32_t *)result->pixels, result->width, result->height,
                    block->strikeout_color.u32, 0, pen_y - font_priv->strikeout_offset,
                    result_w, font_priv->strikeout_thickness);
        }
    }

    return result;
}

static su_bool32_t process_sw_events(void) {
    size_t i;
    for ( i = 0; i < sw.out.events_count; ++i) {
        sw_event_t *event = &sw.out.events[i];
        switch (event->out.type) {
        case SW_EVENT_WAYLAND_OUTPUT_CREATE:
        case SW_EVENT_WAYLAND_SURFACE_ERROR_MISSING_PROTOCOL:
        case SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER:
        case SW_EVENT_WAYLAND_SURFACE_DESTROY:
        case SW_EVENT_LAYOUT_BLOCK_DESTROY:
        case SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_IMAGE:
        case SW_EVENT_WAYLAND_SURFACE_FAILED_TO_SET_CURSOR_SHAPE:
        case SW_EVENT_WAYLAND_OUTPUT_DESTROY:
        case SW_EVENT_WAYLAND_POINTER_ENTER:
        case SW_EVENT_WAYLAND_POINTER_LEAVE:
        case SW_EVENT_WAYLAND_POINTER_MOTION:
        case SW_EVENT_WAYLAND_POINTER_BUTTON:
        case SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_CANCELLED:
        case SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_FINISHED:
        case SW_EVENT_WAYLAND_POINTER_SCROLL:
        case SW_EVENT_WAYLAND_POINTER_CREATE:
        case SW_EVENT_WAYLAND_SEAT_DESTROY:
        case SW_EVENT_WAYLAND_POINTER_DESTROY:
        case SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK:
        case SW_EVENT_WAYLAND_SURFACE_ERROR_LAYOUT_FAILED:
        case SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_FAILED_TO_SET_DECORATIONS:
        case SW_EVENT_WAYLAND_KEYBOARD_DESTROY:
        case SW_EVENT_WAYLAND_KEYBOARD_ENTER:
        case SW_EVENT_WAYLAND_KEYBOARD_LEAVE:
        case SW_EVENT_WAYLAND_SEAT_CREATE:
        case SW_EVENT_WAYLAND_KEYBOARD_CREATE:
        case SW_EVENT_WAYLAND_KEYBOARD_KEY:
        case SW_EVENT_WAYLAND_KEYBOARD_KEY_REPEAT:
        case SW_EVENT_WAYLAND_KEYBOARD_MOD:
        case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ACTION:
        case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_SOURCE_ACTIONS:
        case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_ENTER:
        case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_LEAVE:
        case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_MOTION:
        case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_DND_DROP:
        case SW_EVENT_WAYLAND_DATA_DEVICE_DESTROY:
        case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_MIME_OFFERS:
        case SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_ACTION:
        case SW_EVENT_WAYLAND_DATA_DEVICE_COPY_DND_DROP_PERFORMED:
        case SW_EVENT_WAYLAND_DATA_DEVICE_PASTE_NEW_DATA:
        case SW_EVENT_WAYLAND_DATA_DEVICE_CREATE:
            break;
        case SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_CLOSE:
            state.running = SU_FALSE;
            return SU_FALSE;
        default:
            SU_ASSERT_UNREACHABLE;
        }
    }

    return sw.in.update_and_render;
}

static void handle_signal(int sig) {
    SU_DEBUG_LOG("%s: %d", strsignal(sig), sig);
    SU_NOTUSED(sig);
    state.running = SU_FALSE;
}

int main(void) {
    su_bool32_t c;
    sw_pixmap_t *pm;
    static struct sigaction sigact;
    static sw_wayland_surface_t surface;
    static sw_layout_block_t root;
    size_t i;
    static sw_layout_block_text_t block;

    static char *font_files[] = {
#if 0
        "/usr/share/fonts/noto-emoji/NotoColorEmoji.ttf", /* png */
#else
        "/home/user/Downloads/tmp/TwitterColorEmoji-SVGinOT-Linux-15.1.0/TwitterColorEmoji-SVGinOT.ttf", /* svg */
#endif
        /*"/usr/share/fonts/nerdfonts/CaskaydiaCoveNerdFont-Regular.ttf", */
        /*"/usr/share/fonts/noto/NotoSansDevanagari-Regular.ttf",*/
         "/usr/share/fonts/liberation-fonts/LiberationSans-Regular.ttf",
#if 1
        "/usr/share/fonts/cascadia-code/CascadiaMono.ttf",
#else
        "/usr/share/fonts/cascadia-code/CascadiaMonoItalic.ttf",
#endif
        "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/noto/NotoSansDevanagari-Regular.ttf",
        "/usr/share/fonts/noto/NotoSansMongolian-Regular.ttf",
        "/usr/share/fonts/noto/NotoSansPhagsPa-Regular.ttf",
    };
    static sw_font_t fonts[SU_LENGTH(font_files)];
    /* "hello world | fi | اَلْعَرَبِيَّةُ | עִבְרִית | 👨‍👩‍👧‍👦 🇸🇪 🧍‍♀️ | привет, мир! | 你好世界" */
#if 1
    su_string_t text = su_string_("hello world | fi | اَلْعَرَبِيَّةُ | עִבְרִית | 👨‍👩‍👧‍👦 🇸🇪 🧍‍♀️ | привет, мир! | 你好世界");
#elif 0
    su_string_t text = su_string_("\xF0\x9D\xBC\x80 \xF0\xAB\xA0\x9D \xF0\x96\xBF\xA4 \xF0\x91\xBC\x82 \xF0\x90\xBB\xBD \xF0\x9A\xBF\xB0 \xF0\x93\x90\xB0     \x80 \xC0\xAF \xC3 \xE2\x28\xA1 \xF0\x9F\x92 \xF4\x90\x80\x80 \xED\xA0\x80");
#elif 0
    su_string_t text = su_string_(/*"\xCC\x81 A\xCC\x81\xCC\x80" "\xD9\x8E"*/ "\xEF\xB7\xB2" /*"\xE0\xA5\x98"*/);
#elif 0
    su_string_t text = su_string_("\xE1\xA0\xA0\xE1\xA0\xA8\xE1\xA0\xB6 | \xEA\xA1\x80\xEA\xA1\x83");
#elif 0
    su_string_t text = su_string_("\xF0\x9D\xBC\x80 \xF0\xAB\xA0\x9D \xF0\x96\xBF\xA4 \xF0\x91\xBC\x82 \xF0\x90\xBB\xBD \xF0\x9A\xBF\xB0 \xF0\x93\x90\xB0     \x80 \xC0\xAF \xC3 \xE2\x28\xA1 \xF0\x9F\x92 \xF4\x90\x80\x80 \xED\xA0\x80 | \xCC\x81 A\xCC\x81\xCC\x80 \xD9\x8E \xEF\xB7\xB2 \xE0\xA5\x98");
#elif 1
    su_string_t text = su_string_("👨‍👩‍👧‍👦🇸🇪🧍‍♀️👨‍👩‍👧‍👦🇸🇪🧍‍♀️👨‍👩‍👧‍👦🇸🇪🧍‍♀️👨‍👩‍👧‍👦🇸🇪🧍‍♀️👨‍👩‍👧‍👦🇸🇪🧍‍♀️👨‍👩‍👧‍👦🇸🇪🧍‍♀️👨‍👩‍👧‍👦🇸🇪🧍‍♀️👨‍👩‍👧‍👦🇸🇪🧍‍♀️👨‍👩‍👧‍👦🇸🇪🧍‍♀️👨‍👩‍👧‍👦🇸🇪🧍‍♀️👨‍👩‍👧‍👦🇸🇪🧍‍♀️");
#endif
    block.text_color = color(0xFFFFFFFF);
    block.fonts = fonts;
    block.fonts_count = SU_LENGTH(font_files);
    block.letter_spacing = 0;
    /* block.vertical = SU_TRUE;
    block.underline_color = color(0xFFFF0000);
    block.strikeout_color = color(0x4400FF00); */



    setlocale(LC_ALL, "");
    SU_ASSERT(su_locale_is_utf8());

    su_arena_init(&state.scratch_arena, &gp_alloc, 16384);

    init();

    SU_ARRAY_ALLOC(block.text.codepoints, &scratch_alloc, text.len);
    block.text.codepoints_count = su_convert_valid_utf8_to_utf32(text, block.text.codepoints);

    for ( i = 0; i < SU_LENGTH(font_files); ++i) {
        sw_font_t *font = &block.fonts[i];
        su_read_entire_file(su_string_(font_files[i]), &font->data, &gp_alloc);
        font->pixel_size = 48;
        font->idx = 0;
        font->glyph_load_flags = SW_GLYPH_LOAD_FLAG_COLOR;
        font->glyph_render_mode = SW_GLYPH_RENDER_MODE_NORMAL;
    }

    sw.in.backend_type = SW_BACKEND_TYPE_WAYLAND;
    sw.in.gp_alloc = &gp_alloc;
    sw.in.scratch_alloc = &scratch_alloc;
    sw.in.update_and_render = SU_TRUE;

    /* hack for stbi allocator */
    sw__context = &sw;
    state.running = SU_TRUE;

    {
        int64_t s = su_now_ms(CLOCK_MONOTONIC);
        pm = render(&block);
        su_log_stderr("%ld ms", su_now_ms(CLOCK_MONOTONIC) - s);
    }

    
#if 0
    {
        su_fat_ptr_t data;
        su_string_t path = su_string_("test_pm");
        size_t len = ((pm->width * pm->height * 4) + sizeof(pm->width) + sizeof(pm->height));
#if 0
        data.ptr = pm;
        data.len = len;
        su_write_entire_file(path, data);
#else
        su_read_entire_file(path, &data, &scratch_alloc);
        SU_ASSERT(SU_MEMCMP(pm, data.ptr, len) == 0);
#endif
    }
#endif

    root.in.fill = SW_LAYOUT_BLOCK_FILL_ALL_SIDES;
    root.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
    root.in.color._.argb32.u32 = 0xFF000000;
    root.in.type = SW_LAYOUT_BLOCK_TYPE_IMAGE;
    root.in._.image.type = SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SW_PIXMAP;
    root.in._.image.data.ptr = pm;
    root.in._.image.data.len = (sizeof(pm->width) + sizeof(pm->height) + (pm->width * pm->height * 4));

    surface.in.root = &root;
    surface.in.type = SW_WAYLAND_SURFACE_TYPE_TOPLEVEL;
    surface.in._.toplevel.decoration_mode = SW_WAYLAND_TOPLEVEL_DECORATION_MODE_SERVER_SIDE;

    SU_LLIST_APPEND_TAIL(&sw.in.backend.wayland.toplevels, &surface);

    sigact.sa_handler = handle_signal;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGPIPE, &sigact, NULL);

main_loop:
    do {
        c = sw_set(&sw);
        SU_ASSERT(c == SU_TRUE);
        sw.in.update_and_render = SU_FALSE;
    } while (process_sw_events());

    if (!state.running) {
        goto cleanup;
    }

    su_arena_reset(&state.scratch_arena, &gp_alloc);

    poll(sw.out.backend.wayland.fds, sw.out.backend.wayland.fds_count, (int)(sw.out.t - su_now_ms(CLOCK_MONOTONIC)));
    
    goto main_loop;

cleanup:
    sw.in.backend_type = SW_BACKEND_TYPE_NONE;
    c = sw_set(&sw);
    SU_ASSERT(c == SU_TRUE);
    SU_FREE(&gp_alloc, sw.out.events);

    fini();

    su_arena_fini(&state.scratch_arena, &gp_alloc);

    for ( i = 0; i < SU_LENGTH(font_files); ++i) {
        SU_FREE(&gp_alloc, fonts[i].data.ptr);
    }

    SU_FREE(&gp_alloc, pm);

    SU_NOTUSED(c);
    return 0;
}
