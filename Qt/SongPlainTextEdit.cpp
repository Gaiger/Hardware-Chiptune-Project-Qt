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

void SongPlainTextEdit::ShowSongs(void)
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

	if(false == is_generating){
		QPlainTextEdit::document()->clearUndoRedoStacks();
		QPlainTextEdit::document()->setModified(false);
		return ;
	}

	int playing_song_index = generating_song_index - 1;
	if( 0 > playing_song_index || playing_song_index > QPlainTextEdit::document()->blockCount() - 1){
		return ;
	}

	QTextBlock current_song_textblock = QPlainTextEdit::document()->findBlockByNumber(playing_song_index);
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
					QPlainTextEdit::document()->findBlockByNumber(playing_song_index + 1));

		if(viewport_geometry.topLeft().y() < next_line_rect.topLeft().y()
				&& viewport_geometry.bottomRight().y() > next_line_rect.bottomRight().y()){
			break;
		}
#define MIN_NUMBER_OF_TOP_COUNTS_WHILE_SCROLLING			(2)
		int scrolling_value = current_song_textblock.firstLineNumber() - MIN_NUMBER_OF_TOP_COUNTS_WHILE_SCROLLING;
		if(playing_song_index + 1 == QPlainTextEdit::document()->blockCount()){
			scrolling_value = QPlainTextEdit::verticalScrollBar()->maximum();
		}

		QPlainTextEdit::verticalScrollBar()->setValue(scrolling_value);
	}while(0);
}

/**********************************************************************************/

void SongPlainTextEdit::HandleGeneratingTrackStateChanged(bool is_playing, int generating_track_index, int generating_line_index)
{
	Q_UNUSED(generating_track_index);
	Q_UNUSED(generating_line_index);

	QPlainTextEdit::setReadOnly(is_playing);
}

/**********************************************************************************/

int SongPlainTextEdit::ParseDocument(void)
{
	return 0;
}

/**********************************************************************************/

int SongPlainTextEdit::UpdateScores(void)
{
	return ParseDocument();
}
