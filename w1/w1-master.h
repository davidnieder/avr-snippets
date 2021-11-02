#ifndef W1_MASTER_H
#define W1_MASTER_H

#include <avr/io.h>

/* registers, pin of the bus */
#define W1DREG DDRC
#define W1OUTREG PORTC
#define W1INREG PINC
#define W1PIN PC0

typedef uint8_t w1id_t[8];

struct w1_search_state	{
	int8_t last_deviation;
	w1id_t device_id;
};

#define W1_INIT_SLAVES_PRESENT 0
#define W1_INIT_NO_PRESENCE 1

#define W1_INITIAL_SEARCH_STATE { -1, {} }
#define W1_SEARCH_FAILED -1
#define W1_SEARCH_DONE 0
#define W1_SEARCH_MORE_AVAIL 1


/*
 *
 */
uint8_t w1_reset();

/*
 *
 */
void w1_write_byte(uint8_t byte);

/*
 *
 */
uint8_t w1_read_byte();

/*
 *
 */
void w1_skip_rom();

/*
 *
 */
void w1_read_rom(w1id_t device);

/*
 *
 */
void w1_match_rom(w1id_t device);

/*
 *
 */
int8_t w1_search_rom(struct w1_search_state *state);

/*
 *
 */
uint8_t w1_find_devices(w1id_t *id_buffer, uint8_t size);

/*
 *
uint8_t w1_alarm_search(w1id_t *device_buffer, uint8_t size);
 */

/*
 *
int8_t w1_family_search(uint8_t family_code, w1id_t *device_buffer, uint8_t size);
 */

/*
 * get a human readable representation of the given 1-wire id.
 * 'buf' needs to be at least 17 bytes wide.
 *
 * returns the pointer to 'buf'
 */
char *w1_id2str(w1id_t id, char *buf);

#endif
