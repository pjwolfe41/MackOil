

#include <stdio.h>
#include <math.h>
#include <direct.h>
#include "main.h"
#include "readdata.h"
#include "leastsq.h"
#include "list.h"
#include "map.h"

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK FitDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ListDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

HINSTANCE ghInstance;

bool dataopen = false;
char dataname[MAX_PATH] = {0};
char datadir[MAX_PATH] = {0};
char fitreport_filename[MAX_PATH] = {0};
char listreport_filename[MAX_PATH] = {0};
char map_filename[MAX_PATH] = {0};


/*
  =======================================================================================
  =======================================================================================
*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	int status;
	HWND hDlg;

	ghInstance = hInst;

	InitCommonControls();

	hDlg = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_FIT2D_DLG), NULL, MainDlgProc);

	if (hDlg) {
		ShowWindow(hDlg, 1);

		while ((status = GetMessage(&msg, 0, 0, 0)) != 0) {
			if (status == -1) {
				return -1;
			}
			else if (!IsDialogMessage (hDlg, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	
    return (int) msg.wParam;
}

/*
  =======================================================================================
  =======================================================================================
*/
BOOL CALLBACK MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME ofn;
	char szFile[260];
	char *p;
	INT_PTR retval;

	switch (msg) 
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_DATA_PATH, "");
		SetDlgItemText(hDlg, IDC_DATA_TITLE, "");
		SetDlgItemText(hDlg, IDC_DATA_COUNT, "");
		
		SetDlgItemText(hDlg, IDC_FIT_STATUS, "");
		SetDlgItemText(hDlg, IDC_LIST_STATUS, "");
		SetDlgItemText(hDlg, IDC_MAP_STATUS, "");

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case IDC_OPEN_DATA_BUTTON:
			SetDlgItemText(hDlg, IDC_DATA_PATH, "");
			SetDlgItemText(hDlg, IDC_DATA_TITLE, "");
			SetDlgItemText(hDlg, IDC_DATA_COUNT, "");

			SetDlgItemText(hDlg, IDC_FIT_STATUS, "");
			SetDlgItemText(hDlg, IDC_LIST_STATUS, "");
			SetDlgItemText(hDlg, IDC_MAP_STATUS, "");

			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = szFile;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "All\0*.*\0";
			ofn.nFilterIndex = 2;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			ofn.lpstrTitle = "Locate the Project Folder and .DAT File";

			if (GetOpenFileName(&ofn)==TRUE)
			{	
				SetDlgItemText(hDlg, IDC_DATA_PATH, szFile);
			}

			p = szFile + strlen(szFile) - strlen(".dat");
			if (_stricmp(p, ".dat") != 0) {
				MessageBox(0, "File name does not end in .dat", "Invalid File", 0);
				dataname[0] = 0;
				break;
			}

			*p = 0;
			while (*p != '\\') {
			    --p;
			}
			strcpy_s(dataname, sizeof(dataname), ++p);

			if (strlen(dataname) == 0) {
				MessageBox(0, "Not a valid data name", "Invalid Data Name", 0);
				dataname[0] = 0;
				break;
			}

			*p = 0;
			if (_chdir(szFile) != 0) {
				MessageBox(0, "Cannot chdir to the data folder", "Access Error", 0);
				dataname[0] = 0;
				break;
			}
			strcpy_s(datadir, sizeof(datadir), szFile);

			char *title;
            unsigned long *code;
			double *x, *y, *z;

			int datacount;
			datacount = readdata(dataname, &title, &code, &x, &y, &z);

			if (datacount < 1) {
				MessageBox(0, "Cannot read data file", "Access Error", 0);
				dataname[0] = 0;
				freedata();
				break;
			}

			SetDlgItemText(hDlg, IDC_DATA_TITLE, title);
			SetDlgItemInt(hDlg, IDC_DATA_COUNT, datacount, FALSE);
			freedata();

			dataopen = true;
			break;

		case IDC_FIT_BUTTON:
			if (!dataopen) {
				MessageBox(0, "You must Open Data first", "Data Not Open", 0);
				return FALSE;
			}
			INT_PTR fitdegree;
			SetDlgItemText(hDlg, IDC_FIT_STATUS, "");
			fitdegree = DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_FIT_DLG), hDlg, FitDlgProc, NULL);
			if (fitdegree > 0) {
				char status[100];
				sprintf_s(status, sizeof(status), "Fits to degree %d complete; see %s", fitdegree,
					                                                                fitreport_filename); 
				SetDlgItemText(hDlg, IDC_FIT_STATUS, status);
			}
			else {
				SetDlgItemText(hDlg, IDC_FIT_STATUS, "");
			}

			break;

		case IDC_LIST_BUTTON:
			if (!dataopen) {
				MessageBox(0, "You must Open Data first", "Data Not Open", 0);
				return FALSE;
			}
			SetDlgItemText(hDlg, IDC_LIST_STATUS, "");
			retval = DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_LIST_DLG), hDlg, ListDlgProc, NULL);

			if (retval) {
				char status[100];
				sprintf_s(status, sizeof(status), "List complete; see %s", listreport_filename); 
				SetDlgItemText(hDlg, IDC_LIST_STATUS, status);
			}
			else {
				SetDlgItemText(hDlg, IDC_LIST_STATUS, "");
			}

			break;

		case IDC_MAP_BUTTON:
			if (!dataopen) {
				MessageBox(0, "You must Open Data first", "Data Not Open", 0);
				return FALSE;
			}
			SetDlgItemText(hDlg, IDC_MAP_STATUS, "");
			retval = DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_MAP_DLG), hDlg, MapDlgProc, NULL);

			if (retval) {
				char status[100];
				sprintf_s(status, sizeof(status), "Map complete; see %s", map_filename); 
				SetDlgItemText(hDlg, IDC_MAP_STATUS, status);
			}
			else {
				SetDlgItemText(hDlg, IDC_MAP_STATUS, "");
			}

			break;
			
		case IDCANCEL:
		case IDC_EXIT_BUTTON:
			DestroyWindow(hDlg);
			break;
		}

		return TRUE;

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;		
	}

	return FALSE;
}

