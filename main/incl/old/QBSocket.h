#ifndef __POS_CHQ_H__
#define __POS_CHQ_H__

#ifdef __cplusplus
extern "C" {
#endif


    
    
typedef struct PRESCAN 
{
	char    barcode[15];
	int     art_no	   ;	
	double  qty	        ;
} PRESCAN;


#define article_queue_max_length 500



extern PRESCAN article_queue[article_queue_max_length+1];

extern int start_client(int port , char *servername, 
                        char * id, int updated, int till_no, int cust_no, int store_no,
                        char * buffer_article, int buffer_article_len                 );

//extern char error_qb_msg[500];
extern char *format_qb_msg;

#ifdef __cplusplus
}
#endif

#endif
