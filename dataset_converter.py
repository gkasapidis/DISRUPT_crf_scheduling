import xlrd
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import sys
from copy import deepcopy
from collections import OrderedDict

DEBUG = False;

def myprint(*args):
    if DEBUG:
        print(args)

# EXCEL DATA
# Old File
# wb = xlrd.open_workbook('ILOG Schedule Model Inputs 20180618.xlsx')
#New File
wb = xlrd.open_workbook('Database CNR-V02.xlsx')


# PROBLEM DATA OBJECTS
dock_horizon = OrderedDict() # This should be used as a base datetime stucture for storing component capaticies
plan_horizon = {}
dockSchedule = OrderedDict()
productionSchedule = []
trucks = {}
components = {}
container_capacities = {}
mixCodes = {}
initial_stock = {}


#Constant HOPEFULLY Data
seat_codes = ["067012464801",
            "067014017501",
            "067014018701",
            "067014064101",
            "067014015901",
            "067014062401",
            "067014055901",
            "067014060401",
            "067014023801",
            "067012464802",
            "067014017502",
            "067014018702",
            "067014064102",
            "067014015902",
            "067014062402",
            "067014055902",
            "067014060402",
            "067014023802",
            "067012464803",
            "067014017503",
            "067014018703",
            "067014064103",
            "067014015903",
            "067014062403",
            "067014055903",
            "067014060403",
            "067014023803",
            "067012464804",
            "067014017504",
            "067014018704",
            "067014064104",
            "067014015904",
            "067014062404",
            "067014055904",
            "067014060404",
            "067014023804"]
        
wheel_codes =["067009789301",
            "067009789302",
            "067009706801",
            "067009706802",
            "067009789601",
            "067009789602",
            "067009789101",
            "067009789102",
            "067009706401",
            "067009706402",
            "067009706201",
            "067009706202",
            "067009706501",
            "067009706502",
            "067009788801",
            "067009788802",
            "067009788901",
            "067009788902",
            "067009706101",
            "067009706102"]

dashboard_shells = ["06700802440",
            "06700491600",
            "06700619780",
            "06700491580",
            "06700619790",
            "06700619770",
            "06700491550",
            "06700876210",
            "06700719780",
            "06700802450",
            "06700491610",
            "06700619750",
            "06700491590",
            "06700619800",
            "06700619810",
            "06700491560",
            "06700876220",
            "06700719820"]

class truck:
    def __init__(self, sname):
        self.name = sname
        self.components = {};
        self.items = None

    def report(self):
        print("Truck ", self.name, "Load:")
        for c in self.components:
            print(c, self.components[c])

class component:
    def __init__(self,suid):
        self.uid = suid
        self.resource_horizon = OrderedDict()
        self.acc_resource_horizon = OrderedDict()
        self.container_capacity = 1 #This will be overriden whenever required
    
    def report_resource_horizon(self):
        for d in self.resource_horizon:
            for t in self.resource_horizon[d]:
                print((d,t), self.resource_horizon[d][t])        
    
    def report_acc_horizon(self):
        for d in self.acc_resource_horizon:
            for t in self.acc_resource_horizon[d]:
                print((d,t), self.acc_resource_horizon[d][t])        

    def report(self):
        print("PN: ", self.uid)
        #self.report_resource_horizon()
        self.report_acc_horizon()

    def calc_acc_resource_horizon(self):
        current_cap = 0;
        self.acc_resource_horizon = deepcopy(self.resource_horizon)
        
        for d in self.resource_horizon:
            for t in self.resource_horizon[d]:
                current_cap += self.resource_horizon[d][t]
                self.acc_resource_horizon[d][t] = current_cap

class mixCode:
    def __init__(self, suid):
        self.uid = suid
        self.component_list = []

    def report(self):
        print("Mix Code ",self.uid)
        print(self.component_list)
        

