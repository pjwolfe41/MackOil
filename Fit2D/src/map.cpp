#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <direct.h>

#include "map.h"

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif

#include "utility.h"
#include "prepdata.h"
#include "outopt.h"

/*	prototypes of local routines	*/

static void write_panel (FILE* mfp, char *maptitle, DATATYPE data_type,
                         int npoint, unsigned long *code,
						 double *x_inch, double *y_inch, double *z,
						 double *fit, double *resid,
						 double hscale, double vscale,
						 double xmin, double xmax, double ymin, double ymax,
						 int zdigits,
                         int col, int ncol, int row, int nrow);

void doPrint(HWND hWnd, char *dirpath, char* filename);
void writePDF(HWND hWnd, char *dirpath, char *filename, HDC hDC);
void printHeader(HDC hDC);


void map (int argc, char *argv[], HWND hWnd, char *dirpath, char *map_filename)
{
    char *datatitle;
    char *maptitle;
    char *zname;
    DATATYPE data_type;
    double hscale, vscale;
    double xmin, xmax, ymin, ymax;
    double *x, *y, *z;
    double *fit, *resid;
    int fit_degree;
    int i, j, k;
    int ncol, nrow;
    int npoint;
    int zdigits;
    unsigned long *code;

	///////////////
	doPrint(hWnd, dirpath, map_filename);
	return;
	///////////////

	FILE *mfp;

	fopen_s (&mfp, map_filename, "wb");
	if (mfp == NULL)
		error_stop ("cannot open file", map_filename);

/*	get data to plot	*/

    npoint = prepdata (argc, argv,
                       &datatitle, &data_type, &code, &x, &y, &z,
                       &fit_degree, &fit, &resid);

/*	use select conditions to select points	*/

    k = 0;
    for (i = 0; i < npoint; i++)
    {
		double fitval, residval;
		if (fit_degree > 0) {
			fitval = fit[i];
			residval = resid[i];
		}
		else {
			fitval = residval = 0.0;
		}
        if (opt_select (code[i], x[i], y[i], z[i], fit_degree, fitval, residval))
        {
			code[k] = code[i];
			x[k] = x[i];
			y[k] = y[i];
			z[k] = z[i];
			if (fit_degree)
			{
				fit[k] = fit[i];
				resid[k] = resid[i];
			}
			++k;
		}
    }
    npoint = k;
    if (npoint == 0)
        error_stop ("no points remain after selections in output.opt","");

/*	prepare map title	*/

    opt_zvalue (&zname, &zdigits);
    if (strlen (datatitle) > 100 || strlen (zname) > 100)
        error_stop ("data title or observed value name too long", "");
    maptitle = (char *) malloc (250);
    if (maptitle == NULL)
        error_stop ("cannot allocate vector for map title", "");
    switch (data_type)
    {
        case CODE:
			sprintf_s (maptitle, 250, "%s - Station Codes", datatitle);
			break;
        case OBS:
			sprintf_s (maptitle, 250, "%s - %s Values", datatitle, zname);
			break;
		case FIT:
			sprintf_s (maptitle, 250, "%s - Degree %d Fit of %s Values",
	                                      datatitle, fit_degree, zname);
			break;
 		case RESID:
			sprintf_s (maptitle, 250, "%s - Degree %d Fit Residual of %s Values",
	                                      datatitle, fit_degree, zname);
			break;
    }


/*	determine range of coordinate values	*/

    xmin = ymin = 1.e+30;
    xmax = ymax = -xmin;
    for (i = 0; i < npoint; i++)
    {
        xmin = x[i] < xmin ? x[i] : xmin;
        ymin = y[i] < ymin ? y[i] : ymin;
        xmax = x[i] > xmax ? x[i] : xmax;
        ymax = y[i] > ymax ? y[i] : ymax;
    }

/*	convert coordinates to map scale inches, assuming that the	*/
/*	x and y values are longitude and latitude, respectively		*/

    {
		double center_latitude = (ymin + ymax) / 2;
		double cos_cent;
        double degrees_per_radian = M_PI / 180.;
		double s, t;
        long ratio;

        opt_scale (&ratio);

        cos_cent = cos (center_latitude * degrees_per_radian);
        t = 1 - cos_cent * cos_cent * 0.006693422;
        s = (cos_cent * 2.5026656e+8) / sqrt (t);
		hscale = ratio / (s * degrees_per_radian);
        t = (6.305541e+16 - 0.006693422 * s * s);
        t = t * sqrt (t) / 6.284403e+16;
        vscale = ratio / (t * degrees_per_radian);

        for (i = 0; i < npoint; i++)
        {
            x[i] = - x[i] / hscale;	/* switch sign on longitude for */
	                                /* correct map orientation	*/
            y[i] = y[i] / vscale;
        }

        {				/* must also switch sign on	*/
			double hold = xmin;		/* xmin and xmax		*/
			xmin = - xmax / hscale;
            xmax = - hold / hscale;
		}

        ymin = ymin / vscale;
        ymax = ymax / vscale;
    }

/*	determine grid of panels	*/

    // ymax += 1;		/* leave one inch at top for the title */

#define PANEL_WIDTH_INCHES  26.0	// 7.5
#define PANEL_HEIGHT_INCHES 22.0	// 10.0

    ncol = (int) ceil ((xmax - xmin) / PANEL_WIDTH_INCHES);	
    nrow = (int) ceil ((ymax - ymin) / PANEL_HEIGHT_INCHES);

	/*
    xmin = xmin + (xmax - xmin - PANEL_WIDTH_INCHES * ncol) / 2;
    xmax = xmin + ncol * PANEL_WIDTH_INCHES;
    ymin = ymin + (ymax - ymin - PANEL_HEIGHT_INCHES * nrow) / 2;
    ymax = ymin + nrow * PANEL_HEIGHT_INCHES;
	*/

/*	loop to write panels	*/

    for (j = 0; j < ncol; j++)
    {
        for (k = 0; k < nrow; k++)
			write_panel (mfp, maptitle, data_type,
	                 npoint, code, 
					 x, y, z, 
					 fit, resid,
					 hscale, vscale,
					 xmin, xmax, ymin, ymax,
					 zdigits,
	                 j, ncol, k, nrow);
    }

    return;
}

