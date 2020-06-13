#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <time.h>

#include <assert.h>

#include "include/sockets.h"
#include "include/picoHTTPparser.h"
#include "include/daemon.h"

#define MAX_COLA 1
#define MAX_BUFF 4096
#define MAX_HEADERS 128

#define OK 200
#define NOT_FOUND 404
#define BAD_REQUEST 400

#define NOMBRE_SERVER "Servidor Turbo"

#define GET_METHOD 1
#define POST_METHOD 2
#define OPTIONS_METHOD 3

/*Definicion de rutas a archivos*/
#define FILES_PATH "files/"
#define INDEX_PATH "files/index"
#define INDEXHTML_PATH "files/index.html"

/*Para scripts*/
#define PY_EXTENSION ".py"
#define PHP_EXTENSION ".php"

#define ARG_SCRIPT "variableGET"
#define STDIN_SCRIPT "variablePOST"

/*
 * Estructura para almacenar los datos de una peticion. 
 * Contiene:
 *      -Version: Referencia la version http utilizada
 *      -Metodo: GET POST OPTIONS...
 *      -num_headers: El numero de cabeceras de la peticion
 *      -ruta: Ruta para el objeto solicitado
 *      -rutalen: Tamanio de la ruta
 *      -headers: Array con los headers en la peticion.
 *          |---> Estos a su vez estan almacenados como:
 *                +string* name   -> El nombre del header
 *                +long name_len  -> Longitud del string nombre
 *                +string* value  -> El valor del campo
 *                +long value_len -> Longitud del string value
 */
typedef struct _Peticion
{
    int version;
    char *metodo;
    int num_headers;
    char *ruta;
    char recurso_solicitado[1024];
    size_t rutalen;
    struct phr_header headers[MAX_HEADERS];
    char post_body[1024];
} Peticion;

/*********************************************************************************/
/*********************************************************************************/
/*************Funciones encargadas del procesamiento de las peticiones************/
/*********************************************************************************/
/*********************************************************************************/

/*
 * En esta funcion nos encargamos de parsear la peticion que nos llegue
 * Tras llamar a esta funcion obtenemos una estructura con toda la informacion
 * Acto seguido debemos llamar a procesar_respuesta, que se encargara de crear una respuesta
 */

/********
 * FUNCION: procesar_peticion
 * ARGS_IN: int sockfd - el socket del que recibimos la peticion
 * DESCRIPCION: Procesa la peticion
 * ARGS_OUT:
 ********/
void procesar_peticion(int sockfd);

/*********************************************************************************/
/*********************************************************************************/
/********************Funciones encargadas de generar respuestas*******************/
/*********************************************************************************/
/*********************************************************************************/

/*
 * Nos permite procesar una respuesta pasandole la informacion
 * de la peticion. 
 * Se encargará de generar la respuesta correcta o de invocar los errores pertinentes.
 */
/********
 * FUNCION: procesa una respuesta
 * ARGS_IN: int sockfd - el socket al que enviamos la respuesta
 *          Peticion* peticion la peticion
 * DESCRIPCION: Procesa la respuesta según la peticion
 * ARGS_OUT:
 ********/

void procesar_respuesta(int sockfd, Peticion *peticion);

/*********************************************************************************/
/*********************************************************************************/
/*****************Funciones encargadas crear respuestas concretas*****************/
/*********************************************************************************/
/*********************************************************************************/

/*
 * Crea una respuesta para el error 404 (Not Found).
 */
/********
 * FUNCION: error_404
 * ARGS_IN: int sockfd - el socket al que enviamos la respuesta
 *          int version - la version http
 *          int codigo_respuesta - el codigo de la respuesta
 *          char* nombre_servidor - El nombre del servidor
 * DESCRIPCION: Error 404
 * ARGS_OUT:
 ********/

void error_404(int sockfd, int version, int codigo_respuesta, char *nombre_servidor);
/*
 * Crea una respuesta para el error 400 (Bad Request).
 */

/********
 * FUNCION: error_400
 * ARGS_IN: int sockfd - el socket al que enviamos la respuesta
 *          int version - la version http
 *          int codigo_respuesta - el codigo de la respuesta
 *          char* nombre_servidor - El nombre del servidor
 * DESCRIPCION: Error 400
 * ARGS_OUT:
 ********/
