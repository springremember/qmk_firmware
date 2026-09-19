// Copyright 2024 qk61-vim
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "version.h"              // QMK_BUILDDATE (full build timestamp)
#include "dynamic_keymap.h"       // dynamic_keymap_reset()
#include "keymap_introspection.h" // keycode_at_keymap_location_raw()
#include "qmk-vim/src/vim.h"
#include "qmk-vim/src/modes.h"
#include "qmk-vim/src/process_func.h"
#include "common/rdmctmzt_common.h"
#include "common/user_battery.h"

// Vendor globals defined in qk61.c, used to yield the RGB indicator layer.
extern bool Key_Fn_Status;
extern bool User_Key_Batt_Num_Show;
extern bool Test_Led;

// qmk-vim current keycode processor (swapped to drive the replace-mode handler)
extern process_func_t process_func;

// Vendor MCU reset (qk61 common/user_system.c), used by the Fn+Esc EEPROM reset.
extern void mcu_reset(void);

#ifdef VIM_DOT_REPEAT
extern void add_repeat_keycode(uint16_t keycode);
#endif

enum layers {
    _WIN_BASE = 0,
    _MAC_BASE,
    _WIN_FN,
    _MAC_FN,
};

enum custom_keycodes {
    MENU_TAP_RIGHT = SAFE_RANGE,
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
        KC_LSFT, KC_Z,     KC_X,     KC_C,      KC_V,      KC_B,     KC_N,     KC_M,    KC_COMM,  KC_DOT,             KC_SLSH,            MT(MOD_RSFT, KC_UP),
        KC_LCTL, KC_LGUI,  KC_LALT,                        KC_SPC,                                MO(2),    MT(MOD_RALT, KC_LEFT), MT(MOD_RCTL, KC_DOWN), MENU_TAP_RIGHT
    ),
    [_MAC_BASE] = LAYOUT_60_ansi(
        KC_ESC,  KC_1,     KC_2,     KC_3,      KC_4,      KC_5,     KC_6,     KC_7,    KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,
        KC_TAB,  KC_Q,     KC_W,     KC_E,      KC_R,      KC_T,     KC_Y,     KC_U,    KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,
        KC_CAPS, KC_A,     KC_S,     KC_D,      KC_F,      KC_G,     KC_H,     KC_J,    KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,
        KC_LSFT, KC_Z,     KC_X,     KC_C,      KC_V,      KC_B,     KC_N,     KC_M,    KC_COMM,  KC_DOT,             KC_SLSH,            MT(MOD_RSFT, KC_UP),
        KC_LCTL, KC_LALT,  KC_LGUI,                        KC_SPC,                                MO(3),    MT(MOD_RGUI, KC_LEFT), MT(MOD_RCTL, KC_DOWN), MENU_TAP_RIGHT
    ),
    [_WIN_FN] = LAYOUT_60_ansi(
        KC_GRV,  KC_F1,    KC_F2,    KC_F3,     KC_F4,     KC_F5,    KC_F6,    KC_F7,   KC_F8,    KC_F9,    KC_F10,   KC_VOLD,  KC_VOLU,  KC_BSPC,
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
    )
};
// clang-format on

/* ===== Force the compiled keymap into the VIA dynamic keymap once per flashed
 * firmware build.  VIA's own EEPROM magic only uses the year/month/day part of
 * QMK_BUILDDATE, so several builds on the same day do NOT reset the dynamic
 * keymap (which shadows keymaps/vim).  Hashing the full timestamp (including
 * time) makes every new build refresh the keymap on its first boot. ===== */
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

    enable_vim_mode();
    insert_mode();

    rgb_matrix_mode_noeeprom(RGB_MATRIX_CYCLE_OUT_IN_DUAL);
    rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), 120); // 60% of 200
    rgb_matrix_set_speed_noeeprom(28);                                           // ~11% of 255
}

/* ===== Esc release-swallow flag =====
 * Set when a mode exit already happened on Esc's press (visual exit handed to
 * qmk-vim, or leaving replace mode) so the matching release is swallowed and
 * does not emit a second, spurious real Esc. */
static bool esc_swallow_release = false;

/* ===== Esc tap/hold: short tap = real Esc, long press = Normal mode ===== */
#define ESC_HOLD_TIME 200
static uint16_t esc_press_timer = 0;

