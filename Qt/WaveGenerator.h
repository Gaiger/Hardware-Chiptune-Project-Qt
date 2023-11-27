#ifndef WAVEGENERATOR_H
#define WAVEGENERATOR_H
#include <QObject>
#include <QByteArray>

#include <QMutex>


class WaveGeneratorPrivate;

class WaveGenerator : public QObject
{
	Q_OBJECT
public:
	explicit WaveGenerator(QString filename, QObject *parent = nullptr);
	~WaveGenerator() Q_DECL_OVERRIDE;

	void LoadFile(QString filename);
	void SetStartPlaySong(int song_index);

public:
	QByteArray FetchData(int const size);

private:
	signals:
	void GenerateWaveDataRequested(void);
private slots:
	void HandleGenerateWaveDataRequested(void);
private:
	void GenerateWaveData(bool is_synchronized = true);

private :
	WaveGeneratorPrivate * m_p_private;
	QMutex m_mutex;
};

#endif // WAVEGENERATOR_H
