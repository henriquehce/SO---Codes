#include <chrono>
#include <stdio.h>
#include <string.h>
#include <fstream>

class Timer
{
public:
    Timer()
    {
        start = clock.now();
    }
    // Returns the duration in seconds.
    double GetElapsed()
    {
        auto end = clock.now();
        auto duration = end - start;
        return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() * 1.e-9;
    }
private:
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock clock;

    Timer(const Timer&) = delete;
    Timer operator=(const Timer*) = delete;
};

// Função para obter o número de page faults atuais
void PrintPageFaults()
{
    std::ifstream statusFile("/proc/self/status");
    std::string line;
    int minorFaults = 0, majorFaults = 0;

    while (std::getline(statusFile, line))
    {
        if (line.rfind("Minflt:", 0) == 0)
        {
            minorFaults = std::stoi(line.substr(line.find_last_of("\t") + 1));
        }
        else if (line.rfind("Majflt:", 0) == 0)
        {
            majorFaults = std::stoi(line.substr(line.find_last_of("\t") + 1));
        }
    }

}

void BusyWait(int ms)
{
    auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
    for (;;)
    {
        if (std::chrono::steady_clock::now() > end)
            break;
    }
}

void FastMeasure()
{
    printf("Busy waiting to raise the CPU frequency...\n");
    BusyWait(500);

    const int bufSizes[] = {1024 * 1024 * 1024}; // 1 GB, 2 GB, 4 GB
    const int iterationCount = 100;

    for (int bufSize : bufSizes)
    {
        printf("\nTesting with buffer size: %d MB\n", bufSize / (1024 * 1024));
        
        // Imprime o número de page faults antes do teste para o tamanho atual
        printf("Before testing buffer size %d MB:\n", bufSize / (1024 * 1024));
        PrintPageFaults();

        {
            Timer timer;
            for (int i = 0; i < iterationCount; ++i)
            {
                int* p = new int[bufSize / sizeof(int)];
                delete[] p;
            }
            printf("%1.4f s to allocate %d MB %d times.\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount);
        }
        
        {
            Timer timer;
            double deleteTime = 0.0;
            for (int i = 0; i < iterationCount; ++i)
            {
                int* p = new int[bufSize / sizeof(int)];
                Timer deleteTimer;
                delete[] p;
                deleteTime += deleteTimer.GetElapsed();
            }
            printf("%1.4f s to allocate %d MB %d times (%1.4f s to delete).\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount, deleteTime);
        }
        
        {
            int* p = new int[bufSize / sizeof(int)]();
            {
                Timer timer;
                for (int i = 0; i < iterationCount; ++i)
                {
                    memset(p, 1, bufSize);
                }
                printf("%1.4f s to write %d MB %d times.\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount);
            }
            {
                Timer timer;
                int sum = 0;
                for (int i = 0; i < iterationCount; ++i)
                {
                    for (size_t index = 0; index < bufSize / sizeof(int); ++index)
                    {
                        sum += p[index];
                    }
                }
                printf("%1.4f s to read %d MB %d times, sum = %d.\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount, sum);
            }
            delete[] p;
        }
        
        {
            Timer timer;
            double deleteTime = 0.0;
            for (int i = 0; i < iterationCount; ++i)
            {
                int* p = new int[bufSize / sizeof(int)];
                memset(p, 1, bufSize);
                Timer deleteTimer;
                delete[] p;
                deleteTime += deleteTimer.GetElapsed();
            }
            printf("%1.4f s to allocate and write %d MB %d times (%1.4f s to delete).\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount, deleteTime);
        }
        
        {
            Timer timer;
            int sum = 0;
            for (int i = 0; i < iterationCount; ++i)
            {
                int* p = new int[bufSize / sizeof(int)];
                for (size_t index = 0; index < bufSize / sizeof(int); ++index)
                {
                    sum += p[index];
                }
                delete[] p;
            }
            printf("%1.4f s to allocate and read %d MB %d times, sum = %d.\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount, sum);
        }

        // Imprime o número de page faults após o teste para o tamanho atual
        printf("After testing buffer size %d MB:\n", bufSize / (1024 * 1024));
        PrintPageFaults();
    }
}

int main(int argc, char* argv[])
{
    FastMeasure();

    return 0;
}
