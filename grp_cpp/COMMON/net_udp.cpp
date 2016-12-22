// TCP/IP network functions for use with mechatronics control programs
// Created 12/30/96, DM Auslander
//  These functions are based on User Datagram Protocol (UDP)

#include <winsock.h>                    // Winsock header file
#include <iostream.h>
#include <stdlib.h>
#include <conio.h>
#include <sys\timeb.h>  // for ftime()
#include "net_udp.hpp"

#define WINSOCK_VERSION 0x0101  // Program requires Winsock version 1.1
#define SEND_FLAGS 0   // No send() flags specified
#define RECV_FLAGS 0   // No recv() flags specified
#define BASE_PORT 1234  // Base address for ports
#define INTELx86 0  // Data format code

static SOCKET nSocket;   // Socket handle used by this program

// Packet information
#define MAX_PROC 50
#define PACKET_BUFFER_SIZE 5000
#define ADDRESS_SIZE 20

static unsigned char *pPacketBuffer[MAX_PROC];  // Pointers to buffers
        // for outgoing packets
static int PacketNext[MAX_PROC];  // Indexes into packet buffers
static int ThisProcess,NumProc;
static unsigned long ProcessAddress[MAX_PROC];  // Internet addresses
static int ProcessConnected[MAX_PROC];  // Indicates when process is
    // connected to net
static int NetConnected;

// Unions used for coding/decoding packets
union pack_short
    {
    short s;
    unsigned char c[2];
    };

union pack_double
    {
    double d;
    unsigned char c[8];
    };


      // Data block codes. Only two types of blocks are recognized
      // for control programs: messages and shared data. Any number
      // of these can be mixed in a single packet.

      //  Block codes:
      //    Structures
      //      1  standard message
      //      2  shared data array

// Functions to extract information from a packet

int ExtractShortValue(unsigned char *packet_array,int next,int *pvalue)
    {
    union pack_short pp;
    int i = next;

    pp.c[0] = packet_array[i++];
    pp.c[1] = packet_array[i++];
    *pvalue = (int)pp.s;
    return(i);
    }

int ExtractDoubleValue(unsigned char *packet_array,int next,double *pvalue)
    {
    union pack_double pp;
    int i = next;

    for(int j = 0;j < 8; j++)pp.c[j] = packet_array[i++];
    *pvalue = pp.d;
    return(i);
    }

int ExtractDoubleArray(unsigned char *packet_array,int next,
        double **pvalue,int *pnval,int *size)
    {
    int i = next;
    int nval,j;
    double v,*pv;

    i = ExtractShortValue(packet_array,i,&nval);  // Get number of values
    *pnval = nval;
    if(nval > *size)
        {
        if(*size > 0)delete [] pvalue;   // Delete old array
        *pvalue = new double [nval + 20];  // Create larger array
        *size = nval + 20;  // Return new size value
        }

    pv = *pvalue;
    for(j = 0; j < nval; j++)
        {
        i = ExtractDoubleValue(packet_array,i,&v);
        pv[j] = v;  // store array value
        }
    return(i);
    }

// Storage for double array
static int vSize = 0;  // Initial size of array
static double *v;

