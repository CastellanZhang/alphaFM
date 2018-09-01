#include <iostream>
#include <map>
#include <fstream>
#include "src/Frame/pc_frame.h"
#include "src/FTRL/ftrl_trainer.h"

using namespace std;

string train_help()
{
    return string(
            "\nusage: cat sample | ./fm_train [<options>]"
            "\n"
            "\n"
            "options:\n"
            "-m <model_path>: set the output model path\n"
            "-mf <model_format>: set the output model format, txt or bin\tdefault:txt\n"
            "-dim <k0,k1,k2>: k0=use bias, k1=use 1-way interactions, k2=dim of 2-way interactions\tdefault:1,1,8\n"
            "-init_stdev <stdev>: stdev for initialization of 2-way factors\tdefault:0.1\n"
            "-w_alpha <w_alpha>: w is updated via FTRL, alpha is one of the learning rate parameters\tdefault:0.05\n"
            "-w_beta <w_beta>: w is updated via FTRL, beta is one of the learning rate parameters\tdefault:1.0\n"
            "-w_l1 <w_L1_reg>: L1 regularization parameter of w\tdefault:0.1\n"
            "-w_l2 <w_L2_reg>: L2 regularization parameter of w\tdefault:5.0\n"
            "-v_alpha <v_alpha>: v is updated via FTRL, alpha is one of the learning rate parameters\tdefault:0.05\n"
            "-v_beta <v_beta>: v is updated via FTRL, beta is one of the learning rate parameters\tdefault:1.0\n"
            "-v_l1 <v_L1_reg>: L1 regularization parameter of v\tdefault:0.1\n"
            "-v_l2 <v_L2_reg>: L2 regularization parameter of v\tdefault:5.0\n"
            "-core <threads_num>: set the number of threads\tdefault:1\n"
            "-im <initial_model_path>: set the initial model path\n"
            "-imf <initial_model_format>: set the initial model format, txt or bin\tdefault:txt\n"
            "-fvs <force_v_sparse>: if fvs is 1, set vi = 0 whenever wi = 0\tdefault:0\n"
            "-mnt <model_number_type>: double or float\tdefault:double\n"
    );
}


template<typename T>
int train(const trainer_option& opt)
{
    ftrl_trainer<T> trainer(opt);

    if(opt.b_init)
    {
        cout << "load model..." << endl;
        if(!trainer.load_model(opt.init_model_path, opt.initial_model_format))
        {
            cerr << "failed to load model" << endl;
            return 1;
        }
        cout << "model loading finished" << endl;
    }

    pc_frame frame;
    frame.init(trainer, opt.threads_num);
    frame.run();

    cout << "output model..." << endl;
    if(!trainer.output_model(opt.model_path, opt.model_format))
    {
        cerr << "failed to output model" << endl;
        return 1;
    }
    cout << "model outputting finished" << endl;
    return 0;
}


int main(int argc, char* argv[])
{
    static_assert(sizeof(void *) == 8, "only 64-bit code generation is supported.");
    cin.sync_with_stdio(false);
    cout.sync_with_stdio(false);
    srand(time(NULL));
    trainer_option opt;
    try
    {
        opt.parse_option(utils::argv_to_args(argc, argv));
    }
    catch(const invalid_argument& e)
    {
        cerr << "invalid_argument:" << e.what() << endl;
        cerr << train_help() << endl;
        return 1;
    }
    if("float" == opt.model_number_type)
    {
        return train<float>(opt);
    }
    return train<double>(opt);
}

