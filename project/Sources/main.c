/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"
#include "core_cm4.h"

#include "common.h"
#include "snake.h"

/* Macros for bit-level registers manipulation */
#define GPIO_PIN_MASK	0x1Fu
#define GPIO_PIN(x)		(((1)<<(x & GPIO_PIN_MASK)))


#define R0 0x04000000
#define R1 0x01000000
#define R2 0x00000200
#define R3 0x02000000
#define R4 0x10000000
#define R5 0x00000080
#define R6 0x08000000
#define R7 0x20000000

#define A0 0x00000100
#define A1 0x00000400
#define A2 0x00000040
#define A3 0x00000800

/* Constants specifying delay loop duration */
#define	tdelay1			20000
#define tdelay2 		20
#define tdelay3 		60

// snake length
#define LENGTH 5

#define INTENZITY delay(1,10)

point_t point;
snake_part_t parts[LENGTH];
snake_t snake;

// shows if game is on
int game = 0;

// if direction already changed
int directionChanged = 0;

/* Configuration of the necessary MCU peripherals */
void SystemConfig() {

	/* Turn on all port clocks */
	SIM->SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK;


	/* Set corresponding PTA pins (column activators of 74HC154) for GPIO functionality */
	PORTA->PCR[8] = ( 0|PORT_PCR_MUX(0x01) );  // A0
	PORTA->PCR[10] = ( 0|PORT_PCR_MUX(0x01) ); // A1
	PORTA->PCR[6] = ( 0|PORT_PCR_MUX(0x01) );  // A2
	PORTA->PCR[11] = ( 0|PORT_PCR_MUX(0x01) ); // A3

	/* Set corresponding PTA pins (rows selectors of 74HC154) for GPIO functionality */
	PORTA->PCR[26] = ( 0|PORT_PCR_MUX(0x01) );  // R0
	PORTA->PCR[24] = ( 0|PORT_PCR_MUX(0x01) );  // R1
	PORTA->PCR[9] = ( 0|PORT_PCR_MUX(0x01) );   // R2
	PORTA->PCR[25] = ( 0|PORT_PCR_MUX(0x01) );  // R3
	PORTA->PCR[28] = ( 0|PORT_PCR_MUX(0x01) );  // R4
	PORTA->PCR[7] = ( 0|PORT_PCR_MUX(0x01) );   // R5
	PORTA->PCR[27] = ( 0|PORT_PCR_MUX(0x01) );  // R6
	PORTA->PCR[29] = ( 0|PORT_PCR_MUX(0x01) );  // R7

	/* Set corresponding PTE pins (output enable of 74HC154) for GPIO functionality */
	PORTE->PCR[28] = ( 0|PORT_PCR_MUX(0x01) ); // #EN

	// Configure the Signal Multiplexer for SW6, configure SW6 to interrupt on falling edge
	PORTE->PCR[10] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010); // SW6
	PORTE->PCR[12] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010);
	PORTE->PCR[26] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010);
	PORTE->PCR[27] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010);

	// Clear the Interrupt flags in Port E
	PORTE->ISFR = 0xFFFFFFFF;

	// Enable the Interrupt in the NVIC
	NVIC->ISER[2] = (1 << 27);

	/* Change corresponding PTA port pins as outputs */
	PTA->PDDR = GPIO_PDDR_PDD(0x3F000FC0);

	// ---------- Timer ----------
	// Taken code from demo IMP

	// Select External Reference Clock in MCG (Transition to FBE from FEI)
	// C1[CLKS] written to 10
	MCG->C1 = MCG_C1_CLKS(2);

	// Wait for Reference clock to switch to external reference
	while (MCG->S & MCG_S_IREFST_MASK);

	// Wait for MCGOUT to switch over to the external reference clock
	while (((MCG->S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2);

	// Transition to bypassed low power external mode (BLPE)
	// C2[LP] written to 1
	MCG->C2 = MCG_C2_LP_MASK;

	// Clear the Interrupt pending flag in the NVIC just in case
	NVIC->ICPR[2] = (1 << 27);

	// Enable the clock on the PIT
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;

	// Turn on the PIT
	PIT->MCR = 0x00;

	// Load Timer Value into Channel 0
	// LDVAL trigger = (period/clock period) - 1
	// LDVAL trigger = (0.5 / (1 / 50,000,000)) - 1
	PIT->CHANNEL[0].LDVAL = 24999999;

	// Enable Interrupt and Start Timer
	PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

	// Enable PIT0 Interrupt Channel in NVIC
	NVIC->ISER[2] |= (1 << 4);

	// ---------- Timer ----------
}

/*
 * Port E ISR, buttons interrupt
 */
void PORTE_IRQHandler()
{
	if (PORTE->ISFR & 0x1000)	{
		snake_change_direction (&snake, RIGHT);
	} else if (PORTE->ISFR & 0x400) {
		snake_change_direction (&snake, UP);
	} else if (PORTE->ISFR & 0x4000000) {
		snake_change_direction (&snake, LEFT);
	} else if (PORTE->ISFR & 0x8000000) {
		snake_change_direction (&snake, DOWN);
	}

	// Clear all Port E interrupt flag
	PORTE->ISFR = PORT_ISFR_ISF_MASK;
}

/* Conversion of requested column number into the 4-to-16 decoder control.  */
void column_select(unsigned int col_num)
{
	unsigned i, result, col_sel[4];

	for (i =0; i<4; i++) {
		result = col_num / 2;	  // Whole-number division of the input number
		col_sel[i] = col_num % 2;
		col_num = result;

		switch(i) {

			// Selection signal A0
		    case 0:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(8))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(8)));
				break;

			// Selection signal A1
			case 1:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(10))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(10)));
				break;

			// Selection signal A2
			case 2:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(6))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(6)));
				break;

			// Selection signal A3
			case 3:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(11))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(11)));
				break;

			// Otherwise nothing to do...
			default:
				break;
		}
	}
}

