#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define RUN_TEST_1(test_name, count, ...) \
    { \
        printf("Тест [%s]:\n", test_name); \
        int res = 0; \
        double eps = 1e-15; \
        enum Status st = is_convex_polygon(&res, eps, count, __VA_ARGS__); \
        if (st != STATUS_OK) { \
            print_status(st, argv[0]); \
        } else { \
            printf("result = %s\n", (res == 1 ? "Выпуклый" : "Не выпуклый")); \
        } \
    } 

#define RUN_TEST_2(test_name, x, n, ...) \
    { \
        printf("Тест [%s]:\n", test_name); \
        double res = 0; \
        enum Status st = polynomial_value(&res, x, n, __VA_ARGS__); \
        if (st != STATUS_OK) { \
            print_status(st, argv[0]); \
        } else { \
            printf("%.2f\n", res); \
        } \
    } 

#define RUN_TEST_3(test_name, base, count, ...) \
    { \
        printf("Тест [%s]:\n", test_name); \
        enum Status st = find_kaprekar(base, count, __VA_ARGS__); \
        if (st != STATUS_OK) { \
            print_status(st, argv[0]); \
        } \
    } 

#define RUN_TEST_4(test_name, count, ...) \
    { \
        printf("Тест [%s]:\n", test_name); \
        double res = 0; \
        enum Status st = geometric_mean(&res, count, __VA_ARGS__); \
        if (st != STATUS_OK) { \
            print_status(st, argv[0]); \
        } else { \
            printf("result = %.2f\n", res); \
        } \
    } 

#define RUN_TEST_5(test_name, base, exp) \
    { \
        printf("Тест [%s]:\n", test_name); \
        double res = 0; \
        enum Status st = fast_pow_wrapper(base, exp, &res); \
        if (st != STATUS_OK) { \
            print_status(st, argv[0]); \
        } else { \
            printf("%s = %.2f\n", test_name, res); \
        } \
    } 

#define RUN_TEST_6(test_name, f, a, b, eps) \
    { \
        printf("Тест [%s]:\n", test_name); \
        double res = 0; \
        enum Status st = bisection(f, a, b, eps, &res); \
        if (st != STATUS_OK) { \
            print_status(st, argv[0]); \
        } else { \
            printf("root = %.3f\n", res); \
        } \
    } 

enum Status {
    STATUS_OK,
    STATUS_INVALID_ARGC,
    STATUS_INVALID_FLOAT,
    STATUS_OVERFLOW,
    STATUS_INVALID_ARG,
    STATUS_LIMIT,
    STATUS_ERR_POLYGON,
    STATUS_ERR_MEMORY,
    STATUS_NO_ROOTS
};

void print_status(enum Status st, char *prefix) {
    switch (st) {
        case STATUS_OK:
            return;
        case STATUS_OVERFLOW:
            fprintf(stderr, "%s: переполнение\n", prefix);
            break;
        case STATUS_INVALID_ARG:
            fprintf(stderr, "%s: недопустимый аргумент\n", prefix);
            break;
        case STATUS_INVALID_ARGC:
            fprintf(stderr, "%s: неверное количество аргументов\n", prefix);
            break;
        case STATUS_INVALID_FLOAT:
            fprintf(stderr, "%s: недопустимое значение вещественного числа\n", prefix);
            break;
        case STATUS_LIMIT:
            fprintf(stderr, "%s: достигнут лимит итераций\n", prefix);
            break;
        case STATUS_ERR_POLYGON:
            fprintf(stderr, "%s: заданные точки не задают многоугольник\n", prefix);
            break;
        case STATUS_ERR_MEMORY:
            fprintf(stderr, "%s: не удалось выделить память\n", prefix);
            break;
        case STATUS_NO_ROOTS:
            fprintf(stderr, "%s: не удалось найти корень на заданном интервале\n", prefix);
            break;
        default:
            fprintf(stderr, "%s: неизвестный статус код\n", prefix);
    }
}

enum Status parse_float(char *s, double *out) {
    char *endptr;
    double d = strtod(s, &endptr);
    if (endptr == s || *endptr != '\0')
        return STATUS_INVALID_FLOAT;
    
    if (d >= HUGE_VAL || d <= -HUGE_VAL) 
        return STATUS_OVERFLOW;

    *out = d;
    return STATUS_OK;
}

int overflow(double val) {
    return isinf(val) || isnan(val);
}

