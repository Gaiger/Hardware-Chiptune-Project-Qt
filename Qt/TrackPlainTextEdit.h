#ifndef TRACKPLAINTEXTEDIT_H
#define TRACKPLAINTEXTEDIT_H

#include <TuneManager.h>
#include <QPlainTextEdit>

class TrackPlainTextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	TrackPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent = nullptr);
	void ShowTrack(int index);

private slots:
	void HandlePlayingTrackStateChanged(bool is_playing, int playing_track_index, int playing_line_index);
private:
	TuneManager *m_p_tune_manager;
	int m_current_shown_track_index;
};

#endif // TRACKPLAINTEXTEDIT_H
