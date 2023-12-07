#include <QTextBlock>
#include <QRegExp>

#include <QDebug>
#include "InstrumentPlainTextEdit.h"

InstrumentPlainTextEdit::InstrumentPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent)
	: QPlainTextEdit(parent),
	  m_p_tune_manager(p_tune_manager),
	  m_current_shown_index(-1)
{
	QPlainTextEdit::setFocusPolicy(Qt::ClickFocus);
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPixelSize(22);
	QWidget::setFont(font);

	QObject::connect(m_p_tune_manager, &TuneManager::GeneratingSongStateChanged,
					 this, &InstrumentPlainTextEdit::HandleGeneratingSongStateChanged);
	QObject::connect(m_p_tune_manager, &TuneManager::GeneratingTrackStateChanged,
					 this, &InstrumentPlainTextEdit::HandleGeneratingTrackStateChanged);
}

/**********************************************************************************/

void InstrumentPlainTextEdit::ShowInstrument(int index)
{
	QList<QString> note_name_list = m_p_tune_manager->GetNoteNameList();

	TuneManager::instrument *p_instruments;
	int number_of_instruments;
	m_p_tune_manager->GetInstruments(&p_instruments, &number_of_instruments);
	TuneManager::instrument *p_current_instument = &p_instruments[index];

	QString whole_text;
	for(int i = 0; i < p_current_instument->length; i++) {
		QString line_string;
		line_string += QString::asprintf( "%02x: %c ", i, p_current_instument->line[i].cmd);

		if( p_current_instument->line[i].cmd == '+' ||  p_current_instument->line[i].cmd == '=') {
			if( p_current_instument->line[i].param) {
				uint8_t param = p_current_instument->line[i].param;
				QString note_string = note_name_list.at((param - 1) % note_name_list.size());
				line_string += QString::asprintf("%s%d",
												 note_string.toLatin1().constData(),
												 (param - 1) / note_name_list.size());
			} else {
				line_string += QString::asprintf("---");
			}
		} else {
			line_string += QString::asprintf("%02x", p_current_instument->line[i].param);
		}

		whole_text += line_string;
		if(p_current_instument->length - 1 != i){
			whole_text += "\r\n";
		}
	}

	QPlainTextEdit::blockSignals(true);
	QTextCursor textcursor(QPlainTextEdit::document());
	textcursor.select(QTextCursor::Document);
	textcursor.insertText(whole_text);
	QPlainTextEdit::blockSignals(false);

	if(m_current_shown_index != index){
		QPlainTextEdit::document()->clearUndoRedoStacks();
	}
	m_current_shown_index = index;

	QPlainTextEdit::document()->setModified(false);
}

/**********************************************************************************/

void InstrumentPlainTextEdit::HandleGeneratingSongStateChanged(bool is_generating, int generating_song_index)
{
	Q_UNUSED(generating_song_index);

	QPlainTextEdit::setReadOnly(is_generating);
}

/**********************************************************************************/

void InstrumentPlainTextEdit::HandleGeneratingTrackStateChanged(bool is_generating, int generating_track_index, int generating_line_index)
{
	Q_UNUSED(generating_track_index);
	Q_UNUSED(generating_line_index);

	QPlainTextEdit::setReadOnly(is_generating);
}

/**********************************************************************************/

int InstrumentPlainTextEdit::ParseTokensToInstrline(QString cmd_string, QString parameter_string, TuneManager::instrline *p_instrline)
{
	if(1 != cmd_string.size()){
		return -1;
	}
	char cmd = cmd_string.at(0).toLatin1();
	uint8_t param;
	switch(cmd){
	case 0:
		param = 0;
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
			bool is_ok;
			int value = parameter_string.toInt(&is_ok, 16);
			if(false == is_ok || 0 > value || 0xFF < value){
				return -2;
			}
			param = (uint8_t)value;
		}while(0);
		break;
	case '+':
	case '=':
		do{
			QList<QString> note_name_list = m_p_tune_manager->GetNoteNameList();
			QString note_string = QStringRef(&parameter_string, 0, note_name_list.at(0).size()).toString();
			int note_index = note_name_list.indexOf(note_string);
			if(-1 == note_index){
				return -2;
			}

			QStringRef number_string(&parameter_string, note_string.size(),
									 parameter_string.size() - note_name_list.at(note_index).size());
			bool is_ok;
			int value = number_string.trimmed().toInt(&is_ok, 10);
			if(false == is_ok || 0 > value || 6 < value){
				return -2;
			}

			param  = (uint8_t)((note_index + 1) + value * note_name_list.size());
		}while(0);
		break;
	default:
		return -1;
		break;
	}

	if(nullptr != p_instrline) {
		p_instrline->cmd = cmd;
		p_instrline->param = param;
	}

	return 0;
}

int InstrumentPlainTextEdit::ParseDocument(bool is_update_to_memory)
{
	TuneManager::instrument *p_instruments;
	int number_of_instruments;
	m_p_tune_manager->GetInstruments(&p_instruments, &number_of_instruments);

	QTextDocument *p_textdocument = QPlainTextEdit::document();

	QRegExp regexp;
	//QString pattern = ".*(d|f|i|j|l|m|t|v|w|\\+|\\=|\\~)\\s+(\\S{1,2}(?:\\s*\\S+)?)\\s*.*";
	//QString pattern = ".*(\\w|\\+|\\=|\\~)\\s+(\\S{1,2}(?:\\s*\\S+)?)\\s*.*";
	QString pattern = ".*([^\\:|\\s]+)\\s+(\\S{1,2}(?:\\s*\\S+)?)\\s*.*";
	regexp.setCaseSensitivity(Qt::CaseInsensitive);
	regexp.setPattern(pattern);

	int ii = 0;
	for(int i = 0; i < p_textdocument->lineCount(); i++){
		QString line_string = p_textdocument->findBlockByNumber(i).text();
		if(true == line_string.trimmed().isEmpty()){
			continue;
		}
		QString error_string = "ERROR : Instrument line " + QString::number(i + 1);
		error_string += " : <b>" + p_textdocument->findBlockByLineNumber(i).text() + "</b><br>";

		if(-1 == regexp.indexIn(p_textdocument->findBlockByNumber(i).text())){
			error_string +=	"expression is not recognizable";
			emit ParseTimbreErrorOccurred(error_string);
			return -1;
		}

		//qDebug() << regexp.cap(1) << regexp.cap(2);
		TuneManager::instrline instrline;
		int ret = ParseTokensToInstrline(regexp.cap(1).toLower(), regexp.cap(2).toUpper(), &instrline);
		do{
			if(-1 == ret){
				error_string +=	"cmd <b>" + regexp.cap(1) + "</b> is unknown";
				break;
			}
			if(-2 == ret){
				error_string +=	"param <b>" + regexp.cap(2) +"</b> is not acceptable";
			}
		} while(0);

		if(0 != ret){
			emit ParseTimbreErrorOccurred(error_string);
			return -2;
		}

		if(true == is_update_to_memory){
			memcpy(&p_instruments[m_current_shown_index].line[ii], &instrline, sizeof(TuneManager::instrline));
		}
		ii++;
	}

	if(true == is_update_to_memory){
		p_instruments[m_current_shown_index].length = ii;
	}

	return 0;
}

/**********************************************************************************/

int InstrumentPlainTextEdit::UpdateTimbre(void)
{
	int ret = ParseDocument(false);
	if(0 != ret){
		return ret;
	}
	ParseDocument(true);
	ShowInstrument(m_current_shown_index);
	return 0;
}
