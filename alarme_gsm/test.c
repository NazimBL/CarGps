#include <18F2550.h>
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN,NOMCLR
#use delay(clock=48000000)
#use rs232(baud=4800,xmit=pin_c6,stream=HOSTPC)
#use fast_io(a)
#use rs232(baud=4800,bits=8,xmit=pin_b1,rcv=pin_a3,stream=GPS)

void main()
{
   char c;
   while(1)
   {
      c=fgetc(GPS);
      fputc(c,HOSTPC);
   }
}