int ParseMessage(unsigned char *Buf,int nCharRecv)
    {
    // Break out component parts of the message and print results
    // Return 0 for OK, -1 for error
    int nused = 0;  // Number of characters used in decoding buffer

    if(Buf[nused++] != INTELx86)
        {
        // Incorrect data format (Only Intelx86 recognized here)
        cout << "<ParseMessage> Incorrect data format, 1st char = "
        << (int)Buf[0] << "\n";
        return(-1);
        }
    while(nused < nCharRecv)
        {
        // Continue this loop until the message is entirely consumed
        int BlockType = Buf[nused++];

        switch(BlockType)
            {
            case 1:  // Message block
                {
                int ToProc,ToTask,BoxNo,FromProc,FromTask;
                int MsgFlag,RetRecpt,DelMode;
                double MsgValue;

                nused = ExtractShortValue(Buf,nused,&ToProc);
                nused = ExtractShortValue(Buf,nused,&ToTask);
                nused = ExtractShortValue(Buf,nused,&BoxNo);
                nused = ExtractShortValue(Buf,nused,&FromProc);
                nused = ExtractShortValue(Buf,nused,&FromTask);
                nused = ExtractShortValue(Buf,nused,&MsgFlag);
                nused = ExtractDoubleValue(Buf,nused,&MsgValue);
                nused = ExtractShortValue(Buf,nused,&RetRecpt);
                nused = ExtractShortValue(Buf,nused,&DelMode);

                StoreMessage(ToProc,ToTask,BoxNo,FromProc,
                    FromTask,MsgFlag,MsgValue,RetRecpt,DelMode);
                break;
                }

            case 2:  // Shared value array
                {
                int n,ToProc,ToTask;

                nused = ExtractShortValue(Buf,nused,&ToProc);
                nused = ExtractShortValue(Buf,nused,&ToTask);
                nused = ExtractDoubleArray(Buf,nused,&v,&n,&vSize);

                StoreSharedData(ToProc,ToTask,n,v);
                break;
                }

            case 100:  // Are-you-there packet
                {
                unsigned char *pbuf;
                int iNext;

                if(Buf[nused++] != ThisProcess)
                    {
                    cout << "<Parse> Incorrect proc # in ARE-YOU-THERE\n";
                    return(-1);
                    }
                // Respond to Process 0 with I-AM-HERE
                pbuf = pPacketBuffer[0]; // Pointer to buffer
                iNext = PacketNext[0];  // Next free slot
                  // 1st byte for Intel data format (already there)
                pbuf[iNext++] = 101;  // 2nd byte indicates I-AM-HERE
                pbuf[iNext++] = (unsigned char)ThisProcess;
                  // 3rd byte has number of this process
                PacketNext[0] = iNext;

                if(SendData() != 0)  // Send out Packets
                    {
                    cout << "<ContactProcesses> Error sending I-AM-HERE data\n";
                    return(1);
                    }
                break;
                }

            case 101:  // I-AM-HERE packet
                {
                int FromProc;

                FromProc = Buf[nused++];
                if((FromProc < 0) || (FromProc >= NumProc))
                    {
                    cout << "<Parse> Invalid ProcNo in I-AM-HERE "
                        << FromProc << "\n";
                    return(1);
                    }
                ProcessConnected[FromProc] = 1;
                break;
                }

            case 102:  // START packet
                {
                if(Buf[nused++] != ThisProcess)
                    {
                    cout << "<Parse> Incorrect proc # in ARE-YOU-THERE\n";
                    return(-1);
                    }

                NetConnected = 1;
                break;
                }

            default:
                // No other type implemented
                cout << "\nUnknown Block Type\n";
                return(-1);
            }
        }
   //cout << "\nEnd of Packet\n";
   return(0);
   }

// Functions for assembling a packet

int InsertShortValue(unsigned char *packet_array,int npack,int value)
    {
    union pack_short ps;
    int i = npack;

    if((int)((short)value) != value)
        {
        cout << "<InsertShortValue> Truncation error\n";
        exit(6);
        }
    ps.s = (short)value;
    packet_array[i++] = ps.c[0];
    packet_array[i++] = ps.c[1];
    return(i);
    }

int InsertDoubleValue(unsigned char *packet_array,int npack,double value)
    {
    union pack_double pd;
    int i = npack;

    pd.d = value;
    for(int j = 0; j < 8; j++)packet_array[i++] = pd.c[j];
    return(i);
    }

int InsertDoubleArray(unsigned char *packet_array,int npack,
        double *array,int nval)
    {
    union pack_double pd;
    union pack_short ps;
    int i = npack;

    ps.s = (short)nval;    // Enter number of values in array
    packet_array[i++] = ps.c[0];
    packet_array[i++] = ps.c[1];

    for(int k = 0; k < nval; k++)  // Enter the array values
        {
        pd.d = array[k];
        for(int j = 0; j < 8; j++)packet_array[i++] = pd.c[j];
        }
    return(i);
    }


union AddressConvert // To get dotted-decimal addresses
        // from 32-bit addresses, or vice-versa
   {
   u_long IPAddr;
   struct
      {
      u_char b1,b2,b3,b4;
      }bbb;
   };

