#pragma once

namespace v3d
{
namespace event
{
    enum KeyModifierCode : u8
    {
        KeyModifier_Uknown = 0x00,
        KeyModifier_Ctrl = 0x01,
        KeyModifier_Alt = 0x02,
        KeyModifier_Shift = 0x04,
        KeyModifier_CapsLock = 0x08
    };

    enum KeyCode : u32
    {
        KeyUknown,  // Default

        KeyLButton,  // Left mouse button
        KeyRButton,  // Right mouse button
        KeyCancel,  // Control-break processing
        KeyMButton,  // Middle mouse button (three-button mouse)
        KeyXButton1,  // Windows 2000/XP: X1 mouse button
        KeyXButton2,  // Windows 2000/XP: X2 mouse button

        KeyBack,  // BACKSPACE key
        KeyTab,  // TAB key
        KeyClear,  // CLEAR key
        KeyReturn,  // ENTER key
        KeyShift,  // SHIFT key
        KeyControl,  // CTRL key
        KeyMenu,  // ALT key
        KeyPause,  // PAUSE key
        KeyCapital,  // CAPS LOCK key
        KeyKana,  // IME Kana mode
        KeyHanguel,  // IME Hanguel mode (maintained for compatibility use KEY_HANGUL)
        KeyHangul,  // IME Hangul mode
        KeyJunja,  // IME Junja mode
        KeyFinal,  // IME final mode
        KeyHanja,  // IME Hanja mode
        KeyKanji,  // IME Kanji mode
        KeyEscape,  // ESC key
        KeyConvert,  // IME convert
        KeyNonconvert,  // IME nonconvert
        KeyAccept,  // IME accept
        KeyModechange,  // IME mode change request
        KeySpace,  // SPACEBAR
        KeyPrior,  // PAGE UP key
        KeyNext,  // PAGE DOWN key
        KeyEnd,  // END key
        KeyHome,  // HOME key
        KeyLeft,  // LEFT ARROW key
        KeyUp,  // UP ARROW key
        KeyRight,  // RIGHT ARROW key
        KeyDown,  // DOWN ARROW key
        KeySelect,  // SELECT key
        KeyPrint,  // PRINT key
        KeyExecut,  // EXECUTE key
        KeySnapshot,  // PRINT SCREEN key
        KeyInsert,  // INS key
        KeyDelete,  // DEL key
        KeyHelp,  // HELP key
        KeyKey_0,  // 0 key
        KeyKey_1,  // 1 key
        KeyKey_2,  // 2 key
        KeyKey_3,  // 3 key
        KeyKey_4,  // 4 key
        KeyKey_5,  // 5 key
        KeyKey_6,  // 6 key
        KeyKey_7,  // 7 key
        KeyKey_8,  // 8 key
        KeyKey_9,  // 9 key
        KeyKey_A,  // A key
        KeyKey_B,  // B key
        KeyKey_C,  // C key
        KeyKey_D,  // D key
        KeyKey_E,  // E key
        KeyKey_F,  // F key
        KeyKey_G,  // G key
        KeyKey_H,  // H key
        KeyKey_I,  // I key
        KeyKey_J,  // J key
        KeyKey_K,  // K key
        KeyKey_L,  // L key
        KeyKey_M,  // M key
        KeyKey_N,  // N key
        KeyKey_O,  // O key
        KeyKey_P,  // P key
        KeyKey_Q,  // Q key
        KeyKey_R,  // R key
        KeyKey_S,  // S key
        KeyKey_T,  // T key
        KeyKey_U,  // U key
        KeyKey_V,  // V key
        KeyKey_W,  // W key
        KeyKey_X,  // X key
        KeyKey_Y,  // Y key
        KeyKey_Z,  // Z key
        KeyLWin,  // Left Windows key (Microsoft® Natural® keyboard)
        KeyRWin,  // Right Windows key (Natural keyboard)
        KeyApps,  // Applications key (Natural keyboard)
        KeySleep,  // Computer Sleep key
        KeyNumpad0,  // Numeric keypad 0 key
        KeyNumpad1,  // Numeric keypad 1 key
        KeyNumpad2,  // Numeric keypad 2 key
        KeyNumpad3,  // Numeric keypad 3 key
        KeyNumpad4,  // Numeric keypad 4 key
        KeyNumpad5,  // Numeric keypad 5 key
        KeyNumpad6,  // Numeric keypad 6 key
        KeyNumpad7,  // Numeric keypad 7 key
        KeyNumpad8,  // Numeric keypad 8 key
        KeyNumpad9,  // Numeric keypad 9 key
        KeyMultiply,  // Multiply key
        KeyAdd,  // Add key
        KeySeparator,  // Separator key
        KeySubtract,  // Subtract key
        KeyDecimal,  // Decimal key
        KeyDivide,  // Divide key
        KeyF1,  // F1 key
        KeyF2,  // F2 key
        KeyF3,  // F3 key
        KeyF4,  // F4 key
        KeyF5,  // F5 key
        KeyF6,  // F6 key
        KeyF7,  // F7 key
        KeyF8,  // F8 key
        KeyF9,  // F9 key
        KeyF10,  // F10 key
        KeyF11,  // F11 key
        KeyF12,  // F12 key
        KeyF13,  // F13 key
        KeyF14,  // F14 key
        KeyF15,  // F15 key
        KeyF16,  // F16 key
        KeyF17,  // F17 key
        KeyF18,  // F18 key
        KeyF19,  // F19 key
        KeyF20,  // F20 key
        KeyF21,  // F21 key
        KeyF22,  // F22 key
        KeyF23,  // F23 key
        KeyF24,  // F24 key
        KeyNumlock,  // NUM LOCK key
        KeyScroll,  // SCROLL LOCK key
        KeyLShift,  // Left SHIFT key
        KeyRShift,  // Right SHIFT key
        KeyLControl,  // Left CONTROL key
        KeyRControl,  // Right CONTROL key
        KeyLMenu,  // Left MENU key
        KeyRMenu,  // Right MENU key
        KeyPlus,  // Plus Key   (+)
        KeyComma,  // Comma Key  (,)
        KeyMinus,  // Minus Key  (-)
        KeyPeriod,  // Period Key (.)
        KeyAttn,  // Attn key
        KeyCrSel,  // CrSel key
        KeyExSel,  // ExSel key
        KeyErEof,  // Erase EOF key
        KeyPlay,  // Play key
        KeyZoom,  // Zoom key
        KeyPA1,  // PA1 key
        KeyOem_Clear,  // Clear key

        Key_Codes_Count = 0xFF   // Count
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct KeyCodes
    {
    public:

        inline void add(KeyCode code, u32 systemCode)
        {
            _key.emplace(systemCode, code);
        }

        KeyCode get(u32 code)
        {
            auto iter = _key.find(code);
            if (iter != _key.end())
            {
                return iter->second;
            }

            return KeyUknown;
        }

    private:

        std::map<u32, KeyCode> _key;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace event
} //namespace v3d
