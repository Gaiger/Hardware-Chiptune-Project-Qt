#ifdef _MSC_VER
	#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#ifndef _STUFF_H_
#define _STUFF_H_
	#if defined TRACKLEN
		#undef TRACKLEN
	#endif
	#include "../stuff.h"
#endif
#include "song_manager.h"

int const get_track_length(void);
int get_song_length(void);
void set_song_length(int songlen);
uint8_t get_track_position(void);
uint8_t get_song_position(void);
bool is_generating_song(void);
bool is_generating_track(void);



extern char validcmds[14];

struct instrline {
	uint8_t			cmd;
	uint8_t			param;
};

struct instrument {
	int			length;
	struct instrline	line[256];
};

struct songline {
	uint8_t			track[4];
	uint8_t			transp[4];
};

static int s_song_length;
int get_song_length(void){return s_song_length;}

struct instrument instrument[256], iclip;
struct track track[256], tclip;
struct songline song[256];



static void readsong(int pos, int ch, uint8_t *dest) {
	dest[0] = song[pos].track[ch];
	dest[1] = song[pos].transp[ch];
}

static void readtrack(int num, int pos, struct trackline *tl) {
	tl->note = track[num].line[pos].note;
	tl->instr = track[num].line[pos].instr;
	tl->cmd[0] = track[num].line[pos].cmd[0];
	tl->cmd[1] = track[num].line[pos].cmd[1];
	tl->param[0] = track[num].line[pos].param[0];
	tl->param[1] = track[num].line[pos].param[1];
}

static void readinstr(int num, int pos, uint8_t *il) {
	if(pos >= instrument[num].length) {
		il[0] = 0;
		il[1] = 0;
	} else {
		il[0] = instrument[num].line[pos].cmd;
		il[1] = instrument[num].line[pos].param;
	}
}

extern void initchip(void (*p_handle_read_song)(int pos, int ch, uint8_t *dest),
						  void (*p_handle_read_track)(int num, int pos, struct trackline *tl),
						  void (*p_handle_read_instr)(int num, int pos, uint8_t *il));

void initialize_chip(void)
{
	initchip(readsong, readtrack, readinstr);
}

void savefile(char *fname) {
	FILE *f;
	int i, j;

	f = fopen(fname, "w");
	if(!f) {
		fprintf(stderr, "save error!\n");
		return;
	}

	fprintf(f, "musicchip tune\n");
	fprintf(f, "version 1\n");
	fprintf(f, "\n");
	for(i = 0; i < s_song_length; i++) {
		fprintf(f, "songline %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i,
			song[i].track[0],
			song[i].transp[0],
			song[i].track[1],
			song[i].transp[1],
			song[i].track[2],
			song[i].transp[2],
			song[i].track[3],
			song[i].transp[3]);
	}
	fprintf(f, "\n");
	for(i = 1; i < 256; i++) {
		for(j = 0; j < get_track_length(); j++) {
			struct trackline *tl = &track[i].line[j];

			if(tl->note || tl->instr || tl->cmd[0] || tl->cmd[1]) {
				fprintf(f, "trackline %02x %02x %02x %02x %02x %02x %02x %02x\n",
					i,
					j,
					tl->note,
					tl->instr,
					tl->cmd[0],
					tl->param[0],
					tl->cmd[1],
					tl->param[1]);
			}
		}
	}
	fprintf(f, "\n");
	for(i = 1; i < 256; i++) {
		if(instrument[i].length > 1) {
			for(j = 0; j < instrument[i].length; j++) {
				fprintf(f, "instrumentline %02x %02x %02x %02x\n",
					i,
					j,
					instrument[i].line[j].cmd,
					instrument[i].line[j].param);
			}
		}
	}

	fclose(f);
}

