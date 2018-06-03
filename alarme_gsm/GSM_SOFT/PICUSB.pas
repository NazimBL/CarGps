///////////////////////////////////////////////////////////////////////////////////////////////////
//
//   Ejemplo de comunicaciones PIC <-> PC vía USB 2.0
//
//   by Redraven http://picmania.garcia-cuervo.com
//
///////////////////////////////////////////////////////////////////////////////////////////////////

unit PICUSB;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, usbAPI, ExtCtrls, shellapi,
  ComCtrls, jpeg, Buttons, LMDCustomButton, LMDButton, LMDCustomControl,
  LMDCustomPanel, LMDCustomBevelPanel, LMDCustomParentPanel,
  LMDCustomGroupBox, LMDGroupBox, LMDBaseEdit, LMDCustomMemo, LMDMemo,
  LMDCustomEdit, LMDCustomMaskEdit, LMDCustomExtSpinEdit, LMDSpinEdit,
  LMDCustomComponent, LMDContainerComponent, LMDBaseDialog, LMDAboutDlg,
  LMDControl, LMDBaseControl, LMDBaseGraphicControl, LMDBaseLabel,
  LMDCustomLabel, LMDCustomLabelFill, LMDLabelFill, LMDEdit,
  LMDCustomPanelFill, LMDButtonPanel, LMDGraphicControl, LMDBaseImage,
  LMDCustomNImage, LMDNImage, CPortCtl, OleCtrls;
Const
    vid_pid  : PCHAR  = 'vid_0777&pid_5084'+#0;
   // vid_pid  : PCHAR  = 'vid_04D8&pid_000B'+#0;
    out_pipe : PCHAR = '\MCHP_EP1' + #0;
    in_pipe  : PCHAR = '\MCHP_EP1' + #0;


    GET_FIRMWARE      = $07;
    SAVE_N        = $01;
    SAVE_MSG      = $02;
    SEND_RS232    = $03;

    READ_MSG      = $05;
    GET_RS232     = $06;
    SAVE_FLAG     = $08;
    GET_MODULE    = $0E;
    READ_N1        = $14;
    READ_N2       = $15;
    READ_N3        = $16;
    READ_N4        = $17;
    SAVE_SM    =$09;
    SAVE_SN    =$0A;
    SOLD_FLAG  =$0B;
    SAVE_PWD   =$0C;
    MA_FLAG    =$0D;

    UsbBufSize    = 64;

     

