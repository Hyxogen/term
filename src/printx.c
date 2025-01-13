#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#ifndef strnlen
size_t strnlen(const char *str, size_t maxlen)
{
	const char *tmp = str;

	while (maxlen-- && *tmp)
		++tmp;
	return tmp - str;

}
#endif

static int write_signed(char *dest, intmax_t v)
{
	int val = v % 10;
	int nwritten = 0;

	if ((v / 10) != 0)
		nwritten = write_signed(dest, v / 10);

	if (v < 0)
		val = -val;
	dest[nwritten++] = '0' + val;

	return nwritten;
}

static int write_unsigned(char *dest, uintmax_t v, const char *base, int radix)
{
	int nwritten = 0;
	if (v / radix)
		nwritten = write_unsigned(dest, v / radix, base, radix);

	dest[nwritten++] = base[v % radix];
	return nwritten;
}

static int write(const char *str, size_t len, int (*put)(int c, void *),
		 void *opaque)
{
	int nwritten = 0;

	for (size_t i = 0; i < len; ++i) {
		int ret = put(str[i], opaque);
		if (ret < 0)
			return ret;
		nwritten += 1;
	}
	return nwritten;
}

static int pad(int ch, size_t n, int (*put)(int c, void *), void *opaque)
{
	int nwritten = 0;

	while (n--) {
		int ret = put(ch, opaque);
		if (ret < 0)
			return -1;
		nwritten += ret;
	}
	return nwritten;
}

static int write_num(const char *num, int numlen, const char *prefix,
		     size_t prefixlen, char padding, int minus, int width,
		     int prec, int (*put)(int c, void *), void *opaque)
{
	int nwritten = 0;
	int totallen = numlen + prefixlen;

	if (prec > 0 && numlen < prec)
		totallen += prec - numlen;
	totallen += prefixlen;

	int ret;
	if (width > 0 && !minus && totallen < width) {
		ret = pad(padding, width - totallen, put, opaque);
		if (ret < 0)
			goto write_error;
	}

	ret = write(prefix, prefixlen, put, opaque);
	if (ret < 0)
		goto write_error;

	if (prec > 0 && numlen < prec) {
		ret = pad('0', prec - numlen, put, opaque);
		if (ret < 0)
			goto write_error;
	}

	ret = write(num, numlen, put, opaque);
	if (ret < 0)
		goto write_error;

	if (width > 0 && minus && totallen < width) {
		ret = pad(padding, width - totallen, put, opaque);
		if (ret < 0)
			goto write_error;
	}

	return nwritten;
write_error:
	return -1;
}

