#include <string.h>
#ifdef GPS
   #define SIM_PKEY     PIN_A1
#else
   #define SIM_PKEY     PIN_A2
#endif
#define POWER        PIN_C2
#ifdef GPS
   #define SIM_STATUS   PIN_A0
#else
   #define SIM_STATUS   PIN_A1
#endif
#ifdef GPS
   #define SIM_RING     PIN_B1   // Inp
#else
   #define SIM_RING     PIN_A4   // Inp
#endif
#define SIM_CTS      PIN_A1   // Inp
#define SIM_RES      PIN_A2   // Out
#define SIM_ONOFF    PIN_A3   // Out
#define SIM_RTS      PIN_A4   // Out
#define SIM_DTR      PIN_A5   // Out
#define SIM_DCD      PIN_A6   // Inp
#define SIM_DSR      PIN_A7   // Inp
#define SIM_PWRCTL   PIN_B5   // Inp
#define GPS_RX       PIN_A3
#define GPS_TX       PIN_A2
#define GPS_MRST     PIN_A4
#define GSM_RX       PIN_C7
#define GSM_TX       PIN_C6
#define SIM_RX_BUFFER_SIZE 128
#USE RS232(BAUD=9600, XMIT=GSM_TX, RCV=GSM_RX,stream=GSM)
#ifdef GPS
#USE RS232(BAUD=4800, XMIT=GPS_TX, RCV=GPS_RX,stream=GPS)
#endif
BYTE SIM_rx_buffer[SIM_RX_BUFFER_SIZE];
BYTE SIM_next_in = 0;
BYTE SIM_next_out = 0;
BYTE SerialBuffer_pos=0;
BYTE flag,flag1;
char msg_tx=0,sms_index=0,sms_buff_l,nl0;
#define SerialBuffer SIM_rx_buffer 

#rom int8 0xf00000={'0','0','0','0','0','0','0','0','0','0'} // n�1
#rom int8 0xf0000A={'0','0','0','0','0','0','0','0','0','0'} // n�2
#rom int8 0xf00014={'0','0','0','0','0','0','0','0','0','0'} // n�3
#rom int8 0xf0001e={'0','0','0','0','0','0','0','0','0','0'} // n�4
#rom int8 0xf00028={4,'0','0','0','0'} 
#rom int8 0xf00032={'*','0','0','0','#'} 
#bit receive_flag=flag.1
#bit end_receive=flag.2
#bit OK=flag.3
#bit sms_ok=flag.4
#bit n_ok=flag.5
#bit halt_flag=flag.6
#bit sold=flag.7
#bit sold_ask=flag1.0
#bit suit_flag=flag1.1
#bit index_ok=flag1.2
#bit sms_ok2=flag1.3
#bit pass_ok=flag1.4
#bit sms_sold_ask=flag1.5
#bit start_ok=flag1.6
#bit c_sold=flag1.7
byte speed_limit=0;
byte speed_add=50;
//byte sms_ok=0;
int32 timer1Tick;
char head[6],temp[100],vergule_pos[20],vergule_n,txt_test[5],pass_word[16];
char w = 0,no_ok,speed,sold_msg[5],rep_sold=0;
int j,i0 = 0;
char buffer[64],sms_buffer0[32],sms_buffer[64];
char index_val[4],index_val0[4],index_l=0;
char token[20]; 
int16 t0=0,sold_val,sold_min=100;
int32 time_count=0;
int8 tel_n[10],tel_n1[10],pos,sold_msg_l;
const char pass_add=40,sold_add=50;
char speed1[5],direction0[5];
//***************************************************************
//    test si il ya de caractere envoyer par SIM508
//    return: 
//    0 si il n'ya pas de caractere
//    1 si il ya un caractere
//***************************************************************

#define SIM_GSM_KbHit (SIM_next_in!=SIM_next_out)


//***************************************************************
//    Attendre l'arriv� d'un caractere de modem SIM508
//
//    Return: 
//      le code ascii du caractere recevie
//***************************************************************

