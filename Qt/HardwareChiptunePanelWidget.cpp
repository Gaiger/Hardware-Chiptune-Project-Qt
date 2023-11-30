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
					 this, &HardwareChiptunePanelWidget::HandlePlayingSongStateChanged);

	QObject::connect(p_player->GetTuneManager(), &TuneManager::PlayingTrackStateChanged,
					 this, &HardwareChiptunePanelWidget::HandlePlayingTrackStateChanged);

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

#define UNICODE_PLAY_ICON						u8"\u25B7"
#define UNICODE_STOP_ICON						u8"\u25A1"

void HardwareChiptunePanelWidget::HandlePlayingSongStateChanged(bool is_playing, int playing_song_index)
{
	Q_UNUSED(playing_song_index);

	if(false == is_playing){
		if(ui->PlaySongPushButton->text() == QString(UNICODE_STOP_ICON) ){
			ui->PlaySongPushButton->setText(QString(UNICODE_PLAY_ICON));
		}
	}
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::HandlePlayingTrackStateChanged(bool is_playing, int playing_track_index, int playing_line_index)
{
	Q_UNUSED(playing_track_index);
	Q_UNUSED(playing_line_index);

	if(false == is_playing){
		if(ui->PlayTrackPushButton->text() == QString(UNICODE_STOP_ICON) ){
			ui->PlayTrackPushButton->setText(QString(UNICODE_PLAY_ICON));
		}
	}
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::on_PlaySongPushButton_released(void)
{
	do
	{

		if( ui->PlaySongPushButton->text() == QString(UNICODE_PLAY_ICON)){
			m_p_player->PlaySong( ui->SongIndexSpinBox->value());
			ui->PlaySongPushButton->setText(QString(UNICODE_STOP_ICON));
			break;
		}

		m_p_player->Stop();
		ui->PlaySongPushButton->setText(QString(UNICODE_PLAY_ICON));
	}while(0);
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::on_TrackIndexSpinBox_valueChanged(int i)
{
	m_p_track_plain_textedit->UpdateShowedTrack(i);
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::on_PlayTrackPushButton_released(void)
{
	do
	{
		if( ui->PlayTrackPushButton->text() == QString(UNICODE_PLAY_ICON)){
			int playing_track_index;
			playing_track_index = ui->TrackIndexSpinBox->value();
			m_p_player->PlayTrack(ui->TrackIndexSpinBox->value());
			ui->PlayingTrackIndexLabel->setText(QString::asprintf("%02x", playing_track_index));
			ui->PlayTrackPushButton->setText(QString(UNICODE_STOP_ICON));
			break;
		}
		m_p_player->Stop();
		ui->PlayingTrackIndexLabel->setText(QString(""));
		ui->PlayTrackPushButton->setText(QString(UNICODE_PLAY_ICON));
	}while(0);
}


