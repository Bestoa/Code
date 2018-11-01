void insertionsort_demo(int *array, int start, int end) {
    int i, j, k;
    int key;
    for(i = 1; i < end; i++) {
        key = array[i];
        for (j = 0; j < i && array[j] < key; j++);
        for (k = i; k > j; k--) {
            array[k] = array[k - 1];
        }
        array[j] = key;
    }
}

void init_sort_func(void (**sort_fun)(int *, int, int)) {
    *sort_fun = insertionsort_demo;
}