BYTE SIM_GSM_Getc() 
{
  BYTE c;

  while(!SIM_GSM_KbHit) ;
  c=SIM_rx_buffer[SIM_next_out];
  SIM_next_out=(SIM_next_out+1) % SIM_RX_BUFFER_SIZE;
  return(c);
}

//***************************************************************
//    envoi d'un caractere au modem SIM508
//***************************************************************

void SIM_GSM_Putc(BYTE c) 
{ 
   #USE RS232(BAUD=9600, XMIT=GSM_TX, RCV=GSM_RX)
   putc(c); 
}

//***************************************************************
// chargement de buffer de reception de modem
//***************************************************************

void SIM_FlushRxBuffer(void) 
{
  while(SIM_GSM_KbHit) SIM_GSM_getc();
}

//***************************************************************
//       gestion des interruptions
//***************************************************************

//***************************************************************
// interruption de fin de reception d'un caracter au USART
//***************************************************************
/*#int_rda 
void serial_isr() 
{
   int t;
   
   SIM_rx_buffer[SIM_next_in]=fgetc(COM_GSM);
   t=SIM_next_in;
   SIM_next_in=(SIM_next_in+1) % SIM_RX_BUFFER_SIZE;
   if(SIM_next_in==SIM_next_out)
   SIM_next_in=t;           // Buffer pieno !!
}
*/
//***************************************************************
// Timer 1
//***************************************************************

/*#int_timer1
void wave_timer() 
{
   set_timer1(63036);
   timer1Tick--;
}
*/

//***************************************************************
// Start Timer
//***************************************************************

void StartTimer(int32 ms) 
{
   set_timer1(63036);
   timer1Tick=ms;
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(int_timer1);
}   

//***************************************************************
// Stop Timer
//***************************************************************

void StopTimer(void) 
{
  disable_interrupts(int_timer1);
}   

//***************************************************************
// Test Timer
//***************************************************************

int CheckTimer(void) 
{
  if (timer1Tick<=0) 
  {
     StopTimer();
     return 1;
  } 
  else
  {
     return 0;
  }
}   

//***************************************************************
// Reception des chaines de caractere de reponse de modem
// 
// to timeout en  mS
// buffer Buffer de memorisation de reponse (256 car)
//
// Return 0 si il depasse timeout ou il depasse la capacit� max(256 car)
// Return 1 si il recepte la reponse OK<CR><LF>
//***************************************************************

int SIM_WaitResponse(int32 to) 
{
  int32 t1;
  receive_flag=0;
  t1=0;
  for (;;) 
  {
     if(end_receive)
     {
       if(OK) return(1);
       end_receive=0;
     }
     else
     {
         delay_ms(1);
         t1++;
         if(t1>to) return(0);
     }
    } 
} 

//***************************************************************
// Alimentation de Modem SIM508
//***************************************************************
/*
void SIM_SendONOFF(void) 
{
   output_high(POWER);
   delay_ms(2000);
   output_low(POWER);
   output_float(SIM_PKEY);
   delay_ms(1000);
   output_low(SIM_PKEY);
   delay_ms(5000);
   output_float(SIM_PKEY);
   delay_ms(1000);
}
*/
void SIM_SendONOFF(void) 
{
   PWR=1;
   write_alarm_outputs(out);
   //fprintf(DEBUG,"PWR=1\n\r");
   delay_ms(2000);
   PWR=0;
   //fprintf(DEBUG,"PWR=0\n\r");
   write_alarm_outputs(out);
   output_float(SIM_PKEY);
   //fprintf(DEBUG,"PKEY=1\n\r");
   delay_ms(1000);
   output_low(SIM_PKEY);
   //fprintf(DEBUG,"PKEY=0\n\r");
   delay_ms(5000);
   output_float(SIM_PKEY);
   //fprintf(DEBUG,"PKEY=1\n\r");
   delay_ms(1000);
}
//***************************************************************
// Alimentation et initialisation de modem par AT commande
//***************************************************************

