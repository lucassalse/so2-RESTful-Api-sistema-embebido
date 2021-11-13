
#include <stdio.h>
#include <ulfius.h>
#include <jansson.h>
#include <pwd.h>
#include <sys/types.h>
#include <syslog.h>
#include<string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include "utilidades.h"


#define PORT 8081

#define BUFFER 500
#define PATH_DOWNLOADS "/var/goes_service/downloads"
#define PATH_URL  "tp3.com"


char* get_time() ;
void write_log(char*, char*, char*) ;
void get_year(char *palabras[], char* body) ;
char * get_doy(int year, int mon, int day) ;
int find_file(char* file_name) ;
json_t *get_files(Lista* lista) ;

void cargar_lista() ;

void *funcionDelHilo (void *) ;

int id = 0 ;
Lista files ;
Lista files_downloading ;
File file_dwg ;
Argumentos argumentos ;


/**
 * @brief Funcion get_goes de la api encargada de listar todos los archvios existentes.
 * 
 * @param request 
 * @param response 
 * @param user_data 
 * @return int 
 */
int callback_get_get_goes (const struct _u_request * request, struct _u_response * response, void * user_data){
  
  printf(" Se produjo un: %s\n", request->http_verb); // Request realizado

  if(user_data == NULL){
    //printf("user_data is NULL  \n") ;
  }
  json_t *files_get = get_files(&files) ;
  ulfius_set_json_body_response(response, 200, files_get);

  char mensaje[BUFFER] ;
  char *timestamp = get_time() ;

  sprintf(mensaje, " _ Archivos en el server: <%d>_. \n", contar(&files)) ;
  write_log(timestamp, "goes_service", mensaje) ;

  return U_CALLBACK_CONTINUE;
}

/**
 * @brief Funcion encargada de obtener un archvio determinado pedido por el usuario y si no e encuentra iniciar una descarga
 * 
 * @param request 
 * @param response 
 * @param user_data 
 * @return int 
 */
int callback_post_get_goes(const struct _u_request * request, struct _u_response * response, void * user_data){

  printf(" Se produjo un: %s\n", request->http_verb); // Request realizado
  
  if(user_data == NULL){
    //printf("user_data is NULL  \n") ;
  }

  json_t *json_request = ulfius_get_json_body_request(request, NULL);
  json_t *body;
  
  char *datatime_juliano[3] ; // datatime obtenido por el usuario
  char datetime_no_const [BUFFER] ; // Obtiene el datatime con el formato del usuario
  char product_no_const [BUFFER] ; // Obtiene el product con el formato del usuario
  char *product_format ; // product sin el OR
  char date_time_format[BUFFER] ; // year-doy
  const char *product   = json_string_value(json_object_get(json_request, "product")) ;
  const char *datetime  = json_string_value(json_object_get(json_request, "datetime")) ;

  if(product == NULL || datetime == NULL){
    body = json_pack("{s:s}", "description", "Bad request");
    ulfius_set_json_body_response(response, 400, body) ;
    return U_CALLBACK_CONTINUE;
  }

  // Copiamos la informacion (product, datetime) pasada por el usuario
  strcpy(datetime_no_const, datetime) ;
  strcpy(product_no_const, product) ;
  product_format = strstr(product, "ABI") ;  
  get_year(datatime_juliano, datetime_no_const) ;

  int year = atoi( datatime_juliano[0] ) ;
  int mon  = atoi( datatime_juliano[1] ) ;
  int day  = atoi (datatime_juliano[2] ) ;

  sprintf(date_time_format, "%s/%s", datatime_juliano[0], get_doy(year, mon, day) ) ;

  if (recorrer_get_prod_date(&files, product_no_const, date_time_format) == NULL && recorrer_get_prod_date(&files_downloading, product_no_const, date_time_format) == NULL ){
    
    // Si no esta en la lista de descargados o descargando iniciamos una nueva descarga
    strcpy (argumentos.product_no_const,product_no_const ) ;
    strcpy (argumentos.date_time_format, date_time_format) ;
    strcpy (argumentos.product_format, product_format) ;
    argumentos.year = year ;
    argumentos.mon = mon ;
    argumentos.day = day ;
    
    pthread_t idHilo;
    pthread_create (&idHilo, NULL, funcionDelHilo, NULL); 

    body = json_pack("{s:s}", "description", "El archivo no se encuentra voy a descargarlo");
    ulfius_set_json_body_response(response, 200, body) ;
    return U_CALLBACK_CONTINUE;

  }

  // El archivo esta descargado o se encuentra en descarga
  else{

    if (recorrer_get_prod_date(&files, product_no_const, date_time_format) != NULL) {
      Nodo* puntero = recorrer_get_prod_date(&files, product_no_const, date_time_format) ;
      
      struct stat st ;
      char path_file_name [500] ;
      sprintf(path_file_name, "%s/%s", PATH_DOWNLOADS, puntero->file.file_name) ;

      if (stat( path_file_name, &st) == -1 ){
        perror("ERROR en el stat") ;
      }

      if(st.st_size == puntero->file.size_file){ 

        json_t *json_goes_object = json_object();
        json_t *json_goes_array  = json_array();
        json_object_set_new(json_goes_object, "files", json_goes_array);

        body = json_pack("{s:i, s:s, s:i}", "file_id", puntero->file.id, "link", puntero->file.link, "filesize", puntero->file.size_file) ;

        json_array_append(json_goes_array, body) ;

        ulfius_set_json_body_response(response, 200, json_goes_object) ;
        return U_CALLBACK_CONTINUE;
      }

      else{
        //printf("El archivo ese esta DSCARGANDO AUN \n") ;
        char mensaje[BUFFER] ;
        char *timestamp = get_time() ;

        sprintf(mensaje, " Archivo en proceso de descarga \n") ;
        write_log(timestamp, "goes_service", mensaje) ;

        body = json_pack("{s:s}", "description", "Descargando todavia");
        ulfius_set_json_body_response(response, 200, body) ;
        return U_CALLBACK_CONTINUE;
      }

    }
    else{
      //printf("El archivo ese esta DSCARGANDO AUN \n") ;
      char mensaje[BUFFER] ;
      char *timestamp = get_time() ;

      sprintf(mensaje, " Archivo en proceso de descarga \n") ;
      write_log(timestamp, "goes_service", mensaje) ;

      body = json_pack("{s:s}", "description", "Descargando todavia");
      ulfius_set_json_body_response(response, 200, body) ;
      return U_CALLBACK_CONTINUE;

    }

  }
  return U_CALLBACK_CONTINUE;
}


