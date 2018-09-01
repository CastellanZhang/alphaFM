#ifndef FTRL_UTILS_H
#define FTRL_UTILS_H

#include <string>
#include <vector>
using namespace std;

class utils
{
public:
    static void split_string(string& line, char delimiter, vector<string>* r);
    static int sgn(double x);
    static double uniform();
    static double gaussian();
    static double gaussian(double mean, double stdev);
    static vector<string> argv_to_args(int argc, char* argv[]);
};


#endif //FTRL_UTILS_H
