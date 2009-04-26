/*------------------------------------------------------------------------------+ */
/* */
/*       This source code has been made available to you by IBM on an AS-IS */
/*       basis.  Anyone receiving this source is licensed under IBM */
/*       copyrights to use it in any way he or she deems fit, including */
/*       copying it, modifying it, compiling it, and redistributing it either */
/*       with or without modifications.  No license under IBM patents or */
/*       patent applications is to be implied by the copyright license. */
/* */
/*       Any user of this software should understand that IBM cannot provide */
/*       technical support for this software and will not be responsible for */
/*       any consequences resulting from the use of this software. */
/* */
/*       Any person who transfers this source code or any derivative work */
/*       must include the IBM copyright notice, this paragraph, and the */
/*       preceding two paragraphs in the transferred software. */
/* */
/*       COPYRIGHT   I B M   CORPORATION 1995 */
/*       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M */
/*------------------------------------------------------------------------------- */

/*----------------------------------------------------------------------------- */
/* Function:     ext_bus_cntlr_init */
/* Description:  Initializes the External Bus Controller for the external */
/*		peripherals. IMPORTANT: For pass1 this code must run from */
/*		cache since you can not reliably change a peripheral banks */
/*		timing register (pbxap) while running code from that bank. */
/*		For ex., since we are running from ROM on bank 0, we can NOT */
/*		execute the code that modifies bank 0 timings from ROM, so */
/*		we run it from cache. */
/*	Bank 0 - Flash */
/*	Bank 1 - not used */
/*	Bank 2 - dm9000, ethernet chip */
/*	Bank 3 - not used */
/*	Bank 4 - usb client */
/*	Bank 5 - not used */
/*	Bank 6 - not used */
/*	Bank 7 - not used */
/*----------------------------------------------------------------------------- */
#include <config.h>
#include <ppc4xx.h>
#include <ppc_asm.tmpl>

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


#define EBIU_BRCRH0	0x00000000
#define EBIU_BRCRH1	0x00000000
#define EBIU_BRCRH2	0x40000000
#define EBIU_BRCRH3	0x40000000
#define EBIU_BRCRH4	0x00000000
#define EBIU_BRCRH5	0x00000000
#define EBIU_BRCRH6	0x00000000
#define EBIU_BRCRH7	0x40000000

#define EBIU_BRCR0	(BAS(0xf8)|BS(0x3)|BU(0x3)|SLF(0x0)|BME(0x0)|BW(0x1)|RE(0x0)|TWT(0x06)|CSN(0x1)|OEN(0x0)|WBN(0x0)|WBF(0x1)|TH(0x2)|BRS(0x0))
#define EBIU_BRCR1	(BAS(0x10)|BS(0x3)|BU(0x3)|SLF(0x0)|BME(0x0)|BW(0x1)|RE(0x1)|TWT(0x0f)|CSN(0x1)|OEN(0x1)|WBN(0x1)|WBF(0x0)|TH(0x7)|BRS(0x0))
#define EBIU_BRCR2	0x207ccfbe
#define EBIU_BRCR3	0x407cfffe
#define EBIU_BRCR4	0xf51898ff
#define EBIU_BRCR5	0x507ca0ef
#define EBIU_BRCR6	0xff00bfff
#define EBIU_BRCR7	0xce3f0003

     	.globl	ext_bus_cntlr_init
ext_bus_cntlr_init:
	///////////////////////////////////////////////
	// SysClk 54MHz.
	// 18.51 nano second per SysClk.
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

	addis	r0,r0,0x8000
	ori	r0,r0,0x0000
	mtdcr	ebiu0_biucr,r0

        blr

/*----------------------------------------------------------------------------- */
/* Function:     sdram_init */
/* Description:  Dummy implementation here - done in C later */
/*----------------------------------------------------------------------------- */
        .globl  sdram_init
sdram_init:
	/*-------------------------------------------------------------------------+
	| CONFIGURE THE HSMC CONTROLLERS
	| Double map HSMC into 0x00000000/0x80000000  0x20000000/0xA0000000
	+-------------------------------------------------------------------------*/
                                  /* Configure HSMC 0 - bank 0*/
        /*bl ..bankinit2*/
        addis   r0,r0,0x6c00
        ori     r0,r0,0x0000
        mtdcr   hsmc1_gr,r0

        addis   r0,r0,0x200d
        ori     r0,r0,0x2D55
        mtdcr   hsmc1_br0,r0

        addis   r0,r0,0x8077
        ori     r0,r0,0x0000
        mtdcr   hsmc1_cr0,r0

hsmc1_0:
        mfdcr   r3,hsmc1_cr0
        addis   r4,r0,0x0007
        and.    r4,r3,r4
        bne     hsmc1_0

        addis   r0,r0,0x8078
        ori     r0,r0,0x0000
        mtdcr   hsmc1_cr0,r0

