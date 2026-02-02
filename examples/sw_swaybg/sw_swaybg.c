#define _DEFAULT_SOURCE

#if !defined(DEBUG)
	#define DEBUG 1
#endif

#if !defined(SU_WITH_DEBUG)
	#define SU_WITH_DEBUG DEBUG
#endif /* defined(SU_WITH_DEBUG) */
#define SU_LOG_PREFIX "sw_swaybg: "
#define SU_IMPLEMENTATION
#define SU_STRIP_PREFIXES
#include <sutil.h>

#define SW_WITH_MEMORY_BACKEND 0
#define SW_WITH_WAYLAND_BACKEND 1
#define SW_WITH_WAYLAND_KEYBOARD 0
#define SW_WITH_WAYLAND_CLIPBOARD 0
#define SW_WITH_TEXT 0
#if !defined(SW_WITH_SVG)
	#define SW_WITH_SVG 1
#endif /* !defined(SW_WITH_SVG) */
#if !defined(SW_WITH_PNG)
	#define SW_WITH_PNG 1
#endif /* !defined(SW_WITH_PNG) */
#if !defined(SW_WITH_JPG)
	#define SW_WITH_JPG 1
#endif /* !defined(SW_WITH_JPG) */
#if !defined(SW_WITH_TGA)
	#define SW_WITH_TGA 1
#endif /* !defined(SW_WITH_TGA) */
#if !defined(SW_WITH_BMP)
	#define SW_WITH_BMP 1
#endif /* !defined(SW_WITH_BMP) */
#if !defined(SW_WITH_PSD)
	#define SW_WITH_PSD 1
#endif /* !defined(SW_WITH_PSD) */
#if !defined(SW_WITH_GIF)
	#define SW_WITH_GIF 1
#endif /* !defined(SW_WITH_GIF) */
#if !defined(SW_WITH_HDR)
	#define SW_WITH_HDR 1
#endif /* !defined(SW_WITH_HDR) */
#if !defined(SW_WITH_PIC)
	#define SW_WITH_PIC 1
#endif /* !defined(SW_WITH_PIC) */
#if !defined(SW_WITH_PNM)
	#define SW_WITH_PNM 1
#endif /* !defined(SW_WITH_PNM) */

#if !defined(SW_WITH_DEBUG)
	#define SW_WITH_DEBUG DEBUG
#endif /* defined(SW_WITH_DEBUG) */
#define SW_IMPLEMENTATION
#include <swidgets.h>

#include <locale.h>
#include <signal.h>

typedef enum background_mode {
	BACKGROUND_MODE_STRETCH,
	BACKGROUND_MODE_FILL,
	BACKGROUND_MODE_FIT,
	BACKGROUND_MODE_CENTER,
	BACKGROUND_MODE_TILE,
	BACKGROUND_MODE_SOLID_COLOR,
	BACKGROUND_MODE_INVALID
} background_mode_t;

typedef struct config {
    string_t output;
    background_mode_t mode;
    sw_color_argb32_t color;
    string_t image_path;
	fat_ptr_t loaded_image;
} config_t;

typedef struct layout_block {
	sw_layout_block_t _; /* must be first */
	sw_wayland_surface_t *surface;
} layout_block_t;

typedef struct state {
    sw_context_t sw;
    config_t *configs;
	size_t configs_count;
    arena_t scratch_arena;
    bool32_t running;
	PAD32;
} state_t;


static state_t state;

static const allocator_t gp_alloc = { libc_alloc, libc_free };

static void *scratch_alloc_alloc(const allocator_t *alloc, size_t size, size_t alignment) {
	void *ret = arena_alloc(&state.scratch_arena, &gp_alloc, size, alignment);
	NOTUSED(alloc);
	return ret;
}

static void scratch_alloc_free(const allocator_t *alloc, void *ptr) {
	NOTUSED(alloc); NOTUSED(ptr);
}

static const allocator_t scratch_alloc = { scratch_alloc_alloc, scratch_alloc_free };

static bool32_t parse_sway_color(char *cstr, sw_color_argb32_t *dest) {
	char *p;
    size_t len;
	uint32_t rgba;
	uint32_t a, r, g ,b;

    if (!cstr || ((len = STRLEN(cstr)) == 0)) {
        return FALSE;
    }

	if (cstr[0] == '#') {
		cstr++;
		len--;
	}

	if ((len != 8) && (len != 6)) {
		return FALSE;
	}

	rgba = (uint32_t)strtoul(cstr, &p, 16);
	if (*p != '\0') {
		return FALSE;
	}

	if (len == 8) {
		a = ((rgba >> 0) & 0xFF);
		r = ((rgba >> 24) & 0xFF);
		g = ((rgba >> 16) & 0xFF);
		b = ((rgba >> 8) & 0xFF);
	} else {
		a = 0xFF;
		r = ((rgba >> 16) & 0xFF);
		g = ((rgba >> 8) & 0xFF);
		b = ((rgba >> 0) & 0xFF);
	}

	dest->u32 = ((a << 24) | (r << 16) | (g << 8) | (b << 0));

	return TRUE;
}

