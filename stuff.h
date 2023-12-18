#define TRACKLEN 32

#include <stdint.h>

struct trackline {
	uint8_t	note;
	uint8_t	instr;
	uint8_t	cmd[2];
	uint8_t	param[2];
	};

struct track {
	struct trackline	line[TRACKLEN];
};


uint8_t interrupthandler();

void silence();
void iedplonk(int, int);

void start_generating_song(int);
void start_generating_track(int);
void loadfile(char *);
