#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


typedef struct Get_argument {
	unsigned long long cmin;
	unsigned long long cmax;
	unsigned long long m;
	int valid;
} Get_argument;


unsigned long long nod(unsigned long long a, unsigned long long b) {
	unsigned long long ostatok = a % b;
	while (ostatok) {
		a = b;
		b = ostatok;
		ostatok = a % b;
	}
	return b;
}

// Проверка на простоту для get_a
int is_prostoe(unsigned long long n) {
	if (n < 2) return 0;
	if (n == 2) return 1;
	if (n % 2 == 0) return 0;

	int limit = (int)sqrt(n);
	for (int i = 3; i <= limit; i += 2) {
		if (n % i == 0) {
			return 0;
		}
	}
	return 1;
}
