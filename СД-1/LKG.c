#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


// Структура для хранения аргументов команды get_c
typedef struct Get_С_argument {
	unsigned long long cmin;
	unsigned long long cmax;
	unsigned long long m;
	int valid;  // Флаг, что все аргументы найдены
} Get_С_argument;

// Нахождение НОД (Алгоритм Евклида) для функции get_c
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
	if (n == 2) return 1;      // 2 - простое
	if (n % 2 == 0) return 0; // четные > 2 - не простые

	int limit = (unsigned long long)sqrt(n);
	for (int i = 3; i <= limit; i += 2) { // только нечетные делители
		if (n % i == 0) {
			return 0;
		}
	}
	return 1;
}