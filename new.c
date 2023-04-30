#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Global variables */
volatile int pixel_buffer_start;
volatile bool startGame = false;
volatile int *HEX_ptr = (int *)0xFF200020;
bool gameOver = false;
int penguinX = 130; // Starting X position of penguin
int points = 0;
int random_counter = 0;
/*Function Declarations*/
void clear_screen();
void plot_pixel(int x, int y, short int colour);
void wait_for_vsync();
void draw_anvil(int x, int y);
void draw_fish(int x, int y);
void draw_box(int x, int y, int width, int height, short int color);
void background();
void draw_penguin(int x, int y);
void write_char(int x, int y, char c);
void clear_text();
void erase_penguin();
void check_loss(int aX, int aY);
void displayScore();
bool inc_point(int fX, int fY);

/*INTERRUPT FUNCIONS*/
void disable_A9_interrupts(void);
void set_A9_IRQ_stack(void);
void config_GIC(void);
void config_KEYs(void);
void enable_A9_interrupts(void);

struct Anvil
{
	int anvilx;
	int anvily;
	int prevAnvily;
};
struct Anvil array_of_anvil[4];

struct Fish
{
	int fishx;
	int fishy;
	int prevFishy;
};
struct Fish array_of_fish[4];

int main(void)
{
	/* Interrupts intializer */
	disable_A9_interrupts(); // disable interrupts in the A9 processor
	set_A9_IRQ_stack();		 // initialize the stack pointer for IRQ mode
	config_GIC();			 // configure the general interrupt controller
	config_KEYs();			 // configure pushbutton KEYs to generate interrupts
	enable_A9_interrupts();	 // enable interrupts in the A9 processor

	volatile int *pixel_ctrl_ptr = (int *)0xFF203020; // Pointer to DMA controller's front buffer register

	/* Set front pixel buffer to start of FPGA On-chip memory */
	*(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the Back Buffer register of DMA
	wait_for_vsync();					// now, swap the front/back buffers, to set the front buffer location

	/* Initialize a pointer to the pixel buffer, used by drawing functions (plot_pixel)*/
	pixel_buffer_start = *pixel_ctrl_ptr; // pixel_buffer_start points to the memory in front buffer

	background(); // Draw the background of Game

	/* set back pixel buffer to start of SDRAM memory */
	*(pixel_ctrl_ptr + 1) = 0xC0000000;
	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // Will draw on the back buffer from here on

	/*Display Start Screen Messages*/
	clear_text(); // Ensure character buffer is clear
	char *text = "The Penguin Game";
	int xLocation = 32;
	int yLocation = 28;
	while (*text)
	{
		write_char(xLocation, yLocation, *text);
		xLocation++;
		text++;
	}
	text = "Press Key 0 to Start";
	xLocation = 30;
	yLocation = 30;

	while (*text)
	{
		write_char(xLocation, yLocation, *text);
		xLocation++;
		text++;
	}
	text = "Press Key 3 for Instructions";
	xLocation = 26;
	yLocation = 32;

	while (*text)
	{
		write_char(xLocation, yLocation, *text);
		xLocation++;
		text++;
	}

	/*Wait until player clicks KEY0 to start the game */
	while (startGame == false)
	{
		;
	}
	/* Clear the character buffer */
	clear_text();

	// initializing locations of fish and anvil to start the game of with

	int displacement = 0;
	int diff_start_height = 16;
	int i;
	for (i = 0; i < 4; ++i)
	{
		array_of_anvil[i].anvilx = displacement;
		array_of_anvil[i].anvily = diff_start_height;
		array_of_anvil[i].prevAnvily = 0;
		displacement = displacement + 90;
		diff_start_height += 15;
		if (diff_start_height == 31)
		{
			diff_start_height = 0;
		}
	}

	int x = 0;
	int fish_height = 16;
	for (i = 0; i < 4; ++i)
	{
		array_of_fish[i].fishx = x;
		array_of_fish[i].fishy = fish_height;
		array_of_fish[i].prevFishy = 0;
		x = x + 90;
		fish_height += 50;
		if (fish_height == 46)
		{
			fish_height = 0;
		}
	}

	bool gameWon = false;
	/*Game loop */
	while (!gameOver)
	{

		background(); // Redraw background

		erase_penguin();
		// used to randomize the locations of fish and anvil
		int array_randomizer[] = {0, 50, 90, 140, 180, 230, 270};

		int indexOne = rand() % 8;
		int indexTwo = rand() % 8;

		while (indexTwo == indexOne)
		{
			indexTwo = rand() % 8;
		}

		int indexThree = rand() % 8;

		while (indexThree == indexOne || indexThree == indexTwo)
		{
			indexThree = rand() % 5;
		}

		int indexFour = rand() % 8;

		while (indexFour == indexOne || indexFour == indexTwo || indexFour == indexThree)
		{
			indexFour = rand() % 8;
		}

		int indexFive = rand() % 8;

		while (indexFive == indexOne || indexFive == indexTwo || indexFive == indexThree || indexFive == indexFour)
		{
			indexFive = rand() % 8;
		}

		int indexSix = rand() % 8;

		while (indexSix == indexOne || indexSix == indexTwo || indexSix == indexThree || indexSix == indexFour || indexSix == indexFive)
		{
			indexSix = rand() % 8;
		}

		int indexSeven = rand() % 8;

		while (indexSeven == indexOne || indexSeven == indexTwo || indexSeven == indexThree || indexSeven == indexFour || indexSeven == indexFive || indexSeven == indexSix)
		{
			indexSeven = rand() % 8;
		}
		
        int i;
		for (i = 0; i < 4; ++i)
		{

			if (array_of_anvil[i].anvily < 210)
			{
				draw_anvil(array_of_anvil[i].anvilx, array_of_anvil[i].anvily); // Draw anvil
				check_loss(array_of_anvil[i].anvilx, array_of_anvil[i].anvily);
				array_of_anvil[i].prevAnvily = array_of_anvil[i].anvily;
				int rand_speed = 18;
				array_of_anvil[i].anvily = array_of_anvil[i].anvily + rand_speed;
			}
			else
			{
				array_of_anvil[i].anvily = 16;
				array_of_anvil[i].anvilx = array_randomizer[indexFour];
			}
		}

		for (i = 0; i < 4; ++i)
		{

			if (array_of_fish[i].fishy < 210)
			{
				bool draw = inc_point(array_of_fish[i].fishx, array_of_fish[i].fishy);
				if (draw == false)
				{
					draw_fish(array_of_fish[i].fishx, array_of_fish[i].fishy); // Draw fish
					array_of_fish[i].prevFishy = array_of_fish[i].fishy;
					int random_speed = 9;
					array_of_fish[i].fishy = array_of_fish[i].fishy + random_speed;
				}
				else
				{
					array_of_fish[i].fishy = 16;
					array_of_fish[i].fishx = array_randomizer[indexTwo];
				}
			}
			else
			{
				array_of_fish[i].fishy = 16;
				array_of_fish[i].fishx = array_randomizer[indexSeven];
			}
		}

		draw_penguin(penguinX, 173); // Draw penguin

		wait_for_vsync();							// swap front and back buffers on VGA vertical sync
		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
		displayScore();
		if (points == 10)
		{
			gameWon = true;
			gameOver = true;
		}
	}

	if (gameWon == false)
	{
		// Display Game Over text once player loses
		text = "GAME OVER!";
		xLocation = 35;
		yLocation = 32;
		while (*text)
		{
			write_char(xLocation, yLocation, *text);
			xLocation++;
			text++;
		}
	}
	else
	{
		// Display Game Won text once player loses
		text = "YOU WIN!";
		xLocation = 35;
		yLocation = 32;
		while (*text)
		{
			write_char(xLocation, yLocation, *text);
			xLocation++;
			text++;
		}
	}
	return 0;
}

// Check for loss
void check_loss(int aX, int aY)
{
	if (aY > 160)
	{
	    int i;
	    int j;
		for (i = aX + 10; i < aX + 10 + 10; ++i)
			for (j = penguinX; j < penguinX + 51; ++j)
				if (i == j)
				{
					gameOver = true;
				}
	}
}

// Check for points
bool inc_point(int fX, int fY)
{
    int i,j;
	if (fY > 160)
	{
		for (i = fX; i < fX + 31; ++i)
		{
			for (j = penguinX; j < penguinX + 51; ++j)
			{
				if (i == j)
				{
					points++;
					return true;
				}
			}
		}
	}
	return false;
}

void displayScore()
{
	char array_of_hex_bits[10] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01100111};
	int firstDigit = points % 10;
	int secondDigit = (points / 10) % 10;
	int thirdDigit = (points / 100) % 10;
	*HEX_ptr = array_of_hex_bits[thirdDigit] << 16 | array_of_hex_bits[secondDigit] << 8 | array_of_hex_bits[firstDigit];
}

