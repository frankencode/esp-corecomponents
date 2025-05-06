import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from data.cc_list_insert_randomized_runtime import x as x_blist
from data.cc_list_insert_randomized_runtime import y as y_blist
from data.std_deque_insert_randomized_runtime import x as x_std_deque
from data.std_deque_insert_randomized_runtime import y as y_std_deque
from data.std_vector_insert_randomized_runtime import x as x_std_vector
from data.std_vector_insert_randomized_runtime import y as y_std_vector

def divide_n(x, y):
    for i in range(0, len(x)):
        y[i] /= x[i]

divide_n(x_blist, y_blist)
divide_n(x_std_deque, y_std_deque)
divide_n(x_std_vector, y_std_vector)

fig = plt.figure()
ax = fig.add_axes([0.15, 0.1, 0.8, 0.8])
ax.plot(x_blist, y_blist, "C0H-")
ax.plot(x_std_deque, y_std_deque, "C4H-")
ax.plot(x_std_vector, y_std_vector, "C1H-")
ax.set_title('Runtime cost of randomized list insertions (RISC-V)')
ax.set_xlabel('Number of elements')
ax.set_ylabel('Duration [us]')
ax.legend(labels = ('blist<int>', 'std::deque<int>', 'std::vector<int>'),loc = 'upper left')

plt.savefig("plots/list_insert_randomized_plot.svg")
