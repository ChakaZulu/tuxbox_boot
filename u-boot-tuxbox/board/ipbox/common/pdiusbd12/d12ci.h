#ifndef __D12CI_H__
#define __D12CI_H__

#define D12_NOLAZYCLOCK			0x02
#define D12_CLOCKRUNNING		0x04
#define D12_INTERRUPTMODE		0x08
#define D12_SOFTCONNECT			0x10
#define D12_ENDP_NONISO			0x00
#define D12_ENDP_ISOOUT			0x40
#define D12_ENDP_ISOIN			0x80
#define D12_ENDP_ISOIO			0xC0

#define D12_CLOCK_12M			0x03
#define D12_CLOCK_4M			0x0b
#define D12_SETTOONE			0x40
#define D12_SOFONLY			0x80

#define D12_DMASINGLE			0x00
#define D12_BURST_4			0x01
#define D12_BURST_8			0x02
#define D12_BURST_16			0x03
#define D12_DMAENABLE			0x04
#define D12_DMA_INTOKEN			0x08
#define D12_AUTOLOAD			0x10
#define D12_NORMALPLUSSOF		0x20
#define D12_ENDP4INTENABLE		0x40
#define D12_ENDP5INTENABLE		0x80	// bug fixed in V2.1

#define D12_INT_ENDP0OUT		0x01
#define D12_INT_ENDP0IN			0x02
#define D12_INT_ENDP1OUT		0x04
#define D12_INT_ENDP1IN			0x08
#define D12_INT_ENDP2OUT		0x10
#define D12_INT_ENDP2IN			0x20
#define D12_INT_BUSRESET		0x40
#define D12_INT_SUSPENDCHANGE		0x80
#define D12_INT_EOT			0x0100

#define D12_SETUPPACKET			0x20

#define D12_BUFFER0FULL			0x20
#define D12_BUFFER1FULL			0x40

#define D12_FULLEMPTY			0x01
#define D12_STALL			0x02

void D12_SetAddressEnable(unsigned char bAddress, unsigned char bEnable);
void D12_SetEndpointEnable(unsigned char bEnable);
void D12_SetMode(unsigned char bConfig, unsigned char bClkDiv);
void D12_SetDMA(unsigned char bMode);
unsigned short D12_ReadInterruptRegister(void);
unsigned char D12_SelectEndpoint(unsigned char bEndp);
unsigned char D12_ReadLastTransactionStatus(unsigned char bEndp);
unsigned char D12_ReadEndpointStatus(unsigned char bEndp);
void D12_SetEndpointStatus(unsigned char bEndp, unsigned char bStalled);
void D12_SendResume(void);
unsigned short D12_ReadCurrentFrameNumber(void);
unsigned short D12_ReadChipID(void);

unsigned char D12_ReadEndpoint(unsigned char endp, unsigned char * buf, unsigned char len);
unsigned char D12_WriteEndpoint(unsigned char endp, unsigned char * buf, unsigned char len);
void D12_AcknowledgeEndpoint(unsigned char endp);

unsigned char D12_ReadMainEndpoint(unsigned char * buf); // V2.2

#if 0
unsigned char D12Eval_inportb(void);
void D12Eval_outportb(unsigned char val, unsigned char mask);
#endif

#endif
