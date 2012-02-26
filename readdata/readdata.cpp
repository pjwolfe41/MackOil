#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readdata.h"
#include "utility.h"

#define  BLOCK_SIZE  500

/*
**  read a data set; see header file for description.
*/

int readdata (char *dataname, char **title,
              unsigned long **code, double **x, double **y, double **z)
{
    char filename[20];
#define  BUFSIZE 250
    char titlebuf[BUFSIZE];
    double obs, dlat, dat, dlong, dong;
    double *xx = NULL;
    double *yy = NULL;
    double *zz = NULL;
    int status;
    unsigned int alloc_size = 0;
    unsigned int i;
    unsigned long *cc = NULL;
    unsigned long inpcode;
    FILE *fp;

/*	open the file	*/

    strcpy_s (filename, sizeof(filename), dataname);
    strcat_s (filename, sizeof(filename), ".dat");
	fopen_s (&fp, filename, "r");
    if (fp == NULL)
        error_stop ("cannot open file", filename);

/*	get data title	*/

    *title = NULL;
    while (*title == NULL)
    {
        char *beg, *end;
		int len;

        if (fgets (titlebuf, BUFSIZE, fp) == NULL)
			error_stop ("no data found in file", filename);
        beg = titlebuf;
		while (*beg == ' ' || *beg == '\t')
			++beg;
		end = titlebuf + strlen (titlebuf);
		while (*end == '\0' || *end == '\n' || *end == ' ' || *end == '\t')
			--end;
		len = (int) (end - beg + 1);
		if (len >= 0)
		{
			*title = new char[((unsigned) len + 1)];
			if (*title == NULL)
				error_stop ("cannot allocate space for data title", "");
			strncpy_s (*title, len + 1, beg, (unsigned) len);
			(*title)[len] = '\0';
		}
    }

/*	read the data	*/

    fprintf (stderr, "read data\n");
    for (i = 0; ; i++)
    {
		status = fscanf_s (fp, "%ld %lf %lf %lf %lf %lf", &inpcode, &obs,
	                                       &dlat, &dat, &dlong, &dong);

        fprintf (stderr, "station %ld    \r", inpcode);
		if (status == EOF)
			break;
        if (status != 6)
			printf ("NOT 6\n");

		if (i >= alloc_size)
		{
			alloc_size += BLOCK_SIZE;
			cc = (unsigned long *)
	                   realloc (cc, alloc_size * sizeof (unsigned long));
			xx = (double *) realloc (xx, alloc_size * sizeof (double));
			yy = (double *) realloc (yy, alloc_size * sizeof (double));
			zz = (double *) realloc (zz, alloc_size * sizeof (double));
			if (cc == NULL || xx == NULL || yy == NULL || zz == NULL)
				error_stop ("cannot allocate arrays for input data", "");
		}
		obs = obs / 100;
		dat = dat / 100;
		dong = dong / 100;

		cc[i] = inpcode;

		yy[i] = (double) (dlat + dat / 60);
		xx[i] = (double) (dlong + dong / 60);
		xx[i] = xx[i];

		zz[i] = (double) obs;
    }
    fprintf (stderr, "\n");

    if (i == 0)
        error_stop ("no data found in file", filename);

    fclose (fp);

    cc = (unsigned long *) realloc (cc, i * sizeof (unsigned long));
    xx = (double *) realloc (xx, i * sizeof (double));
    yy = (double *) realloc (yy, i * sizeof (double));
    zz = (double *) realloc (zz, i * sizeof (double));
    if (cc == NULL || xx == NULL || yy == NULL || zz == NULL)
        error_stop ("cannot reallocate arrays for input data", "");

    *code = cc;
    *x = xx;
    *y = yy;
    *z = zz;

    return ((int) i);
}