enum Status is_convex_polygon(int *res, double eps, int n, ...) {
    if (n < 3) return STATUS_INVALID_ARGC;
    if (!res || eps < 0.0) return STATUS_INVALID_ARG;
    if (overflow(eps)) return STATUS_OVERFLOW;
    
    va_list args;
    va_start(args, n);
    
    double *x = malloc(n * sizeof(double));
    double *y = malloc(n * sizeof(double));
    if (!x || !y) {
        free(x); 
        free(y);
        return STATUS_ERR_MEMORY;
    }
    
    for (int i = 0; i < n; i++) {
        x[i] = va_arg(args, double);
        y[i] = va_arg(args, double);
        if (overflow(x[i]) || overflow(y[i])) {
            free(x); 
            free(y);
            va_end(args);
            return STATUS_INVALID_FLOAT;
        }
    }
    va_end(args);
    
    int sign = 0;
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        int k = (i + 2) % n;
        
        double cross = (x[j] - x[i]) * (y[k] - y[j]) - 
                       (y[j] - y[i]) * (x[k] - x[j]);
        
        if (fabs(cross) < eps) continue; 
        
        int current_sign = (cross > 0) ? 1 : -1;
        if (sign == 0) {
            sign = current_sign;
        } else if (sign != current_sign) {
            free(x); 
            free(y);
            *res = 0;
            return STATUS_OK;
        }
    }

    if (sign == 0) {
        *res = -1;
        free(x); 
        free(y);
        return STATUS_ERR_POLYGON;
    }
    
    free(x); 
    free(y);
    *res = 1;
    return STATUS_OK;
}

enum Status polynomial_value(double *res, double x, int n, ...) {
    if (n < 0 || !res) return STATUS_INVALID_ARG;
    
    va_list args;
    va_start(args, n);
    
    double result = 0.0;
    for (int i = 0; i <= n; i++) {
        double coeff = va_arg(args, double);
        if (overflow(coeff)) {
            va_end(args);
            return STATUS_INVALID_FLOAT;
        }
        result = result * x + coeff;
        if (overflow(result)) {
            va_end(args);
            return STATUS_OVERFLOW;
        }
    }
    va_end(args);
    
    *res = result;
    return STATUS_OK;
}

enum Status parse_ll(const char *str, int base, long long *res) {
    if (!str || !res) return STATUS_INVALID_ARG;
    if (base < 2 || base > 36) return STATUS_INVALID_ARG;

    const unsigned char *p = (const unsigned char *)str;

    if (*p == '\0') return STATUS_INVALID_ARG;

    unsigned long long acc = 0;
    unsigned long long ubase = (unsigned long long)base;
    const unsigned long long LLONG_MAX_U = (unsigned long long)LLONG_MAX;

    while (*p != '\0') {
        unsigned char c = *p;
        int digit;

        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'z') {
            digit = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'Z') {
            digit = c - 'A' + 10;
        } else {
            return STATUS_INVALID_ARG; 
        }

        if (digit >= base) return STATUS_INVALID_ARG;

        if (acc > (LLONG_MAX_U - (unsigned long long)digit) / ubase) {
            return STATUS_OVERFLOW;
        }

        acc = acc * ubase + (unsigned long long)digit;
        p++;
    }

    *res = (long long)acc;
    return STATUS_OK;
}

enum Status is_kaprekar(char *str, int base, int *res) {
    if (base < 2) return STATUS_INVALID_ARG;
    if (!str || !res) return STATUS_INVALID_ARG;
    long long num;
    enum Status st = parse_ll(str, base, &num);
    if (st != STATUS_OK) return st;
    if (num < 0) return STATUS_INVALID_ARG;
    
    if (num == 0) {
        *res = 1;
        return STATUS_OK;
    }
    if (num > LLONG_MAX / num) return STATUS_OVERFLOW;
    long long square = num * num;
    if (square == 0 && num == 0) {
        *res = 1;
        return STATUS_OK;
    }
    
    base = 10;
    long long temp = square;
    int p = 0;
    while (temp > 0) {
        p++;
        temp /= base;
    }
    
    for (int i = 1; i < p; i++) {
        long long power = 1;
        for (int j = 0; j < i; j++) power *= base;
        
        long long right = square % power;
        long long left = square / power;
        
        if (right > 0 && left + right == num) {
            *res = 1;
            return STATUS_OK;
        }
    }
    *res = 0;
    return STATUS_OK;
}

