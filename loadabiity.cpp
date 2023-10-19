#include "loadability.h"


#include <QFile>
#include <QTextStream>
#include <QDebug>

std::mutex Loadability::protectResources;
std::vector<double> Loadability::resources;
int Loadability::period = 3;

double Loadability::getCPUCurrentValue(){
    PDH_HQUERY cpuQuery;
    PDH_HCOUNTER cpuTotal;
    PdhOpenQuery(NULL, NULL, &cpuQuery);
//    PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhAddEnglishCounter(cpuQuery, L"\\Processor(*)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);

    PDH_FMT_COUNTERVALUE counterVal;

    Sleep(1000);
    PdhCollectQueryData(cpuQuery);
    Sleep(1000);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);

    return ((int) counterVal.doubleValue == 0) ? -1.0 : counterVal.doubleValue;
}

double Loadability::GetNetworkUsage() {
    PDH_HQUERY query;
    PDH_HCOUNTER networkCounter;

    PDH_STATUS status = PdhOpenQuery(nullptr, 0, &query);
    if (status != ERROR_SUCCESS) {
        std::cerr << "PdhOpenQuery error: " << status << std::endl;
        return -1.0; // Handle the error
    }

    // Add the network counter
    status = PdhAddCounterW(query, L"\\Network Interface(*)\\Bytes Total/sec", 0, &networkCounter);
    if (status != ERROR_SUCCESS) {
        std::cerr << "PdhAddCounter error: " << status << std::endl;
        return -1.0; // Handle the error
    }

    status = PdhCollectQueryData(query);
    if (status != ERROR_SUCCESS) {
        std::cerr << "PdhCollectQueryData error: " << status << std::endl;
        return -1.0; // Handle the error
    }

    PDH_FMT_COUNTERVALUE counterValue;
    status = PdhGetFormattedCounterValue(networkCounter, PDH_FMT_LARGE, nullptr, &counterValue);
    if (status != ERROR_SUCCESS) {
        std::cerr << "PdhGetFormattedCounterValue error: " << status << std::endl;
        return -1.0; // Handle the error
    }

    return static_cast<double>(counterValue.largeValue);
}

Loadability::Loadability(bool cpuStatus, bool memStatus, bool netStatus)
{
    writerThread = std::thread([this]() {
        writeInTextFile();
    });

    resourceUpdater = std::thread([this, cpuStatus, memStatus, netStatus]() {
        updateVisualRecources(cpuStatus, memStatus, netStatus);
    });

    std::clog << "LOADABILITY: THREADS CREATED" << std::endl;
}

Loadability::~Loadability()
{
    {
        std::lock_guard<std::mutex> lock(stopCondition);
        stop = true;
    }

    if (writerThread.joinable())
        writerThread.join();
    if (resourceUpdater.joinable())
        resourceUpdater.join();

    std::clog << "LOADABILITY: THREADS STOPPED" << std::endl;
}

void Loadability::writeInTextFile()
{
    std::ofstream outputFile("log.txt");
    std::cout << "LOADABILITY: WE ARE in TeXT FILE" << std::endl;
    while (true)
    {
        std::cout << "WE ARE IN THREAD" << std::endl;
        std::string textToAppend;
        std::vector<double> result;
        {
            std::cout << "Lock" << std::endl;
            std::lock_guard<std::mutex> lock(protectResourceGetter);
            result = getResult(true, true, true);
            std::cout << "UnLock" << std::endl;
        }
        for (const auto& r: result){
            textToAppend += std::to_string(r) + " \n";
        }

        if (outputFile.is_open() && !outputFile.fail()) {

            std::cout << textToAppend << std::endl;
            outputFile << textToAppend << std::endl;

            std::cout << "LOADABILITY: ADDED STRING TO FILE" << std::endl;
        } else {
            std::cerr << "LOADABILITY: FILE iS NOT OPEN error" << std::endl;
            break;
        }

        {
            std::lock_guard<std::mutex> lock(stopCondition);
            if (stop)
                break;
        }

        std::cout << "Current period" << period << std::endl;
        Sleep(period * 1000); // in miliseconds
    }
    outputFile.close();
}

void Loadability::updateVisualRecources(bool cpuStatus, bool memStatus, bool netStatus){
    while (true) {
        {
            std::lock_guard<std::mutex> lock(protectResources);
            {
                std::lock_guard<std::mutex> lock(protectResourceGetter);
                std::cout << "UPDATING RESOURCES" << std::endl;
                resources = getResult(cpuStatus, memStatus, netStatus);
            }
        }

        {
            std::lock_guard<std::mutex> lock(stopCondition);
            if (stop)
                break;
        }
        Sleep(100);
    }
}


std::vector<double> Loadability::getResult(bool cpuStatus, bool memStatus, bool netStatus)
{
    std::vector<double> systemInfo;
    MEMORYSTATUSEX memory_status;
    ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
    memory_status.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memory_status)) {
        double availableVRAMPercentage = static_cast<double>(memory_status.ullAvailVirtual) / memory_status.ullTotalVirtual * 100.0;
        double availableRAMPercentage = static_cast<double>(memory_status.ullAvailPhys) / memory_status.ullTotalPhys * 100.0;
        systemInfo.push_back(availableRAMPercentage);
        systemInfo.push_back(availableVRAMPercentage);
    } else {
        systemInfo.push_back(-1.0);
        systemInfo.push_back(-1.0);
    }


    // CPU
    systemInfo.push_back(getCPUCurrentValue());

    // Network
    systemInfo.push_back(GetNetworkUsage());
    return systemInfo;
}
