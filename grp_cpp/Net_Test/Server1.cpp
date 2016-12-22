// Sample of a simple server using the UDP protocol
// server1.cpp, created 12/4/96, DM Auslander

#include <winsock.h>                    // Winsock header file
#include <iostream.h>
#include <stdlib.h>
#include <conio.h>

#define PROG_NAME "Simple Server"

#define WINSOCK_VERSION 0x0101      // Program requires Winsock version 1.1
#define DEFAULT_PROTOCOL 0              // No protocol specified, use default
#define SEND_FLAGS 0                            // No send() flags specified
#define RECV_FLAGS 0                            // No recv() flags specified

      // Data block codes. Each block starts with a block-type code (char)
      // then the number of values, if necessary (short)
      // These codes can also be used for structures such as the
      //   standard message.

      //  Block codes:
      //    Single values (value follows directly)
       //       0  unsigned char
      //    1  short
      //    2  long
      //    3  float
      //    4  double
      //    5  unsigned short
      //    6  unsigned long
      //    Arrays (number of values is next)
       //       7  unsigned chars
      //    8  shorts
      //    9  longs
      //    10  floats
      //    11  doubles
      //    12  unsigned shorts
      //    13  unsigned longs
      //    Structures
      //    10  standard message (...)


// Functions to extract information from a packet

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
        double *pvalue,int *pnval)
    {
    int i = next;
    int nval,j;
    double v;

    i = ExtractShortValue(packet_array,i,&nval);  // Get number of values
    *pnval = nval;
    for(j = 0; j < nval; j++)
        {
        i = ExtractDoubleValue(packet_array,i,&v);
        pvalue[j] = v;  // store array value
        }
    return(i);
    }


int ParseMessage(unsigned char *Buf,int nCharRecv)
    {
    // Break out component parts of the message and print results
    // Return 0 for OK, -1 for error
    int nused = 0;  // Number of characters used in decoding buffer

    if(Buf[nused++] != 0)
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
            case 0:  // unsigned char
            cout << "Unsigned char = " << (int)Buf[nused++] << " ";
            break;

            case 1:  // short
                {
                int value;

                nused = ExtractShortValue(Buf,nused,&value);
                cout << "Short = " << value << " ";
                break;
                }

            case 4:  // double
                {
                double value;

                nused = ExtractDoubleValue(Buf,nused,&value);
                cout << "Double = " << value << " ";
                break;
                }

            case 11:  // array of doubles
                {
                int n;
                double v[10];

                nused = ExtractDoubleArray(Buf,nused,v,&n);
                cout << "Array of Doubles ";
                for(int i = 0;i < n; i++)cout << v[i] << " ";
                break;
                }

            default:
                // No other type implemented yet!
                cout << "\nUnknown Block Type\n";
                return(-1);
            }
        }
   cout << "\nEnd of Packet\n";
   return(0);
   }

void main(void)
    {
    WSADATA wsaData;  // Winsock implementation details
    SOCKET nSocket;   // Socket handle used by this program
    SOCKADDR_IN sockAddr,RecvSockAddr; // Socket address structure
    #define MESSAGE_BUFFER_SIZE 5000
    unsigned char MessageBuffer[MESSAGE_BUFFER_SIZE]; // Buffer to hold
           // incoming information

    union  // To get dotted-decimal addresses from 32-bit addresses
       {
       u_long IPAddr;
       struct
          {
          u_char b1,b2,b3,b4;
          }bbb;
       }AddrCon;

    int nCharRecv;   // Number of characters received
    int nBind;          // Result value from bind()

    if (WSAStartup(WINSOCK_VERSION, &wsaData))
        {
        cout << "Could not load Windows Sockets DLL." <<
                    PROG_NAME << "\n";
        exit(1);
        }


    nSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (nSocket == INVALID_SOCKET)
        {
        cout << "Invalid socket!!" << PROG_NAME << "\n";
        exit(2);
        }
    // Configure the socket


    // Define the socket address
    sockAddr.sin_family = AF_INET;  // Internet address family
    sockAddr.sin_port = 1234;
    sockAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    nBind = bind(nSocket,(LPSOCKADDR) &sockAddr,sizeof(sockAddr));
    if(nBind == SOCKET_ERROR)
       {
       cout << "Can't Bind, nBind= " << nBind  << "\n";
       exit(3);
       }
    cout << " nBind= " << nBind  << "\n";

    // Set socket to non-blocking
    unsigned long NoBlock = TRUE,BytesToRead = 0;
    int RetVal = ioctlsocket(nSocket, FIONBIO, &NoBlock);
    if(RetVal == SOCKET_ERROR)
       {
       cout << " Error in setting socket to non-blocking\n";
       exit(4);
       }

    cout << "Hit any key to exit.\n";

    while(1)  // Infinite loop (exit by hitting any key)
        {
        do    // Wait for message to arrive
            {
            if(kbhit())
               {
               cout << "User-requested exit\n";
                    WSACleanup();   // Free all allocated program resources and exit
               exit(0);
               }
            RetVal = ioctlsocket(nSocket, FIONREAD, &BytesToRead);
            if(RetVal == SOCKET_ERROR)
                {
                cout << "Error waiting for message\n";
                exit(5);
                }
            }while(BytesToRead <= 0);

        // Get the message
        int lenRecvSockAddr = sizeof(RecvSockAddr);
        nCharRecv = recvfrom(nSocket,(char *)MessageBuffer,MESSAGE_BUFFER_SIZE,
           RECV_FLAGS,(LPSOCKADDR)&RecvSockAddr,&lenRecvSockAddr);
        cout << "nCharRecv " << nCharRecv << "\n";

        // Decode sender address
        AddrCon.IPAddr = RecvSockAddr.sin_addr.s_addr;
        cout << "Sender Address " << (int)AddrCon.bbb.b1
          << " " << (int)AddrCon.bbb.b2 << " "
        << (int)AddrCon.bbb.b3 << " " << (int)AddrCon.bbb.b4 << " " << "\n";
          // Parse the message into component values
        if(ParseMessage(MessageBuffer,nCharRecv) < 0)break;
        }
    WSACleanup();   // Free all allocated program resources and exit
    cout << "Hit any key to exit.\n";
    while(!kbhit()) ;  // Wait to exit
    }

