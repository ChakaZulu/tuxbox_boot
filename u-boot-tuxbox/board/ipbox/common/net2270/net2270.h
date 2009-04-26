#ifndef __NET2270_H__
#define __NET2270_H__

#if defined(CONFIG_RELOOK510S)
#define 	NET2270_IO_BASE		0xfee00000
#else
#define		NET2270_IO_BASE		0xff500000 // for others
#endif

// USB -> Registers
// direct access
#define REGADDRPTR   ((volatile unsigned short *) (NET2270_IO_BASE+0x00))   // Register Address Pointer for indirect register addressing
#define REGDATA      ((volatile unsigned short *) (NET2270_IO_BASE+0x02))     // Register Data Port for indirect register addressing
#define IRQSTAT0     ((volatile unsigned short *) (NET2270_IO_BASE+0x04))     // Interrupt Status Register low byte
#define IRQSTAT1     ((volatile unsigned short *) (NET2270_IO_BASE+0x06))     // Interrupt Status Register high byte
#define PAGESEL      ((volatile unsigned short *) (NET2270_IO_BASE+0x08))     // Page Select Register, select current Endpoint
#define EP_DATA      ((volatile unsigned short *) (NET2270_IO_BASE+0x0a))     // Endpoint Data Register
#define EP_STAT0     ((volatile unsigned short *) (NET2270_IO_BASE+0x0c))     // Endpoint Main Status low byte
#define EP_STAT1     ((volatile unsigned short *) (NET2270_IO_BASE+0x0e))     // Endpoint Main Status high byte
#define EP_TRANSFER0 ((volatile unsigned short *) (NET2270_IO_BASE+0x10))     // For IN Endpoint, number of bytes to transfer to host, low byte
#define EP_TRANSFER1 ((volatile unsigned short *) (NET2270_IO_BASE+0x12))     // For IN Endpoint, number of bytes to transfer to host, mid byte
#define EP_TRANSFER2 ((volatile unsigned short *) (NET2270_IO_BASE+0x14))     // For IN Endpoint, number of bytes to transfer to host, high byte
#define EP_IRQENB    ((volatile unsigned short *) (NET2270_IO_BASE+0x16))     // Endpoint Interrupt Enable
#define EP_AVAIL0    ((volatile unsigned short *) (NET2270_IO_BASE+0x18))     // For IN Endpoints, number of available spaces in buffer
#define EP_AVAIL1    ((volatile unsigned short *) (NET2270_IO_BASE+0x1a))     // For OUT Endpoints, number of bytes in buffer
#define EP_RSPCLR    ((volatile unsigned short *) (NET2270_IO_BASE+0x1c))     // Endpoint Response Register Clear
#define EP_RSPSET    ((volatile unsigned short *) (NET2270_IO_BASE+0x1e))     // Endpoint Response Register Set
#define USBCTL0      ((volatile unsigned short *) (NET2270_IO_BASE+0x30))     // USB Control, low byte
#define USBCTL1      ((volatile unsigned short *) (NET2270_IO_BASE+0x32))     // USB Control, high byte
#define FRAME0       ((volatile unsigned short *) (NET2270_IO_BASE+0x34))     // Frame Counter, low byte
#define FRAME1       ((volatile unsigned short *) (NET2270_IO_BASE+0x36))     // Frame Counter, high byte
#define DMAREQ       ((volatile unsigned short *) (NET2270_IO_BASE+0x38))     // DMA Request Control Register
#define NET_SCRATCH  ((volatile unsigned short *) (NET2270_IO_BASE+0x3a))     // General Purpose Scratchpad register

