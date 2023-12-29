#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "esp_clk_tree.h"

extern "C" { void app_main(void); }


#include "discord/core.hpp"
#include "token.h" // define MY_DISCORD_TOKEN as "TOKEN..."

using namespace Lunaris;
using namespace PocketDiscord;

void event_handler(const gateway_events& ev, const pJSON& j)
{

}

void app_main(void)
{
    const ram_info b4{};

    BotBase* bot /*= nullptr;*/= new BotBase();

    //vTaskDelay(pdMS_TO_TICKS(10000));

    /*for(size_t p = 0; p < 10; ++p) {
        ESP_LOGI("MAIN", "================================ ITERATION %zu OF 10 ================================", p + 1);
        bot = new BotBase();
        vTaskDelay(pdMS_TO_TICKS(1000));
        delete bot;
        ESP_LOGI("MAIN", "================================ END OF ITERATION %zu OF 10 ================================", p + 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }*/

    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        auto thebot = bot->make_bot(MY_DISCORD_TOKEN, 
            gateway_intents::GUILDS |
            gateway_intents::GUILD_MESSAGES |
            gateway_intents::GUILD_MEMBERS | 
            gateway_intents::GUILD_MESSAGE_REACTIONS,
            event_handler);

        vTaskDelay(pdMS_TO_TICKS(30000));
    }
    
    delete bot;
    
    const ram_info af{};
        
    ESP_LOGI("MAIN", 
        "Memory usage before:\n"
        "Free: %zu\n"
        "Total: %zu\n"
        "Usage Percentage: %.2f%%",
        b4.mem_free, b4.mem_total, 100.0f - b4.mem_free * 100.0f / b4.mem_total
    );
        
    ESP_LOGI("MAIN", 
        "Memory usage now:\n"
        "Free: %zu\n"
        "Total: %zu\n"
        "Usage Percentage: %.2f%%",
        af.mem_free, af.mem_total, 100.0f - af.mem_free * 100.0f / af.mem_total
    );

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        
        const ram_info af2{};
        
        ESP_LOGI("MAIN", 
            "Memory usage recurrent:\n"
            "Free: %zu\n"
            "Total: %zu\n"
            "Usage Percentage: %.2f%%",
            af2.mem_free, af2.mem_total, 100.0f - af2.mem_free * 100.0f / af2.mem_total
        );
    }
}