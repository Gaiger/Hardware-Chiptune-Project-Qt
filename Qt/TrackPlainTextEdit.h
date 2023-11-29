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
	void UpdateTrackPlaying(void);
private:
	int m_current_shown_track_index;
};

#endif // TRACKPLAINTEXTEDIT_H
