#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int* num = malloc(sizeof(int));
    *num = 7;

    int p_id = fork();
    if (p_id == 0) {
        printf("I am child, have %d\n", *num);
    }
}
