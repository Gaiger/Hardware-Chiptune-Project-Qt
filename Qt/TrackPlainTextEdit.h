#ifndef TRACKPLAINTEXTEDIT_H
#define TRACKPLAINTEXTEDIT_H

#include <QPlainTextEdit>

class TrackPlainTextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	TrackPlainTextEdit(QWidget *parent = nullptr);
	void UpdateTrack(void);
};

#endif // TRACKPLAINTEXTEDIT_H
