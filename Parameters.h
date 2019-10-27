//
// Created by gkass on 15/02/2018.
//

//Global classes
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>

//USE THAT FLAG TO REPORT WORK AND OFF DAYS OF SCHEDULE
#define PLANNING_HORIZON_DEBUG
//Optimization Parameters
#define INF 100000000;

#ifndef CRF_RESCHEDULING_PARAMETERS_H
#define CRF_RESCHEDULING_PARAMETERS_H

#include "debug_utils.h"
using namespace std;


//Extra tuples
struct int2_tuple {
    int day;
    int hour;
    int2_tuple(int a, int b){
        day = a;
        hour = b;
    }

    int2_tuple(){
        day = -1;
        hour = -1;
    }
};

struct cc_tuple {
    string name;
    int2_tuple infWindow;
};

#endif //CRF_RESCHEDULING_PARAMETERS_H


//Planning Horizon

//Actual plan start date
extern int PLAN_START_DAY; //OLD
extern int PLAN_START_MONTH;
extern int PLAN_START_YEAR;

//Actual database start date
extern int DB_START_DAY; //OLD
extern int DB_START_MONTH;
extern int DB_START_YEAR;

//int PLAN_END_DAY = 30;
//int PLAN_END_MONTH = 4;
//int PLAN_END_YEAR = 2018;

extern int PLAN_END_DAY;
extern int PLAN_END_MONTH;
extern int PLAN_END_YEAR;

extern int  MAX_COMPONENTS;
//Max limits have been set to 5 weeks
extern int MAX_ORDERS;
extern int MAX_DAYS;
extern int MAX_JPD;
extern int MAX_ORDERS_PER_PRODUCTION_PERIOD;
extern int MAX_STATIONS;
extern int ASSEMBLY_LINE_LENGTH;
extern int CYCLE_TIME;
extern float REAL_CYCLE_TIME;

extern int TIME_INTERVALS_PER_DAY;
extern int JOB_PER_DAY;




