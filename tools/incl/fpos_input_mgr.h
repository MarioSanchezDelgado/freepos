/*
 *     Module Name       : fpos_input_mgr.h
 *
 *     Type              : Include file input manager
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

#ifndef FPOS_INPUT_MGR_H
#define FPOS_INPUT_MGR_H

#ifdef __cplusplus
//#include "OPOSKeyboard.h"
//extern CPosKeyboard* init_keyb(void (*)(void));
#endif
#define DEINIT_AND_INIT_KEYB_IMPLEMENTED /* For use in SetupKeys!! */

#ifdef __cplusplus
extern "C" {
#endif

#define BUFFER_LENGTH   128

#define OCIA1_MASK         1
#define OCIA2_MASK         2
#define KEYBOARD_MASK      4

#define KEYLOCK_N_MASK     8
#define KEYLOCK_S_MASK    16
#define KEYLOCK_L_MASK    32
#define KEYLOCK_X_MASK    64

#define DOUBLE_NULL_KEY        0x3610                /* dummy convert key */

#define INP_TOO_MANY_KEYS         -202
#define INP_KBD_ERROR             -203
#define INP_POWER_FAILURE         -204           /* POWER FAILURE IN INPUT */
#define INP_STATE_ABORT_REQUEST   -205
#define INP_NO_DATA                0x0000

#define MAX_OCIA_DATA_LENGTH  50

typedef struct keyverify
{ 
  short (*fn)(struct keyverify *,char *,short);
} VERIFY_KEY;

typedef struct keydisp
{
  short (*fn)(struct keydisp *,char *);  /* ANY display function          */
} INPUT_DISPLAY;

typedef struct inp_control
{
  INPUT_DISPLAY  *display;
  short           device_mask;
  short           key_length;
  int             buffer_length;
  VERIFY_KEY     *filter;
} INPUT_CONTROLLER;

extern short is_function_key(short);
extern short inp_init(short, void (*)(void));
extern void  inp_exit(short);
extern short inp_get_data(INPUT_CONTROLLER *, char *);
extern short inp_pick_up_key(short);         /* application compatibility */
extern void  inp_abort_data(void);
extern short inp_data_avail(short);
extern short inp_peek_key(short);
extern void  deinit_keyb(void);

extern void  rs_wait_for_any_input(void);
extern short rs_wait_for_key_in_set(short *);

extern int vfy_preamble(char);
#define ID_BARCD_39 'B'

/* KEYLOCK */
extern short keylock_attached;
extern short (*KeyLockApprovalFunc)(short,short);  /* keylock approval function  */
                                              /* used for keylock emulation */
extern short rs_keylock_position(void);
extern void  rs_wait_keylock_pos(short);
#define      rs_wait_for_key_in_S()    rs_wait_keylock_pos(KEYLOCK_SUPERVISOR)
#define      rs_wait_for_key_in_N()    rs_wait_keylock_pos(KEYLOCK_NORMAL)
#define      rs_wait_for_key_in_L()    rs_wait_keylock_pos(KEYLOCK_LOCK)
#define      rs_wait_for_key_in_X()    rs_wait_keylock_pos(KEYLOCK_EXCEPTION)


/* CASH DRAWER */
#define RS_INIT_ERROR       -1001
#define RS_HANDLE_ERROR     -1002
#define RS_DRAWER_FAILURE   -1003
#define CASH_DRAWER1         1
#define DRAWER_OPEN          0
#define DRAWER_CLOSED        1
extern short rs_cash_drawer_status(short);
extern short rs_open_cash_drawer(short);

/* CUSTOMER DISPLAY */
#define CDSP_RIGHT_JUSTIFY          -1
extern short cdsp_clear(void);
extern short cdsp_clear_line(short);
extern short cdsp_write_string(char *, short, short);
extern short cdsp_write_abs_string(char *);
extern short cdsp_set_csr(short, short);

/* SEVERAL */
#define POWER_OK             0         
#define POWER_FAIL           1
#define POWER_DOWN           2

extern int tone[];
//typedef struct
//{
//  BYTE *identifier;
//  LONG size;
//} RDI_RETURN_TYPE;

extern void  rs_error_tone(void);
extern short rs_power_failed(void);
extern short rs_power_down(void);


/*
  -----------------------------------------------------------------------
   Module name     : inp_disp.h  - Template Input Display
  -----------------------------------------------------------------------
*/

#define MAX_FORMAT_LENGTH     80

typedef struct tdisp1 {
  short (*fn)(struct tdisp1*,char *);  /* one of the display functions      */
  char *format;                       /* for example "_____.__"            */
  char *cover;                        /* characters of above to cover: "_" */
  short window;                        /* window to display it in           */
  short row,col;                       /* display position of first '_'     */
} TEMPLATE_DISPLAY1;

typedef struct tdisp2 {
  short (*fn)(struct tdisp2*,char *);  /* one of the display functions      */
  char *format;                       /* for example "_____.__"            */
  char *cover;                        /* characters of above to cover: "_" */
  short  window;                       /* window to display it in           */
  short  row,col;                      /* display position of first '_'     */
  char password_char;                /* Character to show instead of key  */
} TEMPLATE_PASSWORD;

extern  void (*fn_inp_idle)(void);     /* External hookup variable      */

extern  short r2l_display(TEMPLATE_DISPLAY1 *display,char *data);
extern  short l2r_display(TEMPLATE_DISPLAY1 *display,char *data);
extern  short l2r_password(TEMPLATE_PASSWORD *display,char *data);
extern  short r2l_password(TEMPLATE_PASSWORD *display,char *data);
extern  void  format_display(TEMPLATE_DISPLAY1 *, char *);
extern  void  format_display_passwrd(TEMPLATE_PASSWORD *, char *);

#ifdef __cplusplus
}
#endif

#endif
