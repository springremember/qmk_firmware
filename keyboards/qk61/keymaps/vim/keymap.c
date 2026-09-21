// Copyright 2024-2026 qk61-vim
// SPDX-License-Identifier: GPL-2.0-or-later
//
// QK61 vim keymap, built on the qmk-vim-fn engine (qmk-vim-fn/engine).
// The engine is QMK-agnostic; vim_glue.c adapts it.  Keyboard-layer duties
// (Caps/Esc, mouse mode, Shift+Esc, myfn, RGB) stay here.

#include QMK_KEYBOARD_H
#include "version.h"              // QMK_BUILDDATE (full build timestamp)
#include "dynamic_keymap.h"       // dynamic_keymap_reset()
#include "keymap_introspection.h" // keycode_at_keymap_location_raw()
#include "common/rdmctmzt_common.h"
#include "common/user_battery.h"
#include "qmk-vim-fn/engine/include/kv.h"

// vim_glue.c
void    vim_glue_init(void);
void    vim_glue_task(uint32_t now_ms);
bool    vim_glue_kbd(uint16_t keycode);
bool    vim_glue_key_up(uint16_t keycode);

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
        KC_LCTL, KC_LGUI,  KC_LALT,                        KC_SPC,                                VIM_MOUSE, MO(4),   KC_APP,   KC_RCTL
    ),
    [_MAC_BASE] = LAYOUT_60_ansi(
        KC_ESC,  KC_1,     KC_2,     KC_3,      KC_4,      KC_5,     KC_6,     KC_7,    KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,
        KC_TAB,  KC_Q,     KC_W,     KC_E,      KC_R,      KC_T,     KC_Y,     KC_U,    KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,
        KC_CAPS, KC_A,     KC_S,     KC_D,      KC_F,      KC_G,     KC_H,     KC_J,    KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,
        KC_LSFT, KC_Z,     KC_X,     KC_C,      KC_V,      KC_B,     KC_N,     KC_M,    KC_COMM,  KC_DOT,             KC_SLSH,            KC_RSFT,
        KC_LCTL, KC_LALT,  KC_LGUI,                        KC_SPC,                                VIM_MOUSE, MO(4),   KC_APP,   KC_RCTL
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
    // 未声明键由 process_record_myfn() 吞键；Fn+Space 电量、Fn+Caps 开关 Vim 在此处理。
    [_FN] = LAYOUT_60_ansi(
        _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______,
        _______, MD_BLE1, MD_BLE2, MD_BLE3, MD_24G,  _______, _______, _______, _______, _______, _______, KC_VOLD, KC_VOLU, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,          _______,
        _______, _______, _______,                     _______,                              _______, _______, _______,          _______
    )
};
// clang-format on

/* ===== Force the compiled keymap into the VIA dynamic keymap once per flashed
 * firmware build. ===== */
static uint32_t fw_build_id(void) {
    uint32_t h = 2166136261u;
    for (const char *s = QMK_BUILDDATE; *s; s++) {
        h ^= (uint8_t)*s;
        h *= 16777619u;
    }
    return h;
}

/* ===== Vim is always on: start in typing (Insert) mode ===== */
void keyboard_post_init_user(void) {
    uint32_t stored_id = 0;
    eeconfig_read_user_datablock(&stored_id, 0, sizeof(stored_id));
    if (stored_id != fw_build_id()) {
        dynamic_keymap_reset(); // write the compiled keymap into EEPROM
        uint32_t id = fw_build_id();
        eeconfig_update_user_datablock(&id, 0, sizeof(id));
    }

    vim_glue_init();

    rgb_matrix_mode_noeeprom(RGB_MATRIX_CYCLE_OUT_IN_DUAL);
    rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), 106);
    rgb_matrix_set_speed_noeeprom(28);
}

/* ===== Esc release-swallow flag ===== */
static bool esc_swallow_release = false;

/* ===== Caps tap/hold ===== */
#define CAPS_HOLD_TIME 200
static uint16_t caps_press_timer = 0;
static bool     caps_was_insert  = false;

/* ===== Mouse mode (keyboard layer; engine reports only) ===== */
#define MOUSE_HOLD_TIME 200
static bool     mouse_mode_active = false;
static kv_mode_t mouse_entry_mode = KV_MODE_INSERT;
static uint16_t mouse_press_timer = 0;
static bool     mouse_held        = false;
static bool     mouse_lbtn_held   = false;
static uint16_t mouse_lbtn_timer  = 0;

