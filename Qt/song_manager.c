#ifdef _MSC_VER
	#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include <ncurses.h>
#include <math.h>
//#include <err.h?

#ifndef _STUFF_H_
#define _STUFF_H_
	#if defined TRACKLEN
		#undef TRACKLEN
	#endif
	#include "../stuff.h"
#endif
#include "song_manager.h"


#define SETLO(v,x) v = ((v) & 0xf0) | (x)
#define SETHI(v,x) v = ((v) & 0x0f) | ((x) << 4)

int songx, songy, songoffs, songlen = 1;
int trackx, tracky, trackoffs, tracklen = TRACKLEN;
int instrx, instry, instroffs;
int currtrack = 1, currinstr = 1;
int currtab = 0;
int octave = 4;

char filename[1024];

char *notenames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "H-"};

char *validcmds = "0dfijlmtvw~+=";

struct instrline {
	u8			cmd;
	u8			param;
};

struct instrument {
	int			length;
	struct instrline	line[256];
};

struct songline {
	u8			track[4];
	u8			transp[4];
};

struct instrument instrument[256], iclip;
struct track track[256], tclip;
struct songline song[256];

enum {
	PM_IDLE,
	PM_PLAY,
	PM_EDIT
};

int playmode = PM_IDLE;

void readsong(int pos, int ch, u8 *dest) {
	dest[0] = song[pos].track[ch];
	dest[1] = song[pos].transp[ch];
}

void readtrack(int num, int pos, struct trackline *tl) {
	tl->note = track[num].line[pos].note;
	tl->instr = track[num].line[pos].instr;
	tl->cmd[0] = track[num].line[pos].cmd[0];
	tl->cmd[1] = track[num].line[pos].cmd[1];
	tl->param[0] = track[num].line[pos].param[0];
	tl->param[1] = track[num].line[pos].param[1];
}

void readinstr(int num, int pos, u8 *il) {
	if(pos >= instrument[num].length) {
		il[0] = 0;
		il[1] = 0;
	} else {
		il[0] = instrument[num].line[pos].cmd;
		il[1] = instrument[num].line[pos].param;
	}
}

void savefile(char *fname) {
	FILE *f;
	int i, j;

	f = fopen(fname, "w");
	if(!f) {
		fprintf(stderr, "save error!\n");
		return;
	}

	fprintf(f, "musicchip tune\n");
	fprintf(f, "version 1\n");
	fprintf(f, "\n");
	for(i = 0; i < songlen; i++) {
		fprintf(f, "songline %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i,
			song[i].track[0],
			song[i].transp[0],
			song[i].track[1],
			song[i].transp[1],
			song[i].track[2],
			song[i].transp[2],
			song[i].track[3],
			song[i].transp[3]);
	}
	fprintf(f, "\n");
	for(i = 1; i < 256; i++) {
		for(j = 0; j < tracklen; j++) {
			struct trackline *tl = &track[i].line[j];

			if(tl->note || tl->instr || tl->cmd[0] || tl->cmd[1]) {
				fprintf(f, "trackline %02x %02x %02x %02x %02x %02x %02x %02x\n",
					i,
					j,
					tl->note,
					tl->instr,
					tl->cmd[0],
					tl->param[0],
					tl->cmd[1],
					tl->param[1]);
			}
		}
	}
	fprintf(f, "\n");
	for(i = 1; i < 256; i++) {
		if(instrument[i].length > 1) {
			for(j = 0; j < instrument[i].length; j++) {
				fprintf(f, "instrumentline %02x %02x %02x %02x\n",
					i,
					j,
					instrument[i].line[j].cmd,
					instrument[i].line[j].param);
			}
		}
	}

	fclose(f);
}

