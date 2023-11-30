#ifndef TUNEMANAGER_H
#define TUNEMANAGER_H
#include <QObject>
#include <QByteArray>

#include <QMutex>


class TuneManagerPrivate;

class TuneManager : public QObject
{
	Q_OBJECT
public:
	explicit TuneManager(QObject *parent = nullptr);
	explicit TuneManager(QString filename, QObject *parent = nullptr);
	~TuneManager() Q_DECL_OVERRIDE;

	void LoadFile(QString filename);
	void SetStartPlaySong(int start_song_index);
	void SetPlayTrack(int track_index);
	void Stop();
public:
	QByteArray FetchData(int const size);

public:
	signals:
	void PlayingSongStateChanged(bool is_playing, int playing_song_index);
	void PlayingTrackStateChanged(bool is_playing, int playing_track_index, int playing_line_index);

private :
	void timerEvent(QTimerEvent *p_event) Q_DECL_OVERRIDE;
private:
	signals:
	void GenerateWaveDataRequested(void);
private slots:
	void HandleGenerateWaveDataRequested(void);
private:
	void GenerateWaveData(bool is_synchronized = true);

private :
	TuneManagerPrivate *m_p_private;
	QMutex m_mutex;
};

#endif // TUNEMANAGER_H
