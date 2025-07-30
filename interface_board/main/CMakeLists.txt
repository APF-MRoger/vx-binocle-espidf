idf_component_register(SRCS "sma_filter.c" "main.cpp" 
                    INCLUDE_DIRS "." "conf"
                    REQUIRES driver esp_timer)