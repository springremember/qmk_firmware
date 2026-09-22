// Copyright 2024-2026 qk61-vim
// SPDX-License-Identifier: GPL-2.0-or-later
//
// QK61 vim keymap, built on the qmk-vim-fn engine (qmk-vim-fn/engine).
// The engine is QMK-agnostic; the shared qmk/ layer adapts it and owns the
// vim interception chain (vim_pipeline_process).  Only QK61-specific pieces
// live here: the keymap, the physical-key flash, Ctrl+Alt+Del, the Fn+Esc 3s
// reset, the myfn declared table / Fn+Space battery, the platform hook and the
// RGB indicator wiring.
// Design authority: qmk-vim-fn/vim/design.md §4.9/§4.10/§4.12.

#include QMK_KEYBOARD_H
#include "keymap_introspection.h" // keycode_at_keymap_location_raw()
#include "common/rdmctmzt_common.h"
#include "common/user_battery.h"
#include "qmk-vim-fn/engine/include/kv.h"
#include "qmk-vim-fn/qmk/vim_glue.h"
#include "qmk-vim-fn/qmk/vim_keymap_common.h"

// Vendor globals defined in qk61.c, used to yield the RGB indicator layer.
extern bool Key_Fn_Status;
extern bool User_Key_Batt_Num_Show;
extern uint8_t User_Key_Batt_Count;
extern bool Test_Led;

// Vendor MCU reset (qk61 common/user_system.c), used by the Fn+Esc EEPROM reset.
extern void mcu_reset(void);

enum layers {
    _WIN_BASE = 0,
    _MAC_BASE,
    _WIN_FN,
    _MAC_FN,
    _FN = MYFN_LAYER,
};

enum custom_keycodes {
    VIM_MOUSE = QK_KB_22, // right-Alt position: tap = mouse mode, hold = RAlt/RGUI
};

/* Resolve every key from the compiled keymaps, ignoring the VIA dynamic keymap
 * (which QK61 otherwise loads from EEPROM and which shadows keymaps/vim). This
 * is a strong override of the weak keymap_key_to_keycode() in keymap_common.c. */
uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key) {
    if (key.row < MATRIX_ROWS && key.col < MATRIX_COLS) {
        return keycode_at_keymap_location_raw(layer, key.row, key.col);
    }
    return KC_NO;
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_WIN_BASE] = LAYOUT_60_ansi(
        KC_ESC,  KC_1,     KC_2,     KC_3,      KC_4,      KC_5,     KC_6,     KC_7,    KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,
        KC_TAB,  KC_Q,     KC_W,     KC_E,      KC_R,      KC_T,     KC_Y,     KC_U,    KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,
        KC_CAPS, KC_A,     KC_S,     KC_D,      KC_F,      KC_G,     KC_H,     KC_J,    KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,
        KC_LSFT, KC_Z,     KC_X,     KC_C,      KC_V,      KC_B,     KC_N,     KC_M,    KC_COMM,  KC_DOT,             KC_SLSH,            KC_RSFT,
        KC_LCTL, KC_LGUI,  KC_LALT,                        KC_SPC,                                VIM_MOUSE, MO(_FN),  KC_APP,   KC_RCTL
    ),
    [_MAC_BASE] = LAYOUT_60_ansi(
        KC_ESC,  KC_1,     KC_2,     KC_3,      KC_4,      KC_5,     KC_6,     KC_7,    KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,
        KC_TAB,  KC_Q,     KC_W,     KC_E,      KC_R,      KC_T,     KC_Y,     KC_U,    KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,
        KC_CAPS, KC_A,     KC_S,     KC_D,      KC_F,      KC_G,     KC_H,     KC_J,    KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,
        KC_LSFT, KC_Z,     KC_X,     KC_C,      KC_V,      KC_B,     KC_N,     KC_M,    KC_COMM,  KC_DOT,             KC_SLSH,            KC_RSFT,
        KC_LCTL, KC_LALT,  KC_LGUI,                        KC_SPC,                                VIM_MOUSE, MO(_FN),  KC_APP,   KC_RCTL
    ),
    [_WIN_FN] = LAYOUT_60_ansi(
        KC_GRV,  KC_F1,    KC_F2,    KC_F3,     KC_F4,     KC_F5,    KC_F6,    KC_F7,   KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_BSPC,
        KC_TAB,  MD_BLE1,  MD_BLE2,  MD_BLE3,   MD_24G,    KC_PSCR,  KC_SCRL,  KC_PAUS, KC_I,     KC_O,     KC_P,     UG_SPDD,  UG_SPDU,  UG_NEXT,
        KC_CAPS, TO(0),    TO(1),    KC_D,      KC_F,      KC_INS,   KC_HOME,  KC_PGUP, KC_K,     KC_L,     UG_HUED,  UG_HUEU,            KC_ENT,
        KC_LSFT, RM_NEXT,  KC_NO,    RM_SATD,   RM_SATU,   KC_DEL,   KC_END,   KC_PGDN, RM_VALD,  RM_VALU,            KC_UP,              QK_BAT,
        KC_LCTL, QK_WLO,   KC_LALT,                        RM_TOGG,                               KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_NO
    ),
    [_MAC_FN] = LAYOUT_60_ansi(
        KC_GRV,  KC_BRID,  KC_BRIU,  KC_MCTL,   KC_LPAD,   KC_5,     KC_6,     KC_MPRV, KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_BSPC,
        KC_TAB,  MD_BLE1,  MD_BLE2,  MD_BLE3,   MD_24G,    KC_PSCR,  KC_SCRL,  KC_PAUS, KC_I,     KC_O,     KC_P,     UG_SPDD,  UG_SPDU,  UG_NEXT,
        KC_CAPS, TO(0),    TO(1),    KC_D,      KC_F,      KC_INS,   KC_HOME,  KC_PGUP, KC_K,     KC_PGUP,  UG_HUED,  UG_HUEU,            KC_ENT,
        KC_LSFT, KC_NO,    KC_NO,    RM_SATD,   RM_SATU,   KC_DEL,   KC_END,   KC_PGDN, RM_VALD,  RM_VALU,            KC_UP,              QK_BAT,
        KC_LCTL, KC_LALT,  KC_LGUI,                        RM_TOGG,                               KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_NO
    ),
    // 新 Fn 层（myfn 约定，见 qmk-vim-fn/fn/readme.md）：F1..F12 / 音量 / 蓝牙·2.4G 切换。
    // 未声明键由共享 myfn 骨架吞键；Fn+Space 电量、Fn+Caps 开关 Vim 在此处理。
    [_FN] = LAYOUT_60_ansi(
        _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______,
        _______, MD_BLE1, MD_BLE2, MD_BLE3, MD_24G,  _______, _______, _______, _______, _______, _______, KC_VOLD, KC_VOLU, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,          _______,
        _______, _______, _______,                     _______,                              _______, _______, _______,          _______
    )
};
// clang-format on

/* ===== Vim is always on: start in typing (Insert) mode ===== */
void keyboard_post_init_user(void) {
    // NOTE: no dynamic_keymap_reset() here.  keymap_key_to_keycode() below is a
    // strong override that resolves every key from the compiled keymaps, so the
    // EEPROM dynamic keymap is never read anyway.  The old build-id hash +
    // dynamic_keymap_reset() rewrote ~960 bytes through the QK61 emulated-flash
    // driver (per-byte program with interrupts disabled) during the USB
    // enumeration window, which wedged enumeration ("unknown device").
    vim_glue_init();

    rgb_matrix_mode_noeeprom(RGB_MATRIX_CYCLE_OUT_IN_DUAL);
    rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), 106);
    rgb_matrix_set_speed_noeeprom(28);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    Key_Fn_Status = layer_state_cmp(state | default_layer_state, MYFN_LAYER);
    return state;
}

/* ===== QK61-specific cfg callbacks ===== */

static bool qk61_is_mac(void) { return Keyboard_Info.Mac_Win_Mode == INIT_MAC_MODE; }

/* myfn 约定（qmk-vim-fn/fn/readme.md）：声明本键盘能实现的键；未声明键由共享
 * 骨架吞键。声明的无前置键（F 区/音量）以及 Caps/Esc 由骨架放行给后续步骤。 */
