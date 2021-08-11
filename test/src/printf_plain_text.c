#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    printf("This plain text with a new line used to do a thunk when built in non optimized mode.\nFor instanse, using -Og will do.\n");

    return 0;
}
