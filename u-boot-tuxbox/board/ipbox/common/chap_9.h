#ifndef __CHAP_9_H__
#define __CHAP_9_H__

//*************************************************************************
// USB Protocol Layer
//*************************************************************************

//*************************************************************************
// USB standard device requests
//*************************************************************************
void get_status(void);
void clear_feature(void);
void set_feature(void);
void set_address(void);
void get_descriptor(void);
void get_configuration(void);
void set_configuration(void);
void get_interface(void);
void set_interface(void);

void reserved(void);

#ifdef CONFIG_NET2270
void Configuration_Handler (void);
#endif

#endif