void loadfile(char *fname) {
	FILE *f;
	char buf[1024];
	int cmd[3];
	int i1, i2, trk[4], transp[4], param[3], note, instr;
	int i;

	snprintf(filename, sizeof(filename), "%s", fname);

	f = fopen(fname, "r");
	if(!f) {
		return;
	}

	songlen = 1;
	while(!feof(f) && fgets(buf, sizeof(buf), f)) {
		if(9 == sscanf(buf, "songline %x %x %x %x %x %x %x %x %x",
			&i1,
			&trk[0],
			&transp[0],
			&trk[1],
			&transp[1],
			&trk[2],
			&transp[2],
			&trk[3],
			&transp[3])) {

			for(i = 0; i < 4; i++) {
				song[i1].track[i] = trk[i];
				song[i1].transp[i] = transp[i];
			}
			if(songlen <= i1) songlen = i1 + 1;
		} else if(8 == sscanf(buf, "trackline %x %x %x %x %x %x %x %x",
			&i1,
			&i2,
			&note,
			&instr,
			&cmd[0],
			&param[0],
			&cmd[1],
			&param[1])) {

			track[i1].line[i2].note = note;
			track[i1].line[i2].instr = instr;
			for(i = 0; i < 2; i++) {
				track[i1].line[i2].cmd[i] = cmd[i];
				track[i1].line[i2].param[i] = param[i];
			}
		} else if(4 == sscanf(buf, "instrumentline %x %x %x %x",
			&i1,
			&i2,
			&cmd[0],
			&param[0])) {

			instrument[i1].line[i2].cmd = cmd[0];
			instrument[i1].line[i2].param = param[0];
			if(instrument[i1].length <= i2) instrument[i1].length = i2 + 1;
		}
	}

	fclose(f);
}

void get_songlines(void** pp_songlines, int **pp_number_of_songlines)
{
	*pp_songlines = (void**)&song[0];
	*pp_number_of_songlines = &songlen;
	return;
}

bool is_song_playing(int *p_processing_song_index)
{
	if(0 == playsong){
		return false;
	}

	*p_processing_song_index = songpos;
	return true;
}

void get_tracks(void ** pp_track, int *p_track_number, int *p_track_length)
{
	*pp_track = (void**)&track[0];
	*p_track_number = sizeof(track)/sizeof(struct track);
	*p_track_length = tracklen;
}

struct channel {
	u8	tnum;
	s8	transp;
	u8	tnote;
	u8	lastinstr;
	u8	inum;
	u8	iptr;
	u8	iwait;
	u8	inote;
	s8	bendd;
	s16	bend;
	s8	volumed;
	s16	dutyd;
	u8	vdepth;
	u8	vrate;
	u8	vpos;
	s16	inertia;
	u16	slur;
} ;

extern struct channel channel[4];

bool is_track_playing(int *p_playing_track_index, int *p_playing_line_index)
{
	if(0 == playtrack){
		return false;
	}

	*p_playing_track_index = channel[0].tnum;
	*p_playing_line_index = trackpos;
	return true;
}

void get_instruments(void ** pp_instruments, int *p_instrument_number)
{
	*pp_instruments = &instrument[0];
	*p_instrument_number = sizeof(instrument)/sizeof(struct instrument);
}


void optimize() {
	u8 used[256], replace[256];
	int i, j;

	memset(used, 0, sizeof(used));
	for(i = 0; i < songlen; i++) {
		for(j = 0; j < 4; j++) {
			used[song[i].track[j]] = 1;
		}
	}

	j = 1;
	replace[0] = 0;
	for(i = 1; i < 256; i++) {
		if(used[i]) {
			replace[i] = j;
			j++;
		} else {
			replace[i] = 0;
		}
	}

	for(i = 1; i < 256; i++) {
		if(replace[i] && replace[i] != i) {
			memcpy(&track[replace[i]], &track[i], sizeof(struct track));
		}
	}

	for(i = 0; i < songlen; i++) {
		for(j = 0; j < 4; j++) {
			song[i].track[j] = replace[song[i].track[j]];
		}
	}

	for(i = 1; i < 256; i++) {
		u8 last = 255;

		for(j = 0; j < tracklen; j++) {
			if(track[i].line[j].instr) {
				if(track[i].line[j].instr == last) {
					track[i].line[j].instr = 0;
				} else {
					last = track[i].line[j].instr;
				}
			}
		}
	}
}