/* ===== Fn+Esc: press swallowed under Fn, release swallowed too ===== */
static bool fn_esc_swallow = false;
static bool fn_esc_press_swallowed = false; /* press was swallowed by Fn+Esc */

/* ===== Fn + Esc held >= 3s = reset EEPROM + reboot ===== */
#define RESET_HOLD_MS 3000
static uint16_t reset_timer  = 0;
static bool     reset_armed  = false;
static bool     reset_fired  = false;

static inline bool fn_layer_active(void) {
    return layer_state_cmp(layer_state | default_layer_state, MYFN_LAYER);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    Key_Fn_Status = layer_state_cmp(state | default_layer_state, MYFN_LAYER);
    return state;
}

/* ===== myfn 约定（qmk-vim-fn/fn/readme.md）=====
 * 未声明键（含修饰键）一律吞键；Fn+Space 电量、Fn+T 空跑在此处理；
 * F 区/音量由 _FN 层键码直接输出（放行）。修饰键放行给 keymap 其它分支。 */
static bool fn_batt_held = false;

static bool is_defined_myfn_key(uint16_t kc) {
    if (kc >= KC_F1 && kc <= KC_F12) return true;
    if (kc == KC_VOLD || kc == KC_VOLU) return true;
    if (kc == KC_SPC || kc == KC_CAPS || kc == KC_ESC) return true; // 各分支处理
    if (kc == KC_T) return true;                                     // 空跑（吞）
    return false;
}

static bool process_record_myfn(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_SPC) { // 电量
        if (record->event.pressed) {
            if (!fn_layer_active()) return true;
            fn_batt_held           = true;
            User_Key_Batt_Num_Show = true;
            User_Key_Batt_Count    = 0;
            return false;
        }
        if (fn_batt_held) {
            fn_batt_held           = false;
            User_Key_Batt_Num_Show = false;
            User_Key_Batt_Count    = 0;
            return false;
        }
        return true;
    }
    if (fn_layer_active()) {
        if (keycode == KC_T) return false; // 切有线：有物理开关 → 空跑
        // 未声明键（含修饰键）一律吞键（QK61 无原厂 Fn 刷机组合，见迁移计划 §2.2）
        if (!is_defined_myfn_key(keycode)) return false;
    }
    return true;
}

/* ===== 按键短暂亮灯 ===== */
#define FLASH_MS 200
static const uint8_t flash_led[] = {
    29, 46, 44, 31, 17, 32, 33, 34, 22, 35, 36, 37, 48, 47, 23, 24, 15, 18, 30, 19, 21, 45, 16, 43, 20, 42,
    13, 14, 40, 41, 52, 53, 55, 56, 58, 59, 60};
#define FLASH_LED_COUNT (sizeof(flash_led) / sizeof(flash_led[0]))
static uint16_t flash_time[FLASH_LED_COUNT] = {0};

/* ===== keymap-layer press-swallow list =====
 * Keys whose press this layer consumes (Shift+Esc, Normal-mode shortcuts)
 * must have their release consumed too (design §4.10 / readme §9). */
#define SWALLOW_MAX 8
static uint16_t swallow_kc[SWALLOW_MAX];
static int      swallow_n = 0;

static void swallow_add(uint16_t kc) {
    if (swallow_n < SWALLOW_MAX) swallow_kc[swallow_n++] = kc;
}
static bool swallow_take(uint16_t kc) {
    for (int i = 0; i < swallow_n; i++) {
        if (swallow_kc[i] == kc) {
            swallow_kc[i] = swallow_kc[--swallow_n];
            return true;
        }
    }
    return false;
}

