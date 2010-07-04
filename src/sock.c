#include <stdio.h>
#include <string.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Ecore_Evas.h>
#include "sock.h"
#include "gm.h"

#define MSG_ACTIVATE "Activate"

static Ecore_Evas *main_win;
static Ecore_Con_Server *server;

typedef struct
{
    char* msg;
    int size;
} client_data_t;


static Eina_Bool _client_add(void *param  __attribute__((unused)),
            int ev_type __attribute__((unused)), void *ev)
{
    Ecore_Con_Event_Client_Add* e = ev;
    client_data_t* msg = malloc(sizeof(client_data_t));
//    printf("_client_add\n");
    msg->msg = strdup("");
    msg->size = 0;
    ecore_con_client_data_set(e->client, msg);
    return 0;
}

static Eina_Bool _client_del(void *param  __attribute__((unused)),
            int ev_type __attribute__((unused)), void *ev)
{
    Ecore_Con_Event_Client_Del *e = ev;
    client_data_t *msg = ecore_con_client_data_get(e->client);

//    printf("_client_del\n");
    /* Handle */
   if(strlen(MSG_ACTIVATE) == msg->size &&
        !strncmp(MSG_ACTIVATE, msg->msg, msg->size))
    {
        ecore_evas_show(main_win);
        ecore_evas_raise(main_win);
        Evas* evas = ecore_evas_get(main_win);
        gm_choicebox_raise_root(evas);
    }

    //printf(": %.*s(%d)\n", msg->size, msg->msg, msg->size);

    free(msg->msg);
    free(msg);
    return 0;
}

static Eina_Bool _client_data(void *param  __attribute__((unused)),
             int ev_type  __attribute__((unused)), void *ev)
{
    Ecore_Con_Event_Client_Data *e = ev;
    client_data_t *msg = ecore_con_client_data_get(e->client);
    msg->msg = realloc(msg->msg, msg->size + e->size);
    memcpy(msg->msg + msg->size, e->data, e->size);
    msg->size += e->size;
    return 0;
}

void
gm_socket_server_start(Ecore_Evas *ee, const char *name)
{
   main_win = ee;
   server = ecore_con_server_add(ECORE_CON_LOCAL_USER, name, 0, NULL);
   if(!server)
   {
        server = ecore_con_server_connect(ECORE_CON_LOCAL_USER, name, 0, NULL);
        if(!server)
        {
           printf("Can't setup server\n");
           return; /* FIXME: do I need to die here? */
        }
        ecore_con_server_send(server, MSG_ACTIVATE, strlen(MSG_ACTIVATE));
        ecore_con_server_flush(server);
        ecore_con_server_del(server);
        exit(0); /* FIXME */
    }
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, _client_add, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, _client_data, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, _client_del, NULL);
}

void
gm_socket_server_stop()
{
    ecore_con_server_del(server);
}
