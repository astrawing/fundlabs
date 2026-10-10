#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <limits.h>

enum status_code {
    STATUS_OK = 0,
    STATUS_ERR_ARGS_COUNT,      
    STATUS_ERR_FILE_NOT_EXISTS, 
    STATUS_ERR_FILE_OPEN,      
    STATUS_ERR_FILE_READ,        
    STATUS_ERR_FILE_WRITE,       
    STATUS_ERR_INVALID_DATA,    
    STATUS_ERR_BUFFER_OVERFLOW,  
    STATUS_ERR_OVERFLOW,       
    STATUS_ERR_SAME_PATHS,     
    STATUS_ERR_MEMORY            
};

struct file_paths {
    char in[PATH_MAX];
    char out[PATH_MAX];
};

void print_status(const enum status_code st) {
    switch (st) {
        case STATUS_OK:
            break;
        case STATUS_ERR_ARGS_COUNT:
            fprintf(stderr, "Error: неверное количество аргументов.\n");
            break;
        case STATUS_ERR_FILE_NOT_EXISTS:
            fprintf(stderr, "Error: входной файл не существует.\n");
            break;
        case STATUS_ERR_FILE_OPEN:
            fprintf(stderr, "Error: не удалось открыть файл.\n");
            break;
        case STATUS_ERR_FILE_READ:
            fprintf(stderr, "Error: ошибка чтения из файла.\n");
            break;
        case STATUS_ERR_FILE_WRITE:
            fprintf(stderr, "Error: ошибка записи в файл.\n");
            break;
        case STATUS_ERR_INVALID_DATA:
            fprintf(stderr, "Error: некорректные входные данные.\n");
            break;
        case STATUS_ERR_BUFFER_OVERFLOW:
            fprintf(stderr, "Error: переполнение буфера токена.\n");
            break;
        case STATUS_ERR_OVERFLOW:
            fprintf(stderr, "Error: число слишком велико (переполнение long long).\n");
            break;
        case STATUS_ERR_SAME_PATHS:
            fprintf(stderr, "Error: входной и выходной файлы совпадают.\n");
            break;
        case STATUS_ERR_MEMORY:
            fprintf(stderr, "Error: не удалось выделить память.\n");
            break;
        default:
            fprintf(stderr, "Error: неизвестная ошибка.\n");
            break;
    }
}

enum status_code validate_args(int argc, char **argv, struct file_paths *paths) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return STATUS_ERR_ARGS_COUNT;
    }

    if(realpath(argv[1], paths->in) == NULL) { return STATUS_ERR_FILE_NOT_EXISTS; }
    realpath(argv[2], paths->out);

    if (strcmp(paths->in, paths->out) == 0) return STATUS_ERR_SAME_PATHS;

    return STATUS_OK;
} 

enum status_code open_files(const struct file_paths *paths, FILE **in_file, FILE **out_file) {
    if (paths == NULL || in_file == NULL || out_file == NULL) {
        return STATUS_ERR_INVALID_DATA;  
    }

    *in_file = NULL;
    *out_file = NULL;

    *in_file = fopen(paths->in, "r");
    if (*in_file == NULL) {
        return STATUS_ERR_FILE_OPEN;
    }

    *out_file = fopen(paths->out, "w");
    if (*out_file == NULL) {
        fclose(*in_file);  
        *in_file = NULL;
        return STATUS_ERR_FILE_OPEN;
    }

    return STATUS_OK;
}

enum status_code read_token(FILE *f, char *buf, size_t buf_size) {
    if (f == NULL || buf == NULL || buf_size < 1) {
        return STATUS_ERR_INVALID_DATA;
    }
    int c;
    size_t i = 0;
    
    while ((c = fgetc(f)) != EOF && isspace(c));
    
    while (c != EOF && !isspace(c)) {
        if (i >= buf_size - 1) {
            return STATUS_ERR_OVERFLOW;  
        }

        buf[i++] = c;
        c = fgetc(f);
    }
    
    buf[i] = '\0';
    return STATUS_OK;
}

enum status_code char_to_digit(char c, int *digit) {
    if (c >= '0' && c <= '9') {
        *digit = c - '0';
    } else if (c >= 'a' && c <= 'z') {
        *digit = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'Z') {
        *digit = c - 'A' + 10;
    } else {
        return STATUS_ERR_INVALID_DATA;
    }
    return STATUS_OK;
}

enum status_code process_token(const char *token, int *min_base, long long *decimal_value, int *sign) {
    if (token == NULL || min_base == NULL || decimal_value == NULL) {
        return STATUS_ERR_INVALID_DATA;
    }
    
    const char *p = token;
    *sign = 1;
    if (*p == '-') {
        *sign = -1;
        ++p;
        if (*p == '\0') {
            return STATUS_ERR_INVALID_DATA;
        }
    }
    
    int max_digit = -1;

    for (*p; *p != '\0'; ++p) {
        int digit;
        enum status_code status = char_to_digit(*p, &digit);
        if (status != STATUS_OK) {
            return status;
        }
        if (digit > max_digit) {
            max_digit = digit;
        }
    }
    
    int base = (max_digit + 1 < 2) ? 2 : max_digit + 1;
    *min_base = base;
    
    const char *q = token;
    if (*sign == -1) q++;
    long long value = 0;
    for (*q; *q != '\0'; ++q) {
        int digit;
        enum status_code status = char_to_digit(*q, &digit);
        if (status != STATUS_OK) {
            return status;
        }
        
        if (value > (LLONG_MAX - digit) / base) {
            return STATUS_ERR_OVERFLOW;
        }
        
        value = value * base + digit;
    }
    
    *decimal_value = value;
    return STATUS_OK;
}

enum status_code write_result(FILE *fout, const char *token, int min_base, long long decimal_value, int sign) {
    if (fout == NULL || token == NULL) {
        return STATUS_ERR_INVALID_DATA;
    }
    
    const char *p = token;
    if (sign == -1) p++;
    while (*p == '0' && *(p + 1) != '\0') {
        ++p;
    }
    
    if ((sign == -1) && (*p == '0')) sign = 1;
    if (fprintf(fout, "%s%s %d %s%lld\n", (sign == 1 ? "" : "-"), p, min_base, (sign == 1 ? "" : "-"), decimal_value) < 0) {
        return STATUS_ERR_FILE_WRITE;
    }
    
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    struct file_paths paths;
    FILE *fin = NULL, *fout = NULL;
    enum status_code st = validate_args(argc, argv, &paths);
    if (st != STATUS_OK) {
        print_status(st);
        return st;
    }

    st = open_files(&paths, &fin, &fout);
    if (st != STATUS_OK) {
        print_status(st);
        return st;
    }

    char token_buf[BUFSIZ];
    while (1) {
        st = read_token(fin, token_buf, BUFSIZ);
        if (st != STATUS_OK) {
            printf("123\n");
            print_status(st);
            break;
        }

        if (token_buf[0] == '\0') break;

        int min_base = 0;
        long long decimal_value = 0;
        int sign = 1;

        st = process_token(token_buf, &min_base, &decimal_value, &sign);
        if (st != STATUS_OK) {
            print_status(st);
            break;
        }
        if (!sign) token_buf[0] = '0';

        st = write_result(fout, token_buf, min_base, decimal_value, sign);
        if (st != STATUS_OK) {
            print_status(st);
            break;
        }
    }

    fclose(fin);
    fclose(fout);

    return 0;
}