int vprintx(int (*put)(int, void *), void *opaque, const char *fmt, va_list ap)
{
	int nwritten = 0;
	while (*fmt) {
		if (*fmt != '%') {
			int rc = put(*fmt++, opaque);
			if (rc < 0)
				goto write_error;

			nwritten += rc;
			continue;
		}

		fmt += 1;

		int hash = 0;
		int zero = 0;
		int minus = 0;
		int space = 0;
		int plus = 0;

		while (1) {
			int flag = *fmt;

			if (flag == '#')
				hash = 1;
			else if (flag == '0')
				zero = 1;
			else if (flag == '-')
				minus = 1;
			else if (flag == '+')
				plus = 1;
			else
				break;

			fmt += 1;
		}

		if (zero && minus)
			zero = 0;

		int width = -1;

		if (*fmt == '*') {
			width = va_arg(ap, int);

			if (width < 0) {
				minus = 1;
				width = -width;
			}

			fmt += 1;
		} else if (isdigit(*fmt)) {
			width = 0;
			while (isdigit(*fmt)) {
				width = width * 10 + (*fmt - '0');
				fmt += 1;
			}
		}

		int prec = -1;
		if (*fmt == '.') {
			fmt += 1;

			if (*fmt == '*') {
				prec = va_arg(ap, int);
				fmt += 1;
			} else {

				if (*fmt == '-') {
					fmt += 1;

				} else {
					prec = 0;

					while (isdigit(*fmt)) {
						prec = prec * 10 + (*fmt - '0');
						fmt += 1;
					}
				}
			}
		}

		char size = 0;

		if (strncmp(fmt, "hh", 2) == 0) {
			size = 'H';
			fmt += 2;
		} else if (strncmp(fmt, "h", 1) == 0) {
			size = 'h';
			fmt += 1;
		} else if (strncmp(fmt, "ll", 2) == 0) {
			size = 'L';
			fmt += 2;
		} else if (strncmp(fmt, "l", 1) == 0) {
			size = 'l';
			fmt += 1;
		} else if (strncmp(fmt, "z", 1) == 0) {
			size = 'z';
			fmt += 1;
		}

		char buf[32];
		int padding = zero ? '0' : ' ';

		switch (*fmt) {
		case 'd':
		case 'i': {
			intmax_t v;

			if (size == 'L')
				v = va_arg(ap, long long);
			else if (size == 'l')
				v = va_arg(ap, long);
			else
				v = va_arg(ap, int);

			int numlen = write_signed(buf, v);

			const char *prefix = "";
			if (v < 0)
				prefix = "-";
			else if (plus)
				prefix = "+";
			else if (space)
				prefix = " ";

			int prefixlen = v < 0 || plus || space;
			int ret =
			    write_num(buf, numlen, prefix, prefixlen, padding,
				      minus, width, prec, put, opaque);
			if (ret < 0)
				goto write_error;
			nwritten += ret;
			break;
		}
		case 'o':
		case 'u':
		case 'x':
		case 'X':
		case 'p': {
			uintmax_t v;

			if (*fmt == 'p') {
				v = (uintmax_t)(uintptr_t)va_arg(ap, void *);
				hash = 1;
			} else if (size == 'z') {
				v = va_arg(ap, size_t);
			} else if (size == 'L') {
				v = va_arg(ap, unsigned long long);
			} else if (size == 'l') {
				v = va_arg(ap, unsigned long);
			} else {
				v = va_arg(ap, unsigned int);
			}

			int radix = 10;
			const char *base = "0123456789abcdef";
			const char *prefix = "";

			if (*fmt == 'o') {
				radix = 8;
				if (hash)
					prefix = "0";
			} else if (*fmt == 'x' || *fmt == 'p') {
				radix = 16;
				if (hash)
					prefix = "0x";
			} else if (*fmt == 'X') {
				radix = 16;
				base = "0123456789ABCDEF";
				if (hash)
					prefix = "0X";
			}

			int numlen = write_unsigned(buf, v, base, radix);
			int ret =
			    write_num(buf, numlen, prefix, strlen(prefix),
				      padding, minus, width, prec, put, opaque);
			if (ret < 0)
				goto write_error;
			nwritten += ret;
			break;
		}
		case 's': {
			const char *s = va_arg(ap, const char *);
			size_t len = prec < 0 ? strlen(s) : strnlen(s, prec);
			size_t padlen = (width > 0 && len < (size_t)width)
					    ? width - len
					    : 0;

			int ret;
			if (!minus) {
				ret = pad(padding, padlen, put, opaque);
				if (ret < 0)
					goto write_error;
				nwritten += ret;
			}

			ret = write(s, len, put, opaque);
			if (ret < 0)
				goto write_error;
			nwritten += ret;

			if (minus) {
				ret = pad(padding, padlen, put, opaque);
				if (ret < 0)
					goto write_error;
				nwritten += ret;
			}

			break;
		}
		case 'c': {
			int ch = va_arg(ap, int);
			int padlen = width > 0 ? width - 1 : 0;

			int ret;
			if (!minus) {
				ret = pad(padding, padlen, put, opaque);
				if (ret < 0)
					goto write_error;
				nwritten += ret;
			}

			ret = put(ch, opaque);
			if (ret < 0)
				goto write_error;
			nwritten += ret;

			if (minus) {
				ret = pad(padding, padlen, put, opaque);
				if (ret < 0)
					goto write_error;
				nwritten += ret;
			}
			break;
		}
		case '%': {
			int ret = put('%', opaque);
			if (ret < 0)
				goto write_error;
			break;
		}
		}

		fmt += 1;
	}

	return nwritten;
write_error:
	return -1;
}

int printx(int (*put)(int c, void *), void *opaque, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	int res = vprintx(put, opaque,fmt, args);

	va_end(args);
	return res;
}