// Draw the background, turquoise sky and white ground
void background()
{
    int color1 = 0x090107; // dark blue color
    int color2 = 0xFFFF;   // white color

    int i = 0;
    while (i < 320 * 200)
    {
        plot_pixel(i % 320, i / 320, color1);
        i++;
    }

    while (i < 320 * 240)
    {
        plot_pixel(i % 320, i / 320, color2);
        i++;
    }
}




// Draw an anvil at location (x,y)
void draw_anvil(int x, int y)
{
	short int color = 0x5AAA;
	
    int j;
    
	for (j = x; j <= x + 30; ++j)
	{
		plot_pixel(j, y, color);
	}
	for (j = x + 1; j <= x + 1 + 28; ++j)
	{
		plot_pixel(j, y + 1, color);
	}
	for (j = x + 2; j <= x + 2 + 26; ++j)
	{
		plot_pixel(j, y + 2, color);
	}
	for (j = x + 3; j <= x + 3 + 24; ++j)
	{
		plot_pixel(j, y + 3, color);
	}
	for (j = x + 4; j <= x + 4 + 22; ++j)
	{
		plot_pixel(j, y + 4, color);
	}
	for (j = x + 5; j <= x + 5 + 20; ++j)
	{
		plot_pixel(j, y + 5, color);
	}
	for (j = x + 6; j <= x + 6 + 18; ++j)
	{
		plot_pixel(j, y + 6, color);
	}
	for (j = x + 7; j <= x + 7 + 16; ++j)
	{
		plot_pixel(j, y + 7, color);
	}
	for (j = x + 10; j <= x + 10 + 10; ++j)
	{
		plot_pixel(j, y + 8, color);
	}
	for (j = x + 10; j <= x + 10 + 10; ++j)
	{
		plot_pixel(j, y + 9, color);
	}
	for (j = x + 10; j <= x + 10 + 10; ++j)
	{
		plot_pixel(j, y + 10, color);
	}
	for (j = x + 10; j <= x + 10 + 10; ++j)
	{
		plot_pixel(j, y + 11, color);
	}
	for (j = x + 10; j <= x + 10 + 10; ++j)
	{
		plot_pixel(j, y + 12, color);
	}
	for (j = x + 10; j <= x + 10 + 10; ++j)
	{
		plot_pixel(j, y + 13, color);
	}
	for (j = x + 10; j <= x + 10 + 10; ++j)
	{
		plot_pixel(j, y + 14, color);
	}
}

