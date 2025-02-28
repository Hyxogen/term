#ifndef STERM_FTERM_H
#define STERM_FTERM_H

#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <sterm/parser.h>
#include <sterm/types.h>

#define TERM_DEFAULT_TABSTOP 8

struct term;
struct termops;
struct encoder;

extern unsigned char psf2_default_font[];
extern unsigned int psf2_default_font_len;

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

	u8 font_size;

	struct font font;

	unsigned char *pixels;
};

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

struct encoder_ops {
	i64 (*put)(struct encoder *, char ch);
};

struct encoder {
	char buf[16];
	unsigned char offset;
	const struct encoder_ops *ops;
};

struct termchar {
	u32 cp;
	u32 fg;
	u32 bg;
	/* TODO erasable? */
};

struct termops {
	/* mandatory */
	void (*draw_char)(struct term *, unsigned cx, unsigned cy, u32 cp, u32 fg, u32 bg);
	void (*clear_char)(struct term *, unsigned cx, unsigned cy, u32 col);
	void (*get_dimensions)(struct term *, unsigned *cols, unsigned *rows);

	/* at least one of: */
        u32 (*encode_color)(struct term *, enum termcolor);
        u32 (*encode_rgb)(struct term *, u8 r, u8 g, u8 b);	/* needed for 24-bit terminals */

	/* optional */
	void (*release)(void *);
};

struct term {
	unsigned row;
	unsigned col;

	unsigned rows;
	unsigned cols;

	struct encoder encoder;

	u32 fg_color;
	u32 bg_color;
	u32 black;
	u32 white;
	bool inverse;
	bool draw_cursor;

	unsigned scroll_top;
	unsigned scroll_bot;

	unsigned char buf[64];
	unsigned char buf_off;
	bool overflowed;

	unsigned short params[16];
	unsigned short param;
	unsigned char nparams;

	struct parser parser;

	struct termchar *chars;
	bool *tabstops;
	const struct termops *ops;

	int fd;

	void *priv;
};


extern const struct termops fb_ops;

int term_init(struct term *term, const struct termops *ops, void *ctx);
void term_free(struct term *term);
void term_write(struct term *term, const void *buf, size_t n);

__attribute__((format(printf, 3, 4)))
int printx(int (*put)(int c, void *), void *opaque, const char *fmt, ...);
int vprintx(int (*put)(int, void *), void *opaque, const char *fmt, va_list ap);

#endif
