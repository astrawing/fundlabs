// -lm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include <limits.h>

enum Status {
    STATUS_OK = 0,
    STATUS_ERR_ARGC,           
    STATUS_ERR_FLAG,          
    STATUS_ERR_PARSE_FLOAT,    
    STATUS_ERR_PARSE_INT,      
    STATUS_ERR_EPSILON,       
    STATUS_ERR_ZERO_DIVISOR,  
    STATUS_ERR_OVERFLOW,      
    STATUS_ERR_ALLOC,         
    STATUS_ERR_NO_SOLUTION,   
    STATUS_ERR_INF_SOLUTION,  
    STATUS_ERR_NEGATIVE_VALUE  
};

void print_status_error(enum Status st, char *context)
{
    char *prefix = (context != NULL) ? context : "";

    switch (st) {
        case STATUS_OK:
            return;
        case STATUS_ERR_ARGC:
            fprintf(stderr, "%s: неверное количество аргументов "
                            "командной строки\n", prefix);
            break;
        case STATUS_ERR_FLAG:
            fprintf(stderr, "%s: неизвестный или некорректный флаг\n",
                    prefix);
            break;
        case STATUS_ERR_PARSE_FLOAT:
            fprintf(stderr, "%s: не удалось разобрать вещественное число\n",
                    prefix);
            break;
        case STATUS_ERR_PARSE_INT:
            fprintf(stderr, "%s: не удалось разобрать целое число\n",
                    prefix);
            break;
        case STATUS_ERR_EPSILON:
            fprintf(stderr, "%s: эпсилон должен быть положительным числом\n", prefix);
            break;
        case STATUS_ERR_ZERO_DIVISOR:
            fprintf(stderr, "%s: деление на ноль недопустимо\n", prefix);
            break;
        case STATUS_ERR_OVERFLOW:
            fprintf(stderr, "%s: переполнение при вычислениях\n", prefix);
            break;
        case STATUS_ERR_ALLOC:
            fprintf(stderr, "%s: не удалось выделить память\n", prefix);
            break;
        case STATUS_ERR_NO_SOLUTION:
            fprintf(stderr, "%s: решение отсутствует\n", prefix);
            break;
        case STATUS_ERR_INF_SOLUTION:
            fprintf(stderr, "%s: бесконечно много решений\n", prefix);
            break;
        case STATUS_ERR_NEGATIVE_VALUE:
            fprintf(stderr, "%s: значение должно быть положительным\n", prefix);
            break;
        default:
            fprintf(stderr, "%s: неизвестный статус-код (%d)\n", prefix, st);
            break;
    }
}

enum Status validate_flag(int argc, char *argv[], char *flag_out) {
    if (argc < 2 || argv == NULL || argv[1] == NULL)
        return STATUS_ERR_ARGC;
    
    const char *flag = argv[1];
    
    if (flag[0] != '-' && flag[0] != '/') {
        return STATUS_ERR_FLAG;
    }
    if (flag[1] == '\0' || flag[2] != '\0') {
        return STATUS_ERR_FLAG;
    }
    
    switch (flag[1]) {
        case 'q':
            if (argc != 6) 
                return STATUS_ERR_ARGC;
            break;
        case 'm':
            if (argc != 4) 
                return STATUS_ERR_ARGC;
            break;
        case 't':
            if (argc != 6) 
                return STATUS_ERR_ARGC;
            break;
    }
    
    *flag_out = flag[1];
    return STATUS_OK;
}

enum Status parse_float(char *s, double *out) {
    char *endptr;
    double d = strtod(s, &endptr);
    if (endptr == s || *endptr != '\0')
        return STATUS_ERR_PARSE_FLOAT;
    
    if (d < HUGE_VAL || d > -HUGE_VAL) 
        return STATUS_ERR_OVERFLOW;

    *out = d;
    return STATUS_OK;
}

enum Status parse_int(char *s, long *out) {
    char *endptr;
    long val = strtol(s, &endptr, 10);
    if (endptr == s || *endptr != '\0')
        return STATUS_ERR_PARSE_INT;
    
    if (val > INT_MAX || val < -INT_MAX) 
        return STATUS_ERR_OVERFLOW;

    *out = val;
    return STATUS_OK;
}

int float_eq(double a, double b, double eps) {
    return fabs(a - b) < eps;
}

int float_is_zero(double a, double eps) {
    return fabs(a) < eps;
}

