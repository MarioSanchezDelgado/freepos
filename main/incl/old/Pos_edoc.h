/*
 *     Module Name       : POS_EDOC.H
 *
 *     Type              : Include file Electronic Document Generation
 *                         
 *
 *     Author/Location   : MAKRO PERU, Lima
 *
 *     Copyright Makro Supermayorista SA
 *               Av. Jorge Chavez 1218
 *               Santiago de Surco
 *               Lima
 *               Peru
 *
 * --------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * --------------------------------------------------------------------------
 * DATE        REASON                                                AUTHOR
 * --------------------------------------------------------------------------
 * 01-May-2015 Initial Release Electronic Document Funcionality     M.L.S.D.
 * --------------------------------------------------------------------------
 */

#ifndef __POS_EDOC_H__
#define __POS_EDOC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TRAINING_MODE_ON  1
#define TRAINING_MODE_OFF 0

#define EDOC_CONFIGURATION 1
#define EDOC_GENERATION 2
#define EDOC_CONFIRMATION 3

#define FACTURA 1
#define BOLETA  3

#define DTNDSR 0
#define DNI    1
#define FRIGN_CRNT 4
#define RUC    6
#define PASSPORT   7

#define CURR_PERU  "PEN"
#define CURR_US    "USD"
#define CURR_ERROR "ERR"

#define DISCPERC "DISCPERC("

#define BUF_RX_LEN 1024

#define N_FIELDS 3
#define HEAD 0
#define BODY 1
#define TAIL 2

#define DE_OK 0
#define ALLOC_ERROR -5

#define PERROR(x) printf_log("Error Code[%d]F[%s]L[%s]", x, __FILE__, __LINE__)

#define TAB '\t'
#define PIPE '|'
#define CR '\n'

typedef struct de_data_conf {
  char host[32];
  long port_epos;
  char header_req[8];
  char tail_req[8];
  char cod_gravadas[8];
  char cod_exoneradas[8];
  char cod_percepcion[8];
  char ubigeo[8];
  char descr_bol[32];
  char descr_fac[32];
  char descr_dir[32];
  char descr_dist[32];
  char date_format[16];
  int  de_pdf_len_line;
} DE_DATA_CONF;

typedef struct art_desc_mactmml {
  double perc;
  double amount;
  double qty_min;
  long start_date;
  long end_date;
  long art_no_array[512];
  long art_grp_array[512];
  short card_type_no;
  double qty_acc;
} ART_DESC_MACTMML;

typedef struct edoc_desc_item {
  double desc;
  short times;
  _TCHAR descr[4096];
} EDOC_DESC_ITEM;

typedef struct edoc_item {
  double price;       /* Total price */
  short art_ind;      /* KGM or UIN */
  double qty;         /* Quantity sold */
  _TCHAR art_no[64];  /* Article code */
  _TCHAR art_grp_no[64];
  _TCHAR descr[4096];  
  short  vat_no;      /* Tax code */ 
  double vat_perc;    /* Tax percentage */ 
  double goods_value;
  
  double disc_msam_perc[1024];   /* MSAM discount percent*/
  double disc_msam_amount;   /* MSAM discount amount */
  double disc_manual;   /* Manual discount */
  
  double disc_amount; 
  double disc_price;  /* price - desc_amount */
  double net_price;   /* (price - discounts)/(100 + vat_perc)% */
  double vat_amount;  /* disc_price - net_price */
  short  flag_percep;
  
  _TCHAR disc_base_descr[1024];
  double disc_base_qty;
  double disc_base_price;
  short  disc_base_vat_no;
  double disc_base_goods_value;
  
  short  mactmml_times;
}EDOC_ITEM;

extern char serie_corr[64];
extern char de_serie[10 + 1];
extern long de_correlative;
extern char hash_resp[32];

extern long edoc_item_length;
extern short training_mode;
extern short gen_edoc;
extern short document_type;
extern short emisor_id_type;

extern unsigned char de_neto[64];
extern unsigned char de_impuesto[64];
extern unsigned char de_descuentos[64];
extern unsigned char de_recargos[64];
extern unsigned char de_total[64];

extern double de_neto_d;
extern double de_impuesto_d;
extern double de_descuentos_d;
extern double de_recargos_d;
extern double de_total_d;

extern double de_gravadas;
extern double de_gratuitas;
extern double de_exoneradas;
extern double de_inafectas;
extern double de_percepcion;

extern int n_items;
extern int desc_items;
extern EDOC_ITEM edoc_item_array[1024];
extern EDOC_DESC_ITEM desc_item[1024];
extern DE_DATA_CONF wp_de_data_conf;

extern short de_reverse_mode;
extern short is_detraction;
extern double detraction_total;


void format2decimals(char *, char *);

int de_pos_config(void);
int de_gen(void);
int de_conf(void);

int de_pos_config_body(void);
int de_gen_body(void);
int de_conf_body(void);

int search_discounts_mactmml(ART_DESC_MACTMML *,long , short);
void match_discounts(EDOC_ITEM *, ART_DESC_MACTMML *);
void acc_discounts(EDOC_ITEM *, ART_DESC_MACTMML *);
void edoc_invoice(void);

void de_get_articles(void);
void de_get_articles_unsorted(void);
void de_get_msam(void);

int NoToLet_de(char *, char *);
int de_snd_rcv(char *, int, char *, int *);
int ind2unidad(short, char *);
void de_date_format(char *, long);
char * curr_code_iso(long);
void de_totales(void);
void de_read_conf_file();
void clean_mem(char *, int);
void clear_edoc_vars(void);
void ed_serie_correlative_hash(char *, int);

#ifdef __cplusplus
}
#endif

#endif