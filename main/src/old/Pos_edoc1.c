/*
 *     Module Name       : POS_EDOC1.C
 *
 *     Type              : Electronic Document Generation Utilitiess Functions
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
 * 01-May-2015 Initial Release Electronic Document development       M.L.S.D.
 * --------------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <winsock.h>
#include <io.h>

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

long edoc_item_length = 0;
short training_mode = TRAINING_MODE_OFF;
short gen_edoc = TRUE;
short document_type = BOLETA;
short emisor_id_type = RUC;
unsigned char de_neto[64] = {"0.00"};
unsigned char de_impuesto[64] = {"0.00"};
unsigned char de_descuentos[64] = {"0.00"};
unsigned char de_recargos[64] = {"0.00"};
unsigned char de_total[64] = {"0.00"};

double de_neto_d = 0;
double de_impuesto_d = 0;
double de_descuentos_d = 0;
double de_total_d = 0;

double de_gravadas = 0;
double de_gratuitas = 0;
double de_exoneradas = 0;
double de_inafectas = 0;
double de_percepcion = 0;

double detraction_total = 0;

int n_items = 0;

DE_DATA_CONF wp_de_data_conf;
EDOC_ITEM edoc_item_array[1024] = {0};
EDOC_DESC_ITEM desc_item[1024] = {0};
char serie_corr[64] = {0};
char de_serie[10 + 1] = {0};
long de_correlative = 0;
char hash_resp[32] = {0};

void clean_mem(char * buf, int len)
{
  memset(buf, 0, len);
  free(buf);
  buf = NULL;
}

void clear_edoc_vars(void)
{
  memset(&edoc_item_array, 0, n_items * sizeof(EDOC_ITEM));
	memset(&desc_item, 0, n_items * sizeof(EDOC_DESC_ITEM));
	memset(de_neto, 0, sizeof(de_neto));
	memset(de_impuesto, 0, sizeof(de_impuesto));
	memset(de_descuentos, 0, sizeof(de_descuentos));
	memset(de_recargos, 0, sizeof(de_recargos));
	memset(de_total, 0, sizeof(de_total));
}

void ed_serie_correlative_hash(char * rec_buffer, int res_len)
{
	int i = 0, flag_tab = 0;
	int min = 0, max = 0, pos_hash_tab = 0;
	char aux_correlative[32];
	char * paux;

	for(i = 0; i < res_len; i++)
	{
		if(rec_buffer[i] == (9 & 0xFF))
		{
			flag_tab++;
			if(flag_tab == 2)
				min = i;
			if(flag_tab == 3)
				max = i;
			if(flag_tab == 4)
				pos_hash_tab = i;
		}
	}

	if(max != 0 || min != 0){
		memcpy(serie_corr, rec_buffer + min + 1, max - min - 1);
		memcpy(hash_resp, rec_buffer + pos_hash_tab + 1, res_len - 1 - pos_hash_tab - 1);

		memset(de_serie, 0, sizeof(de_serie));
		paux = strchr(serie_corr, '-');

		memcpy(de_serie, serie_corr, paux - serie_corr);
		memcpy(aux_correlative, paux + 1, strlen(serie_corr) - (paux - serie_corr));
		de_correlative = atol(aux_correlative);
		printf_log("serie[%s] correlative[%ld]", de_serie, de_correlative);

		strcpy(pos_invoice.invoice_serie, de_serie);  //mlsd ctree dni
		pos_invoice.invoice_correlative = de_correlative;
    
		if(document_type == FACTURA){
		  strcpy(pos_system.last_serie_fac, de_serie);
		  pos_system.last_corr_fac = de_correlative;
		  pos_upda_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
		}
		else{
		  strcpy(pos_system.last_serie_bol, de_serie);
		  pos_system.last_corr_bol = de_correlative;
		  pos_upda_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
		}
	}

	if(de_reverse_mode == YES){
		memset(pos_invoice.invoice_serie, 0, sizeof(pos_invoice.invoice_serie));
		pos_invoice.invoice_correlative = 0;
	}  
}

void search_file(FILE * f, const char * str, char * res)
{
  char line[256];
  char l_aux[256];
  char * ptr;
  
  memset(line, 0, sizeof(line));
  memset(l_aux, 0, sizeof(l_aux));
  
  while(fgets(line, sizeof(line), f))
  {
    memcpy(l_aux, line, strlen(line) - 1);
    if(strncmp(l_aux, str, strlen(str)) == 0){
      if((ptr = strchr(l_aux, '=')) != NULL){
        sprintf(res, "%s", ptr + 1);
        return;
      }
    }
  }
}

void de_read_conf_file()
{
  FILE * conf_file;
  char n_aux[32];
  
  conf_file = fopen("winpos.conf", "r");
  
  memset(&wp_de_data_conf, 0, sizeof(wp_de_data_conf));
  
  
  search_file(conf_file, "HOST", wp_de_data_conf.host);
  printf_log("HOST[%s]", wp_de_data_conf.host);
  
  memset(n_aux, 0, sizeof(n_aux));
  search_file(conf_file, "PORT_EPOS", n_aux);
  wp_de_data_conf.port_epos = atol(n_aux);
  printf_log("port_epos[%ld]", wp_de_data_conf.port_epos);
  
  search_file(conf_file, "HEADER_REQ", wp_de_data_conf.header_req);
  printf_log("HEADER_REQ[%s]", wp_de_data_conf.header_req);
  search_file(conf_file, "TAIL_REQ", wp_de_data_conf.tail_req);
  printf_log("TAIL_REQ[%s]", wp_de_data_conf.tail_req);

  search_file(conf_file, "COD_GRAVADAS", wp_de_data_conf.cod_gravadas);
  printf_log("COD_GRAVADAS[%s]", wp_de_data_conf.cod_gravadas);
  search_file(conf_file, "COD_EXONERADAS", wp_de_data_conf.cod_exoneradas);
  printf_log("COD_EXONERADAS[%s]", wp_de_data_conf.cod_exoneradas);
  search_file(conf_file, "COD_PERCEPCION", wp_de_data_conf.cod_percepcion);
  printf_log("COD_PERCEPCION[%s]", wp_de_data_conf.cod_percepcion);

  search_file(conf_file, "UBIGEO", wp_de_data_conf.ubigeo);
  printf_log("UBIGEO[%s]", wp_de_data_conf.ubigeo);
  search_file(conf_file, "DESCR_BOL", wp_de_data_conf.descr_bol);
  printf_log("DESCR_BOL[%s]", wp_de_data_conf.descr_bol);
  search_file(conf_file, "DESCR_FAC", wp_de_data_conf.descr_fac);
  printf_log("DESCR_FAC[%s]", wp_de_data_conf.descr_fac);
  search_file(conf_file, "DESCR_DIR", wp_de_data_conf.descr_dir);
  printf_log("DESCR_DIR[%s]", wp_de_data_conf.descr_dir);
  search_file(conf_file, "DESCR_DIST", wp_de_data_conf.descr_dist);
  printf_log("DESCR_DIST[%s]", wp_de_data_conf.descr_dist);
  search_file(conf_file, "DATE_FORMAT", wp_de_data_conf.date_format);
  printf_log("DATE_FORMAT[%s]", wp_de_data_conf.date_format);
  
  memset(n_aux, 0, sizeof(n_aux));
  search_file(conf_file, "DE_PDF_LEN_LINE", n_aux);
  wp_de_data_conf.de_pdf_len_line = atoi(n_aux);
  printf_log("de_pdf_len_line[%d]", wp_de_data_conf.de_pdf_len_line);
  
  return;
}


char * curr_code_iso(long currency_no)
{
	switch(currency_no){
		case 1: return CURR_PERU;
		case 2: return CURR_US;
		default: return CURR_ERROR;
	}
}

int unid2char(char c, char * buffer)
{
    switch(c){ 
        case '0': break; 
        case '1': strcpy(buffer, "UNO "); break; 
        case '2': strcpy(buffer, "DOS "); break; 
        case '3': strcpy(buffer, "TRES "); break; 
        case '4': strcpy(buffer, "CUATRO "); break; 
        case '5': strcpy(buffer, "CINCO "); break; 
        case '6': strcpy(buffer, "SEIS "); break; 
        case '7': strcpy(buffer, "SIETE "); break; 
        case '8': strcpy(buffer, "OCHO "); break; 
        case '9': strcpy(buffer, "NUEVE ");  break; 
        default: strcpy(buffer, "ERR ");
    }
    return 0;
}

int dec2char(char c, char cp1,char * buffer, char * buffer_u, short flag)
{
  switch(c)
  { 
      case '0': break; 
      case '1': 
        if(cp1 != '0'){ 
          switch(cp1){ 
            case '1':strcpy(buffer,"ONCE "); memset(buffer_u, 0, strlen(buffer_u)); break; 
            case '2':strcpy(buffer,"DOCE "); memset(buffer_u,0, strlen(buffer_u)); break; 
            case '3':strcpy(buffer,"TRECE "); memset(buffer_u,0, strlen(buffer_u)); break; 
            case '4':strcpy(buffer,"CATORCE "); memset(buffer_u,0, strlen(buffer_u)); break; 
            case '5':strcpy(buffer,"QUINCE "); memset(buffer_u,0, strlen(buffer_u)); break; 
            case '6':strcpy(buffer,"DIECISEIS "); memset(buffer_u,0, strlen(buffer_u)); break; 
            case '7':strcpy(buffer,"DIECISIETE "); memset(buffer_u,0, strlen(buffer_u)); break; 
            case '8':strcpy(buffer,"DIECIOCHO ");  memset(buffer_u,0, strlen(buffer_u)); break;
            case '9':strcpy(buffer,"DIECINUEVE "); memset(buffer_u,0, strlen(buffer_u)); break;
            default: strcpy(buffer,"ERR ");
          }
           
          if(flag == 1)
            strcat(buffer, "MIL ");
        }else
           strcpy(buffer,"DIEZ ");
        break; 
      case '2': 
        if(cp1 !='0') strcpy(buffer,"VEINTI");
        else strcpy(buffer,"VEINTE ");
        break; 
      case '3':
        if(cp1 !='0') strcpy(buffer,"TREINTA Y ");
        else strcpy(buffer,"TREINTA ");
        break; 
      case '4':
        if(cp1 !='0') strcpy(buffer,"CUARENTA Y ");
        else strcpy(buffer,"CUARENTA "); 
        break; 
      case '5':
        if(cp1 !='0') strcpy(buffer,"CINCUENTA Y ");
        else strcpy(buffer,"CINCUENTA ");
        break; 
      case '6':
        if(cp1 !='0') strcpy(buffer,"SESENTA Y ");
        else strcpy(buffer,"SESENTA "); 
        break; 
      case '7':
        if(cp1 !='0') strcpy(buffer,"SETENTA Y ");
        else strcpy(buffer,"SETENTA "); 
        break; 
      case '8':
        if(cp1 !='0') strcpy(buffer,"OCHENTA Y "); 
        else strcpy(buffer,"OCHENTA "); 
        break;
      case '9':
        if(cp1 !='0') strcpy(buffer,"NOVENTA Y ");
        else strcpy(buffer,"NOVENTA ");
        break; 
  } 

  return 0;
}

int cien2char(char c, char cp, char cpp, char * buffer)
{
  switch(c)
  { 
    case '0': break; 
    case '1': 
      if(cp =='0' && cpp =='0')
         strcpy(buffer,"CIEN "); 
      else
         strcpy(buffer,"CIENTO ");
      break; 
    case '2': strcpy(buffer,"DOSCIENTOS "); break; 
    case '3': strcpy(buffer,"TRESCIENTOS "); break; 
    case '4': strcpy(buffer,"CUATROCIENTOS "); break; 
    case '5': strcpy(buffer,"QUINIENTOS "); break; 
    case '6': strcpy(buffer,"SEISCIENTOS "); break; 
    case '7': strcpy(buffer,"SETECIENTOS "); break; 
    case '8': strcpy(buffer,"OCHOCIENTOS "); break; 
    case '9': strcpy(buffer,"NOVECIENTOS "); break; 
  } 

  return 0;
}

/*********************************************/
/*funcion que convierte de numeros a palabras*/
/*********************************************/
int NoToLet_de(char *A, char * result){ 
  char *numero=A;
  char decimal[5], entero[50];
  char Unidades[64];
  char decenas[64];
  char centenas[64];
  char miles[64];
  char dec_miles[64];
  char Letras_send[145];
  int point = 0, i = 0;
  int ancho3 = strlen(numero);
  short flag_negativo = 0;
  char * aux;

  memset(decimal, 0, sizeof(decimal));
  memset(entero, 0, sizeof(entero));
  memset(Unidades, 0, sizeof(Unidades));
  memset(decenas, 0, sizeof(decenas));
  memset(centenas, 0, sizeof(centenas));
  memset(miles, 0, sizeof(miles));
  memset(dec_miles, 0, sizeof(dec_miles));
  memset(Letras_send, 0, sizeof(Letras_send));

  if(numero[0] == '-')
  {
    flag_negativo = 1;
    aux = numero + 1;
    numero = aux;
  }
  
  for(i = 0; i < ancho3; i++){
    if(numero[i] == '.')
      point = i;
  }
  
  memcpy(decimal, numero + point + 1, ancho3 - point - 1);
  memcpy(entero, numero, point);

  i = (int)strlen(entero) - 1;

  if(i < 0)
      goto armar;

  unid2char(entero[i], Unidades);

  i--;
  if(i < 0)
      goto armar;
 
  dec2char(entero[i], entero[i + 1], decenas, Unidades, 0);

  i--;
  if(i < 0)
      goto armar;

  cien2char(entero[i], entero[i + 1], entero[i + 2], centenas);

  i--;
  if(i < 0)
      goto armar;

  if(entero[i] != '1')
    unid2char(entero[i], miles);
    
  strcat(miles + (int)strlen(miles), "MIL ");

  i--;
  if(i < 0)
      goto armar;
  
  dec2char(entero[i], entero[i + 1], dec_miles, miles, 1);

armar:
  strcat(Letras_send, dec_miles);
  strcat(Letras_send + strlen(dec_miles), miles);
  strcat(Letras_send + strlen(miles), centenas);
  strcat(Letras_send + strlen(centenas), decenas);
  strcat(Letras_send + strlen(decenas), Unidades);
  strcat(Letras_send + strlen(Unidades), "Y ");
  strcat(Letras_send + 2, decimal);
  strcat(Letras_send + strlen(decimal),"/100");
  strcat(Letras_send + 4," NUEVOS SOLES");
  
  if(flag_negativo == 0)
    memcpy(result, Letras_send, (int)strlen(Letras_send));
  else{
    memcpy(result, "(-)" , 3);
    memcpy(result + 3, Letras_send, (int)strlen(Letras_send));
  }
    
  return 0; 
}