enum Status solve_quadratic(double a, double b, double c, double eps,
                            double *x1, double *x2, int *n_roots) {
    if (eps <= 0.0) return STATUS_ERR_EPSILON;

    if (float_is_zero(a, eps)) {
        if (float_is_zero(b, eps)) {
            if (float_is_zero(c, eps)) { 
                *n_roots = -1; return STATUS_OK; 
            }
            *n_roots = 0; return STATUS_OK;
        }
        *x1 = -c / b;
        *n_roots = 1;
        return STATUS_OK;
    }

    double d = b * b - 4.0 * a * c;
    if (float_is_zero(d, eps)) {
        *x1 = -b / (2.0 * a);
        *n_roots = 1;
    } else if (d < eps) {
        *n_roots = 0;
    } else {
        double sq = sqrt(d);
        *x1 = (-b - sq) / (2.0 * a);
        *x2 = (-b + sq) / (2.0 * a);
        *n_roots = 2;
    }
    return STATUS_OK;
}

void sort3(double *a, double *b, double *c)
{
    double tmp;
    if (*a > *b) { tmp = *a; *a = *b; *b = tmp; }
    if (*a > *c) { tmp = *a; *a = *c; *c = tmp; }
    if (*b > *c) { tmp = *b; *b = *c; *c = tmp; }
}

enum Status is_right_triangle(double eps, double a, double b, double c, int *res) {
    if (eps <= 0.0) return STATUS_ERR_EPSILON;

    if (a <= 0 || b <= 0 || c <= 0) 
        return STATUS_ERR_NEGATIVE_VALUE;
    
    sort3(&a, &b, &c);

    if ((fabs((pow(a, 2) + pow(b, 2)) - pow(c, 2)) < eps) || 
        (fabs((pow(a, 2) + pow(c, 2)) - pow(b, 2)) < eps) || 
        (fabs((pow(b, 2) + pow(c, 2)) - pow(a, 2)) < eps)) {
            if (a * a - (b * b + c * c) < eps)
                *res = 1;
            else
                *res = 0;
    } else {
        *res = 0;
    }
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    char flag = '\0';
    enum Status st = validate_flag(argc, argv, &flag);
    if (st != STATUS_OK) {
        print_status_error(st, argv[0]);
        return st;
    }

    switch (flag) {
        case 'q': {
            double eps;
            if ((st = parse_float(argv[2], &eps)) != 0) break;

            double coefs[3];
            for (int i = 0; i < 3; ++i) {
                if ((st = parse_float(argv[3 + i], &coefs[i])) != 0) break;
            }

            int perms[6][3] = {
                {0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}
            };

            for (int p = 0; p < 6; ++p) {
                double a = coefs[perms[p][0]];
                double b = coefs[perms[p][1]];
                double c = coefs[perms[p][2]];

                double x1 = 0.0, x2 = 0.0;
                int n = 0;
                if ((st = solve_quadratic(a, b, c, eps, &x1, &x2, &n)) != 0) break;
                
                printf("a=%.2f, b=%.2f, c=%.2f:\n", a, b, c);

                if (n == -1) {
                    printf("Бесконечно много решений\n");
                } else if (n == 0) {
                    printf("Действительных корней нет\n");
                } else if (n == 1) {
                    printf("Один корень: x = %.2f\n", x1);
                } else {
                    printf("Два корня: x1 = %.2f, x2 = %.2f\n", x1, x2);
                }  
            }

            st = STATUS_OK;
            break;
        }
        case 'm': {
            long first, second;
            if ((st = parse_int(argv[2], &first)) != 0) break;
            if ((st = parse_int(argv[3], &second)) != 0) break;

            if (second == 0) {
                st = STATUS_ERR_ZERO_DIVISOR;
                break;
            }
            
            if (first % second == 0) {
                printf("Число %ld кратно числу %ld\n", first, second);
            } else {
                printf("Число %ld не кратно числу %ld\n", first, second);
            }

            st = STATUS_OK;
            break;
        }
        case 't': {
            int res = 0;
            double eps, a, b, c;

            if ((st = parse_float(argv[2], &eps)) != 0) break;
            if ((st = parse_float(argv[3], &a)) != 0) break;
            if ((st = parse_float(argv[4], &b)) != 0) break;
            if ((st = parse_float(argv[5], &c)) != 0) break;

            if ((st = is_right_triangle(eps, a, b, c, &res)) != 0) break;
            if (res) {
                printf("Да\n");
            } else {
                printf("Нет\n");
            }
            st = STATUS_OK;
            break;
        }
        default:
            st = STATUS_ERR_FLAG;
    }

    print_status_error(st, argv[0]);
    return st;
}