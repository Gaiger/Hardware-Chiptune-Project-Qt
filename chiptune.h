#ifndef _CHIPTUNE_H_
#define _CHIPTUNE_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

#define TRACKLEN									(32)

void chiptune_initialize(bool is_read_raw);

void chiptune_start_generating_song(int);
void chiptune_start_generating_track(int);

void chiptune_silence();
void chiptune_iedplonk(int, int);

uint8_t chiptune_interrupthandler();
bool chiptune_is_song_generating(int *p_processing_song_index);
bool chiptune_is_track_generating(int *p_generating_track_index, int *p_generating_line_index);

extern const char validcmds[14];

#define PACKING_INSTRUMENT_NUMBER					(15)
#define PACKING_TRACK_CMD_NUMBER					(1)

struct trackline {
	uint8_t	note;
	uint8_t	instr;
	uint8_t	cmd[2];
	uint8_t	param[2];
};

void chiptune_setup_data_callback_functions( int(*handle_get_max_track)(void),
										int(*handle_get_song_length)(void),
										uint8_t(*handle_get_chunk_datum)(int index));

void chiptune_setup_lights_callback_function(void (*handle_set_lights_enabled)(uint8_t light_bits));

void chiptune_setup_raw_data_reader( void (*handle_read_song)(int pos, int ch, uint8_t *dest),
							  void (*handle_read_track)(int num, int pos, struct trackline *tl),
							  void (*handle_read_instr)(int num, int pos, uint8_t *il));

#ifdef __cplusplus
}
#endif


#endif // _CHIPTUNE_H_
