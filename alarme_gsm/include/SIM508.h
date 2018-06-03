#include <string.h>
#include <input.c>
#ifdef GPS0
   #ifdef ALARME
   #define SIM_PKEY     PIN_A1
   #else
   #define SIM_PKEY     PIN_C1
   #endif
#else
   #ifdef   ALARME
   #define SIM_PKEY     PIN_A2
   #else
   #define SIM_PKEY     PIN_A3
   #endif
#endif
#define POWER        PIN_C2

#ifdef GPS0
   #define SIM_STATUS   PIN_A0
#else
   #ifdef   ALARME 
   #define SIM_STATUS   PIN_A1
   #else
   #define SIM_STATUS   PIN_C0
   #endif
#endif

#ifdef GPS0
   #ifdef   ALARME
   #define SIM_RING     PIN_B1   // Inp
   #else
   #define SIM_RING     PIN_C0   // Inp
   #endif
   #define GPS_MRST     PIN_A4
   #define GPS_RX       PIN_A3
   #define GPS_TX       PIN_A2
   #USE RS232(BAUD=4800, XMIT=GPS_TX, RCV=GPS_RX,stream=GPS)
#else
   #define SIM_RING     PIN_A4   // Inp
#endif

#define GSM_RX       PIN_C7
#define GSM_TX       PIN_C6
#define SIM_RX_BUFFER_SIZE 128

#USE RS232(BAUD=9600, XMIT=GSM_TX, RCV=GSM_RX,stream=GSM)
#ifdef GPS0
#USE RS232(BAUD=4800,XMIT=GPS_TX,RCV=GPS_RX,stream=GPS_COM)
#endif

BYTE SIM_rx_buffer[SIM_RX_BUFFER_SIZE];
BYTE SIM_next_in = 0;
BYTE SIM_next_out = 0;
BYTE SerialBuffer_pos=0;
BYTE flag,flag1;
char x[15],y[15],x0[15],y0[15];
char msg_tx=0,sms_index=0,sms_buff_l,nl0,ring_count=0;
#define SerialBuffer SIM_rx_buffer 

#rom int8 0xf00000={'0','0','0','0','0','0','0','0','0','0'} // n°1
#rom int8 0xf0000A={'0','0','0','0','0','0','0','0','0','0'} // n°2
#rom int8 0xf00014={'0','0','0','0','0','0','0','0','0','0'} // n°3
#rom int8 0xf0001e={'0','0','0','0','0','0','0','0','0','0'} // n°4
#rom int8 0xf00028={4,'0','0','0','0'} 
#rom int8 0xf00032={5,'*','0','0','0','#'} 
#bit RING_FLG=flag.0
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
byte GSMStat;
#bit GSMBUSY=GSMStat.0
#bit GSMOK=GSMStat.1
#bit GSMERR=GSMStat.2
#bit GSMNOC=GSMStat.3
byte speed_limit=40;
//byte speed_add=50;
//byte sms_ok=0;
int32 timer1Tick;
char head[7],temp0[64],vergule_pos[20],vergule_n,txt_test[5],pass_word[16];
char temp[100];
char w = 0,no_ok,speed,sold_msg[5],rep_sold=0;
int j,i0 = 0;
char buffer[64],sms_buffer0[32],sms_buffer[64];
char index_val[4],index_val0[4],index_l=0;
char token[20]; 
int16 t0=0,sold_val,sold_min=100;
int32 time_count=0;
int8 tel_n[11],tel_n1[11],pos,sold_msg_l;
const char pass_add=40,sold_add=50,speed_add=70;
char speed1[5],direction0[5],date[7],time[7];
//***************************************************************
//    test si il ya de caractere envoyer par SIM508
//    return: 
//    0 si il n'ya pas de caractere
//    1 si il ya un caractere
//***************************************************************

#define SIM_GSM_KbHit (SIM_next_in!=SIM_next_out)


//***************************************************************
//    Attendre l'arrivé d'un caractere de modem SIM508
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
// Return 0 si il depasse timeout ou il depasse la capacité max(256 car)
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
   #ifdef ALARME
   PWR=1;
   write_alarm_outputs(out);
   #else 
   output_high(PWR);
   #endif
   //fprintf(DEBUG,"PWR=1\n\r");
   delay_ms(1000);
   #ifdef ALARME
   PWR=0;
   write_alarm_outputs(out);
   #else
      output_low(PWR);
   #endif
   delay_ms(1000);
   output_float(SIM_PKEY);
   //fprintf(DEBUG,"PKEY=1\n\r");
   delay_ms(1000);
   output_low(SIM_PKEY);
   //fprintf(DEBUG,"PKEY=0\n\r");
   delay_ms(2000);
   output_float(SIM_PKEY);
   //fprintf(DEBUG,"PKEY=1\n\r");
   delay_ms(5000);
}
//***************************************************************
// Alimentation et initialisation de modem par AT commande
//***************************************************************

