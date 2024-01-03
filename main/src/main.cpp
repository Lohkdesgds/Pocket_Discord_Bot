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

#include "smartjsonalloc.h"

using namespace Lunaris;
using namespace PocketDiscord;

void printch(char c) { putchar(c); }

void check_ram_notify()
{
    const ram_info nw{};
    const float mem1 = (100.0f - (nw.mem_free   * 100.0f / nw.mem_total));
    const float mem2 = (100.0f - (nw.memex_free * 100.0f / nw.memex_total));
    const float mem3 = (100.0f - (nw.mem32_free * 100.0f / nw.mem32_total));

    if (mem1 >= 80.0f) {
        ESP_LOGW("RMDBG", "High MEM usage: %.3f%%", mem1);
    }
    if (mem2 >= 80.0f) {
        ESP_LOGW("RMDBG", "High MEMEX usage: %.3f%%", mem2);
    }
    if (mem3 >= 80.0f) {
        ESP_LOGW("RMDBG", "High MEM32 usage: %.3f%%", mem3);        
    }

    //const float mem4 = (100.0f - (nw.memi_free  * 100.0f / nw.memi_total));

    //ESP_LOGI("RAMDEBUG", "MEM=%.3f%% MEMEX=%.3f%% MEM32=%.3f%%",
    //    (100.0f - (nw.mem_free   * 100.0f / nw.mem_total)),
    //    (100.0f - (nw.memex_free * 100.0f / nw.memex_total)),
    //    (100.0f - (nw.mem32_free * 100.0f / nw.mem32_total))
    //);
}

void event_handler(const gateway_events& t, const JSON& j)
{
    static const char* TAG = "EVHLR";
    //const char* prt = (const char*)j["t"];
    //if (prt == nullptr) prt = "NULL";

    ESP_LOGI(TAG, "Got event %li at core %i in MAIN!",
        static_cast<int32_t>(t), (int)xPortGetCoreID()
    );

    //j.print(printch, 0);
    
    //post_ram_usage();
}

