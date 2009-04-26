/*------------------------------------------------------------------------------+ */
/* */
/*	 This source code has been made available to you by IBM on an AS-IS */
/*	 basis.  Anyone receiving this source is licensed under IBM */
/*	 copyrights to use it in any way he or she deems fit, including */
/*	 copying it, modifying it, compiling it, and redistributing it either */
/*	 with or without modifications.  No license under IBM patents or */
/*	 patent applications is to be implied by the copyright license. */
/* */
/*	 Any user of this software should understand that IBM cannot provide */
/*	 technical support for this software and will not be responsible for */
/*	 any consequences resulting from the use of this software. */
/* */
/*	 Any person who transfers this source code or any derivative work */
/*	 must include the IBM copyright notice, this paragraph, and the */
/*	 preceding two paragraphs in the transferred software. */
/* */
/*	 COPYRIGHT   I B M   CORPORATION 1995 */
/*	 LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M */
/*------------------------------------------------------------------------------- */

#include <config.h>
#include <ppc4xx.h>
#include <ppc_asm.tmpl>


/*----------------------------------------------------------------------------- */
/* Function:	 ext_bus_cntlr_init */
/* Description:  Initializes the External Bus Controller for the external */
/*		peripherals. IMPORTANT: For pass1 this code must run from */
/*		cache since you can not reliably change a peripheral banks */
/*		timing register (pbxap) while running code from that bank. */
/*		For ex., since we are running from ROM on bank 0, we can NOT */
/*		execute the code that modifies bank 0 timings from ROM, so */
/*		we run it from cache. */
/*	Bank 0 - Flash */
/*	Bank 1 - ASIC */
/*	Bank 2 - IDE */
/*	Bank 3 - IDE */
/*	Bank 4 - usb client */
/*	Bank 5 - ASIC */
/*	Bank 6 - dm9000, ethernet chip */
/*	Bank 7 - HEADER. J2. May be SMART CARD slot. */
/*		No.. Bank 7 is used for IDE line mode DMA. */
/*		Cannot be used for any other purpose */
/*----------------------------------------------------------------------------- */
#define EBIU_BRCRH0	0x00000000
#define EBIU_BRCRH1	0x00000000
#define EBIU_BRCRH2	0x40000000
#define EBIU_BRCRH3	0x40000000
#define EBIU_BRCRH4	0x00000000
#define EBIU_BRCRH5	0x40000000
#define EBIU_BRCRH6	0x40000000
#define EBIU_BRCRH7	0x40000000

#define BAS(val)	(val<<(31-7))
#define BS(val)		(val<<(31-10))
#define BU(val)		(val<<(31-12))
#define SLF(val)	(val<<(31-13))
#define BME(val)	(val<<(31-14))
#define BW(val)		(val<<(31-16))
#define RE(val)		(val<<(31-17))
#define TWT(val)	(val<<(31-23))
#define CSN(val)	(val<<(31-24))
#define OEN(val)	(val<<(31-25))
#define WBN(val)	(val<<(31-26))
#define WBF(val)	(val<<(31-27))
#define TH(val)		(val<<(31-30))
#define BRS(val)	(val<<(31-31))

#define EBIU_BRCR0	(BAS(0xf0)|BS(0x4)|BU(0x3)|SLF(0x0)|BME(0x0)|BW(0x1)|RE(0x0)|TWT(0x08)|CSN(0x1)|OEN(0x0)|WBN(0x0)|WBF(0x1)|TH(0x2)|BRS(0x0))
#define EBIU_BRCR1	(BAS(0x10)|BS(0x3)|BU(0x3)|SLF(0x0)|BME(0x0)|BW(0x1)|RE(0x1)|TWT(0x0f)|CSN(0x1)|OEN(0x1)|WBN(0x1)|WBF(0x0)|TH(0x7)|BRS(0x0))	//0x1078cfee
#define EBIU_BRCR2	(BAS(0x20)|BS(0x3)|BU(0x3)|SLF(0x1)|BME(0x0)|BW(0x1)|RE(0x1)|TWT(0x0F)|CSN(0x1)|OEN(0x0)|WBN(0x1)|WBF(0x1)|TH(0x7)|BRS(0x0))	//0x207ccfbe
#define EBIU_BRCR3	(BAS(0x40)|BS(0x3)|BU(0x3)|SLF(0x1)|BME(0x0)|BW(0x1)|RE(0x1)|TWT(0x2F)|CSN(0x1)|OEN(0x1)|WBN(0x1)|WBF(0x1)|TH(0x7)|BRS(0x0))	//0x407cfffe
#define EBIU_BRCR4	(BAS(0xef)|BS(0x0)|BU(0x3)|SLF(0x0)|BME(0x0)|BW(0x0)|RE(0x0)|TWT(0x1f)|CSN(0x1)|OEN(0x1)|WBN(0x1)|WBF(0x1)|TH(0x7)|BRS(0x1))
#define EBIU_BRCR5	(BAS(0xed)|BS(0x0)|BU(0x3)|SLF(0x0)|BME(0x0)|BW(0x0)|RE(0x0)|TWT(0x02)|CSN(0x0)|OEN(0x0)|WBN(0x0)|WBF(0x0)|TH(0x1)|BRS(0x1))	//0x507ca0ef
#define EBIU_BRCR6	(BAS(0xee)|BS(0x0)|BU(0x3)|SLF(0x1)|BME(0x0)|BW(0x1)|RE(0x1)|TWT(0x10)|CSN(0x1)|OEN(0x0)|WBN(0x1)|WBF(0x1)|TH(0x7)|BRS(0x1))	//0xf61cd0bf
#define EBIU_BRCR7	(BAS(0xCE)|BS(0x1)|BU(0x3)|SLF(0x1)|BME(0x1)|BW(0x2)|RE(0x0)|TWT(0x00)|CSN(0x0)|OEN(0x0)|WBN(0x0)|WBF(0x0)|TH(0x1)|BRS(0x1))	//0xce3f0003

	.globl	ext_bus_cntlr_init
