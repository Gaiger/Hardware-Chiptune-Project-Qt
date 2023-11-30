#ifndef SONGPLAINTEXTEDIT_H
#define SONGPLAINTEXTEDIT_H

#include <QPlainTextEdit>

class SongPlainTextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit SongPlainTextEdit(QWidget *parent = nullptr);

	void UpdateSongs(void);
	void UpdateSongPlaying(void);

public slots:
	void HandlePlayingSongStateChanged(bool is_playing, int playing_song_index);
private slots:
	void HandleCursorPositionChanged(void);

private:
	void CorrectCursorPosition(void);
	void HighlightCurrentLine(void);

private:
	int m_previous_textcuror_position;
};

#endif // SONGPLAINTEXTEDIT_H
