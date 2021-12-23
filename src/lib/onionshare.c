#include <onionshare.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <torcmds.h>
#include "templates.h"
#include "services.h"

int onsh_init(onionshare* onionshare, torc_info torc_info) {
    // connect tor controller
    if(torc_connect_controller(&onionshare->controller, torc_info) != 0) {
        ONSH_LOG_ERROR("FAILED TO CONNECT TO TOR CONTROL PORT");
        return 1;
    }

    // authenticate tor controller
    if(!torc_authenticate(&onionshare->controller, NULL)) { // TODO: OPTIONAL PASSWORD AUTH
        ONSH_LOG_ERROR("FAILED TO AUTHENTICATE WITH TOR CONTROL PORT");
        torc_close_controller(&onionshare->controller);
        return 1;
    }

    // create response lock
    if(pthread_mutex_init(&onionshare->lock, NULL) != 0) {
        TORC_LOG_ERROR("FAILED TO INITIALIZE ONIONSHARE MUTEX LOCK");
        return 1;
    }

    // initialize mongoose webserver
    mg_mgr_init(&onionshare->mgr);

    onionshare->running = true;
    return 0;
}

void onsh_loop(onionshare* onionshare) {
    onionshare->async = false;
    bool running = true;
    while(running) {
        mg_mgr_poll(&onionshare->mgr, 1000);
        pthread_mutex_lock(&onionshare->lock);
        running = onionshare->running;
        pthread_mutex_unlock(&onionshare->lock);
    }
}

static void* async_loop(void* onsh) {
    onsh_loop((onionshare*) onsh);
    return 0;
}

void onsh_loop_async(onionshare* onionshare) {
    onionshare->async = true;
    pthread_create(&onionshare->async_thread, NULL, async_loop, onionshare);
}

void onsh_stop(onionshare* onionshare) {
    pthread_mutex_lock(&onionshare->lock);
    onionshare->running = false;
    pthread_mutex_unlock(&onionshare->lock);
    if(onionshare->async) pthread_join(onionshare->async_thread, NULL);
}

void onsh_close(onionshare* onionshare) {
    mg_mgr_free(&onionshare->mgr);
    torc_close_controller(&onionshare->controller);
}

// starts mongoose ws for service
static int onsh_start_mg_service(onsh_service* service, struct mg_mgr* mgr, int port) {
    const char* addr_prefix = "http://localhost:%d";
    size_t addr_size = strlen(addr_prefix) + 4; // +3 for remaining 3 port chars and +1 for null ending
    char addr[addr_size];
    snprintf(addr, addr_size, addr_prefix, port);

    service->mg_conn = mg_http_listen(mgr, addr, service->routing, service->fn_data);
    if(service->mg_conn == NULL) {
        ONSH_LOG_ERROR("FAILED TO CREATE LISTENER FOR ONIONSHARE SERVICE");
        return 1;
    }

    service->alive = true;

    return 0;
}

void onsh_stop_service(onionshare* onionshare, onsh_service* service) {
    // close mongoose server
    if(service->mg_conn != NULL) service->mg_conn->is_closing = true;

    // delete onion from control connection
    torc_command command;
    if(!torc_del_onion(&onionshare->controller, &command, service->service_id)) {
        // failed to delete onion, it will be closed when the controller is stopped
        ONSH_LOG_ERROR("FAILED TO DELETE HIDDEN SERVICE");
    }

    // free memory
    if(service->type == SEND_SERVICE) {
        onsh_send_service* send_service = service->fn_data;
        free(send_service->url);
    } else if(service->type == RECEIVE_SERVICE) {
        onsh_receive_service* receive_service = service->fn_data;
        free(receive_service->url);
    } else if(service->type == CHAT_SERVICE) {
        onsh_chat_service* chat_service = service->fn_data;

        free(chat_service->url);

        // free users list
        for(int i = 0; i < chat_service->user_num; i++) {
            chat_user user = chat_service->users[i];
            free(user.name);
        }
        free(chat_service->users);
    } else if(service->type == WEBSITE_SERVICE) {
        onsh_website_service* website_service = service->fn_data;
        free(website_service->url);
    }
    free(service->fn_data);
    free(service->service_id);
}

// return -1 on fail
static int get_available_port() {
    // create socket for finding open port
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        ONSH_LOG_ERROR("FAILED TO CREATE SOCKET FOR FINDING OPEN PORT");
        return -1;
    }

    // bind with sin_port to 0, to find an open port
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = 0; // setting to 0 finds open port
    if(bind(sock, (struct sockaddr*) &servaddr, sizeof(servaddr)) != 0) {
        ONSH_LOG_ERROR("FAILED TO BIND SOCKET FOR FINDING OPEN PORT");
        return -1;
    }

    // read found port
    socklen_t len = sizeof(servaddr);
    if(getsockname(sock, (struct sockaddr*)&servaddr, &len) == -1) {
        ONSH_LOG_ERROR("FAILED TO READ SOCKET ADDR FOR FINDING OPEN PORT");
        return -1;
    }

    close(sock);

    return servaddr.sin_port;
}

static char* onsh_start_onion_service(onionshare* onionshare, int port) {
    if(port > 65535 || port <= 0) {
        ONSH_LOG_ERROR("INVALID PORT FOR ONION SERVICE");
        return NULL;
    }

    // create new hidden service on random local port
    char port_pair[9]; // 80,XXXXX\0 (9 characters)
    snprintf(port_pair, 9, "80,%d", port);
    torc_command command; // TODO: CLIENT AUTH
    torc_add_onion_response response = torc_add_new_onion(&onionshare->controller, &command, port_pair, TORC_FLAGS_DISCARD_PK, 0);
    if(!response.sent || !command.response.ok || response.service_id == NULL) {
        ONSH_LOG_ERROR("FAILED TO ADD ONION FOR CHAT SERVICE");
        return NULL;
    }

    // copy service id
    char* service_id = malloc(strlen(response.service_id) + 1);
    strcpy(service_id, response.service_id);
    torc_free_command(&command);
    return service_id;
}

char* onsh_start_chat_service(onionshare* onionshare, onsh_service* service, char* title) {
    int port = get_available_port();
    if(port <= 0) return NULL;

    // create and start onionshare service
    char* service_id = onsh_start_onion_service(onionshare, port);
    if(service_id == NULL) return NULL;
    if(onsh_create_chat_service(service, service_id, title) != 0) {
        free(service_id);
        ONSH_LOG_ERROR("FAILED TO CREATE CHAT SERVICE");
        return NULL;
    }
    if(onsh_start_mg_service(service, &onionshare->mgr, port) != 0) {
        onsh_stop_service(onionshare, service); // stop frees service_id and everything
        ONSH_LOG_ERROR("FAILED TO START CHAT SERVICE");
        return NULL;
    }
    return service_id;
}