ext_bus_cntlr_init:
	///////////////////////////////////////////////
	// SysClk 63MHz.
	// 15.87 nano second per SysClk.
	///////////////////////////////////////////////

	addis	r0,0,EBIU_BRCRH0@h
	ori	r0,r0,EBIU_BRCRH0@l
	mtdcr	ebiu0_brcrh0,r0
	addis	r0,0,EBIU_BRCR0@h
	ori	r0,r0,EBIU_BRCR0@l
	mtdcr	ebiu0_brcr0,r0

	addis	r0,0,EBIU_BRCRH1@h
	ori	r0,r0,EBIU_BRCRH1@l
	mtdcr	ebiu0_brcrh1,r0
	addis	r0,0,EBIU_BRCR1@h
	ori	r0,r0,EBIU_BRCR1@l
	mtdcr	ebiu0_brcr1,r0

	addis	r0,0,EBIU_BRCRH2@h
	ori	r0,r0,EBIU_BRCRH2@l
	mtdcr	ebiu0_brcrh2,r0
	addis	r0,0,EBIU_BRCR2@h
	ori	r0,r0,EBIU_BRCR2@l
	mtdcr	ebiu0_brcr2,r0

	addis	r0,0,EBIU_BRCRH3@h
	ori	r0,r0,EBIU_BRCRH3@l
	mtdcr	ebiu0_brcrh3,r0
	addis	r0,0,EBIU_BRCR3@h
	ori	r0,r0,EBIU_BRCR3@l
	mtdcr	ebiu0_brcr3,r0

	addis	r0,0,EBIU_BRCRH4@h
	ori	r0,r0,EBIU_BRCRH4@l
	mtdcr	ebiu0_brcrh4,r0
	addis	r0,0,EBIU_BRCR4@h
	ori	r0,r0,EBIU_BRCR4@l
	mtdcr	ebiu0_brcr4,r0

	addis	r0,0,EBIU_BRCRH5@h
	ori	r0,r0,EBIU_BRCRH5@l
	mtdcr	ebiu0_brcrh5,r0
	addis	r0,0,EBIU_BRCR5@h
	ori	r0,r0,EBIU_BRCR5@l
	mtdcr	ebiu0_brcr5,r0

	addis	r0,0,EBIU_BRCRH6@h
	ori	r0,r0,EBIU_BRCRH6@l
	mtdcr	ebiu0_brcrh6,r0
	addis	r0,0,EBIU_BRCR6@h
	ori	r0,r0,EBIU_BRCR6@l
	mtdcr	ebiu0_brcr6,r0

	addis	r0,0,EBIU_BRCRH7@h
	ori	r0,r0,EBIU_BRCRH7@l
	mtdcr	ebiu0_brcrh7,r0
	addis	r0,0,EBIU_BRCR7@h
	ori	r0,r0,EBIU_BRCR7@l
	mtdcr	ebiu0_brcr7,r0

	blr

/*----------------------------------------------------------------------------- */
/* Function:	 sdram_init */
/* Description:  Dummy implementation here - done in C later */
/*----------------------------------------------------------------------------- */
	.globl	sdram_init
