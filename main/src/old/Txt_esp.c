/*
 *     Module Name       : TXT_ESP.C
 *
 *     Type              : Application TEXT Structures
 *                         with SPANISH text
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
 * 23-Nov-2000 Initial Release WinPOS                                    J.H.
 * --------------------------------------------------------------------------
 * 12-Ago-2011 Change for sales Turkey						            ACM -
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#ifdef TXT_ESP

#pragma message(__FILE__ ": Using Spanish text.")

/*---------------------------------------------------------------------------*/
/* ERROR MESSAGES                                                            */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                        1         2         3         4         5         6*/
/*              0123456789012345678901234567890123456789012345678901234567890*/
/*---------------------------------------------------------------------------*/
_TCHAR *err_msg_TXT[]={
/* 0000 */  _T("")                     /* Is used, do not fill it with text! */
/* 0001 */ ,_T("Presione <BORRAR> para continuar")
/* 0002 */ ,_T("Entrada invalida")
/* 0003 */ ,_T("Llamar al Supervisor")
/* 0004 */ ,_T("Error del Sistema")
/* 0005 */ ,_T("Falla de poder!")
/* 0006 */ ,_T("Falla en Impresora: sin papel")
/* 0007 */ ,_T("Por favor inserte el cheque/slip!")
/* 0008 */ ,_T("Por favor retire el cheque/slip!")
/* 0009 */ ,_T("Printer motor jam!")
/* 0010 */ ,_T("Falla general de impresion!")
/* 0011 */ ,_T("Falla en lectura de tarjeta, intente nuevamente!")
/* 0012 */ ,_T("Error de reloj!")
/* 0013 */ ,_T("Falla de teclado")
/* 0014 */ ,_T("Falla en lectura de articulo, intente nuevamente!")
/* 0015 */ ,_T("Condicion fuera de memoria") /* Out of memory condition */
/* 0016 */ ,_T("Dispositivo MSR no disponible!")
/* 0017 */ ,_T("Puerto 1 OCIAI no disponible!")
/* 0018 */ ,_T("Falla en llave de bloqueo!")  /* keylock */
/* 0019 */ ,_T("Falla General en cmos")
/* 0020 */ ,_T("Falla en inicializacion de la caja") /* Retail device */
/* 0021 */ ,_T("Falla de caida de Programa")
/* 0022 */ ,_T("Falla en formateo de disco Ram")
/* 0023 */ ,_T("Fuera de memoria")
/* 0024 */ ,_T("Tecla de chequeo")
/* 0025 */ ,_T("Gire la llave a la posicion L")
/* 0026 */ ,_T("Llamar al Supervisor o presione NO")
/* 0027 */ ,_T("Opcion de menu invalida")
/* 0028 */ ,_T("Pasaporte anterior invalido, llamar al Supervisor")
/* 0029 */ ,_T("Monto introducido excede el limite para este tipo de pago!")
/* 0030 */ ,_T("Primero introduzca el monto, luego presione tipo de pago!")
/* 0031 */ ,_T("Cheque no permitido, llamar al Supervisor")
/* 0032 */ ,_T("Forma de Pago ilegal!")
/* 0033 */ ,_T("El function no es posible, subtotal es cero!")
/* 0034 */ ,_T("No existe ultimo articulo para procesar!")
/* 0035 */ ,_T("Numero de articulo Invalido!")
/* 0036 */ ,_T("La Cantidad introducida excede (9999)!")
/* 0037 */ ,_T("El campo de entrada de datos debe estar vacio para ejecutar esta funcion!")
/* 0038 */ ,_T("Numero de articulo de descuento Invalido!")
                                 /* Translation note: length %s is maximal 8 */
/* 0039 */ ,_T("Error (%s) durante la inicializacion de la caja!")
/* 0040 */ ,_T("Tecla de funcion Invalida!")
/* 0041 */ ,_T("Muchas teclas presionadas!")
/* 0042 */ ,_T("Cero no es legal!")
/* 0043 */ ,_T("Cliente no conocido en Recepcion de Cliente")
/* 0044 */ ,_T("Password invalido")
/* 0045 */ ,_T("Password invalido, ultima oportunidad!")
/* 0046 */ ,_T("Password invalido, llamar al Supervisor!")
/* 0047 */ ,_T("No hay articulos para eliminar!")
/* 0048 */ ,_T("Monto introducido excede el valor del articulo!")
/* 0049 */ ,_T("Hora ilegal!")
/* 0050 */ ,_T("Fecha ilegal!")
/* 0051 */ ,_T("Fecha del Sistema incorrecta, chequee la fecha en la pantalla!")
/* 0052 */ ,_T("Demasiados renglones en la factura")
/* 0053 */ ,_T("Articulo bloqueado")
                                 /* Translation note: length %s is maximal 2 */
/* 0054 */ ,_T("Cliente bloqueado (%s), llamar al Supervisor")
/* 0055 */ ,_T("Combinacion Cantidad X Precio excede el maximo!")
/* 0056 */ ,_T("Codigo de barra Ilegal!")
/* 0057 */ ,_T("Monto a retirar excede el monto actual de caja!")
/* 0058 */ ,_T("Cajero ya registrado en otra caja")
/* 0059 */ ,_T("Cajero no autorizado para salirse!")  /* OJO */
/* 0060 */ ,_T("Numero de cajero desconocido")
/* 0061 */ ,_T("Correr 'Fin de Dia' antes de cambiar de modo!")
/* 0062 */ ,_T("Presione TOTAL")
/* 0063 */ ,_T("Monto del cheque excede el limite")
/* 0064 */ ,_T("No use el scanner en este campo!")
/* 0065 */ ,_T("Pasaporte expirado, llamar al Supervisor")
/* 0066 */ ,_T("Numero de pasaporte Ilegal!")
/* 0067 */ ,_T("Ingreso/Egreso de Caja no procesado.")
/* 0068 */ ,_T("Turno actual de cajero no ha cerrado.")
/* 0069 */ ,_T("Valor introducido excede el maximo!")
/* 0070 */ ,_T("Descuento no permitido en este articulo de deposito!")
/* 0071 */ ,_T("Descuento no permitido en este articulo de precio reducido!")
                                /* Translation note: length %s is maximal 16 */
/* 0072 */ ,_T("No introduzca monto para tarjeta de credito!")
/* 0073 */ ,_T("Monto extra cargado para este tipo de pago!")
/* 0074 */ ,_T("Presione ENTER para aceptar o NO para cancelar")
/* 0075 */ ,_T("Muchos cajeros, corra un Fin de Dia y trate de nuevo!")
/* 0076 */ ,_T("No introduzca 2 cajeros en 1 minuto, por favor espere!")
/* 0077 */ ,_T("El articulo a procesar ha sido aprobado por el supervisor!")
                                /* Translation note: length %s is maximal 16 */
/* 0078 */ ,_T("%s no permitido en combinacion con otros tipos!")
/* 0079 */ ,_T("Mantenga la copia de la factura en su caja!")
/* 0080 */ ,_T("Entregue esta factura al cliente.")
/* 0081 */ ,_T("Archivo de Articulo corrupto, llamar a EDP!")
/* 0082 */ ,_T("Forma de pago diferente al pago seleccionado")
/* 0083 */ ,_T("No es posible vender mas moneda extranjera que la recibida")
/* 0084 */ ,_T("NO necesita pagar derechos de membresia esta vez, llamar al supervisor")
/* 0085 */ ,_T("NO necesita pagar derechos de membresia este a$o, llamar al supervisor")
/* 0086 */ ,_T("Muchas teclas")
/* 0087 */ ,_T("Valor invalido para variable cust_on_pos de GENVAR")
/* 0088 */ ,_T("Error de Sintaxis en Descuento Multisam")
/* 0089 */ ,_T("No se puede aplicar descuento Multisam - subset esta vacio")

/*******/
/* 0090 */ ,_T("Conformar Cheque")
/* 0091 */ ,_T("Advertencia: No se aceptan cheques a este cliente")
/* 0092 */ ,_T("OCIAII port 2 not available!")
/* 0093 */ ,_T("Por Favor cerrar printer cover")
/* 0094 */ ,_T("Cliente no Registrado!")
/* 0095 */ ,_T("Confirme Guardar Factura")
/* 0096 */ ,_T("Factura pendiente ya existe, Sobrescribirla?")
/* 0097 */ ,_T("Factura Pendiente de %s existe, Recuperarla?")
/* 0098 */ ,_T("Parse error: syntax error in query conditions")
/* 0099 */ ,_T("Consulta retorna exceso de registros")
/* 0100 */ ,_T("No conseguio registros")
/* 0101 */ ,_T("Internal OPOS keyboard error (%s)")
/* 0102 */ ,_T("POS is OFFLINE : Llamar al Supervisor para colocar monto manualmente?")
/* 0103 */ ,_T("El voucher no existe en backoffice")
/* 0104 */ ,_T("El voucher esta bloqueado")
/* 0105 */ ,_T("Numero de voucher invalido")
/* 0106 */ ,_T("No se pudo desbloquear la Nota de Credito!")
/* 0107 */ ,_T("Error en configuracion de red: Caja no definida")
/* 0108 */ ,_T("Error en configuracion de red: Nueva Caja no permitida")
/* 0109 */ ,_T("Monto de Efectivo excede maximo permitido")
/* 0110 */ ,_T("Monto maximo de efectivo alcanzado")
/* 0111 */ ,_T("ERROR: backlog corrupt, CLOSE PROGRAM via EDP MENU !!!")
/* 0112 */ ,_T("Check invoice! Two identical lines printed")
/* 0113 */ ,_T("Devolucion cobro pasaporte ?")
/* 0114 */ ,_T("Time out during reading scanner. Data is probably lost!")
/* 0115 */ ,_T("Scanner/keyboard conflict, try again!")
/* 0116 */ ,_T("Time out during multisam parsing!")
/* 0117 */ ,_T("Too many multisam errors!")
/* 0118 */ ,_T("Donation amount can not be higher than %s")
/* 0119 */ ,_T("Donation amount can not be higher than change value")
/* 0120 */ ,_T("Customer not found using search by fisc_no!")
/* 0121 */ ,_T("Contacte a su Administrador, el numero consecutivo ha finalizado")
/* 0122 */ ,_T("Cliente sin RUC solo podra generar boleta")
/* 0123 */ ,_T("Vale de Pavo invalido, verifique por favor")
/* 0124 */ ,_T("Codigo de Pavo invalido, verifique por favor")
/* 0125 */ ,_T("El peso del pavo debe ser mayor o igual a 7 Kilos")   /* 12-Ago-2011 acm - add message error */
/* 0126 */ ,_T("Solo se permite un vale de Pavo por Boleta/Factura")  /* 12-Ago-2011 acm - add message error */
/* 0127 */ ,_T("El articulo Pavo ingresado no corresponde con el Vale")  /* 12-Ago-2011 acm - add message error */
/* 0128 */ ,_T("El articulo ingresado debe ser un codigo de Pavo")   /* 12-Ago-2011 acm - add message error */

/* 25-Set-2012 acm - {*/

/* 0129 */ ,_T("QB:Codigo QueueBusting es invalido")  
/* 0130 */ ,_T("QB:Error en la conexion del Server QueueBusting")
/* 0131 */ ,_T("QB/FMT:Articulo %s es Invalido")
/* 0132 */ ,_T("QB/POS:Articulo %s es Invalido")
/* 0133 */ ,_T("QB:Filas recibidas por el Server QueueBusting no debe ser mayor que 500")
/* 0134 */ ,_T("QB:Filas recibidas por el Server QueueBusting difiere al esperado")
/* 0135 */ ,_T("QB:No se puede crear el socket QueueBusting en el POS")
/* 0136 */ ,_T("QB:No se puede leer hostname por nombre")
/* 0137 */ ,_T("QB:No se encontro codigo QueueBusting")
/* 0138 */ ,_T("QB:Error al procesar commando SQL en servidor QueueBusting")
/* 0139 */ ,_T("El Cliente debe ser cliente horeca para ejecutar esta funcion!") // v3.4.8 acm -
/* 0140 */ ,_T("El Articulo no es un Horeca. Verifique!")                        // v3.4.8 acm -
/* 0141 */ ,_T("Fecha Actual no es Vigente para HORECA")                         // v3.4.8 acm -
/* 0142 */ ,_T("El ValePavo no es Valido o ya fue usado")                        // v3.5 acm -
/* 0143 */ ,_T("Error en la conexion verifique estado del POS")
/* 0144 */ ,_T("El Valepavo ya fue usado")

/* 0145 */ ,_T("ValePavo: Valepavo no esta registrado")
/* 0146 */ ,_T("ValePavo: Error desconocido, avise a sistemas")
/* 0147 */ ,_T("ValePavo: Error en Base de datos")
/* 0148 */ ,_T("DNI no es valido, Verifique")
/* 0149 */ ,_T("Ingrese Nombre de Cliente")

/* 0150 */ ,_T("Error en la conexion verifique estado del POS")
/* 0151 */ ,_T("El Anticipo ya fue usado")
/* 0152 */ ,_T("Anticipo: Anticipo no esta registrado")
/* 0153 */ ,_T("Anticipo: Error desconocido, avise a sistemas")
/* 0154 */ ,_T("Anticipo: Error en Base de datos")
/* 0155 */ ,_T("Anticipo: Percepcion Anticipo debe ser menor a Percepcion Ticket")
/* 0156 */ ,_T("Anticipo: IGV Anticipo debe ser menor a IGV Ticket")
/* 0157 */ ,_T("Anticipo: Total Anticipo debe ser menor a Total Ticket")
/* 0158 */ ,_T("Anticipo: Total Anticipo datafile distinto a anticipo BD")
/* 0159 */ ,_T("Anticipo: Anticipo Factura es para Ticket Factura")
/* 0160 */ ,_T("Anticipo: Anticipo Boleta es para Ticket Boleta")
/* 0161 */ ,_T("Anticipo: RUC/DNI del Anticipo debe ser igual al ticket")
/* 25-Set-2012 acm - }*/
/* 0162 */ ,_T("EPOS error - Call supervisor") 
};

