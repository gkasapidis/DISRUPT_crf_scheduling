//
// Created by gregkwaste on 5/6/19.
//



#ifndef CRF_RESCHEDULING_COMPONENT_H
#define CRF_RESCHEDULING_COMPONENT_H

#include "planning_horizon.h"

struct res_window {
    my_date_time start_dt;
    my_date_time end_dt;
    int res; //>0 feasible, <0 not feasible

    res_window(){
        start_dt = my_date_time();
        end_dt = my_date_time();
        res = 0;
    }
};

//This struct should be used to describe the resource consumption for a day of production
class res_horizon {
public:
    int start_inv;
    int end_inv;
    int *res;

    res_horizon(){
        res = new int[TIME_INTERVALS_PER_DAY];
        clear();
    }

    //Assignment operator
    res_horizon& operator=(const res_horizon &ref_res) {
        start_inv = ref_res.start_inv;
        end_inv = ref_res.end_inv;
        for (int i=0; i<TIME_INTERVALS_PER_DAY; i++)
            res[i] = ref_res.res[i];
        return *this;
    }

    ~res_horizon(){
        delete[] res;
    }

    void clear() {
        start_inv = 0;
        end_inv = 0;
        for (int i=0; i<TIME_INTERVALS_PER_DAY; i++)
            res[i] = 0;
    }
};


class component {
public:
    string ID;
    int component_type_id; //No idea if this should be used, it is stored in the database
    float saturation; //Used for holding the saturation
    int container_capacity; //Quantity of items contained on a transportation container
    int units_per_order; //This is probably 1 per order
    res_horizon** delivery_hor;
    res_horizon** used_hor;
    res_horizon** acc_hor;
    bool ignored; //Used to skip component during feasibility checks
    bool affected; //Used to mark components that are included in delayed trucks

    vector<res_window*> feas_windows;
    vector<res_window*> infeas_windows;
    int NumberOfInfeasibilities = 0;

    //int distance_to_station; //This holds the distance in seconds from the component warehouse to the assembly station
    //int station_position; //This holds the position of the station from the start of the line that the component is required/assembled
    int station_id; //This holds the station ID where the component is to be assembled

    component(){
        if (global_planning_horizon != nullptr){
            container_capacity = 1;
            allocate_consumption_horizon();
        } else{
            //Default values
            container_capacity = 1;
        }
        ignored = false;
        affected = false;
        station_id = 0;
    };

    void allocate_consumption_horizon(){
        int daycount = global_planning_horizon->daycount;
        //Allocate data structure for holding resource capacities
        delivery_hor = new res_horizon*[daycount];
        for (int i=0; i<daycount; i++)
            delivery_hor[i] = new res_horizon();

        //Allocate data structure for holding resource consumption
        used_hor = new res_horizon*[daycount];
        for (int i=0; i<daycount; i++)
            used_hor[i] = new res_horizon();

        //Allocate data structure for accumulated resources
        acc_hor = new res_horizon*[daycount];
        for (int i=0; i<daycount; i++)
            acc_hor[i] = new res_horizon();

        feas_windows = vector<res_window*>();
        infeas_windows = vector<res_window*>();

        reset_comsuptions();
    }

    void reset_comsuptions(){
        for (int i=0; i<global_planning_horizon->daycount; i++){
            //I should not modify the delivery horizon here
            used_hor[i]->clear();
            acc_hor[i]->clear();
        }

        for (int i=0; i<feas_windows.size(); i++)
            delete feas_windows[i];

        for (int i=0; i<infeas_windows.size(); i++)
            delete infeas_windows[i];

        feas_windows.clear();
        infeas_windows.clear();
    }


    ~component(){
        for (int i=0; i<global_planning_horizon->daycount; i++){
            delete delivery_hor[i];
            delete used_hor[i];
            delete acc_hor[i];
        }

        delete[] delivery_hor; // Delete pointer array
        delete[] used_hor; // Delete pointer array
        delete[] acc_hor; // Delete pointer array

        for (int i=0;i<feas_windows.size();i++)
            delete feas_windows[i];

        for (int i=0;i<infeas_windows.size();i++)
            delete infeas_windows[i];

        feas_windows.clear();
        infeas_windows.clear();
    }

