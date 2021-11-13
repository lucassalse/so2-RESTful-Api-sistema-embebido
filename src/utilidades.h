
#include <stdio.h>
#include <ulfius.h>
#include <jansson.h>
#include <pwd.h>
#include <sys/types.h>
#include <syslog.h>
#include<string.h>
#include <sys/types.h>
#include <sys/stat.h>


typedef struct File{
    int id ;
    char link [500] ;
    char date_time [500] ;
    char name_product [500] ;
    char file_name [200] ;
    long int size_file ;
} File;



typedef struct Nodo{
    File file;
    struct Nodo* siguiente ;
} Nodo;


typedef struct Lista{
    Nodo* cabeza ;
    int longitud ;
} Lista ;



typedef struct Argumentos{
    int year ;
    int mon ;
    int day ;
    char product_no_const [100] ;
    char date_time_format [100] ;
    char product_format [100] ;
} Argumentos;

void eliminar_ultimo(Lista*) ;
int esta_vacia(Lista*) ;
void eliminar_principio(Lista*) ;
int contar(Lista*) ;
void insertar_final(Lista*, File*) ;
void insertar_principio(Lista*, File*) ;
void eliminar_nodo(Nodo*) ;
Nodo* crear_nodo (File*) ;
void mostrar_nodo(Nodo*) ;
void recorrer_mostar(Lista* lista) ;
Nodo* recorrer_get_fileState(Lista*, char*) ;
Nodo* recorrer_get_datetime(Lista* lista, char* datetime) ;
Nodo* recorrer_get_product(Lista* lista, char* product) ;
Nodo* recorrer_get_prod_date(Lista* lista, char* product, char* datetime) ;


