# qmk-vim-fn engine (QMK-agnostic core) + shared QMK adapter/keymap layer
SRC += qmk-vim-fn/engine/src/classify.c
SRC += qmk-vim-fn/engine/src/ctx.c
SRC += qmk-vim-fn/engine/src/emit.c
SRC += qmk-vim-fn/engine/src/command.c
SRC += qmk-vim-fn/engine/src/engine.c
SRC += qmk-vim-fn/qmk/vim_glue.c
SRC += qmk-vim-fn/qmk/vim_keymap_common.c

# 镜像体积敏感：QK61/FS026 上固件 >~0x13F58(81752B) 会导致 USB 枚举失败；
# LTO 将全功能镜像压到 ~72K，彻底避开该问题。
LTO_ENABLE = yes
