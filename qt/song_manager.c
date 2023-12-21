#ifdef _MSC_VER
	#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "tune_manager.h"
#include "song_manager.h"

static int s_max_track;
uint8_t s_chunks[24 * 1024] = {0};
int s_chunk_size = 0;

int get_max_track(void){ return s_max_track; }
uint8_t * get_chunks_ptr(void){ return &s_chunks[0]; }


int const get_track_length(void);

extern char validcmds[14];


#define TRACKLEN									(32)

struct songline {
	uint8_t			track[4];
	uint8_t			transp[4];
};

struct trackline {
	uint8_t	note;
	uint8_t	instr;
	uint8_t	cmd[2];
	uint8_t	param[2];
	};

struct track {
	struct trackline	line[TRACKLEN];
};

struct instrline {
	uint8_t			cmd;
	uint8_t			param;
};

struct instrument {
	int			length;
	struct instrline	line[256];
};

struct songline *s_p_songines;
struct track *s_p_tracks;
struct instrument *s_p_instruments;



static void readsong(int pos, int ch, uint8_t *dest) {
	dest[0] = s_p_songines[pos].track[ch];
	dest[1] = s_p_songines[pos].transp[ch];
}

static void readtrack(int num, int pos, struct trackline *tl) {
	tl->note = s_p_tracks[num].line[pos].note;
	tl->instr = s_p_tracks[num].line[pos].instr;
	tl->cmd[0] = s_p_tracks[num].line[pos].cmd[0];
	tl->cmd[1] = s_p_tracks[num].line[pos].cmd[1];
	tl->param[0] = s_p_tracks[num].line[pos].param[0];
	tl->param[1] = s_p_tracks[num].line[pos].param[1];
}

static void readinstr(int num, int pos, uint8_t *il) {
	if(pos >= s_p_instruments[num].length) {
		il[0] = 0;
		il[1] = 0;
	} else {
		il[0] = s_p_instruments[num].line[pos].cmd;
		il[1] = s_p_instruments[num].line[pos].param;
	}
}

void initchip(void);

void setup_raw_data_reader( void (*p_handle_read_song)(int pos, int ch, uint8_t *dest),
							  void (*p_handle_read_track)(int num, int pos, struct trackline *tl),
							  void (*p_handle_read_instr)(int num, int pos, uint8_t *il));

void initialize_chip(void)
{
	//setup_raw_data_reader(readsong, readtrack, readinstr);
	initchip();
}

void set_songs(int song_length, void* p_songlines, void* p_tracks, void* p_instruments)
{
	s_p_songines = p_songlines;
	s_p_tracks = p_tracks;
	s_p_instruments = p_instruments;
}


void optimize() {
	uint8_t used[256], replace[256];
	int i, j;

	memset(used, 0, sizeof(used));
	int song_length = get_song_length();
	for(i = 0; i < song_length; i++) {
		for(j = 0; j < 4; j++) {
			used[s_p_songines[i].track[j]] = 1;
		}
	}

	j = 1;
	replace[0] = 0;
	for(i = 1; i < 256; i++) {
		if(used[i]) {
			replace[i] = j;
			j++;
		} else {
			replace[i] = 0;
		}
	}

	for(i = 1; i < 256; i++) {
		if(replace[i] && replace[i] != i) {
			memcpy(&s_p_tracks[replace[i]], &s_p_tracks[i], sizeof(struct track));
		}
	}

	for(i = 0; i < song_length; i++) {
		for(j = 0; j < 4; j++) {
			s_p_songines[i].track[j] = replace[s_p_songines[i].track[j]];
		}
	}

	for(i = 1; i < 256; i++) {
		uint8_t last = 255;

		for(j = 0; j < get_track_length(); j++) {
			if(s_p_tracks[i].line[j].instr) {
				if(s_p_tracks[i].line[j].instr == last) {
					s_p_tracks[i].line[j].instr = 0;
				} else {
					last = s_p_tracks[i].line[j].instr;
				}
			}
		}
	}
	get_chunk_information(&s_chunk_size, NULL);
	get_chunks(&s_max_track, &song_length, &s_chunks[0], NULL, NULL);
	initialize_chip();
}