// indirect access
#define IRQENB0         0x20       // Interrupt Enable Register, low byte
#define IRQENB1         0x21       // Interrupt Enable Register, high byte
#define LOCCTL          0x22       // Local Bus Control Register
#define CHIPREV         0x23       // Chip Revision Number
#define EP_MAXPKT0      0x28       // Endpoint Max Packet Size, low byte
#define EP_MAXPKT1      0x29       // Endpoint Max Packet Size, high byte
#define EP_CFG          0x2A       // Endpoint Configuration
#define OURADDR         0x30       // Our USB address
#define USBDIAG         0x31       // Diagnostic Register
#define USBTEST         0x32       // USB 2.0 Test Control Register
#define XCVRDIAG        0x33       // Transceiver Diagnostic Register
#define SETUP0          0x40       // Setup byte 0
#define SETUP1          0x41       // Setup byte 1
#define SETUP2          0x42       // Setup byte 2
#define SETUP3          0x43       // Setup byte 3
#define SETUP4          0x44       // Setup byte 4
#define SETUP5          0x45       // Setup byte 5
#define SETUP6          0x46       // Setup byte 6
#define SETUP7          0x47       // Setup byte 7

#define NET2270_REV_NR          0x32

#define     SOF_INTERRUPT                                       7
#define     DMA_DONE_INTERRUPT                                  6
#define     SETUP_PACKET_INTERRUPT                              5
#define     ENDPOINT_C_INTERRUPT                                3
#define     ENDPOINT_B_INTERRUPT                                2
#define     ENDPOINT_A_INTERRUPT                                1
#define     ENDPOINT_0_INTERRUPT                                0

#define     RESET_INTERRUPT                                     7
#define     ROOT_PORT_RESET_INTERRUPT                           6
#define     RESUME_INTERRUPT                                    5
#define     SUSPEND_REQUEST_CHANGE_INTERRUPT                    4
#define     SUSPEND_REQUEST_INTERRUPT                           3
#define     VBUS_INTERRUPT                                      2
#define     CONTROL_STATUS_INTERRUPT                            1

#define     PAGE_SELECT                                         1
//Page Select selector ( See Page Select in PAGESEL )
#define EP_0                                    0
#define EP_A                                    1
#define EP_B                                    2
#define EP_C                                    3

#define     BUFFER_FULL                                         7
#define     BUFFER_EMPTY                                        6
#define     NAK_OUT_PACKETS                                     5
#define     SHORT_PACKET_TRANSFERRED_INTERRUPT                  4
#define     DATA_PACKET_RECEIVED_INTERRUPT                      3
#define     DATA_PACKET_TRANSMITTED_INTERRUPT                   2
#define     DATA_OUT_TOKEN_INTERRUPT                            1
#define     DATA_IN_TOKEN_INTERRUPT                             0

#define     BUFFER_FLUSH                                        7
#define     USB_STALL_SENT                                      5
#define     USB_IN_NAK_SENT                                     4
#define     USB_IN_ACK_RCVD                                     3
#define     USB_OUT_NAK_SENT                                    2
#define     USB_OUT_ACK_SENT                                    1
#define     TIMEOUT                                             0

#define     SHORT_PACKET_TRANSFERRED_INTERRUPT_ENABLE           4
#define     DATA_PACKET_RECEIVED_INTERRUPT_ENABLE               3
#define     DATA_PACKET_TRANSMITTED_INTERRUPT_ENABLE            2
#define     DATA_OUT_TOKEN_INTERRUPT_ENABLE                     1
#define     DATA_IN_TOKEN_INTERRUPT_ENABLE                      0

#define     ALT_NAK_OUT_PACKETS                                 7
#define     HIDE_STATUS_PHASE                                   6
#define     AUTO_VALIDATE                                       5
#define     INTERRUPT_MODE                                      4
#define     CONTROL_STATUS_PHASE_HANDSHAKE                      3
#define     NAK_OUT_PACKETS_MODE                                2
#define     ENDPOINT_TOGGLE                                     1
#define     ENDPOINT_HALT                                       0

#define     USB_ROOT_PORT_WAKEUP_ENABLE                         5
#define     USB_DETECT_ENABLE                                   3
#define     IO_WAKEUP_ENABLE                                    1

#define     GENERATE_RESUME                                     3
#define     USB_HIGH_SPEED                                      2
#define     USB_FULL_SPEED                                      1
#define     VBUS_PIN                                            0

#define     DMA_BUFFER_VAILD                                    7
#define     DMA_REQUEST                                         6
#define     DMA_REQUEST_ENABLE                                  5
#define     DMA_CONTROL_DACK                                    4
#define     EOT_POLARITY                                        3
#define     DACK_POLARITY                                       2
#define     DREQ_POLARITY                                       1
#define     DMA_ENDPOINT_SELECT                                 0