/*
  =======================================================================================
  =======================================================================================
*/
BOOL CALLBACK FitDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int degree = 0;

	switch (msg) 
	{
	case WM_INITDIALOG:
		
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{			

		case IDOK:
			degree = GetDlgItemInt(hDlg, IDC_FIT_DEGREE, NULL, FALSE);
			if (degree <= 0 || degree > 25) {
				MessageBox(0, "Maximum Fit Degree must not exceed 25.", "ERROR", 0);
				return FALSE;
			}
			sprintf_s(fitreport_filename, sizeof(fitreport_filename), "%s Fit %d Report.txt", 
				                                                   dataname, degree);
			leastsq(dataname, degree, fitreport_filename);
			EndDialog(hDlg, degree);
			break;

		case IDCANCEL:
			EndDialog(hDlg, 0);
		}

		return TRUE;
	}

	return FALSE;
}

/*
  =======================================================================================
  =======================================================================================
*/
BOOL CALLBACK ListDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char *listtype;
	char textdegree[10];
	int degree;
	char fitdataname[100];

	switch (msg) 
	{
	case WM_INITDIALOG:
		
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{

		case IDOK:
			listtype = "";
			degree = 0;
			fitdataname[0] = '\0';
			if (IsDlgButtonChecked(hDlg, IDC_LIST_CODE)) listtype = "code";
			if (IsDlgButtonChecked(hDlg, IDC_LIST_OBS)) listtype = "obs";
			if (IsDlgButtonChecked(hDlg, IDC_LIST_FIT)) listtype = "fit";
			if (IsDlgButtonChecked(hDlg, IDC_LIST_RESID)) listtype = "resid";
			if (*listtype == '\0') {
				MessageBox(0, "Please select one of code, obs, fit, resid", "ERROR", 0);
				return FALSE;
			}
			if (strcmp(listtype, "fit") == 0 || strcmp(listtype, "resid") == 0) {
				degree = GetDlgItemInt(hDlg, IDC_LIST_FIT_DEGREE, NULL, FALSE);
				if (degree <= 0 || degree > 25) {
					MessageBox(0, "Fit Degree must be specified and may not exceed 25.", "ERROR", 0);
					return FALSE;
				}
				GetDlgItemText(hDlg, IDC_LIST_FIT_NAME, fitdataname, sizeof(fitdataname));
			}

			int argc;
			char *argv[5];
			argc = 3;
			argv[0] = "list";
			argv[1] = dataname;
			argv[2] = listtype;
			textdegree[0] = '\0';
			if (strcmp(listtype, "fit") == 0 || strcmp(listtype, "resid") == 0) {
				sprintf_s(textdegree, sizeof(textdegree), "%d", degree);
				argv[3] = textdegree;
				argc = 4;
				if (strcmp(fitdataname, "") != 0) {
					argv[4] = fitdataname;
					argc = 5;
				}
			}
			sprintf_s(listreport_filename, sizeof(listreport_filename), "%s List %s", dataname, listtype);
			if (strcmp(textdegree, "") != 0) {
				strncat_s(listreport_filename, sizeof(listreport_filename), " ", _TRUNCATE);
				strncat_s(listreport_filename, sizeof(listreport_filename), textdegree, _TRUNCATE);
			}
			if (strcmp(fitdataname, "") != 0) {
				strncat_s(listreport_filename, sizeof(listreport_filename), " ", _TRUNCATE);
				strncat_s(listreport_filename, sizeof(listreport_filename), fitdataname, _TRUNCATE);
			}
			strncat_s(listreport_filename, sizeof(listreport_filename), ".txt", _TRUNCATE);

			list(argc, argv, listreport_filename);

			EndDialog(hDlg, 1);
			break;

		case IDCANCEL:
			EndDialog(hDlg, NULL);
		}

		return TRUE;
	}

	return FALSE;
}

