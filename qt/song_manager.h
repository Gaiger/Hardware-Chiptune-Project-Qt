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


void initialize_chip(void);

void loadfile(char *fname);
void savefile(char *fname);

void optimize(void);

int get_chunk_information(int *p_chunk_size, int *p_offet_number);
int get_chunks(int *p_max_track, int *p_song_length, uint8_t *p_chunks, int *p_section_begin_indexes, int *p_offsets);

int set_chunks(int max_track, int song_length, uint8_t *p_chunks, int chunk_size);

void get_songlines(void** pp_songlines, int *p_number_of_songlines);
void set_songlines(void* p_songlines, int number_of_songlines);

void get_tracks(void ** pp_track, int *p_track_number, int *p_track_length);
void get_instruments(void ** pp_instruments, int *p_instrument_number);

bool is_song_playing(int *p_processing_song_index);
bool is_track_playing(int *p_playing_track_index, int *p_playing_line_index);
#ifdef __cplusplus
}
#endif

#endif // _SONG_MANAGER_H_
