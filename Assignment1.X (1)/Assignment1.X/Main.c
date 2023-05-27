#define _XTAL_FREQ   4000000UL  // needed for the delays, set to 4 MH= your crystal frequency
// CONFIG1H
#pragma config OSC = XT         // Oscillator Selection bits (XT oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = ON         // Watchdog Timer Enable bit (WDT enabled)
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = ON      // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = ON         // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-003FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (004000-007FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (008000-00BFFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (00C000-00FFFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-003FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (004000-007FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (008000-00BFFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (00C000-00FFFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (00C000-00FFFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0007FFh) not protected from table reads executed in other blocks)

#include <xc.h>
#include <stdio.h>
#define STARTVALUE  3036

#ifndef XC_LCD_X8_H
#define _XTAL_FREQ   4000000UL 
#define LCD_TYPE 2
#define LCD_LINE_TWO 0x40
#define LCD_LINE_SIZE 16
#define lcd_output_enable(x) PORTEbits.RE1 = x
#define lcd_output_rs(x) PORTEbits.RE2 = x
struct lcd_pin_map {
    unsigned un1    : 1;
    unsigned rs     : 1;
    unsigned rw     : 1;
    unsigned enable : 1;
    unsigned data   : 4;
} lcd  __at(0xF83);
#endif

void delay_ms(unsigned int n);
void delay_cycles(unsigned char n);
void init_adc_no_lib(void);
float read_adc_voltage(unsigned char channel);
int read_adc_raw_no_lib(unsigned char channel);
void lcd_send_nibble(unsigned char n);
void lcd_send_byte(unsigned char cm_data, unsigned char n);
void lcd_init(void);
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_putc(char c);
void lcd_puts(char *s);
void restartTimer0(void);
unsigned char LCD_INIT_STRING[4] = {0x08, 0xc, 1, 6};



void delay_ms(unsigned int n) {
    int i;
    for (i=0; i < n; i++) __delaywdt_ms(1) ; 
}

void delay_cycles(unsigned char n) {
    int x;
    for (x = 0; x <= n; x++) CLRWDT();
}

void reloadTimer0(void) {
    TMR0H = (unsigned char)((STARTVALUE >>  8) & 0x00FF);
    TMR0L = (unsigned char)(STARTVALUE & 0x00FF );   
}



void init_adc_no_lib(void) {
    ADCON0 = 0;
    ADCON0bits.ADON = 1;
    ADCON2 = 0b10001001;
}

int read_adc_raw_no_lib(unsigned char channel) {
    int raw_value;
    ADCON0bits.CHS = channel;
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO) {};
    raw_value = ADRESH << 8 | ADRESL;
    return raw_value;
}

float read_adc_voltage(unsigned char channel) {
    int raw_value;
    float voltage;
    raw_value = read_adc_raw_no_lib(channel);
    voltage = (raw_value * 5) / 1023.0;
    return voltage;
}

void lcd_send_nibble(unsigned char n) {
    lcd.data = n;
    delay_cycles(1);
    lcd_output_enable(1);
    __delaywdt_us(2);
    lcd_output_enable(0);
}

void lcd_send_byte(unsigned char cm_data, unsigned char n) {
    lcd_output_rs(cm_data);
    delay_cycles(1);
    delay_cycles(1);
    lcd_output_enable(0);
    lcd_send_nibble(n >> 4);
    lcd_send_nibble(n & 0x0f);
    if (cm_data) __delaywdt_us(200);
    else delay_ms(2);
}

void lcd_init(void) {
    unsigned char i;
    lcd_output_rs(0);
    lcd_output_enable(0);

    delay_ms(25);   
    for (i = 1; i <= 3; ++i) {
        lcd_send_nibble(3);
        delay_ms(6);
    }

    lcd_send_nibble(2);
    for (i = 0; i <= 3; ++i) lcd_send_byte(0, LCD_INIT_STRING[i]);
}

void lcd_gotoxy(unsigned char x, unsigned char y) {
    unsigned char address;

    switch (y) {
        case 1: address = 0x80;
            break;
        case 2: address = 0xc0;
            break;
        case 3: address = 0x80 + LCD_LINE_SIZE;
            break;
        case 4: address = 0xc0 + LCD_LINE_SIZE;
            break;
    }
    address += x - 1;
    lcd_send_byte(0, (unsigned char) (0x80 | address));
}

