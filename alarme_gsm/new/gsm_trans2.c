
#include <18F2550.h>
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN,NOMCLR
#use delay(clock=48000000)
#define GPS 1
//#define DEBUG0 1
#ifdef DEBUG0 
   #USE RS232(BAUD=9600, XMIT=PIN_B0,INVERT,stream=DEBUG)
#endif
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
#define DB 1
//#define LED    PIN_B5

const int8 Lenbuf = 32;

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
#define SAVE_SM         0x09 
#define SAVE_SN         0x0A 
#define SOLD_FLAG       0x0B 
#define SAVE_PWD        0x0C
#define MA_FLAG         0x0D 
#define GET_MODULE      0x0E

char Version[] = "v.1.0";
#ifdef GPS
   char Module[] = "AL_SIM508";
#else 
   char Module[] = "AL_SIM300";
#endif
int  i2=0;
int16 count,count0;

int8 recbuf[Lenbuf];
int8 sndbuf[Lenbuf];
char flag0, tel_n2[10],al0,rs232_l=1;
unsigned char Rx,Rx0;
#bit al=flag0.1
//const int8 txt[64]={"Test de system"};

char x[15],y[15],z[8];
char n_tel=0,k,n,n0,g;

/**************************************************************************************************/
#int_RDA
void  RDA_isr(void) 
{
   
   no_ok=0;
   Rx = fgetc(GSM);
  // fputc(Rx,DEBUG);
   //if (receive_flag ==0)
   //{
      if ((Rx == 10)&&(Rx0 == 13)&&(SerialBuffer_pos>0)) 
      {
         if(((SerialBuffer[0]=='O')&&(SerialBuffer[1]=='K'))||((SerialBuffer[1]=='O')&&(SerialBuffer[2]=='K')))
         {
            OK=1;
         }
         else
         {
             if(sms_index==0)
             {
               strcpy(buffer,SerialBuffer);
               rs232_l=SerialBuffer_pos;
               for(pos=0;pos<SerialBuffer_pos;pos++)
               {
                  if(SerialBuffer[pos]=='+')
                  {
                     #ifdef DEBUG0
                     fputc('*',DEBUG); 
                     #endif
                    if((SerialBuffer[pos+2]=='M')&&(SerialBuffer[pos+3]=='T'))//CMTI
                    {  
                       sms_index=1;
                       sms_ok=1;
                       index_l=0;
                    }
                    if((SerialBuffer[pos+2]=='U')&&(SerialBuffer[pos+3]=='S'))//CUSD
                    {
                        if(sold_ask)
                       {
                           sold=1;
                           sold_msg_l=SerialBuffer_pos;
                        }
                      }
                  }
               }  
             }
             else if(sms_index==1)
             {
                sms_buff_l=SerialBuffer_pos;
                strcpy(sms_buffer,SerialBuffer);
                sms_index=2;
                sms_ok=1;
                
             }
             else if(sms_index==2) 
             {
               
                strcpy(sms_buffer0,SerialBuffer);
                sms_index=3;
                sms_ok=1;
             }
            OK=0;
            no_ok=1;
         }
         SerialBuffer_pos = 0;
         receive_flag = 1;
         end_receive=1;
        
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
               #ifdef DEBUG0 
                  fputc('-',DEBUG);
               #endif
            }
            else
            {
               SerialBuffer[SerialBuffer_pos] = Rx;
               SerialBuffer_pos++;
               SerialBuffer[SerialBuffer_pos] = 0;
               //receive_flag = 1;
               end_receive=0;
               
            }
         }
      }

}

