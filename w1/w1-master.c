#include <util/delay.h>
#include "w1-master.h"


#define w1_bus_pull_low() W1DREG |= (1<<W1PIN); W1OUTREG &= ~(1<<W1PIN);
#define w1_bus_release() W1DREG &= ~(1<<W1PIN); W1OUTREG &= ~(1<<W1PIN);

#define SKIP_ROM		0xCC
#define READ_ROM		0x33
#define MATCH_ROM		0x55
#define SEARCH_ROM		0xF0
#define ALARM_SEARCH	0xEC


uint8_t
w1_reset()
{
	uint8_t slave_detected;

	/* reset signal, pull bus low for eight time slots */
	w1_bus_pull_low();
	_delay_us(480);
	w1_bus_release();

	/* sample bus for presence */
	_delay_us(60+10);
	slave_detected = !(W1INREG & (1<<W1PIN));
	_delay_us(480-(60+10));

	return slave_detected ? W1_INIT_SLAVES_PRESENT : W1_INIT_NO_PRESENCE;
}

static void
w1_write_bit(uint8_t bit)
{
	if (bit)	{
		/* "write 1" signal */
		w1_bus_pull_low();
		_delay_us(6);
		w1_bus_release();
		_delay_us(64);
	} else {
		/* "write 0" signal */
		w1_bus_pull_low();
		_delay_us(60);
		w1_bus_release();
		_delay_us(10);
	}
}

static uint8_t
w1_read_bit()
{
	uint8_t result;

	w1_bus_pull_low();
	_delay_us(6);
	w1_bus_release();
	_delay_us(9);
	result = (W1INREG & (1<<W1PIN));
	_delay_us(55);

	return result;
}

void
w1_write_byte(uint8_t byte)
{
	uint8_t i;
	for (i=0; i<8; i++)	{
		w1_write_bit(byte & (1<<i));
	}
}

uint8_t
w1_read_byte()
{
	uint8_t i, byte = 0;
	for (i=0; i<8; i++)	{
		byte |= (w1_read_bit() << i);
	}

	return byte;
}

void
w1_skip_rom()
{
	w1_write_byte(SKIP_ROM);
}

void
w1_read_rom(w1id_t dev)
{
	w1_write_byte(READ_ROM);
	for (int8_t i=0; i<8; i++)	{
		*dev++ = w1_read_byte();
	}
}

void
w1_match_rom(w1id_t dev_id)
{
	w1_write_byte(MATCH_ROM);
	for (int8_t i=0; i<8; i++)	{
		w1_write_byte(dev_id[i]);
	}
}

int8_t
w1_search_rom(struct w1_search_state *state)
{
	uint8_t id_bit, id_bit_compl;
	int8_t ret, new_deviation = -1;

	ret = w1_reset();
	if (ret == W1_INIT_NO_PRESENCE) return W1_SEARCH_FAILED;
	w1_write_byte(SEARCH_ROM);

	for (uint8_t pos=0; pos<64; pos++)	{
		id_bit = w1_read_bit();
		id_bit_compl = w1_read_bit();

		if (id_bit & id_bit_compl)	{
			/* read two 1-bits */
			return W1_SEARCH_FAILED;
		}
		if (id_bit ^ id_bit_compl)	{
			/* bit value is equal among all active slaves */
			if (id_bit)
				state->device_id[pos/8] |= (1<<(pos%8));
			else
				state->device_id[pos/8] &= ~(1<<(pos%8));
		}
		else	{
			/* bit value differs among active slaves */
			if (pos == state->last_deviation)	{
				/* this is the highest index at which a choice was made
				 * in the last search. take the other path this time */
				state->device_id[pos/8] |= (1<<(pos%8));
			}
			else if (pos > state->last_deviation)	{
				/* take 0 path first */
				state->device_id[pos/8] &= ~(1<<(pos%8));
				new_deviation = pos;
			}
			else	{
				/* follow path of previous search */
				if (!(state->device_id[pos/8] & (1<<(pos%8))))	{
					new_deviation = pos;
				}
			}
		}

		/* signal selected search path */
		w1_write_bit(state->device_id[pos/8] & (1<<(pos%8)));
	}

	state->last_deviation = new_deviation;
	if (new_deviation == -1)	{
		return W1_SEARCH_DONE;
	} else	{
		return W1_SEARCH_MORE_AVAIL;
	}
}

uint8_t
w1_find_devices(w1id_t *dev_buf, uint8_t buf_size)
{
	int8_t ret;
	uint8_t cnt = 0;
	struct w1_search_state search_state = W1_INITIAL_SEARCH_STATE;

	while (cnt < buf_size) {
		ret = w1_search_rom(&search_state);
		if (ret == W1_SEARCH_FAILED) {
			return 0;
		}

		for (uint8_t i=0; i<8; i++) {
			dev_buf[cnt][i] = search_state.device_id[i];
		}

		cnt++;
		if (ret == W1_SEARCH_DONE) {
			break;
		}
	}
	return cnt;
}

char *
w1_id2str(w1id_t id, char *buf)
{
	uint8_t hn, ln;

	for (uint8_t i=0; i<8; i++) {
		hn = (id[i] & 0xF0) >> 4;
		ln = (id[i] & 0x0F);
		hn = hn < 10 ? hn + '0' : hn + 'A'-10;
		ln = ln < 10 ? ln + '0' : ln + 'A'-10;

		buf[i*2] = hn;
		buf[i*2+1] = ln;
	}
	buf[16] = '\0';
	return buf;
}
