#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "allocator.h"

int main(){

    int x = 10;
    int *ptr = &x;

    printf("Value of x: %d\n", x);
    printf("Size of x: %zu bytes\n", sizeof(x));
    printf("Size of pointer ptr: %zu bytes\n", sizeof(ptr));

    printf("\n------\n\n");

    int arr[3] = {1, 2, 3};
    printf("Values in array arr: %d, %d, %d\n", arr[0], arr[1], arr[2]);
    printf("Size of array arr: %zu bytes\n", sizeof(arr));
    printf("Size of pointer to array arr: %zu bytes\n", sizeof(&arr));
    printf("Address of first element in array arr: %p\n", (void*)&arr[0]);
    printf("Address of second element in array arr: %p\n", (void*)&arr[1]);
    printf("Address of third element in array arr: %p\n", (void*)&arr[2]);
    ptrdiff_t diff = (&arr[1] - &arr[0]) * sizeof(int);
    printf("Difference between addresses of arr[1] and arr[0]: %td\n", diff);
    ptrdiff_t diff2 = (&arr[2] - &arr[1]) * sizeof(int);
    printf("Difference between addresses of arr[2] and arr[1]: %td\n", diff2);
    ptrdiff_t diff3 = (&arr[2] - &arr[0]) * sizeof(int);
    printf("Difference between addresses of arr[2] and arr[0]: %td\n", diff3);

    printf("\n------\n\n");

    //int address = sbrk(0);
    //printf("Current program break (end of heap): %p\n", (void*)address);

    printf("\n------\n\n");

    bool bolean = true;

    printf("Size of boolean variable: %zu bytes\n", sizeof(bolean));

    printf("\n------\n\n");

    int *dynamic_array = (int *)allocate_memory(5 * sizeof(int));
    if (dynamic_array == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        dynamic_array[i] = i + 1;
    }

    printf("Values in dynamically allocated array: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", dynamic_array[i]);
    }
    printf("\naddress of dynamically allocated array: %p\n", (void*)dynamic_array);

    free_memory(dynamic_array);
    

    return 0;
}