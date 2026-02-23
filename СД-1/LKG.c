#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>


// Структура для хранения аргументов команды get_c
typedef struct Get_С_argument {
	unsigned long long cmin;
	unsigned long long cmax;
	unsigned long long m;
	int valid;  // Флаг, что все аргументы найдены
} Get_C_argument;
// Структура для результата get_c
typedef struct Get_C_result {
    unsigned long long* numbers;  // массив найденных чисел
    int count;                    // количество найденных чисел
    int error;                    // флаг ошибки (0 - нет ошибки, 1 - ошибка)
} Get_C_result;
// Структура для хранения аргументов команды get_a
typedef struct Get_A_argument {
    unsigned long long m;
    int valid;  // Флаг, что все аргументы найдены
} Get_A_argument;

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

// Функция для нахождения всех простых делителей числа, возвращает количество
int get_prime_factors(unsigned long long n, unsigned long long* factors) {
    int count = 0;
    unsigned long long temp_n = n;

    if (n <= 1) return 0;

    // Проверка делителя 2
    if (temp_n %2 == 0) {
        factors[count++] = 2; // Добавили 2 так как оно точно простое
        while (temp_n % 2 == 0) {
            temp_n /= 2; // Убрали все степени двойки так как они не простые
        }
    }

    // Перебор нечетных делителей от 3 до корня от n
    for (unsigned long long i = 3; i * i <= temp_n; i += 2) {
        if (temp_n % i == 0) {
            if (is_prostoe(i)) {
                factors[count++] = i;
            }
            while (temp_n % i == 0) {
                temp_n /= i; // Убирали все степени i
            }
        }
    }

    // Если остаток - простое число
    if (temp_n > 1) {
        if (is_prostoe(temp_n)) {
            factors[count++] = temp_n;
        }
        
    }

    return count;
}
// функция для освобождения структуры с результатми get_c
void free_get_c_result(Get_C_result* result) {
    if (result != NULL && result->numbers != NULL) {
        free(result->numbers);
        result->numbers = NULL;
        result->count = 0;
        result->error = 0;
    }
}
// Парсинг get_c из строки
Get_C_argument parse_get_c(char* command) {
Get_C_argument args = { 0, 0, 0, 0 };
char* token;
char* next_token = NULL;

// Пропускаем первое слово (get_c)
token = strtok_s(command, " ", &next_token);
if (token == NULL) return args;

// Разбираем оставшиеся аргументы
while ((token = strtok_s(NULL, " ", &next_token)) != NULL) {
    // Ищем знак равно
    char* equals_pos = strchr(token, '=');
    if (equals_pos == NULL) continue;

    // Разделяем на ключ и значение
    *equals_pos = '\0';
    char* key = token;
    char* value_str = equals_pos + 1;

    // Проверяем, что значение состоит только из цифр
    int is_digit = 1;
    for (char* p = value_str; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            is_digit = 0;
            break;
        }
    }
    if (!is_digit) continue;

    // Преобразуем значение в число
    unsigned long long value = 0;
    if (sscanf(value_str, "%llu", &value) != 1) {
        continue;  // Не удалось преобразовать
    }
    // Заполняем структуру
    if (strcmp(key, "cmin") == 0) {
        args.cmin = value;
    }
    else if (strcmp(key, "cmax") == 0) {
        args.cmax = value;
    }
    else if (strcmp(key, "m") == 0) {
        args.m = value;
    }
}

// Проверяем, что все аргументы заданы и корректны
if (args.cmin > 0 && args.cmax > 0 && args.m > 0) {
    if (args.cmin < args.m && args.cmax < args.m && args.cmin <= args.cmax) {
        args.valid = 1;
    }
}

return args;
}

