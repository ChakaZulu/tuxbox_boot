#ifndef _ASM_STB04XXX_H
#define _ASM_STB04XXX_H

#define RESET_VECTOR	0xfffffffc
#define CACHELINE_MASK	(CFG_CACHELINE_SIZE - 1)

#define ccr0		0x3b3
//#define ctr		0x009
//#define CTR		0x009
#define dbcr0		0x3f2
#define dbcr1		0x3bd
#define dbsr		0x3f0
#define dccr		0x3fa
#define dcwr		0x3ba
#define dear		0x3d5
#define esr		0x3d4
#define evpr		0x3d6
#define iccr		0x3fb
#define icdbdr		0x3d3
#define lrreg		0x008
#define pid		0x3b1
#define pit		0x3db
//#define pvr		0x11f
//#define PVR		0x11f
#define sgr		0x3b9
#define sler		0x3bb
#define sprg0		0x110
#define sprg1		0x111
#define sprg2		0x112
#define sprg3		0x113
#define sprg4		0x114
#define sprg5		0x115
#define sprg6		0x116
#define sprg7		0x117
#define srr0		0x01a
#define srr1		0x01b
#define srr2		0x3de
#define srr3		0x3df
#define tbhi		0x11D
#define tblo		0x11C
#define tcr		0x3da
#define tsr		0x3d8
//#define xer		0x001
//#define XER		0x001
#define zpr		0x3b0

#define icbs0_cntl16	0x016
#define icbs0_amap0	0x018
#define icbs0_amap1	0x019

#define epi0_cr		0x020
#define epi0_sr		0x021
#define epi0_epba	0x022
#define epi0_ipba	0x023


#define cic0_cr		0x030
#define cic_cr		cic0_cr


#define dma0_s1		0x031
#define dma0_s2		0x032

#define cic0_vcr	0x033
#define cic0_sel3	0x035
#define cic0_muxo	0x036
#define cic0_muxod	0x037
#define cic0_muxtc	0x038
#define cic0_muxi	0x039


#define dma1_s1		0x03a
#define dma1_s2		0x03b

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

#define opbw0_gesr	0x0b0
#define opbw0_gear	0x0b2

#define usb0_hc_memerr	0x0b5

#define dma0_cr0	0x0c0
#define dma0_ct0	0x0c1
#define dma0_da0	0x0c2
#define dma0_sa0	0x0c3
#define dma0_cc0	0x0c4
#define dma0_cr1	0x0c8
#define dma0_ct1	0x0c9
#define dma0_da1	0x0ca
#define dma0_sa1	0x0cb
#define dma0_cc1	0x0cc
#define dma0_cr2	0x0d0
#define dma0_ct2	0x0d1
#define dma0_da2	0x0d2
#define dma0_sa2	0x0d3
#define dma0_cc2	0x0d4
#define dma0_cr3	0x0d8
#define dma0_ct3	0x0d9
#define dma0_da3	0x0da
#define dma0_sa3	0x0db
#define dma0_cc3	0x0dc
#define dma0_sr0	0x0e0

#define cpm0_fr		0x102

#define cic0_sccr	0x120

#define denc0_cr1	0x131
#define denc0_rr1	0x132
#define denc0_cr2	0x133
#define denc0_rr2	0x134
#define denc0_rr3	0x135
#define denc0_rr4	0x136
#define denc0_rr5	0x137
#define denc0_ccdr	0x138
#define denc0_cccr	0x139
#define denc0_trr	0x13A
#define denc0_tosr	0x13B
#define denc0_tesr	0x13C
#define denc0_rlsr	0x13D
#define denc0_vlsr	0x13E
#define denc0_vsr	0x13F

#define vid0_cntl	0x140
#define vid0_mode	0x141
#define vid0_stc0	0x142
#define vid0_stc1	0x143
#define vid0_pts0	0x144
#define vid0_pts1	0x145
#define vid0_fifo	0x146
#define vid0_fifos	0x147
#define vid0_cmd	0x148
#define vid0_cmdd	0x149
#define vid0_cmdst	0x14A
#define vid0_cmdad	0x14B
#define vid0_procia	0x14C
#define vid0_procid	0x14D
#define vid0_dispc	0x151
#define vid0_int	0x152
#define vid0_mask	0x153
#define vid0_dispm	0x154
#define vid0_dispd	0x155
#define vid0_vbcntl	0x156
#define vid0_ttxcntl	0x157
#define vid0_border	0x158
#define vid0_cgosd	0x15A
#define vid0_rbthr	0x15B
#define vid0_sbosd	0x15C
#define vid0_ptsd	0x15E
#define vid0_ptsctl	0x15F
#define vid0_wprot	0x165
#define vid0_vcqa	0x167
#define vid0_vcql	0x168
#define vid0_blksz	0x169
#define vid0_srcad	0x16a
#define vid0_udbase	0x16B
#define vid0_01base	0x16C
#define vid0_odbase	0x16D
#define vid0_basead	0x16E
#define vid0_rbbase	0x16F
#define vid0_memaddr	0x170
#define vid0_memdata	0x171
#define vid0_memcntl	0x172
#define vid0_vcwa	0x173
#define vid0_vcwl	0x174
#define vid0_mseg0	0x175
#define vid0_mseg1	0x176
#define vid0_mseg2	0x177
#define vid0_mseg3	0x178
#define vid0_fbbase	0x179
#define vid0_cursor	0x17A
#define vid0_sclsiz	0x17B
#define vid0_crpsiz	0x17C
#define vid0_sclbrd	0x17D
#define vid0_crpoff	0x17E
#define vid0_rbsz	0x17F