static bool qk61_myfn_declared(uint16_t keycode) {
    if (keycode >= KC_F1 && keycode <= KC_F12) return true;
    if (keycode == KC_VOLD || keycode == KC_VOLU) return true;
    if (keycode == KC_SPC || keycode == KC_CAPS || keycode == KC_ESC) return true;
    if (keycode == KC_T) return true; // 有物理开关 → 空跑（吞键）
    return false;
}

/* Fn+Space 电量提示（消费，不打空格）；Fn+T 空跑（消费）；其余已声明键放行。 */
static bool fn_batt_held = false;

static bool qk61_myfn(uint16_t keycode, bool pressed) {
    if (keycode == KC_SPC) {
        if (pressed) {
            fn_batt_held           = true;
            User_Key_Batt_Num_Show = true;
            User_Key_Batt_Count    = 0;
        } else if (fn_batt_held) {
            fn_batt_held           = false;
            User_Key_Batt_Num_Show = false;
            User_Key_Batt_Count    = 0;
        }
        return true; // consume
    }
    if (keycode == KC_T) return true; // Fn+T 空跑：吞键
    return false;                     // F 区/音量/Caps/Esc 放行
}

/* ===== 按键短暂亮灯 ===== */
#define FLASH_MS 200
static const uint8_t flash_led[] = {
    29, 46, 44, 31, 17, 32, 33, 34, 22, 35, 36, 37, 48, 47, 23, 24, 15, 18, 30, 19, 21, 45, 16, 43, 20, 42,
    13, 14, 40, 41, 52, 53, 55, 56, 58, 59, 60};
#define FLASH_LED_COUNT (sizeof(flash_led) / sizeof(flash_led[0]))
static uint16_t flash_time[FLASH_LED_COUNT] = {0};

/* ===== Fn + Esc (physical [0,0]) held >= 3s = reset EEPROM + reboot ===== */
#define RESET_HOLD_MS 3000
static uint16_t reset_timer = 0;
static bool     reset_armed = false;
static bool     reset_fired = false;

/* QK61-specific post-myfn hook (pipeline step 3): flash, Ctrl+Alt+Del and the
 * Fn+Esc EEPROM reset.  Returns true when the key is consumed. */
static bool qk61_hook_post(uint16_t keycode, keyrecord_t *record) {
    // ---- 按键短暂亮灯 ----
    if (record->event.pressed && record->event.key.row < MATRIX_ROWS && record->event.key.col < MATRIX_COLS) {
        uint8_t led = g_led_config.matrix_co[record->event.key.row][record->event.key.col];
        if (led != NO_LED) {
            for (uint8_t i = 0; i < FLASH_LED_COUNT; i++) {
                if (flash_led[i] == led) {
                    uint16_t t    = timer_read();
                    flash_time[i] = t ? t : 1;
                    break;
                }
            }
        }
    }

    // ---- 左Ctrl + 左Alt + 退格 = Ctrl+Alt+Delete ----
    // On press: pair Backspace in the shared table and send Delete; the release
    // is consumed by that table regardless of whether Ctrl/Alt are still held
    // (never re-evaluate get_mods() on key-up -- design §4.10).
    if (keycode == KC_BSPC && record->event.pressed && (get_mods() & MOD_BIT(KC_LCTL)) &&
        (get_mods() & MOD_BIT(KC_LALT))) {
        vim_glue_swallow(KC_BSPC);
        tap_code(KC_DEL);
        return true;
    }

    // ---- Fn+Space 电量：Space 抬起（Fn 已先放开）也要收尾 ----
    if (keycode == KC_SPC && !record->event.pressed && fn_batt_held) {
        fn_batt_held           = false;
        User_Key_Batt_Num_Show = false;
        User_Key_Batt_Count    = 0;
    }

    // ---- Fn + Esc (physical [0,0]) held >= 3s = EEPROM reset ----
    if (record->event.key.row == 0 && record->event.key.col == 0) {
        if (record->event.pressed) {
            if (layer_state_cmp(layer_state | default_layer_state, MYFN_LAYER)) {
                vim_glue_swallow(keycode); // press consumed; release via glue table
                if (!reset_armed) {
                    reset_armed = true;
                    reset_fired = false;
                    reset_timer = vim_timer_start();
                }
                return true; // Fn+Esc: swallow the press (no real Esc)
            }
        } else {
            reset_armed = false; // release is consumed by the glue pairing table
        }
    }
    return false;
}

