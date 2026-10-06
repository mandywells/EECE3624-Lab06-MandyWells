/*
 * Lab06.c
 *
 * Created: 10/4/2022 9:16:57 AM
 * Initial Coder: jfhutton
 * Current Coder: Mandy Wells
 * Modified:      10/6/2026
 *
 * This lab uses hardware LEDs wired to PORTA, an interrupt from the joystick center
 * button, and a timer interrupt to build a game loop program that can display 
 * different patterns on the LEDs.
 *
 * While there are many "down and dirty" ways to get this coding done, try to 
 * remember your coding and data structures classes.  Things like ENUM, Arrays, Functions
 * could help make for more elegant coding.
 *
 */ 

#include <avr/io.h>              // Needed for AVR IO defines
#include <avr/interrupt.h>       // Needed for AVR interrupt devices

#define LEDS            PORTA    // alias PORTA

	
// global variables for communication between ISRs and main
const unsigned char TCNT0_COUNT_SET = 0x8E;// Count for 1ms loop (Provided by Prof Hutton)
// <TBD> Your global variables should go here
enum patterns{
	LowToHigh,		//A0 to A7
	HighToLow,		//A7 to A0
	BackAndForth,	//A0 to A7 to A0
	LowToHighEO,		//A0, A2, A4, A6 (every other LED)
	BackAndForthHalf //A0 to A3 to A0
};

//global variable to keep track of current LED pattern
volatile enum patterns Mode;
volatile uint16_t Tick;

int main(void){
	// Variables for main
	// <TBD> Your local main variables should go here
	uint16_t CurrentLED;
	uint16_t PrevLED;
	// State machine initialization.
	Tick = 0;
	Mode = LowToHigh;
	
	// Initialization for LEDs
	// Set direction for A ports.
	// (Prof Note:  This is similar to our two line assembly commands.)
	
	DDRA = 0xFF;  // Set the Direction for all PORTA pins to be outputs
	LEDS = 0xFF;  // Set the PORTA for all pins to be high (i.e. OFF)
	//LEDS = 0x00;  // TEST - Set all PORTA pins to be low (i.e. ON)
	
	// Initialization for Timer Interrupt
	// With a 7.3MHz crystal we have a 0.137us period
	// To build a 1ms timer "tick" we
	// - set the pre-scaler to 1/64 (8.77us)
	// - set the TCNT1 to count 114 counts (0-114=0x8E)
	// - enable the interrupt on overflow
	// (Prof Note: these are the commands you need, BUT they are commented
	//  out to start.  Be sure you fully understand them (book, datasheet, etc)
	//  before you enable them...  You also have to have the proper ISR routine
	//  ready for this to work.)
	// (Prof Note Two:  You will need to show your own calculations in the 
	//  Lab Report to verify the TCNT0_COUNT_SET value!)
	
	TCCR0 = (1<<CS02);
	TCNT0 = TCNT0_COUNT_SET;
	TIMSK = (1<<TOIE0);
	
	// Port Initialization
	// Using the LED PORTA initialization above and your Lab05 code, 
	// Configure the joystick button.
	// (Prof Note: you will need to add another jumper wire, or 
	// change the ones you have, to get the center button working!
	
	// <TBD>  Student code here
	DDRB &= ~(1 << DDB0);
	PORTB |= (1 << DDB0);
	DDRD &= ~(1 << INT0);
	PORTD |= (1 << INT0);
	
	// Interrupt Enable Block
	// Using your Lab05 code, you will need to update these
	// lines from 0x00 to have the appropriate mask.
	EICRA = (1 << ISC01) | (1<< ISC00); 
	EIMSK = (1 << INT0);
	
	// Enable Global Interrupts
	sei();
	
	CurrentLED = 0;
	LEDS = ~(1 << CurrentLED);
	PrevLED = 1;
	
	// Main Loop
	while (1) {
		// Your game loop will go here.

		if(Tick >= 50){
			Tick = 0;
			
			switch(Mode){
				case(HighToLow):
					//turn off current LED, turn on next LED in the pattern
					if(CurrentLED == 0){
						CurrentLED = 8;
					}
					CurrentLED -= 1;

					LEDS = ~(1 << CurrentLED);
					break;
				case(LowToHigh):
					CurrentLED += 1;
					if(CurrentLED == 8){
						CurrentLED = 0;
					}
				
					LEDS = ~(1 << CurrentLED);
					break;
				case(BackAndForth):
					if(CurrentLED == 0){
						PrevLED = CurrentLED;
						CurrentLED = 1;
					}
					else if(CurrentLED == 7){
						PrevLED = CurrentLED;
						CurrentLED = 6;
					}
					else{
						if(PrevLED < CurrentLED){
							PrevLED = CurrentLED;
							CurrentLED += 1;
						}
						else{
							PrevLED = CurrentLED;
							CurrentLED -= 1;	
						}
					}
				
					LEDS = ~(1 << CurrentLED);
					break;
				case(LowToHighEO):
					CurrentLED += 2;

					if(CurrentLED == 8){
						CurrentLED = 0;
					}

					LEDS = ~(1 << CurrentLED);
					break;
				case(BackAndForthHalf):
					if(CurrentLED == 0){
						PrevLED = CurrentLED;
						CurrentLED = 1;
					}
					else if(CurrentLED == 3){
						PrevLED = CurrentLED;
						CurrentLED = 2;
					}
					else{
						if(PrevLED < CurrentLED){
							PrevLED = CurrentLED;
							CurrentLED += 1;
						}
						else{
							PrevLED = CurrentLED;
							CurrentLED -= 1;
						}
					}
					
					LEDS = ~(1 << CurrentLED);
					break;
			}
		}
	}
}

// This is the required format of an ISR routine for a 
// Timer.  Your ISR code should go inside
// (Prof Note:  When you define this correctly, the JUMP TABLE
//  will be properly updated by the compiler.  (Thank you 
ISR(TIMER0_OVF_vect){
	// <TBD>  ISR code for timer interrupt.
	Tick = Tick + 1;
	TCNT0 = TCNT0_COUNT_SET; 
}

// This is the required format of an ISR routine for a
// Hardware Pin Interrupt.  Format is "PIN#_vect" where 
// '#' is the number of the interrupt...  PD0->0, PD1->1, etc
ISR(INT0_vect){
	// <TBD>  
	switch(Mode){
		case(LowToHigh):
			Mode = HighToLow;
			break;
		case(HighToLow):
			Mode = BackAndForth;
			break;
		case(BackAndForth):
			Mode = LowToHighEO;
			break;
		case(LowToHighEO):
			Mode = BackAndForthHalf;
			break;
		case(BackAndForthHalf):
			Mode = LowToHigh;
			break;
	}
}