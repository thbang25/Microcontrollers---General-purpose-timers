#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f0xx.h"
#include "lcd_stm32f0.h"

#define TRUE 1 //check
#define FALSE 0 //check

uint32_t bitpattern1 = 0x0000; // no LEDs lit
uint32_t bitpattern2 = 0xFFFF; // all LEDs lit
uint32_t bitpattern3 = 0xAAAA; // alternating LEDS lit

#define DELAY1 16 // DELAY1 used to create a delay of 22ms
#define DELAY2 65535 // DELAY2 used to create a delay of 22ms

volatile uint8_t counter = 0;

typedef uint8_t flag_t;//create a new variable
flag_t startFlag = FALSE; //set condition
flag_t lapFlag = FALSE; //set condition
flag_t stopFlag = FALSE; //set condition
flag_t resetFlag = TRUE; //set condition

void helloWorld();
void timer(void);

uint8_t minutes =0;
uint8_t seconds =0;
uint8_t hundredths =0;
char timeStr[10];

void timer(void){
	init_LCD();
	lcd_command(CLEAR);
	lcd_putstring("Time");
}


//our home screen
void helloWorld(){
    init_LCD();
    lcd_command(CLEAR);
    lcd_putstring("Stopwatch");
    lcd_command(LINE_TWO);
    lcd_putstring( "Press SW0...");
}


void initGPIO(void) {
    // Enable clock
    RCC->AHBENR |= (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN|RCC_AHBENR_GPIOBEN);



    // Configure
        GPIOB->MODER |= (GPIO_MODER_MODER0_0 |
						 GPIO_MODER_MODER1_0 |
						 GPIO_MODER_MODER2_0 |
						 GPIO_MODER_MODER3_0 |
						 GPIO_MODER_MODER4_0 |
						 GPIO_MODER_MODER5_0 |
						 GPIO_MODER_MODER6_0 |
						 GPIO_MODER_MODER7_0 );

        GPIOA->PUPDR |=(GPIO_PUPDR_PUPDR0_0|
        		GPIO_PUPDR_PUPDR1_0|
				GPIO_PUPDR_PUPDR2_0|
				GPIO_PUPDR_PUPDR3_0);
}

//delay function for the LEDS
void stop(void) {
    uint32_t i, j;
    for (i = 0; i < DELAY1; i++) {
        for (j = 0; j <DELAY2 ; j++);
    }
}


void initTIM14(void){
//Enable clock for TIM14
RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;

//Configure counter
TIM14->CR1 &= ~(TIM_CR1_DIR);
TIM14->ARR = 100-1;//99
TIM14->PSC = 4800-1;//4799

//Enable update event interrupt
TIM14->DIER |= TIM_DIER_UIE;

//Stop timer
TIM14->CR1 &= ~(TIM_CR1_CEN);

//Enable Timer 14 update event NVIC interrupt
NVIC_EnableIRQ(TIM14_IRQn);}

//Interrupt handler
void TIM14_IRQHandler(){
//Check for timer overflow event
if(TIM14->SR & TIM_SR_UIF){
//Clear update interrupt flag
TIM14->SR &= ~(TIM_SR_UIF);

//Increment
hundredths++;

//Calculate minutes and seconds from hundredths
if(hundredths == 100){
   hundredths = 0;
   seconds++;}
if(seconds == 60){
   seconds = 0;
   minutes++;}}
}



void checkPB(void){
//SW0
	if (!(GPIOA->IDR & GPIO_IDR_0)){
		startFlag = TRUE;
		lapFlag = FALSE;
		stopFlag = FALSE;
		resetFlag = FALSE;
//SW1
	}
	if (!(GPIOA->IDR & GPIO_IDR_1)){
		startFlag = TRUE;
		lapFlag = TRUE;
		stopFlag = FALSE;
		resetFlag = FALSE;
	}
//SW2
	if (!(GPIOA->IDR & GPIO_IDR_2)){
		startFlag = TRUE;
		lapFlag = FALSE;
		stopFlag = TRUE;
		resetFlag = FALSE;
	}
//SW3
	if (!(GPIOA->IDR & GPIO_IDR_3)){
		startFlag = FALSE;
		lapFlag = FALSE;
		stopFlag = FALSE;
		resetFlag = TRUE;
	}



}

void displayPattern(uint8_t pattern) {
    GPIOB->ODR |= pattern;
}



void display(void){

	if(startFlag & !lapFlag & !stopFlag & !resetFlag){//CHECK0
		GPIOB->ODR |= GPIO_ODR_0;
		stop();
		GPIOB->ODR &=~ bitpattern2;
		TIM14->CR1 |= TIM_CR1_CEN; //start clock

}

	if(startFlag & lapFlag & !stopFlag & !resetFlag){//CHECK1
		GPIOB->ODR |= GPIO_ODR_1;
		stop();
		GPIOB->ODR &=~ bitpattern2;
		counter++;
	}
	if(startFlag & !lapFlag & stopFlag & !resetFlag){//CHECK2
		GPIOB->ODR |= GPIO_ODR_2;
		stop();
		GPIOB->ODR &=~ bitpattern2;
	}
	if(startFlag & !lapFlag & !stopFlag & resetFlag){//CHECK3
		GPIOB->ODR |= GPIO_ODR_3;
		stop();
		GPIOB->ODR &=~ bitpattern2;
		minutes =0;
		seconds =0;
		hundredths =0;
		counter = 0;

	}

}


void convert2BCDASCII(const uint8_t min, const uint8_t sec, const uint8_t hund, char* resultPtr) {
    // Convert minutes to BCD and store as two ASCII characters
    uint8_t minutesConv = (((min / 10) << 4) | (min % 10));
    *resultPtr++ = ((minutesConv >> 4) + '0');
    *resultPtr++ = ((minutesConv & 0x0F) + '0');
    *resultPtr++ = ':';

    // Convert seconds to BCD and store as two ASCII characters
    uint8_t secondsConv = (((sec / 10) << 4) | (sec % 10));
    *resultPtr++ = ((secondsConv >> 4) + '0');
    *resultPtr++ = ((secondsConv & 0x0F) + '0');
    *resultPtr++ = '.';

    // Convert hundredths of a second to BCD and store as two ASCII characters
    uint8_t hundredsConv = (((hund / 10) << 4) | (hund % 10));
    *resultPtr++ = ((hundredsConv >> 4) + '0');
    *resultPtr++ = ((hundredsConv & 0x0F) + '0');

    // Add null terminator to the end of the resulting string
    *resultPtr = '\0';
}





int main(void) {
	helloWorld();
	initGPIO();
	initTIM14();
	TIM14_IRQHandler();





	while(1){

		checkPB();
		display();
		convert2BCDASCII(minutes, seconds, hundredths, timeStr);
	}
}
