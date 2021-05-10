#include <stdlib.h>
#include <stdio.h>
#include "headers.h"
#include "util.h"

int main(int argc, char *argv[])
{
    string a;
    int_to_byte_string(5, a);
    fprintf(stdout, "%s", a);
    return 0;
}
