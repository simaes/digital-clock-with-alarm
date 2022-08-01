#include <mega32.h>
#include <alcd.h>
#include <stdio.h>
#include <delay.h>

char ref[]={0xFE, 0xFD, 0xFB, 0xF7};
int keys[]={1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0};
unsigned long int key_value, inputcounter;
char keypad();
void display();
void alarm();

char st[16]; 
int sec=0, min=0, hour=0;
int min_alm=0, hour_alm=0;
char status[2][8] = {"!SET","SET"};
int if_started=0, if_alarm=0;

// Timer1 output compare A interrupt service routine
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
 if(if_started==1){
    if (sec<60)
    {
        sec++;
    }

    if (sec==60)
    {
        if (min<60)
        {
            min++;
        }
        sec=0;
    }

    if (min==60)
    {
        if (hour<24)
        {
            hour++;
        }
        min=0;
    }
    
    if (hour==24)

    {
        hour=0;
    }
   }
}

void main(void)
{
DDRC = 0x00;    //input
PORTC = 0xFF;   //Pull up
DDRD = 0xFF;    //output

DDRB = 0xFF; 
PORTB.5 = 0xFF;   //output
PORTB.4 = 0xFF;
PORTB.0= 0x00;

lcd_init(16);

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 0.977 kHz
// Mode: CTC top=OCR1A
// OC1A output: Disconnected
// OC1B output: Disconnected
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer Period: 1.024 ms
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: On
// Compare B Match Interrupt: Off
TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (1<<WGM12) | (1<<CS12) | (0<<CS11) | (1<<CS10);
OCR1A=815;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=(0<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (1<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (0<<OCIE0) | (0<<TOIE0);

// Global enable interrupts
#asm("sei")

while(1)
      { 
        keypad();
        display();
        alarm();
      }
}

char keypad()
{
while(1)
{
    int row=0, col=-1, pos=-1; 
    for(row=0; row<4; row++)
    { 
        PORTD = ref[row]; 
        if(PINC.0==0)
        {   
            while(PINC.0==0);
            col=0;
            break;
        }
        if(PINC.1==0)
        {   
            while(PINC.1==0);
            col=1;
            break;
        }
        if(PINC.2==0)
        {   
            while(PINC.2==0);
            col=2;
            break;
        }                
    }   
    if(col != -1)
    {             
        pos = row*3+col;  // r,c=0 pos=0, r=1,c=2 pos=5
        inputcounter = ( inputcounter *10 ) + keys[pos];
        if(pos == 9){      
            key_value = ( inputcounter / 10 ) ;
            inputcounter = 0; 
            //Format: hhmm
            min = key_value%100;
            hour = key_value/100;
            if(min>60)
            {
            min = min%60;
            hour = hour + 1;            
            }
            if_started=1;
            lcd_clear();
        }
        if(pos == 11){
            key_value = ( inputcounter / 10 ) ;
            inputcounter = 0; 
            //Format: hhmm
            min_alm = key_value%100;
            hour_alm = key_value/100;
            if(min_alm>60)
            {
            min_alm = min_alm%60;
            hour_alm = hour_alm + 1;            
            }
            if_alarm=1;
            lcd_clear();
        }
    }
    return 0;  
}
}

void display()
{ 
   sprintf(st, "%i:%i:%i ALM:%s", hour, min, sec, status + if_alarm); 
   lcd_gotoxy(0,0);
   lcd_puts(st);  
   
   sprintf(st, "ALARM: %i:%i", hour_alm, min_alm); 
   lcd_gotoxy(0,1);
   lcd_puts(st);                            
}

void alarm()
{
    if((hour==hour_alm)&&(min==min_alm)) 
    {
      if(if_alarm){
        PORTB.6=1;  //buzzer on  
        PORTB.0=1; //led on
      }  
    }
    else
    {
       PORTB.6=0; //buzzer off
       PORTB.0=0; //led off
    }
    if(PINB.5==0)
    {
        PORTB.6=0; //buzzer off
        PORTB.0=0; //led off        
    }  
    else if(PINB.4==0)
    {      
        if((if_alarm==1)&&(hour==hour_alm)&&(min==min_alm))
        {
            while(PINB.4==0);
            min_alm = min_alm + 2;  //delay for 2min
            PORTB.6=0; //buzzer off
            PORTB.0=0; //led off 
        }
    }
}