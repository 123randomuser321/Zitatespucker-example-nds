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
#define HELP_L			"L --> first Zitat"
#define HELP_R			"R --> last Zitat"
#define HELP_START		"START --> Exit"
#define MAX_X			32


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
#ifdef DSIWARE
#include <fat.h>
#else
#include <filesystem.h>
#endif


/* Standard headers */
#include <ctype.h>


/* Libraries */
#define ZITATESPUCKER_JSON
#include <Zitatespucker/Zitatespucker.h>


/* static function declarations */

/*
	Wait until the user presses KEY_START, then return.
*/
static inline void waitforstart(void);

/*
	Wait until the user presses KEY_START, then call exit(retcode).
*/
static inline void waitforstartexit(int retcode);

/*
	print ZitatEntry onto the current Console.
	cursorY is a pointer to the cursorY member of the current Console and is used for positioning.
*/
static void printZitat(ZitatespuckerZitat *ZitatEntry, int *cursorY);

/*
	Get the number of characters (including starting position) until, but not including, the next newline or NULL terminator.
*/
//static size_t strlentilnew(char *str);

/*
	Get the number of characters (including starting position) until, but not including, the first newline or NULL terminator,
	within lim characters.

	If no such character is found, the string is read backwards from lim until a blank space (isblank()) is found.
	If no such space exists, lim is returned.
*/
static size_t strlentilnewlim(char *str, size_t lim);

/*
	Prints str within the current Console, with each line centered.

	cursorY is a pointer to the cursorY member of the current Console and is used for positioning.
*/
static void printCenteredLines(char *str, int *cursorY);


int main(int argc, char **argv)
{
	/* 0. init displays */
	// tbh, this is mostly copied
	PrintConsole topScreen;
	PrintConsole bottomScreen;

	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	consoleInit(&topScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

	consoleSelect(&topScreen);


	/* 0.5 What mode are we in? (purely diagnostic) */
	printf("Mode: %s\n\n", (isDSiMode() ? "DSi (TWL)" : "DS (NTR)"));

	
	#ifdef DSIWARE
	/* 1. init SDCard */
	if (!fatInitDefault()) {
		printf("FAT:\nFailed to init!\n");
		waitforstartexit(1);
	} else
		printf("FAT:\nInit succeeded.\n\n");
	

	/* 2. Chango to Zitatespucker dir */
	if (chdir("sd:/Zitatespucker") != 0) {
		printf("FAT:\nCouldn't chdir into Zitatespucker,\nmissing directory?\n");
		waitforstartexit(2);
	} else
		printf("FAT:\nchdir into Zitatespucker.\n\n");
	#else
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
	#endif
	

	/* 3. Get the Zitat */
	ZitatespuckerZitat *ZitatList = ZitatespuckerJSONGetZitatAllFromFile(FILENAME_JSON);
	if (ZitatList == NULL) {
		printf("Zitatespucker:\nGot a NULL pointer,\nmissing file?\n");
		waitforstartexit(3);
	} else {
		printf("Zitatespucker:\nGot a valid pointer.\nList contains %d entries.\n", ZitatespuckerZitatListLen(ZitatList));
	}
	
	#ifdef DEBUG
	waitforstart();
	#endif
	consoleClear();


	/* 4. fill sub display with controls */
	// aligning the button infos makes it more appealing, imo
	// also, we center Y, roughly
	consoleSelect(&bottomScreen);
	uint8_t pstart = centerpos(MAX_X, strlen(HELP_LEFT));
	printf("\x1b[%d;%dH", 8, pstart);
	printf("%s\n", HELP_RIGHT);
	printf("\x1b[%d;%dH", bottomScreen.cursorY, pstart);
	printf("%s\n", HELP_LEFT);
	printf("\x1b[%d;%dH", bottomScreen.cursorY + 1, pstart);
	printf("%s\n", HELP_L);
	printf("\x1b[%d;%dH", bottomScreen.cursorY, pstart);
	printf("%s\n", HELP_R);
	printf("\x1b[%d;%dH", bottomScreen.cursorY + 1, pstart);
	printf("%s\n", HELP_START);


	/* 5. fill top display with initial entry */
	consoleSelect(&topScreen);
	printZitat(ZitatList, &(topScreen.cursorY));


	/* 6. enter key-getting loop */
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
				printZitat(cur, &(topScreen.cursorY));
			}
		} else if (keys & KEY_RIGHT) {
			if (cur->nextZitat != NULL) {
				cur = cur->nextZitat;
				consoleClear();
				printZitat(cur, &(topScreen.cursorY));
			}
		} else if (keys & KEY_L) {
			while (cur->prevZitat != NULL)
				cur = cur->prevZitat;
			consoleClear();
			printZitat(cur, &(topScreen.cursorY));
		} else if (keys & KEY_R) {
			while (cur->nextZitat != NULL)
				cur = cur->nextZitat;
			consoleClear();
			printZitat(cur, &(topScreen.cursorY));
		}
	}

	/* 7. cleanup */
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

