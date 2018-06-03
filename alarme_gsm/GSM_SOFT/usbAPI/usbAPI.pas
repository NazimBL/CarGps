//--------------------------------------------------------------------------------
// Interface for Usb-API for PIC 18F4550 Demo board
// Author: Gerhard Burger
// E-Mail: gCoolfire@yahoo.de
// Web: http://members.aon.at/geburger/
// Last Update: 2nd of June 2005
// Version 1.0
// Compiler: Borland Delphi, Version 7.0
// This software is free for non commercial use, for commercial use, please contact
// the author
//
// THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
// WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
// TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
// IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
// CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
//---------------------------------------------------------------------------------
Unit UsbAPI;
Interface
Uses Windows, SysUtils;

Const MPUSB_FAIL                 = 0;
      MPUSB_SUCCESS              = 1;
      MP_WRITE                   = 0;
      MP_READ                    = 1;
      MAX_NUM_MPUSB_DEV          = 127;

// MAX_NUM_MPUSB_DEV is an abstract limitation.
// It is very unlikely that a computer system will have more
// then 127 USB devices attached to it. (single or multiple USB hosts)

Function MPUSBGetDeviceCount(pVID_PID: PCHAR) : DWORD; cdecl;

Function MPUSBOpen(instance : DWORD ; pVID_PID :PCHAR; pEP : PCHAR;
                   dwDir : DWORD; dwReserved : DWORD): THANDLE;cdecl;

Function MPUSBGetDLLVersion():DWORD;cdecl;

// Reads a data package to the USB device
// HANDLE: Handle to the device
// pData: Pointer to the input buffer
// dwDataLen: expected count of bytes to read
// dwDataLenRead: actually count read data
// waiting time
Function MPUSBRead(HANDLE: THANDLE; pData :Pointer; dwDataLen :DWORD;
                   pDataLenRead :PDWORD;  dwMilliseconds :DWORD):DWORD; cdecl;

// Writes a data package to the USB device
// HANDLE: Handle to the device
// pData: Pointer to the output buffer
// dwDataLen: expected count of bytes to send
// dwDataLenSent: actually count of sent data
// waiting time
Function MPUSBWrite(HANDLE :THANDLE; pData :Pointer; dwDataLen : DWORD;
                    pLengthSent :PDWORD; dwMilliseconds :DWORD):DWORD; cdecl;

// see microchip documentation
Function MPUSBReadInt(HANDLE :THANDLE; pData :Pointer; dwDataLen : DWORD;
                      pLengthReceive :PDWORD; dwMilliseconds :DWORD):DWORD; cdecl;

Function MPUSBClose(HANDLE : THANDLE ): Boolean;cdecl;

Implementation

Function MPUSBGetDLLVersion;     cdecl; external  'MPUSBAPI.Dll' index 1;
Function MPUSBGetDeviceCount;    cdecl; external  'MPUSBAPI.Dll' index 2;
Function MPUSBOpen;              cdecl; external  'MPUSBAPI.Dll' index 3;
Function MPUSBRead;              cdecl; external  'MPUSBAPI.Dll' index 4;
Function MPUSBWrite;             cdecl; external  'MPUSBAPI.Dll' index 5;
Function MPUSBReadInt;           cdecl; external  'MPUSBAPI.Dll' index 6;
Function MPUSBClose;             cdecl; external  'MPUSBAPI.Dll' index 7;

end.
