#include <stdbool.h>
#include <sterm/types.h>
#include <sterm/ctrl.h>
#include <sterm/sterm.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifndef TERM_BARE_METAL
#define TERM_BARE_METAL 0
#endif

#if TERM_BARE_METAL
#define eprintf(...)
#else
#include <stdio.h>
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#endif

#define DECTCEM 25 /* text cursor enable */
#define TERM_CSI "\033"

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))

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
	eprintf("tried to osc_start\n");
}

void osc_put(struct parser *ctx, uint32_t cp)
{
	(void)ctx;
	(void)cp;
	/* TODO */
	eprintf("tried to osc_put\n");
}

void osc_end(struct parser *ctx, uint32_t cp)
{
	(void)ctx;
	(void)cp;
	/* TODO */
	eprintf("tried to osc_end\n");
}

void put(struct parser *ctx, uint32_t cp)
{
	(void)ctx;
	(void)cp;
	/* TODO */
}

void hook(struct parser *ctx, uint32_t cp)
{
	struct term *term = ctx->priv;

	switch (cp) {
	default:
		eprintf("unsupported DCS '%c'\n", cp);
		break;
	}
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
	term->buf[0] = '\0';
	term->param = 0;
	term->nparams = 0;
}

static void term_reset_tabstops(struct term *term, unsigned tabstop)
{
	for (unsigned i = 0; i < term->cols; i += tabstop) {
		term->tabstops[i] = true;
	}
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
				     ch->cp, ch->flags);
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

	assert(abs(dir) < term->scroll_bot - term->scroll_top);

	if (dir < 0) {
		/* scroll up */
		region_start = (void*) &term->chars[term->cols * (-dir + term->scroll_top)];
		paste_start = (void*) &term->chars[term->cols * term->scroll_top];

		nlines = term->scroll_bot + dir - term->scroll_top;
	} else {
		/* scroll down */
		region_start = (void*) &term->chars[term->cols * term->scroll_top];
		paste_start = (void*) &term->chars[term->cols * (dir + term->scroll_top)];

		nlines = term->scroll_bot - dir - term->scroll_top;
	}

	if (nlines > term->scroll_bot - term->scroll_top)
		nlines = term->scroll_bot - term->scroll_top;

	size_t nchars = nlines * term->cols;
	char *end = (void*) &term->chars[term->cols * term->scroll_bot];

	char *region_end = region_start + nchars * sizeof(struct termchar);
	char *paste_end = paste_start + nchars * sizeof(struct termchar);

	memmove(paste_start, region_start, region_end - region_start);

	if (dir > 0) {
		memset(region_start, 0, paste_start - region_start);
	} else {
		assert(end >= paste_end);
		memset(paste_end, 0, end - paste_end);
	}

	sterm_redraw(term);
}

static void term_put_char(struct term *term, unsigned cx, unsigned cy, u32 cp, u32 fg, u32 bg, unsigned flags)
{
	struct termchar *ch = term_get_at(term, cx, cy);
	ch->cp = cp;

	if (term->inverse) {
		ch->fg = bg;
		ch->bg = fg;
	} else {
		ch->fg = fg;
		ch->bg = bg;
	}
	ch->flags = flags;
	term_redraw(term, cx, cy);
}

static void term_draw_cursor(struct term *term)
{
	if (!term->draw_cursor)
		return;

	struct termchar *ch = term_get_at(term, term->col, term->row);
	term->ops->draw_char(term, term->col, term->row, term->black, term->white, ch->cp ? ch->cp : ' ', ch->flags);
}

static void term_clear_cursor(struct term *term)
{
	term_redraw(term, term->col, term->row);
}

static void term_linefeed(struct term *term)
{
	if (++term->row >= term->scroll_bot) {
		term->row -= 1;
		term_scroll(term, -1);
	}
}

static long term_get_next_tabstop(const struct term *term, unsigned cx)
{
	assert(cx < term->cols);

	cx += 1;
	for (; cx < term->cols; cx++) {
		if (term->tabstops[cx])
			return cx;
	}
	return -1;
}

void execute(struct parser *ctx, uint32_t cp)
{
	struct term *term = ctx->priv;

	term_clear_cursor(term);
	switch (cp) {
	case 0x00: /* null, NUL */
		break;
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
	case '\t': { /* horizontal tab, TB */
		long i = term_get_next_tabstop(term, term->col);
		if (i < 0)
			i = term->cols - 1;
		term->col = (unsigned) i;
		break;
	}
	default:
		eprintf("unsupported control char 0x%02x\n", cp);
		break;
	}

	term_draw_cursor(term);
}

static unsigned short term_get_param(const struct term *term, unsigned short idx)
{
	assert(idx < (sizeof(term->params)/sizeof(term->params[0])));

	if (idx >= term->nparams)
		return 0;
	return term->params[idx];
}