/* ===== Shift + Esc combos (only Insert) ===== */
static bool pr_shift_combos(uint16_t keycode, keyrecord_t *record, uint8_t mods) {
    if (kv_get_mode() != KV_MODE_INSERT) return false;
    if (record->event.pressed) {
        if ((keycode == KC_ESC) && (mods & MOD_BIT_LSHIFT) &&
            !(mods & (MOD_BIT_LCTRL | MOD_BIT_RCTRL | MOD_BIT_LALT | MOD_BIT_RALT | MOD_BIT_LGUI | MOD_BIT_RGUI))) {
            uint8_t saved_mods = get_mods();
            clear_mods();
            tap_code16(LSFT(KC_GRV)); // ~
            set_mods(saved_mods);
            swallow_add(keycode);
            return true;
        }
        if ((mods & MOD_BIT_RSHIFT) && (keycode == KC_ESC) &&
            !(mods & (MOD_BIT_LCTRL | MOD_BIT_RCTRL | MOD_BIT_LALT | MOD_BIT_RALT | MOD_BIT_LGUI | MOD_BIT_RGUI))) {
            uint8_t saved_mods = get_mods();
            clear_mods();
            tap_code16(KC_GRV); // Right Shift + Esc = `
            set_mods(saved_mods);
            swallow_add(keycode);
            return true;
        }
    }
    return false;
}

/* ===== Esc: Insert/Normal = real Esc; Visual = exit selection ===== */
static bool pr_esc(uint16_t keycode, keyrecord_t *record, bool vim_on) {
    if (keycode != KC_ESC || !vim_on) return false;
    kv_mode_t m = kv_get_mode();
    if (record->event.pressed) {
        if (m == KV_MODE_VISUAL || m == KV_MODE_VISUAL_LINE) {
            kv_set_mode(KV_MODE_NORMAL);
            esc_swallow_release = true;
            return true;
        }
        if (m == KV_MODE_NORMAL && kv_pending()) {
            kv_cancel();
            esc_swallow_release = true;
            return true;
        }
        // Insert / Normal-idle: real Esc, let it fall through to the engine
        // (which returns PASSTHROUGH) so QMK sends the key.
        esc_swallow_release = false;
        return false;
    }
    if (esc_swallow_release) {
        esc_swallow_release = false;
        return true;
    }
    return false;
}

/* ===== Caps: short = toggle Insert/Normal; long = momentary Normal; Fn+Caps = vim toggle ===== */
static bool pr_caps(uint16_t keycode, keyrecord_t *record, bool vim_on) {
    if (keycode != KC_CAPS) return false;
    if (fn_layer_active()) {
        if (record->event.pressed) {
            if (vim_on) {
                kv_disable();
            } else {
                kv_enable();
                kv_set_mode(KV_MODE_INSERT);
            }
        }
        return true;
    }
    if (!vim_on) return false;
    kv_mode_t m = kv_get_mode();
    if (record->event.pressed) {
        caps_was_insert  = (m != KV_MODE_NORMAL);
        caps_press_timer = timer_read() ? timer_read() : 1;
        kv_set_mode(KV_MODE_NORMAL);
    } else {
        bool held = caps_press_timer && timer_elapsed(caps_press_timer) >= CAPS_HOLD_TIME;
        caps_press_timer = 0;
        if (held) {
            if (caps_was_insert) kv_set_mode(KV_MODE_INSERT);
        } else if (!caps_was_insert) {
            kv_set_mode(KV_MODE_INSERT);
        }
    }
    return true;
}

/* ===== Mouse mode ===== */
static void mouse_enter(void) {
    mouse_entry_mode  = kv_get_mode();
    mouse_mode_active = true;
    kv_set_mode(KV_MODE_MOUSE);
}
/* Remember the keycode actually registered per mouse key, so the release
 * unregisters exactly that one even if Shift changed mid-hold (P0-2). */
static uint16_t mouse_j_reg = KC_NO;
static uint16_t mouse_k_reg = KC_NO;

static void mouse_exit(void) {
    mouse_mode_active = false;
    unregister_code(MS_LEFT);
    unregister_code(MS_DOWN);
    unregister_code(MS_UP);
    unregister_code(MS_RGHT);
    unregister_code(MS_WHLU);
    unregister_code(MS_WHLD);
    unregister_code(MS_BTN2);
    mouse_j_reg = KC_NO;
    mouse_k_reg = KC_NO;
    if (mouse_lbtn_held) {
        unregister_code(MS_BTN1);
        mouse_lbtn_held = false;
    }
    mouse_lbtn_timer = 0;
    kv_set_mode(mouse_entry_mode);
}