int SIM_PowerOn(void) 
{
  
  #USE RS232(BAUD=9600, XMIT=GSM_TX, RCV=GSM_RX) 
    
  
   for (;;) 
   {
     // if (input(SIM_STATUS)==0)
      //{
         SIM_sendONOFF();
//      } 
      fprintf(GSM,"AT\r\n");
      delay_ms(1000);
      fprintf(GSM,"AT\r\n");
      delay_ms(1000);
      if(OK) 
      {
        // fprintf(DEBUG,"AT OK\r\n");
             
        fprintf(GSM,"AT+IPR=9600\r\n");
        delay_ms(200);  
        fprintf(GSM,"AT+CPBS=\"SM\"\n\r");
        delay_ms(200);
        fprintf(GSM,"AT+CMGF=1\r\n");
        delay_ms(200);
        fprintf(GSM,"AT+CMGD=1\r\n");
        delay_ms(200);
        fprintf(GSM,"AT+CLIP=1\r\n");
        delay_ms(200);
        fprintf(GSM,"AT+CNMI=3,1\r\n");
        delay_ms(200);
        fprintf(GSM,"ATE0\r\n");
        delay_ms(200); 
        
        return(1);
     }
     //else
     //{
       delay_ms(2000);
     //  SIM_SendONOFF();
     //}  
     
  }   
}

//***************************************************************
// Test si le modem est connect� aux resaux GSM
//
// Return 0 si le modem n'est pas registr�e
// return 1 si le modem est registr�e
//***************************************************************

int SIM_NetworkChecking() 
{
   

   fprintf(GSM,"AT+CREG?\r");
      
   if (SIM_WaitResponse(500)>0) 
   {
      //fprintf(GPS,"s=%s\n\r",temp);
      sprintf(token,"+CREG: 0,1");
      if (strstr(temp,token)==0) return 1;
      sprintf(token,"+CREG: 1,1");
      if (strstr(temp,token)==0) return 1;
   }   
   else 
   {
      //fprintf(GPS,"Timeout\n\r");
      return 0;
   }
}
//*************************************************************************************************
int read_tel_n(char n,char *tel_n3)
{
   n=n*10;
   for(i0=0;i0<10;i0++)
   {
      tel_n3[i0]=read_eeprom(n+i0);
   }
   tel_n3[i0]=0;
   return 1;
}
//*************************************************************************************************
int make_msg(int n,char *msg)
{  
   
   n=n*32;
   for(i0=0;i0<32;i0++)
   {
      msg[i0]=read_eeprom(n+i0+40);
      if(msg[i0]==0) break;
   }
  /* suit:
   msg[++i0]=' ';
   pos=0;
   for(i0=0;i0<15;i0++) msg[pos+i0]=read_eeprom(168+i0);;
   pos=pos+15;
   for(i0=0;i0<15;i0++) msg[pos+i0]=read_eeprom(183+i0);
   pos=pos+15;
   for(i0=0;i0<8;i0++) msg[pos+i0]=read_eeprom(198+i0);
   pos=pos+8;
   for(i0=0;i0<6;i0++) msg[pos+i0]=read_eeprom(206+i0);
   pos=pos+6;
   for(i0=0;i0<6;i0++) msg[pos+i0]=read_eeprom(212+i0);
   pos=pos+6;
   for(i0=0;i0<6;i0++) msg[pos+i0]=read_eeprom(218+i0);
   pos=pos+6;
   for(i0=0;i0<6;i0++) msg[pos+i0]=read_eeprom(224+i0);*/
   /*
   for(i0=0;i0<15;i0++) msg[pos+i0]='x';
   pos=pos+15;
   for(i0=0;i0<15;i0++) msg[pos+i0]='y';
   pos=pos+15;
   for(i0=0;i0<8;i0++) msg[pos+i0]='z';
   pos=pos+8;
   for(i0=0;i0<6;i0++) msg[pos+i0]='t';
   pos=pos+6;
   for(i0=0;i0<6;i0++) msg[pos+i0]='d';
   pos=pos+6;
   for(i0=0;i0<6;i0++) msg[pos+i0]='c';
   pos=pos+6;
   for(i0=0;i0<6;i0++) msg[pos+i0]='s';*/
   
   return 1;
}

