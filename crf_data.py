import jsonpickle


#Constraint Types
constraint_type_dict = {
    'Weekly Maximum Capacity': 0,
    'Daily Maximum Capacity': 1,
}

#Constraint Tags
constraint_tag_dict = {
    'Buy': 0,
    'Allocation': 1,
    'Make': 2,
}

#Data structures
class constraint_stat:
    def __init__(self):
        self.max_cap = 0
        self.min_cap = 0
        self.used    = 0
        self.max_sat = 0
        self.min_sat = 0
        self.vol_sat = 0
    
    def toJSON(self):
        return json.dumps(self, default=lambda o: o.__dict__, 
            sort_keys=True, indent=4)


class constraint:
    def __init__(self):
        self.name = ""
        self.tag = ""
        self.tag_id = -1
        self.type = ""
        self.type_id = -1
        self.timeperiods = {}
        return

    def toJSON(self):
        return json.dumps(self, default=lambda o: o.__dict__, 
            sort_keys=True, indent=4)


constraints = {}

dirfolder = ""
f = open(dirfolder + "constraints.csv")
lines = f.readlines()

weeks = [i for i in lines[1].rstrip().split(",") if i]
weeknum = len(weeks)

#Parse Constraints
for i in range(3, len(lines)):
    l = lines[i].rstrip()
    l_split = l.split(",")
    #New Constraint
    c = constraint();
    c.name = l_split[0]
    c.tag = l_split[1]
    c.type = l_split[3]




    print c.name, c.tag, c.type
    index = 4
    #Parse period information
    j = 0;
    while (j < weeknum):
        stat = constraint_stat()
        stat.max_cap = 0 if not l_split[index + 0] else int(l_split[index + 0])
        stat.min_cap = 0 if not l_split[index + 1] else int(l_split[index + 1])
        stat.used    = 0 if not l_split[index + 2] else int(l_split[index + 2])
        stat.max_sat = 0 if not l_split[index + 3] else int(l_split[index + 3])
        stat.min_sat = 0 if not l_split[index + 4] else int(l_split[index + 4])
        stat.vol_sat = 0 if not l_split[index + 5] else float(l_split[index + 5].rstrip("%")) / 100.0
        c.timeperiods[weeks[j]] = stat
        index += 6
        j += 1

    #Add Constraint
    constraints[c.name] = c

f.close()

outfile = "constraints.json"
f = open(outfile, "w")
json_dmp = jsonpickle.encode(constraints)
f.write(json_dmp)
f.close()
