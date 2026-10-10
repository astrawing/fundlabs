#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <limits.h>

#define SZ 32

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR_INVALID_ARGS,
    STATUS_ERROR_ARG_COUNT,
    STATUS_ERROR_MEMORY_ALLOC,
    STATUS_ERROR_IO,
    STATUS_ERROR_EMPTY_DATA,
    STATUS_ERROR_OVERFLOW,
    STATUS_ERROR_INVALID_STRING 
} Status;

void handle_error(const Status status) {
    switch (status) {
        case STATUS_ERROR_INVALID_ARGS:
            fprintf(stderr, "Ошибка: неверные аргументы\n");
            break;
        case STATUS_ERROR_ARG_COUNT:
            fprintf(stderr, "Ошибка: неверное количество аргументов\n");
            break;
        case STATUS_ERROR_MEMORY_ALLOC:
            fprintf(stderr, "Ошибка: не удалось выделить память\n");
            break;
        case STATUS_ERROR_IO:
            fprintf(stderr, "Ошибка: ошибка ввода-вывода\n");
            break;
        case STATUS_ERROR_EMPTY_DATA:
            fprintf(stderr, "Ошибка: пустые данные\n");
            break;
        case STATUS_ERROR_OVERFLOW:
            fprintf(stderr, "Ошибка: переполнение\n");
            break;
        case STATUS_ERROR_INVALID_STRING:
            fprintf(stderr, "Ошибка: неверный формат строки\n");
            break;
        case STATUS_OK:
        default:
            break;
    }
}

Status read_number(char *buf, size_t buf_size) {
    if (buf == NULL || buf_size < 1) {
        return STATUS_ERROR_INVALID_ARGS;
    }
    int c;
    size_t i = 0;
    
    while ((c = getchar()) != EOF && isspace(c));

    if (c == '-') {
        if (i >= buf_size - 1) {
            return STATUS_ERROR_OVERFLOW;
        }
        buf[i++] = '-';
        c = getchar();
    }

    while (c == '0') c = getchar();
    if (isspace(c)) {
        buf[0] = '0';
        buf[1] = '\0';
        return STATUS_OK;
    }
    
    while (c != '\n' && c != EOF && !isspace(c)) {
        if (i >= buf_size - 1) {
            return STATUS_ERROR_OVERFLOW;  
        }

        buf[i++] = c;
        c = getchar();
    }

    while (isspace(c) && c != '\n' && c != EOF) c = getchar();
    if (c != '\n' && c != EOF) {
        buf[0] = '\0';
        return STATUS_ERROR_INVALID_STRING;
    }
    
    buf[i] = '\0';
    return STATUS_OK;
}

Status char_to_digit(char c, int *digit) {
    if (c >= '0' && c <= '9') {
        *digit = c - '0';
    } else if (c >= 'A' && c <= 'Z') {
        *digit = c - 'A' + 10;
    } else {
        return STATUS_ERROR_INVALID_ARGS;
    }
    return STATUS_OK;
}

Status validate_number(const char *buf, int base) {
    if (!buf || base < 2 || base > 36 || *buf == '\0') {
        return STATUS_ERROR_INVALID_ARGS;
    }

    size_t i = 0;

    if (buf[i] == '-') {
        i++;
        if (buf[i] == '\0') {
            return STATUS_ERROR_INVALID_STRING;
        }
    }

    while (buf[i] != '\0') {
        int digit;
        Status st = char_to_digit(buf[i], &digit);
        if (st != STATUS_OK) return st;
        if (digit > base - 1) return STATUS_ERROR_INVALID_STRING;
        i++;
    }

    return STATUS_OK;
}

Status to_long(const char *buf, int base, long *res) {
    if (!buf || !res || base < 2 || base > 36) {
        return STATUS_ERROR_INVALID_ARGS;
    }

    const char *p = buf;
    int is_negative = 0;

    if (*p == '-') {
        is_negative = 1;
        p++;
    }

    unsigned long num = 0;

    while (*p != '\0') {
        int digit;
        Status st = char_to_digit(*p, &digit);
        if (st != STATUS_OK) return st;

        if (num > ULONG_MAX / base) return STATUS_ERROR_OVERFLOW;
        num *= base;

        if (num > ULONG_MAX - digit) return STATUS_ERROR_OVERFLOW;
        num += digit;

        p++;
    }

    if (is_negative) {
        if (-num < (unsigned long)LONG_MIN) {
            return STATUS_ERROR_OVERFLOW;
        }
        *res = -(long)num;
    } else {
        if (num > (unsigned long)LONG_MAX) {
            return STATUS_ERROR_OVERFLOW;
        }
        *res = (long)num;
    }
    
    return STATUS_OK;
}

