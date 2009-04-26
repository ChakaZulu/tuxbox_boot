


#ifndef _ASM_STB034XX_H
#define _ASM_STB034XX_H

#ifndef CONFIG_STB034xx
#error Oops~!
#endif

#define RESET_VECTOR	0xfffffffc
#define CACHELINE_MASK	(CFG_CACHELINE_SIZE - 1) 

#define cdbcr		0x3d7
//#define ctr		0x009
#define ctrreg		0x009
#define dac1		0x3f6
#define dbcr		0x3f2
#define dbsr		0x3f0
#define dccr		0x3fa
#define dcwr		0x3ba
#define dear		0x3d5
#define esr		0x3d4
#define evpr		0x3d6
#define iac1		0x3f4
#define iccr		0x3fb
#define icdbdr		0x3d3
#define lrreg		0x008
#define pid		0x3b1
#define pit		0x3db
//#define pvr		0x11f
#define sgr		0x3b9
#define skr		0x3bc
#define sler		0x3bb
#define sprg0		0x110
#define sprg1		0x111
#define sprg2		0x112
#define sprg3		0x113
#define srr0		0x01a
#define srr1		0x01b
#define srr2		0x3de
#define srr3		0x3df
#define tbhi		0x3dc
#define tbhu		0x3cc
#define tblo		0x3dd
#define tblu		0x3cd
#define tcr		0x3da
#define tsr		0x3d8
#define xerreg		0x001
//#define xer		0x001
#define zpr		0x3b0

#define cbs_cbscr	0x010
#define cbs0_cr		0x010

#define dcr_xsdr	0x020
#define dcr_xprr	0x021

#define cic_cr		0x030
#define cic_sel1	0x031
#define cic_sel2	0x032
#define cic0_cr		0x030
#define cic0_vcr	0x033
#define cic0_sel3	0x035

#define sgpo		0x036
#define sgpod		0x037
#define sgptc		0x038
#define sgpi		0x039

#define UIC_DCR_BASE	0x040
#define uic0_sr		uicsr
#define uic0_srs	uicsrs
#define uic0_er		uicer
#define uic0_cr		uiccr
#define uic0_pr		uicpr
#define uic0_tr		uictr
#define uic0_msr	uicmsr
#define uic0_vr		uicvr
#define uic0_vcr	uicvcr

#define plb0_pesr0	0x054
#define plb0_pear0	0x056
#define plb0_pacr0	0x057
#define plb1_pesr1	0x064
#define plb1_pear1	0x066
#define plb1_pacr1	0x067

#define ebiu0_brcrh0	0x070
#define ebiu0_brcrh1	0x071
#define ebiu0_brcrh2	0x072
#define ebiu0_brcrh3	0x073
#define ebiu0_brcrh4	0x074
#define ebiu0_brcrh5	0x075
#define ebiu0_brcrh6	0x076
#define ebiu0_brcrh7	0x077
#define ebiu0_brcr0	0x080
#define ebiu0_brcr1	0x081
#define ebiu0_brcr2	0x082
#define ebiu0_brcr3	0x083
#define ebiu0_brcr4	0x084
#define ebiu0_brcr5	0x085
#define ebiu0_brcr6	0x086
#define ebiu0_brcr7	0x087
#define ebiu0_bear	0x090
#define ebiu0_besr	0x091
#define ebiu0_biucr	0x09a

#define opb_gesr0	0x0B0
#define opb_gear	0x0B2

#define DMA_DCR_BASE	0x0c0
#define dma0_cr0	dmacr0
#define dma0_ct0	dmact0
#define dma0_da0	dmada0
#define dma0_sa0	dmasa0
#define dma0_cc0	dmasb0
#define dma0_cr1	dmacr1
#define dma0_ct1	dmact1
#define dma0_da1	dmada1
#define dma0_sa1	dmasa1
#define dma0_cc1	dmasb1
#define dma0_cr2	dmacr2
#define dma0_ct2	dmact2
#define dma0_da2	dmada2
#define dma0_sa2	dmasa2
#define dma0_cc2	dmasb2
#define dma0_cr3	dmacr3
#define dma0_ct3	dmact3
#define dma0_da3	dmada3
#define dma0_sa3	dmasa3
#define dma0_cc3	dmasb3
#define dma0_sr		dmasr

#define cp_cpmsr	0x100
#define cp_cpmer	0x101
#define cp_cpmfr	0x102

#define clkgpcr		0x120
#define clkapcr		0x121

#define denc_idr	0x130
#define denc_cr1	0x131
#define denc_rr1	0x132
#define denc_cr2	0x133
#define denc_rr2	0x134
#define denc_rr3	0x135
#define denc_rr4	0x136
#define denc_rr5	0x137
#define denc_ccdr	0x138
#define denc_cccr	0x139
#define denc_trr	0x13A
#define denc_tosr	0x13B
#define denc_tesr	0x13C
#define denc_rlsr	0x13D
#define denc_vlsr	0x13E
#define denc_vsr	0x13F

