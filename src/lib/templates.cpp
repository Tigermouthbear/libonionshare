#include "templates.h"

#include <inja.hpp>
#include <cstring>
#include "resources.h"

static char* to_cstr(const std::basic_string<char>& string) {
    char* out = (char*) malloc(string.size() + 1);
    if(out == nullptr) return nullptr;
    strcpy(out, string.data());
    return out;
}

char* template_403_html(const char* static_url_path) {
    nlohmann::json data;
    data["static_url_path"] = static_url_path;
    return to_cstr(inja::render(std::string(reinterpret_cast<const char*>(resource_403_html), resource_403_html_size), data));
}

char* template_404_html(const char* static_url_path) {
    nlohmann::json data;
    data["static_url_path"] = static_url_path;
    return to_cstr(inja::render(std::string(reinterpret_cast<const char*>(resource_404_html), resource_404_html_size), data));
}

char* template_405_html(const char* static_url_path) {
    nlohmann::json data;
    data["static_url_path"] = static_url_path;
    return to_cstr(inja::render(std::string(reinterpret_cast<const char*>(resource_405_html), resource_405_html_size), data));
}

char* template_500_html(const char* static_url_path) {
    nlohmann::json data;
    data["static_url_path"] = static_url_path;
    return to_cstr(inja::render(std::string(reinterpret_cast<const char*>(resource_500_html), resource_500_html_size), data));
}

char* template_chat(const char* static_url_path, const char* title/*OPT NULL*/) {
    nlohmann::json data;
    data["static_url_path"] = static_url_path;
    if(title != nullptr) data["title"] = title;
    return to_cstr(inja::render(std::string(reinterpret_cast<const char*>(resource_chat_html), resource_chat_html_size), data));
}

static void add_files_to_valuemap(nlohmann::json* data, const char* key, template_file_info* files, int files_size) {
    nlohmann::json::array_t list = nlohmann::json::array();
    for(int i = 0; i < files_size; i++) {
        template_file_info file_info = files[i];
        nlohmann::json value;
        value["link"] = file_info.link;
        value["basename"] = file_info.basename;
        if(file_info.size_human != nullptr) value["size_human"] = file_info.size_human;
        list.push_back(value);
    }
    (*data)[key] = list;
}

static void add_breadcrumbs_to_valuemap(nlohmann::json* data, const char* key, template_breadcrumb* breadcrumbs, int breadcrumbs_size) {
    nlohmann::json::array_t list = nlohmann::json::array();
    for(int i = 0; i < breadcrumbs_size; i++) {
        template_breadcrumb breadcrumb = breadcrumbs[i];
        list.push_back({ breadcrumb.first, breadcrumb.second });
    }
    (*data)[key] = list;
}

char* template_listing(const char* static_url_path, template_file_info* dirs, int dirs_size, template_file_info* files, int files_size,
                       template_breadcrumb* breadcrumbs, int breadcrumbs_size, const char* breadcrumbs_leaf, const char* title/*OPT NULL*/) {
    nlohmann::json data;
    data["static_url_path"] = static_url_path;
    data["breadcrumbs_leaf"] = breadcrumbs_leaf;
    add_files_to_valuemap(&data, "dirs", dirs, dirs_size);
    add_files_to_valuemap(&data, "files", files, files_size);
    add_breadcrumbs_to_valuemap(&data, "breadcrumbs", breadcrumbs, breadcrumbs_size);
    if(title != nullptr) data["title"] = title;
    return to_cstr(inja::render(std::string(reinterpret_cast<const char*>(resource_listing_html), resource_listing_html_size), data));
}

char* template_receive(const char* static_url_path, bool disable_files, bool disable_text, const char* title/*OPT NULL*/) {
    nlohmann::json data;
    data["static_url_path"] = static_url_path;
    data["disable_files"] = disable_files;
    data["disable_text"] = disable_text;
    if(title != nullptr) data["title"] = title;
    return to_cstr(inja::render(std::string(reinterpret_cast<const char*>(resource_receive_html), resource_receive_html_size), data));
}

char* template_send(const char* static_url_path, const char* filename, const char* filesize, const char* filesize_human, bool download_individual_files,
                    bool is_zipped, template_file_info* dirs, int dirs_size, template_file_info* files, int files_size,
                    template_breadcrumb* breadcrumbs, int breadcrumbs_size, const char* breadcrumbs_leaf, const char* title/*OPT NULL*/) {
    nlohmann::json data;
    data["static_url_path"] = static_url_path;
    data["filename"] = filename;
    data["filesize"] = filesize;
    data["filesize_human"] = filesize_human;
    data["is_zipped"] = is_zipped;
    data["download_individual_files"] = download_individual_files;
    data["breadcrumbs_leaf"] = breadcrumbs_leaf;
    add_files_to_valuemap(&data, "dirs", dirs, dirs_size);
    add_files_to_valuemap(&data, "files", files, files_size);
    add_breadcrumbs_to_valuemap(&data, "breadcrumbs", breadcrumbs, breadcrumbs_size);
    if(title != nullptr) data["title"] = title;
    return to_cstr(inja::render(std::string(reinterpret_cast<const char*>(resource_send_html), resource_send_html_size), data));
}

char* template_thankyou(char* static_url_path) {
    nlohmann::json data;
    data["static_url_path"] = static_url_path;
    return to_cstr(inja::render(std::string(reinterpret_cast<const char*>(resource_thankyou_html), resource_thankyou_html_size), data));
}
