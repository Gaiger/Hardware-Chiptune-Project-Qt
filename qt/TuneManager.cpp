#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <QThread>
#include <QTimer>
#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include "TuneManager.h"
#include "tune_manager.h"

#include "../chiptune.h"


class TuneManagerPrivate
{
public:
	void GenerateWave(int const length)
	{
		QByteArray generated_bytearray;
		generated_bytearray.reserve(length);
		for(int i = 0; i < length; i++) {
			uint8_t value = chiptune_interrupthandler();
			generated_bytearray += value;
		}

		m_wave_bytearray += generated_bytearray;
	}

	void ResetGeneratingWave(void)
	{
		m_inquiring_playing_state_timer.stop();
		chiptune_silence();
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
				chiptune_start_generating_track(index);
				break;
			}
			chiptune_start_generating_song(index);
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
		m_is_B_note_as_H_note = false;
		m_lights_bits = 0;
		memset(&m_songlines[0], 0, sizeof(m_songlines));
		memset(&m_tracks[0], 0, sizeof(m_tracks));
		memset(&m_instruments[0], 0, sizeof(m_instruments));
	}

	bool IsGeneratingSongStateChanged(void)
	{
		bool is_changed = false;
		int generating_song_index = -1;

		bool is_generating_song = chiptune_is_song_generating(&generating_song_index);
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

		bool is_generating_track = chiptune_is_track_generating(&generating_track_index, &generating_line_index);
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
	}

	int LoadSongFile(QString filename_string)
	{
		FILE *file_ptr;
		char buf[1024];
		int cmd[3];
		int i1, i2, trk[4], transp[4], param[3], note, instr;
		int i;

		file_ptr = fopen(filename_string.toLatin1().constData(), "r");
		if(NULL == file_ptr) {
			return -1;
		}

		m_song_length = 1;
		while(!feof(file_ptr) && fgets(buf, sizeof(buf), file_ptr)) {
			if(9 == sscanf(buf, "songline %x %x %x %x %x %x %x %x %x",
				&i1,
				&trk[0],
				&transp[0],
				&trk[1],
				&transp[1],
				&trk[2],
				&transp[2],
				&trk[3],
				&transp[3])) {

				for(i = 0; i < 4; i++) {
					m_songlines[i1].track[i] = trk[i];
					m_songlines[i1].transp[i] = transp[i];
				}
				if(m_song_length <= i1){
					m_song_length = i1 + 1;
				}
			} else if(8 == sscanf(buf, "trackline %x %x %x %x %x %x %x %x",
				&i1,
				&i2,
				&note,
				&instr,
				&cmd[0],
				&param[0],
				&cmd[1],
				&param[1])) {

				m_tracks[i1].line[i2].note = note;
				m_tracks[i1].line[i2].instr = instr;
				for(i = 0; i < 2; i++) {
					m_tracks[i1].line[i2].cmd[i] = cmd[i];
					m_tracks[i1].line[i2].param[i] = param[i];
				}
			} else if(4 == sscanf(buf, "instrumentline %x %x %x %x",
				&i1,
				&i2,
				&cmd[0],
				&param[0])) {

				m_instruments[i1].line[i2].cmd = cmd[0];
				m_instruments[i1].line[i2].param = param[0];
				if(m_instruments[i1].length <= i2){
					m_instruments[i1].length = i2 + 1;
				}
			}
		}

		fclose(file_ptr);
		return 0;
	}


	int SaveSongFile(QString filename_string) {
		FILE *file_ptr;
		int i, j;

		file_ptr = fopen(filename_string.toLatin1().constData(), "w");
		if(NULL == file_ptr) {
			fprintf(stderr, "save error!\n");
			return -1;
		}

		fprintf(file_ptr, "musicchip tune\n");
		fprintf(file_ptr, "version 1\n");
		fprintf(file_ptr, "\n");
		for(i = 0; i < m_song_length; i++) {
			fprintf(file_ptr, "songline %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				i,
				m_songlines[i].track[0],
				m_songlines[i].transp[0],
				m_songlines[i].track[1],
				m_songlines[i].transp[1],
				m_songlines[i].track[2],
				m_songlines[i].transp[2],
				m_songlines[i].track[3],
				m_songlines[i].transp[3]);
		}
		fprintf(file_ptr, "\n");
		for(i = 1; i < 256; i++) {
			for(j = 0; j < TRACKLEN; j++) {
				TuneManager::trackline *tl = &m_tracks[i].line[j];

				if(tl->note || tl->instr || tl->cmd[0] || tl->cmd[1]) {
					fprintf(file_ptr, "trackline %02x %02x %02x %02x %02x %02x %02x %02x\n",
						i,
						j,
						tl->note,
						tl->instr,
						tl->cmd[0],
						tl->param[0],
						tl->cmd[1],
						tl->param[1]);
				}
			}
		}
		fprintf(file_ptr, "\n");
		for(i = 1; i < 256; i++) {
			if(m_instruments[i].length > 1) {
				for(j = 0; j < m_instruments[i].length; j++) {
					fprintf(file_ptr, "instrumentline %02x %02x %02x %02x\n",
						i,
						j,
						m_instruments[i].line[j].cmd,
						m_instruments[i].line[j].param);
				}
			}
		}

		fclose(file_ptr);
		return 0;
	}

	void OrganizeTracks(void)
	{
		uint8_t used[256], replace[256];
		memset(used, 0, sizeof(used));

		int i, j;


		for(i = 0; i < m_song_length; i++) {
			for(j = 0; j < 4; j++) {
				used[m_songlines[i].track[j]] = 1;
			}
		}

		j = 1;
		replace[0] = 0;
		for(i = 1; i < 256; i++) {
			if(used[i]) {
				replace[i] = j;
				j++;
			} else {
				replace[i] = 0;
			}
		}

		for(i = 1; i < 256; i++) {
			if(replace[i] && replace[i] != i) {
				memcpy(&m_tracks[replace[i]], &m_tracks[i], sizeof(TuneManager::track));
			}
		}

		for(i = 0; i < m_song_length; i++) {
			for(j = 0; j < 4; j++) {
				m_songlines[i].track[j] = replace[m_songlines[i].track[j]];
			}
		}

		for(i = 1; i < 256; i++) {
			uint8_t last = 255;

			for(j = 0; j < TRACKLEN; j++) {
				if(m_tracks[i].line[j].instr) {
					if(m_tracks[i].line[j].instr == last) {
						m_tracks[i].line[j].instr = 0;
					} else {
						last = m_tracks[i].line[j].instr;
					}
				}
			}
		}
	}

	void UpdateChunks(void)
	{
		m_chunks_bytearray.clear();
		int chunk_size;
		int offset_number;

		get_packing_into_chunk_information(m_song_length, &m_songlines[0], &m_tracks[0],
							  &m_instruments[0], &chunk_size, &offset_number);

		m_chunks_bytearray.reserve(chunk_size);
		m_chunks_bytearray.resize(chunk_size);

		pack_into_chunks(m_song_length, &m_songlines[0], &m_tracks[0],
				 &m_instruments[0], &m_max_track, (uint8_t*)m_chunks_bytearray.constData(), nullptr, nullptr);

		chiptune_initialize(false);
	}

