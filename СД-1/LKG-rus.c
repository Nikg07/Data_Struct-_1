#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Переносимость: для Windows используем strtok_s как strtok_r
#ifdef _WIN32
#include <windows.h>
#define strtok_r strtok_s
#else
    // На Unix-подобных системах strtok_r доступен в string.h
#endif

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

// Структура для хранения типов покерных комбинаций
typedef enum {
    ZERO,
    PAIR,
    TWO_PAIRS,
    SET,
    FULL_HOUSE,
    QUADS,
    POKER
} Type_of_hand;

// Структура для результата поиска периода
typedef struct {
    unsigned long long period;      // найденный период
    unsigned long long lambda;      // длина цикла
    unsigned long long mu;          // длина предпериода (сколько шагов до входа в цикл)
    int found;                      // найден ли период (1 - да, 0 - нет)
} PeriodResult;

//=============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ

// Алгоритм Евклида для нахождения наибольшего общего делителя
unsigned long long nod(unsigned long long a, unsigned long long b) {
    unsigned long long remains = a % b;
    while (remains) {
        a = b;
        b = remains;
        remains = a % b;
    }
    return b;
}




// Проверка числа на простоту
int is_prime(unsigned long long n) {
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
int get_prime_divider(unsigned long long n, unsigned long long* factors, int max_factors) {
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
            if (count < max_factors && is_prime(i)) {
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
        if (count < max_factors && is_prime(temp_n)) {
            factors[count++] = temp_n;
        }
    }

    return count;
}





// Компаратор для сортировки unsigned long long по возрастанию
int cmp_ull(const void* a, const void* b) {
    unsigned long long aa = *(unsigned long long*)a;
    unsigned long long bb = *(unsigned long long*)b;
    if (aa < bb) return -1;
    if (aa > bb) return 1;
    return 0;
}





// Компаратор для сортировки int по убыванию
int cmp_int_desc(const void* a, const void* b) {
    int aa = *(int*)a;
    int bb = *(int*)b;
    return bb - aa;
}




// Определение типа руки
Type_of_hand hand_type(unsigned long long hand[5]) {
    unsigned long long sorted[5];
    for (int i = 0; i < 5; i++) {
        sorted[i] = hand[i];
    }

    // Сортировка руки для простоты поиска одинаковых карт
    qsort(sorted, 5, sizeof(unsigned long long), cmp_ull);

    int counts[5] = { 0 };
    int idx = 0;
    unsigned long long current = sorted[0];
    int cnt = 1;

    // Поиск одинаковых чисел в отсортированной руке
    for (int i = 1; i < 5; i++) {
        if (sorted[i] == current) {
            cnt++;// одинаковое число
        }
        else {
            counts[idx++] = cnt; // сохранияем кол-во предыдущих
            current = sorted[i]; // следующее число
            cnt = 1;// сброс счетчика
        }
    }

    counts[idx++] = cnt; // сохранение последнего рукава

    qsort(counts, idx, sizeof(int), cmp_int_desc);
    // определение типа комбинации по паттерну
    if (idx == 1) {
        return POKER; // все 5 одинаковые
    }
    if (idx == 2) {
        return (counts[0] == 4) ? QUADS : FULL_HOUSE;// 4+1 или 3+2
    }
    if (idx == 3) {
        return (counts[0] == 3) ? SET : TWO_PAIRS;// 3+1+1 или 2+2+1
    }
    if (idx == 4) {
        return PAIR;// 2+1+1+1
    }
    if (idx == 5) {
        return ZERO;// все разные
    }

    return -1;
}




// Подсчет количества комбинаций
int count_observed(unsigned long long* numbers, int total, int observed[7]) {
    // Инициализируем массив типов руки нулями
    for (int i = 0; i < 7; i++) {
        observed[i] = 0;
    }

    // Количество полных рук
    int num_hands = total / 5;
    if (num_hands == 0) return 0;

    // Обрабатываем каждую руку формируя массив из 5
    for (int h = 0; h < num_hands; h++) {
        unsigned long long hand[5];
        for (int i = 0; i < 5; i++) {
            hand[i] = numbers[h * 5 + i];
        }

        Type_of_hand type = hand_type(hand);

        observed[type]++;
    }

    return num_hands;
}




// Определение приблизительного количества вариантов чисел
unsigned long long find_range(unsigned long long* numbers, int count,
    unsigned long long* min_val, unsigned long long* max_val) {

    if (count == 0) return 0;

    for (int i = 1; i < count; i++) {
        if (numbers[i] < *min_val) *min_val = numbers[i];
        if (numbers[i] > *max_val) *max_val = numbers[i];
    }

    unsigned long long M = *max_val - *min_val + 1;
    return M;
}


// вычисление ожидаемых частот для покер-теста
void compute_expected(unsigned long long M, int num_hands, double expected[7]) {
    // Общее количество возможных упорядоченных пятёрок
    double total_outcomes = pow((double)M, 5.0);

    // Для удобства вычислим все необходимые биномиальные коэффициенты
    double C_M_5, C_M_4, C_M_3, C_M_2, C_M_1;
    double C_Mminus1_3, C_Mminus1_2, C_Mminus2_1;

    // Вычисляем сочетания (используем формулы, чтобы избежать переполнения)
    if (M >= 5) {
        C_M_5 = (double)M * (M - 1) * (M - 2) * (M - 3) * (M - 4) / 120.0;
    }
    else {
        C_M_5 = 0.0;
    }

    // C(M,4) = M*(M-1)*(M-2)*(M-3)/24
    if (M >= 4) {
        C_M_4 = (double)M * (M - 1) * (M - 2) * (M - 3) / 24.0;
    }
    else {
        C_M_4 = 0.0;
    }

    // C(M,3) = M*(M-1)*(M-2)/6
    if (M >= 3) {
        C_M_3 = (double)M * (M - 1) * (M - 2) / 6.0;
    }
    else {
        C_M_3 = 0.0;
    }

    // C(M,2) = M*(M-1)/2
    if (M >= 2) {
        C_M_2 = (double)M * (M - 1) / 2.0;
    }
    else {
        C_M_2 = 0.0;
    }

    // C(M,1) = M
    C_M_1 = (double)M;

    // C(M-1,3)
    if (M - 1 >= 3) {
        C_Mminus1_3 = (double)(M - 1) * (M - 2) * (M - 3) / 6.0;
    }
    else {
        C_Mminus1_3 = 0.0;
    }

    // C(M-1,2)
    if (M - 1 >= 2) {
        C_Mminus1_2 = (double)(M - 1) * (M - 2) / 2.0;
    }
    else {
        C_Mminus1_2 = 0.0;
    }

    // C(M-2,1) = M-2, только для версий M >= 3
    if (M >= 3) {
        C_Mminus2_1 = (double)(M - 2);
    }
    else {
        C_Mminus2_1 = 0.0;
    }

    // Количество исходов для каждого типа
    double counts[7];

    // zero
    counts[0] = C_M_5 * 120.0;  // 5! = 120

    // pair
    counts[1] = C_M_1 * C_Mminus1_3 * (120.0 / 2.0); // 5!/2! = 120/2 = 60

    // two pairs
    counts[2] = C_M_2 * C_Mminus2_1 * (120.0 / (2.0 * 2.0)); // 5!/(2!2!) = 120/4 = 30

    // set
    counts[3] = C_M_1 * C_Mminus1_2 * (120.0 / 6.0); // 5!/3! = 120/6 = 20

    // full-house
    counts[4] = C_M_1 * (M - 1) * (120.0 / (6.0 * 2.0)); // 5!/(3!2!) = 120/(6*2)=120/12=10

    // quads
    counts[5] = C_M_1 * (M - 1) * (120.0 / 24.0); // 5!/4! = 120/24 = 5

    // poker
    counts[6] = C_M_1;  // 5!/5! = 1

    // Вычисление вероятности и ожидаемых частотот
    for (int i = 0; i < 7; i++) {
        double prob = counts[i] / total_outcomes;
        expected[i] = prob * num_hands;
    }
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
// ОПРЕДЕЛЕНИЯ ПЕРИОДА ПОСЛЕДОВАТЕЛЬНОСТИ



// Алгоритм Флойда для поиска цикла в последовательности
PeriodResult find_sequence_period(unsigned long long* numbers, int count) {

    PeriodResult result = { 0, 0, 0, 0 };

    if (count < 2) {
        return result;
    }

    // Шаг 1: Поиск точки встречи (алгоритм "черепаха и заяц")
    int tortoise = 0;
    int hare = 1;
    int steps = 0;
    int max_steps = count;

    // Ищем первое совпадение значений
    while (tortoise < count && hare < count && steps < max_steps) {
        if (numbers[tortoise] == numbers[hare]) {
            // Нашли потенциальное начало цикла
            break;
        }
        tortoise++;
        hare += 2;
        steps++;
    }

    if (tortoise >= count || hare >= count || numbers[tortoise] != numbers[hare]) {
        // Пробуем другой подход - ищем повтор первого элемента
        for (int i = 1; i < count; i++) {
            if (numbers[i] == numbers[0]) {
                // Нашли повтор первого элемента - возможно это начало цикла
                tortoise = 0;
                hare = i;
                break;
            }
        }
        if (tortoise != 0 || hare == 0) {
            return result;
        }
    }

    // Шаг 2: Нахождение длины предпериода (mu)
    int mu = 0;
    int ptr1 = 0;
    int ptr2 = hare;

    while (ptr1 < count && ptr2 < count && ptr1 < ptr2 && numbers[ptr1] == numbers[ptr2]) {
        ptr1++;
        ptr2++;
        mu++;
    }

    // Шаг 3: Нахождение длины цикла (lambda)
    int lambda = 1;
    int start = mu;
    int current = mu + 1;

    // Ищем, когда значение повторится
    while (current < count) {
        if (numbers[current] == numbers[start]) {
            // Проверяем, что это действительно цикл
            int is_cycle = 1;
            for (int k = 0; k < lambda && start + k < count && current + k < count; k++) {
                if (numbers[start + k] != numbers[current + k]) {
                    is_cycle = 0;
                    break;
                }
            }
            if (is_cycle) {
                break;
            }
        }
        current++;
        lambda++;
        if (current >= count) {
            lambda = 0;
            break;
        }
    }

    if (lambda > 0 && start + lambda <= count) {
        result.mu = mu;
        result.lambda = lambda;
        result.period = lambda;  // Период - это длина цикла
        result.found = 1;
    }

    return result;
}

//=============================================================================
// ФУНКЦИИ ПАРСИНГА КОМАНД

// Парсинг команды get_c из строки
Get_C_argument parse_get_c(char* command) {
    Get_C_argument args = { 0, 0, 0, 0 };
    char* token;
    char* saveptr = NULL;  // для strtok_r

    // Пропускаем первое слово (get_c)
    token = strtok_r(command, " ", &saveptr);
    if (token == NULL) return args;

    // Разбираем остальные аргументы вида ключ=значение
    while ((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
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
    char* saveptr = NULL;

    token = strtok_r(command, " ", &saveptr);
    if (token == NULL) return args;

    while ((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
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
    char* saveptr = NULL;

    token = strtok_r(command, " ", &saveptr);
    if (token == NULL) return args;

    while ((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
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
    if (args.a >= 0 && args.x0 >= 0 && args.c >= 0 && args.m > 0 && args.n >= 0) {
        args.valid = 1;
    }

    return args;
}




// Парсинг команды test с возвратом массива чисел
unsigned long long* parse_test(char* command, int* out_count) {
    char* cmd_copy = strdup(command);
    if (!cmd_copy) return NULL;

    char* saveptr;
    char* token = strtok_r(cmd_copy, " \t", &saveptr);
    if (!token) {
        free(cmd_copy);
        return NULL;
    }

    if (strcmp(token, "test") != 0) {
        free(cmd_copy);
        return NULL;
    }

    token = strtok_r(NULL, " \t", &saveptr);
    if (!token) {
        free(cmd_copy);
        return NULL;
    }

    char* equals_pos = strchr(token, '=');
    if (!equals_pos) {
        free(cmd_copy);
        return NULL;
    }

    *equals_pos = '\0';
    char* key = token;
    char* test_file_name = equals_pos + 1;

    if (strcmp(key, "inp") != 0) {
        free(cmd_copy);
        return NULL;
    }

    FILE* test_file = fopen(test_file_name, "r");
    if (!test_file) {
        free(cmd_copy);
        return NULL;
    }

    int capacity = 10;
    unsigned long long* num_arr = (unsigned long long*)malloc(capacity * sizeof(unsigned long long));
    if (!num_arr) {
        fclose(test_file);
        free(cmd_copy);
        return NULL;
    }

    int count = 0;
    unsigned long long value = 0;

    while (fscanf(test_file, "%llu", &value) == 1) {
        // Увеличение размера если количество больше
        if (count >= capacity) {
            capacity *= 2;
            unsigned long long* new_arr = (unsigned long long*)realloc(num_arr, capacity * sizeof(unsigned long long));
            if (!new_arr) {
                free(num_arr);
                fclose(test_file);
                return NULL;
            }
            num_arr = new_arr;
        }
        num_arr[count++] = value;
    }


    fclose(test_file);
    *out_count = count;
    return num_arr;
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
    int factor_count = get_prime_divider(m, factors, 100);

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





// Функция poker_test с определением периода
int poker_test(unsigned long long* numbers, int num_count,
    int observed[7], double expected[7], FILE* poker_test_f) {

    unsigned long long min_val = numbers[0];
    unsigned long long max_val = numbers[0];

    // Вычисление M
    unsigned long long M = find_range(numbers, num_count, &min_val, &max_val);
    fprintf(poker_test_f, "Диапазон значений: от %llu до %llu, M = %llu\n", min_val, max_val, M);

    // Подсчёт наблюдаемых частот
    int num_hands = count_observed(numbers, num_count, observed);
    if (num_hands == 0) {
        fprintf(poker_test_f, "Недостаточно чисел для формирования хотя бы одной руки (нужно минимум 5).\n");
        free(numbers);
        return 0;
    }

    fprintf(poker_test_f, "Количество полных рук (по 5 чисел): %d\n", num_hands);

    // Вычисление ожидаемых частот
    compute_expected(M, num_hands, expected);

    // ДОБАВЛЕНО: определение периода последовательности
    fprintf(poker_test_f, "\n========== АНАЛИЗ ПЕРИОДА ==========\n");
    PeriodResult period_result = find_sequence_period(numbers, num_count);

    if (period_result.found) {
        fprintf(poker_test_f, "Период: %llu\n", period_result.period);

        // Сравнение с M
        if (period_result.period > M) {
            fprintf(poker_test_f, "период больше m\n");
        }
        else if (period_result.period < M) {
            fprintf(poker_test_f, "период меньше m\n");
        }
        else {
            fprintf(poker_test_f, "период равен m\n");
        }
    }
    else {
        fprintf(poker_test_f, "Период не найден (последовательность не зациклилась в пределах %d чисел)\n", num_count);
    }
    fprintf(poker_test_f, "====================================\n\n");

    free(numbers);

    return num_hands;
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




// вывод покер-теста в отдельный файл
void write_poker_test_result(int observed[7], double expected[7], int num_hands, FILE* poker_test_f) {
    double chi2 = 0.0;

    fprintf(poker_test_f, "\n========== РЕЗУЛЬТАТЫ ТЕСТА ПОКЕРА ==========\n");
    fprintf(poker_test_f, "Тип комбинации   | Наблюдаемые | Ожидаемые  | (O-E)^2/E\n");
    fprintf(poker_test_f, "-----------------+-------------+------------+-----------\n");

    char* type_names[7] = {
        "Все разные",
        "Одна пара ",
        "Две пары  ",
        "Тройка    ",
        "Фулл-хаус ",
        "Каре      ",
        "Покер     "
    };

    for (int i = 0; i < 7; i++) {
        double diff = observed[i] - expected[i];
        double term = (diff * diff) / expected[i];
        chi2 += term;

        fprintf(poker_test_f, "%s | %11d | %10.2f | %10.4f\n",
            type_names[i], observed[i], expected[i], term);
    }

    fprintf(poker_test_f, "------------------------------------------------\n");
    fprintf(poker_test_f, "ХИ-КВАДРАТ = %.4f\n", chi2);
    fprintf(poker_test_f, "Степени свободы = 6\n");

    // Критические значения для 6 степеней свободы (таблица хи-квадрат)
    double critical_005 = 12.59;
    double critical_001 = 16.81;

    fprintf(poker_test_f, "\nВЫВОД О СЛУЧАЙНОСТИ:\n");
    if (chi2 < critical_005) {
        fprintf(poker_test_f, "✓ Последовательность ПРОШЛА тест покера (p > 0.05).\n");
        fprintf(poker_test_f, "  Распределение комбинаций близко к случайному.\n");
    }
    else if (chi2 < critical_001) {
        fprintf(poker_test_f, "⚠ Последовательность ПРОШЛА тест с уровнем значимости 0.01 < p < 0.05.\n");
        fprintf(poker_test_f, "  Имеются умеренные отклонения от случайности.\n");
    }
    else {
        fprintf(poker_test_f, "✗ Последовательность НЕ ПРОШЛА тест покера (p < 0.01).\n");
        fprintf(poker_test_f, "  Распределение комбинаций значимо отличается от случайного.\n");
    }
    fprintf(poker_test_f, "==============================================\n");
}





//=============================================================================
int main(void) {
#ifdef _WIN32
    SetConsoleOutputCP(1251);
#endif

    // Открытие входного файла
    FILE* input = fopen("input.txt", "r");
    if (input == NULL) {
        printf("Ошибка: не могу открыть input.txt\n");
#ifdef _WIN32
        system("pause");
#endif
        return 1;
    }

    // Чтение команды из файла
    char command[256] = { 0 };
    if (fgets(command, sizeof(command), input) == NULL) {
        printf("Ошибка: файл input.txt пуст\n");

        // Открываем output для записи ошибки
        FILE* output = fopen("output.txt", "w");
        if (output != NULL) {
            fprintf(output, "incorrect command\n");
            fclose(output);
        }

        fclose(input);
#ifdef _WIN32
        system("pause");
#endif
        return 1;
    }

    // Удаление символа перевода строки в конце
    size_t len = strlen(command);
    if (len > 0 && command[len - 1] == '\n') {
        command[len - 1] = '\0';
    }

    // Создание копии команды для парсинга
    char command_copy[256];
    strcpy(command_copy, command);

    // Получение ключевого слова
    char* saveptr = NULL;
    char* keyword = strtok_r(command_copy, " ", &saveptr);

    if (keyword == NULL) {
        FILE* output = fopen("output.txt", "w");
        if (output != NULL) {
            fprintf(output, "incorrect command\n");
            fclose(output);
        }
        fclose(input);
#ifdef _WIN32
        system("pause");
#endif
        return 1;
    }

    // Обработка команды get_c
    if (strcmp(keyword, "get_c") == 0) {
        // Открываем output на запись
        FILE* output = fopen("output.txt", "w");
        if (output == NULL) {
            printf("Ошибка: не могу открыть output.txt для записи\n");
            fclose(input);
#ifdef _WIN32
            system("pause");
#endif
            return 1;
        }

        char args_copy[256];
        strcpy(args_copy, command);

        Get_C_argument args = parse_get_c(args_copy);

        if (args.valid) {
            Get_C_result result = get_c(args.cmin, args.cmax, args.m);
            write_get_c_result(output, result);
            free_get_c_result(&result);
        }
        else {
            fprintf(output, "incorrect command\n");
        }

        fclose(output);
    }
    // Обработка команды get_a
    else if (strcmp(keyword, "get_a") == 0) {
        // Открываем output на запись
        FILE* output = fopen("output.txt", "w");
        if (output == NULL) {
            printf("Ошибка: не могу открыть output.txt для записи\n");
            fclose(input);
#ifdef _WIN32
            system("pause");
#endif
            return 1;
        }

        char args_copy[256];
        strcpy(args_copy, command);

        Get_A_argument args = parse_get_a(args_copy);

        if (args.valid) {
            unsigned long long a = get_a(args.m);
            write_get_a_result(output, a);
        }
        else {
            fprintf(output, "incorrect command\n");
        }

        fclose(output);
    }
    // Обработка команды lcg
    else if (strcmp(keyword, "lcg") == 0) {
        // Открываем output на запись
        FILE* output = fopen("output.txt", "w");
        if (output == NULL) {
            printf("Ошибка: не могу открыть output.txt для записи\n");
            fclose(input);
#ifdef _WIN32
            system("pause");
#endif
            return 1;
        }

        char args_copy[256];
        strcpy(args_copy, command);

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

        fclose(output);
    }
    // Обработка команды test
    else if (strcmp(keyword, "test") == 0) {
        // Открываем файл для результатов теста
        FILE* poker_test_f = fopen("poker_test.txt", "w");
        if (poker_test_f == NULL) {
            printf("Ошибка: не могу создать poker_test.txt\n");
            fclose(input);
            // Открываем output для записи ошибки
            FILE* output = fopen("output.txt", "w");
            if (output != NULL) {
                fprintf(output, "incorrect command\n");
                fclose(output);
            }

            fclose(input);
#ifdef _WIN32
            system("pause");
#endif
            return 1;
        }

        int num_count = 0;
        int observed[7] = { 0 };
        double expected[7] = { 0.0 };

        // Функция parse_test должна быть модифицирована для чтения из файла
        // или создана новая функция parse_test_from_file
        unsigned long long* numbers = parse_test(command, &num_count);

        if (numbers != NULL && num_count > 0) {
            int num_hands = poker_test(numbers, num_count, observed, expected, poker_test_f);

            if (num_hands > 0) {
                write_poker_test_result(observed, expected, num_hands, poker_test_f);
            }
            else {
                fprintf(poker_test_f, "incorrect work\n");
            }
        }
        else {
            fprintf(poker_test_f, "incorrect command\n");
        }

        fclose(poker_test_f);
    }
    // Неизвестная команда
    else {
        FILE* output = fopen("output.txt", "w");
        if (output != NULL) {
            fprintf(output, "incorrect command\n");
            fclose(output);
        }
    }

    // Закрытие входного файла
    fclose(input);

#ifdef _WIN32
    system("pause");
#endif

    return 0;
}