sdram_init:
	/*
	 * SDRAM0
	 */
	addis	r0,0,0
	mtdcr	sdram0_besr,r0	// reset/unlock besr

	addis	r0, 0,0x0000
	ori	r0,r0,0x4010	// 32MByte
	mtdcr	sdram0_br0,r0

	addis	r0, 0,0x0200
	ori	r0,r0,0x4010
	mtdcr	sdram0_br1,r0

	addis	r0, 0, 0x01f0
	ori	r0,r0, 0x0000
	mtdcr	sdram0_cr0,r0	// 32MByte

	addis	r0, 0, 0x01f0
	ori	r0,r0, 0x0000
	mtdcr	sdram0_cr1,r0

	mfdcr	r0,sdram0_cr0
	ori	r0,r0,0x8000	// Enable SDRAM Controller
	mtdcr	sdram0_cr0,r0
	mfdcr	r0,sdram0_cr1
	ori	r0,r0,0x8000	// Enable SDRAM Controller
	mtdcr	sdram0_cr1,r0


	/*
	 * SDRAM1
	 */
	addis	r0,0,0
	mtdcr	sdram1_besr,r0	// reset/unlock besr

	addis	r0, 0,0xa000
	ori	r0,r0,0x4010	// 32MByte
	mtdcr	sdram1_br0,r0

	addis	r0,0,0
	mtdcr	sdram1_br1,r0

	addis	r0, 0, 0x01f0
	ori	r0,r0, 0x0000
	mtdcr	sdram1_cr0,r0	// 0MByte

	addis	r0,0,0
	mtdcr	sdram1_cr1,r0

	mfdcr	r0,sdram1_cr0
	ori	r0,r0,0x8000	// Enable SDRAM Controller
	mtdcr	sdram1_cr0,r0

	blr

/*----------------------------------------------------------------------------- */
/* Function:	 gpio_init */
/* Description:  initialize gpio port */
/*----------------------------------------------------------------------------- */

/*
00000000001111111111222222222233
01234567890123456789012345678901

00010000000000000000000000000000					O
11011000011111111011111111100111					TC
11000000000000000000000000000000					OD


0000000000000000000011111111111111111111222222222222222222223333
0011223344556677889900112233445566778899001122334455667788990011
##||##||##||##||##||##||##||##||##||##||##||##||##||##||##||##||
0000110001010000010100000000000001010000101000011001010000010101	OS
0000000001000000010100000000000001010000101000011001010000010101	TS
0000000000010000010000000000000000010000000000000000000000000000	IS0
0000010000000000000000000000000000000000000000000000000000000000	IS1
0000000000000000000000000000000000000000000000000000000000000000	IS3
*/

#define GPIO_O		0x00
#define GPIO_TC		0x04
#define GPIO_OS0	0x08
#define GPIO_OS1	0x0c
#define GPIO_TS0	0x10
#define GPIO_TS1	0x14
#define GPIO_OD		0x18
#define GPIO_I		0x1c
#define GPIO_IS10	0x30
#define GPIO_IS11	0x34
#define GPIO_IS20	0x38
#define GPIO_IS21	0x3c
#define GPIO_IS30	0x40
#define GPIO_IS31	0x44

#define VGPIO_O		0x10000000
#define VGPIO_TC	0xd87fbfe7
#define VGPIO_OS0	0x0c505000
#define VGPIO_OS1	0x50a19415
#define VGPIO_TS0	0x00405000
#define VGPIO_TS1	0x50a19415
#define VGPIO_OD	0xc0000000
#define VGPIO_IS10	0x00104000
#define VGPIO_IS11	0x10000000
#define VGPIO_IS20	0x04000000
#define VGPIO_IS21	0x00000000
#define VGPIO_IS30	0x00000000
#define VGPIO_IS31	0x00000000

	.globl	gpio_init