public:
	bool m_is_B_note_as_H_note;

	QByteArray m_wave_bytearray;
	int m_wave_prebuffer_length;

	QTimer m_inquiring_playing_state_timer;

	bool m_is_generating_song;
	int m_generating_song_index;

	bool m_is_generating_track;
	int m_generating_track_index;
	int m_generating_line_index;

	int m_song_length;
	TuneManager::songline m_songlines[256];
	TuneManager::track m_tracks[256];
	TuneManager::instrument m_instruments[256];

	int m_max_track;
	QByteArray m_chunks_bytearray;
	uint8_t m_lights_bits;
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

	m_p_private = new TuneManagerPrivate();
	m_p_private->CleanAll();
	m_p_private->m_p_public = this;
	QObject::connect(&m_p_private->m_inquiring_playing_state_timer, &QTimer::timeout,
					this, &TuneManager::InquireGeneratingState);

	set_tune_mananger(this);
	setup_chiptune_data_callback_functions();
	setup_chiptune_lights_callback_function();
	setup_chiptune_raw_reader();
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

	m_p_private->LoadSongFile(filename_string);
	m_p_private->OrganizeTracks();
	m_p_private->UpdateChunks();
	chiptune_initialize(false);
	return 0;
}

/**********************************************************************************/

int TuneManager::SaveSongFile(QString filename_string)
{
	QMutexLocker locker(&m_mutex);
	m_p_private->SaveSongFile(filename_string);
	return 0;
}

