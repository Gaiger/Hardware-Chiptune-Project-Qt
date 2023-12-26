#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "chiptune.h"

int const s_track_length = TRACKLEN;

volatile uint8_t callbackwait;

static uint8_t s_trackwait;
static uint8_t s_track_line_index;
static uint8_t s_song_index;

static bool s_is_generating_song;
static bool s_is_generating_track;

static bool s_is_read_raw = false;

static uint8_t s_light_counter[2] = {0};

static int (*s_handle_get_max_track)(void) = NULL;
static int (*s_handle_get_song_length)(void) = NULL;
static uint8_t (*s_handle_get_chunk_datum)(int index) = NULL;

static void (*s_handle_set_lights_enabled)(uint8_t light_bits) = NULL;

static void (*s_handle_read_song)(int pos, int ch, uint8_t *dest) = NULL;
static void (*s_handle_read_track)(int num, int pos, struct trackline *tl) = NULL;
static void (*s_handle_read_instr)(int num, int pos, uint8_t *il) = NULL;


/*const uint16_t freqtable[] = {
	0x010b, 0x011b, 0x012c, 0x013e, 0x0151, 0x0165, 0x017a, 0x0191, 0x01a9,
	0x01c2, 0x01dd, 0x01f9, 0x0217, 0x0237, 0x0259, 0x027d, 0x02a3, 0x02cb,
	0x02f5, 0x0322, 0x0352, 0x0385, 0x03ba, 0x03f3, 0x042f, 0x046f, 0x04b2,
	0x04fa, 0x0546, 0x0596, 0x05eb, 0x0645, 0x06a5, 0x070a, 0x0775, 0x07e6,
	0x085f, 0x08de, 0x0965, 0x09f4, 0x0a8c, 0x0b2c, 0x0bd6, 0x0c8b, 0x0d4a,
	0x0e14, 0x0eea, 0x0fcd, 0x10be, 0x11bd, 0x12cb, 0x13e9, 0x1518, 0x1659,
	0x17ad, 0x1916, 0x1a94, 0x1c28, 0x1dd5, 0x1f9b, 0x217c, 0x237a, 0x2596,
	0x27d3, 0x2a31, 0x2cb3, 0x2f5b, 0x322c, 0x3528, 0x3851, 0x3bab, 0x3f37,
	0x42f9, 0x46f5, 0x4b2d, 0x4fa6, 0x5462, 0x5967, 0x5eb7, 0x6459, 0x6a51,
	0x70a3, 0x7756, 0x7e6f
};*/

const uint16_t freqtable[] = {
	0x0085, 0x008d, 0x0096, 0x009f, 0x00a8, 0x00b2, 0x00bd, 0x00c8, 0x00d4,
	0x00e1, 0x00ee, 0x00fc, 0x010b, 0x011b, 0x012c, 0x013e, 0x0151, 0x0165,
	0x017a, 0x0191, 0x01a9, 0x01c2, 0x01dd, 0x01f9, 0x0217, 0x0237, 0x0259,
	0x027d, 0x02a3, 0x02cb, 0x02f5, 0x0322, 0x0352, 0x0385, 0x03ba, 0x03f3,
	0x042f, 0x046f, 0x04b2, 0x04fa, 0x0546, 0x0596, 0x05eb, 0x0645, 0x06a5,
	0x070a, 0x0775, 0x07e6, 0x085f, 0x08de, 0x0965, 0x09f4, 0x0a8c, 0x0b2c,
	0x0bd6, 0x0c8b, 0x0d4a, 0x0e14, 0x0eea, 0x0fcd, 0x10be, 0x11bd, 0x12cb,
	0x13e9, 0x1518, 0x1659, 0x17ad, 0x1916, 0x1a94, 0x1c28, 0x1dd5, 0x1f9b,
	0x217c, 0x237a, 0x2596, 0x27d3, 0x2a31, 0x2cb3, 0x2f5b, 0x322c, 0x3528,
	0x3851, 0x3bab, 0x3f37
};

const int8_t sinetable[] = {
	0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 106, 112, 117, 122, 125, 126,
	127, 126, 125, 122, 117, 112, 106, 98, 90, 81, 71, 60, 49, 37, 25, 12,
	0, -12, -25, -37, -49, -60, -71, -81, -90, -98, -106, -112, -117, -122,
	-125, -126, -127, -126, -125, -122, -117, -112, -106, -98, -90, -81,
	-71, -60, -49, -37, -25, -12
};