void format2decimals(char * str2dec, char * str)
{
	int plus = 0;
	short length = 0;
	
	if(str[0] == '-')
		plus++;

	if(strlen(str + plus) <= 2){
		str2dec[0 + plus] = '0';
		memcpy(str2dec + 1  + (int)strlen(str + plus) - 2, ".", 1);
		memcpy(str2dec + 1 + (int)strlen(str + plus) - 1, str + plus + (int)strlen(str + plus) - 2, 2);
    if(strlen(str + plus) == 1){
      str2dec[0 + plus] = '0';
      str2dec[1 + plus] = '.';
      str2dec[2 + plus] = '0';
      str2dec[3 + plus] = str[0];
    }
	}
	else{
		memcpy(str2dec, str + plus, (int)strlen(str + plus) - 2);
		memcpy(str2dec + (int)strlen(str + plus) - 2, ".", 1);
		memcpy(str2dec + (int)strlen(str + plus) - 1, str + plus + (int)strlen(str + plus) - 2, 2);
	}
	
	length = strlen(str2dec);
	if(plus == 1)
		memcpy(str2dec + length, "-", 1);
   
	return;
}

void de_date_format(char * str, long date)
{
  char date_aux[16];

	memset(date_aux, 0, sizeof(date_aux));
  sprintf(date_aux, "%ld", date);
  sprintf(str, "%.4s-%.2s-%s", date_aux, date_aux + 4, date_aux +6);

  return;
}

