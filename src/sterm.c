#include <sterm/sterm.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
/* TODO remove */
#include <stdio.h>
#include <unistd.h>

#define ANSI_ESCAPE 0x1b
#define LINEFEED 0x0a
#define CARRIAGE_RETURN 0x0d
#define BACKSPACE 0x08
#define TERM_CSI "\033"

/* https://vt100.net */
/* https://wiki.archlinux.org/title/KMSCON */

static bool sterm_is_terminator(int ch)
{
	return isalpha(ch) || ch == '~' || ch == '@';
}

static struct termchar *sterm_get_at(const struct sterm *term, unsigned col, unsigned row)
{
	return (struct termchar*) &term->chars[row * term->width + col];
}

static void sterm_clear_cursor(struct sterm *term)
{
	struct termchar *ch = sterm_get_at(term, term->col, term->row);
	term->ops->draw_char(term, term->col, term->row, ch->fg, ch->bg, ch->codepoint);
}

static void sterm_draw_cursor(struct sterm *term)
{
	if (!term->draw_cursor)
		return;

	struct termchar *ch = sterm_get_at(term, term->col, term->row);
	term->ops->draw_char(term, term->col, term->row, term->black, term->white, ch->codepoint ? ch->codepoint : ' ');
}

static void sterm_redraw_row(struct sterm *term, unsigned row)
{
	for (unsigned col = 0; col < term->width; col++) {
		struct termchar *ch = sterm_get_at(term, col, row);
		if (ch->codepoint == 0)
			term->ops->clear_char(term, col, row, ch->bg);
		else
			term->ops->draw_char(term, col, row, ch->fg, ch->bg, ch->codepoint);
	}
}

static void sterm_redraw(struct sterm *term)
{
	for (unsigned row = 0; row < term->height; row++) {
		sterm_redraw_row(term, row);
	}
}


static void sterm_put_at(struct sterm *term, unsigned col, unsigned row, u32 cp, u32 fg, u32 bg)
{
	assert(col < term->width && row < term->height);
	struct termchar *ch = sterm_get_at(term, col, row);

	ch->codepoint = cp;
	ch->fg = fg;
	ch->bg = bg;

	term->ops->draw_char(term, col, row, fg, bg, cp);
}

