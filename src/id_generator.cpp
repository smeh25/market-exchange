#include "../include/id_generator.hpp"

IdGenerator::IdGenerator(uint64_t start_id) 
    : current_id(start_id) 
{
}


uint64_t IdGenerator::next() {
    return ++current_id;
}