void lcd_putc(char c) {
    switch (c) {
        case '\f': lcd_send_byte(0, 1);
            delay_ms(2);
            break;
        case '\n': lcd_gotoxy(1, 2);
            break;
        case '\b': lcd_send_byte(0, 0x10);
            break;
        default: lcd_send_byte(1, c);
            break;
    }
}

void lcd_puts(char *s) {
    while (*s) {
        lcd_putc(*s);
        s++;
    }
}

void init(void) {
    
    // Setup Port
    ADCON0 = 0x00;
    ADCON1 = 0x0C; // 3 Analog Channels
    TRISA  = 0xFF; // All Inputs
    TRISB  = 0xFF; // All Inputs
    TRISC  = 0x80; // RX Input, Others Outputs
    TRISD  = 0x00; // All Outputs
    TRISE  = 0x00; // All Outputs
    PORTD   = 0   ;
    PIE1    = 0   ;
    // Configure Timer 3 for PWM (1:8 prescaler, 16-bit mode)
    T3CON = 0x31; // T3CKPS1 = 1, T3CKPS0 = 0, TMR3ON = 1
    TMR3 = 0;
    TMR3IF = 0;
    TMR3IE = 1;
    //Interrupts
    INTCONbits.GIEH   = 1;    // Enable global interrupt bits
    INTCONbits.GIEL   = 1;    // Enable global interrupt bits
    INTCON2 = 0;
    INTCON3 = 0;
    INTCON2bits.INTEDG0 = 1;  // Interrupt 0 on rising edge
    INTCON2bits.INTEDG1 = 1;  // Interrupt 1 on rising edge
    INTCON2bits.INTEDG2 = 1;  // Interrupt 2 on rising edge
    INTCON3bits.INT1IE  = 1;  // Enable external interrupt 1
    INTCON3bits.INT2IE  = 1;  // Enable external interrupt 2
    RCONbits.IPEN = 0;        // Disable Interrupt priority , All are high
    INTCONbits.INT0IE   = 1;  // Enable external interrupt 0
    PORTCbits.RC5 = 0; // Turn Off Heater
    T3CONbits.TMR3ON = 1; 
    //variables

    
    //Timer0
    T0CON = 0;
    T0CONbits.T0PS0  = 1; // 16 Pre_Scalar
    T0CONbits.T0PS1  = 1;
    T0CONbits.T0PS2  = 0;
    T0CONbits.TMR0ON = 1;
    INTCONbits.TMR0IE = 1;    // Enable Timer0 Interrupt
    T0CONbits.TMR0ON  = 1;    // Start timer 0
    reloadTimer0();
    
}

void reloadTimer3(void)
{  
    TMR3H = (unsigned char)((STARTVALUE >>  8) & 0x00FF);
    TMR3L = (unsigned char)(STARTVALUE & 0x00FF );   
}

void init_pwm1(void)//10 bit accurcy
{ //cooler
    PR2 = 255; //10 bit accurcy
    T2CON = 0;
    CCP1CON = 0x0C; // Select PWM mode 
    //CCP1CONbits.CCP1M = 0x0C // can also be set like that
    T2CONbits.TMR2ON = 1;  // turn the timer on 
    TRISCbits.RC2 =0;  //CCP1 is output
}

void set_pwm1_raw(unsigned int raw_value)//raw value 0 -- 1023 corresponds to  0--100%
{
    CCPR1L = (raw_value >> 2) & 0x00FF; // Load the upper 8 bit in CCPL1
    CCP1CONbits.DC1B = raw_value & 0x0003; //first two bits in bits 4, 5 of CCP1COn
}