Type
    PByteBuffer = ^TByteBuffer;
    TByteBuffer = Array[0..63] of Byte;
    PUsbData    = ^TUsbData;
    TUSBData    = Record
       Cmd:       Byte;
       Data:      Array[0..UsbBufSize-1] of Byte;
    End;
    TVersionInfo = Array[0..3] of Byte;



    TUSBGSM = class(TForm)
    StatusBar1: TStatusBar;
    rc_box: TLMDGroupBox;
    LMDButton1: TLMDButton;
    LMDButton2: TLMDButton;
    LMDButton3: TLMDButton;
    in_box: TLMDGroupBox;
    LMDEdit1: TLMDEdit;
    LMDEdit2: TLMDEdit;
    LMDEdit3: TLMDEdit;
    LMDEdit4: TLMDEdit;
    LMDLabelFill1: TLMDLabelFill;
    LMDLabelFill2: TLMDLabelFill;
    LMDLabelFill3: TLMDLabelFill;
    LMDLabelFill4: TLMDLabelFill;
    LMDButton4: TLMDButton;
    LMDButton5: TLMDButton;
    CheckBox1: TCheckBox;
    Label1: TLabel;
    LMDNImage2: TLMDNImage;
    LMDButtonPanel1: TLMDButtonPanel;
    LMDMemo2: TLMDMemo;
    LMDMemo1: TLMDMemo;
    LMDNImage1: TLMDNImage;
    LMDButton6: TLMDButton;
    LMDButton7: TLMDButton;
    LMDButton8: TLMDButton;
    LMDButton9: TLMDButton;
    LMDButton10: TLMDButton;
    LMDButton11: TLMDButton;
    LMDButton12: TLMDButton;
    LMDEdit5: TLMDEdit;
    LMDEdit6: TLMDEdit;
    procedure FormCreate(Sender: TObject);
    procedure btnGetUSBDriverVersionClick(Sender: TObject);
    procedure btnGetUSBSummaryClick(Sender: TObject);
    procedure btnGetFirmwareVersionClick(Sender: TObject);
    procedure btnClearDumpClick(Sender: TObject);
    procedure BtnCerrarClick(Sender: TObject);
    procedure LMDButton1Click(Sender: TObject);
    procedure LMDButton2Click(Sender: TObject);
    procedure LMDButton3Click(Sender: TObject);
    procedure LMDButton4Click(Sender: TObject);
    procedure LMDButtonPanel1Buttons0Click(Sender: TObject;
      index: Integer);
    procedure LMDButtonPanel1Buttons1Click(Sender: TObject;
      index: Integer);
    procedure LMDButtonPanel1Buttons2Click(Sender: TObject;
      index: Integer);
    procedure LMDButton5Click(Sender: TObject);
    procedure LMDButton10Click(Sender: TObject);
    procedure LMDButton6Click(Sender: TObject);
    procedure LMDMemo2DblClick(Sender: TObject);
    procedure LMDButton8Click(Sender: TObject);
    procedure LMDButton9Click(Sender: TObject);
  private
  public
    myOutPipe: THANDLE;
    myInPipe:  THANDLE;
    function   USBCheckInvalidHandle():string;
    function   USBSendReceivePacket(SendData: PByteBuffer; SendLength: DWORD; ReceiveData: PByteBuffer; var ReceiveLength: DWORD; SendDelay: Word; ReceiveDelay:Word):DWORD;
    function   GetUSBSummary():Integer;
    function   GetUSBDriverVersion(): TVersionInfo;
    function   SendUSBSimpleCommand(Command: byte; Title: string): string;
    function   SendUSBParamCommand(Command: byte; Param: string; Title: string): string;
    function   GetUSBRequest(Command: byte; ReceiveLength: DWORD; Title: string; Prefix: string): string;
  end;

var
  USBGSM: TUSBGSM;
  tx_pid,tx_vid,al_pid,al_vid,tg_pid,tg_vid:string;

implementation

{$R *.dfm}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Funciones de comuniaciones USB
//
///////////////////////////////////////////////////////////////////////////////////////////////////

function TUSBGSM.USBCheckInvalidHandle():string;
var
   res: string;
Begin
  if (GetLastError() = ERROR_INVALID_HANDLE) then
  Begin
    MPUSBClose(myOutPipe);
    MPUSBClose(myInPipe);
    myOutPipe := INVALID_HANDLE_VALUE;
    myInPipe  := INVALID_HANDLE_VALUE;
    res := 'USB INVALID_HANDLE_VALUE';
  End
  else
    res := 'Error Code ' + IntToStr(GetLastError());
  result := res;
End;

Function TUSBGSM.USBSendReceivePacket(SendData: PByteBuffer; SendLength: DWORD; ReceiveData: PByteBuffer; var ReceiveLength: DWORD; SendDelay: Word; ReceiveDelay:Word):DWORD;
var
   SentDataLength:  DWORD ;
   ExpectedReceiveLength: DWORD;
Begin
     ExpectedReceiveLength := ReceiveLength;
     if(myOutPipe <> INVALID_HANDLE_VALUE) and ( myInPipe <> INVALID_HANDLE_VALUE) then
     Begin
          if MPUSBWrite(myOutPipe,SendData,SendLength,@SentDataLength,SendDelay) <> 0 then
          Begin
               if(MPUSBRead(myInPipe,ReceiveData,ExpectedReceiveLength,@ReceiveLength,ReceiveDelay)) <> 0 then
               Begin
                    if (ReceiveLength = ExpectedReceiveLength) Then
                    Begin
                         result:=1;
                         Exit;
                    End
                    else
                    Begin
                         if (ReceiveLength < ExpectedReceiveLength) then
                         begin
                              result:=2;
                              Exit;
                         End;
                    End
               End
               else
               lmdMemo1.Lines.Add(USBCheckInvalidHandle());
          End
          else
              lmdMemo1.Lines.Add(USBCheckInvalidHandle());
     End;
     result:=0;
