#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readdata.h"
#include "utility.h"
#include "prepdata.h"

static void usage (char *prog);

/*
**  prepare data for output program; see header file for description.
*/

int prepdata (int argc, char *argv[], 
			  char **title, DATATYPE *data_type,
              unsigned long **code, double **x, double **y, double **z,
			  int *fit_degree, double **fit, double **resid)
{
    char *dataname;
    char *fitdataname;
    DATATYPE type;
    int degree = 0;
    int npoint;

/*	process arguments	*/

    if (argc < 3)
        usage (argv[0]);
    dataname = argv[1];
    if (_stricmp (argv[2], "code") == 0)
        type = CODE;
    else if (_stricmp (argv[2], "obs") == 0)
        type = OBS;
    else if (_stricmp (argv[2], "fit") == 0)
        type = FIT;
    else if (_stricmp (argv[2], "resid") == 0)
        type = RESID;
    else
        usage (argv[0]);
    *data_type = type;
    if (type == FIT || type == RESID)
    {
        if (argc < 4 || argc > 5)
			usage (argv[0]);
		degree = atoi (argv[3]);
		*fit_degree = degree;
		if (argc == 5)
			fitdataname = argv[4];
		else
			fitdataname = dataname;
	}
	else
	{
		*fit_degree = 0;
		if (argc != 3)
			usage (argv[0]);
	}

/*	read data	*/

    npoint = readdata (dataname, title, code, x, y, z);

/*	compute fit and residual	*/

    if (type == FIT || type == RESID)
    {
        char filename[20];
		char version[13];
		double *coef;
		double *power;
		double xsub, xmul, ysub, ymul;
		double *f, *r;
		int deg;
        int i;
		int max_degree;
		int ncoef;
		FILE *fp;

		f = (double *) malloc ((unsigned) npoint * sizeof (double));
		r = (double *) malloc ((unsigned) npoint * sizeof (double));
		if (f == NULL || r == NULL)
			error_stop ("cannot allocate space for fit and residual", "");

		strcpy_s (filename, sizeof(filename), fitdataname);
		strcat_s (filename, sizeof(filename), ".fit");
		fopen_s (&fp, filename, "rb");
		if (fp == NULL)
			error_stop ("cannot open file", filename);

		if (fread (version, 12, 1, fp) != 1)
			error_stop ("error reading version header from", filename);
		version[12] = '\0';
		if (strcmp (version, "fit ver 1.00") != 0)
			error_stop (filename, "is not a fit coefficient file");
		{
			long md;
			if (fread (&md, sizeof (long), 1, fp) != 1)
				error_stop ("error reading max_degree from", filename);
			max_degree = (int) md; 
        }
		if (degree < 1 || degree > max_degree)
			error_stop ("requested fit degree not found in", filename);
		if (fread (&xsub, sizeof (double), 1, fp) != 1)
			error_stop ("error reading xsub from", filename);
		if (fread (&xmul, sizeof (double), 1, fp) != 1)
			error_stop ("error reading xmul from", filename);
		if (fread (&ysub, sizeof (double), 1, fp) != 1)
			error_stop ("error reading ysub from", filename);
		if (fread (&ymul, sizeof (double), 1, fp) != 1)
			error_stop ("error reading ymul from", filename);
		coef = (double *) malloc ((unsigned) coef_cnt (max_degree) *
	                                                     sizeof (double));
		if (coef == NULL)
			error_stop ("cannot allocate space for fit coefficients", "");
		for (deg = max_degree; deg > 0; deg--)
		{
			unsigned int nout = (unsigned) coef_cnt (deg);
			if (fread (coef, sizeof (double), nout, fp) != nout)
				error_stop ("error reading coefficient vector from", filename);
			if (deg == degree)
				break;
		}
		if (fclose (fp))
			error_stop ("cannot close file", filename);

		ncoef = coef_cnt (degree);
		power = (double *) malloc ((unsigned) ncoef * sizeof (double));
		if (power == NULL)
			error_stop ("cannot allocate space for powers vector", "");
        for (i = 0; i < npoint; i++)
        {
			double xx = ((*x)[i] - xsub) * xmul;
			double yy = ((*y)[i] - ysub) * ymul;
			double ff = 0;
			int k;

			powers (degree, xx, yy, power);
			for (k = 0; k < ncoef; k++)
				ff += coef[k] * power[k];
			f[i] = (double) ff;
			r[i] = (*z)[i] - f[i];
        }
		free (power);
		free (coef);
		*fit = f;
		*resid = r;
    }

    return (npoint);
}


static void usage (char *prog)
{
    char *p = prog;
    while (*p != '\0' && *p != '.')
        ++p;
    *p = '\0';

    fprintf (stderr,
         "\nusage:  %s  data_name data_type fit_degree fit_data_name\n", prog);
    fprintf (stderr, "  data_name - name of the data set\n");
    fprintf (stderr, "  data_type - code, obs, fit, or resid\n");
    fprintf (stderr,
                   "  fit_degree - fit degree if data_type is fit or resid\n");
    fprintf (stderr,
           "  fit_data_name - name of the data set used to compute the fit\n");
    fprintf (stderr, "                  if different from data_name\n");
          
    exit (0);
}
