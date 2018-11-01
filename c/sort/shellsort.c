void shellsort_demo(int *array, int start, int end) {
    int i, j, gap;
    int key;
    for(gap = (end - start) >> 1; gap > 0; gap >>= 1) {
        for(i = gap; i < end; i++) {
            key = array[i];
            for (j = i - gap; j >= 0 && array[j] > key; j -= gap)
                array[j + gap] = array[j];
            array[j + gap] = key;
        }
    }
}

void init_sort_func(void (**sort_fun)(int *, int, int)) {
    *sort_fun = shellsort_demo;
}

