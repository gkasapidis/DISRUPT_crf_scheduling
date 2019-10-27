#include <iterator>
#include <tuple>
#include <cstring>
#include <memory.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iomanip>


//Custom Includes
#include "Parameters.h" //Parameters and very generic classes
#include "csv_utils.h" //CSV Importing functions
#include "db_interface.h" //Database Connection
#include "LocalSearch.h" //Local Search Algorithm

#include "json.hpp" //Json Library (https://github.com/nlohmann/json)
#include "events.h" //DISRUPT event definitions



using namespace std;
using namespace nlohmann;

enum CONSTRAINT_TYPE{WEEKLY_MAX_CAPACITY, DAILY_MAX_CAPACITY};
enum CONSTRAINT_TAG{BUY, ALLOCATE, MAKE};

int componentNum = 0;


//Setup Scheduling Parameters
//Planning Horizon
//int PLAN_START_DAY = 15; //OLD


int DB_START_DAY = 15;
int DB_START_MONTH = JANUARY;
int DB_START_YEAR = 2018;

int PLAN_START_DAY = 15;
//int PLAN_START_DAY = 22;
int PLAN_START_MONTH = JANUARY;
int PLAN_START_YEAR = 2018;

int PLAN_END_DAY = 19;
int PLAN_END_MONTH = JANUARY;
int PLAN_END_YEAR = 2018;

my_date_time db_start_date;
my_date_time plan_start_date;
my_date_time plan_end_date;
my_date_time artificial_plan_end_date;

//Init global planning horizon
planning_horizon *global_planning_horizon;

int  MAX_COMPONENTS = 100;
//Max limits have been set to 5 weeks
int MAX_ORDERS = 5000;
int MAX_DAYS = 5*7;
int MAX_JPD = 140;
int MAX_ORDERS_PER_PRODUCTION_PERIOD = 18;
int MAX_STATIONS = 9;
int CYCLE_TIME = 6;
float REAL_CYCLE_TIME = 6.1;

int TIME_INTERVALS_PER_DAY = 160;
int ASSEMBLY_LINE_LENGTH = 92;
int JOB_PER_DAY = 138;


//Input Parameters
//Global parameters defined from main arguments
int jobID = 0;
int alternativeActionID = 0;
int executionMode = 0;
int scheduleCount = 0;
string productionStartTime;
string productionEndTime;
int eventCount = 0;
int *eventIDs;
int scopetype = 0;

//Dabatase connections
database_connection *disrupt_db_conn, *crf_db_conn;
//DB Connection Info
string user;
string password;
string hostname;
int port;

//Algorithm Parameters
int time_limit; //Time limit of Tabu Search (in seconds)
int tabu_iterations; //Number of internal tabu search iterations without observing improvement (must be >=1)
int tabu_tenure; //Tabu Move Life/Search History (must be between 12 and 60) must be >= tabu_list_size to make sense


bool parse_initial_stock(problem *p, const char* filename) {
    //Try to parse constraints
    std::ifstream infile(filename);
    if (!infile) {
        printf("Input file not found\n");
        return false;
    }

    std::string line;

    int line_counter = 0;
    std::getline(infile, line);
    std::istringstream iss(line);
    std::vector<std::string> split;

    int entries;
    iss >> entries;

    for (int i = 0; i < entries; i++) {
        std::getline(infile, line);
        iss.clear();
        iss.str(line);
        split = getNextLineAndSplitIntoTokens(iss);

        string c_id = split[0];
        int amount = stoi(split[1]);

        //Set initial stock for component
        component *c = p->components[c_id];
        c->delivery_hor[0]->start_inv = amount;
    }

    return true;
}

void usage(){
    printf("------CRF Rescheduling Module-------\n");
    printf("Usage: ./crf_rescheduler mode\n");
    printf("Mode Options:\n");
    printf("\t0 Current Schedule Evaluation\n");
    printf("\t1 Rescheduling\n");
    printf("------------------------------------\n");
}



void save_to_file(string filename, string content){
    std::ofstream outfile(filename);
    outfile << content << endl;
    outfile.close();
}

