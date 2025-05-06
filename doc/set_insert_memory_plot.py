import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from data.cc_set_insert_randomized_memory import x as x_blist_rnd
from data.cc_set_insert_randomized_memory import y as y_blist_rnd
from data.cc_set_insert_ascending_memory import x as x_blist_asc
from data.cc_set_insert_ascending_memory import y as y_blist_asc
from data.std_set_insert_randomized_memory import x as x_std
from data.std_set_insert_randomized_memory import y as y_std

fig = plt.figure()
ax = fig.add_axes([0.15, 0.1, 0.8, 0.8])
ax.plot(x_blist_rnd, y_blist_rnd, "C0H-")
ax.plot(x_blist_asc, y_blist_asc, "C2H-")
ax.plot(x_std, y_std, "C1H-")
ax.set_title('Memory cost of set insertions (RISC-V)')
ax.set_xlabel('Number of elements')
ax.set_ylabel('Heap usage [bytes]')
ax.legend(labels = ('blist_set<int>::insert() // randomized', 'blist_set<int>::insert() // ascending', 'std::set<int>::insert()'), loc = 'upper left')

plt.savefig("plots/set_insert_memory_plot.svg")
