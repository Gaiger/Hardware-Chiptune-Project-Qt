#include <QThread>
#include <QTimer>
#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

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

		InquireGeneratingState();
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

		InquireGeneratingState();
		m_inquiring_playing_state_timer.setInterval(25);
		m_inquiring_playing_state_timer.start();
	}

	void CleanAll(void)
	{
		ResetGeneratingWave();
		m_is_generating_song = false;
		m_is_generating_track = false;
		m_p_songlines = nullptr;
		m_p_number_of_songlines = nullptr;

		m_p_tracks = nullptr;
		m_number_of_tracks = 0;
		m_track_length = 0;

		m_is_B_note_as_H_note = false;
	}

	bool IsGeneratingSongStateChanged(void)
	{
		bool is_changed = false;
		int generating_song_index = -1;

		bool is_generating_song = is_song_playing(&generating_song_index);
		do
		{
			if(is_generating_song != m_is_generating_song){
				is_changed = true;
			}

			if(false == is_generating_song){
				break;
			}

			if(m_generating_song_index != generating_song_index){
				is_changed = true;
			}
		}while(0);

		m_is_generating_song = is_generating_song;
		m_generating_song_index = generating_song_index;
		return is_changed;
	}

	bool IsGeneratingTrackStateChanged(void)
	{
		bool is_changed = false;
		int generating_track_index = -1;
		int generating_line_index = -1;

		bool is_generating_track = is_track_playing(&generating_track_index, &generating_line_index);
		do
		{
			if(is_generating_track != m_is_generating_track)
			{
				is_changed = true;
			}

			if(false == is_generating_track){
				break;
			}

			if(m_generating_track_index != generating_track_index
					|| m_generating_line_index != generating_line_index){
				is_changed = true;
			}
		}while(0);

		m_is_generating_track = is_generating_track;
		m_generating_track_index = generating_track_index;
		m_generating_line_index = generating_line_index;
		return is_changed;
	}

	void InquireGeneratingState(void)
	{
		if(true == TuneManagerPrivate::IsGeneratingSongStateChanged()){
			emit m_p_public->GeneratingSongStateChanged(m_is_generating_song, m_generating_song_index);
		}

		if(true == TuneManagerPrivate::IsGeneratingTrackStateChanged()){
			emit  m_p_public->GeneratingTrackStateChanged(m_is_generating_track,
											 m_generating_track_index,
											 m_generating_line_index);
		}

	#if(0)
		if(false == m_p_private->m_is_generating_song &&
				false == m_p_private->m_is_generating_track)
		{
			emit GeneratingWaveStopped();
		}
	#endif
	}


public:
	bool m_is_B_note_as_H_note;

	TuneManager::songline *m_p_songlines;
	int *m_p_number_of_songlines;

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

	TuneManager *m_p_public;
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
	m_p_private->m_p_public = this;
	QObject::connect(&m_p_private->m_inquiring_playing_state_timer, &QTimer::timeout,
					this, &TuneManager::InquireGeneratingState);
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

void TuneManager::SetHNoteAsBNote(bool is_H_note_as_B_note)
{
	m_p_private->m_is_B_note_as_H_note = is_H_note_as_B_note;
}

/**********************************************************************************/

void TuneManager::LoadFile(QString filename_string)
{
	QMutexLocker locker(&m_mutex);
	loadfile(filename_string.toLatin1().data());
	optimize();
	get_songlines((void**)&m_p_private->m_p_songlines, &m_p_private->m_p_number_of_songlines);
	get_tracks((void**)&m_p_private->m_p_tracks, &m_p_private->m_number_of_tracks, &m_p_private->m_track_length);
	get_instruments((void**)&m_p_private->m_p_instruments, &m_p_private->m_number_of_instruments);
	return ;
}

/**********************************************************************************/

void TuneManager::SaveFile(QString filename_string)
{
	QMutexLocker locker(&m_mutex);
	savefile(filename_string.toLatin1().data());
}

/**********************************************************************************/

