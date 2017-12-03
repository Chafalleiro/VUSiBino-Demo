/**
 * Project: Paper. 4 pin Fan http://chafalladas.com
 * Author: Alfonso Abelenda Escudero alfonso@abelenda.es
 * Inspired by V-USB example code by Christian Starkjohann and v-usb tutorials from http://codeandlife.com
 * Hardware based on tinyUSBboard http://matrixstorm.com/avr/tinyusbboard/ and paperduino perfboard from http://txapuzas.blogspot.com.es
 * Copyright: (C) 2017 Alfonso Abelenda Escudero
 * License: GNU GPL v3 (see License.txt)
 */
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>


void appendLogText(HWND hwndDlg, int infoBox, LPCTSTR newText)
{
//Log snippet based on these answers
//https://stackoverflow.com/questions/7200137/appending-text-to-a-win32-editbox-in-c

    int left,right;
    int len = SendMessage(GetDlgItem(hwndDlg, infoBox), EM_GETSEL,(WPARAM)&left,(LPARAM)&right);
    //Set the focus on the text edit control.
    SetFocus( GetDlgItem(hwndDlg, infoBox));
    // Simulate ctrl+page down to put the cursor at the bottom the box.
    HWND temp = GetActiveWindow(); //Prevent the keystrokes to be sent out of the dialog.
    if (temp == hwndDlg) // Then your current window has focus
    {
        keybd_event( VK_CONTROL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0 );
        keybd_event( VK_END, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0 );
    // Simulate a key release
        keybd_event( VK_CONTROL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
        keybd_event( VK_END, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    }
    //Enter the text to the log at the end of the textbox
    SendMessage(GetDlgItem(hwndDlg, infoBox), EM_GETSEL,(WPARAM)&left,(LPARAM)&right);
    SendMessage(GetDlgItem(hwndDlg, infoBox), EM_SETSEL, len, len);
    SendMessage(GetDlgItem(hwndDlg, infoBox), EM_REPLACESEL, 0, (LPARAM)newText);
    SendMessage(GetDlgItem(hwndDlg, infoBox), EM_SETSEL,left,right);
}


