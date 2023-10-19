#include "loadability.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

std::mutex Loadability::protectResources;
std::vector<double> Loadability::resources;
int Loadability::period = 3;

#ifdef _WIN32
static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

ULONGLONG GetTotalPacketsReceived() {
    MIB_IPSTATS ipStats;
    if (GetIpStatistics(&ipStats) != NO_ERROR) {
        std::cerr << "Error getting IP statistics" << std::endl;
        return -1;
    }
    return ipStats.dwInReceives;
}

ULONGLONG GetTotalPacketsSent() {
    MIB_IPSTATS ipStats;
    if (GetIpStatistics(&ipStats) != NO_ERROR) {
        std::cerr << "Error getting IP statistics" << std::endl;
        return -1;
    }
    return ipStats.dwOutRequests;
}
#endif


#ifdef __linux__
struct sysinfo memInfo;

static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

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
        SleepS(1);
    }
}
#endif

double Loadability::getCPUCurrentValue()
{
#ifdef __WIN32
//    FILETIME idleTimePrev, kernelTimePrev, userTimePrev;
//    FILETIME idleTimeNow, kernelTimeNow, userTimeNow;

//    if (GetSystemTimes(&idleTimePrev, &kernelTimePrev, &userTimePrev)) {
//        // Sleep for a short period to measure usage over time
//        Sleep(1000); // Sleep for 1 second

//        if (GetSystemTimes(&idleTimeNow, &kernelTimeNow, &userTimeNow)) {
//            // Calculate time intervals
//            ULONGLONG prevIdleTime = ((ULONGLONG)idleTimePrev.dwHighDateTime << 32) + idleTimePrev.dwLowDateTime;
//            ULONGLONG prevKernelTime = ((ULONGLONG)kernelTimePrev.dwHighDateTime << 32) + kernelTimePrev.dwLowDateTime;
//            ULONGLONG prevUserTime = ((ULONGLONG)userTimePrev.dwHighDateTime << 32) + userTimePrev.dwLowDateTime;

//            ULONGLONG nowIdleTime = ((ULONGLONG)idleTimeNow.dwHighDateTime << 32) + idleTimeNow.dwLowDateTime;
//            ULONGLONG nowKernelTime = ((ULONGLONG)kernelTimeNow.dwHighDateTime << 32) + kernelTimeNow.dwLowDateTime;
//            ULONGLONG nowUserTime = ((ULONGLONG)userTimeNow.dwHighDateTime << 32) + userTimeNow.dwLowDateTime;

//            // Calculate time differences
//            ULONGLONG totalDelta = (nowIdleTime - prevIdleTime) + (nowKernelTime - prevKernelTime) + (nowUserTime - prevUserTime);
//            ULONGLONG userDelta = nowUserTime - prevUserTime;

//            // Calculate CPU usage as a percentage
//            double cpuUsage = (1.0 - ((double)nowIdleTime - prevIdleTime) / totalDelta) * 100.0;

//            std::cout << "CPU Usage: " << cpuUsage << "%" << std::endl;
//        } else {
//            std::cerr << "GetSystemTimes (2nd call) failed with error code: " << GetLastError() << std::endl;
//        }
//    } else {
//        std::cerr << "GetSystemTimes (1st call) failed with error code: " << GetLastError() << std::endl;
//    }

    return -1.0;
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
#ifdef __WIN32
    ULONGLONG initialReceived = GetTotalPacketsReceived();
    ULONGLONG initialSent = GetTotalPacketsSent();
    if (initialReceived == -1.0 || initialSent == -1.0) {
        return -1.0;
    }

    SleepS(1);

    ULONGLONG finalReceived = GetTotalPacketsReceived();
    ULONGLONG finalSent = GetTotalPacketsSent();
    if (initialReceived == -1.0 || initialSent == -1.0) {
        return -1.0;
    }

    ULONGLONG packetsReceivedPerSecond = finalReceived - initialReceived;
    ULONGLONG packetsSentPerSecond = finalSent - initialSent;

    std::cout << "Packets Received per Second: " << packetsReceivedPerSecond << std::endl;
    std::cout << "Packets Sent per Second: " << packetsSentPerSecond << std::endl;

    return (double)(packetsReceivedPerSecond + packetsSentPerSecond);
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

    SleepS(1);

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
        return { availableRAMPercentage, availableVRAMPercentage };
    }
    else
    {
        return { -1.0, -1.0 };
    }
#endif
}

Loadability::Loadability()
{
    writerThread = std::thread([this]() {
        writeInTextFile();
    });

    resourceUpdater = std::thread([this]() {
        updateVisualRecources();
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
            result = getResult();
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
        SleepS(period);
    }
    outputFile.close();
}

void Loadability::updateVisualRecources()
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(protectResources);
            {
                std::lock_guard<std::mutex> lock(protectResourceGetter);
                std::cout << "UPDATING RESOURCES" << std::endl;
                resources = getResult();
            }
        }

        {
            std::lock_guard<std::mutex> lock(stopCondition);
            if (stop)
                break;
        }
        SleepS(0.1);
    }
}

std::vector<double> Loadability::getResult()
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
