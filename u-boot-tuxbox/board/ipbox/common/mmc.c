/*
#include <asm/errno.h>
#include <linux/types.h>
#define uchar unsigned char
*/

#include <config.h>
#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <part.h>

#ifdef CFG_CMD_MMC

#ifdef CONFIG_PPC405_GPIO
#include "gpio.h"
#else
#error no CONFIG_PPC405_GPIO defined!
#endif

#define CFG_MMC_BASE 0xF0000000
#define MMC_BLOCK_SIZE			512


#define MMC_CMD_0_GO_IDLE				0
#define MMC_CMD_1_SEND_OP_COND			1
#define MMC_CMD_9_SEND_CSD				9
#define MMC_CMD_10_SEND_CID				10
#define MMC_CMD_12_STOP					12
#define MMC_CMD_13_SEND_STATUS			13
#define MMC_CMD_16_BLOCKLEN				16
#define MMC_CMD_17_READ_SINGLE			17
#define MMC_CMD_18_READ_MULTIPLE		18
#define MMC_CMD_24_WRITE_SINGLE			24

// bit definitions for R1
#define MMC_R1_IN_IDLE 	0x01
#define MMC_R1B_BUSY_BYTE 0x00

// different defines for tokens etc ...
#define MMC_START_TOKEN_SINGLE  0xfe
#define MMC_START_TOKEN_MULTI   0xfc

// return values from a write operation
#define MMC_DATA_ACCEPT 		0x2
#define MMC_DATA_CRC	 		0x5
#define MMC_DATA_WRITE_ERROR	0x6


typedef struct _VOLUME_INFO{
  unsigned char   	size_MB;
  unsigned long int size;
  unsigned char   	sector_multiply;
  unsigned int   	sector_count;
  unsigned int 	sector_size;
  unsigned char	name[6];
} VOLUME_INFO;


static block_dev_desc_t mmc_dev;

block_dev_desc_t * mmc_get_dev(int dev)
{
	return ((block_dev_desc_t *)&mmc_dev);
}



static uchar mmc_buf[MMC_BLOCK_SIZE];
static int mmc_ready = 0;
static unsigned int mmc_size=0;


// these pins are I/O
#define GPIO9  9
#define GPIO10 10
#define GPIO11 11
#define GPIO12 12

// SPI port defines
#define SCK		GPIO12
#define SI		GPIO10
#define SO		GPIO11
#define CS		GPIO9


static inline void stb2500_gpio_setpin(unsigned int pin, unsigned int to)
{
	gpio_out (0,0x80000000>>pin,to?0x80000000>>pin:0);
}

static inline unsigned int stb2500_gpio_getpin(unsigned int pin)
{
	unsigned long data=0;
	gpio_in(0, 0x80000000>>pin ,&data);
	return (data >> (31-pin)) &1;
}


static inline int
bit_get(unsigned int pin){
	return stb2500_gpio_getpin(pin);
}

// set a pin to 3,3V
static inline void
bit_set(unsigned int pin){
	stb2500_gpio_setpin(pin,1);
}


// set a pin to 0V
static inline void
bit_clear(unsigned int pin){
	stb2500_gpio_setpin(pin,0);
}



// this function is used to toggle the spi clock
// data is read at a low to high transition of the SCK pin
static void SPI_clock(void){
	bit_clear(SCK);
	bit_set(SCK);
}


// write a byte to the spi bus
// data is send MSB first
static unsigned char spi_io(unsigned char byte){
	int i;
	unsigned char byte_out = 0;
	for(i = 7; i>=0; i--){
		if(byte & (1<<i)){
			bit_set(SI);
		} else {
			bit_clear(SI);
		};
		SPI_clock();
		byte_out += bit_get(SO)<<i;
	};
	return byte_out;
}

// selects the CS for spi xfer
static inline void select_cs(void)
{
	// pull down the MMC CS line
	bit_clear(CS);
}

// deselects the CS for spi xfer
static inline void deselect_cs(void)
{
	// pull up the MMC CS card
	bit_set(CS);
}


// stops the MMC transmission and sends the 8 clock cycles needed by the mmc for cleanup
static void MMC_cleanup(void){
	// deselect the MMC card
	deselect_cs();
	// pulse the SCK 8 times
	spi_io(0xff);
}

// sends a block of data from the mem over the spi bus
static void spi_io_mem(unsigned char *data, int length){
	// transmit 'length' bytes over spi
	while(length){
		spi_io(*data);
		data++;
		length--;
	}
}

// waits for the card to send the start data block token
static void MMC_wait_for_start_token(unsigned int max_errors){
  unsigned char retval;
  do{
    // get a byte from the spi bus
    retval = spi_io(0xff);
    // keep track of the trys
    max_errors--;
  }while((retval != MMC_START_TOKEN_SINGLE) );
}



// send cmd + arguments + default crc for init command. once in spi mode
// we dont need crc so we can keep this constant
static void MMC_send_cmd(unsigned char cmd, unsigned long int data){
	// default command sequence
	static unsigned char buffer[6] ;
	// fill sequence with our specific data
	buffer[0]=0x40 + cmd;
	buffer[1]=(data>>24)&0xff;
	buffer[2]=(data>>16)&0xff;
	buffer[3]=(data>>8)&0xff;
	buffer[4]=data&0xff;
	buffer[5]=0x95;
	// dispach data
	spi_io_mem(buffer,6);
}

