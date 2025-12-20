cmake_minimum_required(VERSION 3.10)
project(MarketExchange)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell the compiler to look in the 'include' folder for headers
include_directories(include)

# Find all your .cpp files in the src directory
file(GLOB SOURCES "src/*.cpp")

# Create the executable
add_executable(market_exchange ${SOURCES})

# Link ZMQ (You will need this once you start the networking part)
find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)

# Link them to your executable
target_link_libraries(market_exchange PRIVATE cppzmq)