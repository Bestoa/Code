all: link-demo dlopen-demo

.PHONY:
libadd.so: add.c
	gcc -o libadd.so add.c -fPIC -shared

.PHONY:
link-demo: link-demo.c libadd.so
	gcc -o link-demo link-demo.c -L. -ladd

.PHONY:
dlopen-demo: dlopen-demo.c libadd.so
	gcc -o dlopen-demo dlopen-demo.c -ldl

clean:
	rm -rvf dlopen-demo link-demo libadd.so
