#ifndef _FRONT_DEV_H
#define _FRONT_DEV_H

extern int front_init (void);
extern void front_putc (const char c);
extern int front_send_packet(const char *packet);
extern int front_getc (void);
extern int front_convert( char letter );
extern int front_send_u2( unsigned char *packet );

#endif
