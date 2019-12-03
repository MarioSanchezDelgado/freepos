/*
 *     Module Name       : TXT_ENG.C
 *
 *     Type              : Application TEXT Structures with ENGLISH text
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
 * 13-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * --------------------------------------------------------------------------
 * 24-Apr-2002 Implemented use of Version_mgr instead of linkdate      J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#ifdef TXT_ENG

#pragma message(__FILE__ ": Using English text.")

/*---------------------------------------------------------------------------*/
/* ERROR MESSAGES                                                            */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                        1         2         3         4         5         6*/
/*              0123456789012345678901234567890123456789012345678901234567890*/
/*---------------------------------------------------------------------------*/
_TCHAR *err_msg_TXT[]={
/* 0000 */  _T("")                     /* Is used, do not fill it with text! */
/* 0001 */ ,_T("Press <CLEAR> to continue")
/* 0002 */ ,_T("Invalid input!")
/* 0003 */ ,_T("Call Supervisor")
/* 0004 */ ,_T("Error unknown!")
/* 0005 */ ,_T("Power failure!")
/* 0006 */ ,_T("Printer failure: out of paper")
/* 0007 */ ,_T("Please insert check/slip!")
/* 0008 */ ,_T("Please remove check/slip!")
/* 0009 */ ,_T("Printer motor jam!")
/* 0010 */ ,_T("General printer failure!")
/* 0011 */ ,_T("Card read failure, swipe card again!")
/* 0012 */ ,_T("Clock error!")
/* 0013 */ ,_T("Keyboard failure")
/* 0014 */ ,_T("Item read failure, try again!")
/* 0015 */ ,_T("Out of memory condition")
/* 0016 */ ,_T("MSR device not available!")
/* 0017 */ ,_T("Scanner port not available!")
/* 0018 */ ,_T("Keylock failure!")
/* 0019 */ ,_T("General cmos failure")
/* 0020 */ ,_T("Retail device initialization failure")
/* 0021 */ ,_T("Software power down failure")
/* 0022 */ ,_T("Ram disk format failure")
/* 0023 */ ,_T("Out of memory condition")
/* 0024 */ ,_T("Test key")
/* 0025 */ ,_T("Turn key to L position")
/* 0026 */ ,_T("") /* unused */
/* 0027 */ ,_T("Not a valid menu option")
/* 0028 */ ,_T("Invalid old passport, see reception")
/* 0029 */ ,_T("Amount entered exceeds limit for this paymenttype!")
/* 0030 */ ,_T("First enter amount then press paymenttype!")
/* 0031 */ ,_T("Cheques not allowed, see reception")
/* 0032 */ ,_T("Illegal paymenttype!")
/* 0033 */ ,_T("Tendering not possible, subtotal is zero!")
/* 0034 */ ,_T("No last item to process!")
/* 0035 */ ,_T("Invalid article number!")
/* 0036 */ ,_T("Quantity entered exceeds limit (9999)!")
/* 0037 */ ,_T("Input field must be empty to execute this function!")
/* 0038 */ ,_T("Invalid discount article number!")
                                 /* Translation note: length %s is maximal 8 */
/* 0039 */ ,_T("Error (%s) during initialising till!")
/* 0040 */ ,_T("Illegal function key!")
/* 0041 */ ,_T("Too many keys pressed!")
/* 0042 */ ,_T("Zero not legal!")
/* 0043 */ ,_T("Customer not known on Customer Reception")
/* 0044 */ ,_T("Invalid pincode!")
/* 0045 */ ,_T("Invalid pincode, last possibility!")
/* 0046 */ ,_T("Invalid pincode, call supervisor!")
/* 0047 */ ,_T("No items to void!")
/* 0048 */ ,_T("Amount entered exceeds article goods value!")
/* 0049 */ ,_T("Illegal time!")
/* 0050 */ ,_T("Illegal date!")
/* 0051 */ ,_T("Incorrect system date, check date on invoice screen!")
/* 0052 */ ,_T("Too many invoice lines")
/* 0053 */ ,_T("Article is blocked")
                                 /* Translation note: length %s is maximal 2 */
/* 0054 */ ,_T("Customer is blocked (%s), see reception")
/* 0055 */ ,_T("Combination Quantity, Price and Goodsvalue exceeds maximum!")
/* 0056 */ ,_T("Illegal barcode!")
/* 0057 */ ,_T("Amount to lift exceeds current till-amount!")
/* 0058 */ ,_T("Cashier already logged in on another till")
/* 0059 */ ,_T("Cashier not allowed to logon!")
/* 0060 */ ,_T("Unknown cashier number")
/* 0061 */ ,_T("Run 'End of day' before changing mode!")
/* 0062 */ ,_T("Press TOTAL")
/* 0063 */ ,_T("Cheque amount exceeds limit")
/* 0064 */ ,_T("Do not use the scanner for this field!")
/* 0065 */ ,_T("Passport expired, see reception")
/* 0066 */ ,_T("Illegal passport number!")
/* 0067 */ ,_T("Till lift/refill is not processed.")
/* 0068 */ ,_T("Current recovered shift is not closed.")
/* 0069 */ ,_T("Entered value exceeds maximum!")
/* 0070 */ ,_T("No discount allowed on a deposit article!")
/* 0071 */ ,_T("No discount allowed on a reduced to clear article!")
                                /* Translation note: length %s is maximal 16 */
/* 0072 */ ,_T("Do not enter an amount for %s!")
/* 0073 */ ,_T("Extra amount charged for this payment type!")
/* 0074 */ ,_T("Press ENTER to accept or NO to cancel")
/* 0075 */ ,_T("Too many shifts, run an End Of Day and try again!")
/* 0076 */ ,_T("No 2 shifts in 1 minute, please wait!")
/* 0077 */ ,_T("The article to process has been approved by the supervisor!")
                                /* Translation note: length %s is maximal 16 */
/* 0078 */ ,_T("%s not allowed in combination with other types!")
/* 0079 */ ,_T("Keep the copy invoice at your till!")
/* 0080 */ ,_T("Give this invoice to the customer.")
/* 0081 */ ,_T("Article file corrupted, call EDP!")
/* 0082 */ ,_T("Payment type not equal to selected payment")
/* 0083 */ ,_T("Not possible to sell more foreign money than you received.")
/* 0084 */ ,_T("NO need to pay membership fee this time only, need supervisor")
/* 0085 */ ,_T("NO need to pay membership fee for this Year, need supervisor")
/* 0086 */ ,_T("Too many keys")
/* 0087 */ ,_T("Invalid value for GENVAR variable cust_on_pos")
/* 0088 */ ,_T("Syntax error in Multisam Discount Actions")
/* 0089 */ ,_T("Cannot apply Multisam discount action - subset is empty")

/*******/
/* 0090 */ ,_T("Conformar Cheque")
/* 0091 */ ,_T("Advertencia: No se aceptan cheques a este cliente")
/* 0092 */ ,_T("OCIAII port 2 not available!")
/* 0093 */ ,_T("Please close printer cover")
/* 0094 */ ,_T("Cliente no Registrado!")
/* 0095 */ ,_T("Confirm save invoice")
/* 0096 */ ,_T("Invoice already exists, Overwrite?")
/* 0097 */ ,_T("A pending invoice from %s exists, Get invoice?")
/* 0098 */ ,_T("Parse error: syntax error in query conditions")
/* 0099 */ ,_T("Query returned too many records")
/* 0100 */ ,_T("No records found")
/* 0101 */ ,_T("Internal OPOS keyboard error (%s)")
/* 0102 */ ,_T("POS is OFFLINE : Call Supervisor to give in amount manually?")
/* 0103 */ ,_T("Voucher not known on backoffice")
/* 0104 */ ,_T("This voucher is blocked")
/* 0105 */ ,_T("The voucher number has not the correct length")
/* 0106 */ ,_T("Failed to unblock the voucher!")
/* 0107 */ ,_T("Network configuration error: till undefined")
/* 0108 */ ,_T("Network config error: no new shift allowed")
/* 0109 */ ,_T("Start float exceeds maximum amount in cashdrawer")
/* 0110 */ ,_T("Maximum amount for cash in cashdrawer reached")
/* 0111 */ ,_T("ERROR: backlog corrupt, CLOSE PROGRAM via EDP MENU !!!")
/* 0112 */ ,_T("Check invoice! Two identical lines printed")
/* 0113 */ ,_T("Return Cfee?")
/* 0114 */ ,_T("Time out during reading scanner. Data is probably lost!")
/* 0115 */ ,_T("Scanner/keyboard conflict, try again!")
/* 0116 */ ,_T("Time out during multisam parsing!")
/* 0117 */ ,_T("Too many multisam errors!")
/* 0118 */ ,_T("Donation amount can not be higher than %s")
/* 0119 */ ,_T("Donation amount can not be higher than change value")
/* 0120 */ ,_T("Customer not found using search by fisc_no!")
/* 0121 */ ,_T("Please Contact your Administrator, the end sequence has finished")
};