const char validcmds[] = "0dfijlmtvw~+=";
uint16_t s_offsets[256];

enum {
	WF_TRI,
	WF_SAW,
	WF_PUL,
	WF_NOI
};

volatile struct oscillator
{
	uint16_t	freq;
	uint16_t	phase;
	uint16_t	duty;
	uint8_t		waveform;
	uint8_t		volume;	// 0-255
} osc[4];

struct channel
{
	uint8_t		tnum;
	int8_t		transp;
	uint8_t		tnote;
	uint8_t		lastinstr;
	uint8_t		inum;
	uint8_t		iptr;
	uint8_t		iwait;
	uint8_t		inote;
	int8_t		bendd;
	int16_t		bend;
	int8_t		volumed;
	int16_t		dutyd;
	uint8_t		vdepth;
	uint8_t		vrate;
	uint8_t		vpos;
	int16_t		inertia;
	uint16_t	slur;
} channel[4];



struct unpacker_t
{
	uint16_t	next_byte_offset;
	uint8_t		working_byte;
	uint8_t		remain_bit_number;
};

struct unpacker_t s_song_unpacker;
struct unpacker_t s_track_unpacker[4];

/**********************************************************************************/

static void initialize_unpacker(struct unpacker_t *p_unpacker, uint16_t offset)
{
	p_unpacker->next_byte_offset = offset;
	p_unpacker->remain_bit_number = 0;
}

/**********************************************************************************/

static uint8_t fetch_one_bit(struct unpacker_t *p_unpacker)
{
	uint8_t val;

	if(0 == p_unpacker->remain_bit_number)
	{
		p_unpacker->working_byte = s_handle_get_chunk_datum(p_unpacker->next_byte_offset++);
		p_unpacker->remain_bit_number = 8;
	}

	p_unpacker->remain_bit_number--;
	val = p_unpacker->working_byte & 1;
	p_unpacker->working_byte >>= 1;

	return val;
}

/**********************************************************************************/

static uint16_t fetch_bits(struct unpacker_t *p_unpacker, uint8_t n)
{
	uint16_t val = 0;
	uint8_t i;

	for(i = 0; i < n; i++)
	{
		if(fetch_one_bit(p_unpacker))
		{
			val |= (1 << i);
		}
	}

	return val;
}

static void get_instrument(uint8_t instrument_index, uint8_t line_index,
								uint8_t * p_cmd_param)
{
	uint8_t cmd_id = s_handle_get_chunk_datum(s_offsets[instrument_index] + 2 * line_index + 0);
	uint8_t param =s_handle_get_chunk_datum(s_offsets[instrument_index] + 2 * line_index  + 1);
	uint8_t cmd = 0;
	if(0 != cmd_id){
		cmd = validcmds[cmd_id];
	}

	p_cmd_param[0] = cmd;
	p_cmd_param[1] = param;
}

/**********************************************************************************/

void chiptune_silence()
{
	uint8_t i;

	for(i = 0; i < 4; i++) {
		osc[i].volume = 0;
	}
	s_is_generating_song = false;
	s_is_generating_track = false;
}

void runcmd(uint8_t ch, uint8_t cmd, uint8_t param)
{
	switch(cmd) {
		case 0:
			channel[ch].inum = 0;
			break;
		case 'd':
			osc[ch].duty = param << 8;
			break;
		case 'f':
			channel[ch].volumed = param;
			break;
		case 'i':
			channel[ch].inertia = param << 1;
			break;
		case 'j':
			channel[ch].iptr = param;
			break;
		case 'l':
			channel[ch].bendd = param;
			break;
		case 'm':
			channel[ch].dutyd = param << 6;
			break;
		case 't':
			channel[ch].iwait = param;
			break;
		case 'v':
			osc[ch].volume = param;
			break;
		case 'w':
			osc[ch].waveform = param;
			break;
		case '+':
			channel[ch].inote = param + channel[ch].tnote - 12 * 4;
			break;
		case '=':
			channel[ch].inote = param;
			break;
		case '~':
			if(channel[ch].vdepth != (param >> 4)) {
				channel[ch].vpos = 0;
			}
			channel[ch].vdepth = param >> 4;
			channel[ch].vrate = param & 15;
			break;
	}
}

