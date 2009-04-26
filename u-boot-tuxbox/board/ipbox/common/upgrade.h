#ifndef _DGS_UPGRADE_H
#define _DGS_UPGRADE_H

#define IMG_MAGIC       (('F'<<24)|('i'<<16)|('m'<<8)|('g'<<0))
#define DATA_OFFSET     0x1000
#define BUF_SIZE        DATA_OFFSET             // must be same with data_offset???
#define HEADER_SIZE	DATA_OFFSET
#define H_NAME_SIZE	32
struct _image_header
{
	unsigned long header_crc;	/* crc from 4 ~ HEADER_SIZE */
	unsigned long magic;		/* magic number. IMG_MAGIC */
	unsigned long structure_size;	/* size of this structure */

	unsigned long vendor_id;	/* vender id */
	unsigned long product_id;	/* product id */

	unsigned long hw_model;		/* MY_HW_MODEL */
	unsigned long hw_version;

	unsigned long start_addr;	/* start address of write to flash. */
	unsigned long erase_size;	/* erase size from start_addr */

	unsigned long data_offset;	/* offset of acture data */
	unsigned long data_size;	/* size of acture data */
	unsigned long data_crc;		/* crc from data_offset ~ (data_offset+data_size) */

	char name[H_NAME_SIZE];		/* name of this image */
};

extern int burn_flash( void );

/*
 * buffer status for upg_buffer.
 *  will be stored in upg_buffer_status.
 */
enum _upg_buffer_status
{
	UPGBUF_UNUSED = 0,
	UPGBUF_DOWNLOAD,
	UPGBUF_ERASE,
	UPGBUF_WRITE,
};

extern unsigned char *upg_buffer;
extern int upg_buffer_len;
extern enum _upg_buffer_status upg_buffer_status;
extern char *upg_buffer_message;
extern int upg_verbose_message;
/*
 * upg_buffer_errorno
 *  0 : no error
 *  1 : wrong header crc
 *  2 : wrong data crc
 *  3 : incorrect vendor id
 *  4 : incorrect product id
 *  5 : wrong hardware model or version
 *  6 :
 *  7 :
 *  8 : flash erase failed
 *  9 : flash write failed
 */
extern int upg_buffer_errorno;

#endif