/*---------------------------------------------------------------------------*/
/* CH01 MESSAGES (INVOICE SCREEN)                                            */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                        1         2         3         4         5         6*/
/*              0123456789012345678901234567890123456789012345678901234567890*/
/*---------------------------------------------------------------------------*/
_TCHAR *scrn_inv_TXT[]={
/* 0000 */  _T("")
#ifndef NO_VIEW_POS_STATE
/* 0001 */ ,_T("R")                                   /* Retour indicator.   */
/* 0002 */ ,_T("T")                                   /* Training indicator. */
#else
/* 0001 */ ,_T("")
/* 0002 */ ,_T("")
#endif
/* 0003 */ ,_T(" ONLINE")
/* 0004 */ ,_T("OFFLINE")
/* 0005 */ ,_T("CFG_ERR")
/* 0006 */ ,_T("          ITEM                                VAT QTY          PRICE      VALUE")
/* 0007 */ ,_T("  TOTAL")
/* 0008 */ ,_T("CHANGE")
/* 0009 */ ,_T("VAT")
/* 0010 */ ,_T("GDS")
/* 0011 */ ,_T("DUE   ")
/* 0012 */ ,_T("PAID")
/* 0013 */ ,_T("FEE")
/* 0014 */ ,_T("")
/* 0015 */ ,_T("SALES MODE")
/* 0016 */ ,_T("RETURN MODE")
/* 0017 */ ,_T("START OF DAY")
/* 0018 */ ,_T("NEW SHIFT")
/* 0019 */ ,_T("CASHIER ID:")
/* 0020 */ ,_T("PIN CODE  :")
/* 0021 */ ,_T("FLOAT     :")
/* 0022 */ ,_T("ON BREAK, WAIT FOR CASHIER")
/* 0023 */ ,_T("NO:")
/* 0024 */ ,_T("DEPOSIT")       /* Used as a description for an unknown      */
                                /*     deposit article.                      */
/* 0025 */ ,_T("DISCOUNT")      /* Used as a descr. for the discount line    */
/* 0026 */ ,_T("MBS WINPOS,  Version %s, %12.12s %5.5s ")
/* 0027 */ ,_T("Description                       Sell price Sell unit Mail Article no")
/* 0028 */ ,_T("_________________________________ __________ _________ ____ __________")
/* 0029 */ ,_T("IGV Percent         Base   Valor IGV")
/* 0030 */ ,_T("IGV Percent         Base   Valor IGV    IGV Percent         Base   Valor IGV")
/* 0031 */ ,_T(" %1hd %7.7s %14.14s %11.11s")
/* 0032 */ ,_T("%2hd %-34.34s Base:%14.14s Valor IGV:%11.11s")
/* 0033 */ ,_T("BUSQUEDA POR RUC/DNI")
/* 0034 */ ,_T("   (Normal)")
/* 0035 */ ,_T("   (Small )")
/* 0036 */ ,_T("Choice   Store/Cust_no   Cust_name                        Fisc_no")
/* 0037 */ ,_T("  %1d        %02d-%06ld     %-32.32s %-16.16s")
};


