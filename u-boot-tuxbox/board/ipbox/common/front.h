#ifndef _DGS_FRONT_H
#define _DGS_FRONT_H

extern int front_waitkey( int *keys, int timeout );
extern int front_singlekey( int timeout );
extern void front_puts (const char *s);
extern int front_getkey (void);
extern int front_tstc (void);
extern int front_persent( int now, int total );

enum front_key
{
	key_null = 0,
	key_power,
	key_left,
	key_right,
	key_up,
	key_down,
	key_ok,

	key_0,
	key_1,
	key_2,
	key_3,
	key_4,
	key_5,
	key_6,
	key_7,
	key_8,
	key_9,

	key_release,

	key_front_power,
	key_front_left,
	key_front_right,
	key_front_up,
	key_front_down,
	key_front_ok,

	key_front_p_left,	/* power + left */
	key_front_p_right,	/* power + right */
	key_front_p_up,		/* power + up */
	key_front_p_down,	/* power + down */
	key_front_p_ok,		/* power + ok */

	key_front_release,
};

#endif
