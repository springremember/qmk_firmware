ENCODER_MAP_ENABLE = yes
VIA_ENABLE = yes

# qmk-vim-fn engine (QMK-agnostic core) + shared QMK adapter/keymap layer
SRC += qmk-vim-fn/engine/src/classify.c
SRC += qmk-vim-fn/engine/src/ctx.c
SRC += qmk-vim-fn/engine/src/emit.c
SRC += qmk-vim-fn/engine/src/command.c
SRC += qmk-vim-fn/engine/src/engine.c
SRC += qmk-vim-fn/qmk/vim_glue.c
SRC += qmk-vim-fn/qmk/vim_keymap_common.c