int ind2unidad(short ind, char * unidad)
{
  switch(ind){
    case 0: strcpy(unidad, "NIU"); break;
    case 2: strcpy(unidad, "KGM"); break;
    default: strcpy(unidad, "ERR");
  }
  return 0;
}

int de_snd_rcv(char * pbuffer, int offset, char * rbuffer, int * res_len)
{
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent * server;
  WSADATA wsaData;
  int iResult = 0;
  
  // Initialize Winsock
	if((iResult = WSAStartup(0x202, &wsaData)) != 0)
  {
    printf_log("Client: WSAStartup() failed with error %d", iResult);
    WSACleanup();
    return -1;
  }

  portno = wp_de_data_conf.port_epos;
  server = gethostbyname(wp_de_data_conf.host);
  if (server == NULL){
    printf_log("gethostbyname error-host name[%s]", wp_de_data_conf.host);
    return -4;
  }
  
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = server->h_addrtype;
  memcpy(&(serv_addr.sin_addr), server->h_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    printf_log("sockfd[%d][%d]", sockfd, WSAGetLastError());
    return -3;
  }
  
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    err_invoke(EPOS_CONNECT_ERROR); 
    printf_log("connect error [%d] Host[%s] Port[%d]", WSAGetLastError(), wp_de_data_conf.host, portno);
    return -5;
  }

  n = send(sockfd, pbuffer, offset, 0);
  if (n < 0){
    printf_log("send error [%d]", WSAGetLastError());
    return -6;
  }
  printf_log("Message sent[%s] n[%d] e[%d]", pbuffer, n, WSAGetLastError());
  
  *res_len = recv(sockfd, rbuffer, BUF_RX_LEN, 0);
  if(*res_len > 0){
    printf_log("Message received [%s] n[%d] e[%d]", rbuffer, *res_len, WSAGetLastError());
  } else if(*res_len == 0) {
    printf_log("connection closed [%d]", WSAGetLastError());
    return -8;
  } else {
    printf_log("recv error [%d]", WSAGetLastError());
    return -9;
  }
  
  closesocket(sockfd);
  WSACleanup();
  
  return DE_OK;
}



