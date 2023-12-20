idf_component_register(
    SRCS 
        src/main.cpp
    INCLUDE_DIRS "."
    #REQUIRES
    #  json esp_websocket_client esp_http_client
    PRIV_REQUIRES
      esp_wifi nvs_flash fatfs esp_timer
)

component_compile_options(-std=gnu++17)