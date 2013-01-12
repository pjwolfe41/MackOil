

#include <stdio.h>
#include <math.h>
#include "main.h"

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK FitDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ListDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

HINSTANCE ghInstance;


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

	switch (msg) 
	{
	case WM_INITDIALOG:
		
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case IDC_OPEN_DATA_BUTTON:
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