static bool pr_mouse_mode(uint16_t keycode, keyrecord_t *record) {
    // VIM_MOUSE: tap toggles mouse mode, hold = RAlt/RGUI.
    if (keycode == VIM_MOUSE) {
        if (record->event.pressed) {
            mouse_press_timer = timer_read() ? timer_read() : 1;
            mouse_held        = false;
        } else {
            if (mouse_held) {
                unregister_code(Keyboard_Info.Mac_Win_Mode == INIT_MAC_MODE ? KC_RGUI : KC_RALT);
            } else if (mouse_press_timer) {
                if (mouse_mode_active) {
                    mouse_exit();
                } else {
                    mouse_enter();
                }
            }
            mouse_press_timer = 0;
            mouse_held        = false;
        }
        return true;
    }
    return false;
}

/* Remember the keycode actually registered per mouse key, so the release
 * unregisters exactly that one even if Shift changed mid-hold (P0-2). */
static bool pr_mouse_keys(uint16_t keycode, keyrecord_t *record) {
    if (!mouse_mode_active) return false;
    // QMK reports the base keycode plus get_mods(); handle Shift here.
    const bool shift = (get_mods() & MOD_MASK_SHIFT) != 0;
    if (record->event.pressed) {
        switch (keycode) {
            case KC_H: register_code(MS_LEFT);  return true;
            case KC_J:
                mouse_j_reg = shift ? MS_WHLD : MS_DOWN;
                register_code(mouse_j_reg);
                return true;
            case KC_K:
                mouse_k_reg = shift ? MS_WHLU : MS_UP;
                register_code(mouse_k_reg);
                return true;
            case KC_L: register_code(MS_RGHT);  return true;
            case KC_SPC:
                mouse_lbtn_timer = timer_read() ? timer_read() : 1;
                return true;
            case KC_ENT: register_code(MS_BTN2); return true;
            default: break;
        }
    } else {
        switch (keycode) {
            case KC_H: unregister_code(MS_LEFT);  return true;
            case KC_J:
                if (mouse_j_reg != KC_NO) unregister_code(mouse_j_reg);
                mouse_j_reg = KC_NO;
                return true;
            case KC_K:
                if (mouse_k_reg != KC_NO) unregister_code(mouse_k_reg);
                mouse_k_reg = KC_NO;
                return true;
            case KC_L: unregister_code(MS_RGHT);  return true;
            case KC_SPC:
                if (mouse_lbtn_held) {
                    unregister_code(MS_BTN1);
                    mouse_lbtn_held = false;
                } else if (mouse_lbtn_timer) {
                    tap_code(MS_BTN1);
                }
                mouse_lbtn_timer = 0;
                return true;
            case KC_ENT: unregister_code(MS_BTN2); return true;
            default: break;
        }
    }
    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    const uint8_t mods   = get_mods();
    const bool    vim_on = kv_vim_enabled();

    // ---- myfn 约定（在 vim / 鼠标处理之前拦截）----
    if (!process_record_myfn(keycode, record)) return false;

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
    if (keycode == KC_BSPC && (mods & MOD_BIT(KC_LCTL)) && (mods & MOD_BIT(KC_LALT))) {
        if (record->event.pressed) tap_code(KC_DEL);
        return false;
    }

    // ---- Fn + Esc (physical [0,0]) held >= 3s = EEPROM reset ----
    if (record->event.key.row == 0 && record->event.key.col == 0) {
        if (record->event.pressed) {
            if (fn_layer_active()) {
                fn_esc_swallow        = true;
                fn_esc_press_swallowed = true;
                if (!reset_armed) {
                    reset_armed = true;
                    reset_fired = false;
                    reset_timer = timer_read() ? timer_read() : 1;
                }
            }
        } else {
            reset_armed = false;
            if (fn_esc_swallow) {
                fn_esc_swallow = false;
                return false;
            }
            // A press that was swallowed while Fn was held must have its
            // release swallowed too, even if Fn was released first.
            if (fn_esc_press_swallowed) {
                fn_esc_press_swallowed = false;
                return false;
            }
        }
    }

    // ---- Mouse-mode key handling ----
    if (pr_mouse_mode(keycode, record)) return false;
    if (pr_mouse_keys(keycode, record)) return false;
    // Any other key exits mouse mode and is re-identified (design §4.9).
    // Modifiers are excluded: Shift+J/K are wheel scrolls, not an exit.
    if (mouse_mode_active && !IS_MODIFIER_KEYCODE(keycode)) {
        if (record->event.pressed) mouse_exit();
        // fall through: the key is handled by the engine/QMK below
    }

    // ---- Right Shift combos ----
    if (pr_shift_combos(keycode, record, mods)) return false;

    // ---- Esc / Caps ----
    if (pr_esc(keycode, record, vim_on)) return false;
    if (pr_caps(keycode, record, vim_on)) return false;

    // ---- Keyboard-layer Normal-mode shortcuts (design/readme §9) ----
    // QMK reports the base keycode + get_mods(); match on base + held mods,
    // strip the physical modifiers, then send the plain key (design §2.1).
    if (vim_on && kv_get_mode() == KV_MODE_NORMAL && record->event.pressed) {
        bool is_bspc = (keycode == KC_BSPC);
        bool is_spc  = (keycode == KC_SPC);
        bool is_mins = (keycode == KC_MINS);
        bool is_eql  = (keycode == KC_EQL)  && (mods & MOD_MASK_SHIFT);
        bool is_slsh = (keycode == KC_SLSH);
        bool is_cf   = (keycode == KC_F)    && (mods & MOD_MASK_CTRL);
        bool is_cb   = (keycode == KC_B)    && (mods & MOD_MASK_CTRL);
        if (is_bspc || is_spc || is_mins || is_eql || is_slsh || is_cf || is_cb) {
            kv_cancel(); // clear any half-typed command first
            uint8_t saved = get_mods();
            clear_mods();
            if (is_bspc)      tap_code(KC_LEFT);
            else if (is_spc)  tap_code(KC_RGHT);
            else if (is_mins) { tap_code(KC_UP); tap_code(KC_HOME); }
            else if (is_eql)  { tap_code(KC_DOWN); tap_code(KC_HOME); }
            else if (is_slsh) tap_code16(LCTL(KC_F));
            else if (is_cf)   tap_code(KC_PGDN);
            else if (is_cb)   tap_code(KC_PGUP);
            set_mods(saved);
            swallow_add(keycode); // consume the matching release too (§9)
            return false;
        }
    }

    // ---- Engine: feed key-down; pass-through keys fall back to QMK.
    //      QMK: return true = normal handling, false = consumed.
    //      key-up passes through unless the press was consumed. ----
    if (record->event.pressed) {
        return !vim_glue_kbd(keycode);
    }
    if (swallow_take(keycode)) return false; // keymap-layer press was swallowed
    return !vim_glue_key_up(keycode);
}