static background_mode_t parse_background_mode(char *cstr) {
    if (!cstr) {
        return BACKGROUND_MODE_INVALID;
    }

	if (STRCMP(cstr, "stretch") == 0) {
		return BACKGROUND_MODE_STRETCH;
	} else if (STRCMP(cstr, "fill") == 0) {
		return BACKGROUND_MODE_FILL;
	} else if (STRCMP(cstr, "fit") == 0) {
		return BACKGROUND_MODE_FIT;
	} else if (STRCMP(cstr, "center") == 0) {
		return BACKGROUND_MODE_CENTER;
	} else if (STRCMP(cstr, "tile") == 0) {
		return BACKGROUND_MODE_TILE;
	} else if (STRCMP(cstr, "solid_color") == 0) {
		return BACKGROUND_MODE_SOLID_COLOR;
	} else {
        return BACKGROUND_MODE_INVALID;
    }
}

static void store_config(config_t *config) {
    size_t i = 0;
    for ( ; i < state.configs_count; ++i) {
        config_t *c = &state.configs[i];
        if (string_equal(config->output, c->output)) {
            if (config->image_path.len > 0) {
                c->image_path = config->image_path;
            }
            if (config->color.u32) {
                c->color = config->color;
            }
            if (config->mode != BACKGROUND_MODE_INVALID) {
                c->mode = config->mode;
            }
            return;
        }
    }
	MEMCPY(&state.configs[state.configs_count++], config, sizeof(*config));
}

static su_bool32_t layout_block_handle_prepare(sw_layout_block_t *block_, sw_context_t *ctx) {
	layout_block_t *block = (layout_block_t *)block_;
	NOTUSED(ctx);
	block->_.in.content_width = block->surface->out.width;
	block->_.in.content_height = block->surface->out.height;
	return TRUE;
}

static void configure_output(sw_wayland_output_t *output, config_t *config) {
	static sw_wayland_region_t empty_input_region = { 0, 0, 0, 0 };
	sw_wayland_surface_t *surface;
	layout_block_t *root;
	sw_wayland_surface_layer_t *layer;

	ALLOCCT(surface, &gp_alloc);
	surface->in.type = SW_WAYLAND_SURFACE_TYPE_LAYER;
	surface->in.input_regions = &empty_input_region;
	surface->in.input_regions_count = 1;

	layer = &surface->in._.layer;
	layer->output = output;
	layer->exclusive_zone = -1;
	layer->anchor = SW_WAYLAND_SURFACE_LAYER_ANCHOR_ALL;
	layer->layer = SW_WAYLAND_SURFACE_LAYER_LAYER_BACKGROUND;

	ALLOCCT(root, &gp_alloc);

	surface->in.root = &root->_;

	switch (config->mode) {
	case BACKGROUND_MODE_FILL: /* TODO */
	case BACKGROUND_MODE_FIT: /* TODO */
	case BACKGROUND_MODE_STRETCH:
		root->_.in.type = SW_LAYOUT_BLOCK_TYPE_IMAGE;
		root->_.in._.image.data = config->loaded_image;
		root->surface = surface;
		root->_.in.prepare = layout_block_handle_prepare;
		break;
	case BACKGROUND_MODE_CENTER: /* ? TODO: match original swaybg behaviour with images > output w/h */
		root->_.in.type = SW_LAYOUT_BLOCK_TYPE_IMAGE;
		root->_.in._.image.data = config->loaded_image;
		root->_.in.fill = SW_LAYOUT_BLOCK_FILL_ALL_SIDES;
		root->_.in.content_anchor = SW_LAYOUT_BLOCK_CONTENT_ANCHOR_CENTER_CENTER;
		break;
	case BACKGROUND_MODE_TILE:
		root->_.in.type = SW_LAYOUT_BLOCK_TYPE_IMAGE;
		root->_.in.content_repeat = SW_LAYOUT_BLOCK_CONTENT_REPEAT_NORMAL;
		root->_.in._.image.data = config->loaded_image;
		root->surface = surface;
		root->_.in.prepare = layout_block_handle_prepare;
		break;
	case BACKGROUND_MODE_SOLID_COLOR:
		root->_.in.type = SW_LAYOUT_BLOCK_TYPE_SPACER;
		root->_.in.fill = SW_LAYOUT_BLOCK_FILL_ALL_SIDES;
		root->_.in.color._.argb32.u32 = config->color.u32;
		break;
	case BACKGROUND_MODE_INVALID:
	default:
		ASSERT_UNREACHABLE;
	}

	LLIST_APPEND_TAIL(&state.sw.in.backend.wayland.layers, surface);

	state.sw.in.update_and_render = TRUE;
}

