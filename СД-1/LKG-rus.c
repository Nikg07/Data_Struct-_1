#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>

//=============================================================================
// СТРУКТУРЫ

// Структура для хранения аргументов команды get_c
typedef struct Get_C_argument {
    unsigned long long cmin;  // нижняя граница поиска
    unsigned long long cmax;  // верхняя граница поиска
    unsigned long long m;     // модуль
    int valid;                // флаг корректности аргументов
} Get_C_argument;

// Структура для результата get_c
typedef struct Get_C_result {
    unsigned long long* numbers;  // массив найденных чисел
    int count;                    // количество найденных чисел
    int error;                    // код ошибки (0 - нет ошибки)
} Get_C_result;

// Структура для хранения аргументов команды get_a
typedef struct Get_A_argument {
    unsigned long long m;  // модуль
    int valid;             // флаг корректности аргументов
} Get_A_argument;

// Структура для хранения аргументов команды lcg
typedef struct LCG_argument {
    unsigned long long a;   // множитель
    unsigned long long x0;  // начальное значение
    unsigned long long c;   // приращение
    unsigned long long m;   // модуль
    unsigned long long n;   // количество генерируемых чисел
    int valid;              // флаг корректности аргументов
} LCG_argument;

//=============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ


// Алгоритм Евклида для нахождения наибольшего общего делителя
unsigned long long nod(unsigned long long a, unsigned long long b) {
    unsigned long long ostatok = a % b;
    while (ostatok) {
        a = b;
        b = ostatok;
        ostatok = a % b;
    }
    return b;
}

// Проверка числа на простоту
int is_prostoe(unsigned long long n) {
    if (n < 2) return 0;          // числа меньше 2 не простые
    if (n == 2) return 1;         // 2 - простое число
    if (n % 2 == 0) return 0;     // четные числа больше 2 не простые

    // Проверяем нечетные делители до корня из n
    unsigned long long limit = (unsigned long long)sqrt(n);
    for (unsigned long long i = 3; i <= limit; i += 2) {
        if (n % i == 0) {
            return 0;  // найден делитель - число не простое
        }
    }
    return 1;  // делителей не найдено - число простое
}

// Нахождение всех простых делителей числа. Возвращает количество найденных делителей
int get_prime_factors(unsigned long long n, unsigned long long* factors, int max_factors) {
    int count = 0;
    unsigned long long temp_n = n;  // рабочая копия числа

    if (n <= 1) return 0;  // у чисел <= 1 нет простых делителей

    // Обработка делителя 2
    if (temp_n % 2 == 0) {
        if (count < max_factors) {
            factors[count++] = 2;  // 2 всегда простое число
        }
        // Удаляем все степени двойки
        while (temp_n % 2 == 0) {
            temp_n /= 2;
        }
    }

    // Проверка нечетных делителей от 3 до корня из числа
    for (unsigned long long i = 3; i * i <= temp_n; i += 2) {
        if (temp_n % i == 0) {
            // Если i - простое число, добавляем его в массив
            if (count < max_factors && is_prostoe(i)) {
                factors[count++] = i;
            }
            // Удаляем все степени i
            while (temp_n % i == 0) {
                temp_n /= i;
            }
        }
    }

    // Если осталось число больше 1, оно является простым делителем
    if (temp_n > 1) {
        if (count < max_factors && is_prostoe(temp_n)) {
            factors[count++] = temp_n;
        }
    }

    return count;
}

// Освобождение памяти, выделенной под результат get_c
void free_get_c_result(Get_C_result* result) {
    if (result != NULL && result->numbers != NULL) {
        free(result->numbers);
        result->numbers = NULL;
        result->count = 0;
        result->error = 0;
    }
}

//=============================================================================
// ФУНКЦИИ ПАРСИНГА КОМАНД


// Парсинг команды get_c из строки
Get_C_argument parse_get_c(char* command) {
    Get_C_argument args = { 0, 0, 0, 0 };
    char* token;
    char* next_token = NULL;

    // Пропускаем первое слово (get_c)
    token = strtok_s(command, " ", &next_token);
    if (token == NULL) return args;

    // Разбираем остальные аргументы вида ключ=значение
    while ((token = strtok_s(NULL, " ", &next_token)) != NULL) {
        char* equals_pos = strchr(token, '=');
        if (equals_pos == NULL) continue;

        // Разделяем строку на ключ и значение
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

        // Преобразуем строку в число
        unsigned long long value = 0;
        if (sscanf(value_str, "%llu", &value) != 1) {
            continue;
        }

        // Заполняем соответствующее поле структуры
        if (strcmp(key, "cmin") == 0) args.cmin = value;
        else if (strcmp(key, "cmax") == 0) args.cmax = value;
        else if (strcmp(key, "m") == 0) args.m = value;
    }

    // Проверка корректности всех аргументов
    if (args.cmin > 0 && args.cmax > 0 && args.m > 0) {
        if (args.cmin < args.m && args.cmax < args.m && args.cmin <= args.cmax) {
            args.valid = 1;  // все аргументы корректны
        }
    }

    return args;
}