// Парсинг команды get_a из строки
Get_A_argument parse_get_a(char* command) {
    Get_A_argument args = { 0, 0 };
    char* token;
    char* next_token = NULL;

    // Пропускаем первое слово (get_a)
    token = strtok_s(command, " ", &next_token);
    if (token == NULL) return args;

    // Разбираем оставшиеся аргументы
    while ((token = strtok_s(NULL, " ", &next_token)) != NULL) {
        char* equals_pos = strchr(token, '=');
        if (equals_pos == NULL) continue;

        *equals_pos = '\0';
        char* key = token;
        char* value_str = equals_pos + 1;

        // Проверяем, что значение состоит только из цифр
        int is_digit = 1;
        for (char* p = value_str; *p != '\0'; p++) {
            if (*p < '0' || *p > '9') {
                is_digit = 0;
                break;
            }
        }
        if (!is_digit) continue;

        // Преобразуем значение в число
        unsigned long long value = 0;
        if (sscanf(value_str, "%llu", &value) != 1) {
            continue;  // Не удалось преобразовать
        }

        if (strcmp(key, "m") == 0) {
            args.m = value;
        }
    }

    // Проверяем корректность
    if (args.m > 0) {
        args.valid = 1;
    }

    return args;
}




// Функция get_c - возвращает структуру с массивом всех c, взаимно простых с m
Get_C_result get_c(unsigned long long cmin, unsigned long long cmax, unsigned long long m) {
    Get_C_result result = { NULL,0,0 };

    // Проверка корректности входных данных
    if (cmin >= m || cmax >= m) {
        result.error = 1;  // cmin или cmax больше или равны m
        return result;
    }

    if (cmin > cmax) {
        result.error = 2;  // cmin больше cmax
        return result;
    }

    if (cmin == 0) {
        result.error = 3;  // cmin должно быть > 0
        return result;
    }

    // Сначала нужно узнать, сколько будет чисел, чтобы выделить память
    int estimated_count = 0;
    for (unsigned long long c = cmin; c <= cmax; c++) {
        if (c > 0 && c < m && nod(c, m) == 1) {
            estimated_count++;
        }
    }

    // Выделяем память под массив
    result.numbers = (unsigned long long*)malloc(estimated_count * sizeof(unsigned long long));
    if (result.numbers == NULL) {
        result.error = 4;  // Ошибка выделения памяти
        return result;
    }

    // Заполняем массив
    int index = 0;
    for (unsigned long long c = cmin; c <= cmax; c++) {
        if (c > 0 && c < m && nod(c, m) == 1) {
            result.numbers[index] = c;
            index++;
        }
    }

    result.count = index;
    return result;
}



// Функция get_a
unsigned long long get_a(unsigned long long m) {

    // Проверка корректности
    if (m <= 1) {
        return 0;  // Нет решения
    }

    // Находим все простые делители m
    unsigned long long factors[100];
    int factor_count = get_prime_factors(m, factors);

    if (factor_count == 0) {
        return 0;
    }

    // Ищем минимальное a
    for (unsigned long long a = 2; a < m; a++) {
        int conditions_met = 1;

        // a-1 должно делиться на каждый простой делитель m
        for (int i = 0; i < factor_count; i++) {
            if ((a - 1) % factors[i] != 0) {
                conditions_met = 0;
                break;
            }
        }

        //  если m делится на 4, то a-1 должно делиться на 4
        if (conditions_met && m % 4 == 0) {
            if ((a - 1) % 4 != 0) {
                conditions_met = 0;
            }
        }

        if (conditions_met) {
            return a;
        }
    }

    return 0;
}

// Функция для вывода результата get_c
void print_get_c_result(Get_C_result result) {
    if (result.error != 0) {
        printf("Ошибка в get_c: ");
        switch (result.error) {
        case 1:
            printf("cmin или cmax больше или равны m\n");
            break;
        case 2:
            printf("cmin больше cmax\n");
            break;
        case 3:
            printf("cmin должно быть больше 0\n");
            break;
        case 4:
            printf("Ошибка выделения памяти\n");
            break;
        default:
            printf("Неизвестная ошибка\n");
        }
        return;
    }

    if (result.count == 0) {
        printf("Не найдено чисел, удовлетворяющих условиям\n");
        return;
    }

    printf("Найдено %d чисел:\n", result.count);
    printf("Массив чисел: [");
    for (int i = 0; i < result.count; i++) {
        printf("%llu", result.numbers[i]);
        if (i < result.count - 1) {
            printf(", ");
        }
    }
    printf("]\n");

    
}

int main() {
    SetConsoleOutputCP(1251);
    return 0;
}
