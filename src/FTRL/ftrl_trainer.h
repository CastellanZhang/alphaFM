#ifndef FTRL_TRAINER_H_
#define FTRL_TRAINER_H_

#include "../Frame/pc_frame.h"
#include "ftrl_model.h"
#include "../Sample/fm_sample.h"
#include "../Utils/utils.h"
#include "../Lock/lock_pool.h"


struct trainer_option
{
    trainer_option() : k0(true), k1(true), factor_num(8), init_mean(0.0), init_stdev(0.1), w_alpha(0.05), w_beta(1.0), w_l1(0.1), w_l2(5.0),
               v_alpha(0.05), v_beta(1.0), v_l1(0.1), v_l2(5.0), model_format("txt"), initial_model_format("txt"),
               threads_num(1), b_init(false), force_v_sparse(false) {}
    string model_path, model_format, init_model_path, initial_model_format, model_number_type;
    double init_mean, init_stdev;
    double w_alpha, w_beta, w_l1, w_l2;
    double v_alpha, v_beta, v_l1, v_l2;
    int threads_num, factor_num;
    bool k0, k1, b_init, force_v_sparse;
    
    void parse_option(const vector<string>& args)
    {
        int argc = args.size();
        if(0 == argc) throw invalid_argument("invalid command\n");
        for(int i = 0; i < argc; ++i)
        {
            if(args[i].compare("-m") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                model_path = args[++i];
            }
            else if(args[i].compare("-mf") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                model_format = args[++i];
                if("bin" != model_format && "txt" != model_format)
                    throw invalid_argument("invalid command\n");
            }
            else if(args[i].compare("-dim") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                vector<string> strVec;
                string tmpStr = args[++i];
                utils::split_string(tmpStr, ',', &strVec);
                if(strVec.size() != 3)
                    throw invalid_argument("invalid command\n");
                k0 = 0 == stoi(strVec[0]) ? false : true;
                k1 = 0 == stoi(strVec[1]) ? false : true;
                factor_num = stoi(strVec[2]);
            }
            else if(args[i].compare("-init_stdev") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                init_stdev = stod(args[++i]);
            }
            else if(args[i].compare("-w_alpha") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                w_alpha = stod(args[++i]);
            }
            else if(args[i].compare("-w_beta") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                w_beta = stod(args[++i]);
            }
            else if(args[i].compare("-w_l1") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                w_l1 = stod(args[++i]);
            }
            else if(args[i].compare("-w_l2") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                w_l2 = stod(args[++i]);
            }
            else if(args[i].compare("-v_alpha") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                v_alpha = stod(args[++i]);
            }
            else if(args[i].compare("-v_beta") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                v_beta = stod(args[++i]);
            }
            else if(args[i].compare("-v_l1") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                v_l1 = stod(args[++i]);
            }
            else if(args[i].compare("-v_l2") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                v_l2 = stod(args[++i]);
            }
            else if(args[i].compare("-core") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                threads_num = stoi(args[++i]);
            }
            else if(args[i].compare("-im") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                init_model_path = args[++i];
                b_init = true; //if im field exits, that means b_init = true
            }
            else if(args[i].compare("-imf") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                initial_model_format = args[++i];
                if("bin" != initial_model_format && "txt" != initial_model_format)
                    throw invalid_argument("invalid command\n");
            }
            else if(args[i].compare("-fvs") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                int fvs = stoi(args[++i]);
                force_v_sparse = (1 == fvs) ? true : false;
            }
            else if(args[i].compare("-mnt") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                model_number_type = args[++i];
            }
            else
            {
                throw invalid_argument("invalid command\n");
                break;
            }
        }
    }
};


template<typename T>
class ftrl_trainer : public pc_task
{
public:
    ftrl_trainer(const trainer_option& opt);
    virtual void run_task(vector<string>& dataBuffer);
    bool load_model(const string& modelPath, const string& modelFormat);
    bool output_model(const string& modelPath, const string& modelFormat);
    
private:
    void train(int y, const vector<pair<string, double> >& x);
    
private:
    ftrl_model<T>* pModel;
    lock_pool* pLockPool;
    double w_alpha, w_beta, w_l1, w_l2;
    double v_alpha, v_beta, v_l1, v_l2;
    bool k0;
    bool k1;
    bool force_v_sparse;
};


template<typename T>
ftrl_trainer<T>::ftrl_trainer(const trainer_option& opt)
{
    w_alpha = opt.w_alpha;
    w_beta = opt.w_beta;
    w_l1 = opt.w_l1;
    w_l2 = opt.w_l2;
    v_alpha = opt.v_alpha;
    v_beta = opt.v_beta;
    v_l1 = opt.v_l1;
    v_l2 = opt.v_l2;
    k0 = opt.k0;
    k1 = opt.k1;
    force_v_sparse = opt.force_v_sparse;
    pModel = new ftrl_model<T>(opt.factor_num, opt.init_mean, opt.init_stdev);
    pLockPool = new lock_pool();
}