#if(0)
static FILE *exportfile = 0;
static int exportbits = 0;
static int exportcount = 0;
static int exportseek = 0;

void putbit(int x) {
	if(x) {
		exportbits |= (1 << exportcount);
	}
	exportcount++;
	if(exportcount == 8) {
		if(exportfile) {
			fprintf(exportfile, "\t.byte\t0x%02x\n", exportbits);
		}
		exportseek++;
		exportbits = 0;
		exportcount = 0;
	}
}

void exportchunk(int data, int bits) {
	int i;

	for(i = 0; i < bits; i++) {
		putbit(!!(data & (1 << i)));
	}
}

int alignbyte() {
	if(exportcount) {
		if(exportfile) {
			fprintf(exportfile, "\t.byte\t0x%02x\n", exportbits);
		}
		exportseek++;
		exportbits = 0;
		exportcount = 0;
	}
	if(exportfile) fprintf(exportfile, "\n");
	return exportseek;
}

int packcmd(u8 ch) {
	if(!ch) return 0;
	if(strchr(validcmds, ch)) {
		return strchr(validcmds, ch) - validcmds;
	}
	return 0;
}

void exportdata(FILE *f, int maxtrack, int *resources) {
	int i, j;
	int nres = 0;

	exportfile = f;
	exportbits = 0;
	exportcount = 0;
	exportseek = 0;

	for(i = 0; i < 16 + maxtrack; i++) {
		exportchunk(resources[i], 13);
	}

	resources[nres++] = alignbyte();

	for(i = 0; i < songlen; i++) {
		for(j = 0; j < 4; j++) {
			if(song[i].transp[j]) {
				exportchunk(1, 1);
				exportchunk(song[i].track[j], 6);
				exportchunk(song[i].transp[j], 4);
			} else {
				exportchunk(0, 1);
				exportchunk(song[i].track[j], 6);
			}
		}
	}

	for(i = 1; i < 16; i++) {
		resources[nres++] = alignbyte();

		if(instrument[i].length > 1) {
			for(j = 0; j < instrument[i].length; j++) {
				exportchunk(packcmd(instrument[i].line[j].cmd), 8);
				exportchunk(instrument[i].line[j].param, 8);
			}
		}

		exportchunk(0, 8);
	}

	for(i = 1; i <= maxtrack; i++) {
		resources[nres++] = alignbyte();

		for(j = 0; j < tracklen; j++) {
			u8 cmd = packcmd(track[i].line[j].cmd[0]);

			exportchunk(!!track[i].line[j].note, 1);
			exportchunk(!!track[i].line[j].instr, 1);
			exportchunk(!!cmd, 1);

			if(track[i].line[j].note) {
				exportchunk(track[i].line[j].note, 7);
			}

			if(track[i].line[j].instr) {
				exportchunk(track[i].line[j].instr, 4);
			}

			if(cmd) {
				exportchunk(cmd, 4);
				exportchunk(track[i].line[j].param[0], 8);
			}
		}
	}
}

void export() {
	FILE *f = fopen("exported.s", "w");
	FILE *hf = fopen("exported.h", "w");
	int i, j;
	int maxtrack = 0;
	int resources[256];

	exportfile = 0;
	exportbits = 0;
	exportcount = 0;
	exportseek = 0;

	for(i = 0; i < songlen; i++) {
		for(j = 0; j < 4; j++) {
			if(maxtrack < song[i].track[j]) maxtrack = song[i].track[j];
		}
	}

	fprintf(f, "\t.global\tsongdata\n\n");

	fprintf(hf, "#define MAXTRACK\t0x%02x\n", maxtrack);
	fprintf(hf, "#define SONGLEN\t\t0x%02x\n", songlen);

	fprintf(f, "songdata:\n");

	exportdata(0, maxtrack, resources);

	fprintf(f, "# ");
	for(i = 0; i < 16 + maxtrack; i++) {
		fprintf(f, "%04x ", resources[i]);
	}
	fprintf(f, "\n");

	exportdata(f, maxtrack, resources);

	fclose(f);
	fclose(hf);
}

#endif