/**
 * main function
 */
int main(void) {
  struct _u_instance instance;
  //Lista files ;

  cargar_lista() ;

  // Initialize instance with the port number
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    fprintf(stderr, "Error ulfius_init_instance, abort\n");
    return(1);
  }

  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", "/api/servers/get_goes", NULL, 0, &callback_get_get_goes, NULL);

  ulfius_add_endpoint_by_val(&instance, "POST", "/api/servers/get_goes", NULL, 0, &callback_post_get_goes, NULL);

  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Start framework on port %d\n", instance.port);
  

    // Wait for the user to press <enter> on the console to quit the application
    while(1) ;
  } else {
    fprintf(stderr, "Error starting framework\n");
  }
  printf("End framework\n");

  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);

  return 0;
}


/**
 * @brief Funcion encargada de obtener el datetime con su correspondiente formato
 * 
 * @return char* datetime con formato
 */
char* get_time(){
  
  char *datetime = malloc(50 * sizeof(char) + 1);
  int d, m, y, h, min, seg;

  time_t now;

  time(&now);

  struct tm *t = localtime(&now);

  h   = t->tm_hour ;
  min = t->tm_min ;
  seg = t->tm_sec ;

  d   = t->tm_mday ;        
  m   = t->tm_mon + 1 ;   	
  y   = t->tm_year + 1900 ;

  sprintf(datetime,"%02d-%02d-%d %02d:%02d:%02d\n", d, m, y, h, min, seg) ;
  datetime[strlen(datetime)-1] = '\0';

  return datetime ;

}
/**
 * @brief Funcion encargada de escribir en el log los eventos generados
 * 
 * @param timestamp 
 * @param name_service Nombre del servicio (service_goes)
 * @param mensaje Mensaje del evento a guardar en el log
 */
void write_log(char* timestamp, char* name_service, char* mensaje){
  
  FILE *log;
  log = fopen("log.txt", "a+");

  if (log == NULL){
    perror ("error open log") ;
  }
  fprintf(log, "%s | %s  |  %s", timestamp, name_service, mensaje) ;
  fclose(log) ;

}

/**
 * @brief Funcion encargada de obtener y parsear los datos ingresados por el usuario de formato juliano
 * 
 * @param palabras Arreglo donde obtendremos el parseo de year, mon, day
 * @param body datetime del usuario a parsear
 */
void get_year(char *palabras[], char* body){
  int count = 0;
  char *delimiters = "%";
  palabras[0] = strtok (body, delimiters);
  for (count = 1; count < 3; count++)
    {
      palabras[count] = strtok (NULL, delimiters);
      //Encuentro null salgo
      if (palabras[count] == NULL)
        break;
    }
}