void draw_fish(int x, int y)
{

    int j;
    draw_box(x, y, 20, 10, 0);					  // outside black outline
    draw_box(x + 1, y + 1, 20 - 2, 10 - 2, 0xf7dc); // inside light brown rectangle
    draw_box(x + 10, y - 2, 10, 10, 0);			  // tail black outline
    draw_box(x + 11, y - 1, 8, 8, 0xff00);		  // tail interior
    draw_box(x + 1, y + 2, 5, 3, 0);				  // left wing outline
    draw_box(x + 2, y + 3, 5 - 2, 3 - 2, 0xf7dc);   // left wing interior
    draw_box(x + 15, y + 2, 5, 3, 0);				  // right wing outline
    draw_box(x + 16, y + 3, 5 - 2, 3 - 2, 0xf7dc);  // right wing interior
    for (j = x + 7; j <= x + 12; ++j)
    { // beak
        plot_pixel(j, y + 7, 0xffa500);
    }
    for (j = x + 4; j <= x + 6; ++j)
    { // eye
        draw_box(j, y + 4, 1, 1, 0);
        draw_box(j, y + 6, 1, 1, 0);
    }
    for (j = x + 14; j <= x + 16; ++j)
    { // feet
        draw_box(j, y + 9, 1, 2, 0);
        draw_box(j, y + 11, 1, 2, 0);
    }


}

// Draw the penguin
void draw_penguin(int x, int y)
{

    draw_box(x + 5, y + 5, 10, 5, 0);    // ear one
    draw_box(x + 35, y + 5, 10, 5, 0);   // ear two
    draw_box(x, y + 10, 50, 40, 0xAAAA); // head
    draw_box(x + 10, y + 35, 30, 20, 0xAAAA); // body
    draw_box(x + 5, y + 15, 10, 10, 0xFFFF); // eye one
    draw_box(x + 35, y + 15, 10, 10, 0xFFFF); // eye two
    draw_box(x + 7, y + 17, 6, 6, 0); // eye one pupil
    draw_box(x + 37, y + 17, 6, 6, 0); // eye two pupil
    draw_box(x + 10, y + 40, 10, 10, 0xAAAA); // leg one
    draw_box(x + 30, y + 40, 10, 10, 0xAAAA); // leg two


}

