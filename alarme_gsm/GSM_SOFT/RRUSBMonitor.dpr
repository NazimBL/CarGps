program RRUSBMonitor;

uses
  Forms,
  PICUSB in 'PICUSB.pas' {frmUsbMonitor},
  UsbAPI in 'usbAPI\usbAPI.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TfrmUsbMonitor, frmUsbMonitor);
  Application.Run;
end.