/* ===== Caps tap/hold: long hold = momentary Normal (release restores typing),
 * short tap = toggle Normal mode ===== */
#define CAPS_HOLD_TIME 200
static uint16_t caps_press_timer = 0;
static bool     caps_was_insert  = false;

/* ===== Normal mode Space: short = left click, long = hold left (drag) ===== */
#define SPC_HOLD_TIME 200
static uint16_t spc_press_timer = 0;
static bool     spc_holding     = false;

/* ===== Replace mode (vim R: overwrite chars until Esc) ===== */
static bool replace_active = false;

/* ===== Menu key (MENU_TAP_RIGHT) =====
 * Normal mouse context: pointer right (hold = mousekey acceleration).
 * Other modes:           tap = Right arrow, long press = KC_APP (Menu key). */
static uint16_t menu_timer   = 0;
static bool     menu_pressed = false;
static bool     menu_held    = false;

/* ===== Fn + Esc held >= 3s = reset EEPROM (eeconfig_init) + reboot ===== */
#define RESET_HOLD_MS 3000
static uint16_t reset_timer  = 0;
static bool     reset_armed  = false;
static bool     reset_fired  = false;

static inline bool fn_layer_active(void) {
    return IS_LAYER_ON(_WIN_FN) || IS_LAYER_ON(_MAC_FN);
}

/* ===== 26 letters: key-triggered brief flash =====
 * LED index per keycode KC_A..KC_Z, derived from g_led_config.matrix_co and
 * LAYOUT_60_ansi. 0 = inactive. */
#define LETTER_FLASH_MS 200
static const uint8_t letter_led[26] = {
    29, 46, 44, 31, 17, 32, 33, 34, 22, 35, 36, 37, 48, 47, 23, 24, 15, 18, 30, 19, 21, 45, 16, 43, 20, 42};
static uint16_t letter_flash[26] = {0};

static bool process_replace_mode(uint16_t keycode, const keyrecord_t *record) {
    if (record->event.pressed) {
        uint16_t base = keycode;
        if (keycode >= QK_MODS && keycode <= QK_MODS_MAX) {
            // only a pure Shift modifier is printable here, let others through
            if ((keycode & 0xFF00) != QK_LSFT) {
                return true;
            }
            base = keycode & 0xFF;
        }
        // printable + tab: A-Z, 1-0, space, punctuation row (vim R overwrites tab too)
        if ((base >= KC_A && base <= KC_Z) || (base >= KC_1 && base <= KC_0) || base == KC_SPC || base == KC_TAB || (base >= KC_MINS && base <= KC_SLSH)) {
            tap_code(KC_DELETE); // overwrite char under cursor
            tap_code16(keycode); // type the (possibly shifted) char, cursor advances
            return false;
        }
    }
    return true;
}

// vim R == replace mode until Esc/Caps (normal_mode clears it)
static void enter_replace_mode(void) {
    replace_active = true;
    process_func   = process_replace_mode;
}

// Called by qmk-vim whenever Normal mode is entered, keeps state in sync.
// Strong symbol so it overrides qmk-vim's weak default.
void normal_mode_user(void) {
    replace_active = false;
#ifdef VIM_NUMBERED_JUMPS
    { // consume any stale count so a leftover "3" cannot turn the next
      // dd/cc/yy into a multi-line operation
        extern int16_t motion_counter;
        motion_counter = 0;
    }
#endif
}

// Normal mode bindings
bool process_normal_mode_user(uint16_t keycode, const keyrecord_t *record) {
    if (record->event.pressed) {
        switch (keycode) {
            case KC_BSPC:
                tap_code(KC_LEFT);
                return false;
            case KC_SPC:
                tap_code(KC_RIGHT);
                return false;
            case KC_MINS:
                tap_code(KC_UP);
                tap_code(KC_HOME);
                return false;
            case LSFT(KC_EQL):
                tap_code(KC_DOWN);
                tap_code(KC_HOME);
                return false;
            case LCTL(KC_F):
                tap_code(KC_PGDN);
                return false;
            case LCTL(KC_B):
                tap_code(KC_PGUP);
                return false;
            case KC_SLSH:
            case LSFT(KC_SLSH):
                tap_code16(LCTL(KC_F));
                return false;
            case KC_ENT:
                tap_code(KC_ENT); // real Enter in Normal mode
                return false;
            case KC_TAB:
                tap_code(KC_TAB); // real Tab passes through in Normal mode
                return false;
            case LSFT(KC_J):
                tap_code(KC_END); // join next line onto this one
                tap_code(KC_DELETE);
                return false;
            case LSFT(KC_R):
                enter_replace_mode();
                return false;
            case LSFT(KC_G):
                // G (or nG) -> bottom; 1G == gg -> very top, like vim.
                {
                    extern int16_t motion_counter;
                    if (motion_counter == 1) {
                        tap_code16(LCTL(KC_HOME));
                    } else {
                        tap_code16(LCTL(KC_END));
                    }
                    motion_counter = 0;
                }
                return false;
            default:
                break;
        }
    }
    return true;
}

