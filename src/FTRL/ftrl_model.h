#ifndef FTRL_MODEL_H_
#define FTRL_MODEL_H_

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include "../Utils/utils.h"
#include "../Mem/mem_pool.h"
#include "../Mem/my_allocator.h"
#include "model_bin_file.h"

using namespace std;

//每一个特征维度的模型单元
template<typename T>
class ftrl_model_unit
{
public:
    T wi;
    T w_ni;
    T w_zi;
    
private:
    static size_t offset_v;
    static size_t offset_vn;
    static size_t offset_vz;
    static size_t class_size;
    static int ext_mem_size;
    static int factor_num;

public:
    static void static_init(int _factor_num)
    {
        factor_num = _factor_num;
        offset_v = 0;
        offset_vn = offset_v + _factor_num;
        offset_vz = offset_vn + _factor_num;
        class_size = sizeof(ftrl_model_unit<T>);
        using node_type = std::__detail::_Hash_node<std::pair<const char* const, ftrl_model_unit<T> >, false>;
        size_t offset_this = get_value_offset_in_Hash_node((node_type*)NULL);
        size_t padding = sizeof(node_type) - offset_this - class_size;
        if(padding > 4)
        {
            cerr << "ftrl_model_unit::static_init: padding size exception" << endl;
            exit(1);
        }
        ext_mem_size = (int)(3 * _factor_num * sizeof(T)) - (int)padding;
    }
    
    
    inline static int get_ext_mem_size()
    {
        return ext_mem_size;
    }
    
    
    static size_t get_mem_size()
    {
        return class_size + 3 * factor_num * sizeof(T);
    }
    
    
    static ftrl_model_unit* create_instance(int _factor_num, double v_mean, double v_stdev)
    {
        size_t mem_size = get_mem_size();
        void* pMem = malloc(mem_size);
        ftrl_model_unit* pInstance = new(pMem) ftrl_model_unit();
        pInstance->instance_init(_factor_num, v_mean, v_stdev);
        return pInstance;
    }
    
    
    static ftrl_model_unit* create_instance(int _factor_num, const vector<string>& modelLineSeg)
    {
        size_t mem_size = get_mem_size();
        void* pMem = malloc(mem_size);
        ftrl_model_unit* pInstance = new(pMem) ftrl_model_unit();
        pInstance->instance_init(_factor_num, modelLineSeg);
        return pInstance;
    }
    
    
    ftrl_model_unit()
    {}
    
    
    void instance_init(int _factor_num, double v_mean, double v_stdev)
    {
        wi = 0.0;
        w_ni = 0.0;
        w_zi = 0.0;
        for(int f = 0; f < _factor_num; ++f)
        {
            vi(f) = utils::gaussian(v_mean, v_stdev);
            v_ni(f) = 0.0;
            v_zi(f) = 0.0;
        }
    }
    
    
    void instance_init(int _factor_num, const vector<string>& modelLineSeg)
    {
        wi = stod(modelLineSeg[1]);
        w_ni = stod(modelLineSeg[2 + _factor_num]);
        w_zi = stod(modelLineSeg[3 + _factor_num]);
        for(int f = 0; f < _factor_num; ++f)
        {
            vi(f) = stod(modelLineSeg[2 + f]);
            v_ni(f) = stod(modelLineSeg[4 + _factor_num + f]);
            v_zi(f) = stod(modelLineSeg[4 + 2 * _factor_num + f]);
        }
    }
    
    
    inline T& vi(size_t f) const
    {
        char* p = (char*)this + class_size;
        return *((T*)p + offset_v + f);
    }
    
    
    inline T& v_ni(size_t f) const
    {
        char* p = (char*)this + class_size;
        return *((T*)p + offset_vn + f);
    }
    
    
    inline T& v_zi(size_t f) const
    {
        char* p = (char*)this + class_size;
        return *((T*)p + offset_vz + f);
    }
    
    
    void reinit_vi(double v_mean, double v_stdev)
    {
        for(int f = 0; f < factor_num; ++f)
        {
            vi(f) = utils::gaussian(v_mean, v_stdev);
        }
    }
    
    
    inline bool is_none_zero()
    {
        if(0.0 != wi) return true;
        for(int f = 0; f < factor_num; ++f)
        {
            if(0.0 != vi(f)) return true;
        }
        return false;
    }
    
    
    friend inline ostream& operator <<(ostream& os, const ftrl_model_unit& mu)
    {
        os << mu.wi;
        for(int f = 0; f < ftrl_model_unit::factor_num; ++f)
        {
            os << " " << mu.vi(f);
        }
        os << " " << mu.w_ni << " " << mu.w_zi;
        for(int f = 0; f < ftrl_model_unit::factor_num; ++f)
        {
            os << " " << mu.v_ni(f);
        }
        for(int f = 0; f < ftrl_model_unit::factor_num; ++f)
        {
            os << " " << mu.v_zi(f);
        }
        return os;
    }
};