static void debug_memory_usage_total(void* param)
{
    while(1) {
        check_ram_notify();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// POOR IMPL, FOR TESTING
class FileJSON : public IterateableJSONRef {
    File* m_fp;
public:
    FileJSON(File*&& fp) : IterateableJSONRef(), m_fp(fp) { fp = nullptr; }
    ~FileJSON() { delete m_fp; }

    char get(const size_t at) const { m_fp->seek(at, File::seek_mode::SET); int v = m_fp->getc(); return v < 0 ? '\0' : static_cast<char>(v); }
    void read(char* ptr, const size_t len, const size_t at) const { m_fp->seek(static_cast<long>(at), File::seek_mode::SET); m_fp->read(ptr, len); }
    size_t max_off() const { return static_cast<size_t>(-1); } // assume infinite storage lmao
};

static void task_json_id(void* param)
{
    int& cas = *(int*)param;

    const char key1[] = "name";
    const char key2[] = "bio";
    const char nil[] = "NULL";

    try {
        const char* fp = "";

        switch(cas) {
        case 0:
            fp = "ex_64kb.txt";
            break;
        case 2:
            fp = "ex_128kb.txt";
            break;
        case 4:
            fp = "ex_256kb.txt";
            break;
        case 6:
            fp = "ex_512kb.txt";
            break;
        case 8:
            fp = "ex_1mb.txt";
            break;
        case 10:
            fp = "ex_5mb.txt";
            break;
        default:
            vTaskDelete(NULL);
            return;
        }

        ESP_LOGI("BMK", "Benchmarking JSON file %s", fp);

        auto f = new File(fp, "rb");
        if (f->bad()) {
            ESP_LOGI("BMK", "Cannot benchmarking JSON file %s!", fp);
            ++cas;
            vTaskDelete(NULL);
            return;
        }

        //f->seek(0, File::seek_mode::SET);

        ESP_LOGI("BMK", "Wrapping data into JSON iterateable class...");
        
        auto wrap = new FileJSON((File*&&)f);

        ESP_LOGI("BMK", "Parsing now...");
        
        JSON j((FileJSON*&&)wrap);
        
        ESP_LOGI("BMK", "Parsed! Testing reading pos #100 name and bio:");

        JSON off = j[100];

        const char* nam = off[key1].get_string();
        const char* bio = off[key2].get_string();

        if (!nam) nam = nil;
        if (!bio) bio = nil;

        ESP_LOGI("BMK", "=> { %s, %s }", nam, bio);
    }
    catch(const std::exception& e) {
        ESP_LOGE("BMK", "Exception! %s", e.what());
    }
    catch(...){
        ESP_LOGE("BMK", "Exception! UNKNOWN");
    }

    ++cas;
    vTaskDelete(NULL);
}

void app_main(void)
{
    xTaskCreate(debug_memory_usage_total, "MEMTRACKD", 2560, nullptr, tskIDLE_PRIORITY, nullptr);

    //const ram_info b4{};
    vTaskDelay(pdMS_TO_TICKS(5000));

    BotBase* bot /*= nullptr;*/= new BotBase();
    vTaskDelay(pdMS_TO_TICKS(2000));

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

        vTaskDelay(pdMS_TO_TICKS(40000));
    }

    //int task_worked = 0;
    //while(task_worked < 8) {
    //    xTaskCreatePinnedToCore(task_json_id, "RECTEST", 16384, (void*)&task_worked, 10, nullptr, 0); // core0 = main
    //    while(task_worked % 2 == 0) vTaskDelay(pdMS_TO_TICKS(200));
    //    ++task_worked;
    //}
    
//    volatile int task_worked = 0;
//
//    xTaskCreatePinnedToCore(test_multicore_file, "CORE0TEST", 3072, (void*)&task_worked, 10, nullptr, 0);
//
//    while(task_worked != 2) vTaskDelay(20);
//
//    vTaskDelay(pdMS_TO_TICKS(1000));
//    
//    xTaskCreatePinnedToCore(test_multicore_file, "CORE1TEST", 3072, (void*)&task_worked, 10, nullptr, 1);
//    
//    while(task_worked < 4) vTaskDelay(20);
//    
//    vTaskDelay(pdMS_TO_TICKS(1000));
//    task_worked = 10;
//
//    xTaskCreatePinnedToCore(test_multicore_file, "CORE0TEST2", 3072, (void*)&task_worked, 10, nullptr, 0);
//
//    while(task_worked != 12) vTaskDelay(20);
//
//    vTaskDelay(pdMS_TO_TICKS(1000));
//    
//    xTaskCreatePinnedToCore(test_multicore_file, "CORE1TEST2", 3072, (void*)&task_worked, 10, nullptr, 1);
//    
//    while(task_worked < 14) vTaskDelay(20);
//


    vTaskDelay(pdMS_TO_TICKS(10000));
    delete bot;
    
    //const ram_info af{};
    //    
    //ESP_LOGI("MAIN", 
    //    "Memory usage before:\n"
    //    "Free: %zu\n"
    //    "Total: %zu\n"
    //    "Usage Percentage: %.2f%%",
    //    b4.mem_free, b4.mem_total, 100.0f - b4.mem_free * 100.0f / b4.mem_total
    //);
    //    
    //ESP_LOGI("MAIN", 
    //    "Memory usage now:\n"
    //    "Free: %zu\n"
    //    "Total: %zu\n"
    //    "Usage Percentage: %.2f%%",
    //    af.mem_free, af.mem_total, 100.0f - af.mem_free * 100.0f / af.mem_total
    //);

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        
        //const ram_info af2{};
        //
        //ESP_LOGI("MAIN", 
        //    "Memory usage recurrent:\n"
        //    "Free: %zu\n"
        //    "Total: %zu\n"
        //    "Usage Percentage: %.2f%%",
        //    af2.mem_free, af2.mem_total, 100.0f - af2.mem_free * 100.0f / af2.mem_total
        //);
    }
}





