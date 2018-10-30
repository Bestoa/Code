extern void swap(int *, int *);

void selectionsort_demo(int *array, int start, int end) {
    int i, j, min;
    for (i = start; i < end; i++) {
        min = i;
        for (j = i + 1; j < end; j++) {
            if (array[j] < array[min])
                min = j;
        }
        swap(&array[i], &array[min]);
    }
}

void init_sort_func(void (**sort_fun)(int *, int, int)) {
    *sort_fun = selectionsort_demo;
}
