#include "TuneManager.h"
#include "tune_manager.h"

static TuneManager *s_p_tune_manager;

void set_tune_mananger(void *p_tune_manager)
{
	s_p_tune_manager = (TuneManager*)p_tune_manager;
}

int get_song_length(void)
{
	TuneManager::songline * p_songlines;
	int song_length;
	s_p_tune_manager->GetSongLines(&p_songlines, &song_length);
	return song_length;
}
