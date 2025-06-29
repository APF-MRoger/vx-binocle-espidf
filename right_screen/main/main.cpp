#include <stdio.h>
#include "esp_panel_board_custom_conf.h"
#include "esp_display_panel.hpp"
#include "esp_lib_utils.h"



using namespace esp_panel::drivers;
using namespace esp_panel::board;

static const char *TAG = "main";

extern "C" void app_main()
{
    Board *board = new Board();
    assert(board);

    ESP_LOGI(TAG,"Initializing board");
    ESP_UTILS_CHECK_FALSE_EXIT(board->init(),"Board init failed");
    ESP_UTILS_CHECK_FALSE_EXIT(board->begin(),"Board begin failed");
    auto lcd = board->getLCD();
    auto expander = board->getIO_Expander();
    expander->getBase()->printStatus();
    lcd->colorBarTest();

}