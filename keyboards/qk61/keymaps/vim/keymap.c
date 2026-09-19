/* Copyright 2023 Yiancar-Designs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
#include "common/rdmctmzt_common.h"

// Bottom-row / right-shift navigation scheme:
//   * the physical Menu key (KC_APP) becomes Fn  -> hold = layer, tap = Left
//   * the physical Fn key (MO) becomes Menu      -> hold = Menu,  tap = Right
//   * Right Ctrl                                 -> hold = Ctrl,  tap = Down
//   * Right Shift                                -> hold = Shift, tap = Up
enum custom_keycodes {
    MENU_TAP_RIGHT = SAFE_RANGE,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_60_ansi(
        QK_GESC, KC_1,     KC_2,     KC_3,      KC_4,      KC_5,     KC_6,     KC_7,    KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,
        KC_TAB,  KC_Q,     KC_W,     KC_E,      KC_R,      KC_T,     KC_Y,     KC_U,    KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,
        KC_CAPS, KC_A,     KC_S,     KC_D,      KC_F,      KC_G,     KC_H,     KC_J,    KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,
        KC_LSFT, KC_Z,     KC_X,     KC_C,      KC_V,      KC_B,     KC_N,     KC_M,    KC_COMM,  KC_DOT,             KC_SLSH,            MT(MOD_RSFT, KC_UP),
        KC_LCTL, KC_LGUI,  KC_LALT,                        KC_SPC,                                KC_RALT,  LT(2, KC_LEFT), MT(MOD_RCTL, KC_DOWN), MENU_TAP_RIGHT
    ),
    [1] = LAYOUT_60_ansi(
        QK_GESC, KC_1,     KC_2,     KC_3,      KC_4,      KC_5,     KC_6,     KC_7,    KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,
        KC_TAB,  KC_Q,     KC_W,     KC_E,      KC_R,      KC_T,     KC_Y,     KC_U,    KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,
        KC_CAPS, KC_A,     KC_S,     KC_D,      KC_F,      KC_G,     KC_H,     KC_J,    KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,
        KC_LSFT, KC_Z,     KC_X,     KC_C,      KC_V,      KC_B,     KC_N,     KC_M,    KC_COMM,  KC_DOT,             KC_SLSH,            MT(MOD_RSFT, KC_UP),
        KC_LCTL, KC_LALT,  KC_LGUI,                        KC_SPC,                                KC_RGUI,  LT(3, KC_LEFT), MT(MOD_RCTL, KC_DOWN), MENU_TAP_RIGHT
    ),
    [2] = LAYOUT_60_ansi(
        KC_GRV,  KC_F1,    KC_F2,    KC_F3,     KC_F4,     KC_F5,    KC_F6,    KC_F7,   KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_BSPC,
        KC_TAB,  MD_BLE1,  MD_BLE2,  MD_BLE3,   MD_24G,    KC_PSCR,  KC_SCRL,  KC_PAUS, KC_I,     KC_O,     KC_P,     UG_SPDD,  UG_SPDU,  UG_NEXT,
        KC_CAPS, TO(0),    TO(1),    KC_D,      KC_F,      KC_INS,   KC_HOME,  KC_PGUP, KC_K,     KC_L,     UG_HUED,  UG_HUEU,            KC_ENT,
        KC_LSFT, RM_NEXT,  KC_NO,    RM_SATD,   RM_SATU,   KC_DEL,   KC_END,   KC_PGDN, RM_VALD,  RM_VALU,            KC_UP,              QK_BAT,
        KC_LCTL, QK_WLO,   KC_LALT,                        RM_TOGG,                               KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_NO
    ),
    [3] = LAYOUT_60_ansi(
        KC_GRV,  KC_BRID,  KC_BRIU,  KC_MCTL,   KC_LPAD,   KC_5,     KC_6,     KC_MPRV, KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_BSPC,
        KC_TAB,  MD_BLE1,  MD_BLE2,  MD_BLE3,   MD_24G,    KC_PSCR,  KC_SCRL,  KC_PAUS, KC_I,     KC_O,     KC_P,     UG_SPDD,  UG_SPDU,  UG_NEXT,
        KC_CAPS, TO(0),    TO(1),    KC_D,      KC_F,      KC_INS,   KC_HOME,  KC_PGUP, KC_K,     KC_PGUP,  UG_HUED,  UG_HUEU,            KC_ENT,
        KC_LSFT, KC_NO,    KC_NO,    RM_SATD,   RM_SATU,   KC_DEL,   KC_END,   KC_PGDN, RM_VALD,  RM_VALU,            KC_UP,              QK_BAT,
        KC_LCTL, KC_LALT,  KC_LGUI,                        RM_TOGG,                               KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_NO
    )
};

// Tap-hold for the Menu key: short tap = Right, long press = Menu (held).
static uint16_t menu_timer   = 0;
static bool     menu_pressed = false;
static bool     menu_held    = false;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case MENU_TAP_RIGHT:
            if (record->event.pressed) {
                menu_pressed = true;
                menu_held    = false;
                menu_timer   = timer_read();
            } else {
                menu_pressed = false;
                if (menu_held) {
                    unregister_code(KC_APP);
                    menu_held = false;
                } else {
                    tap_code(KC_RGHT);
                }
            }
            return false;
    }
    return true;
}

void matrix_scan_user(void) {
    if (menu_pressed && !menu_held && timer_elapsed(menu_timer) >= TAPPING_TERM) {
        menu_held = true;
        register_code(KC_APP);
    }
}
