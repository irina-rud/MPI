//
// Created by riv on 29.04.16.
//

#include "StripedHashSet.h"

template<typename T>
StripedHashSet::StripedHashSet(int mutex, int growth_factor = 2){
    num_stripes = mutex;
    this->growth_factor = growth_factor;


}

template<typename T>
bool StripedHashSet::contains(const T &e) {
    return false;
}

template<typename T>
void StripedHashSet::remove(const T &e) {

}

template<typename T>
void StripedHashSet::add(const T &e) {

}