// Right Shift combos: Right Shift + Esc = grave (add left Shift for ~),
// Right Shift + 1..0/-/= = F1..F12. Any other key keeps normal right-shift
// behaviour. Always mask with the 8-bit MOD_BIT_* constants.
static bool pr_shift_combos(uint16_t keycode, keyrecord_t *record, uint8_t mods) {
    if ((keycode == KC_ESC) && (mods & MOD_BIT_LSHIFT) &&
        !(mods & (MOD_BIT_LCTRL | MOD_BIT_RCTRL | MOD_BIT_LALT | MOD_BIT_RALT | MOD_BIT_LGUI | MOD_BIT_RGUI))) {
        if (record->event.pressed) {
            uint8_t saved_mods = get_mods();
            clear_mods();
            tap_code16(LSFT(KC_GRV)); // ~
            set_mods(saved_mods);
        }
        return true;
    }
    if ((mods & MOD_BIT_RSHIFT) &&
        (keycode == KC_ESC || (keycode >= KC_1 && keycode <= KC_0) || keycode == KC_MINS || keycode == KC_EQL)) {
        if (record->event.pressed) {
            uint16_t repl;
            if (keycode == KC_ESC) {
                repl = KC_GRV;
            } else if (keycode == KC_MINS) {
                repl = KC_F11;
            } else if (keycode == KC_EQL) {
                repl = KC_F12;
            } else {
                uint8_t num = (keycode == KC_0) ? 10 : (keycode - KC_1 + 1);
                repl        = KC_F1 + num - 1;
            }
            uint8_t saved_mods = get_mods();
            clear_mods();
            tap_code16(repl);
            set_mods(saved_mods);
        }
        return true;
    }
    return false;
}

// Esc: short tap sends a real Esc; long press switches to Normal mode.
static bool pr_esc(uint16_t keycode, keyrecord_t *record, bool vim_on, uint8_t vmode) {
    if (keycode != KC_ESC || !vim_on) return false;

    if (record->event.pressed) {
        if (vmode == VISUAL_MODE || vmode == VISUAL_LINE_MODE) {
            esc_swallow_release = true;
        } else if (replace_active) {
            normal_mode();
            esc_swallow_release = true;
        } else {
            if (vmode != INSERT_MODE) {
                normal_mode();
            }
            esc_swallow_release = false;
            esc_press_timer     = timer_read();
        }
    } else {
        if (esc_swallow_release) {
            esc_swallow_release = false;
        } else if (esc_press_timer && timer_elapsed(esc_press_timer) >= ESC_HOLD_TIME) {
            esc_press_timer = 0;
            normal_mode();
        } else {
            esc_press_timer = 0;
            tap_code(KC_ESC);
        }
    }
    return true;
}

