/*
 *     Module Name       : FPOS_STATE.H
 *
 *     Type              : External POS State objects
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
 */

#ifndef __FPOS_STATE_H__
#define __FPOS_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* PUBLIC STATES IN CHRONOLOGICAL ORDER */

extern STATE_OBJ PosStandBy_ST;







extern STATE_OBJ EdpMode_ST;
extern STATE_OBJ CashierMode_ST;
extern STATE_OBJ CashPinCd_ST;
extern STATE_OBJ StartFloat_ST;
extern STATE_OBJ CustomerMode_ST;
extern STATE_OBJ CustomerFee_ST;
extern STATE_OBJ COperatorMode_ST;
extern STATE_OBJ XRCashno_ST;
extern STATE_OBJ CTillLift_ST;
extern STATE_OBJ CTillWallet_ST;
extern STATE_OBJ CTillRefill_ST;
extern STATE_OBJ CTillRefillOk_ST;
extern STATE_OBJ EdpMenu_ST;
extern STATE_OBJ Invoicing_ST;
extern STATE_OBJ InvoicingQty_ST;
extern STATE_OBJ DoTotal_ST;
extern STATE_OBJ Prev_DoTotal_ST;
extern STATE_OBJ Prev_Amnt_Enough_ST;

extern STATE_OBJ PerceptionInput_name_ST;

extern STATE_OBJ Passday700_ST;
extern STATE_OBJ Input_name_ST;

extern STATE_OBJ ShowChange_ST;
extern STATE_OBJ Correction_ST;
extern STATE_OBJ ReadVoidLine_ST;
extern STATE_OBJ NPriceArt_ST;
extern STATE_OBJ NPWeightArt_ST;
extern STATE_OBJ NWWeightArt_ST;
extern STATE_OBJ VoidLastItem_ST;
extern STATE_OBJ SoMode_ST;
extern STATE_OBJ OperatorMode_ST;
extern STATE_OBJ OpXRCashno_ST;
extern STATE_OBJ SupervisorMode_ST;
extern STATE_OBJ SvXRCashno_ST;
extern STATE_OBJ SvTillLift_ST;
extern STATE_OBJ SvTillWallet_ST;
extern STATE_OBJ SvTillRefill_ST;
extern STATE_OBJ SvTillRefillOk_ST;
extern STATE_OBJ LogOn_ST;
extern STATE_OBJ ChangeMode_ST;
extern STATE_OBJ Training_ST;
extern STATE_OBJ GeneralVars_ST;
extern STATE_OBJ FillSysDate_ST;
extern STATE_OBJ FillSysTime_ST;
extern STATE_OBJ PaymentWays_ST;
extern STATE_OBJ PaymAmount_ST;
extern STATE_OBJ FillGVAmnt_ST;
extern STATE_OBJ FillDefaultMode_ST;
extern STATE_OBJ BOperatorMode_ST;
extern STATE_OBJ BXRCashno_ST;
extern STATE_OBJ BTillLift_ST;
extern STATE_OBJ BTillWallet_ST;
extern STATE_OBJ BTillRefill_ST;
extern STATE_OBJ BTillRefillOk_ST;
extern STATE_OBJ Credit_ST;
extern STATE_OBJ CreditQty_ST;
extern STATE_OBJ CPriceArt_ST;
extern STATE_OBJ CPriceCorr_ST;
extern STATE_OBJ CPWeightArt_ST;
extern STATE_OBJ CWWeightArt_ST;
extern STATE_OBJ Discount_ST;
extern STATE_OBJ DiscountQty_ST;
extern STATE_OBJ DPriceArt_ST;
extern STATE_OBJ DPWeightArt_ST;
extern STATE_OBJ DWWeightArt_ST;
extern STATE_OBJ DiscAmount_ST;
extern STATE_OBJ DiscAmount_ST_Turkey;
extern STATE_OBJ StartBreak_ST;
extern STATE_OBJ EndBreak_ST;
extern STATE_OBJ PaymExtraPerc_ST;
extern STATE_OBJ PaymExtraMin_ST;
extern STATE_OBJ PaymExtraVat_ST;
extern STATE_OBJ PaymType_ST;
extern STATE_OBJ Calculadora_ST;
extern STATE_OBJ StartArtFind_ST;
extern STATE_OBJ ArtFindResult_ST;
extern STATE_OBJ DoVoucher_ST;
extern STATE_OBJ DoVoucherAmount_ST;
extern STATE_OBJ AskForDonation_ST;
extern STATE_OBJ Donation_ST;
extern STATE_OBJ SelectPrinter_ST;
extern STATE_OBJ SelectCustomer_ST;
extern STATE_OBJ SelectDocument_ST;