End;

function TUSBGSM.GetUSBSummary():Integer;
Var
   tempPipe:THandle;
   count:DWORD;
   max_Count:DWORD;
   i,compResult:Byte;
Begin
     max_count := MPUSBGetDeviceCount(vid_pid);
     if(max_count=0) then
     Begin
          result:= max_count;
          lmdMemo1.Lines.add('No device found');
          exit;
     End
     Else
         CompResult :=StrComp(PChar('vid_'+tx_vid+'&pid_'+tx_pid),PChar(vid_pid));
         if compresult=0 then lmdmemo1.lines.add( 'Transmiteur GSM est Détecté')
         else
         begin
            CompResult :=StrComp(PChar('vid_'+al_vid+'&pid_'+al_pid),PChar(vid_pid));
            if compresult=0 then lmdmemo1.lines.add( 'Alarme GSM est Détecté')
            else
            begin
                 CompResult :=StrComp(PChar('vid_'+tg_vid+'&pid_'+tg_pid),PChar(vid_pid));
                 if compresult=0 then lmdmemo1.lines.add( 'System de telegesion GSM est Détecté')
                  else lmdmemo1.lines.add( ' currently attached');
            end;
         end;
     count := 0;
     For i:=0 to MAX_NUM_MPUSB_DEV-1 Do
     Begin
          tempPipe := MPUSBOpen(i,vid_pid,NIL,MP_READ,0);
          if(tempPipe <> INVALID_HANDLE_VALUE) then
          Begin
               //memo1.lines.add('Instance Index ' + IntToStr(i));
               MPUSBClose(tempPipe);
               Inc(count);
          end;
          if(count = max_count) Then break;
     end;
     result:= max_count;
End;

function TUSBGSM.GetUSBDriverVersion(): TVersionInfo;
var
   temp:DWORD;
   VersionInfo: TVersionInfo;
begin
  temp := MPUSBGetDLLVersion();
  move(temp,VersionInfo,sizeof(VersionInfo));
  result := VersionInfo;
end;

function TUSBGSM.GetUSBRequest(Command: byte; ReceiveLength: DWORD; Title: string; Prefix: string): string;
Var
   Selection:   DWORD;
   RecvLength:  DWORD;
   send_buf:    TByteBuffer;
   receive_buf: TByteBuffer;
   p: array[0..UsbBufSize-1] of char;
   i: integer;
   s: string;
Begin
    Selection :=0;
    myOutPipe := MPUSBOpen(selection,vid_pid, out_pipe, MP_WRITE, 0);
    myInPipe  := MPUSBOpen(selection,vid_pid, in_pipe,  MP_READ,  0);
    If (myOutPipe = INVALID_HANDLE_VALUE) or (myInPipe = INVALID_HANDLE_VALUE) then
    Begin
      s := 'USB Failed to open data pipes.';
      Exit;
    End;
    send_buf[0] := Command;
    RecvLength  := ReceiveLength;
    if (USBSendReceivePacket(@send_buf,1,@receive_buf,RecvLength,1000,1000) = 1) Then
    Begin
         for i:=0 to ReceiveLength do p[i] :=Chr(receive_buf[i]);
         s := Prefix+strpas(p);
    End
    Else
      s := 'USB Operation Failed : '+Title;
    MPUSBClose(myOutPipe);
    MPUSBClose(myInPipe);
    myOutPipe := INVALID_HANDLE_VALUE;
    myInPipe := INVALID_HANDLE_VALUE;
    result := s;
end;

function TUSBGSM.SendUSBSimpleCommand(Command: byte; Title: string): string;
var
   selection: DWORD;
   send_buf: TUsbData;
   SentDataLength: DWORD;
   s: string;
