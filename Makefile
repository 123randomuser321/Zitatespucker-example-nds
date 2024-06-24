#	SPDX-License-Identifier: 0BSD
#
#	Zitatespucker-example-blocksds:
#	BlocksDS-based Zitatespucker frontend for the Nintendo DS
#
#	Copyright (C) 2024 by Sembo Sadur <labmailssadur@gmail.com>
#	
#	Permission to use, copy, modify, and/or distribute this software
#	for any purpose with or without fee is hereby granted.
#
#	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#	WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
#	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
#	OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
#	NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


BLOCKSDS		?= /opt/blocksds/core
BLOCKSDSEXT		?= /opt/blocksds/external

# User config
NAME		:= zitatespucker_example_blocksds
GAME_TITLE	:= Zitatespucker
GAME_AUTHOR	:= 123randomuser321
GAME_ICON	:= logo.bmp

# Source code paths
NITROFSDIR	:= nitrofs

# defines
ifneq ($(DEBUG),)
	DEBUGDEF = -D DEBUG=1
endif
# no float conversion for printf, we don't use it
DEFINES	:= -D PICOLIBC_LONG_LONG_PRINTF_SCANF $(DEBUGDEF)

# Libraries
LIBS	:= -lnds9 -ljansson -lZitatespucker
LIBDIRS	:= $(BLOCKSDSEXT)/jansson $(BLOCKSDSEXT)/Zitatespucker

include $(BLOCKSDS)/sys/default_makefiles/rom_arm9/Makefile
