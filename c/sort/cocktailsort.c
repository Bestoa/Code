extern void swap(int *, int *);

void cocktailsort_demo(int *array, int start, int end) {
    int i, swaped = 1;
    int left = start, right = end - 1;
    while(swaped) {
        swaped = 0;
        for (i = left; i < right - 1; i++) {
            if (array[i] > array[i + 1]) {
                swap(&array[i], &array[i + 1]);
                swaped = 1;
            }
        }
        for (i = right - 1; i > left; i--) {
            if (array[i] < array[i - 1]) {
                swap(&array[i], &array[i - 1]);
                swaped = 1;
            }
        }
        right--;
        left++;
    }
}

void init_sort_func(void (**sort_fun)(int *, int, int)) {
    *sort_fun = cocktailsort_demo;
}