//void saveScheduleToDB(int scheduleLocalID){
//
//    ostringstream Sin;
//    stringstream query;
//    string metrics = "";
//    string schedule = "";
//
//
//    //DB Stuff Init
//    connect_to_db(db_settings.disrupt_db_name);
//    string table_name = "optimized_plan";
//    query.str("");
//
//    //Fetch Schedule
//    saveToStream(Sin);
//    schedule = Sin.str();
//
//    //Fetch Metrics
//    Sin.str("");
//    Sin << "MakeSpan: " <<  globals->elite->cost_array[MAKESPAN]/(60.0 *10) << "min" << endl;
//    Sin << "TFT: " <<  globals->elite->cost_array[TOTAL_FLOW_TIME]/(60.0 *10) << "min" << endl;
//    Sin << "Tidle: " <<  globals->elite->cost_array[TOTAL_IDLE_TIME]/(60.0 *10) << "min" << endl;
//    saveToStream(Sin);
//    metrics = Sin.str();
//    cout << metrics << endl;
//
//    //Prepare Plot
//    char *plot_cmd = new char[50];
//    if (executionMode == 1)
//        sprintf(plot_cmd, "./gant_charter.py %d", reschedulingTime);
//    else
//        sprintf(plot_cmd, "./gant_charter.py %d", 0);
//    system(plot_cmd);
//
//    //Fetch Image
//    ifstream f;
//    f.open("gant.png", std::ios::binary);
//
//    f.seekg(0, std::ios::end);
//    size_t fsize = (size_t) f.tellg();
//
//    //Allocate char array
//    char *data = new char[fsize];
//    char *chunk = new char[2*fsize + 1];
//    char *c_query = new char[fsize*5];
//
//    //Fetch data
//    f.seekg(0, std::ios::beg);
//    f.read(data, fsize);
//    f.close();
//
//    //Flatten data
//    mysql_real_escape_string(&m, chunk, data, fsize);
//
//    char *q_stat = "INSERT INTO %s (date, jobID, alternativePlanID, optimizedPlan, gannt) VALUES (NOW(), %d, %d, '%s', '%s');";
//    //int len = snprintf(q_stat, sizeof(q_stat)+sizeof(chunk), chunk);
//    int len = sprintf(c_query, q_stat, table_name, jobID, alternativeActionID, schedule.c_str(), chunk);
//
//    //query << "INSERT INTO " << table_name << " (date, jobID, alternativePlanID, optimizedPlan) VALUES ( "
//    //                        << "NOW()" <<", '" << jobID << "', '" << alternativeActionID << "', '"
//    //                        << schedule << "' );";
//    execute_query(&m, c_query, "success");
//    //int res = mysql_query(&m, c_query);
//    //if( res == 0) cout << "Query Successfull" << endl;
//
//    disconnect_from_db();
//
//    delete[] data;
//    delete[] chunk;
//    delete[] c_query;
//    delete[] plot_cmd;
//};
//



void export_to_database(problem *base_p, problem *p){
    //Temp variables
    stringstream query;
    string table_name = "optimized_plan";

    //Get schedule string
    string schedule = p->toString(0);

    //Save string to optimized plan table
    ostringstream Sin;

    //Fetch Metrics
    Sin.str("");
    Sin << "JPH: " <<  p->calculateJPH() << endl;
    Sin << schedule;

    //Save original schedule to file
    save_to_file("output_base.txt", to_string(base_p->calculateJPH()) + '\n' + base_p->toString(1));

    //Save schedule to file
    save_to_file("output.txt", to_string(p->calculateJPH()) + '\n' + p->toString(1));

    //Prepare Plot
    char *plot_cmd = new char[200];
    printf("Plotting...");
    sprintf(plot_cmd, "./gant_charter.py %d", 0);
    system(plot_cmd);
    printf("Done\n");

    //Fetch Image
    ifstream f;
    f.open("gant.png", std::ios::binary);

    f.seekg(0, std::ios::end);
    size_t fsize = (size_t) f.tellg();

    if (fsize < 0){
        printf("Unable to read file or non existing plot\n");
        return;
    }

    //Allocate char array
    char *data = new char[fsize];
    char *chunk = new char[2*fsize + 1];
    char *c_query = new char[fsize*5];

    //Fetch data
    f.seekg(0, std::ios::beg);
    f.read(data, fsize);
    f.close();

    //Flatten data
    mysql_real_escape_string(&disrupt_db_conn->m, chunk, data, fsize);

    char *q_stat = "INSERT INTO %s (date, jobID, alternativePlanID, optimizedPlan, gannt) VALUES (NOW(), %d, %d, '%s', '%s');";
    //int len = snprintf(q_stat, sizeof(q_stat)+sizeof(chunk), chunk);
    int len = sprintf(c_query, q_stat, table_name.c_str(), jobID, 2, schedule.c_str(), chunk);

    execute_query(disrupt_db_conn, c_query);

    //int res = mysql_query(&m, c_query);
    //if( res == 0) cout << "Query Successfull" << endl;


    int optimizedPlanID = mysql_insert_id(&disrupt_db_conn->m);
    printf("OptimizedPlanID %d\n", optimizedPlanID);

    //Save schedule to production_plan table
    table_name = "production_plan";

    printf("Exporting Production Orders to Database\n");
    //Store schedule to the disrupt database
    for (int i=0; i < (int) p->order_list.size(); i++){
        order *o = p->order_list[i];

        if (o->production_start_time < plan_start_date)
            continue;

        if (o->dummy || o->ID < 0)
            continue;

        //Append order to query
        query.str("");
        query << "INSERT INTO " << table_name << " (productionPlanID, productionTime, mixCodeID, seqID, "
                                                 "optimizedPlanID) VALUES ( "
              //<< productionPlanID << ", " //TODO: What is that ID
              << 1 << ", "
              << "'" << o->production_end_time.toString(DATE_FORMAT::YYYY_MM_DD_DASH) << "', "
              << o->mixCode->ID << ", "
              << o->seqID + 1 << ", " //Use 1-index for the order sequencing
              << optimizedPlanID << " );";
        //cout << query.str() << endl;
        execute_query(disrupt_db_conn, query); //Add all orders in the database at once
    }

    printf("Exported %d Orders to the Database \n", (int) p->orders.size());

    //Cleanup
    delete[] data;
    delete[] chunk;
    delete[] c_query;

}


