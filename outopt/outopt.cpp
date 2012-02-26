#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "utility.h"
#include "outopt.h"

/*	prototype of local routines	*/

static void read_output_opt (void);
static int nextspecs (char ***field);
static int not_int (char *field);
static int not_float (char *field);

/*	data read from the output options file	*/

static struct
{
    int  spec_found;
    char *zname;
    int  digits;
} zvalue;

static struct
{
    int  spec_found;
    long ratio;
} scale;

static struct
{
    int  spec_found;
    char *labelfont;
    int  labelsize;
    int  labelrotate;
} post;

static struct
{
    int    spec_found;
    double meshsize;
    int    gridmethod;
    double intervalsize;
    int    numberoflevels;
    char   smoothmethod;
    int    approx_pts;
    int    bs_order;
} contour;

typedef struct
{
    char   *label;
    double xpos;
    double ypos;
    char   *font;
    int    fontsize;
    int    rotate;
}  MAPLABEL;

static struct
{
    int      nspec;
    MAPLABEL *maplab;
} maplabel;

typedef struct
{
    int    width;
    double intensity;
    int    npoint;
    double *x;
    double *y;
}  MAPLINE;

static struct
{
    int     nspec;
    MAPLINE *maplin;
} mapline;

typedef enum {CODE, OBS, X, Y, FIT, RESID} DATATYPE;

typedef enum {LT, LE, EQ, NE, GT, GE} LOG_OPERATOR;

typedef struct
{
    DATATYPE datatype;
    LOG_OPERATOR logop;
    double   value;
    int  or_next;
} SELECT;

static struct
{
    int    nspec;
    SELECT *condition;
} select;


/*
**  return option ZVALUE; see header file for description.
*/

void opt_zvalue (char **zname, int *digits)
{
    read_output_opt ();

    if (! zvalue.spec_found)
        error_stop ("ZVALUE not found in output options file","");
    *zname = zvalue.zname;
    *digits = zvalue.digits;
}

/*
**  return option SCALE; see header file for description.
*/

void opt_scale (long *ratio)
{
    read_output_opt ();

    if (! scale.spec_found)
        error_stop ("SCALE not found in output options file","");
    *ratio = scale.ratio;
}

/*
**  return option POST; see header file for description.
*/

void opt_post (char **labelfont, int *labelsize, int *labelrotate)
{
    read_output_opt ();

    if (! post.spec_found)
        error_stop ("POST not found in output options file","");
    *labelfont = post.labelfont;
    *labelsize = post.labelsize;
    *labelrotate = post.labelrotate;
}

/*	return option CONTOUR gridding parameters; see header file	*/
/*	for details							*/

void opt_grid (double *meshsize, int *gridmethod)
{
    read_output_opt ();

    if (! contour.spec_found)
        error_stop ("CONTOUR not found in output options file", "");
    *meshsize = contour.meshsize;
    *gridmethod = contour.gridmethod;
}

/*	return option CONTOUR interval and smoothing specifications;	*/
/*	see header file for details					*/

void opt_contour (double *intervalsize, int *nlevels,
		  char *smoothmethod, int *bsplineorder, int *npoints)
{
    if (! contour.spec_found)
        error_stop ("CONTOUR not found in output options file", "");
    *intervalsize = contour.intervalsize;
    *nlevels = contour.numberoflevels;
    *smoothmethod = contour.smoothmethod;
    *bsplineorder = contour.bs_order;
    *npoints = contour.approx_pts;
}

/*
**  draw option MAPLABEL labels on the map; see header file for description.
*/

void opt_maplabel (double hscale, double vscale, double xleft, double ybot)
{
    int i;

    read_output_opt ();

    for (i = 0; i < maplabel.nspec; i++)
    {
        double xx, yy;

		printf ("gsave\n");
        printf ("/%s findfont\n", maplabel.maplab[i].font);
		printf ("%d scalefont setfont\n", maplabel.maplab[i].fontsize);
		xx = - maplabel.maplab[i].xpos;
		xx = xx / hscale - xleft;
		yy = maplabel.maplab[i].ypos;
		yy = yy / vscale - ybot;
		printf ("%g inch %g inch moveto\n", xx, yy);
		printf ("%d rotate\n", maplabel.maplab[i].rotate);
		printf ("(%s) centeredshow\n", maplabel.maplab[i].label);
		printf ("grestore\n");
    }
}

/*
**  draw option MAPLINE lines on the map; see header file for description.
*/

