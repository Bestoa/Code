CC=gcc
RM=rm
CFLAGS=-Wall
all: cocktailsort stoogesort bubblesort selectionsort qsort insertionsort 
__GEN__COMMON_OBJ.o: sort_base.c 
	$(CC) $^ $(CFLAGS) -o $@ -c
cocktailsort: __GEN__COMMON_OBJ.o cocktailsort.c
	$(CC) $^ $(CFLAGS) -o $@
stoogesort: __GEN__COMMON_OBJ.o stoogesort.c
	$(CC) $^ $(CFLAGS) -o $@
bubblesort: __GEN__COMMON_OBJ.o bubblesort.c
	$(CC) $^ $(CFLAGS) -o $@
selectionsort: __GEN__COMMON_OBJ.o selectionsort.c
	$(CC) $^ $(CFLAGS) -o $@
qsort: __GEN__COMMON_OBJ.o qsort.c
	$(CC) $^ $(CFLAGS) -o $@
insertionsort: __GEN__COMMON_OBJ.o insertionsort.c
	$(CC) $^ $(CFLAGS) -o $@
clean:
	$(RM) -rvf *.o cocktailsort stoogesort bubblesort selectionsort qsort insertionsort 