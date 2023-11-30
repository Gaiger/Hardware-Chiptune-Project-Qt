#ifndef TRACKPLAINTEXTEDIT_H
#define TRACKPLAINTEXTEDIT_H

#include <QPlainTextEdit>

class TrackPlainTextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	TrackPlainTextEdit(QWidget *parent = nullptr);
	void UpdateTrack(void);
	void UpdateShowedTrack(int i);

public slots:
	void HandlePlayingTrackStateChanged(bool is_playing, int playing_track_index, int playing_line_index);
private:
	int m_current_shown_track_index;
};

#endif // TRACKPLAINTEXTEDIT_H
