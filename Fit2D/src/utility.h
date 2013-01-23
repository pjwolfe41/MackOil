
#include <windows.h>

/*
**  error_stop - print error message and terminate run.
**
**  input arguments:
**	msg1 - first part of error message;
**	msg2 - second part of error message.
*/

void error_stop (char *msg1, char *msg2);

/*
**  coef_cnt - return the number of coefficients for a polynomial of
**             specified degree.
**	argument:
**	    degree - the polynomial degree.
*/

int coef_cnt (int degree);


/*
**  powers - compute powers of x and y up to a specified degree.
**	arguments:
**          degree - the polynomial degree;
**          xx - the x value;
**          yy - the y value;
**          power - vector in which the computed powers are stored.
*/

void powers (int degree, double xx, double yy, double *power);
