/*
 *     Module Name       : SCRN_MGR.H
 *
 *     Type              : Include file screen manager
 *                         
 *
 *     Author/Location   : Getronics, Distribution & Retail, Nieuwegein
 *
 *     Copyright Makro International AG
 *               Aspermonstrasse 24
 *               7006 CHUR
 *               Switzerland
 *
 * --------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * --------------------------------------------------------------------------
 * DATE        REASON                                                  AUTHOR
 * --------------------------------------------------------------------------
 * 21-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef SCRN_MGR_H
#define SCRN_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/* MACROS                                                                     */
#define NO_WINDOW               255 
#define INVERSE_ON_CONTROL_CHR  _T('\1')
#define INVERSE_OFF_CONTROL_CHR _T('\2')
#define INVERSE_ON_STRING       _T("\1")
#define INVERSE_OFF_STRING      _T("\2")
#define INVERSE_ON              1
#define INVERSE_OFF             0
#define SCROLL                  1
#define NO_SCROLL               0
#define CARET                   1
#define NO_CARET                0
#define CURSOR_OFF              0x2000
#define CURSOR_ON               0x0707

#define S80x25                  0
#define S40x12                  1
#define WHITE_ON_BLACK          0
#define BLACK_ON_WHITE          1 
#define BDR_NONE                0

#define SCRN_WINDOW_UNDEFINED   -107


/* TYPEDEF STRUCTURE                                                          */
typedef struct
{
  unsigned short window_nr;
  short row;
  short col;
  short num_of_rows;
  short num_of_cols;
  unsigned short scroll_yn;
  short large_char;
  short color_reverse;
  short move_caret;
} WINDOW;

extern WINDOW *screen_wnd;

/* FUNCTIONS                                                                  */
extern short scrn_init_windows(short);
extern short scrn_define_window(unsigned short, short, short, short, short, 
                                 short, short, short, unsigned short, short);
extern short scrn_select_window(unsigned short);
extern short scrn_string_out(_TCHAR *, short, short);
extern short scrn_get_string(short, short, _TCHAR *, short);
extern short scrn_string_out_curr_location(_TCHAR *);
extern short scrn_clear_rest_of_line(unsigned short,short, short);
extern short scrn_clear_window(unsigned short);
extern short scrn_close_windows(void);
extern short scrn_get_current_window(void);
extern short scrn_get_info(unsigned short, WINDOW *);
extern void  scrn_line_down(unsigned short, short);
extern void  scrn_line_up(unsigned short, short);
extern void  scrn_set_csr(unsigned short, unsigned short);
extern void  scrn_get_csr(unsigned short *, unsigned short *);
extern void  scrn_draw_line(short, short, short, _TCHAR);

#ifdef __cplusplus
}
#endif

#endif
