/* test_qk61.c — keyboard-local host test for the qk61 vim keymap.
 *
 * Not part of qmk-vim-fn: it lives in the qk61 branch because it exercises the
 * QK61-specific glue configuration (VIM_MOUSE trigger, the qk61_myfn table and
 * the qk61_hook_post Ctrl+Alt+Del chord) against the shared qmk-vim-fn layer.
 * The shared layer is consumed via the qmk-vim-fn submodule (../qmk-vim-fn),
 * whose generic QMK stub (qmk_stub.h) provides the host API.
 *
 * Run:  cd keyboards/qk61/keymaps/vim/test && make
 */
#include "qmk_stub.h"
#include "emit.h" /* kv_emit_flush_now() */
#include "qmk-vim-fn/engine/include/kv.h"
#include "qmk-vim-fn/qmk/vim_glue.h"
#include "qmk-vim-fn/qmk/vim_keymap_common.h"

/* QK61's right-Alt-position VIM_MOUSE keycode (QMK_KB_22 in keymap.c). */
#define QK_VIM_MOUSE 0x7E16
/* QK61 Win-lock pins the Mac modifier to RGUI. */
#ifndef KC_RCMD
#define KC_RCMD KC_RGUI
#endif

/* ---------------- host state (mirrors the shared glue tests) ---------------- */
layer_state_t layer_state         = 0;
layer_state_t default_layer_state = 0;

static uint8_t s_mods;
uint8_t get_mods(void) { return s_mods; }
void    clear_mods(void) { s_mods = 0; }
void    set_mods(uint8_t m) { s_mods = m; }
void    register_mods(uint8_t m) { s_mods |= m; }
void    unregister_mods(uint8_t m) { s_mods &= (uint8_t)~m; }

#define REG_CAP 64
static uint16_t s_reg[REG_CAP];
static int      s_reg_n;

void register_code(uint16_t kc) {
    if (IS_MODIFIER_KEYCODE(kc)) s_mods |= (uint8_t)(1u << (kc - KC_LCTL));
    if (s_reg_n < REG_CAP) s_reg[s_reg_n++] = kc;
}
void unregister_code(uint16_t kc) {
    if (IS_MODIFIER_KEYCODE(kc)) s_mods &= (uint8_t)~(1u << (kc - KC_LCTL));
    for (int i = 0; i < s_reg_n; i++) {
        if (s_reg[i] == kc) { s_reg[i] = s_reg[--s_reg_n]; return; }
    }
}
void tap_code(uint16_t kc) { register_code(kc); unregister_code(kc); }
void tap_code16(uint16_t kc) { tap_code((uint16_t)(kc & 0xFF)); }

static uint16_t g_now;
uint16_t timer_read(void) { return g_now; }
uint16_t timer_elapsed(uint16_t since) { return (uint16_t)(g_now - since); }

static int reg_count(uint16_t kc) {
    int n = 0;
    for (int i = 0; i < s_reg_n; i++) if (s_reg[i] == kc) n++;
    return n;
}

static uint16_t s_phys[REG_CAP];
static int      s_phys_n;
static int      s_orphan;

static void host_press(uint16_t kc) { register_code(kc); }
static void host_release(uint16_t kc) { unregister_code(kc); }

/* ---------------- test bookkeeping ---------------- */
static int g_pass, g_fail;
#define CHECK(cond)                                                        \
    do {                                                                   \
        if (cond) { g_pass++; }                                            \
        else { g_fail++; printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); } \
    } while (0)

/* ---------------- simulated QK61 cfg ---------------- */
#define FN_LAYER 4 /* QK61 MYFN_LAYER */

/* Exact transcription of qk61_myfn_declared() (keymap.c). */
static bool qk61_declared_sim(uint16_t keycode) {
    if (keycode >= KC_F1 && keycode <= KC_F12) return true;
    if (keycode == KC_VOLD || keycode == KC_VOLU) return true;
    if (keycode == KC_SPC || keycode == KC_CAPS || keycode == KC_ESC) return true;
    if (keycode == KC_T) return true; // physical switch: swallow
    return false;
}

/* Exact transcription of qk61_myfn(): Fn+Space battery (consume), Fn+T
 * (consume, no-op), everything else declared passes to QMK. */
static bool s_batt_held;
static bool qk61_myfn_sim(uint16_t keycode, bool pressed) {
    if (keycode == KC_SPC) {
        if (pressed) s_batt_held = true;
        else if (s_batt_held) s_batt_held = false;
        return true;
    }
    if (keycode == KC_T) return true;
    return false;
}

