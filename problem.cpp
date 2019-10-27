//
// Created by gregkwaste on 5/6/19.
//

#include "problem.h"


problem::problem() {
    components = std::unordered_map<string, component*>();
    orders = std::unordered_map<int, order*>();
    order_list = std::vector<order*>();
    delay_events = std::vector<delay_event*>();
    mixCodes = std::unordered_map<int, mix_code*>();
    trucks = std::unordered_map<int, truck*>();
    shift_map = std::vector<vector<shift*>>();

    //Init station distances double array
    next_stationPos = new int[MAX_STATIONS];
    station_distances = new int*[MAX_STATIONS];
    for (int i=0;i<MAX_STATIONS;i++){
        station_distances[i] = new int[MAX_STATIONS];
    }

    //Init orders structure
    for (int i=0; i<global_planning_horizon->daycount; i++){
        vector<shift*> day_shift_vec = vector<shift*>();
        stringstream temp_date;
        char date_buffer[500];
        string datestring = global_planning_horizon->ID_TO_DATE[i] + " " + global_planning_horizon->ID_TO_HOUR[0];
        my_date_time date_mdt;
        date_mdt.init(datestring, DD_MM_YYYY_SLASH);

        //printf("Date Start Time %s\n", date_mdt.toString().c_str());

        for (int j=0; j<2; j++){
            shift *s = new shift();
            s->start_time = date_mdt;
            s->start_time.addSeconds(8 * 60 *60 * j);
            s->end_time = s->start_time;
            s->end_time.addSeconds(8 * 3600); //Add 8 hours
            s->day = i;

            //printf("Shift Start Time %s\n", s->start_time.toString().c_str());
            //printf("Shift End   Time %s\n", s->end_time.toString().c_str());

            //Sanity check, but using timegm should be ok
            if (s->start_time.data.tm_hour == 5){
                assert(false);
            }


            int time_offset = 8*60*j;
            //Add unavaibility windows to shift
            auto *un_w1 = new unavail_window();
            un_w1->start_time = s->start_time;
            un_w1->start_time.addMinutes(2 * 60);
            un_w1->end_time = un_w1->start_time;
            un_w1->end_time.addMinutes(10);
            //TODO: Not sure if I should add proper information for the actual day and hour of production
            s->unavailability_windows.push_back(un_w1);

            auto *un_w2 = new unavail_window();
            un_w2->start_time = un_w1->start_time;
            un_w2->start_time.addMinutes(2 * 60);
            un_w2->end_time = un_w2->start_time;
            un_w2->end_time.addMinutes(10);
            //TODO: Not sure if I should add proper information for the actual day and hour of production
            s->unavailability_windows.push_back(un_w2);

            auto *un_w3 = new unavail_window();
            un_w3->start_time = un_w2->start_time;
            un_w3->start_time.addMinutes(2 * 60);
            un_w3->end_time = un_w3->start_time;
            un_w3->end_time.addMinutes(10);
            //TODO: Not sure if I should add proper information for the actual day and hour of production
            s->unavailability_windows.push_back(un_w3);


            auto *un_w4 = new unavail_window();
            un_w4->start_time = un_w3->start_time;
            un_w4->start_time.addMinutes( 60 + 30);
            un_w4->end_time = un_w4->start_time;
            un_w4->end_time.addMinutes(30);
            //TODO: Not sure if I should add proper information for the actual day and hour of production
            s->unavailability_windows.push_back(un_w4);

            //Add production periods on shift

            /*
            auto *pr1 = new production_period();
            pr1->index = 0;
            pr1->shift_id = j;
            //2:00 hr
            pr1->order_limit = 20;
            pr1->start_time = s->start_time;
            pr1->start_time.actual_day_id = i;
            pr1->start_time.actual_hour_id = 8 * j + 0;
            pr1->start_time.updateDateTimeFromActualDateTimeIDs();
            pr1->end_time = pr1->start_time;
            pr1->end_time.addSeconds(2 * 3600);
            pr1->end_time.actual_day_id = i;
            pr1->end_time.actual_hour_id = 8 * j + 2;
            pr1->end_time.updateDateTimeFromActualDateTimeIDs();

            s->pr_periods.push_back(pr1);

            //1:50
            auto *pr2 = new production_period();
            pr2->index = 1;
            pr2->shift_id = j;
            pr2->start_time = pr1->end_time;
            pr2->start_time.addMinutes(10);
            pr2->start_time.actual_day_id = i;
            pr2->start_time.actual_hour_id = 8 * j + 2;
            //pr2->start_time.updateDateTimeFromActualDateTimeIDs();
            pr2->end_time = pr2->start_time;
            pr2->end_time.addMinutes(60 + 50);
            pr2->end_time.actual_day_id = i;
            pr2->end_time.actual_hour_id = 8 * j + 4;
            //pr2->end_time.updateDateTimeFromActualDateTimeIDs();
            pr2->order_limit = 18;
            s->pr_periods.push_back(pr2);

            //1:50
            auto *pr3 = new production_period();
            pr3->index = 2;
            pr3->shift_id = j;
            pr3->start_time = pr2->end_time;
            pr3->start_time.addMinutes(10);
            pr3->start_time.actual_day_id = i;
            pr3->start_time.actual_hour_id = 8 * j + 4;
            //pr3->start_time.updateDateTimeFromActualDateTimeIDs();
            pr3->end_time = pr3->start_time;
            pr3->end_time.addMinutes(60 + 50);
            pr3->end_time.actual_day_id = i;
            pr3->end_time.actual_hour_id = 8 * j + 6;
            //pr3->end_time.updateDateTimeFromActualDateTimeIDs();
            pr3->order_limit = 18;

            s->pr_periods.push_back(pr3);

            //1:20
            auto *pr4 = new production_period();
            pr4->index = 3;
            pr4->shift_id = j;
            pr4->start_time = pr3->end_time;
            pr4->start_time.addMinutes(10);
            pr4->start_time.actual_day_id = i;
            pr4->start_time.actual_hour_id = 8 * j + 6;
            //pr4->start_time.updateDateTimeFromActualDateTimeIDs();
            pr4->end_time = pr4->start_time;
            pr4->end_time.addMinutes(60 + 20);
            pr4->end_time.actual_day_id = i;
            pr4->end_time.actual_hour_id = 8 * j + 7;
            //pr4->end_time.updateDateTimeFromActualDateTimeIDs();
            pr4->order_limit = 13;

            s->pr_periods.push_back(pr4);
            */

            //s->report();
            day_shift_vec.push_back(s);
        }
        shift_map.push_back(day_shift_vec);
    }

    cost = 0;
}

problem::~problem(){
    for (auto c: components)
        delete c.second;
    components.clear();

    for (auto mc: mixCodes)
        delete mc.second;
    mixCodes.clear();

    for (auto t: trucks)
        delete t.second;
    trucks.clear();

    for (auto o: orders)
        delete o.second;
    orders.clear();
    order_list.clear();
    AssemblyLinePositionMap.clear();
    AssemblyLineStationMap.clear();
    component_warehouse_to_station_distance.clear();

    //Delete stations
    for (auto s: stations)
        delete s;

    //Delete station distances array
    for (int i=0;i<MAX_STATIONS;i++)
        delete station_distances[i];
    delete station_distances;
    delete next_stationPos;


    //Cleanup Events
    for (size_t i=0; i<delay_events.size(); i++)
        delete delay_events[i];
    delay_events.clear();


    //Cleanup the shift
    for (int i=0; i<global_planning_horizon->daycount; i++){
        vector<shift*> shift_day_vec = shift_map[i];
        //Delete the shifts
        for (auto s: shift_day_vec)
            delete s;
        shift_map[i].clear();
    }

    shift_map.clear(); //Clean the empty vector
}



