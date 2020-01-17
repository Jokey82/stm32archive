#include "stm32f10x.h"
#include<stdbool.h>
#include<stdio.h>
//INDICATOR PARAMS 
#define CHARGES 4
//KEYBOARD SIZE
#define KEY_LINES 4
#define KEY_COLS 4
//PORTS
#define CHARGE_PORT GPIOC
#define DIG_PORT GPIOA
#define KEY_PORT GPIOB 
//KEYBOARD LINES (POWER)
#define LINE0 GPIO_ODR_ODR0
#define LINE1 GPIO_ODR_ODR1
#define LINE2 GPIO_ODR_ODR2
#define LINE3 GPIO_ODR_ODR10
#define LINE0SET GPIO_BSRR_BS0
#define LINE1SET GPIO_BSRR_BS1
#define LINE2SET GPIO_BSRR_BS2
#define LINE3SET GPIO_BSRR_BS10
#define LINE0RESET GPIO_BSRR_BR0
#define LINE1RESET GPIO_BSRR_BR1
#define LINE2RESET GPIO_BSRR_BR2
#define LINE3RESET GPIO_BSRR_BR10

//KEYBOARD COLUMS (SCAN)
#define COL0 GPIO_IDR_IDR11
#define COL1 GPIO_IDR_IDR12
#define COL2 GPIO_IDR_IDR13
#define COL3 GPIO_IDR_IDR14
//INDICATOR CHARGES
#define D0 GPIO_ODR_ODR8
#define D1 GPIO_ODR_ODR7
#define D2 GPIO_ODR_ODR6
#define D3 GPIO_ODR_ODR9
#define ALLCHARGES (D0 | D1 | D2 | D3)
//INDICATOR SEGMENTS
#define SEG_A GPIO_ODR_ODR8  //left bottom
#define SEG_B GPIO_ODR_ODR9  //bottom
#define SEG_C GPIO_ODR_ODR10 //right bottom
#define SEG_D GPIO_ODR_ODR11 //center
#define SEG_E GPIO_ODR_ODR12 //top
#define SEG_F GPIO_ODR_ODR2  //right top
#define SEG_G GPIO_ODR_ODR3  //left top
#define SEG_A_C GPIO_ODR_ODR0  //center
#define SEG_B_C GPIO_ODR_ODR1  //top left
#define SEG_C_C GPIO_ODR_ODR4 //top+bottom
#define SEG_D_C GPIO_ODR_ODR5 //top right
#define SEG_E_C GPIO_ODR_ODR6 //bottom right
#define SEG_F_C GPIO_ODR_ODR7 //bottom left
//INDICATOR DIGITS
#define DIG0 (SEG_A | SEG_B | SEG_C|SEG_E|SEG_F | SEG_G) 
#define DIG1 (SEG_C |SEG_F)
#define DIG2 (SEG_B | SEG_D | SEG_E | SEG_F | SEG_A)
#define DIG3 (SEG_B | SEG_C |SEG_D|SEG_E|SEG_F)
#define DIG4 (SEG_C |SEG_D|SEG_F | SEG_G)
#define DIG5 (SEG_B | SEG_C |SEG_D|SEG_E | SEG_G)
#define DIG6 (SEG_A | SEG_B | SEG_C |SEG_D|SEG_E|SEG_G)
#define DIG7 (SEG_C |SEG_E|SEG_F)
#define DIG8 (SEG_A | SEG_B | SEG_C |SEG_D|SEG_E|SEG_F | SEG_G)
#define DIG9 (SEG_B | SEG_C |SEG_D |SEG_E|SEG_F | SEG_G)

#define DIG0_C (SEG_B_C | SEG_C_C |SEG_D_C|SEG_E_C|SEG_F_C)
#define DIG1_C (SEG_D_C | SEG_E_C)
#define DIG2_C (SEG_C_C | SEG_D_C | SEG_F_C | SEG_A_C)
#define DIG3_C (SEG_C_C | SEG_D_C | SEG_A_C | SEG_E_C )

