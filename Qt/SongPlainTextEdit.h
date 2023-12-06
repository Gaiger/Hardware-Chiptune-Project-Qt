#ifndef SONGPLAINTEXTEDIT_H
#define SONGPLAINTEXTEDIT_H

#include <QPlainTextEdit>
#include <TuneManager.h>

class SongPlainTextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit SongPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent = nullptr);

	void ShowSongs(void);
	int UpdateScores(void);
public:
	signals:
	void ParseScoresErrorOccurred(const QString &error_string);

private slots:
	void HandleGeneratingSongStateChanged(bool is_generating, int generating_song_index);
	void HandleGeneratingTrackStateChanged(bool is_generating, int generating_track_index, int generating_line_index);
private slots:
	void HandleCursorPositionChanged(void);
private:
	int ParseDocument(void);

private:
	void CorrectCursorPosition(void);
	void HighlightCurrentLine(void);

private:
	TuneManager *m_p_tune_manager;
	int m_previous_textcuror_position;
};

#endif // SONGPLAINTEXTEDIT_H
