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
				start_generating_track(index);
				break;
			}
			start_generating_song(index);
		}while(0);

		InquireGeneratingState();
		m_inquiring_playing_state_timer.setInterval(25);
		m_inquiring_playing_state_timer.start();
	}

	void CleanAll(void)
	{
		ResetGeneratingWave();
		m_s_is_generating_song = false;
		m_is_generating_track = false;
		m_is_B_note_as_H_note = false;
	}

	bool IsGeneratingSongStateChanged(void)
	{
		bool is_changed = false;
		int generating_song_index = -1;

		bool s_is_generating_song = is_song_playing(&generating_song_index);
		do
		{
			if(s_is_generating_song != m_s_is_generating_song){
				is_changed = true;
			}

			if(false == s_is_generating_song){
				break;
			}

			if(m_generating_song_index != generating_song_index){
				is_changed = true;
			}
		}while(0);

		m_s_is_generating_song = s_is_generating_song;
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
			emit m_p_public->GeneratingSongStateChanged(m_s_is_generating_song, m_generating_song_index);
		}

		if(true == TuneManagerPrivate::IsGeneratingTrackStateChanged()){
			emit  m_p_public->GeneratingTrackStateChanged(m_is_generating_track,
											 m_generating_track_index,
											 m_generating_line_index);
		}

	#if(0)
		if(false == m_p_private->m_s_is_generating_song &&
				false == m_p_private->m_is_generating_track)
		{
			emit GeneratingWaveStopped();
		}
	#endif
	}


public:
	bool m_is_B_note_as_H_note;

	QByteArray m_wave_bytearray;
	int m_wave_prebuffer_length;

	QTimer m_inquiring_playing_state_timer;

	bool m_s_is_generating_song;
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
	if( QMetaType::UnknownType == QMetaType::type("EXPORT_TYPE")){
			qRegisterMetaType<TuneManager::EXPORT_TYPE>("EXPORT_TYPE");
	}

	if( QMetaType::UnknownType == QMetaType::type("TUNE_TYPE")){
			qRegisterMetaType<TuneManager::TUNE_TYPE>("TUNE_TYPE");
	}

	initialize_chip();
	m_p_private = new TuneManagerPrivate();
	m_p_private->CleanAll();
	m_p_private->m_p_public = this;
	QObject::connect(&m_p_private->m_inquiring_playing_state_timer, &QTimer::timeout,
					this, &TuneManager::InquireGeneratingState);
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

int TuneManager::LoadSongFile(QString filename_string)
{
	QMutexLocker locker(&m_mutex);
	if(false == QFileInfo(filename_string).isFile()){
		return -1;
	}

	loadfile(filename_string.toLatin1().data());
	optimize();

	return 0;
}

/**********************************************************************************/

int TuneManager::SaveSongFile(QString filename_string)
{
	QMutexLocker locker(&m_mutex);
	savefile(filename_string.toLatin1().data());
	return 0;
}

/**********************************************************************************/

int TuneManager::ImportChunkDataFile(QString filename_string)
{
	if(false == QFileInfo(filename_string).isFile()){
		return -1;
	}
	QByteArray readdata_bytearray;

	QFile file(filename_string);
	file.open(QFile::ReadOnly);
	readdata_bytearray = file.readAll();
	file.close();

	int max_track, songlen;
	memcpy(&max_track, readdata_bytearray.constData(), 4);
	memcpy(&songlen, readdata_bytearray.constData() + 4, 4);
	set_chunks(max_track, songlen, (uint8_t*)readdata_bytearray.constData() + 8, readdata_bytearray.size() - 8);

	return 0;
}

/**********************************************************************************/

