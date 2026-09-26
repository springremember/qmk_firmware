/* test_nut65.c — keyboard-local host test for the nut65 vim keymap.
 *
 * Not part of qmk-vim-fn: it lives in the nut65 branch because it exercises the
 * NUT65-specific glue configuration (the myfn declared contract, the vendor
 * keys that pass through to nut65.c's process_record_kb tail, and the
 * Fn+RightShift+Esc bootloader combo) against the shared qmk-vim-fn layer.
 * The shared layer is consumed via the qmk-vim-fn submodule (../qmk-vim-fn),
 * whose generic QMK stub (qmk_stub.h) provides the host API.
 *
 * Run:  cd keyboards/leku/nut65/keymaps/vim/test && make
 */
#include "qmk_stub.h"
#include "emit.h" /* kv_emit_flush_now() */
#include "qmk-vim-fn/engine/include/kv.h"
#include "qmk-vim-fn/qmk/vim_glue.h"
#include "qmk-vim-fn/qmk/vim_keymap_common.h"

/* Vendor keycodes not present in qmk_stub.h.  Values live in QK_KB-ish space
 * (0x7Dxx) with T_OTHER low bytes so the engine classifies them as pass-through
 * and none collide with a layer-key range. */
#define VK_EE_CLR  0x7D00
#define VK_HS_BATQ 0x7D01
#define VK_BT1     0x7D02
#define VK_BT2     0x7D03
#define VK_BT3     0x7D04
#define VK_2G4     0x7D05
#define VK_USB     0x7D06

/* NUT65 uses KC_RCMD for the Mac long-press modifier; QMK's KC_RCMD is a
 * #define alias of KC_RGUI (0xE7), which the stub spells KC_RGUI. */
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

static uint32_t g_now;
uint16_t timer_read(void) { return (uint16_t)g_now; }
uint16_t timer_elapsed(uint16_t since) { return (uint16_t)((uint16_t)g_now - since); }
uint32_t timer_read32(void) { return g_now; }
uint32_t timer_elapsed32(uint32_t since) { return g_now - since; }

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

/* ---------------- simulated NUT65 cfg ---------------- */
#define FN_LAYER 5 /* NUT65 _FN index (0 _BL,1 _FL,2 _MBL,3 _MFL,4 _DEFA,5 _FN) */

/* Exact transcription of nut65_myfn_declared() (nut65 keymap.c). */
static bool nut65_declared_sim(uint16_t keycode) {
    if (keycode >= KC_F1 && keycode <= KC_F12) return true;
    if (keycode == KC_VOLD || keycode == KC_VOLU) return true;
    if (keycode == KC_CAPS || keycode == KC_ESC) return true;
    if (keycode == VK_EE_CLR) return true;
    if (keycode == VK_HS_BATQ) return true;
    if (keycode == VK_BT1 || keycode == VK_BT2 || keycode == VK_BT3 ||
        keycode == VK_2G4 || keycode == VK_USB) return true;
    if (keycode == KC_L) return true; /* v2.11: Fn+L sleep */
    return false;
}

/* ---------------- simulated NUT65 deep-sleep power state (v2.11) ----------
 * Faithful transcription of the pw_* state + power_combo_process() +
 * nut65_myfn() in keyboards/leku/nut65/keymaps/vim/keymap.c.  Positions come
 * from keyboard.json: Fn = [4,11] (MO(_FN)), top-right = [0,14] (_BL Delete).
 * pw_enter_sleep()/pw_boot_wireless() side effects (clear_keyboard / lpwr /
 * suspend) are recorded, not executed. */
#define PW_ROW_FN  4
#define PW_COL_FN  11
#define PW_ROW_TOP 0
#define PW_COL_TOP 14

static bool pr_boot_combo_sim(uint16_t keycode, keyrecord_t *record); /* defined below */