int search_string(char * source, char * key, char * end, int * max, int * min)
{
  int i;
  int rc = -1;
  int len_source = strlen(source);
  int len_key = strlen(key);
  int here = 0;
  int flag_start = 0;
  int flag_end = 0;
  int result;
  
  for(i = 0; i < len_source; i++){
    if((result = memcmp(source + i, key, len_key)) == 0 ){
      *min = i + len_key;
      here = i + len_key;
      flag_start = 1;
      break;
    }
  }

  for(i = 0; i < len_source - here; i++){
      if((result = memcmp(source + here + i, end, 1)) == 0 ){
      *max = i + here;
      flag_end = 1;
      break;
    }
  }

  if((flag_start == 1) && (flag_end == 1))
    rc = 0;

  return rc;
}

int find_mmml_disc(_TCHAR * text_line, ART_DESC_MACTMML * disc_array)
{
  _TCHAR aux[16];
  int min = 0;
  int max = 0;
  
  memset(aux, 0, sizeof(aux));

  search_string(text_line, "DISCPERC(", ")", &max, &min);
  
  if(min == 0){
		search_string(text_line, "DISCAMOUNT(", ")", &max, &min);
		memcpy(aux, text_line + min, max - min);
		disc_array->amount = atof(aux); 
  }
  else{
		memcpy(aux, text_line + min, max - min);
		disc_array->perc = atof(aux);  
  }
  
  return 0;
}