/**********************************************************************************/

int TuneManager::ImportChunkDataFile(QString filename_string)
{
	QMutexLocker locker(&m_mutex);
	if(false == QFileInfo(filename_string).isFile()){
		return -1;
	}
	QByteArray readdata_bytearray;

	QFile file(filename_string);
	file.open(QFile::ReadOnly);
	readdata_bytearray = file.readAll();
	file.close();


	memcpy(&m_p_private->m_max_track, readdata_bytearray.constData(), 4);
	memcpy(&m_p_private->m_song_length, readdata_bytearray.constData() + 4, 4);
	m_p_private->m_chunks_bytearray = readdata_bytearray.mid(8, -1);
	memset(&m_p_private->m_songlines[0], 0, sizeof(m_p_private->m_songlines));
	memset(&m_p_private->m_tracks[0], 0, sizeof(m_p_private->m_tracks));
	memset(&m_p_private->m_instruments[0], 0, sizeof(m_p_private->m_instruments));

	unpack_from_chunks(m_p_private->m_max_track, m_p_private->m_song_length,
			 (uint8_t*) m_p_private->m_chunks_bytearray.constData(),
			 m_p_private->m_chunks_bytearray.size(),
			 &m_p_private->m_songlines[0], &m_p_private->m_tracks[0], &m_p_private->m_instruments[0]);

	m_p_private->OrganizeTracks();
	chiptune_initialize(false);
	return 0;
}

/**********************************************************************************/

