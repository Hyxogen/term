#include <sterm/sterm.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
/* TODO remove */
#include <stdio.h>

#define ANSI_ESCAPE 0x1b
#define LINEFEED 0x0a
#define CARRIAGE_RETURN 0x0d
#define BACKSPACE 0x08
#define TERM_CSI "\033"

static bool sterm_is_escape_ch(u32 cp)
{
	return cp < 127;
}

static void sterm_clear_escape(struct sterm *term)
{
	term->in_escape = false;
	term->escape_off = 0;
}

static int get_dec_mode_bit(int Ps)
{
	switch (Ps) {
	case 1: /* Application Cursor Keys (DECCKM) */
	case 2: /* Designate USASCII for character sets G0-G3 (DECANM), and set VT100 mode. */
	case 3: /* 132 Column Mode (DECCOLM) */
	case 4: /* Smooth (Slow) Scroll (DECSCLM) */
	case 5: /* Reverse Video (DECSCNM) */
	case 6: /* Origin Mode (DECOM) */
	case 7: /* Wraparound Mode (DECAWM) */
	case 8: /* Auto-repeat Keys (DECARM) */
	case 9: /* Send Mouse X & Y on button press. See the section Mouse Tracking. */
	case 10: /* Show toolbar (rxvt) */
	case 12: /* Start Blinking Cursor (att610) */
	case 18: /* Print form feed (DECPFF) */
	case 19: /* Set print extent to full screen (DECPEX) */
	case 25: /* Show Cursor (DECTCEM) */
	case 30: /* Show scrollbar (rxvt). */
	case 35: /* Enable font-shifting functions (rxvt). */
	case 38: /* Enter Tektronix Mode (DECTEK) */
	case 40: /* Allow 80 → 132 Mode */
	case 41: /* more(1) fix (see curses resource) */
	case 42: /* Enable Nation Replacement Character sets (DECNRCM) */
	case 44: /* Turn On Margin Bell */
	case 45: /* Reverse-wraparound Mode */
	case 46: /* Start Logging (normally disabled by a compile-time option) */
	case 47: /* Use Alternate Screen Buffer (unless disabled by the titeInhibit resource) */
	case 66: /* Application keypad (DECNKM) */
	case 67: /* Backarrow key sends backspace (DECBKM) */
	case 1000: /* Send Mouse X & Y on button press and release. See the section Mouse Tracking. */
	case 1001: /* Use Hilite Mouse Tracking. */
	case 1002: /* Use Cell Motion Mouse Tracking. */
	case 1003: /* Use All Motion Mouse Tracking. */
	case 1010: /* Scroll to bottom on tty output (rxvt). */
	case 1011: /* Scroll to bottom on key press (rxvt). */
	case 1035: /* Enable special modifiers for Alt and NumLock keys. */
	case 1036: /* Send ESC when Meta modifies a key (enables the metaSendsEscape resource). */
	case 1037: /* Send DEL from the editing-keypad Delete key */
	case 1047: /* Use Alternate Screen Buffer (unless disabled by the titeInhibit resource) */
	case 1048: /* Save cursor as in DECSC (unless disabled by the titeInhibit resource) */
	case 1049: /* Save cursor as in DECSC and use Alternate Screen Buffer, clearing it first (unless disabled by the titeInhibit resource). This combines the effects of the 1 0 4 7 and 1 0 4 8 modes. Use this with terminfo-based applications rather than the 4 7 mode. */
	case 1051: /* Set Sun function-key mode. */
	case 1052: /* Set HP function-key mode. */
	case 1053: /* Set SCO function-key mode. */
	case 1060: /* Set legacy keyboard emulation (X11R6). */
	case 1061: /* Set Sun/PC keyboard emulation of VT220 keyboard. */
	case 2004: /* Set bracketed paste mode. */
	default:
		return -1;
	};
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
		term->fg_color = sterm_get_color(term, TERMCOLOR_BLACK);
		return;
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
		return;
	}

	/* TODO bold, underline, inverse */
	if (i >= 30 && i < 37) {
		term->fg_color = sterm_get_color(term, colors[i - 30]);
	} else if (i >= 40 && i < 47) {
		term->bg_color = colors[i - 40];
	}
}