/*---------------------------------------------------------------------------*/
/* CH01 MESSAGES (INVOICE SCREEN)                                            */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                     1         2         3         4         5         6   */
/*           0123456789012345678901234567890123456789012345678901234567890   */
/*---------------------------------------------------------------------------*/

_TCHAR *scrn_inv_TXT[]={
/* 0000 */  _T("")
#ifndef NO_VIEW_POS_STATE
/* 0001 */ ,_T("D")                                   /* Indicador de Devoluciones  */
/* 0002 */ ,_T("E")                                   /* Indicador de Entrenamiento */
#else
/* 0001 */ ,_T("")
/* 0002 */ ,_T("")
#endif
/* 0003 */ ,_T(" ONLINE")
/* 0004 */ ,_T("OFFLINE")
/* 0005 */ ,_T("CFG_ERR")
/* 0006 */ ,_T("          ART.        DESCRIPCION             IGV CANT         PRECIO     TOTAL")
/* 0007 */ ,_T("  TOTAL")
/* 0008 */ ,_T("VUELTO")
/* 0009 */ ,_T("IGV")
/* 0010 */ ,_T("GDS")
/* 0011 */ ,_T("PAGAR ")
/* 0012 */ ,_T("PAGO")
/* 0013 */ ,_T("D.M")
/* 0014 */ ,_T("")
/* 0015 */ ,_T("MODO VENTA")
/* 0016 */ ,_T("MODO DEVOLUCION")
/* 0017 */ ,_T("COMIENZO DE DIA")
/* 0018 */ ,_T("CAJERO NUEVO")
/* 0019 */ ,_T("CAJERO  ID:")
/* 0020 */ ,_T("PASSWRD   :")
/* 0021 */ ,_T("MONTO     :")
/* 0022 */ ,_T("**** CAJERO EN RECESO ****")
/* 0023 */ ,_T("NO:")
/* 0024 */ ,_T("DEPOSITO")          /* Used as a description for an unknown      */
                                    /*     deposit article.                      */
/* 0025 */ ,_T("DESCUENTO")         /* Used as a descr. for the discount line    */
/* 0026 */ ,_T("MBS WINPOS,  Version %s, %12.12s %5.5s - %15.15s")
/* 0027 */ ,_T("Descripcion                       Pr.Venta   Unidades  Mail Article no")
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
/* 0038 */ ,_T("VALE DE PAVO: ")      
};


