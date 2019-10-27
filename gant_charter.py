#!/usr/bin/python2
import matplotlib
#Uncomment this line on the server version
#matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.pyplot import cm 
import matplotlib.patches as patches
import sys 


def main(args):
    if (len(args) < 1):
        raise Exception("Missing Input File")

    #Setup Plot
    fig, ax = plt.subplots(figsize=(30, 15))

    # parse the input file now
    f=open(args[0], "r")
    lines = f.readlines()
    f.close()

    obj = float(lines[0])
    x = []
    x_ticks =[]
    y = []
    for i in range(len(lines[1:-1])):
        l = lines[1:][i]
        split = l.split(" ")
        x.append(i)
        x_ticks.append(split[0])
        y.append(int(split[1])*60/float(split[2]))
    
    ax.bar(x, y, width=0.5, tick_label=x_ticks)
    ax.set_xlabel('Production Days')
    ax.set_ylabel('Daily JPH')

    # for i in range(machineNum):
    # 	l = f.readline().rstrip().split(" ")
    # 	print l
    #     mach_ID, mach_Op_Num, mach_line_name = l
    #     mach_ID = int(mach_ID)
    #     mach_Op_Num = int(mach_Op_Num)
    #     if not mach_Op_Num:
    #         continue
    #     y = counter * 10
    #     mach_labs_pos.append(y + 5)
    #     mach_labs.append("Line " + mach_line_name + " Machine" + str(i))

    #     #ax.text(-30.0, y+5, "Machine " + str(i), fontdict=None, withdash=False)

    #     for j in range(mach_Op_Num):
    #         op_uid, order_id, op_job, op_id, op_start, op_end = map(int, f.readline().split(" "))
    #         op_start = op_start/600.0 #Convert to mins, start value is in decisecs
    #         op_end = op_end/600.0 #Convert to mins
    #         makespan = max(makespan, op_end)
    #         ax.add_patch(patches.Rectangle(
    #             (op_start, y),   # (x,y)
    #              op_end - op_start,          # width
    #              10.0,          # height
    #              fill=True,
    #              alpha=0.6,
    #              linestyle='--',
    #              edgecolor="black",
    #              facecolor=order_colors[order_id]
    #              #hatch='+'
    #             ))
    #         tex_cx = op_start + (op_end - op_start)/2.0
    #         tex_cy = y + 5.0
    #         #tex = "Job: " + str(op_job) + " ID: " + str(op_id)
    #         tex = "Order#" + str(order_id) + "_#" + str(op_job)
    #         ax.annotate(tex, (tex_cx , tex_cy), color = 'b', weight='bold',
    #                         fontsize = 10, ha='center', va='center')
    #     counter += 1

    #ax.set_yticks(mach_labs_pos)
    #ax.set_yticklabels(mach_labs)
    #ax.set_xlim([0, makespan])
    #ax.set_ylim([0, counter*10])
    ax.set_title('SCHEDULE REPORT')
    fig.savefig('gant.png', dpi=300, bbox_inches='tight')
    #plt.show()

if (__name__ =="__main__" ):
    #main(sys.argv)
    main([r"output.txt"])
    #main([r"Scheduling/output.out"])
