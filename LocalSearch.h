//
// Created by gregkwaste on 5/6/19.
//



#ifndef CRF_RESCHEDULING_LOCALSEARCH_H
#define CRF_RESCHEDULING_LOCALSEARCH_H

#include "problem.h"

//LS Moves Enum
enum LS_MOVES {RELOCATE, SWAP, MOVE_NUM};


struct ls_move {
    int start_index;
    int end_index;
    int cost;
};



//Local Search
class LocalSearch{
private:
    int tabu_iterations;
    int tabu_tenure;
    int time_limit;
    vector<ls_move*> tabu_list;
    int iter_counter;

    //Private LS order limits for faster search
    //This should hold the order id that is not "frozen" since it is before the start of the planning horizon
    int order_start_id = -1;


public:
    problem *currentSol;
    problem *localElite;

    LocalSearch(int ts_tenure, int ts_iters, int t_lim){
        //Setup parameters
        tabu_iterations = ts_iters;
        tabu_tenure = ts_tenure;
        time_limit = t_lim;
        iter_counter = 0;

        //Initialize tabu list
        tabu_list = vector<ls_move*>();
    }

    //Deconstructor
    ~LocalSearch(){
        //Delete solutions
        delete currentSol;
        delete localElite;

        //Free tabu List
        for (int i=0;i<tabu_list.size();i++)
            delete tabu_list[i];
        tabu_list.clear();

    }

    void setup(problem *sol){
        sol->cost = sol->calculateCost(false);
        //Initialize Solutions
        currentSol = new problem();
        localElite = new problem();
        //Copy solutions
        *currentSol = *sol;
        *localElite = *sol;

        //Set ref solution for currentSol
        currentSol->ref = sol;
        localElite->ref = sol;

        //Re-evaluate solutions based on the updated reference solution
        currentSol->cost = currentSol->calculateCost(false);
        localElite->cost = localElite->calculateCost(false);
    }


    bool isTabu(ls_move &mv){

        for (int i=0;i<tabu_list.size();i++){
            ls_move *s_mv = tabu_list[i];
            if ((s_mv->start_index == mv.start_index) && (s_mv->end_index == mv.end_index))
                return true;
        }
        return false;
    }

    void addMoveToTabuList(ls_move &mv){
        ls_move* s_mv = new ls_move();
        s_mv->start_index = mv.start_index;
        s_mv->end_index = mv.end_index;
        s_mv->cost = mv.cost;

        tabu_list.push_back(s_mv);

        if (tabu_list.size() > tabu_tenure){
            delete tabu_list[0];
            tabu_list.erase(tabu_list.begin());
        }
    }


    void relocateOrder(ls_move &mv){
        //Fetch order id
        order *o1 = currentSol->order_list[mv.start_index];

        //Fix assembly line positions
        int temp = currentSol->order_list[mv.end_index]->assembly_line_position;
        for (int i=mv.end_index; i > mv.start_index; i--){
            order *o_prev = currentSol->order_list[i - 1];
            order *o = currentSol->order_list[i];
            o->assembly_line_position = o_prev->assembly_line_position;
        }
        o1->assembly_line_position = temp;

        //Fix Sequence IDs
        temp = currentSol->order_list[mv.end_index]->seqID;
        for (int i=mv.end_index; i > mv.start_index; i--){
            order *o = currentSol->order_list[i];
            order *o_prev = currentSol->order_list[i - 1];
            o->seqID = o_prev->seqID;
        }
        o1->seqID = temp;

        o1->has_moved = true;

        //Apply relocation
        currentSol->order_list.erase(currentSol->order_list.begin() + mv.start_index);
        currentSol->order_list.insert(currentSol->order_list.begin() + mv.end_index, o1);

    }

    void revertRelocation(ls_move &mv){
        //Fetch order id
        order *o1 = currentSol->order_list[mv.end_index];

        //Fix assembly line positions
        int temp = currentSol->order_list[mv.start_index]->assembly_line_position;

        for (int i=mv.start_index; i < mv.end_index; i++){
            order *o_next = currentSol->order_list[i + 1];
            order *o = currentSol->order_list[i];
            o->assembly_line_position = o_next->assembly_line_position;
        }

        o1->assembly_line_position = temp;

        //Fix assembly line positions
        temp = currentSol->order_list[mv.start_index]->seqID;

        for (int i=mv.start_index; i < mv.end_index; i++){
            order *o_next = currentSol->order_list[i + 1];
            order *o = currentSol->order_list[i];
            o->seqID = o_next->seqID;
        }

        o1->seqID = temp;
        o1->has_moved = false;

        //Apply relocation
        currentSol->order_list.erase(currentSol->order_list.begin() + mv.end_index);
        currentSol->order_list.insert(currentSol->order_list.begin() + mv.start_index, o1);

        //Report Solution just for testing


    }


