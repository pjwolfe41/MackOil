#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>

#include "utility.h"

/*
**  error stop routine; see header file for description.
*/

void error_stop (char *msg1, char *msg2)
{
	char msgbuf[100];
	sprintf_s (msgbuf, sizeof(msgbuf), "***** error: %s %s", msg1, msg2);
	MessageBox(0, msgbuf, "error_stop", 0);

    exit (0);
}

/*
**  compute number of coefficients; see header file for description.
*/

int coef_cnt (int degree)
{
    return ((degree + 1) * (degree + 2) / 2);
}


/*
**  compute powers of x and y; see header file for description.
*/

void powers (int degree, double xx, double yy, double *power)
{
    int j, k, m, n;

    power[0] = 1;
    j = 0;
    m = 1;
    for (n = 1; n <= degree; n++)
    {
		for (k = 0; k < n; k++)
			power[m++] = power[j++] * xx;
		power[m++] = power[j - 1] * yy;
    }
}