static unsigned short term_get_param_def(const struct term *term, unsigned short idx, unsigned short def)
{
	unsigned i = term_get_param(term, idx);
	if (!i)
		return def;
	return i;
}

static void dec_mode_exec(struct term *term, uint32_t cp)
{
	bool set;
	if (cp == 'h') {
		set = true;
	} else if (cp == 'l') {
		set = false;
	} else {
		eprintf("unknown dec private mode suffix %c (0x%02x)\n", cp, cp);
		return;
	}

	unsigned short param = term_get_param(term, 0);
	switch (param) {
	case DECTCEM:
		term->draw_cursor = set;

		if (set)
			term_draw_cursor(term);
		else
			term_clear_cursor(term);
		break;
	case 6:
		assert(0);
		break;
	default:
		eprintf("unsupported dec mode %u\n", param);
		break;
	}
}

static void term_exec_sgr(struct term *term)
{
	if (term->nparams == 0) {
		/* HACK: an empty sgr just implies that the first param is 0 */
		term->param = 0;
		term_commit_param(term);
	}

	for (unsigned i = 0; i < term->nparams; i++) {
		unsigned short param = term_get_param(term, i);
		switch (param) {
		case 0:
			term->fg_color = term->white;
			term->bg_color = term->black;
			term->inverse = false;
			term->sgr_flags = 0;
			break;
		case 1:
			term->sgr_flags |= SGR_FLAG_BOLD;
			break;
		case 4:
		case 24:
			term->sgr_flags &= ~SGR_FLAG_UNDERLINED;
			term->sgr_flags |= SGR_FLAG_UNDERLINED * (param == 4);
			break;
		case 7:
		case 27:
			term->inverse = param == 7;
			break;
		default:
			eprintf("unsupported SGR %u\n", param);
			return;
		}
	}
}

static void term_erase_char(struct term *term, unsigned cx, unsigned cy)
{
	term_put_char(term, cx, cy, 0, term->fg_color, term->bg_color, 0);
}

static void term_erase_range(struct term *term, unsigned ax, unsigned ay, unsigned bx, unsigned by)
{
	if (ay > by) {
		term_erase_range(term, bx, by, ax, ay);
	} if (ay == by && ax > bx) {
		term_erase_range(term, bx, by, ax, ay);
	} else {
		unsigned col = ax;
		unsigned row = ay;
		unsigned beg = ay * term->cols + ax;
		unsigned end = by * term->cols + bx;
		assert(end <= term->rows * term->cols);

		for (; beg < end; beg++, col++) {
			if (col && (col % term->cols) == 0) {
				col = 0;
				row += 1;
			}

			term_erase_char(term, col, row);
		}
	}
}

static bool term_in_scroll_region(const struct term *term)
{
	return term->row >= term->scroll_top && term->row < term->scroll_bot;
}

static void term_dump_csi(const struct term *term, u32 cp)
{
	eprintf("CSI %c (0x%02x)\n", cp, cp);
	eprintf("buf:    \"%s\"\n", term->buf);
	eprintf("params: ");
	if (term->nparams == 0) {
		eprintf("no params\n");
	} else {
		for (unsigned i = 0; i < term->nparams; i++)
			eprintf("%u ", term->params[i]);
		eprintf("\n");
	}
}

static int term_put_application(struct term *term, int ch)
{
	if (term->put)
		return term->put(ch, term->put_ctx);
	return 0;
}

static void term_put_unsigned(struct term *term, unsigned i)
{
	if (i / 10)
		term_put_application(term, i / 10);
	term_put_application(term, '0' + (i % 10));
}

static void term_put_str(struct term *term, const char *str)
{
	while (*str) {
		term_put_application(term, *str++);
	}
}

