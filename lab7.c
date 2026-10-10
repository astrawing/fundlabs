#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <limits.h>

enum status_code {
    STATUS_OK = 0,
    STATUS_ERR_ARGS_COUNT,
    STATUS_ERR_FLAG,
    STATUS_ERR_MEMORY,
    STATUS_ERR_FILE_OPEN,
    STATUS_ERR_FILE_READ,
    STATUS_ERR_FILE_WRITE,
    STATUS_ERR_INVALID_DATA,
    STATUS_ERR_EMPTY_INPUT,
    STATUS_ERR_FILE_NOT_EXISTS,
    STATUS_ERR_BUFFER_OVERFLOW,
    STATUS_ERR_FILES_EQUAL
};

enum flag_type {
    FLAG_R,
    FLAG_A,
    FLAG_UNKNOWN
};

struct token_list {
    char **items;
    size_t count;
    size_t capacity;
};

struct file_paths {
    char in1[PATH_MAX];
    char in2[PATH_MAX];
    char out[PATH_MAX];
};

static enum flag_type parse_flag(const char *arg) {
    if (arg == NULL || strlen(arg) < 2) {
        return FLAG_UNKNOWN;
    }
    if (arg[0] != '-' && arg[0] != '/') {
        return FLAG_UNKNOWN;
    }
    if (arg[1] == 'r') {
        return FLAG_R;
    }
    if (arg[1] == 'a') {
        return FLAG_A;
    }
    return FLAG_UNKNOWN;
}

enum status_code to_base4_string(const char *src, char **out) {
    if (src == NULL || out == NULL) {
        return STATUS_ERR_INVALID_DATA;
    }
    size_t len = strlen(src);
    if (len == 0) {
        *out = (char *)malloc(1);
        if (*out == NULL) return STATUS_ERR_MEMORY;
        (*out)[0] = '\0';
        return STATUS_OK;
    }

    size_t buf_size = len * 5 + 1;
    char *buf = (char *)malloc(buf_size);
    if (buf == NULL) {
        return STATUS_ERR_MEMORY;
    }

    size_t pos = 0;
    for (size_t i = 0; i < len; ++i) {
        char byte = src[i];
        if (i > 0) {
            buf[pos++] = ' ';
        }
        char digits[4];
        digits[0] = (char)('0' + ((byte >> 6) & 0x3));
        digits[1] = (char)('0' + ((byte >> 4) & 0x3));
        digits[2] = (char)('0' + ((byte >> 2) & 0x3));
        digits[3] = (char)('0' + (byte & 0x3));
        size_t start = 0;
        while (start < 3 && digits[start] == '0') ++start;
        for (size_t k = start; k < 4; ++k) {
            buf[pos++] = digits[k];
        }
    }
    buf[pos] = '\0';
    *out = buf;
    return STATUS_OK;
}

enum status_code to_base8_string(const char *src, char **out) {
    if (src == NULL || out == NULL) {
        return STATUS_ERR_INVALID_DATA;
    }
    size_t len = strlen(src);
    if (len == 0) {
        *out = (char *)malloc(1);
        if (*out == NULL) return STATUS_ERR_MEMORY;
        (*out)[0] = '\0';
        return STATUS_OK;
    }

    size_t buf_size = len * 4 + 1;
    char *buf = (char *)malloc(buf_size);
    if (buf == NULL) {
        return STATUS_ERR_MEMORY;
    }
    
    size_t pos = 0;
    for (size_t i = 0; i < len; ++i) {
        char byte = src[i];
        if (i > 0) {
            buf[pos++] = ' ';
        }
        char digits[3];
        digits[0] = (char)('0' + ((byte >> 6) & 0x7));
        digits[1] = (char)('0' + ((byte >> 3) & 0x7));
        digits[2] = (char)('0' + (byte & 0x7));
        size_t start = 0;
        while (start < 3 && digits[start] == '0') ++start;
        for (size_t k = start; k < 3; ++k) {
            buf[pos++] = digits[k];
        }
    }
    buf[pos] = '\0';
    *out = buf;
    return STATUS_OK;
}

enum status_code to_lower_latin(const char *src, char **out) {
    if (src == NULL || out == NULL) {
        return STATUS_ERR_INVALID_DATA;
    }
    size_t len = strlen(src);
    char *buf = (char *)malloc(len + 1);
    if (buf == NULL) {
        return STATUS_ERR_MEMORY;
    }
    for (size_t i = 0; i < len; ++i) {
        char ch = src[i];
        if (ch >= 'A' && ch <= 'Z') {
            buf[i] = ch - 'A' + 'a';
        } else {
            buf[i] = ch;
        }
    }
    buf[len] = '\0';
    *out = buf;
    return STATUS_OK;
}

