#ifndef HARDWARECHIPTUNEPANELWIDGET_H
#define HARDWARECHIPTUNEPANELWIDGET_H

#include <QWidget>

#include "AudioPlayer.h"

#include "SongPlainTextEdit.h"
#include "TrackPlainTextEdit.h"

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
	void on_PlaySongPushButton_clicked(bool is_checked);

	void on_TrackIndexSpinBox_valueChanged(int i);
	void on_PlayTrackPushButton_clicked(bool is_checked);
private:
	SongPlainTextEdit *m_p_song_plain_textedit;
	TrackPlainTextEdit *m_p_track_plain_textedit;
private:
	AudioPlayer *m_p_player;
private:
	Ui::HardwareChiptunePanelWidget *ui;
};

#endif // HARDWARECHIPTUNEPANELWIDGET_H
