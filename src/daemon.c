#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

void do_daemon(void)
{
    pid_t pid;
    int sid, dir;

    pid = fork();
    /*Caso error*/
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    /*Cerramos al proceso padre*/
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    /*Cambiamos el modo*/
    //umask(0);
    setlogmask(LOG_UPTO(LOG_INFO));
    openlog("Server system messages", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL3);
    syslog(LOG_ERR, "Initiating new server");

    sid = setsid();
    if (sid < 0)
    {
        printf("Error en sid\n");
        syslog(LOG_ERR, "Error creating a new SIF for child process");
        exit(EXIT_FAILURE);
    }

    //dir = chdir("/");
    //if (dir < 0)
    //{
    //    printf("Error en dir\n");
    //    syslog(LOG_ERR, "Error changing the working directory");
    //}

    syslog(LOG_INFO, "Closing standar file descriptors");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return;
}