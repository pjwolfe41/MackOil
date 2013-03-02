
/*
**  opt_zvalue - return option ZVALUE.
**
**  output arguments:
**	zname - name of the observed values;
**	digits - the number of digits to the right of the decimal.
*/

void opt_zvalue (char **zname, int *digits);

/*
**  opt_scale - return option SCALE.
**
**  output arguments:
**	ratio - the map scale.
*/

void opt_scale (long *ratio);

/*
**  opt_post - return option POST.
**
**  output arguments:
**	labelfont - the font used to label the points;
**	labelsize - the font size used to label the points;
**	labelrotate - the number of degrees to rotate the labels.
*/

void opt_post (char **labelfont, int *labelsize, int *labelrotate);

/*
**  opt_maplabel - draw option MAPLABEL labels on the map.
**
**  input arguments:
**	hscale - the horizontal scale in degrees of longitude per inch;
**	vscale - the vertical scale in degrees of latitude per inch;
**	xleft - the left border of the current map panel;
**	ybot - the bottom border of the current map panel.
*/

void opt_maplabel (HDC pdfDC, double hscale, double vscale, double xleft, double ybot, 
				                                double xscale, double yscale, int border);

/*
**  opt_mapline - draw option MAPLINE lines on the map.
**
**  input arguments:
**	hscale - the horizontal scale in degrees of longitude per inch;
**	vscale - the vertical scale in degrees of latitude per inch;
**	xleft - the left border of the current map panel;
**	ybot - the bottom border of the current map panel.
*/

void opt_mapline (HDC pdfDC, double hscale, double vscale, double xleft, double ybot, 
				                                double xscale, double yscale, int border);


/*
**  opt_select - see if a point satisfies the option SELECT conditions.
**
**  input arguments:
**	code - the station code;
**	x - the x coordinate (longitude);
**	y - the y coordinate (latitude);
**	z - the observed value;
**	fit_computed - non-zero if a fit value and residual are computed;
**	fit - the fit value, if any;
**	resid - the residual value, if any.
**
**  a non-zero value is returned if the point satisfies all the SELECT
**  conditions.
*/

int opt_select (unsigned long code, double x, double y, double z, 
                int fit_computed, double fit, double resid);

/*
**  opt_grid - return option CONTOUR gridding parameters.
**
**  output arguments:
**	meshsize - the grid mesh size in inches;
**	gridmethod - the distance norm used for gridding.
*/

void opt_grid (double *meshsize, int *gridmethod);

/*
**  opt_contour - return option CONTOUR levels and smoothing parameters.
**
**  output arguments:
**	intervalsize - the contour interval size;
**	nlevels	- the approximate number of contour levels to be used;
**	smoothmethod - the smoothing method, as follows:
**			   'L' - linear;
**			   'C' - cubic spline;
**			   'B' - B spline; 
**	bsplineorder - the order of the B spline smoothing;
**	npoints - approximate number of points to use for spline smoothing.
**
**  only one of intervalsize and nlevels has a non-zero value.
*/

void opt_contour (double *intervalsize, int *nlevels,
		  char *smoothmethod, int *bsplineorder, int *npoints);
