#ifndef LOADABILITY_H
#define LOADABILITY_H

#pragma once
#pragma comment(lib,"pdh.lib")

#define TXTLOG "log.txt"

#include <QFile>
#include <tchar.h>
#include <windows.h>
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <pdh.h>
#include <fstream>
#include <iostream>
#include <cmath>

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
    double GetNetworkUsage();

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
