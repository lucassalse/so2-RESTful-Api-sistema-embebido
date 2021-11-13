/**
 * test.c
 * Small Hello World! example
 * to compile with gcc, run the following command
 * gcc -o test test.c -lulfius
 */
#include <stdio.h>
#include <ulfius.h>
#include <jansson.h>
#include <pwd.h>
#include <sys/types.h>
#include <syslog.h>
#include<string.h>


#define PORT 8080

#define BUFFER 500

json_t *get_users() ;
char* get_time() ;
void write_log(char*, char*, char*) ;

/**
 * Callback function for the web application on /helloworld url call
 */
int callback_get_users (const struct _u_request * request, struct _u_response * response, void * user_data) {
  
  printf(" Se produjo un: %s\n", request->http_verb); // Request realizado

  if(user_data == NULL){
    //printf("user_data is NULL  \n") ;
  }

  json_t *users = get_users();

  ulfius_set_json_body_response(response, 200, users);
  
  return U_CALLBACK_CONTINUE;
}

int callback_post_users(const struct _u_request * request, struct _u_response * response, void * user_data){

  printf(" Se produjo un: %s\n", request->http_verb); // Request realizado
  
  if(user_data == NULL){
    //printf("user_data is NULL  \n") ;
  }

  json_t *json_request = ulfius_get_json_body_request(request, NULL);
  json_t *body;

  const char *user   = json_string_value(json_object_get(json_request, "username"));
  const char *password = json_string_value(json_object_get(json_request, "password"));


  //printf ("El usuario recibido es: %s \n", user) ;
  //printf ("La contraseña recibida es: %s \n", password) ;

  if(user == NULL || password == NULL)
    {
      body = json_pack("{s:s}", "description", "Bad request");
      ulfius_set_json_body_response(response, 400, body) ;
      return U_CALLBACK_CONTINUE;
    }

  struct passwd *info = getpwnam(user);
  if(info != NULL) 
    {
      body = json_pack("{s:s}", "description", "El usuario ya existe") ;
      ulfius_set_json_body_response(response, 409, body) ;
      return U_CALLBACK_CONTINUE;
    }

  char command [BUFFER] ;
  //sprintf(command, "sudo useradd %s -p %s", user, password) ;
  sprintf(command, "sudo useradd -m -p $(perl -e 'print crypt($ARGV[0], \"password\")' 'luis') luis") ; //, password, user) ;
  
  printf("%s \n", command) ;
  // sudo useradd -m -p $(perl -e 'print crypt($ARGV[0], "password")' 'jorge') jorge

  FILE *pf;
  pf = popen(command, "r") ;

    if(pf == NULL){
       perror("popen");
    }

  struct passwd *new_user_info = getpwnam(user) ;
  while(new_user_info == NULL){
    new_user_info = getpwnam(user) ;
  }

  char *timestamp = get_time();
  //printf("%s \n", timestamp) ;
  body = json_pack("{s:i, s:s, s:s}", "id", new_user_info->pw_uid, "username", user, "created_at", timestamp) ;
  ulfius_set_json_body_response(response, 200, body) ;

  char mensaje[BUFFER] ;
  
  sprintf(mensaje, "Usuario %d creado \n", new_user_info->pw_uid) ;
  write_log(timestamp, "user_service", mensaje) ;


  return U_CALLBACK_CONTINUE;
}


/**
 * main function
 */
int main(void) {
  struct _u_instance instance;

  // Initialize instance with the port number
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    fprintf(stderr, "Error ulfius_init_instance, abort\n");
    return(1);
  }

  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", "/api/users", NULL, 0, &callback_get_users, NULL);

  ulfius_add_endpoint_by_val(&instance, "POST", "/api/users", NULL, 0, &callback_post_users, NULL);

  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Start framework on port %d\n", instance.port);
  

    // Wait for the user to press <enter> on the console to quit the application
    //getchar();
    while(1) ;
  } else {
    fprintf(stderr, "Error starting framework\n");
  }
  printf("End framework\n");

  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);

  return 0;
}


json_t *get_users()
{
  json_t *json_users_object = json_object();
  json_t *json_users_array  = json_array();

  int cant_users = 0 ;

  json_object_set_new(json_users_object, "data", json_users_array);

  struct passwd *passwd_info = getpwent(); // usersThe getpwent() function returns a pointer to a structure containing the broken-out fields of a record from the password database (e.g., the local  password  file  /etc/passwd

  while(passwd_info)
    {
      cant_users++ ;
      json_t *json_array_data;
      json_array_data = json_pack("{s:i, s:s}", "user_id", passwd_info->pw_uid,
                                  "username", passwd_info->pw_name);

      json_array_append(json_users_array, json_array_data);

      passwd_info = getpwent(); // return  NULL  if  there  are no more entries

    }

  endpwent();

  char mensaje[BUFFER] ;
  char *timestamp = get_time() ;

  sprintf(mensaje, "Usuarios listados: %d \n", cant_users) ;
  write_log(timestamp, "user_service", mensaje) ;

  return json_users_object;
}


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

void write_log(char* timestamp, char* name_service, char* mensaje){
  
  FILE *log;
  log = fopen("log.txt", "a+");

  if (log == NULL){
    perror ("error open log") ;
  }
  fprintf(log, "%s | %s  |  %s", timestamp, name_service, mensaje) ;
  fclose(log) ;

}
