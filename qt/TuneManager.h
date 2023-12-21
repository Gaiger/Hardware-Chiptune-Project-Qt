#ifndef TUNEMANAGER_H
#define TUNEMANAGER_H
#include <QObject>
#include <QByteArray>

#include <QMutex>

//#define _HNOTE_AS_BNOTE

class TuneManagerPrivate;
class TuneManager : public QObject
{
	Q_OBJECT
public:
	explicit TuneManager(QObject *parent = nullptr);
	~TuneManager() Q_DECL_OVERRIDE;

	int LoadSongFile(QString filename_string);
	int SaveSongFile(QString filename_string);
	int ImportChunkDataFile(QString filename_string);
	enum EXPORT_TYPE
	{
		C_SOURCECODE,
		BINARY_DATA,
		AVR_ASM_AND_C_HEADER,
		TEXT,
	};Q_ENUM(EXPORT_TYPE)
	int ExportChunkDataFile(QString filename_string, TuneManager::EXPORT_TYPE export_type = TuneManager::C_SOURCECODE);

	void SetHNoteAsBNote(bool is_H_note_as_B_note);

	struct songline {
		uint8_t			track[4];
		uint8_t			transp[4];
	};
	void GetSongLines(TuneManager::songline ** pp_songlines, int * p_number_of_songlines);
	void SetSongLines(TuneManager::songline * p_songlines, int number_of_songlines);

	struct trackline {
		uint8_t	note;
		uint8_t	instr;
		uint8_t	cmd[2];
		uint8_t	param[2];
		};
#ifndef TRACKLEN
	#define TRACKLEN				(32)
#endif
	struct track {
		struct trackline	line[TRACKLEN];
	};
	void GetTracks(TuneManager::track ** pp_tracks, int * p_track_number, int * p_track_length);

	struct instrline {
		uint8_t		cmd;
		uint8_t		param;
	};
	struct instrument {
		int			length;
		struct instrline	line[256];
	};
	void GetInstruments(TuneManager::instrument ** pp_instruments, int * p_number_of_instruments);

	const QList<QString> GetNoteNameList(void);

	void UpdateTunes(void);
	int GetMaxTrack(void);
	uint8_t *GetChunksPtr(void);
	void SetLightBits(uint8_t light_bits);
public :
	enum TUNE_TYPE
	{
		SONG,
		TRACK,
	};Q_ENUM(TUNE_TYPE)
	void SetGeneratingWave(int tune_type, int index);
	void ResetGeneratingWave();
	bool IsGeneratingWave(int *p_tune_type = nullptr);
	QByteArray FetchWave(int const length);

public:
	signals:
	void GeneratingSongStateChanged(bool is_generating_song, int generating_song_index);
	void GeneratingTrackStateChanged(bool is_generating_track, int generating_track_index, int generating_line_index);
	void LightChanged(int light_index, bool is_turn_on);
private slots:
	void InquireGeneratingState(void);

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
