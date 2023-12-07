#ifndef TRACKPLAINTEXTEDIT_H
#define TRACKPLAINTEXTEDIT_H

#include <TuneManager.h>
#include <HighlightWholeLinePlainTextEdit.h>

class TrackPlainTextEdit : public HighlightWholeLinePlainTextEdit
{
	Q_OBJECT
public:
	TrackPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent = nullptr);

	void ShowTrack(int index);
	int UpdateMeasure(void);
public:
	signals:
	void ParseMeasureErrorOccurred(const QString &error_string);

private slots:
	void HandleGeneratingSongStateChanged(bool is_generating, int generating_song_index);
	void HandleGeneratingTrackStateChanged(bool is_generating, int generating_track_index, int generating_line_index);
private:
	int ParseDocument(bool is_update_to_memory);
	int ParseTokensToTrackline(QString note_string, QString instr_string, QList<QString> cmd_param_string_list,
							   TuneManager::trackline *p_trackline);

private:
	TuneManager *m_p_tune_manager;
	int m_current_shown_track_index;
};

#endif // TRACKPLAINTEXTEDIT_H