/*---------------------------------------------------------------------------*/
/* INPUT LINE MESSAGES                                                       */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                     1         2         3         4         5         6   */
/*           0123456789012345678901234567890123456789012345678901234567890   */
/*---------------------------------------------------------------------------*/

_TCHAR *input_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("INGRESE NRO. DE CLIENTE")
/* 0002 */ ,_T("INGRESE COD. DE CAJERO")
/* 0003 */ ,_T("INGRESE ART. / CANT.")
/* 0004 */ ,_T("INGRESE ART. / CANT. O PRESIONE <NO>")
/* 0005 */ ,_T("INGRESE PRECIO 1X")
/* 0006 */ ,_T("PESO REQUERIDO")
/* 0007 */ ,_T("ANULAR ULTIMO ARTICULO SI/NO?")
/* 0008 */ ,_T("CODIGO DEL SUPERVISOR")
/* 0009 */ ,_T("MONTO RETIRADO:")
/* 0010 */ ,_T("NUMERO VALIJA:")
/* 0011 */ ,_T("MONTO REPOSICION:")
/* 0012 */ ,_T("MONTO REPOSICION OK SI/NO?")
/* 0013 */ ,_T("MODO ACTUAL:")
/* 0014 */ ,_T("SI/NO?")
/* 0015 */ ,_T("NUEVO LIMITE")
/* 0016 */ ,_T("FECHA ACTUAL :")
/* 0017 */ ,_T("(FORMATO AAAAMMDD)")
/* 0018 */ ,_T("NUEVA FECHA  :")
/* 0019 */ ,_T("HORA ACTUAL  :")
/* 0020 */ ,_T("NUEVA HORA   :")
/* 0021 */ ,_T("MONTO ACTUAL POR DEFECTO EN VENTAS:")
/* 0022 */ ,_T("MONTO ACTUAL POR DEFECTO EN DEVOLUCION:")
/* 0023 */ ,_T("NUEVO MONTO:")
/* 0024 */ ,_T("MODO ACTUAL POR DEFECTO:")
/* 0025 */ ,_T("CAMBIAR A")
/* 0026 */ ,_T("CERRAR FACTURA SI/NO?")
/* 0027 */ ,_T("Llamar al Supervisor o presionar NO")
#ifdef SNI
/* 0028 */ ,_T("Girar llave a la posicion 1")
#else
/* 0028 */ ,_T("Oprima la tecla F7")
#endif
/* 0029 */ ,_T("CERRAR GAVETA")
/* 0030 */ ,_T("INGRESE MONTO PAGADO Y PRESIONE TIPO DE PAGO")
/* 0031 */ ,_T("DESCUENTO ART. / CANT.")
/* 0032 */ ,_T("INGRESE DESCUENTO ART. / CANT. O PRESIONE <NO>")
/* 0033 */ ,_T("INGRESE PRECIO DE CREDITO 1X")
/* 0034 */ ,_T("NRO. ARTICULO DE DESCUENTO")
/* 0035 */ ,_T("MONTO DE DESCUENTO 1X")
/* 0036 */ ,_T("USE LINEA ARRIBA/ABAJO O INGRESE COD. ARTICULO PARA MOVER LINEA")
/* 0037 */ ,_T("ANULAR ARTICULO SI/NO?")
/* 0038 */ ,_T("INSERTE EL DINERO Y CIERRE LA GAVETA")
/* 0039 */ ,_T("INGRESE EL PASSWRD DE EDP PARA IR AL MENU:")
/* 0040 */ ,_T("Presione cualquier tecla para continuar")
/* 0041 */ ,_T("")
/* 0042 */ ,_T("CREDITO ART. / CANT.")
/* 0043 */ ,_T("OK (SI/NO)")
/* 0044 */ ,_T("EL DERECHO DE MEMBRESIA EXPIRA, DESEA PAGAR AHORA? (SI/NO)")
/* 0045 */ ,_T("LA MEMBRESIA HA EXPIRADO, DESEA PAGAR AHORA ? (SI/NO)")
/* 0046 */ ,_T("INGRESE NUMERO DE VOUCHER O <NO>")
/* 0047 */ ,_T("INGRESE CANTIDAD DE LA NOTA DE CREDITO")
/* 0048 */ ,_T("DESEA DONAR   SI/NO?")
/* 0049 */ ,_T("INGRESE CANTIDAD DONAR")
/* 0050 */ /*,_T("SELECT PRINTER FOR INVOICE")*/
/* 0050 */ ,_T("SELECCIONE TIPO DE DOCUMENTO")
/* 0051 */ ,_T("SELECT CUSTOMER FROM LIST")
/* 0052 */ ,_T("CLIENTE SOLO CON DNI NO SE GENERARA FACTURA")  /*JCP*/
/* 0053 */ ,_T("SCANEE EL PAVO")  /*JCP*/  
/* 0054 */ ,_T("SCANEE EL VOUCHER DEL PAVO")   /*JCP*/  
/* 0055 */ ,_T("INGRESE ART-PAVO / CANT.")    /*12-Ago-2011 acm -*/
/* 0056 */ ,_T("SCANEE EL !VALE! DEL PAVO")   /*12-Ago-2011 acm -*/
/* 0057 */ ,_T("SCANEE EL QUEUE TICKET") /*25-Set-2012 acm -*/
/* 0058 */ ,_T("INGRESE ART HORECA. / CANT. ")  /*v3.4.7 acm -*/  

};

