#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <QDebug>
#include "TrackPlainTextEdit.h"

TrackPlainTextEdit::TrackPlainTextEdit(QWidget *parent)
	:  QPlainTextEdit(parent)
{

}

/**********************************************************************************/


/**********************************************************************************/

extern "C"
{
#include "../stuff.h"
const char * const notenames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "H-"};

void get_tracks(struct track ** pp_track, int *p_track_number, int *p_track_length);
bool is_track_playing(int *p_playing_track_index);
}

/**********************************************************************************/

void TrackPlainTextEdit::UpdateTrack(void)
{

	struct track *p_track;
	int track_number;
	int track_length;
	get_tracks(&p_track, &track_number, &track_length);
	int track_index = 1;
	//qDebug() << "track ";// << Qt::hex << index;
	for(int i = 0; i < track_length; i++){
		char line_buffer[1024];
		char string_buffer[1024];
		snprintf(line_buffer, sizeof(line_buffer), "%02x: ", i);
		if(p_track[track_index].line[i].note) {
			snprintf(string_buffer, sizeof(string_buffer), "%s%d",
				notenames[(p_track[track_index].line[i].note - 1) % 12],
				(p_track[track_index].line[i].note - 1) / 12);
		} else {
			snprintf(string_buffer, sizeof(string_buffer), "---");
		}
		strncat(&line_buffer[0], string_buffer, sizeof(line_buffer));

		snprintf(&string_buffer[0], sizeof(string_buffer), " %02x", p_track[track_index].line[i].instr);
		strncat(&line_buffer[0], string_buffer, sizeof(line_buffer));
		for(int j = 0; j < 2; j++) {
			if(p_track[track_index].line[i].cmd[j]) {
				snprintf(&string_buffer[0], sizeof(string_buffer), " %c%02x",
					p_track[track_index].line[i].cmd[j],
					p_track[track_index].line[i].param[j]);
			} else {
				snprintf(&string_buffer[0], sizeof(string_buffer), " ...");
			}
			strncat(&line_buffer[0], string_buffer, sizeof(line_buffer));
		}
#if(0)
		if(playtrack && ((i + 1) % tracklen) == trackpos) {
			addch('*');
		}
#endif
		//printf("%s \r\n",  &line_buffer[0]);
		qDebug() << QString(&line_buffer[0]);
	}

}