gpio_init:

	/*--------------------------------------------------------------------+
	| GPIO 0
	+--------------------------------------------------------------------*/
	addis	r4,r0,0x4006	       /* GPIO 0 */

	addis	r0,r0,VGPIO_O@h
	ori	r0,r0,VGPIO_O@l
	stw	r0,GPIO_O(r4)		/* GPO */

	addis	r0,r0,VGPIO_OS0@h
	ori	r0,r0,VGPIO_OS0@l
	stw	r0,GPIO_OS0(r4)		/* GPOS 00:31 */
	addis	r0,r0,VGPIO_OS1@h
	ori	r0,r0,VGPIO_OS1@l
	stw	r0,GPIO_OS1(r4)		/* GPOS 32:63 */

	addis	r0,r0,VGPIO_TS0@h
	ori	r0,r0,VGPIO_TS0@l
	stw	r0,GPIO_TS0(r4)		/* GPTS 00:31 */
	addis	r0,r0,VGPIO_TS1@h
	ori	r0,r0,VGPIO_TS1@l
	stw	r0,GPIO_TS1(r4)		/* GPTS 32:63 */

	addis	r0,r0,VGPIO_TC@h
	ori	r0,r0,VGPIO_TC@l
	stw	r0,GPIO_TC(r4)		/* GPTC */

	addis	r0,r0,VGPIO_OD@h
	ori	r0,r0,VGPIO_OD@l
	stw	r0,GPIO_OD(r4)		/* GPOD */

	addis	r0,r0,VGPIO_IS10@h
	ori	r0,r0,VGPIO_IS10@l
	stw	r0,GPIO_IS10(r4)	/* GPIS1 00:31 */
	addis	r0,r0,VGPIO_IS11@h
	ori	r0,r0,VGPIO_IS11@l
	stw	r0,GPIO_IS11(r4)	/* GPIS1 32:63 */

	addis	r0,r0,VGPIO_IS20@h
	ori	r0,r0,VGPIO_IS20@l
	stw	r0,GPIO_IS20(r4)	/* GPIS2 00:31 */
	addis	r0,r0,VGPIO_IS21@h
	ori	r0,r0,VGPIO_IS21@l
	stw	r0,GPIO_IS21(r4)	/* GPIS2 32:63 */

	addis	r0,r0,VGPIO_IS30@h
	ori	r0,r0,VGPIO_IS30@l
	stw	r0,GPIO_IS30(r4)	/* GPIS3 00:31 */
	addis	r0,r0,VGPIO_IS31@h
	ori	r0,r0,VGPIO_IS31@l
	stw	r0,GPIO_IS31(r4)	/* GPIS3 32:63 */

	blr

/*----------------------------------------------------------------------------- */
/* Function:	 chip_control_init */
/* Description:  initialize chip control register */
/*----------------------------------------------------------------------------- */
	.globl	chip_control_init
chip_control_init:
	// Tristate for other masters, diable ready time outs
	//
	xor	r0, r0, r0
	oris	r1, r0,0x8000
	mtdcr	biucr, r1


	//
	// Set up default chip interconnect
	//
	lis	r3,0xC94B
	ori	r3,r3,0xEDEE
	//mtdcr	cic0_sccr,r3
	addis	r0,r0,0x1080
	ori	r0,r0,0x0003
	mtdcr	cic0_cr,r0
	addi	r0,r0,0x0000
	mtdcr	cic0_sel3,r0
	addis	r0,r0,0x004A
	ori	r0,r0,0x0000
	mtdcr	cic0_vcr,r0

	//Initialize the Cross-bar switch
	addis	r1,r0,0x0000
	ori	r1,r1,0x0002
	mtdcr	cbscr,r1

	//Set the hidden dcr
	addis	r1,r0,0x4000
	ori	r1,r1,0x2000
	mtdcr	dcr_34,r1

	//-----------------------------------------------------------------------
	// A few important regs first, segmet guard, d-cache write through
	//-----------------------------------------------------------------------
	addi	r0,0,0
	mtsgr	r0
	mtdcwr	r0

	//-----------------------------------------------------------------------
	// Set up some machine state registers.
	//-----------------------------------------------------------------------
	addi	r4, r0,0x0000		// initialize r4 to zero
	mtspr	esr, r4			// clear Exception Syndrome Reg
	mttcr	r4			// timer control register
	addis	r4, r0,0xFFFF		// set r4 to 0xFFFFFFFF (status in the
	ori	r4, r4,0xFFFF		// dbsr is cleared by setting bits to 1)
	mtdbsr	r4			// clear/reset the dbsr
	mtdcr	besr, r4		// clear Bus Error Syndrome Reg

	//-----------------------------------------------------------------------
	// Clear reservation bit.
	//-----------------------------------------------------------------------
	addis	r10, r0,0x0000
	lwarx	r3, r10, r10		// get some data/set resv bit
	stwcx.	r3, r10, r10		// store out and clear resv bit

	//-----------------------------------------------------------------------
	// Clear XER.
	//-----------------------------------------------------------------------
	addis	r0, r0,0x0000
	mtxer	r0

	//-----------------------------------------------------------------------
	// Mask off all interrupts in the universal interrupt controller
	//-----------------------------------------------------------------------
	// this will done again in linux kernel initialize routins...
	//  especially, in arch/ppc/platforms/4xx/relook400.c in relook400 case.
	//
	addis	r1, r0,0xffff
	ori	r1, r1,0xffff
	mtdcr	uicsr, r1

	addi	r0,0,0
	mtdcr	uicer, r0

	addis	r1, r0,0x0000
	ori	r1, r1,0x0000
	mtdcr	uictr, r1

	addis	r1, r0,0xffff
	ori	r1, r1,0x8fa0
	mtdcr	uicpr, r1

	blr