#Iterate in workbook sheets
for sh_id in range(wb.nsheets):
    ws = wb.sheet_by_index(sh_id)
    print(ws.name)

    #Parse worksheet
    cols = []
    #Print Sheet Information
    print("Sheet:", ws.name, \
      "Active Columns", ws.ncols, \
      "Active Rows", ws.nrows)

    if (ws.name == "TruckLoad"):
        for i in range(3, ws.ncols, 3):
            active_row_id = 2
            while (True):
                #On each iteration fetch the compoents of a truck
                #Fetch name and quantity
                truck_name = ws.cell_value(active_row_id, i)
                truck_itemNum = ws.cell_value(active_row_id, i + 1)
                #print(truck_name, truck_itemNum)
                t = truck(truck_name)

                #Store truck to the dictionary
                trucks[truck_name] = t

                #Load components
                active_row_id += 1
                while(True):
                    if (active_row_id == ws.nrows):
                        break
                    comp_name = ws.cell_value(active_row_id, i)
                    comp_itemNum = ws.cell_value(active_row_id, i + 1)
                    if not comp_name.isnumeric():
                        break
                    comp_name = str(comp_name)
                    # print("Loading Compoment",comp_name, comp_itemNum)
                    #Save component to truck list

                    #If its a seat then make sure to add the codes for Seats2,3,4
                    if comp_name in seat_codes:
                        seat2 = comp_name[0:-1] + "2"
                        t.components[seat2] = int(comp_itemNum)
                        seat3 = comp_name[0:-1] + "3"
                        t.components[seat3] = int(comp_itemNum)
                        seat4 = comp_name[0:-1] + "4"
                        t.components[seat4] = int(comp_itemNum)
                    #If its a wheel then make sure to add the codes for Wheels2
                    elif comp_name in wheel_codes:
                        wheel2 = comp_name[0:-1] + "2"
                        t.components[wheel2] = int(comp_itemNum)

                    t.components[comp_name] = int(comp_itemNum)
                    active_row_id += 1

                #Check if we should exit
                if (active_row_id == ws.nrows):
                    break
                if not ws.cell_value(active_row_id,i):
                    break
                

    
    elif (ws.name == "DockSchedule"):
        #skip 2 rows and start parsing the current machine setup
        for i in range(1, ws.nrows):
            try:
                datetime = xlrd.xldate_as_tuple(ws.cell_value(i, 0), wb.datemode)
                print(datetime)
                t1 = None
                t2 = None
                t3 = None
                
                dock1 = ws.cell_value(i, 1)
                if (dock1):
                    try:
                        t1 = trucks[dock1]
                    except:
                        t1 = None
                
                dock2 = ws.cell_value(i, 2)
                if (dock2):
                    try:
                        t2 = trucks[dock2]
                    except:
                        t2 = None

                dock3 = ws.cell_value(i, 3)
                if (dock3):
                    try:
                        t3 = trucks[dock3]
                    except:
                        t3 = None
                
                #Store in dock schedule
                date = datetime[0:3]
                time = datetime[3:]

                if not date in dockSchedule:
                    dockSchedule[date] = OrderedDict()

                dockSchedule[date][time] = [t1, t2, t3]

                #Also save the dock horizon
                if not date in dock_horizon:
                    dock_horizon[date] = OrderedDict()
                dock_horizon[date][time] = 0

            except:
                continue

    elif (ws.name == "PartNumber"):
        for j in range(1, ws.ncols):
            for i in range(2, ws.nrows):
                val = ws.cell_value(i, j)
                if not val:
                    continue
                comp = component(val)
                print("Storing component", val)
                components[str(val).rstrip()] = comp
    
    elif (ws.name == "MixCode"):
        for i in range(2, ws.nrows):
            mix_code_id = int(ws.cell_value(i, 0))
            mc = mixCode(mix_code_id)
            for j in range(1, ws.ncols):
                val = ws.cell_value(i, j)
                if not int(val):
                    continue
                val = str(val).rstrip()
                mc.component_list.append(val)
            mixCodes[mix_code_id] = mc

    elif (ws.name == "ProductionTime"):
        for i in range(1, ws.nrows):
            datetime = xlrd.xldate_as_tuple(ws.cell_value(i, 0), wb.datemode)
            mix_code_id = int(ws.cell_value(i, 1))
            productionSchedule.append((datetime, mix_code_id))
    
    elif (ws.name == "Stock level"):
        #Parse the raw product numbers
        for i in range(11, ws.nrows):
            component_name = str(ws.cell_value(i, 0))
            if not component_name:
                continue
            load = int(ws.cell_value(i, 2))
            initial_stock[component_name] = load
            print("Component", component_name, "Level", load)

    elif (ws.name == "Container_Capacity"):
        #Parse the raw product numbers
        for i in range(2, ws.nrows):
            component_name = str(ws.cell_value(i, 0))
            cont_cap = int(ws.cell_value(i, 1))
            container_capacities[component_name] = cont_cap
            print("Component", component_name, "Container Capacity", cont_cap)