//https://overiq.com/c-examples/c-program-to-calculate-the-day-of-year-from-the-date/
/**
 * @brief Funcion encargada de obtener el dia del año que corresponde en base al año, mes y dia
 * 
 * @param year 
 * @param mon 
 * @param day 
 * @return char* El dia del año
 */
char * get_doy(int year, int mon, int day){
    //printf("ESTOY EN LA FUNCION DOY \n") ;
    int days_in_feb = 28,  doy;    // day of year

    doy = day;

    char * number_doy = malloc(50 * sizeof(char) + 1);

    // check for leap year
    if( (year % 4 == 0 && year % 100 != 0 ) || (year % 400 == 0) )
    {
        days_in_feb = 29;
    }

    switch(mon)
    {
        case 2:
            doy += 31;
            break;
        case 3:
            doy += 31+days_in_feb;
            break;
        case 4:
            doy += 31+days_in_feb+31;
            break;
        case 5:
            doy += 31+days_in_feb+31+30;
            break;
        case 6:
            doy += 31+days_in_feb+31+30+31;
            break;
        case 7:
            doy += 31+days_in_feb+31+30+31+30;
            break;            
        case 8:
            doy += 31+days_in_feb+31+30+31+30+31;
            break;
        case 9:
            doy += 31+days_in_feb+31+30+31+30+31+31;
            break;
        case 10:
            doy += 31+days_in_feb+31+30+31+30+31+31+30;            
            break;            
        case 11:
            doy += 31+days_in_feb+31+30+31+30+31+31+30+31;            
            break;                        
        case 12:
            doy += 31+days_in_feb+31+30+31+30+31+31+30+31+30;            
            break;                                    
    }
    
    printf("Day of year: %d \n", doy);
    if (doy<10){
      sprintf(number_doy, "00%d" , doy) ;
    }
    if (doy<100 && doy >9){
      sprintf(number_doy, "0%d" , doy) ;
    }
    if (doy>99){
      sprintf(number_doy, "%d" , doy) ;
    }
    return number_doy ;
}

/**
 * @brief Funcion encargada de determinar si un file se encuentra en el directorio
 * 
 * @param file_name File que queremos buscar
 * @return int 1: si se encuentra; 0 si no se encuentra
 */
int find_file(char* file_name){

  char readbuff[100] ;
  char command_ls [500] ;
  //int len ;
  sprintf(command_ls, "ls %s | grep %s", PATH_DOWNLOADS, file_name) ;
  printf("El comando ls es: %s \n", command_ls ) ;
  FILE *cmd_grep;
  cmd_grep = popen(command_ls, "r") ;

  if(cmd_grep == NULL){
    perror("popen");
  }
  fgets(readbuff, 100, cmd_grep) ;
  pclose(cmd_grep) ;

  if (strlen( readbuff) > 10){
    return 1 ;
  }
  else{
    return 0 ;
  }
  return 0 ;
}

json_t *get_files(Lista *lista){
  json_t *json_users_object = json_object();
  json_t *json_users_array  = json_array();

  json_object_set_new(json_users_object, "files", json_users_array);

  Nodo* puntero = lista->cabeza ;
    while(puntero != NULL){   

      json_t *json_array_data ;  
      json_array_data = json_pack("{s:i, s:s, s:i}", "file_id", puntero->file.id, "link", puntero->file.link, "filesize", puntero->file.size_file);

      json_array_append(json_users_array, json_array_data);
      puntero = puntero->siguiente ;
    }

    return json_users_object ;

}

/**
 * @brief Funcion encargada de guardar los archivos ya existentes en una lista
 * 
 */
