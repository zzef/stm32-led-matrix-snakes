#include "stm32f10x_map.h"
#include <string.h>

#define PA0 0
#define PA1 4
#define PA2 8
#define PA3 12
#define PA4 16
#define PA5 20
#define PA6 24
#define PA7 28

#define OPP_50MHZ 0x3
#define IPP 0x8

#define CLR 3
#define IN_CLK 2
#define OUT_CLK 1
#define DATA 0

#define HIGH 1
#define LOW 0

#define WIDTH 8
#define HEIGHT 8

#define UP 4
#define DOWN 7
#define RIGHT 5
#define LEFT 6
#define TIMESTEP 50
#define MAX_SNAKE_SIZE 30

u8 PIXEL_BUFFER[8];

u8 snakesz;
u8 snakebx[MAX_SNAKE_SIZE];
u8 snakeby[MAX_SNAKE_SIZE];
u8 snake_dir;

u8 foodx;
u8 foody;

u32 time;

u32 counter;

u8 check(u8 pin) {
	return GPIOA->IDR & 1<<pin;
}

void set(u8 pin, u8 set) {
	if (set<HIGH) {
		GPIOA->BSRR |= 1<<(pin+16);
		return;
	}
	GPIOA->BSRR |= 1<<pin;
}

void shift_out(u8 value, u8 size){
	u8 mask = 1;
	for (u8 i=0; i<size; i++) {
		set(DATA,value & mask);
		set(IN_CLK,HIGH);
		set(IN_CLK,LOW);
		mask=mask<<1;
	}
}


void display() {
	set(OUT_CLK,HIGH);
	set(OUT_CLK,LOW);
}


void clear_shift_register() {
	set(CLR,LOW);set(CLR,HIGH);
}


void flip_pixel_buffer() {
	for (u8 i = 0; i<HEIGHT; i++) {
		shift_out(~PIXEL_BUFFER[i],WIDTH);
		shift_out(1<<i,WIDTH);
		display();
	} 

}

void draw_point(u8 x, u8 y) {
	PIXEL_BUFFER[y]|=1<<x;	
}

u16 randm(u16 min, u16 max) {
	return TIM4->CNT;
}

void add_body(u8 x, u8 y) {
	snakebx[snakesz]=x;
	snakeby[snakesz]=y;
	snakesz++;
}

void draw_food() {
	draw_point(foodx,foody);
}

void draw_snake() {
	for (int i=0; i<snakesz; i++) {
		draw_point(snakebx[i],snakeby[i]);
	}
}

void move_snake() {


	switch(snake_dir) {
		case(UP): snakeby[0]--; break;
		case(DOWN): snakeby[0]++; break;
		case(LEFT): snakebx[0]--; break;
		case(RIGHT): snakebx[0]++; break;
	}

	snakebx[0]=snakebx[0]%8;
	snakeby[0]=snakeby[0]%8;

	for (int i=snakesz-1; i>=1; i--) {
		snakebx[i]=snakebx[i-1];
		snakeby[i]=snakeby[i-1];
	}
}


void init_game() {
	snakesz=0;
	snake_dir=UP;
	u16 rand = randm(0,65535);
	foodx = (u8) (((0x1100 & rand) >> 8) % 7);
	foody = (u8) ((0x0011 & rand) % 7);
	add_body(4,4);
	add_body(0,0);


}

void handle_input() {

	if (check(UP)) {
		if (snake_dir==DOWN)
			return;
		snake_dir=UP;
	}	
	else if (check(RIGHT)) {
		if (snake_dir==LEFT)
			return;
		snake_dir=RIGHT;
	}
	else if (check(LEFT)) {
		if (snake_dir==RIGHT)
			return;
		snake_dir=LEFT;
	}
	else if (check(DOWN)) {
		if (snake_dir==UP)
			return;
		snake_dir=DOWN;
	}

}


void update() {


	if (time<TIMESTEP)
		return;
	time=0;

	if (foodx==snakebx[0] && foody==snakeby[0]) {
		u16 rand = randm(0,65535);
		foodx = (u8) (((0x1100 & rand) >> 8) % 7);
		foody = (u8) ((0x0011 & rand) % 7);
		add_body(foodx,foody);
	}

	move_snake();
	
	for (int i = 2; i<snakesz; i++) {
		if (snakebx[0]==snakebx[i] && snakeby[0]==snakeby[i]) {
			init_game();
		}
	}

	handle_input();

}

void clear_pixel_buffer() {
	for (int i = 0; i<HEIGHT; i++) {
		PIXEL_BUFFER[i]=0;		
	}
}

void loop() {

	clear_pixel_buffer();
	update();
	draw_snake();
	draw_food();
	flip_pixel_buffer();
	time++;
	
}

void setup() {
	RCC->APB2ENR |= (1<<2); //enabling APB2 periphal clock 
	GPIOA->CRL = 0; //Initializing all port configurations as zero
	//setting ports A0,A1,A2 and A3 to output push-pull max speed 50MHz
	GPIOA->CRL |= (OPP_50MHZ<<PA0); 
	GPIOA->CRL |= (OPP_50MHZ<<PA1);
	GPIOA->CRL |= (OPP_50MHZ<<PA2);
	GPIOA->CRL |= (OPP_50MHZ<<PA3);
	GPIOA->CRL |= (IPP<<PA4);
	GPIOA->CRL |= (IPP<<PA5);
	GPIOA->CRL |= (IPP<<PA6);
	GPIOA->CRL |= (IPP<<PA7);
	set(CLR,HIGH); //set clear pin to high because it is active low
	RCC->APB1ENR |= (1<<2);
	TIM4->CR1 = 1;
	TIM4->PSC = 65535;
	TIM4->ARR = 65535;
	time=0;
	counter=0;
	init_game();
}


int main(void)	{
	
	/*
	PIXEL_BUFFER[0]=0b00111100;
	PIXEL_BUFFER[1]=0b01000010;
	PIXEL_BUFFER[2]=0b10000001;
	PIXEL_BUFFER[3]=0b10000001;
	PIXEL_BUFFER[4]=0b10100101;
	PIXEL_BUFFER[5]=0b10011001;
	PIXEL_BUFFER[6]=0b01000010;
	PIXEL_BUFFER[7]=0b00111100;
	*/
	
	setup();
	while (1) {
		loop();
	}
}
