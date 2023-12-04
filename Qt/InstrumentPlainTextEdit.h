#ifndef INSTRUMENTPLAINTEXTEDIT_H
#define INSTRUMENTPLAINTEXTEDIT_H

#include <QPlainTextEdit>
#include <TuneManager.h>

class InstrumentPlainTextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit InstrumentPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent = nullptr);

	void ShowInstrument(int index);
private slots:
	void HandleGeneratingSongStateChanged(bool is_generating, int generating_song_index);
	void HandleGeneratingTrackStateChanged(bool is_generating, int generating_track_index, int generating_line_index);
private:
	TuneManager *m_p_tune_manager;
};

#endif // INSTRUMENTPLAINTEXTEDIT_H
