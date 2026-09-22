// Copyright 2024-2026 nut65-vim
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* ===== custom settings ===== */
// RGB matrix LED index used to indicate the current vim mode (Caps key position).
// Consumed by vim_keymap_common via cfg.led_index (design §4.12: the keyboard
// only supplies the LED position).
#define VIM_LED_INDEX 6
