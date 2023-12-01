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

	struct songline {
		uint8_t			track[4];
		uint8_t			transp[4];
	};

	void GetSongLines(TuneManager::songline **pp_songlines, int * p_number_of_songlines);

	struct trackline {
		uint8_t	note;
		uint8_t	instr;
		uint8_t	cmd[2];
		uint8_t	param[2];
		};
#define TRACKLEN				(32)
	struct track {
		struct trackline	line[TRACKLEN];
	};

	void GetTracks(TuneManager::track ** pp_track, int * p_track_number, int * p_track_length);

	enum TUNE_TYPE
	{
		SONG,
		TRACK,
	};Q_ENUM(TUNE_TYPE)
	void SetGeneratingWave(int tune_type, int index);
	void StopGeneratingWave();
	QByteArray FetchWave(int const length);

public:
	signals:
	void PlayingSongStateChanged(bool is_playing, int playing_song_index);
	void PlayingTrackStateChanged(bool is_playing, int playing_track_index, int playing_line_index);

private slots:
	void InquirePlayingState(void);

private:
	signals:
	void GenerateWaveRequested(int length);
private slots:
	void HandleGenerateWaveRequested(int length);
private:
	void GenerateWave(int length, bool is_synchronized = true);

private :
	TuneManagerPrivate *m_p_private;
	QMutex m_mutex;
};

#endif // TUNEMANAGER_H
