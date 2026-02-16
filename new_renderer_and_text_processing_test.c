#define _XOPEN_SOURCE 700
/*#define SU_STRIP_PREFIXES*/
#define SU_IMPLEMENTATION
#include "sutil.h"

#define SW_IMPLEMENTATION
#include "swidgets.h"

#include <locale.h>
#include <signal.h>

#include <ft2build.h>
#include <freetype/ftmodapi.h>
#include <freetype/otsvg.h>
/*#include <freetype/ftcache.h>*/
#include <harfbuzz/hb-ft.h>

#define RSVG_TEST 0
#if RSVG_TEST
    #include "new_renderer_and_text_processing_rsvg_test.h"
#endif

typedef enum sw_glyph_load_flag {
    SW_GLYPH_LOAD_FLAG_DEFAULT = 0,
    SW_GLYPH_LOAD_FLAG_COLOR = (1L << 20),
    SW_GLYPH_LOAD_FLAG_NO_HINTING = (1L << 1),
    SW_GLYPH_LOAD_FLAG_FORCE_AUTOHINT = (1L << 5),
    SW_GLYPH_LOAD_FLAG_NO_AUTOHINT = (1L << 15)
    /* ? TODO: NO_BITMAP VERTICAL_LAYOUT SBITS_ONLY NO_SVG ?? COMPUTE_METRICS BITMAP_METRICS_ONLY */
} sw_glyph_load_flag_t;

typedef uint32_t sw_glyph_load_flag_mask_t;

typedef enum sw_glyph_render_mode {
    SW_GLYPH_RENDER_MODE_NORMAL = 0,
    SW_GLYPH_RENDER_MODE_MONO = 2,
    SW_GLYPH_RENDER_MODE_LCD = 3,
    SW_GLYPH_RENDER_MODE_LCD_V = 4
    /* TODO: sdf */
} sw_glyph_render_mode_t;

typedef struct sw_font_in {
    su_fat_ptr_t data;
    size_t idx;
    uint32_t pixel_size;
    sw_glyph_render_mode_t glyph_render_mode;
    sw_glyph_load_flag_mask_t glyph_load_flags;
    SU_PAD32;
} sw_font_in_t;

typedef struct sw__cached_glyph {
    uint32_t width, height, pitch;
    FT_Pixel_Mode pixel_mode;
    int32_t left, top; 
    float scale_factor; /* ? TODO: scale FT_PIXEL_MODE_BGRA right away */
    uint32_t pixel_size;
    sw_glyph_render_mode_t render_mode;
    sw_glyph_load_flag_mask_t load_flags;
    uint8_t *data;
} sw__cached_glyph_t;

typedef struct sw__font {
    su_fat_ptr_t data;
    FT_Face face;
    hb_font_t *hb_font;
    size_t idx;
    uint32_t pixel_size;
    sw_glyph_load_flag_mask_t glyph_load_flags;
    /* ? TODO: cache multiple sizes / render modes / load flags */
    sw__cached_glyph_t **glyph_cache; /* size: FT_Face.num_glyphs */
} sw__font_t;

typedef struct sw_font {
    sw_font_in_t in;
    SW__PRIVATE_FIELDS(sizeof(sw__font_t));
} sw_font_t;

typedef struct sw_layout_block_text {
	su_string_t text;
	sw_font_t *fonts; /* fallback order */
	size_t fonts_count;
    /* for now, the only way to get text layed out vertically is to set this flag */
    /* ? TODO: implicit vertical, based on string contents */
    su_bool32_t vertical;
    /* TODO: utf8/utf32 */
	sw_color_argb32_t text_color; /* TODO: sw_color_t */
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
    /* TODO */
	/*uint32_t codepoint;
	int32_t cluster;
	int32_t x, y;
	int32_t width, height;*/

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
    int32_t w, h;
    su_bool32_t vertical;
    SU_PAD32;
} sw__text_run_t;

typedef struct sw__text_run_cache {
	SU_HASH_TABLE_FIELDS(su_string_t);
    sw__text_run_t text_run; /* ? TODO: cache multiple runs for different font combinations */
	FT_Face *faces;
	size_t faces_count;
} sw__text_run_cache_t;

/* TODO: better hash function */
SU_HASH_TABLE_DECLARE_DEFINE(sw__text_run_cache_t, su_string_t, su_stbds_hash_string, su_string_equal, SU_MEMCPY, 16)