// gets a 1 byte long R1
static unsigned char MMC_get_R1(void){
	unsigned char retval;
	unsigned int max_errors = 1024;
	// wait for first valid response byte
	do{
		retval = spi_io(0xff);
		max_errors--;
	}while(  (retval & 0x80) && (max_errors>0));

	return retval;
}

// gets n bytes plus crc from spi bus
static void MMC_get_data(unsigned char *ptr_data, unsigned int length){
  MMC_wait_for_start_token(1024);
  while(length){
    *ptr_data = spi_io(0xff);
    //USART_sendint(*ptr_data);
	//USART_send(' ');
	length--;
    ptr_data++;
  }
  // get the 2 CRC bytes
  spi_io(0xff);
  spi_io(0xff);
}


// reads the CID reg from the card
static void MMC_get_CID(unsigned char *ptr_data){
  // select card
	select_cs();
	// tell the MMC card that we want to know its status
	MMC_send_cmd(MMC_CMD_10_SEND_CID,0x0);
	// get the response
	MMC_get_R1();
	// get the register data
	MMC_get_data(ptr_data, 16);
	// cleanup behind us
	MMC_cleanup();
}

// reads the CSD reg from the card
static void MMC_get_CSD(unsigned char *ptr_data){
	// select card
	select_cs();
	// tell the MMC card that we want to know its status
	MMC_send_cmd(MMC_CMD_9_SEND_CSD,0x0);
	// get the response
	MMC_get_R1();
	// get the register data
	MMC_get_data(ptr_data, 16);
	// cleanup behind us
	MMC_cleanup();
}


// returns the :
// 		size of the card in MB ( ret * 1024^2) == bytes
// 		sector count and multiplier MB are in unsigned char == C_SIZE / (2^(9-C_SIZE_MULT))
// 		name of the media
static void MMC_get_volume_info(VOLUME_INFO* vinf){
	unsigned char data[16];
	// read the CSD register
	MMC_get_CSD(data);
	// get the C_SIZE value. bits [73:62] of data
	// [73:72] == data[6] && 0x03
	// [71:64] == data[7]
	// [63:62] == data[8] && 0xc0
	vinf->sector_count = data[6] & 0x03;
	vinf->sector_count <<= 8;
	vinf->sector_count += data[7];
	vinf->sector_count <<= 2;
	vinf->sector_count += (data[8] & 0xc0) >> 6;

	// get the val for C_SIZE_MULT. bits [49:47] of data
	// [49:48] == data[5] && 0x03
	// [47]    == data[4] && 0x80
	vinf->sector_multiply = data[9] & 0x03;
	vinf->sector_multiply <<= 1;
	vinf->sector_multiply += (data[10] & 0x80) >> 7;

	// work out the MBs
	// mega bytes in unsigned char == C_SIZE / (2^(9-C_SIZE_MULT))
	vinf->size    = (vinf->sector_count * 512 )<< (vinf->sector_multiply+2);
	vinf->size_MB = vinf->size >> 20; // MB
	vinf->sector_size = 512;

	// get the name of the card
	MMC_get_CID(data);
	vinf->name[0] = data[3];
	vinf->name[1] = data[4];
	vinf->name[2] = data[5];
	vinf->name[3] = data[6];
	vinf->name[4] = data[7];
	vinf->name[5] = '\0';

}



int
/****************************************************/
mmc_block_read(uchar *dst, ulong src, ulong len)
/****************************************************/
{
	int length = len;
	// select card
	select_cs();
	// tell the MMC card that we want to know its status
	MMC_send_cmd(MMC_CMD_17_READ_SINGLE, src);
	// get the response
	MMC_get_R1();
	// wait till the mmc starts sending data
	MMC_wait_for_start_token(255);

	while(length--){
		*dst = spi_io(0xff);
		dst++;
	}

	// get the 2 CRC bytes
	spi_io(0xff);
	spi_io(0xff);
	// give enough time
	MMC_cleanup();
	return 0;
}



int
/****************************************************/
mmc_block_write(ulong dst, uchar *src, int len)
/****************************************************/
{
	int length = len;

	// select card
	select_cs();
	// tell the MMC card that we want to write a sector
	MMC_send_cmd(MMC_CMD_24_WRITE_SINGLE, dst);
	// get the response
	MMC_get_R1();
	// send the start token
	spi_io(MMC_START_TOKEN_SINGLE);

	while(length--){
		spi_io(*src);
		src++;
	};
	// send 2 crcs
	spi_io(0xff);
	spi_io(0xff);
	// get the data response token
	/*
	can be one of the following :
		MMC_DATA_ACCEPT
		MMC_DATA_CRC
		MMC_DATA_WRITE_ERROR
	*/
	unsigned char tmp =  (spi_io(0xff) & 0xf) >> 1;
	if(tmp != MMC_DATA_ACCEPT){
		printf("mmc_drv.ko : ERROR : MMC_write_sector %d\n", tmp);
		return -1;
	}
	// all ok, wait while busy
	while(spi_io(0xff) == MMC_R1B_BUSY_BYTE) {};

	return 0;
}