hsmc1_1:
        mfdcr   r3,hsmc1_cr0
        addis   r4,r0,0x0008
        and.    r4,r3,r4
        bne     hsmc1_1

        addis   r0,r0,0x0037
        ori     r0,r0,0x0000
        mtdcr   hsmc1_data,r0

        addis   r0,r0,0x8070
        ori     r0,r0,0x8000
        mtdcr   hsmc1_cr0,r0

hsmc1_2:
        mfdcr   r3,hsmc1_cr0
        addis   r4,r0,0x0000
        ori     r4,r4,0x8000
        and.    r4,r3,r4
        bne     hsmc1_2

                                  /* Configure HSMC 0 - bank 1 */
        addis   r0,r0,0x208d
        ori     r0,r0,0x2D55
        mtdcr   hsmc1_br1,r0

        addis   r0,r0,0x8077
        ori     r0,r0,0x0000
        mtdcr   hsmc1_cr1,r0

hsmc1_3:
        mfdcr   r3,hsmc1_cr1
        addis   r4,r0,0x0007
        and.    r4,r3,r4
        bne     hsmc1_3

        addis   r0,r0,0x8078
        ori     r0,r0,0x0000
        mtdcr   hsmc1_cr1,r0

hsmc1_4:
        mfdcr   r3,hsmc1_cr1
        addis   r4,r0,0x0008
        and.    r4,r3,r4
        bne     hsmc1_4

        addis   r0,r0,0x0037
        ori     r0,r0,0x0000
        mtdcr   hsmc1_data,r0

        addis   r0,r0,0x8070
        ori     r0,r0,0x8000
        mtdcr   hsmc1_cr1,r0

hsmc1_5:
        mfdcr   r3,hsmc1_cr1
        addis   r4,r0,0x0000
        ori     r4,r4,0x8000
        and.    r4,r3,r4
        bne     hsmc1_5

        addis   r0,r0,0x0FE1
        mtdcr   hsmc1_crr,r0

                                  /* Configure HSMC 0 - bank 0*/
        addis   r0,r0,0x6c00
        ori     r0,r0,0x0000
        mtdcr   hsmc0_gr,r0

        addis   r0,r0,0x000d
        ori     r0,r0,0x2D55
        mtdcr   hsmc0_br0,r0

        addis   r0,r0,0x8077
        ori     r0,r0,0x0000
        mtdcr   hsmc0_cr0,r0

hsmc0_0:
        mfdcr   r3,hsmc0_cr0
        addis   r4,r0,0x0007
        and.    r4,r3,r4
        bne     hsmc0_0

        addis   r0,r0,0x8078
        ori     r0,r0,0x0000
        mtdcr   hsmc0_cr0,r0

hsmc0_1:
        mfdcr   r3,hsmc0_cr0
        addis   r4,r0,0x0008
        and.    r4,r3,r4
        bne     hsmc0_1

        addis   r0,r0,0x0037
        ori     r0,r0,0x0000
        mtdcr   hsmc0_data,r0

        addis   r0,r0,0x8070
        ori     r0,r0,0x8000
        mtdcr   hsmc0_cr0,r0

hsmc0_2:
        mfdcr   r3,hsmc0_cr0
        addis   r4,r0,0x0000
        ori     r4,r4,0x8000
        and.    r4,r3,r4
        bne     hsmc0_2

                                  /* Configure HSMC 0 - bank 1 */
        addis   r0,r0,0x008d
        ori     r0,r0,0x2D55
        mtdcr   hsmc0_br1,r0

        addis   r0,r0,0x8077
        ori     r0,r0,0x0000
        mtdcr   hsmc0_cr1,r0

hsmc0_3:
        mfdcr   r3,hsmc0_cr1
        addis   r4,r0,0x0007
        and.    r4,r3,r4
        bne     hsmc0_3

        addis   r0,r0,0x8078
        ori     r0,r0,0x0000
        mtdcr   hsmc0_cr1,r0

hsmc0_4:
        mfdcr   r3,hsmc0_cr1
        addis   r4,r0,0x0008
        and.    r4,r3,r4
        bne     hsmc0_4

        addis   r0,r0,0x0037
        ori     r0,r0,0x0000
        mtdcr   hsmc0_data,r0

        addis   r0,r0,0x8070
        ori     r0,r0,0x8000
        mtdcr   hsmc0_cr1,r0

hsmc0_5:
        mfdcr   r3,hsmc0_cr1
        addis   r4,r0,0x0000
        ori     r4,r4,0x8000
        and.    r4,r3,r4
        bne     hsmc0_5

        addis   r0,r0,0x0FE1
        mtdcr   hsmc0_crr,r0

	/*--------------------------------------------------------------------+
	| Clear	reservation bit.
	+--------------------------------------------------------------------*/
