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

void startPrint(HWND hWnd, char *dirpath, char* filename, char* maptitle);
void printHeader(char* maptitle);
void postPoint(int x, int y, char* value, char *labelfont, int labelsize, int labelrotate);
void printStations (DATATYPE data_type, int npoint, unsigned long *code,
				    double *x_inch, double *y_inch, double *zval,
				    double *fit, double *resid,
				    double hscale, double vscale,
				    double xmin, double xmax, double ymin, double ymax,
				    int zdigits);
void endPrint();

HDC pdfDC;


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
    int i, k;
    int npoint;
    int zdigits;
    unsigned long *code;

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

        {							/* must also switch sign on	*/
			double hold = xmin;		/* xmin and xmax		*/
			xmin = - xmax / hscale;
            xmax = - hold / hscale;
		}

        ymin = ymin / vscale;
        ymax = ymax / vscale;
    }

/*	create pdf map file */

	startPrint(hWnd, dirpath, map_filename, maptitle);
	printHeader(maptitle);
	printStations(data_type, npoint, code, x, y, z, fit, resid,
				  hscale, vscale, xmin, xmax, ymin, ymax, zdigits);
	endPrint();
	return;
}

#define DESIRED_PAGE_WIDTH  600
int pageWidth;
int pageHeight;
int currentTop;

void startPrint(HWND hWnd, char *dirpath, char *filename, char* maptitle)
{
	char *pdfDriver = "Win2PDF";

	pdfDC = CreateDC("WINSPOOL", pdfDriver, NULL, NULL);
	if (!pdfDC) {
		error_stop("cannot open PDF driver: ", pdfDriver);
	}

	int horzres = GetDeviceCaps(pdfDC, HORZRES);
	int vertres = GetDeviceCaps(pdfDC, VERTRES);

	// Let's find the aspect ratio of the page 
	double aspectRatio = (double) vertres / (double)horzres;

	pageWidth = DESIRED_PAGE_WIDTH;
	pageHeight = (int)((double)DESIRED_PAGE_WIDTH * aspectRatio);

	SetMapMode(pdfDC, MM_ISOTROPIC);
	SetViewportExtEx(pdfDC, horzres, vertres, NULL);
	SetWindowExtEx(pdfDC, pageWidth, pageHeight, NULL); 

	DOCINFO docInfo = {0};
	docInfo.cbSize = sizeof(docInfo);

	char fullpath[MAX_PATH] = {0};
	strcpy_s(fullpath, sizeof(fullpath), dirpath);
	strcat_s(fullpath, sizeof(fullpath), filename);

	docInfo.lpszDocName = filename;
	docInfo.lpszOutput = fullpath;

	StartDoc(pdfDC, &docInfo);
	StartPage(pdfDC);
}

void endPrint()
{
	EndPage(pdfDC);
	EndDoc(pdfDC); 

	DeleteDC(pdfDC);
}

void printHeader(char* maptitle)
{
	HFONT font, oldFont;
	int oldBkMode, left;
	COLORREF oldTextColor;
	SIZE sz;

	currentTop = 0;
	
	if (pdfDC) {		
		font = CreateFont(11, 0, 0, 0, 500,								
					   0, 0, 0, 0, OUT_TT_PRECIS, 0, CLEARTYPE_NATURAL_QUALITY, 0, "Helvetica-Bold");	   

		oldFont = (HFONT) SelectObject(pdfDC, font);
		oldBkMode = SetBkMode(pdfDC, TRANSPARENT);
		oldTextColor = SetTextColor(pdfDC, RGB(48, 48, 48));

		GetTextExtentPoint(pdfDC, maptitle, (int)strlen(maptitle), &sz);
		left = pageWidth / 2 - sz.cx / 2;
		currentTop = sz.cy + 1;

		TextOut(pdfDC, left, currentTop, maptitle, (int) strlen(maptitle));
			
		SelectObject(pdfDC, oldFont);
		SetBkMode(pdfDC, oldBkMode);
		SetTextColor(pdfDC, oldTextColor);
		DeleteObject(font);
	}
}

