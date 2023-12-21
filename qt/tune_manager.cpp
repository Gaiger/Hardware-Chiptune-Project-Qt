#include "TuneManager.h"
#include "tune_manager.h"
#include "../chiptune.h"

static TuneManager *s_p_tune_manager;

void set_tune_mananger(void *p_tune_manager)
{
	s_p_tune_manager = (TuneManager*)p_tune_manager;
}


static int get_max_track(void)
{
	return s_p_tune_manager->GetMaxTrack();
}


static int get_song_length(void)
{
	TuneManager::songline * p_songlines;
	int song_length;
	s_p_tune_manager->GetSongLines(&p_songlines, &song_length);
	return song_length;
}

static uint8_t get_chunk_datum(int index)
{
	return *(s_p_tune_manager->GetChunksPtr() + index);
}

void setup_chiptune_callback_functions(void)
{
	chiptune_setup_callback_functions(get_max_track, get_song_length, get_chunk_datum);
}

/**********************************************************************************/

static TuneManager::songline* get_songlines(void)
{
	TuneManager::songline * p_songlines;
	int song_length;
	s_p_tune_manager->GetSongLines(&p_songlines, &song_length);
	return p_songlines;
}

static TuneManager::track* get_tracks(void)
{
	TuneManager::track *p_tracks;
	int number_of_tracks;
	int track_length;
	s_p_tune_manager->GetTracks(&p_tracks, &number_of_tracks, &track_length);
	return p_tracks;
}

static TuneManager::instrument* get_instruments(void)
{
	TuneManager::instrument *p_instruments;
	int number_of_instruments;

	s_p_tune_manager->GetInstruments(&p_instruments, &number_of_instruments);
	return p_instruments;
}

static void readsong(int pos, int ch, uint8_t *dest) {
	TuneManager::songline *p_songines = get_songlines();
	dest[0] = p_songines[pos].track[ch];
	dest[1] = p_songines[pos].transp[ch];
}

static void readtrack(int num, int pos, struct trackline *tl) {
	TuneManager::track *p_tracks = get_tracks();
	tl->note = p_tracks[num].line[pos].note;
	tl->instr = p_tracks[num].line[pos].instr;
	tl->cmd[0] = p_tracks[num].line[pos].cmd[0];
	tl->cmd[1] = p_tracks[num].line[pos].cmd[1];
	tl->param[0] = p_tracks[num].line[pos].param[0];
	tl->param[1] = p_tracks[num].line[pos].param[1];
}

static void readinstr(int num, int pos, uint8_t *il) {
	TuneManager::instrument *p_instruments = get_instruments();
	if(pos >= p_instruments[num].length) {
		il[0] = 0;
		il[1] = 0;
	} else {
		il[0] = p_instruments[num].line[pos].cmd;
		il[1] = p_instruments[num].line[pos].param;
	}
}

void setup_chiptune_raw_reader(void)
{
	chiptune_setup_raw_data_reader(readsong, readtrack, readinstr);
}

/**********************************************************************************/

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

