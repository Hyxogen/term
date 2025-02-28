#include <sterm/sterm.h>
#include <sterm/types.h>
#include <string.h>

/* TODO remove */
#include <stdio.h>

static u32 fb_encode_color(struct term *term, u8 red, u8 green, u8 blue)
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
static void fb_clear(struct term *term, unsigned col, unsigned row, u32 color)
{
	struct framebuf *fb = term->priv;

	u32 ch_width = fb->font.psf->hdr.width * fb->font_size;
	u32 ch_height = fb->font.psf->hdr.height * fb->font_size;

	u32 xoff = col * ch_width;
	u32 yoff = row * ch_height;

	for (u32 y = yoff; y < yoff + ch_height; y++) {
		for (u32 x = xoff; x < xoff + ch_width; x++) {
			fb_put(term->priv, x, y, color);
		}
	}
}

static void fb_draw_char(struct term *term, unsigned col, unsigned row, u32 fg, u32 bg, u32 cp, unsigned flags)
{
	struct framebuf *fb = term->priv;
	struct font *font = &fb->font;
	if (flags & SGR_FLAG_BOLD)
		font = &fb->bold_font;

	const u8 *glyph = font_get_glyph(font, cp);

	u32 bytes_per_row = (font->psf->hdr.width + 7) / 8;

	u32 font_size = fb->font_size;

	u32 width = font->psf->hdr.width * font_size;
	u32 height = font->psf->hdr.height * font_size;

	u32 xoff = col * width;
	u32 yoff = row * height;

	for (u32 y = 0; y < height; y++) {
		u8 fy = y / font_size;
		const u8 *row = &glyph[fy * bytes_per_row];

		for (u32 x = 0; x < width; x++) {
			u8 fx = x / font_size;
			u8 col = row[fx / 8];
			u8 mask = 0x80 >> (fx % 8);

			bool draw = col & mask;
			if (y + 1 == height && (flags & SGR_FLAG_UNDERLINED))
				draw = true;

			fb_put(fb, x + xoff, y + yoff,
			       draw ? fg : bg);
		}
	}
}

static void fb_get_dimensions(struct term *term, unsigned *cols, unsigned *rows)
{
	struct framebuf *fb = term->priv;

	*cols = fb->width / (fb->font.psf->hdr.width * fb->font_size);
	*rows = fb->height / (fb->font.psf->hdr.height * fb->font_size);
}

const struct termops fb_ops = {
	.draw_char = fb_draw_char,
	.clear_char = fb_clear,
	.encode_rgb = fb_encode_color,
	.get_dimensions = fb_get_dimensions,
};
