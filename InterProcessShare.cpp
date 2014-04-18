////////////////////////////////////////////////////////////////////////////
// InterProcessShare.cpp -- Win32 interprocess shared data
// This file is part of MZC3.  See file "ReadMe.txt" and "License.txt".
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////

#ifdef UNITTEST
    // unit test and example
    #include <cstdio>
    using namespace std;
    int main(void)
    {
        MInterProcessShare<int> share;
        share.Create(TEXT("MInterProcessShare test data"));

        int n;
        do
        {
            int *p = share.Lock();
            n = 0;
            if (p)
            {
                n = (*p)++;
                share.Unlock();
            }
            printf("%d\n", n);
            ::Sleep(1000);
        } while (n < 30);
        return 0;
    }
#endif