    void exchangeOrderPositions(ls_move &mv){
        //Fetch order id
        order *o1 = currentSol->order_list[mv.start_index];
        order *o2 = currentSol->order_list[mv.end_index];

        //exchange sequence IDs
        int temp = o1->seqID;
        o1->seqID = o2->seqID;
        o2->seqID = temp;

        //Exchange Line Positions
        temp = o1->assembly_line_position;
        o1->assembly_line_position = o2->assembly_line_position;
        o2->assembly_line_position = temp;

        //Start Index is always lower than the end index
        if (mv.end_index < mv.start_index)
            assert(false);

        //Insert o2 in the position of o1
        currentSol->order_list.erase(currentSol->order_list.begin() + mv.start_index);
        currentSol->order_list.insert(currentSol->order_list.begin() + mv.start_index, o2);

        //Insert o1 in the position of o2
        currentSol->order_list.erase(currentSol->order_list.begin() + mv.end_index);
        currentSol->order_list.insert(currentSol->order_list.begin() + mv.end_index, o1);
    }

    void applySwap(ls_move &mv){
        exchangeOrderPositions(mv);
    }

    void applyRelocate(ls_move &mv){

        //TODO: Relocation move makes no sense here (And this doesn't work at all)

        //Relocation leaves a dummy behind and places the order to the new position

        //Fetch order id
        order *o1 = currentSol->order_list[mv.start_index];
        order *o2 = currentSol->order_list[mv.end_index];

        //Load Production Periods
        //production_period *o1_pr = currentSol->order_map[mv.start_pos.day][mv.start_pos.shift]->pr_periods[mv.start_pos.pr_period];
        //production_period *o2_pr = currentSol->order_map[mv.end_pos.day][mv.end_pos.shift]->pr_periods[mv.end_pos.pr_period];


        //Set from to dummy
        //o1_pr->orders[mv.start_pos.index] = -1;
        //Override target dummy to the new id
        //o2_pr->orders[mv.end_pos.index] = o1_id;

        //printf("Relocating Order %d from day %d, shift %d, period %d, index %d to day %d, shift %d, period %d, index %d \n",
        //       mv.start_pos.day, mv.start_pos.shift, mv.start_pos.pr_period, mv.start_pos.index,
        //       mv.end_pos.day, mv.end_pos.shift, mv.end_pos.pr_period, mv.end_pos.index);

        //Change stats of o1
        //o1->production_hour = (int) ((PROCESSING_TIME * o1->period_sequenceID + o2_pr->start_time) / 60.0f);

        currentSol->reportSchedule(0);
    }


    void applyMove(ls_move &mv, int &move_mode){
        switch(move_mode){
            case SWAP:
                applySwap(mv);
                break;
            case RELOCATE:
                reportRelocationMove(mv);
                relocateOrder(mv);
                break;
        }

        //Save move to tabu_list
        addMoveToTabuList(mv);
        //Resimulate plan
        currentSol->resetOrderDelayStatus();
        currentSol->simulatePlan(true, true);
    }

    int calcRelocateCost(order *o, int status){
        //Status values
        //0: if the order did not move compared to the ref solution
        //1: if the order moved from/to day 0
        //2: Otherwise

        if (status == 1)
            return 175;
        else if (status == 2)
            return 50;
        else
            return 0;
    }

    //Swap move methods
    bool checkSwap(order *o1, order *o2){
        //Store production day and hours temporarily
        int o1_day = o1->production_start_time.actual_day_id;
        int o1_hour = o1->production_start_time.actual_hour_id;
        int o2_day = o2->production_start_time.actual_day_id;
        int o2_hour = o2->production_start_time.actual_hour_id;

        //Find unique component sets
        vector<string> uc_1 = vector<string>();
        vector<string> uc_2 = vector<string>();

        o1->mixCode->findUniqueComponents(o2->mixCode, uc_1);
        o2->mixCode->findUniqueComponents(o1->mixCode, uc_2);

        bool feasible = true;
        //Check if o1 can move to o2's position
        for (string c : uc_1){
            component *comp = currentSol->components[c];
            my_date_time visit_time = o2->station_visit_times[comp->station_id];

            if (!comp->check_feasibility(visit_time, TIME_INTERVALS_PER_DAY))
                feasible = false;
        }

        //Check if o2 can move to o1's position
        for (string c : uc_2){
            component *comp = currentSol->components[c];
            my_date_time visit_time = o1->station_visit_times[comp->station_id];

            if (!comp->check_feasibility(visit_time, TIME_INTERVALS_PER_DAY))
                feasible = false;
        }

        //Cleanup
        uc_1.clear();
        uc_2.clear();

        return feasible;
    }


