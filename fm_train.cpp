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
            "-im <initial_model_path>: set the initial value of model\n"
            "-fvs <force_v_sparse>: if fvs is 1, set vi = 0 whenever wi = 0\tdefault:0\n"
    );
}

vector<string> argv_to_args(int argc, char* argv[]) 
{
    vector<string> args;
    for(int i = 1; i < argc; ++i)
    {
        args.push_back(string(argv[i]));
    }
    return args;
}


int main(int argc, char* argv[])
{
    cin.sync_with_stdio(false);
    cout.sync_with_stdio(false);
    srand(time(NULL));
    trainer_option opt;
    try
    {
        opt.parse_option(argv_to_args(argc, argv));
    }
    catch(const invalid_argument& e)
    {
        cout << "invalid_argument:" << e.what() << endl;
        cout << train_help() << endl;
        return EXIT_FAILURE;
    }

    ftrl_trainer trainer(opt);

    if(opt.b_init) 
    {
        ifstream f_temp(opt.init_m_path.c_str());
        if(!trainer.loadModel(f_temp)) 
        {
            cout << "wrong model" << endl;
            return EXIT_FAILURE;
        }
        f_temp.close();
    }

    pc_frame frame;
    frame.init(trainer, opt.threads_num);
    frame.run();

    ofstream f_model(opt.model_path.c_str(), ofstream::out);
    trainer.outputModel(f_model);
    f_model.close();

    return 0;
}

