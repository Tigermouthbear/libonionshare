#include <onionshare.h>

static onionshare onsh;

static void signal_handler(int signo) {
    onsh_stop(&onsh);
}

int main() {
    if(onsh_init(&onsh, torc_default_addr_info()) != 0) {
        printf("FAILED TO INITIALIZE ONIONSHARE");
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    onsh_service service;
    char* chat_service_id = onsh_start_chat_service(&onsh, &service, "Tiger's OnionShare Chat");
    if(chat_service_id == NULL) {
        printf("FAILED TO START CHAT SERVICE");
        return 1;
    }
    printf("\nSTARTED CHAT SERVICE AT http://%s.onion\n\n", chat_service_id);

    onsh_loop(&onsh);
    onsh_stop_service(&onsh, &service);
    onsh_close(&onsh);

    return 0;
}