problem* parse_database(database_connection *db_conn, database_connection *db_conn_disrupt) {
    //Input Parameters Report
    printf("Planning Horizon:\n");
    printf("Start Date: %s\n", plan_start_date.toString().c_str());
    printf("End Date: %s\n", plan_end_date.toString().c_str());

    problem *p = new problem(); //Create new problem

    //Local variables for querying the database
    stringstream query;
    string table_name;
    int num_fields;
    MYSQL_ROW row;

    //Query truck table
    printf("Parsing Trucks...\n");
    {
        query.str("");
        table_name = "truck";
        query << "SELECT * FROM " << table_name << " ORDER BY id ";
        execute_query(db_conn, query);
        num_fields = mysql_num_fields(db_conn->result);
            
        //Iterate in data
        int truck_counter = 0;
        while ((row = mysql_fetch_row(db_conn->result))) {
            truck *t = new truck();
            t->ID = (row[0] != nullptr) ? stoi(row[0]) : -1;
            t->name = (row[1] != nullptr) ? row[1] : "";
            t->content_id = (row[2] != nullptr) ? stoi(row[2]) : -1;

            p->trucks[t->ID] = t;
            truck_counter++;
        }

#ifdef DEBUGNONO
        for (int i=0;i < p->trucks.size();i++){
            p->trucks[i]->report();
        }
#endif
        db_conn->free_result();
        printf("Total Trucks loaded %d\n", truck_counter);
        p->truck_count = truck_counter;
    }


    //Query truck table
    printf("Parsing Truck Load...\n");
    {
        query.str("");
        table_name = "truck_load";
        query << "SELECT * FROM " << table_name << " ORDER BY id ";
        execute_query(db_conn, query);
        num_fields = mysql_num_fields(db_conn->result);

        //Iterate in data
        while ((row = mysql_fetch_row(db_conn->result))) {
            int truck_id = (row[1] != nullptr) ? stoi(row[1]) : -1;
            string comp_name = (row[2] != nullptr) ? trim(row[2]) : "";
            int comp_quant = (row[3] != nullptr) ? stoi(row[3]) : -1;

            //Assign component to truck
            truck *t = p->trucks[truck_id];
            t->load_component(comp_name, comp_quant);
        }

#ifdef DEBUGNONO
        for (int i=0;i < p->trucks.size();i++){
            p->trucks[i]->report();
        }
#endif
        db_conn->free_result();
    }


    //Initialize Stations
    printf("Initializing Stations...\n");
    {
        for (int i=0;i<MAX_STATIONS;i++){
            station * s = new station();
            s->ID = i;
            s->last_processing_time = plan_start_date;

            //Stations are created and ordered in their visiting orders
            switch(i){
                //Start of the line
                case 0:
                    s->position = 0;
                    break;
                //Sunroofs
                case 1:
                    s->position = 11 - 1;
                    break;
                //Cross beam + Dashboard Kit
                case 2:
                    s->position = 26 - 1;
                    break;
                //Steering Column
                case 3:
                    s->position = 27 - 1;
                    break;
                //Dashboard Shell + AirBag
                case 4:
                    s->position = 29 - 1;
                    break;
                //Wheels
                case 5:
                    s->position = 76 - 1;
                    break;
                //Seats 3,4 Station
                case 6:
                    s->position = 81 - 1;
                    break;
                //Seats 1,2 Station
                case 7:
                    s->position = 82 - 1;
                    break;
                //Line End
                case 8:
                    s->position = 92 - 1;
                    break;
                default:
                    printf("Unhandled station ID. Check the data\n");
                    break;
            }
            p->stations.push_back(s);
        }

        //Setup Station to floor distances
        for (int i=0;i<MAX_STATIONS;i++){
            station *s1 = p->stations[i];
            //Setup Assembly Line to Station maps
            p->AssemblyLineStationMap[s1->ID] = s1->position;
            p->AssemblyLinePositionMap[s1->position] = s1->ID;

            for (int j=0;j<MAX_STATIONS;j++){
                station *s2 = p->stations[j];
                p->station_distances[s1->ID][s2->ID] = s2->position - s1->position - 1;
            }
        }

#ifdef DEBUG
        //Report Stations
        for (int i=0;i<MAX_STATIONS;i++){
            p->stations[i]->report();
        }

        //Report Assembly Line Maps
        printf("Assembly Line Station Map\n");
        for (auto e : p->AssemblyLineStationMap){
            printf("%d : %d\n", e.first, e.second);
        }

        printf("Assembly Line Position Map\n");
        for (auto e : p->AssemblyLinePositionMap){
            printf("%d : %d\n", e.first, e.second);
        }

        printf("Station Distances\n");
        for (int i=0;i<MAX_STATIONS;i++){
            for (int j=0;j<MAX_STATIONS;j++){
                printf("Distance between stations %d,%d = %d\n",
                        i ,j, p->station_distances[i][j]);
            }
        }
#endif

    }

    //Query Distances
    printf("Parsing Part Distances...\n");
    {
        query.str("");
        table_name = "distance";
        query << "SELECT * FROM " << table_name;
        execute_query(db_conn, query);
        num_fields = mysql_num_fields(db_conn->result);

        //Iterate in data
        int component_counter = 0;
        while ((row = mysql_fetch_row(db_conn->result))) {
            int comp_type_id = (row[0] != nullptr) ? stoi(row[1]) : -1;
            int distance = (row[1] != nullptr) ? stoi(row[2]) : -1;

            //Save distances
            p->component_warehouse_to_station_distance[comp_type_id] = distance;

        }

#ifdef DEBUG
        for (auto e: p->component_warehouse_to_station_distance){
            printf("Component Type ID %d - Distance to Station %d\n", e.first, e.second);
        }
#endif
        db_conn->free_result();
    }

    //Initialize Station Distances

    //Query part numbers
    printf("Parsing Parts...\n");
    {
        query.str("");
        table_name = "part";
        query << "SELECT * FROM " << table_name;
        execute_query(db_conn, query);
        num_fields = mysql_num_fields(db_conn->result);

        //Iterate in data
        int component_counter = 0;
        while ((row = mysql_fetch_row(db_conn->result))) {
            string comp_name = (row[0] != nullptr) ? trim(row[0]) : "";
            int comp_type_id = (row[1] != nullptr) ? stoi(row[1]) : -1;

            //Create component
            component *c = new component();
            c->ID = comp_name;
            c->component_type_id = comp_type_id;

            //Set the position of the assembly station of the component in minutes
            switch(c->component_type_id){
                //Seats 1, 2
                case 1:
                case 2:
                    c->station_id = 7;
                    break;
                //Seats 3, 4
                case 3:
                case 4:
                    c->station_id = 6;
                    break;
                //Wheels
                case 5:
                case 6:
                    //c->station_position = 76;
                    c->station_id = 5;
                    break;
                //Dashboard Shell
                case 10:
                    //c->station_position = 29;
                    c->station_id = 4;
                    break;
                //Steering Column
                case 11:
                    //c->station_position = 27;
                    c->station_id = 3;
                    break;
                //Cross Beam
                case 7:
                    //c->station_position = 26;
                    c->station_id = 2;
                    break;
                //Air Bag
                case 9:
                    //c->station_position = 29;
                    c->station_id = 4;
                    break;
                //Sun Roof
                case 8:
                    //c->station_position = 11;
                    c->station_id = 1;
                    break;
                //Dashboard Kit components
                default:
                    //c->station_position = 26;
                    c->station_id = 2;
                    break;
            }

            //Store component
            p->components[c->ID] = c;

            component_counter++;
        }

#ifdef DEBUGNONO
        for (auto c: p->components){
            c.second->report();
        }
#endif
        db_conn->free_result();
        printf("Total Parts loaded %d\n", component_counter);
        p->component_count = component_counter;
    }

    //Query part inventory
    printf("Parsing Part Inventory...\n");
    {
        query.str("");
        table_name = "part_inventory";
        query << "SELECT * FROM " << table_name;
        execute_query(db_conn, query);
        num_fields = mysql_num_fields(db_conn->result);

        //Iterate in data
        while ((row = mysql_fetch_row(db_conn->result))) {
            //printf("Loading Inventory for produt %s\n", row[3]);
            string comp_name = (row[3] != nullptr) ? trim(row[3]) : "";
            string location = (row[4] != nullptr) ? trim(row[4]) : "";
            int comp_quantity = stoi(row[1]);

            //Load Component
            component *c = p->components[comp_name];

            if (location == "Lineside"){
                //The component is available immediately
                //Simulation does not use lineside components
                //c->delivery_hor[0]->start_inv += comp_quantity;
            } else if (location == "Warehouse"){
                //Add the time distance that is required from the warehouse to the particular component station
                int station_distance = p->component_warehouse_to_station_distance[c->component_type_id];
                my_date_time station_delivery_time = plan_start_date;
                station_delivery_time.addSeconds(station_distance);
                c->addDelivery(&station_delivery_time, comp_quantity);
            } else{
                printf("Unknown location\n");
                assert(false);
            }
        }


#ifdef DEBUGNONO
        for (auto c: p->components){
            printf("Initial Stock for Component %s : %d\n",
                    c.second->ID.c_str(), c.second->delivery_hor[0]->start_inv);
        }
#endif
        db_conn->free_result();
    }

    //Parse Events
    //Query order table
    printf("Parsing Events...\n");
    {
        for (int i=0;i<eventCount;i++){
            int event_id = eventIDs[i];

            //Fetch event from database
            query.str("");
            table_name = "event";
            query << "SELECT * FROM " << table_name << " WHERE eventID = " << event_id;
            execute_query(db_conn_disrupt, query);
            num_fields = mysql_num_fields(db_conn_disrupt->result);

            row = mysql_fetch_row(db_conn_disrupt->result);

            if (row == nullptr)
                continue;

            //Parse event
            if (stoi(row[3]) == 2){

                if ((string) row[8] == "{}"){
                    printf("Empty Event Arguments nothing to do with event\n");
                    continue;
                }
                //Parse delay event
                json descr = json::parse(row[8]); //Parse Event Parameters

                int truck_id = stoi(row[6]);
                string delay_text = descr.at("/estimated_delay"_json_pointer);
                int delay = stoi(delay_text);

                delay_event *e = new delay_event(event_id, truck_id, delay);

                p->delay_events.push_back(e);
                //p->applyEvent(e); //Immediately Apply Event
            } else if (stoi(row[3]) == 8){
                //Parse paintshhop event
                json descr = json::parse(row[8]); //Parse Event Parameters

                int target_id = stoi(row[6]);

                paint_event *e = new paint_event(event_id, target_id);

                //Do not save paint events for now
            } else
                printf("Warning Not Supported Event. Contact Developer!\n");
        }

        db_conn_disrupt->free_result();
    }

    //Query part numbers
    printf("Parsing Dock Schedule...\n");
    {
        query.str("");
        table_name = "dock_schedule";
        query << "SELECT * FROM " << table_name << " ORDER BY id";
        execute_query(db_conn, query);
        num_fields = mysql_num_fields(db_conn->result);


        bool t31_delayed = false; //Capture delays on the interested truck

        //Iterate in data
        while ((row = mysql_fetch_row(db_conn->result))) {
            int id = (row[0] != nullptr) ? stoi(row[0]) : -1;
            int truck_id = (row[2] != nullptr) ? stoi(row[2]) : -1;
            string unload_time = (row[4] != nullptr) ? trim(row[4]) : "";
            my_date_time ut_converted = my_date_time(unload_time, YYYY_MM_DD_DASH);

            //Fix ATC's time conversion
            if (ut_converted.data.tm_hour == 0){
                ut_converted.addMinutes(12 * 60); // 00 is actually 12 pm
                ut_converted.datetime = timegm(&ut_converted.data);
                ut_converted.updateActualDateTimeIDs();
            }

            if (ut_converted < db_start_date) continue; //Filter rubbish data
            if (ut_converted > plan_end_date) continue;

            bool truck_delayed = false;

            //Check if truck is affected
            for (int i=0;i<p->delay_events.size();i++){
                delay_event *e = p->delay_events[i];
                if (e->truckID == truck_id){
                    //Apply delay
                    truck_delayed = true;
                    if (truck_id == 92){
                        t31_delayed = true;
                        ut_converted.addMinutes(1440);
                    } else
                        ut_converted.addMinutes(e->delay);
                }
            }

            //The load is available 60 minutes after the unload time
            ut_converted.addMinutes(60);

            //Clamp arriving hour to start of the shift in case it is completely out of the last shift
            if (ut_converted.data.tm_hour >= 22){
                ut_converted.addMinutes(12 * 60); // Get to the next day
                //Reset time to 6 am
                ut_converted.data.tm_sec = 0;
                ut_converted.data.tm_min = 0;
                ut_converted.data.tm_hour = 6;
                ut_converted.datetime = timegm(&ut_converted.data);
                ut_converted.updateActualDateTimeIDs();
            }

            //Skip deliveries after the planning horizon
            if (ut_converted > plan_end_date)
                continue;

            printf("Truck %d arrived at Docks in %s\n", truck_id, ut_converted.toString().c_str());

            //Find truck
            truck *t = p->trucks[truck_id];
            t->unloaded = true; //Set truck to active status (it unloads components)
            //Assign unloading time
            t->unloading_time = ut_converted;

            //Iterate in loaded components
            for (auto c : t->components) {
                //Load component
                if (p->components.find(c.first) == p->components.end()) {
                    printf("Component %s Not Found in DB\n", c.first.c_str());
                    continue;
                }

                component *comp = p->components[c.first];

                //Add the time distance that is required from the warehouse to the particular component station
                int station_distance = p->component_warehouse_to_station_distance[comp->component_type_id];
                my_date_time station_delivery_time = ut_converted;
                station_delivery_time.addSeconds(station_distance);
                station_delivery_time.updateActualDateTimeIDs();

                //Fix station delivery hour to fit on an actual shift
                while (station_delivery_time.actual_hour_id < 0){
                    station_delivery_time.addMinutes(1.0);
                }

                //Explicitly handle Older trucks
                if (station_delivery_time < plan_start_date){
                    comp->delivery_hor[0]->start_inv += c.second;
                    continue;
                }

                comp->addDelivery(&station_delivery_time, c.second);
                //Set component affected status
                if (truck_delayed)
                    comp->affected = true;
            }
        }

#ifdef DEBUG

        for (auto c: p->components) {
            c.second->report(global_planning_horizon->daycount);
        }

#endif
        db_conn->free_result();


        //MANUALLY REDUCE 06701406820 START INVENTORY
        if (t31_delayed)
            p->components["06701406820"]->delivery_hor[0]->res[0] -= 19;
    }

    //Identify and ignore components that are never delivered
    for (auto c : p->components) {
        component *comp = c.second;

        comp->ignored = !comp->hasDelivery();

        if (comp->ignored){
            printf("Component %s has no delivery\n", c.first.c_str());
        }
    }



    //Query Mix Codes
    printf("Mix Codes...\n");
    {
        query.str("");
        table_name = "mix_code";
        query << "SELECT * FROM " << table_name;
        execute_query(db_conn, query);
        num_fields = mysql_num_fields(db_conn->result);

        //Iterate in data
        int mc_counter = 0;
        while ((row = mysql_fetch_row(db_conn->result))) {
            int id = (row[0] != nullptr) ? stoi(row[0]) : -1;

            //Create mix_code
            mix_code *mc = new mix_code();
            mc->ID = id;
            p->mixCodes[id] = mc;
            mc_counter++;
        }

#ifdef DEBUGNONO
        for (auto c: p->components){
            c.second->report();
        }
#endif
        db_conn->free_result();
        printf("Total MixCodes loaded %d\n", mc_counter);
        //Add an extra empty mixcode for dummy operations
        mix_code *mc = new mix_code();
        mc->ID = -1;
        p->mixCodes[-1] = mc;
    }


    printf("Connecting Mix Codes with part numbers...\n");
    {
        table_name = "mix_code_part_number";
        query.str("");
        query << "SELECT * FROM " << table_name;
        execute_query(db_conn, query);
        num_fields = mysql_num_fields(db_conn->result);

        while ((row = mysql_fetch_row(db_conn->result))) {
            int mix_code_id = (row[1] != nullptr) ? stoi(row[1]) : -1;
            string part_id = (row[2] != nullptr) ? trim(row[2]) : "";

            assert(mix_code_id != -1);
            //Load MixCode
            mix_code *mc = p->mixCodes[mix_code_id];
            mc->add_component(part_id);
        }

#ifdef DEBUGNONO

        for (auto mc: p->mixCodes) {
            mc.second->report();
        }

#endif
        db_conn->free_result();
    }

    //Query order table
    printf("Parsing Production Orders...\n");
    {
        query.str("");
        table_name = "production_plan";
        query << "SELECT * FROM " << table_name << " ORDER BY id ";
        execute_query(db_conn, query);
        num_fields = mysql_num_fields(db_conn->result);

        int global_seqID = 0;
        int day_counter = -1;
        my_date_time old_date = my_date_time();
        int current_line_position = 91; //Zero indexing ( 92 - 1 )

        //Iterate in data
        while ((row = mysql_fetch_row(db_conn->result))) {
            int id = (row[0] != nullptr) ? stoi(row[0]) : -1;
            string datetime = (row[3] != nullptr) ? trim(row[3]) : "";
            my_date_time dt_converted = my_date_time(datetime, YYYY_MM_DD_DASH);
            dt_converted.updateActualDateTimeIDs();
            int mix_code_id = (row[4] != nullptr) ? stoi(row[4]) : -1;
            int cis_id = (row[5] != nullptr) ? stoi(row[5]) : -1;

            //Check if order date is within the planning horizon
            if (dt_converted > plan_end_date)
                continue;
            if (dt_converted < db_start_date) continue; //Filter rubbish data

            //Check if order has been completed before the start of the planning horizon
            //In this case the inventory of all corresponding components should be decreased
            if (dt_converted < plan_start_date){
                //Reduce initial inventory for the corresponding components
                mix_code *order_mixcode = p->mixCodes[mix_code_id];
                for (int i=0; i < order_mixcode->componentNum; i++) {
                    component *c = p->components[order_mixcode->componentIDs[i]];
                    c->delivery_hor[0]->start_inv = max(0, c->delivery_hor[0]->start_inv - 1);
                }
                continue;
            }

            //Detect if day changed
            if (old_date.data.tm_mday != dt_converted.data.tm_mday) {
                day_counter++;
                old_date = dt_converted;
            }

            //Generate Order
            order *o = new order();
            o->ID = id;
            o->old_seqID = global_seqID;
            o->seqID = global_seqID;
            o->CIS = cis_id;
            o->production_end_time = dt_converted;


            my_date_time eff_production_start_time;

            //Assign line position
            if (global_seqID + 1 < 92){
                o->assembly_line_position = current_line_position;


                //In the case where the planning horizon starts after the database horizon,
                //make sure to decrease the component inventories for the orders that are positioned on the line
                //at this moment, based on their position on the line

                if (plan_start_date > db_start_date){

                    //Iterate over the stations
                    for (int i=0;i<MAX_STATIONS;i++){
                        int station_pos = p->stations[i]->position;

                        if (current_line_position > station_pos){
                            mix_code *order_mixcode = p->mixCodes[mix_code_id];
                            for (int j=0; j < order_mixcode->componentNum; j++) {
                                component *c = p->components[order_mixcode->componentIDs[j]];
                                if (c->station_id != i)
                                    continue;
                                else
                                    c->delivery_hor[0]->start_inv = max(0, c->delivery_hor[0]->start_inv - 1);
                            }


                        }

                    }
                }

                current_line_position--;
                eff_production_start_time = plan_start_date;
            } else {
                eff_production_start_time = plan_start_date;
                eff_production_start_time.addMinutes((global_seqID - 92) * REAL_CYCLE_TIME);
            }

            //Increase global counter
            global_seqID++;

            /*
            if (daily_order_counter_map[day_counter] == JOB_PER_DAY) {
                //Add dummy orders to the last production periods of the day shifts
                production_period *last_pr;

                //Last period of the first shift
                last_pr = p->order_map[day_counter][0]->pr_periods[3];
                while (last_pr->orders.size() < (size_t) last_pr->max_order_limit)
                    last_pr->appendOrder(-1);

                //Last period of the second shift
                last_pr = p->order_map[day_counter][1]->pr_periods[3];
                while (last_pr->orders.size() < (size_t) last_pr->max_order_limit)
                    last_pr->appendOrder(-1);

                day_counter += 1;
            }*/

            o->mixCode = p->mixCodes[mix_code_id];

            assert(o->mixCode != NULL);

            //Schedule order in the earliest possible time (do not check for feasibility)
            p->scheduleOrder(o, &eff_production_start_time, false, false);

            p->orders[o->ID] = o;
            p->order_list.push_back(o);
        }

#ifdef DEBUG
        printf("Order Report\n");
        for (int i = 0; i < (int) p->order_list.size(); i++) {
            p->order_list[i]->report();
        }
        /*
        printf("Station 0 Visit Times\n");
        for (int i = 0; i < (int) p->order_list.size(); i++) {
            printf("Order %5d visited station at %s\n",
                    p->order_list[i]->ID, p->order_list[i]->station_visit_times[0].toString().c_str());
        }
         */
#endif
        db_conn->free_result();
        p->order_count = p->order_list.size(); //Set Number of input orders
    }

    return p;

}

