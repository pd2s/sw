#define _XOPEN_SOURCE 700
#define SU_STRIP_PREFIXES
#define SU_IMPLEMENTATION
#include "sutil.h"

#define SW_IMPLEMENTATION
#include "swidgets.h"

#include <locale.h>
#include <signal.h>

#include <ft2build.h>
#include <freetype/ftmodapi.h>
#include <freetype/otsvg.h>
#include <harfbuzz/hb-ft.h>

#define RSVG_TEST 0
#if RSVG_TEST
#include "new_renderer_and_text_processing_rsvg_test.h"
#endif

typedef struct font {
    /* in */
    fat_ptr_t data;
    double pixel_size;
    size_t idx;

    /* out */
    FT_Face face;
    hb_font_t *hb_font;
} font_t;

typedef struct glyph {
    hb_codepoint_t idx;
    bool32_t valid;
    FT_Face face;
    int32_t x_advance;
    int32_t y_advance;
    int32_t x_offset;
    int32_t y_offset;
} glyph_t;

typedef struct partial_run {
    size_t start_idx, len;
    hb_script_t script;
    PAD32;
} partial_run_t;


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
static fat_ptr_t p;
static arena_t scratch_arena;
static bool32_t running = TRUE;

static void *gp_alloc_alloc(allocator_t *alloc, size_t size, size_t alignment) {
	void *ptr;
	int s;

	NOTUSED(alloc);

	ASSERT(size > 0);
	ASSERT((alignment > 0) && ((alignment == 1) || ((alignment & (alignment - 1)) == 0)));

	alignment = MAX(alignment, sizeof(void *));

	s = posix_memalign(&ptr, alignment, (size + alignment - 1) & ~(alignment - 1));
	if ( UNLIKELY(s != 0)) {
		su_abort(s, "posix_memalign: %s", strerror(s));
	}
	ASSERT(((uintptr_t)ptr % alignment) == 0);
	return ptr;
}

static void gp_alloc_free(allocator_t *alloc, void *ptr) {
	NOTUSED(alloc);
	free(ptr);
}

static void *gp_alloc_realloc(allocator_t *alloc, void *ptr, size_t new_size, size_t new_alignment) {
	void *ret = realloc(ptr, new_size); /* TODO: alignment */

	NOTUSED(alloc);
	NOTUSED(new_alignment);

	if (UNLIKELY(!ret)) {
		su_abort(errno, "realloc: %s", strerror(errno));
	}
	return ret;
}

static allocator_t gp_alloc = { gp_alloc_alloc, gp_alloc_realloc, gp_alloc_free };

static void *scratch_alloc_alloc(allocator_t *alloc, size_t size, size_t alignment) {
	void *ret = arena_alloc(&scratch_arena, &gp_alloc, size, alignment);
	NOTUSED(alloc);
	return ret;
}

static void scratch_alloc_free(allocator_t *alloc, void *ptr) {
	NOTUSED(alloc); NOTUSED(ptr);
}

static void *scratch_alloc_realloc(allocator_t *alloc, void *ptr, size_t new_size, size_t new_alignment) {
	void *ret = arena_alloc(&scratch_arena, &gp_alloc, new_size, new_alignment);

	NOTUSED(alloc);

	if (ptr) {
		memcpy(ret, ptr, MIN(new_size, arena_alloc_get_size(ptr)));
	}
	return ret;
}

static allocator_t scratch_alloc = { scratch_alloc_alloc, scratch_alloc_realloc, scratch_alloc_free };

static void *scratch_alloc_alloc_ft(FT_Memory memory, long size) {
    NOTUSED(memory);
    return (size > 0) ? scratch_alloc.alloc(&scratch_alloc, (size_t)size, 32) : NULL;
}

static void scratch_alloc_free_ft(FT_Memory memory, void *ptr) {
    NOTUSED(memory); NOTUSED(ptr);
}

static void *scratch_alloc_realloc_ft(FT_Memory memory, long old_size, long new_size, void* ptr) {
    NOTUSED(memory); NOTUSED(old_size);
    return (new_size > 0) ? scratch_alloc.realloc(&scratch_alloc, ptr, (size_t)new_size, 32) : NULL;
}

void *hb_malloc_impl(size_t size) {
    return (size > 0) ? scratch_alloc.alloc(&scratch_alloc, size, 16) : NULL;
}