#if 1
        addis   r10,r0,0x0000
        lwarx   r3,r10,r10             /* get some data/set resv bit          */
        stwcx.  r3,r10,r10             /* store out and clear resv bit        */
#endif
        blr

/*----------------------------------------------------------------------------- */
/* Function:     gpio_init */
/* Description:  initialize gpio port */
/*----------------------------------------------------------------------------- */
#define AGPIO0_OR	0x00
#define AGPIO0_TCR	0x04
#define AGPIO0_OS0R	0x08
#define AGPIO0_OS1R	0x0c
#define AGPIO0_TS0R	0x10
#define AGPIO0_TS1R	0x14
#define AGPIO0_ODR	0x18
#define AGPIO0_IR	0x1c
#define AGPIO0_IS10R	0x30
#define AGPIO0_IS11R	0x34
#define AGPIO0_IS20R	0x38
#define AGPIO0_IS21R	0x3c
#define AGPIO0_IS30R	0x40
#define AGPIO0_IS31R	0x44

#define VGPIO0_OR	0x00800200
#define VGPIO0_TCR	0x33e03ff0
#define VGPIO0_OS0R	0x50a00155
#define VGPIO0_OS1R	0xa000002f
#define VGPIO0_TS0R	0x50a00155
#define VGPIO0_TS1R	0xa000002f
#define VGPIO0_ODR	0xc0000000
#define VGPIO0_IS10R	0x50054155
#define VGPIO0_IS11R	0x00000000
#define VGPIO0_IS20R	0x00000000
#define VGPIO0_IS21R	0x00000000
#define VGPIO0_IS30R	0x00000440
#define VGPIO0_IS31R	0x00000000

	.globl  gpio_init
gpio_init:

	/*--------------------------------------------------------------------+
	| JFH GPIO 0
	+--------------------------------------------------------------------*/
	addis   r4,r0,0x4006           /* GPIO 0 */

	addis   r0,r0,VGPIO0_OS0R@h
	ori     r0,r0,VGPIO0_OS0R@l
	stw     r0,AGPIO0_OS0R(r4)
	addis   r0,r0,VGPIO0_OS1R@h
	ori     r0,r0,VGPIO0_OS1R@l
	stw     r0,AGPIO0_OS1R(r4)
	addis   r0,r0,VGPIO0_TS0R@h
	ori     r0,r0,VGPIO0_TS0R@l
	stw     r0,AGPIO0_TS0R(r4)
	addis   r0,r0,VGPIO0_TS1R@h
	ori     r0,r0,VGPIO0_TS1R@l
	stw     r0,AGPIO0_TS1R(r4)
	addis   r0,r0,VGPIO0_ODR@h
	ori     r0,r0,VGPIO0_ODR@l
	stw     r0,AGPIO0_ODR(r4)
	addis   r0,r0,VGPIO0_TCR@h
	ori     r0,r0,VGPIO0_TCR@l
	stw     r0,AGPIO0_TCR(r4)
	addis   r0,r0,VGPIO0_OR@h
	ori     r0,r0,VGPIO0_OR@l
	stw     r0,AGPIO0_OR(r4)

	addis   r0,r0,VGPIO0_IS10R@h
	ori     r0,r0,VGPIO0_IS10R@l
	stw     r0,AGPIO0_IS10R(r4)
	addis   r0,r0,VGPIO0_IS11R@h
	ori     r0,r0,VGPIO0_IS11R@l
	stw     r0,AGPIO0_IS11R(r4)
	addis   r0,r0,VGPIO0_IS20R@h
	ori     r0,r0,VGPIO0_IS20R@l
	stw     r0,AGPIO0_IS20R(r4)
	addis   r0,r0,VGPIO0_IS21R@h
	ori     r0,r0,VGPIO0_IS21R@l
	stw     r0,AGPIO0_IS21R(r4)
	addis   r0,r0,VGPIO0_IS30R@h
	ori     r0,r0,VGPIO0_IS30R@l
	stw     r0,AGPIO0_IS30R(r4)
	addis   r0,r0,VGPIO0_IS31R@h
	ori     r0,r0,VGPIO0_IS31R@l
	stw     r0,AGPIO0_IS31R(r4)

	/* for harddisk reset */
	addis	r0, r0, 0x0010
	ori	r0, r0, 0x0000
	mtdcr	sgptc, r0
	addis	r0, r0, 0x0000
	ori	r0, r0, 0x0000
	mtdcr	sgpod, r0
	addis	r0, r0, 0x0000
	ori	r0, r0, 0x0000
	mtdcr	sgpo, r0

	blr

/*----------------------------------------------------------------------------- */
/* Function:     chip_control_init */
/* Description:  initialize chip control register */
/*----------------------------------------------------------------------------- */
	.globl  chip_control_init
