#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

#define ARRSIZ 20

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR_INVALID_ARGS,
    STATUS_ERROR_ARG_COUNT,
    STATUS_ERROR_MEMORY_ALLOC,
    STATUS_ERROR_IO,
    STATUS_ERROR_EMPTY_DATA,
    STATUS_ERROR_OVERFLOW
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
        case STATUS_OK:
        default:
            break;
    }
}

Status parse_int(const char *str, int *out_value) {
    if (str == NULL || out_value == NULL) {
        return STATUS_ERROR_INVALID_ARGS;
    }
    
    while (isspace(*str)) str++;
    if (*str == '\0') return STATUS_ERROR_INVALID_ARGS;
    
    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    if (!isdigit(*str)) return STATUS_ERROR_INVALID_ARGS;
    
    long long result = 0;
    while (isdigit(*str)) {
        int digit = *str - '0';
        if (result > (INT_MAX - digit) / 10) {
            return STATUS_ERROR_OVERFLOW;
        }
        result = result * 10 + (*str - '0');
        str++;
    }
    
    while (isspace(*str)) str++;
    if (*str != '\0') return STATUS_ERROR_INVALID_ARGS;
    
    result *= sign;
    if (result < INT_MIN || result > INT_MAX) {
        return STATUS_ERROR_INVALID_ARGS;
    }
    
    *out_value = (int)result;
    return STATUS_OK;
}

Status fill_array_random(int *arr, const size_t size, const int a, const int b) {
    if (arr == NULL || a > b) return STATUS_ERROR_INVALID_ARGS;
    if (size == 0) return STATUS_ERROR_EMPTY_DATA;
    
    const int range = b - a + 1;
    for (size_t i = 0; i < size; i++) {
        arr[i] = a + rand() % range;
    }
    
    return STATUS_OK;
}

Status find_and_swap_minmax(int *arr, const size_t size) {
    if (arr == NULL || size == 0) return STATUS_ERROR_INVALID_ARGS;
    
    size_t min_idx = 0;
    size_t max_idx = 0;
    
    for (size_t i = 1; i < size; i++) {
        if (arr[i] < arr[min_idx]) {
            min_idx = i;
        }
        if (arr[i] > arr[max_idx]) {
            max_idx = i;
        }
    }
    
    if (min_idx != max_idx) {
        const int temp = arr[min_idx];
        arr[min_idx] = arr[max_idx];
        arr[max_idx] = temp;
    }
    
    return STATUS_OK;
}

