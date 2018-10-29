all: qsort stoogesort

qsort: sort_base.c qsort.c
	gcc sort_base.c qsort.c -Wall -o qsort

stoogesort: sort_base.c stoogesort.c
	gcc sort_base.c stoogesort.c -Wall -o stoogesort

clean:
	rm -rf qsort stoogesort
