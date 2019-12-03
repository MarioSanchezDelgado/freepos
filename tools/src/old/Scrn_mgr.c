/*
 *     Module Name       : SCRN_MGR.C
 *
 *     Type              : Screen manager
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
 * 15-Nov-2001 Solved bug in scrn_clear_rest_of_line(). It deleted one
 *             character too much.                                     J.D.M.
 * --------------------------------------------------------------------------
 */
 
#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "mem_mgr.h"
#include "scrn_mgr.h"
#include "scrl_mgr.h"
#include "ConDLL.h"

/* VARIABLES                                                                 */
static short init_ok = FALSE;
static unsigned short current_screen_wnd = NO_WINDOW;

static short NR_OF_WINDOWS = 0;
#define WINDOW_NR_OK (window_nr < NR_OF_WINDOWS)
WINDOW *screen_wnd = NULL;

static void DoScroll(BOOL bUp, unsigned short window_nr, short headerlines);

/*******************************************************************************
Function    : scrn_init_windows
Description : Initialize windows structure
INPUT       : None
OUTPUT      : - short   Indication that windows are already initialized
                        SUCCEED, otherwise
*******************************************************************************/

short scrn_init_windows(short nr_of_windows)
{
  short status = FAIL;

  if ( init_ok == FALSE ) {
    /* Init screen window array */
    NR_OF_WINDOWS = nr_of_windows;
    memset(screen_wnd, NO_WINDOW, nr_of_windows*sizeof(WINDOW));
    scrl_init_windows();
    init_ok = TRUE;
    
    /* OK */
    status = SUCCEED;
  }

  return ( status );
} /* scrn_init_windows */


/*******************************************************************************
Function    : scrn_define_window
Description : Define window identified by window number
INPUT       : unsigned short  window_nr   The window number
              short           row     Row to begin
              short           col     Col to begin
              short           numrows   Number of rows
              short           numcols   Number of columns
              short           charsize    Character size, Large or not
              short           color     Color type , B_O_W or W_O_B
              short           border    Border type ,      ! NOT USED !
              unsigned short  scroll_yes_no Scroll or Not Scroll
OUTPUT      : - short Indication that window is illegal
                      SUCCEED otherwise
*******************************************************************************/

short scrn_define_window(unsigned short window_nr, short row, short col,
                         short numrows, short numcols, short charsize,
                         short color, short border, unsigned short scroll_yes_no,
                         short move_caret)
{
  short status = FAIL;

  if ( init_ok && WINDOW_NR_OK ) {                           /* define window */
    screen_wnd[window_nr].window_nr = window_nr;
    screen_wnd[window_nr].num_of_rows = numrows;
    screen_wnd[window_nr].num_of_cols = numcols;

//    screen_wnd[window_nr].num_of_rows = charsize==S40x12?numrows*GetSizingFactor():numrows;
//    screen_wnd[window_nr].num_of_cols = charsize==S40x12?numcols*GetSizingFactor():numcols;

    screen_wnd[window_nr].row = row;
    screen_wnd[window_nr].col = col;
    screen_wnd[window_nr].scroll_yn = scroll_yes_no;
    screen_wnd[window_nr].large_char = charsize;
    screen_wnd[window_nr].color_reverse = color;
    screen_wnd[window_nr].move_caret = move_caret;   /* input insertion point */
    if ( scroll_yes_no ) {
      status = scrl_define_window(window_nr);     /* window is used to scroll */
    }
    else {
      status = SUCCEED;
    }
  }

  return ( status );
} /* scrn_define_window */


/*******************************************************************************
Function    : scrn_select_window
Description : Select window defined by window number
INPUT       : unsigned short  window_nr   The window number
OUTPUT      : - short Indication that window is illegal
                      SCRN_WINDOW_UNDEFINED window_nr not defined
                      SUCCEED, otherwise
*******************************************************************************/

short scrn_select_window(unsigned short window_nr)
{
  short status = FAIL;

  if ( init_ok && WINDOW_NR_OK )  {
    if ( screen_wnd[window_nr].window_nr != NO_WINDOW ) {
      current_screen_wnd = window_nr;       /* make window the current window */
      status = SUCCEED;
    }
    else {
      status = SCRN_WINDOW_UNDEFINED;
    }
  }

  return ( status );
}  /* scrn_select_window */


