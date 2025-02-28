#ifndef STERM_FTERM_H
#define STERM_FTERM_H

#include <stdbool.h>
#include <sterm/parser.h>
#include <sterm/types.h>
#include <sterm/psf.h>

#define TERM_DEFAULT_TABSTOP 8

#define SGR_FLAG_BOLD 0x01
#define SGR_FLAG_UNDERLINED 0x02

struct term;
struct termops;
struct encoder;

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

	const struct font *font;
	const struct font *bold_font;

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

struct encoder {
	char buf[16];
	unsigned char offset;
	i64 (*put)(struct encoder *, char ch);
};

struct termchar {
	u32 cp;
	u32 fg;
	u32 bg;
	unsigned flags;
};

struct termops {
	/* mandatory */
	void (*draw_char)(struct term *, unsigned cx, unsigned cy, u32 cp, u32 fg, u32 bg, unsigned flags);
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
	unsigned sgr_flags;

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

	int (*put)(int, void*);
	void *put_ctx;

	void *priv;
};

extern const struct termops fb_ops;

int term_init(struct term *term, const struct termops *ops, void *ctx, int (*put)(int, void*), void *put_ctx);
void term_free(struct term *term);
void term_write(struct term *term, const void *buf, size_t n);

#endif