#define v_c_cntl	0x140
#define v_c_mode	0x141
#define v_s_stc0	0x142
#define v_s_stc1	0x143
#define v_s_pts0	0x144
#define v_s_pts1	0x145
#define v_fifo		0x146
#define v_fifo_s	0x147
#define v_cmd		0x148
#define v_cmd_d		0x149
#define v_cmd_st	0x14A
#define v_cmd_ad	0x14B
#define v_procia	0x14C
#define v_procid	0x14D
#define v_osd_m		0x151
#define v_host_i	0x152
#define v_mask		0x153
#define v_dispm		0x154
#define v_dispd		0x155
#define v_vb_ctl	0x156
#define v_displb	0x157
#define v_disptb	0x158
#define v_osd_la	0x159
#define v_rb_thr	0x15B
#define v_stc_ca	0x15D
#define v_ptsctl	0x15F
#define v_w_prot	0x165
#define v_vc_qa		0x167
#define v_vc_ql		0x168
#define v_udbase	0x16B
#define v_v0base	0x16C
#define v_v1base	0x16D
#define v_osbase	0x16E
#define v_rbbase	0x16F
#define v_dramad	0x170
#define v_dramdt	0x171
#define v_dramcs	0x172
#define v_vc_wa		0x173
#define v_vc_wl		0x174
#define v_m_seg0	0x175
#define v_m_seg1	0x176
#define v_m_seg2	0x177
#define v_m_seg3	0x178
#define v_fbuff_base	0x179
#define v_tl_border	0x17B
#define v_tr_dly	0x17C
#define v_small_board	0x17D
#define v_hv_zoom	0x17E
#define v_rb_sz		0x17F

#define xptlr		0x180
#define xptdt		0x181
#define xptir		0x182

#define dcp_kiar	0x190
#define dcp_kidr	0x191

#define a_ctrl0		0x1a0
#define a_ctrl1		0x1a1
#define a_ctrl2		0x1a2
#define a_command	0x1a3
#define a_isr		0x1a4
#define a_imr		0x1a5
#define a_dsr		0x1a6
#define a_stc		0x1a7
#define a_csr		0x1a8
#define a_latcnt	0x1a9
#define a_pts		0x1aa
#define a_tgctrl	0x1ab
#define a_tgval		0x1ac
#define a_auxd		0x1ad
#define a_strmid	0x1ae
#define a_sqar		0x1af
#define a_dsp_st	0x1b0
#define a_qlr		0x1b1
#define a_dsp_c		0x1b2
#define a_inst_d	0x1b4
#define a_war		0x1b5
#define a_seg1r		0x1b6
#define a_seg2r		0x1b7
#define a_atf		0x1b9
#define a_atr		0x1ba
#define a_atc		0x1bb
#define a_seg3r		0x1bc
#define a_offset	0x1bd
#define a_wrl		0x1be
#define a_plb_pr	0x1bf

#define hsmc_mcgr	0x1c0
#define hsmc_mcbesr	0x1c1
#define hsmc_mcbear	0x1c2
#define hsmc_mcbr0	0x1c4
#define hsmc_mccr0	0x1c5
#define hsmc_mcbr1	0x1c7
#define hsmc_mccr1	0x1c8
#define hsmc_sysr	0x1d1
#define hsmc_data	0x1d2
#define hsmc_mccrr	0x1d3

#define ocm_pbar	0x1E0

#define hsmc0_gr	0x1e0
#define hsmc0_besr	0x1e1
#define hsmc0_bear	0x1e2
#define hsmc0_br0	0x1e4
#define hsmc0_cr0	0x1e5
#define hsmc0_br1	0x1e7
#define hsmc0_cr1	0x1e8
#define hsmc0_sysr	0x1f1
#define hsmc0_data	0x1f2
#define hsmc0_crr	0x1f3

#define hsmc1_gr	0x1c0
#define hsmc1_besr	0x1c1
#define hsmc1_bear	0x1c2
#define hsmc1_br0	0x1c4
#define hsmc1_cr0	0x1c5
#define hsmc1_br1	0x1c7
#define hsmc1_cr1	0x1c8
#define hsmc1_sysr	0x1d1
#define hsmc1_data	0x1d2
#define hsmc1_crr	0x1d3

#define msr_ape		0x00100000
#define msr_apa		0x00080000
#define msr_we		0x00040000
#define msr_ce		0x00020000
#define msr_ile		0x00010000
#define msr_ee		0x00008000
#define msr_pr		0x00004000
#define msr_me		0x00001000
#define msr_de		0x00000200
#define msr_ir		0x00000020
#define msr_dr		0x00000010
#define msr_le		0x00000001

#define stack_reg_image_size 160

#endif