// Парсинг команды get_a из строки
Get_A_argument parse_get_a(char* command) {
    Get_A_argument args = { 0, 0 };
    char* token;
    char* next_token = NULL;

    token = strtok_s(command, " ", &next_token);
    if (token == NULL) return args;

    while ((token = strtok_s(NULL, " ", &next_token)) != NULL) {
        char* equals_pos = strchr(token, '=');
        if (equals_pos == NULL) continue;

        *equals_pos = '\0';
        char* key = token;
        char* value_str = equals_pos + 1;

        // Проверка на цифры
        int is_digit = 1;
        for (char* p = value_str; *p != '\0'; p++) {
            if (*p < '0' || *p > '9') {
                is_digit = 0;
                break;
            }
        }
        if (!is_digit) continue;

        unsigned long long value = 0;
        if (sscanf(value_str, "%llu", &value) != 1) {
            continue;
        }

        if (strcmp(key, "m") == 0) args.m = value;
    }

    // Проверка корректности
    if (args.m > 0) args.valid = 1;
    return args;
}

// Парсинг команды lcg из строки
LCG_argument parse_lcg(char* command) {
    LCG_argument args = { 0, 0, 0, 0, 0, 0 };
    char* token;
    char* next_token = NULL;

    token = strtok_s(command, " ", &next_token);
    if (token == NULL) return args;

    while ((token = strtok_s(NULL, " ", &next_token)) != NULL) {
        char* equals_pos = strchr(token, '=');
        if (equals_pos == NULL) continue;

        *equals_pos = '\0';
        char* key = token;
        char* value_str = equals_pos + 1;

        // Проверка на цифры
        int is_digit = 1;
        for (char* p = value_str; *p != '\0'; p++) {
            if (*p < '0' || *p > '9') {
                is_digit = 0;
                break;
            }
        }
        if (!is_digit) continue;

        unsigned long long value = 0;
        if (sscanf(value_str, "%llu", &value) != 1) {
            continue;
        }

        // Заполняем соответствующее поле
        if (strcmp(key, "a") == 0) args.a = value;
        else if (strcmp(key, "x0") == 0) args.x0 = value;
        else if (strcmp(key, "c") == 0) args.c = value;
        else if (strcmp(key, "m") == 0) args.m = value;
        else if (strcmp(key, "n") == 0) args.n = value;
    }

    // Проверка, что все аргументы найдены
    // (значения могут быть 0, поэтому проверяем только наличие)
    if (args.a > 0 || args.a == 0) {
        if (args.x0 > 0 || args.x0 == 0) {
            if (args.c > 0 || args.c == 0) {
                if (args.m > 0) {  // m должно быть строго больше 0
                    if (args.n > 0 || args.n == 0) {
                        args.valid = 1;
                    }
                }
            }
        }
    }

    return args;
}

//=============================================================================
// ОСНОВНЫЕ ФУНКЦИИ

// Функция get_c - находит все c, взаимно простые с m в диапазоне [cmin, cmax]
Get_C_result get_c(unsigned long long cmin, unsigned long long cmax, unsigned long long m) {
    Get_C_result result = { NULL, 0, 0 };

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
        result.error = 3;  // cmin должно быть больше 0
        return result;
    }

    // Первый проход: подсчет количества подходящих чисел
    int estimated_count = 0;
    for (unsigned long long c = cmin; c <= cmax; c++) {
        if (c > 0 && c < m && nod(c, m) == 1) {
            estimated_count++;
        }
    }

    // Выделение памяти под массив результатов
    result.numbers = (unsigned long long*)malloc(estimated_count * sizeof(unsigned long long));
    if (result.numbers == NULL) {
        result.error = 4;  // Ошибка выделения памяти
        return result;
    }

    // Второй проход: заполнение массива
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

// Функция get_a - находит минимальное a, удовлетворяющее теореме о максимальном периоде
unsigned long long get_a(unsigned long long m) {
    if (m <= 1) return 0;  // нет решения для m <= 1

    // Получаем все простые делители m
    unsigned long long factors[100];
    int factor_count = get_prime_factors(m, factors, 100);

    if (factor_count == 0) return 0;  // нет простых делителей

    // Поиск минимального a, удовлетворяющего условиям
    for (unsigned long long a = 2; a < m; a++) {
        int conditions_met = 1;

        // Условие: (a-1) должно делиться на каждый простой делитель m
        for (int i = 0; i < factor_count; i++) {
            if ((a - 1) % factors[i] != 0) {
                conditions_met = 0;
                break;
            }
        }

        // Дополнительное условие: если m делится на 4, то (a-1) должно делиться на 4
        if (conditions_met && m % 4 == 0) {
            if ((a - 1) % 4 != 0) {
                conditions_met = 0;
            }
        }

        if (conditions_met) {
            return a;  // найдено минимальное подходящее a
        }
    }

    return 0;  // решение не найдено
}