chip_control_init:
	/*
         * Initialize Storage Guarded and Data Cache Write-thru registers
         * Initialize Interrupt Save/Restore registers.
         */

        addi    r0,0,0
        mtspr   sgr,r0
        mtspr   dcwr,r0
        mtspr   srr0,r0
        mtspr   srr1,r0
        mtspr   srr2,r0
        mtspr   srr3,r0
        
 
	/*--------------------------------------------------------------------+
	| Clear	SGR and	DCWR (403GC, 403GCX, 401 cores).
	+--------------------------------------------------------------------*/
	addi	r4,r0,0x0000
	mtsgr	r4
	mtdcwr	r4

	addis	r4,r0,0xa080
	ori	r4,r4,0x0000
	mtspr	evpr,r4

	/*--------------------------------------------------------------------+
	| Clear/set up some machine state registers.
	+--------------------------------------------------------------------*/
	addi	r4,r0,0x0000		/* initialize r4 to zero	     */
	mtdcr	ebiu0_besr,r4		/* clear Bus Error Syndrome Reg	     */
	mtspr	esr,r4			/* clear Exception Syndrome Reg	     */
	mttcr	r4			/* timer control register	     */
	mtdcr	uic0_er,r4		/* disable all interrupts	     */
	addis	r4,r0,0xFFFF		/* set r4 to 0xFFFFFFFF	(status	in   */
	ori	r4,r4,0xFFFF		/* dbsr	is clered by writing 1)	     */
	mtdbsr	r4			/* clear/reset the dbsr		     */
	mtdcr	uic0_sr,r4		/* clear all pending interrupts	     */
	addi	r4,r0,0x0000	/* was 1000*/	/* set ME bit (Machine Exceptions)   */
	oris	r4,r4,0x0000		/*was 2*//* set CE bit (Critical	Exceptions)  */
	mtmsr	r4			/* change MSR			     */


	/*--------------------------------------------------------------------+
	| Clear	XER.
	+--------------------------------------------------------------------*/
	addis	r0,r0,0x0000
	mtxer	r0
	
	/*
	* Invalidate i-cache and d-cache TAG arrays.
	*/

        addi    r7,r0,128              /* set r7 to # of lines in data cache  */
                                       /* for loop count- vesta has 128 lines */
..dcache:
        addi    r6,0,0x0000            /* clear GPR 6                         */
        mtctr   r7                     /* set loop ctr                        */
..dcloop:
        dccci   0,r6                   /* invalidate line                     */
        addi    r6,r6,0x20             /* bump to next line                   */
        bdnz    ..dcloop

        addi    r6,0,0x0000            /* clear GPR 6                         */
        iccci   0,r6                   /* invalidate entire i cache           */	
	
	/*--------------------------------------------------------------------+
	| Clear	reservation bit.
	+--------------------------------------------------------------------*/
#if 0
        addis   r10,r0,0x0000
        lwarx   r3,r10,r10             /* get some data/set resv bit          */
        stwcx.  r3,r10,r10             /* store out and clear resv bit        */
#endif

#if 0
/*
* Enable I-cache to speed up the remainder of the boot
*/
        addis   r1,r0,0x8000
        addi    r1,r1,0x0001
        mtspr   iccr,r1
#endif

	/*
	* Set up default chip interconnect
	*/

	addis   r0,r0,0xD080
	ori     r0,r0,0x0468
	mtdcr   cic0_cr,r0
	addis   r0,r0,0x1070         /* if we not use RISCWATCH then must be modified */
	mtdcr   cic0_sel3,r0
	addis   r0,r0,0x0063
	ori     r0,r0,0x1700
	mtdcr   cic0_vcr,r0
	addi    r0,r0,0x0002
	mtdcr   cbs0_cr,r0

	/*bl ..bankinit*/

	addis	r10, 0, 0
        mtdcr   uic0_er,r10                      /* disable all interrupts   */

	/* UIC_IIC0 | UIC_IIC1 | UIC_U0 | UIC_IR_RCV | UIC_IR_XMIT */
	lis	r10,    0x00600e20@h
	ori	r10,r10,0x00600e20@l
	mtdcr	uic0_pr,r10

	lis	r10,    0x01000000@h		/* UIC_EIR1 */
	ori	r10,r10,0x01000000@l
	mtdcr	uic0_tr,r10

        lis     r10,0xFFFF                       /* r10 <- 0xFFFFFFFF        */
        ori     r10,r10,0xFFFF                   /*                          */
        mtdbsr  r10                              /* clear/reset the dbsr     */
        mtdcr   uic0_sr,r10                      /* clear pending interrupts */

        li      r10,0x1000                       /* set Machine Exception bit*/
        oris    r10,r10,0x2                      /* set Criticl Exception bit*/
        mtmsr   r10                              /* change MSR               */

	blr
