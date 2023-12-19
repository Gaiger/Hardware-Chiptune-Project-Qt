#include <QTextBlock>
#include <QScrollBar>

#include <QDebug>

#include "TrackPlainTextEdit.h"

TrackPlainTextEdit::TrackPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent)
	:  HighlightWholeLinePlainTextEdit(parent),
	  m_p_tune_manager(p_tune_manager),
	  m_current_shown_track_index(-1)
{
	QPlainTextEdit::setFocusPolicy(Qt::ClickFocus);

	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPixelSize(16);
	QWidget::setFont(font);

	QObject::connect(m_p_tune_manager, &TuneManager::GeneratingSongStateChanged,
					 this, &TrackPlainTextEdit::HandleGeneratingSongStateChanged);
	QObject::connect(m_p_tune_manager, &TuneManager::GeneratingTrackStateChanged,
					 this, &TrackPlainTextEdit::HandleGeneratingTrackStateChanged);
}

/**********************************************************************************/

void TrackPlainTextEdit::ShowTrack(int index)
{
	QList<QString> note_name_list = m_p_tune_manager->GetNoteNameList();

	TuneManager::track *p_tracks;
	int number_of_tracks;
	int track_length;
	m_p_tune_manager->GetTracks(&p_tracks, &number_of_tracks, &track_length);
	TuneManager::track *p_current_track = &p_tracks[index];

	QString whole_text;
	for(int i = 0; i < track_length; i++){
		QString line_string;
		line_string += QString::asprintf("%02x: ", i);

		uint8_t note = p_current_track->line[i].note;
		if(p_current_track->line[i].note) {
			QString note_string = note_name_list.at((note - 1) % note_name_list.size());
			line_string += QString::asprintf("%s%d",
											 note_string.toLatin1().constData(),
											(note - 1) / note_name_list.size() );
		} else {
			line_string += QString::asprintf("---");
		}

		line_string += QString::asprintf(" %02x",  p_current_track->line[i].instr);
		for(int j = 0; j < 2; j++) {
			if(p_current_track->line[i].cmd[j]) {
				line_string += QString::asprintf( " %c%02x", p_current_track->line[i].cmd[j],
											   p_current_track->line[i].param[j]);
			} else {
				line_string += QString(" ...");
			}
		}

		whole_text += line_string;
		if(track_length - 1 != i){
			whole_text += "\n";
		}
	}

	QPlainTextEdit::blockSignals(true);
	QTextCursor textcursor(QPlainTextEdit::document());
	textcursor.select(QTextCursor::Document);
	textcursor.insertText(whole_text);
	QPlainTextEdit::blockSignals(false);

	if(m_current_shown_track_index != index){
		QPlainTextEdit::document()->clearUndoRedoStacks();
	}
	m_current_shown_track_index = index;

	QPlainTextEdit::document()->setModified(false);
}

/**********************************************************************************/

void TrackPlainTextEdit::HandleGeneratingSongStateChanged(bool is_playing, int generating_song_index)
{
	Q_UNUSED(generating_song_index);

	QPlainTextEdit::setReadOnly(is_playing);
}

/**********************************************************************************/

void TrackPlainTextEdit::HandleGeneratingTrackStateChanged(bool is_generating, int generating_track_index, int generating_line_index)
{
	Q_UNUSED(generating_track_index);

	QPlainTextEdit::setReadOnly(is_generating);
	int highlight_line_index = generating_line_index - 1;
	if(false == is_generating){
		highlight_line_index = -1;
	}
	HighlightWholeLine(highlight_line_index);
}

/**********************************************************************************/

int TrackPlainTextEdit::ParseTokensToTrackline(QString note_string, QString instr_string,
											  QList<QString> cmd_param_string_list,
											   TuneManager::trackline *p_trackline)
{
	uint8_t	note = 0;
	uint8_t	instr = 0;
	uint8_t	cmd[2] = {0x00, 0x00};
	uint8_t	param[2] = {0x00, 0x00};

	do{
		note = 0;

		if(true == note_string.isEmpty()){
			break;
		}

		if(QString("-") == note_string.at(0)){
			break;
		}

		QList<QString> note_name_list = m_p_tune_manager->GetNoteNameList();
		QString combined_note_string = QStringRef(&note_string, 0, note_name_list.at(0).size()).toString();
		int note_index = note_name_list.indexOf(combined_note_string);
		if(-1 == note_index){
			return -1;
		}

		QStringRef number_string(&note_string, combined_note_string.size(),
								 note_string.size() - note_name_list.at(note_index).size());
		bool is_ok;
		int value = number_string.trimmed().toInt(&is_ok, 10);
		if(false == is_ok || 0 > value || 6 < value){
			return -1;
		}

		note = (uint8_t)((note_index + 1) + value * note_name_list.size());
	}while(0);

	do
	{
		instr = 0;
		if(true == instr_string.isEmpty()){
			break;
		}

		bool is_ok;
		instr = instr_string.toInt(&is_ok, 16);
		if(false == is_ok){
			return -2;
		}
	}while(0);

	do
	{
		for(int i = 0; i < sizeof(cmd)/sizeof(cmd[0]); i++){
			cmd[i] = 0;
			param[i] = 0;
			if(true == cmd_param_string_list.at(i).isEmpty()){
				continue;
			}

			if(QString(".") == cmd_param_string_list.at(i).at(0)){
				continue;
			}

			cmd[i] = cmd_param_string_list.at(i).at(0).toLatin1();
			switch(cmd[i]){
			case 0:
				param[i] = 0;
				break;
			case 'd':
			case 'f':
			case 'i':
			case 'j':
			case 'l':
			case 'm':
			case 'v':
			case 't':
			case 'w':
			case '~':
				do{
					QStringRef parameter_string = QStringRef(&cmd_param_string_list.at(i), 1, cmd_param_string_list.at(i).size() - 1);
					parameter_string.trimmed();
					bool is_ok;
					int value = parameter_string.toInt(&is_ok, 16);
					if(false == is_ok || 0 > value || 0xFF < value){
						if(0 == i){
							return -4;
						}
						return -6;
					}

					param[i] = (uint8_t)value;
				}while(0);
				break;
			default:
				if( 0 == i){
					return -3;
				}
				return -5;
				break;
			}
		}
	}while(0);

	if(nullptr != p_trackline){
		p_trackline->note = note;
		p_trackline->instr = instr;
		memcpy(&p_trackline->cmd[0], &cmd[0], sizeof(cmd));
		memcpy(&p_trackline->param[0], &param[0], sizeof(param));
	}

	return 0;
}