/*---------------------------------------------------------------------------*/
/* INPUT LINE MESSAGES                                                       */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                        1         2         3         4         5         6*/
/*              0123456789012345678901234567890123456789012345678901234567890*/
/*---------------------------------------------------------------------------*/
_TCHAR *input_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("ENTER CUSTOMER NUMBER")
/* 0002 */ ,_T("ENTER CASHIER NUMBER")
/* 0003 */ ,_T("ENTER ITEM / QTY")
/* 0004 */ ,_T("ENTER ITEM / QTY OR PRESS <NO>")
/* 0005 */ ,_T("ENTER PRICE 1X")
/* 0006 */ ,_T("WEIGHT REQUIRED")
/* 0007 */ ,_T("VOID LAST ITEM Y/N?")
/* 0008 */ ,_T("SUPERVISOR CODE OR <OPERATOR>")
/* 0009 */ ,_T("AMOUNT LIFTED:")
/* 0010 */ ,_T("WALLET NUMBER:")
/* 0011 */ ,_T("REFILL AMOUNT:")
/* 0012 */ ,_T("REFILL AMOUNT OK Y/N")
/* 0013 */ ,_T("CURRENT MODE:")
/* 0014 */ ,_T("Y/N?")
/* 0015 */ ,_T("NEW LIMIT")
/* 0016 */ ,_T("CURRENT DATE :")
/* 0017 */ ,_T("(FORMAT YYYYMMDD)")
/* 0018 */ ,_T("NEW DATE     :")
/* 0019 */ ,_T("CURRENT TIME :")
/* 0020 */ ,_T("NEW TIME     :")
/* 0021 */ ,_T("CURRENT DEFAULT FLOAT AMOUNT SALES:")
/* 0022 */ ,_T("CURRENT DEFAULT FLOAT AMOUNT RETURN:")
/* 0023 */ ,_T("NEW AMOUNT:")
/* 0024 */ ,_T("CURRENT DEFAULT MODE:")
/* 0025 */ ,_T("SWITCH TO")
/* 0026 */ ,_T("CLOSE INVOICE Y/N?")
/* 0027 */ ,_T("Call Supervisor or press NO")
#ifdef SNI
/* 0028 */ ,_T("Turn key to position 1")
#else
/* 0028 */ ,_T("Turn key to N position")
#endif
/* 0029 */ ,_T("CLOSE DRAWER")
/* 0030 */ ,_T("ENTER PAID AMOUNT AND PRESS PAYMENTTYPE")
/* 0031 */ ,_T("DISCOUNT ITEM / QTY")
/* 0032 */ ,_T("ENTER DISCOUNT ITEM / QTY OR PRESS <NO>")
/* 0033 */ ,_T("ENTER CREDIT PRICE 1X")
/* 0034 */ ,_T("DISCOUNT ITEM NO")
/* 0035 */ ,_T("DISCOUNT AMOUNT 1X")
/* 0036 */ ,_T("USE SCROLL-KEYS OR ENTER ARTICLE NUMBER TO MOVE BAR")
/* 0037 */ ,_T("VOID ITEM Y/N?")
/* 0038 */ ,_T("INSERT MONEY AND CLOSE DRAWER")
/* 0039 */ ,_T("ENTER EDP PINCODE FOR MENU:")
/* 0040 */ ,_T("Press any key to continue")
/* 0041 */ ,_T("")
/* 0042 */ ,_T("CREDIT ITEM / QTY")
/* 0043 */ ,_T("OK (Y/N)")
/* 0044 */ ,_T("CUSTOMER MEMBERSHIP EXPIRES PAY NOW ? (Y/N)")
/* 0045 */ ,_T("MEMBERSHIP HAS BEEN EXPIRED PAY NOW ? (Y/N)")
/* 0046 */ ,_T("ENTER VOUCHER NUMBER OR PRESS <NO>")
/* 0047 */ ,_T("ENTER CREDIT NOTE AMOUNT")
/* 0048 */ ,_T("DO YOU WISH TO DONATE Y/N?")
/* 0049 */ ,_T("ENTER DONATION AMOUNT")
/* 0050 */ ,_T("SELECT PRINTER FOR INVOICE")
/* 0051 */ ,_T("SELECT CUSTOMER FROM LIST")
};

