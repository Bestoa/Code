#include <stdio.h>
#include <dlfcn.h>

int main() {
    int (*add)(int, int);
    void *h = dlopen("libadd.so", RTLD_LAZY);
    char *err;
    if (!h) {
        printf("load libadd.so failed\n");
        return 0;
    }
    add = dlsym(h, "add");
    if ((err = dlerror()) != NULL) {
        printf("%s\n", err);
        return 0;
    }
    printf("result = %d\n", add(3, 4));
    dlclose(h);
    return 0;
}
