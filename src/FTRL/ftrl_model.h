#ifndef FTRL_MODEL_H_
#define FTRL_MODEL_H_

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <iostream>
#include <cmath>
#include "../Utils/utils.h"

using namespace std;

//每一个特征维度的模型单元
class ftrl_model_unit
{
public:
    double wi;
    double w_ni;
    double w_zi;
    vector<double> vi;
    vector<double> v_ni;
    vector<double> v_zi;
    mutex mtx;
public:
    ftrl_model_unit(int factor_num, double v_mean, double v_stdev)
    {
        wi = 0.0;
        w_ni = 0.0;
        w_zi = 0.0;
        vi.resize(factor_num);
        v_ni.resize(factor_num);
        v_zi.resize(factor_num);
        for(int f = 0; f < factor_num; ++f)
        {
            vi[f] = utils::gaussian(v_mean, v_stdev);
            v_ni[f] = 0.0;
            v_zi[f] = 0.0;
        }
    }

    ftrl_model_unit(int factor_num, const vector<string>& modelLineSeg)
    {
        vi.resize(factor_num);
        v_ni.resize(factor_num);
        v_zi.resize(factor_num);
        wi = stod(modelLineSeg[1]);
        w_ni = stod(modelLineSeg[2 + factor_num]);
        w_zi = stod(modelLineSeg[3 + factor_num]);
        for(int f = 0; f < factor_num; ++f)
        {
            vi[f] = stod(modelLineSeg[2 + f]);
            v_ni[f] = stod(modelLineSeg[4 + factor_num + f]);
            v_zi[f] = stod(modelLineSeg[4 + 2 * factor_num + f]);
        }
    }

    void reinit_vi(double v_mean, double v_stdev)
    {
        int size = vi.size();
        for(int f = 0; f < size; ++f)
        {
            vi[f] = utils::gaussian(v_mean, v_stdev);
        }
    }

    friend inline ostream& operator <<(ostream& os, const ftrl_model_unit& mu)
    {
        os << mu.wi;
        for(int f = 0; f < mu.vi.size(); ++f)
        {
            os << " " << mu.vi[f];
        }
        os << " " << mu.w_ni << " " << mu.w_zi;
        for(int f = 0; f < mu.v_ni.size(); ++f)
        {
            os << " " << mu.v_ni[f];
        }
        for(int f = 0; f < mu.v_zi.size(); ++f)
        {
            os << " " << mu.v_zi[f];
        }
        return os;
    }
};



class ftrl_model
{
public:
    ftrl_model_unit* muBias;
    unordered_map<string, ftrl_model_unit*> muMap;

    int factor_num;
    double init_stdev;
    double init_mean;

public:
    ftrl_model(double _factor_num);
    ftrl_model(double _factor_num, double _mean, double _stdev);
    ftrl_model_unit* getOrInitModelUnit(string index);
    ftrl_model_unit* getOrInitModelUnitBias();

    double predict(const vector<pair<string, double> >& x, double bias, vector<ftrl_model_unit*>& theta, vector<double>& sum);
    double getScore(const vector<pair<string, double> >& x, double bias, unordered_map<string, ftrl_model_unit*>& theta);
    void outputModel(ofstream& out);
    bool loadModel(ifstream& in);
    void debugPrintModel();

private:
    double get_wi(unordered_map<string, ftrl_model_unit*>& theta, const string& index);
    double get_vif(unordered_map<string, ftrl_model_unit*>& theta, const string& index, int f);
private:
    mutex mtx;
    mutex mtx_bias;
};


ftrl_model::ftrl_model(double _factor_num)
{
    factor_num = _factor_num;
    init_mean = 0.0;
    init_stdev = 0.0;
    muBias = NULL;
}

ftrl_model::ftrl_model(double _factor_num, double _mean, double _stdev)
{
    factor_num = _factor_num;
    init_mean = _mean;
    init_stdev = _stdev;
    muBias = NULL;
}