int find_qmin_arts_date(_TCHAR * text_line, ART_DESC_MACTMML * disc_array, short case_find)
{
  _TCHAR aux[32];
  _TCHAR aux_start_date[9];
  _TCHAR aux_end_date[9];
  int min = 0;
  int max = 0;
  int offset = 0;
  int rc = 0;
  int arts = 0;
  int grps = 0;
  _TCHAR * paux = text_line;
  _TCHAR * paux1;
  
  memset(aux, 0, sizeof(aux));
  
  if(case_find == 0){
    printf_log("CASE FIND 0 [%s][%d]", __FILE__, __LINE__);
    /* Looking for minimun quantity */
    if((rc = search_string(paux, "QMIN(", ",", &max, &min)) == -1)
      return rc;
      
    memset(aux, 0, sizeof(aux));
    memcpy(aux, paux + min, max - min);
    paux += max;
    disc_array->qty_min = (float) atof(aux);
    
    while(rc != -1){
      if((rc = search_string(paux, "ART(", ")", &max, &min)) != -1){
        memset(aux, 0, sizeof(aux));
        memcpy(aux, paux + min, max - min);
        disc_array->art_no_array[arts] = atol(aux);
        paux += max;
        arts++;
      }
    }
    
    paux1 = text_line;
    while(rc != -1){
      if((rc = search_string(paux1, "ART_GRP(", ")", &max, &min)) != -1){
        memset(aux, 0, sizeof(aux));
        memcpy(aux, paux1 + min, max - min);
        disc_array->art_grp_array[grps] = atol(aux);
        paux1 += max;
        grps++;
      }
    }
    
    memset(aux, 0, sizeof(aux));
    search_string(paux, "DATE_BETWEEN(", ")", &max, &min);
    memcpy(aux, paux + min, max - min);
    memset(aux_start_date, 0, sizeof(aux_start_date));
    memset(aux_end_date, 0, sizeof(aux_end_date));
    memcpy(aux_start_date, aux, 8);
    memcpy(aux_end_date, aux + 9, 8);
    disc_array->start_date = atol(aux_start_date);
    disc_array->end_date = atol(aux_end_date);
  }
  else{
    printf_log("entro else [%s][%d]", __FILE__, __LINE__);
    while(rc != -1){
      if((rc = search_string(paux, "ART(", ")", &max, &min)) != -1){
        memset(aux, 0, sizeof(aux));
        memcpy(aux, paux + min, max - min);
        disc_array->art_no_array[arts] = atol(aux);
        paux += max;
        arts++;
      }
     /* else {
        printf_log("Entro else while");
        return rc;
      }*/
        
    }

    paux1 = text_line;
    printf_log("paux1[%s]", paux1);
    rc = 0;
    while(rc != -1){
      if((rc = search_string(paux1, "ART_GRP(", ")", &max, &min)) != -1){
        memset(aux, 0, sizeof(aux));
        memcpy(aux, paux1 + min, max - min);
        disc_array->art_grp_array[grps] = atol(aux);
        printf_log("art_grp_array[grps] [%ld][%d]", disc_array->art_grp_array[grps], grps);
        paux1 += max;
        grps++;
      }
    }
    
    /* Looking for minimun quantity */
    search_string(paux, "QMIN(", ",", &max, &min);
    memset(aux, 0, sizeof(aux));
    memcpy(aux, paux + min, max - min);
    paux += max;
    disc_array->qty_min = atof(aux);
    
    /* Looking for CARD_TYPE */
    memset(aux, 0, sizeof(aux));
    max = 0;
    min = 0;
    search_string(paux, "CARD_TYPE(", ")", &max, &min);
    memset(aux, 0, sizeof(aux));
    memcpy(aux, paux + min, max - min);
    disc_array->card_type_no = atoi(aux);
    paux += max;
    
    /* Looking for dates */
    max = 0;
    min = 0;
    search_string(paux, "DATE_BETWEEN(", ")", &max, &min);
    memset(aux, 0, sizeof(aux));
    memcpy(aux, paux + min, max - min);
    memset(aux_start_date, 0, sizeof(aux_start_date));
    memset(aux_end_date, 0, sizeof(aux_end_date));
    memcpy(aux_start_date, aux, 8);
    memcpy(aux_end_date, aux + 9, 8);
    disc_array->start_date = atol(aux_start_date);
    disc_array->end_date = atol(aux_end_date);
  }
  return 0;
}

