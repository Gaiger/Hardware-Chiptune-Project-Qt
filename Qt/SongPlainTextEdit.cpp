#include <QTextBlock>
#include <QScrollBar>

#include <QDebug>

#include "SongPlainTextEdit.h"

SongPlainTextEdit::SongPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent)
	: HighlightWholeLinePlainTextEdit(parent),
	  m_p_tune_manager(p_tune_manager),
	  m_previous_textcuror_position(0)
{
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPixelSize(20);
	QWidget::setFont(font);

	//QPlainTextEdit::setCursorWidth(10);

	QObject::connect(m_p_tune_manager, &TuneManager::GeneratingSongStateChanged,
					 this, &SongPlainTextEdit::HandleGeneratingSongStateChanged);
	QObject::connect(m_p_tune_manager, &TuneManager::GeneratingTrackStateChanged,
					 this, &SongPlainTextEdit::HandleGeneratingTrackStateChanged);

	QObject::connect(this, &QPlainTextEdit::cursorPositionChanged,
					 this, &SongPlainTextEdit::HandleCursorPositionChanged);
}

/**********************************************************************************/

void SongPlainTextEdit::HandleCursorPositionChanged(void)
{
	//CorrectCursorPosition();
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

void SongPlainTextEdit::ShowSong(void)
{
	TuneManager::songline *p_songs;
	int number_of_songs;
	m_p_tune_manager->GetSongs(&p_songs, &number_of_songs);

	QString whole_text;
	for(int i = 0; i < number_of_songs; i++) {
		QString line_string;
		line_string += QString::asprintf("%02x: ", i);

		for(int j = 0; j < 4; j++) {
			line_string += QString::asprintf("%02x:%02x", p_songs[i].track[j], p_songs[i].transp[j]);
			if(j != 3) {
				line_string += QString::asprintf(" ");
			}
		}
		whole_text += line_string;
		if(number_of_songs - 1 != i){
			whole_text += "\r\n";
		}
	}

	QPlainTextEdit::blockSignals(true);
	QTextCursor textcursor(QPlainTextEdit::document());
	textcursor.select(QTextCursor::Document);
	textcursor.insertText(whole_text);
	QPlainTextEdit::blockSignals(false);

	QPlainTextEdit::document()->clearUndoRedoStacks();
	QPlainTextEdit::document()->setModified(false);
}

/**********************************************************************************/

void SongPlainTextEdit::HandleGeneratingSongStateChanged(bool is_generating, int generating_song_index)
{
	QPlainTextEdit::setReadOnly(is_generating);
	int highlight_line_index = generating_song_index - 1;
	if(false == is_generating){
		highlight_line_index = -1;
	}
	HighlightWholeLine(highlight_line_index);
}

/**********************************************************************************/

void SongPlainTextEdit::HandleGeneratingTrackStateChanged(bool is_playing, int generating_track_index, int generating_line_index)
{
	Q_UNUSED(generating_track_index);
	Q_UNUSED(generating_line_index);

	QPlainTextEdit::setReadOnly(is_playing);
}

/**********************************************************************************/

int SongPlainTextEdit::ParseDocument(bool is_update_to_memory)
{
	return 0;
}

/**********************************************************************************/

int SongPlainTextEdit::UpdateScores(void)
{
	int ret = ParseDocument(false);
	if(0 != ret){
		return ret;
	}

	ParseDocument(true);
	ShowSong();
	return 0;
}
