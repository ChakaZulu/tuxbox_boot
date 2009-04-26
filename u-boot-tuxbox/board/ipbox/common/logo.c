#include <common.h>
#include <command.h>
#include <asm/processor.h>

extern int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp);


#include "ucode.h"

#define DB_DRIVER_MEMORY_SIZE (1024*1024*16)


#define GZIPED_WEL		1
#define JPEG_WEL		2

#if defined(CONFIG_RELOOK100S)		/* mutant has only one bank */

#if defined(CFG_MUTANT_EXTRA_MEM)

#warning "INFO: logo in lower dram Extra mem (SUPERMUTANT)"
#define SRAM_BASE    0x038A0000

#else

#warning "INFO: logo in lower dram"
#define SRAM_BASE    0x018A0000

#endif

#else

#warning "INFO: logo in upper dram"
#define SRAM_BASE		0xA18A0000

#endif

// Audio
#define STBAUDIO_BASE 	SRAM_BASE
#define STBAUDIO_SIZE 	0x60000
//Queues
#define XP_MEM_BASE		(STBAUDIO_BASE+STBAUDIO_SIZE)
#define XP_MEM_SIZE     0x000E0000
//Pvr
#define PVR_BASE		XP_MEM_BASE+XP_MEM_SIZE
#define PVR_SIZE		0x00020000
//video
#define VIDEOMEM_BASE 	(PVR_BASE+PVR_SIZE)
#define VIDEOMEM_SIZE 	0x400000
//Framebuffer
#define FBMEM_BASE		(VIDEOMEM_BASE+VIDEOMEM_SIZE)
#define FBMEM_SIZE		0x200000

#define SEGMENTS_SIZE 4 //0=1mb 1=2mb 2=4mb 3=8mb 4=16mb
#define SEGMENTS_REG (((SEGMENTS_SIZE <<28)&0xF0000000) | ((VIDEOMEM_BASE>>4)& 0x0FFF0000)) | (((SEGMENTS_SIZE<<12)&0xF000) |((VIDEOMEM_BASE >> 20)&0xFFF))


#define USER_BUFFER_BASE  0x0A0000
#define FRAMEBUF_BASE     0x0B0000
#define RATE_BUFFER_BASE  0x300000
#define RATE_BUFFER_SIZE  0x100000


#define DB_MEM_BASE VIDEOMEM_BASE+RATE_BUFFER_BASE
//0xA0400000 + 0x300000


#define VID_DCR_BASE                    0x140
#define VID_CHIP_CTRL                   VID_DCR_BASE + 0x00
#define VID_WRT_PROT                    VID_DCR_BASE + 0x25
#define VID_PROC_IADDR                  VID_DCR_BASE + 0x0c
#define VID_PROC_IDATA                  VID_DCR_BASE + 0x0d
#define VID_CMD_STAT                    VID_DCR_BASE + 0x0a
#define VID_CMD                         VID_DCR_BASE + 0x08
#define VID_CMD_DATA                    VID_DCR_BASE + 0x09
#define VID_CMD_ADDR                    VID_DCR_BASE + 0x0b

#define DECOD_WR_PROT_DISABLE           0x00000001
#define DECOD_WR_PROT_ENABLE            0x00000000
#define DECOD_CHIP_CONTROL_SVP          0x00000002
#define DECOD_CHIP_CONTROL_SVD          0x00000001
#define DECOD_COMD_STAT_PENDING         0x00000001
#define DECOD_COM_RES_VID_BUF           (0x0008<<1)
#define DECOD_COM_CONF                  (0x0009<<1)
#define DECOD_COMD_CHAIN                0x00000001

#define VID_WAIT_CMD_TIME				10

int debug =0;
#define debug_print	if (debug ) printf


void memcpy_raw (void * dest, const void * src, int num);


void os_sleep(unsigned long ms)
{
	udelay (ms*1000);
}

int wait_cmd_done(unsigned int uRetry)
{
    unsigned long cmd_reg;

    while (1)
    {
        cmd_reg = mfdcr(VID_CMD_STAT);

        if ((cmd_reg & DECOD_COMD_STAT_PENDING) == 0)
        {
            return (0);
        }
        else
        {
            if (--uRetry == 0)
            {
                return (-1);
            }
            os_sleep(VID_WAIT_CMD_TIME);
            //debug_print("retry = %d\n", uRetry);
        }
    }
}