    void modifyOrderConsumption(order *o, int amount){
        //Figure out order's next visiting station
        //Find current station of o1_prev in order to fetch the earliest processing time for o2
        int current_station_pos = o->assembly_line_position;
        int current_station_id = currentSol->getAssemblyStationID(current_station_pos);

        //Get to the next station if there is no station in this position
        if (current_station_id < 0){
            current_station_id = currentSol->getNextAssemblyStation(current_station_pos);
            current_station_pos = currentSol->AssemblyLineStationMap[current_station_id];
        }

        //Modify consumptions
        for (int i=current_station_id; i<MAX_STATIONS; i++){
            station *s = currentSol->stations[i];

            for (int j=0; j < o->mixCode->componentNum; j++) {
                component *c = currentSol->components[o->mixCode->componentIDs[j]];
                if (c->station_id != s->ID)
                    continue;

                my_date_time o1_station_visit_time = o->station_visit_times[i];

                c->addConsumption(&o1_station_visit_time, amount);
                c->accumulateConsumption(global_planning_horizon->daycount);

            }
        }
    }

    bool canImproveSolution(ls_move mv){
        //Load orders
        order *o1 =currentSol->order_list[mv.start_index];
        order *o2 =currentSol->order_list[mv.end_index];


        for (int i=0;i<MAX_STATIONS;i++){
            cout << "o1 visited Station " << i << " at " << o1->station_visit_times[i].toString() << endl;
        }

        //Remove order component consumptions based on their current stats
        modifyOrderConsumption(o1, -1);
        modifyOrderConsumption(o2, -1);

        //Will probe what happens when o2 gets scheduled at o1's position

        //Set o2's position to o1's position
        int o2_position = o2->assembly_line_position;
        o2->assembly_line_position = o1->assembly_line_position;

        //Get o1 prev
        order *o1_prev = currentSol->getPrevOrder(o1);

        //Find current station of o1_prev in order to fetch the earliest processing time for o2
        int o1_prev_current_station_pos = o1_prev->assembly_line_position;
        int o1_prev_current_station_id = currentSol->getAssemblyStationID(o1_prev_current_station_pos);

        //Get to the next station if there is no station in this position
        if (o1_prev_current_station_id < 0){
            o1_prev_current_station_id = currentSol->getNextAssemblyStation(o1_prev_current_station_pos);
            o1_prev_current_station_pos = currentSol->AssemblyLineStationMap[o1_prev_current_station_id];
        }

        my_date_time production_start_time = o1_prev->station_visit_times[o1_prev_current_station_id];
        currentSol->advanceTime(&production_start_time, 1, REAL_CYCLE_TIME);
        o2->assembly_line_position = o1_prev_current_station_pos; //Start studying order from the very first station that the previous order visits

        my_date_time old_visit_times[MAX_STATIONS];

        //Set station visiting times accordingly
        for (int i=0;i<MAX_STATIONS;i++){
            station *s = currentSol->stations[i];
            old_visit_times[i] = s->last_processing_time;
            s->last_processing_time = o1_prev->station_visit_times[i];
        }


        currentSol->components["06701117070"]->report();

        my_date_time new_end_time = currentSol->estimateOrderEndTime(o2, &production_start_time, true);

        //Restore stuff
        o2->assembly_line_position = o2_position;

        //Re-add consumptions of o1
        modifyOrderConsumption(o1, 1);
        modifyOrderConsumption(o2, 1);

        //Restore station last_processing_times
        for (int i=0;i<MAX_STATIONS;i++){
            station *s = currentSol->stations[i];
            s->last_processing_time = old_visit_times[i];
        }

        return (new_end_time < o1->production_end_time);
    }


    bool findFeasibleMove(ls_move &mv, int &move_mode){

        int minim_cost = INF; //Only accept improving costs or same cost moves
        bool move_found = false;
        //Search for Swap Move
        //move_found = findBestFeasibleSwap(mv, minim_cost);
        move_found = findBestFeasibleRelocate(mv, minim_cost);
        if (move_found)
            move_mode = RELOCATE;

        return move_found;
    }