/*---------------------------------------------------------------------------*/
/* APPROVAL MESSAGES                                                         */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                     1         2         3         4         5         6   */
/*           0123456789012345678901234567890123456789012345678901234567890   */
/*---------------------------------------------------------------------------*/

_TCHAR *appr_msg_TXT[]={
/* 0000 */  _T("")                      /* Is used, do not fill it with text!    */
/* 0001 */ ,_T("ANULAR FACTURA?")
/* 0002 */ ,_T("CAMBIAR DEVOLUCION/VENTA PARA PROXIMA FACTURA?")
/* 0003 */ ,_T("ABRIR GAVETA?")
/* 0004 */ ,_T("MENU OPERADOR?")
/* 0005 */ ,_T("DESPRENDA LA FACTURA!")
/* 0006 */ ,_T("DAR CREDITO AL PROXIMO ARTICULO?")
/* 0007 */ ,_T("DESCUENTO EN PROXIMO ARTICULO?")
/* 0008 */ ,_T("AVANZAR PAGINA?")
};


/*---------------------------------------------------------------------------*/
/* GENERAL MESSAGES                                                          */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                     1         2         3         4         5         6   */
/*           0123456789012345678901234567890123456789012345678901234567890   */
/*---------------------------------------------------------------------------*/

_TCHAR *prompt_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("Procesando...")
/* 0002 */ ,_T("")                      /* not used */
/* 0003 */ ,_T("Manager pin code or <NO>:")
/* 0004 */ ,_T("Imprimiendo...")
/* 0005 */ ,_T("(Chequear Tecla-SEL si no hace nada.")
/* 0006 */ ,_T("Leyendo...")
/* 0007 */ ,_T("Ordenando...")
/* 0008 */ ,_T("Contando...")
/* 0009 */ ,_T("EDP pin code or <NO>    :")
/* 0010 */ ,_T("Borrando data de facturas viejas, espere un momento por favor!")
/* 0011 */ ,_T("Retrieving crazy article history, just a moment please!")
};



