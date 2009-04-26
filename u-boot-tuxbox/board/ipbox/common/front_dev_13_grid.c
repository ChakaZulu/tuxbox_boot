#include <common.h>

#ifdef CONFIG_DGS_FRONT

#include <command.h>
#include <asm/processor.h>

#include "common/front.h"
#include "front_dev.h"

#define MICOM_REQ_WR_VFDBUF	0xd0
#define MICOM_REQ_WR_VFDUPDATE 	0xd1

#define UART0_BASE      0x40040000
#define UART1_BASE      0x40000000

#define FRONT_BASE	UART1_BASE

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
 * Line Status Register.
 *----------------------------------------------------------------------------*/
#define asyncLSRDataReady1            0x01
#define asyncLSROverrunError1         0x02
#define asyncLSRParityError1          0x04
#define asyncLSRFramingError1         0x08
#define asyncLSRBreakInterrupt1       0x10
#define asyncLSRTxHoldEmpty1          0x20
#define asyncLSRTxShiftEmpty1         0x40
#define asyncLSRRxFifoError1          0x80

//#define DEBUG
#ifdef DEBUG
#define fdebug(fmt,arg...) printf(fmt,##arg)
#else
#define fdebug(fmt,arg...) do{}while(0)
#endif

#define FRONT_PACKCNT		2
#define FRONT_RMCKEY		0xe0
#define FRONT_FRTKEY_H		0xe1
#define FRONT_FRTKEY_L		0xe2

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
	bdiv = (378000000/18+tmp/2)/tmp;	// 378MHz / 18 = 21MHz...

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

void front_putc (char c)
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

int front_send_packet(const char *packet)
{
	int a;

	for( a = 0; a < 5; a++)
		front_putc( packet[a] );

	return 0;
}

int front_persent( int now, int total )
{
	char buf[16];

	sprintf( buf, "received %2d", now*100/total );
	front_puts( buf );

	return 0;
}

void front_putchar( int pos, char ch )
{
	static const unsigned short num2seg[] =
	{
		0x3123,		// 0
		0x0408,		// 1
		0x30c3,		// 2
		0x21c3,		// 3
		0x01e2,		// 4
		0x21e1,		// 5
		0x31e1,		// 6
		0x0123,		// 7
		0x31e3,		// 8
		0x21e3,		// 9
	};

	static const unsigned short Char2seg[] =
	{
		0x11e3,		// A
		0x25cb,		// B
		0x3021,		// C
		0x250b,		// D
		0x30e1,		// E
		0x10e1,		// F
		0x31a1,		// G
		0x11e2,		// H
		0x2409,		// I
		0x0809,		// J
		0x1264,		// K
		0x3020,		// L
		0x1136,		// M
		0x1332,		// N
		0x3123,		// O
		0x10e3,		// P
		0x3323,		// Q
		0x12e3,		// R
		0x21e1,		// S
		0x0409,		// T
		0x3122,		// U
		0x1824,		// V
		0x1b22,		// W
		0x0a14,		// X
		0x04e2,		// Y
		0x2805,		// Z
	};
	unsigned char packet[6];
	unsigned short data;

	switch( ch )
	{
		case 'A' ... 'Z':
			ch -= 'A'-'a';
		case 'a' ... 'z':
			data = Char2seg[ch-'a'];
			break;
		case '0' ... '9':
			data = num2seg[ch-'0'];
			break;
		case '-':
			data = 0xc0;
			break;
		case '\'' :
			data = 0x0004;
			break;
		default:
			data = 0;
			break;
	}

	packet[0] = MICOM_REQ_WR_VFDBUF;
	packet[1] = pos;
	packet[2] = data&0xff;
	packet[3] = (data>>8)&0xff;
	packet[4] = 0x00;
	packet[5] = 0x00;
	
	front_send_packet( packet );
}

void front_puts (const char *s)
{
	int a;
	int pos;
	unsigned char packet[6];
	
	pos = 13 - strlen(s);
	if( pos < 0 )
		pos = 0;
	pos /= 2;

	for( a=0; a<pos; a++ )
		front_putchar( a, ' ' );
	for( ; *s && pos<13; pos++, s++ )
		front_putchar( pos, *s );
	for( ; pos<13; pos++ )
		front_putchar( pos, ' ' );

	packet[0] = MICOM_REQ_WR_VFDUPDATE;
	packet[1] = 0x00;
	packet[2] = 0x00;
	packet[3] = 0x00;
	packet[4] = 0x00;
	packet[5] = 0x00;
	front_send_packet(packet);	
}

int front_getc ()
{
	unsigned char status = 0;

	while (1) {
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

typedef struct
{
	unsigned short code;
	enum front_key key;
} key_table_t;
const static key_table_t rmckeys[] =
{
	{ 0x0a, key_power },
	{ 0x1d, key_left },
	{ 0x1c, key_right },
	{ 0x1a, key_up },
	{ 0x1b, key_down },
	{ 0x1f, key_ok },
	{ 0x10, key_0 },
	{ 0x11, key_1 },
	{ 0x12, key_2 },
	{ 0x13, key_3 },
	{ 0x14, key_4 },
	{ 0x15, key_5 },
	{ 0x16, key_6 },
	{ 0x17, key_7 },
	{ 0x18, key_8 },
	{ 0x19, key_9 },
	{ 0xff, key_release },
};
#define rmckeynum	(sizeof(rmckeys)/sizeof(rmckeys[0]))
const static key_table_t fntkeys[] =
{
	{ 0x1000, key_front_power },
	{ 0x0002, key_front_left },
	{ 0x0004, key_front_right },
	{ 0x4000, key_front_up },
	{ 0x0040, key_front_down },
	{ 0x0020, key_front_ok },
	{ 0x1002, key_front_p_left },
	{ 0x1004, key_front_p_right },
	{ 0x5000, key_front_p_up },
	{ 0x1040, key_front_p_down },
	{ 0x1020, key_front_p_ok },
	{ 0x0000, key_front_release },
};
#define fntkeynum	(sizeof(fntkeys)/sizeof(fntkeys[0]))


int front_getkey( void )
{
	static int pack_cnt = 0;
	static unsigned char pack_buf[FRONT_PACKCNT];
	static int front_key = 0;
	int a;
	int key = key_null;

	if( front_tstc() )
	{
		pack_buf[pack_cnt] = front_getc();
		fdebug( "serial get %02x\n", pack_buf[pack_cnt] );
		pack_cnt ++;

		if( pack_cnt >= FRONT_PACKCNT )
		{
			//fdebug( "got packet %02x %02x\n", pack_buf[0], pack_buf[1] );
			pack_cnt = 0;

			switch( pack_buf[0] )
			{
				case FRONT_FRTKEY_H:
					front_key = pack_buf[1]<<8;
					break;
				case FRONT_FRTKEY_L:
					if( front_key >= 0 )
					{
						front_key |= pack_buf[1];
						fdebug( "%04x\n", front_key );

						for( a=0; a<fntkeynum; a++ )
							if( front_key == fntkeys[a].code )
							{
								key = fntkeys[a].key;
								break;
							}

						front_key = -1;
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
				default:
					//fdebug( "unknown.\n" );
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
