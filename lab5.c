#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>

enum Status {
    STATUS_OK,
    STATUS_OVERFLOW,
    STATUS_INVALID_ARG,
    STATUS_INVALID_ARGC,
    STATUS_INVALID_FLOAT
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
        default:
            fprintf(stderr, "%s: неизвестный статус код\n", prefix);
    }
}

enum Status parse_float(char *s, double *out) {
    char *endptr;
    double d = strtod(s, &endptr);
    if (endptr == s || *endptr != '\0')
        return STATUS_INVALID_FLOAT;
    
    if (d == HUGE_VAL || d == -HUGE_VAL) 
        return STATUS_OVERFLOW;

    *out = d;
    return STATUS_OK;
}

int overflow(double val) {
    return isinf(val) || isnan(val);
}

enum Status calculate_sum_a(double x, double eps, double *result) {
    if (result == NULL) return STATUS_INVALID_ARG;

    int n = 0;
    double sum = 0.0;
    double term = 1.0;

    while (fabs(term) > eps) {
        sum += term;
        term = term * x / (n + 1);
        n++;
        if (overflow(term) || overflow(sum)) return STATUS_OVERFLOW;
    }

    *result = sum;
    return STATUS_OK;
}

enum Status calculate_sum_b(double x, double eps, double *result) {
    if (result == NULL) return STATUS_INVALID_ARG;

    int n = 0;
    double sum = 0.0;
    double term = 1.0;

    while (fabs(term) > eps) {
        sum += term;
        term = -term * x * x / ((2 * n + 1) * (2 * n + 2));
        n++;
        if (overflow(term) || overflow(sum)) return STATUS_OVERFLOW;
    }

    *result = sum;
    return STATUS_OK;
}

enum Status calculate_sum_c(double x, double eps, double *result) {
    if (result == NULL) return STATUS_INVALID_ARG;

    int n = 0;
    double sum = 0.0;
    double term = 1.0;

    while (fabs(term) > eps) {
        sum += term;
        term = term * 9 * (n + 1) * (n + 1) * x * x / ((3 * n + 1) * (3 * n + 2));
        n++;
        if (overflow(term) || overflow(sum)) return STATUS_OVERFLOW;
        if (n > 10000) break;
    }

    *result = sum;
    return STATUS_OK;
}

enum Status calculate_sum_d(double x, double eps, double *result) {
    if (result == NULL) return STATUS_INVALID_ARG;

    int n = 1;
    double sum = 0.0;
    double term = -0.5 * x * x; 

    while (fabs(term) > eps) {
        sum += term;
        term = -term * (2.0 * n + 1.0) * (x * x) / (2.0 * n + 2);
        n++;
        if (overflow(term) || overflow(sum)) return STATUS_OVERFLOW;
        if (n > 10000) break;
    }

    *result = sum;
    return STATUS_OK;
}

double f_a(double x, double eps) {
    if (x < eps) return 1.0;
    return log(1.0 + x) / x;
}

double f_b(double x, double eps) {
    return exp((x * x) / 2.0);
}

double f_c(double x, double eps) {
    if (x == 1) x = 1.0 - eps;
    return log(1.0 / (1.0 - x));
}

double f_d(double x, double eps) {
    if (x < eps) return 1.0;
    return pow(x, x);
}

enum Status integrate_n(double f(double, double), double a, double b, double eps, int n, double *result) {
    if (n < 1) return STATUS_INVALID_ARG;
    double h = (b - a) / n;
    double sum = 0.5 * (f(a, eps) + f(b, eps));
    
    for (int i = 1; i < n; i++) {
        sum += f(a + i * h, eps);
        if (overflow(sum * h)) return STATUS_OVERFLOW;
    }
    
    *result = sum * h;
    return STATUS_OK;
}

double integrate_with_eps(double f(double, double), double a, double b, double eps, double *result) {
    enum Status st = 0;
    int n = 4;
    double I_old = 0.0;
    if ((st = integrate_n(f, a, b, eps, n, &I_old)) != 0) return st;
    
    while (1) {
        n *= 2;
        double I_new = 0.0;
        if ((st = integrate_n(f, a, b, eps, n, &I_new)) != 0) return st;
        
        if (fabs(I_new - I_old) / 3.0 < eps) {
            // деление на 3 по правилу Рунге оценки погрешности
            *result = I_new;
            return STATUS_OK;
        }
        
        I_old = I_new;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_status(STATUS_INVALID_ARGC, argv[0]);
        return STATUS_INVALID_ARGC;
    }

    enum Status st = 0;
    double args[2] = {0.0, 0.0};
    for (int i = 0; i < 2; ++i) {
        if ((st = parse_float(argv[i+1], &args[i])) != 0) {
            print_status(st, argv[0]);
            return st;
        }
    }

    printf("1) Ряды:\n");

    double sum = 0.0;
    calculate_sum_a(args[0], args[1], &sum);
    printf("  a. %f\n", sum);

    sum = 0.0;
    calculate_sum_b(args[0], args[1], &sum);
    printf("  b. %f\n", sum);
    
    sum = 0.0;
    if ((st = calculate_sum_c(args[0], args[1], &sum)) == 0) {
        printf("  c. %f\n", sum);
    } else {
        printf("  c. Ряд расходится\n");
    }

    sum = 0.0;
    if ((st = calculate_sum_d(args[0], args[1], &sum)) == 0) {
        printf("  d. %f\n", sum);
    } else {
        printf("  d. Ряд расходится\n");
    }
    
    printf("2) Интегралы:\n");

    double rs = 0.0;
    if ((st = integrate_with_eps(f_a, 0.0, 1.0, args[1], &rs)) == 0) {
        printf("  a. %f\n", rs);
    } else {
        printf("  a. Интеграл расходится\n");
    }

    if ((st = integrate_with_eps(f_b, 0.0, 1.0, args[1], &rs)) == 0) {
        printf("  b. %f\n", rs);
    } else {
        printf("  b. Интеграл расходится\n");
    }

    if ((st = integrate_with_eps(f_c, 0.0, 1.0, args[1], &rs)) == 0) {
        printf("  c. %f\n", rs);
    } else {
        printf("  c. Интеграл расходится\n");
    }

    if ((st = integrate_with_eps(f_d, 0.0, 1.0, args[1], &rs)) == 0) {
        printf("  d. %f\n", rs);
    } else {
        printf("  d. Интеграл расходится\n");
    }
}