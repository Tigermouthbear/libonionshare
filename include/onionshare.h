#ifndef LIBONIONSHARE_ONIONSHARE_H
#define LIBONIONSHARE_ONIONSHARE_H

#include <stdbool.h>
#include <torc.h>
#include "../src/lib/mongoose.h"

typedef struct {
    bool running;
    torc controller;
    struct mg_mgr mgr;
} onionshare;

typedef struct {
    char* service_id;
    bool alive;
    int type;
    mg_event_handler_t routing;
    void* fn_data;
    struct mg_connection* mg_conn;
} onsh_service;

int onsh_init(onionshare* onionshare, torc_info torc_info);
void onsh_loop(onionshare* onionshare);
void onsh_stop(onionshare* onionshare);
void onsh_close(onionshare* onionshare);

void onsh_stop_service(onionshare* onionshare, onsh_service* service);

// returns service id
char* onsh_start_chat_service(onionshare* onionshare, onsh_service* service, char* title/*OPT NULL*/);

#endif //LIBONIONSHARE_ONIONSHARE_H
