#include <QDebug>
#include <QEventLoop>
#include <QBuffer>

#ifndef _STUFF_H_
#define _STUFF_H_
extern "C"{
#include "../stuff.h"
}
#endif

#include "AudioPlayer.h"

class AudioIODevice: public QIODevice
{
public :
	AudioIODevice(void){ }

protected :
	qint64 readData(char *data, qint64 maxlen) Q_DECL_OVERRIDE
	{
		int read_size = maxlen;
		if(m_audio_data_bytearray.size() < read_size)
			read_size = m_audio_data_bytearray.size();

		memcpy(data, m_audio_data_bytearray.data(), read_size);
		m_audio_data_bytearray.remove(0, read_size);
		return read_size;
	}

	qint64 writeData(const char *data, qint64 len) Q_DECL_OVERRIDE
	{
		m_audio_data_bytearray.append(QByteArray(data, len));
		return len;
	}

private:
		QByteArray m_audio_data_bytearray;
};

/**********************************************************************************/

AudioPlayer::AudioPlayer(WaveGenerator *p_wave_generator, QObject *parent)
	: QObject(parent),
	m_p_audio_output(nullptr),
	m_p_audio_io_device(nullptr)
{
	m_p_wave_generator = p_wave_generator;

	m_p_wave_generator->moveToThread(&m_wave_generator_working_thread);
	m_wave_generator_working_thread.start(QThread::HighPriority);
}

/**********************************************************************************/

AudioPlayer::~AudioPlayer(void)
{
	AudioPlayer::Stop();
	AudioPlayer::Clean();
	m_wave_generator_working_thread.quit();
	do
	{
		if(false == m_wave_generator_working_thread.isRunning()){
			break;
		}
	}while(1);
	m_p_wave_generator = nullptr;
}

/**********************************************************************************/

void AudioPlayer::Clean(void)
{
	if(nullptr != m_p_audio_output){
		delete m_p_audio_output;
	}
	m_p_audio_output = nullptr;

	if(nullptr != m_p_audio_io_device){
		delete m_p_audio_io_device;
	}
	m_p_audio_io_device = nullptr;
}

/**********************************************************************************/

void AudioPlayer::PlaySong(int start_song_index)
{
	m_p_wave_generator->SetStartPlaySong(start_song_index);
	AudioPlayer::Play();
}

void AudioPlayer::PlayTrack(int track_index)
{
	m_p_wave_generator->SetPlayTrack(track_index);
	AudioPlayer::Play();
}
/**********************************************************************************/

void AudioPlayer::Play(int const sampling_rate, int const sampling_size, int const channel_counts)
{
	QAudioFormat format;
	format.setSampleRate(sampling_rate);
	format.setChannelCount((int)channel_counts);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);

	switch(sampling_size)
	{
	case SAMPLING_SIZE_1:
		format.setSampleSize(8);
		format.setSampleType(QAudioFormat::UnSignedInt);
		break;

	case SAMPLING_SIZE_2:
	default:
		format.setSampleSize(16);
		format.setSampleType(QAudioFormat::SignedInt);
		break;
	}

	QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
	qDebug() << info.supportedSampleRates();
	if (!info.isFormatSupported(format)) {
		qWarning()<<"raw audio format not supported by backend, cannot play audio.";
	}
	qDebug() << info.deviceName();
	qDebug() << format;

	m_p_audio_output = new QAudioOutput(info, format);
	QObject::connect(m_p_audio_output, &QAudioOutput::notify, this, &AudioPlayer::HandleAudioNotify);
	QObject::connect(m_p_audio_output, &QAudioOutput::stateChanged, this, &AudioPlayer::HandleAudioStateChanged);
	m_p_audio_output->setVolume(0.40);

	int notify_interval = 100;
	int audio_buffer_size = 2 * notify_interval * format.sampleRate() * format.channelCount() * format.sampleSize()/8/1000;
	m_p_audio_output->setNotifyInterval(notify_interval);

	m_p_audio_output->setBufferSize(audio_buffer_size);
	qDebug() <<" m_p_audio_output->bufferSize() = " << m_p_audio_output->bufferSize();

	m_p_audio_io_device = new AudioIODevice();
	m_p_audio_io_device->open(QIODevice::ReadWrite);
	AudioPlayer::AppendAudioData(m_p_wave_generator->FetchData(audio_buffer_size));
	m_p_audio_output->start(m_p_audio_io_device);
}

/**********************************************************************************/

void AudioPlayer::AppendAudioData(QByteArray data_bytearray)
{
	QMutexLocker lock(&m_accessing_io_device_mutex);
	if(nullptr == m_p_audio_io_device){
		return ;
	}
	m_p_audio_io_device->write(data_bytearray);
}

/**********************************************************************************/

void AudioPlayer::Stop(void)
{
	QMutexLocker lock(&m_accessing_io_device_mutex);
	if(nullptr != m_p_audio_output){
		m_p_audio_output->stop();
	}
	AudioPlayer::Clean();
}

/**********************************************************************************/

void AudioPlayer::HandleAudioNotify(void)
{

	//qDebug() << Q_FUNC_INFO  << "elapsed " <<
	//			m_p_audio_output->elapsedUSecs()/1000.0/1000.0
	//		 << "seconds";

	int remain_audio_buffer_size = m_p_audio_output->bytesFree();
	if(0 == m_p_audio_output->bytesFree()){
		return ;
	}

	QByteArray append_bytearray
			= m_p_wave_generator->FetchData(remain_audio_buffer_size);
	AudioPlayer::AppendAudioData(append_bytearray);
}

/**********************************************************************************/

void AudioPlayer::HandleAudioStateChanged(QAudio::State state)
{
	qDebug() << Q_FUNC_INFO << state;
	switch (state)
	{
	case QAudio::ActiveState:
		break;
	case QAudio::SuspendedState:
		break;
	case QAudio::IdleState :
			// Finished playing (no more data)
			//m_p_audio_output->stop();
		break;

	case QAudio::StoppedState:
		// Stopped for other reasons
		if (m_p_audio_output->error() != QAudio::NoError) {
			// Error handling
		}
		break;

	case QAudio::InterruptedState:
		break;
	default:
		// ... other cases as appropriate
		break;
	}
}

/**********************************************************************************/
