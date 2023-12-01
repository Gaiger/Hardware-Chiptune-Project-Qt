#ifndef SONGPLAINTEXTEDIT_H
#define SONGPLAINTEXTEDIT_H

#include <TuneManager.h>
#include <QPlainTextEdit>

class SongPlainTextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit SongPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent = nullptr);

	void ShowSongs(void);

private slots:
	void HandlePlayingSongStateChanged(bool is_playing, int playing_song_index);
private slots:
	void HandleCursorPositionChanged(void);

private:
	void CorrectCursorPosition(void);
	void HighlightCurrentLine(void);

private:
	TuneManager *m_p_tune_manager;
	int m_previous_textcuror_position;
};

#endif // SONGPLAINTEXTEDIT_H