int wait_rep(char test,int16 time )
{
  
   t0=0;
   depart:
   if(SerialBuffer_pos>0)
   {
      if((SerialBuffer[0]==test)&&(t0<time)) return(1);
   }
   else 
   {  
      if(t0==time) return (0);
      t0++;
      delay_ms(1);
      goto depart;
   } 
   //if(kbhit()) 
   //{  c=getc();
   //printf("char=%c:%d\n\r",c,c);
   //if(t0<time) return 1;
   //else return 0;
      //if((c!=test)&&(t0<(time-1))) goto depart;
   //}
   //else return 0;
}
//*************************************************************************************************
int wait_(int16 time )
{
  
   
   t0=0;
   depart:
   if(msg_tx==1) 
   {
      return(1);
   }
   else 
   {  
      if(t0==time) return (0);
      t0++;
      delay_ms(1);
      goto depart;
   } 
   //if(kbhit()) 
   //{  c=getc();
   //printf("char=%c:%d\n\r",c,c);
   //if(t0<time) return 1;
   //else return 0;
      //if((c!=test)&&(t0<(time-1))) goto depart;
   //}
   //else return 0;
}
//*************************************************************************************************
int   send_sms(char *tel_n,char *msg,int mode)
{
   unsigned char tmp1 = 0,tmp2 = 1,tmp3 = 0,tmp4 = 0,NewMsg[49];
   unsigned char NewNumber[16],shift = 0,rep;
   
   if(mode==0)  //Message to PDU Format
   {
      while (Msg[tmp1] != 0)
      {
         NewMsg[tmp2] = (Msg[tmp1]&0x7F)>>shift;
         if (shift != 0)
         {
            NewMsg[tmp2-1] |= (Msg[tmp1]&((1<<shift)-1))<<(8-shift);
         }
         shift++;
         tmp1++;
         if (shift == 8)
         {
            shift=0;
         }
         else
         {
            tmp2++;
         }
      }
      NewMsg[0]= tmp1;
   
      //Number to PDU Format
      while (tel_n[tmp3] !=0)
      {
         tmp4 = tel_n[tmp3+1];
         NewNumber[tmp3+1] = tel_n[tmp3];
         if (tmp4 == 0)
         {
            NewNumber[tmp3] = 'F';
            tmp3=tmp3 + 2;
            NewNumber[tmp3] = 0x0;
            break;
         }
         NewNumber[tmp3] = tmp4;
         tmp3=tmp3 + 2;
         NewNumber[tmp3] = 0x0;
      }
   
      //Send SMS 
      printf("AT+CMGS=%i\r\n",(tmp2+(tmp3/2)+0x07));
      while(getc()!=' ');
      //Adresse SMSC (optional)
      printf("0791947101670000");
      //Header1
      printf("113C");
      //Distination Number
      if(NewNumber[tmp3-2] =='F')
      {
         tmp3--;
      }
      printf("%02X91",tmp3);
      printf(NewNumber);
      //Header2
      printf("0000A8");
      //SendMessage String
      for (tmp1 = 0;tmp1<tmp2;tmp1++)
      {
         printf("%02X", NewMsg[tmp1]);
      }
      printf("%c",0x1a);
      gets(NewMsg);
      gets(NewMsg);
      if(NewMsg[0]=='+')
      {
         return (0);
      };
      return (1);
   }
   else if(mode==1)
   {
      fprintf(GSM,"AT+CMGF=1\r\n");
      if (SIM_WaitResponse(20000)) 
     {
         printf("AT+CMGS=\"%s\"\r\n",tel_n);
         rep=wait_rep('>',100);
         if(rep==0) return (0);
         printf("%s : ",msg);
         rep=wait_rep('>',100);
         if(rep==0) return (0);
         printf("%c : ",0x1a);
     }
   }
   else return (0);
   
}
//*************************************************************************************************
int bip(char *tel_n)
{
   int16  time;
   //buffer[1]='s';
   //fprintf(GSM,"ATE1\r\n");
   delay_ms(1000);
   fprintf(GSM,"ATD%s;\r\n",tel_n);
   for(time=0;time<30000;time++)
   {
      if(no_ok)
      {
         no_ok=0;
         //fprintf(DEBUG,"rep=%s\r\n",buffer);
         //sprintf(token,"NO CARRIER");
         //fprintf(DEBUG,"buff=%c",buffer[1]);
         if (buffer[1]=='B') 
         {
            //fprintf(DEBUG,"UNREACHABLE\r\n");
            //fprintf(GSM,"ATE0\r\n");
            delay_ms(1000);
            return 1;
         }
         return 0;
      }
      delay_ms(1);
      
   }
   //fprintf(GSM,"ATH\r\n");
   /*if (SIM_WaitResponse(50000)>0) 
   //{
      
      sprintf(token,"NO CARRIER");
      if (strstr(temp,token)==0) return -1;
      else return 1;
  /* }   
   else 
   {
      return 0;
   }*/
}

