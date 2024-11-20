#include <iostream>
#include <iomanip>
#include <string.h>
#include <algorithm>    // std::max


class LongestCommonSubsequenceSerial {
private:
    const std::string sequence_a;
    const std::string sequence_b;
    const uint length_a; // Length of sequence_a.
    const uint length_b; // Length of sequence_b.

    uint **matrix; /* Solution matrix - matrix[i][j] stores the length of
        the longest common subsequence of the first i-1 elements of sequence_a 
        and the first j-1 elements of sequence_b. */

    

public:
    LongestCommonSubsequenceSerial(const std::string sequence_a, const std::string sequence_b)
        : sequence_a(sequence_a), sequence_b(sequence_b), length_a(sequence_a.length()), length_b(sequence_b.length())
    {
        matrix = new uint *[length_a];
        for (uint i = 0; i < length_a; i++) {
            matrix[i] = new uint [length_b];
            for (uint j = 0; j < length_b; j++) {
                matrix[i][j] = 0;
            }
        }
    }

    ~LongestCommonSubsequenceSerial() {
        for (uint i = 0; i < length_a; i++) {
            delete matrix[i];
        }
        delete[] matrix;
    }

    // Print the matrix to the console.
    void print() const {
        /* Prints the matrix in the format:
         *      b1 b2 b3
         * a0 [ x  x  x ]
         * a1 [ x  x  x ]
         * a2 [ x  x  x ]
         */
        // Print the elements of sequence_b along the top row.
        std::cout << std::setw(3) << " "  << std::right ; /* Extra padding before first element 
            to account for sequence_a being printed down the left side. */
        for (uint j = 0; j < length_b; j++) {
            std::cout << std::setw(2) << sequence_b[j];
        }
        std::cout << "\n";

        for (uint i = 0; i < length_a; i++) {
            // Print sequence_a down the left side of the matrix.
            std::cout << std::left << std::setw(2) << sequence_a[i] << "[" 
                << std::right;
            for (uint j = 0; j < length_b; j++) {
                std::cout << std::setw(2) << matrix[i][j];
            }
            std::cout << " ]\n";
        }
        std::cout << std::endl;
    }
};

int main(int argc, char *argv[]) {

    std::string sequence_a = "drfghjk";
    std::string sequence_b = "dlpkgcqiuyhnjk";

    const LongestCommonSubsequenceSerial lcs(sequence_a, sequence_b);
    lcs.print();

    return 0;
}
