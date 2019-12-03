/*
 *     Module Name       : MAP_KEYS.H
 *
 *     Type              : Input function key definitions
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
 * 05-jan-2000 Initial Release WinPOS                                    M.W.
 * --------------------------------------------------------------------------
 */

#ifndef __MAP_KEYS_H__
#define __MAP_KEYS_H__


#ifdef __cplusplus
extern "C" {
#endif

#define LEN_KEY_BUFFER   80

extern short  InitKeyMapTable(void);
extern int    MapKeyToFunction(int);

typedef struct
{
  _TCHAR *Description;
  short   Value;
} KEYDEF;

extern KEYDEF *SetKeyMapping(short);

#define KEYLOCK_NONE        -1         /* No keylock (between positions)  */

                                 /* Implemented as a makro for efficiency */
extern short  *TablePointer;
#define MapKeyToFunction(key)         TablePointer[(int)key]

enum DEFINED_KEYMAPS {
   NORMAL_KEYS
  ,ASCII_KEYS
  ,NUMBER_KEYMAPS   /* always last */
};

/* Translate Keys from numpad to normal numeric keys                      */
/* Keys from numpad are in hex in registry                                */
                                                     /* NON-FUNCTION KEYS */
#define KEY_0                  (int)'0'
#define KEY_1                  (int)'1'
#define KEY_2                  (int)'2'
#define KEY_3                  (int)'3'
#define KEY_4                  (int)'4'
#define KEY_5                  (int)'5'
#define KEY_6                  (int)'6'
#define KEY_7                  (int)'7'
#define KEY_8                  (int)'8'
#define KEY_9                  (int)'9'
#define DOT_KEY                (int)'.'

#define KEY_A                  (int)'A'
#define KEY_B                  (int)'B'
#define KEY_C                  (int)'C'
#define KEY_D                  (int)'D'
#define KEY_E                  (int)'E'
#define KEY_F                  (int)'F'
#define KEY_G                  (int)'G'
#define KEY_H                  (int)'H'
#define KEY_I                  (int)'I'
#define KEY_J                  (int)'J'
#define KEY_K                  (int)'K'
#define KEY_L                  (int)'L'
#define KEY_M                  (int)'M'
#define KEY_N                  (int)'N'
#define KEY_O                  (int)'O'
#define KEY_P                  (int)'P'
#define KEY_Q                  (int)'Q'
#define KEY_R                  (int)'R'
#define KEY_S                  (int)'S'
#define KEY_T                  (int)'T'
#define KEY_U                  (int)'U'
#define KEY_V                  (int)'V'
#define KEY_W                  (int)'W'
#define KEY_X                  (int)'X'
#define KEY_Y                  (int)'Y'
#define KEY_Z                  (int)'Z'
#define KEY_PROCENT            (int)'%'
                                                         /* FUNCTION KEYS */

  /* Add new functions at bottom, value must be larger than NO_MAPPED_KEY */
#define NO_MAPPED_KEY          300

#define PAGE_UP_KEY            301
#define PAGE_DOWN_KEY          302
#define LINE_UP_KEY            303
#define LINE_DOWN_KEY          304
#define PRINTER_UP_KEY         305
#define OPEN_DRAWER_KEY        306
#define OPERATOR_KEY           307
#define VOID_LAST_ITEM_KEY     308
#define REPEAT_LAST_ITEM_KEY   309
#define VOID_LINE_KEY          310
#define VOID_INVOICE_KEY       311
#define CLEAR_KEY              312
#define NEW_PAGE_KEY           313
#define NO_KEY                 314
#define DISCOUNT_KEY           315
#define CREDIT_KEY             316
#define SUB_TOTAL_KEY          317
#define TOTAL_KEY              318
#define TIMES_KEY              319
#define ENTER_KEY              320

#define PAYMENT_WAY_0_KEY      321
#define PAYMENT_WAY_1_KEY      322
#define PAYMENT_WAY_2_KEY      323
#define PAYMENT_WAY_3_KEY      324
#define PAYMENT_WAY_4_KEY      325
#define PAYMENT_WAY_5_KEY      326
#define PAYMENT_WAY_6_KEY      327
#define PAYMENT_WAY_7_KEY      328
#define PAYMENT_WAY_8_KEY      329
#define PAYMENT_WAY_9_KEY      330

#define TOGGLE_SIGN_KEY        331
#define BACKSPACE_KEY          332

#define ART_FINDER_KEY         333
#define SAVE_INVOICE_KEY       334
#define SEL_PRINTER_KEY        335

#define TURKEY_KEY             336 /* FMa - Vale Pavos */
#define QUEUEBUSTING_KEY       337 /* 25-Set-2012 acm - */
#define HORECA_KEY             338 /* v3.4.8 acm - */
//#define SEL_DOCUMENT_KEY       339
#define ENTER_KEY_TOTAL        340

#define PREV_TOTAL_KEY         341
#define PREV_AMNT_ENOUGH       342
#define DOCID_FOUND            343

#define PASSDAY700_KEY         345  //mlsd



/* Add new function keys here.                        */
/* NOTE: range 500 to 600 is reserved for pos_keys.h  */

#define OCIA1_DATA             1000
#define OCIA2_DATA             1001
#define M4430_DATA             1002
#define MSR_DATA               1003

/* NOTE: do not add functions after the keylocks! This is because to         */
/* determine  if a keylock key was pressed the following formula is used:    */
/*    if (PressedKey / KEYLOCK_LOCK) keylock_is_used;                        */
/* (Only relevant when no keylock is used).                                  */

#define KEYLOCK_LOCK           1024       /* 0 Locked                        */
#define KEYLOCK_NORMAL         1025       /* 1 Normal                        */
#define KEYLOCK_SUPERVISOR     1026       /* 2 Supervisor                    */
#define KEYLOCK_EXCEPTION      1027       /* 3 Exception                     */

/* Do NOT add function keys larger than the KEYLOCKS! */

#ifdef __cplusplus
}
#endif

#endif  /* __MAP_KEYS_H__ */