void chiptune_iedplonk(int note, int instr) {
	channel[0].tnote = note;
	channel[0].inum = instr;
	channel[0].iptr = 0;
	channel[0].iwait = 0;
	channel[0].bend = 0;
	channel[0].bendd = 0;
	channel[0].volumed = 0;
	channel[0].dutyd = 0;
	channel[0].vdepth = 0;
}

void chiptune_start_generating_track(int track_index)
{
	channel[0].tnum = track_index;
	channel[1].tnum = 0;
	channel[2].tnum = 0;
	channel[3].tnum = 0;
	s_track_line_index = 0;
	s_trackwait = 0;
	s_is_generating_track = true;
	s_is_generating_song = false;
}

void chiptune_start_generating_song(int song_position)
{
	s_song_index = song_position;
	s_track_line_index = 0;
	s_trackwait = 0;
	s_is_generating_track = false;
	s_is_generating_song = true;

	do
	{
		if(true == s_is_read_raw){
			break;
		}
		initialize_unpacker(&s_song_unpacker, s_offsets[0]);
		for(int i = 0; i < song_position * 4; i++){
			uint8_t is_transp = (uint8_t)fetch_bits(&s_song_unpacker, 1);
			fetch_bits(&s_song_unpacker, 6);
			if(0 != is_transp){
				fetch_bits(&s_song_unpacker, 4);
			}
		}
	}while(0);
}


