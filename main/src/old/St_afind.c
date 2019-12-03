/*
 *     Module Name       : ST_AFIND.C
 *
 *     Type              : Article finder states: StartArtFind_ST, ArtFindResult_ST.
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
 * 12-May-2000 Initial Release WinPOS                                  R.N.B.
 * --------------------------------------------------------------------------
 * 03-Dec-2001 Use Ascii Keymapping in StartArtFind_ST                   M.W.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop
                                            
#include "Template.h" /* Every source should use this one! */
                                            /* POS (library) include files.  */
#include "misc_tls.h"
#include "misc_tls.h"
#include "stri_tls.h"
                                            /* Toolsset include files.       */
#include "inp_mgr.h"
#include "scrn_mgr.h"
#include "state.h"
#include "tm_mgr.h"
#include "mapkeys.h"
                                            /* Application include files.    */
#include "pos_inp.h"
#include "pos_recs.h"
#include "pos_tm.h"
#include "pos_dflt.h"
#include "pos_func.h"
#include "pos_keys.h"
#include "pos_st.h"
#include "pos_txt.h"
#include "pos_vfy.h"
#include "WPos_mn.h"
#include "st_main.h"
#include "condll.h"
#include "registry.h"
#include "intrface.h"
#include "qry_mgr.h"
#include "scrl_mgr.h"
#include "pos_scrl.h"
#include "pos_errs.h"
#include "err_mgr.h"

#define  ART_DESC_INPUT_LENGTH    33


ARTICLE_DEF  article_record;                   /* record buffer */

static QRY_FIELD article_fields[] = {
  &article_record.descr,  QRY_STRING, NO_IDX   /* field_id 0 */
};

static QRY_OBJECT article_query = {
  ARTICLE_FNO,
  &article_record,
  sizeof(ARTICLE_DEF),
  sizeof(article_fields)/sizeof(QRY_FIELD),
  article_fields
};

static short    vfy_add_to_query  (_TCHAR *, short);
static short    clear_condition   (_TCHAR *, short);
static short    execute_query     (_TCHAR *, short);
static short    deinit_query      (_TCHAR *, short);
static short    vfy_enter_key     (_TCHAR *, short);
static short    handle_scroll     (_TCHAR *, short);

static CONTROL_ELEMENT StartArtFind_CTL[4];
static CONTROL_ELEMENT ArtFindResult_CTL[4];

static _TCHAR reg_cursor[11] = _T("");
static short  shift_pos_caret;
static short  qry_id = INVALID_QUERY;
TM_INDX selected_item;
static int   fetch_count = 0;
                                                      
_TCHAR last_condition[ART_DESC_INPUT_LENGTH+1];       /* for article descr */
long   art_no_from_finder = 0;        /* art_no returned by art finder     */
static STATE_OBJ *called_from_state = &StartArtFind_ST ;

static TM_ARTF_GROUP c_artf;

/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   StartArtFind_ST                                       */
/*-------------------------------------------------------------------------*/
static void StartArtFind_VW(void)
{
  static short no_cls_states[] = {
        ST_START_ART_FIND
        ,0
  };

  if( called_by_state(no_cls_states)!=SUCCEED ) {
    cls();
  }

  if (*reg_cursor == _T('\0')) {
    ReadEnvironmentValue(TREE_SCRN_SETTINGS, _T("ShowCursor"), reg_cursor, 10);
    if (_tcsicmp(reg_cursor, _T("vertical")) != 0) {
      shift_pos_caret = 0;
    }
    else {
      shift_pos_caret = 1;     /* shift caret 1 position with respect to cursor */
    }
  }

  ShiftPosCaret(shift_pos_caret);

  scrn_select_window(ART_FIND_HEADER_WINDOW);
  scrn_string_out(scrn_inv_TXT[27], 0, 0);

  scrn_select_window(ART_FIND_LIST_WINDOW);
  scrn_string_out(scrn_inv_TXT[28], 0, 0);

  if (qry_id == INVALID_QUERY) {
    qry_id = qry_register_query(&article_query);
  }

  if (ST_ART_FIND_RESULT != state_previous_number()
      && ST_START_ART_FIND != state_previous_number()) {
    *last_condition = _T('\0');
    called_from_state = state_previous_address();

    StartArtFind_CTL[0].next = called_from_state;
    ArtFindResult_CTL[0].next = called_from_state;
    ArtFindResult_CTL[1].next = called_from_state;
  }

  SetKeyMapping(ASCII_KEYS);

//  if( called_by_state(no_cls_states) == SUCCEED ) {
//    view_lan_state(FORCED_UPDATE); /* put at end because an error may occur */
//  }

  return;
} /* StartArtFind_VW */


