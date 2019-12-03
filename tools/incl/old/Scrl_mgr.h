/*
 *     Module Name       : SCRL_MGR.H
 *
 *     Type              : Include file scroll manager
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
 * 05-Dec-2001 Changes for Article Finder.                               M.W.
 * --------------------------------------------------------------------------
 */

#ifndef SCRL_MGR_H
#define SCRL_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

//extern const short SCRL_MAX_FUNCTIONS;
extern void (*scrl_functions[])(short);

/* MACROS                                                                     */
#define NR_OF_SCROLL_WINDOWS   3
#define BNDRY_CHK_OK ((function_nr >= 0) && (function_nr < SCRL_MAX_FUNCTIONS))

/* TYPEDEF                                                                    */
typedef struct element
{
  short elemid;
  short function_nr;
  struct element *next;
} ELEM;

typedef struct scroll_set_of_data
{ 
  short nr_element;
  ELEM *first;
  ELEM *last;
} SCRL_SET;

typedef struct scrolling_window
{
  unsigned short window_nr;
  short top;
  short bottom;
  SCRL_SET *line;
  SCRL_SET *header;
} SCROLL_WINDOW;


/* MACROS                                                                     */
#define SCRL_UNKNOWN_LINE                 -1504

/* FUNCTIONS                                                                  */

extern void  scrl_init(void);
extern short scrl_add_fn(void (*)(), short);
extern short scrl_add_line (unsigned short,short,short);
extern short scrl_append_line (unsigned short,short,short);
extern short scrl_insert_line(unsigned short, short, short, short, short);
extern short scrl_update_line (unsigned short,short,short);
extern short scrl_delete_line (unsigned short,short,short);
extern short scrl_goto_line (unsigned short,short,short);
extern short scrl_redraw_lines (unsigned short);
extern short scrl_page_up (unsigned short);
extern short scrl_page_down (unsigned short);
extern short scrl_line_up (unsigned short);
extern short scrl_line_down (unsigned short);
extern short scrl_get_numrows (unsigned short);
extern short scrl_update_header_line (unsigned short,short,short);
extern short scrl_add_header_line (unsigned short,short,short);
extern short scrl_delete (unsigned short);
extern short scrl_init_windows (void);
extern short scrl_define_window (unsigned short);
extern short scrl_close_windows (void);
short  scrl_modify_id(short, short, short);

#ifdef __cplusplus
}
#endif

#endif