void erase_penguin()
{
    int i,j;
	for (i = 172; i < 240; ++i)
	{
		for (j = 0; j < 320; ++j)
		{
			if (i < 200)
			{
				plot_pixel(j, i, 0x4454);
			}
			else
			{
				plot_pixel(j, i, 0xffff);
			}
		}
	}
}

// Draw a box at (x,y) with width, and height
void draw_box(int x, int y, int width, int height, short int color)
{
    int i,j;
	for (i = y; i <= y + height; ++i)
	{
		for (j = x; j <= x + width; ++j)
		{
			plot_pixel(j, i, color);
		}
	}
}

// Drawing black everywhere on screen
void clear_screen()
{
    int x,y;
	for (x = 0; x < 320; ++x)
	{
		for (y = 0; y < 240; ++y)
		{
			plot_pixel(x, y, 0x0000);
		}
	}
}

// Plot a pixel at location (x, y) with the colour line_color
void plot_pixel(int x, int y, short int colour)
{
	*(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = colour;
}

// Synchronizing with VGA timer and swap between front and back buffers
void wait_for_vsync()
{
	volatile int *pixel_ctrl_ptr = (int *)0xFF203020; // pixel controller
	register int status;
	*pixel_ctrl_ptr = 1; // Start the synchronization process

	status = *(pixel_ctrl_ptr + 3);
	while ((status & 0x01) != 0)
	{
		status = *(pixel_ctrl_ptr + 3);
	}
}

// Clear the character buffer
void clear_text()
{
    int x,y;
	for (x = 0; x < 80; ++x)
	{
		for (y = 0; y < 60; ++y)
		{
			char *space = " ";
			write_char(x, y, *space);
		}
	}
}

// Write a character into the character buffer
void write_char(int x, int y, char c)
{
	volatile char *characterBuffer = (char *)(0xC9000000 + (y << 7) + x);
	*characterBuffer = c;
}

////INTERRUPTS////
/*This code is from the ARM document*/
/* setup the KEY interrupts in the FPGA */
void config_KEYs()
{
	volatile int *KEY_ptr = (int *)0xFF200050; // pushbutton KEY base address
	*(KEY_ptr + 2) = 0xF;					   // enable interrupts for the two KEYs
}
/* This file:
 * 1. defines exception vectors for the A9 processor
 * 2. provides code that sets the IRQ mode stack, and that dis/enables
 * interrupts
 * 3. provides code that initializes the generic interrupt controller
 */

void pushbutton_ISR(void);
void config_interrupt(int, int);

void __attribute__((interrupt)) __cs3_isr_irq(void)
{
	// Read the ICCIAR from the CPU Interface in the GIC
	int interrupt_ID = *((int *)0xFFFEC10C);
	if (interrupt_ID == 73) // check if interrupt is from the KEYs
		pushbutton_ISR();
	else
		while (1)
			; // if unexpected, then stay here
	// Write to the End of Interrupt Register (ICCEOIR)
	*((int *)0xFFFEC110) = interrupt_ID;
}

void __attribute__((interrupt)) __cs3_reset(void)
{
	while (1)
		;
}
void __attribute__((interrupt)) __cs3_isr_undef(void)
{
	while (1)
		;
}
void __attribute__((interrupt)) __cs3_isr_swi(void)
{
	while (1)
		;
}
void __attribute__((interrupt)) __cs3_isr_pabort(void)
{
	while (1)
		;
}
void __attribute__((interrupt)) __cs3_isr_dabort(void)
{
	while (1)
		;
}
void __attribute__((interrupt)) __cs3_isr_fiq(void)
{
	while (1)
		;
}

/*
 * Turn off interrupts in the ARM processor
 */
void disable_A9_interrupts(void)
{
	int status = 0b11010011;
	asm("msr cpsr, %[ps]"
		:
		: [ps] "r"(status));
}

/*
 * Initialize the banked stack pointer register for IRQ mode
 */
void set_A9_IRQ_stack(void)
{
	int stack, mode;
	stack = 0xFFFFFFFF - 7; // top of A9 onchip memory, aligned to 8 bytes
	/* change processor to IRQ mode with interrupts disabled */
	mode = 0b11010010;
	asm("msr cpsr, %[ps]"
		:
		: [ps] "r"(mode));
	/* set banked stack pointer */
	asm("mov sp, %[ps]"
		:
		: [ps] "r"(stack));
	/* go back to SVC mode before executing subroutine return! */
	mode = 0b11010011;
	asm("msr cpsr, %[ps]"
		:
		: [ps] "r"(mode));
}

/*
 * Turn on interrupts in the ARM processor
 */
void enable_A9_interrupts(void)
{
	int status = 0b01010011;
	asm("msr cpsr, %[ps]"
		:
		: [ps] "r"(status));
}

/*
 * Configure the Generic Interrupt Controller (GIC)
 */
void config_GIC(void)
{
	config_interrupt(73, 1); // configure the FPGA KEYs interrupt (73)
	// Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all
	// priorities
	*((int *)0xFFFEC104) = 0xFFFF;
	// Set CPU Interface Control Register (ICCICR). Enable signaling of
	// interrupts
	*((int *)0xFFFEC100) = 1;
	// Configure the Distributor Control Register (ICDDCR) to send pending
	// interrupts to CPUs
	*((int *)0xFFFED000) = 1;
}

/*
 * Configure Set Enable Registers (ICDISERn) and Interrupt Processor Target
 * Registers (ICDIPTRn). The default (reset) values are used for other registers
 * in the GIC.
 */
void config_interrupt(int N, int CPU_target)
{
	int reg_offset, index, value, address;
	/* Configure the Interrupt Set-Enable Registers (ICDISERn).
	 * reg_offset = (integer_div(N / 32) * 4
	 * value = 1 << (N mod 32) */
	reg_offset = (N >> 3) & 0xFFFFFFFC;
	index = N & 0x1F;
	value = 0x1 << index;
	address = 0xFFFED100 + reg_offset;
	/* Now that we know the register address and value, set the appropriate bit */
	*(int *)address |= value;
	/* Configure the Interrupt Processor Targets Register (ICDIPTRn)
	 * reg_offset = integer_div(N / 4) * 4
	 * index = N mod 4 */
	reg_offset = (N & 0xFFFFFFFC);
	index = N & 0x3;
	address = 0xFFFED800 + reg_offset + index;
	/* Now that we know the register address and value, write to (only) the
	 * appropriate byte */
	*(char *)address = (char)CPU_target;
}

/*Pushbutton - Interrupt Service Routine
 *
 * This routine checks which KEY has been pressed. It writes to HEX0
 *******************************************************************/
void pushbutton_ISR(void)
{
	/* KEY base address */
	volatile int *KEY_ptr = (int *)0xFF200050;
	int press;
	press = *(KEY_ptr + 3); // read the pushbutton interrupt register
	*(KEY_ptr + 3) = press; // Clear the interrupt

	if (press & 0x1)
	{ // KEY0
		if (startGame == false)
		{
			startGame = true;
		}
		else if (startGame == true)
		{
			; // do nothing
		}
	}
	else if (press & 0x2)
	{ // KEY1

		penguinX = penguinX + 10; // Move penguin to the right

		if (penguinX >= 260)
		{ // Keep penguin from going out of right bound
			penguinX = 260;
		}
	}
	else if (press & 0x4)
	{							  // KEY2
		penguinX = penguinX - 10; // Move penguin to the left
		if (penguinX <= 0)
		{ // Keep penguin from going out of left bound
			penguinX = 0;
		}
	}
	else if (press & 0x8)
	{ // which is KEY3
		if (startGame == true)
		{
			gameOver = true; // Let player quit
		}
		else if (startGame == false)
		{
			clear_text();
			char *text = "Instructions:";
			int xLocation = 32;
			int yLocation = 26;
			while (*text)
			{
				write_char(xLocation, yLocation, *text);
				xLocation++;
				text++;
			}
			text = "Press KEY 1 to move Player Right";
			xLocation = 24;
			yLocation = 28;
			while (*text)
			{
				write_char(xLocation, yLocation, *text);
				xLocation++;
				text++;
			}
			text = "Press KEY 2 to move Player Left";
			xLocation = 24;
			yLocation = 30;
			while (*text)
			{
				write_char(xLocation, yLocation, *text);
				xLocation++;
				text++;
			}
			text = "Press KEY 3 to Quit Game in Game";
			xLocation = 24;
			yLocation = 32;
			while (*text)
			{
				write_char(xLocation, yLocation, *text);
				xLocation++;
				text++;
			}

			text = "Press KEY 0 to Start Game";
			xLocation = 27;
			yLocation = 40;
			while (*text)
			{
				write_char(xLocation, yLocation, *text);
				xLocation++;
				text++;
			}
		}
	}

	return;
}