#Report components
#for c in components:
#    print(c)

#Report trucks
#for t in trucks:
#    trucks[t].report()


#PostProcessing
#At first component resource horizons should be updated
for c in components:
    components[c].resource_horizon = deepcopy(dock_horizon)


#Set container capacities on the desired components
for cont_cap in container_capacities:
    if cont_cap in components:
        components[cont_cap].container_capacity =  container_capacities[cont_cap]


#Set container capacities on the initial stock capacities
for c, v in initial_stock.items():
    if c in seat_codes:
        v = 16
        continue
    if c in wheel_codes:
        continue
    if c in dashboard_shells:
        continue
    
    #initial_stock[c] = v * components[c].container_capacity #Very high values :S :S :S



#Process the dockschedule and properly calculate the capacity of the components
for date in dockSchedule:
    d = dockSchedule[date]
    for t in d:
        dt = d[t]
        #Iterate in trucks that reach the docks at that time
        for truck in dt:
            if truck is None:
                continue
            for c in truck.components:
                main_c = components[c]
                #Load capacity
                main_c.resource_horizon[date][t] = truck.components[c]

#Process the 16 first orders and make sure that wheels/seats/dashboard components are set added to the init stock
for i in range(16):
    order = productionSchedule[i]
    #iterate in mc component list
    mix_code = mixCodes[order[1]]
    for c in mix_code.component_list:
        if c in seat_codes:
            initial_stock[c] = 16
        
        if c in wheel_codes:
            initial_stock[c] = 9
        
        if c in dashboard_shells:
            initial_stock[c] = 10


# Report Initial stock
# print(initial_stock)

#Export components
f = open("components.csv","w")
f.write(str(len(components.keys())) + " " + str(16*len(dock_horizon.keys())) + "\n")
for c in components:
    f.write(str(components[c].uid) + " " + str(components[c].container_capacity) + "\n")
    for d in components[c].resource_horizon:
        for t in components[c].resource_horizon[d]:
            f.write(str(components[c].resource_horizon[d][t]) + ",");
            #f.write("10,"); #FORCE INFINITE RESOURCES
    f.write("\n")
f.close()

#Export mix codes
f = open("mix_codes.csv","w")
f.write(str(len(mixCodes.keys())) + "\n")
for c in mixCodes:
    f.write(str(mixCodes[c].uid) + " " + str(len(mixCodes[c].component_list)) + "\n")
    f.write(",".join(list(map(str, mixCodes[c].component_list))))
    f.write("\n")
f.close()


#Export production Schedule
f = open("production_orders.csv","w")
f.write(str(len(productionSchedule)) + "\n")
for c in productionSchedule:
    #Format Date
    date = "{0:02d}/{1:02d}/{2:02d} {3:02d}:{4:02d}:{5:02d}".format(c[0][1], c[0][2], c[0][0], c[0][3], c[0][4], c[0][5])
    f.write(date + "," + str(c[1]) +  "\n")
f.close()


#Export initial stock
f = open("initial_stock.csv","w")
f.write(str(len(initial_stock.keys())) + "\n")
for c,v in initial_stock.items():
    f.write(c + "," + str(v) +  "\n")
f.close()

#Export stuff to a dat file to be used by CP
def export_data():
    f = open("data.csv", "w")
    f.close()



if __name__=='__main__':
    export_data();
    quit()

    print("Select Mode")
    x = input("0: Export Data")
    x = int(x)
    if (x == 0):
        export_data()
    else:
        print("Wrong Mode")

