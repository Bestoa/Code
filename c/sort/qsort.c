extern void swap(int *, int *);

void qsort_demo(int *array, int start, int end) {
    int i = start, j = start;
    int flag = array[end - 1];
    if (start >= end)
        return;
    for (i = start; i < end; i++) {
        if (array[i] < flag) {
            swap(&array[i], &array[j]);
            j++;
        }
    }
    swap(&array[j], &array[end - 1]);
    qsort_demo(array, start, j);
    qsort_demo(array, j + 1, end);
}

void init_sort_func(void (**sort_fun)(int *, int, int)) {
    *sort_fun = qsort_demo;
}
