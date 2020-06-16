#include <queue>
#include <string>
#include <vector>
#include <cstdlib>
#include <string.h>

#include <ctime>
#include <sys/time.h>

#include <pthread.h>

#include "traceItem/traceItem.h"
#include "utils/traceUtils.h"

using namespace std;

typedef struct stAdderPara
{
    string tracePathPara;
    string workPathPara;
    string logPathPara;
} adderParaST;

bool initMode = true;
std::queue<traceItemST> traceQueue;
pthread_mutex_t mutex;
pthread_cond_t cond_queue = PTHREAD_COND_INITIALIZER;

bool adder_sleep = false;
bool trace_end = false;

void traceUser(ofstream &logfile)
{
    struct timeval timeStart;
    struct timeval timeEnd;
    int timeCost = 0;
    traceItemST thisUserTrace;
    cout << "[USER]: Start" << endl;
    opType tempTraceOP;
    int count = 0;
    while (traceQueue.size() > 0 || !trace_end)
    {
        if (traceQueue.size() > 0)
        {
            pthread_mutex_lock(&mutex);
            thisUserTrace = traceQueue.front();
            //handleTraceItem(thisUserTrace);
            count++;
            traceQueue.pop();
            pthread_mutex_unlock(&mutex);
            //cout << "[USER]: replaying: " << count << endl;
            if (!check_uselessOP(thisUserTrace.traceOptype))
            {
                gettimeofday(&timeStart, NULL);
                handleTraceItem(thisUserTrace);
                gettimeofday(&timeEnd, NULL);
                logfile << thisUserTrace.opName << "\t" << timeStart.tv_sec << "." << timeStart.tv_usec << "\t" << timeEnd.tv_sec << "." << timeEnd.tv_usec << "\t" << timeCost << endl;
            }
            //usleep(500000);
        }
    }
    logfile.close();
    cout << " [USER--END] " << endl;
}

void *traceAdder(void *adderParaPara)
{

    adderParaST adderPara = *(adderParaST *)adderParaPara;

    string tempLine;
    traceItemST thisAdderTrace;
    ifstream trace;
    int addCount = 0;
    cout << "[ADDER]: Start to add trace:" << adderPara.tracePathPara << endl;
    trace.open(adderPara.tracePathPara.c_str(), ios::in);

    while (!trace.eof())
    {
        if (traceQueue.size() < 500)
        {
            getline(trace, tempLine);
            if (tempLine.empty())
            {
                continue;
            }
            else
            {
                thisAdderTrace = getTrace(tempLine, adderPara.workPathPara);
                pthread_mutex_lock(&mutex);
                traceQueue.push(thisAdderTrace);
                addCount++;
                //cout << "[ADDER]: NO." << addCount << ":" << tempLine << endl;
                pthread_mutex_unlock(&mutex);
            }
        }
    }
    trace_end = true;
    trace.close();
    cout << " [ADDER--END] ADD: " << addCount << endl;
}

int main(int argc, char **argv)
{
    if (argc != 4 && argc != 5)
    {
        cout << "Parameter: 1.Trace Path 2.Mode 3.Work Path(end with /), 4.No" << endl;
        return 0;
    }
    string tracePath = string(argv[1]);
    string modeString = string(argv[2]);
    string workString = string(argv[3]);
    string workerNo;
    int wrongTraceCount = 0;
    if (argc == 4)
    {
        workerNo = "";
    }
    else
    {
        workerNo = string(argv[4]);
    }
    //string logDir = "/home/ceph/cyx/metadata-management/experiments/scripts/migrate/ai-training/traceLog/";
    string logDir = "/home/ceph/duo/traceReplay_multiThread/test/";

    string logPath = logDir + *(splitString_STL(workString, "/").end() - 2) + workerNo + ".log";
    ofstream logFile;
    logFile.open(logPath.c_str(), ios::trunc);

    cout << "[INFO]: Current Log File: " << logPath.c_str() << endl;

    if (modeString == "init")
    {
        initMode = true;
    }
    else if (modeString == "run")
    {
        initMode = false;
    }
    else
    {
        cout << "Mode Value: 1.init 2.run" << endl;
        return 0;
    }

    vector<string> subTracePath = splitString_STL(tracePath, "/");
    //string workPath = "/mnt/ceph-client-1/trace/"+subTracePath.back();
    string workPath = workString + subTracePath.back();
    string thisLine;
    ifstream trace;
    vector<traceItemST> traceVector;
    traceItemST tempTrace;

    int tracePos;
    opType traceOP;

    if (initMode)
    {
        trace.open(tracePath.c_str(), ios::in);
        cout << "[WAN]: Init trace is SINGLE THREAD!" << endl;
        while (!trace.eof())
        {
            getline(trace, thisLine);
            tempTrace = getTrace(thisLine, workPath);
            traceVector.push_back(tempTrace);
        }
        trace.close();
        cout << "[INFO]: Read trace finished, start Init File" << endl;
        for (int tracePos = 0; tracePos < traceVector.size(); tracePos++)
        {
            if (initMode)
            {
                initFileSet(traceVector[tracePos]);
            }
        }
        wrongTraceCount = genFromFileSet();
        cout << "[INFO]: Init end, handle: " << wrongTraceCount << "wrong trace." << endl;
        return 0;
    }

    // Not init, replay trace by mutli-thread

    int trace_used = 0;

    cout << "[INFO]: Run Mode, replay trace by mutli-thread" << endl;
    adderParaST adderPara = {tracePath, workPath, logPath};
    pthread_t user;

    pthread_mutex_init(&mutex, NULL);
    pthread_create(&user, NULL, traceAdder, &adderPara);

    traceUser(logFile);

    logFile.close();
    pthread_join(user, NULL);
    cout << "[MAIN--END]" << endl;

    return 0;
}
