#include <syslog.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>

#include "script.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (10 * ( EVENT_SIZE + 16 ))

Script::Script(char* configName)
{
    strcpy(configPath, configName);
    inoInit = inotify_init();
    watchDesc = -1;
}

bool Script::readConfig()
{
    FILE *configFile = fopen(configPath, "r");
    char toWatch[PATH_MAX];
    if (configFile == NULL)
    {
        syslog(LOG_ERR, "Can't open config. Error number is %d.", errno);
        exit(1);
    }
    if (fscanf(configFile, "%s", dirNameIn) < 0 || fscanf(configFile, "%s", dirNameOut) < 0)
    {
        fclose(configFile);
        return false;
    }

    strcpy(toWatch, dirNameIn);
    strcat(toWatch, "/");
    watchDesc = inotify_add_watch(inoInit, toWatch, IN_ALL_EVENTS );
    if(watchDesc < 0)
    {
        syslog(LOG_ERR, "Can't watch for %s file. Error number is %d", toWatch, errno);
    }

    fclose(configFile);
    return true;
}

bool Script::reReadConfig()
{
    syslog(LOG_DEBUG, "Going to read config.");
    if (watchDesc != -1)
    {
        inotify_rm_watch(inoInit, watchDesc);
    }
    syslog(LOG_DEBUG, "Rereading config.");
    if(readConfig())
    {
        return true;
    }
    return false;
}

bool Script::perform()
{
    DIR *dir1, *dir2;
    struct dirent *entry;
    char file[PATH_MAX];
    char command[PATH_MAX];
    char others[PATH_MAX];
    char img[PATH_MAX];

    dir1 = opendir(dirNameIn);
    dir2 = opendir(dirNameOut);
    syslog(LOG_DEBUG, "Directories names: %s ,%s.", dirNameIn, dirNameOut);

    if (dir1 == NULL || dir2 == NULL)
    {
        syslog(LOG_ERR, "Can't open directories in perform. Error number is %d.", errno);
        return false;
    }
    else
    {
        syslog(LOG_DEBUG, "Directories opened.");
    }

    while ((entry = readdir(dir2)) != NULL)
    {
        strcpy(file, dirNameOut);
        strcat(file, "/");
        strcat(file, entry->d_name);
        strcpy(command, "rm -rf ");
        strcat(command, file);
        if(system(command) != 0)
        {
            syslog(LOG_DEBUG, "Can't perform command: %s.", command);
        }
        syslog(LOG_DEBUG, "File %s will be removed.", file);
        syslog(LOG_DEBUG, "File %s removed.", entry->d_name);
    }

    closedir(dir2);

    strcpy(others, dirNameOut);
    strcat(others, "/OTHERS");
    strcpy(command, "mkdir ");
    strcat(command, others);
    if(system(command) != 0)
    {
        syslog(LOG_DEBUG, "Can't create OTHERS folder.");
        return false;
    }

    strcpy(img, dirNameOut);
    strcat(img, "/IMG");
    strcpy(command, "mkdir ");
    strcat(command, img);
    if(system(command) != 0)
    {
        syslog(LOG_DEBUG, "Can't create IMG folder.");
        return false;
    }
    while ((entry = readdir(dir1)) != NULL)
    {
        strcpy(file, dirNameIn);
        strcat(file, "/");
        strcat(file, entry->d_name);
        strcpy(command, "cp ");
        strcat(command, file);
        strcat(command, " ");
        if(strstr(entry->d_name, ".png") == NULL)
        {
            strcat(command, others);
        }
        else
        {
            strcat(command, img);
        }

        if(system(command) != 0)
        {
            syslog(LOG_DEBUG, "Can't perform command: %s.", command);
        }
        syslog(LOG_DEBUG, "File %s will be copied.", file);
        syslog(LOG_DEBUG, "File %s copied.", entry->d_name);
    }

    closedir(dir1);
    return true;

}

bool Script::haveChanges()
{
    char buffer[EVENT_BUF_LEN];

    int length = read(inoInit, buffer, EVENT_BUF_LEN );

    struct inotify_event *event;
    for (int i = 0; i < length; i += EVENT_SIZE + event->len)
    {
        event = ( struct inotify_event * ) &buffer[ i ];
        if ( event->len )
        {
            if ( event->mask & IN_ISDIR )
            {
                if ( event->mask & IN_ATTRIB )
                {
                    syslog(LOG_INFO, "Directory %s metadata changed.\n", event->name );
                    return true;
                }
                else if ( event->mask & IN_DELETE)
                {
                    syslog(LOG_INFO, "File %s was deleted.\n", event->name );
                    return true;
                }
            }
            else
            {
                if ( event->mask & IN_CREATE )
                {
                    syslog(LOG_INFO, "New file %s was created.\n", event->name );
                    return true;
                }
                else if ( event->mask & IN_ATTRIB )
                {
                    syslog(LOG_INFO, "File %s metadata changed.\n", event->name );
                    return true;
                }
                else if ( event->mask & IN_MODIFY )
                {
                    syslog(LOG_INFO, "File %s was modified.\n", event->name );
                    return true;
                }
                else if ( event->mask & IN_DELETE_SELF)
                {
                    syslog(LOG_INFO, "File %s was deleted.\n", event->name );
                    return true;
                }
                else if ( event->mask & IN_MOVED_TO)
                {
                    syslog(LOG_INFO, "File %s was moved to.\n", event->name );
                    return true;
                }
                else if ( event->mask & IN_MOVED_FROM)
                {
                    syslog(LOG_INFO, "File %s was moved from.\n", event->name );
                    return true;
                }
            }
        }
    }

    return false;
}

Script:: ~Script()
{
    close(inoInit);
}
