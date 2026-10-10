#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

enum Status {
    STATUS_OK = 0,
    STATUS_ERR_ARGC,           
    STATUS_ERR_FLAG_FORMAT,    
    STATUS_ERR_FLAG_UNKNOWN,   
    STATUS_ERR_NUMBER_FORMAT,  
    STATUS_ERR_NUMBER_RANGE,   
    STATUS_ERR_NO_MULTIPLES,   
    STATUS_ERR_RANGE_E,        
    STATUS_ERR_OVERFLOW,       
    STATUS_ERR_INVALID_ARG,   
    STATUS_ERR_NOT_SIMPLE,    
    STATUS_ERR_NOT_POSITIVE   
};

enum Status validate_uint(char *str, unsigned long long *out) {
    if (str == NULL || out == NULL || *str == '\0') {
        return STATUS_ERR_NUMBER_FORMAT;
    }
    for (char *p = str; *p != '\0'; ++p) {
        if (!isdigit(*p)) {
            if (*p == '-') {
                return STATUS_ERR_NOT_POSITIVE;
            }
            return STATUS_ERR_NUMBER_FORMAT;
        }
    }

    unsigned long long result = 0;
    for (char *p = str; *p != '\0'; ++p) {
        unsigned int digit = (unsigned int)(*p - '0');
        if (result * 10 + digit < result) {
            return STATUS_ERR_NUMBER_RANGE;
        }
        result = result * 10 + digit;
    }
    *out = result;
    return STATUS_OK;
}

enum Status validate_flag(char *flag_str) {
    if (flag_str == NULL) {
        return STATUS_ERR_INVALID_ARG;
    }
    if (flag_str[0] != '-' && flag_str[0] != '/') {
        return STATUS_ERR_FLAG_FORMAT;
    }
    if (flag_str[1] == '\0' || flag_str[2] != '\0') {
        return STATUS_ERR_FLAG_UNKNOWN;
    }
    char f = flag_str[1];
    if (f != 'h' && f != 'p' && f != 's' &&
        f != 'e' && f != 'a' && f != 'f') {
        return STATUS_ERR_FLAG_UNKNOWN;
    }
    return STATUS_OK;
}

enum Status do_h(unsigned long long x) {
    if (x == 0 || x > 100) {
        return STATUS_ERR_NO_MULTIPLES;
    }

    printf("Натуральные числа в пределах 100, кратные %llu:\n", x);
    for (int i = 1; i <= 100; ++i) {
        if (i % x == 0) {
            printf("%d ", i);
        }
    }
    printf("\n");

    return STATUS_OK;
}

enum Status do_p(unsigned long long x, int *is_prime_res) {
    if (is_prime_res == NULL) return STATUS_ERR_INVALID_ARG;
    if (x == 1 || x == 0) return STATUS_ERR_NOT_SIMPLE;

    for (unsigned long long i = 2; i * i <= x; i++) {
        if (x % i == 0) {
            *is_prime_res = 0;
            return STATUS_OK;
        }
    }
    *is_prime_res = 1;
    return STATUS_OK;
}

enum Status do_s(unsigned long long x) {
    if (x == 0) {
        printf("0\n");
        return STATUS_OK;
    }

    char buf[16];
    size_t len = 15;
    unsigned long long v = x;
    while (v > 0) {
        const int d = v % 16;
        buf[len--] = (d < 10) ? '0' + d : 'A' + d - 10;
        v /= 16;
    }

    for (int i = len + 1; i < 16; i++) {
        printf("%c", buf[i]);
        if (i > 1) printf(" ");
    }
    printf("\n");
    return STATUS_OK;
}

enum Status do_e(unsigned long long x) {
    printf("Таблица степеней (основания 1..10, показатели 1..%llu):\n", x);
    for (unsigned long long base = 1; base <= 10; ++base) {
        printf("%2llu: ", base);
        unsigned long long val = 1;
        for (unsigned long long exp = 1; exp <= x; ++exp) {
            val *= base;
            printf("%llu ", val);
        }
        printf("\n");
    }
    return STATUS_OK;
}

