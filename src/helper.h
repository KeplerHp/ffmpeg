#ifndef HELPERS_H
#define HELPERS_H

#include <atomic>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <typeinfo>
#include <sys/time.h>
#include <time.h>

/* time log */
enum LOG_TYPE {
    MEMCPY_START,
    MEMCPY_END,
    CYCLE_START,
    UNIFORM_COPY,
    WAIT_SUBMIT,
    GET_IMAGE,
    RENDER,
    RENDER_START,
    RENDER_END,
    RENDER_WAIT_SYN,
    MEMCPY,
    RENDER_ON_WINDOW,
    CYCLE_END,
    SERIALIZE_START,
    SOCKET_START,
    SOCKET_END,
    SERIALIZE_END,
    PRESENT_START,
    PRESENT_END,
    API_RUN,
};

#define LOG_MAX_SIZE (1 << 24)

struct MyTimeLog
{
    // std::chrono::time_point<std::chrono::steady_clock> time;
    unsigned long long cycle;
    LOG_TYPE type;
    int id;
};

unsigned long long get_cpu_cycle() {
    // unsigned long long t1;
    // unsigned long long t2;
    // __asm__ __volatile__(
    // "rdtsc" :
    // "=d" (t1),
    // "=a" (t2)
    // );
    // return ((t1 << 32)|(t2));
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000000+tv.tv_usec;
}


std::vector<struct MyTimeLog> logStamps(LOG_MAX_SIZE); 
// std::vector<float> logT(LOG_MAX_SIZE); 

std::atomic<int> log_index;
// std::atomic<int> log_indexT;
unsigned long long startTime = get_cpu_cycle(); 

#define NOW(my_type, my_id) { \
    logStamps[log_index++] = {.cycle=get_cpu_cycle(), .type=my_type, .id=my_id}; \
} 

/*
#define NOW_START(my_type, my_id) { \
    logStamps[log_index++] = {.time = std::chrono::high_resolution_clock::now(), .type=MEMCPY_START, .id=my_id}; \
}


#define NOW_END(my_id) { \
    auto t_end = std::chrono::high_resolution_clock::now(); \
    float duration_end = std::chrono::duration<float, std::chrono::milliseconds::period>(t_end-startTime).count(); \
    logStamps[log_index++] = {.time = duration_end, .type=MEMCPY_END, .id=my_id}; \
}
*/

void exportLog() {
    std::ofstream ofs;

    ofs.open("../log/log", std::ios::out);
    ofs.setf(std::ios::fixed);
    ofs.precision(4);
    // ofs << "[" << (startTime)/2900000.f << "]   " << "Start Time" << std::endl;
    for (int i = 0; i < log_index; i++) {
        struct MyTimeLog logItem = logStamps[i];    
        
        switch (logItem.type)
        {
        case MEMCPY_START:
            
            // ofs << "[" << std::chrono::duration<float, std::chrono::milliseconds::period>(logItem.time-startTime).count() << "]   " << "CPU " << logItem.id << " Start" << std::endl;
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "CPU " << logItem.id << " Start" << std::endl;
            break;
        case MEMCPY_END:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "CPU " << logItem.id << " End" << std::endl;
            break;
        case CYCLE_START:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "CYCLE Start " << logItem.id  << std::endl;
            break;   
        case UNIFORM_COPY:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "Uniform Copy Start " << logItem.id  << std::endl;
            break; 
        case WAIT_SUBMIT:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "Wait Submit Finish " << logItem.id  << std::endl;
            break;
        case GET_IMAGE:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "Get Swapchain Image" << logItem.id  << std::endl;
            break;
        case RENDER:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "Parrallel Render " << logItem.id  << std::endl;
            break;
        case RENDER_START:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "GPU " << logItem.id  << " Rendering Start " << std::endl;
            break;
        case RENDER_END:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "GPU " << logItem.id  << " Rendering End " << std::endl;
            break;
        case RENDER_WAIT_SYN:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "Wait Synchronization after Render " << logItem.id  << std::endl;
            break;
        case MEMCPY:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "Copy Device to Device " << logItem.id  << std::endl;
            break;
        case RENDER_ON_WINDOW:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "Primary Device Copy the Image in Swapchin " << logItem.id  << std::endl;
            break;
        case CYCLE_END:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "CYCLE End " << logItem.id  << std::endl;
            break;
        case SERIALIZE_START:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "SERIALIZE START " << logItem.id  << std::endl;
            break;
        case SOCKET_START:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "SOCKET START " << logItem.id  << std::endl;
            break;
        case SERIALIZE_END:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "SERIALIZE END " << logItem.id  << std::endl;
            break;
        case SOCKET_END:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "SOCKET END " << logItem.id  << std::endl;
            break;
        case PRESENT_START:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "PRESENT START " << logItem.id  << std::endl;
            break;
        case PRESENT_END:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "PRESENT END " << logItem.id  << std::endl;
            break;
        case API_RUN:
            ofs << "[" << (logItem.cycle-startTime)/1000.f << "]   " << "API " << logItem.id  << " run"<< std::endl;
            break;
        default:
            break;
        }
    }
    std::cout << "log_index " << log_index << std::endl;
    /*for (int i = 0; i < log_indexT; i++) {
        ofs << "[" << logT[i] << "]"<< std::endl;
    }*/

    ofs.close();
}

#endif