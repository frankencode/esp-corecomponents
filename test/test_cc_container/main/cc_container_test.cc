#include <cc/List>
#include <cc/Map>
#include <cc/MultiMap>
#include <cc/Set>
#include <cc/Array>
#include <cc/Function>
#include <cc/Random>
#include <cc/stdio>
#include <sdkconfig.h>

#include <unity.h>
#include <unity_test_utils.h>

using namespace cc;

TEST_CASE("cc_list_append", "[cc]")
{
    const int n = 1024;

    List<int> list;
    for (int i = 0; i < n; ++i) list << i;

    TEST_ASSERT(list.count() == n);

    printf("list.count() = %ld\n", list.count());

    bool contentOk = true;
    for (int i = 0; i < n; ++i) {
        contentOk = contentOk && (list.at(i) == i);
    }

    TEST_ASSERT(contentOk);
    TEST_ASSERT(list.tree().checkBalance());
}

TEST_CASE("cc_list_insert_front", "[cc]")
{
    const int n = 1024;

    List<int> list;
    for (int i = 0; i < n; ++i) list.pushFront(i);

    TEST_ASSERT(list.count() == n);

    bool contentOk = true;
    for (int i = 0; i < n; ++i) {
        contentOk = contentOk && (list.at(n - i - 1) == i);
    }

    TEST_ASSERT(contentOk);
    TEST_ASSERT(list.tree().checkBalance());

    #if 0
    list.tree().check([](auto *node){
        printf("node->fill_ = %d\n", static_cast<int>(node->fill_));
        return true;
    });
    #endif
}

TEST_CASE("cc_list_sort", "[cc]")
{
    const int n = 128;

    List<int> a;
    Random random{0};
    for (int i = 0; i < n; ++i)
        a.append(random.get(0, n));

    auto b = a.sorted();
    // CC_INSPECT(a);
    // CC_INSPECT(b);

    TEST_ASSERT(a.count() == b.count());

    for (int i = 0; i < b.count() - 1; ++i) {
        TEST_ASSERT(b.at(i) <= b.at(i + 1));
        TEST_ASSERT(a.find(b.at(i)));
    }
}

TEST_CASE("cc_list_copy_on_write", "[cc]")
{
    List<int> a;
    Random random{0};
    for (int i = 0; i < 20; ++i) {
        a.append(random.get(0, 99));
    }
    // List<int> b { a };
    List<int> b = a;
    b.append(4);
    TEST_ASSERT(a.count() + 1 == b.count());
    for (auto pos = a.head(); pos; ++pos)
        TEST_ASSERT(a.at(pos) == b.at(+pos));

    TEST_ASSERT(sizeof(List<int>) == sizeof(void *));
}

TEST_CASE("cc_map_insert_operator", "[cc]")
{
    Map<int> m;
    m(0) = 1;
    m(1) = 2;
    TEST_ASSERT(m.count() == 2);
    TEST_ASSERT(m(0) == 1);
    TEST_ASSERT(m(1) == 2);
}

TEST_CASE("cc_map_insert_remove", "[cc]")
{
    Map<int> map;
    const int n = 42;
    {
        Random random{0};
        for (int i = 0; i < n; ++i) {
            int key = random.get();
            int value = random.get();
            map.insert(key, value);
        }
    }
    TEST_ASSERT(map.count() == n);
    {
        Random random{0};
        for (int i = 0; i < n; ++i) {
            int key = random.get();
            int value = random.get();
            TEST_ASSERT(map.value(key) == value);
        }
    }
    {
        Random random{0};
        for (int i = 0; i < n; ++i) {
            int j = random.get(0, map.count() - 1);
            map.remove(map.at(j).key());
        }
    }
    TEST_ASSERT(map.count() == 0);
}

TEST_CASE("cc_map_morph_to_list", "[cc]")
{
    Map<int> map;
    const int n = 5;
    {
        Random random { 0 };
        for (int i = 0; i < n; ++i) {
            int key = random.get();
            int value = random.get();
            map.insert(key, value);
        }
    }

    List<KeyValue<int>> list { std::move(map) };
    {
        Random random { 0 };
        for (int i = 0; i < n; ++i) {
            int key = random.get();
            int value = random.get();
            TEST_ASSERT(list.find(KeyValue{key, value}));
        }
    }
}

TEST_CASE("cc_multimap_insert", "[cc]")
{
    MultiMap<int> m;
    m.insert(0, 1);
    m.insert(0, 2);
    m.insert(1, 3);
    m.insert(1, 4);
    TEST_ASSERT(m.at(0).key() == 0);
    TEST_ASSERT(m.at(0).value() == 1);
    TEST_ASSERT(m.at(1).key() == 0);
    TEST_ASSERT(m.at(1).value() == 2);
    TEST_ASSERT(m.at(2).key() == 1);
    TEST_ASSERT(m.at(2).value() == 3);
    TEST_ASSERT(m.at(3).key() == 1);
    TEST_ASSERT(m.at(3).value() == 4);
}

TEST_CASE("cc_multimap_insert_remove", "[cc]")
{
    MultiMap<int> map;
    const int n = 42;
    {
        Random random { 0 };
        for (int i = 0; i < n; ++i) {
            int key = random.get();
            int value = random.get();
            map.insert(key, value);
        }
    }
    TEST_ASSERT(map.count() == n);
    {
        Random random { 0 };
        for (int i = 0; i < n; ++i) {
            int key = random.get();
            int value = random.get();
            Locator pos;
            TEST_ASSERT(map.find(key, &pos));
            TEST_ASSERT(map.at(pos).value() == value);
        }
    }
    {
        Random random { 0 };
        for (int i = 0; map.count() > 0; ++i) {
            int j = random.get(0, map.count() - 1);
            map.remove(map.at(j).key());
        }
    }
    TEST_ASSERT(map.count() == 0);
}

TEST_CASE("cc_multimap_morph_to_list", "[cc]")
{
    MultiMap<int> map;
    const int n = 5;
    {
        Random random { 0 };
        for (int i = 0; i < n; ++i) {
            int key = random.get();
            int value = random.get();
            map.insert(key, value);
        }
    }

    List<KeyValue<int>> list { std::move(map) };
    {
        Random random { 0 };
        for (int i = 0; i < n; ++i) {
            int key = random.get();
            int value = random.get();
            TEST_ASSERT(list.find(KeyValue{key, value}));
        }
    }
}

extern "C" void app_main(void)
{
    #if 1
    unity_run_menu();
    #else
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
    #endif
}