/*******************************************************************************
Function    : scrn_get_info
Description : Returns window information in WINDOW structure
INPUT       : unsigned short  window_nr   The window number
              WINDOW *        win_info    A pointer to a window structure
OUTPUT      : short           SCRN_WINDOW_UNDEFINED window_nr not defined
                              SUCCEED, otherwise
*******************************************************************************/

short scrn_get_info(unsigned short window_nr, WINDOW *win_info)
{
  if ( init_ok && WINDOW_NR_OK && screen_wnd[window_nr].window_nr != NO_WINDOW ) {
    memcpy(win_info, &screen_wnd[window_nr], sizeof(WINDOW));
    return(SUCCEED);
  }
  else {
    return(SCRN_WINDOW_UNDEFINED);
  }
} /* scrn_get_info */


/*******************************************************************************
Function    : scrn_clear_window
Description : Clear window defined by window number
INPUT       : unsigned short  window_nr   The window number
OUTPUT      : - short Indication that window is illegal
                      SUCCEED, otherwise
*******************************************************************************/

short scrn_clear_window(unsigned short window_nr)
{
  short status = FAIL;
  short i;
  _TCHAR buf[81];

  if ( init_ok && WINDOW_NR_OK ) {
    /* Select current window */
    if ( window_nr != current_screen_wnd ) {
      scrn_select_window(window_nr);
    }
    
    /* Fill string with spaces */
    ch_memset(buf, _T(' '), sizeof(buf));
    buf[min(screen_wnd[window_nr].num_of_cols,80)] = _T('\0');
    
    /* Write string */
    for(i=0;i<screen_wnd[window_nr].num_of_rows;i++) {
      scrn_string_out(buf, i, 0);
    }
    
    /* OK */
    status = SUCCEED;
  }

  return ( status );
} /* scrn_clear_window */


/*******************************************************************************
Function    : scrn_close_windows
Description : Close all windows
INPUT       : None
OUTPUT      : - short SUCCEED
*******************************************************************************/

short scrn_close_windows()
{
  if ( init_ok == TRUE ) {
   init_ok = FALSE;
   scrl_close_windows();
   current_screen_wnd = NO_WINDOW;
  }

  return ( SUCCEED );
} /* scrn_close_windows */


/*******************************************************************************
Function    : scrn_string_out
Description : Print a string on the defined line of the screen
INPUT       : _TCHAR * text     text to be printed on the screen
              short row      row on which to print text
              short col      col on which to print text
OUTPUT      : - short Indication that window is illegal
                      Indication that Row or Column is outside window-range
                      SUCCEED, otherwise
*******************************************************************************/

short scrn_string_out(_TCHAR *text, short row, short col)
{
  short status, inverse_yn, window_nr;
  short x,y;
  WORD flags;

  status = FAIL;
  window_nr = current_screen_wnd;
  inverse_yn = screen_wnd[window_nr].color_reverse == BLACK_ON_WHITE;

  if ( init_ok && WINDOW_NR_OK ) {
    if ( !( row >= screen_wnd[window_nr].num_of_rows ||
            col >= screen_wnd[window_nr].num_of_cols || row < 0 || col < 0 )) {
                                /* window is not illegal and not out of range */
      /* Cursor Position */
      if (screen_wnd[window_nr].large_char == S40x12) {
        y = screen_wnd[window_nr].row+row*GetSizingFactor();
        x = screen_wnd[window_nr].col+col*GetSizingFactor();
      }
      else {
        y = screen_wnd[window_nr].row+row;
        x = screen_wnd[window_nr].col+col;
      }
      scrn_set_csr(y,x);
      
      /* Determine correct color */
      if ( *text == INVERSE_ON_CONTROL_CHR || *text == INVERSE_OFF_CONTROL_CHR ) {
        if ( *text == INVERSE_ON_CONTROL_CHR ) {
          inverse_yn = !inverse_yn;
        }
        text++;
      }

      /* Determine flags */
      flags = TEXT_NORMAL;
      if ( inverse_yn ) {
        flags |= TEXT_REVERSE;
      }
      if ( screen_wnd[window_nr].large_char == S40x12) {
        flags |= TEXT_BIG;
      }
      if ( screen_wnd[window_nr].move_caret == CARET) {
        flags |= MOVE_CARET;
      }
      
      /* Write String */
      DisplayString(text, x, y, flags);
      
      /* Finished */
      status = SUCCEED;
    }
  }

  return ( status );
} /* scrn_string_out */