// Функция lcg - генерирует последовательность псевдослучайных чисел
// Возвращает 1 при успехе, 0 при ошибке
int lcg(unsigned long long a, unsigned long long x0, unsigned long long c,
    unsigned long long m, unsigned long long n, unsigned long long* results) {

    // Проверка условий из задания
    if (n == 0) return 0;        // n=0 - нет решения
    if (a >= m) return 0;        // a >= m - нет решения
    if (c >= m) return 0;        // c >= m - нет решения
    if (x0 >= m) return 0;       // x0 >= m - нет решения

    // Генерация последовательности по формуле: Xn+1 = (a * Xn + c) mod m
    unsigned long long current = x0;

    for (unsigned long long i = 0; i < n; i++) {
        current = (a * current + c) % m;
        results[i] = current;
    }

    return 1;  // успешно
}

//=============================================================================
// ФУНКЦИИ ДЛЯ ЗАПИСИ РЕЗУЛЬТАТОВ В ФАЙЛ


// Запись результата get_c в файл
void write_get_c_result(FILE* output, Get_C_result result) {
    if (result.error != 0 || result.count == 0) {
        fprintf(output, "no solution\n");
        return;
    }

    // Вывод всех найденных чисел через пробел
    for (int i = 0; i < result.count; i++) {
        fprintf(output, "%llu", result.numbers[i]);
        if (i < result.count - 1) {
            fprintf(output, " ");
        }
    }
    fprintf(output, "\n");
}

// Запись результата get_a в файл
void write_get_a_result(FILE* output, unsigned long long a) {
    if (a == 0) {
        fprintf(output, "no solution\n");
    }
    else {
        fprintf(output, "%llu\n", a);
    }
}

// Запись результата lcg в файл
void write_lcg_result(FILE* output, unsigned long long* results, unsigned long long n, int success) {
    if (!success) {
        fprintf(output, "no solution\n");
        return;
    }

    // Вывод сгенерированных чисел через пробел
    for (unsigned long long i = 0; i < n; i++) {
        fprintf(output, "%llu", results[i]);
        if (i < n - 1) {
            fprintf(output, " ");
        }
    }
    fprintf(output, "\n");
}

//=============================================================================
int main() {
    SetConsoleOutputCP(1251);

    // Открытие входного файла
    FILE* input = fopen("input.txt", "r");
    if (input == NULL) {
        printf("Ошибка: не могу открыть input.txt\n");
        system("pause");
        return 1;
    }

    // Открытие выходного файла
    FILE* output = fopen("output.txt", "w");
    if (output == NULL) {
        printf("Ошибка: не могу открыть output.txt\n");
        fclose(input);
        system("pause");
        return 1;
    }

    // Чтение команды из файла
    char command[256] = { 0 };
    if (fgets(command, sizeof(command), input) == NULL) {
        printf("Ошибка: файл input.txt пуст\n");
        fprintf(output, "incorrect command\n");
        fclose(input);
        fclose(output);
        system("pause");
        return 1;
    }

    // Удаление символа перевода строки в конце
    size_t len = strlen(command);
    if (len > 0 && command[len - 1] == '\n') {
        command[len - 1] = '\0';
    }

    // Создание копии команды для парсинга (strtok_s изменяет строку)
    char command_copy[256];
    strcpy_s(command_copy, sizeof(command_copy), command);

    // Получение ключевого слова (первое слово в строке)
    char* next_token = NULL;
    char* keyword = strtok_s(command_copy, " ", &next_token);

    if (keyword == NULL) {
        fprintf(output, "incorrect command\n");
        fclose(input);
        fclose(output);
        system("pause");
        return 1;
    }

    // Обработка команды get_c
    if (strcmp(keyword, "get_c") == 0) {
        char args_copy[256];
        strcpy_s(args_copy, sizeof(args_copy), command);

        Get_C_argument args = parse_get_c(args_copy);

        if (args.valid) {
            Get_C_result result = get_c(args.cmin, args.cmax, args.m);
            write_get_c_result(output, result);
            free_get_c_result(&result);
        }
        else {
            fprintf(output, "incorrect command\n");
        }
    }
    // Обработка команды get_a
    else if (strcmp(keyword, "get_a") == 0) {
        char args_copy[256];
        strcpy_s(args_copy, sizeof(args_copy), command);

        Get_A_argument args = parse_get_a(args_copy);

        if (args.valid) {
            unsigned long long a = get_a(args.m);
            write_get_a_result(output, a);
        }
        else {
            fprintf(output, "incorrect command\n");
        }
    }
    // Обработка команды lcg
    else if (strcmp(keyword, "lcg") == 0) {
        char args_copy[256];
        strcpy_s(args_copy, sizeof(args_copy), command);

        LCG_argument args = parse_lcg(args_copy);

        if (args.valid) {
            // Выделение памяти под массив результатов
            unsigned long long* results = (unsigned long long*)malloc(args.n * sizeof(unsigned long long));
            if (results == NULL) {
                fprintf(output, "no solution\n");
            }
            else {
                int success = lcg(args.a, args.x0, args.c, args.m, args.n, results);
                write_lcg_result(output, results, args.n, success);
                free(results);
            }
        }
        else {
            fprintf(output, "incorrect command\n");
        }
    }
    // Неизвестная команда
    else {
        fprintf(output, "incorrect command\n");
    }

    // Закрытие файлов
    fclose(input);
    fclose(output);

    return 0;
}