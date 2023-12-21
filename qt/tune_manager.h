#ifndef TUNE_MANAGER_H
#define TUNE_MANAGER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "TuneManager.h"

void set_tune_mananger(void *p_tune_manager);
void setup_chiptune_callback_functions(void);
void setup_chiptune_raw_reader(void);

int get_packing_into_chunk_information(int song_length, TuneManager::songline *p_songines,
						  TuneManager::track *p_tracks, TuneManager::instrument *p_instruments,
						  int *p_chunk_size, int *p_offet_number);
int pack_into_chunks(int song_length, TuneManager::songline *p_songines,
			   TuneManager::track *p_tracks, TuneManager::instrument *p_instruments,
			   int *p_max_track, uint8_t *p_chunks, int *p_section_begin_indexes, int *p_offsets);
int unpack_from_chunks(int max_track, int song_length, uint8_t *p_chunks, int chunk_size,
			   TuneManager::songline *p_songines, TuneManager::track *p_tracks, TuneManager::instrument *p_instruments);

#ifdef __cplusplus
}
#endif

#endif // TUNE_MANAGER_H
