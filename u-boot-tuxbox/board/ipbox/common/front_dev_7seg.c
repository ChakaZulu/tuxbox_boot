#include <common.h>

#ifdef CONFIG_DGS_FRONT

#include <command.h>
#include <asm/processor.h>

#include "common/front.h"
#include "front_dev.h"

#define MICOM_REQ_WR_VFDBUF	0xd0
#define MICOM_REQ_WR_VFD	0xcb

#define UART0_BASE      0x40040000
#define UART1_BASE      0x40000000
#define UART2_BASE      0x40010000

#if defined(CONFIG_RELOOK100S)		/* mutant is on uart0 */
#define FRONT_BASE	UART0_BASE
#else								/* cubecafe's are on uart2 */
#define FRONT_BASE	UART2_BASE
#endif

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

static const unsigned char num2seg[]=
{
	0xc0,
	0xf9,
	0xa4,
	0xb0,
	0x99,
	0x92,
	0x82,
	0xd8,
	0x80,
	0x98,
};
static const unsigned char Char2seg[]=
{
	0x88,	// A
	0x80,	// B
	0xc6,	// C
	0xa1,	// D
	0x86,	// E
	0x8e,	// F
	0x82,	// G
	0x89,	// H
	0xf9,	// I
	0xf0,	// J
	0xff,	// K
	0xc7,	// L
	0xaa, 	// M
	0xab,	// N
	0xc0,	// O
	0x8c,	// P
	0x98,	// Q
	0x88,	// R
	0x92,	// S
	0xf8,	// T
	0xc1,	// U
	0xff,	// V
	0xff,	// W
	0xff, 	// X
	0xff,	// X
	0x91,	// Y
	0xff,	// Z
};
static const unsigned char char2seg[]=
{
	0x88, 	// a
	0x83,	// b
	0xa7,	// c
	0xa1,	// d
	0x86,	// e
	0x8e,	// f
	0x98,	// g
	0x8b,	// h
	0xf9,	// i
	0xf0,	// j
	0xff,	// k
	0xc7,	// l
	0xaa,	// m
	0xab,	// n
	0xa3, 	// o
	0x8c,	// p
	0x98,	// q
	0xaf,	// r
	0x92,	// s
	0x87,	// t
	0xe3,	// u
	0xff,	// v
	0xff,	// w
	0xff,	// x
	0x91,	// y
	0xff,	// z
};

int front_persent( int now, int total )
{
	unsigned char packet[6];
	const unsigned char show_table_l[14] =
		{ ~0x08, ~0x10, ~0x40, ~0x02, ~0x01, ~0x20, ~0x40, ~0x04,
			~0x08, ~0x10, ~0x20, ~0x01, ~0x02, ~0x04 };
	const unsigned char show_table_r[14] =
		{ ~0x08, ~0x04, ~0x40, ~0x20, ~0x01, ~0x02, ~0x40, ~0x10,
			~0x08, ~0x04, ~0x02, ~0x01, ~0x20, ~0x10 };
	static int table_index = 0;
	int persent = now *100 / total;
	unsigned char buf[4];

	buf[0] = show_table_l[table_index];
	buf[1] = persent<10?0xff:num2seg[persent/10%10];
	buf[2] = num2seg[persent%10];
	buf[3] = show_table_r[table_index];

	packet[0] = MICOM_REQ_WR_VFDBUF;
	packet[1] = buf[0];
	packet[2] = buf[1];
	packet[3] = buf[2];
	packet[4] = buf[3];

	front_send_packet( packet );

	table_index ++;
	table_index %= 14;

	return 0;
}

void front_putchar( int pos, char ch )
{
	unsigned char packet[6];
	unsigned char data;

	if(pos>3)
	{
		fdebug("front_putchar() pos(%d) out of range \n",pos);
		return;
	}

	switch( ch )
	{
		case 'A' ... 'Z':
			data = Char2seg[ch-'A'];
			break;
		case 'a' ... 'z':
			data = char2seg[ch-'a'];
			break;
		case '0' ... '9':
			data = num2seg[ch-'0'];
			break;
		case '-' :
			data = 0xbf;
			break;
		case '_' :
			data = 0xf7;
			break;
		default:
			data = 0xff;
			break;
	}

	packet[0] = MICOM_REQ_WR_VFD;
	packet[1] = (unsigned char)pos;
	packet[2] = data;
	packet[3] = 0x00;
	packet[4] = 0x00;
	packet[5] = 0x00;

	front_send_packet( packet );
}

unsigned char maptable(char ch)
{
	unsigned char data;
	switch( ch )
	{
		case 'A' ... 'Z':
			data = Char2seg[ch-'A'];
			break;
		case 'a' ... 'z':
			data = char2seg[ch-'a'];
			break;
		case '0' ... '9':
			data = num2seg[ch-'0'];
			break;
		case '-' :
			data = 0xbf;
			break;
		case '_' :
			data = 0xf7;
			break;
		default:
			data = 0xff;
			break;
	}
	return data;
}

void front_puts (const char *s)
{
	unsigned char packet[5] = {0,0,0,0,0};

	packet[0] = MICOM_REQ_WR_VFDBUF;
    packet[1] = maptable(s[0]);
	packet[2] = maptable(s[1]);
	packet[3] = maptable(s[2]);
	packet[4] = maptable(s[3]);

	front_send_packet( packet );

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
#if defined(CONFIG_RELOOK100S)		/* mutant has different mapping */
	{ 0x1a, key_power },
	{ 0x02, key_left },
	{ 0x03, key_right },
	{ 0x00, key_up },
	{ 0x01, key_down },
#else								/*cubecafe's use the relook400 mapping */
	{ 0x0a, key_power },
	{ 0x1d, key_left },
	{ 0x1c, key_right },
	{ 0x1a, key_up },
	{ 0x1b, key_down },
#endif
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
	{ 0x00f7, key_front_power },
	{ 0x007f, key_front_left },
	{ 0x00bf, key_front_right },
	{ 0x00df, key_front_up },
	{ 0x00ef, key_front_down },
	{ 0x00fe, key_front_ok },
	{ 0x0077, key_front_p_left },
	{ 0x00b7, key_front_p_right },
	{ 0x00d7, key_front_p_up },
	{ 0x00e7, key_front_p_down },
	{ 0x00f6, key_front_p_ok },
	{ 0x00ff, key_front_release },
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
		fdebug( "serial get => %02x\n", pack_buf[pack_cnt] );
		pack_cnt ++;

		if( pack_cnt >= FRONT_PACKCNT )
		{
			fdebug( "got packet : %02x %02x %02x\n", pack_buf[0], pack_buf[1], pack_buf[2] );
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
						fdebug( "front key : %04x\n", front_key );

						for( a=0; a<fntkeynum; a++ )
						{
							if( front_key == fntkeys[a].code )
							{
								key = fntkeys[a].key;
								break;
							}
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
					fdebug( "---> unknown.\n" );
					for( a=0; a<FRONT_PACKCNT-1; a++ )
						pack_buf[a] = pack_buf[a+1];
					pack_cnt = FRONT_PACKCNT-1;
					break;
			}
		}
	}

	fdebug("---> return key : %d\n",key);

	return key;
}

#endif
