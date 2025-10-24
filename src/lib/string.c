#include "globals.h"

void dec2str(int32 n, char *d) {
	int i = 0;
	if (n < 0) {
		d[i++] = '-';
		n = -n;
	}
	int start = i;
	do {
		d[i++] = '0' + (n % 10);
		n /= 10;
	} while (n);
	d[i] = '\0';
	for (int a = start, b = i - 1; a < b; a++, b--) {
		char c = d[a];
		d[a] = d[b];
		d[b] = c;
	}
}

const char n_to_ascii[] = "0123456789abcdef";

void hex2str(nat32 n, char *d) {
	int i = 0;
	do {
		d[i++] = n_to_ascii[n & 0xf];
		n >>= 4;
	} while (n);
	d[i++] = 'x';
	d[i++] = '0';
	d[i] = 0;
	for (int a = 0, b = i - 1; a < b; a++, b--) {
		char c = d[a];
		d[a] = d[b];
		d[b] = c;
	}
}

nat32 strlen(const char *s) {
	nat32 i = 0;
	while (s[i]) i++;
	return i;
}

int32 strcmp(const char *a, const char *b) {
	while (*a && *a == *b) a++, b++;
	return (unsigned char)*a - (unsigned char)*b;
}

int32 strncmp(const char *a, const char *b, nat32 n) {
	for (nat32 i = 0; i < n; i++) {
		if (a[i] != b[i] || !a[i] || !b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
	}
	return 0;
}

void strcpy(char *d, const char *s) {
	while ((*d++ = *s++));
}

void strncpy(char *d, const char *s, nat32 n) {
	nat32 i = 0;
	for (; i < n && s[i]; i++) d[i] = s[i];
	if (i < n) d[i] = 0;
}

void strlower(char *s) {
	for (; *s; s++) if (*s >= 'A' && *s <= 'Z') *s += 32;
}

void snprintf(char *buf, nat32 size, const char *fmt, ...) {
	va_list v;
	va_start(v, fmt);
	nat32 i = 0;
	for (nat32 j = 0; fmt[j] && i < size - 1; j++) {
		if (fmt[j] == '%' && fmt[j+1]) {
			char t = fmt[++j];
			if (t == 's') {
				const char *s = va_arg(v, char*);
				while (*s && i < size - 1) buf[i++] = *s++;
			} else if (t == 'd') {
				char tmp[12];
				dec2str(va_arg(v,int), tmp);
				for (nat32 k = 0; tmp[k] && i < size - 1; k++) buf[i++] = tmp[k];
			} else if (t == 'x') {
				char tmp[12];
				hex2str(va_arg(v,nat32), tmp);
				for (nat32 k = 0; tmp[k] && i < size - 1; k++) buf[i++] = tmp[k];
			} else if (t == 'c') {
				buf[i++] = (char)va_arg(v,int);
			}
		} else buf[i++] = fmt[j];
	}
	buf[i] = 0;
	va_end(v);
}