/*	local subroutine to write one panel	*/

static void write_panel (FILE* mfp, char *maptitle, DATATYPE data_type,
                         int npoint, unsigned long *code,
						 double *x_inch, double *y_inch, double *zval,
						 double *fit, double *resid,
						 double hscale, double vscale,
						 double xmin, double xmax, double ymin, double ymax,
						 int zdigits,
                         int col, int ncol, int row, int nrow)
{
    int i;
    double x, y;
    double xleft, xright;
    double ybot, ytop;

    static char column_label[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/*	determine border of the panel	*/

    xleft = xmin + col * PANEL_WIDTH_INCHES; 
    xright = xleft + PANEL_WIDTH_INCHES;
    //xleft -= .5;   ????
    //xright += .5;
    ytop = ymax - row * PANEL_HEIGHT_INCHES;
    ybot = ytop - PANEL_HEIGHT_INCHES;
    //ytop += .5;
    //ybot -= .5;

/*	put instructions for assembly along the borders of the panel	*/

    fprintf (mfp, "/Helvetica findfont\n");
    fprintf (mfp, "10 scalefont setfont\n");

	/*
    if (col > 0)
        fprintf (mfp, "(%s  panel %c-%d   attach panel %c-%d here) attachleft\n", 
			 maptitle, column_label[col], row + 1, column_label[col - 1], row + 1);
    if (row > 0)
        fprintf (mfp, "(%s  panel %c-%d   attach panel %c-%d here) attachtop\n",
	         maptitle, column_label[col], row + 1, column_label[col], row);
    if (col < ncol - 1)
        fprintf (mfp, "(%s  panel %c-%d   attach panel %c-%d here) attachright\n",
			maptitle, column_label[col], row + 1, column_label[col + 1], row + 1);
    if (row < nrow - 1)
        fprintf (mfp, "(%s  panel %c-%d   attach panel %c-%d here) attachbottom\n",
			maptitle, column_label[col], row + 1, column_label[col], row + 2);
	*/

/*	draw the map border	*/

	/*
    fprintf (mfp, "%g %g %g %g mapborder\n", xmin - xleft, xmax - xleft,
                                       ymin - ybot, ymax - ybot);
    */

/*	define clipping area for remaining information	*/

	/*
    fprintf (mfp, "clippage\n");
	*/

/*	put the title at the top center of the top row panels	*/

	/*
    if (row == 0)
    {
        fprintf (mfp, "/Helvetica findfont\n");
        fprintf (mfp, "18 scalefont setfont\n");

		fprintf (mfp, "%g inch 20.5 inch moveto\n", PANEL_WIDTH_INCHES / 3);  // (xmin + xmax) / 2 - xleft);
		fprintf (mfp, "(%s) centeredshow\n", maptitle);
    }
	*/

/*	add labels and lines from the options file	*/

    opt_maplabel (hscale, vscale, xleft, ybot);
    opt_mapline (hscale, vscale, xleft, ybot);

/*	define parameters of posted values	*/

    {
        char *labelfont;
		int labelsize;
		int labelrotate;

        opt_post (&labelfont, &labelsize, &labelrotate);

        fprintf (mfp, "/%s findfont\n", labelfont);
        fprintf (mfp, "%d scalefont setfont\n", labelsize);
		fprintf (mfp, "/labelrotate %d def\n", labelrotate);
		fprintf (mfp, "/labelxpos %d def\n", 3);
		fprintf (mfp, "/labelypos %d def\n", -3);
    }

/*	post the points		*/

    for (i = 0; i < npoint; i++)
    {
        // x = x_inch[i] + 3.0;
		x = x_inch[i];
        if (x < xleft || x > xright)
			continue;
		y = y_inch[i];
		if (y < ybot || y > ytop)
			continue;
		fprintf (mfp, "%g inch %g inch moveto ", x - xleft, y - ybot);
		switch (data_type)
		{
			case CODE:
				fprintf (mfp, "(%ld)", code[i]);
				break;
			case OBS:
				fprintf (mfp, "(%.*f)", zdigits, zval[i]);
				break;
			case FIT:
				fprintf (mfp, "(%.*f)", zdigits, fit[i]);
				break;
			case RESID:
				fprintf (mfp, "(%.*f)", zdigits, resid[i]);
				break;
		}
		fprintf (mfp, " postpoint\n");
    }

/*	display the page	*/

    fprintf (mfp, "showpage\n");
}

void doPrint(HWND hWnd, char *dirpath, char *filename)
{
	char *pdfDriver = "Win2PDF";

	HDC hDC = CreateDC("WINSPOOL", pdfDriver, NULL, NULL);
	if (!hDC) {
		error_stop("cannot open PDF driver: ", pdfDriver);
	}

	writePDF(hWnd, dirpath, filename, hDC);

	DeleteDC(hDC);
}

#define DESIRED_PAGE_WIDTH  578
int pageWidth;
int pageHeight;
int currentTop;

void writePDF(HWND hWnd, char *dirpath, char* filename, HDC hDC)
{	
	int horzres = GetDeviceCaps(hDC, HORZRES);
	int vertres = GetDeviceCaps(hDC, VERTRES);

	// Let's find the aspect ratio of the page 
	double aspectRatio = (double) vertres / (double)horzres;

	pageWidth = DESIRED_PAGE_WIDTH;
	pageHeight = (int)((double)DESIRED_PAGE_WIDTH * aspectRatio);

	SetMapMode(hDC, MM_ISOTROPIC);
	SetViewportExtEx(hDC, horzres, vertres, NULL);
	SetWindowExtEx(hDC, pageWidth, pageHeight, NULL); 

	DOCINFO docInfo = {0};
	docInfo.cbSize = sizeof(docInfo);

	char fullpath[MAX_PATH] = {0};
	strcpy_s(fullpath, sizeof(fullpath), dirpath);
	strcat_s(fullpath, sizeof(fullpath), filename);

	docInfo.lpszDocName = filename;
	docInfo.lpszOutput = fullpath;

	StartDoc(hDC, &docInfo);
	
	StartPage(hDC);
	printHeader(hDC);
	EndPage(hDC);

	EndDoc(hDC); 
}

void printHeader(HDC hDC)
{
	HFONT font, oldFont;
	int oldBkMode, left;
	COLORREF oldTextColor;

	char headerline[] = "THIS IS THE HEADER LINE";
	char *nextline;
	int nline = 1;
	currentTop = 0;
	
	if (hDC) {		
		font = CreateFont(9, 0, 0, 0, 500,								
					   0, 0, 0, 0, OUT_TT_PRECIS, 0, CLEARTYPE_NATURAL_QUALITY, 0, "Tahoma");	   

		oldFont = (HFONT) SelectObject(hDC, font);
		oldBkMode = SetBkMode(hDC, TRANSPARENT);
		oldTextColor = SetTextColor(hDC, RGB(48, 48, 48));

		left = pageWidth / 10;

		currentTop += 12;

		for (int i = 0; i < nline; ++i) {
			nextline = headerline + i * 100; 
			currentTop += 12;
			TextOut(hDC, left, currentTop, nextline, (int) strlen(nextline));
		}
			
		SelectObject(hDC, oldFont);
		SetBkMode(hDC, oldBkMode);
		SetTextColor(hDC, oldTextColor);
		DeleteObject(font);
	}
}


