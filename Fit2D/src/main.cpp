

#include <stdio.h>
#include <math.h>
#include <direct.h>
#include "main.h"
#include "readdata.h"

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK FitDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ListDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

HINSTANCE ghInstance;

char dataname[100] = "";


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

	switch (msg) 
	{
	case WM_INITDIALOG:
		
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case IDC_OPEN_DATA_BUTTON:
			SetDlgItemText(hDlg, IDC_DATA_PATH, "");
			SetDlgItemText(hDlg, IDC_DATA_TITLE, "");

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
			break;

		case IDC_FIT_BUTTON:
			DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_FIT_DLG), hDlg, FitDlgProc, NULL);
			break;

		case IDC_LIST_BUTTON:
			DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_LIST_DLG), hDlg, ListDlgProc, NULL);
			break;

		case IDC_MAP_BUTTON:
			DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_MAP_DLG), hDlg, MapDlgProc, NULL);
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
	switch (msg) 
	{
	case WM_INITDIALOG:
		
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{

		case IDOK:
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
BOOL CALLBACK ListDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	case WM_INITDIALOG:
		
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{

		case IDOK:
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
	switch (msg) 
	{
	case WM_INITDIALOG:
		
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{

		case IDOK:
			break;

		case IDCANCEL:
			EndDialog(hDlg, NULL);
		}

		return TRUE;
	}

	return FALSE;
}