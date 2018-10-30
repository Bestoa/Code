all: qsort stoogesort selectionsort

qsort: sort_base.c qsort.c
	gcc sort_base.c qsort.c -Wall -o qsort

stoogesort: sort_base.c stoogesort.c
	gcc sort_base.c stoogesort.c -Wall -o stoogesort

selectionsort: sort_base.c selectionsort.c
	gcc sort_base.c selectionsort.c -Wall -o selectionsort

clean:
	rm -rf qsort stoogesort selectionsort
