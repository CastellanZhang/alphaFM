#ifndef FM_SAMPLE_H_
#define FM_SAMPLE_H_

#include <string>
#include <vector>
#include <iostream>

using namespace std;


class fm_sample
{
public:
    int y;
    vector<pair<string, double> > x;
    fm_sample(const string& line);
private:
    static const string spliter;
    static const string innerSpliter;
};

const string fm_sample::spliter = " ";
const string fm_sample::innerSpliter = ":";


fm_sample::fm_sample(const string& line)
{
    this->x.clear();
    size_t posb = line.find_first_not_of(spliter, 0);
    size_t pose = line.find_first_of(spliter, posb);
    int label = atoi(line.substr(posb, pose-posb).c_str());
    this->y = label > 0 ? 1 : -1;
    string key;
    double value;
    while(pose < line.size())
    {
        posb = line.find_first_not_of(spliter, pose);
        if(posb == string::npos)
        {
            break;
        }
        pose = line.find_first_of(innerSpliter, posb);
        if(pose == string::npos)
        {
            cerr << "wrong line of sample input\n" << line << endl;
            exit(1);
        }
        key = line.substr(posb, pose-posb);
        posb = pose + 1;
        if(posb >= line.size())
        {
            cerr << "wrong line of sample input\n" << line << endl;
            exit(1);
        }
        pose = line.find_first_of(spliter, posb);
        value = stod(line.substr(posb, pose-posb));
        if(value != 0)
        {
            this->x.push_back(make_pair(key, value));
        }
    }
}


#endif /*FM_SAMPLE_H_*/
