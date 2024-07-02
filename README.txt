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
