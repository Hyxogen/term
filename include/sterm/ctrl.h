#ifndef STERM_CTRL_H
#define STERM_CTRL_H

#define C0_CTRL_CHARS \
	X(NUL, "Null", 0x00), \
	X(SOH, "Start of heading", 0x01), \
	X(STX, "Start of text", 0x02), \
	X(ETX, "End of text", 0x03), \
	X(EOT, "End of transmission", 0x04), \
	X(ENQ, "Enquiry", 0x05), \
	X(ACK, "Acknowledge", 0x06), \
	X(BEL, "Bell", 0x07), \
	X(BS, "Backspace", 0x08), \
	X(HT, "Horizontal tabulation", 0x09), \
	X(LF, "Linefeed", 0x0A), \
	X(VT, "Vertical tabulation", 0x0B), \
	X(FF, "Form feed", 0x0C), \
	X(CR, "Carriage return", 0x0D), \
	X(SO, "Shift out", 0x0E), \
	X(SI, "Shift in", 0x0F), \
	X(DLE, "Data link escape", 0x10), \
	X(DC1, "Device control 1", 0x11), \
	X(DC2, "Device control 2", 0x12), \
	X(DC3, "Device control 3", 0x13), \
	X(DC4, "Device control 4", 0x14), \
	X(NAK, "Negative acknowledge", 0x15), \
	X(SYN, "Synchronous idle", 0x16), \
	X(ETB, "End of transmission block", 0x17), \
	X(CAN, "Cancel", 0x18), \
	X(EM, "End of medium", 0x19), \
	X(SUB, "Substitute", 0x1A), \
	X(ESC, "Escape", 0x1B), \
	X(FS, "File separator", 0x1C), \
	X(GS, "Group separator", 0x1D), \
	X(RS, "Record separator", 0x1E), \
	X(US, "Unit separator", 0x1F),

#define C1_CTRL_CHARS \
	X(PAD, "Padding", 0x80), \
	X(HOP, "High octet present", 0x81), \
	X(BPH, "Break permitted here", 0x82), \
	X(NBH, "No break here", 0x83), \
	X(IND, "Index", 0x84), \
	X(NEL, "Next line", 0x85), \
	X(SSA, "Start of selected area", 0x86), \
	X(ESA, "End of selected area", 0x87), \
	X(HTS, "Character tabulation set", 0x88), \
	X(HTJ, "Character tabulation set with justifictation", 0x89), \
	X(VTS, "Line tabulation set", 0x8A), \
	X(PLD, "Partial line forward", 0x8B), \
	X(PLU, "Partial line backward", 0x8C), \
	X(RI, "Reverse line feed", 0x8D), \
	X(SS2, "Single-shift two", 0x8E), \
	X(SS3, "Single-shift three", 0x8F), \
	X(DCS, "Device control string", 0x90), \
	X(PU1, "Private use 1", 0x91), \
	X(PU2, "Private use 2", 0x92), \
	X(STS, "Set transmit state", 0x93), \
	X(CCH, "Cancel character", 0x94), \
	X(MW, "Message waiting", 0x95), \
	X(SPA, "Start of protected area", 0x96), \
	X(EPA, "End of protected area", 0x97), \
	X(SOS, "Start of string", 0x98), \
	X(SGCI, "Single graphic character introducer", 0x99), \
	X(SCI, "Single character intro introducer", 0x9A), \
	X(CSI, "Control sequence introducer", 0x9B), \
	X(ST, "String terminator", 0x9C), \
	X(OSC, "Operating system command", 0x9D), \
	X(PM, "Private message", 0x9E), \
	X(APC, "Applied program command", 0x9F),

enum c0_ctrl {
#define X(short_name, long_name, code) C0_##short_name  = code
	C0_CTRL_CHARS
#undef X
};

enum c1_ctrl {
#define X(short_name, long_name, code) C1_##short_name  = code
	C1_CTRL_CHARS
#undef X
};

const char *ctrl_get_long_name(int code);
const char *ctrl_get_short_name(int code);

#endif