// prints point on display
void print_point (point_t p) {
	column_select(p.x);

	switch(p.y) {
		case 0:
			PTA->PDOR |= GPIO_PDOR_PDO(R0);
			INTENZITY;
			PTA->PDOR &= ~GPIO_PDOR_PDO(R0);
			break;

		case 1:
			PTA->PDOR |= GPIO_PDOR_PDO(R1);
			INTENZITY;
			PTA->PDOR &= ~GPIO_PDOR_PDO(R1);
			break;

		case 2:
			PTA->PDOR |= GPIO_PDOR_PDO(R2);
			INTENZITY;
			PTA->PDOR &= ~GPIO_PDOR_PDO(R2);
			break;

		case 3:
			PTA->PDOR |= GPIO_PDOR_PDO(R3);
			INTENZITY;
			PTA->PDOR &= ~GPIO_PDOR_PDO(R3);
			break;

		case 4:
			PTA->PDOR |= GPIO_PDOR_PDO(R4);
			INTENZITY;
			PTA->PDOR &= ~GPIO_PDOR_PDO(R4);
			break;

		case 5:
			PTA->PDOR |= GPIO_PDOR_PDO(R5);
			INTENZITY;
			PTA->PDOR &= ~GPIO_PDOR_PDO(R5);
			break;

		case 6:
			PTA->PDOR |= GPIO_PDOR_PDO(R6);
			INTENZITY;
			PTA->PDOR &= ~GPIO_PDOR_PDO(R6);
			break;

		case 7:
			PTA->PDOR |= GPIO_PDOR_PDO(R7);
			INTENZITY;
			PTA->PDOR &= ~GPIO_PDOR_PDO(R7);
			break;

		// Otherwise nothing to do...
		default:
			break;
	}
}

void animate_transition () {
	// all rows
	PTA->PDOR |= GPIO_PDOR_PDO(0x3F000280);

	for (int i=0; i<16; i++) {
		column_select(i);
		PTE->PDDR &= ~GPIO_PDDR_PDD( GPIO_PIN(28) );
		delay(tdelay1, tdelay2);
		PTE->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(28));
	}

	PTA->PDOR &= GPIO_PDOR_PDO(0x00000000);

	// delay after animation
	delay(tdelay1, tdelay3);

}

void animate_whole_display () {
	// all rows
	PTA->PDOR |= GPIO_PDOR_PDO(0x3F000280);

	for (int j = 0; j < 4000; j++) {
		for (int i=0; i<16; i++) {
			column_select(i);
			PTE->PDDR &= ~GPIO_PDDR_PDD( GPIO_PIN(28) );
			delay(1,50);
			PTE->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(28));
		}
	}

	PTA->PDOR &= GPIO_PDOR_PDO(0x00000000);

	// delay after animation
	delay(tdelay1, tdelay3);
}

void game_init ()
{
	snake_ctor (&snake);
	snake_parts_init (parts, LENGTH);
	snake_insert_parts (&snake, parts, LENGTH);
}

void game_restart ()
{
	animate_transition();

	game_init();
}

void PIT0_IRQHandler()
{
	// Clear the Interrupt Flag in the PIT module
	PIT->CHANNEL[0].TFLG = 0x01;

	if (game && snake_move(&snake, LENGTH)) {
		// restart game after collision
		game = 0;

		delay(tdelay1, tdelay3);

		game_restart();

		game = 1;
	}
}

int main(void)
{
	SystemConfig();

	game_init();

	animate_whole_display();

	delay(tdelay1, tdelay3);

	// game start
	game = 1;

    while (1) {
		if (game) {
			snake_get_point (&snake, &point);
			print_point(point);
		}
    }

    /* Never leave main */
    return 0;
}
