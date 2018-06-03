
#include <18F2550.h>
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN,NOMCLR
#use delay(clock=48000000)
#USE RS232(BAUD=9600, XMIT=PIN_B0,INVERT,stream=DEBUG)
#include ".\include\alarme.h"
#include ".\include\sim508.h"
///////////////////////////////////////////////////////////////////////////////////////////////////
#define USB_HID_DEVICE     FALSE             // deshabilitamos el uso de las directivas HID
#define USB_EP1_TX_ENABLE  USB_ENABLE_BULK   // turn on EP1(EndPoint1) for IN bulk/interrupt transfers
#define USB_EP1_RX_ENABLE  USB_ENABLE_BULK   // turn on EP1(EndPoint1) for OUT bulk/interrupt transfers
#define USB_EP1_TX_SIZE    32                // size to allocate for the tx endpoint 1 buffer
#define USB_EP1_RX_SIZE    32                // size to allocate for the rx endpoint 1 buffer
#define USB_CON_SENSE_PIN PIN_B0

///////////////////////////////////////////////////////////////////////////////////////////////////
#include <pic18_usb.h>                      // Microchip PIC18Fxx5x Hardware layer for CCS's PIC USB driver
//#include ".\include\rr2_USB_Monitor.h"      // 
#include ".\include\EMNT_USB.h"               // 
#include <usb.c>                            // handles usb setup tokens and get descriptor reports

///////////////////////////////////////////////////////////////////////////////////////////////////

#use fast_io(b)
#use fast_io(c)
#use fast_io(a)
#use fast_io(e)
#define RecCommand   recbuf[0]

//#define LED    PIN_B5

const int8 Lenbuf = 32;
unsigned char m0,pass_add=100,k1,pass_l;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// RAM, RAM, RAM
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#define RecCommand   recbuf[0]

#define SAVE_N          0x01
#define SAVE_MSG        0x02
#define SEND_RS232      0x03
#define READ_N          0x04
#define READ_MSG        0x05
#define GET_RS232       0x06
#define GET_FIRMWARE    0x07
#define SAVE_FLAG       0x08

char Version[] = "v.1.0";
int  i,i2=0;
int16 count,count0;

int8 recbuf[Lenbuf];
int8 sndbuf[Lenbuf];
char flag0, tel_n2[10],al0;
unsigned char Rx,Rx0;
#bit al=flag0.1
//const int8 txt[64]={"Test de system"};

char x[15],y[15],z[8],c[6],s1[6],d[6],t[6];
char n_tel=0,k,n,n0,g,m=0;

/**************************************************************************************************/
#int_RDA
void  RDA_isr(void) 
{
   
   no_ok=0;
   Rx = fgetc(GSM);
   //fputc(Rx,DEBUG);
   //if (receive_flag ==0)
   //{
      if ((Rx == 10)&&(Rx0 == 13)&&(SerialBuffer_pos>0)) 
      {
         if(((SerialBuffer[0]=='O')&&(SerialBuffer[1]=='K'))||((SerialBuffer[1]=='O')&&(SerialBuffer[2]=='K')))
         {
            OK=1;
           // fprintf(DEBUG,"so=%s\n\r",SerialBuffer);
            
         }
         else
         {
            if((SerialBuffer[1]=='+')&&(SerialBuffer[2]=='C')&&(SerialBuffer[3]=='M')&&(SerialBuffer[4]=='T'))
            {
               sms_index=1;
            }
            else
            {
               if(sms_index==1)
               {
                  strcpy(sms_buffer,SerialBuffer);
                  sms_index=2;
               }
               else if(sms_index==2) strcpy(sms_buffer0,SerialBuffer);
               else strcpy(buffer,SerialBuffer);
            }
            OK=0;
            no_ok=1;
           // fprintf(DEBUG,"s=%s\n\r",buffer);
            //
         }
         SerialBuffer_pos = 0;
         receive_flag = 1;
         end_receive=1;
        // fputc('-',GPS);
        // fputc(0x0D,GPS);
      }
      else
      {
         if (Rx == 13) 
         {
            Rx0=13;
         }
         else
         {
            if(Rx=='>') 
            {
               msg_tx=1;
               //fputc('-',DEBUG);
            }
            else
            {
               SerialBuffer[SerialBuffer_pos] = Rx;
               SerialBuffer_pos++;
               SerialBuffer[SerialBuffer_pos] = 0;
               //receive_flag = 1;
               end_receive=0;
               //fputc(Rx,GPS);
               //fputc(0x0D,GPS);
            }
         }
   //   }
   }

}

