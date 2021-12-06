#ifndef LIBONIONSHARE_TEMPLATES_H
#define LIBONIONSHARE_TEMPLATES_H

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

typedef struct {
    char* link;
    char* basename;
    char* size_human;
} template_file_info;

typedef struct {
    char* first;
    char* second;
} template_breadcrumb;

char* template_403_html(char* static_url_path);
char* template_404_html(char* static_url_path);
char* template_405_html(char* static_url_path);
char* template_500_html(char* static_url_path);

char* template_chat(char* static_url_path, char* title/*OPT NULL*/);

char* template_listing(char* static_url_path, template_file_info* dirs, int dirs_size, template_file_info* files, int files_size,
                       template_breadcrumb* breadcrumbs, int breadcrumbs_size, char* breadcrumbs_leaf, char* title/*OPT NULL*/);

char* template_receive(char* static_url_path, bool disable_files, bool disable_text, char* title/*OPT NULL*/);

char* template_send(char* static_url_path, char* filename, char* filesize, bool download_individual_files, template_file_info* dirs, int dirs_size, template_file_info* files, int files_size,
                    template_breadcrumb* breadcrumbs, int breadcrumbs_size, char* breadcrumbs_leaf, char* title/*OPT NULL*/);

char* template_thankyou(char* static_url_path);

#ifdef __cplusplus
}
#endif

#endif //LIBONIONSHARE_TEMPLATES_H