void opt_mapline (double hscale, double vscale, double xleft, double ybot)
{
    int i;

    read_output_opt ();

    for (i = 0; i < mapline.nspec; i++)
    {
        int j;

        printf ("gsave\n");
		printf ("%g setgray\n", mapline.maplin[i].intensity);
		printf ("%d setlinewidth\n", mapline.maplin[i].width);
		printf ("newpath\n");
		for (j = 0; j < mapline.maplin[i].npoint; j++)
		{
			double xx, yy;

			xx = - mapline.maplin[i].x[j];
			xx = xx / hscale - xleft;
			yy = mapline.maplin[i].y[j];
			yy = yy / vscale - ybot;
			printf ("%g inch %g inch", xx, yy);
			if (j == 0)
				printf (" moveto\n");
			else
				printf (" lineto\n");
		}
		printf ("stroke\n");
        printf ("grestore\n");
    }
}

/*
**  see if a point satisfies the option SELECT conditions; see header file
**  for description.
*/

int opt_select (unsigned long code, double x, double y, double z, 
                int fit_computed, double fit, double resid)
{
    int i;
    int  select_point = 1;

    read_output_opt ();

    for (i = 0; i < select.nspec; i++)
    {
        double data = 0;
		double value; 

        switch (select.condition[i].datatype)
		{
			case CODE:
				data = code;
				break;
			case OBS:
				data = z;
				break;
			case X:
				data = x;
				break;
			case Y:
				data = y;
				break;
			case FIT:
				if (fit_computed == 0)
					continue;	/* skip condition */
				data = fit;
				break;
			case RESID:
				if (fit_computed == 0)
					continue;	/* skip condition */
				data = resid;
				break;
		}

#define  EPS .000001
		value = select.condition[i].value;
		switch (select.condition[i].logop)
		{
			case LT:
				if (data >= value)
					select_point = 0;
				break;
			case LE:
				if (data > value)
					select_point = 0;
				break;
			case EQ:
				if (data == 0)
				{
					if (fabs (value) > EPS)
						select_point = 0;
				}
				else
				{
					if (fabs ((data - value) / data) > EPS)
						select_point = 0;
				}
				break;
			case NE:
				if (data == 0)
				{
					if (fabs (value) < EPS)
						select_point = 0;
				}
				else
				{
					if (fabs ((data - value) / data) < EPS)
						select_point = 0;
				}
				break;
			 case GT:
				if (data <= value)
					select_point = 0;
				 break;
			 case GE:
				if (data < value)
					select_point = 0;
				break;
		}
		if (select.condition[i].or_next)
		{
			if (select_point)
				++i;			/* skip OR clause */
			else
				select_point = 1;	/* retry with OR clause */
		}
		if (! select_point)
			break;
	}

    return (select_point);
}


/*
**  read the output options file, check the specifications, and leave them
**  in the global data structures
*/

