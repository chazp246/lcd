#include "stm8s.h"
#include "milis.h"

/*#include "delay.h"*/
/*#include <stdio.h>*/
/*#include "../lib/uart.c"*/

#define _ISOC99_SOURCE
#define _GNU_SOURCE

#define LED_PORT GPIOC
#define LED_PIN  GPIO_PIN_5
#define LED_ON   GPIO_WriteHigh(LED_PORT, LED_PIN);
#define LED_OFF  GPIO_WriteLow(LED_PORT, LED_PIN);
#define LED_FLIP GPIO_WriteReverse(LED_PORT, LED_PIN);

#define BTN_PORT GPIOE
#define BTN_PIN  GPIO_PIN_4
#define BTN_PUSH (GPIO_ReadInputPin(BTN_PORT, BTN_PIN)==RESET) 

#define CLK_GPIO GPIOD		// port na kterém je CLK vstup budiče
#define CLK_PIN  GPIO_PIN_4	// pin na kterém je CLK vstup budiče
#define DATA_GPIO GPIOD		// port na kterém je DIN vstup budiče
#define DATA_PIN  GPIO_PIN_3	// pin na kterém je DIN vstup budiče
#define CS_GPIO GPIOD		// port na kterém je LOAD/CS vstup budiče
#define CS_PIN  GPIO_PIN_2	// pin na kterém je LOAD/CS vstup budiče

#define CLK_HIGH 			GPIO_WriteHigh(CLK_GPIO, CLK_PIN)
#define CLK_LOW 			GPIO_WriteLow(CLK_GPIO, CLK_PIN)
#define DATA_HIGH           GPIO_WriteHigh(DATA_GPIO, DATA_PIN)
#define DATA_LOW 			GPIO_WriteLow(DATA_GPIO, DATA_PIN)
#define CS_HIGH       		GPIO_WriteHigh(CS_GPIO, CS_PIN)
#define CS_LOW 				GPIO_WriteLow(CS_GPIO, CS_PIN)

// makra adres/příkazů pro čitelnější ovládání MAX7219
#define NOOP 		0  	// No operation
#define DIGIT0 		1	// zápis hodnoty na 1. cifru
#define DIGIT1 		2	// zápis hodnoty na 1. cifru
#define DIGIT2 		3	// zápis hodnoty na 1. cifru
#define DIGIT3 		4	// zápis hodnoty na 1. cifru
#define DIGIT4 		5	// zápis hodnoty na 1. cifru
#define DIGIT5 		6	// zápis hodnoty na 1. cifru
#define DIGIT6 		7	// zápis hodnoty na 1. cifru
#define DIGIT7 		8	// zápis hodnoty na 1. cifru
#define DECODE_MODE 	9	// Aktivace/Deaktivace znakové sady (my volíme vždy hodnotu DECODE_ALL)
#define INTENSITY 	10	// Nastavení jasu - argument je číslo 0 až 15 (větší číslo větší jas)
#define SCAN_LIMIT 	11	// Volba počtu cifer (velikosti displeje) - argument je číslo 0 až 7 (my dáváme vždy 7)
#define SHUTDOWN 	12	// Aktivace/Deaktivace displeje (ON / OFF)
#define DISPLAY_TEST 	15	// Aktivace/Deaktivace "testu" (rozsvítí všechny segmenty)

// makra argumentů
// argumenty pro SHUTDOWN
#define DISPLAY_ON		1	// zapne displej
#define DISPLAY_OFF		0	// vypne displej
// argumenty pro DISPLAY_TEST
#define DISPLAY_TEST_ON 	1	// zapne test displeje
#define DISPLAY_TEST_OFF 	0	// vypne test displeje
// argumenty pro DECODE_MOD
#define DECODE_ALL		0b11111111 // (lepší zápis 0xff) zapíná znakovou sadu pro všechny cifry
#define DECODE_NONE		0 // vypíná znakovou sadu pro všechny cifry

// makra argumentů
// argumenty pro SHUTDOWN
#define DISPLAY_ON		1	// zapne displej
#define DISPLAY_OFF		0	// vypne displej
// argumenty pro DISPLAY_TEST
#define DISPLAY_TEST_ON 	1	// zapne test displeje
#define DISPLAY_TEST_OFF 	0	// vypne test displeje
// argumenty pro DECODE_MOD
#define DECODE_ALL		0b11111111 // (lepší zápis 0xff) zapíná znakovou sadu pro všechny cifry
#define DECODE_NONE		0 // vypíná znakovou sadu pro všechny cifry