void error_400(int sockfd, int version, int codigo_respuesta, char *nombre_servidor);
/*
 * Crea una respuesta para el Metodo options
 */
/********
 * FUNCION: options
 * ARGS_IN: int sockfd - el socket al que enviamos la respuesta
 *          int version - la version http
 *          int codigo_respuesta - el codigo de la respuesta
 *          char* server_name - El nombre del servidor
 * DESCRIPCION: crea respuesta para options
 * ARGS_OUT:
 ********/

void options(int sockfd, int version, int codigo_respuesta, char *server_name);

/*
 * Construye la cabecera
 */
/********
 * FUNCION: get_cabecera
 * ARGS_IN: char *path - lo que solicitan
 *          int version_http - la version http
 *          char* mensaje - aqui va el mensaje
 * DESCRIPCION: crea la cabecera
 * ARGS_OUT: mensaje
 ********/
char *get_cabecera(char *path, int version_http, char *mensaje);

/*
 * Permite obtener la fecha para ponerla en la cabecera
 */
/********
 * FUNCION: get_fecha
 * ARGS_IN: char *fecha - aqui va la fecha
 * DESCRIPCION: obtiene la fecha
 * ARGS_OUT:
 ********/

void get_fecha(char *fecha)
{
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(fecha, 128, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    return;
}

/*
 * Permite obtener la longitud para la cabecera
 */
/********
 * FUNCION: get_lenght
 * ARGS_IN: char *path - lo que solicitan
 * DESCRIPCION: obtiene la longitud de path
 * ARGS_OUT: la longitud
 ********/
long get_lenght(char *path)
{
    struct stat s;

    if (path == NULL)
    {
        return -1;
    }
    stat(path, &s);

    return s.st_size;
}

char **str_split(char *a_str, const char a_delim)
{
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char *) * count);

    if (result)
    {
        size_t idx = 0;
        char *token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    struct sockaddr *cliaddr;
    pid_t childpid;
    int n_procesos, i;

    if (argc < 3)
    {
        printf("ERROR. Introduce un puerto y un tamano maximo de cola\n");
        syslog(LOG_INFO, "Error initializing the server\n");
        exit(EXIT_FAILURE);
    }

    /*Socket, bind y accept*/
    listenfd = initiate_server(atoi(argv[1]), atoi(argv[2]));

    //do_daemon();

    if (listenfd == -1)
    {
        syslog(LOG_INFO, "Error initializing the server\n");
        exit(EXIT_FAILURE);
    }
    
    do_daemon();
    /*Bucle infinito del servidor*/
    while (1)
    {
        connfd = accept_connection(listenfd);
        if (connfd < 0)
        {
            syslog(LOG_INFO, "Error in connfd\n");
            continue;
        }
        

        n_procesos++;
        /*Si estamos en el proceso hijo*/
        if ((childpid = fork()) == 0)
        {
            //close(listenfd); //Este le cierra el hijo
            procesar_peticion(connfd);
            exit(EXIT_SUCCESS);
        }
        close(connfd); //Este le cierra el padre
        for (i = 0; i < n_procesos; i++)
        {
            wait(childpid);
        }
    }

    return 1;
}

