#include "globals.h"
#include "lib/math.h"

f64 pow(f64 x, int n) {
	f64 r = 1;
	for(int i = 0; i < n; i++)
		r *= x;
	return r;
}

f64 factorial(int n) {
	f64 r = 1;
	for(int i = 2; i <= n; i++)
		r *= i;
	return r;
}

f64 sin(f64 x) {
	f64 result = 0;
	int terms = 10;
	for(int n = 0; n < terms; n++) {
		int sign = (n % 2 == 0) ? 1 : -1;
		result += sign * pow(x, 2*n + 1) / factorial(2*n + 1);
	}
	return result;
}

f64 cos(f64 x) {
	f64 result = 0;
	int terms = 10;
	for(int n = 0; n < terms; n++) {
		int sign = (n % 2 == 0) ? 1 : -1;
		result += sign * pow(x, 2*n) / factorial(2*n);
	}
	return result;
}

f64 atof(const char *s) {
	while(*s == ' ' || *s == '\t') s++;

	int neg = 0;
	if(*s == '-') {
		neg = 1;
		s++;
	} else if(*s == '+') {
		s++;
	}

	f64 val = 0;
	while(*s >= '0' && *s <= '9') {
		val = val * 10 + (*s - '0');
		s++;
	}

	if(*s == '.') {
		s++;
		f64 frac = 1;
		while(*s >= '0' && *s <= '9') {
			frac *= 0.1;
			val += (*s - '0') * frac;
			s++;
		}
	}

	return neg ? -val : val;
}