#define PACKING_INSTRUMENT_NUMBER					(15)
#define PACKING_TRACK_CMD_NUMBER					(1)

/**********************************************************************************/

struct packer_t
{
	uint8_t *p_chunk;
	int bits;
	int bit_counter;
	int chunk_size;

	int *p_section_index;
};

/**********************************************************************************/

static void put_one_bit(struct packer_t *p_packer, int x) {
	if(x) {
		p_packer->bits |= (1 << p_packer->bit_counter);
	}
	p_packer->bit_counter += 1;
	if(p_packer->bit_counter == 8) {
		if(p_packer->p_chunk) {
			p_packer->p_chunk[0] = p_packer->bits;
			p_packer->p_chunk += 1;
		}
		p_packer->chunk_size += 1;
		p_packer->bits = 0;
		p_packer->bit_counter = 0;
	}
}

/**********************************************************************************/

static void put_data(struct packer_t *p_packer, int data, int bits)
{
	for(int i = 0; i < bits; i++) {
		put_one_bit(p_packer, !!(data & (1 << i)));
	}
}

/**********************************************************************************/

static int align_to_byte(struct packer_t *p_packer)
{
	if(p_packer->bit_counter) {
		if(p_packer->p_chunk) {
			p_packer->p_chunk[0] = p_packer->bits;
			p_packer->p_chunk += 1;
		}
		p_packer->chunk_size += 1;
		p_packer->bits = 0;
		p_packer->bit_counter = 0;
	}

	if(NULL != p_packer->p_chunk){
		if(NULL != p_packer->p_section_index){
			p_packer->p_section_index[0] = p_packer->chunk_size;
			p_packer->p_section_index += 1;
		}
	}

	return p_packer->chunk_size;
}

/**********************************************************************************/

static int convert_to_cmd_id(uint8_t ch) {
	if(!ch) return 0;
	if(strchr(validcmds, ch)) {
		return (int)(strchr(validcmds, ch) - validcmds);
	}
	return 0;
}

/**********************************************************************************/

static int convert_to_chunks(int max_track, uint8_t *p_chunks, int *p_chunk_size,
						 int *p_section_begin_indexes, int *p_offsets, int *p_offet_number) {

	int song_length = get_song_length();
	struct packer_t packer;
	memset(&packer,0, sizeof(struct packer_t));
	packer.p_chunk = p_chunks;
	packer.p_section_index = p_section_begin_indexes;

	for(int i = 0; i < 1 + PACKING_INSTRUMENT_NUMBER + max_track; i++) {
		put_data(&packer, p_offsets[i], 13);
	}

	int nres = 0;
	p_offsets[nres++] = align_to_byte(&packer);
	for(int i = 0; i < song_length; i++) {
		for(int j = 0; j < 4; j++) {
			if(s_p_songines[i].transp[j]) {
				put_data(&packer, 1, 1);
				put_data(&packer, s_p_songines[i].track[j], 6);
				put_data(&packer, s_p_songines[i].transp[j], 4);
			} else {
				put_data(&packer, 0, 1);
				put_data(&packer, s_p_songines[i].track[j], 6);
			}
		}
	}

	for(int i = 1; i < 1 + PACKING_INSTRUMENT_NUMBER ; i++) {
		p_offsets[nres++] = align_to_byte(&packer);

		if(s_p_instruments[i].length > 1) {
			for(int j = 0; j < s_p_instruments[i].length; j++) {
				put_data(&packer, convert_to_cmd_id(s_p_instruments[i].line[j].cmd), 8);
				put_data(&packer, s_p_instruments[i].line[j].param, 8);
			}
		}

		put_data(&packer, 0, 8);
	}

	for(int i = 1; i <= max_track; i++) {
		p_offsets[nres++] = align_to_byte(&packer);

		for(int j = 0; j < get_track_length(); j++) {
			put_data(&packer, !!s_p_tracks[i].line[j].note, 1);
			put_data(&packer, !!s_p_tracks[i].line[j].instr, 1);

			for(int k = 0; k < PACKING_TRACK_CMD_NUMBER; k++){
				uint8_t cmd_id = convert_to_cmd_id(s_p_tracks[i].line[j].cmd[k]);
				put_data(&packer, !!cmd_id, 1);
			}

			if(s_p_tracks[i].line[j].note) {
				put_data(&packer, s_p_tracks[i].line[j].note, 7);
			}

			if(s_p_tracks[i].line[j].instr) {
				put_data(&packer, s_p_tracks[i].line[j].instr, 4);
			}

			for(int k = 0; k < PACKING_TRACK_CMD_NUMBER; k++){
				uint8_t cmd_id = convert_to_cmd_id(s_p_tracks[i].line[j].cmd[k]);
				if(cmd_id) {
					put_data(&packer, cmd_id, 4);
					put_data(&packer, s_p_tracks[i].line[j].param[k], 8);
				}
			}
		}
	}

	*p_chunk_size = packer.chunk_size;
	if(NULL != p_offet_number){
		*p_offet_number = nres;
	}
	return 0;
}