int ResetPacket(int iProc)
    {
    unsigned char *pc;

    // Returns error: 1 for error, 0 for OK
    if(iProc == ThisProcess)
        {
        // No packet buffer needed for this process
        pPacketBuffer[iProc] = NULL;
        return(0);
        }
    if(pPacketBuffer[iProc] == NULL)
        {
        // Allocate new memory
        pc = new unsigned char [PACKET_BUFFER_SIZE];
        if(pc == NULL)
            {
           cout << "<ResetPacket> Can't allocate packet buffer\n";
           return(1);
           }
        pPacketBuffer[iProc] = pc;  // Pointer to space for packet assembly
        }
    pc = pPacketBuffer[iProc];
    pc[0] = INTELx86;  // Data format code
    PacketNext[iProc] = 1;  // First character is always the data format
    return(0);
    }

void NetCleanUp(void)
    {
    if(vSize > 0)delete [] v;  // Free memory used for buffer
    WSACleanup();   // Free all allocated program resources
    // Free memory used for packet buffers
    for(int i = 0; i < NumProc; i++)
        {
        if(pPacketBuffer[i] != NULL)
            delete [] pPacketBuffer[i];
        }
    }

int ContactProcesses(void)
    {
    // Make sure all processes are on line before proceeding
    // Process #0 is the master for this procedure
    // It sends out probe packets to the other processes.
    // When all processes have responded it sends out a
    //   START signal to all other processes

    // Returns error: 1 for error, 0 for OK

    // Probe packets are sent out at timed intervals to avoid
    // network overload.  The generic ftime() is used for timing
    // so these functions can be tested independently of other
    // control programs.

    double Time0,Time,TimeNext;
    double DeltaTime = 0.5;  // Seconds
    struct timeb tt;
    unsigned char *pbuf;
    int AllConnected,iNext;

    ftime(&tt);  // Establish the base time
    Time0 = tt.time + (double)tt.millitm / 1000.0;
    TimeNext = DeltaTime;
    
    // This is the master for the initialization procedure
    // In the first phase, send out an "are-you-there" packet
    // to all processes that have not yet responded.
    // When all processes have responded, send out a "start"
    // packet to all processes

    for(;;)
        {
        // Stay in this loop until all processes have been connected
        // Send out "are-you-there" packets
        for(int i = 0; i < NumProc; i++)
            {
            if(i == ThisProcess)continue;  // Don't send to self
            if(ProcessConnected[i] == 1)continue;

            pbuf = pPacketBuffer[i]; // Pointer to buffer
            iNext = PacketNext[i];  // Next free slot
             // 1st byte for Intel data format (already there)
            pbuf[iNext++] = 100;  // 2nd byte indicates are-you-there
            pbuf[iNext++] = (unsigned char)i;  // 3rd byte has number of target process
            PacketNext[i] = iNext;
            }
        if(SendData() != 0)  // Send out Packets
            {
            cout << "<ContactProcesses> Error sending ARE-YOU-THERE data\n";
            return(1);
            }
        // Check to see if all processes are connected
        // After DeltaTime seconds, go back and send out ARE-YOU-THEREs again
        do
            {
            if(CheckNet() != 0)
                {
                cout << "<ContactProcesses> Error checking data\n";
                return(1);
                }
            AllConnected = 1;
            for(int i = 0; i < NumProc; i++)
                {
                if(ProcessConnected[i] != 1)AllConnected = 0;
                }
            if(AllConnected)break;
            ftime(&tt);  // Establish the base time
            Time = tt.time + (double)tt.millitm / 1000.0 - Time0;
            }while(Time < TimeNext);

        if(AllConnected)break;  // Done with connect loop
        TimeNext += DeltaTime;
        }
    // Send out START signals
    for(int i = 0; i < NumProc; i++)
        {
        if(i == ThisProcess)continue;  // Don't send to self

        pbuf = pPacketBuffer[i]; // Pointer to buffer
        iNext = PacketNext[i];  // Next free slot
          // 1st byte for Intel data format (already there)
        pbuf[iNext++] = 102;  // 2nd byte indicates START
        pbuf[iNext++] = (unsigned char)i;  // 3rd byte has number of target process
        PacketNext[i] = iNext;
        }
    if(SendData() != 0)  // Send out Packets
        {
        cout << "<ContactProcesses> Error sending START data\n";
        return(1);
        }
    NetConnected = 1;  // All processes are now on line
    return(0);  // Normal return
    }

void ErrorExit(int n)
    {
    cout << "Error Exit #" << n << "\n";
    cout << "Hit any key to exit.\n";
    while(!kbhit()) ;
    exit(1);
    }

