#include <stdbool.h>
#include <sterm/types.h>
#include <sterm/ctrl.h>
#include <stdio.h>

#define eprintln(...) fprintf(stderr, __VA_ARGS__)

struct encoder;

struct encoder_ops {
	i64 (*put)(struct encoder *, char ch);
};

struct encoder {
	char buf[16];
	unsigned char offset;
	const struct encoder_ops *ops;
};

enum term_state {
	STATE_GROUND,
	STATE_ESCAPE,
	STATE_ESCAPE_INTERMEDIATE,
	STATE_CSI_ENTRY,
	STATE_CSI_PARAM,
	STATE_CSI_INTERMEDIATE,
	STATE_CSI_IGNORE,
	STATE_DCS_ENTRY,
	STATE_DCS_INTERMEDIATE,
	STATE_DCS_IGNORE,
	STATE_DCS_PARAM,
	STATE_DCS_PASTHROUGH,
	STATE_OSC,
	STATE_SOSPMAPC,
};

struct term {
	unsigned rows;
	unsigned cols;

	unsigned row;
	unsigned col;

	struct encoder encoder;

	enum term_state state;

	u32 fg_color;
	u32 bg_color;

	char buf[64];
	unsigned char buf_off;
	bool overflowed;
};

static bool term_is_c0(u32 cp)
{
	return cp <= C0_US;
}

static bool term_is_c1(u32 cp)
{
	return cp >= C1_PAD && cp <= C1_APC;
}

static bool term_is_ctrl(u32 cp)
{
	return term_is_c0(cp) || term_is_c1(cp);
}

static void term_exec_c0(struct term *term, char ctrl)
{
	switch (ctrl) {
	case C0_NUL:
		break;
	case C0_ESC:
		term->state = STATE_ESCAPE;
		break;
	default:
		eprintln("unsupported C0 control charachter: '%s' (0x%02x)\n",
			 ctrl_get_long_name(ctrl), (unsigned)ctrl);
		break;
	}
}

static void term_linefeed(struct term *term)
{
	if (++term->row == term->rows) {
		term->row -= 1;
		term_scroll(term, -1);
	}
}

static void term_clear_buf(struct term *term)
{
	term->buf_off = 0;
	term->overflowed = false;
}

static void term_exec_ctrl(struct term *term, unsigned char ctrl)
{
	switch (ctrl) {
	case C0_NUL:
		break;
	case C0_ESC:
		term->state = STATE_ESCAPE;
		break;
	case C0_SUB:
		/* TODO print substitution char */
		/* fallthrough */
	case C1_ST:
	case C0_CAN:
		term->state = STATE_GROUND;
		break;
	case C1_CSI:
		term->state = STATE_CSI_ENTRY;
		break;
	case C1_DCS:
		term->state = STATE_DCS_ENTRY;
		break;
	case C1_OSC:
		term->state = STATE_OSC;
		break;
	case C1_SOS:
	case C1_PM:
	case C1_APC:
		term->state = STATE_SOSPMAPC;
		break;
	default:
		eprintln("unsupported control charachter: '%s' (0x%02x)\n",
			 ctrl_get_long_name(ctrl), (unsigned)ctrl);
		term->state = STATE_GROUND;
		break;

	}
}

static void term_exec_escape(struct term *term, u32 cp)
{

}

static void term_collect(struct term *term, char ch)
{
	if (term->overflowed || term->buf_off == sizeof(term->buf) - 1) {
		term->overflowed = true;
		return;
	}

	term->buf[term->buf_off++] = ch;
	term->buf[term->buf_off] = '\0';
}

static void term_put_escaped(struct term *term, u32 cp)
{
	term_clear_buf(term);

	if (term_is_c0(cp)) {
		term_exec_ctrl(term, cp);
		return;
	}

	switch (cp) {
	case 0x00 ... 0x17:
	case 0x19:
	case 0x1c ... 0x1f:
		term_exec_escape(term, cp);
		break;
	case 0x20 ... 0x2f:
		term->state = STATE_ESCAPE_INTERMEDIATE;
		term_collect(term, cp);
		break;
	case 0x50: /* DCS */
		term->state = STATE_DCS_ENTRY;
		return;
	case 0x5b: /* CSI */
		term->state = STATE_CSI_ENTRY;
		return;
	case 0x5d: /* OSC */
		term->state = STATE_OSC;
		return;
	case 0x7f: /* ignore */
		break;
	default:
		eprintln("unknown escape character 0%08x", cp);
		break;
	}

	term->state = STATE_GROUND;
}

static void term_esc_dispatch(struct term *term, u32 cp)
{
	if (!term->overflowed) {
		/* TODO execute escape sequence */
	}
	term->state = STATE_GROUND;
}

static void term_put_escaped_intermediate(struct term *term, u32 cp)
{
	if (term_is_c0(cp)) {
		term_exec_ctrl(term, cp);
		return;
	}

	switch (cp) {
	case 0x20 ... 0x2f:
		term_collect(term, cp);
		break;
	case 0x30 ... 0x7e:
		term_esc_dispatch(term, cp);
		break;
	case 0x7f: /* ignore */
		break;
	}
}

static void term_print(struct term *term, u32 cp)
{
	term_set_char(term, term->col++, term->row, cp, term->fg_color, term->bg_color);

	if (term->col == term->cols) {
		term->col = 0;
		term_linefeed(term);
	}
}

static void term_put_ground(struct term *term, u32 cp)
{
	if (term_is_c0(cp)) {
		term_exec_ctrl(term, cp);
	} else {
		term_print(term, cp);
	}
}

static void term_put(struct term *term, char ch)
{
	u64 res = term->encoder.ops->put(&term->encoder, ch);
	if (res < 0) {
		return;
	}
	
	u32 cp = (u32) res;

}
