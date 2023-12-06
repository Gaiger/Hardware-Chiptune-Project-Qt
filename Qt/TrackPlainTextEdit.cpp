#include <QTextBlock>
#include <QScrollBar>

#include <QDebug>

#include "TrackPlainTextEdit.h"

TrackPlainTextEdit::TrackPlainTextEdit(TuneManager *p_tune_manager, QWidget *parent)
	:  QPlainTextEdit(parent),
	  m_p_tune_manager(p_tune_manager),
	  m_current_shown_track_index(-1)
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
	QList<QString> note_name_list = m_p_tune_manager->GetNoteNameList();

	TuneManager::track *p_tracks;
	int numberf_of_tracks;
	int track_length;
	m_p_tune_manager->GetTracks(&p_tracks, &numberf_of_tracks, &track_length);
	TuneManager::track *p_current_track = &p_tracks[index];
	QString whole_text;
	for(int i = 0; i < track_length; i++){
		QString line_string;
		line_string += QString::asprintf("%02x: ", i);

		uint8_t note = p_current_track->line[i].note;
		if(p_current_track->line[i].note) {
			QString note_string = note_name_list.at((note - 1) % note_name_list.size());
			line_string += QString::asprintf("%s%d",
											 note_string.toLatin1().constData(),
											(note - 1) / note_name_list.size() );
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

		whole_text += line_string;
		if(track_length - 1 != i){
			whole_text += "\n";
		}
	}

	QPlainTextEdit::blockSignals(true);
	QTextCursor textcursor(QPlainTextEdit::document());
	textcursor.select(QTextCursor::Document);
	textcursor.insertText(whole_text);
	QPlainTextEdit::blockSignals(false);

	if(m_current_shown_track_index != index){
		QPlainTextEdit::document()->clearUndoRedoStacks();
	}
	m_current_shown_track_index = index;

	QPlainTextEdit::document()->setModified(false);
}

/**********************************************************************************/

void TrackPlainTextEdit::HandleGeneratingSongStateChanged(bool is_playing, int generating_song_index)
{
	Q_UNUSED(generating_song_index);
	QPlainTextEdit::setReadOnly(is_playing);
}

/**********************************************************************************/

void TrackPlainTextEdit::HandleGeneratingTrackStateChanged(bool is_generating, int generating_track_index, int generating_line_index)
{
	QPlainTextEdit::setReadOnly(is_generating);
	int playing_line_index = generating_line_index - 1;

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

	if(generating_track_index != m_current_shown_track_index){
		return ;
	}

	if( 0 > playing_line_index || playing_line_index > QPlainTextEdit::document()->blockCount() - 1){
		return ;
	}

	QPlainTextEdit::blockSignals(true);
	QTextBlock current_song_textblock = QPlainTextEdit::document()->findBlockByNumber(playing_line_index);
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
					QPlainTextEdit::document()->findBlockByNumber(playing_line_index + 1));

		if(viewport_geometry.topLeft().y() < next_line_rect.topLeft().y()
				&& viewport_geometry.bottomRight().y() > next_line_rect.bottomRight().y()){
			break;
		}

#define MIN_NUMBER_OF_TOP_COUNTS_WHILE_SCROLLING			(2)
		int scrolling_value = current_song_textblock.firstLineNumber() - MIN_NUMBER_OF_TOP_COUNTS_WHILE_SCROLLING;
		if(generating_line_index + 1 == QPlainTextEdit::document()->blockCount()){
			scrolling_value = QPlainTextEdit::verticalScrollBar()->maximum();
		}

		QPlainTextEdit::verticalScrollBar()->setValue(scrolling_value);
	}while(0);
}

/**********************************************************************************/
