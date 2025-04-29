#include<xc.h>
#include<stdio.h>
#pragma config WDT = OFF
#pragma config OSC = HS
#pragma config PBADEN = OFF
#define _XTAL_FREQ 20000000
#pragma config CCP2MX = PORTBE
#define STEP_SIZE 4.8
#define max_ADC 1023
#define LCD PORTD
#define RS RC1
#define RW RC0
#define EN RC2
char str[15];
unsigned int count=0;
void lcd_cmd(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_init(void);
void lcd_display_count(char*str);
void control_pump_pwm(unsigned int water_level);
volatile unsigned int x=0,flag =0;
unsigned int humidity=0,water_level=0;
static unsigned int prev_water_level = 0xFFFF;
void  __interrupt(high_priority) myHighIsr(void)
 {
    if(ADIF==1)
    {
       ADIF=0;
       x=(ADRESH<<8)|ADRESL;
       flag=1;
    }
 }
void main(void)
{
    
    //lcd
    TRISB3=0;
    TRISC0=0;
    TRISC2=0;
    TRISC=0;
    TRISA0=1; 
    TRISD =0x00;
   
    //ADC
    ADCON0=0x01;
    ADCON1=0x0E;
    ADCON2=0xBF;
    T2CON = 0x0F;
    TMR2ON = 1;
    TRISC1 = 0;
    PR2 = 155;
    ADIF=0;
    ADIE=1;
    PEIE=1;
    GIE=1;
    GO=1;
    lcd_init();
    
    while(1)
    { 
        if(flag==1)
        {
            unsigned int result=0;
            result=x*STEP_SIZE;
            water_level = (result/(STEP_SIZE*1024))*100;
            //water_level = (r * 100) / max_ADC;
            control_pump_pwm(water_level);
            sprintf(str,"Moisture: %d%% ",water_level );
           
            if (water_level != prev_water_level)
            {
                lcd_cmd(0x01);
                lcd_display_count(str);
                __delay_ms(1000);
                prev_water_level = water_level;
                
            } 
            flag = 0;
            GO= 1;
        }   
    }  
}

void lcd_init(void)
{
    lcd_cmd(0x38);
    __delay_ms(2);
    lcd_cmd(0x0C);
    __delay_ms(2);  
    lcd_cmd(0x01); 
    __delay_ms(2);
    lcd_cmd(0x06); 
    __delay_ms(2);
}
void lcd_cmd(unsigned char cmd)
{
    LCD = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
  __delay_ms(15);
    EN = 0;
  __delay_ms(5);
}
void lcd_data(unsigned char data)
{
    RS = 1;
    RW = 0;
    LCD = data;
    EN = 1;
    __delay_ms(15);
    EN = 0;
    __delay_ms(5);
}
void lcd_display_count(char*str)
{
    unsigned int i=0;
    for(i=0;str[i]!='\0';i++)
    {
        lcd_data(str[i]);
    }
}  
void control_pump_pwm(unsigned int water_level)
{
     if(water_level >= 0 && water_level <= 30 )
    {   
        CCP2CON = 0x2C;
        CCPR2L = 0x0F;   
    }
    if(water_level > 30 && water_level <= 60 )
    {   
        CCP2CON = 0x0C;
        CCPR2L = 0x27;    
    }
    if(water_level > 60 && water_level <= 85 )
    {  
        CCP2CON = 0x0C;
        CCPR2L = 0x75; 
    }
    if(water_level > 80 && water_level <= 100 )
    {
        CCP2CON = 0x1C;
        CCPR2L = 0x9C;       
    }
    
}
////void control_pump_pwm(unsigned int water_level)
////{
//    x = (water_level * 1023) / 100; 
//    CCP1CON = (CCP1CON & 0xCF) | ((x & 0x03) << 4); 
//    CCPR1L = x >> 2; 
//}