void set_pwm1_percent(float value)// value 0--100%, corresponds to 0--1023
{ //cooler
    float tmp = value*1023.0/100.0;//scale 0--100 to 0--1023
    int raw_val = (int)(tmp +0.5); // for rounding
    if ( raw_val> 1023) raw_val = 1023;// Do not exceed max value
    set_pwm1_raw(raw_val);        
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned short seconds = 0;
unsigned short minutes = 0;
unsigned short hours   = 0;
unsigned short modeschange = 0;
unsigned short cooker  = 0;
signed short cookingTime = 0;
unsigned short buzzer = 0;
float t;
float sp,p1,voltage;
int timer=0;
unsigned char modes = 0; 
unsigned char Percent_Heat_Counter=0;
int TON ,hys=0;
char tog=0;
void modeschange1(void)
{
      INTCONbits.INT0IF = 0;
    if (modeschange == 0) modeschange = 1;
    else if (modeschange == 1) modeschange = 2;
    else if (modeschange == 2) modeschange = 3;
    else if (modeschange == 3) modeschange = 4;
    else if (modeschange == 4) modeschange = 5;
    else if (modeschange == 5) modeschange = 6;
    else if (modeschange == 6) modeschange = 0;


}

void buzzerA(void) {
    PORTCbits.RC1 = 1;
    delay_ms(2000);
    PORTCbits.RC1 = 0;
    delay_ms(2000);
    PORTCbits.RC1 = 1;
    delay_ms(2000);
    PORTCbits.RC1 = 0;
    delay_ms(2000);
    PORTCbits.RC1 = 1;
    delay_ms(2000);
    PORTCbits.RC1 = 0;
    delay_ms(2000);
    PORTCbits.RC1 = 1;
    delay_ms(2000);
    PORTCbits.RC1 = 0;
    delay_ms(2000);
    
}
void update_hysteresis(void){
   if(modeschange==6&&PORTBbits.RB3 == 0){
    
        if(hys==3)
            hys=3;
        else
            hys++;
        
        }
   if(modeschange==6&&PORTBbits.RB4 == 0){
    
        if(hys==0)
            hys=0;
        else
            hys--;
        
        }
}
void update_cooking_time(void){
    INTCONbits.TMR0IF = 0;
    cookingTime = seconds + 60*minutes + 60*60*hours;
    if(cooker == 1 && cookingTime != 0 ){
        cookingTime--;
        if(cookingTime == 0){
            cooker = 0;
            PORTCbits.RC5 = 0;
            buzzer = 1;
            Percent_Heat_Counter=0;
        }
      
   }
     hours = cookingTime / 3600;
     minutes = (cookingTime - hours*3600)/60;
     seconds = cookingTime - hours*3600 - minutes*60;
   
   reloadTimer0(); 
}

void incrementTime(void) {
    
    if(modeschange == 0){
        cookingTime++;
    }else if(modeschange == 1){
        cookingTime +=10;
    }else if(modeschange == 2){
        cookingTime +=60;
    }else if(modeschange == 3){
        cookingTime +=600;
    }else if(modeschange == 4){
        cookingTime +=3600;
    }
    
    if(cookingTime > 36000){
        cookingTime = 36000;
    }
    if(hours>4){

         cookingTime=0;
     }
     hours = cookingTime / 3600;
     minutes = (cookingTime - hours*3600)/60;
     seconds = cookingTime - hours*3600 - minutes*60;
        
        
}

void decrementTime(void) {
    if(modeschange == 0){
        cookingTime--;
    }else if(modeschange == 1){
        cookingTime -=10;
    }else if(modeschange == 2){
        cookingTime -=60;
    }else if(modeschange == 3){
        cookingTime -=600;
    }else if(modeschange == 4){
        cookingTime -=3600;
    }
    
    if(cookingTime < 0){
        cookingTime = 0;
    }
     hours = cookingTime / 3600;
     minutes = (cookingTime - hours*3600)/60;
     seconds = cookingTime - hours*3600 - minutes*60;
        
        
}



void Timer3_int(void){
    
        if(timer>=10){
                timer=0;
            }
            else if(timer <= (Percent_Heat_Counter) && Percent_Heat_Counter!=0){
                INTCON3bits.INT1IE=1;
                PORTCbits.RC5=1; 
                timer++;
                delay_ms(40);
                
              
            }
            else{
              INTCON3bits.INT1IE=0;
              PORTCbits.RC5=0;
              timer++;
              delay_ms(40);  
            
            }
           set_pwm1_percent(Percent_Heat_Counter);
           reloadTimer3();
}

int error =0;
void Auto_Heat(void){


    if (t > (sp + hys))
            PORTCbits.RC5=0;
    else {
    error = sp-t; // round to integer
    if( error > 10) error = 10;// makes Heat percentage to max 100%
    else if( error < 5) error= 5; //makes Heat Percentage to 50% in the PWM
    else if( sp > 155) error = 10;
 //else keep the Error as computed Error = SetPoint ? T
        Percent_Heat_Counter = error; //Generates the PWM by Timer3 interrupt described 
 // in the Heater section
        set_pwm1_percent(Percent_Heat_Counter);
        

}



}


void __interrupt(high_priority) highIsr (void) {
    if(INTCONbits.TMR0IF){
        update_cooking_time();
    }else if(INTCONbits.INT0IF) {
         delay_ms(500);
        modeschange1();
       
    }
    else if(INTCON3bits.INT1IF){
        INTCON3bits.INT1IF = 0;
        if(cookingTime != 0){ 
          cooker = 1;
        }
    }else if(INTCON3bits.INT2IF){
        INTCON3bits.INT2IF = 0;
        cooker = 0;
        PORTCbits.RC5 = 0;
    }else if(PIR2bits.TMR3IF){
       Timer3_int();
       PIR2bits.TMR3IF=0;
    }
}  

short v=0;
void main(void) {
    init();
    char Buffer[32]; 
    lcd_init();
    init_adc_no_lib();
    lcd_putc('\f'); // Clears The Display
   
    
    while (1) {
        CLRWDT();
        
        p1 = read_adc_voltage(0);
        voltage = read_adc_voltage(2);
        t = 300 * voltage;
        sp=t/5;
        
        if(PORTBbits.RB3 == 0) incrementTime();
        else if (PORTBbits.RB4 == 0) decrementTime();      
        else if(PORTBbits.RB5 == 0){
            cooker = 0;
            PORTCbits.RC5 = 0;
            seconds = 0;
            minutes = 0;
            hours   = 0;
            Percent_Heat_Counter=0;
        }
       
        
        lcd_gotoxy(1, 1);
        sprintf(Buffer, "Time: %02d:%02d:%02d", hours,minutes,seconds);
        lcd_puts(Buffer);

        lcd_gotoxy(1, 2);
        sprintf(Buffer, "CT:%4.1fC ", t);
        lcd_puts(Buffer);
        
        lcd_gotoxy(11, 2);
        sprintf(Buffer, "CK:%s",cooker==1?" ON":"OFF");
        lcd_puts(Buffer);
                   
        lcd_gotoxy(1, 3);
        sprintf(Buffer, "SP:%4.1fC ",sp);
        lcd_puts(Buffer);
        
        lcd_gotoxy(11, 3);
         sprintf(Buffer, "HT:%s",PORTCbits.RC5==1?" ON":"OFF");
        lcd_puts(Buffer);
        
        lcd_gotoxy(1, 4);
        sprintf(Buffer, "MD:%s", modeschange==0?"S  ":modeschange==1?"10S  ":modeschange==2?"M  ":modeschange==3?"10M  ":modeschange==4?"HR  ":modeschange==5?"HT  ":"HY  ");
        lcd_puts(Buffer);
        
        
        
        if(PORTBbits.RB3 == 0&& modeschange==5){
            if(Percent_Heat_Counter==10)
                Percent_Heat_Counter=10;
            else
          Percent_Heat_Counter++;{
            delay_ms(200);}
        }
        if(PORTBbits.RB4 == 0 && modeschange==5){
            if(Percent_Heat_Counter==0)
                Percent_Heat_Counter=0;
            else  {
                
          Percent_Heat_Counter=Percent_Heat_Counter-1;
            delay_ms(200);
            }
            
        }

        update_hysteresis();
        

        lcd_gotoxy(8, 4);
        if(PORTAbits.RA5==0){
           tog=!tog;
        }
        
        if(tog==0){
                  
        sprintf(Buffer, "P:%d  ",Percent_Heat_Counter*10);
        lcd_puts(Buffer); 
        }
        else{
        sprintf(Buffer, "P:A  ");
        lcd_puts(Buffer);   
        Auto_Heat();
        }
        
       

        
        lcd_gotoxy(13, 4);
        sprintf(Buffer, "H:%d  ",hys);
        lcd_puts(Buffer);
        
        
         lcd_gotoxy(8, 4);
       
         
        if(buzzer == 1){
            buzzer = 0;
            buzzerA();
        }
        
        delay_ms(200);
    }
}


