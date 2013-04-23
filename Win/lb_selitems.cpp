#define BUFFER MAX_PATH
#define BIGBUFF 8192
/*
* FUNCTION: DlgDelFilesProc(HWND, unsigned, UINT, LONG)
*
* PURPOSE: "Delete files" dialog box
*/
BOOL APIENTRY DlgDelFilesProc(
HWND hDlg, /* window handle of dialog box */
UINT message, /* type of message */
UINT wParam, /* message-specific information */
LONG lParam)
{
DWORD cchCurDir;
LPTSTR lpszCurDir;
LPTSTR lpszFileToDelete;
int nSelItems;
int nSelItemsInBuffer;
TCHAR tchBuffer[BUFFER];
TCHAR tchMsgBuff[BUFFER];
int nBuffer[BIGBUFF];
int i;
BOOL fResult;
HWND hListBox;
switch (message) {
case WM_INITDIALOG:
/*
* Initialize the list box by filling it with files from
* the current directory.
*/
lpszCurDir = tchBuffer;
GetCurrentDirectory(cchCurDir, lpszCurDir);
DlgDirList(hDlg, lpszCurDir, IDL_FILES, IDS_PATHTOFILL, 0);
SetFocus(GetDlgItem(hDlg, IDL_FILES));
return FALSE;
case WM_COMMAND:
switch (LOWORD(wParam)) {
case IDOK:
/*
* When the user presses the Delete (IDOK)
* button, delete all the selected files.
*/
lpszFileToDelete = tchBuffer;
hListBox = GetDlgItem(hDlg, IDL_FILES);
nSelItems = SendMessage(hListBox,
LB_GETSELCOUNT, 0, 0);
nSelItemsInBuffer = SendMessage(hListBox,
LB_GETSELITEMS, 512, (LPARAM) nBuffer);
if (nSelItems > nSelItemsInBuffer) {
MessageBox(hDlg, "Too many items selected.",
NULL, MB_OK);
}
else {
/*
* Go through the list backwards because after
* deleting an item the indices change for every
* subsequent item. By going backward, the
* indice are never invalidated.
*/
for (i = nSelItemsInBuffer - 1; i >= 0; i--) {
SendMessage(hListBox, LB_GETTEXT,
nBuffer[i],
(LPARAM) lpszFileToDelete);
fResult = DeleteFile(lpszFileToDelete);
if (!fResult) {
sprintf(tchMsgBuff,
"Could not delete file: %s "
"GetLastError = %u",
(LPARAM) lpszFileToDelete);
/* Call app-defined error handler. */
ErrorHandler(tchMsgBuff);
}
else {
SendMessage(hListBox, LB_DELETESTRING,
nBuffer[i], 0);
}
}
SendMessage(hListBox, LB_SETCARETINDEX, 0, 0);
}
return TRUE;
case IDCANCEL:
/* Destroy the dialog box. */
EndDialog(hDlg, TRUE);
return TRUE;
default:
return FALSE;
}
default:
return FALSE;
}
}
