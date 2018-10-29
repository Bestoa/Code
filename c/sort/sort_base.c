#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM 2000
#define MAX 10000

extern void init_sort_func(void (**)(int *, int, int));

void dump(int array[], int start, int end) {
    int i;
    if (NUM > 100) {
        // Disable dump when the number of elements is larger than 100
        return;
    }
    for (i = start; i < end; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

void validated(int array[], int size) {
    int i;
    for (i = 0; i < size - 2; i++) {
        if (array[i] > array[i + 1]) {
            printf("Valiedated failed\n");
            return;
        }
    }
    printf("Valiedated OK\n");
}

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int main(){
    int array[NUM];
    int i;
    void (*sort_func)(int *, int, int);
    clock_t t1, t2;
    srand(time(NULL));
    for (i = 0; i < NUM; i++) {
        array[i] = rand() % MAX;
    }
    dump(array, 0, NUM);
    printf("===start===\n");
    init_sort_func(&sort_func);
    t1 = clock();
    sort_func(array, 0, NUM);
    t2 = clock();
    printf("===end===\n");
    validated(array, NUM);
    printf("Time = %lf\n", (double)(t2 -t1)/CLOCKS_PER_SEC);
    dump(array, 0, NUM);
    return 0;
}
