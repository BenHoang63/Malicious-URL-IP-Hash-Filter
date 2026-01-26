#include "hash_functions.h"
#include "UnorderedMap.h"
#include <iostream>
#include <fstream>
#include <string>

using HashMapType = UnorderedMap<std::string,int,fnv1a_hash>;
using value_type = std::pair<std::string,int>;

/**
 * ## Malicious URL Filter
 * @brief This class is designed to filter given IP addresses using a hash map for O(1) time.
 */
class malicious_url_filter {
    private:
        HashMapType map;

    public:
        /** 
            ## Malicious URL Filter Constructor
            @brief Creates a malicious_url_filter object. 
        **/
        malicious_url_filter() : map(4481) {
            // add all the blocked IPs to the hash map
            std::ifstream file{"resources/block.txt"};
            std::string line;
            int i = 1;
            while (std::getline(file,line)) {
                map.insert(value_type(line,i));
                line.clear();
                ++i;
            }

        }


        /**
            @brief Determines if IP is a malicious URL.

            @param IP the IP address that is to be checked.
        **/
        bool is_Malicious_URL(std::string IP) {
            auto find_IP = this->map.find(IP);
            if (find_IP != HashMapType::iterator()) 
                return true;
            return false;
        }

        /**
         * @brief Returns the load factor of the hash map.
         * 
         * 
         */
        float load_factor() const { return this->map.load_factor(); }

};

