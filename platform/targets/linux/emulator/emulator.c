/***************************************************************************
 *   Copyright (C) 2020 by Federico Amedeo Izzo IU2NUO,                    *
 *                         Niccolò Izzo IU2KIN                             *
 *                         Frederik Saraci IU2NRO                          *
 *                         Silvano Seva IU2KWO                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/


#include "emulator.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <interfaces/include/display.h>

radio_state Radio_State = {12, 8.2f, 3, 4, 1, false};


typedef int (*_climenu_fn)(void* self, int argc, char ** argv );
typedef struct {
    char * name;
    char * description;
    void * var;
    _climenu_fn fn;
} _climenu_option;

int template(void * _self, int _argc, char ** _argv ){
    _climenu_option * self = (_climenu_option*) _self;
    printf( "%s\n\t%s\n" , self->name, self->description);

    for( int i = 0; i < _argc; i++ ){
        if( _argv[i] != NULL ){
            printf("\tArgs:\t%s\n", _argv[i]);
        }
    }
    return 0; // continue
}
int pressKey(void * _self, int _argc, char ** _argv ){
    _climenu_option * self = (_climenu_option*) _self;
    printf("Press Keys: [\n");
    for( int i = 0; i < _argc; i++ ){
        if( _argv[i] != NULL ){
            printf("\t%s, \n", _argv[i]);
        }
    }
    printf("\t]\n");
    return 0; // continue
}
int screenshot(void * _self, int _argc, char ** _argv ){
    _climenu_option * self = (_climenu_option*) _self;
    uint16_t * fb = (uint16_t*) display_getFrameBuffer();
    return 0; // continue
}

int setFloat(void * _self, int _argc, char ** _argv ){
    _climenu_option * self = (_climenu_option*) _self;
    if( _argc <= 0 || _argv[0] == NULL ){
        printf("%s is %f\n", self->name,  *(float*)(self->var));
    } else {
        sscanf(_argv[0], "%f", (float *)self->var);
        printf("%s is %f\n", self->name,  *(float*)(self->var));
    }
    return 0; // continue

}
int toggleVariable(void * _self, int _argc, char ** _argv ){
    _climenu_option * self = (_climenu_option*) _self;
    *(int*)self->var = ! *(int*)self->var; //yeah, maybe this got a little out of hand
    return 0; // continue

}
int shell_quit(void * _self, int _argc, char ** _argv ){
    return -1; //normal quit
}
int printState(void * _self, int _argc, char **_argv)
{
    printf("\nCurrent state\n");
    printf("RSSI   : %f\n", Radio_State.RSSI);
    printf("Battery: %f\n", Radio_State.Vbat);
    printf("Mic    : %f\n", Radio_State.micLevel);
    printf("Volume : %f\n", Radio_State.volumeLevel);
    printf("Channel: %f\n", Radio_State.chSelector);
    printf("PTT    : %s\n\n", Radio_State.PttStatus ? "true" : "false");
    return 0;
}

int shell_help(void * _self, int _argc, void ** _argv );
_climenu_option _options[] = {
    {"rssi",   "set rssi",             (void*)&Radio_State.RSSI,       setFloat },
    {"vbat",   "set vbat",             (void*)&Radio_State.Vbat,       setFloat },
    {"mic",    "set miclevel",         (void*)&Radio_State.micLevel,   setFloat },
    {"volume", "set volume",           (void*)&Radio_State.volumeLevel,setFloat },
    {"channel","set channel",          (void*)&Radio_State.chSelector, setFloat },
    {"ptt",    "toggle ptt",           (void*)&Radio_State.PttStatus,  toggleVariable },
    {"key",    "press keys (keyname)+",NULL,                           pressKey },
    {"screenshot","press keys (keyname)+",NULL,                        screenshot },
    {"show",   "show current state",   NULL, printState },
    {"help",   "print a full help",    NULL, shell_help },
    {"quit",   "quit", 'q',            NULL, shell_quit },
};
int num_options = (sizeof( _options )/ sizeof(_climenu_option));
int shell_help(void * _self, int _argc, void ** _argv ){
    printf("OpenRTX emulator shell\n");
    printf("name\tdescription\n");
    printf("____\t___________\n");
    for( int i = 0; i < num_options; i++ ){
        _climenu_option * o = &_options[i];
        printf("%s -> %s\n", o->name, o->description);
    }
    return 0; //normal quit
}


_climenu_option * findMenuOption(char * tok){
    for( int i = 0; i < num_options; i++ ){
        _climenu_option * o = &_options[i];
        if( strncmp(tok, o->name, strlen(tok)) == 0 ){ 
            //allows for shortcuts like just r instead of rssi
            //priority for conflicts (like if there's "rssi" and "rebuild" or some other name collision for a shortcut)
            //is set by ordering in the _options array
            return o;
        }
    }
    return NULL;
}
void striplastnewline(char * token){
    if( token == NULL ){
        return;
    }
    if( token[ strlen(token)-1 ] == '\n' ){
        token[ strlen(token)-1 ] = 0; //cut off the last character, a \n because user hit enter key
    }
}
int process_line(char * line){
    char * token = strtok( line, " ");
    striplastnewline(token);
    _climenu_option * o = findMenuOption(token);
    char * args[12] = {NULL};
    int i = 0;
    for( i = 0; i < 12; i++ ){
        //immediately strtok again since first is a command rest are args
        token = strtok(NULL, " ");
        if( token == NULL ){
            break;
        }
        striplastnewline(token);
        args[i] = token;
    }
    if( token != NULL ){
        printf("\nGot too many arguments, args truncated \n");
    }
    if( o != NULL ){
        return o->fn(o, i, args);
    } else {
        return 1; //not understood
    }
}
void *startCLIMenu()
{
    shell_help(NULL,0,NULL);
    printf("> ");
    char shellbuf[256] = {0};
    int ret = 0;
    do {
        fgets(shellbuf, 255, stdin);
        if( strlen(shellbuf) > 1 ){ //1 is just a newline
            ret = process_line(shellbuf);
        } else {
            ret = 0;
        }
        switch(ret){
            default:
                break;
            case 1:
                printf("?\n(type h or help for help)\n");
                ret = 0; //i'd rather just fall through, but the compiler warns. blech.
                printf("\n>");
                break;
            case 0:
                printf("\n>");
                break;
        }
    } while ( ret == 0 );
    printf("73\n");
    exit(0);
}


void emulator_start()
{
    pthread_t cli_thread;
    int err = pthread_create(&cli_thread, NULL, startCLIMenu, NULL);

    if(err)
    {
        printf("An error occurred starting the emulator thread: %d\n", err);
    }
}
