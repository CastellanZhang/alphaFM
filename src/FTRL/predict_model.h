#ifndef PREDICT_MODEL_H_
#define PREDICT_MODEL_H_

#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstring>
#include "../Utils/utils.h"
#include "../Mem/mem_pool.h"
#include "../Mem/my_allocator.h"
#include "model_bin_file.h"
#include "ftrl_model.h"

using namespace std;

//每一个特征维度的模型单元
template<typename T>
class predict_model_unit
{
public:
    T wi;
    
private:
    static size_t offset_v;
    static size_t class_size;
    static int ext_mem_size;
    static int factor_num;

public:
    static void static_init(int _factor_num)
    {
        factor_num = _factor_num;
        offset_v = 0;
        class_size = sizeof(predict_model_unit<T>);
        using node_type = std::__detail::_Hash_node<std::pair<const char* const, predict_model_unit<T> >, false>;
        size_t offset_this = get_value_offset_in_Hash_node((node_type*)NULL);
        size_t padding = sizeof(node_type) - offset_this - class_size;
        if(padding > 4)
        {
            cerr << "predict_model_unit::static_init: padding size exception" << endl;
            exit(1);
        }
        ext_mem_size = (int)(_factor_num * sizeof(T)) - (int)padding;
    }
    
    
    inline static int get_ext_mem_size()
    {
        return ext_mem_size;
    }
    
    
    static predict_model_unit* create_instance(int _factor_num, double _wi, const vector<double>& _vi)
    {
        size_t mem_size = class_size + factor_num * sizeof(T);
        void* pMem = malloc(mem_size);
        predict_model_unit* pInstance = new(pMem) predict_model_unit();
        pInstance->instance_init(_factor_num, _wi, _vi);
        return pInstance;
    }
    
    
    static bool parse_txt_model_line(int _factor_num, const vector<string>& modelLineSeg, double& _wi, vector<double>& _vi)
    {
        bool res = false;
        _wi = stod(modelLineSeg[1]);
        if(0.0 != _wi) res = true;
        for(int f = 0; f < _factor_num; ++f)
        {
            _vi[f] = stod(modelLineSeg[2 + f]);
            if(0.0 != _vi[f]) res = true;
        }
        return res;
    }
    
    
    predict_model_unit()
    {}
    
    
    void instance_init(int _factor_num, double _wi, const vector<double>& _vi)
    {
        wi = _wi;
        for(int f = 0; f < _factor_num; ++f)
        {
            vi(f) = _vi[f];
        }
    }
    
    
    void instance_init(int _factor_num, const ftrl_model_unit<T>& fmu)
    {
        wi = fmu.wi;
        for(int f = 0; f < _factor_num; ++f)
        {
            vi(f) = fmu.vi(f);
        }
    }
    
    
    inline T& vi(size_t f) const
    {
        char* p = (char*)this + class_size;
        return *((T*)p + offset_v + f);
    }
};

template<typename T>
size_t predict_model_unit<T>::offset_v;
template<typename T>
size_t predict_model_unit<T>::class_size;
template<typename T>
int predict_model_unit<T>::ext_mem_size;
template<typename T>
int predict_model_unit<T>::factor_num;


template<typename T>
using predict_hash_map = unordered_map<const char*, predict_model_unit<T>, my_hash, my_equal, my_allocator<pair<const char*, predict_model_unit<T> >, T, predict_model_unit> >;


template<typename T>
class predict_model
{
public:
    predict_model_unit<T>* muBias;
    predict_hash_map<T> muMap;
    int factor_num;

public:
    predict_model(int _factor_num);
    double get_score(const vector<pair<string, double> >& x, double bias, predict_hash_map<T>& theta);
    bool load_model(const string& modelPath, const string& modelFormat);

private:
    bool load_txt_model(const string& modelPath);
    bool load_txt_model(ifstream& in);
    bool load_bin_model(const string& modelPath);
    inline double get_wi(predict_hash_map<T>& theta, const string& index);
    inline double get_vif(predict_hash_map<T>& theta, const string& index, int f);
    inline char* create_fea_c_str(const char* key);
};


template<typename T>
predict_model<T>::predict_model(int _factor_num)
{
    factor_num = _factor_num;
    muBias = NULL;
    predict_model_unit<T>::static_init(_factor_num);
}


