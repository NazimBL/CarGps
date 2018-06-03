BYTE  out,alarme_flag,alarme_flag1;
#ifdef ALARME
#define OUT_ENABLE  PIN_A5
#define OUT_CLOCK   PIN_B2
#define OUT_DO      PIN_B3
#bit LED=out.0
#bit VR_D=out.1
#bit VR_G=out.2
#bit LMR=out.3
#bit SRN=out.4
#bit RL_ARR=out.5
#bit PWR=out.6
#bit ALARM=alarme_flag.0
#bit TELE_LK_OLD=alarme_flag.1
#bit TELE_SN_OLD=alarme_flag.2
#bit TELE_N_LK_OLD=alarme_flag.3
#bit TELE_N_SN_OLD=alarme_flag.4
#bit ARM=alarme_flag.5
#bit SRN_EN=alarme_flag.6
#else
   #define PWR PIN_C2
   #define LED PIN_B5
   #define RL_ARR PIN_A1
#endif
#bit msg_ok=alarme_flag.7
#bit arret_flag=alarme_flag1.0

#define  porte  PIN_C2
#define  choc   PIN_C0
#define  capot  PIN_C1
#define  reset  PIN_E3

#define TELE_LK   PIN_B7
#define TELE_SN   PIN_B6
#define TELE_N_SN   PIN_B5
#define TELE_N_LK   PIN_B4

#ifdef ALARME
void write_alarm_outputs(BYTE data) 
{
  BYTE i;

  output_low(OUT_CLOCK);
  output_low(OUT_ENABLE);

  for(i=0;i<8;i++) 
  {  
      if(bit_test(data,7-i)) output_high(OUT_DO);
      else output_low(OUT_DO);
      delay_ms(1);
      shift_left(data,1,0);
      output_high(OUT_CLOCK);
      delay_ms(1);
      output_low(OUT_CLOCK);
      delay_ms(1);
  }
  output_high(OUT_ENABLE);
  delay_ms(1);
  output_low(OUT_ENABLE);

}

int read_alarm_input()
{
   if(!input(porte)) return(1); 
   if(!input(choc))  return(3); 
   if(!input(capot)) return(2); 
   //if(!input(reset)) return(4); 
   return(0);
   
}

//*****************************************************************************/
void ARM_action()
{
   //fprintf(DEBUG,"ARM\n\r");
   LMR=1;
   SRN=1;
   VR_D=0;
   VR_G=1;
   write_alarm_outputs(out);
   delay_ms(200);
   LMR=0;
   SRN=0;
   write_alarm_outputs(out);
   delay_ms(800);
   VR_D=0;
   VR_G=0;
   write_alarm_outputs(out);
}

void D_ARM_action()
{
   //fprintf(DEBUG,"DEARM\n\r");
   LMR=1;
   SRN=1;
   VR_G=0;
   VR_D=1;
   LED=0;
   write_alarm_outputs(out);
   delay_ms(200);
   LMR=0;
   SRN=0;
   write_alarm_outputs(out);
   delay_ms(200);
   LMR=1;
   SRN=1;
   write_alarm_outputs(out);
   delay_ms(200);
   LMR=0;
   SRN=0;
   write_alarm_outputs(out);
   delay_ms(400);
   VR_D=0;
   VR_G=0;
   write_alarm_outputs(out);
}

void alarme_action(char msg)
{
   //TCCR0=0x04;
   ALARM=1;
   SRN=1;
   LMR=1;
  /* if(msg==0) return(0);
   //Modem_On();
   if(msg==-1) uart_str("ATD050488224;\n",14);
   if(msg==1) uart_str("AT+CMSS=1\n",10);
   if(msg==2) uart_str("AT+CMSS=2\n",10);
   if(msg==3) uart_str("AT+CMSS=3\n",10);
   if(msg==4) uart_str("AT+CMSS=4\n",10);*/



}

void Alarm_off()
{
   ALARM=0;
   SRN=0;
   LMR=0;
  // TCCR0=0x05;

}

void CAR_find()
{
   //fprintf(DEBUG,"DEARM\n\r");
   LMR=1;
   SRN=1;
   LED=0;
   write_alarm_outputs(out);
   delay_ms(800);
   LMR=0;
   SRN=0;
   write_alarm_outputs(out);
   delay_ms(800);
   LMR=1;
   SRN=1;
   write_alarm_outputs(out);
   delay_ms(800);
   LMR=0;
   SRN=0;
   write_alarm_outputs(out);
   delay_ms(800);
   LMR=1;
   SRN=1;
   write_alarm_outputs(out);
   delay_ms(800);
   LMR=0;
   SRN=0;
   write_alarm_outputs(out);
   delay_ms(800);
   LMR=1;
   SRN=1;
   write_alarm_outputs(out);
   delay_ms(800);
   LMR=0;
   SRN=0;
   write_alarm_outputs(out);
}

#endif
