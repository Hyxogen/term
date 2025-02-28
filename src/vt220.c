#include <stdbool.h>
#include <sterm/types.h>
#include <sterm/ctrl.h>
#include <stdio.h>
#include <sterm/sterm.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define eprintln(...) fprintf(stderr, __VA_ARGS__)

void collect(struct parser *ctx, uint32_t cp)
{
	struct term *term = ctx->priv;

	if (term->overflowed || term->buf_off >= sizeof(term->buf) - 1) {
		term->overflowed = true;
	} else {
		term->buf[term->buf_off++] = (unsigned char) cp;
		term->buf[term->buf_off] = '\0';
	}
}

static void term_commit_param(struct term *term)
{
	if (term->nparams >= sizeof(term->params)/sizeof(term->params[0]))
		return;

	term->params[term->nparams++] = term->param;
	term->param = 0;
}

void param(struct parser *ctx, uint32_t cp)
{
	struct term *term = ctx->priv;

	if (cp == ';') {
		term_commit_param(term);
	} else {
		assert(cp >= '0' && cp <= '9');

		term->param = term->param * 10 + (cp - '0');
	}
}

void osc_start(struct parser *ctx, uint32_t cp)
{
	(void)ctx;
	(void)cp;
	/* TODO */
}

void osc_put(struct parser *ctx, uint32_t cp)
{
	(void)ctx;
	(void)cp;
	/* TODO */
}

void osc_end(struct parser *ctx, uint32_t cp)
{
	(void)ctx;
	(void)cp;
	/* TODO */
}

void put(struct parser *ctx, uint32_t cp)
{
	(void)ctx;
	(void)cp;
	/* TODO */
}

void esc_dispatch(struct parser *ctx, uint32_t cp)
{
	(void)ctx;
	(void)cp;
	/* TODO */
}

void csi_dispatch(struct parser *ctx, uint32_t cp)
{
	(void)ctx;
	(void)cp;
	/* TODO */
}

void hook(struct parser *ctx, uint32_t cp)
{
	(void)ctx;
	(void)cp;
	/* TODO */
}

void unhook(struct parser *ctx, uint32_t cp)
{
	(void)ctx;
	(void)cp;
	/* TODO */
}

void clear(struct parser *ctx, uint32_t cp)
{
	struct term *term = ctx->priv;
	(void) cp;

	term->overflowed = false;
	term->buf_off = 0;
}

static struct termchar *term_get_at(struct term *term, unsigned cx, unsigned cy)
{
	assert(cy < term->rows && cx < term->cols);
	return &term->chars[cy * term->cols + cx];
}

static void term_redraw(struct term *term, unsigned cx, unsigned cy)
{
	struct termchar *ch = term_get_at(term, cx, cy);
	if (ch->cp == 0)
		term->ops->clear_char(term, cx, cy, ch->bg);
	else
		term->ops->draw_char(term, cx, cy, ch->fg, ch->bg,
				     ch->cp);
}

static void sterm_redraw_row(struct term *term, unsigned cy)
{
	for (unsigned cx = 0; cx < term->cols; cx++) {
		term_redraw(term, cx, cy);
	}
}

static void sterm_redraw(struct term *term)
{
	for (unsigned cy = 0; cy < term->rows; cy++) {
		sterm_redraw_row(term, cy);
	}
}

static void term_scroll(struct term *term, int dir)
{
	assert(dir != 0);

	char *region_start;
	char *paste_start;
	size_t nlines;

	if (dir < 0) {
		/* scroll up */
		region_start = (void*) &term->chars[term->cols * (-dir + term->scroll_top)];
		paste_start = (void*) &term->chars[term->cols * term->scroll_top];

		nlines = term->rows + dir;
	} else {
		/* scroll down */
		region_start = (void*) &term->chars[term->cols * term->scroll_top];
		paste_start = (void*) &term->chars[term->cols * (dir + term->scroll_top)];

		nlines = term->rows - dir;
	}

	if (nlines > term->scroll_bot - term->scroll_top)
		nlines = term->scroll_bot - term->scroll_top;

	size_t nchars = nlines * term->cols;
	char *end = (void*) &term->chars[term->cols * term->scroll_bot];

	char *region_end = region_start + nchars * sizeof(struct termchar);
	char *paste_end = paste_start + nchars * sizeof(struct termchar);

	memmove(paste_start, region_start, region_end - region_start);

	if (dir > 0)
		memset(term->chars, 0, paste_start - (char*) term->chars);
	if (dir < 0) {
		assert(end >= paste_end);
		memset(paste_end, 0, end - paste_end);
	}

	sterm_redraw(term);
}

static void term_put_char(struct term *term, unsigned cx, unsigned cy, u32 cp, u32 fg, u32 bg)
{
	struct termchar *ch = term_get_at(term, cx, cy);

	ch->cp = cp;
	ch->fg = fg;
	ch->bg = bg;
	term_redraw(term, cx, cy);
}

static void term_draw_cursor(struct term *term)
{
	if (!term->draw_cursor)
		return;

	struct termchar *ch = term_get_at(term, term->col, term->row);
	term->ops->draw_char(term, term->col, term->row, term->black, term->white, ch->cp ? ch->cp : ' ');
}

