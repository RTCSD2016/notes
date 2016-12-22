// Attempt at a simple client using the UDP protocol
// It sends integers and doubles to a server
// client1.cpp, created 12/4/96, DM Auslander

#include <winsock.h>                    // Winsock header file
#include <iostream.h>
#include <stdlib.h>
#include <conio.h>

#define PROG_NAME "Simple Client"

#define WINSOCK_VERSION 0x0101  // Program requires Winsock version 1.1
#define DEFAULT_PROTOCOL 0  // No protocol specified, use default
#define SEND_FLAGS 0  // No send() flags specified
#define RECV_FLAGS 0  // No recv() flags specified
#define SERVER_ADDRESS "136.152.66.28" // "10.0.2.15" //"128.32.142.37"

      // Data block codes. Each block starts with a block-type code (char)
      // then the number of values, if necessary (short)
      // These codes can also be used for structures such as the
      //   standard message.

      //  Block codes:
      //    Single values (value follows directly)
      //    0  unsigned char
      //    1  short
      //    2  long
      //    3  float
      //    4  double
      //    5  unsigned short
      //    6  unsigned long
      //    Arrays (number of values is next)
      //    7  unsigned chars
      //    8  shorts
      //    9  longs
      //    10  floats
      //    11  doubles
      //    12  unsigned shorts
      //    13  unsigned longs
      //    Structures
      //    10  standard message (...)

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

int InsertShortValue(unsigned char *packet_array,int npack,int value)
    {
    union pack_short ps;
    int i = npack;

    if((int)((short)value) != value)
        {
        cout << "<InsertShortValue> Truncation error\n";
        exit(6);
        }
    packet_array[i++] = 1;  // Code for single short value
    ps.s = (short)value;
    packet_array[i++] = ps.c[0];
    packet_array[i++] = ps.c[1];
    return(i);
    }

int InsertDoubleValue(unsigned char *packet_array,int npack,double value)
    {
    union pack_double pd;
    int i = npack;

    packet_array[i++] = 4;  // Code for single double value
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

    packet_array[i++] = 11;  // Code for array of doubles

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


void main(void)
    {
    WSADATA wsaData;                            // Winsock implementation details
    SOCKET nSocket;                       // Socket handle used by this program
    SOCKADDR_IN sockAddr,SendSockAddr;  // Socket address structure

    #define MESSAGE_BUFFER_SIZE 5000
      unsigned char MessageBuffer[MESSAGE_BUFFER_SIZE]; // Buffer to hold
             // outgoing information
    int nbuf;  // Number of characters in buffer
    int nCharSent;          // Number of characters transmitted
    int nBind;          // Result value from bind()
    DWORD ServAddr;
    double vv[2] = {1.3,0.00521};  // Sample data to send

    if (WSAStartup(WINSOCK_VERSION, &wsaData))
        {
        cout << "Could not load Windows Sockets DLL." <<
                    PROG_NAME <<  "\n";
        exit(1);
        }

    ServAddr = inet_addr(SERVER_ADDRESS);
    cout << "ServAddr " << ServAddr << "\n";

    nSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (nSocket == INVALID_SOCKET)
        {
        cout << "Invalid socket!" << PROG_NAME << "\n";
        exit(2);
        }
    // Configure the socket


    // Define the socket address
    sockAddr.sin_family = AF_INET;  // Internet address family
    sockAddr.sin_port = 1235;
    sockAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    nBind = bind(nSocket,(LPSOCKADDR) &sockAddr,sizeof(sockAddr));
    if(nBind == SOCKET_ERROR)
       {
       cout << "Can't Bind, nBind= " << nBind  << "\n";
       exit(3);
       }
    cout << " nBind= " << nBind  << "\n";
    // Send a message
    SendSockAddr.sin_family = AF_INET;  // Internet address family
    SendSockAddr.sin_port = 1234;
    SendSockAddr.sin_addr.s_addr = ServAddr;

    for(int i = 0; i < 3; i++)
      {
       // Send several packets
          // Initial value in buffer indicates the type of data coding used
      nbuf = 0; // Count number of characters in the buffer
      MessageBuffer[nbuf++] = 0; // Code indicating data is in Intelx86
         // (Borland) format

      nbuf = InsertShortValue(MessageBuffer,nbuf,14*(i + 1));
      nbuf = InsertDoubleValue(MessageBuffer,nbuf,234.56 * (i + 1));
      vv[0] *= (i + 1);
      vv[1] *= (i + 1);
      nbuf = InsertDoubleArray(MessageBuffer,nbuf,vv,2);
      cout << "nbuf " << nbuf << " ";

      nCharSent = sendto(nSocket,(char *)MessageBuffer,nbuf,
        SEND_FLAGS,(LPSOCKADDR)&SendSockAddr,sizeof(SendSockAddr));
      cout << "nCharSent " << nCharSent << "\n";
      }
    WSACleanup();   // Free all allocated program resources and exit
    cout << "Hit any key to exit.\n";
    while(!kbhit()) ;  // Wait to exit
    return;
    }

