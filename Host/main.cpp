/**
 * Project: VUSiBino demo http://chafalladas.com
 * Author: Alfonso Abelenda Escudero alfonso@abelenda.es
 * Inspired by V-USB example code by Christian Starkjohann and v-usb tutorials from http://codeandlife.com
 * Hardware based on tinyUSBboard http://matrixstorm.com/avr/tinyusbboard/ and paperduino perfboard from http://txapuzas.blogspot.com.es
 * Copyright: (C) 2017 Alfonso Abelenda Escudero
 * License: GNU GPL v3 (see License.txt)
 */

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <process.h>
#include <stdlib.h>
#include <Windowsx.h>
//Some control functions.
#include <dialog_functions.c>
#include <usb_functions.c>
#include <imported_functions.c>

// this is libusb, see http://libusb.sourceforge.net/

#include "resource.h"

// same as in vusibino.c
#define USB_LED_BLINK 0
#define USB_LED_ON 1
#define USB_LED_OFF 2
#define USB_SEND_MESSAGE  3
#define USB_READ_MESSAGE  4
#define USB_SET_SERIAL 5

unsigned char sw_led = 0;


//Open dialogs
HINSTANCE hInst;


//Function for Main dialog
BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int wmEvent; //Identifiers of control and eventes related with them
    char buffer[256], strInfo[256]; //Buffer of the messages

    time_t now = time(0);       //Get time of the event
    struct tm  tstruct;
    char       buf[80];

    HICON hLedOn;
    hLedOn = (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(ID_LED_ON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    HICON hLedOff;
    hLedOff = (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(ID_LED_OFF), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

    switch(uMsg)
    {
    case WM_INITDIALOG: //Initialize dialog
    {

#define USB_CFG_DEVICE_NAME_LEN 10
/* Same as above for the device name. If you don't want a device name, undefine
* the macros. See the file USB-IDs-for-free.txt before you assign a name if
* you use a shared VID/PID.
*/
// Serial  PLT001

        SendMessage(GetDlgItem(hwndDlg, ID_CHK_LED_OFF), BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(GetDlgItem(hwndDlg, ID_LED), STM_SETIMAGE, IMAGE_ICON, (LPARAM)hLedOff);

        HICON hIcon;
        hIcon = (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
        if (hIcon)
        {
            SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        }

        GetSerial(hwndDlg);
    }
    return TRUE;

    case WM_CLOSE: //Close Dialog
    {
        EndDialog(hwndDlg, 0);
    }
    return TRUE;

    case WM_COMMAND: //Various commands processing
        {

        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

        wmEvent = HIWORD(wParam);//These look kewl I'll save them for later

        switch(LOWORD(wParam))
            {
            case ID_CLOSE: //Close
                {
                    EndDialog(hwndDlg, 0);
                }
                return TRUE;

            case ID_SETSERIAL: //Set new serial number
                {
                    char p_serial[16];
                    Edit_GetLine(GetDlgItem(hwndDlg, ID_T_SERIAL),0,p_serial,16);
                    sendBytes(hwndDlg, ID_STATUS1, ID_STATUS1, USB_SET_SERIAL, "Set Serial", p_serial);
                    GetSerial(hwndDlg);
                }
                return TRUE;
            case ID_SEND: //Set new serial number
                {
                    char p_message[16];
                    Edit_GetLine(GetDlgItem(hwndDlg, ID_EDIT),0,p_message,16);
                    sendBytes(hwndDlg, ID_STATUS1, ID_STATUS1, USB_SEND_MESSAGE, "Sent message", p_message);
                }
                return TRUE;
            case ID_RECEIVE: //Set new serial number
                {
                    sendCommands(hwndDlg, ID_STATUS1, ID_RETURN, USB_READ_MESSAGE, "Read message", "GETBUF", true);
                }
                return TRUE;
            case ID_CHK_LED_ON:
                {
                    if (wmEvent == BN_CLICKED) //Is option clicked?
                    {
                        LRESULT chkState = SendMessage(GetDlgItem(hwndDlg, ID_CHK_LED_ON), BM_GETCHECK, 0, 0); //Which is the status?
                        if (chkState == BST_CHECKED)
                        {
                            sprintf(strInfo, "%s;Led ON -===========================-\r\n", buf); //Format the log line
                            appendLogText(hwndDlg, ID_LOG, strInfo);
                            sw_led = 1;
                            SendMessage(GetDlgItem(hwndDlg, ID_LED), STM_SETIMAGE, IMAGE_ICON, (LPARAM)hLedOn);
                            sendCommands(hwndDlg, ID_STATUS1, ID_STATUS1, USB_LED_ON, "Led ON -===========================-","Pin as output",true);
                        }
                    }
                }
                return TRUE;
            case ID_CHK_LED_OFF:
                {
                    if (wmEvent == BN_CLICKED) //Is option clicked?
                    {
                        LRESULT chkState = SendMessage(GetDlgItem(hwndDlg, ID_CHK_LED_OFF), BM_GETCHECK, 0, 0); //Which is the status?
                        if (chkState == BST_CHECKED)
                        {
                            sprintf(strInfo, "%s;Checkbox Led ON -===========================-\r\n", buf); //Format the log line
                            appendLogText(hwndDlg, ID_LOG, strInfo);
                            sw_led = 0;
                            SendMessage(GetDlgItem(hwndDlg, ID_LED), STM_SETIMAGE, IMAGE_ICON, (LPARAM)hLedOff);
                            sendCommands(hwndDlg, ID_STATUS1, ID_STATUS1, USB_LED_OFF, "Led on -===========================-","Pin as output",true);
                        }
                    }
                }
                return TRUE;
            case ID_CHK_LED_BLINK:
                {
                    if (wmEvent == BN_CLICKED) //Is option clicked?
                    {
                        LRESULT chkState = SendMessage(GetDlgItem(hwndDlg, ID_CHK_LED_BLINK), BM_GETCHECK, 0, 0); //Which is the status?
                        if (chkState == BST_CHECKED)
                        {
                            sprintf(strInfo, "%s;Checkbox Led blinking -===========================-\r\n", buf); //Format the log line
                            appendLogText(hwndDlg, ID_LOG, strInfo);
                            sw_led = 2;
                            SendMessage(GetDlgItem(hwndDlg, ID_LED), STM_SETIMAGE, IMAGE_ICON, (LPARAM)hLedOn);
                            sendCommands(hwndDlg, ID_STATUS1, ID_STATUS1, USB_LED_BLINK, "Led blinking -===========================-","Pin as output",true);
                        }
                    }
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    hInst=hInstance;
    InitCommonControls();
    return DialogBox(hInst, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
}

