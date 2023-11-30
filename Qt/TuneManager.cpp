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
	void GenerateWaveData(void)
	{
		QByteArray generate_bytearray;
		int i;
		for(i = 0; i < m_generate_data_length; i++) {
			uint8_t value = interrupthandler();
			generate_bytearray += value;
		}

		m_wave_bytearray += generate_bytearray;
	}

	void Clean(void)
	{
		m_generate_data_length = 0;

		m_is_playing_song = false;
		m_playing_song_index = -1;

		m_is_playing_track = false;
		m_playing_track_index = -1;
		m_playing_line_index = -1;
		m_wave_bytearray.clear();
	}

	void StopGeneratingWave(void)
	{
		silence();
		Clean();
	}

public:
	QByteArray m_wave_bytearray;
	int m_generate_data_length;

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
	m_p_private->m_inquiring_playing_state_timer.setInterval(25);
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
	delete m_p_private;
	m_p_private = nullptr;
}

void TuneManager::LoadFile(QString filename)
{
	QMutexLocker locker(&m_mutex);

	m_p_private->m_inquiring_playing_state_timer.stop();
	loadfile(filename.toLatin1().data());
	m_p_private->m_inquiring_playing_state_timer.start();
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

void TuneManager::HandleGenerateWaveDataRequested(void)
{
	m_p_private->GenerateWaveData();
}

/**********************************************************************************/

void TuneManager::GenerateWaveData(bool is_synchronized)
{
	QObject::disconnect(this, &TuneManager::GenerateWaveDataRequested,
						this, &TuneManager::HandleGenerateWaveDataRequested);

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

	QObject::connect(this, &TuneManager::GenerateWaveDataRequested,
					 this, &TuneManager::HandleGenerateWaveDataRequested, type);

	emit GenerateWaveDataRequested();
}

/**********************************************************************************/

QByteArray TuneManager::FetchData(int const size)
{
	QMutexLocker locker(&m_mutex);
	if(size > m_p_private->m_generate_data_length){
		m_p_private->m_generate_data_length = size;
	}

	if(m_p_private->m_wave_bytearray.mid(0, size).size() < size){
			GenerateWaveData(true);
	}
	QByteArray fetched_bytearray = m_p_private->m_wave_bytearray.mid(0, size);
	m_p_private->m_wave_bytearray.remove(0, size);

	if(m_p_private->m_wave_bytearray.mid(size, -1).size() < m_p_private->m_generate_data_length){
		GenerateWaveData(false);
	}

	return fetched_bytearray;
}

/**********************************************************************************/

void TuneManager::SetGeneratingWave(int tune_type, int index)
{
	QMutexLocker locker(&m_mutex);

	m_p_private->StopGeneratingWave();
	do
	{
		if(TRACK == tune_type){
			startplaytrack(index);
			break;
		}

		startplaysong(index);
	}while(0);
}

/**********************************************************************************/

void TuneManager::StopGeneratingWave()
{
	QMutexLocker locker(&m_mutex);
	m_p_private->StopGeneratingWave();
}

/**********************************************************************************/