template<typename T>
void ftrl_trainer<T>::run_task(vector<string>& dataBuffer)
{
    for(size_t i = 0; i < dataBuffer.size(); ++i)
    {
        fm_sample sample(dataBuffer[i]);
        train(sample.y, sample.x);
    }
}


template<typename T>
bool ftrl_trainer<T>::load_model(const string& modelPath, const string& modelFormat)
{
    return pModel->load_model(modelPath, modelFormat);
}


template<typename T>
bool ftrl_trainer<T>::output_model(const string& modelPath, const string& modelFormat)
{
    return pModel->output_model(modelPath, modelFormat);
}


//输入一个样本，更新参数
template<typename T>
void ftrl_trainer<T>::train(int y, const vector<pair<string, double> >& x)
{
    ftrl_model_unit<T>* thetaBias = pModel->get_or_init_model_unit_bias();
    vector<ftrl_model_unit<T>*> theta(x.size(), NULL);
    int xLen = x.size();
    vector<mutex*> feaLocks(xLen + 1, NULL);
    for(int i = 0; i < xLen; ++i)
    {
        const string& index = x[i].first;
        theta[i] = pModel->get_or_init_model_unit(index);
        feaLocks[i] = pLockPool->get_feature_lock(index);
    }
    feaLocks[xLen] = pLockPool->get_bias_lock();
    //update w via FTRL
    for(int i = 0; i <= xLen; ++i)
    {
        ftrl_model_unit<T>& mu = i < xLen ? *(theta[i]) : *thetaBias;
        if((i < xLen && k1) || (i == xLen && k0))
        {
            feaLocks[i]->lock();
            if(fabs(mu.w_zi) <= w_l1)
            {
                mu.wi = 0.0;
            }
            else
            {
                if(force_v_sparse && mu.w_ni > 0 && 0.0 == mu.wi)
                {
                    mu.reinit_vi(pModel->init_mean, pModel->init_stdev);
                }
                mu.wi = (-1) *
                    (1 / (w_l2 + (w_beta + sqrt(mu.w_ni)) / w_alpha)) *
                    (mu.w_zi - utils::sgn(mu.w_zi) * w_l1);
            }
            feaLocks[i]->unlock();
        }
    }
    //update v via FTRL
    for(int i = 0; i < xLen; ++i)
    {
        ftrl_model_unit<T>& mu = *(theta[i]);
        for(int f = 0; f < pModel->factor_num; ++f)
        {
            feaLocks[i]->lock();
            T& vif = mu.vi(f);
            T& v_nif = mu.v_ni(f);
            T& v_zif = mu.v_zi(f);
            if(v_nif > 0)
            {
                if(force_v_sparse && 0.0 == mu.wi)
                {
                    vif = 0.0;
                }
                else if(fabs(v_zif) <= v_l1)
                {
                    vif = 0.0;
                }
                else
                {
                    vif = (-1) *
                        (1 / (v_l2 + (v_beta + sqrt(v_nif)) / v_alpha)) *
                        (v_zif - utils::sgn(v_zif) * v_l1);
                }
            }
            feaLocks[i]->unlock();
        }
    }
    vector<double> sum(pModel->factor_num);
    double bias = thetaBias->wi;
    double p = pModel->predict(x, bias, theta, sum);
    double mult = y * (1 / (1 + exp(-p * y)) - 1);
    //update w_n, w_z
    for(int i = 0; i <= xLen; ++i)
    {
        ftrl_model_unit<T>& mu = i < xLen ? *(theta[i]) : *thetaBias;
        double xi = i < xLen ? x[i].second : 1.0;
        if((i < xLen && k1) || (i == xLen && k0))
        {
            feaLocks[i]->lock();
            double w_gi = mult * xi;
            double w_si = 1 / w_alpha * (sqrt(mu.w_ni + w_gi * w_gi) - sqrt(mu.w_ni));
            mu.w_zi += w_gi - w_si * mu.wi;
            mu.w_ni += w_gi * w_gi;
            feaLocks[i]->unlock();
        }
    }
    //update v_n, v_z
    for(int i = 0; i < xLen; ++i)
    {
        ftrl_model_unit<T>& mu = *(theta[i]);
        const double& xi = x[i].second;
        for(int f = 0; f < pModel->factor_num; ++f)
        {
            feaLocks[i]->lock();
            T& vif = mu.vi(f);
            T& v_nif = mu.v_ni(f);
            T& v_zif = mu.v_zi(f);
            double v_gif = mult * (sum[f] * xi - vif * xi * xi);
            double v_sif = 1 / v_alpha * (sqrt(v_nif + v_gif * v_gif) - sqrt(v_nif));
            v_zif += v_gif - v_sif * vif;
            v_nif += v_gif * v_gif;
            //有的特征在整个训练集中只出现一次，这里还需要对vif做一次处理
            if(force_v_sparse && v_nif > 0 && 0.0 == mu.wi)
            {
                vif = 0.0;
            }
            feaLocks[i]->unlock();
        }
    }
}


#endif /*FTRL_TRAINER_H_*/
