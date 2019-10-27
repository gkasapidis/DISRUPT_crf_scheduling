//
// Created by gregkwaste on 5/6/19.
//
#ifndef CRF_RESCHEDULING_PROBLEM_H
#define CRF_RESCHEDULING_PROBLEM_H

//Custom class requirements
#include "component.h"
#include "truck.h"
#include "events.h"

using namespace std;

//Order Move Struct
struct move_pos{
    int day;
    int shift;
    int pr_period;
    int index;
};

//MIX_CODE CLASS
class mix_code{
public:
    int ID;
    string *componentIDs;
    int componentNum;

    mix_code(){
        componentNum = 0;
        componentIDs = new string[30]; //At most 30 components are assigned per mix_code
    }

    //Assignment operator
    mix_code& operator=(const mix_code &ref_mc){
        ID = ref_mc.ID;
        componentNum = ref_mc.componentNum;
        for (int i=0;i<componentNum;i++)
            componentIDs[i] = ref_mc.componentIDs[i];
        return *this;
    }

    void add_component(string comp_name){
        componentIDs[componentNum] = comp_name;
        componentNum++;
    }

    void load_components_from_list(vector<string> compList){
        componentNum = compList.size();

        for (int i=0;i<componentNum;i++)
            componentIDs[i] = compList[i];
    }

    bool hasComponent(string componentID){
        for (int i=0;i<componentNum;i++)
            if (componentIDs[i] == componentID)
                return true;
        return false;
    }

    void findUniqueComponents(mix_code *mc, vector<string> &ref){
        for (int i=0; i<componentNum; i++){
            string c = componentIDs[i];
            if (!mc->hasComponent(c))
                ref.push_back(c);
        }
    }

    void report(){
        printf("Mix Code %d. %d Components: ", ID, componentNum);
        for (int i=0;i<componentNum;i++)
            printf("%s ", componentIDs[i].c_str());
        printf("\n");
    }

    ~mix_code(){
        delete[] componentIDs;
    }
};

//ORDER CLASS
//Production Order
class order{
public:
    int ID;
    //Store original order placement
    my_date_time production_start_time; //Start of the assembly (Entering the first station of the line)
    my_date_time production_end_time; //End of the assembly (Existing the last station of the line)
    my_date_time* station_visit_times;
    int shift_id;
    int production_period_id;
    int period_sequenceID;
    mix_code *mixCode;
    bool dummy;
    int old_seqID; //Sequence ID of the order in the reference solution
    int seqID; //Sequence ID of the order in the current solution
    int CIS;
    bool neglected; //This is set in order to skip the production of an order
    bool delayed; //This is set in order to indicate orders that have been delayed due to component unavailability
    bool has_moved; //This flag marks if an order has moved or not during optimization
    int assembly_line_position; // This holds the position of the order in the assembly line (if any)

    order(){
        dummy = false; //By default set as real order
        neglected = false;
        delayed = false;
        has_moved = false;
        station_visit_times = new my_date_time[MAX_STATIONS];
        assembly_line_position = 0; //By default assume that the order begins at the start of the line
    };

    ~order(){
        delete station_visit_times;
    };

    //Assignment operator
    order& operator=(const order &ref_order){
        this->ID = ref_order.ID;
        this->old_seqID = ref_order.old_seqID;
        this->seqID = ref_order.seqID;
        this->neglected = ref_order.neglected;
        this->has_moved = ref_order.has_moved;
        this->delayed = ref_order.delayed;
        this->CIS = ref_order.CIS;
        this->production_start_time = ref_order.production_start_time;
        this->production_end_time = ref_order.production_end_time;
        this->shift_id = ref_order.shift_id;
        this->production_period_id = ref_order.production_period_id;
        this->period_sequenceID = ref_order.period_sequenceID;
        this->mixCode = ref_order.mixCode;
        this->dummy = ref_order.dummy;
        this->assembly_line_position = ref_order.assembly_line_position;

        //Copy station visiting times
        for (int i=0; i<MAX_STATIONS; i++)
            station_visit_times[i] = ref_order.station_visit_times[i];

        return *this;
    }

    bool hasComponent(string componentID){
        return mixCode->hasComponent(componentID);
    }

    void report(){
        printf("Order %d MixCode %d Production Start Date %s Production End Date %s Initial Line Position %d\n",
               ID, mixCode->ID,
               production_start_time.toString().c_str(),
               production_end_time.toString().c_str(),
               assembly_line_position);
    }

};


//Unavailability window
class unavail_window{
public:
    my_date_time start_time;
    my_date_time end_time;

    unavail_window(){};
    ~unavail_window(){};

    //Assignment operator
    unavail_window& operator=(const unavail_window &ref_uw) {
        start_time = ref_uw.start_time;
        end_time = ref_uw.end_time;
        return *this;
    }

    void report(){
        printf("Unavailability Window [%s, %s]\n",
                start_time.toString().c_str(), end_time.toString().c_str());
    }

    bool contains(my_date_time time){
        if ((time > start_time) && (time < end_time))
            return true;
        return false;
    }
};




//Assembly station Class
class station {
public:
    int ID;
    my_date_time last_processing_time; //Used to keep track of the processing time of the station
    int position; //Keeps the position of the station on the line (index)

    station(){
        //Set default values
        ID = -1;
        position = 0;
    };
    ~station(){};

    void report(){
        cout << "Station " << ID << " Position " << position << endl;
    }
};


//SHIFT CLASS
class shift{
public:
    my_date_time start_time;
    my_date_time end_time;
    vector<unavail_window*> unavailability_windows;
    int day;


    //Returns the actual duraction of the shift in minutes
    float getDuration(my_date_time start_time, my_date_time end_time){
        float duration = end_time.datetime - start_time.datetime;

        for (int i=0;i<unavailability_windows.size();i++){
            unavail_window *unav_win = unavailability_windows[i];

            if ((start_time.datetime <= unav_win->start_time.datetime) &&
                (end_time.datetime >= unav_win->end_time.datetime)){
                duration -= (unav_win->end_time.datetime - unav_win->start_time.datetime);
            }
        }

        return duration / 60.0f;
    }

    shift(){
        day = 0;
        unavailability_windows = vector<unavail_window*>();
    };