/*******************************************************************************
Function    : scrn_get_string
Description : Gets a string from the defined line of the screen
INPUT       : _TCHAR *text_out  buffer to store the text
              short length   number of characters to retrieve
              short row      row on which to fetch text
              short col      col on which to fetch text
OUTPUT      : - short Indication that window is illegal
                      Indication that Row or Column is outside window-range
                      SUCCEED, otherwise
*******************************************************************************/

short scrn_get_string(short row, short col, _TCHAR *text_out, short length)
{
  short status, window_nr;
  short x,y,i;  /* y=row, x=col */
  WORD  flags;
  short inverse_yn, inverse_yn_win, size_factor;

  status = FAIL;
  window_nr = current_screen_wnd;

  if ( init_ok && WINDOW_NR_OK ) {
    if ( !( row >= screen_wnd[window_nr].num_of_rows ||
            col >= screen_wnd[window_nr].num_of_cols || row < 0 || col < 0 )) {
                                /* window is not illegal and not out of range */
                           /* check if length does not cross border of window */
      if ((col+length) >= screen_wnd[window_nr].num_of_cols) {
        length = screen_wnd[window_nr].num_of_cols - col - 1;
      }

      /* Cursor Position */
      size_factor = 1;
      if (screen_wnd[window_nr].large_char == S40x12) {
        size_factor = GetSizingFactor();
      }

      y = screen_wnd[window_nr].row+row*size_factor;
      x = screen_wnd[window_nr].col+col*size_factor;

      /* Get String */
      RetrieveString(x, y, text_out, length, &flags);
      
      inverse_yn = ((flags&TEXT_REVERSE)!=0);
      inverse_yn_win = (screen_wnd[window_nr].color_reverse == BLACK_ON_WHITE);

      if (inverse_yn != inverse_yn_win) {
        for (i=_tcslen(text_out); i>0; i--) {
          text_out[i] = text_out[i-1];
        }
        text_out[0] = INVERSE_ON_CONTROL_CHR;
      }

      /* Finished */
      status = SUCCEED;
    }
  }

  return ( status );
} /* scrn_get_string */

/*******************************************************************************
Function    : scrn_string_out_curr_location
Description : Print a string on the line at the current location
INPUT       : _TCHAR * text     text to be printed on the screen
OUTPUT      : - short SCRN_ILLEGAL_WINDOW illegal window
                      SCRN_INVALID_LOCATION Row or Column outside window-range
                      SUCCEED, otherwise
*******************************************************************************/
short scrn_string_out_curr_location(_TCHAR *text)
{
  short crow, ccol, status;

  scrn_get_csr(&crow, &ccol);                             /* locate the cursor */

                            /* put text on the screen on the current location */
  status = scrn_string_out(text, crow, ccol);

  return (status);
} /* scrn_string_out_curr_location */



/*******************************************************************************
Function    : scrn_clear_rest_of_line
Description : Clear the rest of the line
INPUT       : unsigned short window_nr     window number
              short          row           row to be partly cleared
              short          col           col to be partly cleared
OUTPUT      : - short Indication that window is illegal
                      Indication that Row or Column is outside window-range
                      SUCCEED, otherwise
*******************************************************************************/

