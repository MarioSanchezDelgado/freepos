#include <windows.h>
#include <windowsx.h>

#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// include headers based on OS
#if defined WIN32
#include <winsock.h>  // WinSock subsystem
#elif defined __linux__
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif


// redefine some types and constants based on OS
#if defined WIN32
typedef int socklen_t;  // Unix socket length
#elif defined __linux__
typedef int SOCKET;
#define INVALID_SOCKET -1  // WinSock invalid socket
#define SOCKET_ERROR   -1  // basic WinSock error
#define closesocket(s) close(s);  // Unix uses file descriptors, WinSock doesn't...
#endif

#include "QBSocket.h"
#include "pos_errs.h"


PRESCAN article_queue[article_queue_max_length+1];
/*
#define QB_CODE_QUEUE_BUSTING_INVALID       -32046  
#define QB_CONECTION_QUEUE_BUSTING_INVALID  -32047  
#define QB_ARTICLE_QUEUE_BUSTING_INVALID    -32048  

#define QB_FORMAT_QUEUE_BUSTING_INVALID         -32049   
#define QB_ROWS_BIG_QUEUE_BUSTING_INVALID       -32050   
#define QB_ROWS_QUEUE_BUSTING_INVALID           -32051   
#define QB_SOCKET_CREATED_QUEUE_BUSTING_INVALID -32052   
#define QB_HOSTNAME_QUEUE_BUSTING_INVALID       -32053   
*/

//char error_qb_msg[500];
char * format_qb_msg="B:%s/A:%d/Q:%4.4f";

int tokenizef_scan(char *s,  const  char * format,  va_list ap) ;
int tokenizef(char *s,  const  char *format, ...)
{
	va_list ap;
	int retval;

    va_start(ap,format);

	retval = tokenizef_scan(s, format, ap);

	va_end(ap);

	return retval;
}

 int tokenizef_scan(char *s, const char * format,  va_list ap) 
 {
	
	char *cur_s = s;
	
	char data[100];
	char token;
	
	char type;
	const char * pformat=format;
	int  n_token=0;
    int   nnn;

    int error=0;
    char		*tmp_string;	// ditto 

    long val;
    long double	ld_val;
    char *str_val;
    char *p2=NULL;

    int arg_readed=0;

    //printf("cadena:%s\n",s);
    //printf("formato:%s\n",format);
    // formato = %s|%f,%s

	while((error==0)&&(*cur_s!='\0'))
	{
	  if (*pformat=='%') 
	  {
		  pformat++;
		  switch( *pformat)
		  {
			case 	'd':
				type=*pformat;
				pformat++; 
				break;
			 
			 case	'f':
				type=*pformat;
				pformat++; 
				break;
			 
			 case 	's': 
				type=*pformat;
				pformat++; 
				break;

			 default:
                error=QB_FORMAT_QUEUE_BUSTING_INVALID;
				//printf("ERROR EN FORMATO\n");
		  }
	   }else
	   {
			//printf("ERROR EN FORMATO\n");
            error=QB_FORMAT_QUEUE_BUSTING_INVALID;
	   }
       if (error) 
            break;
       
       //toma el primer token
	   token=*pformat;
	   if  (token) 
	   {
	       //skip caracteres hasta encontrar  %
           /*
           while(*pformat&& (*pformat !='%'))
	       {
		    *pformat++;
	       } 
            */
           *pformat++;
		   p2 =strchr(cur_s, token);
           if (p2){
		        nnn=p2-cur_s;
		        strncpy(data, cur_s, nnn);
		        data[nnn]=0;
                cur_s=p2+1;
           }else
           {
                error=QB_FORMAT_QUEUE_BUSTING_INVALID;
                //printf("ERROR EN FORMATO\n");
           }    

	   }else
	   {
		  strcpy(data, cur_s);
          cur_s+=strlen(data);
	   };

       if (error) 
            break;

	   //printf("token (%d)%s\n",index,data);
       switch(type)
       {
           case 'd':
                    val = strtol((const char *)&data, &tmp_string, 10);
                    *va_arg(ap, unsigned *) = (unsigned) val;
                    arg_readed++;
                    break;
           case 's':
                    str_val = va_arg(ap, char *);
                    strcpy(str_val, data);
                    arg_readed++;
                    break;
           case 'f':
                    ld_val = strtod((const char *)&data, &tmp_string);
                    *va_arg(ap, float *) = (float) ld_val;
                    arg_readed++;
                    break;
   
       }
       token=0;
       type=0;
	} 
    if (error) 
            return  -1;

    return arg_readed;
}
 

char * recieved=NULL;
int recieved_size=0;


