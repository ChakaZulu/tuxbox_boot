/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <ppcboot.h>

#ifdef CONFIG_I2C

#include <commproc.h>
#include <i2c.h>

#define DEBUG_STEP	0
#define PRINTD(x)	if (DEBUG_STEP) printf(x);
#define DELAY_US	100000 	// us to wait before checking the I2c

//#define DEBUG_I2C_RATE	0	// To show selected I2C bus rate

#define I2C_PRAM 0
#define CPCR_FLAG 0x01
#define I2C_CPCR_CMD ( ( 0<<(15-7) ) | ( 1 << (15-11) ) | CPCR_FLAG )
#define I2C_RX_LEN 128 /* Receive buffer length */
#define I2C_TX_LEN 128 /* Transmit buffer length */
#define TXBD_R 0x8000  /* Transmit buffer ready to send */
#define TXBD_W 0x2000  /* Wrap, last buffer in buffer circle */
#define TXBD_L 0x0800  /* Last, this buffer is the last in this frame */
                       /* This bit causes the STOP condition to be sent */
#define TXBD_S 0x0400  /* Start condition.  Causes this BD to transmit a start */
#define RXBD_E 0x8000  /* Receive buffer is empty and can be used by CPM */
#define RXBD_W 0x2000  /* Wrap, last receive buffer in buffer circle */

typedef struct I2C_BD
{
  unsigned short status;
  unsigned short length;
  unsigned char *addr;
} I2C_BD;


static I2C_BD *rxbd, *txbd;  	/* buffer descriptors are defined */
	                	/* globally for this file */

static unsigned char
    rxbuf[I2C_RX_LEN],
    txbuf[I2C_TX_LEN];

// Returns the best value of I2BRG to meet desired clock speed of I2C with
// input parameters (clock speed, filter, and predivider value).
// It returns computer speed value and the difference between it and desired
// speed.
static inline int i2c_roundrate (int hz, int speed, int filter, int modval,
				    int *brgval, int *totspeed)
{
    int moddiv = 1 << (5-(modval & 3)),
	brgdiv,
	div;

    brgdiv = hz / (moddiv * speed);

    *brgval = brgdiv / 2 - 3 - 2*filter ;

    if ((*brgval < 0) || (*brgval > 255))
	return -1 ;

    brgdiv = 2 * (*brgval + 3 + 2 * filter) ;
    div  = moddiv * brgdiv ;
    *totspeed = hz / div ;

    return  0;
}

// Sets the I2C clock predivider and divider to meet required clock speed
static int i2c_setrate (int hz, int speed)
{
    immap_t	*immap = (immap_t *)CFG_IMMR ;
    i2c8xx_t	*i2c	= (i2c8xx_t *)&immap->im_i2c;
    int brgval,
	modval,	// 0-3
	bestspeed_diff = speed,
	bestspeed_brgval=0,
	bestspeed_modval=0,
	bestspeed_filter=0,
	totspeed,
	filter=0;	// Use this fixed value

//    for (filter = 0; filter < 2; filter++)
	for (modval = 0; modval < 4; modval++)
	    if (i2c_roundrate (	hz, speed,
				filter, modval,
				&brgval, &totspeed) == 0)
	    {
		int diff = speed - totspeed ;

		if ((diff >= 0) && (diff < bestspeed_diff))
		{
		    bestspeed_diff 	= diff ;
		    bestspeed_modval 	= modval;
		    bestspeed_brgval 	= brgval;
		    bestspeed_filter 	= filter;
		}
	    }

#ifdef DEBUG_I2C_RATE
    printf("Best is:\n");
    printf("\nCPU=%dhz RATE=%d F=%d I2MOD=%08x I2BRG=%08x DIFF=%dhz",
	    hz, speed,
	    bestspeed_filter, bestspeed_modval, bestspeed_brgval,
	    bestspeed_diff);
#endif
    i2c->i2c_i2mod |= ((bestspeed_modval & 3) << 1) | (bestspeed_filter << 3);
    i2c->i2c_i2brg = bestspeed_brgval & 0xff;

#ifdef DEBUG_I2C_RATE
    printf("i2mod=%08x i2brg=%08x\n", i2c->i2c_i2mod, i2c->i2c_i2brg);
#endif

    return 1 ;
}