static bool g_cable;           /* true = USB cable present */
static bool g_pw_off;          /* deep sleep */
static bool g_pw_wfn;          /* Fn half held */
static bool g_pw_wtop;         /* top-right half held */
static int  g_sleep_calls;     /* pw_enter_sleep() invocations */
static int  g_wake_calls;      /* pw_boot_wireless() invocations */

static bool pw_no_cable_sim(void) { return !g_cable; }

static void pw_enter_sleep_sim(void) {
    g_pw_off = true;
    g_sleep_calls++;
}

static void pw_boot_wireless_sim(void) {
    g_pw_off  = false;
    g_pw_wfn  = false;
    g_pw_wtop = false;
    g_wake_calls++;
}

static bool power_combo_sim(uint16_t keycode, keyrecord_t *record) {
    (void)keycode;
    if (!g_pw_off) return false;
    bool is_fn  = (record->event.key.row == PW_ROW_FN && record->event.key.col == PW_COL_FN);
    bool is_top = (record->event.key.row == PW_ROW_TOP && record->event.key.col == PW_COL_TOP);
    if (record->event.pressed) {
        if (is_fn) g_pw_wfn = true;
        if (is_top) g_pw_wtop = true;
        if (g_pw_wfn && g_pw_wtop) { pw_boot_wireless_sim(); return true; }
        if (is_fn || is_top) return true;
        pw_enter_sleep_sim();
        return true;
    }
    if (is_fn) g_pw_wfn = false;
    if (is_top) g_pw_wtop = false;
    if ((is_fn || is_top) && !g_pw_wfn && !g_pw_wtop) pw_enter_sleep_sim();
    return true;
}

/* Exact transcription of nut65_myfn() (nut65 keymap.c v2.11). */
static bool nut65_myfn_sim(uint16_t keycode, bool pressed) {
    if (keycode == KC_L) {
        if (pressed && pw_no_cable_sim()) pw_enter_sleep_sim();
        return true;
    }
    return false;
}

/* Composed hook_pre = pr_boot_combo then power_combo (mirrors nut65_hook_pre). */
static bool nut65_hook_pre_sim(uint16_t keycode, keyrecord_t *record) {
    if (pr_boot_combo_sim(keycode, record)) return true;
    if (power_combo_sim(keycode, record)) return true;
    return false;
}

/* ---------------- simulated NUT65 pr_boot_combo (Fn+RightShift+Esc) --------
 * Faithful transcription of the decision logic in
 * keyboards/leku/nut65/keymaps/vim/keymap.c (pr_boot_combo).  The real version
 * then does clear_keyboard()/wait_ms()/eeconfig_disable()/bootloader_jump();
 * the reset itself is outside the shared layer, so we record that the jump
 * would have happened instead of invoking it.  The modifier is read off the
 * physical shadow (vim_glue_mods()) because the myfn skeleton swallows Right
 * Shift on _FN, so get_mods() would no longer contain it. */
static int  s_boot_jump_calls;
static bool pr_boot_combo_sim(uint16_t keycode, keyrecord_t *record) {
    if (keycode == VK_EE_CLR && layer_state_cmp(layer_state, FN_LAYER) &&
        (vim_glue_mods() & MOD_BIT(KC_RSFT))) {
        if (record->event.pressed) {
            s_boot_jump_calls++; /* real code: clear_keyboard(); bootloader_jump(); */
        }
        return true;
    }
    return false;
}

static const vim_cfg_t g_cfg = {
    .fn_layer         = FN_LAYER,
    .trigger_kc       = TEST_TRIGGER_KC, /* NUT65 VIM_MOUSE is SAFE_RANGE; value irrelevant here */
    .mod_win          = KC_RALT,
    .mod_mac          = KC_RCMD,
    .is_mac           = NULL,
    .link_ok          = NULL,
    .hold_ms          = 200,
    .shift_esc_enable = true,
    .led_index        = 0,
    .hook_pre         = nut65_hook_pre_sim, /* pr_boot_combo + deep-sleep wake (v2.11) */
    .hook_post_myfn   = NULL,
    .myfn_declared    = nut65_declared_sim,
    .myfn             = nut65_myfn_sim, /* Fn+L sleep; other declared keys pass through */
    .vim_set_enabled  = NULL,
    .shortcuts        = vim_default_shortcuts,
};

