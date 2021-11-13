#include "utilidades.h"

/**
 * @brief Funcion que crea un nuevo nodo para la lista con los parametros de su argumento.
 * 
 * @param file Parametro puntero File con el cual se va a crear el nuevo nodo
 * @return Nodo* Devuelve el nodo creado con sus correspondientes paramentros
 */
Nodo* crear_nodo (File* file){
    Nodo* nodo = (Nodo*) malloc (sizeof(Nodo)) ;
    nodo->file.id = file->id ;
    strcpy(nodo->file.link, file->link) ;
    strcpy(nodo->file.date_time, file->date_time) ;
    strcpy(nodo->file.name_product, file->name_product) ;
    strcpy(nodo->file.file_name, file->file_name) ;
    nodo->file.size_file = file->size_file ;
    nodo->siguiente = NULL ;
    return nodo ;
}

/**
 * @brief Funcion que elimina un nodo de la lista mediante un free()
 * 
 * @param nodo Nodo a eliminar
 */
void eliminar_nodo(Nodo* nodo){
    free (nodo) ;
}
/**
 * @brief Funcion que inserta un nodo al principio de la lista
 * 
 * @param lista Lista que contiene los files
 * @param File File a ser insertado en la lista
 */
void insertar_principio(Lista* lista, File* file){
    Nodo* nodo = crear_nodo(file) ;
    nodo->siguiente = lista->cabeza ;
    lista->cabeza = nodo ;
    lista->longitud++ ;
}
/**
 * @brief Funcion que inserta un nodo al final de la lista
 * 
 * @param lista Lista que contiene files donde se insertara el nuevo
 * @param file File a ser insertado en la lista
 */
void insertar_final(Lista* lista, File* file){
    Nodo* nodo = crear_nodo(file) ;
    if (lista->cabeza == NULL){
        lista->cabeza = nodo ;
    }
    else{
        Nodo* puntero = lista->cabeza ;
        while (puntero->siguiente){
            puntero = puntero->siguiente ;
        }
        puntero->siguiente = nodo ;
    }
    lista->longitud++ ;
}
/**
 * @brief Funcion encargada de mantener la cantidad de elementos en la lista
 * 
 * @param lista Lista de elementos
 * @return int Cantidad de elementos en la lista
 */
int contar(Lista* lista){
    return lista->longitud ;
}
/**
 * @brief Funcion que elimina el primer elemento de la lista
 * 
 * @param lista Lista de elementos
 */
void eliminar_principio(Lista* lista){
    if(lista->cabeza){
        Nodo* eliminado = lista->cabeza ;
        lista->cabeza = lista->cabeza->siguiente ;
        eliminar_nodo(eliminado) ;
        lista->longitud-- ;
    }
}
/**
 * @brief Funcion que me permite saber si la lista esta vacia
 * 
 * @param lista Lista de elementos
 * @return int 
 */
int esta_vacia(Lista* lista){
    return lista->cabeza == NULL ;
}
/**
 * @brief Funcion que se encarga de eliminar el ultimo elemento de la lista
 * 
 * @param lista Lista de elementos
 */
void eliminar_ultimo(Lista* lista){
    if (lista->cabeza){
        if(lista->cabeza->siguiente){
            Nodo* puntero = lista->cabeza ;
            while(puntero->siguiente->siguiente){
                puntero = puntero->siguiente ;
            }
            Nodo* eliminado = puntero->siguiente ;
            puntero->siguiente = NULL ;
            eliminar_nodo(eliminado) ;
            lista->longitud-- ;
        }
        else{
            Nodo* eliminado = lista->cabeza ;
            lista->cabeza = NULL ;
            eliminar_nodo (eliminado) ;
            lista->longitud-- ;
        }
    }
}

/**
 * @brief Funcion encargada de mostrar los elementos de la lista
 * 
 * @param lista lista de elementos
 */
void recorrer_mostar(Lista* lista){
    Nodo* puntero = lista->cabeza ;
    while(puntero != NULL){   
        mostrar_nodo(puntero) ;
        //send_mensaje(puntero, mensaje) ;
        puntero = puntero->siguiente ;

    }
}
/**
 * @brief Funcion encargada de obtener un puntero nodo que coincida con el filename
 * 
 * @param lista 
 * @param filename 
 * @return Nodo* Puntero que contiene el filename pasado como parametro
 */
Nodo* recorrer_get_fileState(Lista* lista, char* filename){
    Nodo* puntero = lista->cabeza ;
    while(puntero != NULL){   
        if (strstr(puntero->file.link, filename)){
            return puntero ;
        }
        puntero = puntero->siguiente ;
    }
    return NULL ;
}
/**
 * @brief Funcion encargada de obtener un puntero nodo que coincida con el datetime
 * 
 * @param lista 
 * @param datetime 
 * @return Nodo* Puntero que contiene el datetime pasado como parametro
 */
Nodo* recorrer_get_datetime(Lista* lista, char* datetime){
    Nodo* puntero = lista->cabeza ;
    while(puntero != NULL){   
        if (strstr(puntero->file.date_time, datetime)){
            return puntero ;
        }
        puntero = puntero->siguiente ;
    }
    return NULL ;
}
/**
 * @brief Funcion encargada de obtener un puntero nodo que coincida con el product
 * 
 * @param lista 
 * @param product 
 * @return Nodo* Puntero que contiene el product pasado como parametro
 */
Nodo* recorrer_get_product(Lista* lista, char* product){
    Nodo* puntero = lista->cabeza ;
    while(puntero != NULL){   
        if (strstr(puntero->file.name_product, product)){
            return puntero ;
        }
        puntero = puntero->siguiente ;
    }
    return NULL ;
}
/**
 * @brief Funcion encargada de obtener un puntero nodo que coincida con el product y el datetime al mismo tiempos
 * 
 * @param lista 
 * @param product 
 * @param datetime 
 * @return Nodo* Puntero que contiene el datetime y product
 */
Nodo* recorrer_get_prod_date(Lista* lista, char* product, char* datetime){
    Nodo* puntero = lista->cabeza ;
    while(puntero != NULL){   
        if (strstr(puntero->file.name_product, product) && strstr(puntero->file.date_time, datetime)){
            return puntero ;
        }
        puntero = puntero->siguiente ;
    }
    return NULL ;
}

/**
 * @brief Funcion que imprime las caracterisiticas del nodo
 * 
 * @param nodo Nodo que se desea imprimir
 */
void mostrar_nodo(Nodo* nodo){
    printf("La id del file es: %d \n", nodo->file.id) ;
    printf("El link del file es: %s \n", nodo->file.link) ;
    printf("El tamanio del file es: %ld \n", nodo->file.size_file) ;
    printf("El datetime es: %s \n", nodo->file.date_time) ;
    printf("El nombre del producto es: %s \n", nodo->file.name_product) ;
    printf("El file_name es: %s \n", nodo->file.file_name) ;

}
