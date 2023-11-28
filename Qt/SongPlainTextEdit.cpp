#include <QDebug>
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
void get_song_data(int *p_songx, int *p_songy, int *p_songoffs, int *p_songlen, void** pp_songlines);
void get_song_playing(int *p_playsong, int *p_songpos);
}

void SongPlainTextEdit::UpdateSongScores(void)
{
	int songx, songy, songoffs, songlen;
	struct songline *p_songs;
	get_song_data(&songx, &songy, &songoffs, &songlen, (void**)&p_songs);


	int i, j;
	char line_buffer[1024];


	for(i = 0; i < songlen; i++) {
		snprintf(line_buffer, sizeof(line_buffer), "%02x: ", i);
		char string_buffer[1024];
		//addstr(buf);

		for(j = 0; j < 4; j++) {

			snprintf(&string_buffer[0], sizeof(string_buffer), "%02x:%02x", p_songs[i].track[j], p_songs[i].transp[j]);
			strncat(&line_buffer[0], &string_buffer[0], 1024);
			//addstr(buf);
			if(j != 3) {
				snprintf(&string_buffer[0], sizeof(string_buffer), " ");
				strncat(&line_buffer[0], &string_buffer[0], 1024);
				//addch(' ');
			}
		}

		//attrset(A_NORMAL);
		//if(playsong && songpos == (i + 1)) addch('*');
#if(0)
		if(is_playsong && songpos == (i + 1)){
			//char string_buffer[1024];
			snprintf(&string_buffer[0], sizeof(string_buffer), "*");
			strncat(&line_buffer[0], &string_buffer[0], 1024);
		}
#endif
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
	//QPlainTextEdit::clear();
	//UpdateSongScores();
	int is_playsong, songpos;
	get_song_playing(&is_playsong, &songpos);

	QList<QTextEdit::ExtraSelection> extraSelections;
	if (!isReadOnly()) {
		 QTextEdit::ExtraSelection selection;

		 QColor lineColor = QColor(128, 128, 128);

		 selection.format.setBackground(lineColor);
		 selection.format.setProperty(QTextFormat::FullWidthSelection, true);
#if(0)
		 QPlainTextEdit
		 QTextCursor playing_text_cursor();
		 playing_text_cursor.set

		 selection.cursor //= SongPlainTextEdit::textCursor();
#endif
		 selection.cursor.clearSelection();

		 extraSelections.append(selection);
	 }

	 setExtraSelections(extraSelections);

}