//*************************************************************************************************
//*************************************************************************************************
void init_GPS()
{
   
   output_high(GPS_MRST);
   delay_ms(100);
   output_low(GPS_MRST);
   delay_ms(100);
   output_high(GPS_MRST);
  // #USE RS232(BAUD=4800, XMIT=GPS_TX, RCV=GPS_RX)
  // for(p=0;p<6;p++) printf("PSRF103,0%d,01,00,00*25\r\n",p);
   
}
//------------------------------------------------------------------------------------------------
int get_gps_speed(char *s0,char *c0)
{
  // #USE RS232(BAUD=4800, XMIT=GPS_TX, RCV=GPS_RX)
   char ok=0;
   #ifdef DEBUG0
      fprintf(DEBUG,"get speed\r\n");
   #endif
   fprintf(GPS,"Get speed\r\n");
   do
   {
      i0=0;
      w='\0';
      vergule_n=0;
      fputc('-',GPS);
      do
      {
         fputc('|',GPS);
         w=fgetc(GPS);
         
        
      }while(w!='$');
      do
      {
         w=fgetc(GPS);
         if(w==',')
         {
            vergule_pos[vergule_n]=i0;
            vergule_n++;
         }
         if(i0<5) head[i0]=w;
         temp[i0]=w;
         i0++;
      }
      while(w!=13);
      #ifdef DEBUG0
      fprintf(DEBUG,"head=%s\r\n",head);
      #endif
      w=fgetc(GPS);
      strcpy(txt_test,"GPVTG");
      if((strncmp(head,txt_test,5))==0) 
      {   
         for(j=0;j<(vergule_pos[1]-vergule_pos[0])-1;j++) c0[j]=temp[j+vergule_pos[0]+1];
         for(j=0;j<(vergule_pos[7]-vergule_pos[6])-1;j++) s0[j]=temp[j+vergule_pos[6]+1];
        
         ok=1;
         return i0;
      }
   }while(ok==0);
}
//------------------------------------------------------------------------------------------------
int get_gps_time(char *d0,char *t1)
{
   //#USE RS232(BAUD=4800, XMIT=GPS_TX, RCV=GPS_RX)
   char ok=0;
   fprintf(GPS,"Get time\r\n");
   do
   {
      i0=0;
      w='\0';
      vergule_n=0;
      do
      {
         w=fgetc(GPS);
      }while(w!='$');
      do
      {
         w=fgetc(GPS);
         if(w==',')
         {
            vergule_pos[vergule_n]=i0;
            vergule_n++;
         }
         if(i0<5) head[i0]=w;
         temp[i0]=w;
         i0++;
      }
      while(w!=13);
      w=fgetc(GPS);
      strcpy(txt_test,"GPRMC");
      if((strncmp(head,txt_test,5))==0) 
      {   
         for(j=0;j<(vergule_pos[1]-vergule_pos[0]);j++) t1[j]=temp[j+vergule_pos[0]+1];
         for(j=0;j<(vergule_pos[9]-vergule_pos[8]);j++) d0[j]=temp[j+vergule_pos[8]+1];
              
         ok=1;
         return i0;
      }
   }while(ok==0);   
}
//------------------------------------------------------------------------------------------------
int get_gps_pos(char *x0,char *y0,char *z0)
{
   //#USE RS232(BAUD=4800, XMIT=GPS_TX, RCV=GPS_RX)
   char ok;
   fprintf(GPS,"Get pos\r\n");
   do
   {
      ok=0;
      i0=0;
      w='\0';
      vergule_n=0;
      do
      {
         fputc('|',GPS);
         w=fgetc(GPS);
         //fputc(w,DEBUG);
      }while(w!='$');
      do
      {
         w=fgetc(GPS);
         //fputc(w,DEBUG);
         if(w==',')
         {
            vergule_pos[vergule_n]=i0;
            vergule_n++;
         }
         if(i0<5) head[i0]=w;
         temp[i0]=w;
         i0++;
      }
      while(w!=13);
      w=fgetc(GPS);
      //fprintf(DEBUG,"h=%s\n\r",head);
      strcpy(txt_test,"GPGGA");
      if((strncmp(head,txt_test,5))==0) 
      {   
         
        
      
         for(j=0;j<(vergule_pos[2]-vergule_pos[1]);j++) x0[j]=temp[j+vergule_pos[1]+1];
         x0[j]=temp[vergule_pos[2]+1];
         //for(g0=j;g0<15;g0++) 
         x0[j+1]='\0';
         for(j=0;j<(vergule_pos[4]-vergule_pos[3]);j++) y0[j]=temp[j+vergule_pos[3]+1];
         y0[j]=temp[vergule_pos[4]+1];
         y0[j+1]='\0';
         for(j=0;j<(vergule_pos[9]-vergule_pos[8]);j++) 
         {
            if(temp[j+vergule_pos[8]+1]==',') break;
            z0[j]=temp[j+vergule_pos[8]+1];
            
         }
         z0[j+1]='\0';
         
         ok=1;
         return i0;
      };
   }while(ok==0);
}

