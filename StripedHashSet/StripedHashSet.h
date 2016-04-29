//
// Created by riv on 29.04.16.
//

#ifndef STRIPEDHASHSET_STRIPEDHASHSET_H
#define STRIPEDHASHSET_STRIPEDHASHSET_H

#include <iostream>

template<typename T, class H = std::hash<T>>
class StripedHashSet {
public:
    StripedHashSet(int mutex, int growth_factor = 2);
    void add(const T& e);
    void remove(const T& e);
    bool contains(const T& e);

private:
    int num_stripes;
    int growth_factor;
    double load_factor;
    double max_load_factor;

};


#endif //STRIPEDHASHSET_STRIPEDHASHSET_H
