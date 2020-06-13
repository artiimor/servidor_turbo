#ifndef SOCKETS_H
#define SOCKETS_H


/********
 * FUNCION: initiate_server
 * ARGS_IN: int port - Valor del puerto de nuestro socket
 *          int max_conn - Tamano de la cola de espera de peticiones para el listen
 * DESCRIPCION: Inicializa el servidor. Llamadas a socket() bind() y listen()
 * ARGS_OUT: Un descriptor a nuestro socket o EXIT_FAILURE si algo falla
 ********/

int initiate_server(int port, int max_conn);

/********
 * FUNCION: initiate_server
 * ARGS_IN: int sockval - socket del que vamos a aceptar la conexión
 * DESCRIPCION: Aceptamos la conexion del cliente
 * ARGS_OUT: int
 ********/

int accept_connection(int sockval);

/********
 * FUNCION: launch_service
 * ARGS_IN: int connval - valor de la conexion
 * DESCRIPCION: inicia el servicio
 * ARGS_OUT:
 ********/

void launch_service(int connval);

#endif