//******************************************************************************
void sms(char *ntel)
{
   #ifdef DEBUG0
      fprintf(DEBUG,"ENVOYER LE MESSAGE au N°=%s\r\n",ntel);
   #endif
   #ifdef GPS
      get_gps_pos(x,y,z);
      get_gps_speed(speed1,direction0);
   #endif
   #ifdef DEBUG0
      fprintf(DEBUG,"x=%s ; y=%s ; z=%s\n\r",x,y,z);
   #endif
   start:
   msg_tx=0;
   fprintf(GSM,"AT+CMGS=\"%s\"\r\n",ntel);
   if(wait_(100)==1)
   {
       start1:
       //fputc('+',DEBUG);
       switch(al0)
       {
         #ifdef GPS
            case 1:fprintf(GSM,"Alerte : Ouvertures Des portes de Vehicule Avec la Position GPS :");
                   fprintf(GSM,"x=%s ; y=%s ; z=%s\n\r",x,y,z);    
                   break;
            case 2:fprintf(GSM,"Alerte : Ouverture De capot de Vehicule Avec la Position GPS :");
                   fprintf(GSM,"x=%s ; y=%s ; z=%s\n\r",x,y,z);
                   break;
            case 3:fprintf(GSM,"Alerte : Détection Des Vibrations de Vehicule Avec la Position GPS :");
                   fprintf(GSM,"x=%s ; y=%s ; z=%s\n\r",x,y,z);
                   break;
            case 5:fprintf(GSM,"Alerte : Arret de Vehicule avec sms au Position GPS :");
                   fprintf(GSM,"x=%s ; y=%s ; z=%s\n\r",x,y,z);
                   break;
            case 6:fprintf(GSM,"Alerte : Le Solde de Votre Puce d'Alarme est en dessous du Solde Minimum :");
                   break;
            default:fprintf(GSM,"Pas D'alerte :Vehicule au Position GPS :");
                   fprintf(GSM,"Lat=%s\r\n Lon=%s\r\nAlt=%s\n\r",x,y,z);
                   fprintf(GSM,"Vet=%s\r\nDir=%s\r\n",speed1,Direction0);
         #else
            case 1:fprintf(GSM,"Alerte : Ouvertures Des portes de Vehicule.");
                   break;
            case 2:fprintf(GSM,"Alerte : Ouverture De capot de Vehicule.");
                   break;
            case 3:fprintf(GSM,"Alerte : Détection Des Vibrations.");
                   break;
            case 5:fprintf(GSM,"Alerte : Arret de Vehicule avec sms:");
                   break;
            case 6:fprintf(GSM,"Alerte : Le Solde de Votre Puce d'Alarme est en dessous du Solde Minimum :");
                   break;
            default: fprintf(GSM,"Pas D'alerte.");
                  
         #endif
       }
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
{        int n0;
         if(arret_flag) 
         {
            RL_ARR=1;
            write_alarm_outputs(out);
         }
         for(n0=0;n0<2;n0++)
         {
            read_tel_n(n0,tel_n);
            //start:
            #ifdef DEBUG0
               fprintf(DEBUG,"BIP au N°=%s\r\n",tel_n);
            #endif
            if (bip(tel_n))
            {
               sms(tel_n);
               output_low(led);
               return 1;
            }
            #ifdef DEBUG0
            else   fprintf(DEBUG," le N°=%s est hors champ\r\n",tel_n);
            #endif
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
   char etat,m1;
   set_tris_b(0B11110011);
   set_tris_a(0B11001001);
   setup_comparator(NC_NC_NC_NC);
   enable_interrupts(INT_RDA);
   enable_interrupts(GLOBAL);
   RL_ARR=1;
   LED=1;
   write_alarm_outputs(out);
  /* delay_ms(1000);
   LED=0;
   write_alarm_outputs(out);
   delay_ms(1000);*/
   if(!input(USB_CON_SENSE_PIN))
   {
      
      SIM_PowerOn();
      LED=1;
      write_alarm_outputs(out);
      delay_ms(7000);
      LED=0;
      write_alarm_outputs(out);
      delay_ms(1000);
   }
  
   //fprintf(GSM,"AT+CSQ\r\n");
   //delay_ms(1000);
   //fprintf(GSM,"AT+COPS=?\r\n");
   //delay_ms(1000);
   //fprintf(GSM,"AT+COPS?\r\n");
   //delay_ms(5000);
   //fprintf(GSM,"AT+CMGL=0\r\n");
   //SIM_SendONOFF();
    etat=read_eeprom(255);
    rep_sold=read_eeprom(254);
    arret_flag=read_eeprom(253);
    sold_min=read_eeprom(252);
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
   #ifdef GPS
      get_gps_pos(x,y,z);
   #ifdef DEBUG0 
      fprintf(DEBUG,"x=%s ; y=%s ; z=%s\n\r",x,y,z);
   #endif
   #endif
//   delay_ms(1000);
   fprintf(GSM,"ATD*200#;\r\n");
   sold_ask=1;
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
               switch(RecCommand)
               {
                  case SAVE_N:// Sauvgarde des nemuros de telephones
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
                     break;
                  }
                  case SAVE_MSG:// Sauvgarde des messages
                  {
                     n=recbuf[2]-49;
                     //printf("%d %s\r\n",Lenbuf,recbuf);
                     n=n*32+40;
                     for(g=4;g<Lenbuf;g++) write_eeprom(n+(g-4),recbuf[g]);
                  }
                  case READ_MSG:// Licture des messages
                  {
                     //n=recbuf[2]-49;
                     //printf("%d %s\r\n",Lenbuf,recbuf);
                     //n=n*32+40;
                     //for(g=4;g<Lenbuf;g++) write_eeprom(n+(g-4),recbuf[g]);
                  }
                  case READ_N:// Licture des n tel
                  {
                     for(n=0;n<4;n++)
                     {
                        n0=n*10;
                        for(g=0;g<10;g++) 
                        {
                           tel_n2[n0+g+n]=read_eeprom(n0+g);
                        }
                        tel_n2[n0+g+n]=':';
                     }
                     usb_put_packet(1,n_tel,44,USB_DTS_TOGGLE);
                     break;
                  }
                  case SEND_RS232: //AT Commande ver Module
                  {
                     printf("%s\r\n",recbuf);
                     break;
                  }
                  case GET_RS232: //AT Commande ver Module
                  {
                     usb_put_packet(1,buffer,rs232_l,USB_DTS_TOGGLE);
                     break;
                  }
                  case GET_FIRMWARE:
                  {
                     usb_put_packet(1,Version,6,USB_DTS_TOGGLE);
                  }
                  case GET_MODULE:
                  {
                     usb_put_packet(1,Module,9,USB_DTS_TOGGLE);
                  }
                  case SAVE_FLAG:
                  {
                     if (recbuf[1]=='0') write_eeprom(255,0);
                     else write_eeprom(255,1);
                  }
                  case SOLD_FLAG:
                  {
                     if (recbuf[1]=='0') write_eeprom(254,0);
                     else write_eeprom(254,1);
                  }
                  case MA_FLAG:
                  {
                     if (recbuf[1]=='0') write_eeprom(253,0);
                     else write_eeprom(253,1);
                     break;
                  }
                  case SAVE_SN:
                  {
                     for(i0=0;i0<5;i0++)
                     {
                        write_eeprom(sold_add+i0,recbuf[i0+1]);
                     }
                  }
                  case SAVE_SM:
                  {
                     write_eeprom(252,recbuf[1]);
                  }
                  case SAVE_PWD:
                  {
                     nl0=recbuf[1]-48;
                     write_eeprom(pass_add,nl0);
                     for(i0=0;i0<nl0;i0++)
                     {
                        write_eeprom(pass_add+1+i0,recbuf[i0+2]);
                     }
                  }
                  
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
               //#ifdef DB fprintf(DEBUG,"Al0=%d\r\n",al0);
               if((msg_ok)&&(ARM)&&(ALARM)) 
               {
                 // #ifdef DB fprintf(DEBUG,"Alarme message \r\n");
                 SRN=1;
                 LMR=1;
                 write_alarm_outputs(out);
                 tx();
                 msg_ok=0;
               }
               //else #ifdef DB fprintf(DEBUG,"Alarme message non autorise\r\n");
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
                  //#ifdef DB fprintf(DEBUG,"count0=%ld\n\r",count0);
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
      if(sold)
      {
         if(sms_sold_ask)
         {
            #ifdef DEBUG0
            fprintf(DEBUG,"New Sold\n\r");
            #endif
            fprintf(GSM,"AT+CMGS=\"");
            for(m1=0;m1<10;m1++) fputc(tel_n1[m1],GSM);
            fprintf(GSM,"\"\r");
            if(wait_(1000)==1)
            {
               start1:
               for(m1=12;m1<sold_msg_l;m1++) fputc(buffer[m1],GSM);
               fputc(13,GSM);
               fputc(10,GSM);
               #ifdef DEBUG0
               fprintf(DEBUG,"wait ok\n\r");
               #endif
            }
            else
            {
               #ifdef DEBUG0
               fprintf(DEBUG,"wait no\n\r");
               #endif
            }
            delay_ms(2000);
            fputc(0x1a,GSM);
            sms_sold_ask=0;
         }
         else
         {
            sold_val=get_sold_val();
            if(sold_val<sold_min)
            {
               //envoi du message d'alerte du min du solde
               if((c_sold)||(rep_sold))
               {
                  al0=6;
                  read_tel_n(0,tel_n2);
                  sms(tel_n2);
                  c_sold=0;
               }
            }
            else c_sold=1;
         }
         sold_ask=0;
         sold=0;
      }
      if((sms_index==1)&&(sms_ok))
      {
         index_ok=0;
         LED=1;
         write_alarm_outputs(out);
         strcpy(index_val0,index_val);
         fprintf(GSM,"AT+CMGR=1\r\n");
         #ifdef DEBUG0
         fprintf(DEBUG,"New Sms\n\r");
         #endif
         delay_ms(1000);
         sms_ok=0;
         sms_ok2=0;
        
      }
      if((sms_index==2)&&(sms_ok)) 
      {
         etape2:
         //fprintf(DEBUG,"index= %d\n\r",sms_index);
         get_sms_n(tel_n1);
         for(i2=0;i2<4;i2++)
         {
            read_tel_n(i2,tel_n2);
            if(strncmp(tel_n2,tel_n1,10)==0) 
            {
               n_ok=1;
               break;
            }
         }
         #ifdef DEBUG0
           fprintf(DEBUG,"SMS Tel N=%s\n\r",tel_n1);
          //fprintf(DEBUG,"SIM Tel N=%s\n\r",tel_n2);
         #endif 
          sms_ok=0;
          sms_ok2=1;
      }
      
      if((sms_index==3)&&(sms_buffer0[1]=='*'))
      {
         if(sms_ok2==0) goto etape2;
         #ifdef DEBUG0
        // fprintf(DEBUG,"index= %d\n\r",sms_index);   
         fprintf(DEBUG,"SMS msg=%s\n\r",sms_buffer0);
         #endif
         if(n_ok)
         {           
            get_pass_word(pass_word);
            pass_ok=pass_word_ok();
            #ifdef DEBUG0
            if(pass_ok) fprintf(DEBUG,"Good pass word\r\n");
            #endif
            if(pass_ok)
            {
               switch(sms_buffer0[0])
               {
                  case 'A':
                  case 'a':
                  {
                     speed_limit=read_eeprom(speed_add);
                     if(speed_limit<100)
                     {
                        #ifdef GPS
                           get_gps_speed(speed1,direction0);
                        #endif
                        if(speed<speed_limit)
                        {
                           RL_ARR=0;
                           write_alarm_outputs(out);
                           al0=5;
                           sms(tel_n1);
                        }
                        else 
                        {
                           halt_flag=1;
                        }
                        #ifdef DEBUG0
                        fprintf(DEBUG,"Arret de moteur avec limitation de vitesse\r\n");
                        #endif
                  
                     }
                     else 
                     {
                        #ifdef DEBUG0
                         fprintf(DEBUG,"Arret de moteur sans limitation de vitesse\r\n");
                        #endif 
                         RL_ARR=0;
                         write_alarm_outputs(out);
                    }
                        break;
                  }
                  case 'C':
                  case 'c':
                  {
                     #ifdef DEBUG0
                     fprintf(DEBUG,"Changer le motde pass\r\n");
                     #endif
                     save_pass_word();
                     break;
                  }
                  case 'M':
                  case 'm':
                  {
                     #ifdef   DEBUG0
                     fprintf(DEBUG,"Marche de moteur\r\n");
                     #endif
                     RL_ARR=1;
                     write_alarm_outputs(out);
                 
                     break;
                  }
                  case 'S':
                  case 's':
                  {
                     #ifdef DEBUG0
                     fprintf(DEBUG,"Sold de puce\r\n");
                     #endif
                     read_sold_msg(sold_msg);
                     fprintf(GSM,"ATD%s;\r\n",sold_msg);
                     sold_ask=1;
                     sms_sold_ask=1;
                     break;
                  }
               }
            }
         }
            
         fprintf(GSM,"AT+CMGD=1\r\n");
         LED=0;
         write_alarm_outputs(out);
         sms_index=0;
         sms_ok=0;
         n_ok=0;
            
      }
      //////////////////////////////////////////////////////////////////////////////
      if(halt_flag)
      {
         #ifdef GPS
         get_gps_speed(speed1,direction0);
         
         if(speed<speed_limit)
         {
             RL_ARR=0;
             write_alarm_outputs(out);
             al0=5;
             for(i2=0;i2<4;i2++)
            {
               read_tel_n(i2,tel_n2);
               sms(tel_n2);
               
            }
            halt_flag=0;
         }
         #endif
      }
      //////////////////////////////////////////////////////////////////////////////
      if(input(SIM_RING))
         {
            LED=1;
            write_alarm_outputs(out);
            for(i2=0;i2<4;i2++)
            {
               read_tel_n(i2,tel_n2);
               get_ring_n(tel_n1);
               if(strncmp(tel_n2,tel_n1,10)==0) 
               {
                 sms(tel_n1);
                 break;
               }
            }
            LED=0;
            write_alarm_outputs(out);
            
         }
         //------------------------------------------------------------------
      }
      
      time_count++;
      if(time_count>225000000)
      {
         time_count=0;
         #ifdef DEBUG0
         fprintf(DEBUG,"Ask sold\r\n");
         #endif
         read_sold_msg(sold_msg);
         fprintf(GSM,"ATD%s;\r\n",sold_msg);
         sold_ask=1;
      }
   }
}
