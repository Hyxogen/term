#include "parser.h"

extern void collect(struct parser *ctx, uint32_t cp);
extern void clear(struct parser *ctx, uint32_t cp);
extern void hook(struct parser *ctx, uint32_t cp);
extern void csi_dispatch(struct parser *ctx, uint32_t cp);
extern void print(struct parser *ctx, uint32_t cp);
extern void execute(struct parser *ctx, uint32_t cp);
extern void put(struct parser *ctx, uint32_t cp);
extern void param(struct parser *ctx, uint32_t cp);
extern void unhook(struct parser *ctx, uint32_t cp);
extern void error(struct parser *ctx, uint32_t cp);
extern void esc_dispatch(struct parser *ctx, uint32_t cp);

void parser_init(struct parser *ctx, void *priv)
{
	ctx->state = STATE_GROUND;
	ctx->priv = priv;
}

void parser_push(struct parser *ctx, uint32_t cp)
{
	if (ctx->state == STATE_DCS_PASSTHROUGH) {
		switch (cp) {
		case 0x9c:
			unhook(ctx, cp);
			ctx->state = STATE_GROUND;
			return;
		}
	}
	if (1) {
		switch (cp) {
		case 0x18:
		case 0x1a:
			execute(ctx, cp);
			ctx->state = STATE_GROUND;
			return;
		}
	}
	if (1) {
		switch (cp) {
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0x8f:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x99:
		case 0x9a:
			execute(ctx, cp);
			ctx->state = STATE_GROUND;
			return;
		}
	}
	if (1) {
		switch (cp) {
		case 0x9c:
			ctx->state = STATE_GROUND;
			return;
		}
	}
	if (1) {
		switch (cp) {
		case 0x9b:
			clear(ctx, cp);
			ctx->state = STATE_CSI_ENTRY;
			return;
		}
	}
	if (1) {
		switch (cp) {
		case 0x1b:
			clear(ctx, cp);
			ctx->state = STATE_ESCAPE;
			return;
		}
	}
	if (1) {
		switch (cp) {
		case 0x98:
		case 0x9e:
		case 0x9f:
			ctx->state = STATE_SOSPMAPC;
			return;
		}
	}
	if (1) {
		switch (cp) {
		case 0x90:
			clear(ctx, cp);
			ctx->state = STATE_DCS_ENTRY;
			return;
		}
	}
	if (ctx->state == STATE_GROUND) {
		switch (cp) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x19:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			execute(ctx, cp);
			ctx->state = STATE_GROUND;
			return;
		}
	}
	if (ctx->state == STATE_GROUND) {
		switch (cp) {
		default:
			print(ctx, cp);
			ctx->state = STATE_GROUND;
			return;
		}
	}
	if (ctx->state == STATE_SOSPMAPC) {
		switch (cp) {
		case 0x9c:
			ctx->state = STATE_GROUND;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE) {
		switch (cp) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x19:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			execute(ctx, cp);
			ctx->state = STATE_ESCAPE;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE) {
		switch (cp) {
		case 0x7f:
			ctx->state = STATE_ESCAPE;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE) {
		switch (cp) {
		case 0x5b:
			ctx->state = STATE_CSI_ENTRY;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE) {
		switch (cp) {
		case 0x5d:
			ctx->state = STATE_OSC_STRING;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE) {
		switch (cp) {
		case 0x50:
			clear(ctx, cp);
			ctx->state = STATE_DCS_ENTRY;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE) {
		switch (cp) {
		case 0x58:
		case 0x5e:
		case 0x5f:
			ctx->state = STATE_SOSPMAPC;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE) {
		switch (cp) {
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			collect(ctx, cp);
			ctx->state = STATE_ESCAPE_INTERMEDIATE;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE) {
		switch (cp) {
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x59:
		case 0x5a:
		case 0x5c:
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
			esc_dispatch(ctx, cp);
			ctx->state = STATE_GROUND;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE) {
		switch (cp) {
		default:
			clear(ctx, cp);
			ctx->state = STATE_ESCAPE;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE_INTERMEDIATE) {
		switch (cp) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x19:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			execute(ctx, cp);
			ctx->state = STATE_ESCAPE_INTERMEDIATE;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE_INTERMEDIATE) {
		switch (cp) {
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			collect(ctx, cp);
			ctx->state = STATE_ESCAPE_INTERMEDIATE;
			return;
		}
	}
	if (ctx->state == STATE_ESCAPE_INTERMEDIATE) {
		switch (cp) {
		default:
			ctx->state = STATE_ESCAPE_INTERMEDIATE;
			return;
		}
	}
	if (ctx->state == STATE_CSI_ENTRY) {
		switch (cp) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x19:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			execute(ctx, cp);
			ctx->state = STATE_CSI_ENTRY;
			return;
		}
	}
	if (ctx->state == STATE_CSI_ENTRY) {
		switch (cp) {
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			collect(ctx, cp);
			ctx->state = STATE_CSI_INTERMEDIATE;
			return;
		}
	}
	if (ctx->state == STATE_CSI_ENTRY) {
		switch (cp) {
		default:
			ctx->state = STATE_CSI_ENTRY;
			return;
		}
	}
	if (ctx->state == STATE_CSI_ENTRY) {
		switch (cp) {
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3b:
			param(ctx, cp);
			ctx->state = STATE_CSI_PARAM;
			return;
		}
	}
	if (ctx->state == STATE_CSI_ENTRY) {
		switch (cp) {
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			collect(ctx, cp);
			ctx->state = STATE_CSI_PARAM;
			return;
		}
	}
	if (ctx->state == STATE_CSI_PARAM) {
		switch (cp) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x19:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			execute(ctx, cp);
			ctx->state = STATE_CSI_PARAM;
			return;
		}
	}
	if (ctx->state == STATE_CSI_PARAM) {
		switch (cp) {
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3b:
			param(ctx, cp);
			ctx->state = STATE_CSI_PARAM;
			return;
		}
	}
	if (ctx->state == STATE_CSI_PARAM) {
		switch (cp) {
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			collect(ctx, cp);
			ctx->state = STATE_CSI_INTERMEDIATE;
			return;
		}
	}
	if (ctx->state == STATE_CSI_PARAM) {
		switch (cp) {
		case 0x3a:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			ctx->state = STATE_CSI_IGNORE;
			return;
		}
	}
	if (ctx->state == STATE_CSI_IGNORE) {
		switch (cp) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x19:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			execute(ctx, cp);
			ctx->state = STATE_CSI_IGNORE;
			return;
		}
	}
	if (ctx->state == STATE_CSI_IGNORE) {
		switch (cp) {
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
			ctx->state = STATE_GROUND;
			return;
		}
	}
	if (ctx->state == STATE_CSI_IGNORE) {
		switch (cp) {
		default:
			ctx->state = STATE_CSI_IGNORE;
			return;
		}
	}
	if (ctx->state == STATE_CSI_INTERMEDIATE) {
		switch (cp) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x19:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			execute(ctx, cp);
			ctx->state = STATE_CSI_INTERMEDIATE;
			return;
		}
	}
	if (ctx->state == STATE_CSI_INTERMEDIATE) {
		switch (cp) {
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			collect(ctx, cp);
			ctx->state = STATE_CSI_INTERMEDIATE;
			return;
		}
	}
	if (ctx->state == STATE_CSI_INTERMEDIATE) {
		switch (cp) {
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
			csi_dispatch(ctx, cp);
			ctx->state = STATE_GROUND;
			return;
		}
	}
	if (ctx->state == STATE_CSI_INTERMEDIATE) {
		switch (cp) {
		case 0x7f:
			ctx->state = STATE_CSI_INTERMEDIATE;
			return;
		}
	}
	if (ctx->state == STATE_CSI_INTERMEDIATE) {
		switch (cp) {
		default:
			ctx->state = STATE_CSI_IGNORE;
			return;
		}
	}
	if (ctx->state == STATE_DCS_ENTRY) {
		switch (cp) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x19:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			ctx->state = STATE_DCS_ENTRY;
			return;
		}
	}
	if (ctx->state == STATE_DCS_ENTRY) {
		switch (cp) {
		case 0x7f:
			ctx->state = STATE_DCS_ENTRY;
			return;
		}
	}
	if (ctx->state == STATE_DCS_ENTRY) {
		switch (cp) {
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			collect(ctx, cp);
			ctx->state = STATE_DCS_INTERMEDIATE;
			return;
		}
	}
	if (ctx->state == STATE_DCS_ENTRY) {
		switch (cp) {
		case 0x3a:
			ctx->state = STATE_DCS_IGNORE;
			return;
		}
	}
	if (ctx->state == STATE_DCS_ENTRY) {
		switch (cp) {
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
			hook(ctx, cp);
			ctx->state = STATE_DCS_PASSTHROUGH;
			return;
		}
	}
	if (ctx->state == STATE_DCS_ENTRY) {
		switch (cp) {
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3b:
			param(ctx, cp);
			ctx->state = STATE_DCS_PARAM;
			return;
		}
	}
	if (ctx->state == STATE_DCS_ENTRY) {
		switch (cp) {
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			collect(ctx, cp);
			ctx->state = STATE_DCS_PARAM;
			return;
		}
	}
	if (ctx->state == STATE_DCS_ENTRY) {
		switch (cp) {
		default:
			ctx->state = STATE_DCS_IGNORE;
			return;
		}
	}
	if (ctx->state == STATE_DCS_IGNORE) {
		switch (cp) {
		default:
			ctx->state = STATE_DCS_IGNORE;
			return;
		}
	}
	if (ctx->state == STATE_DCS_INTERMEDIATE) {
		switch (cp) {
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			ctx->state = STATE_DCS_IGNORE;
			return;
		}
	}
	if (ctx->state == STATE_DCS_INTERMEDIATE) {
		switch (cp) {
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
			hook(ctx, cp);
			ctx->state = STATE_DCS_PASSTHROUGH;
			return;
		}
	}
	if (ctx->state == STATE_DCS_PARAM) {
		switch (cp) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x19:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			ctx->state = STATE_DCS_PARAM;
			return;
		}
	}
	if (ctx->state == STATE_DCS_PARAM) {
		switch (cp) {
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3b:
			param(ctx, cp);
			ctx->state = STATE_DCS_PARAM;
			return;
		}
	}
	if (ctx->state == STATE_DCS_PARAM) {
		switch (cp) {
		case 0x7f:
			ctx->state = STATE_DCS_PARAM;
			return;
		}
	}
	if (ctx->state == STATE_DCS_PARAM) {
		switch (cp) {
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			collect(ctx, cp);
			ctx->state = STATE_DCS_INTERMEDIATE;
			return;
		}
	}
	if (ctx->state == STATE_DCS_PARAM) {
		switch (cp) {
		default:
			ctx->state = STATE_DCS_IGNORE;
			return;
		}
	}
	if (ctx->state == STATE_DCS_PASSTHROUGH) {
		switch (cp) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x19:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
			put(ctx, cp);
			ctx->state = STATE_DCS_PASSTHROUGH;
			return;
		}
	}
	if (ctx->state == STATE_DCS_PASSTHROUGH) {
		switch (cp) {
		case 0x7f:
			ctx->state = STATE_DCS_PASSTHROUGH;
			return;
		}
	}
	if (1) {
		switch (cp) {
		default:
			error(ctx, cp);
			ctx->state = STATE_GROUND;
			return;
		}
	}
}
