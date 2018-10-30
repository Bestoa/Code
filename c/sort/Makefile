all: qsort stoogesort selectionsort

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

clean:
	rm -rf *.o qsort stoogesort selectionsort
