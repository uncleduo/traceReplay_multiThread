#include <queue>
#include <string>
#include <vector>
#include <cstdlib>
#include <string.h>
#include <set>
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
pthread_mutex_t queue_edit_mutex;
pthread_cond_t queue_edit_cond = PTHREAD_COND_INITIALIZER;

bool adder_sleep = false;
bool trace_end = false;
bool debug_mode = false;

int adderSleepThreshold = 500;
int adderWakeThreshold = 100;

void traceUser(ofstream &logfile)
{
    //sleep(1);
    struct timeval timeStart;
    struct timeval timeEnd;
    int timeCost = 0;
    traceItemST thisUserTrace;
    set<opType> localOP = {op_close,op_closedir};
    //cout << "[USER]: Start" << endl;
    opType tempTraceOP;
    int use_count = 0;
    while (!trace_end || traceQueue.size() > 0)
    {
        if (traceQueue.size() > 0)
        {
            pthread_mutex_lock(&queue_edit_mutex);
            //pthread_cond_wait(&queue_edit_cond, &queue_edit_mutex);
            thisUserTrace = traceQueue.front();
            //handleTraceItem(thisUserTrace);
            use_count++;
            traceQueue.pop();
            pthread_mutex_unlock(&queue_edit_mutex);
            if (adder_sleep && traceQueue.size() < adderWakeThreshold)
            {
                    pthread_cond_signal(&queue_edit_cond);
                    adder_sleep = false;
            }
            //cout << "[USER]: replaying: " << use_count << endl;
            if (!check_uselessOP(thisUserTrace.traceOptype))
            {
                gettimeofday(&timeStart, NULL);
                handleTraceItem(thisUserTrace);
                //usleep(10000);
                gettimeofday(&timeEnd, NULL);
                timeCost = (timeEnd.tv_sec - timeStart.tv_sec)*1000000 + (timeEnd.tv_usec - timeStart.tv_usec);
                if(localOP.find(thisUserTrace.traceOptype) == localOP.end()){
                    //remove local op
                    logfile << thisUserTrace.opName << "\t" << timeStart.tv_sec << "." << timeStart.tv_usec << "\t" << timeEnd.tv_sec << "." << timeEnd.tv_usec << "\t" << timeCost << endl;
                }
            }
        }
        else
        {
            if (!trace_end && adder_sleep)
            {
                pthread_cond_signal(&queue_edit_cond);
                adder_sleep = false;
                //cout << "Wake up plz."<<endl;
            }else
            {
                //cout << "[USER--DEBUG]: No item in queue. "<<endl;
            }
            
        }
    }
    logfile.close();
    cout <<traceQueue.size() <<endl;
    //cout << "[USER--END] Used: " << use_count << endl;
}

void *traceAdder(void *adderParaPara)
{

    adderParaST adderPara = *(adderParaST *)adderParaPara;

    string tempLine;
    traceItemST thisAdderTrace;
    ifstream trace;
    int addCount = 0;
    //cout << "[ADDER]: Start to add trace:" << adderPara.tracePathPara << endl;
    trace.open(adderPara.tracePathPara.c_str(), ios::in);

    while (!trace.eof())
    {
        if (traceQueue.size() < adderSleepThreshold)
        {
            getline(trace, tempLine);
            if (tempLine.empty())
            {
                continue;
            }
            else
            {
                thisAdderTrace = getTrace(tempLine, adderPara.workPathPara);
                pthread_mutex_lock(&queue_edit_mutex);
                traceQueue.push(thisAdderTrace);
                addCount++;
                //cout << "[ADDER]: NO." << addCount << ":" << tempLine << endl;
                pthread_mutex_unlock(&queue_edit_mutex);
            }
        }
        else
        {
            adder_sleep = true;
            //cout << "[ADDER]: Sleep now." << endl;
            pthread_mutex_lock(&queue_edit_mutex);
            pthread_cond_wait(&queue_edit_cond, &queue_edit_mutex);
            //cout << "[ADDER]: Wake up." << endl;
            pthread_mutex_unlock(&queue_edit_mutex);
        }
    }
    trace_end = true;
    trace.close();
    //cout << "[ADDER--END] ADD: " << addCount << endl;
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
        if (strcmp(argv[4],"Debug")==0)
        {
            debug_mode = true;
            workerNo = "";
        }else
        {
            workerNo = string(argv[4]);
        }
        
    }
    string logDir = "/home/ceph/cyx/metadata-management/experiments/scripts/migrate/ai-training/traceLog/";
    
    vector<string> subTracePath = splitString_STL(tracePath, "/");
    //string workPath = "/mnt/ceph-client-1/trace/"+subTracePath.back();
    string workPath = workString + subTracePath.back();
    if (debug_mode)
    {
        cout << "[WAN] Debug Mode Now"<<endl;
        logDir = "/home/ceph/duo/traceReplay_multiThread/test/";

    }
    
    string logPath = logDir + *(splitString_STL(workString, "/").end() - 2) + workerNo + ".log";

    ofstream logFile;
    logFile.open(logPath.c_str(), ios::trunc);

    cout << "[INFO]: Current Log File: " << logPath << endl;

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

    pthread_mutex_init(&queue_edit_mutex, NULL);
    pthread_create(&user, NULL, traceAdder, &adderPara);

    traceUser(logFile);

    logFile.close();
    pthread_join(user, NULL);
    cout << "[MAIN--END]" << endl;

    return 0;
}
