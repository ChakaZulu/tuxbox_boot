#include <common.h>
#include <command.h>
#include <devices.h>
#include <net.h>

#include "common/front.h"


DECLARE_GLOBAL_DATA_PTR;



#define TIMEOUT		5		/* Seconds before trying BOOTP again */
#ifndef	CONFIG_NET_RETRY_COUNT
# define TIMEOUT_COUNT	5		/* # of timeouts before giving up  */
#else
# define TIMEOUT_COUNT  (CONFIG_NET_RETRY_COUNT)
#endif

static const char *output_packet;	/* used by first send udp */
static int output_packet_len = 0;

extern u32 dm9k_phy_retry_times;
extern int burn_flash_img( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] );
static int setup_pkt_recvd=0;


static void AckSend(void);
/*
 *	Handle a RARP received packet.
 */
static void
NBootHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
//	int i;
	char buf  [32];
	char myip [15];
	char srip [15];
	dm9k_phy_retry_times=10000;

	if (!strcmp("DGSNETUP" ,&pkt[1]))
	{

		int offset=strlen(&pkt[1])+1;
		front_puts("MAGI");
		printf("Got UPDATE PAKET\n");
		sprintf(srip, "%d.%d.%d.%d", pkt[offset+1],pkt[offset+2],pkt[offset+3],pkt[offset+4]);
		sprintf(myip , "%d.%d.%d.%d",pkt[offset+5],pkt[offset+6],pkt[offset+7],pkt[offset+8]);
		if (pkt[offset+9]>0)
		{
			sprintf(BootFile,"%s",&pkt[offset+10]);
			printf("FILENAME  %s\n",BootFile);
		}
		printf("SERVER IP %s\n",srip);
		printf("STB    IP %s\n",myip);
		setenv("ipaddr",myip);
		setenv("serverip",srip);
		sprintf(buf, "0x%08x", (unsigned char*)gd->dgs_upg_buffer);
		load_addr=gd->dgs_upg_buffer;
		setenv("upgrade_buffer", buf);
		
		NetServerIP = getenv_IPaddr ("serverip");
		NetOurIP    = getenv_IPaddr ("ipaddr");
		setup_pkt_recvd=1;		
	}
}


/*
 *	Timeout on BOOTP request.
 */
static void
NbootTimeout(void)
{
	puts ("NbootTimeout\n");

}

int network_update( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	int i;
	setup_pkt_recvd=0;
	NetSetTimeout(TIMEOUT * CFG_HZ, NbootTimeout);
	NetSetHandler(NBootHandler);
	
	dm9k_phy_retry_times=1000;
	eth_init(gd->bd);
	eth_rx();
	if (setup_pkt_recvd)
	{

		front_puts("TFTP");
		if (NetLoop (TFTP) <= 0)
			return 1;
			
		front_puts("BURN");
		burn_flash_img( 0, 0, 0, 0);
		do_reset (NULL, 0, 0, NULL);
	}
	return 0;
}

U_BOOT_CMD(
		netupd, 14, 0, network_update,
		"netupd  - netupdate\n",
		"netupd netUpdate" );
