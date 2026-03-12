#pragma once

// ── Frame / app resources ─────────────────────────────────────────────────────
#define IDR_MAINFRAME           128   // app icon + LoadFrame ID (MFC convention)

// ── String table ─────────────────────────────────────────────────────────────
#define IDS_APP_TITLE           101
#define IDS_READY               102
#define IDS_CHECKING            103
#define IDS_DONE                104
#define IDS_UNSAVED_CHANGES     105

// ── Toolbar button command IDs ────────────────────────────────────────────────
#define IDC_BTN_RUN_STOP        201
#define IDC_BTN_SAVE_HTML       202
#define IDC_BTN_SAVE_CFG        203
#define IDC_BTN_RELOAD_CFG      204
#define IDC_BTN_AUTOFIT         205
#define IDC_BTN_CFG_WIZ         206
#define IDC_BTN_INFO            207
#define IDC_BTN_INFO_SEP        209   // stretch separator before INFO (right-aligned)
#define IDC_BTN_EXIT            208

// ── Context menu ──────────────────────────────────────────────────────────────
#define IDR_CONTEXT_LIST        401
#define ID_CTX_ENABLE_ALL       401
#define ID_CTX_DISABLE_ALL      402
#define ID_CTX_ENABLE_TCP       405
#define ID_CTX_DISABLE_TCP      406
#define ID_CTX_ENABLE_UDP       407
#define ID_CTX_DISABLE_UDP      408

// ── Dialogs ───────────────────────────────────────────────────────────────────
#define IDD_ABOUT               1003
#define IDD_CONFIG_EDITOR       1010

// ── About dialog controls ─────────────────────────────────────────────────────
#define IDI_ICON_GITHUB         709   // icon_github.ico – used in About dialog
#define IDC_ABOUT_LINK_GITHUB   620
#define IDC_ABOUT_LINK_ICONS8   621
#define IDC_CFG_EDIT_NAME       601
#define IDC_CFG_EDIT_IP         602
#define IDC_CFG_COMBO_TYPE      603
#define IDC_CFG_BTN_ADD_SRV     604
#define IDC_CFG_BTN_REM_SRV     605
#define IDC_CFG_TAB             606
#define IDC_CFG_LIST            607
#define IDC_CFG_EDIT_PORT       608
#define IDC_CFG_COMBO_PROTO     609
#define IDC_CFG_EDIT_DESC       610
#define IDC_CFG_BTN_ADD_PORT    611
#define IDC_CFG_BTN_UPD_PORT    612
#define IDC_CFG_BTN_DEL_PORT    613




// ── Toolbar icon resource IDs (ICON resources – ICO files embedded in EXE) ───
#define IDI_ICON_RUN     701
#define IDI_ICON_STOP    702
#define IDI_ICON_HTML    703
#define IDI_ICON_SAVE    704   // icon_save.ico    – Guardar config
#define IDI_ICON_RELOAD  705
#define IDI_ICON_CFGEDIT 706   // icon_cfgedit.ico – Editor de configuración
#define IDI_ICON_INFO    707   // icon_info.ico    – Información
#define IDI_ICON_EXIT    708
