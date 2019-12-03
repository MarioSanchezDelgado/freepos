/*
 *     Module Name       : POS_FUNC.H
 *
 *     Type              : Include file general functions definitions
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

#ifndef __POS_FUNC_H__
#define __POS_FUNC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DATE_SIZE 11  


#define   NOT_USED   0
#define   USED       1
#define   READ_USED   99

#define   VOUCHER_PREFIX   1




extern _TCHAR * string_right_justify(_TCHAR *, short, short, _TCHAR *);
extern short  wait_for_key_S_or_NO(void);
extern short  wait_for_supervisor_approval(void);
extern short  wait_for_super_appr_or_NO(void);
extern void   wait_for_closed_drawer(_TCHAR *, _TCHAR *);
extern short  open_and_close_drawer(_TCHAR *, short);
extern short  vfy_valepavo_save (_TCHAR *, short);

extern double rnd_curr(double);
extern double floor_price( double );
extern short  ftoa(double, short, _TCHAR *);
extern double atof_price(_TCHAR *);
extern short  ftoa_price(double, short, _TCHAR *);
extern short  calc_in_ex_vat(short, double *, double *, short);
extern double calc_incl_vat(double, short);
extern double calc_excl_vat(double, short);
extern void   cls(void);
extern short  application_inp_idle(void);
extern short  cdsp_closed_till(short,  short);
extern void   inp_debug(_TCHAR *, short, short);
extern void   display_working(int);

extern short  recover_shift_updt(void);
extern short  recover_shift_save(void);
extern short  recover_shift_remv(void);
extern short  recover_shift_2tm(void);

extern _TCHAR *get_compile_date(void);

extern int is_article_prom(long art_no, int prom_type); /* 27-Jan-2012 acm -  */
extern int get_num_article_prom(int prom_type); /* 27-Jan-2012 acm -  */
extern int isvigente_promotion(); /* 27-Jan-2012 acm -  */
extern int ispassday(); /* v3.4.9 acm -  */
extern int isFiscalClient(); /* v3.4.9 acm -  */

extern int is_article_detraccion(long); /*mlsd*/

extern int get_num_cupon(double monto_venta); /* 27-Jan-2012 acm -  */

extern int get_num_gift(double monto_venta);  /* AC2012-003 acm - */
extern double get_invoice_rounded(double monto_vuelto);  // v3.4.7 acm -

extern void printf_ln(short printer ,  const  char *format, ...);
extern int isValeTurkey_Valid(long valepavo_no, long vale_type);

extern int init_invoice_user(_TCHAR *data, short key);

extern VALEPAVO_PROM_DEF  cur_valepavo;
extern ANTICIPO_PROM_DEF  cur_anticipo;


extern int INVOICE_VALID ;                    /* AC2012-003 acm - */
extern int PERCEPTION_ENABLED      ;

extern int get_num_cupon_cine(); //v3.5 acm -
extern int get_num_vale_pavo(); //v3.5 acm -

///////////////////////////////////////////////////////////////////////
extern int get_num_fiesta_futbol(); //v3.5 acm -
extern int get_num_cupon_global(int cupon_global_index); //v3.5 acm -

///////////////////////////////////////////////////////////////////////

extern int printf_log(char* format, ...);

extern double get_valeturkey_count(void);
extern int get_num_cupon_FERIA_ESCOLAR();
extern int is_promocion_FERIA_ESCOLAR( int * p_vigente );///double monto_venta, long fini, long ffin);

extern int is_client_pay_FACTURA();
extern double TOT_GEN_PERCEPTION   ;
extern double PERC_GEN_PERCEPTION  ;
extern int    IS_GEN_PERCEPTION    ;

extern char usr_document[500]; //mname
extern char usr_name    [500]; //mname

extern char last_name    [500];
extern char last_document[500];

extern char usr_perception_document[500];
extern char usr_perception_name    [500];


extern char last_perception_name    [500];
extern char last_perception_document[500];

extern void perception_document_DFLT(INPUT_DISPLAY *x, _TCHAR *data);
extern void perception_name_DFLT(INPUT_DISPLAY *x, _TCHAR *data);

extern void passday700_document_DFLT(INPUT_DISPLAY *x, _TCHAR *data);
extern void name_DFLT(INPUT_DISPLAY *x, _TCHAR *data);

extern int pe_val_ruc( char  * txtNroRuc) ;
extern int pe_val_dni( char  * txtDni) ;



extern short Prev_DoTotal_ST_init(_TCHAR *data, short key);

int is_PERCEPCION( int * p_vigente, 
                  double * p_total_perception, 
                  double * p_porcentaje);///double monto_venta, long fini, long ffin)

extern double POS_ZERO_VALUE;

extern void user_ftoa_porcentaje(double value, char *out_value);


extern  short prc_PerceptionInput_name(_TCHAR *data, short key);
extern short Prev_Amnt_Enought_init(_TCHAR *data, short key);

extern  short prc_Input_name(_TCHAR *data, short key); //mname

extern int getNameFromCardHolder( _TCHAR  * dni, CARDHOLDER_DEF * cardholder);
extern  int getNameFromCust_Perc( char * dni, CUST_PERC_DEF * cust_perc);

extern int get_num_cupon_FIESTA_FUTBOL();
extern int get_num_cupon_CUPON_GLOBAL(int index);
extern int is_promocion_FIESTA_FUTBOL( int * p_vigente );
extern int get_num_cupon_CUPON_GLOBAL(int index);
extern int is_promocion_CUPON_GLOBAL( int * p_vigente, int cupon_index );

extern  _TCHAR *cupon_global_inv_TXT[][3];
extern  int     prom_type_CUPON_GLOBAL [];
extern _TCHAR * cupon_global_xr_TXT[];
extern int cupon_global_vigente(int cupon_index);

extern int IS_ANTICIPO();

extern short used_curr_type ;

extern char *strtrim (char *str, char *out);
extern short  book_voucher(short, double, short);

#ifdef __cplusplus
}
#endif

#endif