#define xpt0_lr		0x180
#define xpt0_dt		0x181
#define xpt0_ir		0x182

#define xpt1_lr		0x190
#define xpt1_dt		0x191
#define xpt1_ir		0x192

#define xpt2_lr		0x2d0
#define xpt2_dt		0x2d1
#define xpt2_ir		0x2d2

#define aud0_ctrl0	0x1a0
#define aud0_ctrl1	0x1a1
#define aud0_ctrl2	0x1a2
#define aud0_cmd	0x1a3
#define aud0_isr	0x1a4
#define aud0_imr	0x1a5
#define aud0_dsr	0x1a6
#define aud0_stc	0x1a7
#define aud0_csr	0x1a8
#define aud0_qar2	0x1a9
#define aud0_pts	0x1aa
#define aud0_tgctrl	0x1ab
#define aud0_qlr2	0x1ac
#define aud0_anc	0x1ad
#define aud0_strmid	0x1ae
#define aud0_qar	0x1af
#define aud0_dsps	0x1b0
#define aud0_qlr	0x1b1
#define aud0_dspc	0x1b2
#define aud0_wlr2	0x1b3
#define aud0_mdr	0x1b4
#define aud0_war	0x1b5
#define aud0_seg1	0x1b6
#define aud0_seg2	0x1b7
#define aud0_rbf	0x1b8
#define aud0_avf	0x1b9
#define aud0_avr	0x1ba
#define aud0_avc	0x1bb
#define aud0_seg3	0x1bc
#define aud0_offset	0x1bd
#define aud0_wrl	0x1be
#define aud0_war2	0x1bf

#define sdram1_besr	0x1c1
#define sdram1_bear	0x1c2
#define sdram1_br0	0x1c4
#define sdram1_cr0	0x1c5
#define sdram1_br1	0x1c7
#define sdram1_cr1	0x1c8
#define sdram1_br2	0x1ca
#define sdram1_cr2	0x1cb
#define sdram1_br3	0x1cd
#define sdram1_cr3	0x1ce

#define sdram0_besr	0x1e1
#define sdram0_bear	0x1e2
#define sdram0_br0	0x1e4
#define sdram0_cr0	0x1e5
#define sdram0_br1	0x1e7
#define sdram0_cr1	0x1e8
#define sdram0_br2	0x1ea
#define sdram0_cr2	0x1eb
#define sdram0_br3	0x1ed
#define sdram0_cr3	0x1ee

#define sdram0_config	0x1f3
#define sdram1_config	0x1d3

#define dma1_cr0	0x200
#define dma1_ct0	0x201
#define dma1_da0	0x202
#define dma1_sa0	0x203
#define dma1_cc0	0x204
#define dma1_cr1	0x208
#define dma1_ct1	0x209
#define dma1_da1	0x20a
#define dma1_sa1	0x20b
#define dma1_cc1	0x20c
#define dma1_cr2	0x210
#define dma1_ct2	0x211
#define dma1_da2	0x212
#define dma1_sa2	0x213
#define dma1_cc2	0x214
#define dma1_cr3	0x218
#define dma1_ct3	0x219
#define dma1_da3	0x21a
#define dma1_sa3	0x21b
#define dma1_cc3	0x21c
#define dma1_sr0	0x220