void procesar_peticion(int sockfd)
{
    char *metodo, *ruta, buff[MAX_BUFF]; //Almacenan datos de la peticion o la peticion
    char recurso[1024];
    char post_args[2048];

    int minor_version, num_headers, check_parse = 0; //valores de la peticion / de control

    size_t bufflen, lastlen, metodo_len, ruta_len; //tamanios
    ssize_t read_len;                              //tamanios

    struct phr_header headers[MAX_HEADERS]; //Cabeceras de la peticion

    int i; //variable reglamentaria para bucles

    Peticion *peticion = NULL; //Para almacenar la peticion

    /*Bucle para parsear la peticion*/
    while (check_parse <= 0)
    {

        while ((read_len = read(sockfd, buff + bufflen, MAX_BUFF - bufflen)) == -1)
            ;
        if (read_len <= 0)
        {
            syslog(LOG_INFO, "Error leyendo peticiones\n");
            exit(EXIT_FAILURE);
        }

        lastlen = bufflen;
        bufflen += read_len;

        /* Parseamos la peticion */
        num_headers = sizeof(headers) / sizeof(headers[0]);

        check_parse = phr_parse_request(buff, bufflen, (const char **)&metodo, &metodo_len, (const char **)&ruta, &ruta_len,
                                        &minor_version, headers, &num_headers, lastlen);

        /*Si ha ocurrido algun error*/
        if (check_parse == -1)
        {
            syslog(LOG_INFO, "Error parseando la peticion\n");
            exit(EXIT_FAILURE);
        }

        /*Comprobamos que no excedemos el máximo que podemos leer*/
        if (bufflen >= MAX_BUFF)
        {
            syslog(LOG_INFO, "Error parseando la peticion. Demasiado larga\n");
            exit(EXIT_FAILURE);
        }
    }

    /*almacenamos la peticion*/
    peticion = (Peticion *)calloc(1, sizeof(Peticion));
    if (peticion == NULL)
    {
        syslog(LOG_INFO, "Error creando la estructura peticion\n");
        exit(EXIT_FAILURE);
    }

    peticion->version = minor_version;
    peticion->metodo = metodo;
    peticion->num_headers = num_headers;
    peticion->ruta = ruta;
    peticion->rutalen = ruta_len;
    for (i = 0; i < num_headers; i++)
    {
        peticion->headers[i] = headers[i];
    }

    sprintf(recurso, "%.*s", (int)ruta_len, ruta);
    printf("%s\n", recurso);
    strcpy(peticion->recurso_solicitado, recurso);

    /*aniadimos el cuerpo de la peticion POST*/
    if (strstr(peticion->metodo, "POST"))
    {
        sprintf(peticion->post_body, "%s", buff + check_parse);
        printf("Argumentos POST: %s\n", peticion->post_body);
    }

    /*Para debug*/
    /*
    printf("EN LA PETICION TENEMOS:\n\n\n\n");

    printf("Version: %d\n\n", peticion->version);
    printf("Metodo: %s\n\n", peticion->metodo);
    printf("Headers: %d\n", peticion->num_headers);
    printf("ruta: %s\n", peticion->ruta);
    printf("recurso solicitado: %s\n", peticion->recurso_solicitado);
    printf("rutalen: %ld\n", peticion->rutalen);
    for (i = 0; i < peticion->num_headers; i++)
    {
        printf("\n\nCABECERA:\n");
        printf("name: %s\n", peticion->headers[i].name);
        printf("name_len: %ld\n", peticion->headers[i].name_len);
        printf("value: %s\n", peticion->headers[i].value);
        printf("size: %ld\n", peticion->headers[i].value_len);
    }

    printf("FIN PETICION \n\n\n\n");
    */

    /*TODO debemos crear una función encargada de procesar una respuesta*/
    procesar_respuesta(sockfd, peticion);
}

