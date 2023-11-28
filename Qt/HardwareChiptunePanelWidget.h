#ifndef HARDWARECHIPTUNEPANELWIDGET_H
#define HARDWARECHIPTUNEPANELWIDGET_H

#include <QWidget>

#include "SongPlainTextEdit.h"

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

private:
	SongPlainTextEdit *m_p_song_plain_text_edit;
private:
	Ui::HardwareChiptunePanelWidget *ui;
};

#endif // HARDWARECHIPTUNEPANELWIDGET_H
