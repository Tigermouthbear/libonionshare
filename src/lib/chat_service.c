#include "services.h"

static int send_chat_msg(onsh_chat_service* chat_service, char* msg) {
    for(int i = 0; i < chat_service->user_num; i++) {
        chat_user user = chat_service->users[i];
        mg_ws_send(user.connection, msg, strlen(msg), WEBSOCKET_OP_TEXT);
    }
    return 0;
}

static char* get_chat_name(onsh_chat_service* chat_service, struct mg_connection* connection) {
    for(int i = 0; i < chat_service->user_num; i++) {
        chat_user user = chat_service->users[i];
        if(user.connection == connection) return user.name;
    }
    return NULL;
}

static bool is_chat_name_available(onsh_chat_service* chat_service, char* name) {
    for(int i = 0; i < chat_service->user_num; i++) {
        chat_user user = chat_service->users[i];
        if(strcmp(user.name, name) == 0) return false;
    }
    return true;
}

#define STATUS_MSG_JOINED 0
#define STATUS_MSG_LEFT 1
#define STATUS_MSG_NAME_UPDATE 2
static void send_chat_status_msg(onsh_chat_service* chat_service, int type, char* name, char* prev_name/*OPT NULL*/) {
    // allocate enough memory for all users and msg
    size_t size = strlen(name) + MAX_CHAT_NAME_LENGTH + (MAX_CHAT_NAME_LENGTH + 3) * chat_service->user_num + 256;
    char* buffer = calloc(size, sizeof(char));
    if(buffer == NULL) {
        perror("[ONIONSHARE] FAILED TO SEND CHAT STATUS MESSAGE");
        return;
    }
    char* curr = buffer;

    // set different message for each update type
    if(type == STATUS_MSG_JOINED) {
        curr += snprintf(buffer, size, "{\"event\":\"status\",\"msg\":\"%s has joined.\",\"connected_users\":[", name);
    } else if(type == STATUS_MSG_LEFT) {
        curr += snprintf(buffer, size, "{\"event\":\"status\",\"msg\":\"%s has left the room.\",\"connected_users\":[", name);
    } else if(type == STATUS_MSG_NAME_UPDATE) {
        curr += snprintf(buffer, size, "{\"event\":\"status\",\"msg\":\"%s has changed their name to %s.\",\"connected_users\":[", prev_name, name);
    }

    // add connected user list
    for(int i = 0; i < chat_service->user_num; i++) {
        // get users name
        char* user_name = chat_service->users[i].name;
        size_t name_len = strlen(user_name);

        // write name and optional comma
        *(curr++) = '\"';
        memcpy(curr, user_name, name_len);
        curr += name_len;
        *(curr++) = '\"';
        if(i != chat_service->user_num - 1) *(curr++) = ',';
    }

    // add json ending
    *(curr++) = ']';
    *(curr++) = '}';
    *curr = 0;

    send_chat_msg(chat_service, buffer);
    free(buffer);
}

static void set_chat_name(onsh_chat_service* chat_service, struct mg_connection* connection, char* name) {
    for(int i = 0; i < chat_service->user_num; i++) {
        chat_user user = chat_service->users[i];
        if(user.connection == connection) {
            char prev[MAX_CHAT_NAME_LENGTH + 1];
            strcpy(prev, user.name);
            strcpy(user.name, name);
            send_chat_status_msg(chat_service, STATUS_MSG_NAME_UPDATE, name, prev);
            break;
        }
    }
}