void *hb_calloc_impl(size_t nmemb, size_t size) {
    size_t s = (size * nmemb);
    if (s > 0) {
        void *ptr = scratch_alloc.alloc(&scratch_alloc, s, 16);
        memset(ptr, 0, s);
        return ptr;
    } else {
        return NULL;
    }
}

void *hb_realloc_impl(void *ptr, size_t size) {
    return (size > 0) ? scratch_alloc.realloc(&scratch_alloc, ptr, size, 16) : NULL;
}

void hb_free_impl(void *ptr) {
    NOTUSED(ptr);
}

FT_Error Load_SBit_Png(FT_GlyphSlot slot, FT_Int x_offset, FT_Int y_offset, FT_Int pix_bits,
        TT_SBit_Metrics metrics, FT_Memory memory, FT_Byte *data, FT_UInt png_len,
        FT_Bool populate_map_and_metrics, FT_Bool metrics_only) {
	stbi__result_info ri;
	stbi__context ctx;
    int width, height, notused;
	uint32_t *pix;

    NOTUSED(memory);
    
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

	memset(&ri, 0, sizeof(ri));
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
        slot->bitmap.buffer = scratch_alloc.alloc(&scratch_alloc,
            (slot->bitmap.rows * (size_t)slot->bitmap.pitch), 64);
    }

	if (ri.bits_per_channel != 8) {
		ASSERT(ri.bits_per_channel == 16);
		pix = (uint32_t *)(void *)stbi__convert_16_to_8((stbi__uint16 *)pix, width, height, 4);
	}
	abgr_to_argb_premultiply_alpha((uint32_t *)(void *)slot->bitmap.buffer, pix, (size_t)width * (size_t)height);

    return FT_Err_Ok;
}

static FT_Error resvg_init_ft(FT_Pointer *data_pointer) {
    NOTUSED(data_pointer);
    return FT_Err_Ok;
}

static void resvg_free_ft(FT_Pointer *data_pointer) {
    NOTUSED(data_pointer);
}

static FT_Error resvg_render_ft(FT_GlyphSlot slot, FT_Pointer *data_pointer) {
    FT_Error ret = FT_Err_Ok;

    NOTUSED(data_pointer);

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

    abgr_to_argb( (uint32_t *)(void *)slot->bitmap.buffer,
        (uint32_t *)(void *)slot->bitmap.buffer,
        slot->bitmap.width * slot->bitmap.rows);

    slot->bitmap.pixel_mode = FT_PIXEL_MODE_BGRA;
    slot->bitmap.num_grays = 256;
    slot->format = FT_GLYPH_FORMAT_BITMAP;

out:
    if (resvg_render_ft_state.tree) {
        resvg_tree_destroy(resvg_render_ft_state.tree);
    }
    memset(&resvg_render_ft_state, 0, sizeof(resvg_render_ft_state));
    return ret;
}

static FT_Error resvg_preset_slot_ft(FT_GlyphSlot slot, FT_Bool cache, FT_Pointer *data_pointer) {
    FT_Error ret = FT_Err_Ok;
    FT_SVG_Document doc = (FT_SVG_Document)slot->other;
    resvg_options *options = resvg_options_create();
    resvg_rect bbox = { 0, 0, 0, 0 };
    float w, h;
    resvg_render_ft_state_t local;

    float descender = ((float)doc->metrics.descender / 64.f);
    float line_height = ((float)doc->metrics.height / 64.f);
    float baseline = (line_height + descender);
    float line_center;

    resvg_render_ft_state_t *state = (cache ? &resvg_render_ft_state : &local);

    NOTUSED(data_pointer);

    if (RESVG_OK != resvg_parse_tree_from_data( (const char *)doc->svg_document,
            (uintptr_t)doc->svg_document_length, options, &state->tree)) {
        memset(state, 0, sizeof(*state));
        ret = FT_Err_Invalid_SVG_Document;
        goto out;
    }

    if (resvg_get_image_bbox(state->tree, &bbox)) {
        w = bbox.width;
        h = bbox.height;
    } else {
        resvg_size size = resvg_get_image_size(state->tree);
        w = size.width;
        h = size.height;
    }

    if ((w <= 0) || (h <= 0)) {
        w = doc->units_per_EM;
        h = doc->units_per_EM;
    }

    state->transform.a = ((float)doc->metrics.x_ppem / w);
    state->transform.d = ((float)doc->metrics.y_ppem / h);
    state->transform.e = ((-bbox.x * state->transform.a) + ((float)doc->delta.x / 64.f));
    state->transform.f = ((-bbox.y * state->transform.d) + ((float)doc->delta.y / 64.f));

    /* TODO: doc->transform */
    state->transform.b = 0;
    state->transform.c = 0;

    w *= state->transform.a;
    h *= state->transform.d;

    line_center = (line_height - h) / 2.f;

    /* ? TODO: match reference implementation */
    slot->bitmap_left = (doc->metrics.x_ppem >= w)
        ? (FT_Int)(((float)doc->metrics.x_ppem - w) / 2.f)
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
        memset(state->id, 0, sizeof(state->id));
    } else {
        su_snprintf(state->id, sizeof(state->id), "#glyph%u", slot->glyph_index);
    }

    if (!cache && state->tree) {
        resvg_tree_destroy(state->tree);
    }