enum Status find_kaprekar(int base, int count, ...) {
    if (base < 2 || base > 36) return STATUS_INVALID_ARG;
    if (count < 1) return STATUS_INVALID_ARGC;
    
    va_list args;
    va_start(args, count);
    
    printf("Числа Капрекара в системе счисления с основанием %d:\n", base);
    int found = 0;
    for (int i = 0; i < count; i++) {
        char *str = va_arg(args, char*);
        if (!str) {
            va_end(args);
            return STATUS_INVALID_ARG;
        }
        int res = 0;
        enum Status st = is_kaprekar(str, base, &res);
        if (st == 0 && res == 1) {
            printf("  %s", str);
            found++;
        } else  if (st != 0) {
            return st;
        }
    }
    va_end(args);
    
    if (found == 0) printf("  не найдено");
    printf("\n");
    return STATUS_OK;
}

enum Status geometric_mean(double *res, int count, ...) {
    if (count <= 0) return STATUS_INVALID_ARGC;
    if (!res) return STATUS_INVALID_ARG;
    
    va_list args;
    va_start(args, count);
    
    double product = 1.0;
    
    for (int i = 0; i < count; i++) {
        double val = va_arg(args, double);
        if (overflow(val)) {
            va_end(args);
            return STATUS_OVERFLOW;
        }
        if (val < 0) {
            va_end(args);
            return STATUS_INVALID_ARG;
        }
        
        product *= val;
        if (overflow(product)) {
            va_end(args);
            return STATUS_OVERFLOW;
        }
    }
    va_end(args);
    
    *res = pow(fabs(product), 1.0 / count);
    return STATUS_OK;
}

enum Status fast_pow(double b, int e, double *out) {
    if (e == 0) {
        *out = 1.0;
        return STATUS_OK;
    }
    
    double half;
    enum Status status = fast_pow(b, e >> 1, &half);
    if (status != STATUS_OK) return status;
    
    double res = half * half;
    if (overflow(res)) return STATUS_OVERFLOW;
    
    if (e & 1) {
        res *= b;
        if (overflow(res)) return STATUS_OVERFLOW;
    }
    
    *out = res;
    return STATUS_OK;
}

enum Status fast_pow_wrapper(double base, int exp, double *result) {
    if (!result) return STATUS_INVALID_ARG;
    if (overflow(base)) return STATUS_OVERFLOW;
    
    int neg = 0;
    if (exp < 0) {
        if (base == 0.0) return STATUS_INVALID_ARG;
        neg = 1;
        if (exp == INT_MIN) return STATUS_OVERFLOW;
        exp = -exp;
    }
    
    double res;
    enum Status status = fast_pow(base, exp, &res);
    if (status != STATUS_OK) return status;
    
    if (neg) res = 1.0 / res;
    if (overflow(res)) return STATUS_OVERFLOW;
    
    *result = res;
    return STATUS_OK;
}

typedef double (*func_t)(double);

enum Status bisection(func_t f, double a, double b, double eps, double *root) {
    if (eps <= 0 || !root || !f) return STATUS_INVALID_ARG;
    if (a > b) {
        double temp = a;
        a = b;
        b = temp;
    }
    double fa = f(a), fb = f(b);
    if (overflow(fa) || overflow(fb)) return STATUS_OVERFLOW;
    
    if ((fa < 0 && fb < 0) || (fa > 0 && fb > 0)) return STATUS_NO_ROOTS; 
    
    int max_iter = 1000000;
    int iter = 0;
    
    while ((b - a) / 2.0 > eps && iter < max_iter) {
        double mid = (a + b) / 2.0;
        double fmid = f(mid);
        if (overflow(fmid)) return STATUS_OVERFLOW;
        
        if (fabs(fmid) < 1e-15) {
            *root = mid;
            return STATUS_OK;
        }
        
        if ((fa < 0 && fmid > 0) || (fa > 0 && fmid < 0)) {
            b = mid;
        } else {
            a = mid;
            fa = fmid;
        }
        iter++;
    }
    if (iter >= max_iter) return STATUS_LIMIT;
    
    *root = (a + b) / 2.0;
    return STATUS_OK;
}

double f1(double x) { return x * x - 4; }
double f2(double x) { return cos(x) - x; }
double f3(double x) { return x * x * x - x - 2; }