ftrl_model_unit* ftrl_model::getOrInitModelUnit(string index)
{
    unordered_map<string, ftrl_model_unit*>::iterator iter = muMap.find(index);
    if(iter == muMap.end())
    {
        mtx.lock();
        ftrl_model_unit* pMU = new ftrl_model_unit(factor_num, init_mean, init_stdev);
        muMap.insert(make_pair(index, pMU));
        mtx.unlock();
        return pMU;
    }
    else
    {
        return iter->second;
    }
}


ftrl_model_unit* ftrl_model::getOrInitModelUnitBias()
{
    if(NULL == muBias)
    {
        mtx_bias.lock();
        muBias = new ftrl_model_unit(0, init_mean, init_stdev);
        mtx_bias.unlock();
    }
    return muBias;
}


double ftrl_model::predict(const vector<pair<string, double> >& x, double bias, vector<ftrl_model_unit*>& theta, vector<double>& sum)
{
    double result = 0;
    result += bias;
    for(int i = 0; i < x.size(); ++i)
    {
        result += theta[i]->wi * x[i].second;
    }
    double sum_sqr, d;
    for(int f = 0; f < factor_num; ++f)
    {
        sum[f] = sum_sqr = 0.0;
        for(int i = 0; i < x.size(); ++i)
        {
            d = theta[i]->vi[f] * x[i].second;
            sum[f] += d;
            sum_sqr += d * d;
        }
        result += 0.5 * (sum[f] * sum[f] - sum_sqr);
    }
    return result;
}


double ftrl_model::getScore(const vector<pair<string, double> >& x, double bias, unordered_map<string, ftrl_model_unit*>& theta)
{
    double result = 0;
    result += bias;
    for(int i = 0; i < x.size(); ++i)
    {
        result += get_wi(theta, x[i].first) * x[i].second;
    }
    double sum, sum_sqr, d;
    for(int f = 0; f < factor_num; ++f)
    {
        sum = sum_sqr = 0.0;
        for(int i = 0; i < x.size(); ++i)
        {
            d = get_vif(theta, x[i].first, f) * x[i].second;
            sum += d;
            sum_sqr += d * d;
        }
        result += 0.5 * (sum * sum - sum_sqr);
    }
    return 1.0/(1.0 + exp(-result));
}


double ftrl_model::get_wi(unordered_map<string, ftrl_model_unit*>& theta, const string& index)
{
    unordered_map<string, ftrl_model_unit*>::iterator iter = theta.find(index);
    if(iter == theta.end())
    {
        return 0.0;
    }
    else
    {
        return iter->second->wi;
    }
}


double ftrl_model::get_vif(unordered_map<string, ftrl_model_unit*>& theta, const string& index, int f)
{
    unordered_map<string, ftrl_model_unit*>::iterator iter = theta.find(index);
    if(iter == theta.end())
    {
        return 0.0;
    }
    else
    {
        return iter->second->vi[f];
    }
}


void ftrl_model::outputModel(ofstream& out)
{
    out << "bias " << *muBias << endl;
    for(unordered_map<string, ftrl_model_unit*>::iterator iter = muMap.begin(); iter != muMap.end(); ++iter)
    {
        out << iter->first << " " << *(iter->second) << endl;
    }
}


void ftrl_model::debugPrintModel()
{
    cout << "bias " << *muBias << endl;
    for(unordered_map<string, ftrl_model_unit*>::iterator iter = muMap.begin(); iter != muMap.end(); ++iter)
    {
        cout << iter->first << " " << *(iter->second) << endl;
    }
}


bool ftrl_model::loadModel(ifstream& in)
{
    string line;
    if(!getline(in, line))
    {
        return false;
    }
    vector<string> strVec;
    utils::splitString(line, ' ', &strVec);
    if(strVec.size() != 4)
    {
        return false;
    }
    muBias = new ftrl_model_unit(0, strVec);
    while(getline(in, line))
    {
        strVec.clear();
        utils::splitString(line, ' ', &strVec);
        if(strVec.size() != 3 * factor_num + 4)
        {
            return false;
        }
        string& index = strVec[0];
        ftrl_model_unit* pMU = new ftrl_model_unit(factor_num, strVec);
        muMap[index] = pMU;
    }
    return true;
}



#endif /*FTRL_MODEL_H_*/
