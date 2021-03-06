#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <direct.h>

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif

#include "utility.h"
#include "prepdata.h"
#include "outopt.h"

/*	prototypes of local routines	*/

static void write_panel (char *maptitle, DATATYPE data_type,
                         int npoint, unsigned long *code,
						 double *x_inch, double *y_inch, double *z,
						 double *fit, double *resid,
						 double hscale, double vscale,
						 double xmin, double xmax, double ymin, double ymax,
						 int zdigits,
                         int col, int ncol, int row, int nrow);


void main (int argcXX, char *argvXX[])
{
    char *datatitle;
    char *maptitle;
    char *zname;
    DATATYPE data_type;
    double hscale, vscale;
    double xmin, xmax, ymin, ymax;
    double *x, *y, *z;
    double *fit, *resid;
    int fit_degree;
    int i, j, k;
    int ncol, nrow;
    int npoint;
    int zdigits;
    unsigned long *code;

    // setbuf (stdout, (char *) NULL);

	////////////// TEST ARGUMENTS
	int argc = 3;
	char *argv[10];
	argv[1] = "atesarea";
	argv[2] = "obs";

	int retval = SetCurrentDirectory("C:\\MACK\\MackOil\\map");
	char buf[200];
	GetCurrentDirectory(200, buf);

/*	get data to plot	*/

    npoint = prepdata (argc, argv,
                       &datatitle, &data_type, &code, &x, &y, &z,
                       &fit_degree, &fit, &resid);

/*	use select conditions to select points	*/

    k = 0;
    for (i = 0; i < npoint; i++)
    {
		double fitval, residval;
		if (fit_degree > 0) {
			fitval = fit[i];
			residval = resid[i];
		}
		else {
			fitval = residval = 0.0;
		}
        if (opt_select (code[i], x[i], y[i], z[i], fit_degree, fitval, residval))
        {
			code[k] = code[i];
			x[k] = x[i];
			y[k] = y[i];
			z[k] = z[i];
			if (fit_degree)
			{
				fit[k] = fit[i];
				resid[k] = resid[i];
			}
			++k;
		}
    }
    npoint = k;
    if (npoint == 0)
        error_stop ("no points remain after selections in output.opt","");

/*	prepare map title	*/

    opt_zvalue (&zname, &zdigits);
    if (strlen (datatitle) > 100 || strlen (zname) > 100)
        error_stop ("data title or observed value name too long", "");
    maptitle = (char *) malloc (250);
    if (maptitle == NULL)
        error_stop ("cannot allocate vector for map title", "");
    switch (data_type)
    {
        case CODE:
			sprintf_s (maptitle, 250, "%s - Station Codes", datatitle);
			break;
        case OBS:
			sprintf_s (maptitle, 250, "%s - %s Values", datatitle, zname);
			break;
		case FIT:
			sprintf_s (maptitle, 250, "%s - Degree %d Fit of %s Values",
	                                      datatitle, fit_degree, zname);
			break;
 		case RESID:
			sprintf_s (maptitle, 250, "%s - Degree %d Fit Residual of %s Values",
	                                      datatitle, fit_degree, zname);
			break;
    }


/*	determine range of coordinate values	*/

    xmin = ymin = 1.e+30;
    xmax = ymax = -xmin;
    for (i = 0; i < npoint; i++)
    {
        xmin = x[i] < xmin ? x[i] : xmin;
        ymin = y[i] < ymin ? y[i] : ymin;
        xmax = x[i] > xmax ? x[i] : xmax;
        ymax = y[i] > ymax ? y[i] : ymax;
    }

/*	convert coordinates to map scale inches, assuming that the	*/
/*	x and y values are longitude and latitude, respectively		*/

    {
		double center_latitude = (ymin + ymax) / 2;
		double cos_cent;
        double degrees_per_radian = M_PI / 180.;
		double s, t;
        long ratio;

        opt_scale (&ratio);

        cos_cent = cos (center_latitude * degrees_per_radian);
        t = 1 - cos_cent * cos_cent * 0.006693422;
        s = (cos_cent * 2.5026656e+8) / sqrt (t);
		hscale = ratio / (s * degrees_per_radian);
        t = (6.305541e+16 - 0.006693422 * s * s);
        t = t * sqrt (t) / 6.284403e+16;
        vscale = ratio / (t * degrees_per_radian);

        for (i = 0; i < npoint; i++)
        {
            x[i] = - x[i] / hscale;	/* switch sign on longitude for */
	                                /* correct map orientation	*/
            y[i] = y[i] / vscale;
        }

        {				/* must also switch sign on	*/
			double hold = xmin;		/* xmin and xmax		*/
			xmin = - xmax / hscale;
            xmax = - hold / hscale;
		}

        ymin = ymin / vscale;
        ymax = ymax / vscale;
    }

/*	determine grid of panels	*/

    // ymax += 1;		/* leave one inch at top for the title */

#define PANEL_WIDTH_INCHES  26.0	// 7.5
#define PANEL_HEIGHT_INCHES 22.0	// 10.0

    ncol = (int) ceil ((xmax - xmin) / PANEL_WIDTH_INCHES);	
    nrow = (int) ceil ((ymax - ymin) / PANEL_HEIGHT_INCHES);

	/*
    xmin = xmin + (xmax - xmin - PANEL_WIDTH_INCHES * ncol) / 2;
    xmax = xmin + ncol * PANEL_WIDTH_INCHES;
    ymin = ymin + (ymax - ymin - PANEL_HEIGHT_INCHES * nrow) / 2;
    ymax = ymin + nrow * PANEL_HEIGHT_INCHES;
	*/

/*	loop to write panels	*/

    for (j = 0; j < ncol; j++)
    {
        for (k = 0; k < nrow; k++)
	    write_panel (maptitle, data_type,
	                 npoint, code, 
					 x, y, z, 
					 fit, resid,
					 hscale, vscale,
					 xmin, xmax, ymin, ymax,
					 zdigits,
	                 j, ncol, k, nrow);
    }

    exit (0);
}

