# keyboards/qk61/keymaps/vim/test/Makefile
#
# Keyboard-local host test for the qk61 vim keymap.  It compiles the shared
# qmk-vim-fn layer (via the keymap's qmk-vim-fn submodule) against the generic
# QMK stub, then links this keymap's own test.  Nothing here is part of the
# firmware build or of qmk-vim-fn itself.
#
#   cd keyboards/qk61/keymaps/vim/test && make
CC       ?= cc
CFLAGS   ?= -std=c11 -O2 -Wall -Wextra
# The shared layer includes headers as "qmk-vim-fn/...", so -I must point at the
# keymaps/vim directory (this test dir's parent).
QMF       = ../qmk-vim-fn
ROOT      = ..
INCLUDES  = -I$(QMF)/engine/include -I$(QMF)/engine/src -I$(QMF)/engine/test/glue -I$(ROOT)
DEFS      = -DQMK_KEYBOARD_H='"qmk_stub.h"'
SRC       = $(QMF)/engine/src/classify.c $(QMF)/engine/src/ctx.c $(QMF)/engine/src/emit.c \
            $(QMF)/engine/src/command.c $(QMF)/engine/src/engine.c \
            $(QMF)/qmk/vim_glue.c $(QMF)/qmk/vim_keymap_common.c

.PHONY: all clean
all: test_qk61
	./test_qk61

test_qk61: test_qk61.c $(SRC)
	$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -o $@ test_qk61.c $(SRC)

clean:
	rm -f test_qk61