/**********************************************************************************/

int get_chunk_information(int *p_chunk_size, int *p_offet_number)
{
	int song_length = get_song_length();
	int offsets[256] = {0};

	int max_track = 0;
	for(int i = 0; i < song_length; i++) {
		for(int j = 0; j < 4; j++) {
			if(max_track < s_p_songines[i].track[j]) {
				max_track = s_p_songines[i].track[j];
			}
		}
	}

	convert_to_chunks(max_track, NULL, p_chunk_size, NULL,
						 offsets, p_offet_number);
	return 0;
}

/**********************************************************************************/

int get_chunks(int *p_max_track, int *p_song_length, uint8_t *p_chunks, int *p_section_begin_indexes, int *p_offsets)
{
	int max_track = 0;
	int song_length = get_song_length();
	for(int i = 0; i < song_length; i++) {
		for(int j = 0; j < 4; j++) {
			if(max_track < s_p_songines[i].track[j]) max_track = s_p_songines[i].track[j];
		}
	}

	int chunks_length;
	int offet_number;
	int section_begin_indexes[1024] = {0};
	int offsets[1024] = {0};
	convert_to_chunks(max_track, NULL, &chunks_length, NULL, &offsets[0], &offet_number);
	convert_to_chunks(max_track, p_chunks, &chunks_length, &section_begin_indexes[0], &offsets[0], &offet_number);

	*p_max_track = max_track;
	*p_song_length = song_length;
	if(NULL != p_section_begin_indexes){
		memcpy(p_section_begin_indexes, &section_begin_indexes[0], sizeof(int) * offet_number);
	}
	if(NULL != p_offsets){
		memcpy(p_offsets, &offsets[0], sizeof(int) * offet_number);
	}
	return 0;
}

/**********************************************************************************/

struct unpacker_t
{
	uint8_t		*p_data;
	uint16_t	next_byte_offset;
	uint8_t		working_byte;
	uint8_t		remain_bit_number;
};

/**********************************************************************************/

static void initialize_unpacker(struct unpacker_t *p_unpacker, uint8_t * p_data, uint16_t offset)
{
	p_unpacker->p_data = p_data;
	p_unpacker->next_byte_offset = offset;
	p_unpacker->remain_bit_number = 0;
}

/**********************************************************************************/

