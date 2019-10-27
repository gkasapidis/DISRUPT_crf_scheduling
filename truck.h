//
// Created by gregkwaste on 5/8/19.
//

#ifndef CRF_RESCHEDULING_TRUCK_H
#define CRF_RESCHEDULING_TRUCK_H

#include <string>
#include <unordered_map>

using namespace std;

class truck {
public:
    string name;
    int ID;
    int content_id;
    bool unloaded;
    my_date_time unloading_time;
    unordered_map<string, int> components;


    truck(){
        ID = -1;
        name = "";
        unloaded = false;
        components = unordered_map<string, int>();
        unloading_time.reset();
    }

    void report(){
        printf("Truck %s, ID %d. Load: \n", name.c_str(), ID);
        for (auto c : components){
            printf("%s : %d \n", c.first.c_str(), c.second);
        }
    }

    void load_component(string comp_name, int comp_quant){
        if (components.find(comp_name) != components.end()){
            components[comp_name] += comp_quant;
        } else{
            components[comp_name] = comp_quant;
        }
    }
};

#endif //CRF_RESCHEDULING_TRUCK_H
