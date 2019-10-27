//
// Created by gregkwaste on 5/6/19.
//

#ifndef CRF_RESCHEDULING_PLANNING_HORIZON_H
#define CRF_RESCHEDULING_PLANNING_HORIZON_H

#include "Parameters.h"
#include "string_utils.h"
#include "assert.h"

//Planning Horizon
//Weekday Enum
const int MONTH_DAYCOUNT[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
enum WEEKDAY{SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, WEEKDAY_COUNT};
enum MONTH{JANUARY, FEBRUARY, MARCH, APRIL, MAY, JUNE, JULY, AUGUST, SEPTEMBER, OCTOBER, NOVEMBER, DECEMBER, MONTH_COUNT};


enum DATE_FORMAT{DD_MM_YYYY_SLASH, DD_MM_YYYY_DASH, YYYY_MM_DD_DASH, YYYY_MM_DD_SLASH};

//Easter Calculator
void GetEasterSunday( int wYear, int& wMonth, int& wDay );

class planning_horizon{
public:
    std::vector<std::vector<int>> weeks;
    std::unordered_map<int, int> day_week_map; // Day to Week Map
    //Date Assignment Map
    std::unordered_map<std::string, int> DATE_TO_ID;
    std::unordered_map<int, std::string> ID_TO_DATE;

    std::unordered_map<std::string, int> HOUR_TO_ID = {
            {"06:00:00", 0}, //Start of the dock schedule as well as the first shift of production
            {"07:00:00", 1},
            {"08:00:00", 2},
            {"09:00:00", 3},
            {"10:00:00", 4},
            {"11:00:00", 5},
            {"12:00:00", 6},
            {"13:00:00", 7},
            {"14:00:00", 8},
            {"15:00:00", 9},
            {"16:00:00", 10},
            {"17:00:00", 11},
            {"18:00:00", 12},
            {"19:00:00", 13},
            {"20:00:00", 14},
            {"21:00:00", 15},
            {"22:00:00", 16}};

    std::unordered_map<int, std::string> ID_TO_HOUR = {
            {0 , "06:00:00"}, //Start of the dock schedule as well as the first shift of production
            {1 , "07:00:00"},
            {2 , "08:00:00"},
            {3 , "09:00:00"},
            {4 , "10:00:00"},
            {5 , "11:00:00"},
            {6 , "12:00:00"},
            {7 , "13:00:00"},
            {8 , "14:00:00"},
            {9 , "15:00:00"},
            {10, "16:00:00"},
            {11, "17:00:00"},
            {12, "18:00:00"},
            {13, "19:00:00"},
            {14, "20:00:00"},
            {15, "21:00:00"},
            {16, "22:00:00"}};

    int daycount;
    int weekcount;


    planning_horizon(){
        //Initialize dates based on the planning horizon
        //Assuming that there are 2 shifts per day
        //Shift 1 : 08:00 - 14:00
        //Shift 2 : 14:00 - 22:00

        //Init
        daycount = 0;
        weekcount = 0;
        weeks = std::vector<std::vector<int>>();
        weeks.push_back(std::vector<int>()); //Add first week
        day_week_map = std::unordered_map<int,int>();
        DATE_TO_ID =  std::unordered_map<std::string,int>();
        ID_TO_DATE = std::unordered_map<int, std::string>();


        //Iterate in plan horizon
        int day = PLAN_START_DAY;
        int month = PLAN_START_MONTH;
        int year = PLAN_START_YEAR;

        //Offday stuff
        int easterday;
        int eastermonth;
        GetEasterSunday(year, eastermonth, easterday);
        char *date = new char[12];
        bool offday = false; //Keep track of the last day
        while (true) {
            //Break conditions
            if (year > PLAN_END_YEAR && (!offday))
                break;
            else if (year == PLAN_END_YEAR && month > PLAN_END_MONTH  && (!offday))
                break;
            else if (year == PLAN_END_YEAR && month == PLAN_END_MONTH && day > PLAN_END_DAY  && (!offday))
                break;

            std::tm time_in = {0, 0, 6, day, month, year - 1900};
            std::time_t time_temp = timegm(&time_in);
            //Convert to localTime
            std::tm *time_out = std::localtime(&time_temp);

            offday = false;
            //Check for national holidays or weekends
            //Weekends
            if ((time_out->tm_wday == SUNDAY) || (time_out->tm_wday == SATURDAY)){
                debug_printf("Weekend\n");
                offday = true;
            }

            if ((day == easterday + 1) && (month == eastermonth)){
                debug_printf("Easter Monday\n");
                offday = true;
            }

            //Liberation Day
            if ((day == 25) && (month == APRIL)){
                debug_printf("Liberation Day\n");
                offday = true;
            }

            //Labor Day
            if ((day == 1) && (month == MAY)){
                debug_printf("May Day\n");
                offday = true;
            }

            //Republic Day
            if ((day == 2) && (month == JUNE)){
                debug_printf("Republic Day\n");
                offday = true;
            }


            if (!offday) {
                //debug_printf("Working on Date %02d/%02d/%04d\n", day, month + 1, year);
                sprintf(date, "%02d/%02d/%04d",day, month + 1, year);
                std::string sdate = std::string(date);
                //Set day Maps
                DATE_TO_ID[sdate] = daycount;
                ID_TO_DATE[daycount] = sdate;
                printf("Day ID %d, Date %s\n", daycount, sdate.c_str());
                //Add date to week
                weeks[weekcount].push_back(daycount);
                day_week_map[daycount] = weekcount;
                //Increase counter
                daycount++;
            }

            //Increment date
            day++;
            if (day > MONTH_DAYCOUNT[month]){
                //Change month
                month++;
                day = 1;
            }
            if (month == MONTH_COUNT) {
                year++;
                month = 0;
                day = 1;
                //Update easter dates
                GetEasterSunday(year, eastermonth, easterday);
            }
            //Increment Week
            if ((time_out->tm_wday == SUNDAY)){
                weekcount++;
                weeks.push_back(std::vector<int>());
            }
        }

        delete[] date;

        //Report Map contents
#ifdef DEBUG
        for (auto t: DATE_TO_ID)
            printf("Date %s ID %d\n", t.first.c_str(), t.second);
        printf("\n\n\n\n");
#endif
        //Report Weeks
        /*
        for (int i=0;i<=weekcount;i++){
            printf("Week %d Days:\n",i);
            for (int j=0;j<weeks[i].size();j++)
                printf("Date %s ID %d\n", ID_TO_DATE[weeks[i][j]].c_str(), weeks[i][j]);

        }
         */

    }

    ~planning_horizon(){
        //Cleanup the weeks
        for (int i=0;i<weeks.size();i++)
            weeks[i].clear();
        weeks.clear();

        //Cleanup day_week_map
        day_week_map.clear();

        //Cleanup Other dictionaries
        HOUR_TO_ID.clear();
        ID_TO_HOUR.clear();
        ID_TO_DATE.clear();
        DATE_TO_ID.clear();

    }
};


//Define Global Planning Horizon pointer
extern planning_horizon *global_planning_horizon;

//This data structure is used to store time + do the mapping with direct indices of the resource tables
//using the planning horizon

class my_date_time{
public:
    tm data;
    time_t datetime;

    //Immediate mapping with the planning horizon
    int actual_day_id;
    int actual_hour_id;

    my_date_time(){
        actual_day_id = -1;
        actual_hour_id = -1;
    };

    my_date_time(int d, int m, int y, int hr, int min, int sec){
        char date[100];
        sprintf(date, "%02d/%02d/%04d %02d:%02d:%02d", d, m, y, hr, min, sec);
        strptime(date, "%d/%m/%Y %H:%M:%S", &data);
        datetime = timegm(&data);
        printf("Initialized Date %s\n", toString().c_str());
    };

    //WARNING: Use with caution
    void updateActualDateTimeIDs(){
        char buffer[50];
        strftime(buffer, 50, "%H:00:00", &data);
        string ttime = string(buffer);
        string tdate = getDate();

        //This prevents datetimes that are out of the plannhing horizon for getting a proper value for an actual date
        actual_day_id = -1;
        actual_hour_id = -1;

        //TODO: Add checks because dates or times may be missing from the maps
        if (global_planning_horizon->DATE_TO_ID.find(tdate) != global_planning_horizon->DATE_TO_ID.end())
            actual_day_id = global_planning_horizon->DATE_TO_ID[tdate];
        //printf("Actual Day ID %d\n", actual_day_id);
        //Override minutes and seconds to find the corrent hour ID
        if (global_planning_horizon->HOUR_TO_ID.find(ttime) != global_planning_horizon->HOUR_TO_ID.end())
            actual_hour_id = global_planning_horizon->HOUR_TO_ID[ttime];
        //printf("Actual Hour ID %d\n", actual_hour_id);
    }

    //WARNING: Use with caution
    void updateDateTimeFromActualDateTimeIDs(){
        string time = global_planning_horizon->ID_TO_HOUR[actual_hour_id];
        string date = global_planning_horizon->ID_TO_DATE[actual_day_id];

        string date_time = date + ' ' + time;
        init(date_time, DD_MM_YYYY_SLASH); //The horizon maps use some default datetime format
    }

    my_date_time(string input, DATE_FORMAT fmt) {
        init(input, fmt);
    }

    string toString(){
        return getDate() + " " + getTime();
    }

    string toString(DATE_FORMAT fmt){
        return getDate(fmt) + " " + getTime();
    }

    string getDate(){
        return getDate(DATE_FORMAT::DD_MM_YYYY_SLASH);
    }

    string getDate(DATE_FORMAT fmt){
        char buffer[20];
        switch (fmt){
            case DATE_FORMAT::DD_MM_YYYY_DASH:
            case DATE_FORMAT::DD_MM_YYYY_SLASH:
                strftime(buffer, 20, "%d/%m/%Y", &data);
                break;
            case DATE_FORMAT::YYYY_MM_DD_DASH:
                strftime(buffer, 20, "%Y/%m/%d", &data);
                break;
        }

        string test = string(buffer);
        return test;
    }

    string getTime(int h, int m, int s){
        char buffer[20];
        sprintf(buffer, "%02d:%02d:%02d",
                h, m, s);
        return string(buffer);
    }

    string getTime(){
        char buffer[50];
        strftime(buffer, 50, "%H:%M:%S", &data);
        return string(buffer);
    }

    //Adds seconds to current time
    void addSeconds(int s){
        data.tm_sec += s;
        datetime = timegm(&data);

        //TODO: Check if extra processing is required here



        //Update data
        updateActualDateTimeIDs();

    }

    //Add time to the current struct
    void addMinutes(float m) {
        addSeconds((int) (m * 60));
    }

    void removeMinutes(float m) {
        addSeconds((int) (-m * 60));
    }

    void removeDays(int d) {
        addSeconds(-d * 86400);
    }

    void addDays(int d) {
        addSeconds(d * 86400);
    }

    void init(string input, DATE_FORMAT fmt){
        //DateTime is expected in the following format
        //01/15/2018 06:00:00
        istringstream iss(input);
        //First fetch the date and time substrings
        string date;
        string time;
        iss >> date;
        iss >> time;

        //Now work separately
        vector<string> date_tokens = vector<string>();
        vector<string> time_tokens = vector<string>();
        char date_buffer[50];

        switch (fmt) {
            case DD_MM_YYYY_SLASH:
                strptime(date.c_str(),"%d/%m/%Y", &data);
                break;
            case YYYY_MM_DD_DASH:
                strptime(date.c_str(),"%Y-%m-%d", &data);
                break;
            case YYYY_MM_DD_SLASH:
                strptime(date.c_str(),"%Y/%m/%d", &data);
                break;
            default:
                printf("Unsupported date format\n");
                assert(false);
                break;
        }

        //Handle time
        strptime(time.c_str(),"%H:%M:%S", &data);
        datetime = timegm(&data);
    }

    bool operator ==(const my_date_time& d){
        if (datetime != d.datetime)
            return false;
        return true;
    }


    bool isOffday(){
        string new_date = getDate();
        auto k1 = global_planning_horizon->DATE_TO_ID.find(new_date);
        auto k2 = global_planning_horizon->DATE_TO_ID.end();
        if ( k1 != k2 )
            return false;
        return true;
    }

    bool operator !=(const my_date_time& d){
        return !(*this == d);
    }

    //Overload Relational Operators
    bool operator <(const my_date_time& d){
        return (datetime < d.datetime);
    }

    bool operator >(const my_date_time& d){
        my_date_time *dd = (my_date_time*) &d;
        return (*dd < *this);
    }

    my_date_time operator+(const my_date_time& d){
        my_date_time new_dt;
        new_dt = *this; //Init with the existing values

        new_dt.datetime += d.datetime;
        new_dt.data = *localtime(&new_dt.datetime);
        return new_dt;
    }


    my_date_time& operator=(const my_date_time& d){
        data = d.data;
        datetime = d.datetime;
        actual_day_id = d.actual_day_id;
        actual_hour_id = d.actual_hour_id;
        return *this;
    }

    bool same_day(const my_date_time& d){
        if (data.tm_mday == d.data.tm_mday && data.tm_mon == d.data.tm_mon)
            return true;
        return false;
    }

    bool same_day(my_date_time* d){
        if (data.tm_mday == d->data.tm_mday && data.tm_mon == d->data.tm_mon)
            return true;
        return false;
    }

    void reset(){
        data = tm();
        actual_day_id = -1;
        actual_hour_id = -1;
    }

    void report() {
        printf("%s - %d %d\n",
                toString().c_str(), actual_day_id, actual_hour_id);
    }

};


//More externs for the problem start, db start and end times
extern my_date_time plan_start_date, plan_end_date, db_start_date;


#endif //CRF_RESCHEDULING_PLANNING_HORIZON_H
