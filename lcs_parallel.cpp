#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <barrier>
#include "cxxopts.hpp"


int main(int argc, char* argv[]) {
    cxxopts::Options options("lcs_parallel", "lcs program for cmpt 431 project using threads");

    options.add_options(
        "inputs", {
            {"nThreads", "Number of threads for the program", cxxopts::value<int>()},
            {"blockSize", "Size of the matrix given to each thread", cxxopts::value<int>()},
            {"sequenceFile1", "Number of threads for the program", cxxopts::value<std::string>()},
            {"sequenceFile2", "Number of threads for the program", cxxopts::value<std::string>()}
    });
    auto command_options = options.parse(argc, argv); 
    int n = command_options["nThreads"].as<int>();
    int bSize = command_options["blockSize"].as<int>();
    std::string seqOne = command_options["sequenceFile1"].as<std::string>();
    std::string seqTwo = command_options["sequenceFile2"].as<std::string>();


    std::cout << "Number of Threads: " << n << std::endl;
    std::cout << "Matrix Block Size: " << bSize << std::endl;
    std::cout << "Sequence 1: " << seqOne << std::endl;
    std::cout << "Sequence 2: " << seqTwo << std::endl;
    std::cout << "END OF PROGRAM";

    return 0;
}
