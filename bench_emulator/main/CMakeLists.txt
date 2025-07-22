idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "." "conf"
                    REQUIRES driver esp_timer esp_wifi esp_http_server nvs_flash)