/*
 *     Module Name       : Mapkeys.c
 *
 *     Type              : Key mapping from registry
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
 * 05-Jan-2000 Initial Release WinPOS                                    M.W.
 * --------------------------------------------------------------------------
 * 21-Sep-2001 Added Pending Invoice.                                    M.W.
 * --------------------------------------------------------------------------
 * 03-Dec-2001 Added Article Finder.                                     M.W.
 * --------------------------------------------------------------------------
 * 05-Dec-2001 Added Spanish keys for Article Finder.                    M.W.
 * --------------------------------------------------------------------------
 * 22-Jan-2002 Removed Spanish keys for Article Finder (NOT USED).       M.W.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>

#include "mapkeys.h"
#include "registry.h"
#include "ConDLL.h"

short ReadFunctionKeys(void);
short FillTable(_TCHAR *, short);
short HexStringtoDec(_TCHAR *);

short TranslateTable[MAX_KEYCODE];
short AsciiTable[MAX_KEYCODE];

KEYDEF KeyMap[] = {
  { _T("PAGE_UP_KEY"),          PAGE_UP_KEY            } /* 000 */
 ,{ _T("PAGE_DOWN_KEY"),        PAGE_DOWN_KEY          } /* 001 */
 ,{ _T("LINE_UP_KEY"),          LINE_UP_KEY            } /* 002 */
 ,{ _T("LINE_DOWN_KEY"),        LINE_DOWN_KEY          } /* 003 */
 ,{ _T("PRINTER_UP_KEY"),       PRINTER_UP_KEY         } /* 004 */
 ,{ _T("OPEN_DRAWER_KEY"),      OPEN_DRAWER_KEY        } /* 005 */
 ,{ _T("OPERATOR_KEY"),         OPERATOR_KEY           } /* 006 */
 ,{ _T("VOID_LAST_ITEM_KEY"),   VOID_LAST_ITEM_KEY     } /* 007 */
 ,{ _T("REPEAT_LAST_ITEM_KEY"), REPEAT_LAST_ITEM_KEY   } /* 008 */
 ,{ _T("VOID_LINE_KEY"),        VOID_LINE_KEY          } /* 009 */
 ,{ _T("VOID_INVOICE_KEY"),     VOID_INVOICE_KEY       } /* 010 */
 ,{ _T("CLEAR_KEY"),            CLEAR_KEY              } /* 011 */
 ,{ _T("NEW_PAGE_KEY"),         NEW_PAGE_KEY           } /* 012 */
 ,{ _T("NO_KEY"),               NO_KEY                 } /* 013 */
 ,{ _T("DISCOUNT_KEY"),         DISCOUNT_KEY           } /* 014 */
 ,{ _T("CREDIT_KEY"),           CREDIT_KEY             } /* 015 */
 ,{ _T("SUB_TOTAL_KEY"),        SUB_TOTAL_KEY          } /* 016 */
 ,{ _T("TOTAL_KEY"),            TOTAL_KEY              } /* 017 */
 ,{ _T("TIMES_KEY"),            TIMES_KEY              } /* 018 */
 ,{ _T("ENTER_KEY"),            ENTER_KEY              } /* 019 */
 ,{ _T("PAYMENT_WAY_0_KEY"),    PAYMENT_WAY_0_KEY      } /* 020 */
 ,{ _T("PAYMENT_WAY_1_KEY"),    PAYMENT_WAY_1_KEY      } /* 021 */
 ,{ _T("PAYMENT_WAY_2_KEY"),    PAYMENT_WAY_2_KEY      } /* 022 */
 ,{ _T("PAYMENT_WAY_3_KEY"),    PAYMENT_WAY_3_KEY      } /* 023 */
 ,{ _T("PAYMENT_WAY_4_KEY"),    PAYMENT_WAY_4_KEY      } /* 024 */
 ,{ _T("PAYMENT_WAY_5_KEY"),    PAYMENT_WAY_5_KEY      } /* 025 */
 ,{ _T("PAYMENT_WAY_6_KEY"),    PAYMENT_WAY_6_KEY      } /* 026 */
 ,{ _T("PAYMENT_WAY_7_KEY"),    PAYMENT_WAY_7_KEY      } /* 027 */
 ,{ _T("PAYMENT_WAY_8_KEY"),    PAYMENT_WAY_8_KEY      } /* 028 */
 ,{ _T("PAYMENT_WAY_9_KEY"),    PAYMENT_WAY_9_KEY      } /* 029 */
 ,{ _T("TOGGLE_SIGN_KEY"),      TOGGLE_SIGN_KEY        } /* 030 */
 ,{ _T("DOT_KEY"),              DOT_KEY                } /* 031 */
 ,{ _T("KEY_0"),                KEY_0                  } /* 032 */
 ,{ _T("KEY_1"),                KEY_1                  } /* 033 */
 ,{ _T("KEY_2"),                KEY_2                  } /* 034 */
 ,{ _T("KEY_3"),                KEY_3                  } /* 035 */
 ,{ _T("KEY_4"),                KEY_4                  } /* 036 */
 ,{ _T("KEY_5"),                KEY_5                  } /* 037 */
 ,{ _T("KEY_6"),                KEY_6                  } /* 038 */
 ,{ _T("KEY_7"),                KEY_7                  } /* 039 */
 ,{ _T("KEY_8"),                KEY_8                  } /* 040 */
 ,{ _T("KEY_9"),                KEY_9                  } /* 041 */
 ,{ _T("VIRTUAL_KEYLOCK_EX"),   KEYLOCK_EXCEPTION      } /* 042 */
 ,{ _T("VIRTUAL_KEYLOCK_L"),    KEYLOCK_LOCK           } /* 043 */
 ,{ _T("VIRTUAL_KEYLOCK_R"),    KEYLOCK_NORMAL         } /* 044 */
 ,{ _T("VIRTUAL_KEYLOCK_S"),    KEYLOCK_SUPERVISOR     } /* 045 */
 ,{ _T("BACKSPACE_KEY"),        BACKSPACE_KEY          } /* 046 */
 ,{ _T("ART_FINDER_KEY"),       ART_FINDER_KEY         } /* 047 */
 ,{ _T("SAVE_INVOICE_KEY"),     SAVE_INVOICE_KEY       } /* 048 */
 ,{ _T("TURKEY_KEY"),           TURKEY_KEY             } /* 049 */ /* FMa - Vales de Pavo */
 ,{ _T("QUEUEBUSTING_KEY"),     QUEUEBUSTING_KEY       } /* 050 */ /* acm - QUEUEBUSITNG */
 ,{ _T("HORECA_KEY"),           HORECA_KEY             } /* 051 */ /* v3.4.8 acm - */
 ,{ _T('\0'),                   -1                     } /* The last one, don't change! */
};