    ~shift(){
        for (int i=0;i<unavailability_windows.size();i++)
            delete unavailability_windows[i];
        unavailability_windows.clear();
    }


    shift& operator=(const shift &ref){
        start_time = ref.start_time;
        end_time = ref.end_time;
        day = ref.day;


        //Clear unavalability windows
        for (int i=0; i<(int) unavailability_windows.size();i++)
            delete unavailability_windows[i];
        unavailability_windows.clear();

        //Copy unavailability windows
        for (int i=0; i<ref.unavailability_windows.size(); i++){
            unavail_window *un_w = new unavail_window();
            *un_w = *ref.unavailability_windows[i];
            unavailability_windows.push_back(un_w);
        }

        return *this;
    }



    void report(){
        printf("Shift Time Period [%s - %s]\n",
                start_time.toString().c_str(), end_time.toString().c_str());
        printf("Unavailability Windows \n");
        for (int i=0;i<unavailability_windows.size();i++){
            unavailability_windows[i]->report();
        }
    }
};



//PROBLEM CLASS
class problem {
public:
    //TODO: Maybe I should implement a solution class and remove the order_map from here.
    std::unordered_map<string, component*> components;
    std::unordered_map<int, order*> orders; //Orders are indexed with their id
    std::vector<order*> order_list; //Order list to index directly
    std::unordered_map<int, truck*> trucks; //Trucks are indexed with their id
    std::vector<delay_event*> delay_events; //Delay Events
    std::unordered_map<int, mix_code*> mixCodes;
    std::vector<std::vector<shift*>> shift_map; //Per day shift map

    int** station_distances; //Station Distances
    int* next_stationPos; //Returns the next assembly station position given another position of the line
    std::unordered_map<int, int> component_warehouse_to_station_distance; //key: Component Type ID, value: Distance to warehouse in seconds
    std::vector<station*> stations; //List of assembly stations
    std::unordered_map<int, int> AssemblyLineStationMap; //key: Station ID, value: Assembly Line Position
    std::unordered_map<int, int> AssemblyLinePositionMap; //key: Assembly Line Position, value: Station ID


    problem *ref; //Should store the basic reference schedule
    //Data Counters
    int order_count = 0;
    int truck_count = 0;
    int component_count = 0;

    //Temp variables
    std::vector<component*> corrupt_components;

    int cost;
    float jph;
    int numberOfInfeasibilities = 0;
    int movedOrdersofSameDay = 0;
    int movedOrderToOtherDay = 1;
    int nonScheduledOrders = 0;

    problem();
    ~problem();

    problem& operator= (const problem &ref){
        cost = ref.cost;
        jph = ref.jph;
        this->ref = ref.ref; //Pass reference to ref solution

        //Copy data counters
        order_count = ref.order_count;
        truck_count = ref.truck_count;
        component_count = ref.component_count;

        //Clear orders
        for (int i=0; i<order_list.size(); i++){
            delete order_list[i];
        }

        orders.clear();
        order_list.clear();
        components.clear();
        mixCodes.clear();
        stations.clear();
        AssemblyLinePositionMap.clear();
        AssemblyLineStationMap.clear();
        component_warehouse_to_station_distance.clear();

        //Clear shift_map
        for (int i=0; i<shift_map.size(); i++){
            vector<shift*> day_shift_vec = shift_map[i];
            for (int j=0; j < (int) day_shift_vec.size(); j++){
                delete day_shift_vec[j];
            }
            day_shift_vec.clear();
        }
        shift_map.clear();

        //Copy Orders
        for (auto o: ref.orders){
            order *ord_ref = o.second;
            order *ord = new order();
            *ord = *ord_ref;
            orders[ord->ID] = ord;
        }

        //Copy order list
        for (int i=0;i<order_count;i++){
            order_list.push_back(orders[ref.order_list[i]->ID]);
        }

        //Copy MixCodes
        for (auto o: ref.mixCodes){
            mix_code *mc_ref = o.second;
            mix_code *mc = new mix_code();
            *mc = *mc_ref;
            mixCodes[mc->ID] = mc;
        }

        //Copy components
        for (auto comp : ref.components){
            component *new_comp = new component();
            *new_comp = *comp.second;
            components[comp.first] = new_comp;
        }

        //Copy stations
        for (auto s : ref.stations){
            station *new_s = new station();
            *new_s = *s;
            stations.push_back(new_s);
        }

        //Copy station Distances
        for (int i=0; i<MAX_STATIONS; i++){
            next_stationPos[i] = ref.next_stationPos[i];
            for (int j=0; j<MAX_STATIONS; j++){
                station_distances[i][j] = ref.station_distances[i][j];
            }
        }

        //Copy Assembly Line Station->Position Map
        AssemblyLineStationMap = ref.AssemblyLineStationMap;
        AssemblyLinePositionMap = ref.AssemblyLinePositionMap;

        //Copy component to warehouse distance map
        component_warehouse_to_station_distance = ref.component_warehouse_to_station_distance;

        //Copy unavailability windows
        for (int i=0;i<ref.shift_map.size();i++){
            vector<shift*> ref_day_shift_vec = ref.shift_map[i];
            vector<shift*> day_shift_vec = vector<shift*>();

            for (int j=0;j<ref_day_shift_vec.size();j++){
                shift *ref_s = ref_day_shift_vec[j];
                shift *s = new shift();
                *s = *ref_s; //Copy shifts
                day_shift_vec.push_back(s);
            }

            shift_map.push_back(day_shift_vec);
        }

        return *this;
    }


    void accumulateConsumption(order *o, int from_day, int to_day) {
        for (int i=0;i< o->mixCode->componentNum;i++){
            string comp_name = o->mixCode->componentIDs[i];
            components[comp_name]->accumulateConsumption(from_day, to_day);
        }
    }

    //Accumulate consumption and calculate corrupted components
    void accumulateConsumption(my_date_time from_time, my_date_time to_time){
        corrupt_components.clear();
        for (auto c: components){

            //Skip ignored components
            if (c.second->ignored)
                continue;

            c.second->accumulateConsumption(from_time.actual_day_id, to_time.actual_day_id);
            c.second->identifyFeasibilityWindows();
            c.second->calculateNumberOfInfeasibilities();

            if (!c.second->check_feasibility(from_time, 0)){
                corrupt_components.push_back(c.second);
                c.second->report();
            }
        }
    }

