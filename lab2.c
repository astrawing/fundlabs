#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>


enum Status {
    STATUS_OK,
    STATUS_INVALID_ARGC,
    STATUS_INVALID_FLOAT,
    STATUS_OVERFLOW,
    STATUS_INVALID_ARG,
    STATUS_LIMIT
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
    return isinf(val) || isnan(val) || fabs(val) >= HUGE_VAL;
}

enum Status factorial(int n, unsigned long *result) {
    if (n < 0) return STATUS_INVALID_ARG;
    if (n == 0) {
        *result = 1;
        return STATUS_OK;
    }

    unsigned long res = 1;
    for (int i = 1; i <= n; ++i) {
        if (res * i < res) {
            return STATUS_OVERFLOW;
        }
        res *= i;
    }

    *result = res;
    return STATUS_OK;
}

enum Status lim_e(long n, double *val) {
    if (n <= 0) return STATUS_INVALID_ARG;
    *val = pow(1.0 + 1.0 / n, n);
    return STATUS_OK;
}

enum Status lim_pi(long n, double *val) {
    if (n <= 0) return STATUS_INVALID_ARG;

    double ln_num = 4.0 * (n * log(2) + lgamma(n + 1));
    double ln_den = log(n) + 2.0 * lgamma(2 * n + 1);
    *val = exp(ln_num - ln_den);
    return STATUS_OK;
}

enum Status lim_ln2(long n, double *val) {
    if (n <= 0) return STATUS_INVALID_ARG;
    *val = n * (pow(2.0, 1.0 / n) - 1.0);
    return STATUS_OK;
}

enum Status compute_by_limit(enum Status lim_func(long, double *), double eps, double *result) {
    if (!lim_func || eps <= 0 || !result) return STATUS_INVALID_ARG;
    long n = 1;
    double prev = 0.0, current;
    enum Status st = 0;
    if ((st = lim_func(n, &current)) != 0) return st;
    
    while (fabs(prev - current) > eps) { 
        prev = current;
        n *= 2;
        if ((st = lim_func(n, &current)) != 0) return st;
        if (overflow(current)) return STATUS_OVERFLOW;
    }
    *result = current;
    return STATUS_OK;
}

enum Status lim_sqrt2(double eps, double *result) {
    if (eps <= 0 || !result) return STATUS_INVALID_ARG;
    double prev = -0.5;             
    double current = prev - prev * prev / 2.0 + 1.0;  
    while (fabs(current - prev) > eps) {
        if (overflow(current)) return STATUS_OVERFLOW;
        prev = current;
        current = prev - prev * prev / 2.0 + 1.0;
    }
    *result = current;
    return STATUS_OK;
}

enum Status lim_gamma(double eps, double *result) {
    if (eps <= 0 || !result) return STATUS_INVALID_ARG;
    int n = 1.0;
    double sum = 1;
    double previous = 0.0, current = 1.0;
    while (fabs(previous - current) > eps) {
        previous = current;
        int old_n = n;
        n *= 2;
        for (int i = old_n + 1; i <= n; i++) { sum += 1.0 / i; }
        current = sum - log(n);

        if(overflow(current)) return STATUS_OVERFLOW;
    }
    *result = current;
    return STATUS_OK;
}

enum Status row_e(double eps, double *result) {
    if (eps <= 0 || !result) return STATUS_INVALID_ARG;
    double sum = 1.0;      
    double term = 1.0;     
    int n = 1;
    while (fabs(term) > eps) {
        term /= n;         
        sum += term;
        n++;
        if (overflow(sum)) return STATUS_OVERFLOW;
    }

    *result = sum;
    return STATUS_OK;
}

enum Status row_pi(double eps, double *result) {
    if (eps <= 0 || !result) return STATUS_INVALID_ARG;
    double sum = 1.0;
    double term = 1.0;
    int n = 2;
    while (fabs(term) > eps) {
        term = -term * (2.0 * n - 3.0) / (2.0 * n - 1.0);
        sum += term;
        n++;
        if (overflow(sum)) return STATUS_OVERFLOW;
    }
    *result = 4 * sum;
    return STATUS_OK;
}

