#include "traceItem/traceItem.h"
#include "utils/traceUtils.h"

using namespace std;

bool initMode = true;


int main(int argc,char **argv){
    if (argc != 4 && argc != 5 ) {
        cout<< "Parameter: 1.Trace Path 2.Mode 3.Work Path(end with /), 4.No" <<endl;
        return 0;
	}
    string tracePath = string(argv[1]);
    string modeString = string(argv[2]);
    string workString = string(argv[3]);
	string workerNo;
    int wrongTraceCount = 0;
	if(argc == 4){
		workerNo = "";
	}else{
		workerNo = string(argv[4]);
	}
    string logDir = "/home/ceph/cyx/metadata-management/experiments/scripts/migrate/ai-training/traceLog/";

    string logPath = logDir + *(splitString_STL(workString, "/").end()-2)+workerNo+".log";
    ofstream logFile;
    logFile.open(logPath.c_str(),ios::trunc);

    if (modeString == "init")
    {
        initMode = true;
    }else if (modeString == "run")
    {
        initMode = false;
    }else
    {
        cout<< "Mode Value: 1.init 2.run"<<endl;
        return 0;
    }
        
    vector<string>subTracePath = splitString_STL(tracePath, "/");
    //string workPath = "/mnt/ceph-client-1/trace/"+subTracePath.back();
    string workPath = workString+subTracePath.back();
    string thisLine;
    ifstream  trace;
    trace.open(tracePath.c_str(), ios::in);    
    traceItemST tempTrace;
    vector<traceItemST>traceVector;
    struct timeval timeStart;
    struct timeval timeEnd;
    int timeCost = 0;
    while (! trace.eof() ){
    getline(trace, thisLine);
    tempTrace=getTrace(thisLine, workPath);
    traceVector.push_back(tempTrace);
    }
    trace.close();
    
    cout << "******Read trace finished, start replaying now******"<<endl;

    int tracePos;
    opType traceOP;
    set<opType>uselessOP {op_error,op_emptyline,op_flush,op_getattr,op_read,op_write,op_symlink,op_close,op_closedir};
    for(int tracePos = 0; tracePos<traceVector.size(); tracePos++)
    {
    if (initMode){
        initFileSet(traceVector[tracePos]);
    }else{
        //cout<<tempTrace.filePath<<" "<< tempTrace.opName << endl;
        traceOP = traceVector[tracePos].traceOptype;
        gettimeofday(&timeStart,NULL); 
        handleTraceItem(traceVector[tracePos]);
        gettimeofday(&timeEnd, NULL);
        timeCost = (timeEnd.tv_sec - timeStart.tv_sec)*1000000 + (timeEnd.tv_usec - timeStart.tv_usec);
        if (uselessOP.find(traceOP) == uselessOP.end())
        {
        logFile<< traceVector[tracePos].opName << "\t" << timeStart.tv_sec<<"."<<timeStart.tv_usec<<"\t" << timeEnd.tv_sec<<"."<<timeEnd.tv_usec<< "\t" << timeCost <<endl;
        }
    }
    }
    
    if (initMode)
    {
        wrongTraceCount = genFromFileSet();
    }
    
    cout << "handle: "<<wrongTraceCount<<"wrong trace."<<endl;
    
    return 0;
}