//FUNCTION DEFINITIONS
void display_digit(uint8_t digit, uint8_t charge);
void display_number(uint8_t number[]);
void display_oneseg(uint8_t n);
void string_num_to_arr(char str[], uint8_t target[]);
void number_to_array(uint16_t num, uint8_t array[]);
void reset_display(void);
uint8_t read_keypad(uint8_t layout[][4]);
void read_number(uint8_t layout[][4], uint8_t target[]);

//GLOBAL VARIABLES
	const uint16_t linesset[] = {LINE0SET, LINE1SET, LINE2SET, LINE3SET};
	const uint32_t linesreset[] = {LINE0RESET, LINE1RESET, LINE2RESET, LINE3RESET};
	const uint16_t columns[] = {COL0, COL1, COL2, COL3};
	const uint16_t digit_arr[] = {DIG0, DIG1, DIG2, DIG3, DIG4, DIG5, DIG6, DIG7, DIG8, DIG9};
	const uint16_t char_digs[] = {DIG0_C, DIG1_C, DIG2_C, DIG3_C};
	const uint16_t charge_arr[] = {D0, D1, D2, D3};
	
int main(void){
//PERIPHERALS INIT
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN | RCC_APB1ENR_TIM2EN;
  //CHARGE PORTS INIT
	CHARGE_PORT->CRL &= ~GPIO_CRL_CNF6;
	CHARGE_PORT->CRL |= GPIO_CRL_MODE6_0;
	CHARGE_PORT->CRL &= ~GPIO_CRL_CNF7;
	CHARGE_PORT->CRL |= GPIO_CRL_MODE7_0;
	CHARGE_PORT->CRH &= ~GPIO_CRH_CNF8;
	CHARGE_PORT->CRH |= GPIO_CRH_MODE8_0;
	CHARGE_PORT->CRH &= ~GPIO_CRH_CNF9;
	CHARGE_PORT->CRH |= GPIO_CRH_MODE9_0;
	//DIGIT PORTS INIT
	DIG_PORT->CRL &= ~GPIO_CRL_CNF0;
	DIG_PORT->CRL |= GPIO_CRL_MODE0_0;
	DIG_PORT->CRL &= ~GPIO_CRL_CNF1;
	DIG_PORT->CRL |= GPIO_CRL_MODE1_0;
	DIG_PORT->CRL &= ~GPIO_CRL_CNF2;
	DIG_PORT->CRL |= GPIO_CRL_MODE2_0;
	DIG_PORT->CRL &= ~GPIO_CRL_CNF3;
	DIG_PORT->CRL |= GPIO_CRL_MODE3_0;
	DIG_PORT->CRL &= ~GPIO_CRL_CNF4;
	DIG_PORT->CRL |= GPIO_CRL_MODE4_0;
	DIG_PORT->CRL &= ~GPIO_CRL_CNF5;
	DIG_PORT->CRL |= GPIO_CRL_MODE5_0;
	DIG_PORT->CRL &= ~GPIO_CRL_CNF6;
	DIG_PORT->CRL |= GPIO_CRL_MODE6_0;
	DIG_PORT->CRL &= ~GPIO_CRL_CNF7;
	DIG_PORT->CRL |= GPIO_CRL_MODE7_0;
	DIG_PORT->CRH &= ~GPIO_CRH_CNF8;
	DIG_PORT->CRH |= GPIO_CRH_MODE8_0;
	DIG_PORT->CRH &= ~GPIO_CRH_CNF9;
	DIG_PORT->CRH |= GPIO_CRH_MODE9_0;
	DIG_PORT->CRH &= ~GPIO_CRH_CNF10;
	DIG_PORT->CRH |= GPIO_CRH_MODE10_0;
	DIG_PORT->CRH &= ~GPIO_CRH_CNF11;
	DIG_PORT->CRH |= GPIO_CRH_MODE11_0;
	DIG_PORT->CRH &= ~GPIO_CRH_CNF12;
	DIG_PORT->CRH |= GPIO_CRH_MODE12_0;
	//KEYBOARD PORTS INIT
	KEY_PORT->CRL &= ~GPIO_CRL_CNF0;
  KEY_PORT->CRL |= GPIO_CRL_MODE0_0;
	KEY_PORT->CRL &= ~GPIO_CRL_CNF1;
  KEY_PORT->CRL |= GPIO_CRL_MODE1_0;
	KEY_PORT->CRL &= ~GPIO_CRL_CNF2;
  KEY_PORT->CRL |= GPIO_CRL_MODE2_0;
	KEY_PORT->CRH &= ~GPIO_CRH_CNF10;
  KEY_PORT->CRH |= GPIO_CRH_MODE10_0;
	//TIMER 3 CONFIG
	TIM3->PSC = 23999;          //PRESCALER
	TIM3->ARR = 500;            //AUTO RELOAD VALUE
	TIM3->CR1 = TIM_CR1_CEN;    //ENABLE TIMER
	//TIMER 4 CONFIG
	TIM4->PSC = 23999;          //PRESCALER
	TIM4->ARR = 5;              //AUTO RELOAD VALUE
	TIM4->CR1 = TIM_CR1_CEN;    //ENABLE TIMER
	//TIMER 2 CONFIG
	TIM2->PSC = 23999;          //PRESCALER
	TIM2->ARR = 1000;              //AUTO RELOAD VALUE
	TIM2->CR1 = TIM_CR1_CEN;    //ENABLE TIMER
	//VARIABLES
	uint8_t number[4] = {0, 0, 0, 0};
	uint8_t keyboard_layout[][4] = {
		{1, 2, 3, 11},
		{4, 5, 6, 12},
		{7, 8, 9, 13}, 
		{1, 0, 15, 14}};
	
	read_number(keyboard_layout, number);
	//MAIN LOOP
	while(1){
			display_number(number);
}
}