enum Status row_ln2(double eps, double *result) {
    if (eps <= 0 || !result) return STATUS_INVALID_ARG;
    double sum = 1.0;
    double term = 1.0;
    int n = 2;
    while (fabs(term) > eps) {
        term = -term * (n - 1.0) / n;
        sum += term;
        n++;
        if (overflow(sum)) return STATUS_OVERFLOW;
    }
    *result = sum;
    return STATUS_OK;
}

enum Status row_sqrt2(double eps, double *result) {
    if (eps <= 0 || !result) return STATUS_INVALID_ARG;
    double prod = 1.0;
    double term = sqrt(sqrt(2));   
    while (fabs(term - 1.0) > eps) {
        prod *= term;
        term = sqrt(term);
        if (overflow(prod)) return STATUS_OVERFLOW;
    }
    *result = prod;
    return STATUS_OK;
}

enum Status row_gamma(double eps, double *result) {
    if (eps <= 0.0 || !result) return STATUS_INVALID_ARG;
    double global_sum = 0.0;
    double k_term;
    int k = 2;

    do {
        double zeta_minus_one = 0.0;
        double m_term;
        int m = 2;
        do {
            m_term = pow((double)m, -k);
            zeta_minus_one += m_term;
            m++;
        } while (m_term > eps);
        k_term = zeta_minus_one / k;
        global_sum += k_term;
        if (overflow(global_sum)) return STATUS_OVERFLOW;
        k++;
    } while (k_term > eps);
    *result = 1.0 - global_sum;
    return STATUS_OK;
}


enum Status eq_e(double eps, double *result) {
    if (eps <= 0 || !result) return STATUS_INVALID_ARG;
    double a = 1.0, b = 3.0, c;
    double fa = log(a) - 1.0; 
    while ((b - a) / 2 > eps) {
        c = (a + b) / 2.0;
        double fc = log(c) - 1.0;
        if (fa * fc < 0.0) {
            b = c;
        } else {
            a = c;
            fa = fc;
        }
        if (overflow(a) || overflow(b) || overflow(c)) return STATUS_OVERFLOW; 
    }
    *result = (a + b) / 2.0;
    return STATUS_OK;
}

enum Status eq_pi(double eps, double *result) {
    if (eps <= 0 || !result) return STATUS_INVALID_ARG;
    double a = 3.0, b = 4.0, c;
    double fa = sin(a); 
    while ((b - a) / 2 > eps) {
        c = (a + b) / 2.0;
        double fc = sin(c);
        if (fa * fc < 0.0) {
            b = c;
        } else {
            a = c;
            fa = fc;
        }
        if (overflow(a) || overflow(b) || overflow(c)) return STATUS_OVERFLOW; 
    }
    *result = (a + b) / 2.0;
    return STATUS_OK;
}

enum Status eq_ln2(double eps, double *result) {
    if (eps <= 0 || !result) return STATUS_INVALID_ARG;
    double a = 0.0, b = 1.0, c;
    double fa = exp(a) - 2; 
    while ((b - a) / 2 > eps) {
        c = (a + b) / 2.0;
        double fc = exp(c) - 2;
        if (fa * fc < 0.0) {
            b = c;
        } else {
            a = c;
            fa = fc;
        }
        if (overflow(a) || overflow(b) || overflow(c)) return STATUS_OVERFLOW; 
    }
    *result = (a + b) / 2.0;
    return STATUS_OK;
}

enum Status eq_sqrt2(double eps, double *result) {
    if (eps <= 0 || !result) return STATUS_INVALID_ARG;
    double a = 1.0, b = 2.0, c;
    double fa = a*a - 2; 
    while ((b - a) / 2.0 > eps) {
        c = (a + b) / 2.0;
        double fc = c*c - 2;
        if (fa * fc < 0.0) {
            b = c;
        } else {
            a = c;
            fa = fc;
        }
        if (overflow(a) || overflow(b) || overflow(c)) return STATUS_OVERFLOW; 
    }
    *result = (a + b) / 2.0;
    return STATUS_OK;
}

double f_gamma(double x, double eps) {
    double sum = x - 1.0;
    double term;
    int k = 2;

    do {
        term = (1.0 / k) - (1.0 / (k + x - 1.0));
        sum += term;
        k++;
    } while (fabs(term) > eps * 0.001);

    return sum;
}

