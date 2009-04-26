#include <common.h>

#ifdef CONFIG_DGS_FRONT

#include <command.h>
#include <asm/processor.h>

#include "common/front.h"
#include "relook300s/front_dev.h"

#define UART0_BASE      0x40040000
#define UART1_BASE      0x40000000

#define FRONT_BASE	UART0_BASE

#define UART_RBR    0x00
#define UART_THR    0x00
#define UART_IER    0x01
#define UART_IIR    0x02
#define UART_FCR    0x02
#define UART_LCR    0x03
#define UART_MCR    0x04
#define UART_LSR    0x05
#define UART_MSR    0x06
#define UART_SCR    0x07
#define UART_DLL    0x00
#define UART_DLM    0x01

/*-----------------------------------------------------------------------------+
  | Line Status Register.
  +-----------------------------------------------------------------------------*/
#define asyncLSRDataReady1            0x01
#define asyncLSROverrunError1         0x02
#define asyncLSRParityError1          0x04
#define asyncLSRFramingError1         0x08
#define asyncLSRBreakInterrupt1       0x10
#define asyncLSRTxHoldEmpty1          0x20
#define asyncLSRTxShiftEmpty1         0x40
#define asyncLSRRxFifoError1          0x80

int front_init (void)
{
	unsigned long reg;
	unsigned short bdiv;
	volatile char val;
	unsigned long tmp;

	reg = mfdcr(cic_cr) & ~0x03000000;
	mtdcr(cic_cr, reg);		/* select internal serial clock */

#if 0
	reg = mfdcr(clkgpcr);
	reg &= 0x06fc0080;
	reg |= 0x00;
#endif

	tmp = 16*9600;
	bdiv = (324000000/16+tmp/2)/tmp;	// 324MHz / 16 = 20.25MHz...	// by parkhw00

	out8 (FRONT_BASE + UART_LCR, 0x80);	/* set DLAB bit */
	out8 (FRONT_BASE + UART_DLL, bdiv);	/* set baudrate divisor */
	out8 (FRONT_BASE + UART_DLM, bdiv >> 8);/* set baudrate divisor */
	out8 (FRONT_BASE + UART_LCR, 0x03);	/* clear DLAB; set 8 bits, no parity */
	out8 (FRONT_BASE + UART_FCR, 0x00);	/* disable FIFO */
	out8 (FRONT_BASE + UART_MCR, 0x00);	/* no modem control DTR RTS */
	val = in8 (FRONT_BASE + UART_LSR);	/* clear line status */
	val = in8 (FRONT_BASE + UART_RBR);	/* read receive buffer */
	out8 (FRONT_BASE + UART_SCR, 0x00);	/* set scratchpad */
	out8 (FRONT_BASE + UART_IER, 0x00);	/* set interrupt enable reg */

	return (0);
}

void front_putc (const char c)
{
	int i;

	/* check THRE bit, wait for transmiter available */
	for (i = 1; i < 3500; i++) {
		if ((in8 (FRONT_BASE + UART_LSR) & 0x20) == 0x20)
			break;
		udelay (100);
	}
	out8 (FRONT_BASE + UART_THR, c);	/* put character out */
}

int front_send_u2( unsigned char *packet )
{
	int a;

	for( a = 0; a < 7; a++)
	{
		front_putc( packet[a] );
		udelay(10000);
	}
	return 0;
}

int front_send_packet(char *packet)
{
	unsigned char chk_sum = 0;

	chk_sum = packet[0] +
		packet[1] +
		packet[2] +
		packet[3] +
		packet[4] +
		packet[5] + 0xcc;
	packet[6] = chk_sum;

	front_send_u2( packet );

	return 0;
}

int front_persent( int now, int total )
{
	unsigned char buf[7];
	const unsigned char show_table_l[14] = 
		{ ~0x08, ~0x10, ~0x40, ~0x02, ~0x01, ~0x20, ~0x40, ~0x04, ~0x08, ~0x10, ~0x20, ~0x01, ~0x02, ~0x04 };
	const unsigned char show_table_r[14] = 
		{ ~0x08, ~0x04, ~0x40, ~0x20, ~0x01, ~0x02, ~0x40, ~0x10, ~0x08, ~0x04, ~0x02, ~0x01, ~0x20, ~0x10 };
	static int table_index = 0;
	int a;
	int persent = now *100 / total;

	buf[0] = 0xaa;
	buf[1] = 0;
	buf[2] = show_table_l[table_index];
	buf[3] = front_convert( (persent<10)?0:(persent/10)%10 + '0' );
	buf[4] = front_convert( persent%10 + '0' );
	buf[5] = show_table_r[table_index];
	buf[6] = 0;
	for( a=0; a<6; a++ )
		buf[6] += buf[a];
	buf[6] += 0xcc;

	front_send_u2( buf );

	table_index ++;
	table_index %= 14;
	
	return 0;
}

void front_puts (const char *s)
{
	int a;
	unsigned char packet[6];

	packet[0] = 0xaa;
	packet[1] = 0x00;
	for( a=0; (a<4)&&s[a]; a++ )
	{
		packet[2+a] = front_convert( s[a] );
	}
	for( ; a<4; a++ )
		packet[2+a] = 0xff;

	front_send_packet( packet );
}

int front_getc ()
{
	unsigned char status = 0;

	while (1) {
#if defined(CONFIG_HW_WATCHDOG)
		WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
#endif	/* CONFIG_HW_WATCHDOG */
		status = in8 (FRONT_BASE + UART_LSR);
		if ((status & asyncLSRDataReady1) != 0x0) {
			break;
		}
		if ((status & ( asyncLSRFramingError1 |
				asyncLSROverrunError1 |
				asyncLSRParityError1  |
				asyncLSRBreakInterrupt1 )) != 0) {
			out8 (FRONT_BASE + UART_LSR,
			      asyncLSRFramingError1 |
			      asyncLSROverrunError1 |
			      asyncLSRParityError1  |
			      asyncLSRBreakInterrupt1);
		}
	}
	return (0x000000ff & (int) in8 (FRONT_BASE));
}

