#include <syslog.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>

#include "script.h"

int readConfig = false;
Script *script;

void sig_handler(int signo)
{
    if(signo == SIGTERM)
    {
        syslog(LOG_INFO, "SIGTERM has been caught! Exiting...");
        if(remove("run/daemon.pid") != 0)
        {
            syslog(LOG_ERR, "Failed to remove the pid file. Error number is %d", errno);
            exit(1);
        }
        exit(0);
    }
    if(signo == SIGHUP)
    {
        syslog(LOG_INFO, "SIGHUP has been caught!");
        script->reReadConfig();
        script->perform();
        readConfig = true;
    }
}

void handle_signals()
{
    if(signal(SIGTERM, sig_handler) == SIG_ERR)
    {
        syslog(LOG_ERR, "Error! Can't catch SIGTERM");
        exit(1);
    }
    if(signal(SIGHUP, sig_handler) == SIG_ERR)
    {
        syslog(LOG_ERR, "Error! Can't catch SIGHUP");
        exit(1);
    }
}

void daemonise()
{
    pid_t pid, sid;
    FILE *pid_fp;

    syslog(LOG_INFO, "Starting daemonisation.");

    //First fork
    pid = fork();
    if(pid < 0)
    {
        syslog(LOG_ERR, "Error occured in the first fork while daemonising. Error number is %d", errno);
        exit(1);
    }

    if(pid > 0)
    {
        syslog(LOG_INFO, "First fork successful (Parent)");
        exit(0);
    }
    syslog(LOG_INFO, "First fork successful (Child)");

    //Create a new session
    sid = setsid();
    if(sid < 0)
    {
        syslog(LOG_ERR, "Error occured in making a new session while daemonising. Error number is %d", errno);
        exit(1);
    }
    syslog(LOG_INFO, "New session was created successfuly!");

    //Second fork
    pid = fork();
    if(pid < 0)
    {
        syslog(LOG_ERR, "Error occured in the second fork while daemonising. Error number is %d", errno);
        exit(1);
    }

    if(pid > 0)
    {
        syslog(LOG_INFO, "Second fork successful (Parent)");
        exit(0);
    }
    syslog(LOG_INFO, "Second fork successful (Child)");

    pid = getpid();

    //Change working directory to Home directory
    if(chdir(getenv("HOME")) == -1)
    {
        syslog(LOG_ERR, "Failed to change working directory while daemonising. Error number is %d", errno);
        exit(1);
    }

    //Grant all permisions for all files and directories created by the daemon
    umask(0);

    //Redirect std IO
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    if(open("/dev/null",O_RDONLY) == -1)
    {
        syslog(LOG_ERR, "Failed to reopen stdin while daemonising. Error number is %d", errno);
        exit(1);
    }
    if(open("/dev/null",O_WRONLY) == -1)
    {
        syslog(LOG_ERR, "Failed to reopen stdout while daemonising. Error number is %d", errno);
        exit(1);
    }
    if(open("/dev/null",O_RDWR) == -1)
    {
        syslog(LOG_ERR, "Failed to reopen stderr while daemonising. Error number is %d", errno);
        exit(1);
    }

    //Create a pid file
    mkdir("run/", 0777);
    pid_fp = fopen("run/daemon.pid", "w");
    if(pid_fp == NULL)
    {
        syslog(LOG_ERR, "Failed to create a pid file while daemonising. Error number is %d", errno);
        exit(1);
    }
    if(fprintf(pid_fp, "%d\n", pid) < 0)
    {
        syslog(LOG_ERR, "Failed to write pid to pid file while daemonising. Error number is %d, trying to remove file", errno);
        fclose(pid_fp);
        if(remove("run/daemon.pid") != 0)
        {
            syslog(LOG_ERR, "Failed to remove pid file. Error number is %d", errno);
        }
        exit(1);
    }
    fclose(pid_fp);
}

int main(int argc, char* argv[])
{
    char currentDir[PATH_MAX];
    char pidDir[PATH_MAX];
    char configDir[PATH_MAX];
    FILE *pidFile;
    pid_t pid;

    if(argc != 2)
    {
        syslog(LOG_ERR, "Wrong commands line\navailable commands:\ndaemon start\ndaemon stop %d", errno);
        exit(1);
    }

    if (getcwd(currentDir, sizeof(currentDir)) != NULL)
    {
        strcpy(configDir, currentDir);
        strcat(configDir, "/config.txt");
        syslog(LOG_DEBUG, "Get config file path");
        syslog(LOG_DEBUG, "config name %s", configDir);
    }
    else
    {
        syslog(LOG_ERR, "Can't get config file path");
    }

//start and stop commands
    strcat(pidDir, getenv("HOME"));
    strcat(pidDir, "/run/daemon.pid");
    if (!strcmp(argv[1], "stop"))
    {
        pidFile = fopen(pidDir, "r");
        if(pidFile == NULL)
        {
            syslog(LOG_DEBUG, "Daemon not started.");
            exit(0);
        }
        else
        {
            fscanf(pidFile, "%d", &pid);
            fclose(pidFile);
            if (pid >= 0)
            {
                syslog(LOG_DEBUG, "Killing daemon");
                syslog(LOG_DEBUG, "pid is %d", pid);
                kill(pid, SIGTERM);
            }
            remove(pidDir);
            exit(0);
        }
    }
    else if (!strcmp(argv[1], "start"))
    {
        pidFile = fopen(pidDir, "r");
        if(pidFile != NULL)
        {
            //killing old daemon
            fscanf(pidFile, "%d", &pid);
            fclose(pidFile);
            syslog(LOG_DEBUG, "Killing old daemon.");
            kill(pid, SIGTERM);
        }
        //and starting new
        daemonise();
        handle_signals();
        syslog(LOG_DEBUG, "Created daemon.");
    }
    else
    {
        syslog(LOG_ERR, "Unknown command\nuse start or stop command.");
        exit(1);
    }

    script = new Script(configDir);
    script->readConfig();
    script->perform();

    while(1)
    {
        if(script->haveChanges())
        {
            script->perform();
        }
        sleep(3);
    }

    delete(script);
    return 0;
}
