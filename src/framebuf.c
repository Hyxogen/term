#include <sterm/sterm.h>
#include <sterm/types.h>
#include <string.h>

static u32 fb_encode_color(struct sterm *term, u8 red, u8 green, u8 blue)
{
	u32 res = 0;

	const struct framebuf *fb = term->priv;

	res |= (red & ((1 << fb->red_mask_size) - 1)) << fb->red_mask_shift;
	res |= (green & ((1 << fb->green_mask_size) - 1)) << fb->green_mask_shift;
	res |= (blue & ((1 << fb->blue_mask_size) - 1)) << fb->blue_mask_shift;

	return res;
}

static inline void *fb_get_pixel_addr(struct framebuf *fb, size_t x, size_t y)
{
	size_t bytes_per_pixel = fb->bpp >> 3;
	return &fb->pixels[fb->pitch * y + x * bytes_per_pixel];
}

static void fb_put(struct framebuf *fb, size_t x, size_t y, u32 color)
{
	unsigned char *start = fb_get_pixel_addr(fb, x, y);
	memcpy(start, &color, sizeof(color));
}

//https://invisible-island.net/xterm/ctlseqs/ctlseqs.txt
//https://ecma-international.org/wp-content/uploads/ECMA-48_5th_edition_june_1991.pdf
static void fb_clear(struct sterm *term, size_t col, size_t row, u32 color)
{
	u32 ch_width = 8;
	u32 ch_height = 8;

	u32 xoff = col * ch_width;
	u32 yoff = row * ch_height;

	for (u32 y = yoff; y < yoff + ch_height; y++) {
		for (u32 x = xoff; x < xoff + ch_width; x++) {
			fb_put(term->priv, x, y, color);
		}
	}
}

static void fb_draw_char(struct sterm *term, size_t col, size_t row, u32 fg, u32 bg, u32 cp)
{
	struct framebuf *fb = term->priv;
	struct font *font = &fb->font;

	const u8 *glyph = font_get_glyph(font, cp);

	u32 bytes_per_row = (font->psf->hdr.width + 7) / 8;

	u32 xoff = col * font->psf->hdr.width;
	u32 yoff = row * font->psf->hdr.height;

	for (u32 y = 0; y < font->psf->hdr.height; ++y) {
		const u8 *row = &glyph[y * bytes_per_row];

		for (u32 x = 0; x < font->psf->hdr.width; ++x) {
			u8 col = row[x / 8];
			u8 mask = 0x80 >> (x % 8);

			fb_put(fb, x + xoff, y + yoff,
			       (col & mask) ? fg : bg);
		}
	}
}

static void fb_get_dimensions(struct sterm *term, unsigned *cols, unsigned *rows)
{
	struct framebuf *fb = term->priv;

	/* TODO allow scaling of the font */
	*cols = fb->width / fb->font.psf->hdr.charsize;
	*rows = fb->height / fb->font.psf->hdr.charsize;
}

static void fb_scroll(struct sterm *term, u32 bg, int dir)
{
	struct framebuf *fb = term->priv;
	if (dir < 0) {
		dir = -dir;

		char *start = fb_get_pixel_addr(fb, fb->width, dir * fb->font.psf->hdr.charsize);
		char *end = fb_get_pixel_addr(fb, fb->width, fb->height);
		memmove(fb->pixels, start, end - start);

		for (int off = dir; off; off--){
			for (u32 col = 0; col < term->width; col++) {
				fb_clear(term, col, term->height - off, bg);
			}
		}
	} else {
		/* TODO */
	}
}

const struct term_ops fb_ops = {
	.draw_char = fb_draw_char,
	.clear_char = fb_clear,
	.encode_rgb = fb_encode_color,
	.get_dimensions = fb_get_dimensions,
	.scroll = fb_scroll,
};