/*
  =======================================================================================
  =======================================================================================
*/
BOOL CALLBACK MapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char *maptype;
	char textdegree[10];
	int degree;
	char fitdataname[100];

	switch (msg) 
	{
	case WM_INITDIALOG:
		
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{

		case IDOK:
			maptype = "";
			degree = 0;
			fitdataname[0] = '\0';
			if (IsDlgButtonChecked(hDlg, IDC_MAP_CODE)) maptype = "code";
			if (IsDlgButtonChecked(hDlg, IDC_MAP_OBS)) maptype = "obs";
			if (IsDlgButtonChecked(hDlg, IDC_MAP_FIT)) maptype = "fit";
			if (IsDlgButtonChecked(hDlg, IDC_MAP_RESID)) maptype = "resid";
			if (*maptype == '\0') {
				MessageBox(0, "Please select one of code, obs, fit, resid", "ERROR", 0);
				return FALSE;
			}
			if (strcmp(maptype, "fit") == 0 || strcmp(maptype, "resid") == 0) {
				degree = GetDlgItemInt(hDlg, IDC_MAP_FIT_DEGREE, NULL, FALSE);
				if (degree <= 0 || degree > 25) {
					MessageBox(0, "Fit Degree must be specified and may not exceed 25.", "ERROR", 0);
					return FALSE;
				}
				GetDlgItemText(hDlg, IDC_MAP_FIT_NAME, fitdataname, sizeof(fitdataname));
			}

			int argc;
			char *argv[5];
			argc = 3;
			argv[0] = "map";
			argv[1] = dataname;
			argv[2] = maptype;
			textdegree[0] = '\0';
			if (strcmp(maptype, "fit") == 0 || strcmp(maptype, "resid") == 0) {
				sprintf_s(textdegree, sizeof(textdegree), "%d", degree);
				argv[3] = textdegree;
				argc = 4;
				if (strcmp(fitdataname, "") != 0) {
					argv[4] = fitdataname;
					argc = 5;
				}
			}
			sprintf_s(map_filename, sizeof(map_filename), "%s Map %s", dataname, maptype);
			if (strcmp(textdegree, "") != 0) {
				strncat_s(map_filename, sizeof(map_filename), " ", _TRUNCATE);
				strncat_s(map_filename, sizeof(map_filename), textdegree, _TRUNCATE);
			}
			if (strcmp(fitdataname, "") != 0) {
				strncat_s(map_filename, sizeof(map_filename), " ", _TRUNCATE);
				strncat_s(map_filename, sizeof(map_filename), fitdataname, _TRUNCATE);
			}
			strncat_s(map_filename, sizeof(map_filename), ".pdf", _TRUNCATE);

			map(argc, argv, hDlg, datadir, map_filename);

			EndDialog(hDlg, 1);
			break;

		case IDCANCEL:
			EndDialog(hDlg, NULL);
		}

		return TRUE;
	}

	return FALSE;
}