void loadfile(char *fname) {
	memset(&song[0], 0, sizeof(song));
	memset(&track[0], 0, sizeof(track));
	memset(&instrument[0], 0, sizeof(instrument));

	FILE *f;
	char buf[1024];
	int cmd[3];
	int i1, i2, trk[4], transp[4], param[3], note, instr;
	int i;

	f = fopen(fname, "r");
	if(!f) {
		return;
	}

	s_song_length = 1;
	while(!feof(f) && fgets(buf, sizeof(buf), f)) {
		if(9 == sscanf(buf, "songline %x %x %x %x %x %x %x %x %x",
			&i1,
			&trk[0],
			&transp[0],
			&trk[1],
			&transp[1],
			&trk[2],
			&transp[2],
			&trk[3],
			&transp[3])) {

			for(i = 0; i < 4; i++) {
				song[i1].track[i] = trk[i];
				song[i1].transp[i] = transp[i];
			}
			if(s_song_length <= i1) s_song_length = i1 + 1;
		} else if(8 == sscanf(buf, "trackline %x %x %x %x %x %x %x %x",
			&i1,
			&i2,
			&note,
			&instr,
			&cmd[0],
			&param[0],
			&cmd[1],
			&param[1])) {

			track[i1].line[i2].note = note;
			track[i1].line[i2].instr = instr;
			for(i = 0; i < 2; i++) {
				track[i1].line[i2].cmd[i] = cmd[i];
				track[i1].line[i2].param[i] = param[i];
			}
		} else if(4 == sscanf(buf, "instrumentline %x %x %x %x",
			&i1,
			&i2,
			&cmd[0],
			&param[0])) {

			instrument[i1].line[i2].cmd = cmd[0];
			instrument[i1].line[i2].param = param[0];
			if(instrument[i1].length <= i2) instrument[i1].length = i2 + 1;
		}
	}

	fclose(f);
}

void get_songlines(void** pp_songlines, int *p_number_of_songlines)
{
	*pp_songlines = (void**)&song[0];
	*p_number_of_songlines = s_song_length;
	return;
}

void set_songlines(void* p_songlines, int number_of_songlines)
{
	//TODO :: p_songlines
	s_song_length = number_of_songlines;
}

bool is_song_playing(int *p_processing_song_index)
{
	if(false == is_generating_song()){
		return false;
	}

	*p_processing_song_index = get_song_position();
	return true;
}

void get_tracks(void ** pp_track, int *p_track_number, int *p_track_length)
{
	*pp_track = (void**)&track[0];
	*p_track_number = sizeof(track)/sizeof(struct track);
	*p_track_length = get_track_length();
}

struct channel {
	uint8_t	tnum;
	int8_t	transp;
	uint8_t	tnote;
	uint8_t	lastinstr;
	uint8_t	inum;
	uint8_t	iptr;
	uint8_t	iwait;
	uint8_t	inote;
	int8_t	bendd;
	int16_t	bend;
	int8_t	volumed;
	int16_t	dutyd;
	uint8_t	vdepth;
	uint8_t	vrate;
	uint8_t	vpos;
	int16_t	inertia;
	uint16_t	slur;
} ;

extern struct channel channel[4];

bool is_track_playing(int *p_playing_track_index, int *p_playing_line_index)
{
	if(false == is_generating_track()){
		return false;
	}

	*p_playing_track_index = channel[0].tnum;
	*p_playing_line_index = get_track_position();
	return true;
}

void get_instruments(void ** pp_instruments, int *p_instrument_number)
{
	*pp_instruments = &instrument[0];
	*p_instrument_number = sizeof(instrument)/sizeof(struct instrument);
}