int search_discounts_mactmml(ART_DESC_MACTMML * disc_array,long mmail_no, short maction_no)
{
  MMML_DEF mmml_record;
  short c = 1;
  short cc = 0;
  short flag = 1;
  
  memset(&mmml_record, 0, sizeof(MMML_DEF));
  mmml_record.mmail_no   =        mmail_no;
  mmml_record.maction_no = (short)maction_no;
  mmml_record.line_no = 1;
  
  cc = pos_get_rec(MMML_TYPE, POS_MMML_SIZE, MACTION_MML_IDX, (void*)&mmml_record, (short) keyEQL);
  if(cc != 0)
    return -1;
    
  switch(mmml_record.action_text[0]){
    case 'D':
      find_mmml_disc(mmml_record.action_text, disc_array);
      do{
        if(find_qmin_arts_date(mmml_record.action_text, disc_array, 0) == -1){
          mmml_record.line_no++;
          cc = pos_get_rec(MMML_TYPE, POS_MMML_SIZE, MACTION_MML_IDX, (void*)&mmml_record, (short) keyEQL);
          if(cc != NO_ERROR){
            flag = 0;
          }
        } else {
          flag = 0;
        }
      } while(flag);
      break;
    case '(':
      find_mmml_disc(mmml_record.action_text, disc_array);
      find_qmin_arts_date(mmml_record.action_text, disc_array, 1);
      break;
    default:
      printf_log("Accion no contemplada");
  }

  return 0;
}

