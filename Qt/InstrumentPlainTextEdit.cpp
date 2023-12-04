#include "InstrumentPlainTextEdit.h"

InstrumentPlainTextEdit::InstrumentPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent)
	: QPlainTextEdit(parent),
	  m_p_tune_manager(p_tune_manager)
{
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
	QPlainTextEdit::clear();
	TuneManager::instrument *p_instruments;
	int number_of_instruments;
	m_p_tune_manager->GetInstruments(&p_instruments, &number_of_instruments);
	TuneManager::instrument *p_current_instument = &p_instruments[index];

	for(int i = 0; i < p_current_instument->length; i++) {
		QString line_string;
		line_string += QString::asprintf( "%02x: %c ", i, p_current_instument->line[i].cmd);

		if( p_current_instument->line[i].cmd == '+' ||  p_current_instument->line[i].cmd == '=') {
			if( p_current_instument->line[i].param) {
				QString note_string = m_p_tune_manager->GetNoteNameList().at((p_current_instument->line[i].param - 1) % 12);
				line_string += QString::asprintf("%s%d",
												 note_string.toLatin1().constData(),
												 (p_current_instument->line[i].param - 1) / 12);
			} else {
				line_string += QString::asprintf("---");
			}
		} else {
			line_string += QString::asprintf("%02x", p_current_instument->line[i].param);
		}

		QPlainTextEdit::blockSignals(true);
		QPlainTextEdit::moveCursor(QTextCursor::End);
		QPlainTextEdit::appendPlainText(line_string);
		QPlainTextEdit::moveCursor(QTextCursor::End);
		QPlainTextEdit::blockSignals(false);
	}
}

/**********************************************************************************/

void InstrumentPlainTextEdit::HandleGeneratingSongStateChanged(bool is_playing, int generating_song_index)
{
	Q_UNUSED(generating_song_index);
	QPlainTextEdit::setReadOnly(is_playing);
}

/**********************************************************************************/

void InstrumentPlainTextEdit::HandleGeneratingTrackStateChanged(bool is_playing, int generating_track_index, int generating_line_index)
{
	Q_UNUSED(generating_track_index);
	Q_UNUSED(generating_line_index);

	QPlainTextEdit::setReadOnly(is_playing);
}
