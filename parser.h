#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

enum parser_state {
	STATE_DCS_PASSTHROUGH,
	STATE_GROUND,
	STATE_CSI_ENTRY,
	STATE_ESCAPE,
	STATE_SOSPMAPC,
	STATE_DCS_ENTRY,
	STATE_OSC_STRING,
	STATE_ESCAPE_INTERMEDIATE,
	STATE_CSI_INTERMEDIATE,
	STATE_CSI_PARAM,
	STATE_CSI_IGNORE,
	STATE_DCS_INTERMEDIATE,
	STATE_DCS_IGNORE,
	STATE_DCS_PARAM,
};

struct parser {
	enum parser_state state;
	void *priv;
};

void parser_init(struct parser *ctx, void *priv);
void parser_push(struct parser *ctx, uint32_t cp);

#endif
