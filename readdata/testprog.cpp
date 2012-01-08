#include <stdio.h>
#include <stdlib.h>

#include "readdata.h"



void main ()
{
    int i;
    int npoint;
    char *title;
    unsigned long *code;
    float *x;
    float *y;
    float *z;

    // setbuf (stdout, (char *) NULL);

    npoint = readdata ("testdata", &title, &code, &x, &y, &z);

    printf ("%s\n", title);

    for (i = 0; i < npoint; i++)
        printf (" %4d   %ld  %g  %g  %g\n", i, code[i], x[i], y[i], z[i]);

    exit (0);
}
