#ifndef FTRL_UTILS_H
#define FTRL_UTILS_H


#include <string>
#include <vector>
using namespace std;

class utils 
{
public:
    void static splitString(string& line, char delimiter, vector<string>* r);
    int static sgn(double x);
    double static uniform();
    double static gaussian();
    double static gaussian(double mean, double stdev);
};


#endif //FTRL_UTILS_H
