#ifndef LOADABILITY_H
#define LOADABILITY_H

#define TXTLOG "log.txt"

#include <QFile>
#include <QTextStream>
#include <QDebug>

#ifdef _WIN32
    #include <tchar.h>
    #include <winsock2.h>
    #include <windows.h>
    #include <pdh.h>
    #include <processthreadsapi.h>
    #include <iphlpapi.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "IPHLPAPI.lib")
#else
    #include "sys/types.h"
    #include "sys/sysinfo.h"
    #include "stdio.h"
    #include "unistd.h"
#endif

#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>

#include "other.h"

class Loadability
{
public:
    Loadability();

    Loadability(const Loadability &) = delete;

    Loadability &operator=(const Loadability &) = delete;

    ~Loadability();

    static std::vector<double> getResources() {
        std::lock_guard<std::mutex> lock(protectResources);
        return resources;
    }

    static void setResources(std::vector<double> newResources) {
        std::lock_guard<std::mutex> lock(protectResources);
        resources = newResources;
    }

    static void setPeriod(int newValue) {
        std::lock_guard<std::mutex> lock(protectPeriod);
        period = newValue;
    }

    static int getPeriod() {
        std::lock_guard<std::mutex> lock(protectPeriod);
        return period;
    }

    inline static std::vector<std::pair<std::string, bool>> &getOrderAndVisibOfInfo(){
        static std::vector<std::pair<std::string, bool>> orderAndVisib = { { "RAM", true } , { "VRAM", true }, { "CPU", true }, { "NETWORK", true } };
        return orderAndVisib;
    }
private:
    void writeInTextFile();
    void updateResources();

    double getCPUCurrentValue();
    double getNetworkUsage();
    std::pair<double, double> getMemoryUsage();

    std::vector<double> getResult();

    static std::mutex protectResources;
    static std::vector<double> resources;
    static std::mutex protectPeriod;
    static int period;

    bool stop = false;
    std::mutex stopCondition;

    // threads

    std::thread writerThread;
    std::thread resourceUpdater;
};
#endif // LOADABILITY_H