/* Exact transcription of the Ctrl+Alt+Del branch of qk61_hook_post():
 * left-Ctrl + left-Alt + Backspace -> swallow BSPC and tap Delete.  (The flash
 * and Fn+Esc 3s-reset branches touch hardware/timers and are not host-testable
 * here.) */
static bool qk61_hook_post_sim(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_BSPC && record->event.pressed && (get_mods() & MOD_BIT(KC_LCTL)) &&
        (get_mods() & MOD_BIT(KC_LALT))) {
        vim_glue_swallow(KC_BSPC);
        tap_code(KC_DEL);
        return true;
    }
    return false;
}

static const vim_cfg_t g_cfg = {
    .fn_layer         = FN_LAYER,
    .trigger_kc       = QK_VIM_MOUSE,
    .mod_win          = KC_RALT,
    .mod_mac          = KC_RCMD,
    .is_mac           = NULL,
    .link_ok          = NULL,
    .hold_ms          = 200,
    .shift_esc_enable = true,
    .led_index        = 0,
    .hook_pre         = NULL,
    .hook_post_myfn   = qk61_hook_post_sim,
    .myfn_declared    = qk61_declared_sim,
    .myfn             = qk61_myfn_sim,
    .vim_set_enabled  = NULL,
    .shortcuts        = vim_default_shortcuts,
};

static bool pipeline(uint16_t kc, bool pressed) {
    keyrecord_t r = {0};
    r.event.pressed = pressed;
    return vim_pipeline_process(kc, &r, &g_cfg);
}

static bool feed(uint16_t kc, bool pressed) {
    bool pass = pipeline(kc, pressed);
    if (pressed) {
        if (pass) { host_press(kc); if (s_phys_n < REG_CAP) s_phys[s_phys_n++] = kc; }
    } else {
        if (pass) {
            bool found = false;
            for (int i = 0; i < s_phys_n; i++) {
                if (s_phys[i] == kc) { s_phys[i] = s_phys[--s_phys_n]; found = true; break; }
            }
            if (!found) s_orphan++;
            host_release(kc);
        }
    }
    return pass;
}

static void reset_engine(void) {
    g_now = 1000;
    s_mods = 0;
    s_reg_n = 0;
    s_phys_n = 0;
    s_orphan = 0;
    s_batt_held = false;
    layer_state = 0;
    default_layer_state = 0;
    vim_glue_init();
}

static void fn_on(void) { layer_state = (1UL << FN_LAYER); }
static void fn_off(void) { layer_state = 0; }

/* ======================================================================
 * 1. declared table membership
 * ====================================================================== */
static void test_declared_table(void) {
    const uint16_t must[] = {KC_F1, KC_F6, KC_F12, KC_VOLD, KC_VOLU, KC_SPC, KC_CAPS, KC_ESC, KC_T};
    for (unsigned i = 0; i < sizeof(must) / sizeof(must[0]); i++) {
        CHECK(qk61_declared_sim(must[i]) == true);
    }
    CHECK(qk61_declared_sim(KC_Z) == false);
    CHECK(qk61_declared_sim(KC_LSFT) == false);
}

/* ======================================================================
 * 2. Fn+Space battery is consumed on both edges; Fn+Z is swallowed.
 * ====================================================================== */
static void test_myfn_contract(void) {
    reset_engine();
    fn_on();
    CHECK(feed(KC_SPC, true) == false);  /* battery: consumed */
    CHECK(s_batt_held == true);
    CHECK(feed(KC_SPC, false) == false);
    CHECK(s_batt_held == false);
    CHECK(s_orphan == 0);

    reset_engine();
    fn_on();
    CHECK(feed(KC_T, true) == false);    /* physical switch: swallowed */
    CHECK(feed(KC_T, false) == false);
    CHECK(s_orphan == 0);

    reset_engine();
    fn_on();
    CHECK(feed(KC_Z, true) == false);    /* undeclared: swallowed */
    CHECK(feed(KC_Z, false) == false);
    CHECK(s_orphan == 0);

    reset_engine();
    fn_on();
    CHECK(feed(KC_F1, true) == true);    /* declared, myfn false: passes */
    CHECK(feed(KC_F1, false) == true);
    fn_off();
}

/* ======================================================================
 * 3. Ctrl+Alt+Del chord (qk61_hook_post): consumed press+release, no stuck
 *    Backspace, and only when both Ctrl and Alt are down.
 * ====================================================================== */
