idf_component_register(SRCS "sma_filter.c" "main.cpp" 
                    INCLUDE_DIRS "." "../../common"
                    REQUIRES driver esp_timer twai_daemon binocan)