static void read_output_opt (void)
{
    static int alldone = 0;

    if (alldone)
        return;

    zvalue.spec_found = 0;
    scale.spec_found = 0;
    post.spec_found = 0;
    contour.spec_found = 0;
    maplabel.nspec = 0;
    maplabel.maplab = NULL;
    mapline.nspec = 0;
    mapline.maplin = NULL;
    select.nspec = 0;
    select.condition = NULL;

    while (1)
    {
		char **field;
        int nspec;
	
		nspec = nextspecs (&field);
		if (! nspec) 
			break;

		if (_stricmp (field[0], "zvalue") == 0)
		{
			if (zvalue.spec_found)
				error_stop (field[0],
		                "specified more than once in file output.opt");
			if (nspec != 3)
				error_stop (field[0], 
		           "has the wrong number of fields in file output.opt");
			zvalue.spec_found = 1;
			zvalue.zname = field[1];
			zvalue.digits = atoi (field[2]);
			if (not_int (field[2]) || zvalue.digits < 0 || zvalue.digits > 10)
				error_stop ("invalid digits field in output.opt ZVALUE:",
		                                                      field[2]);
		}
		else if (_stricmp (field[0], "scale") == 0)
		{
			if (scale.spec_found)
				error_stop (field[0],
		                "specified more than once in file output.opt");
			if (nspec != 2)
				error_stop (field[0], 
		          "has the wrong number of fields in file output.opt");
			scale.spec_found = 1;
			scale.ratio = atol (field[1]);
			if (not_int (field[1]) || scale.ratio < 1 || scale.ratio > 1000000)
				error_stop ("invalid ratio field in output.opt SCALE:",
		                                                      field[1]);
		}
		else if (_stricmp (field[0], "post") == 0)
		{
			if (post.spec_found)
				error_stop (field[0],
		                "specified more than once in file output.opt");
			if (nspec != 4)
				error_stop (field[0], 
		          "has the wrong number of fields in file output.opt");
			post.spec_found = 1;
			post.labelfont = field[1];
			post.labelsize = atoi (field[2]);
			if (not_int (field[2]) || post.labelsize < 1 ||
	                                                  post.labelsize > 100)
				error_stop ("invalid labelsize field in output.opt POST:",
		                                                      field[2]);
			post.labelrotate = atoi (field[3]);
			if (not_int (field[3]))
				error_stop ("invalid labelrotate field in output.opt POST:",
								      field[3]);
		}
		else if (_stricmp (field[0], "contour") == 0)
		{
			if (contour.spec_found)
				error_stop (field[0],
		                "specified more than once in file output.opt");
			if (nspec < 6)
				error_stop (field[0], 
		          "has the wrong number of fields in file output.opt");
			contour.spec_found = 1;
			contour.meshsize = atof (field[1]);
			if (not_float (field[1]) || contour.meshsize <= 0)
				error_stop ("invalid meshsize field in output.opt CONTOUR:", 
		                                                      field[1]);
			contour.gridmethod = atoi (field[2]);
			if (not_int (field[2]) || contour.gridmethod < 1 ||
	                                                contour.gridmethod > 20)
				error_stop ("invalid gridmethod field in output.opt CONTOUR:", 
		                                                      field[2]);
			if (_stricmp (field[3], "SIZE") == 0)
			{
				contour.intervalsize = atof (field[4]);
				contour.numberoflevels = 0;
				if (not_float (field[4]) || contour.intervalsize <= 0)
					error_stop (
						"invalid intervalsize field in output.opt CONTOUR:", 
		                                                      field[4]);
			}
			else if (_stricmp (field[3], "LEVELS") == 0)
			{
				contour.intervalsize = 0;
				contour.numberoflevels = atoi (field[4]);
				if (not_int (field[4]) || contour.numberoflevels <= 0)
					error_stop (
						"invalid numberoflevels field in output.opt CONTOUR:", 
		                                                      field[4]);
			}
			else
				error_stop (
					"intervalspec is not SIZE or LEVELS in output.opt CONTOUR:",
		                                                     field[3]);

			if (_stricmp (field[5], "LINEAR") == 0)
			{
				contour.smoothmethod = 'L';
				if (nspec != 6)
					error_stop (field[0], 
						"has the wrong number of fields in file output.opt");
				contour.approx_pts = 0;
				contour.bs_order = 0;
			}
			else if (_stricmp (field[5], "CUBIC_SPLINE") == 0)
			{
				contour.smoothmethod = 'C';
				if (nspec != 7)
					error_stop (field[0], 
						"has the wrong number of fields in file output.opt");
				contour.approx_pts = atoi (field[6]);
				if (not_int (field[6]) || contour.approx_pts < 4 || 
		                                        contour.approx_pts > 20)
					error_stop ("invalid npoints field in output.opt CONTOUR:",
		                                                      field[6]);
				contour.bs_order = 0;
			}
			else if (_stricmp (field[5], "BSPLINE") == 0)
			{
				contour.smoothmethod = 'B';
				if (nspec != 8)
					error_stop (field[0], 
						"has the wrong number of fields in file output.opt");
				contour.bs_order = atoi (field[6]);
				if (not_int (field[6]) || contour.bs_order < 2 ||
		                                         contour.bs_order > 10)
					error_stop (
						"invalid bspline order field in output.opt CONTOUR:",
		                                                      field[6]);
				contour.approx_pts = atoi (field[7]);
				if (not_int (field[7]) || contour.approx_pts < 4 || 
		                                        contour.approx_pts > 20)
					error_stop ("invalid npoints field in output.opt CONTOUR:",
		                                                      field[7]);
			}
			else
				error_stop ("invalid smoothmethod in output.opt CONTOUR:", 
		                                                     field[5]);
		}
		else if (_stricmp (field[0], "maplabel") == 0)
		{
			MAPLABEL *ml;

			++maplabel.nspec; 
			maplabel.maplab = (MAPLABEL *) realloc (maplabel.maplab, 
	                        (unsigned) maplabel.nspec * sizeof (MAPLABEL));
			if (maplabel.maplab == NULL)
				error_stop ("cannot reallocate MAPLABEL space", "");
			ml = maplabel.maplab + maplabel.nspec - 1;
			if (nspec != 7)
				error_stop (field[0], 
						"has the wrong number of fields in file output.opt");
			ml->label = field[1];
			ml->xpos = atof (field[2]);
			if (not_float (field[2]))
				error_stop ("invalid xpos field in output.opt MAPLABEL:", 
		                                                      field[2]);
			ml->ypos = atof (field[3]);
			if (not_float (field[3]))
				error_stop ("invalid ypos field in output.opt MAPLABEL:", 
		                                                      field[3]);
			ml->font = field[4];
			ml->fontsize = atoi (field[5]);
			if (not_int (field[5]) || ml->fontsize < 1 || ml->fontsize > 100)
				error_stop ("invalid fontsize field in output.opt MAPLABEL:",
		                                                      field[5]);
			ml->rotate = atoi (field[6]);
			if (not_int (field[6]))
				error_stop ("invalid rotate field in output.opt MAPLABEL:",
		                                                      field[6]);
		}
		else if (_stricmp (field[0], "mapline") == 0)
		{
			int i, k;
			MAPLINE *ml;

			++mapline.nspec; 
			mapline.maplin = (MAPLINE *) realloc (mapline.maplin, 
								(unsigned) mapline.nspec * sizeof (MAPLINE));
			if (mapline.maplin == NULL)
				error_stop ("cannot reallocate MAPLINE space", "");
			ml = mapline.maplin + mapline.nspec - 1;
			if (nspec < 3)
				error_stop (field[0], 
					"has the wrong number of fields in file output.opt");
			ml->width = atoi (field[1]);
			if (not_int (field[1]) || ml->width < 1 || ml->width > 100)
					error_stop ("invalid width field in output.opt MAPLINE:",
		                                                      field[1]);
			ml->intensity = atof (field[2]);
			if (not_float (field[2]))
				error_stop ("invalid intensity field in output.opt MAPLINE:",
		                                                      field[2]);
			ml->npoint = (nspec - 3) / 2;
			if (ml->npoint < 2 || ml->npoint * 2 + 3 != nspec)
				error_stop ("invalid coordinate pairs in output.opt MAPLINE",
		                                                            "");
			ml->x = (double *) malloc ((unsigned) ml->npoint * sizeof (double));
			ml->y = (double *) malloc ((unsigned) ml->npoint * sizeof (double));
			if (ml->x == NULL || ml->y == NULL)
				error_stop ("cannot allocate space for MAPLINE coordinates","");
			k = 3;
			for (i = 0; i < ml->npoint; i++)
			{
				ml->x[i] = atof (field[k++]);
				ml->y[i] = atof (field[k++]);
			}
		}
		else if (_stricmp (field[0], "select") == 0)
		{
			SELECT *sel;
			int nclause = 0;
			int k = 0;

			if (nspec == 4)
				nclause = 1;
			else if (nspec == 8)
				nclause = 2;
			else
				error_stop (field[0], 
		                "has the wrong number of fields in output.opt");
	
			while (nclause--)
			{
				++select.nspec; 
				select.condition = (SELECT *) realloc (select.condition, 
	                           (unsigned) select.nspec * sizeof (SELECT));
				if (select.condition == NULL)
					error_stop ("cannot reallocate SELECT space", "");
				sel = select.condition + select.nspec - 1;
				++k;
				{
					DATATYPE dt;
					dt = CODE;
					if (_stricmp (field[k], "code") == 0)
						dt = CODE;
					else if (_stricmp (field[k], "obs") == 0)
						dt = OBS;
					else if (_stricmp (field[k], "x") == 0)
						dt = X;
					else if (_stricmp (field[k], "y") == 0)
						dt = Y;
					else if (_stricmp (field[k], "fit") == 0)
						dt = FIT;
					else if (_stricmp (field[k], "resid") == 0)
						dt = RESID;
					else
						error_stop ("invalid datatype in output.opt SELECT:",
								     field[k]);
					sel->datatype = dt;
				}
				++k;
				{
					LOG_OPERATOR op;

					op = LT;
					if (_stricmp (field[k], "<") == 0)
						op = LT;
					else if (_stricmp (field[k], "<=") == 0)
						op = LE;
					else if (_stricmp (field[k], "=") == 0)
						op = EQ;
					else if (_stricmp (field[k], "!=") == 0)
						op = NE;
					else if (_stricmp (field[k], ">") == 0)
						op = GT;
					else if (_stricmp (field[k], ">=") == 0)
						op = GE;
					else
						error_stop ("invalid operator in output.opt SELECT:",
								     field[k]);
					sel->logop = op;
				}
				++k;
				sel->value = atof (field[k]);
				if (not_float (field[k]))
					error_stop ("invalid value field in output.opt SELECT:",
		                                                      field[k]);
				sel->or_next = nclause;
				if (nclause)
				{
					++k;
					if (_stricmp (field[k], "OR") != 0)
						error_stop ("OR not found in output.opt SELECT:",
		                                                      field[k]);
				}
			}
		}
		else
			error_stop ("unknown output.opt type:", field[0]);

	}

    alldone = 1;
}