KEYDEF AsciiKeyMap[] = {
  { _T("KEY_A"),         KEY_A         } /* 000 */
 ,{ _T("KEY_B"),         KEY_B         } /* 001 */
 ,{ _T("KEY_C"),         KEY_C         } /* 002 */
 ,{ _T("KEY_D"),         KEY_D         } /* 003 */
 ,{ _T("KEY_E"),         KEY_E         } /* 004 */
 ,{ _T("KEY_F"),         KEY_F         } /* 005 */
 ,{ _T("KEY_G"),         KEY_G         } /* 006 */
 ,{ _T("KEY_H"),         KEY_H         } /* 007 */
 ,{ _T("KEY_I"),         KEY_I         } /* 008 */
 ,{ _T("KEY_J"),         KEY_J         } /* 009 */
 ,{ _T("KEY_K"),         KEY_K         } /* 010 */
 ,{ _T("KEY_L"),         KEY_L         } /* 011 */
 ,{ _T("KEY_M"),         KEY_M         } /* 012 */
 ,{ _T("KEY_N"),         KEY_N         } /* 013 */
 ,{ _T("KEY_O"),         KEY_O         } /* 014 */
 ,{ _T("KEY_P"),         KEY_P         } /* 015 */
 ,{ _T("KEY_Q"),         KEY_Q         } /* 016 */
 ,{ _T("KEY_R"),         KEY_R         } /* 017 */
 ,{ _T("KEY_S"),         KEY_S         } /* 018 */
 ,{ _T("KEY_T"),         KEY_T         } /* 019 */
 ,{ _T("KEY_U"),         KEY_U         } /* 020 */
 ,{ _T("KEY_V"),         KEY_V         } /* 021 */
 ,{ _T("KEY_W"),         KEY_W         } /* 022 */
 ,{ _T("KEY_X"),         KEY_X         } /* 023 */
 ,{ _T("KEY_Y"),         KEY_Y         } /* 024 */
 ,{ _T("KEY_Z"),         KEY_Z         } /* 025 */
 ,{ _T("KEY_PROCENT"),   KEY_PROCENT   } /* 026 */
 ,{ _T("CLEAR_KEY"),     CLEAR_KEY     } /* 027 */
 ,{ _T("NO_KEY"),        NO_KEY        } /* 028 */
 ,{ _T("ENTER_KEY"),     ENTER_KEY     } /* 029 */
 ,{ _T("BACKSPACE_KEY"), BACKSPACE_KEY } /* 030 */
 ,{ _T('\0'),             -1           } /* The last one, don't change! */
};

short  cur_mapping = NORMAL_KEYS;
short  *TablePointer = TranslateTable;
KEYDEF *CurKeyMapPointer = KeyMap;

/**************************************************************************
 InitKeyMapTable
 **************************************************************************/