enum Status do_a(unsigned long long x) {
    unsigned long long sum = x*(x+1)/2;
    printf("Сумма чисел от 1 до %llu = %llu\n", x, sum);
    return STATUS_OK;
}

enum Status do_f(unsigned long long x) {
    unsigned long long fact = 1;
    for (unsigned long long i = 2; i <= x; ++i) {
        if (fact > (~0ULL) / i) return STATUS_ERR_OVERFLOW;
        fact *= i;
    }
    printf("%llu! = %llu\n", x, fact);
    return STATUS_OK;
}

void print_status_error(enum Status st) {
    switch (st) {
        case STATUS_ERR_ARGC:
            fprintf(stderr, "неверное число аргументов командной строки.\n");
            break;
        case STATUS_ERR_FLAG_FORMAT:
            fprintf(stderr, "флаг должен начинаться с '-' или '/'.\n");
            break;
        case STATUS_ERR_FLAG_UNKNOWN:
            fprintf(stderr, "неизвестный или некорректный флаг.\n");
            break;
        case STATUS_ERR_NUMBER_FORMAT:
            fprintf(stderr, "число должно состоять только из цифр.\n");
            break;
        case STATUS_ERR_NUMBER_RANGE:
            fprintf(stderr, "число вне допустимого диапазона.\n");
            break;
        case STATUS_ERR_NO_MULTIPLES:
            fprintf(stderr, "чисел, удовлетворяющих условию, не найдено.\n");
            break;
        case STATUS_ERR_RANGE_E:
            fprintf(stderr, "для флага -e число x должно быть не больше 10.\n");
            break;
        case STATUS_ERR_OVERFLOW:
            fprintf(stderr, "переполнение при вычислениях.\n");
            break;
        case STATUS_ERR_INVALID_ARG:
            fprintf(stderr, "некорректный аргумент функции.\n");
            break;
        case STATUS_ERR_NOT_POSITIVE:
            fprintf(stderr, "допускаются только неотрицательные числа.\n");
        case STATUS_ERR_NOT_SIMPLE:
            fprintf(stderr, "число не простое.\n");
        case STATUS_OK:
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <число> <флаг>\n", argv[0]);
        fprintf(stderr, "Флаги: -h, -p, -s, -e, -a, -f\n");
        return STATUS_ERR_ARGC;
    }

    char *num_str  = argv[1];
    char *flag_str = argv[2];

    enum Status st = validate_flag(flag_str);
    if (st != STATUS_OK) {
        print_status_error(st);
        return st;
    }
    char flag = flag_str[1];

    unsigned long long x = 0;
    st = validate_uint(num_str, &x);
    if (st != STATUS_OK) {
        print_status_error(st);
        return st;
    }

    switch (flag) {
        case 'h':
            st = do_h(x);
            if (st != STATUS_OK) {
                print_status_error(st);
            }
            break;

        case 'p': {
            int is_prime_res = 0;
            st = do_p(x, &is_prime_res);
            if (st == STATUS_OK) {
                if (is_prime_res) {
                    printf("Число %llu является простым.\n", x);
                } else {
                    printf("Число %llu является составным.\n", x);
                }
            } else if (st == STATUS_ERR_NOT_SIMPLE) {
                printf("Число %llu не является простым или составным.\n", x);
            } else {
                print_status_error(st);
            }
            break;
        }

        case 's':
            st = do_s(x);
            if (st != STATUS_OK) {
                print_status_error(st);
            }
            break;

        case 'e':
            if (x > 10) {
                print_status_error(STATUS_ERR_RANGE_E);
                return STATUS_ERR_RANGE_E;
            }
            st = do_e(x);
            if (st != STATUS_OK) {
                print_status_error(st);
            }
            break;

        case 'a': {
            st = do_a(x);
            if (st != STATUS_OK) {
                print_status_error(st);
            }
            break;
        }

        case 'f': {
            st = do_f(x);
            if (st == STATUS_OK) {
                
            } else {
                print_status_error(st);
            }
            break;
        }

        default:
            print_status_error(STATUS_ERR_FLAG_UNKNOWN);
            return STATUS_ERR_FLAG_UNKNOWN;
    }
    return st;
}