    bool findBestFeasibleSwap(ls_move &mv, int &minim_cost){
        //The operator works on a feasible solution
        bool move_found = false;

        //Generate temp move struct
        ls_move temp_mv = ls_move();

        //Work on a subset of 1000 orders every time that will be selected at random

        int avail_orders = currentSol->order_count - order_start_id;

        int set_size = 200;
        int set_num = (int) max(avail_orders / set_size + 1, 1);

        //Select a set_index at random

        int set_id = rand() % set_num;

        //int start_index = order_start_id + set_id * set_size;
        //int last_index = min(start_index + set_size, currentSol->order_count);

        //Full Iteration
        int start_index = 0;
        int last_index = currentSol->order_count;

        //Keep old solution cost
        int old_cost = currentSol->cost;

        //Iterate in first order
        for (int i=start_index; i<last_index-1; i++) {
            order *o1 = currentSol->order_list[i];

            if (!o1->delayed)
                continue;

            if (o1->production_start_time > plan_end_date || o1->production_end_time > plan_end_date)
                continue;

            temp_mv.start_index = i;

            //Iterate in second order
            for (int j = i + 1; j < last_index; j++) {
                order *o2 = currentSol->order_list[j];
                if (o2->shift_id < 0 || o1->ID == o2->ID)
                    continue;

                if (o2->production_start_time > plan_end_date)
                    continue;

                temp_mv.end_index = j;

                //Check if the move can improve the solution
                bool isSolutionImproved = canImproveSolution(temp_mv);
                if (!isSolutionImproved)
                    continue;

                if (isTabu(temp_mv))
                    continue;

                //Apply move and keep track of the solution cost

                //Apply Move
                exchangeOrderPositions(temp_mv);

                //Maybe scheduling just one order calculating the earliest start time from the previous order could be helpful to identify gains in
                //scheduling time

                currentSol->simulatePlan(true, false);
                int swap_cost = currentSol->cost;

                //Testing - Report slack per order
                for (int i=0;i<currentSol->order_list.size();i++){
                    order *o = currentSol->order_list[i];

                    int order_slack = currentSol->getOrderSlackTime(o);

                    float diff_from_prev = 0;

                    if (i>0){
                        diff_from_prev = (float) (o->production_end_time.datetime - currentSol->order_list[i - 1]->production_end_time.datetime);
                        diff_from_prev /= 60.0f; //Convert to minutes
                    }

                    printf("Order %4d SeqID: %d Delayed: %d Slack Time: %4d Production End Time %10s Diff from Last %5.4f\n",
                           o->ID, o->seqID, o->delayed, order_slack, o->production_end_time.toString().c_str(), diff_from_prev);
                }


                //Revert Move
                exchangeOrderPositions(temp_mv);

                //Save move if better
                if (swap_cost < minim_cost) {
                    //From
                    mv.start_index = temp_mv.start_index;
                    mv.end_index = temp_mv.end_index;
                    mv.cost = swap_cost;
                    minim_cost = swap_cost;
                    move_found = true;
                    //Return Immediately if we have found an improving move
                    if (minim_cost < old_cost)
                        return move_found;
                }
            }
        }

        return move_found;
    }

    bool findBestFeasibleRelocate(ls_move &mv, int &minim_cost){
        //The operator works on a feasible solution
        bool move_found = false;
        bool improving_streak = true;

        //Generate temp move struct
        ls_move temp_mv = ls_move();

        //Full Iteration
        int start_index = 0;
        int last_index = currentSol->order_count;

        //Keep old solution cost
        int old_cost = currentSol->cost;

        //Find order to relocate
        for (int i=last_index - 1; i > 0; i--) {
            order *o1 = currentSol->order_list[i];

            if (!o1->delayed)
                continue;

            //if (o1->production_start_time > plan_end_date || o1->production_end_time > plan_end_date)
            //    continue;

            temp_mv.start_index = i;

            //Find out position after which we hopw to have an improvement
            order *o1_prev = currentSol->getPrevOrder(o1);
            //Calculate seconds from the predecessor end time to the earliest possible finish time for this order
            long seconds = o1->production_end_time.datetime - o1_prev->production_end_time.datetime;

            //Calculate number of orders that fit into the delay
            int order_num_to_delay = seconds / (REAL_CYCLE_TIME * 60);

            //Iterate in candidate positions
            int max_loop_lim = std::min(i + 2 + order_num_to_delay + 20, (int) currentSol->order_list.size());
            for (int j = i + 1; j < max_loop_lim; j++) {
                order *o2 = currentSol->order_list[j];

                if (o2->production_start_time > plan_end_date)
                    continue;

                temp_mv.end_index = j;

                if (isTabu(temp_mv))
                    continue;

                //reportRelocationMove(temp_mv);
                relocateOrder(temp_mv);
                //testReportCurrentSol();

                currentSol->simulatePlan(true, false);
                int swap_cost = currentSol->cost;
                //int total_delay = testReportCurrentSol();
                //printf("Temp Solution Delay : %d\n", total_delay);

                //Revert Move
                revertRelocation(temp_mv);



                //Save move if better
                if (swap_cost < minim_cost) {
                    //From
                    mv.start_index = temp_mv.start_index;
                    mv.end_index = temp_mv.end_index;
                    mv.cost = swap_cost;
                    move_found = true;
                    //Return Immediately if we have found an improving move

                    improving_streak = true;
                    minim_cost = swap_cost;
                } else
                    improving_streak = false;

                if (minim_cost < old_cost && !improving_streak)
                    return move_found;
            }
        }

        return move_found;
    }


