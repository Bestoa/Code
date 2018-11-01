all: qsort stoogesort selectionsort bubblesort cocktailsort insertionsort

CC = gcc
CFLAGS = -Wall

sort_base.o: sort_base.c
	$(CC) sort_base.c $(CFLAGS) -c

qsort: sort_base.o qsort.c
	$(CC) $^ $(CFLAGS) -o $@

stoogesort: sort_base.o stoogesort.c
	$(CC) $^ $(CFLAGS) -o $@

selectionsort: sort_base.o selectionsort.c
	$(CC) $^ $(CFLAGS) -o $@

bubblesort: sort_base.o bubblesort.c
	$(CC) $^ $(CFLAGS) -o $@

cocktailsort: sort_base.o cocktailsort.c
	$(CC) $^ $(CFLAGS) -o $@

insertionsort: sort_base.o insertionsort.c
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -rf *.o qsort stoogesort selectionsort bubblesort cocktailsort insertionsort