    void resetOrderDelayStatus(){
        for (int i=0;i<order_list.size();i++){
            order *o = order_list[i];
            o->delayed = false;
        }
    }

    bool simulatePlan(bool check_feasibility, bool update_order_stats){
        //printf("Simulating Plan\n");
        //Clear consumptions
        for (auto c: components){
            c.second->reset_comsuptions();
            c.second->accumulateConsumption(global_planning_horizon->daycount);
        }

        //Reset station procesing times
        for (int i=0; i<stations.size(); i++){
            station *s = stations[i];
            s->last_processing_time = plan_start_date;
        }

        //Insert orders in the original sequence
        for (int i=0;i<order_list.size();i++){
            order *o = order_list[i];
            //o->delayed = false;

            //Generate Order
            my_date_time eff_production_start_time;

            //Assign line position
            if (o->seqID + 1  < 92){
                eff_production_start_time = plan_start_date;
            } else {
                eff_production_start_time = plan_start_date;
                eff_production_start_time.addMinutes((o->seqID - 92) * REAL_CYCLE_TIME);
            }

            //Schedule order in the earliest possible time (do not check for feasibility)
            scheduleOrder(o, &eff_production_start_time, check_feasibility, update_order_stats);

        }

        //Finally Calculate the cost of the schedule
        cost = calculateCost(false);
        //metricsReport();
        return true;
    }


    int insertNonScheduledOrders(){
        //Step B: Try to insert  left-over orders back in the schedule in place of the dummy orders
        int fixed_order_count = 0;
        for (int order_id=0; order_id<order_count;order_id++){
            order *o = order_list[order_id];

            //Using an invalid shift_id to find out about non scheduled orders
            if (o->shift_id >= 0)
                continue;

            bool order_scheduled = false;


            //TODO: Try to insert orders that fall over the end of the plannhing horizon
            //TODO: Chances are that this is not required anymore
        }

        return fixed_order_count;
    }

    //Feasibility Checking methods
    int feasibilityCheck(){
        numberOfInfeasibilities = 0;
        corrupt_components.clear();

        //Check components
        for (auto c: components){
            if (c.second->ignored)
                continue;

            if (c.second->infeas_windows.size() > 0){
                corrupt_components.push_back(c.second);
                numberOfInfeasibilities += c.second->NumberOfInfeasibilities;
                printf("Component %12s Type %2d is corrupt - Component Shortage: %4d\n",
                        c.first.c_str(), c.second->component_type_id,  c.second->NumberOfInfeasibilities);
            }
        }

        //printf("A total %d/%d components are corrupt\n",
        //        (int) corrupt_components.size(), component_count);
        return numberOfInfeasibilities;
    }

    //Fetches the predecessor of an order in the schedule
    order *getPrevOrder(order *o){

        //Try to catch extra calls without any errors - hopefully this doesn't destroy a lot of stuff
        if (o == nullptr)
            return nullptr;

        if (o->seqID > 0)
            return order_list[o->seqID - 1];
        else
            return nullptr;
    }

    //Fetches the successor of an order in the schedule
    order *getNextOrder(order *o){
        //Try to catch extra calls without any errors - hopefully this doesn't destroy a lot of stuff
        if (o == nullptr)
            return nullptr;

        if (o->seqID < order_list.size() - 1)
            return order_list[o->seqID + 1];
        else
            return nullptr;
    }

    int hasMovedFromRef(order *o){
        //Returns 0: if the order did not move
        //Returns 1: if the order changed or moved to day 0
        //Returns 2: Otherwise

        //Load ref order
        order *ref_o = ref->orders[o->ID];

        //Check if the order has changed position in the schedule
        if (o->seqID != ref_o->seqID){
            //If it is/was on the line return 0;
            if (o->assembly_line_position > 0 || ref_o->assembly_line_position)
                return 1;
            else
                return 2;
        }

        return 0;
    }

    bool hasDifferentNeighbors(order *p1, order *n1, order *p2, order *n2){
        bool prev_order_status = true;
        bool next_order_status = true;

        if ((p1 != nullptr) && (p2 != nullptr))
            prev_order_status = (p1->ID == p2->ID);

        if ((n1 != nullptr) && (n2 != nullptr))
            next_order_status = (n1->ID == n2->ID);

        //Check order sequence
        return !(prev_order_status && next_order_status);
    }

    int calculateReschedulingCost() {
        //In order to accurately measure the solution cost, we should re-compare with the reference solution
        //so that we can accurately estimate the moving orders and stuff
        int temp_movedOrdersofSameDay = 0;
        int temp_movedOrderToOtherDay = 0;

        for (auto o_kv : orders){
            order *o = o_kv.second;

            if (o->seqID == o->old_seqID)
                continue;

            //Load orders
            order *o_p = getPrevOrder(o);
            order *o_n = getNextOrder(o);
            order *ref_o = ref->orders[o->ID];
            order *ref_o_p = ref->getPrevOrder(ref_o);
            order *ref_o_n = ref->getNextOrder(ref_o);

            //if (!hasDifferentNeighbors(o_p, o_n, ref_o_p, ref_o_n))
            //    continue;

            //If the sequence has no been altered check the actual movement of the order
            int status = hasMovedFromRef(o);

            if (status == 1)
                temp_movedOrdersofSameDay++;
            else if (status == 2)
                temp_movedOrderToOtherDay++;
        }

        //Calculate original cost
        return 175 * temp_movedOrdersofSameDay + 50 * temp_movedOrderToOtherDay;
    }