static void test_cad_chord(void) {
    /* (a) Ctrl+Alt held, BSPC consumed and DEL tapped; release paired. */
    reset_engine();
    CHECK(feed(KC_LCTL, true) == true);
    CHECK(feed(KC_LALT, true) == true);
    CHECK(feed(KC_BSPC, true) == false);       /* consumed by the chord */
    CHECK(reg_count(KC_BSPC) == 0);
    CHECK(reg_count(KC_DEL) == 0);             /* DEL was tapped (reg+unreg) */
    if (feed(KC_BSPC, false) != false) {
        g_fail++;
        printf("FAIL %s:%d  CAD: BSPC release leaked an orphan after Ctrl+Alt+Del\n",
               __FILE__, __LINE__);
    } else {
        g_pass++;
    }
    CHECK(s_orphan == 0);
    feed(KC_LCTL, false);
    feed(KC_LALT, false);

    /* (b) BSPC alone is a normal key (no chord). */
    reset_engine();
    CHECK(feed(KC_BSPC, true) == true);
    CHECK(feed(KC_BSPC, false) == true);
    CHECK(s_orphan == 0);

    /* (c) Ctrl held but Alt released before BSPC: no chord. */
    reset_engine();
    CHECK(feed(KC_LCTL, true) == true);
    CHECK(feed(KC_BSPC, true) == true);
    CHECK(feed(KC_BSPC, false) == true);
    feed(KC_LCTL, false);
    CHECK(s_orphan == 0);
}

/* ======================================================================
 * 4. Normal mode: bare h/j/k/l remain held direction keys even with the Win
 *    modifier (LGUI) or Mac modifier held, combined with that modifier.
 * ====================================================================== */
static void test_hjkl_with_modifier(void) {
    /* (a) GUI held + h -> KC_LEFT register-held (Win+Left), no raw h leak. */
    reset_engine();
    kv_set_mode(KV_MODE_NORMAL);
    CHECK(pipeline(KC_LGUI, true) == true);
    CHECK(pipeline(KC_H, true) == false);
    kv_emit_flush_now();
    CHECK(reg_count(KC_LEFT) == 1);
    CHECK(pipeline(KC_H, false) == false);
    CHECK(reg_count(KC_LEFT) == 0);
    CHECK(pipeline(KC_LGUI, false) == true);

    /* (b) QK61 Win mode also has RALT as the mod_win; Ctrl+l -> KC_RGHT. */
    reset_engine();
    kv_set_mode(KV_MODE_NORMAL);
    CHECK(pipeline(KC_LCTL, true) == true);
    CHECK(pipeline(KC_L, true) == false);
    kv_emit_flush_now();
    CHECK(reg_count(KC_RGHT) == 1);
    CHECK(pipeline(KC_L, false) == false);
    CHECK(reg_count(KC_RGHT) == 0);
    CHECK(pipeline(KC_LCTL, false) == true);

    /* (c) non-hjkl keys still pass the chord through (Ctrl+S etc.). */
    reset_engine();
    kv_set_mode(KV_MODE_NORMAL);
    CHECK(pipeline(KC_LCTL, true) == true);
    CHECK(pipeline(KC_S, true) == true);   /* passthrough */
    CHECK(pipeline(KC_S, false) == true);
    CHECK(pipeline(KC_LCTL, false) == true);
}

/* ======================================================================
 * 5. Insert Esc leaves for Normal (A1) and Caps single-tap only enters Normal.
 * ====================================================================== */
static void test_esc_and_caps(void) {
    reset_engine(); /* INSERT */
    CHECK(pipeline(KC_ESC, true) == true);  /* real Esc emitted */
    CHECK(kv_get_mode() == KV_MODE_NORMAL);
    CHECK(pipeline(KC_ESC, false) == true);

    reset_engine(); /* INSERT */
    CHECK(pipeline(KC_CAPS, true) == false);
    CHECK(kv_get_mode() == KV_MODE_NORMAL);
    CHECK(pipeline(KC_CAPS, false) == false);
    CHECK(kv_get_mode() == KV_MODE_NORMAL);

    /* second Caps tap in Normal is a no-op */
    CHECK(pipeline(KC_CAPS, true) == false);
    CHECK(pipeline(KC_CAPS, false) == false);
    CHECK(kv_get_mode() == KV_MODE_NORMAL);
}

int main(void) {
    test_declared_table();
    test_myfn_contract();
    test_cad_chord();
    test_hjkl_with_modifier();
    test_esc_and_caps();
    printf("qk61-sim: pass=%d fail=%d\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
