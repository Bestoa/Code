#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM 20
#define MAX 100

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void dump(int array[], int start, int end) {
    int i;
    for (i = start; i < end; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

}

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

int main(){
    int array[NUM];
    int i;
    srand(time(NULL));
    for (i = 0; i < NUM; i++) {
        array[i] = rand() % MAX;
    }
    printf("===before===\n");
    dump(array, 0, NUM);
    qsort_demo(array, 0, NUM);
    printf("===end===\n");
    dump(array, 0, NUM);
    return 0;
}
