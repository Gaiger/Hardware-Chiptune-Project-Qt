#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "InstrumentPlainTextEdit.h"

InstrumentPlainTextEdit::InstrumentPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent)
	: QPlainTextEdit(parent),
	  m_p_tune_manager(p_tune_manager)
{
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPixelSize(22);
	QWidget::setFont(font);
}

/**********************************************************************************/

static const char * const notenames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "H-"};

void InstrumentPlainTextEdit::ShowInstrument(int index)
{
	QPlainTextEdit::clear();
	TuneManager::instrument *p_instruments;
	int number_of_instruments;
	m_p_tune_manager->GetInstruments(&p_instruments, &number_of_instruments);
	TuneManager::instrument *p_current_instument = &p_instruments[index];

	for(int i = 0; i < p_current_instument->length; i++) {
		char line_buffer[1024];
		snprintf(line_buffer, sizeof(line_buffer), "%02x: %c ", i, p_current_instument->line[i].cmd);

		char string_buffer[256];
		if( p_current_instument->line[i].cmd == '+' ||  p_current_instument->line[i].cmd == '=') {
			if( p_current_instument->line[i].param) {
				snprintf(&string_buffer[0], sizeof(string_buffer), "%s%d",
					notenames[(p_current_instument->line[i].param - 1) % 12],
					(p_current_instument->line[i].param - 1) / 12);
			} else {
				snprintf(&string_buffer[0], sizeof(string_buffer), "---");
			}
		} else {
			snprintf(&string_buffer[0], sizeof(&string_buffer[0]), "%02x",
					p_current_instument->line[i].param);
		}
		strncat(&line_buffer[0], string_buffer, sizeof(line_buffer));

		QPlainTextEdit::blockSignals(true);
		QPlainTextEdit::moveCursor(QTextCursor::End);
		QPlainTextEdit::appendPlainText(QString(&line_buffer[0]));
		QPlainTextEdit::moveCursor(QTextCursor::End);
		QPlainTextEdit::blockSignals(false);
	}

}