void optimize() {
	uint8_t used[256], replace[256];
	int i, j;

	memset(used, 0, sizeof(used));
	for(i = 0; i < s_song_length; i++) {
		for(j = 0; j < 4; j++) {
			used[song[i].track[j]] = 1;
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
			memcpy(&track[replace[i]], &track[i], sizeof(struct track));
		}
	}

	for(i = 0; i < s_song_length; i++) {
		for(j = 0; j < 4; j++) {
			song[i].track[j] = replace[song[i].track[j]];
		}
	}

	for(i = 1; i < 256; i++) {
		uint8_t last = 255;

		for(j = 0; j < get_track_length(); j++) {
			if(track[i].line[j].instr) {
				if(track[i].line[j].instr == last) {
					track[i].line[j].instr = 0;
				} else {
					last = track[i].line[j].instr;
				}
			}
		}
	}
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

	struct packer_t packer;
	memset(&packer,0, sizeof(struct packer_t));
	packer.p_chunk = p_chunks;
	packer.p_section_index = p_section_begin_indexes;

	for(int i = 0; i < 1 + PACKING_INSTRUMENT_NUMBER + max_track; i++) {
		put_data(&packer, p_offsets[i], 13);
	}

	int nres = 0;
	p_offsets[nres++] = align_to_byte(&packer);
	for(int i = 0; i < s_song_length; i++) {
		for(int j = 0; j < 4; j++) {
			if(song[i].transp[j]) {
				put_data(&packer, 1, 1);
				put_data(&packer, song[i].track[j], 6);
				put_data(&packer, song[i].transp[j], 4);
			} else {
				put_data(&packer, 0, 1);
				put_data(&packer, song[i].track[j], 6);
			}
		}
	}

	for(int i = 1; i < 1 + PACKING_INSTRUMENT_NUMBER ; i++) {
		p_offsets[nres++] = align_to_byte(&packer);

		if(instrument[i].length > 1) {
			for(int j = 0; j < instrument[i].length; j++) {
				put_data(&packer, convert_to_cmd_id(instrument[i].line[j].cmd), 8);
				put_data(&packer, instrument[i].line[j].param, 8);
			}
		}

		put_data(&packer, 0, 8);
	}

	for(int i = 1; i <= max_track; i++) {
		p_offsets[nres++] = align_to_byte(&packer);

		for(int j = 0; j < get_track_length(); j++) {
			put_data(&packer, !!track[i].line[j].note, 1);
			put_data(&packer, !!track[i].line[j].instr, 1);

			for(int k = 0; k < PACKING_TRACK_CMD_NUMBER; k++){
				uint8_t cmd_id = convert_to_cmd_id(track[i].line[j].cmd[k]);
				put_data(&packer, !!cmd_id, 1);
			}

			if(track[i].line[j].note) {
				put_data(&packer, track[i].line[j].note, 7);
			}

			if(track[i].line[j].instr) {
				put_data(&packer, track[i].line[j].instr, 4);
			}

			for(int k = 0; k < PACKING_TRACK_CMD_NUMBER; k++){
				uint8_t cmd_id = convert_to_cmd_id(track[i].line[j].cmd[k]);
				if(cmd_id) {
					put_data(&packer, cmd_id, 4);
					put_data(&packer, track[i].line[j].param[k], 8);
				}
			}
		}
	}

	*p_chunk_size = packer.chunk_size;
	*p_offet_number = nres;
	return 0;
}

/**********************************************************************************/