Begin
  Selection:=0;
  myOutPipe := MPUSBOpen(selection, vid_pid, out_pipe, MP_WRITE, 0);
  if (myOutPipe = INVALID_HANDLE_VALUE) then
  Begin
    s := 'USB Failed to open out data pipe';
    Exit;
  End;
  send_buf.Cmd := Command;
  if MPUSBWrite(myOutPipe,@Send_buf,1,@SentDataLength,1000) <> 0 then
  Begin
    if (SentDataLength <> 1) Then
      s := 'USB Failure on sending : '+Title
    else
      s := Title+' : sended Ok'
  End
  else
    s := USBCheckInvalidHandle();
  MPUSBClose(myOutPipe);
  myOutPipe := INVALID_HANDLE_VALUE;
  result := s;
end;

function TUSBGSM.SendUSBParamCommand(Command: byte; Param: string; Title: string): string;
var
   selection: DWORD;
   send_buf: TUsbData;
   SentDataLength,CompareSentDataLength: DWORD;
   tmp: array[0..UsbBufSize-1] of char;
   i,count0: integer;
   s: string;
   label debut;
Begin

  Selection:=0;
  debut:
  myOutPipe := MPUSBOpen(selection, vid_pid, out_pipe, MP_WRITE, 0);
  if (myOutPipe = INVALID_HANDLE_VALUE) then
  Begin
    s := 'USB Failed to open out data pipe';
    selection:=selection+1;
    if selection<5 then goto debut
    else result := s;
  End;
  
  send_buf.Cmd := Command;
  StrPCopy(tmp,Param);
  CompareSentDataLength := Length(Param);
  
  for i:=0 to CompareSentDataLength do
  begin
       send_buf.Data[i] := ord(tmp[i]);
  end;
  if MPUSBWrite(myOutPipe,@Send_buf,CompareSentDataLength+1,@SentDataLength,1000) <> 0 then
  Begin
    if (SentDataLength <> CompareSentDataLength+1) Then
      s := 'USB Failure on sending : '+Title
    else
      s := Title+' : sended Ok'
  End
  else
   s := USBCheckInvalidHandle();
   MPUSBClose(myOutPipe);
  myOutPipe := INVALID_HANDLE_VALUE;
  result := s;
end;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// De Formulario
//
///////////////////////////////////////////////////////////////////////////////////////////////////

procedure TUSBGSM.FormCreate(Sender: TObject);
begin
     myInPipe  := INVALID_HANDLE_VALUE;
     myOutPipe := INVALID_HANDLE_VALUE;
     tx_vid:='0777';
     al_vid:='0777';
     tg_vid:='0777';
     tx_pid:='5084';
     al_pid:='5082';
     tg_pid:='5083';

end;

procedure TUSBGSM.btnGetUSBDriverVersionClick(Sender: TObject);
var
   miVersionInfo: TVersionInfo;
begin
  miVersionInfo := GetUSBDriverVersion();
  lmdmemo1.text:='USB Driver Version '+intTOStr(miVersionInfo[0])+ '.'+intTOStr(miVersionInfo[1])+'.'+intTOStr(miVersionInfo[2])+'.'+intTOStr(miVersionInfo[3]);
end;

procedure TUSBGSM.btnGetUSBSummaryClick(Sender: TObject);
begin
     GetUSBSummary();
end;

procedure TUSBGSM.btnClearDumpClick(Sender: TObject);
begin
     lmdMemo1.Lines.Clear;
end;

procedure TUSBGSM.btnGetFirmwareVersionClick(Sender: TObject);
Begin
     lmdMemo1.lines.add(GetUSBRequest(GET_FIRMWARE,6,'Get Firmware','USB Firmware : '));
end;


procedure TUSBGSM.BtnCerrarClick(Sender: TObject);
begin
     Close;
end;

procedure TUSBGSM.LMDButton1Click(Sender: TObject);
begin
  GetUSBSummary();
end;

procedure TUSBGSM.LMDButton2Click(Sender: TObject);
begin
  lmdMemo1.lines.add(GetUSBRequest(GET_FIRMWARE,6,'Get Firmware','USB Firmware : '));
end;

