import json
from collections import OrderedDict

#dirfolder = "/home/greg/Dropbox/PhD/DISRUPT STUFF/crf_sample_data/"
dirfolder = ""

#Data Structures
class production_order:
	def __init__(self):
		self.ID = -1
		self.frozen = -1
		self.drive = -1
		self.versionCode = -1
		self.marketCode = -1
		self.internalColor = -1
		self.externalColor = -1
		self.productionDate = ''
		self.constraint_string = ''
		self.constraints_filtered = []
	
	def report(self):
		print("ID", self.ID, "Date",self.productionDate,"Constraints", self.constraint_string)



#Try to load the constraints
f = open("constraints.json",'r')
s = f.read()
f.close()
constraints = json.loads(s)



#Parse Production schedule
f = open(dirfolder + "plant_schedule.csv",'r')
lines = f.readlines()
f.close()

orders = []

for i in range(1, len(lines)):
	l = lines[i]
	l_split = l.rstrip().split(",")

	order = production_order()
	order.ID = i+1
	order.frozen = int(l_split[0])
	order.drive = int(l_split[1])
	order.versionCode = int(l_split[2])
	order.marketCode = int(l_split[3])
	order.internalColor = int(l_split[4])
	order.externalColor = int(l_split[5])
	order.productionDate = l_split[6]
	order.constraint_string = l_split[7]

	orders.append(order)


orders[0].report()


#Try to assemble all the constraint codes
constraint_codes = {}

for o in orders:
	split = o.constraint_string.split(';')
	for s in split:
		if not s in constraint_codes:
			constraint_codes[s] = 0
		else:
			constraint_codes[s] += 1


for c in constraint_codes:
	print c, constraint_codes[c]


#Group production orders by date
dates = OrderedDict()

for o in orders:
	if o.productionDate not in dates:
		dates[o.productionDate] = [o]
	else:
		dates[o.productionDate].append(o)

#Report
for d in dates:
	print d, len(dates[d])



#Gathering data for constraint OPT-XAH on the 3rd week of Jan 15/Jan/18 - 19/Jan/18
count = 0
for i in range(1, 3):
	d = dates['{0:02d}'.format(i)+'/Feb/18 00:00:00']
	print str(i)+'/Feb/18 00:00:00'
	for o in d:
		if 'OPT-156' in o.constraint_string:
			print 'Order ', o.ID
			count += 1

print count