enum status_code validate_args_r(int argc, char **argv, struct file_paths *paths) {
    if (argc != 5 || argv == NULL || paths == NULL) {
        return STATUS_ERR_ARGS_COUNT;
    }
    if(realpath(argv[2], paths->in1) == NULL) { return STATUS_ERR_FILE_NOT_EXISTS; } 
    if(realpath(argv[3], paths->in2) == NULL) { return STATUS_ERR_FILE_NOT_EXISTS; } 
    realpath(argv[4], paths->out);
    
    if (strcmp(paths->in1, paths->out) == 0) return STATUS_ERR_FILES_EQUAL;
    if (strcmp(paths->in2, paths->out) == 0) return STATUS_ERR_FILES_EQUAL;
    if (strcmp(paths->in1, paths->in2) == 0) return STATUS_ERR_FILES_EQUAL;
    return STATUS_OK;
}

enum status_code validate_args_a(int argc, char **argv, struct file_paths *paths) {
    if (argc != 4 || argv == NULL || paths == NULL) {
        return STATUS_ERR_ARGS_COUNT;
    }
    if(realpath(argv[2], paths->in1) == NULL) { return STATUS_ERR_FILE_NOT_EXISTS; }
    realpath(argv[3], paths->out);

    if (paths->in1 == NULL || paths->out == NULL) {
        return STATUS_ERR_FILES_EQUAL;
    }
    return STATUS_OK;
}

enum status_code read_token(FILE *f, char *buf, size_t buf_size, int *end) {
    if (f == NULL || buf == NULL || buf_size == 0 || end == NULL) {
        return STATUS_ERR_INVALID_DATA;
    }

    *end = 0;
    int c;

    do {
        c = fgetc(f);
        if (c == EOF) {
            *end = -1;         
            return STATUS_OK;
        }
    } while (isspace((unsigned char)c));

    size_t len = 0;
    for (;;) {
        if (!isspace(c)) {  
            if (len + 1 < buf_size) {
                buf[len++] = c;
            } else {
                while ((c = fgetc(f)) != EOF && !isspace(c))
                buf[0] = '\0';
                return STATUS_ERR_BUFFER_OVERFLOW;
            }
        } else {
            break;
        }
        c = fgetc(f);
        if (c == EOF) {
            *end = 1;         
            break;
        }
    }

    buf[len] = '\0';
    return STATUS_OK;
}

enum status_code write_token(FILE *f, const char *token) {
    if (f == NULL || token == NULL) {
        return STATUS_ERR_INVALID_DATA;
    }
    
    if (fputs(token, f) == EOF || fputc(' ', f) == EOF) {
        return STATUS_ERR_FILE_WRITE;
    }

    return STATUS_OK;
}

enum status_code flag_r(const struct file_paths *paths) {
    if (paths == NULL || paths->in1 == NULL ||
        paths->in2 == NULL || paths->out == NULL) {
        return STATUS_ERR_INVALID_DATA;
    }

    FILE *in1 = NULL;
    FILE *in2 = NULL;
    FILE *out = NULL;
    enum status_code st = STATUS_OK;

    in1 = fopen(paths->in1, "r");
    if (in1 == NULL) {
        return STATUS_ERR_FILE_OPEN;
    }
    in2 = fopen(paths->in2, "r");
    if (in2 == NULL) {
        fclose(in1);
        return STATUS_ERR_FILE_OPEN;
    }
    out = fopen(paths->out, "w");
    if (out == NULL) {
        fclose(in1);
        fclose(in2);
        return STATUS_ERR_FILE_OPEN;
    }

    char buffer[BUFSIZ];
    int turn = 0;    
    int exhausted1 = 0, exhausted2 = 0;

    while (!exhausted1 || !exhausted2) {
        FILE *cur = (turn == 0) ? in1 : in2;
        int *cur_ex = (turn == 0) ? &exhausted1 : &exhausted2;
        int *oth_ex = (turn == 0) ? &exhausted2 : &exhausted1;

        if (*cur_ex) {     
            turn = !turn;
            continue;
        }

        int end = 0;
        st = read_token(cur, buffer, sizeof(buffer), &end);
        if (st != STATUS_OK) break;

        if (end == -1) {       
            *cur_ex = 1;
            turn = !turn;
            continue;
        }

        st = write_token(out, buffer);
        if (st != STATUS_OK) break;

        if (end == 1) {   
            *cur_ex = 1;
            if (!*oth_ex) turn = !turn;
            continue;
        }

        if (!*oth_ex) {
            turn = !turn;
        }
    }

