import numpy as np
import matplotlib.pyplot as plt

blist_overhead = 8
std_list_overhead = 12
std_forward_list_overhead = 4
std_deque_overhead = 596

data = {
    'blist<int> ({})'.format(blist_overhead): blist_overhead,
    'std::list<int> ({})'.format(std_list_overhead): std_list_overhead,
    'std::forward_list<int> ({})'.format(std_forward_list_overhead): std_forward_list_overhead,
    'std::deque<int> ({})'.format(std_deque_overhead): std_deque_overhead,
}
list_likes = list(data.keys())
performance = list(data.values())
fig = plt.figure(figsize = (12, 5))
plt.bar(list_likes, performance, color = ['C0', 'C2', 'C1', 'C4'], width = 0.4)
plt.ylabel("Overhead [bytes]")
plt.title("Memory overhead of empty container (RISC-V)")
plt.savefig("plots/zero_size_overheads_plot.svg")