// Caps: long hold = momentary Normal, short tap = toggle Normal/Insert.
// Fn+Caps toggles vim; vim off lets Caps act as Caps Lock.
static bool pr_caps(uint16_t keycode, keyrecord_t *record, bool vim_on, uint8_t vmode) {
    if (keycode != KC_CAPS) return false;

    bool fn_active = IS_LAYER_ON(_WIN_FN) || IS_LAYER_ON(_MAC_FN);
    if (fn_active) {
        if (record->event.pressed) {
            toggle_vim_mode();
            insert_mode();
        }
        return true;
    }
    if (!vim_on) {
        return false;
    }
    if (record->event.pressed) {
        caps_was_insert  = (vmode != NORMAL_MODE);
        caps_press_timer = timer_read();
        normal_mode();
#ifdef VIM_DOT_REPEAT
        add_repeat_keycode(KC_NO);
#endif
    } else {
        bool held = caps_press_timer && timer_elapsed(caps_press_timer) >= CAPS_HOLD_TIME;
        caps_press_timer = 0;
        if (held) {
            if (caps_was_insert) insert_mode();
        } else if (!caps_was_insert) {
            insert_mode();
        }
    }
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    const uint8_t mods   = get_mods();
    const bool    vim_on = vim_mode_enabled();
    const uint8_t vmode  = get_vim_mode();
    // Mouse context: Normal mode with vim on, not replace-typing, no modifiers.
    const bool    mouse_ctx = vim_on && vmode == NORMAL_MODE && !replace_active && mods == 0;

    // ---- 26 letters: record a brief flash on any press ----
    if (record->event.pressed && keycode >= KC_A && keycode <= KC_Z) {
        uint16_t t = timer_read();
        letter_flash[keycode - KC_A] = t ? t : 1;
    }

    // ---- Fn + Esc (physical [0,0]) held >= 3s = EEPROM reset (see matrix_scan_user).
    //      Both press and release are swallowed, so Fn+Esc emits nothing. ----
    if (fn_layer_active() && record->event.key.row == 0 && record->event.key.col == 0) {
        if (record->event.pressed) {
            if (!reset_armed) {
                reset_armed = true;
                reset_fired = false;
                reset_timer = timer_read();
            }
        } else {
            reset_armed = false;
        }
        return false;
    }

    // ---- Menu key: Normal = pointer right; other modes = tap Right / hold Menu ----
    if (keycode == MENU_TAP_RIGHT) {
        if (mouse_ctx) {
            if (record->event.pressed) {
                register_code(MS_RGHT);
            } else {
                unregister_code(MS_RGHT);
            }
        } else {
            if (record->event.pressed) {
                menu_pressed = true;
                menu_held    = false;
                menu_timer   = timer_read();
            } else {
                menu_pressed = false;
                if (menu_held) {
                    menu_held = false; // KC_APP already fired in matrix_scan_user
                } else {
                    tap_code(KC_RGHT);
                }
            }
        }
        return false;
    }

    // ---- Normal-mode mouse movement: the bottom-row direction keys.
    //      While held they drive the pointer (mousekey acceleration); the
    //      tap-hold mod/layer of these keys is overridden in this context. ----
    if (mouse_ctx) {
        uint8_t ms = 0;
        if (keycode == MT(MOD_RALT, KC_LEFT) || keycode == MT(MOD_RGUI, KC_LEFT)) {
            ms = MS_LEFT;
        } else if (keycode == MT(MOD_RCTL, KC_DOWN)) {
            ms = MS_DOWN;
        } else if (keycode == MT(MOD_RSFT, KC_UP)) {
            ms = MS_UP;
        }
        if (ms) {
            if (record->event.pressed) {
                register_code(ms);
            } else {
                unregister_code(ms);
            }
            return false;
        }
    }

    // ---- Space mouse leak-guard: release outside the mouse context while
    // dragging/still pending must not leave a stuck left button ----
    if (!record->event.pressed && !mouse_ctx && (spc_holding || spc_press_timer)) {
        unregister_code(MS_BTN1);
        spc_holding     = false;
        spc_press_timer = 0;
    }

    // ---- Right Shift combos ----
    if (pr_shift_combos(keycode, record, mods)) return false;

    // ---- Esc / Caps ----
    if (pr_esc(keycode, record, vim_on, vmode)) return false;
    if (pr_caps(keycode, record, vim_on, vmode)) return false;

    // ---- Space in Normal mode: left click / drag ----
    if (mouse_ctx && keycode == KC_SPC) {
        if (record->event.pressed) {
            spc_press_timer = timer_read();
        } else {
            if (spc_holding) {
                unregister_code(MS_BTN1);
                spc_holding = false;
            } else if (spc_press_timer) {
                tap_code(MS_BTN1);
            }
            spc_press_timer = 0;
        }
        return false;
    }

    // ---- Alt+Tab: pass Tab through with the held Alt so the task switcher
    // stays open; vim Normal mode would tap LALT(Tab), releasing Alt. ----
    if (keycode == KC_TAB && (mods & MOD_MASK_ALT)) {
        return true;
    }

    if (!process_vim_mode(keycode, record)) {
        return false;
    }

    // Insert mode: Right Ctrl + digit -> F1..F10 (Left Ctrl is left alone).
    if (vmode == INSERT_MODE && keycode >= KC_1 && keycode <= KC_0 && (mods & MOD_BIT(KC_RCTL)) && record->event.pressed) {
        uint8_t num = (keycode == KC_0) ? 10 : (keycode - KC_1 + 1);
        tap_code(KC_F1 + num - 1);
        return false;
    }

    return true;
}

