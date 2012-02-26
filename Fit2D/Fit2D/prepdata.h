/*
**  prepdata - prepare data for output program.
**
**  input arguments:
**	argc - the number of arguments to the calling program;
**	argv - the vector of arguments to the calling program.
**
**  output arguments:
**	title - the data title;
**	data_type - the type of the data to be output - values are CODE,
**	            OBS, FIT, RESID defined below;
**	code - vector of station numbers;
**	x - vector of x coordinates;
**	y - vector of y coordinates;
**	z - vector of z values;
**	fit_degree - the degree of fit, if data_type is FIT or RESID, in
**                   which case the next two arguments are also used;
**	fit - the vector of fitted values;
**	resid - the vector of fit residuals.
**
**  the function returns the number of points.
*/

typedef enum {CODE, OBS, FIT, RESID} DATATYPE;

int prepdata (int argc, char *argv[], 
			  char **title, DATATYPE *data_type,
              unsigned long **code, double **x, double **y, double **z,
			  int *fit_degree, double **fit, double **resid);

