#include <stdio.h>
#include <stdlib.h>

#include "prepdata.h"

void main (int argc, char *argv[])
{
    DATATYPE data_type;
    int fit_degree;
    int i;
    int npoint;
    char *title;
    unsigned long *code;
    float *x;
    float *y;
    float *z;
    float *fit;
    float *resid;

    setbuf (stdout, (char *) NULL);

    npoint = prepdata (argc, argv, &title, &data_type, &code, &x, &y, &z,
                                   &fit_degree, &fit, &resid);

    printf ("data_type = ");
    switch (data_type)
    {
        case CODE: 
	    printf ("CODE");
	    break;
        case OBS: 
	    printf ("OBS");
	    break;
        case FIT: 
	    printf ("FIT %d", fit_degree);
	    break;
        case RESID: 
	    printf ("RESID %d", fit_degree);
	    break;
	default:
	    break;
    }
    printf ("\n");

    if (npoint > 0)
    {
        printf ("%s\n", title);

        for (i = 0; i < npoint; i++)
	{
            printf (" %4d   %ld  %g  %g  %.2f", i, code[i], x[i], y[i], z[i]);
	    if (data_type == FIT || data_type == RESID)
	        printf ("  %.2f  %.2f", fit[i], resid[i]);
	    printf ("\n");
        }
    }

    exit (0);
}