    int calculateCost(bool real) {
        //In order to accurately measure the solution cost, we should re-compare with the reference solution
        //so that we can accurately estimate the moving orders and shit
        cost = 0;
        movedOrdersofSameDay = 0;
        movedOrderToOtherDay = 0;
        nonScheduledOrders = 0;
        numberOfInfeasibilities = 0;


        //Measure component infeasibilities
        numberOfInfeasibilities = feasibilityCheck();

        for (auto o_kv : orders){
            order *o = o_kv.second;

            //If the order has not been scheduled, add it to the nonScheduled orders counter
            if (o->production_end_time > plan_end_date){
                nonScheduledOrders++;
            }

            if (o->seqID == o->old_seqID)
                continue;

            //Load orders
            order *o_p = getPrevOrder(o);
            order *o_n = getNextOrder(o);
            order *ref_o = ref->orders[o->ID];
            order *ref_o_p = ref->getPrevOrder(ref_o);
            order *ref_o_n = ref->getNextOrder(ref_o);

            //if (!hasDifferentNeighbors(o_p, o_n, ref_o_p, ref_o_n))
            //    continue;

            //Accumulate relocation costs only if the order has moved
            if (!o->has_moved)
                continue;

            //If the sequence has no been altered check the actual movement of the order
            int status = hasMovedFromRef(o);

            if (status == 1)
                movedOrdersofSameDay++;
            else if (status == 2)
                movedOrderToOtherDay++;
        }


        if (real){
            //Calculate original cost
            return 175 * movedOrdersofSameDay + 50 * movedOrderToOtherDay + 25 * nonScheduledOrders;
        }

        jph = calculateJPH(); //Recalculate jph

        //Calculate Cost
        return  175 * movedOrdersofSameDay + 50 * movedOrderToOtherDay + 25 * nonScheduledOrders + 5000 * (ref->jph - jph);
        //Objective Function Notes
        //The number of non Scheduled orders is included in the objective function just to try to drive the search to
        //solutions where as many orders as possible are executed.
    }

    void repairInitialStock(){
        //Iterate on every component and fix the initial stock
        for (auto c: components){
            //Skip fixing this component
            if (c.second->ID == "06701406820"){
                continue;
            }
            c.second->delivery_hor[0]->start_inv += c.second->NumberOfInfeasibilities;
        }
    }

    void repairInitialStock_CRF(){
        //Iterate in the first 16 orders and fix initial inventory for
        //DASHBOARD SHELLS/WHEELS/SEATS

        unordered_map<string,int> init_stock_map = unordered_map<string, int>();

        int temp_order_count = 0;

        //TODO: This is garbage not sure if it should be fixed or not. Maybe I'll just guarantee feasibility of the base schedule.
    }


    int getOrderSlackTime(order *o){
        int slack_time = INF;
        //Iterate in order components

        //Query next assembly stations for the order
        int current_station_pos = o->assembly_line_position;
        int current_station_id = getAssemblyStationID(current_station_pos);

        //Get to the next station if there is no station in this position
        if (current_station_id < 0){
            current_station_id = getNextAssemblyStation(current_station_pos);
            current_station_pos = AssemblyLineStationMap[current_station_id];
        }

        while (current_station_pos >= 0){
            my_date_time visit_time = o->station_visit_times[current_station_id];

            for (int i=0; i < o->mixCode->componentNum; i++) {
                component *c = components[o->mixCode->componentIDs[i]];
                if (c->station_id != current_station_id)
                    continue;

                //Calculate slack
                slack_time = min(slack_time, c->get_component_slack(visit_time));
            }

            //Find next station
            int next_station_id = getNextAssemblyStation(current_station_pos);

            //We have reached the end of the line
            if (next_station_id < 0) break;

            // Update current station information
            current_station_pos = AssemblyLineStationMap[next_station_id];
            current_station_id = AssemblyLinePositionMap[current_station_pos];

            //Propagate Any delay startin from the next station
            //propagateStationDelay(current_station_id, (int) delay);
        }

        int test = INF; //WTF
        if (slack_time == test)
            return 0;
        else
            return slack_time;
    }


    /*
    //CalculateConstraintSaturation
    void updateConstraintSaturation(){
        for (int i=0;i<components.size();i++){
            int max_cap = 0;
            int usage = 0;
            for (int j=0; j<p_horizon->weekcount; j++){
                max_cap += components[i]->weeks[j].max_cap;
                usage += components[i]->weeks[j].temp_used;
            }
            components[i]->saturation = 100.0f * (float) usage / (float) max_cap;
        }
    }
    */

    //This method is very similar with the insertOrder method. It does practically the same job but does not set any
    //order or station attribute and it also returns false if a feasibility problem is detected.
    //TODO: Maybe I should make this return the delay due to component unavailability, so that I can compare this accros
    //the candidate orders

    bool checkOrderExecutionFeasibility(order *o, my_date_time start_production_time){
        //Init station visit time
        my_date_time visit_time = start_production_time;
        //Query next assembly stations for the order
        int current_station_pos = o->assembly_line_position;
        int current_station_id = getAssemblyStationID(current_station_pos);
        long delay = 0;

        //Get to the next station if there is no station in this position
        if (current_station_id < 0){
            current_station_id = getNextAssemblyStation(current_station_pos);
            current_station_pos = AssemblyLineStationMap[current_station_id];
            //Advance the visit time to reach the next station
            advanceTime(&visit_time, (current_station_pos - o->assembly_line_position), REAL_CYCLE_TIME);
            //propagateStationDelay(current_station_id, (int) delay);
        }

        while (current_station_pos >= 0){
            station *s = stations[current_station_id];

            if (s->last_processing_time > visit_time)
                visit_time =  s->last_processing_time;

            //Iterate in the order's mixcode components and add consumptions on the related components
            for (int i=0; i < o->mixCode->componentNum; i++) {
                component *c = components[o->mixCode->componentIDs[i]];
                if (c->station_id != current_station_id)
                    continue;

                    if (!c->check_feasibility(visit_time, 0))
                        return false;
            }

            //Advance time to get processed in the current station
            delay = advanceTime(&visit_time, 1, REAL_CYCLE_TIME);

            //Find next station
            int next_station_id = getNextAssemblyStation(current_station_pos);

            //We have reached the end of the line
            if (next_station_id < 0) break;

            //Advance the visit time to reach the next station
            delay = advanceTime(&visit_time, station_distances[current_station_id][next_station_id], REAL_CYCLE_TIME);

            // Update current station information
            current_station_pos = AssemblyLineStationMap[next_station_id];
            current_station_id = AssemblyLinePositionMap[current_station_pos];

            //Propagate Any delay startin from the next station
            //propagateStationDelay(current_station_id, (int) delay);
        }

        return true;
    }

