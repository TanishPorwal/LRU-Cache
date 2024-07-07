#include <iostream>

#include "LRU Cache.h"

int main() {
    LRU_Cache<int, int> lru(10);

    for(int i = 0; i < 11; i++)
        lru.insert_or_assign(i, i);

    for(const auto& i : lru)
        std::cout<<i.second.first<<"\n";

    lru.clear();

    return 0;
}