int start_client(int port , 
                 char *servername, 
                 char* id, 
                 int updated, 
                 int till_no, 
                 int cust_no, 
                 int store_no,
                 char * buffer_article, 
                 int buffer_article_len)
{
    struct sockaddr_in  addr;                   // local address variable
    SOCKET              hsock=0;                // local socket
    int                 r=0;                    // will hold return values
    hostent*            h=NULL;                 // server host entry (holds IPs, etc)
    int                 error=0;



    if (!recieved){
        recieved_size=500;
        recieved=(char *)malloc(sizeof(char)*(recieved_size+1));
    }


    memset((void*)&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = inet_addr(servername);
    if(addr.sin_addr.s_addr==INADDR_NONE ) 
    {
        h = gethostbyname(servername);
        if(NULL == h) {
            error=QB_HOSTNAME_QUEUE_BUSTING_INVALID;
            //perror("Could not get host by name");
            return error;
        }
    } else 
    {
        h = gethostbyaddr((const char*)&addr.sin_addr, sizeof(struct sockaddr_in), AF_INET);
        if(NULL == h) {
            error=QB_HOSTNAME_QUEUE_BUSTING_INVALID;
            //perror("Could not get host by address");
            return error;
        }
    }
    // create the local socket
    hsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == hsock) {
        error=QB_SOCKET_CREATED_QUEUE_BUSTING_INVALID;
        //perror("Could not create socket");
        return error;
    }

    // setup the rest of our local address
    addr.sin_family = AF_INET;
    addr.sin_addr   = *((in_addr*)*h->h_addr_list);
    addr.sin_port   = htons(port);


    // connect to the server
    r = connect(hsock, (sockaddr*)&addr, sizeof(struct sockaddr));
    if(SOCKET_ERROR == r) {
        error=QB_CONECTION_QUEUE_BUSTING_ERROR;
        //perror("Cannot connect to server");
        closesocket(hsock);
        return error;
    }
    
    char data[200] ;

    int message_size;
    int nbytes_recv;
    int rows_cmd=0;
    int n_row=0;



    sprintf(data, "ITEMS %s %d %d %d %d\n", id, cust_no, updated, till_no, store_no);
    send(hsock, data, strlen(data), 0);


    nbytes_recv=recv(hsock, data,     4+1+9+1, 0);
    if (nbytes_recv>=0)
    {
            data[nbytes_recv-1]=0;                
            if (strncmp(data,"SIZE",4)==0)
            {
                message_size=atoi(data+6);
                if (recieved_size<message_size)
                {
                     recieved_size=message_size+500;
                     recieved=(char *)realloc(recieved,sizeof(char)*(recieved_size+1));
                }
                nbytes_recv=recv(hsock, recieved,     message_size, 0);
                recieved[nbytes_recv]=0;                
    
                char * pbuffer=recieved;
                char * pbuffer_find;

                char barcode[100];
                int art_no ;
                float qty;
                int nvar=0;

                while (*pbuffer!=NULL)
                {
                    pbuffer_find=strchr(pbuffer,'\n');
                    if (pbuffer_find) *pbuffer_find=0;

                    if (strncmp(pbuffer,"EOF ",4)==0)
                    {
                        rows_cmd=atoi(pbuffer+6);
                        break;
                    }else
                    {
                        barcode[0]=0;
                        art_no=0;
                        qty=0;
                        nvar=tokenizef(pbuffer,"%s|%d|%f", barcode,&art_no,&qty);
                        if (nvar!=3){ //error
                            error=QB_FORMAT_QUEUE_BUSTING_INVALID;
                            sprintf(buffer_article,format_qb_msg ,barcode, art_no, qty);
                            break;
                        }

                        strcpy(article_queue[n_row].barcode,barcode);
                        article_queue[n_row].art_no = art_no;
                        article_queue[n_row].qty    = qty;
                        //printf("vars=%d: %-25.25s  - %-14.14s | %6d | %f \n",nvar, pbuffer,barcode,art_no,qty);

                        if  (art_no!=0 )
                        {
                            n_row++;
                        }

                        if (pbuffer_find) 
                            pbuffer=pbuffer_find+1;
                        else
                          break;
                        if (n_row>=article_queue_max_length)
                        {
                            error=QB_ROWS_BIG_QUEUE_BUSTING_INVALID;
                            break;
                        }
            
                    }
                }
                strcpy(article_queue[n_row].barcode,"");
                article_queue[n_row].art_no = 0;
                article_queue[n_row].qty    = 0;
            }
    }
    sprintf(data, "EXIT\n");
    send(hsock, data, strlen(data), 0);

    closesocket(hsock);
    
    if (rows_cmd==-1){// error sql
        error=QB_QUEUE_BUSTING_SQL_INVALID;
        return error;
    }

    if (n_row!=rows_cmd){
         error=QB_ROWS_QUEUE_BUSTING_INVALID;
         return error;
    }
    if(n_row==0)
    {
         error=QB_QUEUE_BUSTING_NOTFOUND;
         return error;
    }
    return 0;
}