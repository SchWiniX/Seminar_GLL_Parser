import matplotlib.pyplot as plt
import matplotlib.ticker as tick
import numpy as np
import math
import sys

class gll_info:
    def __init__(self, name):
        self.name = name
        #1: input size, 4: time, 6: R size, 7: U total, 8: P total
        #9: GSS nodes total, 10: GSS edges total, 11: GSS size
        self.values = [1, 4, 6, 7, 8, 9, 10, 11]
        self.info = {}
        self.maximum = {}
        for val in self.values:
            self.info[val] = []
            self.maximum[val] = 0


    def append(self, line):
        for pos in self.values:
            value = float(line[pos].split()[0])
            self.info[pos].append(value)
            if value > self.maximum[pos]:
                self.maximum[pos] = value


    def get_max(self, l):
        max = 0
        for pos in l:
            if self.maximum[pos] > max:
                max = self.maximum[pos]
        return max


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
    data = get_data(sys.argv[1])

    for i in data.values:
        print(str(i) + ": " + str(data.maximum[i]))

    #create plot
    fig, host = plt.subplots(figsize=(8,5), layout='constrained')
    
    #create additional y-axes
    ax2 = host.twinx()
    ax3 = host.twinx()
    
    #set limits of axes
    host.set_xlim(0, data.maximum[1])
    host.set_ylim(0, math.ceil(data.maximum[4] / 100.0) * 100)
    ax2.set_ylim(1, math.ceil(data.get_max([6, 11]) / 100.0) * 100)
    ax3.set_ylim(1, math.ceil(data.get_max([7, 8, 9, 10]) / 100.0) * 100)

    #set labels of axes
    host.set_xlabel("Input Size", fontsize="8")
    host.set_ylabel("Time (ms)")
    ax2.set_ylabel("Size (kB)")
    ax3.set_ylabel("Total")

    #set colors of axes
    colors = ['#FF0000', '#006400', '#00FF00', '#000080', '#007791', '#1E90FF', '#6F00FF']
    
    #plot axes
    p1 = host.plot(data.info[1], data.info[4], color=colors[0], label="Time", zorder=10)
    p2 = ax2.plot(data.info[1], data.info[6], color=colors[1], label="Set R Size", zorder=10)
    p3 = ax2.plot(data.info[1], data.info[11], color=colors[2], label="GSS Size", zorder=10)
    p4 = ax3.plot(data.info[1], data.info[7], color=colors[3], label="Total U Set", zorder=10)
    p5 = ax3.plot(data.info[1], data.info[8], color=colors[4], label="Total P Set", zorder=10)
    p6 = ax3.plot(data.info[1], data.info[9], color=colors[5], label="GSS Nodes", zorder=10)
    p7 = ax3.plot(data.info[1], data.info[10], color=colors[6], label="GSS Edges", zorder=10)
    
    #scale axis 2 and 3 with given function
    ax2.set_yscale('function', functions=(forward2, inverse2))
    ax3.set_yscale('function', functions=(forward10, inverse10))

    #set ticks of axis 2 and 3
    ax2.yaxis.set_minor_formatter(tick.NullFormatter())
    ax2.yaxis.set_major_locator(tick.FixedLocator(np.append(np.array([2**x for x in range(0, (int(np.log2(math.ceil(data.get_max([6, 11]) / 100.0) * 100)) + 1))]), math.ceil(data.get_max([6, 11]) / 100.0) * 100)))
    ax3.yaxis.set_minor_formatter(tick.NullFormatter())
    ax3.yaxis.set_major_locator(tick.FixedLocator(np.append(np.array([10**x for x in range(0, (int(np.log2(math.ceil(data.get_max([7, 8, 9, 10]) / 100.0) * 100)) + 1))]), math.ceil(data.get_max([7, 8, 9, 10]) / 100.0) * 100)))

    #move axis 3 to the side
    ax3.spines['right'].set_position(('outward', 60))

    #customize axes
    host.yaxis.label.set_color(colors[0])
    host.tick_params(axis='y', colors=colors[0])
    ax2.yaxis.label.set_color(colors[1])
    ax2.tick_params(axis='y', colors=colors[1])
    ax3.yaxis.label.set_color(colors[3])
    ax3.tick_params(axis='y', colors=colors[3])
    ax2.grid(axis='y', linestyle='dotted', linewidth=1, zorder=0)
    ax3.grid(axis='y', linestyle='dashed', linewidth=1, zorder=0)

    #set graph settings and save pdf
    host.grid(color='black', linestyle = '--', linewidth = 0.5, zorder=0)
    host.legend(loc='upper center', bbox_to_anchor=(0.5, -0.10), fancybox=True, shadow=True, ncol=7, fontsize="8", handles=p1+p2+p3+p4+p5+p6+p7)
    plt.title("GLL Analysis for input " + data.name, fontsize="10")
    plt.savefig("GLL" + data.name + ".png", bbox_inches='tight')


def get_data(filename):
    #read data from file and collect information in arrays and upper limits as integers
    with open(filename, 'r') as file:
        content = file.readlines()
        table = [[cell.strip() for cell in line.split(':')] for line in content]
        name = table[0][1]
        data = gll_info(name)
        for i in range(2, len(table) - 1):
            data.append(table[i])
    return data


def main():
    if len(sys.argv) != 2:
        print("Please give a file")
    else:
        create_graph()

if __name__ == "__main__":
    main()