//******************************************************************************
void sms(char *ntel)
{
   //fprintf(DEBUG,"ENVOYER LE MESSAGE au N°=%s\r\n",ntel);
   get_gps_pos(x,y,z);
   //fprintf(DEBUG,"x=%s ; y=%s ; z=%s\n\r",x,y,z);
   start:
   msg_tx=0;
   fprintf(GSM,"AT+CMGS=\"%s\"\r",ntel);
   if(wait_(100)==1)
   {
       start1:
       //fputc('+',DEBUG);
       switch(al0)
       {
         case 1:fprintf(GSM,"Alerte : Ouvertures Des portes de Vehicule Avec la Position GPS :");break;
         case 2:fprintf(GSM,"Alerte : Ouverture De capot de Vehicule Avec la Position GPS :");break;
         case 3:fprintf(GSM,"Alerte : Détection Des Vibrations de Vehicule Avec la Position GPS :");break;
         case 5:fprintf(GSM,"Alerte : Arret de Vehicule avec sms au Position GPS :");break;
                  default: fprintf(GSM,"Pas D'alerte :Vehicule au Position GPS :");
       }
       fprintf(GSM,"x=%s ; y=%s ; z=%s\n\r",x,y,z);
       delay_ms(2000);
       fputc(0x1a,GSM);
       output_low(led);
       msg_tx=0;
                // goto start1;*/
   }
   //else goto start;
}
//******************************************************************************/

