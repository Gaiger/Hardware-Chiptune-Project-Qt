#include "InstrumentPlainTextEdit.h"

InstrumentPlainTextEdit::InstrumentPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent)
	: QPlainTextEdit(parent),
	  m_p_tune_manager(p_tune_manager)
{
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPixelSize(16);
	QWidget::setFont(font);
}

/**********************************************************************************/

void InstrumentPlainTextEdit::ShowInstrument(int index)
{
	TuneManager::instrument *p_instruments;
	int number_of_instruments;
	m_p_tune_manager->GetInstruments(&p_instruments, &number_of_instruments);
}