short scrn_clear_rest_of_line(unsigned short window_nr, short row, short col)
{
  short status = FAIL;
  unsigned short current_screen_wnd_nr;
  _TCHAR screendata[83];

  current_screen_wnd_nr = current_screen_wnd;

  if ( init_ok && WINDOW_NR_OK ) {
    status = SUCCEED;

    if ( !( row >= screen_wnd[window_nr].num_of_rows ||
            col >= screen_wnd[window_nr].num_of_cols || row < 0 || col < 0 )) {
      if ( window_nr != current_screen_wnd ) {
        scrn_select_window( window_nr );
      }

      /* Number of spaces to be put on the rest of the line to 'clear' it, is
         copied in screendata */
      memset(screendata, 0, sizeof(screendata));
      ch_memset(screendata, _T(' '),
             (screen_wnd[current_screen_wnd].num_of_cols-col)*sizeof(_TCHAR));

      scrn_string_out(screendata, row, col); /* MWN : Write string to screen!! */

      scrn_set_csr(row,col);                /* Set cursor at position row, col */
      if ( current_screen_wnd_nr != window_nr ) {
        scrn_select_window( current_screen_wnd_nr );
      }
    }
  }

  return ( status );
} /* scrn_clear_rest_of_line */


/*******************************************************************************
Function    : scrn_line_down
Description : Scroll line down for this window on the screen
INPUT       : unsigned short window_nr     window number
              short          headerlines   number of header lines
OUTPUT      : None
*******************************************************************************/

void scrn_line_down(unsigned short window_nr, short headerlines)
{
  DoScroll( TRUE, window_nr, headerlines );
} /* scrn_line_down */


/*******************************************************************************
Function    : scrn_line_up
Description : Scroll line up for this window on the screen
INPUT       : unsigned short window_nr     window number
              short          headerlines   number of header lines
OUTPUT      : None
*******************************************************************************/

void scrn_line_up(unsigned short window_nr, short headerlines)
{
  DoScroll( FALSE, window_nr, headerlines );
} /* scrn_line_up */


/*******************************************************************************
Function    : scrn_get_current_window
Description : Which is the current window
INPUT       : None
OUTPUT      : None
*******************************************************************************/

short scrn_get_current_window()
{
  return ( current_screen_wnd );         /* which window is the current one ? */
}  /* scrn_get_current_window */


/*******************************************************************************
Function    : scrn_set_csr
Description : Set cursor position
INPUT       : row, collumn
OUTPUT      : None
*******************************************************************************/

void scrn_set_csr(unsigned short row, unsigned short col)
{
  SetCursorPosition( col, row,
    (short)(screen_wnd[current_screen_wnd].move_caret == CARET ? 1 : 0));
}


/*******************************************************************************
Function    : scrn_get_csr
Description : Get cursor position
INPUT       : None
OUTPUT      : row, collumn
*******************************************************************************/

void scrn_get_csr(unsigned short *row, unsigned short *col)
{
  GetCursorPosition( col, row );
}


/*******************************************************************************
Function    : DoScroll
Description : Scrolls a window
INPUT       : BOOL bUp    Indicates scrolling up
              unsigned short window_nr     window number
              short          headerlines   number of header lines
OUTPUT      : None
*******************************************************************************/

static void DoScroll( BOOL bUp, unsigned short window_nr, short headerlines )
{
  short flags = 0;

  if (screen_wnd[window_nr].color_reverse == BLACK_ON_WHITE) {
    flags |= TEXT_REVERSE;
  }

  ScrollScreenPart(
    (short)(bUp?-1:1),                               /* Displacement in lines */
    (short)(screen_wnd[window_nr].col),                                  /* X */
    (short)(screen_wnd[window_nr].row + headerlines + (bUp?1:0)),        /* Y */
    (short)(screen_wnd[window_nr].num_of_cols),                      /* Width */
    (short)(screen_wnd[window_nr].num_of_rows - headerlines - 1),   /* Height */
    flags );                                                         /* Flags */
} /* DoScroll */


/*---------------------------------------------------------------------------*/
/*                           scrn_draw_line                                  */
/*---------------------------------------------------------------------------*/
/* Draws a line of length characters 'ch', starting at row, col.             */
/*---------------------------------------------------------------------------*/
void scrn_draw_line(short row, short col, short length, _TCHAR ch)
{
  short   row_sav, col_sav;
  _TCHAR *line;

  line = (_TCHAR*)mem_allocate((length+1)*sizeof(_TCHAR));

  if (line) {
    scrn_get_csr(&row_sav, &col_sav);
    ch_memset(line, ch, length*sizeof(_TCHAR));
    line[length] = _T('\0');
    scrn_string_out(line, row, col);
    scrn_set_csr(row_sav, col_sav);
    mem_free(line);
  }
} /* scrn_draw_line */
