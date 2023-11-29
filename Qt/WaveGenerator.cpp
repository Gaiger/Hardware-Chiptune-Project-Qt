#include <QThread>

#include <QDebug>

#include "song_manager.h"
#include "WaveGenerator.h"

class WaveGeneratorPrivate
{
public:
	void GenerateWaveData(void)
	{
		QByteArray generate_bytearray;
		int i;
		for(i = 0; i < m_generate_data_length; i++) {
			uint8_t value = interrupthandler();
			generate_bytearray += value;
			//qDebug() << "value" << value;
		}
		//qDebug() << "generate_bytearray.size() = " << generate_bytearray.size();
		m_wave_bytearray += generate_bytearray;
	}

	void LoadFile(QString filename)
	{
		loadfile(filename.toLatin1().data());
	}

	void SetStartPlaySong(int song_index)
	{
		startplaysong(song_index);
	}

	void SetPlayTrack(int track_index)
	{
		startplaytrack(track_index);
	}

public:
	QByteArray m_wave_bytearray;

	int m_generate_data_length;
};

/**********************************************************************************/

WaveGenerator::WaveGenerator(QObject *parent)
	: QObject(parent),
	m_p_private(nullptr)
{
	m_p_private = new WaveGeneratorPrivate();
	m_p_private->m_generate_data_length = 0;
}

/**********************************************************************************/

WaveGenerator::WaveGenerator(QString filename, QObject *parent)
	: QObject(parent),
	m_p_private(nullptr)
{
	new (this)WaveGenerator(parent);
	m_p_private->LoadFile(filename);
	m_p_private->SetStartPlaySong(0);

}

/**********************************************************************************/

WaveGenerator::~WaveGenerator(void)
{
	QMutexLocker locker(&m_mutex);
	delete m_p_private;
	m_p_private = nullptr;
}

/**********************************************************************************/

void WaveGenerator::HandleGenerateWaveDataRequested(void)
{
	m_p_private->GenerateWaveData();
}

/**********************************************************************************/

void WaveGenerator::GenerateWaveData(bool is_synchronized)
{
	QObject::disconnect(this, &WaveGenerator::GenerateWaveDataRequested,
						this, &WaveGenerator::HandleGenerateWaveDataRequested);

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

	QObject::connect(this, &WaveGenerator::GenerateWaveDataRequested,
					 this, &WaveGenerator::HandleGenerateWaveDataRequested, type);

	emit GenerateWaveDataRequested();
}

/**********************************************************************************/

QByteArray WaveGenerator::FetchData(int const size)
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

void WaveGenerator::SetStartPlaySong(int start_song_index)
{
	QMutexLocker locker(&m_mutex);
	m_p_private->SetStartPlaySong(start_song_index);
}

/**********************************************************************************/

void WaveGenerator::SetPlayTrack(int track_index)
{
	QMutexLocker locker(&m_mutex);
	m_p_private->SetPlayTrack(track_index);
}