out:
    if (options) {
        resvg_options_destroy(options);
    }
    return ret;
}

static sw_pixmap_t *render(void) {
    string_t text = string("hello world | fi | اَلْعَرَبِيَّةُ | עִבְרִית‎ | 👨‍👩‍👧‍👦 🇸🇪 | привет, мир! | 你好世界"); /* "hello world | fi | اَلْعَرَبِيَّةُ | עִבְרִית‎ | 👨‍👩‍👧‍👦 🇸🇪 | привет, мир! | 你好世界" */
    sw_color_argb32_t text_color = { 0xFFFFFFFF };
    static char *font_files[] = {
        /*"/usr/share/fonts/nerdfonts/CaskaydiaCoveNerdFont-Regular.ttf", */
        "/usr/share/fonts/cascadia-code/CascadiaMono.ttf",
        "/usr/share/fonts/liberation-fonts/LiberationSans-Regular.ttf",
#if 0
        "/usr/share/fonts/noto-emoji/NotoColorEmoji.ttf", /* png */
#else
        "/home/user/Downloads/tmp/TwitterColorEmoji-SVGinOT-Linux-15.1.0/TwitterColorEmoji-SVGinOT.ttf", /* svg */
#endif
        "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc",
    };

    font_t fonts[LENGTH(font_files)];

    sw_pixmap_t *ret;
    static SVG_RendererHooks resvg_hooks_ft = { resvg_init_ft, resvg_free_ft, resvg_render_ft, resvg_preset_slot_ft };
    static struct FT_MemoryRec_ ft_alloc = { NULL, scratch_alloc_alloc_ft, scratch_alloc_free_ft, scratch_alloc_realloc_ft };
    FT_Library ft;
    FT_Error e;
    size_t f, r = 0, g;
    uint32_t w = 0, h = 0;
    int32_t pen_x = 0;
    int32_t pen_y = 0; /* baseline */
    hb_glyph_info_t *glyph_infos;
    hb_glyph_position_t *glyph_pos;
    hb_buffer_t *hb_buf;
    partial_run_t *prun;

    hb_unicode_funcs_t *unicode_funcs = hb_unicode_funcs_get_default();
    hb_script_t prev_script;

    partial_run_t *pruns = scratch_alloc.alloc(&scratch_alloc, (sizeof(*pruns) * text.len * LENGTH(fonts)), ALIGNOF(*pruns));
    size_t prun_count = 0;
    /* ? TODO: merge codepoints and glyphs arrays */
    uint32_t *codepoints = scratch_alloc.alloc(&scratch_alloc, (text.len * sizeof(*codepoints)), ALIGNOF(*codepoints));
    size_t codepoints_count;
    glyph_t *glyphs = scratch_alloc.alloc(&scratch_alloc, (text.len * sizeof(*glyphs)), ALIGNOF(*glyphs));


    for ( f = 0; f < LENGTH(fonts); ++f) {
        font_t *font = &fonts[f];
        read_entire_file(string(font_files[f]), &font->data, &scratch_alloc);
        font->pixel_size = 32;
        font->idx = 0;
    }


    e = FT_New_Library(&ft_alloc, &ft);
    NOTUSED(e);
    ASSERT(e == FT_Err_Ok);

    FT_Add_Default_Modules(ft);

#if RSVG_TEST
    e = FT_Property_Set(ft, "ot-svg", "svg-hooks", &rsvg_hooks);
#else
    e = FT_Property_Set(ft, "ot-svg", "svg-hooks", &resvg_hooks_ft);
#endif
    ASSERT(e == FT_Err_Ok);


    memset(glyphs, 0, (text.len * sizeof(*glyphs)));

    hb_buf = hb_buffer_create();
    hb_buffer_add_utf8(hb_buf, text.s, (int)text.len, 0, -1);
    glyph_infos = hb_buffer_get_glyph_infos(hb_buf, (unsigned int *)&codepoints_count);
    
    /*hb_buf = NULL;*/

    prun = NULL;
    prev_script = HB_SCRIPT_INHERITED;
    for ( g = 0; g < codepoints_count; ++g) {
        hb_script_t script = hb_unicode_script(unicode_funcs, glyph_infos[g].codepoint);
        if ((script == HB_SCRIPT_INHERITED) || (script == prev_script)) {
            prun->len++;
        } else {
            prev_script = script;
            prun = &pruns[prun_count++];
            prun->start_idx = g;
            prun->len = 1;
            prun->script = script;
        }
        codepoints[g] = glyph_infos[g].codepoint;
    }


    for ( f = 0; f < LENGTH(fonts); ++f) {
        font_t *font = &fonts[f];
        FT_Face face;
        bool32_t at_least_one_glyph_found = FALSE;
        size_t pc = prun_count;

        if (!font->face) {
            e = FT_New_Memory_Face(ft, font->data.ptr, (FT_Long)font->data.len, (FT_Long)font->idx, &font->face);
            ASSERT(e == FT_Err_Ok);

            e = FT_Set_Char_Size(font->face, (FT_F26Dot6)(font->pixel_size * 64.), 0, 72, 72);
            if (e != FT_Err_Ok) {
                ASSERT(e == 23);
                e = FT_Select_Size(font->face, 0);
                ASSERT(e == FT_Err_Ok);
            }

            /* TODO: FT_Set_Transform */

            font->hb_font = hb_ft_font_create_referenced(font->face);
            ASSERT(font->hb_font != NULL);

            /* TODO: hb_feature_from_string */
        }

        face = font->face;

        for ( ; r < pc; ++r) {
            size_t insert_idx;
            unsigned int gc;

            prun = &pruns[r];
            insert_idx = prun->start_idx;

            if (!hb_buf) {
                hb_buf = hb_buffer_create();
                hb_buffer_add_utf32(hb_buf, codepoints, (int)codepoints_count, (unsigned int)prun->start_idx, (int)prun->len);
            }

            hb_buffer_guess_segment_properties(hb_buf);
            hb_shape(font->hb_font, hb_buf, NULL, 0); /* TODO: features */

            glyph_infos = hb_buffer_get_glyph_infos(hb_buf, &gc);
            glyph_pos = hb_buffer_get_glyph_positions(hb_buf, &gc);

            for ( g = 0; g < gc; ++g) {
                if (glyph_infos[g].codepoint == 0) {
                    size_t start_idx = glyph_infos[g].cluster;
                    hb_script_t script = hb_unicode_script(unicode_funcs, codepoints[start_idx]);
                    partial_run_t *pr = &pruns[prun_count - 1];
                    size_t len = 1;
                    
                    while (((g + 1) < gc) && (glyph_infos[g + 1].cluster == start_idx)) {
                        g++; len++;
                    }
                    
                    /* TODO: properly handle rtl here */
                    if (((pr->start_idx + pr->len) == start_idx) &&
                            ((script == HB_SCRIPT_INHERITED) || (pr->script == script))) {
                        pr->len += len;
                    } else {
                        pr = &pruns[prun_count++];
                        pr->start_idx = start_idx;
                        pr->len = len;
                        pr->script = script;
                    }

                    insert_idx += len;
                } else {
                    glyph_t *glyph = &glyphs[insert_idx++];
                    glyph->idx = glyph_infos[g].codepoint;
                    glyph->valid = TRUE;
                    glyph->face = face;
                    glyph->x_advance = (int32_t)(ceilf((float)glyph_pos[g].x_advance / 64.f));
                    glyph->y_advance = (int32_t)(ceilf((float)glyph_pos[g].y_advance / 64.f));
                    glyph->x_offset = (int32_t)(ceilf((float)glyph_pos[g].x_offset / 64.f));
                    glyph->y_offset = (int32_t)(ceilf((float)glyph_pos[g].y_offset / 64.f));

                    w += (uint32_t)glyph->x_advance;
                    at_least_one_glyph_found = TRUE;
                }
            }

            hb_buf = NULL;
        }

        if (at_least_one_glyph_found) {
            uint32_t font_h = (uint32_t)(ceilf((float)face->size->metrics.height / 64.f));
            if (font_h > h) {
                h = font_h;
                pen_y = (int32_t)((float)h + ceilf((float)face->size->metrics.descender / 64.f));
            }
        }
    }

    ret = gp_alloc.alloc(&gp_alloc, (8 + (w * h * 4)), 8);
    memset(ret->pixels, 0, (w * h * 4));
    ret->width = w;
    ret->height = h;

    for ( g = 0; g < codepoints_count; ++g) {
        glyph_t *glyph = &glyphs[g];
        FT_Face face;
        FT_Bitmap *bitmap;

        if (!glyph->valid) {
            continue;
        }

        if (glyph->idx == 0) {
            face = fonts[0].face;
        } else {
            face = glyph->face;
        }

        bitmap = &face->glyph->bitmap;

        e = FT_Load_Glyph(face, glyph->idx, FT_LOAD_DEFAULT | FT_LOAD_COLOR); /* TODO: flags */
        ASSERT(e == FT_Err_Ok);

        /* TODO: FT_GlyphSlot_Embolden */

        /* TODO: FT_Library_SetLcdFilter */

        e = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL); /* TODO: flags */
        ASSERT(e == FT_Err_Ok);
        ASSERT(face->glyph->format == FT_GLYPH_FORMAT_BITMAP);

        if (bitmap->buffer) {
            int32_t row, col;
            int32_t start_row = 0, start_col = 0;
            int32_t end_row = (int32_t)bitmap->rows, end_col = (int32_t)bitmap->width;
            int32_t x_offset = (pen_x + (face->glyph->bitmap_left + glyph->x_offset));
            int32_t y_offset = (pen_y - (face->glyph->bitmap_top + glyph->y_offset));
            if (y_offset < 0) {
                start_row = -y_offset;
            }
            if ((y_offset + end_row) > (int32_t)h) {
                end_row = ((int32_t)h - y_offset);
            }
            if (x_offset < 0) {
                start_col = -x_offset;
            }
            if ((x_offset + end_col) > (int32_t)w) {
                end_col = ((int32_t)w - x_offset);
            }
            switch (bitmap->pixel_mode) {
            case FT_PIXEL_MODE_GRAY: {
                for ( row = start_row; row < end_row; ++row) {
                    uint8_t *src = &bitmap->buffer[row * bitmap->pitch + start_col];
                    sw_color_argb32_t *dst = &ret->pixels[(row + y_offset) * (int32_t)w + (x_offset + start_col)];
                    for ( col = start_col; col < end_col; ++col) {
                        uint8_t m = *src++;
                        uint32_t sa = (text_color.c.a * m + 127) / 255;
                        uint32_t sr = (text_color.c.r * m + 127) / 255;
                        uint32_t sg = (text_color.c.g * m + 127) / 255;
                        uint32_t sb = (text_color.c.b * m + 127) / 255;

                        sw_color_argb32_t *d = dst++;
#if 0
                        uint32_t inv_sa = (255 - sa);
                        d->c.a = (uint8_t)(sa + ((d->c.a * inv_sa + 127) / 255));
                        d->c.r = (uint8_t)(sr + ((d->c.r * inv_sa + 127) / 255));
                        d->c.g = (uint8_t)(sg + ((d->c.g * inv_sa + 127) / 255));
                        d->c.b = (uint8_t)(sb + ((d->c.b * inv_sa + 127) / 255));
#else
                        d->c.a = (uint8_t)sa;
                        d->c.r = (uint8_t)sr;
                        d->c.g = (uint8_t)sg;
                        d->c.b = (uint8_t)sb;
#endif
                    }
                }
                break;
            }
            case FT_PIXEL_MODE_BGRA: {
#if 0
                for ( row = start_row; row < end_row; ++row) {
                    sw_color_argb32_t *src =
                        (sw_color_argb32_t *)(void *)&bitmap->buffer[row * bitmap->pitch + start_col * 4];
                    sw_color_argb32_t *dst = &ret->pixels[(row + y_offset) * (int32_t)w + (x_offset + start_col)];
                    for ( col = start_col; col < end_col; ++col) {
                        sw_color_argb32_t s = *src++;
                        sw_color_argb32_t *d = dst++;
                        uint32_t inv_sa = (255 - s.c.a);
                        d->c.a = (uint8_t)(s.c.a + ((d->c.a * inv_sa + 127) / 255));
                        d->c.r = (uint8_t)(s.c.r + ((d->c.r * inv_sa + 127) / 255));
                        d->c.g = (uint8_t)(s.c.g + ((d->c.g * inv_sa + 127) / 255));
                        d->c.b = (uint8_t)(s.c.b + ((d->c.b * inv_sa + 127) / 255));
                    }
                }
#else
                size_t span_bytes = ((size_t)(end_col - start_col) * 4);
                for ( row = start_row; row < end_row; ++row) {
                    sw_color_argb32_t *src =
                        (sw_color_argb32_t *)(void *)&bitmap->buffer[row * bitmap->pitch + start_col * 4];
                    sw_color_argb32_t *dst = &ret->pixels[(row + y_offset) * (int32_t)w + (x_offset + start_col)];
                
                    memcpy(dst, src, span_bytes);
                }
#endif
                break;
            }
            default:
                ASSERT_UNREACHABLE;
            }
        }

        pen_x += glyph->x_advance;
    }

    return ret;
}