static void printZitat(ZitatespuckerZitat *ZitatEntry, int *cursorY)
{	
	// Set cursor coordinates: [y;xH
	
	uint8_t pstart;

	if (ZitatEntry->zitat != NULL) {
		printCenteredLines(ZitatEntry->zitat, cursorY);
		printf("\n\n");
	}

	bool yearvalid = ((ZitatEntry->year != 0 || ZitatEntry->annodomini == true));
	bool commentvalid = (ZitatEntry->comment != NULL);
	bool setcomma = (yearvalid || commentvalid);
	
	if (ZitatEntry->author != NULL) {
		pstart = centerpos(MAX_X, strlen(ZitatEntry->author) + setcomma);
		if (pstart < 0) { pstart = 0; }
    	printf("\x1b[%d;%dH", *cursorY, pstart);
		printf("%s%s\n", ZitatEntry->author, (setcomma ? "," : ""));
	}

	if (commentvalid) {
		printCenteredLines(ZitatEntry->comment, cursorY);
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

		printf("\x1b[%d;%dH", *cursorY, centerpos(MAX_X, datelen));

		if (dayvalid)
			printf("%s%d.", (ZitatEntry->day > 9 ? "" : "0"), ZitatEntry->day);
		if (monthvalid)
			printf("%s%d.", (ZitatEntry->month > 9 ? "" : "0"), ZitatEntry->month);
		printf("%d\n", ZitatEntry->year);
	}
}

/*
static size_t strlentilnew(char *str)
{
	size_t ret = 0;

	while (*str != '\n' && *str != '\0') {
		str++;
		ret++;
	}

	return ret;
}
*/

static size_t strlentilnewlim(char *str, size_t lim)
{
	size_t ret = 0;

	while (*str != '\n' && *str != '\0' && ret < lim) {
		str++;
		ret++;
	}

	if (*str != '\n' && *str != '\0') {
		while (!isblank(*str) && ret > 0) {
			str--;
			ret--;
		}
		if (ret == 0)
			ret = lim;
	}

	return ret;
}

static void printCenteredLines(char *str, int *cursorY)
{
	char *cur = str;
	size_t tilnew = strlentilnewlim(cur, MAX_X);
	if (tilnew > MAX_X)
		tilnew = MAX_X;

	for (int i = *cursorY; *cur != '\0'; i++) {
		printf("\x1b[%d;%dH", i, centerpos(MAX_X, tilnew));
		fwrite(cur, sizeof(char), tilnew, stdout);
		cur += tilnew;
		if (*cur == '\0')
			break;
		else {
			if (*cur == '\n')
				cur++;
			tilnew = strlentilnewlim(cur, MAX_X);
			if (tilnew > MAX_X)
				tilnew = MAX_X;
		}
	}
}