/**********************************************************************************/

int TrackPlainTextEdit::ParseDocument(bool is_update_to_memory)
{
	TuneManager::track *p_tracks;
	int numberf_of_tracks;
	int track_length;
	m_p_tune_manager->GetTracks(&p_tracks, &numberf_of_tracks, &track_length);

	QTextDocument *p_textdocument = QPlainTextEdit::document();

	QRegExp regexp;
	QString note_pattern
			= "(\\S{1,2}\\:\\s*)?"
			  "(\\S{2}\\s*\\S{1})"
			  "(?:\\s+(\\S{1,2}))?"
			  "(?:\\s+(\\S{1}\\s*\\S{0,2})|\\.\\.\\.)?"
			  "(?:\\s+(\\S{1}\\s*\\S{0,2})|\\.\\.\\.)?.*";
	regexp.setCaseSensitivity(Qt::CaseInsensitive);
	regexp.setPattern(note_pattern);

	int ii = 0;
	for(int i = 0; i < p_textdocument->lineCount(); i++){
		QString line_string = p_textdocument->findBlockByNumber(i).text();
		if(true == line_string.trimmed().isEmpty()){
			continue;
		}
		QString error_string = "ERROR : Track line " + QString::number(i + 1);
		error_string += " : <b>" + p_textdocument->findBlockByLineNumber(i).text() + "</b><br>";

		if(-1 == regexp.indexIn(p_textdocument->findBlockByNumber(i).text())){
			error_string +=	"expression is not recognizable";
			emit ParseMeasureErrorOccurred(error_string);
			return -1;
		}
		//qDebug() << regexp.cap(1) << regexp.cap(2) << regexp.cap(3) << regexp.cap(4) << regexp.cap(5);
		TuneManager::trackline trackline;
		int ret = ParseTokensToTrackline(regexp.cap(2).toUpper(), regexp.cap(3),
										 QList<QString>() << regexp.cap(4).toLower() << regexp.cap(5).toLower(),
										 &trackline);
		do
		{
			if(-1 == ret){
				error_string +=	"note <b>" + regexp.cap(2) + "</b> is unknown";
				break;
			}
			if(-2 == ret){
				error_string +=	"instr <b>" + regexp.cap(3) +"</b> is not acceptable";
				break;
			}

			if(-3 == ret || -5 == ret){
				QString cmd_param_string = regexp.cap(4);
				if(-5 == ret){
					cmd_param_string = regexp.cap(5);
				}
				error_string +=	"cmd <b>" + QString(cmd_param_string.at(0)) +"</b> is not unknown";
				break;
			}

			if(-4 == ret || -6 == ret){
				QString cmd_param_string = regexp.cap(4);
				if(-6 == ret){
					cmd_param_string = regexp.cap(5);
				}
				QStringRef cmd_removed_string = QStringRef(&cmd_param_string, 1, cmd_param_string.size() -1);
				do
				{
					if(0 == cmd_removed_string.size()){
						error_string += "no param follows cmd  <b>" + QString(cmd_param_string.at(0)) +"</b>";
						break;
					}
					error_string +=	"param <b>" + cmd_removed_string +"</b> is not acceptable";
				}while (0);
				break;
			}
		}while(0);

		if(0 != ret){
			emit ParseMeasureErrorOccurred(error_string);
			return -2;
		}

		if(true == is_update_to_memory){
			memcpy(&p_tracks[m_current_shown_track_index].line[ii], &trackline, sizeof(TuneManager::trackline));
		}
		ii += 1;
	}

	if(ii != track_length){
		QString temp_string = "ERROR : valid lines = " + QString::number(ii);
		temp_string += "<br> not matched the track length = " + QString::number(track_length);
		emit ParseMeasureErrorOccurred(temp_string);
		return -3;
	}

	return 0;
}

/**********************************************************************************/

int TrackPlainTextEdit::UpdateMeasure(void)
{
	int ret = ParseDocument(false);
	if(0 != ret){
		return ret;
	}

	ParseDocument(true);
	m_p_tune_manager->UpdateTunes();
	ShowTrack(m_current_shown_track_index);
	return 0;
}
