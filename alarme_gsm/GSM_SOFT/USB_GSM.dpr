program USB_GSM;

uses
  Forms,
  PICUSB in 'PICUSB.pas' {USBGSM},
  UsbAPI in 'usbAPI\usbAPI.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TUSBGSM, USBGSM);
  Application.Run;
end.