/*---------------------------------------------------------------------------*/
/* APPROVAL MESSAGES                                                         */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                        1         2         3         4         5         6*/
/*              0123456789012345678901234567890123456789012345678901234567890*/
/*---------------------------------------------------------------------------*/
_TCHAR *appr_msg_TXT[]={
/* 0000 */  _T("")                      /* Is used, do not fill it with text!*/
/* 0001 */ ,_T("VOID INVOICE?")
/* 0002 */ ,_T("SWITCH RETURN/SALES FOR NEXT INVOICE?")
/* 0003 */ ,_T("OPEN DRAWER?")
/* 0004 */ ,_T("OPERATOR MENU?")
/* 0005 */ ,_T("REMOVE INVOICE!")
/* 0006 */ ,_T("CREDIT NEXT ARTICLE?")
/* 0007 */ ,_T("DISCOUNT ON NEXT ARTICLE?")
/* 0008 */ ,_T("AVANZAR PAGINA?")
};


/*---------------------------------------------------------------------------*/
/* GENERAL MESSAGES                                                          */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                        1         2         3         4         5         6*/
/*              0123456789012345678901234567890123456789012345678901234567890*/
/*---------------------------------------------------------------------------*/
_TCHAR *prompt_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("Working...")
/* 0002 */ ,_T("")                     /* not used */
/* 0003 */ ,_T("Manager pin code or <NO>:")
/* 0004 */ ,_T("Printing...")
/* 0005 */ ,_T("(Check SEL-button if nothing happens.)")
/* 0006 */ ,_T("Reading...")
/* 0007 */ ,_T("Sorting...")
/* 0008 */ ,_T("Counting...")
/* 0009 */ ,_T("EDP pin code or <NO>    :")
/* 0010 */ ,_T("Deleting old invoice history data, just a moment please!")
/* 0011 */ ,_T("Retrieving crazy article history, just a moment please!")
};



