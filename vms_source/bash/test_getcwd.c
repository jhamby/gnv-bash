#include <stdio.h>
#include "gnv_common.c_first"

int main() {
    char * x;
    x = getcwd(NULL, 0);
    if (x != NULL)
        puts(x);
    else
        perror("getcwd");
}