static void term_clear_cursor(struct term *term)
{
	term_redraw(term, term->col, term->row);
}

static void term_linefeed(struct term *term)
{
	if (++term->row == term->rows) {
		term->row -= 1;
		term_scroll(term, -1);
	}
}

void execute(struct parser *ctx, uint32_t cp)
{
	struct term *term = ctx->priv;

	term_clear_cursor(term);
	switch (cp) {
	case 0x07: /* bell, BEL */
		break;
	case 0x08: /* backspace, BS */
		if (term->col-- == 0)
			term->col = 0;
		break;
	case 0x0a: /* linefeed, LF */
		term_linefeed(ctx->priv);
		break;
	case 0x0d: /* carriage return, CR */
		term->col = 0;
		break;
	default:
		fprintf(stderr, "unsupported control char 0x%02x\n", cp);
		break;
	}

	term_draw_cursor(term);
}

static void term_print(struct term *term, u32 cp)
{
	term_clear_cursor(term);

	term_put_char(term, term->col++, term->row, cp, term->fg_color, term->bg_color);

	if (term->col == term->cols) {
		term->col = 0;
		term_linefeed(term);
	}

	term_draw_cursor(term);
}

void print(struct parser *ctx, u32 cp)
{
	term_print(ctx->priv, cp);
}

extern void parser_push(struct parser *ctx, uint32_t cp);
extern void parser_init(struct parser *ctx, void *priv);

static u32 term_get_color(struct term *term, enum termcolor color)
{
	assert(term->ops->encode_color || term->ops->encode_rgb);

	if (term->ops->encode_color)
		return term->ops->encode_color(term, color);

	switch (color) {
	case TERMCOLOR_BLACK:
		return term->ops->encode_rgb(term, 0, 0, 0);
	case TERMCOLOR_RED:
		return term->ops->encode_rgb(term, 170, 0, 0);
	case TERMCOLOR_GREEN:
		return term->ops->encode_rgb(term, 0, 170, 0);
	case TERMCOLOR_YELLOW:
		return term->ops->encode_rgb(term, 170, 85, 0);
	case TERMCOLOR_BLUE:
		return term->ops->encode_rgb(term, 0, 0, 170);
	case TERMCOLOR_MAGENTA:
		return term->ops->encode_rgb(term, 170, 0, 170);
	case TERMCOLOR_CYAN:
		return term->ops->encode_rgb(term, 0, 170, 170);
	case TERMCOLOR_WHITE:
		return term->ops->encode_rgb(term, 170, 170, 170);
	case TERMCOLOR_BRIGHT_BLACK:
		return term->ops->encode_rgb(term, 85, 85, 85);
	case TERMCOLOR_BRIGHT_RED:
		return term->ops->encode_rgb(term, 255, 85, 85);
	case TERMCOLOR_BRIGHT_GREEN:
		return term->ops->encode_rgb(term, 85, 255, 85);
	case TERMCOLOR_BRIGHT_YELLOW:
		return term->ops->encode_rgb(term, 255, 255, 85);
	case TERMCOLOR_BRIGHT_BLUE:
		return term->ops->encode_rgb(term, 85, 85, 255);
	default:
	case TERMCOLOR_BRIGHT_MAGENTA:
		return term->ops->encode_rgb(term, 255, 85, 255);
	case TERMCOLOR_BRIGHT_CYAN:
		return term->ops->encode_rgb(term, 85, 255, 255);
	case TERMCOLOR_BRIGHT_WHITE:
		return term->ops->encode_rgb(term, 255, 255, 255);
	}
}

static void term_put(struct term *term, unsigned char ch)
{
	u64 res = term->encoder.ops->put(&term->encoder, ch);
	if (res < 0) {
		return;
	}

	parser_push(&term->parser, (u32) res);
}

void term_write(struct term *term, const void *src, size_t n)
{
	const unsigned char *c = src;
	for (size_t i = 0; i < n; i++) {
		term_put(term,c[i]);
	}
}

static i64 ascii_encode(struct encoder *encoder, char ch)
{
	(void)encoder;
	return ch;
}

static const struct encoder_ops ascii_ops = {
	.put = ascii_encode,
};

int term_init(struct term *term, const struct termops *ops, void *ctx)
{
	term->row = 0;
	term->col = 0;

	term->priv = ctx;

	term->buf_off = 0;
	term->overflowed = false;

	term->param = 0;
	term->nparams = 0;

	parser_init(&term->parser, term);

	term->ops = ops;

	term->ops->get_dimensions(term, &term->cols, &term->rows);

	term->white = term_get_color(term, TERMCOLOR_WHITE);
	term->black = term_get_color(term, TERMCOLOR_BLACK);
	term->inverse = false;
	term->draw_cursor = true;

	term->fg_color = term->white;
	term->bg_color = term->black;

	/* TODO properly set encoder */
	term->encoder.ops = &ascii_ops;

	term->chars = calloc(term->cols * term->rows, sizeof(*term->chars));
	if (!term->chars)
		return -1;
	return 0;
}

void term_free(struct term *term)
{
	if (term->ops->release)
		term->ops->release(term->priv);
	free(term->chars);
}