void InitUDP(int NumberProc,int ThisProc,char *Addr[])
    // Initialize a socket
    {
    WSADATA wsaData;  // Winsock implementation details
    SOCKADDR_IN sockAddr; // Socket address structure
    // union AddressConvert AddrCon;
    int nBind;          // Result value from bind()

    ThisProcess = ThisProc;  // Record process # for this process
    NumProc = NumberProc;

    if(NumProc > MAX_PROC)
        {
        cout << "<InitUDP> Too many processes.\n";
        ErrorExit(2);
        }

    if (WSAStartup(WINSOCK_VERSION, &wsaData))
        {
        cout << "<InitUDP> Could not load WinSock DLL.\n";
        ErrorExit(1);
        }

    nSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (nSocket == INVALID_SOCKET)
        {
        cout << "<InitUDP> Invalid socket!!\n";
        NetCleanUp();
        ErrorExit(2);
        }
    // Define the socket address
    sockAddr.sin_family = AF_INET;  // Internet address family
    sockAddr.sin_port = u_short(BASE_PORT + ThisProc);
       // Define the port for this process
    sockAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    nBind = bind(nSocket,(LPSOCKADDR) &sockAddr,sizeof(sockAddr));
    if(nBind == SOCKET_ERROR)
       {
       cout << "Can't Bind, nBind= " << nBind  << "\n";
       NetCleanUp();
       ErrorExit(3);
       }

    // Set socket to non-blocking
    unsigned long NoBlock = TRUE;
    int RetVal = ioctlsocket(nSocket, FIONBIO, &NoBlock);
    if(RetVal == SOCKET_ERROR)
       {
       cout << "<InitUDP> Error in setting socket to non-blocking\n";
       NetCleanUp();
       ErrorExit(4);
       }
    // Allocate memory for packet buffers and convert addresses
    for(int i = 0; i < NumProc; i++)
        {
        if(ResetPacket(i) != 0)
            {
            NetCleanUp();
            ErrorExit(4);
            }
        ProcessAddress[i] = inet_addr(Addr[i]);  // Convert address
        ProcessConnected[i] = 0;  // No processes connected yet
        }
    ProcessConnected[ThisProc] = 1;  // Connection flag for this process

    NetConnected = 0; // Flag is TRUE when all processes are on line
    if(ThisProcess == 0)
        {
        // Process #0 masters the connect procedure
        if(ContactProcesses() != 0)
            {
            cout << "<InitUDP> Couldn't contact other processes\n";
            NetCleanUp();
            ErrorExit(4);
            }
        }
    else
        {
        // Wait for START signal (also responds to ARE-YOU-THERE signals)
        do
            {
            if(CheckNet() != 0)
                {
                cout << "<InitUDP> Error in waiting for START\n";
                ErrorExit(8);
                }
            }while(NetConnected != 1);
        }
    cout << "All processes are on line.\n";
    }

int CheckNet(void)
    // Process any data that has come in from the network
    // Returns error: 0 for OK, 1 for error
    {
    SOCKADDR_IN RecvSockAddr; // Socket address structure
    unsigned char MessageBuffer[PACKET_BUFFER_SIZE]; // Buffer to hold
           // incoming information
    int nCharRecv;   // Number of characters received
    int RetVal;
    unsigned long BytesToRead;

    while(1)  // Infinite loop -- return when out of network data
        {
        RetVal = ioctlsocket(nSocket, FIONREAD, &BytesToRead);
        if(RetVal == SOCKET_ERROR)
            {
            cout << "<CheckNet>Error waiting for message\n";
            return(1);
            }
        if(BytesToRead <= 0)return(0);  // No more data available

        // Get a packet
        int lenRecvSockAddr = sizeof(RecvSockAddr);
        nCharRecv = recvfrom(nSocket,(char *)MessageBuffer,PACKET_BUFFER_SIZE,
           RECV_FLAGS,(LPSOCKADDR)&RecvSockAddr,&lenRecvSockAddr);

        //cout << "nCharRecv " << nCharRecv << "\n";
        //for(int j = 0; j < 10; j++)cout << (int)MessageBuffer[j] << " ";
        //cout << "\n";

        // Parse the message into component values
        if(ParseMessage(MessageBuffer,nCharRecv) < 0)break;
        }
    cout << "<CheckNet>Unexpected break from process loop\n";
    return(1);
    }

