#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <QDebug>
#include <QTextBlock>
#include <QScrollBar>

#include "SongPlainTextEdit.h"

SongPlainTextEdit::SongPlainTextEdit(QWidget *parent)
	: QPlainTextEdit(parent),
	  m_previous_textcuror_position(0)
{
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	QWidget::setFont(font);

	QPlainTextEdit::setCursorWidth(10);
	QObject::connect(this, &QPlainTextEdit::cursorPositionChanged,
					 this, &SongPlainTextEdit::HandleCursorPositionChanged);
}

/**********************************************************************************/

void SongPlainTextEdit::HandleCursorPositionChanged(void)
{
	CorrectCursorPosition();
	HighlightCurrentLine();
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

void SongPlainTextEdit::HighlightCurrentLine(void)
{
#if(0)
	QList<QTextEdit::ExtraSelection> extraSelections;
	if (!isReadOnly()) {
		 QTextEdit::ExtraSelection selection;

		 QColor lineColor = QColor(128, 128, 128);

		 selection.format.setBackground(lineColor);
		 selection.format.setProperty(QTextFormat::FullWidthSelection, true);


		//mergeFormatOnWordOrSelection(fmt);
		 selection.cursor = SongPlainTextEdit::textCursor();
		 selection.cursor.clearSelection();

		 extraSelections.append(selection);
	 }

	 setExtraSelections(extraSelections);
#endif
}

/**********************************************************************************/

struct songline {
	uint8_t			track[4];
	uint8_t			transp[4];
};

extern "C"
{
void get_songlines(void** pp_songlines, int *p_song_length);
bool is_song_playing(int *p_playing_song_index);
}

void SongPlainTextEdit::UpdateSongs(void)
{
	int song_length;
	struct songline *p_songs;
	get_songlines((void**)&p_songs, &song_length);


	int i, j;
	for(i = 0; i < song_length; i++) {
		char line_buffer[1024];
		snprintf(line_buffer, sizeof(line_buffer), "%02x: ", i);

		for(j = 0; j < 4; j++) {
			char string_buffer[256];
			snprintf(&string_buffer[0], sizeof(string_buffer), "%02x:%02x", p_songs[i].track[j], p_songs[i].transp[j]);
			strncat(&line_buffer[0], &string_buffer[0], sizeof(line_buffer));
			if(j != 3) {
				snprintf(&string_buffer[0], sizeof(string_buffer), " ");
				strncat(&line_buffer[0], &string_buffer[0], sizeof(line_buffer));
			}
		}

		QPlainTextEdit::blockSignals(true);
		QPlainTextEdit::moveCursor (QTextCursor::End);
		QPlainTextEdit::appendPlainText(QString(&line_buffer[0]));
		QPlainTextEdit::moveCursor (QTextCursor::End);
		QPlainTextEdit::blockSignals(false);
	}

	QPlainTextEdit::moveCursor (QTextCursor::Start);
}

/**********************************************************************************/

void SongPlainTextEdit::UpdateSongPlaying(void)
{
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

	int playing_song_index;
	if(false == is_song_playing(&playing_song_index)){
		return ;
	}

	playing_song_index -= 1;
	if( 0 > playing_song_index || playing_song_index > QPlainTextEdit::document()->blockCount() - 1){
		return ;
	}

	QTextBlock current_song_textblock = QPlainTextEdit::document()->findBlockByLineNumber(playing_song_index);

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
					QPlainTextEdit::document()->findBlockByLineNumber(playing_song_index + 1));

		if(viewport_geometry.topLeft().y() < next_line_rect.topLeft().y()
				&& viewport_geometry.bottomRight().y() > next_line_rect.bottomRight().y()){
			break;
		}

		int scrolling_value = current_song_textblock.firstLineNumber() - 2;
		if(playing_song_index + 1 == QPlainTextEdit::document()->blockCount()){
			scrolling_value = QPlainTextEdit::verticalScrollBar()->maximum();
		}

		QPlainTextEdit::verticalScrollBar()->setValue(scrolling_value);
	}while(0);

}
