/*
	SPDX-License-Identifier: 0BSD

	Zitatespucker-example-blocksds:
	BlocksDS-based Zitatespucker frontend for the Nintendo DS

    Copyright (C) 2024 by Sembo Sadur <labmailssadur@gmail.com>

	Permission to use, copy, modify, and/or distribute this software
	for any purpose with or without fee is hereby granted.

	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
	WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
	OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
	NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/*
	X = 32, Y = 24
*/


/* definitions */
#define FILENAME_JSON	"zitate.json"
#define HELP_RIGHT		"> --> next Zitat"
#define HELP_LEFT		"< --> previous Zitat"
#define HELP_START		"START --> Exit"


/* macros */

/*
	macro to get the position at which to start printing, so that the output is centered
	X is the length of the line to print into, L is the line to be printed
*/
#define centerpos(X, L)		(((X) - (L) - (((X) - (L)) % 2)) / 2)


/* BlocksDS stuff */
#ifndef ARM9
#define ARM9
#endif
#include <nds.h>
#include <filesystem.h>


/* Libraries */
#define ZITATESPUCKER_JSON
#include <Zitatespucker/Zitatespucker.h>


/* static function declarations */
static inline void waitforstart(void);

static inline void waitforstartexit(int retcode);

static void printZitat(ZitatespuckerZitat *ZitatEntry, PrintConsole *curConsole);


int main(int argc, char **argv)
{
	/* 0. Init at least one screen for diagnostic reasons */
	// tbh, this is mostly copied
	PrintConsole topScreen;
	videoSetMode(MODE_0_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	consoleInit(&topScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleSelect(&topScreen);

	
	/* 1. init nitrofs */
	if (!nitroFSInit(NULL)) {
		printf("NitroFS:\nFailed to init!\n");
		waitforstartexit(1);
	} else
		printf("NitroFS:\nInit succeeded.\n\n");
	

	/* 2. Change to Zitatespucker dir */
	if (chdir("nitro:/Zitatespucker") != 0) {
		printf("NitroFS:\nCouldn't chdir into Zitatespucker,\nmissing directory?\n");
		waitforstartexit(2);
	} else
		printf("NitroFS:\nchdir into Zitatespucker.\n\n");
	

	/* 3. Get the Zitat */
	ZitatespuckerZitat *ZitatList = ZitatespuckerJSONGetZitatAllFromFile(FILENAME_JSON);
	if (ZitatList == NULL) {
		printf("Zitatespucker:\nGot a NULL pointer,\nmissing file?\n");
		waitforstartexit(3);
	} else
		printf("Zitatespucker:\nGot a valid pointer.\n\n");
	
	#ifdef DEBUG
	waitforstart();
	#endif
	consoleClear();
	

	/* 4. init the sub display */
	PrintConsole bottomScreen;
	videoSetModeSub(MODE_0_2D);
	vramSetBankC(VRAM_C_SUB_BG);
	consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);


	/* 5. fill sub display with controls */
	// aligning the button infos makes it more appealing, imo
	// also, we center Y, roughly
	consoleSelect(&bottomScreen);
	uint8_t pstart = centerpos(32, strlen(HELP_LEFT));
	printf("\x1b[%d;%dH", 10, pstart);
	printf("%s\n", HELP_RIGHT);
	printf("\x1b[%d;%dH", bottomScreen.cursorY, pstart);
	printf("%s\n", HELP_LEFT);
	printf("\x1b[%d;%dH", bottomScreen.cursorY + 1, pstart);
	printf("%s\n", HELP_START);


	/* 6. fill top display with initial entry */
	consoleSelect(&topScreen);
	printZitat(ZitatList, &topScreen);


	/* 7. enter key-getting loop */
	uint32_t keys;
	ZitatespuckerZitat *cur = ZitatList;
	while (true) {
		swiWaitForVBlank();
		scanKeys();

		keys = keysDown();

		if (keys & KEY_START)
			break;
		else if (keys & KEY_LEFT) {
			if (cur->prevZitat != NULL) {
				cur = cur->prevZitat;
				consoleClear();
				printZitat(cur, &topScreen);
			}
		} else if (keys & KEY_RIGHT) {
			if (cur->nextZitat != NULL) {
				cur = cur->nextZitat;
				consoleClear();
				printZitat(cur, &topScreen);
			}
		}
	}

	/* 8. cleanup */
	ZitatespuckerZitatFree(ZitatList);

	consoleSelect(&bottomScreen);
	consoleClear();

	consoleSelect(&topScreen);
	consoleClear();
}


/* static function definitions */
static inline void waitforstart(void)
{
	do {
		swiWaitForVBlank();
		scanKeys();
	} while (!(keysDown() & KEY_START));
}

static inline void waitforstartexit(int retcode)
{
	waitforstart();
	exit(retcode);
}

static void printZitat(ZitatespuckerZitat *ZitatEntry, PrintConsole *curConsole)
{
	uint8_t pstart;
	
	printf("%s\n\n", ZitatEntry->zitat);
	bool yearvalid = ((ZitatEntry->year != 0 || ZitatEntry->annodomini == true));
	bool commentvalid = (ZitatEntry->comment != NULL);
	bool setcomma = (yearvalid || commentvalid);
	
	pstart = centerpos(32, strlen(ZitatEntry->author) + setcomma);
	if (pstart < 0) { pstart = 0; }
	// Set cursor coordinates: [y;xH
    printf("\x1b[%d;%dH", curConsole->cursorY, pstart);
	printf("%s%s\n", ZitatEntry->author, (setcomma ? "," : ""));

	if (commentvalid) {
		size_t commentlen = strlen(ZitatEntry->comment);
		if (commentlen > 32) {
			char *cur = ZitatEntry->comment;
			do {
				printf("%.32s\n", cur);
				cur += 32;
			} while ((commentlen = strlen(cur)) > 32);
			if (commentlen > 0) {
				pstart = centerpos(32, commentlen);
				printf("\x1b[%d;%dH", curConsole->cursorY, pstart);
				printf("%s", cur);
			}
		} else {
			pstart = centerpos(32, commentlen);
			printf("\x1b[%d;%dH", curConsole->cursorY, pstart);
			printf("%s", ZitatEntry->comment);
		}
	}
	printf("\n\n");

	if (yearvalid) {
		bool monthvalid = false;
		bool dayvalid = false;
		int datelen = 1;
		uint16_t tmp = ZitatEntry->year;

		while ((tmp /= 10) > 0)
			datelen++;

		if (ZitatEntry->month != 0) {
			monthvalid = true;
			datelen += 3;
			if (ZitatEntry->day != 0) {
				dayvalid = true;
				datelen += 3;
			}
		}

		pstart = centerpos(32, datelen);
		printf("\x1b[%d;%dH", curConsole->cursorY, pstart);

		if (dayvalid)
			printf("%s%d.", (ZitatEntry->day > 9 ? "" : "0"), ZitatEntry->day);
		if (monthvalid)
			printf("%s%d.", (ZitatEntry->month > 9 ? "" : "0"), ZitatEntry->month);
		printf("%d\n", ZitatEntry->year);
	}
}
