#include<stdio.h>

int main(){

    int x = 10;
    int *ptr = &x;

    printf("Value of x: %d\n", x);
    printf("Size of x: %zu bytes\n", sizeof(x));
    printf("Size of pointer ptr: %zu bytes\n", sizeof(ptr));


    return 0;
}