void postPoint(int x, int y, char* value, char *labelfont, int labelsize, int labelrotate)
{
	HFONT font, oldFont;
	int oldBkMode;
	COLORREF oldTextColor;
	HPEN drawPen, oldPen;

	currentTop = 0;
	
	if (pdfDC) {		
		font = CreateFont(labelsize, 0, labelrotate * 10, labelrotate * 10, 500,								
					   0, 0, 0, 0, OUT_TT_PRECIS, 0, CLEARTYPE_NATURAL_QUALITY, 0, labelfont);	   

		oldFont = (HFONT) SelectObject(pdfDC, font);
		oldBkMode = SetBkMode(pdfDC, TRANSPARENT);
		oldTextColor = SetTextColor(pdfDC, RGB(48, 48, 48));

		drawPen = CreatePen(PS_SOLID, 1, 0);
		oldPen = (HPEN) SelectObject(pdfDC, drawPen);
		SelectObject(pdfDC, drawPen);
		Ellipse(pdfDC, x, y, x + 1, y + 1);

		if (labelrotate <= 30) {
			TextOut(pdfDC, x + 3, y - labelsize / 2, value, (int) strlen(value));
		}
		else if (labelrotate <= 60) {
			TextOut(pdfDC, x + 3, y - labelsize, value, (int) strlen(value));
		}
		else {
			TextOut(pdfDC, x - labelsize / 2, y - labelsize / 2, value, (int) strlen(value));
		}
			
		SelectObject(pdfDC, oldFont);
		SetBkMode(pdfDC, oldBkMode);
		SetTextColor(pdfDC, oldTextColor);
		DeleteObject(font);

		SelectObject(pdfDC, oldPen);
		DeleteObject(drawPen);
	}
}

#define BORDER 50

void printStations (DATATYPE data_type, int npoint, unsigned long *code,
				    double *x_inch, double *y_inch, double *zval,
				    double *fit, double *resid,
				    double hscale, double vscale,
				    double xmin, double xmax, double ymin, double ymax,
				    int zdigits)
{
    double x, y;
	int ix, iy;
    double xleft, xright;
    double ybot, ytop;
	char value[100];

/*	determine border of the page	*/

    xleft = xmin; 
    xright = xmax;
    ytop = ymax;
	ybot = ymin;

	double xscale = (pageWidth - 2 * BORDER) / (xmax - xmin);
	double yscale = (pageHeight - 2 * BORDER) / (ymax - ymin);

/*	add labels and lines from the options file	*/

    opt_maplabel (pdfDC, hscale, vscale, xleft, ybot, xscale, yscale, BORDER);

	opt_mapline (pdfDC, hscale, vscale, xleft, ybot, xscale, yscale, BORDER);

/*	define parameters of posted values	*/

	char *labelfont;
	int labelsize;
	int labelrotate;

	opt_post(&labelfont, &labelsize, &labelrotate);

/*	post the points		*/

    for (int i = 0; i < npoint; i++)
    {
		x = x_inch[i];
        if (x < xleft || x > xright)
			continue;
		y = y_inch[i];
		if (y < ybot || y > ytop)
			continue;
		ix = (int) ((x - xleft) * xscale);
		iy = (int) ((y - ybot) * yscale);

		ix += BORDER;
		iy += BORDER;

		switch (data_type)
		{
			case CODE:
				sprintf_s (value, sizeof(value), "%ld", code[i]);
				break;
			case OBS:
				sprintf_s (value, sizeof(value), "%.*f", zdigits, zval[i]);
				break;
			case FIT:
				sprintf_s (value, sizeof(value), "%.*f", zdigits, fit[i]);
				break;
			case RESID:
				sprintf_s (value, sizeof(value), "%.*f", zdigits, resid[i]);
				break;
		}
		
		postPoint(ix, iy, value, labelfont, labelsize, labelrotate);
    }
}