int tx()
{        int n0,rep0;
         for(n0=0;n0<2;n0++)
         {
            read_tel_n(n0,tel_n);
            //start:
            //fprintf(DEBUG,"BIP au N°=%s\r\n",tel_n);
            if (bip(tel_n))
            {
               sms(tel_n);
               output_low(led);
               return 1;
            }
            //else fprintf(DEBUG," le N°=%s est hors champ\r\n",tel_n);
            delay_ms(2000);
         }   
       
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// M A I N
//
///////////////////////////////////////////////////////////////////////////////////////////////////

void main(void) 
{
   char etat;
   set_tris_b(0B11110011);
   set_tris_a(0B11001001);
   setup_comparator(NC_NC_NC_NC);
   enable_interrupts(INT_RDA);
   enable_interrupts(GLOBAL);
   RL_ARR=1;
   LED=1;
   write_alarm_outputs(out);
   delay_ms(1000);
   LED=0;
   write_alarm_outputs(out);
   delay_ms(1000);
   if(!input(USB_CON_SENSE_PIN))
   {
      delay_ms(1000);
      SIM_PowerOn();
      LED=1;
      write_alarm_outputs(out);
      delay_ms(7000);
      LED=0;
      write_alarm_outputs(out);
   }
   //fprintf(GSM,"AT+CSQ\r\n");
   //delay_ms(1000);
   //fprintf(GSM,"AT+COPS=?\r\n");
   //delay_ms(1000);
   //fprintf(GSM,"AT+COPS?\r\n");
   //delay_ms(5000);
   //fprintf(GSM,"AT+CMGL=0\r\n");
   //SIM_SendONOFF();
   write_eeprom(pass_add,4);
   write_eeprom(pass_add+1,'E');
   write_eeprom(pass_add+2,'M');
   write_eeprom(pass_add+3,'N');
   write_eeprom(pass_add+4,'T');
   etat=read_eeprom(255);
   if (etat==1) 
   {
      SIM2EEPROM(1);
      SIM2EEPROM(2);
      SIM2EEPROM(3);
      SIM2EEPROM(4);
   }
   if(input(USB_CON_SENSE_PIN))
   {
      disable_interrupts(global);
      usb_init();
      usb_task(); 
      enable_interrupts(global);
   }
   //fprintf(DEBUG,"BONJOUR\r\n");
  // get_gps_pos(x,y,z);
  // fprintf(DEBUG,"x=%s ; y=%s ; z=%s\n\r",x,y,z);
   
   
   msg_ok=1;
   while (TRUE)
   {
      if(input(USB_CON_SENSE_PIN))
      {
         if(usb_enumerated())
         {
         
            if (usb_kbhit(1))
            {  
               for(k=0;k<Lenbuf;k++) recbuf[k]=0;
               usb_get_packet(1, recbuf, Lenbuf);
               //printf("USB_OK\r");
               if(RecCommand==SAVE_N)// Sauvgarde des nemuros de telephones
               {
                  n=recbuf[2]-49;
                  //printf("%d %s\r\n",n,recbuf);
                  n0=n*10;
                  for(g=0;g<10;g++) 
                  {
                     tel_n[g]=recbuf[g+4];
                     write_eeprom(n0+g,tel_n[g]);
                     
                 }
                // WritePhoneBook(n,tel_n,' ');
                 
               }
               if(RecCommand==SAVE_MSG)// Sauvgarde des messages
               {
                  n=recbuf[2]-49;
                  //printf("%d %s\r\n",Lenbuf,recbuf);
                  n=n*32+40;
                  for(g=4;g<Lenbuf;g++) write_eeprom(n+(g-4),recbuf[g]);
               }
               if(RecCommand==READ_N)
               {
                  usb_put_packet(1,n_tel,6,USB_DTS_TOGGLE);
               }
               
               if(RecCommand==SEND_RS232)
               {
                  printf("%s\r\n",recbuf);
               }
               if(RecCommand==GET_FIRMWARE)
               {
                  usb_put_packet(1,Version,6,USB_DTS_TOGGLE);
               }  
               if(RecCommand==SAVE_FLAG)
               {
                  if (recbuf[1]=='0') write_eeprom(255,0);
                  else write_eeprom(255,1);
               }
            }
         
         }
      }
      else 
      {
      //-----------------------------------------------------------------------
         if(input(TELE_LK))
         {
            if(!TELE_LK_OLD)
            {

               ARM=1;
               ARM_action();
               TELE_LK_OLD=1;
               ALARM=0;
               
            }

         }
         else TELE_LK_OLD=0;
         if(input(TELE_N_LK))
         {
            if(!TELE_N_LK_OLD)
            {
               
               ARM=0;
               ALARM=0;
               msg_ok=1;
               D_ARM_action();
               TELE_N_LK_OLD=1;
               
            }
         }
          else TELE_N_LK_OLD=0;
         if(input(TELE_SN))
         {
            if(!TELE_SN_OLD)
            {
               SRN_EN=1;
               //ARM_action();
               TELE_SN_OLD=1;
            }
            
         }
         TELE_SN_OLD=0;
         if(input(TELE_N_SN))
         {
            if(!TELE_N_SN_OLD)
            {
               SRN_EN=0;
               ALARM=0;
               //D_ARM_action();
               TELE_N_SN_OLD=1;
            }
         }
         TELE_N_SN_OLD=0;
         al0=read_alarm_input();
         if(al0>0)
         {
            if(ARM)
            {
               ALARM=1;
               count0=0;
               
            }
            else 
            {
               ALARM=0;
               msg_ok=1;
               SRN=0;
               LMR=0;
            }
         }
        // else 
         if(ARM)
         {
            count++;
            if(count>40000)
            {  
               //fprintf(DEBUG,"Al0=%d\r\n",al0);
               if((msg_ok)&&(ARM)&&(ALARM)) 
               {
                 // fprintf(DEBUG,"Alarme message \r\n");
                 SRN=1;
                 LMR=1;
                 write_alarm_outputs(out);
                 tx();
                 msg_ok=0;
               }
               //else fprintf(DEBUG,"Alarme message non autorise\r\n");
               count=0;
               if(LED) LED=0;
               else LED=1;
               count0++;
               if(ALARM)
               {  
                  count0++;
                  if(count0>800)
                  {
                     ALARM=0;
                     
                  }
                  //fprintf(DEBUG,"count0=%ld\n\r",count0);
                  if(SRN_EN) SRN=1;
                  else SRN=0;
                  if(LMR) LMR=0;
                  else LMR=1;
               }
               else 
               {
                 
                  SRN=0;
                  LMR=0;
               }
               write_alarm_outputs(out);
            }
         }
      
      
      //-----------------------------------------------------------------------
      if(sms_index==1)
      {
         LED=1;
         write_alarm_outputs(out);
         fprintf(GSM,"AT+CMGR=1\r\n");
         t0=0;
         while((sms_index<2)&&(t0<1000))
         {
            t0++;
            delay_ms(10);
         }
         if((t0<999)&&(sms_index==2))
         {
            get_sms_n(tel_n1);
            for(i2=0;i2<4;i2++)
            {
               read_tel_n(i2,tel_n2);
               if(strncmp(tel_n2,tel_n1,10)==0) 
               {
                 //fprintf(DEBUG,"Get Postion\n\r");
                 //fprintf(DEBUG,"x=%s ; y=%s ; z=%s\n\r",x,y,z);
                 if((sms_buffer[0]=='A')||(sms_buffer[0]=='a'))
                 {
                     if(sms_buffer[1]=='*')
                     {
                        k1=0;
                        pass_l=read_eeprom(pass_add);
                        for(m0=1;m0<pass_l+1;m0++)
                        {
                           if(sms_buffer[m0+1]==read_eeprom(pass_add+m0)) k1++;
                           else break;                          
                           
                        }
                        if(k1==pass_l)
                        {
                           RL_ARR=0;
                           LED=1;
                           write_alarm_outputs(out);
                           sms(tel_n1);
                           
                        }
                     }
                 }
                 if((sms_buffer[0]=='C')||(sms_buffer[0]=='c'))
                 {
                 }
                 break;
            }
              // else 
               //fprintf(DEBUG,"N° tel= %s N° Tel0=%s\n\r",tel_n2,tel_n1);
               //delay_ms(1000);
         }
   
         delay_ms(1000); 
         fprintf(GSM,"AT+CMGD=1\r\n");
         LED=0;
         write_alarm_outputs(out);
         sms_index=0;
         }
      }
      if(input(PIN_B1))
         {
            LED=1;
            write_alarm_outputs(out);
            for(i2=0;i2<4;i2++)
            {
               read_tel_n(i2,tel_n2);
               get_ring_n(tel_n1);
               if(strncmp(tel_n2,tel_n1,10)==0) 
               {
                 //fprintf(DEBUG,"Get Postion\n\r");
                 //fprintf(DEBUG,"x=%s ; y=%s ; z=%s\n\r",x,y,z);
                 sms(tel_n1);
                 break;
               }
              // else 
               //fprintf(DEBUG,"N° tel= %s N° Tel0=%s\n\r",tel_n2,tel_n1);
               //delay_ms(1000);
            }
               LED=0;
               write_alarm_outputs(out);
            
         }
         //------------------------------------------------------------------
      }
      
      
   }
}
