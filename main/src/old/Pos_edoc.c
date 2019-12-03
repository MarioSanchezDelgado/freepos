/*
 *     Module Name       : POS_EDOC.C
 *
 *     Type              : Electronic document generation Functions
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <winsock.h>
#include <io.h>
#include <errno.h>
#include <windows.h>
#include <windowsx.h>
#pragma hdrstop
#include "Template.h" /* Every source should use this one! */
#include "appbase.h"

#include "comm_tls.h"
#include "stri_tls.h"
#include "date_tls.h"

#include "fmt_tool.h"
#include "mem_mgr.h"
#include "prn_mgr.h"
#include "tot_mgr.h"
#include "err_mgr.h"
#include "tm_mgr.h"
#include "inp_mgr.h"
#include "state.h"
#include "llist.h"
#include "sll_mgr.h"

#include "pos_inp.h"
#include "pos_recs.h"
#include "pos_txt.h"
#include "pos_tm.h"
#include "pos_tot.h"
#include "WPos_mn.h"
#include "pos_func.h"
#include "pos_com.h"
#include "pos_tot.h"
#include "write.h"
#include "pos_bp2.h"
#include "pos_bp1.h"
#include "st_main.h"
#include "pos_errs.h"
#include "pos_msam.h"
#include "pos_tot.h"
#include "pos_tran.h"
#include "pos_errs.h"
#include "comm_tls.h"
#include "intrface.h" /* 27-Jan-2012 acm -  */

#include "Pos_edoc.h"


static char line_aux[1024];
static char aux[1024];
static char auxb[1024];
static char n_aux[32];
static char f_aux[16];
char linvoice_buffer[100];
char invoice_no[6 +1];
char prn_time[6];
char prn_date[12 +1];
static char * req_buffer[N_FIELDS] = {NULL, NULL, NULL};
static char body_buffer[32768];
static char * mess_buffer = NULL;
static int mess_len;
PAYMENT_DEF paym;
VOUCHER_VALE_DEF voucher;

int (*de_mess_func[])(void) =
{
  NULL,
  de_pos_config_body,
  de_gen_body,
  de_conf_body
};

char * array_dist[11]=
{
	"",
	"INDEPENDENCIA - LIMA - LIMA", //1
	"CALLAO-PROV. CONST. DEL CALLAO - LIMA", //2
	"SANTA ANITA - LIMA - LIMA", //3
	"SANTIAGO DE SURCO - LIMA - LIMA", //4
	"JOSE LUIS BUSTAMANTE Y RIVERO - AREQUIPA", //5
	"CHICLAYO - CHICLAYO - LAMBAYEQUE", //6
	"SAN JUAN DE LURIGANCHO - LIMA - LIMA", //7
	"TRUJILLO - TRUJILLO - LA LIBERTAD", //8
	"PIURA - PIURA - PIURA", //9
	"COMAS - LIMA - LIMA" //10
};

int _strcat_c(char c, int offset)
{
	char aux_c[2];
	
	memset(aux_c, 0, sizeof(aux_c));
	aux_c[0] = c;
	memcpy(req_buffer[BODY] + offset, &c, 1);

	return 1;
}

/* size    string size completed with zeros to the left */
int _strcat_d(int d, int size, int offset)
{
	if(log10(d) > (size - 1))
		size = (int)log10(d) + 1;
	
	memset(n_aux, 0, sizeof(n_aux));
	sprintf(n_aux, "%0*d", size, d);
	memcpy(req_buffer[BODY] + offset, n_aux, size);

	return size;
}

/* n    number of decimal digits */
int _strcat_f(double f, int n, int offset)
{
	int l = (int)log10(f) + 1 + 1 + n; /* size number + point + size decimals */
  
	memset(f_aux, 0, sizeof(f_aux));
	sprintf(f_aux, "%.*f", n, f);
	memcpy(req_buffer[BODY] + offset, f_aux, l);

	return l;
}

int _strcat_s(char * s, int offset)
{
  char * paux = NULL;
	char * aux_ptr = NULL;
  int  length = (int)strlen(s);
  
  if (length > 0){
		memcpy(req_buffer[BODY] + offset, s, length);
  }
  return length;
}

int conc_message(int * length)
{
  int l = 0;
  int i;

  mess_len = 0;
  for(i = 0; i < N_FIELDS; i++)
      l += length[i];

  if((mess_buffer = calloc(l + 1, sizeof(char))) == NULL)
      return ALLOC_ERROR;

  for(i = 0; i < N_FIELDS; i++){
    memcpy(mess_buffer + mess_len, req_buffer[i], length[i]);
    mess_len += length[i];
		memset(req_buffer[i], 0, length[i]);
		free(req_buffer[i]);
    req_buffer[i] = NULL;
  }

  return DE_OK;
}

int de_head()
{
  int len = strlen(wp_de_data_conf.header_req);
  
  if((req_buffer[HEAD] = calloc(len, sizeof(char))) == NULL){
		printf_log("calloc error:line[%d]", __LINE__);
    return ALLOC_ERROR;
  }

  memcpy(req_buffer[HEAD], wp_de_data_conf.header_req, len);
  return len;
}

