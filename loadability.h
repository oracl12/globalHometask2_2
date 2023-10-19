#ifndef LOADABILITY_H
#define LOADABILITY_H

#define TXTLOG "log.txt"

// #include <QFile>

#ifdef _WIN32
    #include <tchar.h>
    #include <windows.h>
    #include <pdh.h>
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

#include "other.h"

class Loadability
{
public:
    Loadability(bool cpuStatus, bool memStatus, bool netStatus);// create a thread which

    Loadability(const Loadability &) = delete;

    Loadability &operator=(const Loadability &) = delete;

    ~Loadability(); // stop threads file

    static std::vector<double> getResources() {
//        std::lock_guard<std::mutex> lock(protectResources);
        return resources;
    }

    static void setPeriod(int newValue) {
        period = newValue;
    }

    static int getPeriod() {
        return period;
    }

    inline static std::vector<std::pair<std::string, bool>> &getOrderAndVisibOfInfo(){
        static std::vector<std::pair<std::string, bool>> orderAndVisib = { { "RAM", true } , { "VRAM", true }, { "CPU", true }, { "NETWORK", true } };
        return orderAndVisib;
    }
private:
    void writeInTextFile();
    void updateVisualRecources(bool cpuStatus, bool memStatus, bool netStatus);

    double getCPUCurrentValue();
    double getNetworkUsage();
    std::pair<double, double> getMemoryUsage();

    std::vector<double> getResult(bool cpuStatus, bool memStatus, bool netStatus);
    std::mutex protectResourceGetter;

    static std::mutex protectResources;
    static std::vector<double> resources;
    static int period;

    bool stop = false;
    std::mutex stopCondition;

    // threads

    std::thread writerThread;
    std::thread resourceUpdater;
};
#endif // LOADABILITY_H