    //Assignment operator
    component& operator=(const component &ref_comp){
        ID = ref_comp.ID;
        component_type_id = ref_comp.component_type_id;
        container_capacity = ref_comp.container_capacity;
        ignored = ref_comp.ignored;
        affected = ref_comp.affected;
        station_id = ref_comp.station_id;

        //Copy all arrays
        for (int i=0;i<global_planning_horizon->daycount;i++){
            *delivery_hor[i] = *ref_comp.delivery_hor[i];
            *used_hor[i] = *ref_comp.used_hor[i];
            *acc_hor[i] = *ref_comp.acc_hor[i];
        }
        return *this;
    }


    //Calculated the amount of time in minutes that an order assigned at the from_date can be pushed
    //backwards in time
    int get_component_slack(my_date_time from_time){
        int slack_time = 0;


        if (from_time.actual_day_id < 0 || from_time.actual_hour_id < 0)
            return 0;

        //Get timeslot
        int from_timeslot = (from_time.actual_hour_id * 60 + from_time.data.tm_min) / 6;

        //Get component availability at the current slot
        int endq = used_hor[from_time.actual_day_id]->res[from_timeslot];

        for (int day = from_time.actual_day_id; day >=0; day--){
            int start_timeslot;
            if (day == from_time.actual_day_id){
                start_timeslot = from_timeslot - 1;
            }
            else
                start_timeslot = TIME_INTERVALS_PER_DAY - 1;

            //Move to the previous day
            if (start_timeslot < 0)
                continue;

            //Move backwards on the timeslots
            for (int slot = start_timeslot; slot >= 0; slot--){
                if (acc_hor[day]->res[slot] > endq)
                    slack_time += CYCLE_TIME;
                else if (acc_hor[day]->res[slot] == 0)
                    return slack_time;
            }
        }

        if (slack_time > 0){
            //printf("Query Started from Day %d, Timeslot %d\n",
            //        from_time.actual_day_id, from_timeslot);
            //report();
        }

        return slack_time;
    }



    //Checks the availability of the component given a part of the time horizon
    //We need to search the entire planning horizon for 0 accumulated consumptions because after the
    //insertion of consumption this will trigger as infeasible
    bool check_feasibility(my_date_time from_time, int from_timeslot){
        if (ignored)
            return true;

        int from_day = from_time.actual_day_id;
        int calc_from_timeslot = (from_time.actual_hour_id * 60 + from_time.data.tm_min) / 6;

        calc_from_timeslot = min(from_timeslot, calc_from_timeslot);

        for (int day = from_day; day < global_planning_horizon->daycount; day++){
            int start_timeslot;

            if (day == from_day)
                start_timeslot = calc_from_timeslot;
            else
                start_timeslot = 0;

            for (int slot = start_timeslot; slot < TIME_INTERVALS_PER_DAY; slot++){
                if (acc_hor[day]->res[slot] <= 0)
                    return false;
            }
        }
        return true;
    }


    bool integrityCheck() {
        int delivery_count = 0;
        int consumption_count = 0;
        for (int i=0; i<global_planning_horizon->daycount; i++){
            if (i==0)
                delivery_count += delivery_hor[0]->start_inv;

            for (int j=0;j<TIME_INTERVALS_PER_DAY;j++){
                delivery_count += delivery_hor[i]->res[j];
                consumption_count += used_hor[i]->res[j];
            }
        }


        //TODO: Find a way to incorporate the number of infeasibilities
        if (acc_hor[global_planning_horizon->daycount - 1]->end_inv + consumption_count  != delivery_count)
            return false;
        return true;
    }


    void accumulateConsumption(int day){
        accumulateConsumption(0, day);
    }

    void accumulateConsumption(my_date_time start_time){
        accumulateConsumption(start_time.actual_day_id, global_planning_horizon->daycount);
    }

    void accumulateConsumption(int start_day, int end_day){
        int current_level;
        if (start_day > 0)
            current_level = acc_hor[start_day - 1]->end_inv;
        else
            current_level=0;

        for (int i=start_day; i<end_day; i++){

            acc_hor[i]->clear();

            //Accumulate first delivery
            current_level += delivery_hor[i]->start_inv;
            acc_hor[i]->start_inv = current_level;

            //Accumulate consumption & deliveries
            for (int j=0; j<TIME_INTERVALS_PER_DAY; j++){

                //Add delivery (delivery is inserted at the correct day directly
                current_level += delivery_hor[i]->res[j];

                //Subtract the usage
                current_level -= used_hor[i]->res[j];

                acc_hor[i]->res[j] = current_level;
            }
            //Accumulate last delivery
            current_level += delivery_hor[i]->end_inv;
            acc_hor[i]->end_inv = current_level;
        }

    }

