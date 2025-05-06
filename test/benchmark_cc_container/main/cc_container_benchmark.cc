#include <unistd.h>
#include <inttypes.h>
#include <cc/List>
#include <cc/Map>
#include <cc/MultiMap>
#include <cc/Set>
#include <cc/Array>
#include <cc/Random>
#include <cc/stdio>
#include <deque>
#include <list>
#include <forward_list>
#include <map>
#include <set>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h> // esp_timer_get_time()
#include <esp_chip_info.h>
#include <esp_idf_version.h>

#include <unity.h>
#include <unity_test_utils.h>

using cc::print;
using cc::Format;

/** Repeatly call the same benchmarking function (compensate for system noise)
  * \param run benchmarking function
  * \param init initialization function (optional)
  * \return smallest execution time observed
  */
int64_t benchmark(std::function<void()> &&run, std::function<void()> &&init = std::function<void()>{})
{
    int64_t dt_min = 0;
    for (int i = 0; i < 3; ++i) {
        if (init) init();
        int64_t dt = esp_timer_get_time();
        run();
        dt = esp_timer_get_time() - dt;
        if (dt < dt_min || dt_min <= 0) dt_min = dt;
    }
    return dt_min;
}

/** Get the free heap in number of bytes
  */
size_t getFreeHeap()
{
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
    return info.total_free_bytes;
}

/** Generate a sequence of non-repeating pseudo-random numbers
  */
std::vector<int> generateRandomInts(size_t n)
{
    std::vector<int> v;
    v.resize(n);
    cc::Random random { 0 };
    for (int &x: v) x = random();
    return v;
}

/** Print \a data as Python list
  */
void printArray(const char *label, const auto &data)
{
    Format f;
    for (auto x: data) {
        f << x << ", ";
    }
    if (f.count() > 0) f.popBack();
    print("%% = [ %% ]\n", label, f.join());
}

TEST_CASE("cc_blist_tree_overhead", "[cc]")
{
    print("sizeof(cc::blist::Tree<16>::Node) = %%\n", sizeof(cc::blist::Tree<16>::Node));
    print("sizeof(cc::blist::Vector<char, 16>::Leaf) - 16 = %%\n", sizeof(cc::blist::Vector<char, 16>::Leaf) - 16);
    print("sizeof(cc::blist::Tree<16>::Branch) = %%\n", sizeof(cc::blist::Tree<16>::Branch));

    print("sizeof(cc::blist::Vector<long>) = %%\n", sizeof(cc::blist::Vector<long>));
    print("sizeof(std::list<int>) = %%\n", sizeof(std::list<int>));
    print("sizeof(std::forward_list<int>) = %%\n", sizeof(std::forward_list<int>));

    #ifndef CONFIG_CORECOMPONENTS_CONTAINER_ASSERTS
    TEST_ASSERT(sizeof(cc::blist::Vector<long>) <= sizeof(void*) * 2);
    #else
    TEST_ASSERT(sizeof(cc::blist::Vector<long>) <= sizeof(void*) * 3);
    #endif

    {
        size_t h = getFreeHeap();
        std::deque<int> d;
        h -= getFreeHeap();
        print("heap_sizeof(std::deque<int>{}) = %%\n", h);
        print("sizeof(std::deque<int>) = %%\n", sizeof(std::deque<int>));
    }
    {
        size_t h = getFreeHeap();
        std::vector<int> v;
        h -= getFreeHeap();
        print("heap_sizeof(std::vector<int>{}) = %%\n", h);
        print("sizeof(std::vector<int>) = %%\n", sizeof(std::vector<int>));
    }
}

TEST_CASE("cc_list_append_runtime", "[cc]")
{
    const int n = 10000;

    cc::List<int> list;

    auto dt = benchmark(
        [&]{
            for (int i = 0; i < n; ++i) list << i;
        },
        [&]{
            list.deplete();
        }
    );

    print("%% appends to cc::List<int> took %%us\n", n, dt);
}

TEST_CASE("std_deque_append_runtime", "[std]")
{
    const int n = 10000;

    std::deque<int> list;

    auto dt = benchmark(
        [&]{
            for (int i = 0; i < n; ++i) list.push_back(i);
        },
        [&]{
            list.clear();
        }
    );

    print("%% appends to std::deque<int> took %%us\n", n, dt);
}

TEST_CASE("std_vector_append_runtime", "[std]")
{
    const int n = 10000;

    std::vector<int> list;

    auto dt = benchmark(
        [&]{
            for (int i = 0; i < n; ++i) list.push_back(i);
        },
        [&]{
            list.clear();
        }
    );

    print("%% appends to std::vector<int> took %%us\n", n, dt);
}