void generate_routine() {			// called at 50 Hz

	do
	{
		if(false == s_is_generating_track && false == s_is_generating_song){
			break;
		}

		if(0 != s_trackwait){
			s_trackwait -= 1;
			break;
		}
		s_trackwait = 4;

		do
		{
			if(0 != s_track_line_index){
				break;
			}

			if(false == s_is_generating_song){
				break;
			}

			if( s_handle_get_song_length() <= s_song_index) {
				s_is_generating_song = false;
				break;
			}

			for(uint8_t ch = 0; ch < 4; ch++) {
				do
				{
					if(true == s_is_read_raw){
						uint8_t tmp[2];
						s_handle_read_song(s_song_index, ch, tmp);
						channel[ch].tnum = tmp[0];
						channel[ch].transp = tmp[1];
						break;
					}

					uint8_t is_transp = (uint8_t)fetch_bits(&s_song_unpacker, 1);
					uint8_t track_index = (uint8_t)fetch_bits(&s_song_unpacker, 6);
					uint8_t transp = 0;
					if(0 != is_transp){
						transp = (uint8_t)fetch_bits(&s_song_unpacker, 4);
						if(transp & 0x8) {
							transp |= 0xf0;
						}
					}
					channel[ch].tnum = track_index;
					channel[ch].transp = transp;
				}while(0);
			}
			s_song_index++;
		}while(0);

		do
		{
			if(false == s_is_generating_track && false == s_is_generating_song){
				break;
			}

			do
			{
				if(true == s_is_read_raw){
					break;
				}

				if(0 == s_track_line_index){
					for(uint8_t ch = 0; ch < 4; ch++) {
						if(0 == channel[ch].tnum){
							continue;
						}
						initialize_unpacker(&s_track_unpacker[ch],
											s_offsets[1 + PACKING_INSTRUMENT_NUMBER + (channel[ch].tnum - 1)]);
					}
				}
			}while(0);


			for(uint8_t ch = 0; ch < 4; ch++) {
				if(0 == channel[ch].tnum) {
					continue;
				}

				struct trackline tl;
				uint8_t instr = 0;
				do
				{
					if(true == s_is_read_raw){
						s_handle_read_track(channel[ch].tnum, s_track_line_index, &tl);
						break;
					}

					uint8_t note = 0;
					uint8_t cmd[2] = {0};
					uint8_t	param[2] = {0};

					uint8_t fields = (uint8_t)fetch_bits(&s_track_unpacker[ch], 2 + PACKING_TRACK_CMD_NUMBER);
					if(0x01 & (fields >> 0)){
						note = (uint8_t)fetch_bits(&s_track_unpacker[ch], 7);
					}
					if(0x01 & (fields >> 1)){
						instr = (uint8_t)fetch_bits(&s_track_unpacker[ch], 4);
					}
					for(int k = 0; k < PACKING_TRACK_CMD_NUMBER; k++){
						if(0x01 & (fields >> (k + 2)) ){
							uint8_t cmd_id = (uint8_t)fetch_bits(&s_track_unpacker[ch], 4);
							if(0 != cmd_id){
								cmd[k] = validcmds[cmd_id];
								param[k] = (uint8_t)fetch_bits(&s_track_unpacker[ch], 8);
							}
						}
					}

					tl.note = note;
					tl.instr = instr;
					for(int k = 0; k < 2; k++){
						tl.cmd[k] = cmd[k];
						tl.param[k] = param[k];
					}
				}while(0);

				if(tl.note) {
					channel[ch].tnote = tl.note + channel[ch].transp;
					if(0 == tl.instr){
						instr = channel[ch].lastinstr;
					}
				}

				if(instr) {

					if(instr == 2) {
						s_light_counter[1] = 5;
					}
					if(instr == 1) {
						s_light_counter[0] = 5;
						if(channel[ch].tnum == 4) {
							s_light_counter[0] = s_light_counter[1] = 3;
						}
					}
					if(instr == 7) {
						s_light_counter[0] = s_light_counter[1] = 30;
					}

					channel[ch].lastinstr = instr;
					channel[ch].inum = instr;
					channel[ch].iptr = 0;
					channel[ch].iwait = 0;
					channel[ch].bend = 0;
					channel[ch].bendd = 0;
					channel[ch].volumed = 0;
					channel[ch].dutyd = 0;
					channel[ch].vdepth = 0;
				}
				if(tl.cmd[0])
					runcmd(ch, tl.cmd[0], tl.param[0]);
				/*if(tl.cmd[1])
					runcmd(ch, tl.cmd[1], tl.param[1]);*/
			}

			s_track_line_index++;
			s_track_line_index %= s_track_length;
		}while(0);
	}while(0);

	for(uint8_t ch = 0; ch < 4; ch++) {
		int16_t vol;
		uint16_t duty;
		uint16_t slur;

		while(channel[ch].inum && !channel[ch].iwait) {
			uint8_t il[2];
			do
			{
				if(true == s_is_read_raw)
				{
					s_handle_read_instr(channel[ch].inum, channel[ch].iptr, il);
					break;
				}
				get_instrument(channel[ch].inum, channel[ch].iptr, il);
			}while(0);
			channel[ch].iptr++;

			runcmd(ch, il[0], il[1]);
		}
		if(channel[ch].iwait) {
			channel[ch].iwait--;
		}

		slur = freqtable[channel[ch].inote];
		do
		{
			if (0 == channel[ch].inertia) {
				break;
			}

			int16_t diff;
			slur = channel[ch].slur;
			diff = freqtable[channel[ch].inote] - slur;
			//diff >>= channel[ch].inertia;
			do
			{
				if(0 < diff){
					if(diff > channel[ch].inertia) {
						diff = channel[ch].inertia;
					}
					break;
				}

				if(0 > diff){
					if(diff < -channel[ch].inertia) {
						diff = -channel[ch].inertia;
					}
				}
			}while(0);
			slur += diff;
			channel[ch].slur = slur;
		}while(0);

		osc[ch].freq =
			slur +
			channel[ch].bend +
			((channel[ch].vdepth * sinetable[channel[ch].vpos & 63]) >> 2);
		channel[ch].bend += channel[ch].bendd;
		vol = osc[ch].volume + channel[ch].volumed;
		if(vol < 0) {
			vol = 0;
		}
		if(vol > 255) {
			vol = 255;
		}
		osc[ch].volume = (uint8_t)vol;

		duty = osc[ch].duty + channel[ch].dutyd;
		if(duty > 0xe000) {
			duty = 0x2000;
		}
		if(duty < 0x2000) {
			duty = 0xe000;
		}
		osc[ch].duty = duty;

		channel[ch].vpos += channel[ch].vrate;
	}

	if(NULL != s_handle_set_lights_enabled){
		uint8_t light_bits = 0;
		if(0 != s_light_counter[0]) {
			s_light_counter[0] -= 1;
			light_bits |= 0x01;
		}

		if(0 != s_light_counter[1]) {
			s_light_counter[1] -= 1;
			light_bits |= (0x01 << 1);
		}
		s_handle_set_lights_enabled(light_bits);
	}
}