    float getWorkingHoursPerDay(int day){
        float minutes = 0.0f;

        for (int j=0;j<2;j++){
            shift *s = shift_map[day][j];

            my_date_time actual_shift_start = s->start_time;
            my_date_time actual_shift_end = s->end_time;

            //Skip non working shifts
            if (actual_shift_end < plan_start_date || actual_shift_start < plan_start_date || actual_shift_end > plan_end_date || actual_shift_start > plan_end_date)
                continue;


            if (plan_start_date > actual_shift_start)
                actual_shift_start = plan_start_date;

            if (plan_end_date < actual_shift_end)
                actual_shift_end = plan_end_date;

            minutes += s->getDuration(actual_shift_start, actual_shift_end);
        }

        return minutes;
    }

    float getWorkingHours(){
        float minutes = 0.0f;
        for (int i=0;i<shift_map.size();i++){
            minutes += getWorkingHoursPerDay(i);
        }

        return minutes / 60.0f;
    }


    int calculateTotalJobs(){
        int job_counter = 0;
        vector<int> day_order_counter = vector<int>();
        //Init counter vector
        for (int i=plan_start_date.actual_day_id; i<global_planning_horizon->daycount; i++)
            day_order_counter.push_back(0);

        //Iterate in orders
        for (int i=0; i < (int) order_list.size(); i++){
            order *o = order_list[i];

            if (o->production_end_time > plan_end_date)
                continue;

            day_order_counter[o->production_end_time.actual_day_id]++;
        }

        //Calculate the average
        for (int i=plan_start_date.actual_day_id; i<global_planning_horizon->daycount; i++)
            job_counter += day_order_counter[i];

        return job_counter;
    }

    //Calculates the average JobPerDay during the planning horizon
    float calculateJPD(){
        return (calculateTotalJobs() / getWorkingHours()) * 14.0f;
    }

    //Calculates average JobPerHour during the planning horizon
    float calculateJPH(){
        return calculateTotalJobs() / getWorkingHours();
    }

    //Calculates the current schedules dissimilarity from the reference schedule
    static int calculateDissimilarity(problem *sol, problem *ref){
        int counter = 0;

        for (int i=0; i<sol->orders.size(); i++)
            if (!sol->orders[i]->production_start_time.same_day(ref->orders[i]->production_start_time))
                counter++;

        return counter;
    }

    //Schedule Manipulation methods
    void removeOrder(order* o){
        //TODO: Fix this to work on the order sequence

        //Iterate in parts of the mixcode
        for (int i=0; i<o->mixCode->componentNum; i++){
            component *c = components[o->mixCode->componentIDs[i]];
            c->addConsumption(&o->production_start_time, -1);
            c->accumulateConsumption(o->production_start_time.actual_day_id, global_planning_horizon->daycount);
        }

        //Reset order stats
        o->shift_id = -1;
        o->period_sequenceID = -1;
        o->production_start_time.reset();
        o->production_end_time.reset();
    }

    int getAssemblyStationID(int assembly_line_position){
        if (AssemblyLinePositionMap.find(assembly_line_position) != AssemblyLinePositionMap.end()){
            return AssemblyLinePositionMap[assembly_line_position];
        }
        return -1;
    }

    int getNextAssemblyStation(int assembly_line_position){
        int local_line_pos = assembly_line_position + 1;

        while(local_line_pos < ASSEMBLY_LINE_LENGTH){
            int found_id = getAssemblyStationID(local_line_pos);
            if (found_id > 0) return found_id;
            else
                local_line_pos++;
        }
        return -1;
    }

    int getPrevAssemblyStation(int assembly_line_position){
        int local_line_pos = assembly_line_position - 1;

        while(local_line_pos >= 0){
            int found_id = getAssemblyStationID(local_line_pos);
            if (found_id > 0) return found_id;
            else
                local_line_pos--;
        }
        return -1;
    }


    long advanceTime(my_date_time *time, int distance ,float time_length){
        //This method advanced time step by step until the necessery time distance is covered.
        //Making sure that the unavailability windows are not violated
        //It returns the amount of delay that is captured in the way

        int elapsed_distance = 0;
        long accumulated_delay = 0;
        my_date_time temp_time = *time;
        my_date_time old_time = *time;
        while (elapsed_distance < distance){
            //Start marching
            old_time = temp_time;
            temp_time.addMinutes(time_length);
            elapsed_distance++;

            //Keep adding minutes to the
            while (temp_time.actual_day_id < 0 || temp_time.actual_hour_id < 0) {
                temp_time.addMinutes(time_length);
                //Do not do anything if we are out of the planning horizon
                if (temp_time > plan_end_date) {
                    *time = temp_time;
                    return accumulated_delay;
                }
            }

            //if (temp_time.actual_day_id == global_planning_horizon->daycount - 1 && temp_time.actual_hour_id > 14)
            //    printf("break");

            //Check if time is within any unavailability window of the current day shift
            for (int i=0;i<2;i++){
                shift *s = shift_map[temp_time.actual_day_id][i];
                for (int j=0;j<s->unavailability_windows.size();j++){
                    unavail_window *un_w = s->unavailability_windows[j];
                    if (un_w->contains(temp_time)){
                        temp_time = old_time;
                        int working_time_before_unavail = (int) max(0l,  un_w->start_time.datetime - old_time.datetime); //Seconds
                        int remaining_working_time = (int) (REAL_CYCLE_TIME * 60) - working_time_before_unavail;

                        int delay = un_w->end_time.datetime - un_w->start_time.datetime;
                        accumulated_delay += delay;

                        temp_time = un_w->end_time;
                        temp_time.addSeconds(remaining_working_time);

                        //Check if the time is included in the last unavailability window of the last shift.
                        //In this case we have to shift the date to the next day
                        if (j == 3 && i == 1) {
                            temp_time = old_time;
                            //Get to the next valid day
                            temp_time.addMinutes(9 * 60);
                            while(temp_time.actual_day_id < 0 || temp_time.actual_hour_id < 0){
                                temp_time.addMinutes(6);

                                if (temp_time > plan_end_date){
                                    *time = temp_time;
                                    return accumulated_delay;
                                }
                            }
                            //Reset to XX:00:00
                            temp_time.addMinutes(- temp_time.data.tm_min);
                            temp_time.addMinutes(REAL_CYCLE_TIME); //Add working time
                        }
                    }
                }
            }
        }

        //Set time
        *time = temp_time;
        return accumulated_delay;
    }

