#include <string>
#include <iostream>
#include <map>
#include <iostream>
#include "bimap.h"

template struct bimap<int, std::string>;
template struct bimap<int, int>;

using namespace std;

int rand_elem()
{
    return rand() % 16;
}

template <typename BiMapIterator, typename MapIterator>
void check_content(BiMapIterator bimap_begin, BiMapIterator bimap_end,
                   MapIterator map_begin, MapIterator map_end)
{
    while (bimap_begin != bimap_end)
    {
        assert(*bimap_begin == map_begin->first);
        assert(*bimap_begin.flip() == map_begin->second);

        ++bimap_begin;
        ++map_begin;
    }

    assert(map_begin == map_end);
}

int main()
{
    bimap<int, int> a;
    std::map<int, int> left_to_right;
    std::map<int, int> right_to_left;

    for (size_t niter = 0; niter != 100000; ++niter)
    {
        size_t n1 = a.size();
        assert(n1 == left_to_right.size());
        assert(n1 == right_to_left.size());

        check_content(a.begin_left(), a.end_left(), left_to_right.begin(), left_to_right.end());
        check_content(a.begin_right(), a.end_right(), right_to_left.begin(), right_to_left.end());

        bool is_left = rand() % 2;
        int elem = rand_elem();

        bool present_in_bimap;
        bool present_in_map;

        if (is_left)
        {
            present_in_bimap = a.find_left(elem) != a.end_left();
            present_in_map = left_to_right.find(elem) != left_to_right.end();
        }
        else
        {
            present_in_bimap = a.find_right(elem) != a.end_right();
            present_in_map = right_to_left.find(elem) != right_to_left.end();
        }
        assert(present_in_bimap == present_in_map);

        if (present_in_map)
        {
            if (is_left)
                a.erase_left(a.find_left(elem));
            else
                a.erase_right(a.find_right(elem));

            if (is_left)
            {
                right_to_left.erase(left_to_right[elem]);
                left_to_right.erase(elem);
            }
            else
            {
                left_to_right.erase(right_to_left[elem]);
                right_to_left.erase(elem);
            }
        }
        else
        {
            int other;
            if (is_left)
            {
                for (;;)
                {
                    other = rand_elem();
                    if (right_to_left.find(other) == right_to_left.end())
                        break;
                }
            }
            else
            {
                for (;;)
                {
                    other = rand_elem();
                    if (left_to_right.find(other) == left_to_right.end())
                        break;
                }
            }

            if (is_left)
                a.insert(elem, other);
            else
                a.insert(other, elem);

            if (is_left)
            {
                left_to_right[elem] = other;
                right_to_left[other] = elem;
            }
            else
            {
                right_to_left[elem] = other;
                left_to_right[other] = elem;
            }
        }
    }
    return 0;
}

