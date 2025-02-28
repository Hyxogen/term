#ifndef STERM_PSF_H
#define STERM_PSF_H

#include <sterm/types.h>

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

int psf_mem_to_font(struct font *dest, const void *src, size_t size);
const u8 *font_get_glyph(const struct font *font, u32 codepoint);

extern const unsigned char psf2_default_font[];
extern const unsigned int psf2_default_font_len;
extern const unsigned char psf2_default_bold_font[];
extern const unsigned int psf2_default_bold_font_len;


#endif
