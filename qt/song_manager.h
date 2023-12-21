#ifndef _SONG_MANAGER_H_
#define _SONG_MANAGER_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdbool.h>


void initialize_chip(void);

void set_songs(int song_length, void* p_songlines, void* p_tracks, void* p_instruments);

void optimize(void);

int get_chunk_information(int *p_chunk_size, int *p_offet_number);
int get_chunks(int *p_max_track, int *p_song_length, uint8_t *p_chunks, int *p_section_begin_indexes, int *p_offsets);

int set_chunks(int max_track, int song_length, uint8_t *p_chunks, int chunk_size);


void start_generating_song(int);
void start_generating_track(int);

void silence();
void iedplonk(int, int);

uint8_t interrupthandler();
bool is_song_generating(int *p_processing_song_index);
bool is_track_generating(int *p_generating_track_index, int *p_generating_line_index);

#ifdef __cplusplus
}
#endif

#endif // _SONG_MANAGER_H_
