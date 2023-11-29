#include "HardwareChiptunePanelWidget.h"
#include "ui_HardwareChiptunePanelWidget.h"

#include <QGridLayout>

void ReplaceWidget(QWidget *p_widget, QWidget *p_replaced_widget)
{
	QGridLayout *p_layout = new QGridLayout(p_replaced_widget);
	p_layout->addWidget(p_widget, 0, 0);
	p_layout->setContentsMargins(0, 0, 0, 0);
	p_layout->setSpacing(0);
}

/**********************************************************************************/

HardwareChiptunePanelWidget::HardwareChiptunePanelWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HardwareChiptunePanelWidget)
{
	ui->setupUi(this);

	do{
		m_p_song_plaintextedit = new SongPlainTextEdit(this);
		ReplaceWidget(m_p_song_plaintextedit, ui->SongWidget);
	}while(0);

	do{
		m_p_track_plaintextedit = new TrackPlainTextEdit(this);
		ReplaceWidget(m_p_track_plaintextedit, ui->TrackWidget);
	}while(0);

	QObject::startTimer(50);
	m_p_song_plaintextedit->UpdateSongs();
	m_p_track_plaintextedit->UpdateTrack();
}

/**********************************************************************************/

HardwareChiptunePanelWidget::~HardwareChiptunePanelWidget()
{
	delete m_p_song_plaintextedit;
	delete ui;
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::timerEvent(QTimerEvent *p_event)
{
	m_p_song_plaintextedit->UpdateSongPlaying();
	m_p_track_plaintextedit->UpdateTrackPlaying();
	QWidget::timerEvent(p_event);
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::on_TrackIndexSpinBox_valueChanged(int i)
{
	m_p_track_plaintextedit->UpdateShowedTrack(i);
}
