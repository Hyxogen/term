#ifndef STERM_FTERM_H
#define STERM_FTERM_H

#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include <sterm/types.h>

extern unsigned char psf2_default_font[];
extern unsigned int psf2_default_font_len;

#define DEC_BRACKETED_PASTE_BIT 44

struct psf2_hdr {
	u32 magic;
	u32 version;
	u32 hdrsize;
	u32 flags;
	u32 count;
	u32 charsize;
	u32 height;
	u32 width;
};

struct psf2_font {
	struct psf2_hdr hdr;
	u8 data[];
};

struct font {
	const struct psf2_font *psf;
	u8 ascii[256];
	size_t size;
};

int font_read_from(struct font *dest, const void *src, size_t size);
const u8 *font_get_glyph(struct font *font, u32 codepoint);

struct framebuf {
	size_t width;
	size_t height;
	size_t bpp;
	size_t pitch;

	u8 red_mask_size;
	u8 red_mask_shift;
	u8 blue_mask_size;
	u8 blue_mask_shift;
	u8 green_mask_size;
	u8 green_mask_shift;

	struct font font;

	unsigned char *pixels;
};

struct enc_ctx {
	size_t off;
	char buf[8];
};

struct sterm;

enum termcolor {
	TERMCOLOR_BLACK,
	TERMCOLOR_RED,
	TERMCOLOR_GREEN,
	TERMCOLOR_YELLOW,
	TERMCOLOR_BLUE,
	TERMCOLOR_MAGENTA,
	TERMCOLOR_CYAN,
	TERMCOLOR_WHITE,
	TERMCOLOR_BRIGHT_BLACK,
	TERMCOLOR_BRIGHT_RED,
	TERMCOLOR_BRIGHT_GREEN,
	TERMCOLOR_BRIGHT_YELLOW,
	TERMCOLOR_BRIGHT_BLUE,
	TERMCOLOR_BRIGHT_MAGENTA,
	TERMCOLOR_BRIGHT_CYAN,
	TERMCOLOR_BRIGHT_WHITE,
};

struct term_ops {
	/* minimum required */
	void (*draw_char)(struct sterm *, size_t cx, size_t cy, u32 fg, u32 bg, u32 cp);
	void (*clear_char)(struct sterm *, size_t cx, size_t cy, u32 color);
	void (*get_dimensions)(struct sterm *, unsigned *cols, unsigned *rows);

	/* at least one of: */
        u32 (*encode_color)(struct sterm *, enum termcolor);
        u32 (*encode_rgb)(struct sterm *, u8 r, u8 g, u8 b);	/* needed for 24-bit terminals */

	/* optimized/extension functions */
	void (*scroll)(struct sterm *, u32 bg, int dir);
};

struct sterm {
	unsigned row;
	unsigned col;
	unsigned width;
	unsigned height;

	u32 fg_color;
	u32 bg_color;

	bool in_escape;
	u8 escape_off;
	char escape_seq[64];
	bool dcs;

	bool draw_cursor;

	void *priv;
	const struct term_ops *ops;
	struct enc_ctx encoder;

	int fd;
};


i64 enc_push_byte(struct enc_ctx *ctx, u8 byte);

void sterm_init(struct sterm *term, const struct term_ops *ops, void *ctx);
void sterm_free(struct sterm *term);
void sterm_write(struct sterm *term, const void *buf, size_t n);

extern const struct term_ops fb_ops;

__attribute__((format(printf, 3, 4)))
int printx(int (*put)(int c, void *), void *opaque, const char *fmt, ...);
int vprintx(int (*put)(int, void *), void *opaque, const char *fmt, va_list ap);

#endif