void csi_dispatch(struct parser *ctx, uint32_t cp)
{
	struct term *term = ctx->priv;

	if (term->param != 0)
		term_commit_param(term);

	if (term->buf[0] == '?') {
		dec_mode_exec(term, cp);
		return;
	}

	switch (cp) {
	case 'm':
		term_exec_sgr(term);
		break;
	case 'M': /* delete line, (DL) */
		if (!term_in_scroll_region(term))
			break;

		long i = term_get_param_def(term, 0, 1);
		unsigned rem = term->scroll_bot - term->row;

		if (i > rem)
			i = rem;

		unsigned saved_top = term->scroll_top;
		term->scroll_top = term->row;

		term_erase_range(term, 0, term->row, 0, term->row + i);
		term_scroll(term, -i);

		term->scroll_top = saved_top;

		break;
	case 'L': /* insert line, (IL) */
		if (!term_in_scroll_region(term))
			break;

		i = term_get_param_def(term, 0, 1);
		rem = term->scroll_bot - term->row;

		if (i > rem)
			i = rem;

		term_scroll(term, i);
	case 'K': /* erase from cursor to end of line */
		term_erase_range(term, term->col, term->row, term->cols, term->row);
		term_draw_cursor(term);
		break;
	case 'J': /* erase from cursor to end of screen */
		term_erase_range(term, term->col, term->row, 0, term->rows);
		term_draw_cursor(term);
		break;
	case 'r': { /* set top and bottom margins (DECSTBM) */
		unsigned top = term_get_param(term, 0);
		unsigned bot = term_get_param(term, 1);

		top = MIN(top, term->rows);
		bot = MIN(bot, term->rows);

		if (bot < top)
			break;

		if (top)
			top--;

		term->scroll_top = top;
		term->scroll_bot = bot;
		break;
	}
	case 'A': /* cursor up, CUP */
		term_clear_cursor(term);

		i = term_get_param_def(term, 0, 1);

		term->row -= MIN(i, term->row);

		term_draw_cursor(term);
		break;
	case 'B': /* cursor down, CUD */
		term_clear_cursor(term);

		i = term_get_param_def(term, 0, 1);

		term->row = MIN(term->rows - 1, term->row + i);

		term_draw_cursor(term);
		break;
	case 'C':
		term_clear_cursor(term);

		i = term_get_param(term, 0);
		if (!i)
			i = 1;

		term->col = MIN(term->cols - 1, term->col + i);

		term_draw_cursor(term);
		break;
	case 'H': {
		unsigned row = term_get_param(term, 0);
		unsigned col = term_get_param(term, 1);

		if (row >= term->rows)
			row = term->rows;
		if (col >= term->cols)
			col = term->cols;

		if (row)
			row--;
		if (col)
			col--;

		term_clear_cursor(term);
		term->row = row;
		term->col = col;
		term_draw_cursor(term);

		break;
	}
	case 'n': /* device status report (DSR) */
		switch (term_get_param(term, 0)) {
		case 6: /* report cursor position, (CPR) */
			term_put_str(term, TERM_CSI);
			term_put_unsigned(term, term->row + 1);
			term_put_str(term, ";");
			term_put_unsigned(term, term->col + 1);
			term_put_str(term, "R");
			break;
		default:
			eprintf("unknown DSR parameter\n");
			break;
		}
		break;
	case 'g': /* tabulation clear, TBC */
		switch (term_get_param(term, 0)) {
		case 0:
			term->tabstops[term->col] = false;
			break;
		case 3:
			term_reset_tabstops(term, TERM_DEFAULT_TABSTOP);
			break;
		default:
			eprintf("unsupported TBC parameter\n");
			break;
		}
		break;
	default:
		eprintf("unsupported csi sequence: %c (0x%02x)\n", cp, cp);
		term_dump_csi(term, cp);
	}
}

void esc_dispatch(struct parser *ctx, uint32_t cp)
{
	struct term *term = ctx->priv;

	term_clear_cursor(term);

	switch (cp) {
	case 'M': /* reverse index, RI */
		if (term->row-- == 0) {
			term->row = 0;
			term_scroll(term, 1);
		}
		break;
	case 'H': /* horizontal tab set, HTS */
		term->tabstops[term->col] = true;
		break;
	default:
		eprintf("unsupported escape %c (0x%02x)\n", cp, cp);
		break;
	}

	term_draw_cursor(term);
}

static void term_print(struct term *term, u32 cp)
{
	term_clear_cursor(term);

	term_put_char(term, term->col++, term->row, cp, term->fg_color, term->bg_color, term->sgr_flags);

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
	u64 res = term->encoder.put(&term->encoder, ch);
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

int term_init(struct term *term, const struct termops *ops, void *ctx, int (*put)(int, void*), void *put_ctx)
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

	term->scroll_top = 0;
	term->scroll_bot = term->rows;

	term->white = term_get_color(term, TERMCOLOR_WHITE);
	term->black = term_get_color(term, TERMCOLOR_BLACK);
	term->inverse = false;
	term->draw_cursor = true;
	term->sgr_flags = 0;

	term->fg_color = term->white;
	term->bg_color = term->black;

	/* TODO properly set encoder */
	term->encoder.put = &ascii_encode;

	term->chars = calloc(term->cols * term->rows, sizeof(*term->chars));
	term->tabstops = calloc(term->cols, sizeof(*term->tabstops));
	if (!term->chars || !term->tabstops) {
		free(term->chars);
		free(term->tabstops);
		return -1;
	}

	term_reset_tabstops(term, TERM_DEFAULT_TABSTOP);

	term->put = put;
	term->put_ctx = put_ctx;

	return 0;
}

void term_free(struct term *term)
{
	if (term->ops->release)
		term->ops->release(term->priv);
	free(term->chars);
}
