// Header file for Internet/UDP functions
// File: net_udp.hpp

void NetCleanUp(void);
void InitUDP(int NumProc,int ThisProc,char *Addr[]);
void StoreMessage(int ToProc,int ToTask,int BoxNo,int FromProc,
     int FromTask,int MsgFlag,double MsgValue,int RetRecpt,int DelMode);
void StoreSharedData(int ToProc,int ToTask,int n,double *v);
int InsertSharedArray(int ToProc,int ToTask,int nValues,double *Val);
int InsertMessage(int ToProc,int ToTask,int BoxNo,int FromProc,
                    int FromTask,int MsgFlag,double MsgValue,
                    int RetRecpt,int DelMode);
int SendData(void);
int CheckNet(void);

