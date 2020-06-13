#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

/*
 * Funcionamiento básico de la creación de un servidor basado en sockets
 * ________
 * |Socket| ===> Crea un socket que no tiene dirección asignada
 * ¯¯|¯¯¯¯¯
 *   |
 *   | 
 *   v
 * ________
 * | bind | ===> Le asigna una dirección al un socket ya cread0
 * ¯¯|¯¯¯¯¯
 *   |
 *   | 
 *   v
 * ________
 * |listen| ===> Crea la cola de conexiones pendientes
 * ¯¯|¯¯¯¯¯
 *   |
 *   | 
 *   v
 * ________
 * |accept| ===> Acepta una conexion (Espera si no hay ninguna)
 * ¯¯¯¯¯¯¯¯
 * 
 * Tras esto se recibe la petición y se responde. Pero lo anterior la inicializa
 */

int initiate_server(int port, int max_conn)
{
    int sockval, opt_val = 1;
    struct sockaddr_in direction;
    syslog(LOG_INFO, "Creating socket");

    /* Inicializamos un socket:
     * -Queremos que tenga IPv4 (AF_INET)
     * -Conexión basada en byte streams (SOCK_STREAM)
     * -No especificamos protocolo
     */
    if ((sockval = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        syslog(LOG_ERR, "Error creating socket");
        exit(EXIT_FAILURE);
    }

    /*Direccion de nuestro socket*/
    direction.sin_family = AF_INET;                /*TCP / IPfamily*/
    direction.sin_port = htons(port);              /*Asigna un puerto*/
    direction.sin_addr.s_addr = htonl(INADDR_ANY); /*Acepta todas las direcciones*/

    //bzero((void *)&(direction.sin_zero), 8); /*zero a byte string*/
    syslog(LOG_INFO, "Binding socket");

    if (setsockopt(sockval, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val) < 0)
    {
        syslog(LOG_INFO, "Error setsockopt\n");
        exit(EXIT_FAILURE);
    }

    /* Registra el socket en un puerto
     * Le pasamos el socket que hemos inicializado antes (sockval)
     * Y la dirección calculada antes
     */
    if ((bind(sockval, (struct sockaddr *)&direction, sizeof(direction))) < 0)
    {
        syslog(LOG_ERR, "Error binding socket");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Listening connections");

    /* Creamos la cola de conexiones */
    if (listen(sockval, max_conn) < 0)
    {
        syslog(LOG_ERR, "Error listenining");
        exit(EXIT_FAILURE);
    }

    return sockval;
}

int accept_connection(int sockval)
{
    int desc, len;
    struct sockaddr conection;

    len = sizeof(conection);
    
    /* Aceptamos la conexión
     * -De nuestro socket (por eso el sockval)
     * -conection almacena la informacion de la direccion remota
     * -len especifica el tadir_to_ignore/mano de conection
     */
    if ((desc = accept(sockval, &conection, (socklen_t*) &len)) < 0)
    {
        syslog(LOG_ERR, "Error accepting connection");
        exit(EXIT_FAILURE);
    }

    return desc;
}