int SendData(void)
    {
    // Send out any data currently buffered in packets
    // Returns error: 0 for OK, 1 for error

    SOCKADDR_IN SendSockAddr;  // Socket address structure

    unsigned char MessageBuffer[PACKET_BUFFER_SIZE]; // Buffer to hold
             // outgoing information
    int nbuf;  // Number of characters in buffer
    int nCharSent;          // Number of characters transmitted


    for(int i = 0; i < NumProc; i++)
        {
        // Cycle through all processes
        if((i == ThisProcess) || (PacketNext[i] <= 0))continue;
        // Don't send data to "self"; skip empty packets

        SendSockAddr.sin_family = AF_INET;  // Internet address family
        SendSockAddr.sin_port = u_short(BASE_PORT + i);
        SendSockAddr.sin_addr.s_addr = ProcessAddress[i];

        // Copy packet data to local buffer --
        // This should be done as critical region
        nbuf = PacketNext[i]; // size of this packet
        unsigned char *pc = pPacketBuffer[i];  // Pointer to packet
        for(int j = 0; j < nbuf; j++)MessageBuffer[j] = pc[j];
        ResetPacket(i);  // This packet is now available for refilling
        // Critical region can end here

        nCharSent = sendto(nSocket,(char *)MessageBuffer,nbuf,
          SEND_FLAGS,(LPSOCKADDR)&SendSockAddr,sizeof(SendSockAddr));

        //cout << "nCharSent " << nCharSent << "\n";
        //for(int j = 0; j < 10; j++)cout << (int)MessageBuffer[j] << " ";
        //cout << "\n";

        if(nCharSent != nbuf)
            {
            cout << "<SendData> Error sending packet\n";
            return(1);
            }
        }
    return(0);  // Normal return
    }

int InsertMessage(int ToProc,int ToTask,int BoxNo,int FromProc,
                    int FromTask,int MsgFlag,double MsgValue,
                    int RetRecpt,int DelMode)
    {
    // Returns error: 1 for error, 0 for OK
    // Add a message to the appropriate process packet

    if(ToProc == ThisProcess)
        {
        cout << "<InsertMessage> Messages to this process\n"
            << "Should have been handled locally not in network\n";
        return(1);
        }
    int next = PacketNext[ToProc];
    if((next + 25) > PACKET_BUFFER_SIZE)
        {
        cout << "<InsertMessage> Packet overflow, process # "
           << ToProc << "\n";
        return(1);
        }

    unsigned char *pc = pPacketBuffer[ToProc];  // Pointer to buffer

    pc[next++] = 1;  // Code for message block
    next = InsertShortValue(pc,next,ToProc);
    next = InsertShortValue(pc,next,ToTask);
    next = InsertShortValue(pc,next,BoxNo);
    next = InsertShortValue(pc,next,FromProc);
    next = InsertShortValue(pc,next,FromTask);
    next = InsertShortValue(pc,next,MsgFlag);
    next = InsertDoubleValue(pc,next,MsgValue);
    next = InsertShortValue(pc,next,RetRecpt);
    next = InsertShortValue(pc,next,DelMode);

    PacketNext[ToProc] = next;  // Update packet index
    return(0);
    }

int InsertSharedArray(int ToProc,int ToTask,int nValues,double *Val)
    {
    // Returns error: 1 for error, 0 for OK

    if(ToProc == ThisProcess)
        {
        cout << "<InsertSharedArray> Data for this process\n"
            << "Should have been handled locally not in network\n";
        return(1);
        }
    int next = PacketNext[ToProc];
    if((next + 7 + nValues * sizeof(double)) > PACKET_BUFFER_SIZE)
        {
        cout << "<InsertSharedArray> Packet overflow, process # "
            << ToProc << "\n";
        return(1);
        }

    unsigned char *pc = pPacketBuffer[ToProc];  // Pointer to buffer

    pc[next++] = 2;  // Code for shared data array
    next = InsertShortValue(pc,next,ToProc);
    next = InsertShortValue(pc,next,ToTask);
    next = InsertDoubleArray(pc,next,Val,nValues);

    PacketNext[ToProc] = next;  // Update packet index
    return(0);
    }