void matrix_scan_user(void) {
    // Fn + Esc held >= 3s: full EEPROM reset (compiled keymap gets reloaded
    // via eeconfig_init() -> eeconfig_init_via() -> dynamic_keymap_reset()),
    // then reboot.
    if (reset_armed && !reset_fired && timer_elapsed(reset_timer) >= RESET_HOLD_MS) {
        reset_fired = true;
        reset_armed = false;
        clear_keyboard();
        eeconfig_init();
        mcu_reset();
    }

    // Menu long press (non-mouse context): Menu/application key.
    if (menu_pressed && !menu_held && timer_elapsed(menu_timer) >= TAPPING_TERM) {
        menu_held = true;
        tap_code(KC_APP);
    }

    // Space long press (Normal mode): hold the left button (drag) until release.
    if (spc_press_timer && !spc_holding && timer_elapsed(spc_press_timer) >= SPC_HOLD_TIME && vim_mode_enabled() && get_vim_mode() == NORMAL_MODE && !replace_active &&
        get_mods() == 0) {
        register_code(MS_BTN1);
        spc_holding = true;
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    (void)led_min;
    (void)led_max;

    bool special = User_Power_Low || Test_Led;

    // ---- Esc + logo: vim mode colour; logo lit count = battery ----
    if (!special) {
        uint8_t r = 0, g = 0, b = 0;
        bool    have = true;
        if (!vim_mode_enabled()) {
            r = 0xFF; g = 0x00; b = 0x00; // red: vim off
        } else {
            switch (get_vim_mode()) {
                case NORMAL_MODE:      r = 0x00; g = 0x00; b = 0xFF; break;
                case INSERT_MODE:      r = 0x00; g = 0xFF; b = 0x00; break;
                case VISUAL_MODE:
                case VISUAL_LINE_MODE: r = 0x80; g = 0x00; b = 0x80; break;
                default:               have = false; break;
            }
        }
        if (replace_active) { // replace (R) mode is shown orange
            r = 0xFF; g = 0x80; b = 0x00; have = true;
        }
        if (have) {
            rgb_matrix_set_color(0, r, g, b); // Esc key LED
            uint8_t bat = User_Batt_BaiFen;
            if (bat > 100) bat = 100;
            uint8_t lit = (uint8_t)(((uint16_t)bat * 3 + 99) / 100);
            for (uint8_t i = 0; i < 3; i++) {
                if (i < lit) {
                    rgb_matrix_set_color(61 + i, r, g, b);
                } else {
                    rgb_matrix_set_color(61 + i, 0, 0, 0);
                }
            }
        }
    }

    // ---- 26 letters: key-triggered brief flash, follow the global hue ----
    if (!(Key_Fn_Status || Led_Rf_Pair_Flg || User_Power_Low || Test_Led)) {
        uint8_t hue = rgb_matrix_get_hue();
        for (uint8_t i = 0; i < 26; i++) {
            uint8_t idx = letter_led[i];
            if (letter_flash[i]) {
                uint16_t elapsed = timer_elapsed(letter_flash[i]);
                if (elapsed < LETTER_FLASH_MS) {
                    uint8_t v   = (uint8_t)((uint16_t)RGB_MATRIX_MAXIMUM_BRIGHTNESS * (LETTER_FLASH_MS - elapsed) / LETTER_FLASH_MS);
                    hsv_t   hsv = {.h = hue, .s = 255, .v = v};
                    rgb_t   rgb = hsv_to_rgb(hsv);
                    rgb_matrix_set_color(idx, rgb.r, rgb.g, rgb.b);
                } else {
                    letter_flash[i] = 0;
                    rgb_matrix_set_color(idx, 0, 0, 0);
                }
            } else {
                rgb_matrix_set_color(idx, 0, 0, 0);
            }
        }
    }

    return false;
}
