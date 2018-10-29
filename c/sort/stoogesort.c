extern void swap(int *, int *);

void stoogesort_demo(int *array, int start, int end) {
    if (array[start] > array[end - 1]) {
        swap(&array[start], &array[end - 1]);
    }
    if (end - start > 2) {
        int t = (end - start) / 3;
        stoogesort_demo(array, start, end - t);
        stoogesort_demo(array, start + t, end);
        stoogesort_demo(array, start, end - t);
    }
}

void init_sort_func(void (**sort_fun)(int *, int, int)) {
    *sort_fun = stoogesort_demo;
}
