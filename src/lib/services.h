#ifndef LIBONIONSHARE_SERVICES_H
#define LIBONIONSHARE_SERVICES_H

#include <onionshare.h>
#include "templates.h"
#include "mongoose.h"

#define SEND_SERVICE 0
#define RECEIVE_SERVICE 1
#define CHAT_SERVICE 2
#define WEBSITE_SERVICE 3

#define MAX_CHAT_NAME_LENGTH 127
#define MAX_CHAT_MSG_LENGTH 3000

bool onsh_route_static(struct mg_connection* c, struct mg_http_message* hm);

typedef struct {
    char* url;
} onsh_send_service;

int onsh_create_send_service(onsh_service* service, char* service_id);

typedef struct {
    char* url;
} onsh_receive_service;

int onsh_create_receive_service(onsh_service* service, char* service_id);

typedef struct {
    char* name;
    struct mg_connection* connection;
} chat_user;

typedef struct {
    char* url;
    char* title;
    chat_user* users;
    unsigned int users_size;
    unsigned int user_num;
    unsigned int user_count; // counts up for generating unique usernames
} onsh_chat_service;

int onsh_create_chat_service(onsh_service* service, char* service_id, char* title);

typedef struct {
    char* url;
} onsh_website_service;

int onsh_create_website_service(onsh_service* service, char* service_id);

#endif //LIBONIONSHARE_SERVICES_H