/*---------------------------------------------------------------------------*/
/* MENU MESSAGES                                                             */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                     1         2         3         4         5         6   */
/*           0123456789012345678901234567890123456789012345678901234567890   */
/*---------------------------------------------------------------------------*/

_TCHAR *menu_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("X-READ")
/* 0002 */ ,_T("FIN DE CAJERO")
/* 0003 */ ,_T("FIN DE DIA")
/* 0004 */ ,_T("RETIRAR")
/* 0005 */ ,_T("REPONER")
/* 0006 */ ,_T("LOG ON")
/* 0007 */ ,_T("CAMBIAR MODO")
/* 0008 */ ,_T("VARIABLES GENERALES")
/* 0009 */ ,_T("ENTRENAMIENTO")
/* 0010 */ ,_T("ESCOJA OPCION:")
/* 0011 */ ,_T("MENU OPERADOR")
/* 0012 */ ,_T("MENU SUPERVISOR")
/* 0013 */ ,_T("CAJA DEVOLUCIONES")
/* 0014 */ ,_T("CAJA VENTAS")
/* 0015 */ ,_T("CAJA NORMAL")
/* 0016 */ ,_T("CAJA ENTRENAMIENTO")
/* 0017 */ ,_T("MENU TIPOS DE PAGO")                 /* Menu paymenttypes         */
/* 0018 */ ,_T("MENU VARIABLES GENERALES")           /* Menu general variables    */
/* 0019 */ ,_T("FECHA SISTEMA")
/* 0020 */ ,_T("HORA SISTEMA")
/* 0021 */ ,_T("MONTO DE VENTAS POR DEFECTO")
/* 0022 */ ,_T("MONTO DE DEVOLUCIONES POR DEFECTO")
/* 0023 */ ,_T("MODO POR DEFECTO")
/* 0024 */ ,_T("TIPOS DE PAGO")
/* 0025 */ ,_T("CD  DESCRIPCION       LIMITE               PORC  MONTO MIN. CDIGV")
/* 0026 */ ,_T("SALIR DEL PROGRAMA")
/* 0027 */ ,_T("MENU EDP")
/* 0028 */ ,_T("REIMPRIMIR FACTURA")
/* 0029 */ ,_T("LA MEMBRESIA EXPIRA EN 30 DIAS D.D. %s")
/* 0030 */ ,_T("LA MEMBRESIA HA EXPIRADO DESDE %s")
/* 0031 */ ,_T("MONTO MEMBRESIA ES %s PARA %d PASAPORTES")
/* 0032 */ ,_T("PAGUE MEMBRESIA AHORA")
/* 0033 */ ,_T("PAGO MEMBRESIA NO EFECTUADO (CONTINUAR FACTURANDO)")
/* 0034 */ ,_T("PAGO MEMBRESIA NO EFECTUADO (NO ES POSIBLE FACTURAR)")
/* 0035 */ ,_T("DEVOLUCION SIN PAGO DE MEMBRESIA")
/* 0036 */ ,_T("DEVOLUCION DE PAGO DE MEMBRESIA (S-KEY)")
/* 0037 */ ,_T("MENU PAGO DERECHO MEMBRESIA")
/* 0038 */ ,_T("PARA NINGUN CLIENTE EN TIENDA %ld")
/* 0039 */ ,_T("DESCUENTOS OTORGADOS")
/* 0040 */ ,_T("CALCULATOR")
/* 0041 */ ,_T("           P A S S   D A Y")
/* 0042 */ ,_T("MONTO MEMBRESIA ES %s PARA PASS DAY")
/* 0043 */ ,_T("(PASS DAY)")
};


/*---------------------------------------------------------------------------*/
/* CUSTOMER DISPLAY                                                          */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                     1         2         3         4         5         6   */
/*           0123456789012345678901234567890123456789012345678901234567890   */
/*---------------------------------------------------------------------------*/

_TCHAR *cdsp_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("CERRADA")
/* 0002 */ ,_T("ABIERTA")
/* 0003  ,_T("PROXIMO CLIENTE POR FAVOR" */
/* 0003 */ ,_T("PROXIMO CLIENTE") 
/* 0004 */ ,_T("SUBTOTAL:")
/* 0005 */ ,_T("TOTAL:")
/* 0006 */ ,_T("TOTAL:")
/* 0007 */ ,_T("PAGO :")
/* 0008 */ ,_T("VUELTO:")
/* 0009 */ ,_T("GRACIAS  ")
/* 0010 */ ,_T("x")
/* 0011 */ ,_T("EXPIRA   %s")              /* expiring within 30 days */
/* 0012 */ ,_T("EXPIRADO %s")              /* Expired */
/* 0013 */ ,_T("DERECHO VENCIDO  %s")      /* OJO! "FEE DUE" */
/* 0014 */ ,_T("PASS DAY")
};


/*----------------------------------------------------------------------------*/
/* Layout of X-read and Z-read.                                               */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                     1         2         3         4         5         6         7         8         9       */
/*           0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456 */
/*----------------------------------------------------------------------------*/