    my_date_time estimateOrderEndTime(order*o, my_date_time *production_time, bool check_feasibility){
        //Init station visit time
        my_date_time visit_time = *production_time;
        //Query next assembly stations for the order
        int current_station_pos = o->assembly_line_position;
        int current_station_id = getAssemblyStationID(current_station_pos);
        long delay = 0;

        //Get to the next station if there is no station in this position
        if (current_station_id < 0){
            current_station_id = getNextAssemblyStation(current_station_pos);
            current_station_pos = AssemblyLineStationMap[current_station_id];
            //Advance the visit time to reach the next station
            delay = advanceTime(&visit_time, (current_station_pos - o->assembly_line_position), REAL_CYCLE_TIME);
            //propagateStationDelay(current_station_id, (int) delay);
        }

        while (current_station_pos >= 0){
            station *s = stations[current_station_id];

            if (s->last_processing_time > visit_time)
                visit_time =  s->last_processing_time;

            //Stop scheduling orders out of the plannhing horizon
            if (visit_time < plan_end_date){
                //Iterate in the order's mixcode components and add consumptions on the related components
                for (int i=0; i < o->mixCode->componentNum; i++) {
                    component *c = components[o->mixCode->componentIDs[i]];
                    if (c->station_id != current_station_id)
                        continue;

                    //If check feasibility is enabled, check the available component quantity
                    //on the current visit time before adding the consumption
                    if (check_feasibility){
                        while (true){
                            bool isfeasible = c->check_feasibility(visit_time, TIME_INTERVALS_PER_DAY);
                            if (isfeasible)
                                break;
                            else {
                                c->report();
                                //Advance visit time by one cycle time
                                advanceTime(&visit_time, 1, REAL_CYCLE_TIME);
                                o->delayed = true; //Mark order as delayed
                            }
                        }
                    }

                    //No need to add the consumptions here, I'm just checking one order
                }
            }

            //Advance time to get processed in the current station
            delay = advanceTime(&visit_time, 1, REAL_CYCLE_TIME);

            //Find next station
            int next_station_id = getNextAssemblyStation(current_station_pos);

            //We have reached the end of the line
            if (next_station_id < 0) break;

            //Advance the visit time to reach the next station
            delay = advanceTime(&visit_time, station_distances[current_station_id][next_station_id], REAL_CYCLE_TIME);

            // Update current station information
            current_station_pos = AssemblyLineStationMap[next_station_id];
            current_station_id = AssemblyLinePositionMap[current_station_pos];

            //Propagate Any delay startin from the next station
            //propagateStationDelay(current_station_id, (int) delay);
        }

        return visit_time;
    }

    void scheduleOrder(order*o, my_date_time *production_time, bool check_feasibility, bool update_order_stats){
        o->production_start_time = *production_time;

        //TODO: Use the update_order_status flag to make sure that all other order attributes can be updated as well
        //Potentially this will help us to use this method instead of the extra estimateOrderPdocuctionEndTime

        //Init station visit time
        my_date_time visit_time = o->production_start_time;
        //Query next assembly stations for the order
        int current_station_pos = o->assembly_line_position;
        int current_station_id = getAssemblyStationID(current_station_pos);
        long delay = 0;

        //Get to the next station if there is no station in this position
        if (current_station_id < 0){
            current_station_id = getNextAssemblyStation(current_station_pos);
            current_station_pos = AssemblyLineStationMap[current_station_id];
            //Advance the visit time to reach the next station
            delay = advanceTime(&visit_time, (current_station_pos - o->assembly_line_position), REAL_CYCLE_TIME);
            //propagateStationDelay(current_station_id, (int) delay);
        }

        while (current_station_pos >= 0){
            station *s = stations[current_station_id];

            if (s->last_processing_time > visit_time)
                visit_time =  s->last_processing_time;

            //Overwrite productio start time
            if (s->ID == 0)
                o->production_start_time = visit_time;

            //Stop scheduling orders out of the plannhing horizon
            if (visit_time < plan_end_date){

                //Step A - Find latest time to accumulate consumption FOR ALL components that belong to the station
                //Iterate in the order's mixcode components and add consumptions on the related components
                for (int i=0; i < o->mixCode->componentNum; i++) {
                    component *c = components[o->mixCode->componentIDs[i]];
                    if (c->station_id != current_station_id)
                        continue;

                    //If check feasibility is enabled, check the available component quantity
                    //on the current visit time before adding the consumption
                    if (check_feasibility) {
                        while (true) {
                            bool isfeasible = c->check_feasibility(visit_time, TIME_INTERVALS_PER_DAY);
                            if (isfeasible)
                                break;
                            else {
                                //c->report()
                                //Advance visit time by one cycle time
                                advanceTime(&visit_time, 1, REAL_CYCLE_TIME);
                                if (update_order_stats)
                                    o->delayed = true; //Mark order as delayed
                            }
                        }
                    }
                }

                //Step B: Add consumption for all related components in the calculated time
                for (int i=0; i < o->mixCode->componentNum; i++) {
                    component *c = components[o->mixCode->componentIDs[i]];
                    if (c->station_id != current_station_id)
                        continue;
                    //Add consumption to component
                    if (visit_time < plan_end_date){
                        c->addConsumption(&visit_time, 1);
                        c->accumulateConsumption(visit_time);
                    } else {
                        c->report();
                        printf("Visit Time ended up completely out of the planning horizon. Something is wrong\n");
                    }
                }
            }

            //Set order station visiting time
            o->station_visit_times[current_station_id] = visit_time;

            //Advance time to get processed in the current station
            delay = advanceTime(&visit_time, 1, REAL_CYCLE_TIME);
            //Save time as the earliest available time for processing in the station
            s->last_processing_time = visit_time;
            //Since we have explicitly handled the time for the current station, propagate delay only on the previous ones
            //propagateStationDelay(getPrevAssemblyStation(AssemblyLineStationMap[s->ID]), (int) delay);

            //Find next station
            int next_station_id = getNextAssemblyStation(current_station_pos);

            //We have reached the end of the line
            if (next_station_id < 0) break;

            //Advance the visit time to reach the next station
            delay = advanceTime(&visit_time, station_distances[current_station_id][next_station_id], REAL_CYCLE_TIME);

            // Update current station information
            current_station_pos = AssemblyLineStationMap[next_station_id];
            current_station_id = AssemblyLinePositionMap[current_station_pos];

            //Propagate Any delay startin from the next station
            //propagateStationDelay(current_station_id, (int) delay);
        }

        //Compare the last calculated visit time to the o->production_end_time
        //printf("Calculated End Time %s vs Saved End Time %s \n",
        //        visit_time.toString().c_str(), o->production_end_time.toString().c_str());
        o->production_end_time = visit_time;

        if (visit_time.data.tm_mday==20)
            printf("break");


    }

