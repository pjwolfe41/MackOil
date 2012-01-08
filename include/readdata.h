/*
**  readdata - read data set.
**
**  input arguments:
**	dataname - the data set name.
**
**  output arguments:
**	title - the data title;
**	code - vector of station numbers;
**	x - vector of x-coordinates;
**	y - vector of y-coordinates;
**	z - vector of z-values.
**
**  the function returns the number of points.
*/

int readdata (char *dataname, char **title,
              unsigned long **code, float **x, float **y, float **z);