template<typename T>
double predict_model<T>::get_score(const vector<pair<string, double> >& x, double bias, predict_hash_map<T>& theta)
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


template<typename T>
double predict_model<T>::get_wi(predict_hash_map<T>& theta, const string& index)
{
    auto iter = theta.find(index.c_str());
    if(iter == theta.end())
    {
        return 0.0;
    }
    else
    {
        return iter->second.wi;
    }
}


template<typename T>
char* predict_model<T>::create_fea_c_str(const char* key)
{
    size_t len = strlen(key);
    char* p = (char*)mem_pool::get_mem(len + 1);
    strncpy(p, key, len);
    p[len] = 0;
    return p;
}


template<typename T>
double predict_model<T>::get_vif(predict_hash_map<T>& theta, const string& index, int f)
{
    auto iter = theta.find(index.c_str());
    if(iter == theta.end())
    {
        return 0.0;
    }
    else
    {
        return iter->second.vi(f);
    }
}


template<typename T>
bool predict_model<T>::load_model(const string& modelPath, const string& modelFormat)
{
    if("txt" == modelFormat) return load_txt_model(modelPath);
    else if("bin" == modelFormat) return load_bin_model(modelPath);
    else return false;
}


template<typename T>
bool predict_model<T>::load_txt_model(const string& modelPath)
{
    ifstream in(modelPath);
    if(!in) return false;
    bool res = load_txt_model(in);
    in.close();
    return res;
}


template<typename T>
bool predict_model<T>::load_txt_model(ifstream& in)
{
    string line;
    if(!getline(in, line))
    {
        return false;
    }
    vector<string> strVec;
    utils::split_string(line, ' ', &strVec);
    if(strVec.size() != 4)
    {
        return false;
    }
    double _wi = 0;
    vector<double> _vi(0);
    predict_model_unit<T>::parse_txt_model_line(0, strVec, _wi, _vi);
    muBias = predict_model_unit<T>::create_instance(0, _wi, _vi);
    _vi.resize(factor_num);
    while(getline(in, line))
    {
        strVec.clear();
        utils::split_string(line, ' ', &strVec);
        if(strVec.size() != 3 * factor_num + 4)
        {
            return false;
        }
        if(predict_model_unit<T>::parse_txt_model_line(factor_num, strVec, _wi, _vi))
        {
            string& index = strVec[0];
            char* pKey = create_fea_c_str(index.c_str());
            muMap[pKey].instance_init(factor_num, _wi, _vi);
        }
    }
    return true;
}


template<typename T>
bool predict_model<T>::load_bin_model(const string& modelPath)
{
    ftrl_model_unit<T>::static_init(factor_num);
    ftrl_model_unit<T>* pMu = ftrl_model_unit<T>::create_instance(0, 0.0, 0.0);
    model_bin_file mbf;
    if(!mbf.open_file_for_read(modelPath)) return false;
    if(mbf.get_num_byte_len() != sizeof(T)) return false;
    if(mbf.get_factor_num() != (size_t)factor_num) return false;
    if(mbf.get_unit_len() != ftrl_model_unit<T>::get_mem_size()) return false;
    char* buffer = new char[64*1024];
    unsigned short feaLen;
    if(!mbf.read_one_fea(buffer, feaLen)) return false;
    buffer[feaLen] = 0;
    if(ftrl_model<T>::get_bias_fea_name() != string(buffer)) return false;
    double _wi = 0;
    vector<double> _vi(0);
    muBias = predict_model_unit<T>::create_instance(0, _wi, _vi);
    if(!mbf.read_one_unit(pMu)) return false;
    muBias->instance_init(0, *pMu);
    for(size_t i = 1; i < mbf.get_fea_num(); ++i)
    {
        if(!mbf.read_one_fea(buffer, feaLen)) return false;
        buffer[feaLen] = 0;
        if(!mbf.read_one_unit(pMu)) return false;
        if(!(pMu->is_none_zero())) continue;
        char* pKey = create_fea_c_str(buffer);
        muMap[pKey].instance_init(factor_num, *pMu);
    }
    if(!mbf.close_file()) return false;
    free(pMu);
    delete[] buffer;
    return true;
}



#endif /*PREDICT_MODEL_H_*/