    if (fclose(out) != 0 && st == STATUS_OK) {
        st = STATUS_ERR_FILE_WRITE;
    }
    fclose(in1);
    fclose(in2);
    return st;
}

void print_status(const enum status_code st) {
    switch (st) {
        case STATUS_OK:
            break;
        case STATUS_ERR_ARGS_COUNT:
            fprintf(stderr, "Error: неверное количество аргументов.\n");
            break;

        case STATUS_ERR_FLAG:
            fprintf(stderr, "Error: неизвестный флаг. "
                            "Ожидается '-r'/'/r' или '-a'/'/a'.\n");
            break;

        case STATUS_ERR_MEMORY:
            fprintf(stderr, "Error: не удалось выделить память.\n");
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

        case STATUS_ERR_EMPTY_INPUT:
            fprintf(stderr, "Error: входные данные пусты.\n");
            break;

        case STATUS_ERR_FILE_NOT_EXISTS:
            fprintf(stderr, "Error: файла не существует.\n");
            break;

        case STATUS_ERR_BUFFER_OVERFLOW:
            fprintf(stderr, "Error: переполнение буфера.\n");
            break;

        case STATUS_ERR_FILES_EQUAL:
            fprintf(stderr, "Error: файлы равны.\n");
            break; 

        default:
            fprintf(stderr, "Error: неизвестная ошибка.\n");
            break;
    }
}

enum status_code transform_token_a(const char *token, size_t num, char **out) {
    if (token == NULL || out == NULL || num == 0) {
        return STATUS_ERR_INVALID_DATA;
    }
    *out = NULL;

    enum status_code st;

    if (num % 10 == 0) {
        char *temp = NULL;
        st = to_lower_latin(token, &temp);
        if (st != STATUS_OK) {
            return st;
        }
        st = to_base4_string(temp, out);
        free(temp);
        return st;
    }
    if (num % 2 == 0) {
        return to_lower_latin(token, out);
    }
    if (num % 5 == 0) {
        return to_base8_string(token, out);
    }

    size_t len = strlen(token);
    char *copy = (char *)malloc(len + 1);
    if (copy == NULL) {
        return STATUS_ERR_MEMORY;
    }
    memcpy(copy, token, len + 1);
    *out = copy;
    return STATUS_OK;
}

enum status_code flag_a(const struct file_paths *paths) {
    if (paths == NULL || paths->in1 == NULL || paths->out == NULL) {
        return STATUS_ERR_INVALID_DATA;
    }

    FILE *in = NULL;
    FILE *out = NULL;
    enum status_code st = STATUS_OK;

    in = fopen(paths->in1, "r");
    if (in == NULL) {
        return STATUS_ERR_FILE_OPEN;
    }
    out = fopen(paths->out, "w");
    if (out == NULL) {
        fclose(in);
        return STATUS_ERR_FILE_OPEN;
    }

    char buffer[BUFSIZ];

    size_t index = 0;             
    int end = 0;
    while (end != 1) {
        st = read_token(in, buffer, sizeof(buffer), &end);
        if (st != STATUS_OK) break;
        if (end == -1) break;

        size_t num = index + 1; 
        char *transformed = NULL;
        st = transform_token_a(buffer, num, &transformed);
        if (st != STATUS_OK) {
            free(transformed);
            break;
        }

        st = write_token(out, transformed);
        free(transformed);
        if (st != STATUS_OK) {
            break;
        }

        index++;
    }

    if (fclose(out) != 0 && st == STATUS_OK) {
        st = STATUS_ERR_FILE_WRITE;
    }
    fclose(in);
    return st;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <-r|-a> ...\n", argv[0]);
        return STATUS_ERR_ARGS_COUNT;
    }

    enum flag_type flag = parse_flag(argv[1]);
    struct file_paths paths;
    enum status_code st = STATUS_OK;

    switch (flag) {
        case FLAG_R:
            st = validate_args_r(argc, argv, &paths);
            if (st == STATUS_OK) {
                st = flag_r(&paths);
            } 
            break;
        case FLAG_A:
            st = validate_args_a(argc, argv, &paths);
            if (st == STATUS_OK) {
                st = flag_a(&paths);
            }
            break;
        case FLAG_UNKNOWN:
        default:
            fprintf(stderr, "Error: unknown flag '%s'\n", argv[1]);
            return STATUS_ERR_FLAG;
    }

    if (st != STATUS_OK) {
        print_status(st);
        return st;
    }
    return STATUS_OK;
}