procedure TUSBGSM.LMDButton3Click(Sender: TObject);
var miVersionInfo: TVersionInfo;
begin
  miVersionInfo := GetUSBDriverVersion();
  lmdmemo1.text:='USB Driver Version '+intTOStr(miVersionInfo[0])+ '.'+intTOStr(miVersionInfo[1])+'.'+intTOStr(miVersionInfo[2])+'.'+intTOStr(miVersionInfo[3]);
end;

procedure TUSBGSM.LMDButton4Click(Sender: TObject);
var
   miVersionInfo: TVersionInfo;
begin

  lmdMemo2.lines.add(SendUSBParamCommand(SAVE_N,'N1='+lmdedit1.Text,'Commande SAVE 1ere N° Tel'));
  lmdMemo2.lines.add(SendUSBParamCommand(SAVE_N,'N2='+lmdedit2.Text,'Commande SAVE 2eme N° Tel'));
  lmdMemo2.lines.add(SendUSBParamCommand(SAVE_N,'N3='+lmdedit3.Text,'Commande SAVE 3eme N° Tel'));
  lmdMemo2.lines.add(SendUSBParamCommand(SAVE_N,'N4='+lmdedit4.Text,'Commande SAVE 4eme N° Tel')) ;
  if checkbox1.Checked then lmdMemo2.lines.add(SendUSBParamCommand(SAVE_FLAG,'1','Licture de numéros de SIM autorisé '))
  else lmdMemo2.lines.add(SendUSBParamCommand(SAVE_FLAG,'0','Licture de numéros de SIM non autorisé ')) ;
end;

procedure TUSBGSM.LMDButtonPanel1Buttons0Click(Sender: TObject;
  index: Integer);
begin
  in_box.Visible:=false;
  rc_box.Top:=80;
  rc_box.Left:=224;
  rc_box.Width:=481;
  rc_box.Height:=409;
  rc_box.Visible:=true;
end;

procedure TUSBGSM.LMDButtonPanel1Buttons1Click(Sender: TObject;
  index: Integer);
begin
 rc_box.Visible:=false;
  in_box.Top:=80;
  in_box.Left:=224;
  in_box.Width:=481;
  in_box.Height:=409;
  in_box.Visible:=true;
end;

procedure TUSBGSM.LMDButtonPanel1Buttons2Click(Sender: TObject;
  index: Integer);
begin
  rc_box.Visible:=false;
  in_box.Visible:=false;
  
end;

procedure TUSBGSM.LMDButton5Click(Sender: TObject);
begin
  lmdMemo2.lines.add(GetUSBRequest(READ_N1,10,'Lire Tel N ','Tele N1 : '));
  lmdMemo2.lines.add(GetUSBRequest(READ_N2,10,'Lire Tel N ','Tele N2 : '));
  lmdMemo2.lines.add(GetUSBRequest(READ_N3,10,'Lire Tel N ','Tele N3 : '));
   lmdMemo2.lines.add(GetUSBRequest(READ_N4,10,'Lire Tel N ','Tele N4 : '));
end;

procedure TUSBGSM.LMDButton10Click(Sender: TObject);
begin
    lmdMemo2.lines.add(GetUSBRequest(GET_MODULE,9,'GET_MODULE','MODULE : '));
end;

procedure TUSBGSM.LMDButton6Click(Sender: TObject);
begin
 lmdMemo2.lines.add(GetUSBRequest(GET_FIRMWARE,6,'Get Firmware','USB Firmware : '));
end;

procedure TUSBGSM.LMDMemo2DblClick(Sender: TObject);
begin
       LMDMemo2.Clear;
end;

procedure TUSBGSM.LMDButton8Click(Sender: TObject);
begin
  lmdMemo2.lines.add(SendUSBParamCommand(SAVE_PWD,chr(Length(LMDEdit6.Text))+LMDEdit6.Text,'Save PWD'));
end;

procedure TUSBGSM.LMDButton9Click(Sender: TObject);
begin
       lmdMemo2.lines.add(SendUSBParamCommand(SAVE_SN,chr(Length(LMDEdit5.Text))+LMDEdit5.Text,'Save Solde'));
end;

end.

