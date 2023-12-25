#ifndef HIGHLIGHTWHOLELINEPLAINTEXTEDIT_H
#define HIGHLIGHTWHOLELINEPLAINTEXTEDIT_H

#include <QPlainTextEdit>
#include <QTextBlock>
#include <QScrollBar>

class HighlightWholeLinePlainTextEdit : public QPlainTextEdit
{
	Q_OBJECT
protected:
	HighlightWholeLinePlainTextEdit(QWidget *parent) : QPlainTextEdit(parent)
	{
		QObject::connect(this, &QPlainTextEdit::textChanged,
						 this, &HighlightWholeLinePlainTextEdit::HandleTextChanged);
	}

	void HighlightWholeLine(int index)
	{
		if(-1 == index){
			QPlainTextEdit::document()->clearUndoRedoStacks();
			QPlainTextEdit::document()->setModified(false);
			return ;
		}

		CleanHightlight();

		if( 0 > index || index > QPlainTextEdit::document()->blockCount() - 1){
			return ;
		}

		QTextBlock current_song_textblock = QPlainTextEdit::document()->findBlockByNumber(index);
		do{
			QTextBlockFormat fmt;
			fmt.setProperty(QTextFormat::FullWidthSelection, true);
			fmt.setBackground(QPlainTextEdit::palette().base().color().lighter(150));

			QTextCursor current_song_textcursor(QPlainTextEdit::document());
			current_song_textcursor.setPosition(current_song_textblock.position(), QTextCursor::MoveAnchor);
			QPlainTextEdit::blockSignals(true);
			current_song_textcursor.setBlockFormat(fmt);
			QPlainTextEdit::blockSignals(false);
		}while(0);

		do
		{
			QRect viewport_geometry = QPlainTextEdit::viewport()->geometry();
			QRectF next_line_rect = QPlainTextEdit::blockBoundingGeometry(
						QPlainTextEdit::document()->findBlockByNumber(index + 1));

			if(viewport_geometry.topLeft().y() < next_line_rect.topLeft().y()
					&& viewport_geometry.bottomRight().y() > next_line_rect.bottomRight().y()){
				break;
			}
#define MIN_NUMBER_OF_TOP_COUNTS_WHILE_SCROLLING			(2)
			int scrolling_value = current_song_textblock.firstLineNumber() - MIN_NUMBER_OF_TOP_COUNTS_WHILE_SCROLLING;
			if(index + 1 == QPlainTextEdit::document()->blockCount()){
				scrolling_value = QPlainTextEdit::verticalScrollBar()->maximum();
			}

			QPlainTextEdit::verticalScrollBar()->setValue(scrolling_value);
		}while(0);
	}

private slots:
	void HandleTextChanged(void)
	{
		blockSignals(true);
		CleanHightlight();
		blockSignals(true);
	}

private:
	void CleanHightlight(void)
	{
		do{
			QTextBlockFormat fmt;
			fmt.setProperty(QTextFormat::FullWidthSelection, true);
			fmt.setBackground( QPlainTextEdit::palette().base().color());
			QTextCursor cursor(QPlainTextEdit::document());
			for(int i = 0; i < QPlainTextEdit::document()->blockCount(); i++){
				QTextBlock textblock = QPlainTextEdit::document()->findBlockByNumber(i);
				if( QPlainTextEdit::palette().base().color() == textblock.blockFormat().background().color()){
					continue;
				}
				cursor.setPosition(textblock.position(), QTextCursor::MoveAnchor);
				QPlainTextEdit::blockSignals(true);
				cursor.setBlockFormat(fmt);
				QPlainTextEdit::blockSignals(false);
			}
		}while(0);
	}
};

#endif // HIGHLIGHTWHOLELINEPLAINTEXTEDIT_H