void acc_discounts(EDOC_ITEM * aux, ART_DESC_MACTMML * disc_array)
{
  int i = 0;
  
  while(disc_array->art_no_array[i] != 0){
    printf_log("disc_array->art_no_array[i] [%ld][%d]", disc_array->art_no_array[i], i);
    if(((atol(aux->art_no) == disc_array->art_no_array[i]) || (atol(aux->art_grp_no) == disc_array->art_grp_array[i])) && (aux->qty > 0.01)){
      disc_array->qty_acc += aux->qty;
     /* aux->disc_msam_perc[aux->mactmml_times] = disc_array->perc;
      aux->mactmml_times++;
      printf_log("disc_array->perc[%.2f][%d][%d]", disc_array->perc, i);
      aux->disc_msam_amount += disc_array->amount;
      printf_log("disc_array->amount[%.2f][%d][%d]", disc_array->amount, i);*/
    }
    i++;
    printf_log("disc_array->qty_acc[i] [%.3f][%d]", disc_array->qty_acc, i);
  }
}


void match_discounts(EDOC_ITEM * aux, ART_DESC_MACTMML * disc_array)
{
  int i = 0;
  
  while(disc_array->art_no_array[i] != 0){
    printf_log("disc_array->art_no_array[i] [%ld][%d]", disc_array->art_no_array[i], i);
    printf_log("disc_array->art_grp_array[i] [%ld][%d]", disc_array->art_grp_array[i], i);
    if(((atol(aux->art_no) == disc_array->art_no_array[i]) || (atol(aux->art_grp_no) == disc_array->art_grp_array[i])) && (disc_array->qty_acc >= disc_array->qty_min)){
      aux->disc_msam_perc[aux->mactmml_times] = disc_array->perc;
      aux->mactmml_times++;
      printf_log("disc_array->perc[%.2f][%d][%d]", disc_array->perc, i);
      aux->disc_msam_amount += disc_array->amount;
      printf_log("disc_array->amount[%.2f][%d][%d]", disc_array->amount, i);
    }
    i++;
  }
}

