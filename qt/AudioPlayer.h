#ifndef _AUDIOPLAYER_H_
#define _AUDIOPLAYER_H_

#include <QObject>
#include <QAudioOutput>
#include <QThread>

#include <QIODevice>
#include <QMutex>

#include "TuneManager.h"


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

	AudioPlayer(QObject *parent = nullptr);
	~AudioPlayer()  Q_DECL_OVERRIDE;

	void PlaySong(int start_song_index = 1);
	void PlayTrack(int track_index = 1);
	void Stop(void);

	TuneManager * const GetTuneManager(void);
private slots:
	void HandleAudioNotify(void);
	void HandleAudioStateChanged(QAudio::State state);

private :
	void Play(int fill_audio_buffer_interval,
			  int const sampling_rate = 16000, int const sampling_size = 1, int const channel_counts = 1);
	void AppendWave(QByteArray wave_bytearray);
	void Clean();
private:
	QAudioOutput * m_p_audio_output;
	QIODevice *m_p_audio_io_device;
	QMutex m_accessing_io_device_mutex;
	TuneManager *m_p_tune_manager;
	QThread m_tune_manager_working_thread;
};

#endif // _AUDIOPLAYER_H_
