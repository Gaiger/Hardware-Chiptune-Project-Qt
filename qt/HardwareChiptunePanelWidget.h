#ifndef HARDWARECHIPTUNEPANELWIDGET_H
#define HARDWARECHIPTUNEPANELWIDGET_H

#include <QWidget>

#include "AudioPlayer.h"

#include "SongPlainTextEdit.h"
#include "TrackPlainTextEdit.h"
#include "InstrumentPlainTextEdit.h"

namespace Ui {
class HardwareChiptunePanelWidget;
}

class HardwareChiptunePanelWidget : public QWidget
{
	Q_OBJECT
public:
	explicit HardwareChiptunePanelWidget(AudioPlayer *p_player, QWidget *parent = nullptr);
	~HardwareChiptunePanelWidget();
private :
	void timerEvent(QTimerEvent *p_event) Q_DECL_OVERRIDE;

private slots:
	void on_OpenFilePushButton_released(void);
	void on_SaveFilePushButton_released(void);
	void on_ImportChunkDataPushButton_released(void);
	void on_ExportChunkDataPushButton_released(void);

	void on_SongPlayPushButton_released(void);
	void on_SongApplyPushButton_released(void);

	void on_TrackIndexSpinBox_valueChanged(int i);
	void on_TrackPlayPushButton_released(void);
	void on_TrackApplyPushButton_released(void);

	void on_InstrumentIndexSpinBox_valueChanged(int i);
	void on_InstrumentApplyPushButton_released(void);

private slots:
	void HandleShortcut_CTRL_L_Activated(void);
	void HandleShortcut_CTRL_P_Activated(void);

private slots:
	void HandleGeneratingSongStateChanged(bool is_generating, int generating_song_index);
	void HandleGeneratingTrackStateChanged(bool is_generating, int generating_track_index, int generating_line_index);
private:
	void UpdateContents(void);
private:
	SongPlainTextEdit *m_p_song_plaintextedit;
	TrackPlainTextEdit *m_p_track_plaintextedit;
	InstrumentPlainTextEdit *m_p_instrument_plaintextedit;
private:
	AudioPlayer *m_p_player;
private:
	Ui::HardwareChiptunePanelWidget *ui;
};

#endif // HARDWARECHIPTUNEPANELWIDGET_H
