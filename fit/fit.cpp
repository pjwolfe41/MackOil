#include <stdio.h>
#include <stdlib.h>

#include "leastsq.h"

static void usage (void);

void main (int argc, char *argv[])
{
    int maxdeg;

    // setbuf (stdout, (char *) NULL);

    if (argc != 3)
        usage ();
    maxdeg = atoi (argv[2]);
    if (maxdeg < 1 || maxdeg > 25)
        usage ();

    leastsq (argv[1], maxdeg);

    exit (0);
}

static void usage (void)
{
    fprintf (stderr, 
              "\nusage:  fit  data_name max_degree - calculate surface fit\n");
    fprintf (stderr, "  data_name - name of the data set\n");
    fprintf (stderr,
                "  max_degree - maximum degree of fit (1 <= maxdeg <= 25)\n");
    exit (0);
}
