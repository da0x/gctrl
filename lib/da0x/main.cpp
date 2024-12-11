#include <iostream>
#include "uuid.hpp"

int main() {
    std::cout << uuid::v4::generate() << std::endl;
    return 0;
}