/* ===== Shared vim configuration ===== */
static const vim_cfg_t g_cfg = {
    .fn_layer         = MYFN_LAYER,
    .trigger_kc       = VIM_MOUSE,
    .mod_win          = KC_RALT,
    .mod_mac          = KC_RGUI,
    .is_mac           = qk61_is_mac,
    .link_ok          = NULL, // QK61 has no mouse link gate
    .hold_ms          = 200,
    .shift_esc_enable = true,
    .led_index        = VIM_LED_INDEX,
    .hook_pre         = NULL,
    .hook_post_myfn   = qk61_hook_post,
    .myfn_declared    = qk61_myfn_declared,
    .myfn             = qk61_myfn,
    .vim_set_enabled  = NULL, // engine default (kv_enable restarts in INSERT)
    .shortcuts        = vim_default_shortcuts,
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return vim_pipeline_process(keycode, record, &g_cfg);
}

void matrix_scan_user(void) {
    if (reset_armed && !reset_fired &&
        layer_state_cmp(layer_state | default_layer_state, MYFN_LAYER) &&
        vim_timer_elapsed(reset_timer, RESET_HOLD_MS)) {
        reset_fired = true;
        reset_armed = false;
        clear_keyboard();
        eeconfig_init();
        mcu_reset();
    }

    vim_keymap_common_task(timer_read());
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    (void)led_min;
    (void)led_max;

    bool special = User_Power_Low || Test_Led;

    if (!special) {
        const bool      enabled = kv_vim_enabled();
        const kv_mode_t m       = kv_get_mode();
        const bool      mouse   = (m == KV_MODE_MOUSE);
        const bool      have    = enabled || mouse || m == KV_MODE_INSERT || m == KV_MODE_NORMAL ||
                              m == KV_MODE_VISUAL || m == KV_MODE_VISUAL_LINE;
        if (have) {
            uint8_t r = 0, g = 0, b = 0;
            vim_rgb_state_color(enabled, m, kv_pending(), mouse, &r, &g, &b);
            rgb_matrix_set_color(vim_rgb_led_index(), r, g, b); // Esc key LED
            uint8_t bat = User_Batt_BaiFen;
            if (bat > 100) bat = 100;
            uint8_t lit = (uint8_t)(((uint16_t)bat * 3 + 99) / 100);
            for (uint8_t i = 0; i < 3; i++) {
                rgb_matrix_set_color(61 + i, i < lit ? r : 0, i < lit ? g : 0, i < lit ? b : 0);
            }
        }
    }

    if (!special) {
        hsv_t hsv = {.h = rgb_matrix_get_hue(), .s = 255, .v = RGB_MATRIX_MAXIMUM_BRIGHTNESS};
        rgb_t rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(54, rgb.r, rgb.g, rgb.b);
    }

    if (!(Key_Fn_Status || Led_Rf_Pair_Flg || User_Power_Low || Test_Led)) {
        uint8_t hue = rgb_matrix_get_hue();
        for (uint8_t i = 0; i < FLASH_LED_COUNT; i++) {
            uint8_t idx = flash_led[i];
            if (flash_time[i]) {
                uint16_t elapsed = timer_elapsed(flash_time[i]);
                if (elapsed < FLASH_MS) {
                    uint8_t v   = (uint8_t)((uint16_t)RGB_MATRIX_MAXIMUM_BRIGHTNESS * (FLASH_MS - elapsed) / FLASH_MS);
                    hsv_t   hsv = {.h = hue, .s = 255, .v = v};
                    rgb_t   rgb = hsv_to_rgb(hsv);
                    rgb_matrix_set_color(idx, rgb.r, rgb.g, rgb.b);
                } else {
                    flash_time[i] = 0;
                    rgb_matrix_set_color(idx, 0, 0, 0);
                }
            } else {
                rgb_matrix_set_color(idx, 0, 0, 0);
            }
        }
    }

    return false;
}
