#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "utility.h"
#include "prepdata.h"
#include "outopt.h"
#include "list.h"

/*	prototypes of local routines	*/

static char *str_center (const char *str, const int len);


void list (int argc, char *argv[], char *listreport_filename)
{
    char *datatitle;
    char *list_title;
    char *zname;
    DATATYPE data_type;
    double *x, *y, *z;
    double *fit, *resid;
    int fit_degree;
    int i, k;
    int npoint;
    int zdigits;
    int zsize;
    unsigned long *code;

	FILE *rfp;

	fopen_s (&rfp, listreport_filename, "wb");
	if (rfp == NULL)
		error_stop ("cannot open file", listreport_filename);

/*	get data to list	*/

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

/*	prepare list title	*/

    opt_zvalue (&zname, &zdigits);
    if (strlen (datatitle) > 100 || strlen (zname) > 100)
        error_stop ("data title or observed value name too long", "");
    list_title = (char *) malloc (250);
    if (list_title == NULL)
        error_stop ("cannot allocate vector for list title", "");
    switch (data_type)
    {
        case CODE:
			sprintf_s (list_title, 250, "%s - Station Codes", datatitle);
			break;
		case OBS:
			sprintf_s (list_title, 250, "%s - %s Values", datatitle, zname);
			break;
		case FIT:
			sprintf_s (list_title, 250, "%s - Degree %d Fit of %s Values",
	                                      datatitle, fit_degree, zname);
			break;
 		case RESID:
			sprintf_s (list_title, 250, "%s - Degree %d Fit Residual of %s Values",
	                                      datatitle, fit_degree, zname);
			break;
    }

/*	determine size of z values for output formatting	*/

    zsize = zdigits + 1;
    {
        double zmin = 1.e+50;
		double zmax = - zmin;

        for (i = 0; i < npoint; i++)
        {
            zmin = z[i] < zmin ? z[i] : zmin;
            zmax = z[i] > zmax ? z[i] : zmax;
        }
		if (zmin < 0)
		{
			zsize += 1;			/* to allow for - sign */
			if (- zmin > zmax)
				zmax = - zmin;
		}
        if (zmax >= 1)
			zsize += (int) ceil (log10 (zmax));
    }
    if (zsize < (int) strlen (zname))
        zsize = (int) strlen (zname);

/*	print the title and column headings	*/

    fprintf (rfp, "%s\r\n\r\n", list_title);
    fprintf (rfp, "Station   Latitude   Longitude   %s", str_center (zname, zsize));
    if (data_type == FIT || data_type == RESID)
    {
        fprintf (rfp, "   %s", str_center ("Fit", zsize));
        fprintf (rfp, "   %s", str_center ("Residual", zsize));
    }
    fprintf (rfp, "\r\n");

/*	loop to print the points	*/

    for (i = 0; i < npoint; i++)
    {
        double dlat, dat, dlong, dong;

		dlat = floor ((double) y[i]);
		dat = (y[i] - dlat) * 60.;
		dlong = floor ((double) x[i]);
		dong = (x[i] - dlong) * 60.;

        fprintf (rfp, "%6ld    %.0f%6.2f    %.0f%6.2f   %*.*f", code[i], dlat, dat, 
	                                    dlong, dong, zsize, zdigits, z[i]);
		if (data_type == FIT || data_type == RESID)
			fprintf (rfp, "   %*.*f   %*.*f", zsize, zdigits, fit[i], zsize, zdigits, resid[i]);
		fprintf (rfp, "\r\n");
    }

	fclose(rfp);
}

/*
**  copy a string and pad with blanks to center it
*/

static char *str_center (const char *str, const int len)
{
    static char buf[1000];
    char *p = buf;
    int str_size = (int) strlen (str);
    int npad = (len - str_size + 1) / 2;

    while (p - buf < npad)
		*p++ = ' ';
    strcpy_s (p, 100, str);
    p += strlen (str);
    while (p - buf < len)
        *p++ = ' ';
    *p = '\0';

    return (buf);
}
