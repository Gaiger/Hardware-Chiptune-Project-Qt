#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <QtGlobal>
#if defined( Q_OS_WIN )
#include <windows.h>
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <QApplication>
#include "TuneManager.h"
#include "AudioPlayer.h"

#include "HardwareChiptunePanelWidget.h"

int main(int argc, char *argv[])
{
#if defined( Q_OS_WIN )
	if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()){
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		freopen("CONIN$", "r", stdin);
	}
	setvbuf(stdout, NULL, _IONBF, 0);
#endif
	QApplication a(argc, argv);

	AudioPlayer player(&a);
	//player.LoadFile("../test2.song");

	HardwareChiptunePanelWidget hardware_chiptune_panel_widget(&player);
	hardware_chiptune_panel_widget.show();
	hardware_chiptune_panel_widget.setFocus();
	return a.exec();
}
