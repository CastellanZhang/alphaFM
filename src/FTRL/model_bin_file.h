#ifndef MODEL_BIN_FILE_H_
#define MODEL_BIN_FILE_H_

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

struct model_bin_info
{
    size_t num_byte_len = 0;
    size_t factor_num = 0;
    size_t fea_num = 0;
    size_t nonzero_fea_num = 0;
    size_t success_flag = 0;
    size_t unit_len = 0;
    friend inline ostream& operator <<(ostream& os, const model_bin_info& info)
    {
        os << "number_byte_length: " << info.num_byte_len;
        if(sizeof(double) == info.num_byte_len) os << "(double)";
        else if(sizeof(float) == info.num_byte_len) os << "(float)";
        os << endl;
        os << "factor_num: " << info.factor_num << endl;
        os << "feature_num: " << info.fea_num << endl;
        os << "nonzero_feature_num: " << info.nonzero_fea_num << endl;
        os << "success_flag: " << ((1 == info.success_flag) ? "true" : "false") << endl;
        return os;
    }
};


class model_bin_file
{
public:
    bool read_info(const string& fileName)
    {
        ifstream f_in(fileName, ifstream::binary);
        bool res = read_info(f_in);
        f_in.close();
        return res;
    }
    
    
    bool open_file_for_read(const string& fileName)
    {
        isRead = true;
        if(NULL != pInputFile) return false;
        pInputFile = new ifstream(fileName, ifstream::binary);
        if(NULL == pInputFile || !(*pInputFile)) return false;
        if(!read_info(*pInputFile)) return false;
        return true;
    }
    
    
    bool open_file_for_write(const string& fileName, size_t numByteLen, size_t factorNum, size_t unitLen)
    {
        isRead = false;        
        if(NULL != pOutputFile) return false;
        pOutputFile = new ofstream(fileName, ofstream::binary);
        if(NULL == pOutputFile) return false;
        info.num_byte_len = numByteLen;
        info.factor_num = factorNum;
        info.unit_len = unitLen;
        if(!pOutputFile->write((char*)&version, sizeof(version))) return false;
        if(!pOutputFile->write((char*)&info, sizeof(info))) return false;
        return true;
    }
    
    
    size_t get_num_byte_len() const
    {
        return info.num_byte_len;
    }
    
    
    size_t get_unit_len() const
    {
        return info.unit_len;
    }
    
    
    size_t get_fea_num() const
    {
        return info.fea_num;
    }
    
    
    size_t get_factor_num() const
    {
        return info.factor_num;
    }
    
    
    inline bool read_one_fea(char* pFeaMem, unsigned short& feaLen) const
    {
        if(!pInputFile->read((char*)&feaLen, sizeof(feaLen))) return false;
        if(!pInputFile->read(pFeaMem, feaLen)) return false;
        return true;
    }
    
    
    inline bool read_one_unit(void* pUnitMem) const
    {
        if(!pInputFile->read((char*)pUnitMem, info.unit_len)) return false;
        return true;
    }
    
    
    inline bool write_one_fea_unit(const char* pFeaMem, void* pUnitMem, bool isNoneZeroFea)
    {
        unsigned short feaLen = strlen(pFeaMem);
        if(!pOutputFile->write((char*)&feaLen, sizeof(feaLen))) return false;
        if(!pOutputFile->write(pFeaMem, feaLen)) return false;
        if(!pOutputFile->write((char*)pUnitMem, info.unit_len)) return false;
        info.fea_num++;
        if(isNoneZeroFea) info.nonzero_fea_num++;
        return true;
    }
    
    
    bool close_file()
    {
        if(isRead)//load model
        {
            if(NULL == pInputFile) return false;
            pInputFile->close();
            delete pInputFile;
            pInputFile = NULL;
        }
        else //output model
        {
            if(NULL == pOutputFile) return false;
            info.success_flag = 1;
            if(!pOutputFile->seekp(sizeof(version))) return false;
            if(!pOutputFile->write((char*)&info, sizeof(info))) return false;
            pOutputFile->close();
            delete pOutputFile;
            pOutputFile = NULL;
        }
        return true;
    }
    
    
    void print_info()
    {
        cout << "format_version: " << version << endl;
        cout << info << endl;
    }
    
private:
    bool read_info(ifstream& f_in)
    {
        if(!f_in) return false;
        size_t _version = 0;
        if(!f_in.read((char*)&_version, sizeof(_version))) return false;
        if(_version != version) return false;
        if(!f_in.read((char*)&info, sizeof(info))) return false;
        if(1 != info.success_flag) return false;
        return true;
    }
    
private:
    bool isRead;
    ifstream* pInputFile = NULL;
    ofstream* pOutputFile = NULL;
    const size_t version = 1;
    model_bin_info info;
};


#endif /*MODEL_BIN_FILE_H_*/