    void reportRelocationMove(ls_move &mv){
        order *o1 = currentSol->order_list[mv.start_index];
        printf("Relocating order %d from Position %d to Position %d\n",
                o1->ID, mv.start_index, mv.end_index);
    }

    int testReportCurrentSol(){
        //Testing - Report slack per order
        int rough_total_delay = 0;
        for (int i=0;i<currentSol->order_list.size();i++){
            order *o = currentSol->order_list[i];

            int order_slack = currentSol->getOrderSlackTime(o);

            float diff_from_prev = 0;

            if (i>0){
                diff_from_prev = (float) (o->production_end_time.datetime - currentSol->order_list[i - 1]->production_end_time.datetime);
                diff_from_prev /= 60.0f; //Convert to minutes
                rough_total_delay += diff_from_prev;
            }

            printf("Order %4d SeqID: %4d Delayed: %d Slack Time: %4d Production End Time %10s Diff from Last %5.4f\n",
                   o->ID, o->seqID, o->delayed, order_slack, o->production_end_time.toString().c_str(), diff_from_prev);
        }
        return rough_total_delay;
    }

    bool findBestSimilar(ls_move &mv, int &minim_cost){
        return false;
    }

    void calcEfficiencyCounters(){
        //Do not consider any index prior to the start of the planning horizon
        order_start_id = 0;
        for (order_start_id=0; order_start_id<currentSol->order_count; order_start_id++){
            order *o = currentSol->order_list[order_start_id];
            if (o->production_start_time > plan_start_date)
                break;
        }
    }


    void tabu_search(){
        printf("Initiating TS. Starting Solution Cost %d\n",currentSol->cost);

        //Start time clock
        auto start_time = chrono::steady_clock::now();

        while(true) {
            bool feasible = true;

            currentSol->cost = currentSol->calculateCost(false); //This function runs the feasibility check internally
            if (currentSol->numberOfInfeasibilities > 0)
                feasible = false;

            //TODO: Make sure that the lower order index limit makes sense
            //Calculate order limits
            calcEfficiencyCounters();

            //Select a move
            bool move_found = false;
            ls_move mv = ls_move();
            int move_mode;

            move_found = findFeasibleMove(mv, move_mode); //Union Neighborhood

            //Apply the move if found
            if (move_found){
                applyMove(mv, move_mode);
                printf("Move %d applied. New Solution Cost %d\n", move_mode, currentSol->cost);
                currentSol->reportFirstDelayedOrders();
                currentSol->reportRelocatedOrders();

                //Save Solution
                if (currentSol->cost < localElite->cost){
                    // Improving move, apply
                    *localElite = *currentSol;
                    //localElite->integrityCheck(); This doesn't do anything useful. By default all orders will get scheduled for now
                    printf("New Improving Solution. Cost %d. New JPH %4.3f. New Main Cost %d \n",
                            localElite->cost, localElite->jph, localElite->calculateCost(true));
                    iter_counter = 0;
                } else
                    iter_counter++;
            }

            if ((iter_counter % 50 == 0) && (iter_counter > 0)){
                //Try to replace dummy operations with unscheduled orders.
                int fixed_orders = currentSol->insertNonScheduledOrders();
                if (fixed_orders > 0){
                    printf("Managed to Insert %d unscheduled orders\n", fixed_orders);
                    iter_counter = 0;
                }
            }

            //Check exit conditions
            if (iter_counter > tabu_iterations)
                break;
            else
                iter_counter++;


            //Check time limit
            auto end_time = chrono::steady_clock::now();
            int time_elapsed = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();
            if (time_elapsed > time_limit)
                break;
        }
    }
};


#endif //CRF_RESCHEDULING_LOCALSEARCH_H
