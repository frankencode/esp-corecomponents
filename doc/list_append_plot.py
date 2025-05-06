import numpy as np
import matplotlib.pyplot as plt
data = {
    'blist<int>::append()': 1/8938e-9,
    'std::list<int>::push_back()': 1/43965e-9,
    'std::forward_list<int>::insert_after()': 1/42727e-9
}
list_likes = list(data.keys())
performance = list(data.values())
fig = plt.figure(figsize = (12, 5))
plt.bar(list_likes, performance, color = ['C0', 'C1', 'C4'], width = 0.4)
# plt.xlabel("Append operation performance")
plt.ylabel("Operations per second [Hz]")
plt.title("Append performance of list-like data structures (RISC-V)")
plt.savefig("plots/list_append_plot.svg")