int TuneManager::ExportChunkDataFile(QString filename_string, TuneManager::EXPORT_TYPE export_type)
{
	int chunk_size;
	int offset_number;

	get_chunk_information(&chunk_size, &offset_number);

	uint8_t *p_chunks = (uint8_t*)alloca(chunk_size);
	int *p_section_begin_indexes = (int*)alloca(offset_number * sizeof(int));
	int *p_offsets = (int*)alloca(offset_number * sizeof(int));
	int max_track, song_length;

	memset(p_chunks, 0, chunk_size);
	memset(p_section_begin_indexes, 0, offset_number * sizeof(int));
	memset(p_offsets, 0, offset_number * sizeof(int));
	get_chunks(&max_track, &song_length, p_chunks, p_section_begin_indexes, p_offsets);

	QFile file;
	QString out_string;

	do
	{
		if(TuneManager::BINARY_DATA != export_type){
			break;
		}

		file.setFileName(filename_string);
		if(false == file.open(QFile::WriteOnly)){
			return -1;
		}
		file.write((const char *)&max_track, sizeof(int));
		file.write((const char *)&song_length, sizeof(int));
		file.write((const char *)p_chunks, chunk_size);
		file.close();
	}while(0);

	do
	{
		if(TuneManager::BINARY_DATA == export_type){
			break;
		}

		out_string.clear();
		QString basename_string = QFileInfo(filename_string).baseName();
		out_string += QString("#ifndef _") + basename_string.toUpper() + QString("_H_\n");
		out_string += QString("#define _") + basename_string.toUpper() + QString("_H_\n");
		out_string += QString("\n#include <stdint.h>\n\n");
		out_string += QString::asprintf("#define MAX_TRACK\t\t\t\t\t\t\t\t\t(0x%02x)\n", max_track);
		out_string += QString::asprintf("#define SONG_LENGTH\t\t\t\t\t\t\t\t\t\t(0x%02x)\n\n", song_length);
		if(TuneManager::C_HEADER == export_type ||
				TuneManager::TEXT == export_type){
			out_string += QString("const uint8_t chunks[] = {");
			int ii = 0;
			int kk = 0;
			for(int i = 0; i < chunk_size - 1; i++){
				if(i == p_section_begin_indexes[ii]){
					out_string += QString::asprintf("\n");
					ii++;
					kk = 0;
				}
				if(0 == kk % 12){
					out_string += QString::asprintf("\n\t");
				}
				out_string += QString::asprintf("0x%02x, ", p_chunks[i]);
				kk++;
			}
			out_string += QString::asprintf("0x%02x \n};\n\n", p_chunks[chunk_size - 1]);
		}
		out_string += QString("#endif ") + QString("/*") + basename_string.toUpper() + QString("_H_") + QString("*/");

		file.setFileName(filename_string);
		if(TuneManager::AVR_ASM_AND_C_HEADER == export_type){
			file.setFileName(basename_string + ".h");
		}
		if(false == file.open(QFile::WriteOnly|QFile::Text)){
			return -1;
		}
		file.write(out_string.toLatin1());
		file.close();

		if(TuneManager::AVR_ASM_AND_C_HEADER != export_type){
			break;
		}

		out_string.clear();
		out_string += QString::asprintf("\t.global\tsongdata\n\n");
		out_string += QString::asprintf("songdata:\n");
		out_string += QString::asprintf("# ");
		for(int i = 0; i < 16 + max_track; i++) {
			out_string += QString::asprintf("%04x ", p_offsets[i]);
		}
		QString::asprintf("\n");
		int ii = 0;
		for(int i = 0; i < chunk_size; i++){
			if(i == p_section_begin_indexes[ii]){
				out_string += QString::asprintf("\n");
				ii++;
			}
			out_string += QString::asprintf("\t.byte\t0x%02x\n", p_chunks[i]);
		}

		file.setFileName(filename_string);
		if(false == file.open(QFile::WriteOnly|QFile::Text)){
			return -1;
		}
		file.write(out_string.toLatin1());
		file.close();
	}while(0);

	return 0;
}

/**********************************************************************************/

void TuneManager::GetSongLines(TuneManager::songline ** pp_songlines, int * p_number_of_songlines)
{
	get_songlines((void**)pp_songlines, p_number_of_songlines);
	//get_tracks((void**)&m_p_private->m_p_tracks, &m_p_private->m_number_of_tracks, &m_p_private->m_track_length);
	//get_instruments((void**)&m_p_private->m_p_instruments, &m_p_private->m_number_of_instruments);

	//*pp_songlines = m_p_private->m_p_songlines;
	//*p_number_of_songlines = m_p_private->m_number_of_songlines;
}

/**********************************************************************************/

void TuneManager::GetTracks(TuneManager::track ** pp_tracks, int * p_track_number, int * p_track_length)
{
	get_tracks((void**)pp_tracks, p_track_number, p_track_length);
}

/**********************************************************************************/

void TuneManager::GetInstruments(TuneManager::instrument ** pp_instruments, int * p_number_of_instruments)
{
	get_instruments((void**)pp_instruments, p_number_of_instruments);
}

/**********************************************************************************/

void TuneManager::SetSongLines(TuneManager::songline * p_songlines, int number_of_songlines)
{
	set_songlines(p_songlines, number_of_songlines);
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

bool TuneManager::IsGeneratingWave(int *p_tune_type)
{
	do
	{
		if(true == m_p_private->m_s_is_generating_song){
			if(nullptr != p_tune_type){
				*p_tune_type = TuneManager::SONG;
			}
			return true;
		}

		if(true == m_p_private->m_is_generating_track){
			if(nullptr != p_tune_type){
				*p_tune_type = TuneManager::TRACK;
			}
			return true;
		}
	}while(0);

	return false;
}