extern _TCHAR *prn_xr_TXT_tirilla[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("%-96.96s")
/* 0002 */ ,_T("LECTURA - X    C A J A : %3hd     FECHA   : %-12.12s    T U R N O : %3hd    H O R A : %-5.5s")
/* 0003 */ ,_T("                                                              T U R N O : %3hd")
/* 0004 */ ,_T("%3hd %-75.75s%17.17s")
/* 0005 */ ,_T("%4.4s%-75.75s%17.17s")
/* 0006 */ ,_T("CJRO")
/* 0007 */ ,_T("CAJA")
/* 0008 */ ,_T("T O T A L")
/* 0009 */ ,_T("TOT:")
/* 0010 */ ,_T("     HORA  HORA       FACTURA     FACTURA NULAS NULAS VALIJA NO.")
/* 0011 */ ,_T("%4.4s   ON   OFF     ON    OFF    NO LINEAS  NO LINEAS  1   2   3     MONTO INIC.    LIFT/REFILL     DONACIONES")
/* 0012 */ ,_T("")
/* 0013 */ ,_T("TOTAL")
/* 0014 */ ,_T("     -------------- -------------- -------------- -------------- --------------")
/* 0015 */ ,_T("%17.17s")
/* 0016 */ ,_T("    %-75.75s")
/* 0017 */ ,_T("LECTURA - Z    C A J A : %3hd     FECHA   : %-12.12s F I N  D E  D I A     H O R A : %-5.5s")
/* 0018 */ ,_T("%3hd %-75.75s%17.17s")
/* 0019 */ ,_T("%3hd %-75.75s")
/*******/
/* 0020 */ ,_T("STATUS: %8.8s")
/* 0021 */ ,_T("NUMERACION ACTUAL DEL IMPRESORA PEQUENA")
};



/***************************************************************************
 Jonathan PERU para implementacion de la X y Z en tirilla                  */
/*              1234567890123456789012345678901234567890      */

extern _TCHAR *prn_xr_TXT[]={
/* 0000 */  _T("1 ")
/* 0001 */ ,_T("2 %-96.96s")
/* 0002 */ ,_T("LECTURA - X    \nCAJA  : %3hd     FECHA  : %-12.12s  \nTURNO : %3hd     HORA   : %-5.5s")
/* 0003 */ ,_T("4                                                               T U R N O : %3hd")
/* 0004 */ ,_T("5 %3hd %-75.75s%17.17s")
/* 0005 */ ,_T("6 %4.4s%-75.75s%17.17s")
/* 0006 */ ,_T("7 CJRO")
/* 0007 */ ,_T("8 CAJA")
/* 0008 */ ,_T("9 T O T A L")
/* 0009 */ ,_T("10 TOT:")
/* 0010 */ ,_T("11      HORA  HORA       FACTURA     FACTURA NULAS NULAS VALIJA NO.")
/* 0011 */ ,_T("12 %4.4s   ON   OFF     ON    OFF    NO LINEAS  NO LINEAS  1   2   3     MONTO INIC.    LIFT/REFILL     DONACIONES")
/* 0012 */ ,_T("13 ")
/* 0013 */ ,_T("14 TOTAL")
/* 0014 */ ,_T("15      -------------- -------------- -------------- -------------- --------------")
/* 0015 */ ,_T("16 %17.17s")
/* 0016 */ ,_T("17     %-75.75s")
/* 0017 */ ,_T("18 LECTURA - Z    C A J A : %3hd     FECHA   : %-12.12s F I N  D E  D I A     H O R A : %-5.5s")
/* 0018 */ ,_T("19 %3hd %-75.75s%17.17s")
/* 0019 */ ,_T("20 %3hd %-75.75s")
/*******/
/* 0020 */ ,_T("STATUS: %8.8s")
/* 0021 */ ,_T("NUMERACION ACTUAL : ")
/* 0022 */ ,_T("CJRO %3hd")
/* 0023 */ ,_T("%-15.15s -->  %-17.17s")
/* 0024 */ ,_T("TOTAL           -->%17.17s")
/* 0025 */ ,_T("          T O T A L E S")
/* 0026 */ ,_T("HORA ON : %-5.5s ")
/* 0027 */ ,_T("HORA OFF : %-5.5s")
/* 0028 */ ,_T("FACTURA ON  : %d   OFF : %d")
/* 0029 */ ,_T("FACTURA NO  : %d   LINEAS : %d")
/* 0030 */ ,_T("NULAS   NO  : %d   LINEAS : %ld")
/* 0031 */ ,_T("VALIJA NO. 1 2 3")
/* 0032 */ ,_T("MONTO INIC. : %s")
/* 0033 */ ,_T("LIFT/REFILL : %s")
/* 0034 */ ,_T("DONACIONES  : %s")
/* 0035 */ ,_T("LECTURA - Z  \nF I N  D E  D I A  \nCAJA:%2hd  FECHA: %-12.12s HORA: %-5.5s")
/* 0036 */ ,_T("   T O T A L E S   D E   C A J A  ")
/* 0037 */ ,_T("CUPON ANIV  : %d") /*AC2012-003 acm - */
/* 0038 */ ,_T("REDONDEO    : %13.2f") /*V3.4.7 acm - */
/* 0039 */ ,_T("CUPON CINE  : %d") /*v3.5 acm - */
/* 0040 */ ,_T("VALE PAVO   : %d") /*v3.5.1 acm - */
/* 0041 */ ,_T("CUPON FERIA : %d") /*v3.5.1 acm - */
/* 0042 */ ,_T("PERCEPCION  : %6.2f") /*v3.5.1 acm - */
/* 0043 */ ,_T("CUPON FUTBOL: %d") /*v3.5.1 acm - */ // FIESTA_FUTBOL
/* 0044 */ ,_T("FACTURA FINAL :  ")
/* 0045 */ ,_T("BOLETA FINAL :   ")
};


/***************************************************************************/

/*----------------------------------------------------------------------------*/
/* Layout of the invoice.                                                     */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                     1         2         3         4         5         6         7         8         9       */
/*           0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456 */
/*----------------------------------------------------------------------------*/

