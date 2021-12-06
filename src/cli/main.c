#include <onionshare.h>

#define SERVICE_TYPE_NONE 0
#define SERVICE_TYPE_SEND 1
#define SERVICE_TYPE_RECEIVE 2
#define SERVICE_TYPE_CHAT 4
#define SERVICE_TYPE_WEBSITE 8
typedef struct {
    bool ok;
    torc_info torc_info;
    int service_type;
    char* service_title;
} onsh_options;
static onsh_options parse_options(int argc, char** argv);
static void print_help();
static void print_banner();

static onionshare onsh;
static void signal_handler(int signo) {
    onsh_stop(&onsh);
}

int main(int argc, char** argv) {
    print_banner();

    // parse options and make sure they are fine
    onsh_options options = parse_options(argc, argv);
    if(!options.ok) return 1;

    // start onionshare
    if(onsh_init(&onsh, options.torc_info) != 0) {
        fprintf(stderr, "Failed to start onionshare, exiting...\n");
        return 1;
    }
    signal(SIGINT, signal_handler); // set signals for graceful shutdown
    signal(SIGTERM, signal_handler);

    // start requested service
    onsh_service service;
    bool started = false;
    if(options.service_type == SERVICE_TYPE_SEND) {
        fprintf(stderr, "Send mode not supported yet, exiting...\n");
        onsh_stop(&onsh);
    } else if(options.service_type == SERVICE_TYPE_RECEIVE) {
        fprintf(stderr, "Receive mode not supported yet, exiting...\n");
        onsh_stop(&onsh);
    } else if(options.service_type == SERVICE_TYPE_CHAT) {
        char* chat_service_id = onsh_start_chat_service(&onsh, &service, options.service_title);
        if(chat_service_id == NULL) {
            fprintf(stderr, "Failed to start chat service, exiting...\n");
            return 1;
        }
        printf("Started chat service at: http://%s.onion\n", chat_service_id);
        started = true;
    } else if(options.service_type == SERVICE_TYPE_WEBSITE) {
        fprintf(stderr, "Website mode not supported yet, exiting...\n");
        onsh_stop(&onsh);
    }

    // run service
    onsh_loop(&onsh);

    // stop service and onionshare when done
    if(started) onsh_stop_service(&onsh, &service);
    onsh_close(&onsh);

    printf("Safely stopped onionshare.\n");

    return 0;
}

static onsh_options parse_options(int argc, char** argv) {
    onsh_options options;
    options.ok = false;
    options.torc_info = torc_default_addr_info();
    options.service_type = SERVICE_TYPE_NONE;
    options.service_title = NULL;

    if(argc <= 1) {
        print_help();
        return options;
    }

    int c;
    opterr = 0;
    while((c = getopt(argc, argv, "hsrcwt:p:a:")) != -1) {
        switch(c) {
            case 's':
                options.service_type |= SERVICE_TYPE_SEND;
                break;
            case 'r':
                options.service_type |= SERVICE_TYPE_RECEIVE;
                break;
            case 'c':
                options.service_type |= SERVICE_TYPE_CHAT;
                break;
            case 'w':
                options.service_type |= SERVICE_TYPE_WEBSITE;
                break;
            case 't':
                options.service_title = optarg;
                break;
            case 'p':
                options.torc_info.port = (int) strtol(optarg, NULL, 10);
                break;
            case 'a':
                options.torc_info.addr = optarg;
                break;
            case 'h':
                print_help();
                return options;
            case '?':
                if(optopt == 't' || optopt == 'p' || optopt == 'a') fprintf(stderr, "Option -%c requires an argument, exiting...\n", optopt);
                else if(isprint(optopt)) fprintf(stderr, "Unknown option `-%c', exiting...\n", optopt);
                else fprintf(stderr,"Unknown option character `\\x%x', exiting...\n", optopt);
                return options;
            default:
                abort();
        }
    }

    if(options.service_type == SERVICE_TYPE_NONE) {
        fprintf(stderr, "No service type chosen, exiting...\n");
        return options;
    }
    if( options.service_type != SERVICE_TYPE_NONE &&
        options.service_type != SERVICE_TYPE_SEND &&
        options.service_type != SERVICE_TYPE_RECEIVE &&
        options.service_type != SERVICE_TYPE_CHAT &&
        options.service_type != SERVICE_TYPE_WEBSITE) {
        fprintf(stderr, "Multiple service types chosen, please choose one, exiting...\n");
        return options;
    }

    options.ok = true;
    return options;
}

static void print_help() {
    printf("Options:\n");
    printf("-s, Send files\n");
    printf("-r, Receive files\n");
    printf("-c, Chat with others\n");
    printf("-w, Host a website\n");
    printf("-t [title], Title of your anonymous service (optional)\n");
    printf("-a [address], Address of tor control port to use (optional)\n");
    printf("-p [port], Port of tor control port to use (optional)\n");
}

static void print_banner() {
    printf("╭───────────────────────────────────────────╮\n");
    printf("│    *            ▄▄█████▄▄            *    │\n");
    printf("│               ▄████▀▀▀████▄     *         │\n");
    printf("│              ▀▀█▀       ▀██▄              │\n");
    printf("│      *      ▄█▄          ▀██▄             │\n");
    printf("│           ▄█████▄         ███        --   │\n");
    printf("│             ███         ▀█████▀           │\n");
    printf("│             ▀██▄          ▀█▀             │\n");
    printf("│         *    ▀██▄       ▄█▄▄     *        │\n");
    printf("│ *             ▀████▄▄▄████▀               │\n");
    printf("│                 ▀▀█████▀▀                 │\n");
    printf("│             --                     *      │\n");
    printf("│   ▄▀▄               ▄▀▀ █                 │\n");
    printf("│   █ █     ▀         ▀▄  █                 │\n");
    printf("│   █ █ █▀▄ █ ▄▀▄ █▀▄  ▀▄ █▀▄ ▄▀▄ █▄▀ ▄█▄   │\n");
    printf("│   ▀▄▀ █ █ █ ▀▄▀ █ █ ▄▄▀ █ █ ▀▄█ █   ▀▄▄   │\n");
    printf("│                                           │\n");
    printf("│            libonionshare v0.1             │\n");
    printf("│           https://git.io/JMb7j            │\n");
    printf("│                                           │\n");
    printf("│          https://onionshare.org/          │\n");
    printf("╰───────────────────────────────────────────╯\n");
}