void procesar_respuesta(int sockfd, Peticion *peticion)
{
    char *recurso;
    int file;
    char *path;

    int i, j, k;

    /*Para cosas de los mensajes*/
    char cabecera[2048];
    char fecha[128];
    char ultima_mod[128];
    long length;
    char buffer[8192];

    char delim[] = "?";

    /*Para el manejo de scripts*/
    FILE *script;
    char script_buffer[256];
    char **path_separado;
    char **variables = NULL;
    char **variable_get = NULL;
    char **variable_post = NULL;
    char script_command[512];
    long lenght;
    int hay_get = 0;

    struct stat s;

    int ret;

    if (peticion == NULL)
    {
        syslog(LOG_INFO, "Error, peticion es NULL\n");
        free(peticion);
        exit(EXIT_FAILURE);
    }

    /*Construimos el path*/
    path = (char *)calloc(1, sizeof("files") + sizeof(peticion->recurso_solicitado));
    strcpy(path, "files");
    strcat(path, peticion->recurso_solicitado);

    //printf("el path es: %s\n", path);

    /*METODO GET*/
    if (strstr(peticion->metodo, "GET"))
    {

        /*Si no solicitan nada o solo el index devolvemos el index.html*/
        if (strcmp(path, FILES_PATH) == 0 || strcmp(path, INDEX_PATH) == 0)
        {
            free(path);
            path = (char *)calloc(1, sizeof(INDEXHTML_PATH));
            strcpy(path, INDEXHTML_PATH);
        }

        if (strstr(path, PY_EXTENSION) || strstr(path, PHP_EXTENSION))
        {
            path_separado = str_split(path, '?');
            printf("path separado 0: %s\npath separado 1: %s\n", path_separado[0], path_separado[1]);

            /*Obtenemos los argumentos que queremos ejecutar*/
            if (path_separado[1] != NULL)
            {
                variables = str_split(path_separado[1], '&');
                printf("%s\n\n\n", variables[0]);

                for (i = 0; variables[i] != NULL; i++)
                {
                    variable_get = str_split(variables[i], '=');

                    /*Buscamos variableGET*/
                    if (strcmp(variable_get[0], ARG_SCRIPT) == 0)
                    {
                        for (j = 0; j < strlen(variable_get[1]); j++)
                        {
                            if (variable_get[1][j] == '+')
                            {
                                variable_get[1][j] = ' ';
                            }
                        }
                        if (strstr(path, PY_EXTENSION))
                        {
                            sprintf(script_command, "python %s \"%s\" 2>&1", path_separado[0], variable_get[1]);
                        }
                        if (strstr(path, PHP_EXTENSION))
                        {
                            sprintf(script_command, "php %s \"%s\"", path_separado[0], variable_get[1]);
                        }
                        break;
                    }
                }
            }

            if (strcmp(script_command, "") == 0)
            {
                if (strstr(path, PY_EXTENSION))
                {
                    sprintf(script_command, "python %s 2>&1", path_separado[0]);
                }
                if (strstr(path, PHP_EXTENSION))
                {
                    sprintf(script_command, "php %s", path_separado[0]);
                }
            }
            script = popen(script_command, "r");
            if (script == NULL)
            {
                {
                    error_404(sockfd, peticion->version, BAD_REQUEST, NOMBRE_SERVER);
                    free(path);
                    free(peticion);
                    if (path_separado != NULL)
                    {
                        for (i = 0; path_separado[i] != NULL; i++)
                        {
                            free(path_separado[i]);
                        }
                        free(path_separado);
                    }
                    if (variables != NULL)
                    {
                        for (i = 0; variables[i] != NULL; i++)
                        {
                            free(variables[i]);
                        }
                        free(variables);
                    }
                    if (variable_get != NULL)
                    {
                        for (i = 0; variable_get[i] != NULL; i++)
                        {
                            free(variable_get[i]);
                        }
                        free(variable_get);
                    }
                    if (variable_post != NULL)
                    {
                        for (i = 0; variable_post[i] != NULL; i++)
                        {
                            free(variable_post[i]);
                        }
                        free(variable_post);
                    }
                    exit(EXIT_SUCCESS);
                }
            }

            //printf("%s\n", script_buffer);
            /*Construimos la cabecera y el mensaje completo*/
            get_fecha(fecha);
            stat(path_separado[0], &s);

            /*Construimos la respuesta*/
            strftime(ultima_mod, 128, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&s.st_mtime));
            fread(script_buffer, 512, 1, script);
            lenght = strlen(script_buffer);
            printf("%ld\n\n", lenght);
            printf("path: %s\n", path_separado[0]);
            sprintf(cabecera, "HTTP/1.%d %d OK\r\nDate: %s\r\nServer: %s\r\nContent-Length: %ld\r\nLast-Modified: %s\r\nContent-Type: text/plain\r\n\r\n%s", peticion->version, OK, fecha, NOMBRE_SERVER, strlen(script_buffer), ultima_mod, script_buffer);

            printf("mensaje: %s\n", cabecera);

            /*Enviamos la cabecera*/
            send(sockfd, cabecera, strlen(cabecera), 0);
            memset(cabecera, 0, strlen(cabecera));

            free(peticion);
            free(path);
            pclose(script);
            if (path_separado != NULL)
            {
                for (i = 0; path_separado[i] != NULL; i++)
                {
                    free(path_separado[i]);
                }
                free(path_separado);
            }
            if (variables != NULL)
            {
                for (i = 0; variables[i] != NULL; i++)
                {
                    free(variables[i]);
                }
                free(variables);
            }
            if (variable_get != NULL)
            {
                for (i = 0; variable_get[i] != NULL; i++)
                {
                    free(variable_get[i]);
                }
                free(variable_get);
            }
            if (variable_post != NULL)
            {
                for (i = 0; variable_post[i] != NULL; i++)
                {
                    free(variable_post[i]);
                }
                free(variable_post);
            }
            exit(EXIT_SUCCESS);
        }

        /*
         * Caso basico del metodo GET
         * Obtener un archivo que tenemos en el servidor
         */
        file = open(path, O_RDONLY);
        /*Si no hemos podido abrir el archivo pues no existe*/
        if (file == -1)
        {
            error_404(sockfd, peticion->version, BAD_REQUEST, NOMBRE_SERVER);
            free(path);
            free(peticion);
            exit(EXIT_SUCCESS);
        }

        /*Construimos la cabecera*/
        get_cabecera(path, peticion->version, &cabecera);
        printf("mensaje: %s\n", cabecera);

        /*Enviamos la cabecera*/
        send(sockfd, cabecera, strlen(cabecera), 0);
        memset(cabecera, 0, strlen(cabecera));

        /*Enviamos el resto del archivo :)*/
        while ((ret = read(file, buffer, 8192)) > 0)
        {
            send(sockfd, buffer, ret, 0);
        }
    }
    /*METODO POST*/
    else if (strstr(peticion->metodo, "POST"))
    {
        if (strstr(path, PY_EXTENSION) || strstr(path, PHP_EXTENSION))
        {
            path_separado = str_split(path, '?');
            printf("path separado 0: %s\npath separado 1: %s\n", path_separado[0], path_separado[1]);

            variables = str_split(peticion->post_body, '&');
            for (i = 0; variables[i] != NULL; i++)
            {
                variable_post = str_split(variables[i], '=');
                if (strcmp(variable_post[0], STDIN_SCRIPT) == 0)
                {
                    for (j = 0; j < strlen(variable_post[1]); j++)
                    {
                        if (variable_post[1][j] == '+')
                        {
                            variable_post[1][j] = ' ';
                        }
                    }
                    break;
                }
            }
        }

        /*Caso con GET en la URL*/
        if (path_separado[1] != NULL)
        {
            hay_get = 1;
            variables = str_split(path_separado[1], '&');
            printf("%s\n\n\n", variables[0]);

            for (i = 0; variables[i] != NULL; i++)
            {
                variable_get = str_split(variables[i], '=');

                /*Buscamos variableGET*/
                if (strcmp(variable_get[0], ARG_SCRIPT) == 0)
                {
                    for (j = 0; j < strlen(variable_get[1]); j++)
                    {
                        if (variable_get[1][j] == '+')
                        {
                            variable_get[1][j] = ' ';
                        }
                    }
                    break;
                }
            }
        }

        if (hay_get == 1)
        {
            if (strstr(path, PY_EXTENSION))
            {
                sprintf(script_command, "echo %s | python %s \"%s\" 2>&1", variable_post[1], path_separado[0], variable_get[1]);
            }
            if (strstr(path, PHP_EXTENSION))
            {
                sprintf(script_command, "echo %s | php %s %s", variable_post[1], path_separado[0], variable_get[1]);
            }
        }
        else
        {
            if (strstr(path, PY_EXTENSION))
            {
                sprintf(script_command, "echo %s | python %s", variable_post[1], path_separado[0]);
            }
            if (strstr(path, PHP_EXTENSION))
            {
                sprintf(script_command, "echo %s | php %s", variable_post[1], path_separado[0]);
            }
        }

        script = popen(script_command, "r");
        if (script == NULL)
        {
            error_404(sockfd, peticion->version, BAD_REQUEST, NOMBRE_SERVER);
            free(path);
            free(peticion);
            if (path_separado != NULL)
            {
                for (i = 0; path_separado[i] != NULL; i++)
                {
                    free(path_separado[i]);
                }
                free(path_separado);
            }
            if (variables != NULL)
            {
                for (i = 0; variables[i] != NULL; i++)
                {
                    free(variables[i]);
                }
                free(variables);
            }
            if (variable_get != NULL)
            {
                for (i = 0; variable_get[i] != NULL; i++)
                {
                    free(variable_get[i]);
                }
                free(variable_get);
            }
            if (variable_post != NULL)
            {
                for (i = 0; variable_post[i] != NULL; i++)
                {
                    free(variable_post[i]);
                }
                free(variable_post);
            }
            exit(EXIT_SUCCESS);
        }

        //printf("%s\n", script_buffer);
        /*Construimos la cabecera y el mensaje completo*/
        get_fecha(fecha);

        stat(path_separado[0], &s);

        /*Construimos la respuesta*/
        strftime(ultima_mod, 128, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&s.st_mtime));
        fread(script_buffer, 512, 1, script);
        lenght = strlen(script_buffer);
        printf("%ld\n\n", lenght);
        sprintf(cabecera, "HTTP/1.%d %d OK\r\nDate: %s\r\nServer: %s\r\nContent-Length: %ld\r\nLast-Modified: %s\r\nContent-Type: text/plain\r\n\r\n%s", peticion->version, OK, fecha, NOMBRE_SERVER, strlen(script_buffer), ultima_mod, script_buffer);

        printf("mensaje: %s\n", cabecera);

        /*Enviamos la cabecera*/
        send(sockfd, cabecera, strlen(cabecera), 0);
        memset(cabecera, 0, strlen(cabecera));
        free(peticion);
        free(path);
        pclose(script);
        if (path_separado != NULL)
        {
            for (i = 0; path_separado[i] != NULL; i++)
            {
                free(path_separado[i]);
            }
            free(path_separado);
        }
        if (variables != NULL)
        {
            for (i = 0; variables[i] != NULL; i++)
            {
                free(variables[i]);
            }
            free(variables);
        }
        if (variable_get != NULL)
        {
            for (i = 0; variable_get[i] != NULL; i++)
            {
                free(variable_get[i]);
            }
            free(variable_get);
        }
        if (variable_post != NULL)
        {
            for (i = 0; variable_post[i] != NULL; i++)
            {
                free(variable_post[i]);
            }
            free(variable_post);
        }
        exit(EXIT_SUCCESS);
    }

    /*METODO OPTIONS*/
    else if (strstr(peticion->metodo, "OPTIONS"))
    {
        options(sockfd, peticion->version, OK, NOMBRE_SERVER);
    }
    /*SI NO ES GET/POST/OPTIONS devolvemos un 400*/
    else
    {
        error_400(sockfd, peticion->version, BAD_REQUEST, NOMBRE_SERVER);
    }
    free(peticion);
    free(path);
    exit(EXIT_SUCCESS);
}