    void calculateNumberOfInfeasibilities(){
        //Calculate the number of infeasibilities for the component
        NumberOfInfeasibilities = 0;
        for (int w_i=0; w_i < infeas_windows.size(); w_i++){
            res_window *w = infeas_windows[w_i];

            int max_infeasibilities = 0;
            for (int i=w->start_dt.actual_day_id; i < w->end_dt.actual_day_id + 1; i++){
                for (int j=0;j<TIME_INTERVALS_PER_DAY;j++){
                    //Check if we are out of the window
                    if ((i == w->end_dt.actual_day_id) && (j > w->end_dt.actual_hour_id))
                        break;

                    //Accumulated consumption should already be negative in the infeasibility window
                    if (acc_hor[i]->res[j] < max_infeasibilities)
                        max_infeasibilities = acc_hor[i]->res[j];
                }
            }

            NumberOfInfeasibilities = max(NumberOfInfeasibilities, -max_infeasibilities); //Manually fixing the sign
        }
    }

    void identifyFeasibilityWindows(){
        //I assume that the accumulation method has been called
        for (auto w: feas_windows)
            delete w;
        for (auto w: infeas_windows)
            delete w;

        feas_windows.clear();
        infeas_windows.clear();

        //Generate first window
        res_window *w = new res_window();
        w->start_dt = plan_start_date;
        w->res = (acc_hor[0]->res[0] >= 0);

        res_window *active_w = w;
        for (int i=0; i < plan_end_date.actual_day_id + 1; i++){
            for (int j=0;j<TIME_INTERVALS_PER_DAY;j++){
                bool status = (acc_hor[i]->res[j] >= 0);
                if ( status != w->res ){
                    //We have detected a change
                    //Step A: Close open window
                    if (j==0) {
                        active_w->end_dt.actual_day_id = i - 1;
                        active_w->end_dt.actual_hour_id = TIME_INTERVALS_PER_DAY - 1;
                        active_w->end_dt.updateDateTimeFromActualDateTimeIDs();
                    }else{
                        active_w->end_dt.actual_day_id = i;
                        active_w->end_dt.actual_hour_id = j - 1;
                        active_w->end_dt.updateDateTimeFromActualDateTimeIDs();
                    }

                    if (w->res)
                        feas_windows.push_back(active_w);
                    else
                        infeas_windows.push_back(active_w);

                    active_w->res = !status; //Save Status

                    //Step B: Open new window
                    w = new res_window();
                    w->start_dt.actual_day_id = i;
                    w->start_dt.actual_hour_id = j;
                    w->start_dt.updateDateTimeFromActualDateTimeIDs();
                    w->res = status;
                    active_w = w;
                }

            }
        }

        //Check if the last window has been closed
        if (w->end_dt.actual_day_id == -1){
            w->end_dt.actual_day_id = global_planning_horizon->daycount - 1;
            w->end_dt.actual_hour_id = TIME_INTERVALS_PER_DAY - 1;
            w->end_dt.updateDateTimeFromActualDateTimeIDs();
            if (w->res)
                feas_windows.push_back(w);
            else
                infeas_windows.push_back(w);
        }

    }


    void moveResource(my_date_time from, my_date_time to, int amount){
        printf("Moving supply of %s : Amount %d - From %s To %s\n",
               ID.c_str(), amount, from.toString().c_str(), to.toString().c_str());
        int old_day = global_planning_horizon->DATE_TO_ID[from.getDate()];
        int old_hour = global_planning_horizon->HOUR_TO_ID[from.getTime()];
        int new_day = global_planning_horizon->DATE_TO_ID[to.getDate()];
        int new_hour = global_planning_horizon->HOUR_TO_ID[to.getTime()];

        //Move resource
        //Make sure that the remaining quantity is non negative (This should NOT happen during runtime)
        delivery_hor[old_day]->res[old_hour] = max(delivery_hor[old_day]->res[old_hour] - amount, 0);
        delivery_hor[old_day]->res[old_hour] += amount;

        report();
    }

