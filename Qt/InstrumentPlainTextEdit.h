#ifndef INSTRUMENTPLAINTEXTEDIT_H
#define INSTRUMENTPLAINTEXTEDIT_H

#include <QPlainTextEdit>
#include <TuneManager.h>

class InstrumentPlainTextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit InstrumentPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent = nullptr);

	void ShowInstrument(int index);
private:
	TuneManager *m_p_tune_manager;
};

#endif // INSTRUMENTPLAINTEXTEDIT_H
