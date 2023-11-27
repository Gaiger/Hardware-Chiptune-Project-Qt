#ifndef _AUDIOPLAYER_H_
#define _AUDIOPLAYER_H_

#include <QObject>
#include <QAudioOutput>
#include <QThread>

#include <QIODevice>
#include <QMutex>

#include "WaveGenerator.h"


class AudioPlayer : public QObject
{
	Q_OBJECT
public:
	enum SAMPLING_SIZE
	{
		SAMPLING_SIZE_1			= 1,
		SAMPLING_SIZE_2			= 2,

		SAMPLING_SIZE_MAX		= 255,
	}; Q_ENUM(SAMPLING_SIZE)

	enum CHANNEL_COUNTS
	{
		CHANNEL_COUNTS_1		= 1,
		CHANNEL_COUNTS_2		= 2,

		CHANNEL_COUNTS_MAX		= 255,
	}; Q_ENUM(CHANNEL_COUNTS)

	AudioPlayer(WaveGenerator *p_wave_generator, QObject *parent = nullptr);

	~AudioPlayer()  Q_DECL_OVERRIDE;

	void Play(int song = 0);
	void Stop(void);

private slots:
	void HandleAudioNotify(void);
	void HandleAudioStateChanged(QAudio::State state);

private :

	void Initialize(int const sampling_rate = 16000, int const sampling_size = 1, int const channel_counts = 1);
	void AppendAudioData(QByteArray data_bytearray);


	QAudioOutput * m_p_audio_output;
	QIODevice *m_p_audio_io_device;
	QMutex m_accessing_io_device_mutex;
	WaveGenerator *m_p_wave_generator;
	QThread m_wave_generator_working_thread;
};

#endif // _AUDIOPLAYER_H_