static void process_sw_events(void) {
	size_t i;
	for ( i = 0; i < state.sw.out.events_count; ++i) {
		sw_event_t *event = &state.sw.out.events[i];
		switch (event->out.type) {
		case SW_EVENT_WAYLAND_OUTPUT_CREATE: {
			sw_wayland_output_t *output = event->out._.wayland_output;
			size_t j = 0;
			for ( ; j < state.configs_count; ++j) {
				config_t *config = &state.configs[j];
				if (string_equal(config->output, string("*")) ||
						string_equal(config->output, output->out.name)) {
					configure_output(output, config);
					event->in.wayland_output = output;
					break;
				}
			}
			break;
		}
		case SW_EVENT_WAYLAND_SURFACE_ERROR_MISSING_PROTOCOL:
		case SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_CREATE_BUFFER:
			/* TODO: string */
			su_abort(errno, "failed to create layer surface on output " STRING_FMT ": (%u)",
				STRING_ARG(event->out._.wayland_surface->in._.layer.output->out.name), event->out.type);
		case SW_EVENT_WAYLAND_SURFACE_DESTROY:
			LLIST_POP(&state.sw.in.backend.wayland.layers, event->out._.wayland_surface);
			FREE(&gp_alloc, event->out._.wayland_surface);
			break;
		case SW_EVENT_LAYOUT_BLOCK_DESTROY:
			FREE(&gp_alloc, event->out._.layout_block);
			break;
		case SW_EVENT_LAYOUT_BLOCK_ERROR_INVALID_IMAGE: {
			size_t j = 0;
			for ( ; j < state.configs_count; ++j) {
				config_t *config = &state.configs[j];
				if (config->loaded_image.ptr == event->out._.layout_block->in._.image.data.ptr) {
					su_abort(errno, "failed to load image: " STRING_FMT, STRING_ARG(config->image_path));
				}
			}
			break;
		}
		case SW_EVENT_WAYLAND_SEAT_CREATE:
		case SW_EVENT_WAYLAND_POINTER_CREATE:
			event->in.data = NULL;
			break;
		case SW_EVENT_WAYLAND_SURFACE_FAILED_TO_SET_CURSOR_SHAPE:
		case SW_EVENT_WAYLAND_OUTPUT_DESTROY:
		case SW_EVENT_WAYLAND_POINTER_ENTER:
		case SW_EVENT_WAYLAND_POINTER_LEAVE:
		case SW_EVENT_WAYLAND_POINTER_MOTION:
		case SW_EVENT_WAYLAND_POINTER_BUTTON:
		case SW_EVENT_WAYLAND_POINTER_SCROLL:
		case SW_EVENT_WAYLAND_SEAT_DESTROY:
		case SW_EVENT_WAYLAND_POINTER_DESTROY:
		case SW_EVENT_WAYLAND_SURFACE_ERROR_FAILED_TO_INITIALIZE_ROOT_LAYOUT_BLOCK:
		case SW_EVENT_WAYLAND_SURFACE_ERROR_LAYOUT_FAILED:
			break;
		case SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_FAILED_TO_SET_DECORATIONS:
		case SW_EVENT_WAYLAND_SURFACE_TOPLEVEL_CLOSE:
		default:
			ASSERT_UNREACHABLE;
		}
	}
}

static void handle_signal(int sig) {
	NOTUSED(sig);
	DEBUG_LOG("%d: %s", sig, strsignal(sig));
	state.running = FALSE;
}

