## What

This is a frontend for Zitatespucker for the Nintendo DS.
It is just as quick and dirty as its curses-based predecessor.


## Building

You'll need BlocksDS to be correctly installed (I just used the upstream guide)
and the corresponding environment variables to be correclty set ('BLOCKSDS' and 'BLOCKSDSEXT')

Additionally, Zitatespucker and jansson (https://github.com/akheron/jansson) need to be
correctly installed into 'BLOCKSDSEXT' for the build to work.

Drop your Zitatespucker-formatted file named 'zitate.json' into nitrofs/Zitatespucker.

Then, hit 'make'.


## Limitations

There are a lot.

Either I did not comprehend it well, or the NDS console output only understands ASCII.
So, go set up your 'zitate.json' accordingly.

If 'zitat' contains any line longer than 32 characters (DS screen x width),
it will automatically be split across multiple line.
Manually placing "\n" somewhere within a string can be used to control this behavior.

The 'author' string is not expected to be more than 32 characters long (31 if 'comment' exists).

The filename to look up is hardcoded, but you probably expected that (hint: 'zitate.json').


## other notes

The file 'logo.xcf' is for GIMP. It doesn't really serve any purpose, but exists for posterity.

If the zitate.json within NitroFS reaches a size I have yet to determine, funny things happen:
DSi: When launched using TWiLightMenu, the ROM hangs after advancing to a certain entry (in my case number 16) and pressing START (to quit)
3DS: When launched using a DSTT (with YSMenu), the chdir() call to "nitro:/Zitatespucker" fails
3DS: When launched using an R4, the ROM gets stuck on the same entry as on the DSi case, but doesn't hang when pressing START (??????)
3DS: When launched using TWiLightMenu, everything works fine.
melonDS: Everything works fine (both DS and DSi mode; DSi mode even utilizes the firmware of the aforementioned DSi)
DSi: when installed as DSiWare and reading the JSON from SDCard, everything works find.

This problem seems incomprehensible to me, if I am being completely honest.