int front_tstc ()
{
	unsigned char status;

	status = in8 (FRONT_BASE + UART_LSR);
	if ((status & asyncLSRDataReady1) != 0x0) {
		return (1);
	}
	if ((status & ( asyncLSRFramingError1 |
			asyncLSROverrunError1 |
			asyncLSRParityError1  |
			asyncLSRBreakInterrupt1 )) != 0) {
		out8 (FRONT_BASE + UART_LSR,
		      asyncLSRFramingError1 |
		      asyncLSROverrunError1 |
		      asyncLSRParityError1  |
		      asyncLSRBreakInterrupt1);
	}
	return 0;
}

int front_convert( char letter )
{
	switch( letter )
	{
		case '1': return 0xf9;
		case '2': return 0xa4;
		case '3': return 0xb0;
		case '4': return 0x99;
		case '5': return 0x92;
		case '6': return 0x82;
		case '7': return 0xd8;
		case '8': return 0x80;
		case '9': return 0x98;
		case '0': return 0xc0;
		case 'A': return 0x88;
		case 'B': return 0x80;
		case 'C': return 0xc6;
		case 'D': return 0xa1;
		case 'E': return 0x86;
		case 'F': return 0x8e;
		case 'G': return 0x82;
		case 'H': return 0x89;
		case 'I': return 0xf9;
		case 'J': return 0xf0;
		case 'K': break; 
		case 'L': return 0xc7;
		case 'M': break; 
		case 'N': return 0xab;
		case 'O': return 0xc0;
		case 'P': return 0x8c;
		case 'Q': return 0x98;
		case 'R': return 0x88;
		case 'S': return 0x92;
		case 'T': return 0xf8;
		case 'U': return 0xc1;
		case 'V': break;
		case 'W': break;
		case 'X': break;
		case 'Y': return 0x91;
		case 'Z': break; 
		case 'a': return 0x88;
		case 'b': return 0x83;
		case 'c': return 0xa7;
		case 'd': return 0xa1;
		case 'e': return 0x86;
		case 'f': return 0x8e;
		case 'g': return 0x98;
		case 'h': return 0x8b;
		case 'i': return 0xf9;
		case 'j': return 0xf0;
		case 'k': break;
		case 'l': return 0xc7;
		case 'm': break;
		case 'n': return 0xab;
		case 'o': return 0xa3;
		case 'p': return 0x8c;
		case 'q': return 0x98;
		case 'r': return 0xaf;
		case 's': return 0x92;
		case 't': return 0x87;
		case 'u': return 0xe3;
		case 'v': break;
		case 'w': break;
		case 'x': break;
		case 'y': return 0x91;
		case 'z': break;
		case '-': return 0xbf;
		case '_': return 0xf7;
		default : break;
	}

	return 0xff;
}

#ifdef DEBUG
#define fdebug(fmt,arg...) printf(fmt,##arg)
#else
#define fdebug(fmt,arg...) do{}while(0)
#endif

#define FRONT_PACKCNT		2
#define FRONT_RMCKEY		0xa1
#define FRONT_FRTKEY		0xa2
#define FRONT_RMCREL		0xaf

typedef struct
{
	char code;
	enum front_key key;
} key_table_t;
const static key_table_t rmckeys[] =
{
	{ 0x0a, key_power },
	{ 0x00, key_left },
	{ 0x00, key_right },
	{ 0x00, key_up },
	{ 0x00, key_down },
	{ 0x0f, key_ok },
	{ 0xff, key_release },
};
#define rmckeynum	(sizeof(rmckeys)/sizeof(rmckeys[0]))
const static key_table_t fntkeys[] =
{
	{ 0x7f, key_front_power },
	{ 0xfb, key_front_left },
	{ 0xf7, key_front_right },
	{ 0xdf, key_front_up },
	{ 0xef, key_front_down },
	{ 0xbf, key_front_ok },
	{ 0x7b, key_front_p_left },
	{ 0x77, key_front_p_right },
	{ 0x5f, key_front_p_up },
	{ 0x6f, key_front_p_down },
	{ 0x3f, key_front_p_ok },
	{ 0xff, key_front_release },
};
#define fntkeynum	(sizeof(fntkeys)/sizeof(fntkeys[0]))


int front_getkey( void )
{
	static int pack_cnt = 0;
	static int pack_buf[FRONT_PACKCNT];
	int a;
	int key = key_null;

	if( front_tstc() )
	{
		pack_buf[pack_cnt] = front_getc();
		pack_cnt ++;

		if( pack_cnt >= FRONT_PACKCNT )
		{
			fdebug( "got packet %02x %02x\n", pack_buf[0], pack_buf[1] );
			pack_cnt = 0;

			switch( pack_buf[0] )
			{
				case FRONT_FRTKEY:
					for( a=0; a<fntkeynum; a++ )
						if( pack_buf[1] == fntkeys[a].code )
						{
							key = fntkeys[a].key;
							break;
						}
					break;
				case FRONT_RMCKEY:
					for( a=0; a<rmckeynum; a++ )
						if( pack_buf[1] == rmckeys[a].code )
						{
							key = rmckeys[a].key;
							break;
						}
					break;
				case FRONT_RMCREL:
					break;
				default:
					for( a=0; a<FRONT_PACKCNT-1; a++ )
						pack_buf[a] = pack_buf[a+1];
					pack_cnt = FRONT_PACKCNT-1;
					break;
			}
		}
	}

	return key;
}

#endif