int main(int argc, char *argv[]) {
    if (argc != 1) {
        print_status(STATUS_INVALID_ARGC, argv[0]);
        return STATUS_INVALID_ARGC;
    }

    printf("\n= = = = = Тесты для функции №1 = = = = = =\n");
    RUN_TEST_1("Меньше 3 точек", 2, 0.0, 0.0, 1.0, 1.0);
    RUN_TEST_1("Треугольник по часовой", 3, 0.0, 0.0, 0.0, 2.0, 2.0, 0.0);
    RUN_TEST_1("Треугольник против часовой", 3, 0.0, 0.0, 2.0, 0.0, 0.0, 2.0);
    RUN_TEST_1("Квадрат", 4, 0.0, 0.0, 0.0, 2.0, 2.0, 2.0, 2.0, 0.0);
    RUN_TEST_1("Невыпуклый четырехугольник", 4, 0.0, 0.0, 0.0, 3.0, 1.0, 1.0, 3.0, 0.0);
    RUN_TEST_1("Точки на одной прямой", 3, 0.0, 0.0, 1.0, 1.0, 2.0, 2.0);
    RUN_TEST_1("Самопересекающийся", 4, 0.0, 0.0, 2.0, 2.0, 0.0, 2.0, 2.0, 0.0);

    printf("\n= = = = = Тесты для функции №2 = = = = = =\n");
    RUN_TEST_2("n < 0", 2.0, -1, 1.0);
    RUN_TEST_2("x=3: 5", 3.0, 0, 5.0);
    RUN_TEST_2("x=2: 3x + 4", 2.0, 1, 3.0, 4.0);
    RUN_TEST_2("x=2: 2x^3 - 3x^2 + 5", 2.0, 3, 2.0, -3.0, 0.0, 5.0);
    RUN_TEST_2("x=0: 7x^2 + 8x + 9", 0.0, 2, 7.0, 8.0, 9.0);

    printf("\n= = = = = Тесты для функции №3 = = = = = =\n");
    RUN_TEST_3("Valid Kaprekar numbers in base 10", 10, 3, "9", "45", "297");
    RUN_TEST_3("Non-Kaprekar numbers in base 10", 10, 3, "10", "50", "100");
    RUN_TEST_3("Valid Kaprekar numbers in custom bases", 6, 3, "5", "13", "44");
    RUN_TEST_3("Invalid small base error handling", 1, 3, "9", "45", "297");
    RUN_TEST_3("Invalid large base error handling", 37, 3, "9", "45", "297");
    RUN_TEST_3("Letters in number", 16, 3, "9", "2D", "297");
    RUN_TEST_3("Invalid count parameter error handling", 10, 0, "9", "45", "297");

    printf("\n= = = = = Тесты для функции №4 = = = = = =\n");
    RUN_TEST_4("Standard valid calculation with integers", 4, 2.0, 4.0, 1.0, 1.0);
    RUN_TEST_4("Valid calculation with fractional values", 4, 0.5, 2.0, 0.25, 4.0);
    RUN_TEST_4("Calculation containing a zero value", 4, 5.5, 0.0, 12.3, 1.1);
    RUN_TEST_4("Negative value error handling", 4, 3.0, 5.5, -2.1, 4.0);
    RUN_TEST_4("Overflow detection during product calculation", 4, 1e300, 1e300, 1e300, 1e300);
    RUN_TEST_4("Invalid count parameter error handling", 0, 1.0, 2.0, 3.0, 4.0);

    printf("\n= = = = = Тесты для функции №5 = = = = = =\n");
    RUN_TEST_5("2^10", 2.0, 10);
    RUN_TEST_5("2^(-3)", 2.0, -3);
    RUN_TEST_5("1.5^20", 1.5, 20);
    RUN_TEST_5("1.056^316", 1.056, 316);
    
    printf("\n= = = = = Тесты для функции №6 = = = = = =\n");
    RUN_TEST_6("x^2-4=0 на [0,5]", f1, 0.0, 5.0, 1e-6);
    RUN_TEST_6("x^2-4=0 на [-5,0]", f1, -5.0, 0.0, 1e-6);
    RUN_TEST_6("перепутаны границы", f1, 0.0, -5.0, 1e-6);
    RUN_TEST_6("cos(x)-x=0 на [0,1", f2, 0.0, 1.0, 1e-8);
    RUN_TEST_6("x^3-x-2=0 на [1,2]", f3, 1.0, 2.0, 1e-7);
    RUN_TEST_6("нет корня", f1, 3.0, 5.0, 1e-6);
    RUN_TEST_6("NULL", NULL, 3.0, 5.0, 1e-6);
}