#include "services.h"

#include "resources.h"

static bool onsh_http_send(struct mg_connection* c, const char* content_type, const unsigned char* data, const unsigned long size) {
    mg_printf(c, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", content_type, size);
    mg_send(c, data, size);
    return true;
}

bool onsh_route_static(struct mg_connection* c, struct mg_http_message* hm) {
    if(mg_http_match_uri(hm, "/css/style.css")) {
        return onsh_http_send(c, "text/css; charset=UTF-8", resource_style_css, resource_style_css_size);
    } else if(mg_http_match_uri(hm, "/js/chat.js")) {
        return onsh_http_send(c, "application/javascript; charset=UTF-8", resource_chat_js, resource_chat_js_size);
    } else if(mg_http_match_uri(hm, "/js/jquery-3.5.1.min.js")) {
        return onsh_http_send(c, "application/javascript; charset=UTF-8", resource_jquery_3_5_1_min_js, resource_jquery_3_5_1_min_js_size);
    } else if(mg_http_match_uri(hm, "/js/receive.js")) {
        return onsh_http_send(c, "application/javascript; charset=UTF-8", resource_receive_js, resource_receive_js_size);
    } else if(mg_http_match_uri(hm, "/js/send.js")) {
        return onsh_http_send(c, "application/javascript; charset=UTF-8", resource_send_js, resource_send_js_size);
    } else if(mg_http_match_uri(hm, "/img/ajax.gif")) {
        return onsh_http_send(c, "image/gif;", resource_ajax_gif, resource_ajax_gif_size);
    } else if(mg_http_match_uri(hm, "/img/logo.png")) {
        return onsh_http_send(c, "image/png;", resource_logo_png, resource_logo_png_size);
    } else if(mg_http_match_uri(hm, "/img/logo_large.png")) {
        return onsh_http_send(c, "image/png;", resource_logo_large_png, resource_logo_large_png_size);
    } else if(mg_http_match_uri(hm, "/img/web_file.png")) {
        return onsh_http_send(c, "image/png;", resource_web_file_png, resource_web_file_png_size);
    } else if(mg_http_match_uri(hm, "/img/web_folder.png")) {
        return onsh_http_send(c, "image/png;", resource_web_folder_png, resource_web_folder_png_size);
    } else return false;
}