int SIM_PowerOn(void) 
{
   int g1,g2; 
   OK=0;
   for (g1=0;g1<5;g1++) 
   {
      SIM_sendONOFF();
       for (g2=0;g2<5;g2++) 
      {
         fprintf(GSM,"AT\r\n");
         #ifdef DEBUG0
         fprintf(DEBUG,"AT PIC->SIM\r\n");
         #endif
         delay_ms(1000);
         if(OK) 
         {
            #ifdef DEBUG0
            fprintf(DEBUG,"AT OK\r\n");
            #endif 
            fprintf(GSM,"AT+IPR=9600\r\n");
            delay_ms(200);  
            fprintf(GSM,"AT+CPBS=\"SM\"\n\r");
            delay_ms(200);
            fprintf(GSM,"AT+CMGF=1\r\n");
            delay_ms(200);
            fprintf(GSM,"AT+CMGD=1\r\n");
            delay_ms(200);
            fprintf(GSM,"AT+CNMI=3,1\r\n");
            delay_ms(200);
            fprintf(GSM,"ATE0\r\n");
            delay_ms(200);
            fprintf(GSM,"AT+CLIP=1\r\n");
            delay_ms(200);
            return(1);
         }
      }
   } 
   return(0);
}

//***************************************************************
// Test si le modem est connecté aux resaux GSM
//
// Return 0 si le modem n'est pas registrée
// return 1 si le modem est registrée
//***************************************************************
/*
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
*/
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
   int rep[10];
   //buffer[1]='s';
   //fprintf(GSM,"ATE1\r\n");
   //delay_ms(1000);
   no_ok=0;
   fprintf(GSM,"ATD%s;\r\n",tel_n);
   //get_string(rep,2);
   //fprintf(DEBUG,"rep=%s\r\n",rep);
   //get_string(rep,2);
   //fprintf(DEBUG,"rep=%s\r\n",rep);
   for(time=0;time<30000;time++)
   {
      if(no_ok)
      {
         fprintf(DEBUG,"No ok\r\n");
         no_ok=0;
         strcpy(buffer,SerialBuffer);
         fprintf(DEBUG,"buffer=%c%c%c%c\r\n",buffer[0],buffer[1],buffer[2],buffer[3]);
         if ((buffer[0]=='B')||(buffer[1]=='B')) 
         {
            //fprintf(GSM,"ATE0\r\n");
            //delay_ms(1000);
            fprintf(DEBUG,"Busy status\r\n");
            return 1;
         }
         else 
         {
            fprintf(DEBUG,"No carr status %c%c%c%c\r\n",buffer[0],buffer[1],buffer[2],buffer[3]);
            return 0;
         }
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
   
   //output_high(GPS_MRST);
   delay_ms(100);
   output_low(GPS_MRST);
   delay_ms(100);
   output_high(GPS_MRST);
  // #USE RS232(BAUD=4800, XMIT=GPS_TX, RCV=GPS_RX)
   //for(p=0;p<6;p++) printf("PSRF103,0%d,01,00,00*25\r\n",p);
   
}

//------------------------------------------------------------------------------------------------
#ifdef GPS0
int get_gps_speed(char *s0,char *c0)
{
  //#USE RS232(BAUD=4800, XMIT=GPS_TX, RCV=GPS_RX)
   char ok=0,x1;
   do
   {
      i0=0;
      w='\0';
      vergule_n=0;
      do
      {         
         w=fgetc(GPS);
      }while(w>20);
      do
      {
         w=fgetc(GPS);
         if(w>20)
         {
            temp0[i0]=w;
            if(i0<6) head[i0]=w;
            else head[i0]='\0';
            i0++;
         }
      }
      while(w!=13);
      w=fgetc(GPS);
      for(x1=0;x1<i0;x1++) 
      {
         if(temp0[x1]==',')
         {
            vergule_pos[vergule_n]=x1;
            vergule_n++;
         }
      }
      
      if((head[3]=='V')&&(head[4]=='T')&&(head[5]=='G')) 
      {  #ifdef DEBUG0
            fprintf(DEBUG,"Get Speed\r\n");
            //fprintf(DEBUG,"msg=%s\r\n",temp0);
         #endif 
         for(j=0;j<(vergule_pos[1]-vergule_pos[0])-1;j++) c0[j]=temp0[j+vergule_pos[0]+1];
         c0[j]='\0';
         for(j=0;j<(vergule_pos[7]-vergule_pos[6])-1;j++) s0[j]=temp0[j+vergule_pos[6]+1];
         s0[j]='\0';
         ok=1;
         return i0;
      }
   }while(ok==0);
}
//------------------------------------------------------------------------------------------------
int get_gps_time(char *d0,char *t1)
{
   //#USE RS232(BAUD=4800, XMIT=GPS_TX, RCV=GPS_RX)
   char ok=0,x1;
   do
   {
      i0=0;
      w='\0';
      vergule_n=0;
      do
      {         
         w=fgetc(GPS);
      }while(w>20);
      do
      {
         w=fgetc(GPS);
         if(w>20)
         {
            temp0[i0]=w;
            if(i0<6) head[i0]=w;
            else head[i0]='\0';
            i0++;
         }
      }
      while(w!=13);
      w=fgetc(GPS);
      for(x1=0;x1<i0;x1++) 
      {
         if(temp0[x1]==',')
         {
            vergule_pos[vergule_n]=x1;
            vergule_n++;
         }
      }
      #ifdef DEBUG0
        // fprintf(DEBUG,"Get Time\r\n");
        // fprintf(DEBUG,"h=%c%c%c\r\n",head[3],head[4],head[5]);
      #endif
      if((head[3]=='R')&&(head[4]=='M')&&(head[5]=='C')) 
      {  
         #ifdef DEBUG0
            fprintf(DEBUG,"Get Time\r\n");
            fprintf(DEBUG,"msg=%s\r\n",temp0);
         #endif 
         //for(j=0;j<(vergule_pos[1]-vergule_pos[0]);j++) t1[j]=temp0[j+vergule_pos[0]+1];
         for(j=0;j<6;j++) t1[j]=temp0[j+vergule_pos[0]+1];
         t1[j]='\0';
         for(j=0;j<6;j++) d0[j]=temp0[j+vergule_pos[8]+1];
         //for(j=0;j<(vergule_pos[9]-vergule_pos[8]);j++) d0[j]=temp0[j+vergule_pos[8]+1];
         
         d0[j]='\0',
         ok=1;
         return i0;
      }
   }while(ok==0);   
}
//------------------------------------------------------------------------------------------------
int get_gps_pos()
{
   //#USE RS232(BAUD=4800, XMIT=GPS_TX, RCV=GPS_RX)
   char ok,x1;
   do
   {
      ok=0;
      i0=0;
      w='\0';
      vergule_n=0;
      do
      {         
         w=fgetc(GPS);
      }while(w>20);
      do
      {
         w=fgetc(GPS);
         if(w>20)
         {
            temp0[i0]=w;
            if(i0<6) head[i0]=w;
            else head[i0]='\0';
            i0++;
         }
      }
      while(w!=13);
      w=fgetc(GPS);
      for(x1=0;x1<i0;x1++) 
      {
         if(temp0[x1]==',')
         {
            vergule_pos[vergule_n]=x1;
            vergule_n++;
         }
      }
      fprintf(DEBUG,"h=%s\n\r",head);
      //strcpy(txt_test,"$GPGGA");
      #ifdef DEBUG0
         fprintf(DEBUG,"Get Position\r\n");
         fprintf(DEBUG,"h=%c%c%c\r\n",head[3],head[4],head[5]);
      #endif
      if((head[3]=='G')&&(head[4]=='G')&&(head[5]=='A')) 
      {   
         #ifdef DEBUG0
            fprintf(DEBUG,"Get Position\r\n");
            fprintf(DEBUG,"msg=%s\r\n",temp0);
         #endif 
         for(j=0;j<(vergule_pos[2]-vergule_pos[1]);j++) x0[j]=temp0[j+vergule_pos[1]+1];
         x0[j]=temp0[vergule_pos[2]+1];
         //for(g0=j;g0<15;g0++) 
         x0[j+1]='\0';
         for(j=0;j<(vergule_pos[4]-vergule_pos[3]);j++) y0[j]=temp0[j+vergule_pos[3]+1];
         y0[j]=temp0[vergule_pos[4]+1];
         y0[j+1]='\0';
         ok=1;
         x[0]=x0[0];
         x[1]=x0[1];
         y[0]=y0[0];
         y[1]=y0[1];
         return i0;
      };
   }while(ok==0);
}
#endif
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
      //fprintf(GPS,"n_tel %d=%s\r\n",n,n01);
   }
   else
   {
      //fprintf(GPS,"SIM2EEPROM erreur");
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
         tel_n0[i1]='\0';
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
   for(i1=0;i1<read_eeprom(sold_add);i1++)
   {
      msg[i1]=read_eeprom(sold_add+1+i1);
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
//******************************************************************************
byte speed_cal(char s[3])
{  
   
   byte p,s0;
   //strcpy(s,"21.39");
   s0=0;
   p=0;
   for(p=0;p<3;p++)
   {
      if((s[p]>'0')&&(s[p]<'9'))
      {
         s0=s0*10+(s[p]-48);
         //fprintf(DEBUG,"sp=%d s0=%d\r\n",s[p],s0);
      } else break;
   }
   return(s0);
}
//###############################################################################
void gps_dact()
{
   char str0[20],c0,c1=0,f;
   strcpy(str0,"PSRF103,00,00,00,01");
   for(f=0;f<6;f++)
   {
      c1=0;
      str0[9]='0'+f;
      for(c0=0;c0<19;c0++) 
      {
         c1^=str0[c0];
      }
      fprintf(GPS_COM,"$%s*%x\r\n",str0,c1);
   }
}
//------------------------------------------------------------------------------
void gps_act(char f) 
{
   char str0[20],c0,c1=0;
   strcpy(str0,"PSRF103,00,01,00,01");
   str0[9]='0'+f;
   for(c0=0;c0<19;c0++) 
   {
      c1^=str0[c0];
   }
   fprintf(GPS_COM,"$%s*%x\r\n",str0,c1);
}
//------------------------------------------------------------------------------
void init_GPS0()
{
   int o;
   for(o=0;o<15;o++) 
   {
      x[o]='\0';
      y[o]='\0';
   }
   output_float(GPS_MRST);
   delay_ms(1000);
   output_low(GPS_MRST);
   delay_ms(1000);
   output_float(GPS_MRST);
   delay_ms(1000);
   gps_dact();
   
}
//------------------------------------------------------------------------------
int get_gps_pos0()
{
   unsigned char pp=0,vergule_n_p=0,ZL,i,z_count;
   unsigned int16 sx,sy,sx1,sx2,sy1,sy2; 
   char z0[8]; 
   fprintf(DEBUG,"GPS GET POS");
   gps_act(0);
   do
   {
      w=fgetc(GPS_COM);
      if(w==',')
      {
         vergule_pos[vergule_n_p]=pp;
         vergule_n_p++;
      }
      if(pp<6) head[pp]=w;
      temp[pp]=w;
      pp++;
     temp[pp]='\0';   
   }
   while(w!=13);
   w=fgetc(GPS_COM);
   fprintf(DEBUG,"=%s",temp);
   if(pp<70) return (0);
   
   strcpy(txt_test,"$GPGGA");
   if((strncmp(head,txt_test,6))==0) 
   {   
        for(j=0;j<(vergule_pos[2]-vergule_pos[1]);j++) x0[j]=temp[j+vergule_pos[1]+1];
        x0[j]=temp[vergule_pos[2]+1];
        x0[j+1]='\0';
        for(j=0;j<(vergule_pos[4]-vergule_pos[3]);j++) y0[j]=temp[j+vergule_pos[3]+1];
        y0[j]=temp[vergule_pos[4]+1];
        y0[j+1]='\0';
        ok=1;
                
        y[0]=y0[0];
        y[1]=y0[1];
        y[2]=' ';
        y[3]=y0[2];
        y[4]=y0[3];
        sx=((int16)(x0[5]-'0')*1000+(int16)(x0[6]-'0')*100+(int16)(x0[7]-'0')*10+(int16)(x0[8]-'0'))*6;
        sx1=sx/1000;
        sx2=sx%1000;
        x[0]=x0[0];
        x[1]=x0[1];
        x[2]=' ';
        x[3]=x0[2];
        x[4]=x0[3];
        x[5]=' ';
        x[6]=(char)(sx1/10)+'0';
        x[7]=(char)(sx1%10)+'0';
        x[8]='.';
        if(sx2>99)
        {
           x[9]=sx2/100+'0';
           x[10]=(sx2%100)/10+'0';
        }
        else
        {
           x[9]='0';
           x[10]=sx2/10+'0';
        }
        x[11]=x0[10];
        
        sy=((int16)(y0[6]-'0')*1000+(int16)(y0[7]-'0')*100+(int16)(y0[8]-'0')*10+(int16)(y0[9]-'0'))*6;
        sy1=sy/1000;
        sy2=sy%1000;
        y[0]=y0[0];
        y[1]=y0[1];
        y[2]=y0[2];
        y[3]=' ';
        y[4]=y0[3];
        y[5]=y0[4];
        y[6]=' ';
        y[7]=(char)(sy1/10)+'0';
        y[8]=(char)(sy1%10)+'0';
        y[9]='.';
        if(sy2>99)
        {
           y[10]=sy2/100+'0';
           y[11]=(sy2%100)/10+'0';
        }
        else
        {
           y[10]='0';
           y[11]=sy2/10+'0';
        }
        y[12]=y0[11];
        //fprintf(DEBUG,"x0=%s SX=%ld SX1=%ld SX2=%ld\r\n",x0,sx,sx1,sx2);
        return pp;
   }
   else return 0;   
}
