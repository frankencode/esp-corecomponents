import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from data.cc_set_lookup_sparse_randomized_runtime import x as x_blist_sparse
from data.cc_set_lookup_sparse_randomized_runtime import y as y_blist_sparse
from data.cc_set_lookup_dense_randomized_runtime import x as x_blist_dense
from data.cc_set_lookup_dense_randomized_runtime import y as y_blist_dense
from data.std_set_lookup_randomized_runtime import x as x_std
from data.std_set_lookup_randomized_runtime import y as y_std

def divide_n(x, y):
    for i in range(0, len(x)):
        y[i] /= x[i]

divide_n(x_blist_sparse, y_blist_sparse)
divide_n(x_blist_dense, y_blist_dense)
divide_n(x_std, y_std)

fig = plt.figure()
ax = fig.add_axes([0.15, 0.1, 0.8, 0.8])
ax.plot(x_blist_sparse, y_blist_sparse, "C0H-")
ax.plot(x_blist_dense, y_blist_dense, "C2H-")
ax.plot(x_std, y_std, "C1H-")
ax.set_title('Runtime cost of randomized set lookups (RISC-V)')
ax.set_xlabel('Number of elements')
ax.set_ylabel('Duration [us]')
ax.legend(labels = ('blist_set<int>::contains() // sparse', 'blist_set<int>::contains() // dense', 'std::set<int>::contains()'), loc = 'center right')

plt.savefig("plots/set_lookup_randomized_plot.svg")
