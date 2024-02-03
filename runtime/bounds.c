#include <stdlib.h>
#include <stdio.h>

void __coco_check_bounds(int offset, int array_size){
    if(offset < 0 || offset >= array_size) {
        fprintf(stderr, "Out of bounds error, offset: %d is out of bounds for array_size: %d", offset, array_size);
        exit(-1);
    }
}