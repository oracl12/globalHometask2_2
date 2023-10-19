#include "loadability.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

std::mutex Loadability::protectResources;
std::vector<double> Loadability::resources;
int Loadability::period = 3;

struct sysinfo memInfo;

void initProc()
{
    static int counter = 0;
    if (counter == 0)
    {
        std::cout << "Was called init proc" << std::endl;
        FILE *file = fopen("/proc/stat", "r");
        fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow, &lastTotalSys, &lastTotalIdle);
        fclose(file);
        counter++;
        Sleep(1);
    }
}

double Loadability::getCPUCurrentValue()
{
#ifdef __WIN32
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
    return ((int)counterVal.doubleValue == 0) ? -1.0 : counterVal.doubleValue;

#else
    initProc();

    double percent;
    FILE *file;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

    file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
           &totalSys, &totalIdle);
    fclose(file);

    std::cout << totalUser << "b  " << totalUserLow << " " << totalSys << " " << totalIdle << std::endl;
    if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
        totalSys < lastTotalSys || totalIdle < lastTotalIdle)
    {
        percent = -1.0;
    }
    else
    {
        total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
                (totalSys - lastTotalSys);
        percent = total;
        total += (totalIdle - lastTotalIdle);
        percent /= total;
        percent *= 100;
    }

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    std::cout << percent << std::endl;
    return percent;
#endif
}

double Loadability::getNetworkUsage()
{
// bytes received/send for second
#ifdef __WIN32
    PDH_HQUERY query;
    PDH_HCOUNTER networkCounter;

    PDH_STATUS status = PdhOpenQuery(nullptr, 0, &query);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "PdhOpenQuery error: " << status << std::endl;
        return -1.0; // Handle the error
    }

    // Add the network counter
    status = PdhAddCounterW(query, L"\\Network Interface(*)\\Bytes Total/sec", 0, &networkCounter);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "PdhAddCounter error: " << status << std::endl;
        return -1.0; // Handle the error
    }

    status = PdhCollectQueryData(query);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "PdhCollectQueryData error: " << status << std::endl;
        return -1.0; // Handle the error
    }

    PDH_FMT_COUNTERVALUE counterValue;
    status = PdhGetFormattedCounterValue(networkCounter, PDH_FMT_LARGE, nullptr, &counterValue);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "PdhGetFormattedCounterValue error: " << status << std::endl;
        return -1.0; // Handle the error
    }

    return static_cast<double>(counterValue.largeValue);
#else

    std::ifstream myfile("/proc/net/dev");
    std::string line;

    if (!myfile.is_open())
    {
        std::cerr << "File opening error: " << std::endl;
        return -1.0;
    }

    // Skip the first two lines
    for (int i = 0; i < 2; i++)
    {
        std::getline(myfile, line);
    }

    unsigned long long totalReceivedBytes1 = 0;
    unsigned long long totalSentBytes1 = 0;

    while (std::getline(myfile, line))
    {
        unsigned long long receivedBytes, sentBytes;
        if (sscanf(line.c_str(), "%*s %llu %*u %*u %*u %*u %*u %*u %*u %llu", &receivedBytes, &sentBytes) == 2)
        {
            totalReceivedBytes1 += receivedBytes;
            totalSentBytes1 += sentBytes;
        }
    }

    std::cout << totalReceivedBytes1 << std::endl;

    Sleep(1);

    myfile.clear();
    myfile.seekg(0, std::ios::beg);

    for (int i = 0; i < 2; i++)
    {
        std::getline(myfile, line);
        std::cout << line << std::endl;
    }

    unsigned long long totalReceivedBytes2 = 0;
    unsigned long long totalSentBytes2 = 0;

    while (std::getline(myfile, line))
    {
        unsigned long long receivedBytes, sentBytes;
        if (sscanf(line.c_str(), "%*s %llu %*u %*u %*u %*u %*u %*u %*u %llu", &receivedBytes, &sentBytes) == 2)
        {
            totalReceivedBytes2 += receivedBytes;
            totalSentBytes2 += sentBytes;
        }
    }

    unsigned long long receivedDiff = totalReceivedBytes2 - totalReceivedBytes1;
    unsigned long long sentDiff = totalSentBytes2 - totalSentBytes1;

    std::cout << "Received Bytes Difference: " << receivedDiff << std::endl;
    std::cout << "Sent Bytes Difference: " << sentDiff << std::endl;

    myfile.close();

    return static_cast<double>(receivedDiff + sentDiff);
#endif
}

std::pair<double, double> Loadability::getMemoryUsage()
{
#ifdef __linux__
    // VRAM
    sysinfo(&memInfo);

    long long totalVirtualMem = memInfo.totalram;
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;

    long long virtualMemUsed = memInfo.totalram - memInfo.freeram;

    virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
    virtualMemUsed *= memInfo.mem_unit;

    double availableVRAMPercentage = static_cast<double>(totalVirtualMem - virtualMemUsed) / totalVirtualMem * 100.0;

    // RAM
    long long totalPhysMem = memInfo.totalram;
    totalPhysMem *= memInfo.mem_unit;

    long long physMemUsed = memInfo.totalram - memInfo.freeram;
    physMemUsed *= memInfo.mem_unit;
    double availableRAMPercentage = static_cast<double>(totalPhysMem - physMemUsed) / totalPhysMem * 100.0;
    std::cout << availableRAMPercentage << std::endl;
    std::cout << availableVRAMPercentage << std::endl;

    return {availableRAMPercentage, availableVRAMPercentage};
#else
    MEMORYSTATUSEX memory_status;
    ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
    memory_status.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memory_status))
    {
        double availableVRAMPercentage = static_cast<double>(memory_status.ullAvailVirtual) / memory_status.ullTotalVirtual * 100.0;
        double availableRAMPercentage = static_cast<double>(memory_status.ullAvailPhys) / memory_status.ullTotalPhys * 100.0;
        return { availableRAMPercentage, availableVRAMPercentage }
    }
    else
    {
        return { -1.0, -1.0 }
    }
#endif
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
    std::ofstream outputFile(TXTLOG);
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
        for (const auto &r : result)
        {
            textToAppend += std::to_string(r) + " \n";
        }

        if (outputFile.is_open() && !outputFile.fail())
        {

            std::cout << textToAppend << std::endl;
            outputFile << textToAppend << std::endl;

            std::cout << "LOADABILITY: ADDED STRING TO FILE" << std::endl;
        }
        else
        {
            std::cerr << "LOADABILITY: FILE iS NOT OPEN error" << std::endl;
            break;
        }

        {
            std::lock_guard<std::mutex> lock(stopCondition);
            if (stop)
                break;
        }

        std::cout << "Current period" << period << std::endl;
        Sleep(period);
    }
    outputFile.close();
}

void Loadability::updateVisualRecources(bool cpuStatus, bool memStatus, bool netStatus)
{
    while (true)
    {
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
        Sleep(0.1);
    }
}

std::vector<double> Loadability::getResult(bool cpuStatus, bool memStatus, bool netStatus)
{
    std::vector<double> systemInfo;

    // Mem
    systemInfo.push_back(getMemoryUsage().first);
    systemInfo.push_back(getMemoryUsage().second);
    // CPU
    systemInfo.push_back(getCPUCurrentValue());

    // Network
    systemInfo.push_back(getNetworkUsage());
    return systemInfo;
}