void print_array(const int *arr, const size_t size) {
    if (arr == NULL || size == 0) {
        return;
    }
    
    for (size_t i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

Status parse_args(int argc, char *argv[], int *out_a, int *out_b) {
    if (argv == NULL || out_a == NULL || out_b == NULL) return STATUS_ERROR_INVALID_ARGS;
    if (argc != 3) return STATUS_ERROR_ARG_COUNT;

    int a = 0;
    Status status = parse_int(argv[1], &a);
    if (status != STATUS_OK) return status;

    int b = 0;
    status = parse_int(argv[2], &b);
    if (status != STATUS_OK) return status;

    if (a > b) return STATUS_ERROR_INVALID_ARGS;

    *out_a = a;
    *out_b = b;
    return STATUS_OK;
}

void print_array_preview(const int *arr, const size_t size, const size_t max_items) {
    if (arr == NULL || size == 0) {
        return;
    }
    const size_t limit = size < max_items ? size : max_items;
    for (size_t i = 0; i < limit; i++) {
        printf("%d ", arr[i]);
    }
    if (size > max_items) {
        printf("... (всего %zu элементов)", size);
    }
    printf("\n");
}

Status generate_random_array(int **out_arr, size_t *out_size,
                             const int min_val, const int max_val,
                             const size_t min_size, const size_t max_size) {
    if (out_arr == NULL || out_size == NULL) return STATUS_ERROR_INVALID_ARGS;
    if (min_val > max_val) return STATUS_ERROR_INVALID_ARGS;
    if (min_size > max_size || min_size == 0) return STATUS_ERROR_INVALID_ARGS;

    const size_t size = min_size + (size_t)(rand() % (max_size - min_size + 1));

    int *arr = (int *)malloc(size * sizeof(int));
    if (arr == NULL) {
        return STATUS_ERROR_MEMORY_ALLOC;
    }

    const long range = max_val - min_val + 1;
    for (size_t i = 0; i < size; i++) {
        arr[i] = min_val + rand() % range;
    }

    *out_arr = arr;
    *out_size = size;
    return STATUS_OK;
}

Status copy_array(const int *src, const size_t size, int **out_dst) {
    if (src == NULL || out_dst == NULL) return STATUS_ERROR_INVALID_ARGS;
    if (size == 0) return STATUS_ERROR_EMPTY_DATA;

    int *dst = (int *)malloc(size * sizeof(int));
    if (dst == NULL) {
        return STATUS_ERROR_MEMORY_ALLOC;
    }

    for (size_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }

    *out_dst = dst;
    return STATUS_OK;
}

int compare_ints(const void *pa, const void *pb) {
    const int a = *(const int *)pa;
    const int b = *(const int *)pb;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

Status sort_array(int *arr, const size_t size) {
    if (arr == NULL) return STATUS_ERROR_INVALID_ARGS;
    if (size == 0) return STATUS_ERROR_EMPTY_DATA;

    qsort(arr, size, sizeof(int), compare_ints);
    return STATUS_OK;
}

Status find_closest_in_sorted(const int *sorted_arr, const size_t size,
                              const int target, int *out_value) {
    if (sorted_arr == NULL || out_value == NULL) return STATUS_ERROR_INVALID_ARGS;
    if (size == 0) return STATUS_ERROR_EMPTY_DATA;

    size_t l = 0;
    size_t r = size - 1;
    size_t closest = 0;
    while (l <= r) {
        const size_t mid = l + (r - l) / 2;
        if (abs(sorted_arr[mid] - target) < abs(sorted_arr[closest] - target)) {
            closest = mid;
        }
        if (sorted_arr[mid] == target){
            closest = mid;
            break;
        } else if (sorted_arr[mid] < target) {
            l = mid + 1;
        } else {
            r = mid - 1;
        }
    }

    *out_value = sorted_arr[closest];
    return STATUS_OK;
}

Status form_array_c(const int *arr_a, const size_t size_a,
                    const int *sorted_b, const size_t size_b,
                    int **out_arr_c) {
    if (arr_a == NULL || sorted_b == NULL || out_arr_c == NULL) {
        return STATUS_ERROR_INVALID_ARGS;
    }
    if (size_a == 0 || size_b == 0) return STATUS_ERROR_EMPTY_DATA;

    int *arr_c = (int *)malloc(size_a * sizeof(int));
    if (arr_c == NULL) {
        return STATUS_ERROR_MEMORY_ALLOC;
    }

    for (size_t i = 0; i < size_a; i++) {
        int closest = 0;
        Status status = find_closest_in_sorted(sorted_b, size_b, arr_a[i], &closest);
        if (status != STATUS_OK) {
            free(arr_c);
            return status;
        }
        const long long sum = arr_a[i] + closest;
        if (sum < INT_MIN || sum > INT_MAX) {
            free(arr_c);
            return STATUS_ERROR_OVERFLOW;
        }
        arr_c[i] = (int)sum;
    }

    *out_arr_c = arr_c;
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    int a = 0, b = 0;
    Status status = parse_args(argc, argv, &a, &b);
    if (status == STATUS_ERROR_ARG_COUNT) {
        fprintf(stderr, "Использование: %s <a> <b>\n", argv[0]);
        return status;
    } else if (status != STATUS_OK){
        handle_error(status);
        return status;
    }

    int arr[ARRSIZ];
    srand((unsigned int)time(NULL));
    status = fill_array_random(arr, ARRSIZ, a, b);
    if (status != STATUS_OK) {
        handle_error(status);
        return status;
    }

    printf("= = = = = Задание №1 = = = = =\n");
    printf("Исходный массив:\n");
    print_array(arr, ARRSIZ);

    status = find_and_swap_minmax(arr, ARRSIZ);
    if (status != STATUS_OK) {
        handle_error(status);
        return status;
    }

    printf("Массив после обмена min и max:\n");
    print_array(arr, ARRSIZ);

    printf("= = = = = Задание №2 = = = = =\n");
    int *arr_a = NULL;
    int *arr_b = NULL;
    int *arr_b_sorted = NULL;
    int *arr_c = NULL;
    size_t size_a = 0;
    size_t size_b = 0;

    status = generate_random_array(&arr_a, &size_a, -1000, 1000, 10, 10000);
    if (status != STATUS_OK) {
        handle_error(status);
        return status;
    }
    printf("Размер массива A: %zu\n", size_a);

    status = generate_random_array(&arr_b, &size_b, -1000, 1000, 10, 10000);
    if (status != STATUS_OK) {
        free(arr_a);
        handle_error(status);
        return status;
    }
    printf("Размер массива B: %zu\n", size_b);

    status = copy_array(arr_b, size_b, &arr_b_sorted);
    if (status != STATUS_OK) {
        free(arr_b);
        free(arr_a);
        handle_error(status);
        return status;
    }

    status = sort_array(arr_b_sorted, size_b);
    if (status != STATUS_OK) {
        free(arr_b_sorted);
        free(arr_b);
        free(arr_a);
        handle_error(status);
        return status;
    }

    status = form_array_c(arr_a, size_a, arr_b_sorted, size_b, &arr_c);
    if (status != STATUS_OK) {
        free(arr_b_sorted);
        free(arr_b);
        free(arr_a);
        handle_error(status);
        return status;
    }

    printf("Массив A (первые элементы): ");
    print_array_preview(arr_a, size_a, 20);

    printf("Массив B (первые элементы): ");
    print_array_preview(arr_b, size_b, 20);

    printf("Массив C (первые элементы): ");
    print_array_preview(arr_c, size_a, 20);

    free(arr_c);
    free(arr_b_sorted);
    free(arr_b);
    free(arr_a);
}