void chiptune_setup_data_callback_functions( int(*handle_get_max_track)(void),
										int(*handle_get_song_length)(void),
										uint8_t(*handle_get_chunk_datum)(int index))
{
	s_handle_get_max_track = handle_get_max_track;
	s_handle_get_song_length = handle_get_song_length;
	s_handle_get_chunk_datum = handle_get_chunk_datum;
}

void chiptune_setup_lights_callback_function(void (*handle_set_lights_enabled)(uint8_t light_bits))
{
	s_handle_set_lights_enabled = handle_set_lights_enabled;
}

void chiptune_setup_raw_data_reader( void (*handle_read_song)(int pos, int ch, uint8_t *dest),
							  void (*handle_read_track)(int num, int pos, struct trackline *tl),
							  void (*handle_read_instr)(int num, int pos, uint8_t *il))
{
	s_handle_read_song = handle_read_song;
	s_handle_read_track = handle_read_track;
	s_handle_read_instr = handle_read_instr;
}


void chiptune_initialize(bool is_read_raw)
{
	s_is_read_raw = is_read_raw;
	s_trackwait = 0;
	s_track_line_index = 0;
	s_is_generating_song = false;
	s_is_generating_track = false;

	osc[0].volume = 0;
	channel[0].inum = 0;
	osc[1].volume = 0;
	channel[1].inum = 0;
	osc[2].volume = 0;
	channel[2].inum = 0;
	osc[3].volume = 0;
	channel[3].inum = 0;

	do
	{
		struct unpacker_t temp_unpacker;
		initialize_unpacker(&temp_unpacker, 0);
		for(int i = 0; i < 1 + PACKING_INSTRUMENT_NUMBER + s_handle_get_max_track(); i++){
			s_offsets[i] = fetch_bits(&temp_unpacker, 13);
		}
	}while(0);
}

static uint32_t s_noiseseed = 1;

uint8_t chiptune_interrupthandler()
{
	uint8_t i;
	int16_t acc;
	uint8_t newbit;

	newbit = 0;
	if(0x80000000L & s_noiseseed){
		newbit ^= 1;
	}
	if(0x01000000L & s_noiseseed){
		newbit ^= 1;
	}
	if(0x00000040L & s_noiseseed){
		newbit ^= 1;
	}
	if(0x00000200L & s_noiseseed){
		newbit ^= 1;
	}
	s_noiseseed = (s_noiseseed << 1) | newbit;

	if(callbackwait) {
		callbackwait--;
	} else {
		generate_routine();
		callbackwait = 180 - 1;
	}

	acc = 0;
	for(i = 0; i < 4; i++) {
		int8_t value; // [-32,31]

		switch(osc[i].waveform) {
			case WF_TRI:
				if(osc[i].phase < 0x8000) {
					value = -32 + (osc[i].phase >> 9);
				} else {
					value = 31 - ((osc[i].phase - 0x8000) >> 9);
				}
				break;
			case WF_SAW:
				value = -32 + (osc[i].phase >> 10);
				break;
			case WF_PUL:
				value = (osc[i].phase > osc[i].duty)? -32 : 31;
				break;
			case WF_NOI:
				value = (s_noiseseed & 63) - 32;
				break;
			default:
				value = 0;
				break;
		}
		acc += value * osc[i].volume; // rhs = [-8160,7905]

		osc[i].phase += osc[i].freq;
	}
	// acc [-32640,31620]
	return 128 + (acc >> 8);	// [1,251]
}

int const get_track_length(void){ return s_track_length; }

bool chiptune_is_song_generating(int *p_processing_song_index)
{
	if(false == s_is_generating_song){
		*p_processing_song_index = -1;
		return false;
	}

	*p_processing_song_index = s_song_index;
	return true;
}

bool chiptune_is_track_generating(int *p_generating_track_index, int *p_generating_line_index)
{
	if(false == s_is_generating_track){
		*p_generating_track_index = -1;
		*p_generating_line_index = -1;
		return false;
	}

	*p_generating_track_index = channel[0].inum;
	*p_generating_line_index = s_track_line_index;
	return true;
}