// odešle do budiče MAX7219 16bitové číslo složené z prvního a druhého argumentu (nejprve adresa, poté data)
void max7219_posli(uint8_t adresa, uint8_t data){
uint8_t maska; // pomocná proměnná, která bude sloužit k procházení dat bit po bitu
CS_LOW; // nastavíme linku LOAD/CS do úrovně Low (abychom po zapsání všech 16ti bytů mohli vygenerovat na CS vzestupnou hranu)

// nejprve odešleme prvních 8bitů zprávy (adresa/příkaz)
maska = 0b10000000; // lepší zápis je: maska = 1<<7
CLK_LOW; // připravíme si na CLK vstup budiče úroveň Low
while(maska){ // dokud jsme neposlali všech 8 bitů
 if(maska & adresa){ // pokud má právě vysílaný bit hodnotu 1
  DATA_HIGH; // nastavíme budiči vstup DIN do úrovně High
 }
 else{ // jinak má právě vysílaný bit hodnotu 0 a...
  DATA_LOW; // ... nastavíme budiči vstup DIN do úrovně Low
 }
 CLK_HIGH; // přejdeme na CLK z úrovně Low do úrovně High, a budič si zapíše hodnotu bitu, kterou jsme nastavili na DIN
 maska = maska>>1; // rotujeme masku abychom v příštím kroku vysílali nižší bit
 CLK_LOW; // vrátíme CLK zpět do Low abychom mohli celý proces vysílání bitu opakovat
}

// poté pošleme dolních 8 bitů zprávy (data/argument)
maska = 0b10000000;
while(maska){ // dokud jsme neposlali všech 8 bitů
 if(maska & data){ // pokud má právě vysílaný bit hodnotu 1
  DATA_HIGH; // nastavíme budiči vstup DIN do úrovně High
 }
 else{ // jinak má právě vysílaný bit hodnotu 0 a...
  DATA_LOW; // ... nastavíme budiči vstup DIN do úrovně Low
 }
 CLK_HIGH; // přejdeme na CLK z úrovně Low do úrovně High, a v budič si zapíše hodnotu bitu, kterou jsme nastavili na DIN
 maska = maska>>1; // rotujeme masku abychom v příštím kroku vysílali nižší bit
 CLK_LOW; // vrátíme CLK zpět do Low abychom mohli celý proces vysílání bitu opakovat
}

CS_HIGH; // nastavíme LOAD/CS z úrovně Low do úrovně High a vygenerujeme tím vzestupnou hranu (pokyn pro MAX7219 aby zpracoval náš příkaz)
}






// nastaví CLK,LOAD/CS,DATA jako výstupy a nakonfiguruje displej
void max7219_init(void){
GPIO_Init(CS_GPIO,CS_PIN,GPIO_MODE_OUT_PP_LOW_SLOW);
GPIO_Init(CLK_GPIO,CLK_PIN,GPIO_MODE_OUT_PP_LOW_SLOW);
GPIO_Init(DATA_GPIO,DATA_PIN,GPIO_MODE_OUT_PP_LOW_SLOW);
// nastavíme základní parametry budiče
max7219_posli(DECODE_MODE, DECODE_ALL); // zapnout znakovou sadu na všech cifrách
max7219_posli(SCAN_LIMIT, 7); // velikost displeje 8 cifer (počítáno od nuly, proto je argument číslo 7)
max7219_posli(INTENSITY, 5); // volíme ze začátku nízký jas (vysoký jas může mít velkou spotřebu - až 0.25A !)
//max7219_posli(DISPLAY_TEST, DISPLAY_TEST_OFF); // Funkci "test" nechceme mít zapnutou
max7219_posli(SHUTDOWN, DISPLAY_ON); // zapneme displej
}


void init(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);      // taktovani MCU na 16MHz
    GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    init_milis();
    max7219_init();
}



int main(void)
{
    uint32_t time = 0;
    init();
    int i = 0;
    int x = 0;
    int z = 0;
    int d = 0;
    /*init_uart();*/

    while (1) {

        if (milis() - time > 1000) {
            GPIO_WriteReverse(LED_PORT, LED_PIN);
            time = milis();
            max7219_posli(DIGIT0, i);
            max7219_posli(DIGIT1, x);
            max7219_posli(DIGIT2, z);
            max7219_posli(DIGIT3, d);
            i = i + 1;
            if (i == 10){
                i = 0;
                x = x + 1;
            }
            if (x == 10){
                x = 0;
                z = z + 1;
            }
            if (z == 10){
                z = 0;
                d = x + 1;
            }
            if (d == 10){
                i = 0;
                x = 0;
                z = 0;
                d = 0;
            }
        }

        /*LED_FLIP; */
        /*_delay_ms(333);*/
        /*printf("Funguje to!!!\n");*/
    }
}

/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
