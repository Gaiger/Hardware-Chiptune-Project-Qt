#include <QGridLayout>
#include "ui_HardwareChiptunePanelWidget.h"
#include "HardwareChiptunePanelWidget.h"


void ReplaceWidget(QWidget *p_widget, QWidget *p_replaced_widget)
{
	QGridLayout *p_layout = new QGridLayout(p_replaced_widget);
	p_layout->addWidget(p_widget, 0, 0);
	p_layout->setContentsMargins(0, 0, 0, 0);
	p_layout->setSpacing(0);
}

/**********************************************************************************/

HardwareChiptunePanelWidget::HardwareChiptunePanelWidget(AudioPlayer *p_player, QWidget *parent) :
	QWidget(parent),
	m_p_player(p_player),

	ui(new Ui::HardwareChiptunePanelWidget)
{
	ui->setupUi(this);
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPixelSize(20);
	ui->TrackIndexSpinBox->setFont(font);
	ui->SongIndexSpinBox->setFont(font);

	do{
		m_p_song_plain_textedit = new SongPlainTextEdit(this);
		ReplaceWidget(m_p_song_plain_textedit, ui->SongWidget);
	}while(0);

	do{
		m_p_track_plain_textedit = new TrackPlainTextEdit(this);
		ReplaceWidget(m_p_track_plain_textedit, ui->TrackWidget);
	}while(0);

	//QObject::startTimer(50);
	m_p_song_plain_textedit->UpdateSongs();
	m_p_track_plain_textedit->UpdateTrack();


	QObject::connect(p_player->GetTuneManager(), &TuneManager::PlayingSongStateChanged,
					 m_p_song_plain_textedit, &SongPlainTextEdit::HandlePlayingSongStateChanged);

	QObject::connect(p_player->GetTuneManager(), &TuneManager::PlayingTrackStateChanged,
					 m_p_track_plain_textedit, &TrackPlainTextEdit::HandlePlayingTrackStateChanged);
}

/**********************************************************************************/

HardwareChiptunePanelWidget::~HardwareChiptunePanelWidget()
{
	delete m_p_song_plain_textedit;
	delete ui;
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::timerEvent(QTimerEvent *p_event)
{
	QWidget::timerEvent(p_event);
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::on_PlaySongPushButton_clicked(bool is_checked)
{
	do
	{
		if(true == is_checked){
			m_p_player->PlaySong( ui->SongIndexSpinBox->value());
			break;
		}
		m_p_player->Stop();
	}while(0);
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::on_TrackIndexSpinBox_valueChanged(int i)
{
	m_p_track_plain_textedit->UpdateShowedTrack(i);
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::on_PlayTrackPushButton_clicked(bool is_checked)
{
	do
	{
		if(true == is_checked){
			int playing_track_index;
			playing_track_index = ui->TrackIndexSpinBox->value();
			m_p_player->PlayTrack(ui->TrackIndexSpinBox->value());
			ui->PlayingTrackIndexLabel->setText(QString::asprintf("%02x", playing_track_index));
			break;
		}
		m_p_player->Stop();
		ui->PlayingTrackIndexLabel->setText(QString(""));
	}while(0);
}