/* https://poor.dev/blog/terminal-anatomy/ */
static int sterm_put_secondary(int ch, void *opaque)
{
	/* TODO write to attached program */
	return 1;
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
	}
	fprintf(stderr, "\n");
}

static void sterm_exec_escape(struct sterm *term, char *escape)
{
	char *saved = escape;

	if (*escape++ != '[') {
		fprintf(stderr, "unsupported escape sequence:\n");
		dump_escape_seq(saved);
		return;
	}

	size_t len = strlen(escape);
	if (!len)
		return;
        char cmd = escape[len - 1];

        while (!isalpha(*escape) && *escape != '~') {
		long i = 0;
		long j = 0;

		if (isdigit(*escape)) {
			i = atoi(escape);
			while (isdigit(*escape))
				escape++;
		}

		if (!*escape)
			return;

		switch (cmd) {
		case 'm': /* select graphic rendition */
			sterm_select_sgr(term, escape, i);
			break;
		case 'H': /* set cursor pos */
			if (*escape == ';') {
				escape += 1;
				j = strtol(escape, &escape, 10);
			}
			term->row = i;
			term->col = j;
			break;
		case 'J': /* clear screen from cursor to end */
			for (unsigned row = term->row; row < term->height; row++) {
				for (unsigned col = term->col; col < term->width; col++) {
						term->ops->clear_char(term, col, row, term->bg_color);
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
			} else {
				break;
			}

			for (unsigned col = start; col < end; col++) {
					term->ops->clear_char(term, col, term->row, term->bg_color);
			}
			break;
		}
		case 'n': /* report cursor pos */
			if (i == 6)
				sterm_printf(term, "%s%u;%uR", TERM_CSI, term->row, term->col);
			break;
		case 'C': /* move cursor right */
			/* TODO check for overflow */
			term->col += i;
			if (term->col >= term->width)
				term->col = term->width - 1;
			break;
		default:
			fprintf(stderr, "unsupported escape sequence:\n");
			dump_escape_seq(saved);
			continue;
		}

		if (*escape == ';')
			escape++;
	}
}

static void sterm_put_escaped(struct sterm *term, u32 cp)
{
	if (!sterm_is_escape_ch(cp) || term->escape_off >= sizeof(term->escape_seq) - 1) {
		sterm_clear_escape(term);
		return;
	}

	char ch = (char) cp;
	term->escape_seq[term->escape_off++] = ch;

	if (isalpha(ch) || ch == '~') {
		/* end of escape sequence */
		term->escape_seq[term->escape_off] = '\0';
		sterm_exec_escape(term, term->escape_seq);
	}
}

static void sterm_scroll(struct sterm *term, int dir)
{
	if (term->ops->scroll) {
		term->ops->scroll(term, term->bg_color, dir);
	} else {
		/* TODO */
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
	term->ops->draw_char(term, term->col++, term->row, term->fg_color, term->bg_color, cp);

	if (term->col == term->width) {
		term->col = 0;
		sterm_newline(term);
	}
}

static void sterm_clear(struct sterm *term, size_t col, size_t row, u32 color)
{
	term->ops->clear_char(term, col, row, color);
}

static void sterm_draw_cursor(struct sterm *term)
{
	sterm_clear(term, term->col, term->row, term->fg_color);
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

	if (term->in_escape)
		sterm_put_escaped(term, cp);

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
		sterm_clear(term, term->col, term->row, term->bg_color);
		term->col = 0;
		break;
	case BACKSPACE:
		if (term->col) {
			sterm_clear(term, term->col, term->row, term->bg_color);
			term->col -= 1;
		}
		break;
	default:
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

void sterm_init(struct sterm *term, const struct term_ops *ops, void *ctx)
{
	term->row = 0;
	term->col = 0;

	term->priv = ctx;
	term->escape_off = 0;
	term->ops = ops;

	term->ops->get_dimensions(term, &term->width, &term->height);
	printf("width: %u height: %u\n", term->width, term->height);

	/* TODO init encoder */

	term->fg_color = sterm_get_color(term, TERMCOLOR_WHITE);
	term->bg_color = sterm_get_color(term, TERMCOLOR_BLACK);
}