/*---------------------------------------------------------------------------*/
/* MENU MESSAGES                                                             */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                        1         2         3         4         5         6*/
/*              0123456789012345678901234567890123456789012345678901234567890*/
/*---------------------------------------------------------------------------*/
_TCHAR *menu_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("X-READ")
/* 0002 */ ,_T("END OF SHIFT")
/* 0003 */ ,_T("END OF DAY")
/* 0004 */ ,_T("LIFT")
/* 0005 */ ,_T("REFILL")
/* 0006 */ ,_T("LOG ON")
/* 0007 */ ,_T("CHANGE MODE")
/* 0008 */ ,_T("GENERAL VAR.")
/* 0009 */ ,_T("TRAINING")
/* 0010 */ ,_T("ENTER CHOICE:")
/* 0011 */ ,_T("OPERATOR MENU")
/* 0012 */ ,_T("SUPERVISOR MENU")
/* 0013 */ ,_T("RETURN TILL")
/* 0014 */ ,_T("SALES TILL")
/* 0015 */ ,_T("NORMAL TILL")
/* 0016 */ ,_T("TRAINING TILL")
/* 0017 */ ,_T("PAYMENT TYPE MENU")             /* Menu paymenttypes         */
/* 0018 */ ,_T("GENERAL VARIABLES MENU")        /* Menu general variables    */
/* 0019 */ ,_T("SYSTEM DATE")
/* 0020 */ ,_T("SYSTEM TIME")
/* 0021 */ ,_T("DEFAULT FLOAT AMOUNT SALES")
/* 0022 */ ,_T("DEFAULT FLOAT AMOUNT RETURN")
/* 0023 */ ,_T("DEFAULT MODE")
/* 0024 */ ,_T("PAYMENT TYPES")
/* 0025 */ ,_T("CD  DESCRIPTION                    LIMIT    PERC   MIN-AMOUNT SPECIAL")
/* 0026 */ ,_T("EXIT PROGRAM")
/* 0027 */ ,_T("EDP MENU")
/* 0028 */ ,_T("REPRINT INVOICE")
/* 0029 */ ,_T("MEMBERSHIP EXPIRES WITHIN 30 DAYS D.D. %s")
/* 0030 */ ,_T("MEMBERSHIP HAS BEEN EXPIRED SINCE %s")
/* 0031 */ ,_T("FEE AMOUNT IS %s FOR %d CARDHOLDERS")
/* 0032 */ ,_T("PAY FEE NOW")
/* 0033 */ ,_T("NO FEE PAYMENT YET (CONTINUE INVOICING)")
/* 0034 */ ,_T("NO FEE PAYMENT (NO INVOICING POSSIBLE)")
/* 0035 */ ,_T("NO FEE RETURN")
/* 0036 */ ,_T("FEE RETURN (S-KEY)")
/* 0037 */ ,_T("MEMBERSHIP FEE PAYMENT MENU")
/* 0038 */ ,_T("FOR CUSTOMER NO %ld IN STORE ")
/* 0039 */ ,_T("GIVEN DISCOUNTS")
/* 0040 */ ,_T("CALCULATOR")
/* 0041 */ ,_T("           D A Y   P A S S")
/* 0042 */ ,_T("FEE AMOUNT IS %s FOR DAY PASS")
/* 0043 */ ,_T("(DAY PASS)")
};


/*---------------------------------------------------------------------------*/
/* CUSTOMER DISPLAY                                                          */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                        1         2         3         4         5         6*/
/*              0123456789012345678901234567890123456789012345678901234567890*/
/*---------------------------------------------------------------------------*/
_TCHAR *cdsp_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("CLOSED")
/* 0002 */ ,_T("OPEN")
/* 0003 */ ,_T("NEXT CUSTOMER PLEASE")
/* 0004 */ ,_T("SUBTOTAL:")
/* 0005 */ ,_T("TOTAL:")
/* 0006 */ ,_T("TOTAL:")
/* 0007 */ ,_T("PAID :")
/* 0008 */ ,_T("CHANGE:")
/* 0009 */ ,_T("THANK YOU")
/* 0010 */ ,_T("x")
/* 0011 */ ,_T("EXPIRES  %s")              /* expiring within 30 days */
/* 0012 */ ,_T("EXPIRED  %s")              /* Expired */
/* 0013 */ ,_T("FEE DUE  %s")
/* 0014 */ ,_T("DAY PASS")
};


/*----------------------------------------------------------------------------*/
/* Layout of X-read and Z-read.                                               */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                        1         2         3         4         5         6         7         8         9       */
/*              0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456 */
/*----------------------------------------------------------------------------*/
_TCHAR *prn_xr_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("%-96.96s")
/* 0002 */ ,_T("X - R E A D    T I L L : %3hd     D A T E : %-12.12s    S H I F T : %3hd    T I M E : %-5.5s")
/* 0003 */ ,_T("                                                              S H I F T : %3hd")
/* 0004 */ ,_T("%3hd %-75.75s%17.17s")
/* 0005 */ ,_T("%4.4s%-75.75s%17.17s")
/* 0006 */ ,_T("CSHR")
/* 0007 */ ,_T("TILL")
/* 0008 */ ,_T("T O T A L")
/* 0009 */ ,_T("TOT:")
/* 0010 */ ,_T("     TIME  TIME       INVOICE     INVOICE VOID  VOID  WALLET NO.")
/* 0011 */ ,_T("%4.4s   ON   OFF     ON    OFF    NO LINES   NO LINES   1   2   3     START FLOAT    LIFT/REFILL       DONATION")
/* 0012 */ ,_T("")
/* 0013 */ ,_T("TOTAL")
/* 0014 */ ,_T("     -------------- -------------- -------------- -------------- --------------")
/* 0015 */ ,_T("%17.17s")
/* 0016 */ ,_T("    %-75.75s")
/* 0017 */ ,_T("Z - R E A D    T I L L : %3hd     D A T E : %-12.12s E N D  O F  D A Y     T I M E : %-5.5s")
/* 0018 */ ,_T("%3hd %-75.75s%17.17s")
/* 0019 */ ,_T("%3hd %-75.75s")
/*******/
/* 0020 */ ,_T("STATUS: %8.8s")
/* 0021 */ ,_T("CURRENT SEQUENCE NUMBER SMALL PRINTER")
};