static void chat_routing(struct mg_connection* c, int event, void* ev_data, void* fn_data) {
    onsh_chat_service* chat_service = fn_data;
    if(event == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*) ev_data;
        if(!onsh_route_static(c, hm)) {
            if(mg_http_match_uri(hm, "/chat")) {
                mg_ws_upgrade(c, hm, NULL);
            } else if(mg_http_match_uri(hm, "/")) {
                mg_http_reply(c, 200, "Content-Type: text/html; charset=UTF-8\r\n", template_chat(chat_service->url, chat_service->title));
            } else {
                mg_http_reply(c, 404, "Content-Type: text/html; charset=UTF-8\r\n", template_404_html(chat_service->url));
            }
        }
    } else if(event == MG_EV_WS_OPEN) {
        // make sure connection pool is large enough
        if(chat_service->user_num >= chat_service->users_size) {
            size_t size = chat_service->users_size * 2;
            chat_service->users = realloc(chat_service->users, size * sizeof(onsh_chat_service*));
            if(chat_service->users == NULL) {
                perror("[ONIONSHARE] FAILED TO REALLOCATE CHAT CONNECTION POOL");
                return;
            }
            chat_service->users_size = size;
        }

        // add user to pool with unique name
        size_t max_len = MAX_CHAT_NAME_LENGTH + 1;
        char* name = malloc(max_len);
        if(name == NULL) {
            perror("[ONIONSHARE] FAILED TO ALLOCATE MEM FOR CHAT USER");
            return;
        }
        do {
            snprintf(name, max_len, "anonymous#%d", chat_service->user_count++);
        } while(!is_chat_name_available(chat_service, name));
        chat_user user = { name, c };
        chat_service->users[chat_service->user_num++] = user;

        // assign the user its name
        max_len = strlen(name) + 50; // 50 extra should be enough for json
        char frame[max_len];
        int len = snprintf(frame, max_len, "{\"event\":\"username\",\"username\":\"%s\"}", name);
        mg_ws_send(c, frame, len, WEBSOCKET_OP_TEXT);

        // tell users new person joined
        send_chat_status_msg(chat_service, STATUS_MSG_JOINED, user.name, NULL);
    } else if(event == MG_EV_WS_MSG) {
        // on frame text
        struct mg_ws_message* wm = (struct mg_ws_message*) ev_data;
        if((wm->flags & WEBSOCKET_OP_TEXT) == WEBSOCKET_OP_TEXT) {
            // find delimiter
            int i = 0;
            char ch = wm->data.ptr[i];
            while(ch != ':' && i < wm->data.len) ch = wm->data.ptr[++i];
            if(ch == ':' && i > 0) {
                // read chat event
                char* chat_event = malloc(i + 1);
                if(chat_event == NULL) {
                    perror("[ONIONSHARE] FAILED TO READ CHAT EVENT");
                    return;
                }
                memcpy(chat_event, wm->data.ptr, i);
                chat_event[i] = 0;

                // read message
                size_t msg_len = wm->data.len - i - 1;
                if(msg_len != 0) {
                    char* chat_msg = malloc(msg_len + 1);
                    if(chat_msg == NULL) {
                        free(chat_event);
                        perror("[ONIONSHARE] FAILED TO READ CHAT MESSAGE");
                        return;
                    }

                    memcpy(chat_msg, wm->data.ptr + i + 1, msg_len);
                    chat_msg[msg_len] = 0;

                    // chat message logic here
                    if(strcmp(chat_event, "text") == 0 && msg_len < MAX_CHAT_MSG_LENGTH) {
                        char* username = get_chat_name(chat_service, c);
                        if(username != NULL) {
                            size_t max_len = msg_len + strlen(username) + 50; // 50 extra should be enough for json
                            char frame[max_len];
                            snprintf(frame, max_len, "{\"event\":\"message\",\"username\":\"%s\",\"msg\":\"%s\"}", username, chat_msg);
                            send_chat_msg(chat_service, frame);
                        }
                    } else if(strcmp(chat_event, "update_username") == 0) {
                        // update username if no one else is using it
                        // set back to the previous username if someone else is using it
                        if(msg_len <= MAX_CHAT_NAME_LENGTH && is_chat_name_available(chat_service, chat_msg)) {
                            set_chat_name(chat_service, c, chat_msg);
                        } else {
                            // tell the client to set back to previous name
                            char* username = get_chat_name(chat_service, c);
                            size_t max_len = strlen(username) + 50; // 50 extra should be enough for json
                            char frame[max_len];
                            int len = snprintf(frame, max_len, "{\"event\":\"username\",\"username\":\"%s\"}", username);
                            mg_ws_send(c, frame, len, WEBSOCKET_OP_TEXT);
                        }
                    }

                    free(chat_event);
                    free(chat_msg);
                } else {
                    // if we are here, there is no chat message body
                    // all commands have a body so this isnt used
                    free(chat_event);
                }
            }
        }
    } else if(event == MG_EV_CLOSE) {
        char left_name[MAX_CHAT_NAME_LENGTH + 1];

        // remove chat user from connection list
        int index = -1;
        for(int i = 0; i < chat_service->user_num; i++) {
            chat_user user = chat_service->users[i];
            if(user.connection == c) {
                index = i;
                strcpy(left_name, user.name);
                free(user.name);
                break;
            }
        }
        if(index != -1) {
            for(int i = index; i < chat_service->user_num - 1; i++) chat_service->users[i] = chat_service->users[i + 1];
            chat_service->user_num--;
            send_chat_status_msg(chat_service, STATUS_MSG_LEFT, left_name, NULL);
        }
    }
}

int onsh_create_chat_service(onsh_service* service, char* service_id, char* title) {
    service->type = CHAT_SERVICE;
    service->alive = false;

    // create chat service object
    service->fn_data = malloc(sizeof(onsh_chat_service));
    if(service->fn_data == NULL) {
        perror("[ONIONSHARE] FAILED TO CREATE CHAT SERVICE BUFFER");
        return 1;
    }
    onsh_chat_service* chat_service = service->fn_data;
    service->service_id = service_id;
    chat_service->title = title;
    chat_service->user_count = 0;
    chat_service->url = malloc(strlen(service_id) + 14);
    if(chat_service->url == NULL) {
        free(service->fn_data);
        perror("[ONIONSHARE FAILED TO CREATE CHAT SERVICE URL]");
        return 1;
    }
    strcpy(chat_service->url, "http://");
    strcat(chat_service->url, service_id);
    strcat(chat_service->url, ".onion");

    // set routing function
    service->routing = chat_routing;
    service->mg_conn = NULL;

    // create connection list
    chat_service->users_size = 8;
    chat_service->user_num = 0;
    chat_service->users = malloc(chat_service->users_size * sizeof(chat_user));
    if(chat_service->users == NULL) {
        free(service->fn_data);
        free(chat_service->url);
        perror("[ONIONSHARE] FAILED TO CRATE CHAT CONNECTION POOL");
        return 1;
    }

    return 0;
}