/*	local subroutine to write one panel	*/

static void write_panel (char *maptitle, DATATYPE data_type,
                         int npoint, unsigned long *code,
						 double *x_inch, double *y_inch, double *zval,
						 double *fit, double *resid,
						 double hscale, double vscale,
						 double xmin, double xmax, double ymin, double ymax,
						 int zdigits,
                         int col, int ncol, int row, int nrow)
{
    int i;
    double x, y;
    double xleft, xright;
    double ybot, ytop;

    static char column_label[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/*	determine border of the panel	*/

    xleft = xmin + col * PANEL_WIDTH_INCHES; 
    xright = xleft + PANEL_WIDTH_INCHES;
    //xleft -= .5;   ????
    //xright += .5;
    ytop = ymax - row * PANEL_HEIGHT_INCHES;
    ybot = ytop - PANEL_HEIGHT_INCHES;
    //ytop += .5;
    //ybot -= .5;

/*	put instructions for assembly along the borders of the panel	*/

    printf ("/Helvetica findfont\n");
    printf ("10 scalefont setfont\n");

	/*
    if (col > 0)
        printf ("(%s  panel %c-%d   attach panel %c-%d here) attachleft\n", 
			 maptitle, column_label[col], row + 1, column_label[col - 1], row + 1);
    if (row > 0)
        printf ("(%s  panel %c-%d   attach panel %c-%d here) attachtop\n",
	         maptitle, column_label[col], row + 1, column_label[col], row);
    if (col < ncol - 1)
        printf ("(%s  panel %c-%d   attach panel %c-%d here) attachright\n",
			maptitle, column_label[col], row + 1, column_label[col + 1], row + 1);
    if (row < nrow - 1)
        printf ("(%s  panel %c-%d   attach panel %c-%d here) attachbottom\n",
			maptitle, column_label[col], row + 1, column_label[col], row + 2);
	*/

/*	draw the map border	*/

	/*
    printf ("%g %g %g %g mapborder\n", xmin - xleft, xmax - xleft,
                                       ymin - ybot, ymax - ybot);
    */

/*	define clipping area for remaining information	*/

	/*
    printf ("clippage\n");
	*/

/*	put the title at the top center of the top row panels	*/

	/*
    if (row == 0)
    {
        printf ("/Helvetica findfont\n");
        printf ("18 scalefont setfont\n");

		printf ("%g inch 20.5 inch moveto\n", PANEL_WIDTH_INCHES / 3);  // (xmin + xmax) / 2 - xleft);
		printf ("(%s) centeredshow\n", maptitle);
    }
	*/

/*	add labels and lines from the options file	*/

    opt_maplabel (hscale, vscale, xleft, ybot);
    opt_mapline (hscale, vscale, xleft, ybot);

/*	define parameters of posted values	*/

    {
        char *labelfont;
		int labelsize;
		int labelrotate;

        opt_post (&labelfont, &labelsize, &labelrotate);

        printf ("/%s findfont\n", labelfont);
        printf ("%d scalefont setfont\n", labelsize);
		printf ("/labelrotate %d def\n", labelrotate);
		printf ("/labelxpos %d def\n", 3);
		printf ("/labelypos %d def\n", -3);
    }

/*	post the points		*/

    for (i = 0; i < npoint; i++)
    {
        // x = x_inch[i] + 3.0;
		x = x_inch[i];
        if (x < xleft || x > xright)
			continue;
		y = y_inch[i];
		if (y < ybot || y > ytop)
			continue;
		printf ("%g inch %g inch moveto ", x - xleft, y - ybot);
		switch (data_type)
		{
			case CODE:
				printf ("(%ld)", code[i]);
				break;
			case OBS:
				printf ("(%.*f)", zdigits, zval[i]);
				break;
			case FIT:
				printf ("(%.*f)", zdigits, fit[i]);
				break;
			case RESID:
				printf ("(%.*f)", zdigits, resid[i]);
				break;
		}
		printf (" postpoint\n");
    }

/*	display the page	*/

    printf ("showpage\n");
}
