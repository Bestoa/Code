#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM 20
#define MAX 100

extern void init_sort_func(void (**)(int *, int, int));

void dump(int array[], int start, int end) {
    int i;
    for (i = start; i < end; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

int main(){
    int array[NUM];
    int i;
    void (*sort_func)(int *, int, int);
    srand(time(NULL));
    for (i = 0; i < NUM; i++) {
        array[i] = rand() % MAX;
    }
    printf("===before===\n");
    dump(array, 0, NUM);
    init_sort_func(&sort_func);
    sort_func(array, 0, NUM);
    printf("===end===\n");
    dump(array, 0, NUM);
    return 0;
}