template<typename T>
size_t ftrl_model_unit<T>::offset_v;
template<typename T>
size_t ftrl_model_unit<T>::offset_vn;
template<typename T>
size_t ftrl_model_unit<T>::offset_vz;
template<typename T>
size_t ftrl_model_unit<T>::class_size;
template<typename T>
int ftrl_model_unit<T>::ext_mem_size;
template<typename T>
int ftrl_model_unit<T>::factor_num;


template<typename T>
using my_hash_map = unordered_map<const char*, ftrl_model_unit<T>, my_hash, my_equal, my_allocator<pair<const char*, ftrl_model_unit<T> >, T, ftrl_model_unit> >;


template<typename T>
class ftrl_model
{
public:
    ftrl_model_unit<T>* muBias;
    my_hash_map<T> muMap;

    int factor_num;
    double init_stdev;
    double init_mean;

public:
    ftrl_model(int _factor_num);
    ftrl_model(int _factor_num, double _mean, double _stdev);
    ftrl_model_unit<T>* get_or_init_model_unit(const string& index);
    ftrl_model_unit<T>* get_or_init_model_unit_bias();

    double predict(const vector<pair<string, double> >& x, double bias, vector<ftrl_model_unit<T>*>& theta, vector<double>& sum);
    bool output_model(const string& modelPath, const string& modelFormat);
    void output_model_one_line(ostream& out, const char* feaName, ftrl_model_unit<T>* pMu, bool isBias = false);
    bool load_model(const string& modelPath, const string& modelFormat);
    inline bool convert_one_line_of_txt_model_to_vec(ifstream& in, vector<string>& strVec, bool& dataFmtErr, bool isBias = false);
    size_t get_unit_mem_size();
    static const string& get_bias_fea_name();

private:
    bool load_txt_model(const string& modelPath);
    bool load_txt_model(ifstream& in);
    bool load_bin_model(const string& modelPath);
    bool output_txt_model(const string& modelPath);
    bool output_bin_model(const string& modelPath);
    inline char* create_fea_c_str(const char* key);
    
private:
    mutex mtx;
    mutex mtx_bias;
    static const string biasFeaName;
};

template<typename T>
const string ftrl_model<T>::biasFeaName = "bias";


template<typename T>
ftrl_model<T>::ftrl_model(int _factor_num)
{
    factor_num = _factor_num;
    init_mean = 0.0;
    init_stdev = 0.0;
    muBias = NULL;
    ftrl_model_unit<T>::static_init(_factor_num);
}


template<typename T>
ftrl_model<T>::ftrl_model(int _factor_num, double _mean, double _stdev)
{
    factor_num = _factor_num;
    init_mean = _mean;
    init_stdev = _stdev;
    muBias = NULL;
    ftrl_model_unit<T>::static_init(_factor_num);
}


template<typename T>
ftrl_model_unit<T>* ftrl_model<T>::get_or_init_model_unit(const string& index)
{
    auto iter = muMap.find(index.c_str());
    if(iter == muMap.end())
    {
        mtx.lock();
        ftrl_model_unit<T>* pMU = NULL;
        iter = muMap.find(index.c_str());
        if(iter != muMap.end())
        {
            pMU = &(iter->second);
        }
        else
        {
            char* pKey = create_fea_c_str(index.c_str());
            muMap[pKey].instance_init(factor_num, init_mean, init_stdev);
            pMU = &muMap[pKey];
        }
        mtx.unlock();
        return pMU;
    }
    else
    {
        return &(iter->second);
    }
}


template<typename T>
ftrl_model_unit<T>* ftrl_model<T>::get_or_init_model_unit_bias()
{
    if(NULL == muBias)
    {
        mtx_bias.lock();
        if(NULL == muBias)
        {
            muBias = ftrl_model_unit<T>::create_instance(0, init_mean, init_stdev);
        }
        mtx_bias.unlock();
    }
    return muBias;
}


template<typename T>
double ftrl_model<T>::predict(const vector<pair<string, double> >& x, double bias, vector<ftrl_model_unit<T>*>& theta, vector<double>& sum)
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
            d = theta[i]->vi(f) * x[i].second;
            sum[f] += d;
            sum_sqr += d * d;
        }
        result += 0.5 * (sum[f] * sum[f] - sum_sqr);
    }
    return result;
}


template<typename T>
char* ftrl_model<T>::create_fea_c_str(const char* key)
{
    size_t len = strlen(key);
    char* p = (char*)mem_pool::get_mem(len + 1);
    strncpy(p, key, len);
    p[len] = 0;
    return p;
}


