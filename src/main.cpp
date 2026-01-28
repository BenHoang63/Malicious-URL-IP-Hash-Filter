#include "malicious_url_filter.h"
#include <iostream>
#include <string>

int main() {
    malicious_url_filter filter = malicious_url_filter();

    // find the IP in the hash map
    std::string IP{"2.57.149.0/24"};
    if (filter.is_Malicious_URL(IP)) 
        std::cout << IP << " found. This URL is malicious.\n";
    else
        std::cout << IP << " not found. This URL is safe.\n";
    
    std::cout << "Load factor: " << filter.load_factor() << "\n";

}