short InitKeyMapTable(void)
{
  int   i;
  short index;

  for (index=0; index < NUMBER_KEYMAPS; index++) {

    SetKeyMapping(index);
    memset(TablePointer, 0, (MAX_KEYCODE * sizeof(short)));

    if (ReadFunctionKeys()) {
      return (FAIL);
    }

    i = MAX_KEYCODE;
          /* All keys that are not yet mapped will be mapped onto itself */
    while (i--) {
      if (TablePointer[i] == 0) {
        TablePointer[i] = i;
      }
    }
  }

  SetKeyMapping(NORMAL_KEYS);
  return (SUCCEED);
} /* InitKeyMapTable */

/**************************************************************************
 ReadFunctionKeys : Read Function Keys from registry in Translate Table
 If a keylock is used, the virtual keylock keys must not be mapped.
 **************************************************************************/

short ReadFunctionKeys(void)
{
  _TCHAR  Buffer[LEN_KEY_BUFFER+1];
  short   index;
  _TCHAR  Val[100];
  short   keyl_attached = NO;
  int     Flag_AddTable=1;

  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("KEYLOCK_ATTACHED"),Val,sizeof(Val)/sizeof(_TCHAR)-1);
  if (_tcsicmp(Val, _T("YES"))==0) {
    keyl_attached = YES;
  }

  for (index=0; CurKeyMapPointer[index].Description != _T('\0') && CurKeyMapPointer[index].Value != -1; index++) {
    if (keyl_attached==NO || CurKeyMapPointer[index].Value/KEYLOCK_LOCK==0) {
        if (CurKeyMapPointer[index].Value<= TURKEY_KEY) { //v3.4.8 acm -
            ReadEnvironmentValue(TREE_KEY_SETTINGS, CurKeyMapPointer[index].Description, Buffer, LEN_KEY_BUFFER);
            Flag_AddTable=1;
        //v3.4.8 acm -{    
        }else{
            Buffer[0]='\0';
            SetRegistryEcho(FALSE);
            ReadEnvironmentValue(TREE_KEY_SETTINGS, CurKeyMapPointer[index].Description, Buffer, LEN_KEY_BUFFER);
            SetRegistryEcho(TRUE);
            Flag_AddTable=(Buffer[0]!='\0');

        }
        //v3.4.8 acm -}
        if (Flag_AddTable){ //v3.4.8 acm -
            if (FillTable(Buffer, index)) 
            {
                return(FAIL);
            }
        }                   //v3.4.8 acm -

    }
  }
  return (SUCCEED);
} /* ReadFunctionKeys */

/**************************************************************************
 FillTable
 **************************************************************************/

short FillTable(_TCHAR *Buffer, short index)
{
  short   keyvalue, i, j;
  _TCHAR  value[20];
  _TCHAR  dummy[100];


  if (!*Buffer) {
    _stprintf(dummy, _T("%s not defined in registry, program will be aborted!!!"), KeyMap[index].Description);
    MessageBox(NULL, (LPTSTR)dummy, NULL, MB_OK|MB_SETFOREGROUND);
    return(FAIL);
  }

  j=0;
  for (i=0; i <= (short)_tcslen(Buffer); i++) {

    if ((Buffer[i]==_T(',')) || (Buffer[i]==_T('\0'))) {
      value[j] = _T('\0');
      keyvalue=HexStringtoDec(value);
      if (keyvalue==-1) {
        _stprintf(dummy, _T("Wrong entry in registry for %s, program will be aborted!!!"), KeyMap[index].Description);
        MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
        return(FAIL);
      }
      else {
        if (!TablePointer[keyvalue]) {
          TablePointer[keyvalue]=CurKeyMapPointer[index].Value;
        }
        else {
            //TablePointer[keyvalue]
          _stprintf(dummy, _T("Key %X already mapped to key function %s, program will be aborted!!!"), keyvalue, KeyMap[index].Description);
          MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
          return(FAIL);
        }
      }
      j=0;
    }
    else {
      value[j++]=Buffer[i];
    }
  }

  return(SUCCEED);
} /*  FillTable */

/**************************************************************************
 HexStringToDec
 **************************************************************************/

short HexStringtoDec(_TCHAR *Buffer)
{
  long     decValue;
  _TCHAR  *ptrEnd = NULL;

  decValue = _tcstol(Buffer, &ptrEnd, 16);

  if (*ptrEnd==_T('\0') && 
      decValue >=0      &&
      decValue <MAX_KEYCODE) {
    return ((short)decValue);
  }

  return (-1);       /* wrong value in registry */
} /* HexStringToDec */

/**************************************************************************
 SetKeyMapping
 **************************************************************************/

KEYDEF *SetKeyMapping(short keymap)
{
  switch (keymap) {

    case NORMAL_KEYS:
      cur_mapping = NORMAL_KEYS;
      TablePointer = TranslateTable;
      CurKeyMapPointer = KeyMap;
      break;

    case ASCII_KEYS:
      cur_mapping = ASCII_KEYS;
      TablePointer = AsciiTable;
      CurKeyMapPointer = AsciiKeyMap;
      break;

    default:
      break;
  }
  return CurKeyMapPointer;
} /*  SetKeyMapping */
