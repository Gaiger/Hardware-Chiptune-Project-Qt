#include "HardwareChiptunePanelWidget.h"
#include "ui_HardwareChiptunePanelWidget.h"

#include <QGridLayout>

HardwareChiptunePanelWidget::HardwareChiptunePanelWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HardwareChiptunePanelWidget)
{
	ui->setupUi(this);

	QFont font("Courier");
	font.setStyleHint(QFont::TypeWriter);
	QWidget::setFont(font);

	{
		m_p_song_plain_text_edit = new SongPlainTextEdit(this);
		QGridLayout *p_layout = new QGridLayout(ui->SongWidget);
		p_layout->addWidget(m_p_song_plain_text_edit, 0, 0);
		p_layout->setContentsMargins(0, 0, 0, 0);
		p_layout->setSpacing(0);
	}

	QObject::startTimer(100);
	m_p_song_plain_text_edit->UpdateSongScores();
}

/**********************************************************************************/

HardwareChiptunePanelWidget::~HardwareChiptunePanelWidget()
{
	delete m_p_song_plain_text_edit;
	delete ui;
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::timerEvent(QTimerEvent *p_event)
{
	m_p_song_plain_text_edit->UpdateSongPlaying();
	QWidget::timerEvent(p_event);
}
