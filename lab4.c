#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

enum Status {
    STATUS_OK = 0,
    STATUS_ERR_ARGC,          
    STATUS_ERR_FLAG,         
    STATUS_ERR_OPEN_INPUT,   
    STATUS_ERR_OPEN_OUTPUT,  
    STATUS_ERR_ALLOC,        
    STATUS_ERR_READ,           
    STATUS_ERR_WRITE,           
    STATUS_ERR_FILE_EQUAL
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
        case STATUS_ERR_OPEN_INPUT:
            fprintf(stderr, "%s: не удалось открыть входной файл\n", prefix);
            break;
        case STATUS_ERR_OPEN_OUTPUT:
            fprintf(stderr, "%s: не удалось открыть выходной файл\n", prefix);
            break;
        case STATUS_ERR_ALLOC:
            fprintf(stderr, "%s: не удалось выделить память\n", prefix);
            break;
        case STATUS_ERR_READ:
            fprintf(stderr, "%s: ошибка чтения из файла\n", prefix);
            break;
        case STATUS_ERR_WRITE:
            fprintf(stderr, "%s: ошибка записи в файл\n", prefix);
            break;
        case STATUS_ERR_FILE_EQUAL:
            fprintf(stderr, "%s: имена файлов не различаются\n", prefix);
            break; 
        default:
            fprintf(stderr, "%s: неизвестный статус-код (%d)\n", prefix, st);
            break;
    }
}

enum Status validate_flag(int argc, char *argv[],
                          char *flag_out, int *need_out_path)
{
    if (argc < 3 || argv == NULL || argv[1] == NULL)
        return STATUS_ERR_ARGC;

    char *flag = argv[1];

    if (flag[0] != '-' && flag[0] != '/')
        return STATUS_ERR_FLAG;

    if (flag[1] == '\0')
        return STATUS_ERR_FLAG;

    int idx = 1;
    int has_n = 0;

    if (flag[idx] == 'n') {
        has_n = 1;
        ++idx;
    }

    if (has_n && strcmp(argv[2], argv[3]) == 0) 
        return STATUS_ERR_FILE_EQUAL;

    char action = flag[idx];
    ++idx;

    if (flag[idx] != '\0')
        return STATUS_ERR_FLAG;

    if (action != 'd' && action != 'i' && action != 's' && action != 'a')
        return STATUS_ERR_FLAG;

    if (has_n) {
        if (argc != 4)
            return STATUS_ERR_ARGC;
        *need_out_path = 1;
    } else {
        if (argc != 3)
            return STATUS_ERR_ARGC;
        *need_out_path = 0;
    }

    *flag_out = action;
    return STATUS_OK;
}

enum Status make_output_name(char *input, char **out)
{
    char *prefix = "out_";
    size_t prefix_len = strlen(prefix);
    size_t s_len = strlen(input);
    
    char *result = malloc(prefix_len + s_len + 1);
    if (result == NULL)
        return STATUS_ERR_ALLOC;

    strcpy(result, prefix);    
    strcat(result, input); 

    *out = result;
    return STATUS_OK;
}

enum Status process_d(FILE *in, FILE *out)
{
    int ch;
    while ((ch = fgetc(in)) != EOF) {
        if (ch >= '0' && ch <= '9') {
            continue;
        } else {
            fputc(ch, out);
        }
    }
    if (ferror(in))
        return STATUS_ERR_READ;
    return STATUS_OK;
}

enum Status process_i(FILE *in, FILE *out)
{
    int ch;
    int count = 0;

    while ((ch = fgetc(in)) != EOF) {
        if (ch != '\n' && ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))) {
            ++count;
        }
        if (ch == '\n') {
            fprintf(out, "%d\n", count);
            count = 0;
        }
    }
    fprintf(out, "%d\n", count);

    if (ferror(in))
        return STATUS_ERR_READ;

    return STATUS_OK;
}

enum Status process_s(FILE *in, FILE *out)
{
    int ch;
    int count = 0;

    while ((ch = fgetc(in)) != EOF) {
        if (isalnum(ch) || ch == ' ') {
            continue;
        } else if (ch == '\n') {
            count += 1;
            fprintf(out, "%d\n", count);
            count = 0;
        } else {
            count += 1;
        }
    }
    fprintf(out, "%d\n", count);

    if (ferror(in))
        return STATUS_ERR_READ;

    return STATUS_OK;
}

enum Status process_a(FILE *in, FILE *out)
{
    int ch;
    while ((ch = fgetc(in)) != EOF) {
        if (ch >= '0' && ch <= '9') {
            fputc(ch, out);
        } else {
            // if (ch == '\n') fputc(ch, out);
             fprintf(out, "%X", ch);
        }
    }
    if (ferror(in))
        return STATUS_ERR_READ;
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    char flag = '\0';
    int need_out_path = 0;

    enum Status st = validate_flag(argc, argv, &flag, &need_out_path);
    if (st != STATUS_OK) {
        print_status_error(st, argv[0]);
        return st;
    }

    char *input_path = argv[2];
    char *out_path = NULL;

    if (need_out_path) {
        out_path = argv[3];
    } else {
        st = make_output_name(input_path, &out_path);
        if (st != STATUS_OK) {
            print_status_error(st, argv[0]);
            return st;
        }
    }

    FILE *in = fopen(input_path, "r");
    if (in == NULL) {
        print_status_error(STATUS_ERR_OPEN_INPUT, argv[0]);
        return STATUS_ERR_OPEN_INPUT;
    }

    FILE *out = fopen(out_path, "w");
    if (out == NULL) {
        print_status_error(STATUS_ERR_OPEN_OUTPUT, argv[0]);
        fclose(in);
        return STATUS_ERR_OPEN_OUTPUT;
    }

    switch (flag) {
        case 'd':
            st = process_d(in, out);
            break;
        case 'i':
            st = process_i(in, out);
            break;
        case 's':
            st = process_s(in, out);
            break;
        case 'a':
            st = process_a(in, out);
            break;
        default:
            st = STATUS_ERR_FLAG;
    }

    if (fclose(out) != 0 && st == STATUS_OK)
        st = STATUS_ERR_WRITE;
    fclose(in);

    print_status_error(st, argv[0]);
    return st;
}