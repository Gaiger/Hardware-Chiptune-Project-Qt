#include <QThread>
#include <QTimer>
#include <QBuffer>
#include <QDebug>

#include "song_manager.h"
#include "TuneManager.h"


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

	void ResetGeneratingWave(void)
	{
		m_inquiring_playing_state_timer.stop();
		silence();
		m_wave_bytearray.clear();
		m_wave_prebuffer_length = 0;

		m_is_generating_song = false;
		m_generating_song_index = -1;

		m_is_generating_track = false;
		m_generating_track_index = -1;
		m_generating_line_index = -1;
	}

	void SetGeneratingWave(int tune_type, int index)
	{
		ResetGeneratingWave();
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

	void CleanAll(void)
	{
		ResetGeneratingWave();
		m_p_songlines = nullptr;
		m_number_of_songlines = 0;

		m_p_tracks = nullptr;
		m_number_of_tracks = 0;
		m_track_length = 0;
	}

public:
	TuneManager::songline *m_p_songlines;
	int m_number_of_songlines;

	TuneManager::track *m_p_tracks;
	int m_number_of_tracks;
	int m_track_length;

	TuneManager::instrument *m_p_instruments;
	int m_number_of_instruments;

	QByteArray m_wave_bytearray;
	int m_wave_prebuffer_length;

	QTimer m_inquiring_playing_state_timer;

	bool m_is_generating_song;
	int m_generating_song_index;

	bool m_is_generating_track;
	int m_generating_track_index;
	int m_generating_line_index;

};

/**********************************************************************************/

TuneManager::TuneManager(QObject *parent)
	: QObject(parent),
	m_p_private(nullptr)
{
	if( QMetaType::UnknownType == QMetaType::type("TUNE_TYPE")){
			qRegisterMetaType<TuneManager::TUNE_TYPE>("TUNE_TYPE");
	}

	m_p_private = new TuneManagerPrivate();
	m_p_private->CleanAll();
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
	ResetGeneratingWave();
	m_p_private->CleanAll();
	delete m_p_private;
	m_p_private = nullptr;
}

/**********************************************************************************/


void TuneManager::LoadFile(QString filename)
{
	QMutexLocker locker(&m_mutex);
	loadfile(filename.toLatin1().data());

	get_songlines((void**)&m_p_private->m_p_songlines, &m_p_private->m_number_of_songlines);
	get_tracks((void**)&m_p_private->m_p_tracks, &m_p_private->m_number_of_tracks, &m_p_private->m_track_length);
	get_instruments((void**)&m_p_private->m_p_instruments, &m_p_private->m_number_of_instruments);
	return ;
}

/**********************************************************************************/

void TuneManager::GetSongs(TuneManager::songline ** pp_songlines, int * p_number_of_songlines)
{
	*pp_songlines = m_p_private->m_p_songlines;
	*p_number_of_songlines = m_p_private->m_number_of_songlines;
}

/**********************************************************************************/

void TuneManager::GetTracks(TuneManager::track ** pp_tracks, int * p_track_number, int * p_track_length)
{
	*pp_tracks = m_p_private->m_p_tracks;
	*p_track_number = m_p_private->m_number_of_tracks;
	*p_track_length = m_p_private->m_track_length;
}

/**********************************************************************************/

void TuneManager::GetInstruments(TuneManager::instrument ** pp_instruments, int * p_number_of_instruments)
{
	*pp_instruments = m_p_private->m_p_instruments;
	*p_number_of_instruments = m_p_private->m_number_of_instruments;
}

/**********************************************************************************/

void TuneManager::InquirePlayingState(void)
{
	do
	{
		int generating_song_index;
		bool is_generating_song = is_song_playing(&generating_song_index);
		generating_song_index -= 1;
		if(m_p_private->m_generating_song_index == generating_song_index){
			break;
		}

		if(false == is_generating_song){
			generating_song_index = -1;
		}

		emit GeneratingSongStateChanged(is_generating_song, generating_song_index);
		m_p_private->m_is_generating_song = is_generating_song;
		m_p_private->m_generating_song_index = generating_song_index;
	}while(0);

	do
	{
		int generating_track_index, generating_line_index;

		bool is_generating_track = is_track_playing(&generating_track_index, &generating_line_index);
		if(m_p_private->m_generating_track_index == generating_track_index){
			if(m_p_private->m_generating_line_index == generating_line_index){
				break;
			}
		}

		if(false == is_generating_track){
			generating_track_index = -1;
			generating_line_index = -1;
		}
		//qDebug() << generating_line_index;
		emit GeneratingTrackStateChanged(is_generating_track, generating_track_index, generating_line_index);
		m_p_private->m_is_generating_track = is_generating_track;
		m_p_private->m_generating_track_index = generating_track_index;
		m_p_private->m_generating_line_index = generating_line_index;
	}while(0);

#if(0)
	if(false == m_p_private->m_is_generating_song &&
			false == m_p_private->m_is_generating_track)
	{
		emit GeneratingWaveStopped();
	}
#endif
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
	m_p_private->SetGeneratingWave(tune_type, index);
}

/**********************************************************************************/

void TuneManager::ResetGeneratingWave()
{
	QMutexLocker locker(&m_mutex);
	m_p_private->ResetGeneratingWave();
}

/**********************************************************************************/