void parse_events(problem *p, database_connection *db_conn){
    //Local variables for querying the database
    stringstream query;
    string table_name;
    int num_fields;
    MYSQL_ROW row;


}



//This method calculates and
void evaluateSchedule(problem *p){
    printf("--EVALUATION MODE--\n");
    p->metricsReport();
}

//-------------------------------------
//-------------------------------------
//SETTINGS PARSERS
//TODO: This is stupid, I should shorten those methods and use istringstream directly
//and probably just keep the string_setting method

int read_int_setting(ifstream &file){
    istringstream Sin;
    string setting_name;
    int setting;
    char Line[1500];
    file.getline(Line, 1500);
    Sin.clear();
    Sin.str(Line);
    Sin >> setting_name >> setting;
    printf("\t%s = %d\n",setting_name.c_str(), setting);
    return setting;
}

string read_string_setting(ifstream &file){
    istringstream Sin;
    string setting_name;
    string setting;
    char Line[1500];
    file.getline(Line, 1500);
    Sin.clear();
    Sin.str(Line);
    Sin >> setting_name >> setting;
    printf("\t%s = %s\n",setting_name.c_str(), setting.c_str());
    return setting;
}

//Configuration File Parser
int read_settings() {
    //Try to parse algorithm data from settings.ini;
    string file = "./settings.ini";
    //Check If settings file exists
    struct stat buf;
    bool file_exists = false;
    if (stat(file.c_str(), &buf) != -1) {
        ifstream myfile;
        istringstream Sin;
        printf("Reading Settings from settings.ini\n");
        file_exists = true;
        myfile.open(file);
        //Time Limit
        time_limit = read_int_setting(myfile);
        //Tabu Iterations
        tabu_iterations = read_int_setting(myfile);
        //Tabu Tenure
        tabu_tenure = read_int_setting(myfile);
        myfile.close();
    } else {
        printf("Missing Settings File, Using defaults...");
        ofstream myfile;
        myfile.open(file);
        //Set Values for runtime
        time_limit = 1800;
        tabu_iterations = 100;
        tabu_tenure = 10;
        //Write the settings file
        myfile << "TimeLimit " << time_limit << endl;
        myfile << "TabuIterations " << tabu_iterations << endl;
        myfile << "TabuTenure " << tabu_tenure << endl;
        myfile.close();
    }

    //Read Database settings
    //Parse db.ini
    file = "./db.ini";
    ifstream myfile;

    if (stat(file.c_str(), &buf) != -1){
        myfile.open(file);
        char Line[1500];
        istringstream Sin;
        printf("Reading Settings from db.ini\n");
        string setting;
        //host
        hostname = read_string_setting(myfile);
        //user
        user = read_string_setting(myfile);
        //Pass
        password = read_string_setting(myfile);
        //Port
        port = read_int_setting(myfile);
        myfile.close();
    } else {
        printf("Missing Database Configuration File, aborting execution...");
        return false;
    }

    return true;
}