static void StartArtFind_UVW(void)
{
  ShiftPosCaret((short)(shift_pos_caret-1));
  SetKeyMapping(NORMAL_KEYS);
  return;
} /* StartArtFind_UVW */


static VERIFY_ELEMENT StartArtFind_VFY[] =
{                              
  NO_KEY,               (void *)NULL,
  ENTER_KEY,            vfy_add_to_query,
  CLEAR_KEY,            clear_condition,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER Dartdescr33K33 =
{
  (INPUT_DISPLAY *)&dsp_art_descr,
  KEYBOARD_MASK,
  ART_DESC_INPUT_LENGTH, /* key length    */
  ART_DESC_INPUT_LENGTH, /* buffer length */
  (VERIFY_KEY *)&printing_char_upr
};

static PROCESS_ELEMENT StartArtFind_PROC[] =
{
  ENTER_KEY,   execute_query,
  NO_KEY,      deinit_query,
  UNKNOWN_KEY, (void *)NULL
};

static CONTROL_ELEMENT StartArtFind_CTL[] =
{
  NO_KEY,               NULL,  /* filled in view function */
  QUERY_NO_RECORDS,     &StartArtFind_ST,
  QUERY_OKAY,           &ArtFindResult_ST,
  UNKNOWN_KEY,          &StartArtFind_ST
};

extern STATE_OBJ StartArtFind_ST =
{                         
  ST_START_ART_FIND,
  StartArtFind_VW,
  DFLT_last_condition,
  &Dartdescr33K33,
  StartArtFind_VFY,
  StartArtFind_UVW,
  StartArtFind_PROC,
  StartArtFind_CTL
};

/*---------------------------------------------------------------------------*/
/*                           clear_condition                                 */
/*---------------------------------------------------------------------------*/
short clear_condition(_TCHAR *data, short key)
{
  *last_condition = _T('\0');
  vfy_clear_all_key(data, key);

  return UNKNOWN_KEY;
} /* clear_condition */

/*---------------------------------------------------------------------------*/
/*                          vfy_add_to_query                                 */
/*---------------------------------------------------------------------------*/
short vfy_add_to_query(_TCHAR *data, short key)
{
  _tcscpy(last_condition, data);
  qry_add_condition(qry_id, 0, data);

  return key;
} /* vfy_add_to_query */

/*---------------------------------------------------------------------------*/
/*                              execute_query                                */
/*---------------------------------------------------------------------------*/
short execute_query(_TCHAR *data, short key)
{
  short fetch_stat;

  fetch_count = 0;
  tm_reset_struct(TM_ARTF_NAME);

  if (qry_parse_query(qry_id) == PARSE_ERROR) {
    err_invoke(QUERY_PARSE_ERROR);
    return QUERY_NO_RECORDS;
  }

  display_working(YES);

  fetch_stat = qry_fetch_record(qry_id);
  while (fetch_stat == QRY_BUSY) {

    if (++fetch_count > TM_ARTF_MAXI) {
      break;
    }

    c_artf.art_no = article_record.art_no;
    c_artf.cont_sell_unit = article_record.cont_sell_unit;
    _tcscpy(c_artf.descr, article_record.descr);
    c_artf.mmail_no = article_record.mmail_no;
    _tcscpy(c_artf.pack_type, article_record.pack_type);
    c_artf.sell_pr = article_record.sell_pr;

    tm_appe(TM_ARTF_NAME, (_TCHAR *)&c_artf);

    fetch_stat = qry_fetch_record(qry_id);
  }

  display_working(NO);

  if (fetch_count==0) {
    err_invoke(QUERY_ZERO_RECORDS);
    return QUERY_NO_RECORDS;
  }
  else if (fetch_count > TM_ARTF_MAXI) {
    err_invoke(QUERY_TOO_MANY);
    return QUERY_NO_RECORDS;
  }

  return QUERY_OKAY;
} /* execute_query */
 
/*---------------------------------------------------------------------------*/
/*                          deinit_query                                     */
/*---------------------------------------------------------------------------*/
short deinit_query(_TCHAR *data, short key)
{
  qry_unregister_query(qry_id);
  qry_id = INVALID_QUERY;

  return key;
} /* deinit_query */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   ArtFindResult_ST                                      */
/*-------------------------------------------------------------------------*/
static void ArtFindResult_VW(void)
{
  int i;

  static short cls_states[] = {
    0
  };

  if( called_by_state(cls_states)==SUCCEED ) {
    cls();
  }

  SetShowCursor(FALSE);

  scrn_clear_window(ART_FIND_LIST_WINDOW);

  for (i=1; i<=fetch_count; i++) {
    scrl_append_line(ART_FIND_LIST_WINDOW, SCRL_ART_FIND_LINE, (short)i);
  }

  selected_item = 1;
  scrl_goto_line(ART_FIND_LIST_WINDOW, SCRL_ART_FIND_LINE, 1);
  scrl_redraw_lines(ART_FIND_LIST_WINDOW);

 // if( called_by_state(cls_states)!=SUCCEED ) {
 //   view_lan_state(FORCED_UPDATE); /* put at end because an error may occur */
 // }

  return;
} /* ArtFindResult_VW */


static void ArtFindResult_UVW(void)
{
  scrl_delete(ART_FIND_LIST_WINDOW);

  return;
} /* ArtFindResult_UVW */


static VERIFY_ELEMENT ArtFindResult_VFY[] =
{                              
  NO_KEY,               (void *)NULL,
  LINE_UP_KEY,          handle_scroll,
  LINE_DOWN_KEY,        handle_scroll,
  PAGE_UP_KEY,          handle_scroll,
  PAGE_DOWN_KEY,        handle_scroll,
  ART_FINDER_KEY,       (void *)NULL,
  ENTER_KEY,            vfy_enter_key,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER Dno_data0K0 =
{
  (INPUT_DISPLAY *)&dsp_keypos,
  KEYBOARD_MASK,
  0,
  0,
  (VERIFY_KEY *)&no_data
};

static PROCESS_ELEMENT ArtFindResult_PROC[] =
{
  ENTER_KEY,   (void *)NULL,
  NO_KEY,      deinit_query,
  UNKNOWN_KEY, (void *)NULL
};

static CONTROL_ELEMENT ArtFindResult_CTL[] =
{
  NO_KEY,               NULL,  /* filled in view function */
  ENTER_KEY,            NULL,  /* filled in view function */
  ART_FINDER_KEY,       &StartArtFind_ST,
  UNKNOWN_KEY,          &ArtFindResult_ST
};

extern STATE_OBJ ArtFindResult_ST =
{                         
  ST_ART_FIND_RESULT,
  ArtFindResult_VW,
  no_DFLT_2,
  &Dno_data0K0,
  ArtFindResult_VFY,
  ArtFindResult_UVW,
  ArtFindResult_PROC,
  ArtFindResult_CTL
};

/*---------------------------------------------------------------------------*/
/*                          vfy_enter_key                                    */
/*---------------------------------------------------------------------------*/
short vfy_enter_key(_TCHAR *data, short key)
{
  TM_ARTF_GROUP h_item;

  if (selected_item == tm_read_nth(TM_ARTF_NAME, (_TCHAR *)&h_item, selected_item)) {
    art_no_from_finder = h_item.art_no;
    return (key);
  }

  return (UNKNOWN_KEY);
} /* vfy_enter_key */

/*---------------------------------------------------------------------------*/
/*                          handle_scroll                                    */
/*---------------------------------------------------------------------------*/
short handle_scroll(_TCHAR *data, short key)
{
  int selected_item_sav = selected_item;
  /*                                                                       */
  /* Handles the scrolling of the article-list-window.                     */
  /*                                                                       */

  switch(key) {
    case LINE_UP_KEY:
      if (selected_item > 1) {
        selected_item--;
      }
      else {
        rs_error_tone();
      }
      break;
    case LINE_DOWN_KEY:
      if (selected_item < fetch_count) {
        selected_item++;
      }
      else {
        rs_error_tone();
      }
      break;
    case PAGE_UP_KEY:
      if (selected_item > 1) {
        selected_item -= scrl_get_numrows(ART_FIND_LIST_WINDOW);
        if (selected_item < 1) {
          selected_item = 1;
        }
      }
      else {
        rs_error_tone();
      }
      break;
    case PAGE_DOWN_KEY:
      if (selected_item < fetch_count) {
        selected_item += scrl_get_numrows(ART_FIND_LIST_WINDOW);
        if (selected_item > fetch_count) {
          selected_item = fetch_count;
        }
      }
      else {
        rs_error_tone();
      }
      break;
    default:
      break;
  }

  if (selected_item_sav != selected_item) {
    scrl_goto_line(ART_FIND_LIST_WINDOW, SCRL_ART_FIND_LINE, selected_item);
    scrl_redraw_lines(ART_FIND_LIST_WINDOW);
  }

  return(UNKNOWN_KEY);
} /* handle_scroll */