void error_404(int sockfd, int version, int codigo_respuesta, char *server_name)
{
    char mensaje[512];
    char fecha[128];

    get_fecha(fecha);
    /*Construimos el mensaje con la cabecera*/
    sprintf(mensaje, "HTTP/1.%d %d Not Found\r\nDate: %s\r\nServer: %s\r\nContent-Length: %ld\r\nContent-Type: text/plain\r\n\r\nError 404: Not found", version, codigo_respuesta, fecha, server_name, sizeof("Error 404: Not found"));

    send(sockfd, mensaje, strlen(mensaje) + 1, 0);
    /*printf("Mensaje: %s\n",mensaje);*/

    memset(mensaje, 0, strlen(mensaje));
}

void error_400(int sockfd, int version, int codigo_respuesta, char *server_name)
{
    char mensaje[512];
    char fecha[128];

    get_fecha(fecha);
    /*Construimos el mensaje con la cabecera*/
    sprintf(mensaje, "HTTP/1.%d %d Bad Request\r\nDate: %s\r\nServer: %s\r\nContent-Length: %ld\r\nContent-Type: text/plain\r\n\r\nError 400: Bad Request", version, codigo_respuesta, fecha, server_name, sizeof("Error 400: Bad Request"));

    /*Enviamos el mensaje*/
    send(sockfd, mensaje, strlen(mensaje), 0);

    memset(mensaje, 0, strlen(mensaje));
}