#define g0_gcfg		0x240
#define g0_gintmask	0x241
#define g0_gintstatus	0x242
#define g0_g2dcfg	0x250
#define g0_g2dcmd	0x251
#define g0_g2dfgnd	0x252
#define g0_g2dbgnd	0x253
#define g0_g2dpar0	0x254
#define g0_g2dpar1	0x255
#define g0_g2dpar2	0x256
#define g0_g2dpar3	0x257
#define g0_g2dbltctl	0x258
#define g0_g2dbltsz	0x259
#define g0_g2dbltsr	0x25a
#define g0_g2dpmsr	0x25b
#define g0_g2drop	0x25c
#define g0_g2dckey	0x25d
#define g0_g2dpbmr	0x25e
#define g0_g2dpaer	0x25f
#define g0_scale_ctrl	0x260
#define g0_scale_hfact	0x261
#define g0_scale_vfact	0x262
#define g0_scale_hfilt	0x263
#define g0_scale_par0	0x264
#define g0_scale_par1	0x265
#define g0_scale_par2	0x266
#define g0_scale_par3	0x267
#define g0_scale_srcsz	0x268
#define g0_scale_destsz	0x269
#define g0_scale_stride	0x26a
#define g0_clc_cntl	0x270
#define g0_list_base	0x271
#define g0_list_addr	0x272
#define g0_branch_rtn	0x273
#define g0_sub_list_addr 0x274
#define g0_get_sub_list	0x275
#define g0_loop_cnt	0x276
#define g0_cond_br	0x277
#define g0_event_wait	0x278
#define g0_fid_line_cnt	0x279
#define g0_ad_pair_addr	0x27a
#define g0_addr_stack	0x27b
#define g0_match_reg	0x27c
#define g0_match_list	0x27d

#define s1394_pvr_config 0x2c0
#define pvr_start	0x2c1
#define s1394_startwr	0x2c2
#define s1394_startrd	0x2c3
#define pvr_addr	0x2c4
#define s1394_addrwr	0x2c5
#define s1394_addrrd	0x2c6
#define pvr_count	0x2c7
#define s1394_wrcnt	0x2c8
#define s1394_rdcnt	0x2c9
#define s1394_pvr_int	0x2ca
#define s1394_pvr_stat	0x2cb
#define s1394_sel	0x2cc

#define denc1_cr1	0x2e1
#define denc1_rr1	0x2e2
#define denc1_cr2	0x2e3
#define denc1_rr2	0x2e4
#define denc1_rr3	0x2e5
#define denc1_rr4	0x2e6
#define denc1_rr5	0x2e7
#define denc1_ccdr	0x2e8
#define denc1_cccr	0x2e9
#define denc1_trr	0x2ea
#define denc1_tosr	0x2eb
#define denc1_tesr	0x2ec
#define denc1_rlsr	0x2ed
#define denc1_vlsr	0x2ee
#define denc1_vsr	0x2ef
#define dencx_synclock	0x2f0
#define dencx_rra	0x2f1
#define dencx_rrb	0x2f2
#define dencx_dencmux	0x2f3

#define xpt0_config1	0x0000
#define xpt0_control1	0x0001
#define xpt0_festat	0x0002
#define xpt0_feimask	0x0003
#define xpt0_ocmcnfg	0x0004
#define xpt0_settapi	0x0005

#define xpt0_pcrhi	0x0010
#define xpt0_pcrlow	0x0011
#define xpt0_lstchi	0x0012
#define xpt0_lstclow	0x0013
#define xpt0_stchi	0x0014
#define xpt0_stclow	0x0015
#define xpt0_pwm	0x0016
#define xpt0_pcrstct	0x0017
#define xpt0_pcrstcd	0x0018
#define xpt0_stccomp	0x0019
#define xpt0_stccmpd	0x001a

#define xpt0_dsstat	0x0048
#define xpt0_dsimask	0x0049

#define xpt0_vcchng	0x01f0
#define xpt0_acchng	0x01f1
#define xpt0_axenable	0x01fe
#define xpt0_pcrpid	0x01ff

#define xpt0_config2	0x1000
#define xpt0_pbuflvl	0x1002
#define xpt0_intmask	0x1003
#define xpt0_plbcnfg	0x1004

#define xpt0_qint	0x1010
#define xpt0_qintmsk	0x1011
#define xpt0_astatus	0x1012
#define xpt0_aintmask	0x1013
#define xpt0_vstatus	0x1014
#define xpt0_vintmask	0x1015

#define xpt0_qbase	0x1020
#define xpt0_bucketq	0x1021
#define xpt0_qstops	0x1024
#define xpt0_qresets	0x1025
#define xpt0_sfchng	0x1026

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

#define IICMDBUF	0x00
#define IICSDBUF	0x02
#define IICLMADR	0x04
#define IICHMADR	0x05
#define IICCNTL		0x06
#define IICMDCNTL	0x07
#define IICSTS		0x08
#define IICEXTSTS	0x09
#define IICLSADR	0x0A
#define IICHSADR	0x0B
#define IICCLKDIV	0x0C
#define IICINTRMSK	0x0D
#define IICXFRCNT	0x0E
#define IICXTCNTLSS	0x0F
#define IICDIRECTCNTL	0x10

#endif