volatile i2c8xx_t	*i2c;
volatile cbd_t		*tbdf, *rbdf;
volatile iic_t		*iip;

void i2c_init(int speed)
{
	init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);
        immap_t	*immap 	= (immap_t *)CFG_IMMR ;
	volatile cpm8xx_t	*cp;

	/* Get pointer to Communication Processor
	 * and to internal registers
	 */
	cp = (cpm8xx_t *)&immap->im_cpm ;
	iip = (iic_t *)&cp->cp_dparam[PROFF_IIC];
	i2c = (i2c8xx_t *)&(immap->im_i2c);

	// Disable relocation
	iip->iic_rpbase = 0 ;

	/* Initialize Port B I2C pins.
	 */
	cp->cp_pbpar |= 0x00000030;
	cp->cp_pbdir |= 0x00000030;
	cp->cp_pbodr |= 0x00000030;

	/* Disable interrupts.
	 */
	i2c->i2c_i2mod = 0;
	i2c->i2c_i2cmr = 0;
	i2c->i2c_i2cer = 0xff;

	// Set the I2C BRG Clock division factor from desired i2c rate
	// and current CPU rate (we assume sccr dfbgr field is 0;
	// divide BRGCLK by 1)

	PRINTD("\n[I2C  ] Setting rate...");
	i2c_setrate (idata->cpu_clk, speed) ;

	/* Set I2C controller in master mode
	 */
	i2c->i2c_i2com = 0x01;

	// Set SDMA bus arbitration level to 5 (SDCR)
	immap->im_siu_conf.sc_sdcr = 0x0001 ;

	/* Initialize Tx/Rx parameters.*/
#if 0
        iip->iic_rbptr = iip->iic_rbase = BD_IIC_START ;//2018 ;
	iip->iic_tbptr = iip->iic_tbase = iip->iic_rbase + sizeof(I2C_BD) ; //2020 ;
#else
	iip->iic_rbptr = iip->iic_rbase = m8xx_cpm_dpbase_align(8) ;
        iip->iic_tbptr = iip->iic_tbase = iip->iic_rbase + sizeof(I2C_BD);
#endif
	rxbd = (I2C_BD *)((unsigned char *)&cp->cp_dpmem[iip->iic_rbase]);
	txbd = (I2C_BD *)((unsigned char *)&cp->cp_dpmem[iip->iic_tbase]);

#if DEBUG_STEP
	printf("rbase = %04x\n", iip->iic_rbase);
	printf("tbase = %04x\n", iip->iic_tbase);
	printf("Rxbd1=%08x\n", (int)rxbd);
	printf("Txbd1=%08x\n", (int)txbd);
#endif

        cp->cp_cpcr = mk_cr_cmd(CPM_CR_CH_I2C, CPM_CR_INIT_TRX) | CPM_CR_FLG;
	while (cp->cp_cpcr & CPM_CR_FLG);

	/* Set big endian byte order
	 */
	iip->iic_tfcr = 0x15;
	iip->iic_rfcr = 0x15;

	/* Set maximum receive size.
	 */
	iip->iic_mrblr = 128;

	PRINTD("\n[I2C  ] Clearing the buffer memory...");

	// Clear the buffer memory
	memset ((char *)rxbuf, I2C_RX_LEN, 0);
	memset ((char *)txbuf, I2C_TX_LEN, 0);

	PRINTD("\n[I2C  ] Initializing BD's...");

	// Initialize the BD's

	// Rx: Wrap, no interrupt, empty
	rxbd->addr = rxbuf;
	rxbd->status = 0xa800;

	// Tx: Wrap, no interrupt, not ready to send, last
	txbd->addr = txbuf;
	txbd->status = 0x2800;

	// Clear events and interrupts
	i2c->i2c_i2cer = 0xff ;
	i2c->i2c_i2cmr = 0 ;
}

