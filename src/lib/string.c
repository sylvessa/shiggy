#include "globals.h"
#include "lib/string.h"

void dec2str(int32 n, char *des) {
	int len;
	if (n < 0) {
		des[0] = '-';
		n = -n;
		len = 1;
	} else {
		len = 0;
	}
	do {
		des[len++] = (char)('0' + (n % 10));
		n /= 10;
	} while (n != 0);
	des[len] = '\0';
	if (des[0] == '-')
		str_reverse(des + 1);
	else
		str_reverse(des);
}

const char n_to_ascii[] = "0123456789abcdef";

void hex2str(nat32 n, char *des) {
    nat32 len = 0;

    do {
        des[len++] = n_to_ascii[n & 0xf];
        n >>= 4;
    } while (n != 0);

    des[len++] = 'x';
    des[len++] = '0';
    des[len] = '\0';

    str_reverse(des);
}

void str_reverse(char *s) {
    char c;
    for (int i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

nat32 strlen(const char *s) {
	nat32 i = 0;
	while (s[i] != '\0') ++i;
	return i;
}

int32 strcmp(char *s1, char *s2) {
    int32 i;

    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0')
            return 0;
    }

    return s1[i] - s2[i];
}

void strcpy(char *des, const char *src) {
    while (*src != '\0') {
        *des = *src;
        des++;
        src++;
    }

    *des = '\0';
}

void strlower(char *s) {
	for (int i = 0; s[i] != '\0'; i++) {
		if (s[i] >= 'A' && s[i] <= 'Z')
			s[i] += 32;
	}
}

void strncpy(char *des, const char *src, nat32 n) {
    nat32 i;
    for (i = 0; i < n - 1 && src[i] != '\0'; i++)
        des[i] = src[i];
    des[i] = '\0';
}

int32 strncmp(const char *s1, const char *s2, nat32 n) {
    for (nat32 i = 0; i < n; i++) {
        if (s1[i] != s2[i] || s1[i] == '\0' || s2[i] == '\0')
            return (unsigned char)s1[i] - (unsigned char)s2[i];
    }
    return 0;
}

void snprintf(char *buf, nat32 size, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	if (size == 0) {
		va_end(args);
		return;
	}
	nat32 i = 0;
	for (nat32 j = 0; fmt[j] != '\0' && i < size - 1; j++) {
		if (fmt[j] == '%' && fmt[j + 1] != '\0') {
			j++;
			if (fmt[j] == 's') {
				const char *s = va_arg(args, const char*);
				while (*s && i < size - 1) buf[i++] = *s++;
			} else if (fmt[j] == 'd') {
				char tmp[20];
				int v = va_arg(args, int);
				dec2str(v, tmp);
				for (nat32 k = 0; tmp[k] && i < size - 1; k++) buf[i++] = tmp[k];
			} else if (fmt[j] == 'x') {
				char tmp[20];
				unsigned int v = va_arg(args, unsigned int);
				hex2str((nat32)v, tmp);
				for (nat32 k = 0; tmp[k] && i < size - 1; k++) buf[i++] = tmp[k];
			} else if (fmt[j] == 'c') {
				int c = va_arg(args, int);
				if (i < size - 1) buf[i++] = (char)c;
			} else {
				if (i < size - 1) buf[i++] = '%';
				if (i < size - 1) buf[i++] = fmt[j];
			}
		} else {
			buf[i++] = fmt[j];
		}
	}
	buf[i] = '\0';
	va_end(args);
}