void TuneManager::ExportFile(QString filename_string, TuneManager::EXPORT_TYPE export_type)
{
	int data_length;
	int resources_number;

	get_export_data_information(&data_length, &resources_number);

	uint8_t *p_data = (uint8_t*)alloca(data_length);
	int *p_blanklines = (int*)alloca(resources_number * sizeof(int));
	int *p_resources = (int*)alloca(resources_number * sizeof(int));
	int maxtrack, songlen;

	get_export_data(&maxtrack, &songlen, p_data, p_blanklines, p_resources);

	QFile file;
	QString out_string;
	do
	{
		if(TuneManager::AVR_ASM_AND_C_HEADER != export_type){
			break;
		}

		out_string.clear();
		out_string += QString::asprintf("\t.global\tsongdata\n\n");
		out_string += QString::asprintf("songdata:\n");
		out_string += QString::asprintf("# ");
		for(int i = 0; i < 16 + maxtrack; i++) {
			out_string += QString::asprintf("%04x ", p_resources[i]);
		}
		QString::asprintf("\n");
		int ii = 0;
		for(int i = 0; i < data_length; i++){
			if(i == p_blanklines[ii]){
				out_string += QString::asprintf("\n");
				ii++;
			}
			out_string += QString::asprintf("\t.byte\t0x%02x\n", p_data[i]);
		}

		file.setFileName(filename_string);
		if(false == file.open(QFile::WriteOnly|QFile::Text)){
			break;
		}
		file.write(out_string.toLatin1());
		file.close();

		out_string.clear();
		QString basename_string = QFileInfo(filename_string).baseName();
		out_string += QString("#ifndef _") + basename_string.toUpper() + QString("_H_\n");
		out_string += QString("#define _") + basename_string.toUpper() + QString("_H_\n");
		out_string += QString("\n#include <stdint.h>\n\n");
		out_string += QString::asprintf("#define MAXTRACK\t\t\t\t\t\t\t\t\t(0x%02x)\n", maxtrack);
		out_string += QString::asprintf("#define SONGLEN\t\t\t\t\t\t\t\t\t\t(0x%02x)\n\n", songlen);
		out_string += QString("#endif ") + QString("/*") + basename_string.toUpper() + QString("_H_") + QString("*/");

		file.setFileName(basename_string + ".h");
		if(false == file.open(QFile::WriteOnly|QFile::Text)){
			break;
		}
		file.write(out_string.toLatin1());
		file.close();
	}while(0);

	do
	{
		if(false == (TuneManager::C_HEADER == export_type || TuneManager::TEXT == export_type)){
			break;
		}

		out_string.clear();
		QString basename_in_upper_string = QFileInfo(filename_string).baseName().toUpper();
		out_string += QString("#ifndef _") + basename_in_upper_string + QString("_H_\n");
		out_string += QString("#define _") + basename_in_upper_string + QString("_H_\n");
		out_string += QString("\n#include <stdint.h>\n\n");
		out_string += QString::asprintf("#define MAXTRACK\t\t\t\t\t\t\t\t\t(0x%02x)\n", maxtrack);
		out_string += QString::asprintf("#define SONGLEN\t\t\t\t\t\t\t\t\t\t(0x%02x)\n\n", songlen);
		out_string += QString("const uint8_t songdata[] = {");
		int ii = 0;
		int kk = 0;
		for(int i = 0; i < data_length - 1; i++){
			if(i == p_blanklines[ii]){
				out_string += QString::asprintf("\n");
				ii++;
				kk = 0;
			}
			if(0 == kk % 12){
				out_string += QString::asprintf("\n\t");
			}
			out_string += QString::asprintf("0x%02x, ", p_data[i]);
			kk++;
		}
		out_string += QString::asprintf("0x%02x \n};\n\n", p_data[data_length - 1]);
		out_string += QString("#endif ") + QString("/*") + basename_in_upper_string+ QString("_H_") + QString("*/");

		file.setFileName(filename_string);
		if(false == file.open(QFile::WriteOnly|QFile::Text)){
			break;
		}
		file.write(out_string.toLatin1());
		file.close();
	}while(0);

	do
	{
		if(TuneManager::BINARY_DATA != export_type){
			break;
		}

		QByteArray data_bytearray((const char *)p_data, data_length);
		data_bytearray =
				QByteArray((const char *)&maxtrack, sizeof(int))
				+ QByteArray((const char *)&songlen, sizeof(int))
				+ data_bytearray;

		//int length = data_bytearray.size();
		//data_bytearray = QByteArray((const char *)&length, sizeof(int)) + data_bytearray;
		file.setFileName(filename_string);
		if(false == file.open(QFile::WriteOnly)){
			break;
		}
		file.write(data_bytearray);
		file.close();
	}while(0);
}

/**********************************************************************************/

void TuneManager::GetSongLines(TuneManager::songline ** pp_songlines, int ** pp_number_of_songlines)
{
	*pp_songlines = m_p_private->m_p_songlines;
	*pp_number_of_songlines = m_p_private->m_p_number_of_songlines;
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

const QList<QString> TuneManager::GetNoteNameList(void)
{
	QList<QString> note_string;
	do
	{
		if(m_p_private->m_is_B_note_as_H_note){
			note_string << "C-" << "C#" << "D-" << "D#" << "E-"<< "F-"<< "F#" << "G-" << "G#"<< "A-" << "A#" << "H-";
			break;
		}

		note_string << "C-" << "C#" << "D-" << "D#" << "E-"<< "F-"<< "F#" << "G-" << "G#"<< "A-" << "A#" << "B-";
	}while(0);

	return note_string;
}

/**********************************************************************************/

void TuneManager::InquireGeneratingState(void)
{
	m_p_private->InquireGeneratingState();
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