int TuneManager::ExportChunkDataFile(QString filename_string, TuneManager::EXPORT_TYPE export_type)
{
	QMutexLocker locker(&m_mutex);
	int chunk_size;
	int offset_number;

	get_packing_into_chunk_information(m_p_private->m_song_length, &m_p_private->m_songlines[0], &m_p_private->m_tracks[0],
						  &m_p_private->m_instruments[0], &chunk_size, &offset_number);

	uint8_t *p_chunks = (uint8_t*)alloca(chunk_size);
	int *p_section_begin_indexes = (int*)alloca(offset_number * sizeof(int));
	int *p_offsets = (int*)alloca(offset_number * sizeof(int));
	int max_track, song_length;

	song_length = m_p_private->m_song_length;
	memset(p_chunks, 0, chunk_size);
	memset(p_section_begin_indexes, 0, offset_number * sizeof(int));
	memset(p_offsets, 0, offset_number * sizeof(int));
	pack_into_chunks(m_p_private->m_song_length, &m_p_private->m_songlines[0], &m_p_private->m_tracks[0],
			&m_p_private->m_instruments[0], &max_track, p_chunks, p_section_begin_indexes, p_offsets);

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
		if(true == (TuneManager::C_SOURCECODE != export_type
					 && TuneManager::TEXT != export_type)){
			break;
		}

		out_string.clear();
		out_string += QString("#include <stdint.h>\n\n");
		out_string += QString::asprintf("static int s_max_track = 0x%02d\n", max_track);
		out_string += QString::asprintf("static int s_song_length = 0x%02d\n\n", song_length);
		out_string += QString("static uint8_t s_chunks[] = {");
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

		out_string += QString::asprintf("static int get_max_track(void){ return s_max_track; }\n");
		out_string += QString::asprintf("static int get_song_length(void){ return s_song_length; }\n");
		out_string += QString::asprintf("uint8_t get_chunk_datum(int index){ return s_chunks[index]; }\n");
		out_string += QString::asprintf("void setup_chiptune_data_callback_functions(void)\n");
		out_string += QString::asprintf("{\n");
		out_string += QString::asprintf("chiptune_setup_data_callback_functions(get_max_track,"
										" get_song_length, get_chunk_datum);\n");
		out_string += QString::asprintf("}\n");
		file.setFileName(filename_string);
		if(false == file.open(QFile::WriteOnly|QFile::Text)){
			return -1;
		}
		file.write(out_string.toLatin1());
		file.close();
	}while(0);


	do
	{
		if(TuneManager::AVR_ASM_AND_C_HEADER != export_type){
			break;
		}

		out_string.clear();
		QString basename_string = QFileInfo(filename_string).baseName();
		out_string += QString("#ifndef _") + basename_string.toUpper() + QString("_H_\n");
		out_string += QString("#define _") + basename_string.toUpper() + QString("_H_\n\n");
		out_string += QString::asprintf("#define MAX_TRACK\t\t\t\t\t\t\t\t\t(0x%02x)\n", max_track);
		out_string += QString::asprintf("#define SONG_LENGTH\t\t\t\t\t\t\t\t\t(0x%02x)\n\n", song_length);
		out_string += QString("#endif ") + QString("/*_") + basename_string.toUpper() + QString("_H_") + QString("*/");

		file.setFileName(basename_string + ".h");
		if(false == file.open(QFile::WriteOnly|QFile::Text)){
			return -1;
		}
		file.write(out_string.toLatin1());
		file.close();

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
	*pp_songlines = &m_p_private->m_songlines[0];
	*p_number_of_songlines = m_p_private->m_song_length;
}

/**********************************************************************************/

void TuneManager::GetTracks(TuneManager::track ** pp_tracks, int * p_track_number, int * p_track_length)
{
	*pp_tracks = &m_p_private->m_tracks[0];
	*p_track_number = 256;
	*p_track_length = TRACKLEN;
}

/**********************************************************************************/

void TuneManager::GetInstruments(TuneManager::instrument ** pp_instruments, int * p_number_of_instruments)
{
	*pp_instruments = &m_p_private->m_instruments[0];
	*p_number_of_instruments = 256;
}

/**********************************************************************************/

void TuneManager::SetSongLines(TuneManager::songline * p_songlines, int number_of_songlines)
{
	Q_UNUSED(p_songlines);
	m_p_private->m_song_length = number_of_songlines;
}

/**********************************************************************************/

void TuneManager::UpdateTunes(void)
{
	QMutexLocker locker(&m_mutex);
	m_p_private->UpdateChunks();
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

	emit WaveFetched(fetched_wave_bytearray);
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
	QMutexLocker locker(&m_mutex);
	do
	{
		if(true == m_p_private->m_is_generating_song){
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

/**********************************************************************************/

int TuneManager::GetMaxTrack(void)
{
	return m_p_private->m_max_track;
}

/**********************************************************************************/

uint8_t* TuneManager::GetChunksPtr(void)
{
	return (uint8_t*)m_p_private->m_chunks_bytearray.constData();
}

/**********************************************************************************/

void TuneManager::SetLightBits(uint8_t light_bits)
{
	if(((m_p_private->m_lights_bits >> 0 )& 0x01) != ((light_bits >> 0) & 0x01)){
		emit LightChanged(0, (light_bits >> 0) & 0x01);
	}

	if(((m_p_private->m_lights_bits >> 1 )& 0x01) != ((light_bits >> 1) & 0x01)){
		emit LightChanged(1, (light_bits >> 1) & 0x01);
	}

	m_p_private->m_lights_bits = light_bits;
}

