#include <asm/types.h>

extern int gpio_tristate(unsigned long device, unsigned long mask, unsigned long data);
extern int gpio_open_drain(unsigned long device, unsigned long mask, unsigned long data);
extern int gpio_in(unsigned long device, unsigned long mask, volatile unsigned long *data);
extern int gpio_out(unsigned long device, unsigned long mask, unsigned long data);