int de_tail()
{  
  int len = strlen(wp_de_data_conf.tail_req);
  
  if((req_buffer[TAIL]  = calloc(len, sizeof(char))) == NULL){
		printf_log("calloc error:line[%d]", __LINE__);
    return ALLOC_ERROR;		
	}

  memcpy(req_buffer[TAIL] , wp_de_data_conf.tail_req, len);
  return len;
}

int cat_c(char c, int offset, char token)
{
  int length = offset;
  int rc = 0;
  
  if((rc = _strcat_c(c, length)) == -1) return rc; length += 1;
  if((rc = _strcat_c(token, length)) == -1) return rc; length += 1;
  
  return length;
}

int cat_d(int d, int size, int offset, char token)
{
  int length = offset;
  int rc = 0;
  
  if((rc = _strcat_d(d, size, length)) == -1) return rc; length += rc;
  if((rc = _strcat_c(token, length)) == -1) return rc; length += 1;

  return length;
}

int cat_s(char * s, int offset, char token)
{
  int length = offset;
  int rc = 0;

  if((rc = _strcat_s(s, length)) == -1) return rc; length += rc;
  if((rc = _strcat_c(token, length)) == -1) return rc; length += 1;

  return length;
}

int cat_f(double f, int size, int offset, char token)
{
  int length = offset;
  int rc = 0;

  if((rc = _strcat_f(f, size, length)) == -1) return rc; length += rc;
  if((rc = _strcat_c(token, length)) == -1) return rc; length += 1;
  return length;
}

int de_pos_config_body()
{
  int length = 0;
  int rc = 0;
  int till_no;

  if((rc = cat_c('1', length, TAB)) == -1) return rc; length = rc;
  if((rc = cat_d(training_mode, 1, length, TAB)) == -1) return rc; length = rc;
  if((rc = cat_s(genvar.fisc_no_comp + 4, length, TAB)) == -1) return rc; length = rc;

  memset(aux, 0, sizeof(aux));
  memcpy(aux, &till.till_no, 4);
  till_no = (int)((aux[0]&0xFF) + (aux[1]&0xFF) + (aux[2]&0xFF) + (aux[3]&0xFF));

  if((rc = cat_d(till_no, 1, length, TAB)) == -1) return rc; length = rc;
  if((rc = _strcat_d(pos_system.store_no, 2, length)) == -1) return rc; length += rc;
  return length;
}

int de_conf_body()
{
  int length = 0;
  int rc = 0;
  char aux[4];
  int till_no = 0;
  
  if((rc = cat_c('3', length, TAB)) == -1) return rc; length = rc;
  if((rc = cat_d(training_mode, 1, length, TAB)) == -1) return rc; length = rc;
  if((rc = cat_s(genvar.fisc_no_comp + 4, length, TAB)) == -1) return rc; length = rc;

  memset(aux, 0, sizeof(aux));
  memcpy(aux, &till.till_no, 4);
  till_no = (int)((aux[0]&0xFF) + (aux[1]&0xFF) + (aux[2]&0xFF) + (aux[3]&0xFF));

  if((rc = cat_d(till_no, 1, length, TAB)) == -1) return rc; length = rc;
  if((rc = cat_d(document_type, 2, length, TAB)) == -1) return rc; length = rc;
  if((rc = _strcat_s(serie_corr, length)) == -1) return rc; length += rc;
  return length;
}

int de_gen_body_init()
{
  int length = 0;
  int rc = 0;
  int till_no;
	
  if((rc = cat_c('2', length, TAB)) == -1) return rc; length = rc;
  if((rc = cat_d(training_mode, 1, length, TAB)) == -1) return rc; length = rc;
  if((rc = cat_s(genvar.fisc_no_comp + 4, length, TAB)) == -1) return rc; length = rc;

  memset(aux, 0, sizeof(aux));
  memcpy(aux, &till.till_no, 4);
  till_no = (int)((aux[0]&0xFF) + (aux[1]&0xFF) + (aux[2]&0xFF) + (aux[3]&0xFF));

  if((rc = cat_d(till_no, 1, length, TAB)) == -1) return rc; length = rc;
  if((rc = _strcat_d(document_type, 2, length)) == -1) return rc; length += rc;
  return length;
}

