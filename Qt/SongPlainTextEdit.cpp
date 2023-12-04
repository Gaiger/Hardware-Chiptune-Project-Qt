#include <QTextBlock>
#include <QScrollBar>

#include <QDebug>

#include "SongPlainTextEdit.h"

SongPlainTextEdit::SongPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent)
	: QPlainTextEdit(parent),
	  m_p_tune_manager(p_tune_manager),
	  m_previous_textcuror_position(0)
{
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPixelSize(20);
	QWidget::setFont(font);

	QPlainTextEdit::setCursorWidth(10);

	QObject::connect(m_p_tune_manager, &TuneManager::GeneratingSongStateChanged,
					 this, &SongPlainTextEdit::HandleGeneratingSongStateChanged);

	QObject::connect(this, &QPlainTextEdit::cursorPositionChanged,
					 this, &SongPlainTextEdit::HandleCursorPositionChanged);
}

/**********************************************************************************/

void SongPlainTextEdit::HandleCursorPositionChanged(void)
{
	CorrectCursorPosition();
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

void SongPlainTextEdit::CorrectCursorPosition(void)
{
	QString text_string = QPlainTextEdit::toPlainText();

	do
	{
		QTextCursor text_cursor = QPlainTextEdit::textCursor();
		int current_textcuror_position = text_cursor.position();

		if(current_textcuror_position >= QPlainTextEdit::toPlainText().size()){
			text_cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, 1);
			QPlainTextEdit::setTextCursor(text_cursor);
			break;
		}

		QChar current_cursor_character = QPlainTextEdit::toPlainText().at(current_textcuror_position);
		if(true == IsValidChar(current_cursor_character)){
			break;
		}

		do
		{

			bool is_to_right = false;
			if(m_previous_textcuror_position + 1 == current_textcuror_position){
				is_to_right = true;
			}

			int n = 0;
			while(1)
			{
				do
				{
					if(true == is_to_right){
						n += 1;
						break;
					}

					n -= 1;
				}while(0);

				if(true == IsValidChar(QPlainTextEdit::toPlainText().at(current_textcuror_position + n))){
					break;
				}
			}

			if(true == is_to_right){
				text_cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, n);
				break;
			}

			text_cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, -n);
		}while(0);
		QPlainTextEdit::setTextCursor(text_cursor);

	}while(0);

	m_previous_textcuror_position =  QPlainTextEdit::textCursor().position();
}


/**********************************************************************************/

void SongPlainTextEdit::ShowSongs(void)
{
	TuneManager::songline *p_songs;
	int number_of_songs;
	m_p_tune_manager->GetSongs(&p_songs, &number_of_songs);

	int i, j;
	for(i = 0; i < number_of_songs; i++) {
		QString line_string;
		line_string += QString::asprintf("%02x: ", i);

		for(j = 0; j < 4; j++) {
			line_string += QString::asprintf("%02x:%02x", p_songs[i].track[j], p_songs[i].transp[j]);
			if(j != 3) {
				line_string += QString::asprintf(" ");
			}
		}

		QPlainTextEdit::blockSignals(true);
		QPlainTextEdit::moveCursor(QTextCursor::End);
		QPlainTextEdit::appendPlainText(line_string);
		QPlainTextEdit::moveCursor(QTextCursor::End);
		QPlainTextEdit::blockSignals(false);
	}

	QPlainTextEdit::moveCursor(QTextCursor::Start);
}


/**********************************************************************************/

void SongPlainTextEdit::HandleGeneratingSongStateChanged(bool is_playing, int generating_song_index)
{
	QPlainTextEdit::setReadOnly(is_playing);
	if(false == is_playing){
		return ;
	}

	do{
		QTextBlockFormat fmt;
		fmt.setProperty(QTextFormat::FullWidthSelection, true);
		fmt.setBackground( QPlainTextEdit::palette().base().color());
		QTextCursor cursor(QPlainTextEdit::document());
		for(int i = 0; i < QPlainTextEdit::document()->blockCount(); i++){
			QTextBlock textblock = QPlainTextEdit::document()->findBlockByLineNumber(i);
			if( QPlainTextEdit::palette().base().color() == textblock.blockFormat().background().color()){
				continue;
			}
			cursor.setPosition(textblock.position(), QTextCursor::MoveAnchor);
			cursor.setBlockFormat(fmt);
		}
	}while(0);

	if( 0 > generating_song_index || generating_song_index > QPlainTextEdit::document()->blockCount() - 1){
		return ;
	}

	QTextBlock current_song_textblock = QPlainTextEdit::document()->findBlockByLineNumber(generating_song_index);

	do{
		QTextBlockFormat fmt;
		fmt.setProperty(QTextFormat::FullWidthSelection, true);
		fmt.setBackground(QPlainTextEdit::palette().base().color().lighter(150));

		QTextCursor current_song_textcursor(QPlainTextEdit::document());
		current_song_textcursor.setPosition(current_song_textblock.position(), QTextCursor::MoveAnchor);
		current_song_textcursor.setBlockFormat(fmt);
	}while(0);

	do
	{
		QRect viewport_geometry = QPlainTextEdit::viewport()->geometry();
		QRectF next_line_rect = QPlainTextEdit::blockBoundingGeometry(
					QPlainTextEdit::document()->findBlockByLineNumber(generating_song_index + 1));

		if(viewport_geometry.topLeft().y() < next_line_rect.topLeft().y()
				&& viewport_geometry.bottomRight().y() > next_line_rect.bottomRight().y()){
			break;
		}

		int scrolling_value = current_song_textblock.firstLineNumber() - 2;
		if(generating_song_index + 1 == QPlainTextEdit::document()->blockCount()){
			scrolling_value = QPlainTextEdit::verticalScrollBar()->maximum();
		}

		QPlainTextEdit::verticalScrollBar()->setValue(scrolling_value);
	}while(0);
}