/*
**  return the specifications from the next line in the output options file.
*/

static int nextspecs (char ***field)
{
    static char **fld = NULL;
    static FILE *fp = NULL;
    static int  eof = 0;
    static int  fld_size = 0;
    static int  line_number = 0;
    int nfield = 0;

/*	all done if end of file found on prior call	*/

    if (eof)
    {
        fclose (fp);
        free (fld);
        return (0);
    }

/*	initialize on the first call	*/

    if (fp == NULL)
    {
		fopen_s(&fp, "output.opt", "r"); 
        if (fp == NULL)
			error_stop ("cannot open file", "output.opt");
		fld_size = 100;
        fld = (char **) malloc ((unsigned) fld_size * sizeof (char *));
		if (fld == NULL)
			error_stop ("cannot allocate vector fld", "for output.opt specs");
    }

/*	get specs from the next non-blank line	*/

    while (1)
    {
        char buf[200];
	char cur_line[50];
	char *line;
	int cur_fld_len;
	int line_continued;
	int eol_found = 0;

    /*	    read the next line	*/

	line = fgets (buf, 200, fp);
	++line_number;
	sprintf_s (cur_line, sizeof(cur_line), "line %d of file output.opt", line_number);
	if (line == NULL)
	{
	    eof = 1;
	    break;
	}

    /*	    find the fields on the line     */
	
	cur_fld_len = 0;
	line_continued = 0;
	while (*line)
	{
	    if (*line == '"')
	    {
	        if (cur_fld_len > 0)
				error_stop ("embedded quote on", cur_line);
	        while (++line)
			{
				if (*line == '\0' || *line == '\n')
					error_stop ("missing quote on", cur_line);
				if (*line == '"')
				{
					*line = ' ';
					break;
				}
				++cur_fld_len;
			}
	    }

	    if ((*line == '/' && *(line + 1) == '/') || *line == '\n')
	        eol_found = 1;

	    if (*line == '+' && *(line + 1) == '+')
	    {
	        eol_found = 1;
			line_continued = 1;
		{
		    char *p = line + 2;
		    while (*p && *p != '\n')
		    {
				if (*p == '/' && *(p + 1) == '/') 
					break;
		        if (*p != ' ' && *p != '\t')
					error_stop ("characters after ++ on", cur_line);
		        ++p;
		    }
		}
	    }

	    if (*line == ' ' || *line == '\t' || eol_found)
	    {
	        if (cur_fld_len > 0)
			{
				if (nfield == fld_size)
				{
					fld_size += 100;
					fld = (char **) realloc (fld, (unsigned) fld_size *
			                                      sizeof (char *));
					if (fld == NULL)
						error_stop ("cannot reallocate vector fld on",
			                                             cur_line);
				}
				fld[nfield] = (char *) malloc ((unsigned) cur_fld_len + 1);
				if (fld[nfield] == NULL)
					error_stop ("cannot allocate space for option spec on",
			                                              cur_line);
				*line = '\0';
				strcpy_s(fld[nfield++], cur_fld_len + 1, line - cur_fld_len);
				cur_fld_len = 0;
			}
	    }
	    else
	        ++cur_fld_len;

  	    if (eol_found)
	        break;
	    ++line;
	}
	if (! eol_found)
	    error_stop ("line is too long -", cur_line);

    /*	    stop reading if field(s) found and line not continued	*/

	if (nfield > 0 && ! line_continued)
	    break;
    }

/*	set output arguments and return		*/

    *field = fld;
    return (nfield);
}

/*
**  return non-zero if a field does not contain an integer value.
*/

static int not_int (char *field)
{
    char *p = field;

    if (*p == '-' || *p == '+')
        ++p;
    while (*p)
    {
		if (! isdigit (*p++))
			return (1);
    }
    return (0);
}

/*
**  return non-zero if a field does not contain a float value.
*/

static int not_float (char *field)
{
    char *p = field;
    int ndec = 0;

    if (*p == '-' || *p == '+')
        ++p;
    while (*p)
    {
		if (*p == '.')
		{
			if (ndec)
				return (1);
			ndec = 1;
		}
		else if (! isdigit (*p))
			return (1);
        ++p;
    }
    return (0);
}