void SIM2EEPROM(char n)
{
   char n01[10],i;
   SerialBuffer[0]='\0';
   fprintf(GSM,"AT+CPBR=%d\r\n",n);
   delay_ms(100);
   if( SIM_WaitResponse(10000))
   {
      //fprintf(GPS,"\rtemp1=%s\r\n",buffer);
      n01[0]='0';
      //write_eeprom(10*(n-1),'0');
      for(i=12;i<21;i++) 
      {
         //write_eeprom(10*(n-1)+(i-13),temp01[i]);
         n01[i-11]=buffer[i];
      }
      for(i=0;i<10;i++) 
      {
         write_eeprom(10*(n-1)+i,n01[i]);
      }
      n01[i-11]='\0';
      fprintf(GPS,"n_tel %d=%s\r\n",n,n01);
   }
   else
   {
      fprintf(GPS,"SIM2EEPROM erreur");
   }
}
/*
int WritePhoneBook(char index,*n,*nom)
{
    SerialBuffer[0]='\0';
   fprintf(GSM,"AT+CPBW=%d,\"%s\",161,\"%s\"\r\n",index,n,nom);
   delay_ms(100);
   if( SIM_WaitResponse(10000))
   {
      return(1);
   }
   else return(0);
   //"AT+CPBW=1",n,"161,",nom
}*/