static void surface_destroy_sw(sw_wayland_surface_t *surface, sw_context_t *ctx) {
    NOTUSED(surface); NOTUSED(ctx);
}

static sw_wayland_output_t *output_create_sw(sw_wayland_output_t *output, sw_context_t *ctx) {
    sw_wayland_surface_t *surface = gp_alloc.alloc( &gp_alloc, sizeof(*surface), ALIGNOF(*surface));
    NOTUSED(ctx);

    memset(surface, 0, sizeof(*surface));
    surface->in._.layer.output = output;
    surface->in.destroy = surface_destroy_sw;
    surface->in.height = -1;
    surface->in._.layer.exclusive_zone = INT_MIN;
    surface->in._.layer.anchor = SW_WAYLAND_SURFACE_LAYER_ANCHOR_BOTTOM | SW_WAYLAND_SURFACE_LAYER_ANCHOR_LEFT | SW_WAYLAND_SURFACE_LAYER_ANCHOR_RIGHT;

    surface->in.root = gp_alloc.alloc( &gp_alloc, sizeof(*surface->in.root), ALIGNOF(*surface->in.root));
    memset(surface->in.root, 0, sizeof(*surface->in.root));
    surface->in.root->in.type = SW_LAYOUT_BLOCK_TYPE_IMAGE;
    surface->in.root->in._.image.type = SW_LAYOUT_BLOCK_IMAGE_IMAGE_TYPE_SW_PIXMAP;
    surface->in.root->in._.image.data = p;

    su_llist__sw_wayland_surface_t__insert_tail(&sw.in.backend.wayland.layers, surface);

    NOTUSED(sw);
	return output;
}

static void handle_signal(int sig) {
	NOTUSED(sig);
    running = FALSE;
}

int main(void) {
    sw_pixmap_t *pm;
    static struct sigaction sigact;
    bool32_t c;

    setlocale(LC_ALL, "");
    ASSERT(locale_is_utf8());

    arena_init(&scratch_arena, &gp_alloc, 16384);

    sw.in.backend_type = SW_BACKEND_TYPE_WAYLAND;
    sw.in.gp_alloc = &gp_alloc;
    sw.in.scratch_alloc = &scratch_alloc;
    sw.in.backend.wayland.output_create = output_create_sw;
    sw__context = &sw;

    pm = render();
    p.ptr = pm;
    p.len = (8 + (pm->width * pm->height * 4));

    c = sw_init(&sw);
    ASSERT(c == TRUE);

	sigact.sa_handler = handle_signal;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGPIPE, &sigact, NULL);

    while (running) {
        c = sw_flush(&sw);
        ASSERT(c == TRUE);

        arena_reset(&scratch_arena, &gp_alloc);

        poll(&sw.out.backend.wayland.pfd, 1, -1);

        c = sw_process(&sw);
        ASSERT(c == TRUE);
    }

    return 0;
}