//Main
int main(int argc, char *argv[]) {
    srand(1);

    //Fetch Execution Parameters
    if (argc < 5) {
        printf("Wrong CMD Parameters\n");
        usage();
        //Set input file manually here
        //Linux Test
        return -1;
    }

    if (argc >= 5) {
        //jobID
        jobID = atoi(argv[1]);
        printf("Input JobID %d\n", jobID);
        //alternativeActionID
        alternativeActionID = atoi(argv[2]);
        printf("Input alternativeActionID %d\n", alternativeActionID);
        //ScheduleCount
        scheduleCount = atoi(argv[3]);
        printf("Input scheduleCount %d\n", scheduleCount);
        //ExecutionMode: 0: ScheduleGeneration, 1: Rescheduling
        executionMode = atoi(argv[4]);
        printf("Input executionMode %d\n", executionMode);

        //EventCount
        eventCount = atoi(argv[5]);
        printf("Input eventCount %d\n", eventCount);
        //Parse Events if any
        if (eventCount>0) {
            eventIDs = new int[eventCount];
            for (int i=0; i<eventCount; i++)
                eventIDs[i] = atoi(argv[6 + i]);
        }

        //Parse production start/end time
        productionStartTime = (string) (argv[6 + eventCount]) + ":00"; //augment with seconds
        printf("Input productionStartTime %s\n", productionStartTime.c_str());
        plan_start_date.init(productionStartTime, YYYY_MM_DD_SLASH);

        PLAN_START_DAY = plan_start_date.data.tm_mday;
        PLAN_START_MONTH = plan_start_date.data.tm_mon;
        PLAN_START_YEAR = plan_start_date.data.tm_year + 1900;

        productionEndTime = (string) (argv[7 + eventCount]) + ":00"; //augment with seconds;
        printf("Input productionEndTime %s\n", productionEndTime.c_str());
        plan_end_date.init(productionEndTime, YYYY_MM_DD_SLASH);

        PLAN_END_DAY = plan_end_date.data.tm_mday;
        PLAN_END_MONTH = plan_end_date.data.tm_mon;
        PLAN_END_YEAR = plan_end_date.data.tm_year + 1900;
    }

    //Parse Setting Files
    if (!read_settings())
        return -1;

    //Init DB connection settings
    database_settings crf_db_settings, disrupt_db_settings;

    //LocalHost Settings
    //user = "root";
    //password = "pass";
    //hostname = "localhost";
    //port = 3306;

    crf_db_settings.db_name = "crf_db";
    crf_db_settings.hostname = hostname;
    crf_db_settings.user = user;
    crf_db_settings.pass = password;
    crf_db_settings.port = port;

    disrupt_db_settings.db_name = "disrupt_db";
    disrupt_db_settings.hostname = hostname;
    disrupt_db_settings.user = user;
    disrupt_db_settings.pass = password;
    disrupt_db_settings.port = port;


    disrupt_db_conn = new database_connection();
    crf_db_conn = new database_connection();

    assert(connect_to_db(&disrupt_db_settings, disrupt_db_conn) == 1);
    assert(connect_to_db(&crf_db_settings, crf_db_conn) == 1);

    //At this point I should read the settings input from the message bus

    //Setup database start and end time
    db_start_date = my_date_time(DB_START_DAY, DB_START_MONTH + 1, DB_START_YEAR, 6, 0 ,0);
    //db_start_date.updateActualDateTimeIDs();
    //db_start_date.updateDateTimeFromActualDateTimeIDs();

    /*
    //Setup plan start and end time
    plan_start_date.day = PLAN_START_DAY;
    plan_start_date.month = PLAN_START_MONTH;
    plan_start_date.year = PLAN_START_YEAR;
    plan_start_date.hour = 06;
    plan_start_date.updateActualDateTimeIDs();
    plan_start_date.updateDateTimeFromActualDateTimeIDs();

    plan_end_date.day = PLAN_END_DAY;
    plan_end_date.month = PLAN_END_MONTH;
    plan_end_date.year = PLAN_END_YEAR;
    plan_end_date.hour = 22;
    plan_end_date.updateActualDateTimeIDs();
    plan_end_date.updateDateTimeFromActualDateTimeIDs();
    */

    //Init Problem classes
    global_planning_horizon = new planning_horizon();

    //Fix Static Date Structures
    for (auto t : global_planning_horizon->DATE_TO_ID)
        cout << t.first << " " << t.second << endl;
    cout << endl;

    //Make sure Planning horizon start/end dates are not offdays
    while(plan_start_date.isOffday()){
        plan_start_date.addDays(1);
    }

    while(plan_end_date.isOffday()){
        plan_end_date.removeMinutes(60);
    }

    plan_start_date.updateActualDateTimeIDs();
    plan_end_date.updateActualDateTimeIDs();
    db_start_date.updateActualDateTimeIDs();

    printf("Transformed Planning Horizon:\n");
    printf("Plan Start %s %s\n",
            plan_start_date.getDate().c_str(), plan_start_date.getTime().c_str());
    printf("Plan End %s %s\n",
           plan_end_date.getDate().c_str(), plan_end_date.getTime().c_str());

    problem *base_p = parse_database(crf_db_conn, disrupt_db_conn); //Parse Data from DISRUPT DBs
    base_p->ref = base_p; //Set itself as a ref to make no conflicts during cost evaluation

    //if (!parse_initial_stock(base_p, "initial_stock.csv"))
    //    return -1;

    //Fix Infeasible Windows due to the lack of initial stock
    //base_p->repairInitialStock_CRF();

    //The new order parsing code already applies an initial scheduling without checking for feasibility
    //we can accumulate component consumption right here
    //Accumulate Component Consumptions
    base_p->accumulateConsumption(plan_start_date, plan_end_date);

    base_p->componentReport();

    //Finally Calculate the cost of the schedule
    base_p->cost = base_p->calculateCost(false);
    base_p->metricsReport();

    //Repair initial stock based on the corrupted amounts
    //TODO: Replace this with the correct calculation
    base_p->repairInitialStock();

    base_p->simulatePlan(true, true);

    base_p->affectedDomponentReport();

    //Report base problem
    //base_p->componentReport();
    //base_p->corruptedComponentReport();

    //base_p->reportSchedule(0);
    //export_to_database(base_p);

    //Generate new problem for the tabu search algorithm
    problem *p = new problem();
    *p = *base_p;
    p->ref = base_p; //Set reference solution

    //Resimulate plan
    p->simulatePlan(true, true);

    //Report Solution
    for (int i=0;i<p->order_list.size();i++){
        order *o = p->order_list[i];
        o->report();
    }

    //Testing - Report slack per order
    for (int i=0;i<p->order_list.size();i++){
        order *o = p->order_list[i];

        int order_slack = p->getOrderSlackTime(o);

        float diff_from_prev = 0;

        if (i>0){
            diff_from_prev = (float) (o->production_end_time.datetime - p->order_list[i - 1]->production_end_time.datetime);
            diff_from_prev /= 60.0f; //Convert to minutes
        }

        printf("Order %4d SeqID: %d Delayed: %d Slack Time: %4d Production End Time %10s Diff from Last %5.4f\n",
               o->ID, o->seqID, o->delayed, order_slack, o->production_end_time.toString().c_str(), diff_from_prev);
    }

    p->reportSchedule(1);
    p->metricsReport();

    //Initialize LS object
    LocalSearch *ls = new LocalSearch(tabu_tenure, tabu_iterations, time_limit);

    printf("--RESCHEDULING MODE--\n");
    printf("################################\n");

    //Setup local search
    ls->setup(p);

    //Call TS
    ls->tabu_search();

    //Save returned solution
    problem final_sol = problem();
    final_sol = *ls->localElite;

    //New Schedule Reports
    final_sol.constraintCapacityReport();
    final_sol.constraintSaturationReport();
    final_sol.metricsReport();

    printf("Exporting Schedule\n");

    /* Not required output formats
    //Save schedule to file
    string fname = "outSchedule.out";
    string fname_aggr = "outScheduleAggr.out";
    final_sol.exportSchedule(fname);
    final_sol.exportScheduleAggr(fname_aggr);
     */

    //Export to file and to the database
    export_to_database(p, &final_sol);

    //Cleanup
    delete ls;

    //Cleanup Database
    crf_db_conn->disconnect();
    disrupt_db_conn->disconnect();

    delete crf_db_conn;
    delete disrupt_db_conn;

    //Delete problems
    delete p;
    delete base_p;

    //Delete planning horizon
    delete global_planning_horizon;

    return 0;
}
