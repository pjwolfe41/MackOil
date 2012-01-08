#include <stdio.h>
#include <stdlib.h>

#include "utility.h"

void main ()
{

    setbuf (stdout, (char *) NULL);

    printf ("coef_cnt(0) = %d\n", coef_cnt (0));
    printf ("coef_cnt(1) = %d\n", coef_cnt (1));
    printf ("coef_cnt(2) = %d\n", coef_cnt (2));
    printf ("coef_cnt(3) = %d\n", coef_cnt (3));
    printf ("coef_cnt(11) = %d\n", coef_cnt (11));

    {
        double pow[100];
        double xx = 2;
        double yy = 3;
	int i, j;

	for (i = 0; i < 5; i++)
	{
	    powers (i, xx, yy, pow);
	    for (j = 0; j < coef_cnt (i); j++)
	        printf ("%g ", pow[j]);
	    printf ("\n");
	}
    }

    error_stop ("this is a test", "error message");

    exit (0);
}
