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

const uint64_t owner_id = 280450898885607425;

void check_ram_notify();
void event_handler(const gateway_events& t, const JSON& j, HTTPS* h, GatewayBot gb);

static void debug_memory_usage_total(void* param)
{
    while(1) {
        check_ram_notify();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

bool g__keep_running = true;

void app_main(void)
{
    xTaskCreate(debug_memory_usage_total, "MEMTRACKD", 2560, nullptr, tskIDLE_PRIORITY, nullptr);

    vTaskDelay(pdMS_TO_TICKS(5000));

    BotBase* bot = new BotBase();
    
    HeapString token_dynamic;        
    READFILE_FULLY_TO(token_dynamic, "token.txt", "Could not load token certificate.");

    {
        auto thebot = bot->make_bot(token_dynamic.c_str(), 
            //gateway_intents::GUILDS |
            //gateway_intents::GUILD_MESSAGES |
            //gateway_intents::GUILD_MEMBERS | 
            gateway_intents::GUILD_MESSAGE_REACTIONS,
            event_handler);

        while(g__keep_running) {
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
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
    case gateway_events::READY:
        {
            GatewayPresence pre;
            pre.set_gaming("In development!", "Try it owo").set_status(GatewayPresence::status::IDLE);
            gb.send_presence((GatewayPresence&&)pre);
        }
        {
            const char* slash_commands_hardcoded = u8"{\"name\": \"random\",\"description\": \"Get a random number\",\"options\":[]}";
            const char* slash_commands_hardcoded2 = u8"{\"name\": \"shutdown\",\"description\": \"Turn it off safely (onwer only)\",\"options\":[]}";
            h->request(http_request::POST, "/applications/1192811187407171644/commands", slash_commands_hardcoded, strlen(slash_commands_hardcoded));
            h->request(http_request::POST, "/applications/1192811187407171644/commands", slash_commands_hardcoded2, strlen(slash_commands_hardcoded2));
            
        }
        break;
    case gateway_events::INTERACTION_CREATE:
        {
            //j.print([](char ch){putchar(ch);});

            const uint64_t type = j["type"];
            char* buf = nullptr, *bufpost = nullptr;
            size_t len = 0, len_post = 0;
            bool avoid_buf_delete = false;

            switch(type) {
            case 1: // PING // only for webhook apps
                break;
            case 2: // APPLICATION_COMMAND
            {                
                //const uint64_t chid = j["channel_id"].get_uint();
                const auto jid = j["id"];
                const auto jtk = j["token"];
                const auto jcn = j["data"]["name"];
                const auto jui = j["member"]["user"]["id"];


                const HeapString id = jid.get_string();
                const HeapString token = jtk.get_string();
                const HeapString cmd_name = jcn.get_string();
                const HeapString who_ran = jui.get_string();
                const HeapString myself_for_now_idk = "280450898885607425";

                if (strcmp(cmd_name.c_str(), "random") == 0)
                {
                    saprintf(buf, &len, // type: https://discord.com/developers/docs/interactions/receiving-and-responding#interaction-response-object-interaction-callback-type
                        "{\"type\":4,\"data\":{\"content\":\"Your random number is: %lu - generated at timestamp %llu\"}}",
                        esp_random(), get_time_ms()
                    );
                }
                else if (strcmp(cmd_name.c_str(), "shutdown") == 0)
                {
                    if (strcmp(myself_for_now_idk.c_str(), who_ran.c_str()) == 0) {
                        buf = (char*)"{\"type\":4,\"data\":{\"content\":\"Shutting down soon. Keep an eye on the LED\",\"flags\":64}}";
                        len = strlen(buf);
                        avoid_buf_delete = true;
                        
                        g__keep_running = false;
                    }
                    else {
                        buf = (char*)"{\"type\":4,\"data\":{\"content\":\"You are not the owner! Not shutting down.\",\"flags\":64}}";
                        len = strlen(buf);
                        avoid_buf_delete = true;
                    }
                }


                saprintf(bufpost, &len_post,
                    "/interactions/%.*s/%.*s/callback",
                    id.size(), id.c_str(), token.size(), token.c_str()
                );  

                //ESP_LOGI(TAG, "Sending %.*s @ %.*s ...", len, buf, len_post, bufpost);

                const auto rj = h->request(http_request::POST, bufpost, buf, len);
                //rj.print([](char ch){putchar(ch);});
            }
            case 3: // MESSAGE_COMPONENT
            case 4: // APPLICATION_COMMAND_AUTOCOMPLETE
            case 5: // MODAL_SUBMIT
            default:
                break;
            }
                        

            if (!avoid_buf_delete) DEL_EM(buf);
            DEL_EM(bufpost);
        }
        break;
    /*case gateway_events::MESSAGE_CREATE:
        j.print([](char ch){putchar(ch);});
        {
            //const auto author = j["author"];
            if (!j["author"]["bot"].get_bool()) {
                const uint64_t m_mid = j["id"].get_uint();
                const uint64_t m_cid = j["channel_id"].get_uint();
                const uint64_t m_gid = j["guild_id"].get_uint();
                const uint64_t usrid = j["author"]["id"].get_uint();
                const JSON m_content_ref = j["content"];
                const char* m_content = m_content_ref.get_string();
                if (m_content == nullptr) m_content = "NULL";

                ESP_LOGI(TAG, "Message ev from %llu: %s", usrid, m_content);
                //gb.__default_test();//update_presence("This is the name", "This is the state", 0);

                
                char* buf = nullptr, *bufpost = nullptr;
                size_t len = 0, len_post = 0;

                if (strcmp(m_content, "$shutdown") == 0 && usrid == owner_id) {
                    saprintf(buf, &len,
                        "{\"content\": \"Ok! Turning things off now.\",\"message_reference\":{\"message_id\":%llu,\"guild_id\":%llu,\"fail_if_not_exists\":false}}",
                        m_mid, m_gid
                    );
                    saprintf(bufpost, &len_post,
                        "/channels/%llu/messages",
                        m_cid
                    );

                    h->request(http_request::POST, bufpost, buf, len);
                    g__keep_running = false;
                }
                else if (strcmp(m_content, "is the bot on?") == 0) {

                    saprintf(buf, &len,
                        "{\"content\": \"Yes, I am alive! For now...\",\"message_reference\":{\"message_id\":%llu,\"guild_id\":%llu,\"fail_if_not_exists\":false}}",
                        m_mid, m_gid
                    );
                    saprintf(bufpost, &len_post,
                        "/channels/%llu/messages",
                        m_cid
                    );

                    h->request(http_request::POST, bufpost, buf, len);
                }

                DEL_EM(buf);
                DEL_EM(bufpost);
            }
        }
        break;*/
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