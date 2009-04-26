
#define GPIO_BASE	0x40060000
#include <common.h>

#ifdef CONFIG_PPC405_GPIO

#include <asm/io.h>
#include "common/gpio.h"

typedef volatile unsigned long* ioptr;

int gpio_tristate(unsigned long device, unsigned long mask, unsigned long data)
{
	*(ioptr)GPIO0_TCR = (*(ioptr)GPIO0_TCR & ~mask) | (data & mask);

	return 0;
}


int gpio_open_drain(unsigned long device, unsigned long mask, unsigned long data)
{
	*(ioptr)GPIO0_ODR = (*(ioptr)GPIO0_ODR & ~mask) | (data & mask);

	return 0;
}


int gpio_in(unsigned long device, unsigned long mask, volatile unsigned long *data)
{
	*(ioptr)GPIO0_TCR = *(ioptr)GPIO0_TCR & ~mask;

	eieio();

	/*
	** If the previous state was OUT, and GPIO0_IR is read once, then the
	** data that was being OUTput will be read.  One way to get the right
	** data is to read GPIO0_IR twice.
	*/

	*data = *(ioptr)GPIO0_IR;
	*data = *(ioptr)GPIO0_IR & mask;

	return 0;
}


int gpio_out(unsigned long device, unsigned long mask, unsigned long data)
{
	*(ioptr)GPIO0_OR = (*(ioptr)GPIO0_OR & ~mask) | (data & mask);

	eieio();

	*(ioptr)GPIO0_TCR = *(ioptr)GPIO0_TCR | mask;

	return 0;
}

#endif /* CONFIG_PPC405_GPIO */
