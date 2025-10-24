#ifndef MATH_H
#define MATH_H

typedef double f64;

static inline f64 pow(f64 x, int n) {
	f64 r = 1;
	for(int i = 0; i < n; i++)
		r *= x;
	return r;
}

static inline f64 factorial(int n) {
	f64 r = 1;
	for(int i = 2; i <= n; i++)
		r *= i;
	return r;
}

static inline f64 sin(f64 x) {
	f64 result = 0;
	int terms = 10;
	for(int n = 0; n < terms; n++) {
		int sign = (n % 2 == 0) ? 1 : -1;
		result += sign * pow(x, 2*n + 1) / factorial(2*n + 1);
	}
	return result;
}

static inline f64 cos(f64 x) {
	f64 result = 0;
	int terms = 10;
	for(int n = 0; n < terms; n++) {
		int sign = (n % 2 == 0) ? 1 : -1;
		result += sign * pow(x, 2*n) / factorial(2*n);
	}
	return result;
}

#endif