void de_totales(void)
{
  int i;
  int j;
  
  de_neto_d = 0;
  de_impuesto_d = 0;
  de_descuentos_d = 0;
  de_total_d = 0;
  de_gravadas = 0;
  de_gratuitas = 0;
  de_exoneradas = 0;
  de_inafectas = 0;
  de_percepcion = 0;
  detraction_total = 0;
  
  for(i = 0; i < n_items; i++){
    /* Descuento Multisam unitario */
    edoc_item_array[i].disc_price = edoc_item_array[i].goods_value/edoc_item_array[i].qty;
    for(j = 0; j <= edoc_item_array[i].mactmml_times; j++){
      edoc_item_array[i].disc_price = edoc_item_array[i].disc_price * (100 - edoc_item_array[i].disc_msam_perc[j]) / 100;
      printf_log("disc_price[%.3f]", edoc_item_array[i].disc_price);
    }

  edoc_item_array[i].disc_amount = edoc_item_array[i].goods_value/edoc_item_array[i].qty - edoc_item_array[i].disc_price;

  /* Precio unitario con descuentos Multisam */
  edoc_item_array[i].disc_price = (edoc_item_array[i].goods_value/edoc_item_array[i].qty) - edoc_item_array[i].disc_amount;

  if(edoc_item_array[i].disc_price < 0)
    edoc_item_array[i].disc_price = 0;
    
  /*Precio unitario sin IGV*/
  edoc_item_array[i].net_price = 
  (edoc_item_array[i].disc_price + edoc_item_array[i].disc_base_goods_value/edoc_item_array[i].qty) * 100 / (100 + edoc_item_array[i].vat_perc);
  
  /*IGV unitario*/
  edoc_item_array[i].vat_amount = edoc_item_array[i].net_price * edoc_item_array[i].vat_perc / 100;

  de_neto_d += edoc_item_array[i].net_price;
  
  printf_log("disc_amount[%.3f]", edoc_item_array[i].disc_amount);
  printf_log("qty[%.3f]", edoc_item_array[i].qty);
  printf_log("disc_base_goods_value[%.3f]", edoc_item_array[i].disc_base_goods_value);
  de_descuentos_d += floor_price(edoc_item_array[i].disc_amount * edoc_item_array[i].qty + edoc_item_array[i].disc_base_goods_value);

    if(edoc_item_array[i].vat_no == 0){
      de_exoneradas += (edoc_item_array[i].disc_price * edoc_item_array[i].qty + edoc_item_array[i].disc_base_goods_value);
    }else{
      de_gravadas += (edoc_item_array[i].disc_price * edoc_item_array[i].qty + edoc_item_array[i].disc_base_goods_value);
      de_impuesto_d += (edoc_item_array[i].disc_price * edoc_item_array[i].qty + edoc_item_array[i].disc_base_goods_value) * edoc_item_array[i].vat_perc / (100 + edoc_item_array[i].vat_perc);
    }
    //de_total_d += edoc_item_array[i].disc_price * edoc_item_array[i].qty + edoc_item_array[i].disc_base_goods_value;
    
    if(is_article_detraccion(atol(edoc_item_array[i].art_no))){
      detraction_total += (edoc_item_array[i].goods_value - edoc_item_array[i].disc_amount * edoc_item_array[i].qty + edoc_item_array[i].disc_base_goods_value);
    }
  }

  printf_log("de_gravadas[%.3f]", de_gravadas);
  printf_log("de_impuesto_d[%.3f]", de_impuesto_d);
  
  if(detraction_total > 700.0)
    is_detraction = 1;

  de_total_d = floor_price(tot_ret_double(TOT_GEN_INCL)) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL));
  
  sprintf(de_neto, "%.2f", floor_price(de_neto_d));
  sprintf(de_impuesto, "%.2f", floor_price(de_impuesto_d));
  sprintf(de_descuentos, "%.2f", -floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL)));
  sprintf(de_total, "%.2f", floor_price(tot_ret_double(TOT_GEN_INCL)) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL)));
}

void edoc_invoice(void)
{
  int rc = 0;
  
  if(de_pos_config() != DE_OK)
    return;
  
  if ( is_client_pay_FACTURA() )
    document_type = FACTURA;
  else 
    document_type = BOLETA;

  if(de_gen() != DE_OK)
    return;
  
  if(de_conf() != DE_OK)
    return;

  return;
}

