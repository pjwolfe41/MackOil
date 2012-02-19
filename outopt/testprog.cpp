#include <stdio.h>
#include <stdlib.h>

#include "outopt.h"

static int nerror = 0;

void main ()
{
    // setbuf (stdout, (char *) NULL);

    {
        char *zname;
	int digits;

	opt_zvalue (&zname, &digits);
	printf ("opt_zvalue: %s %d\n", zname, digits);
	if (nerror)
	    exit (0);
    }
    {
        long ratio;

	opt_scale (&ratio);
	printf ("opt_scale: %ld\n", ratio);
    }
    {
        char *labelfont;
	int labelsize;
	int labelrotate;

	opt_post (&labelfont, &labelsize, &labelrotate);
	printf ("opt_post: %s %d %d\n", labelfont, labelsize, labelrotate);
    }
    {
        double meshsize;
	int  gridmethod;

	opt_grid (&meshsize, &gridmethod);
	printf ("opt_grid: %g %d\n", meshsize, gridmethod);
    }
    {
	double intervalsize;
	int nlevels;
	char smoothmethod;
	int bsplineorder;
	int npoints;

	opt_contour (&intervalsize, &nlevels, &smoothmethod, &bsplineorder,
	                                                            &npoints);
        printf ("opt_contour: %g %d %c %d %d\n", intervalsize, nlevels,
	                                  smoothmethod, bsplineorder, npoints);
    }
    {
        double hscale = 2;
	double vscale = 12;
	double xleft = 17.;
	double ybot = -5.;

        opt_maplabel (hscale, vscale, xleft, ybot);
    }
    {
        double hscale = 10;
	double vscale = 5;
	double xleft = -100;
	double ybot = 0.;

        opt_mapline (hscale, vscale, xleft, ybot);
    }
    {
        unsigned long code[] = {101, 199, 278, 355, 500, 600, 700, 800, 999};
	float x[] = {1.0, -2.0, 3.5, -4.1, 5.0, -6.6, 7.8, -8.8, 9.0};
	float y[] = {0.0, 1.0, 7.8, 14., .5, -2., -.8, -88., -9.9};
	float z[] = {-67., -45., -11., -.4, 0, .4, 11., 45, 67};
	int fit_computed[] = {1, 2, 0, 0, 5, 0, 7, 8, 9};
	float fit[] = {90., 30., 0, 0, 0, 0, -12., 44., 88.};
	float resid[] = {-10., -5., 0, 0, .2, 0, 0, 1.45, 7.7};
	int npoint = sizeof (x) / sizeof (float);
	int i;

	for (i = 0; i < npoint; i++)
	{
	    printf ("%ld %g %g %g %d %g %g\n", code[i], x[i], y[i], z[i],
	                                 fit_computed[i], fit[i], resid[i]);
	    printf ("  returns %d\n", opt_select (code[i], x[i], y[i], z[i],
	                                fit_computed[i], fit[i], resid[i]));
	}
    }

    exit (0);
}

/*
**  error stop routine modified to continue for testing.
*/

void error_stop (char *msg1, char *msg2)
{
    printf ("*** call to error_stop: %s %s\n", msg1, msg2);
    ++nerror;
}
