#include <QTextBlock>
#include <QScrollBar>

#include <QDebug>

#include "TrackPlainTextEdit.h"

TrackPlainTextEdit::TrackPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent)
	:  QPlainTextEdit(parent),
	  m_p_tune_manager(p_tune_manager),
	  m_current_shown_track_index(0)
{
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPixelSize(16);
	QWidget::setFont(font);

	QObject::connect(m_p_tune_manager, &TuneManager::GeneratingSongStateChanged,
					 this, &TrackPlainTextEdit::HandleGeneratingSongStateChanged);
	QObject::connect(m_p_tune_manager, &TuneManager::GeneratingTrackStateChanged,
					 this, &TrackPlainTextEdit::HandleGeneratingTrackStateChanged);
}

/**********************************************************************************/

void TrackPlainTextEdit::ShowTrack(int index)
{
	QPlainTextEdit::clear();
	m_current_shown_track_index = index;

	TuneManager::track *p_tracks;
	int numberf_of_tracks;
	int track_length;
	m_p_tune_manager->GetTracks(&p_tracks, &numberf_of_tracks, &track_length);
	TuneManager::track *p_current_track = &p_tracks[m_current_shown_track_index];
	for(int i = 0; i < track_length; i++){
		QString line_string;
		line_string += QString::asprintf("%02x: ", i);

		uint8_t note = p_current_track->line[i].note;
		if(p_current_track->line[i].note) {
			QString note_string = m_p_tune_manager->GetNoteNameList().at((note - 1) % 12);
			line_string += QString::asprintf("%s%d",
											 note_string.toLatin1().constData(),
											(note - 1) / 12 );
		} else {
			line_string += QString::asprintf("---");
		}

		line_string += QString::asprintf(" %02x",  p_current_track->line[i].instr);
		for(int j = 0; j < 2; j++) {
			if(p_current_track->line[i].cmd[j]) {
				line_string += QString::asprintf( " %c%02x", p_current_track->line[i].cmd[j],
											   p_current_track->line[i].param[j]);
			} else {
				line_string += QString(" ...");
			}
		}

		QPlainTextEdit::blockSignals(true);
		QPlainTextEdit::moveCursor(QTextCursor::End);
		QPlainTextEdit::appendPlainText(line_string);
		QPlainTextEdit::moveCursor(QTextCursor::End);
		QPlainTextEdit::blockSignals(false);
	}

}

/**********************************************************************************/

void TrackPlainTextEdit::HandleGeneratingSongStateChanged(bool is_playing, int generating_song_index)
{
	Q_UNUSED(generating_song_index);
	QPlainTextEdit::setReadOnly(is_playing);
}

/**********************************************************************************/

void TrackPlainTextEdit::HandleGeneratingTrackStateChanged(bool is_playing, int generating_track_index, int generating_line_index)
{
	QPlainTextEdit::setReadOnly(is_playing);
	if(false == is_playing){
		return ;
	}

	if(generating_track_index != m_current_shown_track_index){
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

	if( 0 > generating_line_index || generating_line_index > QPlainTextEdit::document()->blockCount() - 1){
		return ;
	}

	QTextBlock current_song_textblock = QPlainTextEdit::document()->findBlockByLineNumber(generating_line_index);

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
					QPlainTextEdit::document()->findBlockByLineNumber(generating_line_index + 1));

		if(viewport_geometry.topLeft().y() < next_line_rect.topLeft().y()
				&& viewport_geometry.bottomRight().y() > next_line_rect.bottomRight().y()){
			break;
		}

		int scrolling_value = current_song_textblock.firstLineNumber() - 2;
		if(generating_line_index + 1 == QPlainTextEdit::document()->blockCount()){
			scrolling_value = QPlainTextEdit::verticalScrollBar()->maximum();
		}

		QPlainTextEdit::verticalScrollBar()->setValue(scrolling_value);
	}while(0);
}

/**********************************************************************************/