    /* this probably misses some detail
    void addDelivery(my_date_time *date, int amount){
        //The component quantity is available one hour after the delivery time
        delivery_hor[date->actual_day_id]->res[date->actual_hour_id + 1] += amount;
    }
    */

    void addConsumption(my_date_time* date, int amount){
        //Figure out the time interval for the
        int time_interval = (date->actual_hour_id * 60 + date->data.tm_min) / 6;
        //Consuption is applied on the same day and hour of date
        used_hor[date->actual_day_id]->res[time_interval] += amount;

        if (used_hor[date->actual_day_id]->res[time_interval] < 0){
            report();
            assert(false);
        }

    }

    void report(bool verbose){
        if (!verbose)
            printf("Component %s \n", ID.c_str());
        else
            report();
    }

    void report(){
        //I want to get printed the data regarding the actual planning horizon
        //int eff_start_day = plan_start_date.actual_day_id;
        int eff_start_day = plan_start_date.actual_day_id;
        int eff_stop_day = plan_end_date.actual_day_id;

        //Print Horizon
        printf("%36s", " ");
        for (int i=eff_start_day; i<=eff_stop_day; i++){
            printf("%8s| ", "");
            for (int j=0;j<TIME_INTERVALS_PER_DAY / 10;j++){
                printf("%s %02d %s ",
                       string(23, '-').c_str(), j ,
                       string(22, '-').c_str());
            }
            printf("|%4s ", " ");
        }
        printf("|\n");

        printf("Component %18s Delivery: ", ID.c_str());
        for (int i=eff_start_day; i<=eff_stop_day; i++){
            printf("\t");
            printf("%04d| ", delivery_hor[i]->start_inv);
            for (int j=0;j<TIME_INTERVALS_PER_DAY;j++)
                printf("%04d ", delivery_hor[i]->res[j]);
            printf("|%04d ", delivery_hor[i]->end_inv);
        }

        printf("\nComponent %18s Consumpt: ", ID.c_str());
        for (int i=eff_start_day;i<=eff_stop_day;i++){
            printf("\t");
            printf("%04d| ", used_hor[i]->start_inv);
            for (int j=0;j<TIME_INTERVALS_PER_DAY;j++)
                printf("%04d ", used_hor[i]->res[j]);
            printf("|%04d ", used_hor[i]->end_inv);
        }

        printf("\nComponent %18s Accumula: ", ID.c_str());
        for (int i=eff_start_day;i<=eff_stop_day;i++){
            printf("\t");
            printf("%04d| ", acc_hor[i]->start_inv);
            for (int j=0;j<TIME_INTERVALS_PER_DAY;j++)
                printf("%04d ", acc_hor[i]->res[j]);
            printf("|%04d   ", acc_hor[i]->end_inv);
        }

        printf("\n   Feasible Windows : ");
        for (int i=0; i < feas_windows.size(); i++){
            res_window *w = feas_windows[i];
            printf("[%s - %s], ", w->start_dt.toString().c_str(), w->end_dt.toString().c_str());
        }
        printf("\n InFeasible Windows : ");
        for (int i=0; i < infeas_windows.size(); i++){
            res_window *w = infeas_windows[i];
            printf("[%s - %s], ", w->start_dt.toString().c_str(), w->end_dt.toString().c_str());
        }
        printf("\n\n");
    }

    void resetDelivery(){
        //Resed Used loads
        for (int i=0;i<global_planning_horizon->daycount; i++)
            delivery_hor[i]->clear();
    }

    void resetConsumption(){
        //Resed Used loads
        for (int i=0;i<global_planning_horizon->daycount; i++)
            used_hor[i]->clear();
    }

    void addDelivery(my_date_time *pdt, int load){
        //Figure out the time interval for the
        int time_interval = (pdt->actual_hour_id * 60 + pdt->data.tm_min) / 6;
        delivery_hor[pdt->actual_day_id]->res[time_interval] += load;
    }

    bool hasDelivery(){
        for (int i=0; i<global_planning_horizon->daycount; i++){
            //Check if there are truck deliveries in the planning horizon (initial inventory is exluded)
            for (int j=0; j<TIME_INTERVALS_PER_DAY; j++){
                if (delivery_hor[i]->res[j] != 0)
                    return true;
            }
        }
        return false;
    }
};

#endif //CRF_RESCHEDULING_COMPONENT_H