/*----------------------------------------------------------------------------*/
/* Layout of the invoice.                                                     */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                        1         2         3         4         5         6         7         8         9       */
/*              0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456 */
/*----------------------------------------------------------------------------*/
_TCHAR *prn_inv_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("%-96.96s")
/* 0002 */ ,_T("                   %-30.30s                                                 %02d%02d %04d%6s       %2d")
/* 0003 */ ,_T("                   %-30.30s                                                             %3d      %3d")
/* 0004 */ ,_T("                   %02d %06ld%2s                                                                         %-10.10s    %5s")
/* 0005 */ ,_T(" TOTAL  COD.    IGV%%   VALOR MERC.     MONTO IGV        TOTAL  %-12.12s        %13.13s")
/* 0006 */ ,_T("%6.6s %1hd %5.5s %6.6s %13.13s %13.13s %13.13s")
/* 0007 */ ,_T(" %6.6s   T O T A L  %13.13s %13.13s %13.13s")
/* 0008 */ ,_T("PACKS ")
/* 0009 */ ,_T("- - - - - - - - - - - - - - - - -")
/* 0010 */ ,_T(" %6.6s")
/* 0011 */ ,_T("SUBTOT.")
/* 0012 */ ,_T("CHANGE")
/* 0013 */ ,_T("%-60.60s   - - - - - - - - - - - - - - - - -")
/* 0014 */ ,_T("%-60.60s   MONTO PAGADO        %13.13s")
/* 0015 */ ,_T("                                                          V A N       ......... %13.13s")
/* 0016 */ ,_T("                                                          V I E N E N ......... %13.13s")
/* 0017 */ ,_T("SUBTOTAL    ")
/* 0018 */ ,_T("MONTO     ")
/* 0019 */ ,_T("%-50.50s %-33.33s")
/* 0020 */ ,_T("* * *  TRAINING MODE  * * * * *  TRAINING MODE  * * * * *  TRAINING MODE   * * * * *")
/* 0021 */ ,_T("*  V  O  I  D  **  V  O  I  D  **  V  O  I  D  **  V  O  I  D  **  V  O  I  D  **  V  O  I  D  *")
/* 0022 */ ,_T("CARGO EXTRA ")
/* 0023 */ ,_T("TOTAL ")                  /* Used in prn_inv_TXT[5]              */
/* 0024 */ ,_T("          %1c %7.7s %13.13s %13.13s %13.13s  %-16.16s %6.6s")
/* 0025 */ ,_T("SUBTOTAL PAQTE")          /* Used in prn_inv_TXT[24]             */
/* 0026 */ ,_T("* * * * * * *   C O P Y   I N V O I C E   * * * * * * *   C O P Y   I N V O I C E   * * * * * *")
/* 0027 */ ,_T("%-6.6s %-4.4s = %-11.11s %-3.3s ")
/* 0028 */ ,_T("D.M")
/* 0029 */ ,_T(" %6.6s      %7.7s %13.13s %13.13s %13.13s")
/* 0030 */ ,_T("               GIVEN DISCOUNTS")
/* 0031 */ ,_T("               TOTAL DISCOUNT")
  /********/