void options(int sockfd, int version, int codigo_respuesta, char *server_name)
{
    char fecha[128];
    char mensaje[512];
    get_fecha(fecha);
    sprintf(mensaje, "HTTP/1.%d %d OK\r\nDate: %s\r\nServer: %s\r\nContent-Length: 0\r\nAllow: GET, POST, OPTIONS\r\n", version, codigo_respuesta, fecha, server_name);
    send(sockfd, mensaje, strlen(mensaje), 0);
    memset(mensaje, 0, strlen(mensaje));

    exit(EXIT_SUCCESS);

    return;
}

char *get_cabecera(char *path, int version_http, char *mensaje)
{

    char fecha[128];
    char ultima_mod[128];
    long length;
    struct stat s;

    if (path == NULL || mensaje == NULL)
    {
        return NULL;
    }

    /*Empezamos a procesar la cabecera*/
    /*Obtenemos la fecha y la longitud*/

    get_fecha(fecha);
    length = get_lenght(path);
    stat(path, &s);

    strftime(ultima_mod, 128, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&s.st_mtime));

    if (strstr(path, ".txt") != NULL)
    {
        sprintf(mensaje, "HTTP/1.%d %d OK\r\nDate: %s\r\nServer: %s\r\nContent-Length: %ld\r\nLast-Modified: %s\r\nContent-Type: text/plain\r\n\r\n", version_http, OK, fecha, NOMBRE_SERVER, length, ultima_mod);
    }
    else if (strstr(path, ".html") != NULL || strstr(path, ".htm") != NULL)
    {
        sprintf(mensaje, "HTTP/1.%d %d OK\r\nDate: %s\r\nServer: %s\r\nContent-Length: %ld\r\nLast-Modified: %s\r\nContent-Type: text/html\r\n\r\n", version_http, OK, fecha, NOMBRE_SERVER, length, ultima_mod);
    }
    else if (strstr(path, ".gif") != NULL)
    {
        sprintf(mensaje, "HTTP/1.%d %d OK\r\nDate: %s\r\nServer: %s\r\nContent-Length: %ld\r\nLast-Modified: %s\r\nContent-Type: image/gif\r\n\r\n", version_http, OK, fecha, NOMBRE_SERVER, length, ultima_mod);
    }
    else if (strstr(path, ".jpeg") != NULL || strstr(path, ".jpg") != NULL)
    {
        sprintf(mensaje, "HTTP/1.%d %d OK\r\nDate: %s\r\nServer: %s\r\nContent-Length: %ld\r\nLast-Modified: %s\r\nContent-Type: image/jpeg\r\n\r\n", version_http, OK, fecha, NOMBRE_SERVER, length, ultima_mod);
    }
    else if (strstr(path, ".mpeg") != NULL || strstr(path, ".mpg") != NULL)
    {
        sprintf(mensaje, "HTTP/1.%d %d OK\r\nDate: %s\r\nServer: %s\r\nContent-Length: %ld\r\nLast-Modified: %s\r\nContent-Type: video/mpeg\r\n\r\n", version_http, OK, fecha, NOMBRE_SERVER, length, ultima_mod);
    }
    else if (strstr(path, ".doc") != NULL || strstr(path, ".docx") != NULL)
    {
        sprintf(mensaje, "HTTP/1.%d %d OK\r\nDate: %s\r\nServer: %s\r\nContent-Length: %ld\r\nLast-Modified: %s\r\nContent-Type: application/msword\r\n\r\n", version_http, OK, fecha, NOMBRE_SERVER, length, ultima_mod);
    }
    else if (strstr(path, ".pdf") != NULL)
    {
        sprintf(mensaje, "HTTP/1.%d %d OK\r\nDate: %s\r\nServer: %s\r\nContent-Length: %ld\r\nLast-Modified: %s\r\nContent-Type: application/pdf\r\n\r\n", version_http, OK, fecha, NOMBRE_SERVER, length, ultima_mod);
    }

    return mensaje;
}