int
/****************************************************/
mmc_write(uchar *src, ulong dst, int size)
/****************************************************/
{
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
	ulong mmc_block_size, mmc_block_address;

	if (size == 0) {
		return 0;
	}

	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	dst -= CFG_MMC_BASE;
	end = dst + size;
	part_start = ~mmc_block_address & dst;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & dst;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		(ulong)src, dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(mmc_buf+part_start, src, part_len);
		if ((mmc_block_write(aligned_start, mmc_buf, mmc_block_size)) < 0) {
			return -1;
		}
		dst += part_len;
		src += part_len;
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; dst < aligned_end; src += mmc_block_size, dst += mmc_block_size) {
		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_write(dst, (uchar *)src, mmc_block_size)) < 0) {
			return -1;
		}
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && dst < end) {
		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(mmc_buf, src, part_end);
		if ((mmc_block_write(aligned_end, mmc_buf, mmc_block_size)) < 0) {
			return -1;
		}
	}
	return 0;
}


int
/****************************************************/
mmc_read(ulong src, uchar *dst, int size)
/****************************************************/
{
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
	ulong mmc_block_size, mmc_block_address;

	if (size == 0) {
		return 0;
	}

	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	src -= CFG_MMC_BASE;
	end = src + size;
	part_start = ~mmc_block_address & src;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & src;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(dst, mmc_buf+part_start, part_len);
		dst += part_len;
		src += part_len;
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; src < aligned_end; src += mmc_block_size, dst += mmc_block_size) {
		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read((uchar *)(dst), src, mmc_block_size)) < 0) {
			return -1;
		}
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && src < end) {
		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(dst, mmc_buf, part_end);
	}
	return 0;
}


ulong
/****************************************************/
mmc_bread(int dev_num, ulong blknr, ulong blkcnt, ulong *dst)
/****************************************************/
{
	int mmc_block_size = MMC_BLOCK_SIZE;
	ulong src = blknr * mmc_block_size + CFG_MMC_BASE;

	mmc_read(src, (uchar *)dst, blkcnt*mmc_block_size);
	return blkcnt;
}


int
/****************************************************/
mmc_init(int verbose)
/****************************************************/
{
	int rc =0;

	unsigned char i, j;
	unsigned char res;

	puts("Bitbanging mmc/sd card driver (50KiB/sec)\n");
	for(j = 0; j < 4; j++){

		deselect_cs();
		// the data sheet says that the MMC needs 74 clock pulses to startup
		for(i = 0; i < 100; i++){
			SPI_clock();
			udelay(100);
		};

		udelay(10000);
		// select card
		select_cs();
		// put MMC in idle
		MMC_send_cmd(MMC_CMD_0_GO_IDLE,0x0);
		// get the response
		res = MMC_get_R1();

		printf("mmc_init : response : %d\n", res);
		if(res == 1){
			j = 100;
		}
	}

	if(res != 0x01){
		// we need to indicate the exact error
		if(res == 0xff){
			printf("mmc_init : card not found\n");
			return 1;
		} else {
			printf("mmc_init : invalid response\n");
			return 2;
		};

	};
	printf("mmc_init : Card Found\n");

	while(res==0x01){
		// deselect card
		deselect_cs();
		// send 8 clock pulses
		spi_io(0xff);
		// select card
		select_cs();
		// send wake up signal s.t. MMC leaves idle state and switches to operation mode
		MMC_send_cmd(MMC_CMD_1_SEND_OP_COND,0x0);
		// get response
		res = MMC_get_R1();
	};
	// cleanup behind us
	MMC_cleanup();

	VOLUME_INFO vinf;
	MMC_get_volume_info(&vinf);
	printf("mmc_drv.ko : SIZE : %d,(%d MB), nMUL : %d, COUNT : %d, NAME : %s\n", vinf.size,vinf.size_MB, vinf.sector_multiply, vinf.sector_count, vinf.name);
	mmc_size = vinf.size;

	/* fill in device description */
	mmc_dev.if_type = IF_TYPE_MMC;
	mmc_dev.part_type = PART_TYPE_DOS;
	mmc_dev.dev = 0;
	mmc_dev.lun = 0;
	mmc_dev.type = 0;
	/* FIXME fill in the correct size (is set to 32MByte) */
	mmc_dev.blksz = 512;
	mmc_dev.lba = vinf.size >> 9;
	mmc_dev.removable = 0;
	mmc_dev.block_read = mmc_bread;
	mmc_ready=1;

	fat_register_device(&mmc_dev,1); /* partitions start counting with 1 */

	return rc;
}



int
mmc_ident(block_dev_desc_t *dev)
{
	puts("called mmc_ident\n");
	return 0;
}

int

mmc2info(ulong addr)
{
	/* FIXME hard codes to 32 MB device */
	if (addr >= CFG_MMC_BASE && addr < CFG_MMC_BASE + mmc_size) {
		return 1;
	}
	return 0;
}


#endif
