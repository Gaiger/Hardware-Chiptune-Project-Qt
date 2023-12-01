#include <QThread>
#include <QTimer>

#include <QDebug>

#include "song_manager.h"
#include "TuneManager.h"

class WaveGenerator
{
public:

};

class TuneManagerPrivate
{
public:
	void GenerateWave(int const length)
	{
		QByteArray generated_bytearray;
		generated_bytearray.reserve(length);
		for(int i = 0; i < length; i++) {
			uint8_t value = interrupthandler();
			generated_bytearray += value;
		}

		m_wave_bytearray += generated_bytearray;
	}

	void Clean(void)
	{
		m_wave_prebuffer_length = 0;

		m_is_playing_song = false;
		m_playing_song_index = -1;

		m_is_playing_track = false;
		m_playing_track_index = -1;
		m_playing_line_index = -1;
		m_wave_bytearray.clear();
	}

	void StopGeneratingWave(void)
	{
		m_inquiring_playing_state_timer.stop();
		silence();
		Clean();
	}

	void StartGeneratingWave(int tune_type, int index)
	{
		StopGeneratingWave();
		do
		{
			if(TuneManager::TRACK == tune_type){
				startplaytrack(index);
				break;
			}

			startplaysong(index);
		}while(0);

		m_inquiring_playing_state_timer.setInterval(25);
		m_inquiring_playing_state_timer.start();
	}

public:
	QByteArray m_wave_bytearray;
	int m_wave_prebuffer_length;

	QTimer m_inquiring_playing_state_timer;

	bool m_is_playing_song;
	int m_playing_song_index;

	bool m_is_playing_track;
	int m_playing_track_index;
	int m_playing_line_index;

};

/**********************************************************************************/

TuneManager::TuneManager(QObject *parent)
	: QObject(parent),
	m_p_private(nullptr)
{
	m_p_private = new TuneManagerPrivate();
	m_p_private->Clean();
	QObject::connect(&m_p_private->m_inquiring_playing_state_timer, &QTimer::timeout,
					this, &TuneManager::InquirePlayingState);
}

/**********************************************************************************/

TuneManager::TuneManager(QString filename, QObject *parent)
	: QObject(parent),
	m_p_private(nullptr)
{
	new (this)TuneManager(parent);
	TuneManager::LoadFile(filename);
}

/**********************************************************************************/

TuneManager::~TuneManager(void)
{
	StopGeneratingWave();
	delete m_p_private;
	m_p_private = nullptr;
}

/**********************************************************************************/

void TuneManager::LoadFile(QString filename)
{
	QMutexLocker locker(&m_mutex);
	loadfile(filename.toLatin1().data());
}

/**********************************************************************************/

void TuneManager::InquirePlayingState(void)
{
	do
	{
		int playing_song_index;
		bool is_playing_song = is_song_playing(&playing_song_index);
		playing_song_index -= 1;
		if(m_p_private->m_playing_song_index == playing_song_index){
			break;
		}

		if(false == is_playing_song){
			playing_song_index = -1;
		}

		emit PlayingSongStateChanged(is_playing_song, playing_song_index);
		m_p_private->m_is_playing_song = is_playing_song;
		m_p_private->m_playing_song_index = playing_song_index;
	}while(0);

	do
	{
		int playing_track_index, playing_line_index;

		bool is_playing_track = is_track_playing(&playing_track_index, &playing_line_index);
		if(m_p_private->m_playing_track_index == playing_track_index){
			if(m_p_private->m_playing_line_index == playing_line_index){
				break;
			}
		}

		if(false == is_playing_track){
			playing_track_index = -1;
			playing_line_index = -1;
		}
		//qDebug() << playing_line_index;
		emit PlayingTrackStateChanged(is_playing_track, playing_track_index, playing_line_index);
		m_p_private->m_is_playing_track = is_playing_track;
		m_p_private->m_playing_track_index = playing_track_index;
		m_p_private->m_playing_line_index = playing_line_index;
	}while(0);

}

/**********************************************************************************/

void TuneManager::HandleGenerateWaveRequested(int length)
{
	m_p_private->GenerateWave(length);
}

/**********************************************************************************/

void TuneManager::GenerateWave(int length, bool is_synchronized)
{
	QObject::disconnect(this, &TuneManager::GenerateWaveRequested,
						this, &TuneManager::HandleGenerateWaveRequested);

	Qt::ConnectionType type = Qt::DirectConnection;
	do
	{

		if( QObject::thread() == QThread::currentThread()){
			break;
		}

		//qDebug() << Q_FUNC_INFO << "is_synchronized = " << is_synchronized;
		if(false == is_synchronized){
			type = Qt::QueuedConnection;
			break;
		}
		type = Qt::BlockingQueuedConnection;
	}while(0);

	QObject::connect(this, &TuneManager::GenerateWaveRequested,
					 this, &TuneManager::HandleGenerateWaveRequested, type);
	emit GenerateWaveRequested(length);
}

/**********************************************************************************/

QByteArray TuneManager::FetchWave(int const length)
{
	QMutexLocker locker(&m_mutex);
	if(length > m_p_private->m_wave_prebuffer_length){
		m_p_private->m_wave_prebuffer_length = length;
	}

	if(m_p_private->m_wave_bytearray.mid(0, length).size() < length){
			GenerateWave(length - m_p_private->m_wave_bytearray.mid(0, length).size(),
							 true);
	}

	QByteArray fetched_wave_bytearray = m_p_private->m_wave_bytearray.mid(0, length);
	m_p_private->m_wave_bytearray.remove(0, length);

	if(m_p_private->m_wave_bytearray.mid(length, -1).size() < m_p_private->m_wave_prebuffer_length){
		GenerateWave(m_p_private->m_wave_prebuffer_length, false);
	}

	return fetched_wave_bytearray;
}

/**********************************************************************************/

void TuneManager::SetGeneratingWave(int tune_type, int index)
{
	QMutexLocker locker(&m_mutex);
	m_p_private->StartGeneratingWave(tune_type, index);
}

/**********************************************************************************/

void TuneManager::StopGeneratingWave()
{
	QMutexLocker locker(&m_mutex);
	m_p_private->StopGeneratingWave();
}

/**********************************************************************************/
