// Test ftime() for time telling

#include <iostream.h>
#include <sys\timeb.h>

main(void)
    {
    double time,timenext = 0.0,time0;
    double delta_time = 0.5;
    struct timeb tt;

    ftime(&tt);
    time0 = tt.time + (double)tt.millitm / 1000.0;

    while(1)
        {
        ftime(&tt);
        time = tt.time + (double)tt.millitm / 1000.0 - time0;
        if(timenext <= time)
            {
            cout << timenext << "\n";
            timenext += delta_time;
            }
        }
    }