int de_en(int offset)
{
  char date[13];
  int length = offset;
  int rc = 0;
  PAYMENT_DEF   paym;

  memset(date, 0, sizeof(date));
  paym.paym_cd = 1;
  get_paym(&paym);
  de_date_format(date, pos_invoice.invoice_date);

  if((rc = _strcat_c('\t', length)) == -1) return rc; length += rc;
  if((rc = cat_s("EN", length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_d(document_type, 2, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s(serie_corr, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s("||", length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s(date, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s(curr_code_iso(paym.currency_no), length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s(genvar.fisc_no_comp + 4, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_d(emisor_id_type, 1, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s(genvar.name, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s(genvar.name_comp, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s(wp_de_data_conf.ubigeo, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s("AV. JORGE CHAVEZ NRO. 1218", length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s("Lima", length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s("Lima", length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s("Santiago de Surco", length, PIPE)) == -1) return rc; length = rc;
  if (document_type == FACTURA){
    if((rc = cat_s(cust.fisc_no, length, PIPE)) == -1) return rc; length = rc;
    if((rc = cat_d(RUC, 1, length, PIPE)) == -1) return rc; length = rc;
    if((rc = cat_s(cust.name, length, PIPE)) == -1) return rc; length = rc;
  }
  else{
    if( atoi(de_total) > 700 ){
      if(ispassday()){
        if(IS_GEN_PERCEPTION)
        {
          if((rc = cat_s(usr_perception_document, length, PIPE)) == -1) return rc; length = rc;
        }
        else
        {
          if((rc = cat_s(usr_document, length, PIPE)) == -1) return rc; length = rc;
        }
      }
      else{
        if((rc = cat_s(cust.fisc_no, length, PIPE)) == -1) return rc; length = rc;
      }

      if((rc = cat_d(DNI, 1, length, PIPE)) == -1) return rc; length = rc;
      
      if(ispassday()){
        if(IS_GEN_PERCEPTION){
          if((rc = cat_s(usr_perception_name, length, PIPE)) == -1) return rc; length = rc;
        }
        else
        {
          if((rc = cat_s(usr_name, length, PIPE)) == -1) return rc; length = rc;
        }
      }
      else{
        if((rc = cat_s(cust.name, length, PIPE)) == -1) return rc; length = rc;
      }
      
    }
    else{
      if(IS_GEN_PERCEPTION)
      {
        if((rc = cat_s(usr_perception_document, length, PIPE)) == -1) return rc; length = rc;
        if((rc = cat_d(DNI, 1, length, PIPE)) == -1) return rc; length = rc;
        if((rc = cat_s(usr_perception_name, length, PIPE)) == -1) return rc; length = rc;
      }
      else
      {
        if((rc = cat_s(cust.fisc_no, length, PIPE)) == -1) return rc; length = rc;
        if((rc = cat_d(DNI, 1, length, PIPE)) == -1) return rc; length = rc;
        if((rc = cat_s(cust.name, length, PIPE)) == -1) return rc; length = rc;
      } 
    }
  }
  
  if(ispassday()){
    if((rc = cat_s("", length, PIPE)) == -1) return rc; length = rc;
  }
  else{
     if((rc = cat_s(cust.address, length, PIPE)) == -1) return rc; length = rc;
  }
 
  if((rc = cat_f(floor_price(de_gravadas * 100.0/118.0 + de_exoneradas), 2, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s(de_impuesto, length, PIPE)) == -1) return rc; length = rc;
  //if((rc = cat_s(de_descuentos, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s("", length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s(de_recargos, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s(de_total, length, PIPE)) == -1) return rc; length = rc;
  if((rc = cat_s("", length, PIPE)) == -1) return rc; length = rc;
  /* Los netos que contenga IGV */
  if((rc = cat_s("", length, PIPE)) == -1) return rc; length = rc;
  if (document_type == FACTURA){
    if((rc = cat_s(cust.fisc_no, length, PIPE)) == -1) return rc; length = rc;
    if((rc = cat_d(RUC, 1, length, CR)) == -1) return rc; length = rc;
  }
  else{
    if( atoi(de_total) > 700 ){
      if(ispassday()){
        if((rc = cat_s(usr_name, length, PIPE)) == -1) return rc; length = rc;
      }
      else{
        if((rc = cat_s(cust.name, length, PIPE)) == -1) return rc; length = rc;
      }
      if((rc = cat_d(DNI, 1, length, CR)) == -1) return rc; length = rc;
    }
    else{
      if((rc = cat_s(cust.fisc_no, length, PIPE)) == -1) return rc; length = rc;
      if((rc = cat_d(DNI, 1, length, CR)) == -1) return rc; length = rc;
    }
  }
  return length;
}

int de_doc(int offset)
{
  int length = offset;
  double tot_disc_amount = 0.0;
 
	if(de_gravadas > 0){
		if((length = cat_s("DOC", length, PIPE)) == -1) return length;
		if((length = cat_s(wp_de_data_conf.cod_gravadas, length, PIPE)) == -1) return length;

    tot_disc_amount = (genvar.price_incl_vat == INCLUSIVE) ? floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))
                                                           : floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_EXCL));
		if((length = cat_f(floor_price((de_gravadas)/1.18), 2, length, CR)) == -1) return length;
		if(IS_GEN_PERCEPTION)
		{
			if((length = cat_s("DOC", length, PIPE)) == -1) return length;
			if((length = cat_s(wp_de_data_conf.cod_percepcion, length, PIPE)) == -1) return length;
      if((length = cat_f(get_invoice_perception_currmode(), 2, length, PIPE)) == -1) return length;
      if((length = cat_f(floor_price(de_gravadas + de_exoneradas + get_invoice_perception_currmode()), 2, length, PIPE)) == -1) return length;
      if((length = cat_f(50 * get_invoice_perception_currmode(), 2, length, CR)) == -1) return length;
		}
  }
  
  if(de_exoneradas > 0){
		if((length = cat_s("DOC", length, PIPE)) == -1) return length;
		if((length = cat_s(wp_de_data_conf.cod_exoneradas, length, PIPE)) == -1) return length;
		if((length = cat_f(de_exoneradas, 2, length, CR)) == -1) return length;
  }

  return length;
}

int de_dn(int offset)
{
	int length = offset;
	char buffer[256];

  memset(aux, 0, sizeof(aux));

  sprintf(aux, "%.2f", de_total_d);
	memset(buffer, 0, sizeof(buffer));
	NoToLet_de(aux, buffer);
  
	if((length = cat_s("DN|1|1000", length, PIPE)) == -1) return length;
	if((length = cat_s(buffer, length, CR)) == -1) return length;
  
	if(IS_GEN_PERCEPTION){
		if((length = cat_s("DN|2|2000|COMPROBANTE DE PERCEPCION", length, CR)) == -1) return length;
	}
  
	return length;
}

int de_de(int offset, int idx)
{
  int length = offset;
  char buffer[4];

  memset(buffer, 0, sizeof(buffer));
  ind2unidad(edoc_item_array[idx].art_ind, buffer);

  if((length = cat_s("DE", length, PIPE)) == -1) return length;
  if((length = cat_d(idx + 1, 1, length, PIPE)) == -1) return length;
  if((length = cat_f(edoc_item_array[idx].net_price * (1 + edoc_item_array[idx].vat_perc/100), 3, length, PIPE)) == -1) return length;
  if((length = cat_s(buffer, length, PIPE)) == -1) return length;
  if((length = cat_f(edoc_item_array[idx].qty, 3, length, PIPE)) == -1) return length;
  if((length = cat_f(edoc_item_array[idx].net_price * edoc_item_array[idx].qty, 2, length, PIPE)) == -1) return length;
  if((length = cat_s(edoc_item_array[idx].art_no, length, PIPE)) == -1) return length;
  if((length = cat_s("01", length, PIPE)) == -1) return length;
  if((length = cat_f(edoc_item_array[idx].net_price, 3, length, PIPE)) == -1) return length;
  if((length = cat_f(edoc_item_array[idx].qty * edoc_item_array[idx].net_price, 2, length, CR)) == -1) return length;
  
  return length;
}

int de_dedi(int offset, int idx)
{
  int length = offset;

  if((length = cat_s("DEDI", length, PIPE)) == -1) return length;
  if((length = cat_s(edoc_item_array[idx].descr, length, CR)) == -1) return length;

  if((length = cat_s("DEDI", length, PIPE)) == -1) return length;
  if((length = cat_d(edoc_item_array[idx].vat_no, 1, length, CR)) == -1) return length;

  if(edoc_item_array[idx].flag_percep == TRUE){
    if((length = cat_s("DEDI|P", length, CR)) == -1) return length;
  }

  return length;
}

int de_dedr(int offset, int idx)
{
  int length = offset;
  int rc = 0;

  if((length = cat_s("DEDR|false", length, PIPE)) == -1) return length;
 /* if(edoc_item_array[idx].disc_msam_perc != 0){
    if((length = cat_f(edoc_item_array[idx].disc_msam_amount, 2, length, CR)) == -1) return length;
  }
  else{*/
    if((length = cat_f(floor_price(edoc_item_array[idx].disc_amount * edoc_item_array[idx].qty + edoc_item_array[idx].disc_base_goods_value), 2, length, CR)) == -1) return length;
  //}
  
  return length;
}

int de_deim(int offset, int idx)
{
  int length = offset;
  int rc = 0;

  if((length = cat_s("DEIM", length, PIPE)) == -1) return length;
  if((length = cat_f(edoc_item_array[idx].vat_amount, 2, length, PIPE)) == -1) return length;
  if(edoc_item_array[idx].vat_no != 0){
	  if((length = cat_f(edoc_item_array[idx].net_price, 2, length, PIPE)) == -1) return length;
  }
  else{
	  if((length = cat_f(0, 2, length, PIPE)) == -1) return length;
  }
 
  if((length = cat_f(edoc_item_array[idx].vat_amount, 2, length, PIPE)) == -1) return length;
  if((length = cat_f(edoc_item_array[idx].vat_perc, 2, length, PIPE)) == -1) return length;
  if((length = cat_s("", length, PIPE)) == -1) return length;
	if(edoc_item_array[idx].vat_no != 0){
		if((length = cat_s("10", length, PIPE)) == -1) return length;
	}
	else{
		if((length = cat_s("20", length, PIPE)) == -1) return length;
	}
  
  /* Codigos de tipos de sistema de calculo del isc */
  if((length = cat_s("|1000|IGV|VAT", length, CR)) == -1) return length;

  return length;
}

int de_dr(int offset)
{
  int length = offset;
  int rc = 0;

  get_paym(&paym);
	
  if((length = cat_s("DR|false|019|Glosa", length, PIPE)) == -1) return length;
  if((length = cat_s(curr_code_iso(paym.currency_no), length, PIPE)) == -1) return length;
  if((length = cat_s(de_descuentos, length, PIPE)) == -1) return length;
  if((length = cat_s("1000|IGV|VAT", length, CR)) == -1) return length;
  paym.paym_cd++;

  return length;
}

int de_di(int offset)
{
  int length = offset;

  if((length = cat_s("DI", length, PIPE)) == -1) return length;
  if((length = cat_s(de_impuesto, length, PIPE)) == -1) return length;
  if((length = cat_s(de_impuesto, length, PIPE)) == -1) return length;
  if((length = cat_s("1000|IGV|VAT", length, CR)) == -1) return length;

  return length;
}

int de_pes1(int offset)
{
  int length = offset;
  
  if((length = cat_s("PES|Descuentos", length, CR)) == -1) return length;
  
  return length;
}

int de_pesd1(int offset)
{
  int length = offset;
  short i, j;
  int line = 1;
  int rc = 0;
	int l_line = 0;
	short n_lines = 0;
  double  tot_disc_amount;
  
  if((length = cat_s("PESD", length, PIPE)) == -1) return length;
  if((length = cat_d(line, 1, length, PIPE)) == -1) return length;
  if((length = cat_s("DESCUENTOS OFRECIDOS", length, CR)) == -1) return length;

  for(i = 0; i < desc_items; i++)
  {
		memset(aux, 0, sizeof(aux));
		memset(n_aux, 0, sizeof(n_aux));
		sprintf(aux, "%d   %s", desc_item[i].times, desc_item[i].descr);
		
		l_line = strlen(aux);
		sprintf(n_aux, "%.2f-", desc_item[i].desc);
		l_line += strlen(n_aux);

		n_lines = l_line/wp_de_data_conf.de_pdf_len_line + 1;

		for(j = 0; j < n_lines; j++){
			line++;

			if((length = cat_s("PESD", length, PIPE)) == -1) return length;	
			if((length = cat_d(line, 1, length, PIPE)) == -1) return length;

			if((l_line -  j * wp_de_data_conf.de_pdf_len_line) > wp_de_data_conf.de_pdf_len_line){
				memset(line_aux, ' ', sizeof(line_aux));
				memcpy(line_aux, aux + j * wp_de_data_conf.de_pdf_len_line, wp_de_data_conf.de_pdf_len_line);
				line_aux[wp_de_data_conf.de_pdf_len_line] = '\0';
			}
			else{
				memset(line_aux, ' ', sizeof(line_aux));
				memcpy(line_aux, aux + j * wp_de_data_conf.de_pdf_len_line , strlen(aux + j * wp_de_data_conf.de_pdf_len_line));
				line_aux[wp_de_data_conf.de_pdf_len_line] = '\0';
			}

			if(j == (n_lines - 1)){
				sprintf(n_aux, "%.2f-", desc_item[i].desc);
				memcpy(line_aux + wp_de_data_conf.de_pdf_len_line - strlen(n_aux), n_aux, strlen(n_aux));
			}
			
			line_aux[wp_de_data_conf.de_pdf_len_line] = '\0';
			if((length = cat_s(line_aux, length, CR)) == -1) return length;
		}
	}
	
	line++;
	memset(line_aux, ' ', sizeof(line_aux));

	if((length = cat_s("PESD", length, PIPE)) == -1) return length;
	if((length = cat_d(line, 1, length, PIPE)) == -1) return length;
  tot_disc_amount = (genvar.price_incl_vat == INCLUSIVE) ? floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))
                                                           : floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_EXCL));
	memcpy(line_aux, "TOTAL DESCUENTOS", 16);
	memset(n_aux, 0, sizeof(n_aux));
  if(tot_disc_amount < 0.0)
    sprintf(n_aux, "%.2f-", -tot_disc_amount);
  else
    strcpy(n_aux, "0.00-");
  
	memcpy(line_aux + wp_de_data_conf.de_pdf_len_line - strlen(n_aux), n_aux, strlen(n_aux));
	line_aux[wp_de_data_conf.de_pdf_len_line] = '\0';
	if((length = cat_s(line_aux, length, CR)) == -1) return length;
	
	return length;
}

int de_pes2(int offset)
{
  int length = offset;

  if((length = cat_s("PES|FormaPago", length, CR)) == -1) return length;
	
  return length;
}

int de_pesd2(int offset)
{
	int length = offset;
	int lines = 1;
	short used_paym_cd, extra_paym_cd;
	PAYMENT_DEF paym2;
	double extra_amount, tot_paym;
	short turkey_line = 0;
	double tot_incl;
	int has_valepavo = 0;
  
	tot_incl = tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL));
	extra_paym_cd = (short)tot_ret_double(TOT_CREDIT_PAYM_CD);
	extra_amount = floor_price(tot_ret_double(TOT_LCREDIT_VAT_AMNT)) + floor_price(tot_ret_double(TOT_LCREDIT_AMNT));

	if(get_num_vale_pavo() != 0){
		used_paym_cd = PAYM_WAY_3;
		if (tot_ret_double((short)((used_paym_cd%MAX_PAYM_WAYS)+TOT_PAYM_0)) != 0.0) 
		{
			paym2.paym_cd=used_paym_cd%MAX_PAYM_WAYS;
			get_paym(&paym2);
			tot_paym = floor_price((tot_ret_double((short)(TOT_PAYM_0 + paym2.paym_cd)) * paym2.curr_standard)/paym2.curr_rate);

			if (tot_paym > 0)
			{
				has_valepavo = 1;
				if((length = cat_s("PESD", length, PIPE)) == -1) return length;
				if((length = cat_d(used_paym_cd + turkey_line, 1, length, PIPE)) == -1) return length;
				lines++;
				turkey_line++;
				memset(line_aux, ' ', sizeof(line_aux));
				line_aux[wp_de_data_conf.de_pdf_len_line] = '\0';
				ftoa_price(-tot_paym, sizeof(aux) - 1, aux);
				memset(auxb, 0, sizeof(auxb));
				sprintf(auxb, "ANT.VALE PAVO(No %d)", cur_valepavo.vale_no);
				memcpy(line_aux, auxb, strlen(auxb));
				memset(auxb, 0, sizeof(auxb));
				format2decimals(auxb, aux);
				memcpy(line_aux + wp_de_data_conf.de_pdf_len_line - strlen(auxb), auxb, strlen(auxb));
				if((length = cat_s(line_aux, length, CR)) == -1) return length;

				if((length = cat_s("PESD", length, PIPE)) == -1) return length;
				if((length = cat_d(used_paym_cd + turkey_line, 1, length, PIPE)) == -1) return length;
				lines++;
				turkey_line++;
				memset(line_aux, ' ', sizeof(line_aux));
				line_aux[wp_de_data_conf.de_pdf_len_line] = '\0';
				memset(aux, 0, sizeof(aux));
				ftoa_price(tot_incl - tot_paym, sizeof(aux) - 1, aux);
				memset(auxb, 0, sizeof(auxb));
				sprintf(auxb, "NETO A PAGAR");
				memcpy(line_aux, auxb, strlen(auxb));
				memset(auxb, 0, sizeof(auxb));
				format2decimals(auxb, aux);
				memcpy(line_aux + wp_de_data_conf.de_pdf_len_line - strlen(auxb), auxb, strlen(auxb));
				if((length = cat_s(line_aux, length, CR)) == -1) return length;
			}
		}
	}
	
	used_paym_cd = 1;
	while (used_paym_cd <= MAX_PAYM_WAYS) 
	{
		if (tot_ret_double((short)((used_paym_cd%MAX_PAYM_WAYS)+TOT_PAYM_0)) != 0.0) 
		{
			paym2.paym_cd=used_paym_cd%MAX_PAYM_WAYS;
			get_paym(&paym2);
			tot_paym = floor_price((tot_ret_double((short)(TOT_PAYM_0 + paym2.paym_cd)) * paym2.curr_standard)/paym2.curr_rate);

			if ((has_valepavo) && (used_paym_cd == PAYM_WAY_3))
			{
				/* Do not print the process has alredy done it */
			}
			else{
				if((length = cat_s("PESD", length, PIPE)) == -1) return length;
				if((length = cat_d(used_paym_cd + turkey_line, 1, length, PIPE)) == -1) return length;
				if (used_paym_cd == extra_paym_cd && 0.0 != extra_amount)
					tot_paym -= extra_amount;

				memset(line_aux, ' ', sizeof(line_aux));
				line_aux[wp_de_data_conf.de_pdf_len_line] = '\0';
				sprintf(aux, "%.2f", tot_paym);

				memcpy(line_aux, paym2.paym_descr, strlen(paym2.paym_descr));
				memcpy(line_aux + wp_de_data_conf.de_pdf_len_line - strlen(aux), aux, strlen(aux));

				if((length = cat_s(line_aux, length, CR)) == -1) return length;
				lines++;
			}
		}
		++used_paym_cd;
	}
	
	memset(aux, 0, sizeof(aux));
	memset(auxb, 0, sizeof(auxb));
	memset(line_aux, ' ', sizeof(line_aux));
	line_aux[wp_de_data_conf.de_pdf_len_line] = '\0';
	ftoa_price(get_invoice_donation_currmode(), sizeof(aux), aux);
	if((length = cat_s("PESD", length, PIPE)) == -1) return length;
	if((length = cat_d(lines, 1, length, PIPE)) == -1) return length;
	memcpy(line_aux, "DONACION", 8);
  printf_log("don[%s]", aux);
	if(get_invoice_donation_currmode() != 0){
		format2decimals(auxb, aux);
	}
	else{
		memcpy(auxb, "0.00", 4);
	}
	memcpy(line_aux + wp_de_data_conf.de_pdf_len_line - strlen(auxb), auxb, strlen(auxb));
	if((length = cat_s(line_aux, length, CR)) == -1) return length;
	lines++;
	
	memset(aux, 0, sizeof(aux));
	memset(line_aux, ' ', sizeof(line_aux));
	line_aux[wp_de_data_conf.de_pdf_len_line] = '\0';
	sprintf(aux, "%.2f", (amount_due() * -1));
	if((length = cat_s("PESD", length, PIPE)) == -1) return length;
	if((length = cat_d(lines, 1, length, PIPE)) == -1) return length;
	memcpy(line_aux, "VUELTO", 6);
	memcpy(line_aux + wp_de_data_conf.de_pdf_len_line - strlen(aux), aux, strlen(aux));
	if((length = cat_s(line_aux, length, CR)) == -1) return length;

	return length;
}

int de_pes3(int offset)
{
  int length = offset;

  if((length = cat_s("PES|MensajesDt", length, CR)) == -1) return length;
	
  return length;
}

int de_pesd3(int offset)
{
  int length = offset;
  int rc = 0;
  long linvoice_no = 0;
  int lines = 0;

	if ((get_num_vale_pavo() == 1) && (get_valeturkey_count() >= 1))
	{
    memset(&voucher, 0, sizeof(VOUCHER_VALE_DEF)); 
    rc = get_voucher_bo_turkey(cur_valepavo.vale_no, 
                          cur_valepavo.vale_type, 
                          USED, 
                          pos_system.store_no,
                          cust.cust_no,
                          pos_invoice.invoice_till_no , //till_no
                          selected_invoice_printer +1, //invoice_type
                          linvoice_no , //invoice_no
                          pos_invoice.invoice_date, //invoice_date
                          pos_invoice.invoice_sequence_no,
                          &voucher,3);

    if (rc != SUCCEED)
        return rc;
	
    if((length = cat_s("PESD", length, PIPE)) == -1) return length;
    lines++;
    if((length = cat_d(lines, 1, length, PIPE)) == -1) return length;
    memset(aux, 0, sizeof(aux));
    sprintf(aux, "RazonSocial:%-28.28s" ,cur_valepavo.name_cust);
    if((length = cat_s(aux, length, CR)) == -1) return length;
	
    if((length = cat_s("PESD", length, PIPE)) == -1) return length;
    lines++;
    if((length = cat_d(lines, 1, length, PIPE)) == -1) return length;
    memset(aux, 0, sizeof(aux));
    sprintf(aux, "RUC/DNI    :%-20.20s" ,cur_valepavo.fisc_no);
    if((length = cat_s(aux, length, CR)) == -1) return length;
	
    if((length = cat_s("PESD", length, PIPE)) == -1) return length;
    lines++;
    if((length = cat_d(lines, 1, length, PIPE)) == -1) return length;
    memset(aux, 0, sizeof(aux));
    sprintf(aux, "Nro Doc    :%-20.20s" ,cur_valepavo.fact_no);
    if((length = cat_s(aux, length, CR)) == -1) return length;
    
    if((length = cat_s("PESD", length, PIPE)) == -1) return length;
    lines++;
    if((length = cat_d(lines, 1, length, PIPE)) == -1) return length;
    memset(aux, 0, sizeof(aux));
    sprintf(aux, "Tipo Vale  :%s" ,(cur_valepavo.vale_type==1?"SAN FERNANDO":"ARO"));
    if((length = cat_s(aux, length, CR)) == -1) return length;
    
    if((length = cat_s("PESD", length, PIPE)) == -1) return length;
    lines++;
    if((length = cat_d(lines, 1, length, PIPE)) == -1) return length;
    memset(aux, 0, sizeof(aux));
    sprintf(aux, "Nro Vale   :%d",cur_valepavo.vale_no);
    if((length = cat_s(aux, length, CR)) == -1) return length;

  }
  
  if(is_detraction == 1){
    if((length = cat_s("PESD", length, PIPE)) == -1) return length;
    lines++;
    if((length = cat_d(lines, 1, length, PIPE)) == -1) return length;
    if((length = cat_s("La venta contiene detraccion", length, CR)) == -1) return length;
  }
  
  if((length = cat_s("PESD", length, PIPE)) == -1) return length;
  lines++;
  if((length = cat_d(lines, 1, length, PIPE)) == -1) return length;
  memset(aux, 0, sizeof(aux));
  get_invoice_no(invoice_no);
  get_current_time(prn_time);
  format_string(&string_time5, prn_time);
  prn_fmt_date(pos_system.run_date, genvar.date_format, mon_names, prn_date);
  sprintf(aux, "%-13.13s        %-5.5s         %02d%02d      %04d%6s",prn_date, string_time5.result, pos_system.store_no, pos_system.till_no, pos_system.run_date/10000, invoice_no);
  if((length = cat_s(aux, length, CR)) == -1) return length;

  return length;
}

int de_pe(int offset)
{
  int length = offset;
  char invoice_no[6 + 1];

	/* DIRLOCAL1 */
	if((length = cat_s("PE|DIRLOCAL1", length, PIPE)) == -1) return length;
	if((length = cat_s(genvar.address, length, CR)) == -1) return length;
	
	/* DIRLOCAL2 */
	if((length = cat_s("PE|DIRLOCAL2", length, PIPE)) == -1) return length;
	if((length = cat_s(array_dist[pos_system.store_no], length, CR)) == -1) return length;
	
	/* TELEFLOCAL */
	if((length = cat_s("PE|TELEFLOCAL", length, PIPE)) == -1) return length;
	if((length = cat_s(genvar.tax_txt1 + 6, length, CR)) == -1) return length;
	
	/* NUMSERIE */
	if((length = cat_s("PE|NUMSERIE", length, PIPE)) == -1) return length;
	if((length = cat_s(till.till_id, length, CR)) == -1) return length;
	
	/* PASAPORTE */
	if((length = cat_s("PE|PASAPORTE", length, PIPE)) == -1) return length;
	memset(aux, 0, sizeof(aux));
	sprintf(aux, "%02d-%06ld", cust.store_no, cust.cust_no);
	if((length = cat_s(aux, length, CR)) == -1) return length;
	
	/* CAJA */
	if((length = cat_s("PE|CAJA", length, PIPE)) == -1) return length;
	memset(aux, 0, sizeof(aux));
	sprintf(aux, "%d %d", pos_invoice.invoice_till_no, pos_invoice.invoice_cashier_no);
	if((length = cat_s(aux, length, CR)) == -1) return length;
	
	/* TOTALART */
	if((length = cat_s("PE|TOTALART", length, PIPE)) == -1) return length;
	memset(aux, 0, sizeof(aux));
	ftoa(tot_ret_double(TOT_PACKS), TOTAL_BUF_SIZE, aux);
	if((length = cat_s(aux, length, CR)) == -1) return length;
	
	/* CENTRALTELEF */
	if((length = cat_s("PE|CENTRALTELEF", length, PIPE)) == -1) return length;
	if((length = cat_s("", length, CR)) == -1) return length;
	
	/* PAGWEB */
	if((length = cat_s("PE|PAGWEB", length, PIPE)) == -1) return length;
	if((length = cat_s(genvar.prom_txt_bot3, length, CR)) == -1) return length;
	
	/* NUMINTERNO */
	if((length = cat_s("PE|NUMINTERNO", length, PIPE)) == -1) return length;
	get_invoice_no(invoice_no);
	memset(aux, 0, sizeof(aux));
	sprintf(aux, "%02d %6s", pos_system.store_no, invoice_no);
	if((length = cat_s(aux, length, CR)) == -1) return length;
	
	return length;
}

int de_gen_body()
{
	int length = 0;
	int i = 0;
  
	if((length = de_gen_body_init()) == -1) return length;
  printf_log("de_gen_body_init");
	if((length = de_en(length)) == -1) return length;printf_log("de_en");
	if((length = de_doc(length)) == -1) return length;printf_log("de_doc");
	if((length = de_dn(length)) == -1) return length;printf_log("de_dn");
	
  for(i = 0; i < n_items; i++) {
    if((length = de_de(length, i)) == -1) return length;printf_log("de_de");
    if((length = de_dedi(length, i)) == -1) return length; printf_log("de_dedi");
    if((edoc_item_array[i].disc_msam_perc != 0) || (edoc_item_array[i].disc_manual != 0)){
      if((length = de_dedr(length, i)) == -1) return length;printf_log("de_dedr");
    }
    paym.paym_cd = 1;

    if((length = de_deim(length, i)) == -1) return length;printf_log("de_deim");
  }
  
  if((length = de_di(length)) == -1) return length;printf_log("de_di");
  if(desc_items > 0){
		if((length = de_pes1(length)) == -1) return length;printf_log("de_pes1");
		if((length = de_pesd1(length)) == -1) return length;printf_log("de_pesd1");
  }

	if((length = de_pes2(length)) == -1) return length;printf_log("de_pes2");
	if((length = de_pesd2(length)) == -1) return length; printf_log("de_pesd2");
	if((length = de_pes3(length)) == -1) return length;printf_log("de_pes3");
	if((length = de_pesd3(length)) == -1) return length;printf_log("de_pesd3");
	if((length = de_pe(length)) == -1) return length; printf_log("de_pe");
  
	return length;
}

void init_reqs(int * len)
{
  int i;
  
  for(i = 0; i < N_FIELDS; i++){
    req_buffer[i] = NULL;
    len[i] = 0;
  }
}

int assemble_messages(int * len, const short op)
{
  if((len[HEAD] = de_head()) == ALLOC_ERROR) return len[HEAD];
	req_buffer[BODY] = body_buffer;
	if((len[BODY] = de_mess_func[op]()) == ALLOC_ERROR) return len[BODY];
	if((len[TAIL] = de_tail()) == ALLOC_ERROR) return len[TAIL];
  
  return DE_OK;
}

/* "Configuracion POS" message-1 */
int de_pos_config()
{
	char rec_buffer[BUF_RX_LEN];
	int rc = DE_OK;
	int res_len = 0;
	int len[N_FIELDS];

	memset(rec_buffer, 0, sizeof(rec_buffer));
	init_reqs(len);
  if((rc = assemble_messages(len, EDOC_CONFIGURATION)) != DE_OK) return rc;
	conc_message(len);
	rc = de_snd_rcv(mess_buffer, mess_len, rec_buffer, &res_len);
  clean_mem(mess_buffer, mess_len);
	return rc;
}

/* "Generar DE" message-2 */
int de_gen(void)
{
	char rec_buffer[BUF_RX_LEN];
	int rc = DE_OK;
	int res_len = 0;
	int len[N_FIELDS];

	memset(rec_buffer, 0, sizeof(rec_buffer));
  init_reqs(len);
  if((rc = assemble_messages(len, EDOC_GENERATION)) != DE_OK) return rc;
	conc_message(len);
	rc = de_snd_rcv(mess_buffer, mess_len, rec_buffer, &res_len);
	clean_mem(mess_buffer, mess_len);
  clear_edoc_vars();
  ed_serie_correlative_hash(rec_buffer, res_len);
	return rc;
}

/* "Confirmar DE" message-3 */
int de_conf(void)
{
	char rec_buffer[BUF_RX_LEN];
	int rc = DE_OK;
	int res_len = 0;
	int len[N_FIELDS];

	memset(rec_buffer, 0, sizeof(rec_buffer));
  init_reqs(len);
  if((rc = assemble_messages(len, EDOC_CONFIRMATION)) != DE_OK) return rc;
	conc_message(len);
	rc = de_snd_rcv(mess_buffer, mess_len, rec_buffer, &res_len);
  clean_mem(mess_buffer, mess_len);
	return rc;
}
