extern void swap(int *, int *);

void bubble_demo(int *array, int start, int end) {
    int i, j;
    for (i = end - 1; i > 0; i--) {
        for (j = 0; j < i - 1; j++) {
            if (array[j] > array[j + 1]) {
                swap(&array[j], &array[j + 1]);
            }
        }
    }
}

void init_sort_func(void (**sort_fun)(int *, int, int)) {
    *sort_fun = bubble_demo;
}