/*-------------------------------------------------------------------------+
| Verify microcode load.
+-------------------------------------------------------------------------*/
int verify_microcode(unsigned short *pCode, int nCount)
{
    int i;

    mtdcr(VID_WRT_PROT, DECOD_WR_PROT_DISABLE);

    for (i = 0; i < nCount; i++)
    {
        mtdcr(VID_PROC_IADDR, i);

        if (mfdcr(VID_PROC_IDATA) != pCode[i])
            return -1;
    }

    mtdcr(VID_PROC_IADDR, 0);
    mtdcr(VID_WRT_PROT, DECOD_WR_PROT_ENABLE);
    return (0);
}

int load_microcode(unsigned short *pCode, int nCount)
{
    unsigned long reg;
    int rc;
    int i;

    /*-------------------------------------------------------------------------+
    | Stop video processor.  Enable instruction store writes.
    +-------------------------------------------------------------------------*/
    reg =
        mfdcr(VID_CHIP_CTRL) &
        (~(DECOD_CHIP_CONTROL_SVP | DECOD_CHIP_CONTROL_SVD));
    mtdcr(VID_CHIP_CTRL, reg);
    mtdcr(VID_WRT_PROT, DECOD_WR_PROT_DISABLE);
    /*-------------------------------------------------------------------------+
    | Initialize control store address.
    +-------------------------------------------------------------------------*/
	/* modified by namws. 2005.07.28 */
	mtdcr( VID_PROC_IADDR, 0 );
    //mtdcr(VID_PROC_IADDR, 0x8000);
    /*-------------------------------------------------------------------------+
    | Load microcode.
    +-------------------------------------------------------------------------*/
    for (i = 0; i < nCount; i++)
    {
        mtdcr(VID_PROC_IDATA, pCode[i]);
    }

    /*-------------------------------------------------------------------------+
    | Verify microcode load.
    +-------------------------------------------------------------------------*/
    rc = verify_microcode(pCode, nCount);

    return (rc);
}
extern void mpeg_execute_command (int,int,int);



void mpeg_set_cmd(int uNum,unsigned int uPara[4])
{
    int i;

    for (i = 0; i < uNum; i++)
    {
        mtdcr(VID_CMD_ADDR, i);
        mtdcr(VID_CMD_DATA, uPara[i]);
    }
}

int mepg_exec_cmd(unsigned int	uCmd,int uNum,unsigned int uPara[4],int uRetry,int	uChained )
{
    if (wait_cmd_done(uRetry) != 0)
        return (-1);

    //check if this command has parameters
    if (uNum != 0)
        mpeg_set_cmd(uNum,uPara);

    if (uChained != 0)
    {
        mtdcr(VID_CMD, (uCmd<<1) | DECOD_COMD_CHAIN);
    }
    else
    {
        mtdcr(VID_CMD, (uCmd<<1) );
    }

    //activate command
    mtdcr(VID_CMD_STAT, DECOD_COMD_STAT_PENDING);

    if (wait_cmd_done(uRetry) != 0)
    {
        /*----------------------------------------------------------------------+
        | Time-out.  Force microcode to accept our command.  This is done
        | by writing 0x8200 to the instruction address register.  The
        | only command that can be accepted this way is reset video buffer
        | command.  Command chaining is set so that we can issue the original
        | command.
        +----------------------------------------------------------------------*/

        if ((uCmd != DECOD_COM_RES_VID_BUF)
            || (uChained != 0))
        {
            mtdcr(VID_CMD, DECOD_COM_RES_VID_BUF | DECOD_COMD_CHAIN);
        }

        mtdcr(VID_PROC_IADDR, 0x00008200);

        if (wait_cmd_done(10) != 0)
            return (-1);

        if (uCmd != DECOD_COM_RES_VID_BUF)
        {
            if (uChained != 0)
            {
                mtdcr(VID_CMD, (uCmd<<1) | DECOD_COMD_CHAIN);
            }
            else
            {
                mtdcr(VID_CMD, (uCmd<<1));
            }

            mtdcr(VID_CMD_STAT, DECOD_COMD_STAT_PENDING);

        }
    }
    return (0);
}