template<typename T>
bool ftrl_model<T>::output_model(const string& modelPath, const string& modelFormat)
{
    if("txt" == modelFormat) return output_txt_model(modelPath);
    else if("bin" == modelFormat) return output_bin_model(modelPath);
    else return false;
}


template<typename T>
bool ftrl_model<T>::output_txt_model(const string& modelPath)
{
    ofstream out(modelPath, ofstream::out);
    if(!out) return false;
    out << biasFeaName << " " << muBias->wi << " " << muBias->w_ni << " " << muBias->w_zi << endl;
    for(auto iter = muMap.begin(); iter != muMap.end(); ++iter)
    {
        out << iter->first << " " << iter->second << endl;
    }
    out.close();
    return true;
}


template<typename T>
bool ftrl_model<T>::output_bin_model(const string& modelPath)
{
    model_bin_file mbf;
    if(!mbf.open_file_for_write(modelPath, sizeof(T), factor_num, get_unit_mem_size())) return false;
    if(!mbf.write_one_fea_unit(get_bias_fea_name().c_str(), muBias, true)) return false;// write bias
    for(auto iter = muMap.begin(); iter != muMap.end(); ++iter)
    {
        if(!mbf.write_one_fea_unit(iter->first, &(iter->second), iter->second.is_none_zero())) return false;
    }
    if(!mbf.close_file()) return false;
    return true;
}


template<typename T>
void ftrl_model<T>::output_model_one_line(ostream& out, const char* feaName, ftrl_model_unit<T>* pMu, bool isBias)
{
    if(isBias)
    {
        out << feaName << " " << pMu->wi << " " << pMu->w_ni << " " << pMu->w_zi << endl;
    }
    else
    {
        out << feaName << " " << *pMu << endl;
    }
}


template<typename T>
bool ftrl_model<T>::load_model(const string& modelPath, const string& modelFormat)
{
    if("txt" == modelFormat) return load_txt_model(modelPath);
    else if("bin" == modelFormat) return load_bin_model(modelPath);
    else return false;
}


template<typename T>
bool ftrl_model<T>::load_txt_model(const string& modelPath)
{
    ifstream in(modelPath);
    if(!in) return false;
    bool res = load_txt_model(in);
    in.close();
    return res;
}


template<typename T>
bool ftrl_model<T>::load_txt_model(ifstream& in)
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
    muBias = ftrl_model_unit<T>::create_instance(0, strVec);
    while(getline(in, line))
    {
        strVec.clear();
        utils::split_string(line, ' ', &strVec);
        if(strVec.size() != 3 * factor_num + 4)
        {
            return false;
        }
        string& index = strVec[0];
        char* pKey = create_fea_c_str(index.c_str());
        muMap[pKey].instance_init(factor_num, strVec);
    }
    return true;
}


template<typename T>
bool ftrl_model<T>::load_bin_model(const string& modelPath)
{
    model_bin_file mbf;
    if(!mbf.open_file_for_read(modelPath)) return false;
    if(mbf.get_num_byte_len() != sizeof(T)) return false;
    if(mbf.get_factor_num() != (size_t)factor_num) return false;
    if(mbf.get_unit_len() != get_unit_mem_size()) return false;
    char* buffer = new char[64*1024];
    unsigned short feaLen;
    if(!mbf.read_one_fea(buffer, feaLen)) return false;
    buffer[feaLen] = 0;
    if(get_bias_fea_name() != string(buffer)) return false;
    muBias = ftrl_model_unit<T>::create_instance(0, 0.0, 0.0);
    if(!mbf.read_one_unit(muBias)) return false;
    for(size_t i = 1; i < mbf.get_fea_num(); ++i)
    {
        if(!mbf.read_one_fea(buffer, feaLen)) return false;
        buffer[feaLen] = 0;
        char* pKey = create_fea_c_str(buffer);
        if(!mbf.read_one_unit(&(muMap[pKey]))) return false;
    }
    if(!mbf.close_file()) return false;
    delete[] buffer;
    return true;
}


template<typename T>
const string& ftrl_model<T>::get_bias_fea_name()
{
    return biasFeaName;
}


template<typename T>
inline bool ftrl_model<T>::convert_one_line_of_txt_model_to_vec(ifstream& in, vector<string>& strVec,  bool& dataFmtErr, bool isBias)
{
    static string line;
    dataFmtErr = false;
    if(!getline(in, line))
    {
        return false;
    }
    int fn = isBias ? 0 : factor_num;
    strVec.clear();
    utils::split_string(line, ' ', &strVec);
    if(strVec.size() != 3 * fn + 4)
    {
        dataFmtErr = true;
        return false;
    }
    return true;
}


template<typename T>
size_t ftrl_model<T>::get_unit_mem_size()
{
    return ftrl_model_unit<T>::get_mem_size();
}



#endif /*FTRL_MODEL_H_*/