TEST_CASE("std_list_append_runtime", "[std]")
{
    const int n = 10000;

    std::list<int> list;

    auto dt = benchmark(
        [&]{
            for (int i = 0; i < n; ++i) list.push_back(i);
        },
        [&]{
            list.clear();
        }
    );

    print("%% appends to std::list<int> took %%us\n", n, dt);
}

TEST_CASE("std_forward_list_append_runtime", "[std]")
{
    const int n = 10000;

    std::forward_list<int> list;

    auto dt = benchmark(
        [&]{
            std::forward_list<int>::const_iterator previous;
            for (int i = 0; i < n; ++i) {
                if (i == 0) {
                    list.push_front(i);
                    previous = list.begin();
                }
                else {
                    previous = list.insert_after(previous, i);
                }
            }
        },
        [&]{
            list.clear();
        }
    );

    print("%% appends to std::forward_list<int> took %%us\n", n, dt);
}

TEST_CASE("cc_list_insert_randomized_runtime", "[cc]")
{
    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v = generateRandomInts(counts[counts.size() - 1]);

    for (int n: counts)
    {
        cc::List<int> list;

        int64_t dt = benchmark(
            [&]{
                for (int i = 0; i < n; ++i) {
                    int j = v[i];
                    if (j < 0) j = 0;
                    if (j > list.count()) j = list.count();
                    list.insertAt(j, j);
                }
                TEST_ASSERT(list.size() == n);
            },
            [&]{
                list.deplete();
            }
        );

        print("%%\trandom insertions into cc::List<int> cost \t%%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

TEST_CASE("std_vector_insert_randomized_runtime", "[std]")
{
    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v = generateRandomInts(counts[counts.size() - 1]);

    for (int n: counts)
    {
        std::vector<int> list;

        int64_t dt = benchmark(
            [&]{
                for (int i = 0; i < n; ++i) {
                    int j = v[i];
                    if (j < 0) j = 0;
                    if (j > list.size()) j = list.size();
                    list.insert(list.begin() + j, j);
                }
                TEST_ASSERT(list.size() == n);
            },
            [&]{
                list.clear();
            }
        );

        print("%%\trandom insertions into std::vector<int> cost \t%%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

TEST_CASE("std_deque_insert_randomized_runtime", "[std]")
{
    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v = generateRandomInts(counts[counts.size() - 1]);

    for (int n: counts)
    {
        std::deque<int> list;

        int64_t dt = benchmark(
            [&]{
                for (int i = 0; i < n; ++i) {
                    int j = v[i];
                    if (j < 0) j = 0;
                    if (j > list.size()) j = list.size();
                    list.insert(list.begin() + j, j);
                }
                TEST_ASSERT(list.size() == n);
            },
            [&]{
                list.clear();
            }
        );

        print("%%\trandom insertions into std::deque<int> cost \t%%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

TEST_CASE("cc_list_iteration_runtime", "[cc]")
{
    const int n = 10000;

    cc::List<int> list;
    for (int i = 0; i < n; ++i) {
        list.append(i);
    }

    long sum = 0;

    int64_t dt = benchmark(
        [&]{
            for (int i = 0; i < 10; ++i) {
                for (const int &x: list) {
                    sum += x;
                }
            }
        }
    );

    print("sum = %%\n", sum);
    print("%%\tsteps in a cc::List<int> cost \t%%us\n", n, dt);
}

TEST_CASE("std_list_iteration_runtime", "[cc]")
{
    const int n = 10000;

    std::list<int> list;
    for (int i = 0; i < n; ++i) {
        list.push_back(i);
    }

    long sum = 0;

    int64_t dt = benchmark(
        [&]{
            for (int i = 0; i < 10; ++i) {
                for (const int &x: list) {
                    sum += x;
                }
            }
        }
    );

    print("sum = %%\n", sum);
    print("%%\tsteps in a std::list<int> cost \t%%us\n", n, dt);
}

TEST_CASE("std_deque_iteration_runtime", "[cc]")
{
    const int n = 10000;

    std::deque<int> list;
    for (int i = 0; i < n; ++i) {
        list.push_back(i);
    }

    long sum = 0;

    int64_t dt = benchmark(
        [&]{
            for (int i = 0; i < 10; ++i) {
                for (const int &x: list) {
                    sum += x;
                }
            }
        }
    );

    print("sum = %%\n", sum);
    print("%%\tsteps in a std::deque<int> cost \t%%us\n", n, dt);
}

TEST_CASE("cc_set_insert_randomized_runtime", "[cc]")
{
    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v = generateRandomInts(counts[counts.size() - 1]);

    for (int n: counts)
    {
        cc::Set<int> set;

        int64_t dt = benchmark(
            [&]{
                for (int i = 0; i < n; ++i) {
                    set.insert(v[i]);
                }
            },
            [&]{
                set.deplete();
            }
        );

        print("%%\trandom insertions into cc::Set<int> cost \t%%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

TEST_CASE("cc_set_insert_ascending_runtime", "[cc]")
{
    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v = generateRandomInts(counts[counts.size() - 1]);
    std::sort(v.begin(), v.end());

    for (int n: counts)
    {
        cc::Set<int> set;

        int64_t dt = benchmark(
            [&]{
                for (int i = 0; i < n; ++i) {
                    set.insert(v[i]);
                }
            },
            [&]{
                TEST_ASSERT(set.isDense());
                set.deplete();
            }
        );

        print("%%\tascending insertions into cc::Set<int> cost \t%%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

TEST_CASE("std_set_insert_randomized_runtime", "[std]")
{
    using namespace cc;

    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v;
    {
        v.resize(counts[counts.size() - 1]);
        Random random { 0 };
        for (int &x: v) x = random();
    }

    for (int n: counts)
    {
        std::set<int> set;

        int64_t dt = benchmark(
            [&]{
                for (int i = 0; i < n; ++i) {
                    set.insert(v[i]);
                }
            },
            [&]{
                set.clear();
            }
        );

        print("%%\trandom insertions into std::set<int> took dt\t= %%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

TEST_CASE("cc_set_insert_randomized_memory", "[cc]")
{
    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<size_t> heapSizes;
    heapSizes.reserve(counts.size());

    std::vector<int> v = generateRandomInts(counts[counts.size() - 1]);

    size_t initialFreeHeap = getFreeHeap();

    cc::Set<int> set;

    for (int i = 0, j = 0; i < counts[counts.size() - 1];) {
        set.insert(v[i]);
        if (++i == counts[j]) {
            ++j;
            size_t u = initialFreeHeap - getFreeHeap();
            heapSizes.push_back(u);
            print("%%\trandom insertions into cc::Set<int> cost \t%% bytes\n", i, u);
        }
    }

    printArray("x", counts);
    printArray("y", heapSizes);
}

TEST_CASE("cc_set_insert_ascending_memory", "[cc]")
{
    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<size_t> heapSizes;
    heapSizes.reserve(counts.size());

    std::vector<int> v = generateRandomInts(counts[counts.size() - 1]);
    std::sort(v.begin(), v.end());

    size_t initialFreeHeap = getFreeHeap();

    cc::Set<int> set;

    for (int i = 0, j = 0; i < counts[counts.size() - 1];) {
        set.insert(v[i]);
        if (++i == counts[j]) {
            ++j;
            size_t u = initialFreeHeap - getFreeHeap();
            heapSizes.push_back(u);
            print("%%\tascending insertions into cc::Set<int> cost \t%% bytes\n", i, u);
        }
    }

    printArray("x", counts);
    printArray("y", heapSizes);
}

TEST_CASE("std_set_insert_randomized_memory", "[std]")
{
    using namespace cc;

    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<size_t> heapSizes;
    heapSizes.reserve(counts.size());

    std::vector<int> v;
    {
        v.resize(counts[counts.size() - 1]);
        Random random { 0 };
        for (int &x: v) x = random();
    }

    size_t initialFreeHeap = getFreeHeap();

    std::set<int> set;

    for (int i = 0, j = 0; i < counts[counts.size() - 1];) {
        set.insert(v[i]);
        if (++i == counts[j]) {
            ++j;
            size_t u = initialFreeHeap - getFreeHeap();
            heapSizes.push_back(u);
            print("%%\trandom insertions into std::set<int> cost \t%% bytes\n", i, u);
        }
    }

    printArray("x", counts);
    printArray("y", heapSizes);
}

TEST_CASE("std_unordered_set_insert_randomized_memory", "[std]")
{
    using namespace cc;

    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<size_t> heapSizes;
    heapSizes.reserve(counts.size());

    std::vector<int> v;
    {
        v.resize(counts[counts.size() - 1]);
        Random random { 0 };
        for (int &x: v) x = random();
    }

    size_t initialFreeHeap = getFreeHeap();

    std::unordered_set<int> set;

    for (int i = 0, j = 0; i < counts[counts.size() - 1];) {
        set.insert(v[i]);
        if (++i == counts[j]) {
            ++j;
            size_t u = initialFreeHeap - getFreeHeap();
            heapSizes.push_back(u);
            print("%%\trandom insertions into std::unordered_set<int> cost \t%% bytes\n", i, u);
        }
    }

    printArray("x", counts);
    printArray("y", heapSizes);
}

TEST_CASE("cc_set_lookup_randomized_sparse_runtime", "[cc]")
{
    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v = generateRandomInts(counts[counts.size() - 1]);

    for (int n: counts)
    {
        cc::Set<int> set;

        int64_t dt = benchmark(
            [&]{
                for (int i = 0; i < n; ++i) {
                    if (!set.contains(v[i])) {
                        TEST_ASSERT(false);
                    }
                }
            },
            [&]{
                for (int i = 0; i < n; ++i) {
                    set.insert(v[i]);
                }
            }
        );

        print("%%\trandom lookups into sparse cc::Set<int> cost \t%%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

TEST_CASE("cc_set_lookup_randomized_dense_runtime", "[cc]")
{
    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v = generateRandomInts(counts[counts.size() - 1]);
    std::sort(v.begin(), v.end());

    for (int n: counts)
    {
        cc::Set<int> set;

        int64_t dt = benchmark(
            [&]{
                for (int i = 0; i < n; ++i) {
                    if (!set.contains(v[i])) {
                        TEST_ASSERT(false);
                    }
                }
            },
            [&]{
                for (int i = 0; i < n; ++i) {
                    set.insert(v[i]);
                }
            }
        );

        print("%%\trandom lookups into dense cc::Set<int> cost \t%%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

TEST_CASE("std_set_lookup_randomized_runtime", "[std]")
{
    using namespace cc;

    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v;
    {
        v.resize(counts[counts.size() - 1]);
        Random random { 0 };
        for (int &x: v) x = random();
    }

    for (int n: counts)
    {
        std::set<int> set;

        int64_t dt = benchmark(
            [&]{
                for (int i = 0; i < n; ++i) {
                    if (!set.contains(v[i])) {
                        TEST_ASSERT(false);
                    }
                }
            },
            [&]{
                for (int i = 0; i < n; ++i) {
                    set.insert(v[i]);
                }
            }
        );

        print("%%\trandom lookups into dense std::set<int> cost \t%%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

TEST_CASE("cc_set_destruction_sparse_runtime", "[cc]")
{
    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v = generateRandomInts(counts[counts.size() - 1]);

    for (int n: counts)
    {
        cc::Set<int> set;

        int64_t dt = benchmark(
            [&]{
                set.deplete();
            },
            [&]{
                for (int i = 0; i < n; ++i) {
                    set.insert(v[i]);
                }
            }
        );

        print("%%\tsized sparse cc::Set<int> destruction costs \t%%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

TEST_CASE("cc_set_destruction_dense_runtime", "[cc]")
{
    using namespace cc;

    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v;
    {
        v.resize(counts[counts.size() - 1]);
        Random random { 0 };
        for (int &x: v) x = random();
        std::sort(v.begin(), v.end());
    }

    for (int n: counts)
    {
        cc::Set<int> set;

        int64_t dt = benchmark(
            [&]{
                set.deplete();
            },
            [&]{
                for (int i = 0; i < n; ++i) {
                    set.insert(v[i]);
                }
            }
        );

        print("%%\tsized dense cc::Set<int> destruction costs \t%%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

TEST_CASE("std_set_destruction_runtime", "[std]")
{
    using namespace cc;

    std::vector<int> counts { 100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    std::vector<int64_t> durations;
    durations.reserve(counts.size());

    std::vector<int> v;
    {
        v.resize(counts[counts.size() - 1]);
        Random random { 0 };
        for (int &x: v) x = random();
    }

    for (int n: counts)
    {
        std::set<int> set;

        int64_t dt = benchmark(
            [&]{
                set.clear();
            },
            [&]{
                for (int i = 0; i < n; ++i) {
                    set.insert(v[i]);
                }
            }
        );

        print("%%\tsized std::set<int> destruction costs \t%%us\n", n, dt);
        durations.push_back(dt);
    }

    printArray("x", counts);
    printArray("y", durations);
}

extern "C" void app_main(void)
{
    print("ESP-IDF: %%\n", esp_get_idf_version());
    print("GCC: %%.%%.%%\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    print("GLIBCXX: %%\n", __GLIBCXX__);
    print("CPU: %%\n", CONFIG_IDF_TARGET);

    #if 1
    unity_run_menu();
    #else
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
    #endif
}