typedef struct state {
    FT_Library ft;
    hb_buffer_t *hb_buf;
    hb_unicode_funcs_t *hb_unicode_funcs;
    su_hash_table__sw__text_run_cache_t__t text_cache;
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

static void *sw__text_cache_alloc_alloc(const su_allocator_t *alloc, size_t size, size_t alignment) {
    SU_NOTUSED(alloc);
    return su_arena_alloc(&state.text_cache_arena, &gp_alloc, size, alignment);
}

static void sw__text_cache_alloc_free(const su_allocator_t *alloc, void *ptr) {
    SU_NOTUSED(alloc); SU_NOTUSED(ptr);
}

static const su_allocator_t text_cache_alloc = { sw__text_cache_alloc_alloc, sw__text_cache_alloc_free };

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
	su_abgr_to_argb_premultiply_alpha((uint32_t *)(void *)slot->bitmap.buffer, pix, (size_t)width * (size_t)height);

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

    su_abgr_to_argb( (uint32_t *)(void *)slot->bitmap.buffer,
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

static sw_color_argb32_t color(uint32_t raw) {
    sw_color_argb32_t ret;

    uint32_t a = (raw >> 24) & 0xFF;
    uint32_t r = (raw >> 16) & 0xFF;
    uint32_t g = (raw >> 8) & 0xFF;
    uint32_t b = raw & 0xFF;

    ret.c.a = (uint8_t)a;
    ret.c.r = (uint8_t)((r * a + 127) / 255);
    ret.c.g = (uint8_t)((g * a + 127) / 255);
    ret.c.b = (uint8_t)((b * a + 127) / 255);

    return ret;
}

static su_bool32_t init(void) {
    static SVG_RendererHooks resvg_hooks_ft = { resvg_init_ft, resvg_free_ft, resvg_render_ft, resvg_preset_slot_ft };
    static struct FT_MemoryRec_ alloc_ft = { NULL, alloc_alloc_ft, alloc_free_ft, alloc_realloc_ft };

    FT_Error error = FT_New_Library(&alloc_ft, &state.ft);
    if (error != FT_Err_Ok) {
        return SU_FALSE;
    }
    FT_Add_Default_Modules(state.ft);
#if RSVG_TEST
    error = FT_Property_Set(state.ft, "ot-svg", "svg-hooks", &rsvg_hooks);
#else
    error = FT_Property_Set(state.ft, "ot-svg", "svg-hooks", &resvg_hooks_ft);
#endif
    if (error != FT_Err_Ok) {
        FT_Done_Library(state.ft);
        return SU_FALSE;
    }

    state.hb_unicode_funcs = hb_unicode_funcs_get_default();
    state.hb_buf = hb_buffer_create();
    su_arena_init(&state.text_cache_arena, &gp_alloc, 131072);
    su_hash_table__sw__text_run_cache_t__init(&state.text_cache, &text_cache_alloc, 1024);

    return SU_TRUE;
} 

static void fini(void) {
    su_arena_fini(&state.text_cache_arena, &gp_alloc);

    hb_buffer_destroy(state.hb_buf);
    /*hb_unicode_funcs_destroy(state.hb_unicode_funcs);*/

    FT_Done_Library(state.ft);
}

static void face_on_destroy_ft(void *data) {
    FT_Long i;
    FT_Face face = (FT_Face)data;
    sw_font_t *font = (sw_font_t *)face->generic.data;
    sw__font_t *font_priv = (sw__font_t *)&font->sw__private;
    hb_font_destroy((hb_font_t *)font_priv->hb_font);
    for ( i = 0; i < face->num_glyphs; ++i) {
        sw__cached_glyph_t *g = font_priv->glyph_cache[i];
        if (g) {
            SU_FREE(&gp_alloc, g->data);
            SU_FREE(&gp_alloc, g);
        }
    }
    SU_FREE(&gp_alloc, font_priv->glyph_cache);
}

static sw_pixmap_t *render(sw_layout_block_text_t *block) {
    sw_pixmap_t *ret;
    FT_Error c;
    size_t i, f = 0, r = 1, g = 0;
    int32_t pen_x = 0, pen_y = 0;
    sw__partial_run_t *pruns;
    size_t pruns_count;
    sw__partial_run_t *prun;
    uint32_t *codepoints;
    sw__text_run_t new_text_run;
    sw__text_run_cache_t *cache;
    su_bool32_t fonts_changed = SU_FALSE;
    size_t glyph_advance_field_offset;
    int32_t *axis;

    SU_ASSERT(block->fonts_count > 0);

    for ( f = 0; f < block->fonts_count; ++f) {
        sw_font_t *font = &block->fonts[f];
        /* ? TODO: hash table instead of private field */
        sw__font_t *font_priv = (sw__font_t *)&font->sw__private;

        SU_ASSERT(font->in.pixel_size > 0);
        SU_ASSERT(font->in.data.len > 0);

        if ((font_priv->idx != font->in.idx)
            || (font->in.data.len != font_priv->data.len)
            /* || (SU_MEMCMP(font_priv->data.ptr, font->in.data.ptr, font->in.data.len) != 0)*/) {
            FT_Done_Face(font_priv->face);

            c = FT_New_Memory_Face(state.ft, (FT_Byte *)font->in.data.ptr,
                (FT_Long)font->in.data.len, (FT_Long)font->in.idx, &font_priv->face);
            SU_ASSERT(c == FT_Err_Ok);
            SU_ASSERT(font_priv->face->num_glyphs > 0);

            SU_ARRAY_ALLOCC(font_priv->glyph_cache, &gp_alloc, (size_t)font_priv->face->num_glyphs);

            font_priv->face->generic.finalizer = face_on_destroy_ft;
            font_priv->face->generic.data = font;
            font_priv->idx = font->in.idx;
            font_priv->data.len = font->in.data.len;
            fonts_changed = SU_TRUE;
        }

        if ((font->in.pixel_size != font_priv->pixel_size) ||
                (font->in.glyph_load_flags != font_priv->glyph_load_flags)) {
            c = FT_Set_Pixel_Sizes(font_priv->face, 0, (FT_UInt)font->in.pixel_size);
            if (c != FT_Err_Ok) {
                FT_Pos target = ((FT_Pos)font->in.pixel_size * 64);
                FT_Int best = 0;
                FT_Int best_err = INT_MAX;
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
            /* TODO: hb_feature_from_string */
            if (!font_priv->hb_font) {
                font_priv->hb_font = hb_ft_font_create_referenced(font_priv->face);
                SU_ASSERT(font_priv->hb_font != NULL);
            }
            hb_ft_font_set_load_flags(font_priv->hb_font, (int)font->in.glyph_load_flags);
            hb_ft_font_changed(font_priv->hb_font);
            font_priv->pixel_size = font->in.pixel_size;
            font_priv->glyph_load_flags = font->in.glyph_load_flags;
            fonts_changed = SU_TRUE;
        }

        /* ? TODO: FT_Set_Transform */
    }

    if (!su_hash_table__sw__text_run_cache_t__add(&state.text_cache, &text_cache_alloc, block->text, &cache)) {
        if (!fonts_changed && (cache->faces_count == block->fonts_count) && (block->vertical == cache->text_run.vertical)) {
            for ( i = 0; i < block->fonts_count; ++i) {
                if (((sw__font_t *)block->fonts[i].sw__private)->face != cache->faces[i]) {
                    goto shape;
                }
            }
            goto render;
        }
    } else {
        su_string_init_string(&cache->key, &text_cache_alloc, cache->key);
    }

shape:
    SU_ARRAY_ALLOC(pruns, &scratch_alloc, (block->text.len * block->fonts_count) + 1);
    prun = &pruns[0];
    prun->start_idx = 0;
    prun->len = 0;
    prun->script = HB_SCRIPT_INVALID;
    pruns_count = 1;

    /* TODO: utf32 input with cache, validation */
    /*if (!block->text_utf32)*/ {
        hb_glyph_info_t *glyph_infos;
        hb_script_t prev_script = prun->script;
        unsigned int gc;

        SU_ARRAY_ALLOC(codepoints, &scratch_alloc, block->text.len);

        /* TODO: simd utf8 -> utf32 */
        hb_buffer_reset(state.hb_buf);
        hb_buffer_add_utf8(state.hb_buf, block->text.s, (int)block->text.len, 0, -1);
        glyph_infos = hb_buffer_get_glyph_infos(state.hb_buf, &gc);
        new_text_run.glyphs_count = gc;

        for ( g = 0; g < new_text_run.glyphs_count; ++g) {
            hb_script_t script = hb_unicode_script(state.hb_unicode_funcs, glyph_infos[g].codepoint);
            if ((script == HB_SCRIPT_INHERITED) || (script == prev_script)) {
                prun->len++;
            } else {
                prev_script = script;
                prun = &pruns[pruns_count++];
                prun->start_idx = g;
                prun->len = 1;
                prun->script = script;
            }
            codepoints[g] = glyph_infos[g].codepoint;
        }
    }/* else {
        hb_script_t prev_script = prun->script;

        codepoints = (uint32_t *)(void *)block->text.s;
        new_text_run.glyphs_count = block->text.len;
        for ( g = 0; g < new_text_run.glyphs_count; ++g) {
            hb_script_t script = hb_unicode_script(state.hb_unicode_funcs, codepoints[g]);
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
    }*/

    new_text_run.vertical = block->vertical;
    new_text_run.w = new_text_run.h = 0;
    if (cache->text_run.glyphs &&
            (cache->text_run.glyphs_count <= new_text_run.glyphs_count)) {
        new_text_run.glyphs = cache->text_run.glyphs;
    } else {
        SU_ARRAY_ALLOCC(new_text_run.glyphs, &text_cache_alloc, new_text_run.glyphs_count);
    }

    if (block->vertical) {
        axis = &new_text_run.h;
        glyph_advance_field_offset = SU_OFFSETOF(sw__glyph_t, y_advance);
    } else {
        axis = &new_text_run.w;
        glyph_advance_field_offset = SU_OFFSETOF(sw__glyph_t, x_advance);
    }

    for ( f = 0; f < block->fonts_count; ++f) {
        sw_font_t *font = &block->fonts[f];
        sw__font_t *font_priv = (sw__font_t *)&font->sw__private;
        size_t pc = pruns_count;

        /* TODO: properly handle pruns with partial font coverage */
        for ( ; r < pc; ++r) {
            size_t insert_idx;
            unsigned int gc;
            hb_glyph_info_t *glyph_infos;
            hb_glyph_position_t *glyph_pos;
            hb_direction_t dir;

            prun = &pruns[r];
            insert_idx = prun->start_idx;

            hb_buffer_reset(state.hb_buf);
            hb_buffer_add_codepoints( state.hb_buf, codepoints,
                (int)new_text_run.glyphs_count, (unsigned int)prun->start_idx, (int)prun->len);

            hb_buffer_guess_segment_properties(state.hb_buf);

            dir = hb_buffer_get_direction(state.hb_buf);
            if (!block->vertical && (dir != HB_DIRECTION_RTL)) {
                hb_buffer_set_direction(state.hb_buf, HB_DIRECTION_LTR);
            } else if (block->vertical && (dir != HB_DIRECTION_BTT)) {
                hb_buffer_set_direction(state.hb_buf, HB_DIRECTION_TTB);
            }

            hb_shape(font_priv->hb_font, state.hb_buf, NULL, 0);

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

                if (idx == 0) {
                    uint32_t start_idx = glyph_infos[g].cluster;
                    hb_script_t script = hb_unicode_script(state.hb_unicode_funcs, codepoints[start_idx]);
                    sw__partial_run_t *pr = &pruns[pruns_count - 1];
                    size_t len = 1;

                    start_idx = glyph_infos[g].cluster;

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
                    glyph_cache = ((sw__font_t *)glyph->font->sw__private)->glyph_cache;
                    face = ((sw__font_t *)glyph->font->sw__private)->face;
                } else {
                    glyph = &new_text_run.glyphs[insert_idx++];
                    glyph_priv = (sw__glyph_t *)&glyph->sw__private;
                    glyph->font = font;
                    glyph_cache = font_priv->glyph_cache;
                    face = font_priv->face;
                    glyph_advance = (int32_t *)(void *)((uint8_t *)glyph_priv + glyph_advance_field_offset);
                    glyph_priv->valid = SU_FALSE;

                    *axis -= *glyph_advance;
                }

                if (!glyph_priv->valid) {
                    sw__cached_glyph_t *cached_glyph = glyph_cache[idx];
                    if ( !cached_glyph
                            || (cached_glyph->pixel_size != glyph->font->in.pixel_size)
                            || (cached_glyph->load_flags != glyph->font->in.glyph_load_flags)
                            || (cached_glyph->render_mode != glyph->font->in.glyph_render_mode)) {
                        FT_Bitmap *ft_bitmap;
                        su_bool32_t scale;

                        if (cached_glyph) {
                            SU_FREE(&gp_alloc, cached_glyph->data);
                        } else {
                            SU_ALLOCT(glyph_cache[idx], &gp_alloc);
                            cached_glyph = glyph_cache[idx];
                        }

                        c = FT_Load_Glyph(face, idx, (FT_Int32)glyph->font->in.glyph_load_flags);
                        SU_ASSERT(c == FT_Err_Ok);
                        scale = (face->glyph->format == FT_GLYPH_FORMAT_BITMAP);

                        /* ? TODO: FT_GlyphSlot_Embolden, FT_Library_SetLcdFilter */

                        c = FT_Render_Glyph(face->glyph, (FT_Render_Mode)glyph->font->in.glyph_render_mode);
                        SU_ASSERT(c == FT_Err_Ok);
                        SU_ASSERT(face->glyph->format == FT_GLYPH_FORMAT_BITMAP);

                        ft_bitmap = &face->glyph->bitmap;
                        SU_ASSERT(ft_bitmap->pitch >= 0); /* ? TODO: handle negative */

                        /* very dangerous */
                        cached_glyph->data = (uint8_t *)ft_bitmap->buffer;
                        ft_bitmap->buffer = NULL;

                        cached_glyph->width = ft_bitmap->width;
                        cached_glyph->height = ft_bitmap->rows;
                        cached_glyph->pitch = (uint32_t)ft_bitmap->pitch;
                        cached_glyph->pixel_mode = (FT_Pixel_Mode)ft_bitmap->pixel_mode;
                        cached_glyph->left = face->glyph->bitmap_left;
                        cached_glyph->top = face->glyph->bitmap_top;
                        cached_glyph->pixel_size = glyph->font->in.pixel_size;
                        cached_glyph->load_flags = glyph->font->in.glyph_load_flags;
                        cached_glyph->render_mode = glyph->font->in.glyph_render_mode;
                        cached_glyph->scale_factor = ((scale && (ft_bitmap->width > 0) && (ft_bitmap->rows > 0))
                            ? SU_MIN(
                                (float)glyph->font->in.pixel_size / (float)ft_bitmap->width,
                                (float)glyph->font->in.pixel_size / (float)ft_bitmap->rows)
                            : 1.f);
                    }

                    glyph_priv->x_advance = SU_ABS((int)((((float)glyph_pos[g].x_advance / 64.f) + 0.5f) * cached_glyph->scale_factor));
                    glyph_priv->y_advance = SU_ABS((int)((((float)glyph_pos[g].y_advance / 64.f) + 0.5f) * cached_glyph->scale_factor));
                    glyph_priv->x_offset = (int32_t)(((float)glyph_pos[g].x_offset / 64.f) + 0.5f);
                    glyph_priv->y_offset = (int32_t)(((float)glyph_pos[g].y_offset / 64.f) + 0.5f);
                    glyph_priv->idx = idx;
                    glyph_priv->valid = SU_TRUE;
                    
                    if (SU_UNLIKELY((*glyph_advance == 0) && (cached_glyph->scale_factor != 1.f))) {
                        /* TODO: rework */
                        *glyph_advance = (int32_t)font->in.pixel_size;
                    }

                    *axis += *glyph_advance;
                }
            }
        }
    }

    if (block->fonts_count > cache->faces_count) {
        SU_ARRAY_ALLOC(cache->faces, &text_cache_alloc, block->fonts_count);
    }
    cache->text_run = new_text_run;
    for ( i = 0; i < block->fonts_count; ++i) {
        cache->faces[i] = ((sw__font_t *)block->fonts[i].sw__private)->face;
    }
    cache->faces_count = block->fonts_count;

render:
    if (block->vertical) {
        FT_Size_Metrics *metrics = &((sw__font_t *)block->fonts[0].sw__private)->face->size->metrics;
        cache->text_run.w = (int32_t)(((float)metrics->height / 64.f) + 0.5f); /* TODO: width analog */
        pen_x = (cache->text_run.w / 2);
        axis = &pen_y;
        glyph_advance_field_offset = SU_OFFSETOF(sw__glyph_t, y_advance);
    } else {
        FT_Size_Metrics *metrics = &((sw__font_t *)block->fonts[0].sw__private)->face->size->metrics;
        cache->text_run.h = (int32_t)(((float)metrics->height / 64.f) + 0.5f);
        pen_y = (int32_t)((float)cache->text_run.h + ((float)metrics->descender / 64.f) + 0.5f);
        axis = &pen_x;
        glyph_advance_field_offset = SU_OFFSETOF(sw__glyph_t, x_advance);
    }

    SU_ASSERT(cache->text_run.w > 0);
    SU_ASSERT(cache->text_run.h > 0);

    SU_ALLOCTS( ret, &gp_alloc,
        (sizeof(ret->width) + sizeof(ret->height) + (size_t)(cache->text_run.w * cache->text_run.h * 4)));
    ret->width = (uint32_t)cache->text_run.w;
    ret->height = (uint32_t)cache->text_run.h;
    SU_MEMSET(ret->pixels, 0x00, (size_t)(cache->text_run.w * cache->text_run.h * 4));

    for ( g = 0; g < cache->text_run.glyphs_count; ++g) {
        sw_glyph_t *glyph = &cache->text_run.glyphs[g];
        sw__glyph_t *glyph_priv = (sw__glyph_t *)&glyph->sw__private;
        int32_t *glyph_advance;
        sw_font_t *font;
        sw__font_t *font_priv;
        sw__cached_glyph_t *cached_glyph;

        if (!glyph_priv->valid) {
            continue;
        }

        font = glyph->font;
        glyph_advance = (int32_t *)(void *)((uint8_t *)glyph_priv + glyph_advance_field_offset);

        font_priv = (sw__font_t *)font->sw__private;
        SU_ASSERT(glyph_priv->idx < font_priv->face->num_glyphs);

        cached_glyph = font_priv->glyph_cache[glyph_priv->idx];
        SU_ASSERT(cached_glyph != NULL);

        /* TODO: simd, blend */
        if (cached_glyph->data) {
            int32_t y, x;
            int32_t start_x = 0, start_y = 0;
            int32_t end_x, end_y;
            int32_t dst_x_offs, dst_y_offs;

            if (cached_glyph->scale_factor != 1.f) {
                end_x = (int32_t)((float)cached_glyph->width * cached_glyph->scale_factor + 0.5f);
                end_y = (int32_t)((float)cached_glyph->height * cached_glyph->scale_factor + 0.5f);
                if (block->vertical) {
                    dst_x_offs = ((cache->text_run.w - end_x) / 2);
                    dst_y_offs = pen_y;
                } else {
                    dst_x_offs = pen_x;
                    dst_y_offs = ((cache->text_run.h - end_y) / 2);
                }

                if (dst_x_offs < 0) {
                    start_x = -dst_x_offs;
                }
                if ((dst_x_offs + end_x) > cache->text_run.w) {
                    end_x = (cache->text_run.w - dst_x_offs);
                }
                if ((dst_x_offs > cache->text_run.w) || (dst_y_offs > cache->text_run.h) || (start_x >= end_x)) {
                    goto next_glyph;
                }
                if (dst_y_offs < 0) {
                    start_y = -dst_y_offs;
                }
                if ((dst_y_offs + end_y) > cache->text_run.h) {
                    end_y = (cache->text_run.h - dst_y_offs);
                }

#define FILTER 1 /* 0 - bilinear, 1 - nearest */ /* TODO: more filters */

                /* TODO: optimize */
                switch (cached_glyph->pixel_mode) {
                case FT_PIXEL_MODE_GRAY:
#if (FILTER == 0)
                    for ( y = start_y; y < end_y; ++y) {
                        int32_t src_y = (int32_t)((float)y / cached_glyph->scale_factor);
                        for ( x = start_x; x < end_x; ++x) {
                            int32_t src_x = (int32_t)((float)x / cached_glyph->scale_factor);
                            uint8_t src = cached_glyph->data[src_y * (int32_t)cached_glyph->pitch + src_x];
                            sw_color_argb32_t *dst = &ret->pixels[(y + dst_y_offs) * cache->text_run.w + x + dst_x_offs];
                            dst->c.a = (uint8_t)((block->text_color.c.a * src) >> 8);
                            dst->c.r = (uint8_t)((block->text_color.c.r * src) >> 8);
                            dst->c.g = (uint8_t)((block->text_color.c.g * src) >> 8);
                            dst->c.b = (uint8_t)((block->text_color.c.b * src) >> 8);
                        }
                    }
#elif (FILTER == 1)
                    for ( y = start_y; y < end_y; ++y) {
                        float fy = ((float)y / cached_glyph->scale_factor);
                        int32_t y0 = (int32_t)fy;
                        float wy = (fy - (float)y0);
                        int32_t y1 = (y0 + 1);
                        uint8_t *row0, *row1;
                        if (y1 >= (int32_t)cached_glyph->height) {
                            y1 = ((int32_t)cached_glyph->height - 1);
                        }
                        row0 = &cached_glyph->data[y0 * (int32_t)cached_glyph->pitch];
                        row1 = &cached_glyph->data[y1 * (int32_t)cached_glyph->pitch];
                        for ( x = start_x; x < end_x; ++x) {
                            float fx = ((float)x / cached_glyph->scale_factor);
                            int32_t x0 = (int32_t)fx;
                            float wx = (fx - (float)x0);
                            int32_t x1 = (x0 + 1);
                            if (x1 >= (int32_t)cached_glyph->width) {
                                x1 = ((int32_t)cached_glyph->width - 1);
                            }
                            {
                                float c00 = (float)row0[x0];
                                float c10 = (float)row0[x1];
                                float c01 = (float)row1[x0];
                                float c11 = (float)row1[x1];
                                float w00 = ((1.0f - wx) * (1.0f - wy));
                                float w10 = (wx * (1.0f - wy));
                                float w01 = ((1.0f - wx) * wy);
                                float w11 = (wx * wy);
                                float cov = (c00 * w00 + c10 * w10 + c01 * w01 + c11 * w11);
                                sw_color_argb32_t *d = &ret->pixels[(y + dst_y_offs) * cache->text_run.w + (x + dst_x_offs)];
                                d->c.a = (uint8_t)((uint32_t)(block->text_color.c.a * cov + 127.0f) / 255);
                                d->c.r = (uint8_t)((block->text_color.c.r * d->c.a + 127) / 255);
                                d->c.g = (uint8_t)((block->text_color.c.g * d->c.a + 127) / 255);
                                d->c.b = (uint8_t)((block->text_color.c.b * d->c.a + 127) / 255);
                            }
                        }
                    }
#endif
                    break;
                case FT_PIXEL_MODE_BGRA:
#if (FILTER == 0)
                    for ( y = start_y; y < end_y; ++y) {
                        int32_t src_y = (int32_t)((float)y / cached_glyph->scale_factor);
                        for ( x = start_x; x < end_x; ++x) {
                            int32_t src_x = (int32_t)((float)x / cached_glyph->scale_factor);
                            sw_color_argb32_t *src = (sw_color_argb32_t *)(void *)&cached_glyph->data[
                                src_y * (int32_t)cached_glyph->pitch + src_x * 4];
                            sw_color_argb32_t *dst = &ret->pixels[(y + dst_y_offs) * cache->text_run.w + x + dst_x_offs];
                            *dst = *src;
                        }
                    }
#elif (FILTER == 1)
                    for ( y = start_y; y < end_y; ++y) {
                        float fy = ((float)y / cached_glyph->scale_factor);
                        int32_t y0 = (int32_t)fy;
                        float wy = (fy - (float)y0);
                        int32_t y1 = (y0 + 1);
                        if (y1 >= (int32_t)cached_glyph->height) {
                            y1 = ((int32_t)cached_glyph->height - 1);
                        }
                        for ( x = start_x; x < end_x; ++x) {
                            float fx = ((float)x / cached_glyph->scale_factor);
                            int32_t x0 = (int32_t)fx;
                            float wx = (fx - (float)x0);
                            int32_t x1 = (x0 + 1);
                            if (x1 >= (int32_t)cached_glyph->width) {
                                x1 = ((int32_t)cached_glyph->width - 1);
                            }
                            {
                                sw_color_argb32_t c00 = *(sw_color_argb32_t *)(void *)&cached_glyph->data[y0 * (int32_t)cached_glyph->pitch + x0 * 4];
                                sw_color_argb32_t c10 = *(sw_color_argb32_t *)(void *)&cached_glyph->data[y0 * (int32_t)cached_glyph->pitch + x1 * 4];
                                sw_color_argb32_t c01 = *(sw_color_argb32_t *)(void *)&cached_glyph->data[y1 * (int32_t)cached_glyph->pitch + x0 * 4];
                                sw_color_argb32_t c11 = *(sw_color_argb32_t *)(void *)&cached_glyph->data[y1 * (int32_t)cached_glyph->pitch + x1 * 4];
                                float w00 = ((1.0f - wx) * (1.0f - wy));
                                float w10 = (wx * (1.0f - wy));
                                float w01 = ((1.0f - wx) * wy);
                                float w11 = (wx * wy);
                                sw_color_argb32_t *d = &ret->pixels[(y + dst_y_offs) * cache->text_run.w + (x + dst_x_offs)];
                                d->c.a = (uint8_t)(c00.c.a * w00 + c10.c.a * w10 + c01.c.a * w01 + c11.c.a * w11);
                                d->c.r = (uint8_t)(c00.c.r * w00 + c10.c.r * w10 + c01.c.r * w01 + c11.c.r * w11);
                                d->c.g = (uint8_t)(c00.c.g * w00 + c10.c.g * w10 + c01.c.g * w01 + c11.c.g * w11);
                                d->c.b = (uint8_t)(c00.c.b * w00 + c10.c.b * w10 + c01.c.b * w01 + c11.c.b * w11);
                            }
                        }
                    }
#endif
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
                end_x = (int32_t)cached_glyph->width;
                end_y = (int32_t)cached_glyph->height;
                dst_x_offs = (pen_x + (cached_glyph->left + glyph_priv->x_offset));
                dst_y_offs = (pen_y - (cached_glyph->top + glyph_priv->y_offset));

                if (dst_x_offs < 0) {
                    start_x = -dst_x_offs;
                }
                if ((dst_x_offs + end_x) > cache->text_run.w) {
                    end_x = (cache->text_run.w - dst_x_offs);
                }
                if ((dst_x_offs > cache->text_run.w) || (dst_y_offs > cache->text_run.h) || (start_x >= end_x)) {
                    goto next_glyph;
                }
                if (dst_y_offs < 0) {
                    start_y = -dst_y_offs;
                }
                if ((dst_y_offs + end_y) > cache->text_run.h) {
                    end_y = (cache->text_run.h - dst_y_offs);
                }

                switch (cached_glyph->pixel_mode) {
                case FT_PIXEL_MODE_GRAY:
                    for ( y = start_y; y < end_y; ++y) {
                        uint8_t *src = &cached_glyph->data[y * (int32_t)cached_glyph->pitch + start_x];
                        sw_color_argb32_t *dst = &ret->pixels[(y + dst_y_offs) * cache->text_run.w + dst_x_offs + start_x];
                        for ( x = start_x; x < end_x; ++x) {
                            uint8_t m = *src++;
                            sw_color_argb32_t *d = dst++;
                            d->c.a = (uint8_t)((block->text_color.c.a * m) >> 8);
                            d->c.r = (uint8_t)((block->text_color.c.r * m) >> 8);
                            d->c.g = (uint8_t)((block->text_color.c.g * m) >> 8);
                            d->c.b = (uint8_t)((block->text_color.c.b * m) >> 8);
                        }
                    }
                    break;
                case FT_PIXEL_MODE_BGRA: {
                    size_t span_bytes = ((size_t)(end_x - start_x) * 4);
                    for ( y = start_y; y < end_y; ++y) {
                        SU_MEMCPY(
                            &ret->pixels[(y + dst_y_offs) * cache->text_run.w + (dst_x_offs + start_x)],
                            &cached_glyph->data[y * (int32_t)cached_glyph->pitch + start_x * 4],
                            span_bytes);
                    }
                    break;
                }
                case FT_PIXEL_MODE_MONO: {
                    static const uint8_t mask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
                    for (y = start_y; y < end_y; ++y) {
                        uint8_t *src = &cached_glyph->data[y * (int32_t)cached_glyph->pitch + start_x];
                        sw_color_argb32_t *dst =
                            ret->pixels + (y + dst_y_offs) * cache->text_run.w + dst_x_offs + start_x;
                        for (x = start_x; x < end_x; ++x) {
                            dst[x].u32 = (src[x >> 3] & mask[x & 7]) ? block->text_color.u32 : 0;
                        }
                    }
                    break;
                }
                case FT_PIXEL_MODE_LCD:
                    if (end_x > ((int32_t)cached_glyph->width / 3)) {
                        end_x = ((int32_t)cached_glyph->width / 3);
                    }
                    for ( y = start_y; y < end_y; ++y) {
                        uint8_t *src = &cached_glyph->data[y * (int32_t)cached_glyph->pitch + start_x * 3];
                        sw_color_argb32_t *dst = &ret->pixels[(y + dst_y_offs) * cache->text_run.w + dst_x_offs + start_x];
                        for ( x = start_x; x < end_x; ++x) {
                            sw_color_argb32_t *d = dst++;
                            d->c.a = 0xff;
                            d->c.r = (uint8_t)((block->text_color.c.r * src[0]) >> 8);
                            d->c.g = (uint8_t)((block->text_color.c.g * src[1]) >> 8);
                            d->c.b = (uint8_t)((block->text_color.c.b * src[2]) >> 8);
                            src += 3;
                        }
                    }
                    break;
                case FT_PIXEL_MODE_LCD_V: {
                    int32_t dst_row = start_y;
                    if (end_y > ((int32_t)cached_glyph->height / 3)) {
                        end_y = ((int32_t)cached_glyph->height / 3);
                    }
                    for ( y = (start_y * 3); dst_row < end_y; y += 3, ++dst_row) {
                        uint8_t *src = &cached_glyph->data[y * (int32_t)cached_glyph->pitch + start_x];
                        sw_color_argb32_t *dst = &ret->pixels[(dst_row + dst_y_offs) * cache->text_run.w + dst_x_offs + start_x];
                        for ( x = start_x; x < end_x; ++x) {
                            sw_color_argb32_t *d = dst++;
                            d->c.a = 0xff; /*text_color.c.a;*/
                            d->c.r = (uint8_t)((block->text_color.c.r * src[0 * cached_glyph->pitch]) >> 8);
                            d->c.g = (uint8_t)((block->text_color.c.g * src[1 * cached_glyph->pitch]) >> 8);
                            d->c.b = (uint8_t)((block->text_color.c.b * src[2 * cached_glyph->pitch]) >> 8);
                            src++;
                        }
                    }
                    break;
                }
                case FT_PIXEL_MODE_NONE:
                case FT_PIXEL_MODE_GRAY2:
                case FT_PIXEL_MODE_GRAY4:
                case FT_PIXEL_MODE_MAX:
                default:
                    SU_ASSERT_UNREACHABLE;
                }
            }
        }
next_glyph:
        *axis += *glyph_advance;
    }

    return ret;
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
        /*"/usr/share/fonts/nerdfonts/CaskaydiaCoveNerdFont-Regular.ttf", */
        /*"/usr/share/fonts/noto/NotoSansDevanagari-Regular.ttf",*/
        "/usr/share/fonts/cascadia-code/CascadiaMono.ttf",
        "/usr/share/fonts/liberation-fonts/LiberationSans-Regular.ttf",
#if 1
        "/usr/share/fonts/noto-emoji/NotoColorEmoji.ttf", /* png */
#else
        "/home/user/Downloads/tmp/TwitterColorEmoji-SVGinOT-Linux-15.1.0/TwitterColorEmoji-SVGinOT.ttf", /* svg */
#endif
        "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/noto/NotoSansDevanagari-Regular.ttf",
        "/usr/share/fonts/noto/NotoSansMongolian-Regular.ttf",
        "/usr/share/fonts/noto/NotoSansPhagsPa-Regular.ttf",
    };
    static sw_font_t fonts[SU_LENGTH(font_files)];
    /* "hello world | fi | اَلْعَرَبِيَّةُ | עִבְרִית | 👨‍👩‍👧‍👦 🇸🇪 🧍‍♀️ | привет, мир! | 你好世界" */
#if 1
    block.text = su_string_("hello world | fi | اَلْعَرَبِيَّةُ | עִבְרִית | 👨‍👩‍👧‍👦 🇸🇪 🧍‍♀️ | привет, мир! | 你好世界");
#elif 0
    block.text = su_string_("\xF0\x9D\xBC\x80 \xF0\xAB\xA0\x9D \xF0\x96\xBF\xA4 \xF0\x91\xBC\x82 \xF0\x90\xBB\xBD \xF0\x9A\xBF\xB0 \xF0\x93\x90\xB0     \x80 \xC0\xAF \xC3 \xE2\x28\xA1 \xF0\x9F\x92 \xF4\x90\x80\x80 \xED\xA0\x80");
#elif 0
    block.text = su_string_(/*"\xCC\x81 A\xCC\x81\xCC\x80" "\xD9\x8E"*/ "\xEF\xB7\xB2" /*"\xE0\xA5\x98"*/);
#elif 0
    block.text = su_string_("\xE1\xA0\xA0\xE1\xA0\xA8\xE1\xA0\xB6 | \xEA\xA1\x80\xEA\xA1\x83");
#else
    block.text = su_string_("\xF0\x9D\xBC\x80 \xF0\xAB\xA0\x9D \xF0\x96\xBF\xA4 \xF0\x91\xBC\x82 \xF0\x90\xBB\xBD \xF0\x9A\xBF\xB0 \xF0\x93\x90\xB0     \x80 \xC0\xAF \xC3 \xE2\x28\xA1 \xF0\x9F\x92 \xF4\x90\x80\x80 \xED\xA0\x80 | \xCC\x81 A\xCC\x81\xCC\x80 \xD9\x8E \xEF\xB7\xB2 \xE0\xA5\x98");
#endif
    block.text_color = color(0xFFFFFFFF);
    block.fonts = fonts;
    block.fonts_count = SU_LENGTH(font_files);
    block.vertical = SU_FALSE;

    setlocale(LC_ALL, "");
    SU_ASSERT(su_locale_is_utf8());

    su_arena_init(&state.scratch_arena, &gp_alloc, 16384);

    c = init();
    SU_ASSERT(c == SU_TRUE);

    for ( i = 0; i < SU_LENGTH(font_files); ++i) {
        sw_font_t *font = &block.fonts[i];
        su_read_entire_file(su_string_(font_files[i]), &font->in.data, &gp_alloc);
        font->in.pixel_size = 48;
        font->in.idx = 0;
        font->in.glyph_load_flags = SW_GLYPH_LOAD_FLAG_COLOR;
        font->in.glyph_render_mode = SW_GLYPH_RENDER_MODE_NORMAL;
    }

    sw.in.backend_type = SW_BACKEND_TYPE_WAYLAND;
    sw.in.gp_alloc = &gp_alloc;
    sw.in.scratch_alloc = &scratch_alloc;
    sw.in.update_and_render = SU_TRUE;

    /* hack for stbi allocator */
    sw__context = &sw;
    state.running = SU_TRUE;

    pm = render(&block);
/*#if 0
    block.fonts[0].in.glyph_load_flags = 0;
#else
    block.text = su_string_("hello world");
#endif
    block.vertical = SU_FALSE;
    pm = render(&block);*/

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
        SU_FREE(&gp_alloc, block.fonts[i].in.data.ptr);
    }

    SU_FREE(&gp_alloc, pm);

    SU_NOTUSED(c);
    return 0;
}