#define     SOF_INTERRUPT_ENABLE                                7
#define     DMA_DONE_INTERRUPT_ENABLE                           6
#define     SETUP_PACKET_INTERRUPT_ENABLE                       5
#define     ENDPOINT_C_INTERRUPT_ENABLE                         3
#define     ENDPOINT_B_INTERRUPT_ENABLE                         2
#define     ENDPOINT_A_INTERRUPT_ENABLE                         1
#define     ENDPOINT_0_INTERRUPT_ENABLE                         0

#define     ROOT_PORT_RESET_INTERRUPT_ENABLE                    6
#define     RESUME_INTERRUPT_ENABLE                             5
#define     SUSPEND_REQUEST_CHANGE_INTERRUPT_ENABLE             4
#define     SUSPEND_REQUEST_INTERRUPT_ENABLE                    3
#define     VBUS_INTERRUPT_ENABLE                               2
#define     CONTROL_STATUS_INTERRUPT_ENABLE                     1

#define     BUFFER_CONFIGURATION                                6       
#define     BYTE_SWAP                                           5
#define     DMA_SPLIT_BUS_MODE                                  4
#define     LOCAL_CLOCK_OUTPUT                                  1
#define     DATA_WIDTH                                          0
// Local Clock Output (bits 3:1 in LOCCTL register)
#define LOCAL_CLOCK_OUTPUT_OFF                          0           // No output
#define LOCAL_CLOCK_OUTPUT_3_75MHZ                      1           // 
#define LOCAL_CLOCK_OUTPUT_7_5MHZ                       2           // (Default)
#define LOCAL_CLOCK_OUTPUT_15MHZ                        3           // 
#define LOCAL_CLOCK_OUTPUT_30MHZ                        4           // 
#define LOCAL_CLOCK_OUTPUT_60MHZ                        5           // 
// Buffer Configuration (bits 7:6 in LOCCTL register)
#define BUFFER_CONFIGURATION_EPA512_EPB512                      0       // EPA and EPB double buffered (default)
#define BUFFER_CONFIGURATION_EPA1024_EPB512             1       // EPB double buffered
#define BUFFER_CONFIGURATION_EPA1024_EPB1024            2       // EPA and EPB not double buffered
#define BUFFER_CONFIGURATION_EPA1024DB                  3       // EPA double buffered (EPB disabled)

#define     ENDPOINT_ENABLE                                     7
#define     ENDPOINT_TYPE                                       5       // 1:Isochronous, 2:Bulk, 3:Interrupt
#define         ISOCHRONOUS             1
#define         BULK                    2
#define         INTERRUPT               3
#define     ENDPOINT_DIR 	                                4       // 0:OUT, 1:IN
#define     ENDPOINT_NUM	                                0

#define     FORCE_IMMEDIATE                                     7

#define     FORCE_BIDIRECTIONAL_TO_INPUT                        5
#define     FAST_TIMES                                          4
#define     FORCE_RECEIVED_ERROR                                2
#define     PREVENT_TRANSMIT_BITSTUFF                           1
#define     FORCE_TRANSMIT_CRC_ERROR                            0

// globals
unsigned short max_packet_size;

static u8 inline inportb (u32 port)
{
	return *(volatile u16 *) (NET2270_IO_BASE + port);
}

static void inline outportb (u32 port, u8 val)
{       
	*(volatile u16 *) (NET2270_IO_BASE + port) = val;
}

static u16 inline read_ep_avail ( void )
{
	return (inportb (0x1a) << 8 | inportb (0x18));
}

unsigned short read_fifo(unsigned char ep, unsigned char * buf);
unsigned short read_fifo_full_speed0(unsigned char * buf);
void send_fifo(unsigned char ep, unsigned char * buf, unsigned short len);
void Stall_Ep (unsigned char ep, unsigned char mode);
void Standby (void);
void WakeUp (void);


#define DISABLE	disable_interrupts()
#define ENABLE	enable_interrupts()

#endif