/* 0032 */ ,_T("A F I L I A D O               ")
/* 0033 */ ,_T("")
/* 0034 */ ,_T("   RIF # %-30.30s %-44.44s %-10.10s")
/* 0035 */ ,_T("                                                            ")
/* 0036 */ ,_T(" %6.6s                                                       ")
/* 0037 */ ,_T("                                                               %-16.16s %6.6s")
/* 0038 */ ,_T(" %6.6s                                                       %-12.12s        %13.13s")
/* 0039 */ ,_T(" * * * * * * *     C O P Y    I N V O I C E      * * * * * *")
/* 0040 */ ,_T(" * * F A C T U R A  E N   M O D O  D E V O L U C I O N * * *")
/* 0041 */ ,_T("EXEN.")
/* 0042 */ ,_T("GRAV.")
/* 0043 */ ,_T("                                                 12345678                                               %-13.13s")
/* 0044 */ ,_T("")
/* 0045 */ /*,_T("IGV   Percent       Base       Valor IGV") se comento soporte PERU*/
/* 0045 */ ,_T(" ")
/* 0046 */ /*,_T(" %1hd %10.10s %13.13s %13.13s") se comento soporte PERU*/
/* 0046 */ ,_T("  ")
/* 0047 */ /*,_T("%-16.16s %13.13s") se comento soporte PERU*/
/* 0047 */ ,_T(" ")
/* 0048 */ /*,_T("CHANGE           %13.13s") se comento soporte PERU*/
/* 0048 */ ,_T("  ")
/* 0049 */ ,_T("FECHA DE EXPEDICION")
/* 0050 */ ,_T("%-13.13s  %-5.5s     %02d%02d %04d%6s")
/* 0051 */ ,_T("----------------------------------------")
/* 0052 */ ,_T("***-------****-------*** CUT CUT CUT ***------*****------*** CUT CUT CUT ***-------****-------***")
/* 0053 */ ,_T("***--***--*** CUT CUT CUT  ***--***--***")
/* 0054 */ ,_T("Cod.   Descripcion")
/* 0055 */ ,_T("--> TOTAL VENTA")
/* 0056 */ ,_T("VUELTO")
/* 0057 */ ,_T("DONATION")
/* 0058 */ ,_T("Tarifa     Compra   Base/imp        IGV")
/* 0059 */ ,_T("TOTAL")
/* 0060 */ ,_T("PASSPORT No.    %02d-%06ld")
/* 0061 */ ,_T("TILL No.        %d %d")
/* 0062 */ ,_T("TOTAL NBR.PACKS %-6.6s")
/* 0063 */ ,_T("*** V O I D *** V O I D  *** V O I D ***")
/* 0064 */ ,_T("**** TRAINING MODE *** TRAINING MODE ***")
/* 0065 */ ,_T("**** COPY INVOICE ***** COPY INVOICE ***")
/* 0066 */ ,_T("GIVEN DISCOUNTS")
/* 0067 */ ,_T("TOTAL DISCOUNT")
/* 0068 */ ,_T("       Cant       Valor IGV       Total")
/* 0069 */ ,_T("       ---------- ---------- ----------")
/* 0070 */ ,_T("**** TRAINING MODE * * TRAINING MODE ***")
/* 0071 */ ,_T("**** COPY INVOICE ** ** COPY INVOICE ***")
/* 0072 */ ,_T("* * *  TRAINING MODE  * *   * *  TRAINING MODE  * *   * *  TRAINING MODE   * * * * *")
/* 0073 */ ,_T("* * * * * * *   C O P Y   I N V O I C E   * * *   * * *   C O P Y   I N V O I C E   * * * * * *")
/* 0074 */ ,_T("TICKET NO.      %02dT%09ld")
};

/* mon_names[ i ] contains name of i'th month                                */
_TCHAR *mon_names[] = {
  _T(""), _T("JAN"), _T("FEB"), _T("MAR"), _T("APR"), _T("MAY"), _T("JUN"),
      _T("JUL"), _T("AUG"), _T("SEP"), _T("OCT"), _T("NOV"), _T("DEC")
};

/*----------------------------------------------------------------------------*/
/* Layout of the cheque.                                                      */
/*----------------------------------------------------------------------------*/
extern _TCHAR *prn_ch_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("       MAKRO COMERCIALIZADORA, S.A.")
/* 0002 */ ,_T(" %-13.13s %s  %s")
/* 0003 */ ,_T("     **%s**")
/* 0004 */ ,_T("%ld")
/* 0005 */ ,_T("****")                  /* No more than 10 characters.         */
/* 0006 */ ,_T("                                           NO ENDOSABLE")   /* */
/* 0007 */ ,_T("                   %3d %3d %2d %06ld A*%-3.3s%2d%6s")   /* Auto.cheq.amnt.factura_no*/
};


/*                                                                           */
/* Language depended alphanumeric strings which are used in pos_tran.c       */
/*                                                                           */

_TCHAR *one_till_nine[]={
#if LANGUAGE == DUTCH
  _T("Een"), _T("Twee"), _T("Drie"), _T("Vier"), _T("Vijf"),
  _T("Zes"), _T("Zeven"), _T("Acht"), _T("Negen")
#elif LANGUAGE == ENGLISH
  _T("One"), _T("Two"), _T("Three"), _T("Four"), _T("Five"),
  _T("Six"), _T("Seven"), _T("Eight"), _T("Nine")
#elif LANGUAGE == GREEK
  _T("EMA "), _T("DUO "), _T("TQIA "), _T("TESSEQA "), _T("PEMTE "),
  _T("ENI "), _T("EPTA "), _T("OJTY "), _T("EMMEA ")
#elif LANGUAGE == ESPANOL
  _T(" UNO"),  _T(" DOS"),   _T(" TRES"), _T(" CUATRO"), _T(" CINCO"),
  _T(" SEIS"), _T(" SIETE"), _T(" OCHO"), _T(" NUEVE")
#endif
};

