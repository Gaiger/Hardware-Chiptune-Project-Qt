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

	enum TUNE_TYPE
	{
		SONG,
		TRACK,
	};Q_ENUM(TUNE_TYPE)
	void SetGeneratingWave(int tune_type, int index);
	void StopGeneratingWave();
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