int get_chunk_information(int *p_chunk_size, int *p_offet_number)
{
	int offsets[256] = {0};

	int max_track = 0;
	for(int i = 0; i < s_song_length; i++) {
		for(int j = 0; j < 4; j++) {
			if(max_track < song[i].track[j]) max_track = song[i].track[j];
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
	for(int i = 0; i < s_song_length; i++) {
		for(int j = 0; j < 4; j++) {
			if(max_track < song[i].track[j]) max_track = song[i].track[j];
		}
	}

	int chunks_length;
	int offet_number;
	convert_to_chunks(max_track, NULL, &chunks_length, NULL, p_offsets, &offet_number);
	convert_to_chunks(max_track, p_chunks, &chunks_length, p_section_begin_indexes, p_offsets, &offet_number);

	*p_max_track = max_track;
	*p_song_length = s_song_length;
	return 0;
}

/**********************************************************************************/

struct unpacker_t
{
	uint8_t		*p_data;
	uint16_t	nextbyte;
	uint8_t		buffer;
	uint8_t		bits;
};

/**********************************************************************************/

void initialize_unpacker(struct unpacker_t *p_unpacker, uint8_t * p_data, uint16_t offset)
{
	p_unpacker->p_data = p_data;
	p_unpacker->nextbyte = offset;
	p_unpacker->bits = 0;
}

/**********************************************************************************/

uint8_t fetch_one_bit(struct unpacker_t *p_unpacker)
{
	uint8_t val;

	if(0 == p_unpacker->bits)
	{
		p_unpacker->buffer = p_unpacker->p_data[p_unpacker->nextbyte++];
		p_unpacker->bits = 8;
	}

	p_unpacker->bits--;
	val = p_unpacker->buffer & 1;
	p_unpacker->buffer >>= 1;

	return val;
}

/**********************************************************************************/

uint16_t fetch_bits(struct unpacker_t *p_unpacker, uint8_t n)
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
	(void)chunk_size;
	s_song_length = song_length;
	memset(&song[0], 0, sizeof(song));
	memset(&track[0], 0, sizeof(track));
	memset(&instrument[0], 0, sizeof(instrument));
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
	bool is_track_unpacked_map[256] = {false};
	for(int i = 0; i < s_song_length; i++){
		for(int j = 0; j < 4; j++){
			uint8_t is_transp = (uint8_t)fetch_bits(&song_unpacker, 1);
			uint8_t track_index = (uint8_t)fetch_bits(&song_unpacker, 6);
			uint8_t transp = 0;
			if(is_transp)
			{
			   transp = (uint8_t)fetch_bits(&song_unpacker, 4);
			   if(transp & 0x8) {
				   transp |= 0xf0;
			   }
			}
			song[i].track[j] = track_index;
			song[i].transp[j] = transp;

			do
			{
				if(0 == track_index){
					break;
				}
				if(true == is_track_unpacked_map[track_index]){
					break;
				}

				struct unpacker_t track_unpacker;
				initialize_unpacker(&track_unpacker, p_chunks, offsets[1 + PACKING_INSTRUMENT_NUMBER + (track_index - 1)]);
				for(int k = 0; k < TRACKLEN; k++){
					uint8_t note = 0;
					uint8_t instr = 0;
					uint8_t cmd[2] = {0};
					uint8_t	param[2] = {0};

					uint8_t fields = (uint8_t)fetch_bits(&track_unpacker, 2 + PACKING_TRACK_CMD_NUMBER);
					if((fields >> 0) & 0x01){
						note = (uint8_t)fetch_bits(&track_unpacker, 7);
					}
					if((fields >> 1) & 0x01){
						instr = (uint8_t)fetch_bits(&track_unpacker, 4);
					}
					for(int m = 0; m < PACKING_TRACK_CMD_NUMBER; m++){
						if((fields >> (m + 2)) & 0x01){
							uint8_t cmd_id = (uint8_t)fetch_bits(&track_unpacker, 4);
							if(0 != cmd_id){
								cmd[m] = validcmds[cmd_id];
								param[m] = (uint8_t)fetch_bits(&track_unpacker, 8);
							}
						}
					}

					track[track_index].line[k].note = note;
					track[track_index].line[k].instr = instr;
					for(int m = 0; m < 2; m++){
						track[track_index].line[k].cmd[m] = cmd[m];
						track[track_index].line[k].param[m] = param[m];
					}
				}
				is_track_unpacked_map[track_index] = true;
			}while(0);
		}
	}

	for(int i = 0; i < PACKING_INSTRUMENT_NUMBER; i++){
		instrument[i].length = (offsets[i + 1] - offsets[i] - 1)/2;
		struct unpacker_t instrument_unpacker;
		initialize_unpacker(&instrument_unpacker, p_chunks, offsets[i]);
		for(int j = 0; j < instrument[i].length; j++){
			uint8_t cmd_id = (uint8_t)fetch_bits(&instrument_unpacker, 8);
			uint8_t param = (uint8_t)fetch_bits(&instrument_unpacker, 8);
			uint8_t cmd = 0;
			if(0 != cmd_id){
				cmd = validcmds[cmd_id];
			}
			instrument[i].line[j].cmd = cmd;
			instrument[i].line[j].param = param;
		}
	}

	return 0;
}