static void sterm_scroll(struct sterm *term, int dir)
{
	assert(dir != 0);

	char *region_start;
	char *paste_start;
	size_t nlines;

	if (dir < 0) {
		/* scroll up */
		region_start = (void*) &term->chars[term->width * (-dir + term->scroll_top)];
		paste_start = (void*) &term->chars[term->width * term->scroll_top];

		nlines = term->height + dir;
	} else {
		/* scroll down */

		region_start = (void*) &term->chars[term->width * term->scroll_top];
		paste_start = (void*) &term->chars[term->width * (dir + term->scroll_top)];

		nlines = term->height - dir;
	}

	if (nlines > term->scroll_bot - term->scroll_top)
		nlines = term->scroll_bot - term->scroll_top;

	size_t nchars = nlines * term->width;
	char *end = (void*) &term->chars[term->width * term->scroll_bot];

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

static void sterm_clear_char(struct sterm *term, unsigned col, unsigned row, u32 color)
{
	struct termchar *ch = sterm_get_at(term, col, row);

	ch->codepoint = 0;
	ch->fg = color;
	ch->bg = color;

	term->ops->clear_char(term, col, row, color);
}

static void sterm_delete_chars(struct sterm *term, unsigned col, unsigned row, unsigned nchars)
{
	unsigned rem = term->width - col;
	if (nchars > rem)
		nchars = rem;

	struct termchar *start = sterm_get_at(term, col, row);
	struct termchar *end = sterm_get_at(term, col + nchars, row);

	memset(start, 0, (end - start) * sizeof(*start));

	if (nchars < rem) {
		memmove(start, start + nchars, (rem - nchars) * sizeof(*start));
		memset(start + nchars, 0, (term->width - col - nchars));
	}

	sterm_redraw_row(term, row);
}

static void sterm_reset(struct sterm *term)
{
	term->row = 0;
	term->col = 0;

	term->escape_off = 0;
	term->dcs = false;
	term->in_osc = false;

	term->draw_cursor = true;

	term->ops->get_dimensions(term, &term->width, &term->height);
	printf("width: %u height: %u\n", term->width, term->height);

	term->scroll_top = 0;
	term->scroll_bot = term->height;

	/* TODO init encoder */

	term->inverse = false;

	term->fg_color = term->white;
	term->bg_color = term->black;
	memset(term->chars, 0, term->width * term->height * sizeof(*term->chars));
	sterm_redraw(term);
}

static u32 sterm_get_fg(const struct sterm *term)
{
	if (term->inverse)
		return term->bg_color;
	return term->fg_color;
}

static u32 sterm_get_bg(const struct sterm *term)
{
	if (term->inverse)
		return term->fg_color;
	return term->bg_color;
}

static bool sterm_is_escape_ch(u32 cp)
{
	return cp < 127;
}

static void sterm_clear_escape(struct sterm *term)
{
	term->in_escape = false;
	term->escape_off = 0;
}

static void sterm_exec_dec(struct sterm *term, char *escape)
{
	size_t len = strlen(escape);
	if (!len)
		return;
        char cmd = escape[len - 1];

	while (*escape && sterm_is_terminator(*escape)) {
		long i = 0;

		if (isdigit(*escape)) {
			i = atoi(escape);
			while (isdigit(*escape))
				escape++;
		}

		if (cmd == 'l' || cmd == 'h') {
			if (i == 25) {
				bool hide = cmd == 'l';

				if (hide) {
					sterm_clear_cursor(term);
				}
				term->draw_cursor = !hide;
			} else {
				fprintf(stderr, "tried to change dec mode %4li (%c): unsupported\n", i, cmd);
			}
		} else {
			fprintf(stderr, "unknown DEC command '%c'\n", cmd);
		}
	}

}

static u32 sterm_get_color(struct sterm *term, enum termcolor color)
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

static void sterm_select_sgr(struct sterm *term, char *escape, int i)
{
	if (i < 0)
		return;

	static const enum termcolor colors[] = {
		TERMCOLOR_BLACK,
		TERMCOLOR_RED,
		TERMCOLOR_GREEN,
		TERMCOLOR_YELLOW,
		TERMCOLOR_BLUE,
		TERMCOLOR_MAGENTA,
		TERMCOLOR_CYAN,
		TERMCOLOR_WHITE
	};

	if (i == 0) {
		term->fg_color = sterm_get_color(term, TERMCOLOR_WHITE);
		term->bg_color = sterm_get_color(term, TERMCOLOR_BLACK);
	} else if (i == 7) {
		term->inverse = true;
	} else if (i == 27) {
		term->inverse = false;
	} else if (i == 38 || i == 48) {
		if (*escape++ != ';')
			return;
		long j = strtol(escape, &escape, 10);

		if (j == 2) {
			long r, g, b;

			r = strtol(escape, &escape, 10);
			if (*escape++ != ';')
				return;
			g = strtol(escape, &escape, 10);
			if (*escape++ != ';')
				return;
			b = strtol(escape, &escape, 10);

			if (!term->ops->encode_rgb)
				return;
			u32 color = term->ops->encode_rgb(term, r, g, b);

			if (i == 38)
				term->fg_color = color;
			if (i == 48)
				term->bg_color = color;
		}
	} else if (i >= 30 && i < 37) {
		term->fg_color = sterm_get_color(term, colors[i - 30]);
	} else if (i >= 40 && i < 47) {
		term->bg_color = colors[i - 40];
	}
	/* TODO bold, underline */
}

/* https://poor.dev/blog/terminal-anatomy/ */
static int sterm_put_secondary(int i, void *opaque)
{
	struct sterm *term = opaque;
	char ch = (char) i;
	return (int) write(term->fd, &ch, 1);
}

__attribute__((format(printf, 2, 3)))
static int sterm_printf(struct sterm *term, const char *fmt, ...)
{
	va_list args ;
	va_start(args, fmt);

	int res = vprintx(sterm_put_secondary, term, fmt, args);

	va_end(args);
	return res;

}

static void dump_escape_seq(const char *seq)
{
	while (*seq) {
		if (isprint(*seq)) {
			fprintf(stderr, "%c", *seq);
		} else {
			fprintf(stderr, "\\x%02x", (unsigned) *seq);
		}

		seq += 1;
	}
	fprintf(stderr, "\n");
}

static void unsup_escape(const char *seq)
{
	fprintf(stderr, "unsupported escape sequence:\n");
	dump_escape_seq(seq);
}

static void sterm_exec_osc(struct sterm *term, char *escape)
{
	char *saved = escape;
	long ps;

	ps = strtol(escape, &escape, 10);
	/* TODO check for errors */

	if (*escape++ != ';')
		return;

	bool quest = false;
	if (*escape == '?') {
		quest = true;
		escape += 1;
	}

	switch (ps) {
	case 10:
		if (!quest)
			break;
		sterm_printf(term, "%s[10;rgb:ffff/ffff/ffff", TERM_CSI);
		return;
	case 11:
		if (!quest)
			break;
		sterm_printf(term, "%s[11;rgb:0000/0000/0000", TERM_CSI);
		return;
	}

	if (!quest) {
		fprintf(stderr, "unsupported OSC:\n");
		unsup_escape(escape);
		return;
	}
}

static void sterm_exec_control_char(struct sterm *term, char ch)
{
	fprintf(stderr, "exec control char '%c'\n", ch);
	switch (ch) {
	case 'M':
		fprintf(stderr, "move cursor up\n");
		if (term->row-- == 0) {
			fprintf(stderr, "scroll up\n");
			term->row = 0;
			sterm_scroll(term, 1);
		}
		break;
	case 'P': /* device control string */
		term->dcs = true;
		break;
	case '\\':
		term->dcs = false;
		if (term->in_osc)
			sterm_exec_osc(term, term->osc_seq);
		term->in_osc = false;
		break;
	case '>':
		/* TODO exit alternate keypad mode */
		break;
	case ']':
		term->in_osc = true;
		break;
	default:
		fprintf(stderr, "unknown control char: '%c'\n", ch);
		break;
	}
}

static void sterm_exec_escape(struct sterm *term, char *escape)
{
	char *saved = escape;

	if (*escape++ != '[') {
		unsup_escape(saved);
		return;
	}

	if (*escape == '?') {
		sterm_exec_dec(term, ++escape);
		return;
	}

	if (*escape == '!') {
		if (*++escape != 'p') {
			unsup_escape(saved);
		} else {
			sterm_reset(term);
		}
	}
	
	size_t len = strlen(escape);
	if (!len)
		return;
        char cmd = escape[len - 1];
	bool angle = false;

	if (*escape == '>') {
		escape += 1;
		angle = true;
	}

        do {
		long i = 0;
		long j = 0;

		if (!isdigit(*escape) && *escape != ';' && !sterm_is_terminator(*escape)) {
			unsup_escape(saved);
			break;
		}

		if (isdigit(*escape)) {
			i = atoi(escape);
			while (isdigit(*escape))
				escape++;
		}

		if (!*escape)
			return;

		switch (cmd) {
		case 'm': /* select graphic rendition */
			if (angle) {
				unsup_escape(saved);
				break;
			}
			if (*escape == '%')
				return;
			sterm_select_sgr(term, escape, i);
			break;
		case 'H': /* set cursor pos */
			sterm_clear_cursor(term);
			if (*escape == ';') {
				escape += 1;
				j = strtol(escape, &escape, 10);
			}

			if (i > term->height)
				i = term->height;
			if (j > term->width)
				j = term->width;

			if (i)
				i -= 1;
			if (j)
				j -= 1;

			term->row = i;
			term->col = j;
			break;
		case 'J': /* clear screen from cursor to end */
			for (unsigned row = term->row; row < term->height; row++) {
				for (unsigned col = term->col; col < term->width; col++) {
						sterm_clear_char(term, col, row, term->bg_color);
				}
			}
			break;
		case 'K': { /* clear line ... */
			/* default: from cursor to end */
			unsigned start = term->col;
			unsigned end = term->width;

			if (i == 1) { /* from start to cursor */
				start = 0;
				end = term->col;
			} else if (i == 2) { /* entire line */
				start = 0;
				end = term->width;
			} else if (i != 0) {
				break;
			}

			for (unsigned col = start; col < end; col++) {
					sterm_clear_char(term, col, term->row, term->bg_color);
			}
			break;
		}
		case 'P': /* delete N characters start at cursor pos */
			if (!i)
				sterm_delete_chars(term, term->col, term->row, i);
			break;
		case 'A': /* move cursor up */
			if (!i)
				i = 1;
			if (i > term->row)
				i = term->row;
			term->row -= i;
			break;
		case 'n': /* report cursor pos */
			if (i == 6)
				sterm_printf(term, "%s%u;%uR", TERM_CSI, term->row, term->col);
			else
				unsup_escape(saved);
			break;
		case 'C': /* move cursor right */
			sterm_clear_cursor(term);
			if (!i)
				i = 1;

			if (i > (term->width - term->col))
				i = term->width - term->col;

			term->col += i;
			break;
		case 'D': /* move cursor left */
			sterm_clear_cursor(term);
			if (!i)
				i = 1;
			if (i > term->col)
				i = term->col;

			term->col -= i;
			break;
		case 'c':
			if (angle) {
				if (i != 0) {
					fprintf(stderr, "unknown identification request %li\n", i);
				} else {
					sterm_printf(term, "%s>1;95;0c", TERM_CSI);
					break;
				}
			}

			unsup_escape(saved);
			break;
		case 'r': /* set scroll region */
			if (*escape++ != ';')
				return;
			j = strtol(escape, &escape, 10);
			/* TODO check for errors */

			if (i)
				i -= 1;
			if (j > term->height)
				j = term->height;
			term->scroll_bot = i;
			term->scroll_top = j;
			break;
		case 'p': /* soft reset */
			if (*escape++ != '!')
				return;
			sterm_reset(term);
		default:
			unsup_escape(saved);

			escape += strlen(escape);
			continue;
		}

		if (*escape == ';')
			escape++;
	} while (*escape && !sterm_is_terminator(*escape));
}

static void sterm_put_escaped(struct sterm *term, u32 cp)
{
	if (!sterm_is_escape_ch(cp) || term->escape_off >= sizeof(term->escape_seq) - 1) {
		sterm_clear_escape(term);
		return;
	}

	char ch = (char) cp;
	term->escape_seq[term->escape_off++] = ch;

	if (term->escape_off == 1 && ch != '[') {
		sterm_exec_control_char(term, ch);
		term->in_escape = false;
		term->escape_off = 0;
		return;
	}

	if (sterm_is_terminator(ch)) {
		/* end of escape sequence */
		term->escape_seq[term->escape_off] = '\0';

		fprintf(stderr, "exec escape seq:\n");
		dump_escape_seq(term->escape_seq);

		sterm_exec_escape(term, term->escape_seq);
		term->in_escape = false;
		term->escape_off = 0;
	}
}

static void sterm_put_osc(struct sterm *term, u32 cp)
{
	if (!sterm_is_escape_ch(cp) || term->osc_off >= sizeof(term->osc_seq) - 1) {
		term->osc_off = 0;
		term->in_osc = false;
		return;
	}

	char ch = (char) cp;

	if (sterm_is_terminator(ch) || ch == '\a') {
		sterm_exec_osc(term, term->osc_seq);
		term->osc_off = 0;
		term->in_osc = false;
	} else {
		term->osc_seq[term->osc_off++] = ch;
		term->osc_seq[term->osc_off] = '\0';
	}
}

static void sterm_newline(struct sterm *term)
{
	if (++term->row == term->height) {
		term->row -= 1;
		sterm_scroll(term, -1);
	}
}

static void sterm_put_normal(struct sterm *term, u32 cp)
{
	sterm_put_at(term, term->col++, term->row, cp, sterm_get_fg(term), sterm_get_bg(term));

	if (term->col == term->width) {
		term->col = 0;
		sterm_newline(term);
	}
}

i64 enc_push_byte(struct enc_ctx *ctx, u8 byte)
{
	/* TODO proper encoding */
	(void) ctx;
	return byte;
}

static void sterm_put(struct sterm *term, u8 b)
{
	i64 res = enc_push_byte(&term->encoder, b);
	if (res < 0)
		return;

	u32 cp = (u32) res;

	if (term->in_escape) {
		sterm_put_escaped(term, cp);
		return;
	}

	if (term->in_osc) {
		sterm_put_osc(term, cp);
		return;
	}

	switch (b) {
	case 0x00:
		break;
	case ANSI_ESCAPE:
		term->in_escape = true;
		break;
	case LINEFEED:
		sterm_newline(term);
		break;
	case CARRIAGE_RETURN:
		sterm_clear_cursor(term);
		term->col = 0;
		break;
	case BACKSPACE:
		if (term->col) {
			sterm_clear_cursor(term);
			term->col -= 1;
		}
		break;
	case '\t':
		for (int i = 0; i < 4; i++)
			sterm_put(term, ' ');
		break;
	case '\a': /* bell */
		break;
	default:
		if (!term->dcs)
			sterm_put_normal(term, cp);
		break;
	}
}

void sterm_write(struct sterm *term, const void *buf, size_t n)
{
	const unsigned char *cbuf = buf;
	for (size_t i = 0; i < n; i++) {
		sterm_put(term, cbuf[i]);
	}
	sterm_draw_cursor(term);
}

int sterm_init(struct sterm *term, const struct term_ops *ops, void *ctx)
{
	term->row = 0;
	term->col = 0;

	term->priv = ctx;
	term->escape_off = 0;
	term->ops = ops;
	term->dcs = false;
	term->in_osc = false;

	term->draw_cursor = true;

	term->ops->get_dimensions(term, &term->width, &term->height);
	printf("width: %u height: %u\n", term->width, term->height);

	term->scroll_top = 0;
	term->scroll_bot = term->height;

	/* TODO init encoder */

	term->white = sterm_get_color(term, TERMCOLOR_WHITE);
	term->black = sterm_get_color(term, TERMCOLOR_BLACK);
	term->inverse = false;

	term->fg_color = term->white;
	term->bg_color = term->black;

	term->chars = calloc(term->width * term->height, sizeof(*term->chars));
	if (!term->chars)
		return -1;
	return 0;
}