int get_ring_n(char *tel_n0)
{
  int32 t1;
  char i1;
  receive_flag=0;
  t1=0;
  for (;;) 
  {
     if(end_receive)
     {
       if(buffer[1]=='+')
       {
         for(i1=0;i1<10;i1++) tel_n0[i1]=buffer[i1+9];
         tel_n0[i1]=0;
       }
       //fprintf(DEBUG,"s0= %s\n\r",buffer);
       end_receive=0;
     }
     else
     {
         delay_ms(1);
         t1++;
         if(t1>5000) return(0);
     }
    } 
}
//------------------------------------------------------------------------------
int get_sms_n(char *tel_n0)
{
  
  int32 t1;
  char i1,i0,n_start=60,n_end=60;
 // fprintf(DEBUG,"get_sms_n start\r\n") ;
  receive_flag=0;
  if(end_receive)
  {
     for(i0=0;i0<sms_buff_l;i0++)
     {
       if(sms_buffer[i0]=='+')
       {
          if((sms_buffer[i0+2]=='M') &&(sms_buffer[i0+3]=='G')&&(sms_buffer[i0+4]=='R'))
          {
             suit_flag=1;
             
          }
       }
       if(suit_flag)
       {
            //fprintf(DEBUG,"l=%d i0=%d\n\r",sms_buff_l,i0);
            if(sms_buffer[i0]==',')
            {
               n_start=i0+6;
               n_end=i0+14;
               //fprintf(DEBUG,"start=%d end=%d \n\r",n_start,n_end);
               
            }
            if(i0>n_start-1)
            {
                tel_n0[i0-n_start+1]=sms_buffer[i0]; 
            }
            if(i0>n_end)
            {
               tel_n0[0]='0';
               tel_n0[i0-n_start+1]='\0';
              // fprintf(DEBUG,"s0= %s\n\r",tel_n0); 
              // fprintf(DEBUG,"get_sms_n end:1\r\n");
               return 1;
            }
       }
     }
    // fprintf(DEBUG,"s0= %s\n\r",tel_n0);
     end_receive=0;
  }
  else
  {
      delay_ms(1);
      t1++;
      if(t1>5000);
  }
  // fprintf(DEBUG,"get_sms_n end :0\r\n");    
}

//------------------------------------------------------------------------------
int read_new_sms()
{
   fprintf(GSM,"AT+CMGR=1\r\n");
   get_ring_n(tel_n1);
   
   delay_ms(1000); 
   fprintf(GSM,"AT+CMGD=1\r\n");
   return 0;
   
}
//------------------------------------------------------------------------------
int get_pass_word(char *pass_tmp)
{
   int s0;
   for(s0=2;(sms_buffer0[s0]!='#')&&(s0<18);s0++)
   {
      pass_tmp[s0-2]=sms_buffer0[s0];
      
   }
   nl0=s0-2;
   return 0;
}
//------------------------------------------------------------------------------
void save_pass_word()
{
   int s0,nl,nl0=0;
   
   nl=read_eeprom(pass_add);
   for(s0=3+nl;(sms_buffer0[s0]!='#')&&(s0<32);s0++)
   {
      write_eeprom(pass_add+s0-2-nl,sms_buffer0[s0]);
      nl0++;
      
   }
   write_eeprom(pass_add,nl0);
}
//------------------------------------------------------------------------------
int pass_word_ok()
{
   char nl,s0,c0;
   nl=read_eeprom(pass_add);
   if(nl0!=nl) return 0;
   for(s0=0;s0<nl;s0++)
   {
      c0=read_eeprom(pass_add+s0+1);
      if(pass_word[s0]!=c0)
      {
         //fprintf(DEBUG,"pw %d :c0=%c p=%c\r\n",s0,c0,pass_word[s0]);
         return(0);
         
      }
       
   }
   return(1);
   
   
}
//------------------------------------------------------------------------------
void read_sold_msg(char *msg)
{
   int i1;
   for(i1=0;i1<5;i1++)
   {
      msg[i1]=read_eeprom(sold_add+i1);
   }
}
//------------------------------------------------------------------------------
int16 get_sold_val()
{
   int i1,i2;
   start_ok=0;
   sold_val=0;
   for(i1=0;i1<sold_msg_l;i1++)
   {
      if(buffer[i1]==':') start_ok=1;
      if((buffer[i1]>47)&&(buffer[i1]<58)&&(start_ok))
      {
         #ifdef DEBUG0
         fprintf(DEBUG,"-%c-",buffer[i1]);
         #endif
         sold_val=sold_val*10+(buffer[i1]-48);
      }
      if(buffer[i1]=='.') break;
   }
   return(sold_val);
   #ifdef DEBUG0
   fprintf(DEBUG,"%s\r\nSold= %ld DA\r\n",buffer,sold_val);
   #endif
}