static uint8_t fetch_one_bit(struct unpacker_t *p_unpacker)
{
	uint8_t val;

	if(0 == p_unpacker->remain_bit_number)
	{
		p_unpacker->working_byte = p_unpacker->p_data[p_unpacker->next_byte_offset++];
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

/**********************************************************************************/

int set_chunks(int max_track, int song_length, uint8_t *p_chunks, int chunk_size)
{
	memcpy(&s_chunks[0], p_chunks, chunk_size);
	s_chunk_size = chunk_size;

	uint16_t offsets[256] = {0};
	struct unpacker_t song_unpacker;
	do
	{
		struct unpacker_t temp_unpacker;
		initialize_unpacker(&temp_unpacker, p_chunks, 0);
		for(int i = 0; i < 1 + PACKING_INSTRUMENT_NUMBER + max_track; i++){
			offsets[i] = fetch_bits(&temp_unpacker, 13);
		}
	}while(0);

	initialize_unpacker(&song_unpacker, p_chunks, offsets[0]);
	song_length = get_song_length();
	for(int i = 0; i < song_length; i++){
		for(int j = 0; j < 4; j++){
			uint8_t is_transp = (uint8_t)fetch_bits(&song_unpacker, 1);
			uint8_t track_index = (uint8_t)fetch_bits(&song_unpacker, 6);
			uint8_t transp = 0;
			if(0 != is_transp)
			{
				transp = (uint8_t)fetch_bits(&song_unpacker, 4);
				if(transp & 0x8) {
					transp |= 0xf0;
				}
			}
			s_p_songines[i].track[j] = track_index;
			s_p_songines[i].transp[j] = transp;
		}
	}

	for(int i = 0; i < max_track; i++){
		struct unpacker_t track_unpacker;
		initialize_unpacker(&track_unpacker, p_chunks, offsets[1 + PACKING_INSTRUMENT_NUMBER + (i - 1)]);
		for(int j = 0; j < TRACKLEN; j++){
			uint8_t note = 0;
			uint8_t instr = 0;
			uint8_t cmd[2] = {0};
			uint8_t	param[2] = {0};

			uint8_t fields = (uint8_t)fetch_bits(&track_unpacker, 2 + PACKING_TRACK_CMD_NUMBER);
			if(0x01 & (fields >> 0) ){
				note = (uint8_t)fetch_bits(&track_unpacker, 7);
			}
			if(0x01 & (fields >> 1) ){
				instr = (uint8_t)fetch_bits(&track_unpacker, 4);
			}
			for(int k = 0; k < PACKING_TRACK_CMD_NUMBER; k++){
				if(0x01 & (fields >> (k + 2)) ){
					uint8_t cmd_id = (uint8_t)fetch_bits(&track_unpacker, 4);
					if(0 != cmd_id){
						cmd[k] = validcmds[cmd_id];
						param[k] = (uint8_t)fetch_bits(&track_unpacker, 8);
					}
				}
			}

			s_p_tracks[i].line[j].note = note;
			s_p_tracks[i].line[j].instr = instr;
			for(int k = 0; k < 2; k++){
				s_p_tracks[i].line[j].cmd[k] = cmd[k];
				s_p_tracks[i].line[j].param[k] = param[k];
			}
		}
	}

	for(int i = 0; i < PACKING_INSTRUMENT_NUMBER; i++){
		s_p_instruments[i].length = (offsets[i + 1] - offsets[i] - 1)/2;
		struct unpacker_t instrument_unpacker;
		initialize_unpacker(&instrument_unpacker, p_chunks, offsets[i]);
		for(int j = 0; j < s_p_instruments[i].length; j++){
			uint8_t cmd_id = (uint8_t)fetch_bits(&instrument_unpacker, 8);
			uint8_t param = (uint8_t)fetch_bits(&instrument_unpacker, 8);
			uint8_t cmd = 0;
			if(0 != cmd_id){
				cmd = validcmds[cmd_id];
			}
			s_p_instruments[i].line[j].cmd = cmd;
			s_p_instruments[i].line[j].param = param;
		}
	}

	return 0;
}
