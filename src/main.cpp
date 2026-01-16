#include "hash_functions.h"
#include "UnorderedMap.h"
#include <iostream>
#include <fstream>
#include <string>

using HashMapType = UnorderedMap<std::string,int,fnv1a_hash>;
using value_type = std::pair<std::string,int>;

bool find_ip(HashMapType map, std::string IP) {
    auto find_IP = map.find(IP);
    if (find_IP != HashMapType::iterator()) 
        return true;
    return false;
}

int main() {

    // initialize the hash map
    HashMapType map(4481);

    // add all the blocked IPs to the hash map
    std::ifstream file{"resources/block.txt"};
    std::string line;
    int i = 1;
    while (std::getline(file,line)) {
        map.insert(value_type(line,i));
        line.clear();
        ++i;
    }

    // find the IP in the hash map
    std::string IP{"217.60.239.0/24"};
    bool IP_found = find_ip(map,IP);
    if (IP_found) 
        std::cout << IP << " found.\n";
    else
        std::cout << IP << " not found.\n";

}