#include <QTextBlock>
#include <QScrollBar>

#include <QDebug>

#include "SongPlainTextEdit.h"

SongPlainTextEdit::SongPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent)
	: HighlightWholeLinePlainTextEdit(parent),
	  m_p_tune_manager(p_tune_manager),
	  m_previous_textcuror_position(0)
{
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPixelSize(20);
	QWidget::setFont(font);

	//QPlainTextEdit::setCursorWidth(10);

	QObject::connect(m_p_tune_manager, &TuneManager::GeneratingSongStateChanged,
					 this, &SongPlainTextEdit::HandleGeneratingSongStateChanged);
	QObject::connect(m_p_tune_manager, &TuneManager::GeneratingTrackStateChanged,
					 this, &SongPlainTextEdit::HandleGeneratingTrackStateChanged);
}

/**********************************************************************************/

bool IsValidChar(QChar character)
{
	if(true ==  character.isDigit()){
		return true;
	}

	char c = character.toLower().toLatin1();
	if(c >= 'a' && c <= 'f'){
		return true;
	}
	return false;
}

/**********************************************************************************/

void SongPlainTextEdit::ShowSong(void)
{
	TuneManager::songline *p_songlines;
	int  number_of_songlines;
	m_p_tune_manager->GetSongLines(&p_songlines, &number_of_songlines);

	QString whole_text;
	for(int i = 0; i < number_of_songlines; i++) {
		QString line_string;
		line_string += QString::asprintf("%02x: ", i);

		for(int j = 0; j < 4; j++) {
			line_string += QString::asprintf("%02x:%02x", p_songlines[i].track[j], p_songlines[i].transp[j]);
			if(j != 3) {
				line_string += QString::asprintf(" ");
			}
		}
		whole_text += line_string;
		if(number_of_songlines - 1 != i){
			whole_text += "\r\n";
		}
	}

	bool is_original_empty = QPlainTextEdit::document()->isEmpty();

	QPlainTextEdit::blockSignals(true);
	QTextCursor textcursor(QPlainTextEdit::document());
	textcursor.select(QTextCursor::Document);
	textcursor.insertText(whole_text);
	textcursor.movePosition(QTextCursor::Start);
	QPlainTextEdit::setTextCursor(textcursor);
	QPlainTextEdit::blockSignals(false);

	if(true == is_original_empty){
		QPlainTextEdit::document()->clearUndoRedoStacks();
	}
	QPlainTextEdit::document()->setModified(false);
}

/**********************************************************************************/

void SongPlainTextEdit::HandleGeneratingSongStateChanged(bool is_generating, int generating_song_index)
{
	QPlainTextEdit::setReadOnly(is_generating);
	int highlight_line_index = generating_song_index - 1;
	if(false == is_generating){
		highlight_line_index = -1;
	}
	HighlightWholeLine(highlight_line_index);
}

/**********************************************************************************/

void SongPlainTextEdit::HandleGeneratingTrackStateChanged(bool is_playing, int generating_track_index, int generating_line_index)
{
	Q_UNUSED(generating_track_index);
	Q_UNUSED(generating_line_index);

	QPlainTextEdit::setReadOnly(is_playing);
}

/**********************************************************************************/

int SongPlainTextEdit::ParseTokensToSongline(QList<QString> songline_string_list,
											   TuneManager::songline *p_songline)
{
	int const track_number_in_one_songline
			=  sizeof(TuneManager::songline::track)/sizeof(TuneManager::songline::track[0]);

	for(int i = 0; i < track_number_in_one_songline; i++){
		uint8_t track = 0;
		uint8_t transp = 0;

		do
		{
			if(true == songline_string_list.at(i).trimmed().isEmpty()){
				break;;
			}

			QStringRef songline_string = QStringRef(&songline_string_list.at(i));
			QStringRef track_string = songline_string;
			QStringRef transp_string;
			if(true == songline_string.contains(":")){
				track_string = songline_string.mid(0, songline_string.indexOf(":"));
				transp_string = songline_string.mid(songline_string.indexOf(":") + 1, -1);
			}

			bool is_ok;
			int value;

			value = track_string.toInt(&is_ok, 16);
			if(false == is_ok){
				return -(i * 2 + 1);
			}
			track = (uint8_t)value;

			if(true == transp_string.trimmed().isEmpty()){
				break;
			}

			value = transp_string.toInt(&is_ok, 16);
			if(false == is_ok){
				return -(i * 2 + 2);
			}
			transp = (uint8_t)value;
		}while(0);

		if(nullptr != p_songline){
			p_songline->track[i] = track;
			p_songline->transp[i] = transp;
		}
	}

	return 0;
}

/**********************************************************************************/

int SongPlainTextEdit::ParseDocument(bool is_update_to_memory)
{
	TuneManager::songline *p_songlines;
	int number_of_songlines;
	m_p_tune_manager->GetSongLines(&p_songlines, &number_of_songlines);

	QTextDocument *p_textdocument = QPlainTextEdit::document();

	QRegExp regexp;
	QString note_pattern = "\\s*"
						"(\\S{1,2}\\:\\s+)?"
						"(\\S{1,2}(?:\\:\\S{1,2})?)"
						"(?:\\s+(\\S+(?:\\:\\S+)?))?"
						"(?:\\s+(\\S+(?:\\:\\S+)?))?"
						"(?:\\s+(\\S+(?:\\:\\S+)?))?"
						".*";

	regexp.setCaseSensitivity(Qt::CaseInsensitive);
	regexp.setPattern(note_pattern);

	int ii = 0;
	for(int i = 0; i < p_textdocument->lineCount(); i++){
		QString line_string = p_textdocument->findBlockByNumber(i).text();
		if(true == line_string.trimmed().isEmpty()){
			continue;
		}
		QString error_string = "ERROR : Song line " + QString::number(i + 1);
		error_string += " : <b>" + p_textdocument->findBlockByLineNumber(i).text() + "</b><br>";
		if(-1 == regexp.indexIn(p_textdocument->findBlockByNumber(i).text())){
			error_string +=	"expression is not recognizable";
			emit ParseScoresErrorOccurred(error_string);
			return -1;
		}
		//qDebug() << regexp.cap(1) << regexp.cap(2) << regexp.cap(3) << regexp.cap(4) << regexp.cap(5);
		TuneManager::songline songline;
		int ret = ParseTokensToSongline(
					QList<QString>() << regexp.cap(2) << regexp.cap(3) << regexp.cap(4) << regexp.cap(5),
							  &songline);
		do
		{
			if(0 == ret){
				break;
			}

			int k = ret/-2;
			if(0x01 & ret){
				error_string +=	"track <b>" + regexp.cap(2 + k) + "</b> is not acceptable";
			}

			error_string +=	"transp <b>" + regexp.cap(2 + k) + "</b> is not acceptable";
		}while(0);

		if(0 != ret){
			emit ParseScoresErrorOccurred(error_string);
			return -2;
		}

		if(true == is_update_to_memory){
			memcpy(&p_songlines[ii], &songline, sizeof(TuneManager::songline));
		}
		ii++;
	}

	if(true == is_update_to_memory){
		m_p_tune_manager->SetSongLines(p_songlines, ii);
	}

	return 0;
}

/**********************************************************************************/

int SongPlainTextEdit::UpdateScores(void)
{
	int ret = ParseDocument(false);
	if(0 != ret){
		return ret;
	}

	ParseDocument(true);
	ShowSong();
	return 0;
}
