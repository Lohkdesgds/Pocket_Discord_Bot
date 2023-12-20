#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "esp_clk_tree.h"

extern "C" { void app_main(void); }

const uint64_t blk = ((uint64_t)1) << 8;
const uint64_t maxx = ((((uint64_t)1) << 31) / blk);
const char ch[blk] = {'0'};

#include "discord/core.hpp"

uint32_t get_clk()
{
    uint32_t val;
    esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_CPU, ESP_CLK_TREE_SRC_FREQ_PRECISION_APPROX, &val);
    return val;
}


void app_main(void)
{
    auto* bot = new Lunaris::PocketDiscord::Bot();
    ESP_LOGI("CORE", "CLOCK: %lu", get_clk());

    vTaskDelay(pdMS_TO_TICKS(10000));
    //ESP_LOGI("CORE", "CLOCK: %lu", get_clk());

    delete bot;
    ESP_LOGI("DONE", "\nDONE\n");
    
    

    //FILE* fp = fopen("/sdcard/teste.txt", "wb+");
    //if (!fp) ESP_LOGE("MAIN", "CANNOT OPEN FILE!");
    //else {
    //    ESP_LOGI("MAIN", "Writing a bunch of data");
//
    //    const auto b4 = esp_timer_get_time();
//
    //    for(uint64_t p = 0; p < maxx; ++p) {
    //        if (p % 2048 == 0) ESP_LOGI("MAIN", "%.5f%c...", (100.0f * p / maxx),'%');
    //        //fprintf(fp, "%lu\n", p);
    //        fwrite(ch, sizeof(char), blk, fp);
    //        fflush(fp);
    //    }
//
    //    const auto af = esp_timer_get_time();
//
    //    ESP_LOGI("MAIN", "Good, time spent: %lli us", (af - b4));
    //    fclose(fp);
    //}
    //fp = nullptr;
    //
    //unmount_card();

    while(1) {
        vTaskDelay(2000);
        //ESP_LOGI("CORE", "CLOCK: %lu", get_clk());
        //printf("Hello world\n");
    }
}