extern STATE_OBJ Turkey_ST;
extern STATE_OBJ Turkey2011_ST;
extern STATE_OBJ Invoicing_Turkey2011_ST;


 
#define ST_POS_STAND_BY            1













#define ST_CASHIER_MODE            2
#define ST_CASH_PIN_CD             3
#define ST_START_FLOAT             4
#define ST_CUSTOMER_MODE           5
#define ST_START_BREAK             6
#define ST_END_BREAK               7
#define ST_CUSTOMER_FEE            8
#define ST_INVOICING              10
#define ST_DO_TOTAL               11
#define ST_SHOW_CHANGE            12
#define ST_CORRECTION             13
#define ST_READ_VOID_LINE         14
#define ST_VOID_LAST_ITEM         15
#define ST_INVOICING_QTY          16
#define ST_NPRICE_ART             17
#define ST_NPWEIGHT_ART           18
#define ST_NWWEIGHT_ART           19
#define ST_CREDIT                 20
#define ST_CREDIT_QTY             21
#define ST_CPRICE_ART             22
#define ST_CPWEIGHT_ART           23
#define ST_CWWEIGHT_ART           24
#define ST_CPRICE_CORR            25
#define ST_DISCOUNT               30
#define ST_DISCOUNT_QTY           31
#define ST_DPRICE_ART             32
#define ST_DPWEIGHT_ART           33
#define ST_DWWEIGHT_ART           34
#define ST_DISC_AMOUNT            36
#define ST_SO_MODE                40
#define ST_EDP_MODE               41
#define ST_EDP_MENU               42
#define ST_OPERATOR_MODE          50
#define ST_OP_XR_CASHNO           51
#define ST_COPERATOR_MODE         60
#define ST_XR_CASHNO              61
#define ST_CTILL_LIFT             62
#define ST_CTILL_WALLET           63
#define ST_CTILL_REFILL           64
#define ST_CTILL_REFILLOK         65
#define ST_BOPERATOR_MODE         70
#define ST_BXR_CASHNO             71
#define ST_BTILL_LIFT             72
#define ST_BTILL_WALLET           73
#define ST_BTILL_REFILL           74
#define ST_BTILL_REFILLOK         75
#define ST_SUPERVISOR_MODE        80
#define ST_SV_XR_CASHNO           81
#define ST_LOG_ON                 86
#define ST_CHANGE_MODE            87
#define ST_TRAINING               88
#define ST_GENERAL_VARS           89
#define ST_FILL_SYS_DATE          90
#define ST_FILL_SYS_TIME          91
#define ST_FILL_GV_AMNT           92
#define ST_FILL_DEFAULT_MODE      93
#define ST_PAYMENT_WAYS           94
#define ST_PAYM_AMOUNT            95
#define ST_EXTRA_PERC             96
#define ST_EXTRA_MIN              97
#define ST_EXTRA_VAT              98
#define ST_PAYM_TYPE              99
#define ST_CALCULADORA           100
#define ST_DO_VOUCHER            101
#define ST_DO_VOUCHER_AMOUNT     102
#define ST_ASK_FOR_DONATION      103
#define ST_DONATION              104
#define ST_SELECT_PRINTER        105
#define ST_SELECT_CUST           106
#define ST_START_ART_FIND        110
#define ST_ART_FIND_RESULT       111

#define ST_VALE_ARTICLE          112
#define ST_VALE_ARTICLE_QTY      113
#define ST_QUEUEBUSTING          114
#define ST_SELECT_DOCUMENT       115
#define ST_PREV_DO_TOTAL         116
#define ST_PERCEPTION_NAME       117
#define ST_PREV_AMNT_ENOUGH      118   

#define ST_PASSDAY700            119
#define ST_NAME                  120

#ifdef __cplusplus
}
#endif

#endif
