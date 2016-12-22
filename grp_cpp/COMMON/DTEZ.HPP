#ifndef DTEZ_CPP
extern int d_output;
#endif

#define     IO_BASE             0x02EC
#define     CONTROL             IO_BASE + 1
#define     STATUS              IO_BASE + 1
#define     DATA_IN             IO_BASE
#define     DATA_OUT            IO_BASE

/*      CONTROL BITS                */
#define     OUT_READY           0x01
#define     DATA_IN_FULL        0x02
#define     READY               0x04
#define     COMMAND             0x08

/*      4 BIT OPCODES               */
#define     RESET               0x00        
#define     CLEAR_ERROR         0x01
#define     READ_ERROR          0x02
#define     SET_CLOCK           0x03
#define     SET_D_IN            0x04        
#define     SET_D_OUT           0x05
#define     READ_D_IN           0x06
#define     WRITE_D_OUT         0x07
#define     WRITE_D2A           0x08
#define     SET_D2A             0x09        
#define     TEST                0x0B        
#define     READ_A2D            0x0C
#define     SET_A2D             0x0D
#define     STOP                0x0F        

extern "C"
{
void io_init(void);
int d_in(void);
void d_out(int data);
double a2d(int ch_no);
void d2a(int ch_no, double volts);
void bit_out(int bit_no, int data); 
int bit_in(int bit_no);
}