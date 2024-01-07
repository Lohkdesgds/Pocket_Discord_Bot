#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "esp_clk_tree.h"

extern "C" { void app_main(void); }


#include "discord/core.hpp"
#include "defaults.h"

using namespace Lunaris;
using namespace PocketDiscord;


void check_ram_notify();
void event_handler(const gateway_events& t, const JSON& j, HTTPS* h, GatewayBot gb);

static void debug_memory_usage_total(void* param)
{
    while(1) {
        check_ram_notify();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void)
{
    xTaskCreate(debug_memory_usage_total, "MEMTRACKD", 2560, nullptr, tskIDLE_PRIORITY, nullptr);

    vTaskDelay(pdMS_TO_TICKS(5000));

    BotBase* bot = new BotBase();

    vTaskDelay(pdMS_TO_TICKS(2000));

    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        HeapString token_dynamic;        
        READFILE_FULLY_TO(token_dynamic, "token.txt", "Could not load token certificate.");

        auto thebot = bot->make_bot(token_dynamic.c_str(), 
            //gateway_intents::GUILDS |
            gateway_intents::GUILD_MESSAGES |
            gateway_intents::GUILD_MEMBERS | 
            gateway_intents::GUILD_MESSAGE_REACTIONS,
            event_handler);

        vTaskDelay(pdMS_TO_TICKS(30000));


//        ESP_LOGI("MAIN", "Trying get message on channel idk 6 times.");
//        
//        // 5 + 1
//        for(size_t a = 0; a < 5; ++a) thebot.https()->request(http_request::GET, "/channels/739230609527930940/messages/1192343873452769403");
//        auto json = thebot.https()->request(http_request::GET, "/channels/739230609527930940/messages/1192343873452769403");
//  
//        ESP_LOGI("MAIN", "Got status=%i content:", thebot.https()->get_status());
//        json.print([](char c){ putchar(c); });
//        
//        vTaskDelay(pdMS_TO_TICKS(20000));
//
//        ESP_LOGI("MAIN", "Testing restart of gateway...");
//
//        thebot.gateway()->stop();
//        thebot.gateway()->start();
//        
//        ESP_LOGI("MAIN", "Gateway restart ended.");
//
//        vTaskDelay(pdMS_TO_TICKS(60000));
//
//        ESP_LOGI("MAIN", "Last GET.");
//        thebot.https()->request(http_request::GET, "/channels/739230609527930940/messages/1192343873452769403");        
//        ESP_LOGI("MAIN", "Last GET ended.");
    }

    delete bot;

    vTaskDelete(NULL);
}

void event_handler(const gateway_events& t, const JSON& j, HTTPS* h, GatewayBot gb)
{
    static const char* TAG = "EVHLR";


    ESP_LOGI(TAG, "Got event %li at core %i in MAIN!",
        static_cast<int32_t>(t), (int)xPortGetCoreID()
    );

    if (!h) {
        ESP_LOGI(TAG, "HTTPS was not set yet.");
        return;
    }

    switch(t) {
    case gateway_events::MESSAGE_CREATE:
        {
            if (!j["author"]["bot"].get_bool()) {
                const uint64_t m_mid = j["id"].get_uint();
                const uint64_t m_cid = j["channel_id"].get_uint();
                const uint64_t m_gid = j["guild_id"].get_uint();
                const char* m_content = j["content"].get_string();

                ESP_LOGI(TAG, "Message ev: %s", m_content);
                gb.update_presence("This is the name", "This is the state", 0);

                if (strncmp(m_content, "is the bot on?", strlen(m_content)) == 0) {
                    
                    char* buf = nullptr, *bufpost = nullptr;
                    size_t len = 0, len_post = 0;

                    saprintf(buf, &len,
                        "{\"content\": \"Yes, I am alive!\",\"message_reference\":{\"message_id\":%llu,\"guild_id\":%llu,\"fail_if_not_exists\":false}}",
                        m_mid, m_gid
                    );
                    saprintf(bufpost, &len_post,
                        "/channels/%llu/messages",
                        m_cid
                    );

                    h->request(http_request::POST, bufpost, buf, len);

                    delete[] buf;
                    delete[] bufpost;
                }
            }
        }
        break;
    default:
        break;    
    }
}


void check_ram_notify()
{
    const ram_info nw{};

    static float smem1 = 0.0f, smem2 = 0.0f, smem3 = 0.0f;

    const float mem1 = (100.0f - (nw.mem_free   * 100.0f / nw.mem_total));
    const float mem2 = (100.0f - (nw.memex_free * 100.0f / nw.memex_total));
    const float mem3 = (100.0f - (nw.mem32_free * 100.0f / nw.mem32_total));

    if (mem1 >= 70.0f && smem1 < 70.0f) ESP_LOGW("RMDBG", "Moderate MEM usage: %.3f%%", mem1);
    else if (mem1 >= 80.0f && smem1 < 80.0f) ESP_LOGW("RMDBG", "High MEM usage: %.3f%%", mem1);
    else if (mem1 >= 90.0f && smem1 < 90.0f) ESP_LOGE("RMDBG", "Extremely high MEM usage: %.3f%%", mem1);
    else if (smem1 > 70.0f && mem1 <= 70.0f) ESP_LOGW("RMDBG", "MEM usage going back to normal: %.3f%%", mem1);
    
    if (mem2 >= 70.0f && smem2 < 70.0f) ESP_LOGW("RMDBG", "Moderate MEMEX usage: %.3f%%", mem2);
    else if (mem2 >= 80.0f && smem2 < 80.0f) ESP_LOGW("RMDBG", "High MEMEX usage: %.3f%%", mem2);
    else if (mem2 >= 90.0f && smem2 < 90.0f) ESP_LOGE("RMDBG", "Extremely high MEMEX usage: %.3f%%", mem2);
    else if (smem2 > 70.0f && mem2 <= 70.0f) ESP_LOGW("RMDBG", "MEMEX usage going back to normal: %.3f%%", mem2);
    
    if (mem3 >= 70.0f && smem3 < 70.0f) ESP_LOGW("RMDBG", "Moderate MEM32 usage: %.3f%%", mem3);
    else if (mem3 >= 80.0f && smem3 < 80.0f) ESP_LOGW("RMDBG", "High MEM32 usage: %.3f%%", mem3);
    else if (mem3 >= 90.0f && smem3 < 90.0f) ESP_LOGE("RMDBG", "Extremely high MEM32 usage: %.3f%%", mem3);
    else if (smem3 > 70.0f && mem3 <= 70.0f) ESP_LOGW("RMDBG", "MEM32 usage going back to normal: %.3f%%", mem3);

    smem1 = mem1;
    smem2 = mem2;
    smem3 = mem3;
}