void matrix_scan_user(void) {
    if (reset_armed && !reset_fired && fn_layer_active() && timer_elapsed(reset_timer) >= RESET_HOLD_MS) {
        reset_fired = true;
        reset_armed = false;
        clear_keyboard();
        eeconfig_init();
        mcu_reset();
    }

    // Mouse-mode: hold detection for VIM_MOUSE (RAlt) and Space (drag)
    if (mouse_press_timer && !mouse_held && timer_elapsed(mouse_press_timer) >= MOUSE_HOLD_TIME) {
        mouse_held = true;
        register_code(Keyboard_Info.Mac_Win_Mode == INIT_MAC_MODE ? KC_RGUI : KC_RALT);
    }
    if (mouse_mode_active && mouse_lbtn_timer && !mouse_lbtn_held &&
        timer_elapsed(mouse_lbtn_timer) >= MOUSE_HOLD_TIME) {
        mouse_lbtn_held = true;
        register_code(MS_BTN1);
    }

    vim_glue_task(timer_read());
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    (void)led_min;
    (void)led_max;

    bool special = User_Power_Low || Test_Led;

    if (!special) {
        uint8_t r = 0, g = 0, b = 0;
        bool    have = true;
        if (!kv_vim_enabled()) {
            r = 0xFF; g = 0x00; b = 0x00; // red: vim off
        } else if (mouse_mode_active) {
            r = 0x00; g = 0xFF; b = 0xFF; // cyan: mouse mode
        } else {
            switch (kv_get_mode()) {
                case KV_MODE_NORMAL:
                    if (kv_pending()) { r = 0xFF; g = 0xFF; b = 0x00; } // yellow pending
                    else              { r = 0x00; g = 0x00; b = 0xFF; } // blue
                    break;
                case KV_MODE_INSERT:      r = 0x00; g = 0xFF; b = 0x00; break;
                case KV_MODE_VISUAL:
                case KV_MODE_VISUAL_LINE: r = 0x80; g = 0x00; b = 0x80; break;
                default: have = false; break;
            }
        }
        if (have) {
            rgb_matrix_set_color(0, r, g, b); // Esc key LED
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