int mpeg_showiframe_c(char * iframe,int len,unsigned char * ucode,unsigned int ucode_len)
{

	int reg,i;
	char * baseReg;
	baseReg = (char*)DB_MEM_BASE;

	reg= mfdcr(0x33);
	reg= reg & 0xFFFFFFFE;
	mtdcr (0x33,reg);

	mtdcr (0x175,SEGMENTS_REG);
	mtdcr (0x176,SEGMENTS_REG);
	mtdcr (0x177,SEGMENTS_REG);
	mtdcr (0x178,SEGMENTS_REG);

	load_microcode((unsigned short *)ucode,ucode_len/2);

	mtdcr (0x140,0x1B);
	mtdcr (0x16B,USER_BUFFER_BASE/128);				//User data Base
	mtdcr (0x16C,0);
	mtdcr (0x16F,RATE_BUFFER_BASE/128);	// video buf offset
	mtdcr (0x17F,RATE_BUFFER_SIZE/32);	// video buf size
	reg= mfdcr(0x179);
	reg = reg & 0xFFFF0000;
	reg = reg | (FRAMEBUF_BASE/128);
	mtdcr(0x179,reg);

	mtdcr (0x140,0x1B);
	mepg_exec_cmd(9,0,0,10,0);

	reg = mfdcr(0x154);
	mtdcr (0x154, 0x88E6C0);
	mtdcr (0x151,0xFFFF0000);

	mtdcr (0x15A,0x1FFF0FF);
	mtdcr (0x140,0x2B);
	mtdcr (0x153,0);

	reg = mfdcr (0x140);
	reg = reg | 0x30;
	mtdcr (0x140,reg);

	reg = mfdcr (0x1A0);
	reg = reg | 0x220;
	mtdcr (0x1A0,reg);

	mtdcr (0x168,0);

	reg = mfdcr (0x140);
	reg = reg | 0x800;
	mtdcr (0x140,reg);

	mepg_exec_cmd(8,0,0,10,0);
	mepg_exec_cmd(9,0,0,10,0);
	mepg_exec_cmd(0xE,0,0,10,0);


	if (len == 0)
	{
		return -1;
	}

	for (i=0;i<4;i++)
	{
		memcpy_raw(baseReg + (i*len),iframe,len);
	}

	mtdcr (0x167,0);
	reg = len << 1;
	reg = reg | 0x80000000;
	mtdcr (0x168,reg);

	return 0;

}


void memcpy_raw (void * dest, const void * src, int num)
{
	int i;
	unsigned char * source;
	source = (unsigned char *)src;
	unsigned char * destination;
	destination = (unsigned char *)dest;


	for (i=0;i<num;i++)
	{
		destination[i] =source[i];
	}
}


struct _welcome_header
{
	unsigned long crc;
	unsigned long data_len;
	unsigned long compress_type;
	unsigned long bg_color;
};


int video_draw_startup_iframe( void )
{

	unsigned char *img;
	unsigned char *inflash_ucode;
	struct _welcome_header wel_header;
	unsigned char ucode[8192];
	unsigned long ucode_len=8192;

	int iframe_len;
	unsigned char* iframe;
	unsigned short ucode_magic;

	img = (unsigned char*)CFG_VIDEO_STARTUP_IMAGE_ADDR;
	inflash_ucode = (unsigned char*)CFG_FLASH_BASE+0x2000;

	iframe = (unsigned char*)CFG_VIDEO_STARTUP_IMAGE_ADDR+16;
	memcpy( &wel_header, img, sizeof(wel_header) );
	iframe_len = wel_header.data_len;

	if (wel_header.compress_type==3)  // display only if it's our type.
	{
		ucode_magic=((ucode_gz[0]<<8)|ucode_gz[1]);
		if (ucode_magic == 0x1F8B)
			gunzip (ucode,8192,ucode_gz,&ucode_len);

		ucode_magic=((ucode[0]<<8)|ucode[1]);
		if (ucode_magic == 0x0306)
			mpeg_showiframe_c(iframe,iframe_len,ucode,ucode_len);
		else
		{
			ucode_magic=((inflash_ucode[0]<<8)|inflash_ucode[1]);
			if (ucode_magic == 0x0306)
				memcpy(ucode,inflash_ucode,8192);
			else
				return -1;

			mpeg_showiframe_c(iframe,iframe_len,ucode,ucode_len);

		}
	} else
	if (wel_header.compress_type==4)  // image has ucode
	{
		ucode_magic=((iframe[iframe_len+0]<<8)|iframe[iframe_len+1]);
		if (ucode_magic == 0x0306)
			memcpy(ucode,iframe+iframe_len,8192);
		else if (ucode_magic == 0x1F8B)
			gunzip (ucode,8192,iframe+iframe_len,&ucode_len);
		else
		{
			ucode_magic=((inflash_ucode[0]<<8)|inflash_ucode[1]);
			if (ucode_magic == 0x0306)
				memcpy(ucode,inflash_ucode,8192);
			else
				return -1;
		}

		mpeg_showiframe_c(iframe,iframe_len,ucode,ucode_len);
	}

	return 0;
}





static int do_logo( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
		video_draw_startup_iframe();
		return 0;
}


U_BOOT_CMD(
		logo, 4, 0, do_logo,
		"logo - display logo\n",
		"display logo\n"
	  );

