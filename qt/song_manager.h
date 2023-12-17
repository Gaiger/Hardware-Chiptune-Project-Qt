#ifndef _SONG_MANAGER_H_
#define _SONG_MANAGER_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdbool.h>

#ifndef _STUFF_H_
#define _STUFF_H_
	#if defined TRACKLEN
		#undef TRACKLEN
	#endif
	#include "../stuff.h"
#endif


void readsong(int pos, int ch, u8 *dest);
void readtrack(int num, int pos, struct trackline *tl);
void readinstr(int num, int pos, u8 *il);

void loadfile(char *fname);
void savefile(char *fname);

void optimize(void);

int get_chunk_information(int *p_chunk_length, int *p_offet_number);
int get_chunks(int *p_maxtrack, int *p_songlen, uint8_t *p_chunks, int *p_section_begin_indexes, int *p_offsets);

int set_chunks(int maxtrack, int songlen, uint8_t *p_chunks, int chunk_length);

void get_songlines(void ** pp_songlines, int **pp_number_of_songlines);
void get_tracks(void ** pp_track, int *p_track_number, int *p_track_length);
void get_instruments(void ** pp_instruments, int *p_instrument_number);

bool is_song_playing(int *p_processing_song_index);
bool is_track_playing(int *p_playing_track_index, int *p_playing_line_index);
#ifdef __cplusplus
}
#endif

#endif // _SONG_MANAGER_H_