static int convert_to_chunks(int song_length, TuneManager::songline *p_songines,
							 TuneManager::track *p_tracks, TuneManager::instrument *p_instruments,
							 int max_track, uint8_t *p_chunks, int *p_chunk_size,
						 int *p_section_begin_indexes, int *p_offsets, int *p_offet_number)
{
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
			if(p_songines[i].transp[j]) {
				put_data(&packer, 1, 1);
				put_data(&packer, p_songines[i].track[j], 6);
				put_data(&packer, p_songines[i].transp[j], 4);
			} else {
				put_data(&packer, 0, 1);
				put_data(&packer, p_songines[i].track[j], 6);
			}
		}
	}

	for(int i = 1; i < 1 + PACKING_INSTRUMENT_NUMBER ; i++) {
		p_offsets[nres++] = align_to_byte(&packer);

		if(p_instruments[i].length > 1) {
			for(int j = 0; j < p_instruments[i].length; j++) {
				put_data(&packer, convert_to_cmd_id(p_instruments[i].line[j].cmd), 8);
				put_data(&packer, p_instruments[i].line[j].param, 8);
			}
		}

		put_data(&packer, 0, 8);
	}

	for(int i = 1; i <= max_track; i++) {
		p_offsets[nres++] = align_to_byte(&packer);

		for(int j = 0; j < TRACKLEN; j++) {
			put_data(&packer, !!p_tracks[i].line[j].note, 1);
			put_data(&packer, !!p_tracks[i].line[j].instr, 1);

			for(int k = 0; k < PACKING_TRACK_CMD_NUMBER; k++){
				uint8_t cmd_id = convert_to_cmd_id(p_tracks[i].line[j].cmd[k]);
				put_data(&packer, !!cmd_id, 1);
			}

			if(p_tracks[i].line[j].note) {
				put_data(&packer, p_tracks[i].line[j].note, 7);
			}

			if(p_tracks[i].line[j].instr) {
				put_data(&packer, p_tracks[i].line[j].instr, 4);
			}

			for(int k = 0; k < PACKING_TRACK_CMD_NUMBER; k++){
				uint8_t cmd_id = convert_to_cmd_id(p_tracks[i].line[j].cmd[k]);
				if(cmd_id) {
					put_data(&packer, cmd_id, 4);
					put_data(&packer, p_tracks[i].line[j].param[k], 8);
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

int get_packing_into_chunk_information(int song_length, TuneManager::songline *p_songines,
						  TuneManager::track *p_tracks, TuneManager::instrument *p_instruments,
						  int *p_chunk_size, int *p_offet_number)
{
	int offsets[256] = {0};

	int max_track = 0;
	for(int i = 0; i < song_length; i++) {
		for(int j = 0; j < 4; j++) {
			if(max_track < p_songines[i].track[j]) {
				max_track = p_songines[i].track[j];
			}
		}
	}

	convert_to_chunks(song_length, p_songines, p_tracks, p_instruments, max_track,
					  NULL, p_chunk_size, NULL,
						 offsets, p_offet_number);
	return 0;
}

/**********************************************************************************/

int pack_into_chunks(int song_length, TuneManager::songline *p_songines,
			   TuneManager::track *p_tracks, TuneManager::instrument *p_instruments,
			   int *p_max_track, uint8_t *p_chunks, int *p_section_begin_indexes, int *p_offsets)
{
	int max_track = 0;
	for(int i = 0; i < song_length; i++) {
		for(int j = 0; j < 4; j++) {
			if(max_track < p_songines[i].track[j]) max_track = p_songines[i].track[j];
		}
	}

	int chunks_length;
	int offet_number;
	int section_begin_indexes[1024] = {0};
	int offsets[1024] = {0};
	convert_to_chunks(song_length, p_songines, p_tracks, p_instruments,
					  max_track, NULL, &chunks_length, NULL, &offsets[0], &offet_number);
	convert_to_chunks(song_length, p_songines, p_tracks, p_instruments,
					  max_track, p_chunks, &chunks_length, &section_begin_indexes[0], &offsets[0], &offet_number);

	*p_max_track = max_track;
	if(nullptr != p_section_begin_indexes){
		memcpy(p_section_begin_indexes, &section_begin_indexes[0], sizeof(int) * offet_number);
	}
	if(nullptr != p_offsets){
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

int unpack_from_chunks(int max_track, int song_length, uint8_t *p_chunks, int chunk_size,
			   TuneManager::songline *p_songines, TuneManager::track *p_tracks, TuneManager::instrument *p_instruments)
{
	(void)chunk_size;
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
			p_songines[i].track[j] = track_index;
			p_songines[i].transp[j] = transp;
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

			p_tracks[i].line[j].note = note;
			p_tracks[i].line[j].instr = instr;
			for(int k = 0; k < 2; k++){
				p_tracks[i].line[j].cmd[k] = cmd[k];
				p_tracks[i].line[j].param[k] = param[k];
			}
		}
	}

	for(int i = 0; i < PACKING_INSTRUMENT_NUMBER; i++){
		p_instruments[i].length = (offsets[i + 1] - offsets[i] - 1)/2;
		struct unpacker_t instrument_unpacker;
		initialize_unpacker(&instrument_unpacker, p_chunks, offsets[i]);
		for(int j = 0; j < p_instruments[i].length; j++){
			uint8_t cmd_id = (uint8_t)fetch_bits(&instrument_unpacker, 8);
			uint8_t param = (uint8_t)fetch_bits(&instrument_unpacker, 8);
			uint8_t cmd = 0;
			if(0 != cmd_id){
				cmd = validcmds[cmd_id];
			}
			p_instruments[i].line[j].cmd = cmd;
			p_instruments[i].line[j].param = param;
		}
	}

	return 0;
}