void i2c_send( unsigned char address,
              unsigned char secondary_address,
              int enable_secondary,
              unsigned short size, unsigned char dataout[] )
{
  int i,j;

  if( size > I2C_TX_LEN )  /* Trying to send message larger than BD */
    return;

  PRINTD("\n[I2C  ] Waiting for transmit buffer empty...");
  while( txbd->status & TXBD_R ) ; // Loop until previous data sent

  PRINTD("\n[I2C  ] Formatting addresses...");
  if( enable_secondary ) /* Device has an internal address */
  {
    txbd->length = size + 2;  /* Length of message plus dest addresses */
    txbd->addr[0] = address;
    txbd->addr[0] &= ~(0x01);
    txbd->addr[1] = secondary_address;
    i = 2;
  }
  else
  {
    txbd->length = size + 1;  /* Length of message plus dest address */
    txbd->addr[0] = address;  /* Write destination address to BD */
    txbd->addr[0] &= ~(0x01);  /* Set address to write */
    i = 1;
  }

#if DEBUG_STEP
    printf("Length = %d addr[0] = %08x addr[1] = %08x\n",
	txbd->length,
	txbd->addr[0],
	txbd->addr[1]);
#endif

  /* Copy data to send into buffer */

 PRINTD("\n[I2C  ] Copying data into buffer...");

  for( j = 0; j < size; i++, j++ )
    txbd->addr[ i ] = dataout[j];

  /* Ready to Transmit, wrap, last */

  PRINTD("\n[I2C  ] Waiting to transmit...");

  txbd->status = txbd->status | TXBD_R | TXBD_W | TXBD_L | TXBD_S ;

  /* Enable I2C */

  PRINTD("\n[I2C  ] Enabling I2C...");
  i2c->i2c_i2mod |= 1;

  /* Transmit */
  PRINTD("\n[I2C  ] Transmitting...");
  i2c->i2c_i2com |= 0x80;

  PRINTD("\n[I2C  ] Waiting for transmit buffer empty...");
  udelay (DELAY_US) ;	// This is a patch!

  while( txbd->status & TXBD_R );

  /* Turn off I2C */
  PRINTD("\n[I2C  ] Turning off I2C...");
  i2c->i2c_i2mod &= (~1);

#if DEBUG_STEP
 printf("\nTXBD->CBD_SC=%08x\n", txbd->status);

 if (txbd->status & 4)
    while(1);
#endif
}

void i2c_receive(unsigned char address,
		unsigned char secondary_address,
		int enable_secondary,
                unsigned short size_to_expect, unsigned char datain[] )
{
  int i, j;

  if( size_to_expect > I2C_RX_LEN )
	return;  /* Expected to receive too much */

  /* Turn on I2C */
  i2c->i2c_i2mod |= 0x01;

  /* Setup TXBD for destination address */
  if( enable_secondary )
  {
    txbd->length = 2;
    txbd->addr[0] = address | 0x00;   /* Write data */
    txbd->addr[1] = secondary_address;  /* Internal address */
    txbd->status = TXBD_R;

    /* Reset the rxbd */
    rxbd->status = RXBD_E | RXBD_W;

    /* Begin transmission */
    i2c->i2c_i2com |= 0x80;

  }
  else
  {
    txbd->length = 1 + size_to_expect;
    txbd->addr[0] = address | 0x01;


    /* Buffer ready to transmit, wrap, loop */
    txbd->status |= TXBD_R | TXBD_W | TXBD_L;

    /* Reset the rxbd */
    rxbd->status = RXBD_E | RXBD_W;

    /* Begin transmission */
    i2c->i2c_i2com |= 0x80;

    while( txbd->status & TXBD_R);  /* Loop until transmit completed */
  }

  while( rxbd->status & RXBD_E);  /* Wait until receive is finished */

  for( i= 0, j = 0; j < size_to_expect; j++, i++ )  /* Copy data to datain[] */
    datain[j] = rxbd->addr[i];

  /* Turn off I2C */
  i2c->i2c_i2mod &= (~1);
}

#endif	/* CONFIG_I2C */