    //Event Methods
    void insertEvent(delay_event *e){
        delay_events.push_back(e);
    }

    void applyEvents(){
        for (int i=0;i<delay_events.size();i++)
            applyEvent(delay_events[i]);
    }

    //Apply paint event
    void applyEvent(paint_event *e){
        //Find order
        order *o = nullptr;
        for (int i=0;i<orders.size();i++){
            order *po = orders[i];

            if (po->CIS == e->CIS){
                o = po;
                break;
            }
        }

        //If order has been found remove if from the schedule
        if (o != nullptr){
            o->neglected = true;
        }
    }

    //Apply delay event
    void applyEvent(delay_event *e){
        //Load Truck
        truck *t = ref->trucks[e->truckID]; //Load trucks from the base problem so that we don't copy them all around

        //Fetch the date time that the truck unloads
        my_date_time old_ut = t->unloading_time;
        my_date_time new_ut = old_ut;

        //Add delay to date time
        new_ut.addMinutes(e->delay);


        printf("Old Truck Unloading Date %s and Time %s\n",
               old_ut.getDate().c_str(), old_ut.getTime().c_str());
        printf("New Truck Unloading Date %s and Time %s\n",
                new_ut.getDate().c_str(), new_ut.getTime().c_str());

        //Iterate in loaded components
        for (auto c : t->components) {
            //Load component
            if (components.find(c.first) == components.end()) {
                printf("Component %s Not Found in DB\n", c.first.c_str());
                continue;
            }

            component *comp = components[c.first];
            //Add the time distance that is required from the warehouse to the particular component station
            int station_distance = component_warehouse_to_station_distance[comp->component_type_id];

            my_date_time old_station_delivery_time = old_ut;
            my_date_time station_delivery_time = new_ut;

            //Add station distance time
            station_delivery_time.addSeconds(station_distance);
            old_station_delivery_time.addSeconds(station_distance);

            printf("OLD COMPONENT STATUS\n");
            comp->report();

            comp->addDelivery(&old_station_delivery_time, -c.second); //Remove old delivery
            comp->addDelivery(&station_delivery_time, c.second); //Add new consumption
            printf("NEW COMPONENT STATUS\n");
            comp->report();
        }
    }

    void reportEvents(){
        printf("\n#########DELAY EVENTS REPORT##########\n");
        for (int i=0;i<delay_events.size();i++)
            delay_events[i]->report();
        printf("################################\n");
    }

    bool probeOrderinDay(order *ord, int day){
        int new_day = day;
        int new_week = global_planning_horizon->day_week_map[day];

        //TODO: Rework on this

        return true;
    }

    string exportSolutionAggregatePerDay(){
        //Store schedule to the disrupt database
        string schedule;
        vector<int> day_order_counter = vector<int>();
        //Init counter vector
        for (int i=plan_start_date.actual_day_id; i<global_planning_horizon->daycount; i++)
            day_order_counter.push_back(0);

        //Save Real Cost of schedule
        schedule += to_string(this->calculateCost(false)) + "\n";
        schedule += to_string(this->calculateReschedulingCost()) + "\n";

        //Iterate in orders
        for (int i=0;i<order_list.size();i++){
            order *o = order_list[i];

            if (o->production_end_time.actual_day_id < 0)
                continue;

            //Information on orders after the planning horizon is dum dum.
            if (o->production_end_time > plan_end_date)
                continue;

            day_order_counter[o->production_end_time.actual_day_id]++;
        }

        for (int i=plan_start_date.actual_day_id; i<global_planning_horizon->daycount; i++){
            float working_hours = getWorkingHoursPerDay(i);
            if (working_hours > 1e-5) {
                schedule += global_planning_horizon->ID_TO_DATE[i] + " " +
                            to_string(day_order_counter[i]) + " " + to_string(working_hours) +  '\n';
            }
        }

        return schedule;
    }

    string exportSolutionPerProductionDetailedPerDay(){
        string schedule;

        //Convert the schedule to a DISRUPT database friendly format
        for (int i=0;i<order_list.size();i++){
            order *o = order_list[i];
            if (o->dummy || o->neglected)
                continue;
            //Also make sure to convert the index to 1-index
            schedule += o->production_end_time.toString(DATE_FORMAT::YYYY_MM_DD_DASH) + " " +
                        to_string(o->seqID + 1) + " " + to_string(o->mixCode->ID) + "\n";
        }

        return schedule;
    }

    string toString(int mode){
        if (mode ==0)
            return exportSolutionPerProductionDetailedPerDay();
        else if (mode ==1)
            return exportSolutionAggregatePerDay();
    }