static bool feed_rc(uint16_t kc, bool pressed, uint8_t row, uint8_t col) {
    keyrecord_t r = {0};
    r.event.pressed = pressed;
    r.event.key.row = row;
    r.event.key.col = col;
    bool pass = vim_pipeline_process(kc, &r, &g_cfg);
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

static bool feed(uint16_t kc, bool pressed) { return feed_rc(kc, pressed, 0, 0); }

static void reset_engine(void) {
    g_now = 1000;
    s_mods = 0;
    s_reg_n = 0;
    s_phys_n = 0;
    s_orphan = 0;
    s_boot_jump_calls = 0;
    g_cable = true; /* default wired: a stray myfn call cannot sleep */
    g_pw_off = false;
    g_pw_wfn = false;
    g_pw_wtop = false;
    g_sleep_calls = 0;
    g_wake_calls = 0;
    layer_state = 0;
    default_layer_state = 0;
    vim_keymap_common_init(); /* shared statics + kv_init/enable/INSERT */
}

static void fn_on(void) { layer_state = (1UL << FN_LAYER); }
static void fn_off(void) { layer_state = 0; }

/* ======================================================================
 * 1. myfn declared table membership (static contract check)
 * ====================================================================== */
static void test_declared_table(void) {
    const uint16_t must[] = {
        KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10,
        KC_F11, KC_F12, KC_VOLD, KC_VOLU, KC_CAPS, KC_ESC,
        VK_EE_CLR, VK_HS_BATQ, VK_BT1, VK_BT2, VK_BT3, VK_2G4, VK_USB,
        KC_L,
    };
    for (unsigned i = 0; i < sizeof(must) / sizeof(must[0]); i++) {
        CHECK(nut65_declared_sim(must[i]) == true);
    }
    CHECK(nut65_declared_sim(KC_Z) == false);
    CHECK(nut65_declared_sim(KC_LSFT) == false);
    CHECK(nut65_declared_sim(KC_RSFT) == false);
}

/* ======================================================================
 * 2. every declared _FN key passes on BOTH edges (pipeline == true)
 * ====================================================================== */
static void test_declared_pass_both_edges(void) {
    const uint16_t keys[] = {
        VK_EE_CLR, VK_HS_BATQ, VK_BT1, VK_2G4, VK_USB,
        KC_F1, KC_VOLD,
    };
    for (unsigned i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        reset_engine();
        fn_on();
        uint16_t kc = keys[i];
        if (feed(kc, true) != true) {
            g_fail++;
            printf("FAIL %s:%d  declared key 0x%04X press was not passed through on _FN\n",
                   __FILE__, __LINE__, kc);
        } else {
            g_pass++;
        }
        if (feed(kc, false) != true) {
            g_fail++;
            printf("FAIL %s:%d  declared key 0x%04X release was not passed through on _FN\n",
                   __FILE__, __LINE__, kc);
        } else {
            g_pass++;
        }
        CHECK(s_orphan == 0);
        fn_off();
    }

    /* The rest of the declared set gets the same both-edge contract. */
    const uint16_t more[] = {KC_F2, KC_F6, KC_F12, KC_VOLU, VK_BT2, VK_BT3};
    for (unsigned i = 0; i < sizeof(more) / sizeof(more[0]); i++) {
        reset_engine();
        fn_on();
        CHECK(feed(more[i], true) == true);
        CHECK(feed(more[i], false) == true);
        CHECK(s_orphan == 0);
        fn_off();
    }
}

/* ======================================================================
 * 3. undeclared key on _FN: press swallowed, release paired (no orphan)
 * ====================================================================== */
static void test_undeclared_z_swallowed(void) {
    reset_engine();
    fn_on();
    CHECK(feed(KC_Z, true) == false);  /* swallowed on press */
    CHECK(s_orphan == 0);
    CHECK(feed(KC_Z, false) == false); /* paired release consumed */
    CHECK(s_orphan == 0);
    fn_off();

    /* Sanity: with Fn off, z is an ordinary pass-through on both edges. */
    reset_engine();
    CHECK(feed(KC_Z, true) == true);
    CHECK(feed(KC_Z, false) == true);
    CHECK(s_orphan == 0);
}

/* ======================================================================
 * 4. bare modifier on _FN: press swallowed (no Fn+Shift+Esc leak), release
 *    never strands the host modifier; a modifier registered before Fn is
 *    unregistered normally when released on _FN.
 * ====================================================================== */
static void test_bare_modifier_no_leak_no_stick(void) {
    /* (a) Fn+Shift: the Shift press must not reach the host. */
    reset_engine();
    fn_on();
    CHECK(feed(KC_LSFT, true) == false); /* swallowed: no leak */
    CHECK(reg_count(KC_LSFT) == 0);
    CHECK(get_mods() == 0);
    bool rel = feed(KC_LSFT, false);
    CHECK(rel == false);
    CHECK(s_orphan == 0);
    CHECK(reg_count(KC_LSFT) == 0);
    CHECK(get_mods() == 0);
    fn_off();

    /* (b) A modifier QMK registered BEFORE Fn went down must be released
     * normally while Fn is held (no stuck modifier). */
    reset_engine();
    CHECK(feed(KC_LSFT, true) == true); /* Fn off: QMK registers Shift */
    CHECK(reg_count(KC_LSFT) == 1);
    fn_on();
    if (feed(KC_LSFT, false) != true) {
        g_fail++;
        printf("FAIL %s:%d  Shift release on _FN was swallowed -> stuck Shift\n",
               __FILE__, __LINE__);
    } else {
        g_pass++;
    }
    CHECK(reg_count(KC_LSFT) == 0);
    CHECK(get_mods() == 0);
    CHECK(s_orphan == 0);
    fn_off();

    /* (c) Right Shift must behave identically (pr_boot_combo reads the
     * physical shadow, so the shadow must record RSFT even though the skeleton
     * swallows it). */
    reset_engine();
    fn_on();
    CHECK(feed(KC_RSFT, true) == false);
    CHECK((vim_glue_mods() & MOD_BIT(KC_RSFT)) != 0);
    CHECK(feed(KC_RSFT, false) == false);
    CHECK((vim_glue_mods() & MOD_BIT(KC_RSFT)) == 0);
    CHECK(s_orphan == 0);
    fn_off();
}

/* ======================================================================
 * 5. Fn + RightShift + Esc(EE_CLR) -> bootloader (pr_boot_combo)
 *    The Esc position on _FN resolves to EE_CLR; the combo must fire on that
 *    exact combination and on nothing else.  Myfn swallows Right Shift on _FN,
 *    so this also proves the combo reads the physical shadow, not get_mods().
 * ====================================================================== */
static void test_boot_combo(void) {
    /* (a) Fn + RightShift + Esc fires (shadow holds RSFT even though myfn
     *     swallowed the bare Right Shift press). */
    reset_engine();
    fn_on();
    CHECK(feed(KC_RSFT, true) == false);                  /* myfn swallows the bare modifier */
    CHECK((vim_glue_mods() & MOD_BIT(KC_RSFT)) != 0);     /* ...but the shadow records it */
    CHECK(feed(VK_EE_CLR, true) == false);                /* hook_pre consumes it */
    CHECK(s_boot_jump_calls == 1);
    CHECK(feed(VK_EE_CLR, false) == false);               /* paired release */
    CHECK(s_boot_jump_calls == 1);                        /* no second jump on release */
    CHECK(feed(KC_RSFT, false) == false);
    CHECK(s_orphan == 0);
    fn_off();

    /* (b) Left Shift + Esc must NOT fire (only Right Shift is the combo). */
    reset_engine();
    fn_on();
    CHECK(feed(KC_LSFT, true) == false);
    CHECK(feed(VK_EE_CLR, true) == true);                 /* declared: passes through */
    CHECK(s_boot_jump_calls == 0);
    CHECK(feed(VK_EE_CLR, false) == true);
    CHECK(feed(KC_LSFT, false) == false);
    fn_off();

    /* (c) RightShift + Esc without Fn must NOT fire.  (Right Shift is swallowed
     *     by the lazy-Shift rule even without Fn, but the shadow still holds it.) */
    reset_engine();
    CHECK(feed(KC_RSFT, true) == false);                  /* lazy Shift: swallowed */
    CHECK((vim_glue_mods() & MOD_BIT(KC_RSFT)) != 0);
    CHECK(feed(VK_EE_CLR, true) == true);
    CHECK(s_boot_jump_calls == 0);
    CHECK(feed(VK_EE_CLR, false) == true);
    CHECK(feed(KC_RSFT, false) == false);
    fn_off();

    /* (d) Fn + RightShift + a non-Esc _FN key (F1) must NOT fire. */
    reset_engine();
    fn_on();
    CHECK(feed(KC_RSFT, true) == false);
    CHECK(feed(KC_F1, true) == true);                     /* declared: passes through */
    CHECK(s_boot_jump_calls == 0);
    CHECK(feed(KC_F1, false) == true);
    CHECK(feed(KC_RSFT, false) == false);
    CHECK(s_orphan == 0);
    fn_off();

    /* (e) Shift released before Esc: combination never completes -> no jump. */
    reset_engine();
    fn_on();
    CHECK(feed(KC_RSFT, true) == false);
    CHECK(feed(KC_RSFT, false) == false);                 /* shadow RSFT cleared */
    CHECK(feed(VK_EE_CLR, true) == true);
    CHECK(s_boot_jump_calls == 0);
    CHECK(feed(VK_EE_CLR, false) == true);
    fn_off();
}

/* ======================================================================
 * 6. v2.11 Fn+L short press -> deep sleep (only when no USB cable)
 * ====================================================================== */
static void test_fn_l_sleep(void) {
    /* (a) Fn+L, wireless: sleep fires once, both edges swallowed (no 'l' leak). */
    reset_engine();
    g_cable = false;
    fn_on();
    CHECK(feed(KC_L, true) == false);
    CHECK(g_sleep_calls == 1);
    CHECK(feed(KC_L, false) == false);
    CHECK(s_orphan == 0);
    fn_off();

    /* (b) Fn+L, USB wired: swallowed but does NOT sleep (空跑). */
    reset_engine();
    g_cable = true;
    fn_on();
    CHECK(feed(KC_L, true) == false);
    CHECK(g_sleep_calls == 0);
    CHECK(feed(KC_L, false) == false);
    CHECK(s_orphan == 0);
    fn_off();

    /* (c) L off _FN is an ordinary pass-through on both edges. */
    reset_engine();
    CHECK(feed(KC_L, true) == true);
    CHECK(feed(KC_L, false) == true);
    CHECK(s_orphan == 0);
}

/* ======================================================================
 * 7. v2.11 Fn([4,11]) + top-right([0,14]) = the ONLY wake combo; press order
 *    independent; unrelated keys only re-arm sleep.
 * ====================================================================== */
#define FN_KC MO(FN_LAYER) /* the real Fn key code at [4,11] */
static void test_wake_combo(void) {
    /* (a) Fn first, then top-right -> wake. */
    reset_engine();
    g_pw_off = true;
    CHECK(feed_rc(FN_KC, true, PW_ROW_FN, PW_COL_FN) == false);
    CHECK(g_wake_calls == 0);
    CHECK(g_pw_off == true);
    CHECK(feed_rc(KC_DEL, true, PW_ROW_TOP, PW_COL_TOP) == false);
    CHECK(g_wake_calls == 1);
    CHECK(g_pw_off == false);
    CHECK(feed_rc(KC_DEL, false, PW_ROW_TOP, PW_COL_TOP) == false); /* paired */
    CHECK(feed_rc(FN_KC, false, PW_ROW_FN, PW_COL_FN) == false);    /* paired */
    CHECK(s_orphan == 0);

    /* (b) top-right first, then Fn (row0 before row4 in one scan) -> still wakes. */
    reset_engine();
    g_pw_off = true;
    CHECK(feed_rc(KC_DEL, true, PW_ROW_TOP, PW_COL_TOP) == false);
    CHECK(g_wake_calls == 0);
    CHECK(g_pw_off == true);
    CHECK(feed_rc(FN_KC, true, PW_ROW_FN, PW_COL_FN) == false);
    CHECK(g_wake_calls == 1);
    CHECK(g_pw_off == false);
    CHECK(feed_rc(FN_KC, false, PW_ROW_FN, PW_COL_FN) == false);
    CHECK(feed_rc(KC_DEL, false, PW_ROW_TOP, PW_COL_TOP) == false);
    CHECK(s_orphan == 0);

    /* (c) unrelated key while asleep: consumed, re-arms sleep, never wakes. */
    reset_engine();
    g_pw_off = true;
    CHECK(feed_rc(KC_A, true, 1, 1) == false);
    CHECK(g_wake_calls == 0);
    CHECK(g_pw_off == true);
    CHECK(g_sleep_calls >= 1);
    CHECK(feed_rc(KC_A, false, 1, 1) == false);
    CHECK(s_orphan == 0);

    /* (d) Fn held then released without the top-right half -> re-arm sleep. */
    reset_engine();
    g_pw_off = true;
    CHECK(feed_rc(FN_KC, true, PW_ROW_FN, PW_COL_FN) == false);
    CHECK(g_pw_off == true);
    CHECK(g_sleep_calls == 0); /* holding: stays awake for the partner half */
    CHECK(feed_rc(FN_KC, false, PW_ROW_FN, PW_COL_FN) == false);
    CHECK(g_sleep_calls == 1); /* released without combo: re-arm */
    CHECK(g_wake_calls == 0);
    CHECK(s_orphan == 0);

    /* (e) the wake combo must NOT fire while awake (pw_off == false). */
    reset_engine();
    feed_rc(FN_KC, true, PW_ROW_FN, PW_COL_FN);
    feed_rc(KC_DEL, true, PW_ROW_TOP, PW_COL_TOP);
    CHECK(g_wake_calls == 0);
    feed_rc(KC_DEL, false, PW_ROW_TOP, PW_COL_TOP);
    feed_rc(FN_KC, false, PW_ROW_FN, PW_COL_FN);
    CHECK(g_wake_calls == 0);
    CHECK(s_orphan == 0);
}

/* ======================================================================
 * 8. Esc 切换 + 3s 宽限；Caps 单击开关 vim；右 Shift 懒发送（共享层新语义）
 * ====================================================================== */
static bool lshift_down(void) { return (get_mods() & MOD_BIT_LSHIFT) != 0; }

static void test_esc_caps_rshift(void) {
    /* Esc: Insert（无宽限）吞键进 Normal；Normal 空闲发真 Esc 回 Insert 开宽限 */
    reset_engine();
    CHECK(vim_insert_flash() == false);  /* 开机：无提示色 */
    CHECK(feed(KC_ESC, true) == false);
    CHECK(kv_get_mode() == KV_MODE_NORMAL);
    CHECK(feed(KC_ESC, false) == false);
    CHECK(feed(KC_ESC, true) == true);   /* Normal 空闲 -> 真 Esc */
    CHECK(kv_get_mode() == KV_MODE_INSERT);
    CHECK(vim_insert_flash() == true);   /* 同一窗口驱动橙色「回到打字」提示 */
    CHECK(feed(KC_ESC, false) == true);
    g_now += 2999;
    CHECK(feed(KC_ESC, true) == true);   /* 宽限内 -> 真 Esc，留 Insert */
    CHECK(kv_get_mode() == KV_MODE_INSERT);
    CHECK(vim_insert_flash() == true);
    CHECK(feed(KC_ESC, false) == true);
    g_now += 3000;
    CHECK(vim_insert_flash() == false);  /* 窗口过期 -> 回 Insert 绿 */
    CHECK(feed(KC_ESC, true) == false);  /* 宽限过 -> 吞键进 Normal */
    CHECK(kv_get_mode() == KV_MODE_NORMAL);
    CHECK(feed(KC_ESC, false) == false);

    /* 审计 P1 回归：窗口不得因 16 位计时回绕复活（持续打字 65.5s 假亮橙） */
    reset_engine(); /* INSERT */
    g_now = 1000;
    CHECK(feed(KC_ESC, true) == false);   /* -> NORMAL */
    (void)feed(KC_ESC, false);
    CHECK(feed(KC_ESC, true) == true);    /* t0=1000 开窗 */
    CHECK(vim_insert_flash() == true);
    (void)feed(KC_ESC, false);
    g_now = 1000 + 65000;                 /* 远过 3s，但未回绕 */
    CHECK(vim_insert_flash() == false);
    g_now = 1000 + 65536;                 /* 恰好 16 位回绕点：必须仍为假 */
    CHECK(vim_insert_flash() == false);
    g_now = 1000 + 68535;                 /* 回绕后 +2999ms：仍为假 */
    CHECK(vim_insert_flash() == false);
    CHECK(kv_get_mode() == KV_MODE_INSERT);
    CHECK(feed(KC_ESC, true) == false);   /* 窗口已过期 -> 仍吞键进 Normal */
    CHECK(kv_get_mode() == KV_MODE_NORMAL);
    (void)feed(KC_ESC, false);

    /* Caps 单击（vim on）：press 预览 Normal，release 开关 vim */
    reset_engine();
    CHECK(feed(KC_CAPS, true) == false);
    CHECK(kv_get_mode() == KV_MODE_NORMAL);
    CHECK(kv_vim_enabled() == true);
    CHECK(feed(KC_CAPS, false) == false);
    CHECK(kv_vim_enabled() == false);
    CHECK(feed(KC_CAPS, true) == false);  /* vim off：仍被消费 */
    CHECK(feed(KC_CAPS, false) == false);
    CHECK(kv_vim_enabled() == true);
    CHECK(kv_get_mode() == KV_MODE_INSERT);
    CHECK(vim_insert_flash() == false);  /* Caps 开 vim 不亮橙 */

    /* Fn+Caps 与裸 Caps 相同 */
    reset_engine();
    fn_on();
    CHECK(feed(KC_CAPS, true) == false);
    CHECK(feed(KC_CAPS, false) == false);
    CHECK(kv_vim_enabled() == false);
    fn_off();

    /* 右 Shift 懒发送 */
    reset_engine();
    CHECK(feed(KC_RSFT, true) == false);  /* 孤立右 Shift：无输出 */
    CHECK(!lshift_down());
    CHECK(feed(KC_RSFT, false) == false);
    CHECK(!lshift_down());
    reset_engine();
    CHECK(feed(KC_RSFT, true) == false);
    CHECK(feed(KC_A, true) == true);      /* 右Shift+a -> 临时补左Shift */
    CHECK(lshift_down());
    CHECK(feed(KC_A, false) == true);
    CHECK(lshift_down());
    CHECK(feed(KC_RSFT, false) == false);
    CHECK(!lshift_down());

    /* vim 关闭：右 Shift 恢复正常 */
    reset_engine();
    kv_disable();
    CHECK(feed(KC_RSFT, true) == true);
    CHECK(feed(KC_RSFT, false) == true);
}

int main(void) {
    test_declared_table();
    test_declared_pass_both_edges();
    test_undeclared_z_swallowed();
    test_bare_modifier_no_leak_no_stick();
    test_boot_combo();
    test_fn_l_sleep();
    test_wake_combo();
    test_esc_caps_rshift();
    printf("nut65-sim: pass=%d fail=%d\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