Status to_base_x(char *buffer, size_t buf_size, long value, int base) {
    if (!buffer || base < 2 || base > 36 || buf_size == 0) {
        return STATUS_ERROR_INVALID_ARGS;
    }

    if (value == 0) {
        if (buf_size < 2) {
            return STATUS_ERROR_OVERFLOW;
        }
        buffer[0] = '0';
        buffer[1] = '\0';
        return STATUS_OK;
    }

    unsigned long num;
    int is_negative = 0;

    if (value < 0) {
        is_negative = 1;
        num = (value == LONG_MIN) ? ((unsigned long)LONG_MAX + 1UL) : (unsigned long)(-value);
    } else {
        num = (unsigned long)value;
    }

    size_t pos = buf_size;
    buffer[--pos] = '\0';

    while (num > 0) {
        if (pos == 0) {
            return STATUS_ERROR_OVERFLOW;
        }

        int digit = num % base;
        num /= base;

        char c;
        if (digit < 10) {
            c = (char)('0' + digit);
        } else {
            c = (char)('A' + digit - 10);
        }

        buffer[--pos] = c;
    }

    if (is_negative) {
        if (pos == 0) {
            return STATUS_ERROR_OVERFLOW;
        }
        buffer[--pos] = '-';
    }

    size_t len = buf_size - pos;
    if (pos != 0) {
        memmove(buffer, buffer + pos, len);
    }

    return STATUS_OK;
}

Status safe_add(long a, long b, long *result) {
    if (!result) return STATUS_ERROR_INVALID_ARGS;

    if (b > 0 && a > LONG_MAX - b) {
        return STATUS_ERROR_OVERFLOW;
    }
    if (b < 0 && a < LONG_MIN - b) {
        return STATUS_ERROR_OVERFLOW;
    }

    *result = a + b;
    return STATUS_OK;
}

int main() {

    char temp[8];
    long t = 0;
    Status st = read_number(temp, 8);
    if (st != STATUS_OK) {
        handle_error(st);
        return st;
    }
    st = validate_number(temp, 10);
    if (st != STATUS_OK) {
        fprintf(stderr, "Ошибка: неверное основание системы счисление [2-36]\n");
        return st;
    }
    st = to_long(temp, 10, &t);
    if (st != STATUS_OK) {
        handle_error(st);
        return st;
    }
    int base = (int)t;
    if (base < 2 || base > 36) {
        fprintf(stderr, "Ошибка: неверное основание системы счисление [2-36]\n");
        return st;
    }

    char buf[SZ];
    long sum = 0;
    long max_value = 0;
    unsigned long max_abs = 0;
    int has_numbers = 0;

    while (1) {
        Status st = read_number(buf, SZ);
        if (st != STATUS_OK) {
            handle_error(st);
            return st;
        }

        if (strcmp(buf, "Stop") == 0) {
            break;
        }

        st = validate_number(buf, base);
        if (st != STATUS_OK) {
            handle_error(st);
            return st;
        }

        long value;
        st = to_long(buf, base, &value);
        if (st != STATUS_OK) {
            handle_error(st);
            return st;
        }

        st = safe_add(sum, value, &sum);
        if (st != STATUS_OK) {
            handle_error(st);
            return st;
        }

        unsigned long abs_value = labs(value);
        if (abs_value > max_abs) {
            max_abs = abs_value;
            max_value = value;
        }
    }

    char out_buf[SZ];

    st = to_base_x(out_buf, SZ, max_value, base);
    if (st != STATUS_OK) {
        handle_error(st);
        return st;
    }
    printf("Максимум по модулю: %s\n", out_buf);

    st = to_base_x(out_buf, SZ, sum, base);
    if (st != STATUS_OK) {
        handle_error(st);
        return st;
    }
    printf("Сумма: %s\n", out_buf);

    int bases[] = {9, 18, 27, 36};
    for (int i = 0; i < 4; i++) {
        st = to_base_x(out_buf, SZ, max_value, bases[i]);
        if (st != STATUS_OK) {
            handle_error(st);
            return st;
        }
        printf("Максимум в СС %d: %s\n", bases[i], out_buf);

        st = to_base_x(out_buf, SZ, sum, bases[i]);
        if (st != STATUS_OK) {
            handle_error(st);
            return st;
        }
        printf("Сумма в СС %d: %s\n", bases[i], out_buf);
    }

    return STATUS_OK;
}