#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "leastsq.h"
#include "readdata.h"
#include "utility.h"


/*
**  compute least squares fits; see header file for description.
*/

void leastsq (char *data_name, int max_degree)
{
    int *mdeg;		/* vector of degrees of fit */
    double *power;	/* vector of x-y powers */
    double *csum;	/* vector of column sums */
    double *sum;	/* vector of sums */
    double *rrr;	/* the triangular matrix */
    int  *num1;		/* vector of locations in the matrix */
    double *aaa;	/* vector of coefficients of fits */

    int npoint;			/* number of data points */
    char *title;		/* data title */
    double *x, *y, *z;		/* x, y, and z data arrays */
    unsigned long *code;	/* station code array */

    double xsub, xmul, ysub, ymul;	/* normalization values */

    double rms;			/* rms deviation for highest degree fit */

    int i, j, k, m, n, t;	/* index and counter variables */
    int ka, kb, kc, kk;		/* more counters */
    int limit, low, lup;	/* more counters */

    int term_cnt = coef_cnt (max_degree);
    int sum_cnt = coef_cnt (2 * max_degree);
    int matrix_cnt = (term_cnt * (term_cnt + 1) / 2) + term_cnt;


/*	allocate arrays		*/

    mdeg = (int *) malloc ((unsigned) max_degree * sizeof (int));
    if (mdeg == NULL)
        error_stop ("cannot allocate space for array mdeg", "");

    power = (double *) malloc ((unsigned) sum_cnt * sizeof (double));
    if (power == NULL)
        error_stop ("cannot allocate space for array power", "");

    csum = (double *) malloc ((unsigned) term_cnt * sizeof (double));
    if (csum == NULL)
        error_stop ("cannot allocate space for array csum", "");

    sum = (double *) malloc ((unsigned) sum_cnt * sizeof (double));
    if (sum == NULL)
        error_stop ("cannot allocate space for array sum", "");

    rrr = (double *) malloc ((unsigned) matrix_cnt * sizeof (double));
    if (rrr == NULL)
        error_stop ("cannot allocate space for array rrr", "");

    num1 = (int *) malloc ((unsigned) (term_cnt + 1) * sizeof (int));
    if (num1 == NULL)
        error_stop ("cannot allocate space for array num1", "");

    aaa = (double *) malloc ((unsigned) term_cnt * (unsigned) max_degree *
                                                           sizeof (double));
    if (aaa == NULL)
        error_stop ("cannot allocate sapce for array aaa", "");

/*	begin output report	*/

	char report[200];
	FILE *rfp;

	sprintf_s(report, sizeof(report), "%s Fit %d Report.txt", data_name, max_degree);
	fopen_s (&rfp, report, "wb");
	if (rfp == NULL)
		error_stop ("cannot open file", report);

    {
        time_t td;
		char ctime_buf[100];

		td = time (NULL);
		ctime_s(ctime_buf, 100, &td);
        fprintf (rfp, "Least Squares Surface Fit   %s    %s\r\n", data_name, ctime_buf);
    }

/*	initialize fit degrees array	*/

    for (i = 0; i < max_degree; i++)
		mdeg[i] = max_degree - i;

/*	data input	*/

    npoint = readdata (data_name, &title, &code, &x, &y, &z);
    fprintf (rfp, "\r\n  data title: %s\r\n", title);
    fprintf (rfp, "\r\n  %u data points on surface\r\n", npoint);

/*	compute normalization values for x and y	*/

    fprintf (stderr, "normalize x and y\r\n");
    {
        double xmin, xmax, ymin, ymax, zmin, zmax;

        xmin = ymin = zmin = .5e+30;
        xmax = ymax = zmax = -xmin;
        for (i = 0; i < npoint; i++)
        {
	    xmin = x[i] < xmin ? x[i] : xmin;
	    ymin = y[i] < ymin ? y[i] : ymin;
	    zmin = z[i] < zmin ? z[i] : zmin;
	    xmax = x[i] > xmax ? x[i] : xmax;
	    ymax = y[i] > ymax ? y[i] : ymax;
	    zmax = z[i] > zmax ? z[i] : zmax;
        }
        fprintf (rfp, "\r\n  Range of values   x:  %g to %g\r\n", xmin, xmax);
        fprintf (rfp, "                    y:  %g to %g\r\n", ymin, ymax);
        fprintf (rfp, "                    z:  %g to %g\r\n", zmin, zmax);

        xsub = (xmax + xmin) / 2;
        ysub = (ymax + ymin) / 2;
        xmul = 20 / (xmax - xmin);
        ymul = 20 / (ymax - ymin);

    }

/*	normalize x and y and compute sums	*/

    fprintf (stderr, "compute sums\r\n");
    for (i = 0; i < term_cnt; i++)
        csum[i] = 0; 
    for (i = 0; i < sum_cnt; i++)
        sum[i] = 0; 
    for (i = 0; i < npoint; i++)
    {
	fprintf (stderr, "station %ld\r", code[i]); 
	x[i] = (double) ((x[i] - xsub) * xmul);
	y[i] = (double) ((y[i] - ysub) * ymul);
	powers (2 * max_degree, (double) x[i], (double) y[i], power);
	for (j = 0; j < term_cnt; j++)
	    csum[j] += z[i] * power[j];
	for (j = 0; j < sum_cnt; j++)
	    sum[j] += power[j];
    }
    fprintf (stderr, "\r\n");
    fprintf (rfp, "\r\n  Average values after normalization   x = %.6f,  y = %.6f\r\n",
                                             sum[1] / npoint, sum[2] / npoint);

/*	load matrix	*/

    fprintf (stderr, "load matrix\r\n");
    lup = 0;
    ka = kb = kc = term_cnt;
    for (t = 0; t <= max_degree; t++)
    {
	low = lup + 1;
	lup = low + t;
	for (k = low; k <= lup; k++)
	{
	    int moretodo = 1;
	    n = kb;
	    rrr[n--] = csum[k - 1];
	    m = ka - 1;
	    kb += kc;
	    --kc;
	    ++ka;
	    i = kc;
	    limit = max_degree + 1;
	    while (moretodo)
	    {
		for (j = 0; j < limit; j++)
		{
		    rrr[n] = sum[m];
		    if (i <= 0)
		    {
		        moretodo = 0;
			break;
		    }
		    --i;
		    --m;
		    --n;
		}
		m -= t;
		--limit; 
	    }
	    num1[k - 1] = n + 1;
	}
	ka += max_degree;
    }
    num1[term_cnt] = n + 3;

/*	zero matrix	*/

    fprintf (stderr, "zero matrix\r\n");
    m = 1;
    for (i = 0; i < term_cnt - 1; i++)
    {
        int  nn = num1[i];
        double pivot = rrr[nn - 1];
        t = num1[m++] - 1;
        limit = t;
        for (k = m; k <= term_cnt; k++)
        {
            double factor = rrr[nn] / pivot;
	    ++nn;
	    for (j = nn; j <= limit; j++)
	    {
	        rrr[t] = rrr[t] - rrr[j - 1] * factor;
	        ++t;
	    }
	}
    }

/*	solve matrix	*/

    fprintf (stderr, "solve matrix\r\n");
    ka = 0;
    for (kk = 0; kk < max_degree; kk++)
    {
	limit = coef_cnt (mdeg[kk]) - 1;
	i = limit + 2;
	k = ka + i;
	kb = num1[i - 1] - 1;
	for (m = 0; m <= limit; m++)
	{
	    double tot = 0;
	    --i;
	    --k;
	    kc = num1[i - 1];
	    j = k;
	    t = kc;
	    for (n = 0; n < m; n++)
	        tot += aaa[j++] * rrr[t++];
	    aaa[k - 1] = (rrr[kb - 1] - tot) / rrr[kc - 1];
	    kb = kc - 1;
	}
	ka += term_cnt;
    }

/*	save computed coefficients and normalization values	*/

    fprintf (stderr, "save fit coefficients\r\n");
    {
        char filename[20];
		FILE *fp;

		strcpy_s (filename, sizeof(filename), data_name);
		strcat_s (filename, sizeof(filename), ".fit");
		fopen_s (&fp, filename, "wb");
		if (fp == NULL)
			error_stop ("cannot open file", filename);

		if (fwrite ("fit ver 1.00", 12, 1, fp) != 1)
			error_stop ("error writing fit file:", "version header");
		{
			long md = max_degree;   /* long for Watcom/Turbo compatability */
			if (fwrite (&md, sizeof (long), 1, fp) != 1)
				error_stop ("error writing fit file:", "max_degree");
		}
		if (fwrite (&xsub, sizeof (double), 1, fp) != 1)
			error_stop ("error writing fit file:", "xsub");
		if (fwrite (&xmul, sizeof (double), 1, fp) != 1)
			error_stop ("error writing fit file:", "xmul");
		if (fwrite (&ysub, sizeof (double), 1, fp) != 1)
			error_stop ("error writing fit file:", "ysub");
		if (fwrite (&ymul, sizeof (double), 1, fp) != 1)
			error_stop ("error writing fit file:", "ymul");
		n = 0;
		for (i = 0; i < max_degree; i++)
		{
			unsigned int nout = (unsigned) coef_cnt (mdeg[i]);
			if (fwrite (aaa + n, sizeof (double), nout, fp) != nout)
				error_stop ("error writing fit file:", "coefficient vector");
			 n += term_cnt;
		}
		if (fclose (fp))
			error_stop ("cannot close file", filename);
    }

/*	compute rms deviations	*/

    fprintf (stderr, "compute rms deviations\r\n");
    {
        double *dev;
		double fit, resid;

		dev = (double *) calloc ((unsigned) max_degree, sizeof (double));
		if (dev == NULL)
			error_stop ("cannot allocate space for array dev", "");

        for (j = 0; j < max_degree; j++)
            dev[j] = 0;

        for (i = 0; i < npoint; i++)
        {
			fprintf (stderr, "station %ld\r", code[i]); 
			powers (max_degree, (double) x[i], (double) y[i], power);
			for (j = 0; j < max_degree; j++)
			{
				fit = 0;
				t = j * term_cnt;
				for (k = 0; k < coef_cnt (mdeg[j]); k++)
					fit += aaa[t++] * power[k];
				resid = z[i] - fit;
				dev[j] += resid * resid;
			}
        }
        fprintf (rfp, "\r\n\r\n             Root-mean-square deviations\r\n");
        fprintf (rfp, "\r\ndegree   deviation\r\n");
        for (i = 0; i < max_degree; i++)
			fprintf (rfp, "%4d %12.4f\r\n", mdeg[i], sqrt (dev[i] / npoint));

		rms = sqrt (dev[0] / npoint);

		free (dev);
        fprintf (stderr, "\r\n");
    }

/*	compute histogram of residuals	*/

    fprintf (stderr, "compute residual histogram\r\n");
    {
#define  HISTSIZE 50
        int count[HISTSIZE];
		double fit, resid;
		double factor, interval, min_resid;
		int  totcount;

		factor = 1;
        while (rms * factor < 1)
			factor *= 10;
		while (rms * factor > 1)
			factor /= 10.;
		factor *= 100;
		interval = floor (factor * rms / 4); 
		interval /= factor;
		min_resid = - interval * HISTSIZE / 2; 

		for (j = 0; j < HISTSIZE; j++)
			count[j] = 0;

		for (i = 0; i < npoint; i++)
		{
			fprintf (stderr, "station %ld\r", code[i]); 
			powers (max_degree, (double) x[i], (double) y[i], power);
			fit = 0;
			t = 0;
			for (k = 0; k < term_cnt; k++)
				fit += aaa[t++] * power[k];
			resid = z[i] - fit;
			n = (int) floor ((resid - min_resid) / interval);
			if (n < 0)
				n = 0;
			if (n >= HISTSIZE)
				n = HISTSIZE - 1;
			++count[n];
		}

		fprintf (rfp, "\r\n\r\n      Distribution of residuals from %d degree fit\r\n",
	                                                           max_degree);
		fprintf (rfp, "\r\n        residual range     count\r\n");
		totcount = 0;
		for (j = 0; j < HISTSIZE; j++)
		{
			if (j == 0)
				fprintf (rfp, "    less than %10.3f     %d\r\n", min_resid + interval,
		                                                    count[j]);
			else if (j == HISTSIZE - 1)
				fprintf (rfp, " greater than %10.3f     %d\r\n", min_resid, count[j]);
			else
				fprintf (rfp, "%10.3f to %10.3f     %d\r\n", min_resid, 
		                              min_resid + interval, count[j]);
			totcount += count[j];
			min_resid += interval;
		}  
		fprintf (rfp, "\r\n              total        %d\r\n", totcount);

		fprintf (stderr, "\r\n");
    }

	fclose(rfp);

/*	free arrays	*/

    free (mdeg);
    free (power);
    free (csum);
    free (sum);
    free (aaa); 
    free (rrr);
    free (num1);

    return;
}