static void setup(int argc, char *argv[]) {
    static struct sigaction sigact;
    config_t config;
    size_t i;
	char *s;

	setlocale(LC_ALL, "");
	if (!locale_is_utf8()) {
		su_abort(1, "failed to set UTF-8 locale");
	}

    ARRAY_ALLOC(state.configs, &gp_alloc, argc);

    MEMSET(&config, 0, sizeof(config));
    config.mode = BACKGROUND_MODE_INVALID;
    config.output = string("*");

    ARGPARSE_LOOP_BEGIN {
        switch (ARGPARSE_KEY) {
        case 'c':
			/* TODO: gradient support */
            s = ARGPARSE_VALUE;
            if (!parse_sway_color(s, &config.color)) {
                su_abort( 1,
                    "%s is not a valid color for sw_swaybg. Color should be specified as rrggbb or #rrggbb (no alpha).", s);
            }
            break;
        case 'i':
            if ((s = ARGPARSE_VALUE)) {
                config.image_path = string(s);
            }
            break;
        case 'm':
            s = ARGPARSE_VALUE;
            if ((config.mode = parse_background_mode(s)) == BACKGROUND_MODE_INVALID) {
                 su_abort(1, "invalid mode: %s", s);
            }
            break;
        case 'o':
            if ((s = ARGPARSE_VALUE)) {
                store_config(&config);
                MEMSET(&config, 0, sizeof(config));
                config.mode = BACKGROUND_MODE_INVALID;
                config.output = string(s);
            }
            break;
        case 'v':
            su_abort(0, "sw_swaybg version"); /* TODO */
        case 'h':
        default:
            su_abort( (ARGPARSE_KEY != 'h'),
                "Usage: sw_swaybg <options...>\n"
	    	    "\n"
	    	    "  -c,    Set the background color.\n"
	    	    "  -h,    Show help message and quit.\n"
	    	    "  -i,    Set the image to display.\n"
	    	    "  -m,    Set the mode to use for the image.\n"
	    	    "  -o,    Set the output to operate on or * for all.\n"
	    	    "  -v,    Show the version number and quit.\n"
	    	    "\n"
	    	    "Background Modes:\n"
	    	    "  stretch, fit, fill, center, tile, or solid_color\n"
            );
        }
    } ARGPARSE_LOOP_END

    store_config(&config);

    for ( i = (state.configs_count - 1); i != SIZE_MAX; --i) {
        config_t *c = &state.configs[i];
		if ((c->image_path.len == 0) && !c->color.u32) {
			MEMMOVE(&state.configs[i], &state.configs[i + 1], sizeof(state.configs[0]) * (--state.configs_count - i));
			continue;
		} else if (c->mode == BACKGROUND_MODE_INVALID) {
			c->mode = ((c->image_path.len > 0)
                ? BACKGROUND_MODE_STRETCH
				: BACKGROUND_MODE_SOLID_COLOR);
		}
		if (c->image_path.len > 0) {
			if (!read_entire_file(c->image_path, &c->loaded_image, &gp_alloc)) {
				su_abort(errno, "failed to read file: " STRING_FMT, STRING_ARG(c->image_path));
			}
		}
    }

    arena_init(&state.scratch_arena, &gp_alloc, 8192);

	sigact.sa_handler = handle_signal;
	/* ? TODO: error check */
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGPIPE, &sigact, NULL);

	state.sw.in.backend_type = SW_BACKEND_TYPE_WAYLAND;
	state.sw.in.gp_alloc = &gp_alloc;
	state.sw.in.scratch_alloc = &scratch_alloc;
    state.running = TRUE;
}

static void run(void) {
    while (state.running) {
		do {
			if (!sw_set(&state.sw)) {
				su_abort(errno, "sw_set: %s", strerror(errno));
			}
			state.sw.in.update_and_render = FALSE;
			process_sw_events();
		} while (state.sw.in.update_and_render);

        arena_reset(&state.scratch_arena, &gp_alloc);

        if (poll(state.sw.out.backend.wayland.fds, state.sw.out.backend.wayland.fds_count,
				(int)(state.sw.out.t - now_ms(CLOCK_MONOTONIC))) == -1) {
			if (errno != EINTR) {
				su_abort(errno, "poll: %s", strerror(errno));
			}
    	}
	}
}

static void cleanup(void) {
	state.sw.in.backend_type = SW_BACKEND_TYPE_NONE;
	sw_set(&state.sw);

#if DEBUG
	{
		size_t i;

		process_sw_events();
		FREE(&gp_alloc, state.sw.out.events);

		for ( i = 0; i < state.configs_count; ++i) {
			FREE(&gp_alloc, state.configs[i].loaded_image.ptr);
		}
		FREE(&gp_alloc, state.configs);
		arena_fini(&state.scratch_arena, &gp_alloc);
	}
#endif
}

int main(int argc, char *argv[]) {
    setup(argc, argv);
    run();
    cleanup();

    return 0;
}