//static void test_multicore_file(void* param)
//{
//    volatile int& work_int = *(volatile int*)param;
//    ++work_int;
//
//    const char* fp_name = "undefined.txt";
//
//    if (work_int == 1) fp_name = "testing.txt";
//    else if (work_int == 3) fp_name = "testing2.txt";
//    else if (work_int == 11) fp_name = "testing3.txt";
//    else if (work_int == 13) fp_name = "testing4.txt";
//
//    ESP_LOGI("MAIN", "Starting tests on CORE#%i...", (int)xPortGetCoreID());
//    ESP_LOGI("MAIN", "Testing writing to %s", fp_name);
//
//    File fp(fp_name, "wb+");
//    fp.write("This is an test", 15);
//    fp.flush();
//    fp.close();
//
//    ESP_LOGI("MAIN", "Opening back...");
//
//    File fp2(fp_name, "rb+");
//    if (fp2.bad()) {
//        ESP_LOGE("MAIN", "Cannot open back.");
//    }
//    else {
//        char tmp[16]{};
//        const size_t rr = fp2.read(tmp, 15);
//        tmp[15] = '\0';
//        if (rr) ESP_LOGI("MAIN", "Read %zu bytes: %s", rr, tmp);
//        else ESP_LOGE("MAIN", "FREAD FAILED");
//    }
//    fp2.close();
//
//
//    ESP_LOGI("MAIN", "Benchmark time.");
//
//    char* ptr = new char[1024];
//
//    for(size_t p = 0; p < 1024; ++p) {
//        ptr[p] = ((p % 10) + '0');
//    }
//    
//    File fp3(fp_name, "wb+");
//
//    if (fp3.good()) {
//        auto now = get_time_ms();
//
//        for(size_t p = 0; p < 1024; ++p) {
//            fp3.write(ptr, 1024);
//        }
//        fp3.flush();
//
//        auto later = get_time_ms();
//        
//        ESP_LOGI("MAIN", "Took %llu ms to write 1 MB.", later - now);
//        fp3.seek(0, File::seek_mode::SET);
//
//        vTaskDelay(pdMS_TO_TICKS(200));
//
//        size_t errs = 0;
//        now = get_time_ms();
//        
//        for(size_t p = 0; p < 1024; ++p) {
//            size_t readd = fp3.read(ptr, 1024);
//            if (readd != 1024) errs += (1024 - readd);
//        }
//
//        later = get_time_ms();        
//
//        if (errs > 0) ESP_LOGW("MAIN", "Failed to read 1024 one or more times. Failed to read a total of %zu bytes.", errs);
//        ESP_LOGI("MAIN", "Took %llu ms to read 1 MB.", later - now);
//
//        fp3.close();
//
//
//        vTaskDelay(pdMS_TO_TICKS(200));
//
//        ESP_LOGI("MAIN", "Benchmark, but on MixedJSONRef.");
//        
//        for(size_t p = 0; p < 1024; ++p) {
//            ptr[p] = ((p % 10) + '0');
//        }
//
//        MixedJSONRef mx(1024 * 1024); // testing should cause half in memory cause of fixed now
//
//        now = get_time_ms();
//
//        for(size_t p = 0; p < 1024; ++p) {
//            mx.write(ptr, 1024, p * 1024);
//        }
//        fp3.flush();
//
//        later = get_time_ms();
//        
//        ESP_LOGI("MAIN", "Took %llu ms to write 1 MB on MixedJSONRef.", later - now);
//        fp3.seek(0, File::seek_mode::SET);
//
//        vTaskDelay(pdMS_TO_TICKS(200));
//
//        now = get_time_ms();
//        
//        for(size_t p = 0; p < 1024; ++p) {
//            mx.read(ptr, 1024, p * 1024);
//        }
//
//        later = get_time_ms();        
//
//        ESP_LOGI("MAIN", "Took %llu ms to read 1 MB on MixedJSONRef.", later - now);
//
//        vTaskDelay(pdMS_TO_TICKS(1000));
//    }
//    else {
//        ESP_LOGE("MAIN", "Could not start benchmark.");
//    }
//
//    delete[] ptr;
//    
//    ++work_int;
//    ESP_LOGI("MAIN", "End.");
//
//    vTaskDelete(NULL);
//}