int is_prime(int number) {
    if (number < 2) return 0;
    if (number == 2) return 1; 
    if (number % 2 == 0) return 0; 

    int limit = sqrt(number);
    for (int i = 3; i <= limit; i += 2) {
        if (number % i == 0) {
            return 0; 
        }
    }
    return 1; 
}


enum Status compute_limit(double eps, double *limit) {
    if (eps < 0 || !limit) return STATUS_INVALID_ARG;

    double product = 0.5; 
    double current = log(2.0) * product;
    double previous = 0.0;
    int p = 2;

    do {
        previous = current;
        do { p++; } while (!is_prime(p));
        product *= (p - 1.0) / p;
        current = log((double)p) * product;
        
        if (p > 100000000) return STATUS_LIMIT; 
        
    } while (fabs(previous - current) > eps * 0.01);

    *limit = current;
    return STATUS_OK;
}

enum Status eq_gamma(double eps, double *result) {
    if (eps < 0 || !result) return STATUS_INVALID_ARG;

    double L;
    enum Status status = compute_limit(eps, &L); 
    if (status != STATUS_OK) return status;

    double a = 0.0;
    double b = 10;
    
    double fa = exp(-a) - L; 
    double fb = exp(-b) - L; 
    
    if (fa * fb >= 0) return STATUS_INVALID_ARG; 

    double c;
    while ((b - a) / 2.0 > eps) {
        c = (a + b) / 2.0;
        double fc = exp(-c) - L;

        if (fabs(fc) < 1e-15) break;

        if (fa * fc < 0) {
            b = c;
            fb = fc;
        } else {
            a = c;
            fa = fc;
        }
    }

    *result = (a + b) / 2.0;
    return STATUS_OK;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Использование: %s <eps>\n", argv[0]);
        return STATUS_INVALID_ARGC;
    }

    enum Status st = 0;
    double eps = 0.0;
    if ((st = parse_float(argv[1], &eps)) != 0) {
            print_status(st, argv[0]);
            return st;
        }
    if (eps < 1e-9 || eps > 0.1) {
        printf("Error: недопустимое значение эпсилон [1e-9 - 0.1]\n");
        return STATUS_INVALID_FLOAT;
    }
    
    printf("Вычисление при помощи пределов:\n");
    enum Status (*funcs[])(long, double *) = {lim_e, lim_pi, lim_ln2};
    char *names[] = {"e", "pi", "ln2"};
    double res = 0.0;

    for (int i = 0; i < 3; ++i) {
        if ((st = compute_by_limit(funcs[i], eps, &res)) != 0) {
            print_status(st, argv[0]);
            return st;
        } else {
            printf("%s = %.8f\n", names[i], res);
        }
    }

    res = 0.0;
    if ((st = lim_sqrt2(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("sqrt2 = %.8f\n", res);
    }

    res = 0.0;
    if ((st = lim_gamma(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("gamma = %.8f\n", res);
    }

    printf("Вычисление при помощи рядов:\n");
    res = 0.0;
    if ((st = row_e(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("e = %.8f\n", res);
    }

    res = 0.0;
    if ((st = row_pi(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("pi = %.8f\n", res);
    }

    res = 0.0;
    if ((st = row_ln2(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("ln2 = %.8f\n", res);
    }

    res = 0.0;
    if ((st = row_sqrt2(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("sqrt2 = %.8f\n", res);
    }

    res = 0.0;
    if ((st = row_gamma(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("gamma = %.8f\n", res);
    }

    printf("Вычисление при помощи уравнений:\n");

    res = 0.0;
    if ((st = eq_e(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("e = %.8f\n", res);
    }

    res = 0.0;
    if ((st = eq_pi(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("pi = %.8f\n", res);
    }

    res = 0.0;
    if ((st = eq_ln2(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("ln2 = %.8f\n", res);
    }

    res = 0.0;
    if ((st = eq_sqrt2(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("sqrt2 = %.8f\n", res);
    }

    res = 0.0;
    if ((st = eq_gamma(eps, &res)) != 0) {
        print_status(st, argv[0]);
        return st;
    } else {
        printf("gamma = %.8f\n", res);
    }
}