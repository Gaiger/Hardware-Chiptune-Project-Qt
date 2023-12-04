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
	QFont font20("Monospace");
	font20.setStyleHint(QFont::TypeWriter);
	font20.setPixelSize(20);

	QFont font18_bold(font20);
	font18_bold.setPixelSize(18);
	font18_bold.setBold(true);

	do{
		m_p_song_plaintextedit = new SongPlainTextEdit(m_p_player->GetTuneManager(), this);
		ReplaceWidget(m_p_song_plaintextedit, ui->SongWidget);

		TuneManager::songline *p_songs;
				int number_of_songs;
				m_p_player->GetTuneManager()->GetSongs(&p_songs, &number_of_songs);
				ui->SongIndexSpinBox->setRange(0, number_of_songs - 1);
		ui->SongIndexSpinBox->setFont(font20);
		ui->SongApplyPushButton->setFont(font18_bold);

		QObject::connect(m_p_song_plaintextedit, &QPlainTextEdit::modificationChanged, this,
						 [&](bool is_changed){
			ui->SongApplyPushButton->setEnabled(is_changed);
			ui->SongPlayPushButton->setEnabled(!is_changed);
		});
	}while(0);

	do{
		m_p_track_plaintextedit = new TrackPlainTextEdit(m_p_player->GetTuneManager(), this);
		ReplaceWidget(m_p_track_plaintextedit, ui->TrackWidget);

		TuneManager::track *p_track;
		int number_of_tracks;
		int track_length;
		m_p_player->GetTuneManager()->GetTracks(&p_track, &number_of_tracks, &track_length);
		ui->TrackIndexSpinBox->setRange(0, number_of_tracks - 1);
		ui->TrackIndexSpinBox->setFont(font20);
		ui->TrackApplyPushButton->setFont(font18_bold);

		QObject::connect(m_p_track_plaintextedit, &QPlainTextEdit::modificationChanged, this,
						 [&](bool is_changed){
			ui->TrackApplyPushButton->setEnabled(is_changed);
			ui->TrackPlayPushButton->setEnabled(!is_changed);
		});
	}while(0);

	do{

		m_p_instrument_plaintextedit = new InstrumentPlainTextEdit(m_p_player->GetTuneManager(), this);
		ReplaceWidget(m_p_instrument_plaintextedit, ui->InstrumentWidget);

		TuneManager::instrument *p_instruments;
		int number_of_instruments;
		m_p_player->GetTuneManager()->GetInstruments(&p_instruments, &number_of_instruments);

		ui->InstrumentIndexSpinBox->setRange(0, number_of_instruments - 1);
		ui->InstrumentIndexSpinBox->setFont(font20);

		ui->InstrumentApplyPushButton->setFont(font18_bold);

		QObject::connect(m_p_instrument_plaintextedit, &QPlainTextEdit::modificationChanged, this,
						 [&](bool is_changed){
			ui->InstrumentApplyPushButton->setEnabled(is_changed);
		});
	}while(0);

	//QObject::startTimer(50);
	m_p_song_plaintextedit->ShowSongs();
	m_p_track_plaintextedit->ShowTrack(1);
	m_p_instrument_plaintextedit->ShowInstrument(1);

	QObject::connect(p_player->GetTuneManager(), &TuneManager::GeneratingSongStateChanged,
					 this, &HardwareChiptunePanelWidget::HandleGeneratingSongStateChanged);

	QObject::connect(p_player->GetTuneManager(), &TuneManager::GeneratingTrackStateChanged,
					 this, &HardwareChiptunePanelWidget::HandleGeneratingTrackStateChanged);
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
	QWidget::timerEvent(p_event);
}

/**********************************************************************************/

#define UNICODE_PLAY_ICON						u8"\u25B7"
#define UNICODE_STOP_ICON						u8"\u25A1"

void HardwareChiptunePanelWidget::HandleGeneratingSongStateChanged(bool is_generating, int generating_song_index)
{
	do
	{
		if(true == is_generating){
			ui->SongPlayPushButton->setText(QString(UNICODE_STOP_ICON));
			ui->SongIndexSpinBox->setEnabled(false);
			ui->SongIndexSpinBox->setValue(generating_song_index);
			break;
		}

		ui->SongPlayPushButton->setText(QString(UNICODE_PLAY_ICON));
		ui->SongIndexSpinBox->setEnabled(true);
	}while(0);
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::HandleGeneratingTrackStateChanged(bool is_generating, int generating_track_index, int generating_line_index)
{
	Q_UNUSED(generating_track_index);
	Q_UNUSED(generating_line_index);

	do
	{
		if(true == is_generating){
			ui->TrackPlayPushButton->setText(QString(UNICODE_STOP_ICON));
			break;
		}
		ui->TrackPlayPushButton->setText(QString(UNICODE_PLAY_ICON));
	}while(0);

}

/**********************************************************************************/

void HardwareChiptunePanelWidget::on_SongPlayPushButton_released(void)
{
	do
	{
		if( ui->SongPlayPushButton->text() == QString(UNICODE_PLAY_ICON)){
			m_p_player->PlaySong( ui->SongIndexSpinBox->value());
			ui->SongPlayPushButton->setText(QString(UNICODE_STOP_ICON));
			ui->SongIndexSpinBox->setEnabled(false);
			break;
		}

		m_p_player->Stop();
		ui->SongPlayPushButton->setText(QString(UNICODE_PLAY_ICON));
		ui->SongIndexSpinBox->setEnabled(true);
	}while(0);
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::on_TrackIndexSpinBox_valueChanged(int i)
{
	m_p_player->Stop();
	m_p_track_plaintextedit->ShowTrack(i);
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::on_TrackPlayPushButton_released(void)
{
	do
	{
		if( ui->TrackPlayPushButton->text() == QString(UNICODE_PLAY_ICON)){
			int generating_track_index;
			generating_track_index = ui->TrackIndexSpinBox->value();
			m_p_player->PlayTrack(ui->TrackIndexSpinBox->value());
			ui->TrackPlayPushButton->setText(QString(UNICODE_STOP_ICON));
			break;
		}
		m_p_player->Stop();
		ui->TrackPlayPushButton->setText(QString(UNICODE_PLAY_ICON));
	}while(0);
}

/**********************************************************************************/

void HardwareChiptunePanelWidget::on_InstrumentIndexSpinBox_valueChanged(int i)
{
	m_p_instrument_plaintextedit->ShowInstrument(i);
}
