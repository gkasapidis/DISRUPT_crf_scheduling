//
// Created by gregkwaste on 5/6/19.
//

#ifndef CRF_RESCHEDULING_EVENTS_H
#define CRF_RESCHEDULING_EVENTS_H


#include "Parameters.h"


using namespace std;

class delay_event{
public:
    int ID;
    int truckID;
    int delay; //Delay in minutes

    delay_event(int id, int truck_id, int time){
        ID = id;
        truckID = truck_id;
        delay = time;
    }

    void report() {
        printf("Event %d - Truck ID %d Time Delay %d \n",
               ID, truckID, delay);
    }
};

class paint_event{
public:
    int ID;
    int CIS;

    paint_event(int id, int cis_id){
        ID = id;
        CIS = cis_id;
    }

    void report() {
        printf("Event %d - CIS ID %d \n",
               ID, CIS);
    }
};





#endif //CRF_RESCHEDULING_EVENTS_H