void cargar_lista(){

  FILE *cmd_grep;
  char command_ls [500] ;
  sprintf(command_ls, "ls %s > archivos.txt", PATH_DOWNLOADS) ;
  cmd_grep = popen(command_ls, "r") ;

  if(cmd_grep == NULL){
    perror("popen");
  }
  pclose(cmd_grep) ;  

  FILE *fe;
  char readbuff[100] ;
  char *file_name ;
  char file_name_original[100] ;
  char link [BUFFER] ;
  char datetime_format [BUFFER] ;
  char product_format [BUFFER] ;
  fe = fopen("archivos.txt","r");
  if (fe == NULL){
    perror ("error open archvos") ;
  }
  //int id = 0;

  while(fgets(readbuff, 100, fe) != NULL){

    id ++ ;
    file_name = strtok (readbuff, "\n") ;
    strcpy(file_name_original, file_name) ;

    char * s_character ;
    char  s_year [4] ;
    char  s_year_1 [100] ;
    bzero(s_year_1, sizeof(s_year_1)) ;
    char  s_doy [3] ;
    char  s_doy_1 [100] ;
    bzero(s_doy_1, sizeof(s_doy_1)) ;

    char  *or_abi_1  ;
    char  *or_abi_2  ;
    char  *or_abi_3  ;
    

    s_character= strstr(file_name, "_s") ;

    strncpy(s_year, s_character+2, 4) ;
    strncpy(s_year_1, s_year, 4) ;
    strncpy(s_doy, s_character+6, 3) ;
    strncpy(s_doy_1, s_doy, 3) ;

    sprintf(datetime_format, "%s/%s",s_year_1, s_doy_1) ;

    or_abi_1 = strtok(file_name, "-") ;
    or_abi_2 = strtok(NULL, "-") ;
    or_abi_3 = strtok(NULL, "-") ;

    sprintf(product_format, "%s-%s-%s",or_abi_1, or_abi_2, or_abi_3 ) ;

    char path_file_name [500] ;
    sprintf(path_file_name, "%s/%s", PATH_DOWNLOADS, file_name_original) ;
    struct stat st ;

    if (stat( path_file_name, &st) == -1 ){
        perror("ERROR en el stat") ;
    }
    File file ;
    file.id = id ;
    file.size_file = st.st_size ; 
    sprintf(link, "http://%s/downloads/%s", PATH_URL, file_name_original) ;

    strcpy(file.link, link) ;
    strcpy(file.date_time, datetime_format) ;
    strcpy(file.name_product, product_format) ;
    strcpy(file.file_name, file_name_original) ;
    insertar_final(&files, &file) ;
  }

  fclose (fe) ;
  recorrer_mostar(&files) ;

}
/**
 * @brief Funcion del hilo encargadad de ejecutar la descarga si el archivo no se encuentra
 * 
 * @param parametro 
 * @return void* 
 */
void *funcionDelHilo (void * parametro){

  char command_ls_aws [BUFFER] ; // linea de comando para listar los archivos en aws
  char first_line[BUFFER] ; // Primer file_name leidode aws ls
  char command_download_aws [1000] ;
  char link [BUFFER] ; // Link para enviar al usuario
  //File file_dwg ;
  file_dwg.id = 0;
  strcpy(file_dwg.name_product, argumentos.product_no_const) ;
  strcpy(file_dwg.date_time, argumentos.date_time_format) ;
  insertar_final(&files_downloading, &file_dwg) ;

  // OBTENER ARCHIVO DESEADO A DESCARGAR
  sprintf(command_ls_aws, "aws s3 ls noaa-goes16/%s/%d/%s/00/ --no-sign-request > files_product.txt", argumentos.product_format, argumentos.year, get_doy(argumentos.year, argumentos.mon, argumentos.day)) ;
  printf("El comando para el ls del aws es: %s \n", command_ls_aws) ;

  //Obtebenos el primer archivo de la fecha y producto pedido
  FILE *pf;
  pf = popen(command_ls_aws, "r") ;

  if(pf == NULL){
    perror("popen");
  }
  pclose(pf) ;

  FILE *files_product;

  files_product = fopen("files_product.txt","r");

  fgets(first_line, 500, files_product) ;

  strtok (first_line, " ") ;
  strtok(NULL, " ") ;
  char* size_file = strtok(NULL, " ") ; // Tamanio final que deberia tener
  long int size_f = atol(size_file) ; // Tamanio final que deberia tener en long int
  remove("files_product.txt") ;

  
  char *file_name ; // el nombre del archivo
  file_name = strstr(first_line, "OR") ;
  file_name = strtok (file_name, "\n") ;
  
  id++ ;
  File file ;
  file.id = id ;
  file.size_file = size_f ;
  strcpy(file.name_product, argumentos.product_no_const) ;
  strcpy(file.date_time, argumentos.date_time_format) ;
  strcpy(file.file_name, file_name) ;
  sprintf(link, "http://%s/downloads/%s", PATH_URL, file_name) ;
  strcpy(file.link, link) ;
  insertar_final(&files, &file) ;

  char mensaje[BUFFER] ;
  char *timestamp = get_time() ;

  sprintf(mensaje, " Archivos descargados nuevos: <1> \n") ;
  write_log(timestamp, "goes_service", mensaje) ;

  sprintf(command_download_aws, "aws s3 cp s3://noaa-goes16/%s/%d/%s/00/%s %s --no-sign-request" , argumentos.product_format, argumentos.year, get_doy(argumentos.year, argumentos.mon, argumentos.day), file_name, PATH_DOWNLOADS) ;
  printf(" %s \n", command_download_aws) ;


  
  FILE *df;
  df = popen(command_download_aws, "r") ;

  if(df == NULL){
    perror("popen");
  }

  if (parametro == NULL){

  }
  return NULL;

}
