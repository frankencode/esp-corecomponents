import numpy as np
import matplotlib.pyplot as plt

from data.cc_set_insert_randomized_memory import x as x_blist_rnd
from data.cc_set_insert_randomized_memory import y as y_blist_rnd
from data.cc_set_insert_ascending_memory import x as x_blist_asc
from data.cc_set_insert_ascending_memory import y as y_blist_asc
from data.std_set_insert_randomized_memory import x as x_std
from data.std_set_insert_randomized_memory import y as y_std
from data.std_unordered_set_insert_randomized_memory import x as x_unordered_set_std
from data.std_unordered_set_insert_randomized_memory import y as y_unordered_set_std

def compute_overhead(x, y):
    j = len(y) - 1
    return (y[j] / x[j]) - 4

blist_rnd_overhead = compute_overhead(x_blist_rnd, y_blist_rnd)
blist_asc_overhead = compute_overhead(x_blist_asc, y_blist_asc)
rb_set_overhead = compute_overhead(x_std, y_std)
unordered_set_overhead = compute_overhead(x_unordered_set_std, y_unordered_set_std)

data = {
    'blist_set<int> // sparse ({:.2f})'.format(blist_rnd_overhead): blist_rnd_overhead,
    'blist_set<int> // dense ({:.2f})'.format(blist_asc_overhead): blist_asc_overhead,
    'std::set<int> ({:.2f})'.format(rb_set_overhead): rb_set_overhead,
    'std::unordered_set<int> ({:.2f})'.format(unordered_set_overhead): unordered_set_overhead,
}
list_likes = list(data.keys())
performance = list(data.values())
fig = plt.figure(figsize = (12, 5))
plt.bar(list_likes, performance, color = ['C0', 'C2', 'C1', 'C4'], width = 0.4)
# plt.xlabel("Append operation performance")
plt.ylabel("Overhead per element [bytes]")
plt.title("Memory overhead per item at set size 10000 (RISC-V)")
plt.savefig("plots/set_insert_memory_overhead_plot.svg")
