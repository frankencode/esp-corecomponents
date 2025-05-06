import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from data.cc_set_destruction_sparse_runtime import x as x_blist_sparse
from data.cc_set_destruction_sparse_runtime import y as y_blist_sparse
from data.cc_set_destruction_dense_runtime import x as x_blist_dense
from data.cc_set_destruction_dense_runtime import y as y_blist_dense
from data.std_set_destruction_runtime import x as x_std
from data.std_set_destruction_runtime import y as y_std

fig = plt.figure()
ax = fig.add_axes([0.15, 0.1, 0.8, 0.8])
ax.plot(x_blist_sparse, y_blist_sparse, "C0H-")
ax.plot(x_blist_dense, y_blist_dense, "C2H-")
ax.plot(x_std, y_std, "C1H-")
ax.set_title('Runtime cost of set desctruction (RISC-V)')
ax.set_xlabel('Number of elements')
ax.set_ylabel('Duration [us]')
ax.legend(labels = ('blist_set<int>::deplete() // sparse', 'blist_set<int>::deplete() // dense', 'std::set<int>::clear()'),loc = 'upper left')

plt.savefig("plots/set_destruction_plot.svg")
