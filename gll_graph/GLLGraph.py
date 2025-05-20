import matplotlib.pyplot as plt
import matplotlib.ticker as tick
import numpy as np
import math
import sys

def forward2(x):
    return np.log2(x, where=x>0)

def inverse2(x):
    return 2**x

def forward10(x):
    return np.log10(x, where=x>0)

def inverse10(x):
    return 10**x


def create_graph():
    #get data
    name, input, time, sizeR, sizeU, sizeP, numNodes, numEdges, sizeGSS, maxInput, maxTime, maxSize, maxGSS = get_data(sys.argv[1]) 

    #create plot
    fig, host = plt.subplots(figsize=(8,5), layout='constrained')
    
    #create additional y-axes
    ax2 = host.twinx()
    ax3 = host.twinx()
    
    #set limits of axes
    host.set_xlim(0, maxInput)
    host.set_ylim(0, math.ceil(maxTime / 100.0) * 100)
    ax2.set_ylim(1, math.ceil(maxSize / 100.0) * 100)
    ax3.set_ylim(1, math.ceil(maxGSS / 100.0) * 100)

    #set labels of axes
    host.set_xlabel("Input Size", fontsize="8")
    host.set_ylabel("Time (ms)")
    ax2.set_ylabel("Size (kB)")
    ax3.set_ylabel("Number of GSS Edges and Nodes")

    #set colors of axes
    colors = ['#FF0000', '#000080', '#007791', '#1E90FF', '#6F00FF', '#006400', '#00FF00']
    
    #plot axes
    p1 = host.plot(input, time, color=colors[0], label="Time", zorder=10)
    p2 = ax2.plot(input, sizeR, color=colors[1], label="Set R Size", zorder=10)
    p3 = ax2.plot(input, sizeU, color=colors[2], label="Set U Size", zorder=10)
    p4 = ax2.plot(input, sizeP, color=colors[3], label="Set P Size", zorder=10)
    p5 = ax2.plot(input, sizeGSS, color=colors[4], label="GSS Size", zorder=10)
    p6 = ax3.plot(input, numNodes, color=colors[5], label="GSS Nodes", zorder=10)
    p7 = ax3.plot(input, numEdges, color=colors[6], label="GSS Edges", zorder=10)
    
    #scale axis 2 and 3 with given function
    ax2.set_yscale('function', functions=(forward2, inverse2))
    ax3.set_yscale('function', functions=(forward10, inverse10))

    #set ticks of axis 2 and 3
    ax2.yaxis.set_minor_formatter(tick.NullFormatter())
    ax2.yaxis.set_major_locator(tick.FixedLocator(np.append(np.array([2**x for x in range(0, (int(np.log2(math.ceil(maxSize / 100.0) * 100)) + 1))]), math.ceil(maxSize / 100.0) * 100)))
    ax3.yaxis.set_minor_formatter(tick.NullFormatter())
    ax3.yaxis.set_major_locator(tick.FixedLocator(np.append(np.array([10**x for x in range(0, (int(np.log2(math.ceil(maxGSS / 100.0) * 100)) + 1))]), math.ceil(maxGSS / 100.0) * 100)))

    #move axis 3 to the side
    ax3.spines['right'].set_position(('outward', 60))

    #customize axes
    host.yaxis.label.set_color(colors[0])
    host.tick_params(axis='y', colors=colors[0])
    ax2.yaxis.label.set_color(colors[1])
    ax2.tick_params(axis='y', colors=colors[1])
    ax3.yaxis.label.set_color(colors[5])
    ax3.tick_params(axis='y', colors=colors[5])
    ax2.grid(axis='y', linestyle='dotted', linewidth=1, zorder=0)
    ax3.grid(axis='y', linestyle='dashed', linewidth=1, zorder=0)

    #set graph settings and save pdf
    host.grid(color='black', linestyle = '--', linewidth = 0.5, zorder=0)
    host.legend(loc='upper center', bbox_to_anchor=(0.5, -0.10), fancybox=True, shadow=True, ncol=7, fontsize="8", handles=p1+p2+p3+p4+p5+p6+p7)
    plt.title("GLL Analysis for input " + name, fontsize="10")
    plt.savefig("GLL" + name + ".png", bbox_inches='tight')

def get_data(filename):
    #read data from file and collect information in arrays and upper limits as integers
    input, time, sizeR, sizeU, sizeP, numNodes, numEdges, sizeGSS = ([] for i in range(8))
    maxInput, maxTime, maxSize, maxGSS = 0, 0, 0, 0
    with open(filename, 'r') as file:
        content = file.readlines()
        table = [ [ cell.strip() for cell in line.split(':') ] for line in content ]
        name = table[0][1]
        for i in range(2, len(table) - 1):
            x = float(table[i][1]) #get input size
            if x > maxInput:
                maxInput = x
            input.append(x)
            x = float(table[i][5].split()[0]) #get CPU time
            if x > maxTime:
                maxTime = x
            time.append(x)
            x = float(table[i][6].split()[0]) #get R alloc size
            if x > maxSize:
                maxSize = x
            sizeR.append(x)
            x = float(table[i][7].split()[0]) #get U alloc size
            if x > maxSize:
                maxSize = x
            sizeU.append(x)
            x = float(table[i][8].split()[0]) #get P alloc size
            if x > maxSize:
                maxSize = x
            sizeP.append(x)
            x = float(table[i][9]) #get number of gss nodes
            if x > maxGSS:
                maxGSS = x
            numNodes.append(x)
            x = float(table[i][10]) #get number of gss edges
            if x > maxGSS:
                maxGSS = x
            numEdges.append(x)
            x = float(table[i][11].split()[0]) #get number of gss edges
            if x > maxSize:
                maxSize = x
            sizeGSS.append(x)

    return name, input, time, sizeR, sizeU, sizeP, numNodes, numEdges, sizeGSS, maxInput, maxTime, maxSize, maxGSS

def main():
    if len(sys.argv) != 2:
        print("Please give a file")
    else:
        create_graph()

if __name__ == "__main__":
    main()
