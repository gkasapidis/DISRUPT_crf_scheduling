//
// Created by gregkwaste on 5/6/19.
//

#include "planning_horizon.h"

void GetEasterSunday( int wYear, int& wMonth, int& wDay )
{
    // calculate easter sunday
    // [in]  wYear  - 4-digit year to calculate (but not before 1583)
    // [out] wMonth - month of easter sunday
    // [out] wDay   - day of easter sunday

    int wCorrection = 0;

    if( wYear < 1700 )      wCorrection = 4;
    else if( wYear < 1800 ) wCorrection = 5;
    else if( wYear < 1900 ) wCorrection = 6;
    else if( wYear < 2100 ) wCorrection = 0;
    else if( wYear < 2200 ) wCorrection = 1;
    else if( wYear < 2300 ) wCorrection = 2;
    else if( wYear < 2500 ) wCorrection = 3;

    wDay = (19 * (wYear % 19) + 24) % 30;
    wDay = 22 + wDay + ((2 * (wYear % 4) + 4 * (wYear % 7) + 6 * wDay + 5 + wCorrection) % 7);

    // jump to next month
    if( wDay > 31 )
    {
        wMonth = APRIL;
        wDay -= 31;
    }
    else
    {
        wMonth = MARCH;
    }
}