//FUNCTIONS
//OUTPUT
void display_digit(uint8_t digit, uint8_t charge){
	CHARGE_PORT->ODR = charge_arr[charge];
	DIG_PORT->ODR |= digit_arr[digit];
}
void display_oneseg(uint8_t n){
	DIG_PORT->ODR = char_digs[n];
}

void delay(int n){
	for(int i =0;i<n;i++);
}
void reset_display(void){
	CHARGE_PORT->ODR &= ~ALLCHARGES;
	DIG_PORT->ODR &= ~DIG8;
}

void display_number(uint8_t number[]){
	for(int i=0;i<CHARGES;i++){
			display_digit(number[i], i);
			reset_display();
	}
}


void string_num_to_arr(char str[], uint8_t target[]){
	for(int i = 0; i<CHARGES; i++){
		target[i] = str[i] - '0';
	}
}

void number_to_array(uint16_t num, uint8_t array[]){
	for (int i = 0, j = CHARGES-1; i < CHARGES; i++, j--) {
		array[j] = num % 10;
		num /= 10;
	}
}
//INPUT 

void read_number(uint8_t layout[][4], uint8_t target[4]){
	uint8_t tmp;
	size_t pointer=0;
	bool written = false;
	while(tmp != 11){
			tmp = read_keypad(layout);
		if(tmp < 10 && !written){
			target[pointer] = tmp;
			written = true;
		}
		else if(tmp == 12){
			written = false;
			pointer = pointer>=3?0:pointer+1;
		}
		else if(tmp == 13){
			written = false;
			target[pointer] = 0;
		}
		display_number(target);
		display_oneseg(pointer);
	}
	return; 
}



uint8_t read_keypad(uint8_t layout[][4]){
	for(int line=0;line<KEY_LINES;line++){
		for(int i=0;i<KEY_LINES;i++){
			if(i == line)
				KEY_PORT->BSRR |= linesreset[i];
			else
				KEY_PORT->BSRR |= linesset[i];
		}
		for(int i=0; i<KEY_COLS;i++){
			if(!(KEY_PORT->IDR & columns[i])){
				TIM3->SR &= ~TIM_SR_UIF;
				while(!(TIM3->SR & TIM_SR_UIF)){
					if((KEY_PORT->IDR & columns[i])){
						TIM3->SR &= ~TIM_SR_UIF;
					return 10;
					}
				};
				if(!(KEY_PORT->IDR & columns[i])){
					TIM3->SR &= ~TIM_SR_UIF;
					return layout[line][i];
				}
			}
		}
	}
	return 10;
}
			
