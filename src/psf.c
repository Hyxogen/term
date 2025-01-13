#include <sterm/sterm.h>

int font_read_from(struct font *dest, const void *src, size_t size)
{
	dest->psf = (const struct psf2_font *)src;
	dest->size = size;

	// todo check magic
	if (dest->psf->hdr.flags) {
		for (unsigned i = 0; i < sizeof(dest->ascii); ++i) {
			dest->ascii[i] = i;
		}
		return 0;
	}

	const u8 *end = dest->psf->data + size;
	const u8 *s =
	    dest->psf->data + dest->psf->hdr.count * dest->psf->hdr.charsize;
	u8 glyph = 0;
	while (s < end) {
		u8 b = *s;

		if (b == 0xFF) {
			++glyph;
		} else if ((b & 0xF8) == 0xF0) {
			s += 3;
		} else if ((b & 0xF0) == 0xD0) {
			s += 2;
		} else if ((b & 0xD0) == 0xC0) {
			s += 1;
		} else if ((b & 0x80) == 0x00) {
			dest->ascii[b] = glyph;
		} else {
			// invalid utf8
			continue;
		}

		++s;
	}
	return 0;
}

const u8 *font_get_glyph(struct font *font, u32 codepoint)
{
	return font->psf->data +
	       font->ascii[(u8)codepoint] * font->psf->hdr.charsize;
}
