// Copyright 2024 qk61-vim
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* ===== qmk-vim feature toggles ===== */
#define VIM_I_TEXT_OBJECTS
#define VIM_A_TEXT_OBJECTS
#define VIM_G_MOTIONS
#define VIM_PASTE_BEFORE
#define VIM_REPLACE
#define VIM_DOT_REPEAT
#define VIM_NUMBERED_JUMPS
#define BETTER_VISUAL_MODE

/* ===== qmk-myfn: 自定义新 Fn 层 ===== */
#define MYFN_LAYER 4
/* 新增第 5 层（索引 4），必须同步提高 VIA 动态键位层数上限，否则触发静态断言 */
#define DYNAMIC_KEYMAP_LAYER_COUNT 5
