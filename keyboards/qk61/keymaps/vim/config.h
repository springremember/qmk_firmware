// Copyright 2024-2026 qk61-vim
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* ===== qmk-myfn: 自定义新 Fn 层 ===== */
#define MYFN_LAYER 4
/* 新增第 5 层（索引 4），必须同步提高 VIA 动态键位层数上限，否则触发静态断言 */
#define DYNAMIC_KEYMAP_LAYER_COUNT 5

/* ===== custom settings ===== */
// RGB matrix LED index used to indicate the current vim mode (Esc key position)
#define VIM_LED_INDEX 0