_TCHAR *eleven_till_nineteen[]={
#if LANGUAGE == DUTCH
  _T("Elf"), _T("Twaalf"), _T("Dertien"), _T("Veertien"),
  _T("Vijftien"), _T("Zestien"), _T("Zeventien"), _T("Achttien"), _T("Negentien")
#elif LANGUAGE == ENGLISH
  _T("Eleven"), _T("Twelve"), _T("Thirteen"), _T("Fourteen"),
  _T("Fifteen"), _T("Sixteen"), _T("Seventeen"), _T("Eighteen"), _T("Nineteen")
#elif LANGUAGE == GREEK
  _T("EMTEJA "), _T("DYDEJA "), _T("DEJATQEIS "), _T("DEJATESSEQEIS "),
  _T("DEJAPEMTE "), _T("DEJAENI "), _T("DEJAEPTA "), _T("DEJAOJTY "), _T("DEJAEMMEA ")
#elif LANGUAGE == ESPANOL
  _T(" ONCE"),   _T(" DOCE"),      _T(" TRECE"),      _T(" CATORCE"),
  _T(" QUINCE"), _T(" DIECISEIS"), _T(" DIECISIETE"), _T(" DIECIOCHO"), _T(" DIECINUEVE")
#endif
};

_TCHAR *ten_till_ninety[]={
#if LANGUAGE == DUTCH
  _T("Tien"), _T("Twintig"), _T("Dertig"), _T("Veertig"), _T("Vijftig"),
  _T("Zestig"), _T("Zeventig"), _T("Tachtig"), _T("Negentig")
#elif LANGUAGE == ENGLISH
  _T("Ten"), _T("Twenty"), _T("Thirty"), _T("Forty"), _T("Fifty"),
  _T("Sixty"), _T("Seventy"), _T("Eighty"), _T("Ninety")
#elif LANGUAGE == GREEK
  _T("DEJA "), _T("EIJOSI "), _T("TQIAMTA "), _T("SAQAMTA "), _T("PEMGMTA "),
  _T("ENGMTA "), _T("EBDOLGMTA "), _T("OCDOMTA "), _T("EMEMGMTA ")
#elif LANGUAGE == ESPANOL
  _T(" DIEZ"),    _T(" VEINTI"),   _T(" TREINTA"), _T(" CUARENTA"), _T(" CINCUENTA"),
  _T(" SESENTA"), _T(" SETENTA"), _T(" OCHENTA"), _T(" NOVENTA")
#endif
};

#if LANGUAGE == ESPANOL
_TCHAR *exception[]={
  _T(" UN"), _T(" VEINTE")
};
#endif


#if LANGUAGE == GREEK
_TCHAR *hundred_till_ninehundred[]={
  _T("EJATO "), _T("DIAJOSIES "), _T("TQIAJOSIES "), _T("TETQAJOSIES "), _T("PEMTAJOSIES "),
  _T("ENAJOSIES "), _T("EPTAJOSIES "), _T("OJTAJOSIES "), _T("EMIAJOSIES ")
};
#elif LANGUAGE == ESPANOL       /* Hundred is handled by inter_text1.      */
_TCHAR *hundred_till_ninehundred[]={
  _T(""), _T(" DOS"), _T(" TRES"), _T(" CUATRO"), _T(" QUIN"),
  _T(" SEIS"), _T(" SETE"), _T(" OCHO"), _T(" NOVE")
};
#endif


#if LANGUAGE == GREEK
_TCHAR *inter_text1[]={
  _T(""), _T("WIKIADA "), _T("EJATOLLUQIO "),_T("")
};
#elif LANGUAGE == ESPANOL
_TCHAR *inter_text1[]={
  /* 100     1xx       500      */
  _T(" CIEN"), _T(" CIENTO"), _T("IENTOS")
};
#endif


_TCHAR *inter_text[]={
#if LANGUAGE == DUTCH
  _T("Honderd"), _T("Duizend"), _T("Miljoen"), _T("Miljard")
#elif LANGUAGE == ENGLISH
  _T("Hundred"), _T("Thousand"), _T("Million"), _T("Billion")
#elif LANGUAGE == GREEK
  _T(""), _T("WIKIADES "), _T("EJATOLLUQIA "),_T("")
#elif LANGUAGE == ESPANOL
  _T("CIENTOS"), _T(" MIL"), _T(" MILLON"), _T(" MILLONES")
#endif
};


#endif /* TXT_ENG */