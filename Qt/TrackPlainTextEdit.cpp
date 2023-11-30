#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <QTextBlock>
#include <QScrollBar>

#include <QDebug>

#include "TrackPlainTextEdit.h"

TrackPlainTextEdit::TrackPlainTextEdit(QWidget *parent)
	:  QPlainTextEdit(parent),
	  m_current_shown_track_index(0)
{
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPixelSize(16);

	QWidget::setFont(font);
}

/**********************************************************************************/


/**********************************************************************************/

extern "C"
{
#include "../stuff.h"
const char * const notenames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "H-"};

void get_tracks(struct track ** pp_track, int *p_track_number, int *p_track_length);
bool is_track_playing(int *p_playing_track_index, int *p_playing_line_index);
}

/**********************************************************************************/

void TrackPlainTextEdit::UpdateTrack(void)
{
	UpdateShowedTrack(1);
}

/**********************************************************************************/

void TrackPlainTextEdit::UpdateShowedTrack(int i)
{
	m_current_shown_track_index = i;
	QPlainTextEdit::clear();

	struct track *p_track;
	int track_number;
	int track_length;
	get_tracks(&p_track, &track_number, &track_length);
	//qDebug() << "track ";// << Qt::hex << index;
	for(int i = 0; i < track_length; i++){
		char line_buffer[1024];
		char string_buffer[1024];
		snprintf(line_buffer, sizeof(line_buffer), "%02x: ", i);
		if(p_track[m_current_shown_track_index].line[i].note) {
			snprintf(string_buffer, sizeof(string_buffer), "%s%d",
				notenames[(p_track[m_current_shown_track_index].line[i].note - 1) % 12],
				(p_track[m_current_shown_track_index].line[i].note - 1) / 12);
		} else {
			snprintf(string_buffer, sizeof(string_buffer), "---");
		}
		strncat(&line_buffer[0], string_buffer, sizeof(line_buffer));

		snprintf(&string_buffer[0], sizeof(string_buffer), " %02x", p_track[m_current_shown_track_index].line[i].instr);
		strncat(&line_buffer[0], string_buffer, sizeof(line_buffer));
		for(int j = 0; j < 2; j++) {
			if(p_track[m_current_shown_track_index].line[i].cmd[j]) {
				snprintf(&string_buffer[0], sizeof(string_buffer), " %c%02x",
					p_track[m_current_shown_track_index].line[i].cmd[j],
					p_track[m_current_shown_track_index].line[i].param[j]);
			} else {
				snprintf(&string_buffer[0], sizeof(string_buffer), " ...");
			}
			strncat(&line_buffer[0], string_buffer, sizeof(line_buffer));
		}

		QPlainTextEdit::blockSignals(true);
		QPlainTextEdit::moveCursor (QTextCursor::End);
		QPlainTextEdit::appendPlainText(QString(&line_buffer[0]));
		QPlainTextEdit::moveCursor (QTextCursor::End);
		QPlainTextEdit::blockSignals(false);
	}

}

/**********************************************************************************/

void TrackPlainTextEdit::HandlePlayingTrackStateChanged(bool is_playing, int playing_track_index, int playing_line_index)
{
	if(false == is_playing){
		return ;
	}

	if(playing_track_index != m_current_shown_track_index){
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

	if( 0 > playing_line_index || playing_line_index > QPlainTextEdit::document()->blockCount() - 1){
		return ;
	}

	QTextBlock current_song_textblock = QPlainTextEdit::document()->findBlockByLineNumber(playing_line_index);

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
					QPlainTextEdit::document()->findBlockByLineNumber(playing_line_index + 1));

		if(viewport_geometry.topLeft().y() < next_line_rect.topLeft().y()
				&& viewport_geometry.bottomRight().y() > next_line_rect.bottomRight().y()){
			break;
		}

		int scrolling_value = current_song_textblock.firstLineNumber() - 2;
		if(playing_line_index + 1 == QPlainTextEdit::document()->blockCount()){
			scrolling_value = QPlainTextEdit::verticalScrollBar()->maximum();
		}

		QPlainTextEdit::verticalScrollBar()->setValue(scrolling_value);
	}while(0);
}

/**********************************************************************************/