extern _TCHAR *prn_inv_TXT[]={
/* 0000 */  _T("")
/* 0001 */ ,_T("%-96.96s")
/* 0002 */ ,_T("                   %-30.30s                                                 %02d%02d %04d%6s       %2d")
/* 0003 */ ,_T("                   %-30.30s                                                             %3d      %3d")
/* 0004 */ ,_T("                   %02d %06ld%2s                                                                         %-10.10s    %5s")  /*Numero de Pasaporte fecha*/
/* 0005  ,_T(" %6.6s CODE  VAT %%   GOODS VALUE    VAT AMOUNT         TOTAL  %-12.12s        %13.13s"  */
/* 0005 */ ,_T(" TOTAL  COD.    IGV%%   VALOR MERC.     MONTO IGV        TOTAL  %-12.12s        %13.13s")
/* 0006 */ ,_T("%6.6s %1hd %5.5s %6.6s %13.13s %13.13s %13.13s")
/* 0007 */ ,_T(" %6.6s   T O T A L  %13.13s %13.13s %13.13s")
/* 0008 */ ,_T("PAQTE ")
/* 0009 */ ,_T("- - - - - - - - - - - - - - - - -")
/* 0010 */ ,_T(" %6.6s")
/* 0011 */ ,_T("SUBTOT.")
/* 0012 */ ,_T("VUELTO")
/* 0013 */ ,_T("%-60.60s   - - - - - - - - - - - - - - - - -")
/* 0014 */ ,_T("%-60.60s   MONTO PAGADO        %13.13s")
/* 0015 */ ,_T("                                                          V A N       ......... %13.13s")
/* 0016 */ /*,_T("                                                          V I E N E N ......... %13.13s")*/
/* 0016 */ ,_T("  ")
/* 0017 */ ,_T("SUBTOTAL    ")
/* 0018 */ ,_T("MONTO     ")
/* 0019 */ ,_T("%-50.50s %-33.33s")
/* 0020 */ ,_T("* * *   ENTRENAMIENTO * * * * *   ENTRENAMIENTO * * * * *   ENTRENAMIENTO  * * * * *")
/* 0021 */ ,_T("*  N  U  L  O  **  N  U  L  O  **  N  U  L  O  **  N  U  L  O  **  N  U  L  O  **  N  U  L  O  *")
/* 0022 */ ,_T("CARGO EXTRA ")
/* 0023 */ ,_T("TOTAL ")                  /* Used in prn_inv_TXT[5]              */
/* 0024 */ ,_T("          %1c %7.7s %13.13s %13.13s %13.13s  %-16.16s %6.6s")
/* 0025 */ ,_T("SUBTOTAL PAQTE")          /* Used in prn_inv_TXT[24]             */
/* 0026 */ ,_T("* * * * * *   C O P I A  D E  F A C T U R A  * * * * *   C O P I A  D E  F A C T U R A   * * * *")
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
/* 0039 */ ,_T(" * * * * * * *   C O P I A  D E  F A C T U R A   * * * * * *")
/* 0040 */ ,_T(" * * F A C T U R A  E N   M O D O  D E V O L U C I O N * * *")
/* 0041 */ ,_T("EXEN.")
/* 0042 */ ,_T("GRAV.")
/* 0043 */ ,_T("                                                                                                        %-13.13s")
/* 0044 */ ,_T("")
/* 0045 */ ,_T("                     IGV   Percent       Base       Valor IGV") 
/* 0045 */ /*,_T("   ")*/
/* 0046 */ ,_T("                      %1hd %10.10s %13.13s %13.13s")
/* 0046 */ /*,_T("  ")*/
/* 0047 */ /*,_T("%-16.16s %13.13s")*/
/* 0047 */ ,_T("  ")
/* 0048 */ /*,_T("VUELTO           %13.13s") se comento soporte PERU*/
/* 0048 */ ,_T("  ")
/* 0049 */ ,_T("FECHA DE EXPEDICION")
/* 0050 */ ,_T("%-13.13s  %-5.5s     %02d%02d %04d%6s")
/* 0051 */ ,_T("========================================")
/* 0052 */ ,_T("***-------****-------*** CUT CUT CUT ***------*****------*** CUT CUT CUT ***-------****-------***")
/* 0053 */ ,_T("***--***--*** CUT CUT CUT  ***--***--***")
/* 0054 */ ,_T("  CODIGO DESCRIPCION")
/* 0055 */ ,_T("--> TOTAL VENTA")
/* 0056 */ ,_T("VUELTO")
/* 0057 */ ,_T("DONACION")
/* 0058 */ ,_T("Tarifa     Compra   Base/imp        IGV")
/* 0059 */ ,_T("TOTAL")
/* 0060 */ ,_T("PASAPORTE No.  %02d-%06ld")
/* 0061 */ ,_T("CAJA No.       %d %d")
/* 0062 */ ,_T("TOTAL ARTIC's  %-6.6s")
/* 0063 */ ,_T("*** N U L O *** N U L O  *** N U L O ***")
/* 0064 */ ,_T("*** ENTRENAMIENTO **** ENTRENAMIENTO ***")
/* 0065 */ ,_T("* COPIA DE FACTURA ** COPIA DE FACTURA *")
/* 0066 */ ,_T("DESCUENTOS OFRECIDOS")
/* 0067 */ ,_T("TOTAL DESCUENTOS")
/* 0068 */ ,_T("      CANT  PRECIO UNIT.  COD.IGV  TOTAL")
/* 0069 */ ,_T("       ---------- ---------- ----------")
/* 0070 */ ,_T("*** ENTRENAMIENTO *  * ENTRENAMIENTO ***")
/* 0071 */ ,_T("* COPIA DE FACTURA    COPIA DE FACTURA *")
/* 0072 */ ,_T("* * *   ENTRENAMIENTO * *   * *   ENTRENAMIENTO * *   * *   ENTRENAMIENTO  * * * * *")
/* 0073 */ ,_T("* * * * * *   C O P I A  D E  F A C T U R A  * *   * *   C O P I A  D E  F A C T U R A   * * * *")
/* 0074 */ ,_T("TICKET Nro.        %02dT%09ld")
/* 0075 */ ,_T("           S/N %s")
/* 0076 */ ,_T("TICKET Nro.        %02dT%09ld")  /*JCP*/
/* 0077 */ ,_T("RUC Nro.        %s")  /*JCP*/
/* 0078 */ ,_T("RAZON SOCIAL:   \n%-30.30s")  /*JCP*/
/* 0079 */ ,_T("PARTICIPE EN LA PROMOCION ANIVERSARIO")   /* 27-Jan-2012 acm - add promocion*/
/* 0080 */ ,_T("PROMOCION ANIVERSARIO")                   /* 27-Jan-2012 acm - add promocion*/
/* 0081 */ ,_T("USTED HA GANADO %d CUPONES")              /* 27-Jan-2012 acm - add promocion*/
/* 0082 */ ,_T("USTED HA GANADO %d CUPON")                /* 27-Jan-2012 acm - add promocion*/
/* 0083 */ ,_T("JUEGUE Y GANE EN LA RULETA MAGICA")        /* 27-Jan-2012 acm - add promocion*/
/* 0084 */ ,_T("GANO ENTRADA AL CINE")                     /*      v3.5.0 acm - add promocion*/
/* 0085 */ ,_T("GANO %d CUPON - FERIA ESCOLAR")            /*      v3.6.0 acm - add promocion*/
/* 0086 */ ,_T("GANO %d CUPONES - FERIA ESCOLAR")          /*      v3.6.0 acm - add promocion*/
/* 0087 */ ,_T("GANE CON LA FERIA ESCOLAR")                 /*      v3.6.0 acm - add promocion*/
/* 0088 */ ,_T("--> TOTAL CON PERCEPCION")          /*      v3.6.1 acm - */
/* 0089 */ ,_T("Comprobante percepcion - Venta Interna") /*      v3.6.1 acm - */
/* 0090 */ ,_T("Percepcion del %s   S/.%6.2f")            /*      v3.6.1 acm - */
/* 0091 */ ,_T("PERCEPCION DEL %s      S/. %6.2f")            /*      v3.6.1 acm - */
/* 0092 */ ,_T("DNI CLIENTE         : %s")            /*      v3.6.1 acm - */
/* 0093 */ ,_T("NOMBRES Y APELLIDOS : %s")          /*      v3.6.1 acm - */

/* 0094 */ ,_T("DNI Nro.           : %-8.8s")  /**/
/* 0095 */ ,_T("NOMBRES Y APELLIDOS: %-20.20s")  /**/

/* 0096 */ ,_T("GANO %d CUPON - FIESTA FUTBOL")            /*     FIESTA_FUTBOL */
/* 0097 */ ,_T("GANO %d CUPONES - FIESTA FUTBOL")          /*     FIESTA_FUTBOL */
/* 0098 */ ,_T("PARTICIPE EN LA PROMO FIESTA FUTBOL")      /*     FIESTA_FUTBOL */
/* 0099 */ ,_T("----------------------------------------")
/* 0100 */ ,_T("OP. GRAVADAS")
/* 0101 */ ,_T("OP. GRATUITAS")
/* 0102 */ ,_T("OP. EXONERADAS")
/* 0103 */ ,_T("OP. INAFECTAS")
/* 0104 */ ,_T("TOT. DESCUENTO GLOBAL")
/* 0105 */ ,_T("I.S.C.")
/* 0106 */ ,_T("I.G.V. (18%)")
/* 0107 */ ,_T("TOTAL A PAGAR CON PERCEPCION")
/* 0108 */ ,_T("TOTAL VENTA:")
/* 0109 */ ,_T("CLIENTE        %s")
/* 0110 */ ,_T("DNI            %s")
/* 0111 */ ,_T("DIRECCION      %s")
/* 0112 */ ,_T("Autorizado mediante resolucion")
/* 0113 */ ,_T("Nro. 180050001201/SUNAT")
/* 0114 */ ,_T("Representacion impresa de la")
/* 0115 */ ,_T("Boleta de Venta Electronica")
/* 0116 */ ,_T("Este documento puede ser validado en")
/* 0117 */ ,_T("CODIGO DE BB Y SS SUJETOS A DETRACCION")
/* 0118 */ ,_T("COMPROBANTE DE PERCEPCION")
/* 0119 */ ,_T("PERCEPCION: %7.2f")
/* 0120 */ ,_T("TOTAL A PAGAR CON PERCEPCION: %9.2f")
/* 0121 */ ,_T("TRANSFERENCIA GRATUITA DE UN BIEN Y/O")
/* 0122 */ ,_T("SERVICIO PRESTADO GRATUITAMENTE")
/* 0123 */ ,_T("               %s")
/* 0124 */ ,_T("RAZON SOCIAL   %s")
/* 0125 */ ,_T("RUC            %s")
/* 0126 */ ,_T("Factura de Venta Electronica")
};
/*                     1         2         3         4         5         6         7         8         9       */
/*           0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456 */


/* mon_names[ i ] contains name of i'th month                                */
_TCHAR *mon_names[] = {
        _T(""), _T("ENE"), _T("FEB"), _T("MAR"), _T("ABR"), _T("MAY"), _T("JUN"),
            _T("JUL"), _T("AGO"), _T("SEP"), _T("OCT"), _T("NOV"), _T("DIC")
      };

/*******/

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
/* 0006 */ //,_T("                                           NO ENDOSABLE")   /* */
/* 0006 */   ,_T("                         NO ENDOSABLE")   /* */
/* 0007 */// ,_T("                                        %2d %06ld A*%-3.3s%2d%6s")   /* Auto.cheq.amnt.factura_no*/
/* 0007 */   ,_T("                           %2d %06ld A*%-3.3s%2d%6s")   /* Auto.cheq.amnt.factura_no*/
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

#endif /* TXT_ESP */
