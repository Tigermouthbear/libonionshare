#include "templates.h"

#include <jinja2cpp/template.h>
#include <cstring>
#include "resources.h"

static char* to_cstr(jinja2::Result<std::string> rendered) {
    char* out = (char*) malloc(rendered->size() + 1);
    if(out == nullptr) return nullptr;
    strcpy(out, rendered->data());
    return out;
}

char* template_403_html(char* static_url_path) {
    jinja2::Template tpl;
    tpl.Load(std::string(reinterpret_cast<const char*>(resource_403_html), resource_403_html_size));
    return to_cstr(tpl.RenderAsString({
        {"static_url_path", std::string(static_url_path)}
    }));
}

char* template_404_html(char* static_url_path) {
    jinja2::Template tpl;
    tpl.Load(std::string(reinterpret_cast<const char*>(resource_404_html), resource_404_html_size));
    return to_cstr(tpl.RenderAsString({
        {"static_url_path", std::string(static_url_path)}
    }));
}

char* template_405_html(char* static_url_path) {
    jinja2::Template tpl;
    tpl.Load(std::string(reinterpret_cast<const char*>(resource_405_html), resource_405_html_size));
    return to_cstr(tpl.RenderAsString({
        {"static_url_path", std::string(static_url_path)}
    }));
}

char* template_500_html(char* static_url_path) {
    jinja2::Template tpl;
    tpl.Load(std::string(reinterpret_cast<const char*>(resource_500_html), resource_500_html_size));
    return to_cstr(tpl.RenderAsString({
        {"static_url_path", std::string(static_url_path)}
    }));
}

char* template_chat(char* static_url_path, char* title/*OPT NULL*/){
    jinja2::Template tpl;
    tpl.Load(std::string(reinterpret_cast<const char*>(resource_chat_html), resource_chat_html_size));
    jinja2::ValuesMap params = {
            {"static_url_path", std::string(static_url_path)}
    };
    if(title != nullptr) params.insert({"title", std::string(title)});
    return to_cstr(tpl.RenderAsString(params));
}

static void add_files_to_valuemap(jinja2::ValuesMap* valuesMap, const char* key, template_file_info* files, int files_size) {
    jinja2::ValuesList list;
    for(int i = 0; i < files_size; i++) {
        template_file_info file_info = files[i];
        jinja2::ValuesMap value = {
            {"link", std::string(file_info.link)},
            {"basename", std::string(file_info.basename)}
        };
        if(file_info.size_human != nullptr) value.insert({"size_human", std::string(file_info.size_human)});
        list.push_back(value);
    }
    valuesMap->insert({std::string(key), list});
}

static void add_breadcrumbs_to_valuemap(jinja2::ValuesMap* valuesMap, const char* key, template_breadcrumb* breadcrumbs, int breadcrumbs_size) {
    jinja2::ValuesList list;
    for(int i = 0; i < breadcrumbs_size; i++) {
        template_breadcrumb breadcrumb = breadcrumbs[i];
        list.push_back(jinja2::ValuesList {
            std::string(breadcrumb.first),
            std::string(breadcrumb.second)
        });
    }
    valuesMap->insert({std::string(key), list});
}

char* template_listing(char* static_url_path, template_file_info* dirs, int dirs_size, template_file_info* files, int files_size,
                       template_breadcrumb* breadcrumbs, int breadcrumbs_size, char* breadcrumbs_leaf, char* title/*OPT NULL*/) {
    jinja2::Template tpl;
    tpl.Load(std::string(reinterpret_cast<const char*>(resource_listing_html), resource_listing_html_size));
    jinja2::ValuesMap params = {
            {"static_url_path", std::string(static_url_path)},
            {"breadcrumbs_leaf", std::string(breadcrumbs_leaf)}
    };
    add_files_to_valuemap(&params, "dirs", dirs, dirs_size);
    add_files_to_valuemap(&params, "files", files, files_size);
    add_breadcrumbs_to_valuemap(&params, "breadcrumbs", breadcrumbs, breadcrumbs_size);
    if(title != nullptr) params.insert({"title", std::string(title)});
    return to_cstr(tpl.RenderAsString(params));
}

char* template_receive(char* static_url_path, bool disable_files, bool disable_text, char* title/*OPT NULL*/) {
    jinja2::Template tpl;
    tpl.Load(std::string(reinterpret_cast<const char*>(resource_receive_html), resource_receive_html_size));
    jinja2::ValuesMap params = {
            {"static_url_path", std::string(static_url_path)},
            {"disable_files", disable_files},
            {"disable_text", disable_text},
    };
    if(title != nullptr) params.insert({"title", std::string(title)});
    return to_cstr(tpl.RenderAsString(params));
}

char* template_send(char* static_url_path, char* filename, char* filesize, char* filesize_human, bool download_individual_files, bool is_zipped, template_file_info* dirs, int dirs_size, template_file_info* files, int files_size,
                    template_breadcrumb* breadcrumbs, int breadcrumbs_size, char* breadcrumbs_leaf, char* title/*OPT NULL*/) {
    jinja2::Template tpl;
    tpl.Load(std::string(reinterpret_cast<const char*>(resource_send_html), resource_send_html_size));
    jinja2::ValuesMap params = {
            {"static_url_path", std::string(static_url_path)},
            {"filename", std::string(filename)},
            {"filesize", std::string(filesize)},
            {"filesize_human", std::string(filesize_human)},
            {"is_zipped", is_zipped},
            {"download_individual_files", download_individual_files},
            {"breadcrumbs_leaf", std::string(breadcrumbs_leaf)}
    };
    add_files_to_valuemap(&params, "dirs", dirs, dirs_size);
    add_files_to_valuemap(&params, "files", files, files_size);
    add_breadcrumbs_to_valuemap(&params, "breadcrumbs", breadcrumbs, breadcrumbs_size);
    if(title != nullptr) params.insert({"title", std::string(title)});
    return to_cstr(tpl.RenderAsString(params));
}

char* template_thankyou(char* static_url_path) {
    jinja2::Template tpl;
    tpl.Load(std::string(reinterpret_cast<const char*>(resource_thankyou_html), resource_thankyou_html_size));
    return to_cstr(tpl.RenderAsString({
        {"static_url_path", std::string(static_url_path)}
    }));
}
