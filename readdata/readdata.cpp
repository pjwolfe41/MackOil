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
              unsigned long **code, float **x, float **y, float **z)
{
    char filename[20];
#define  BUFSIZE 250
    char titlebuf[BUFSIZE];
    double obs, dlat, dat, dlong, dong;
    float *xx = NULL;
    float *yy = NULL;
    float *zz = NULL;
    int status;
    unsigned int alloc_size = 0;
    unsigned int i;
    unsigned long *cc = NULL;
    unsigned long inpcode;
    FILE *fp;

/*	open the file	*/

    strcpy (filename, dataname);
    strcat (filename, ".dat");
    fp = fopen (filename, "r");
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
	len = end - beg + 1;
	if (len >= 0)
	{
	    *title = new char[((unsigned) len + 1)];
	    if (*title == NULL)
	        error_stop ("cannot allocate space for data title", "");
	    strncpy (*title, beg, (unsigned) len);
	    (*title)[len] = '\0';
	}
    }

/*	read the data	*/

    fprintf (stderr, "read data\n");
    for (i = 0; ; i++)
    {
        status = fscanf (fp, "%ld %lf %lf %lf %lf %lf", &inpcode, &obs,
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
	    xx = (float *) realloc (xx, alloc_size * sizeof (float));
	    yy = (float *) realloc (yy, alloc_size * sizeof (float));
	    zz = (float *) realloc (zz, alloc_size * sizeof (float));
	    if (cc == NULL || xx == NULL || yy == NULL || zz == NULL)
	        error_stop ("cannot allocate arrays for input data", "");
	}
	obs = obs / 100;
	dat = dat / 100;
	dong = dong / 100;

	cc[i] = inpcode;

	yy[i] = (float) (dlat + dat / 60);
	xx[i] = (float) (dlong + dong / 60);
	xx[i] = xx[i];

	zz[i] = (float) obs;
    }
    fprintf (stderr, "\n");

    if (i == 0)
        error_stop ("no data found in file", filename);

    fclose (fp);

    cc = (unsigned long *) realloc (cc, i * sizeof (unsigned long));
    xx = (float *) realloc (xx, i * sizeof (float));
    yy = (float *) realloc (yy, i * sizeof (float));
    zz = (float *) realloc (zz, i * sizeof (float));
    if (cc == NULL || xx == NULL || yy == NULL || zz == NULL)
        error_stop ("cannot reallocate arrays for input data", "");

    *code = cc;
    *x = xx;
    *y = yy;
    *z = zz;

    return ((int) i);
}
