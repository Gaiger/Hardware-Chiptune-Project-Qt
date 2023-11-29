#ifndef HARDWARECHIPTUNEPANELWIDGET_H
#define HARDWARECHIPTUNEPANELWIDGET_H

#include <QWidget>

#include "SongPlainTextEdit.h"
#include "TrackPlainTextEdit.h"

namespace Ui {
class HardwareChiptunePanelWidget;
}

class HardwareChiptunePanelWidget : public QWidget
{
	Q_OBJECT

public:
	explicit HardwareChiptunePanelWidget(QWidget *parent = nullptr);
	~HardwareChiptunePanelWidget();
private :
	void timerEvent(QTimerEvent *p_event) Q_DECL_OVERRIDE;

private slots:
	void on_TrackIndexSpinBox_valueChanged(int i);
private:
	SongPlainTextEdit *m_p_song_plaintextedit;
	TrackPlainTextEdit *m_p_track_plaintextedit;
private:
	Ui::HardwareChiptunePanelWidget *ui;
};

#endif // HARDWARECHIPTUNEPANELWIDGET_H