    /* DEPRECATED

    //Deterministic Rescheduler
    void det_rescheduler(int currentDay){

        //Iterate in Constraints
        for (int i=0;i<constraints.size();i++){
            //Detect problem in constraints
            for (int j=0; j<p_horizon->weekcount; j++) {
                if (constraints[i]->evaluate(j))
                    continue;

                //At this point there is a problem with constraint i and week j
                //Figure out how many orders need to be transferred
                int orders_to_be_moved = constraints[i]->weeks[j].temp_used - constraints[i]->weeks[j].max_cap;

                //Try to resolve the conflict by moving orders to the next days of production
                int moved_orders = 0;
                int iters = 0;
                while (moved_orders < orders_to_be_moved){
                    //Iterate in days of problematic week
                    for (int i_day=0;i_day<p_horizon->weeks[j].size();i_day++){
                        int problem_day = p_horizon->weeks[j][i_day];
                        //printf("Trying to move orders in day %s\n", ID_TO_DATE[problem_day].c_str());
                        //Iterate in problematic orders
                        for (int i_order = (int) order_map[problem_day].size() - 1; i_order >= 0; i_order--){
                            order *prob_ord = orders[order_map[problem_day][i_order]];
                            //Make sure that order is affected
                            if (!prob_ord->hasConstraint(constraints[i]->ID)) continue;
                            //Make sure not to move a frozen order
                            if (prob_ord->frozen) continue;



                            //Find new date to schedule the order
                            bool order_moved = false;
                            //Assume that we will know about the event 3 weeks earlier
                            for (int n_week = 0; n_week<p_horizon->weekcount; n_week++){
                                for (int n_day=0;n_day<p_horizon->weeks[n_week].size();n_day++){
                                    int newday = p_horizon->weeks[n_week][n_day];
                                    if (newday < currentDay) continue; //continue if we can't schedule before a current day

                                    bool st = probeOrderinDay(prob_ord, newday);

                                    if (st){
                                        printf("Moving Order %d from day %s to day %s\n",
                                               prob_ord->ID,
                                               ID_TO_DATE[prob_ord->productionDate].c_str(),
                                               ID_TO_DATE[newday].c_str());

                                        //Remove order from Schedule
                                        removeOrder(prob_ord, i_order);
                                        //InsertOrder to new date
                                        insertOrder(prob_ord, newday);
                                        moved_orders++;
                                        order_moved = true;
                                        break;
                                    }
                                }
                                if (order_moved || moved_orders >= orders_to_be_moved) break;
                            }

                            if (moved_orders >= orders_to_be_moved) break; //exit
                        }
                        if (moved_orders >= orders_to_be_moved) break; //exit
                    }
                    iters++;

                    //Exit Mechanism
                    if (iters > 5){
                        printf("Deterministic Rescheduler Failed\n");
                        break;
                    }
                }
            }
        }
    }

    */


    //Reports
    void metricsReport(){
        printf("\n########SCHEDULE METRICS########\n");
        printf("\tCalculated Schedule Cost %d\n", cost);
        printf("\tTotal Jobs in current Schedule %d\n", calculateTotalJobs());
        printf("\tAverage JobsPerDay in current Schedule %4.4f\n", calculateJPD());
        printf("\tAverage JobsPerHour in current Schedule %4.4f\n", calculateJPH());
        printf("################################\n");
    }

    void corruptedComponentReport(){
        printf("\n##CORRUPTED COMPONENT REPORT###\n");
        for (auto c: corrupt_components){
            printf("Component %12s is corrupt - Number of Infeasibilities: %d\n",
                   c->ID.c_str(), c->NumberOfInfeasibilities);
            c->report(global_planning_horizon->daycount);
        }
        printf("#################################\n");
    }


    void componentReport(){
        printf("\n##COMPONENT REPORT###\n");
        for (auto c: components)
            c.second->report();
        printf("#################################\n");
    }

    void affectedDomponentReport(){
        printf("\n##AFFECTED COMPONENT REPORT###\n");
        for (auto c: components){
            if (c.second->affected)
                c.second->report();
        }
        printf("#################################\n");
    }

    void orderComponentReport(order *o){
        printf("\n##ORDER COMPONENT REPORT###\n");
        for (int i=0; i<o->mixCode->componentNum; i++){
            component *c = components[o->mixCode->componentIDs[i]];
            c->report();
        }
        printf("#################################\n");
    }


    void constraintSaturationReport(){
        printf("\n##CONSTRAINT SATURATION REPORT###\n");
        //TODO: I have no idea how to handle this for now
        printf("#################################\n");
    }

    void constraintCapacityReport(){
        //Just for Fun report all problem constraints
        printf("\n###CONSTRAINT CAPACITY REPORT###\n");
        feasibilityCheck();

        //TODO: I have no idea how to handle this for now
        printf("##########END OF REPORT#########\n");
    }

    //Schedule report
    void reportSchedule(int mode){
        switch(mode) {
            //Full Report
            case 0: {
                cout << exportSolutionPerProductionDetailedPerDay();
                break;
            }
            //Mini Report
            case 1: {
                cout << exportSolutionAggregatePerDay();
                break;
            }
        }
    }

    void reportFirstDelayedOrders(){
        for (int i=0;i<order_list.size();i++){
            order *o = order_list[i];

            if (o->delayed && i>0){
                order *o_prev = order_list[i-1];
                int diff_from_prev = (float) (o->production_end_time.datetime - o_prev->production_end_time.datetime);
                diff_from_prev /= 60.0f; //Convert to minutes
                printf("Order %d Delayed. Delay %d \n", o->ID, diff_from_prev);
            }
        }
    }

    void reportRelocatedOrders(){
        for (int i=0;i<order_list.size();i++){
            order *o = order_list[i];
            if (o->has_moved){
                printf("Order %d has Moved. Index %d\n", o->ID, i);
            }
        }
    }

    //Schedule export
    void exportSchedule(std::string name){
        std::ofstream f;
        f.open(name.c_str());

        for (int i=plan_start_date.actual_day_id; i<global_planning_horizon->daycount; i++) {
            f << "Day " << global_planning_horizon->ID_TO_DATE[i] << std::endl;
            for (int j = 0; j < orders.size(); j++) {
                order *o = orders[i];
                if (o->dummy || o->neglected)
                    continue;
                if (o->production_end_time.actual_day_id == i)
                    f << "\t Order " << o->ID << std::endl;
            }
        }

        f.close();
    }

    //Schedule export

    //Error Checking
    bool integrityCheck(){
        //Make sure that all orders have been scheduled exactly once
        unordered_map<int,int> order_index_map = unordered_map<int,int>();
        //Init counters fo all order sto zero
        for(auto o: orders){
            order_index_map[o.second->ID] = 0;
        }

        for (int i=0;i<order_list.size();i++){
            order *o = orders[i];
            if (o->ID < 0 || o->seqID < 0)
                return false;
            //TODO: Check if we need more integrity checks per order
        }

        //NOT SURE HOW TO PROPERLY DO THIS FOR NOW. SKIPPING
        for (auto c: components){
            if (!c.second->integrityCheck()){
                return false;
            }
        }

        order_index_map.clear();

        return true;
    }


};

#endif //CRF_RESCHEDULING_PROBLEM_H
