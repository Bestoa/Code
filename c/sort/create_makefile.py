#!/usr/bin/env python3
import os

CC="gcc"
RM="rm"
CFLAGS="-Wall"

COMMON_SOURCE_LIST= [ 'sort_base.c' ]

def get_file_list():
    source_list = []
    for f in os.listdir('.'):
        if os.path.isdir(f) or f in COMMON_SOURCE_LIST:
            pass
        elif f.endswith('.c'):
            source_list.append(f)
    return source_list

def create_makefile():
    source_list = get_file_list()
    obj_list = []
    common_obj_list = []
    for f in source_list:
        obj_list.append(f[0:-2])
    for f in COMMON_SOURCE_LIST:
        common_obj_list.append(f[0:-2])

    print('CC=%s' % CC)
    print('RM=%s' % RM)
    print('CFLAGS=%s' % CFLAGS)
    print('all: ', end = '')
    for o in obj_list:
        print(o, end = ' ')
    print('\n__GEN__COMMON_OBJ.o: ', end = '')
    for f in COMMON_SOURCE_LIST:
        print(f, end = ' ')
    print('\n\t$(CC) $^ $(CFLAGS) -o $@ -c')
    for o, f in zip(obj_list, source_list):
        print('%s: %s %s' % (o, '__GEN__COMMON_OBJ.o', f))
        print('\t$(CC) $^ $(CFLAGS) -o $@')
    print('clean:')
    print('\t$(RM) -rvf *.o ', end = '')
    for o in obj_list:
        print(o, end = ' ')

if __name__ == '__main__':
    create_makefile()
