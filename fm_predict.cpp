#include <iostream>
#include <map>
#include <fstream>
#include "src/Frame/pc_frame.h"
#include "src/FTRL/ftrl_predictor.h"

using namespace std;

struct Option 
{
    Option() : dim(8), threads_num(1) {}
    string model_path, predict_path;
    int threads_num, dim;
};

string predict_help() 
{
    return string(
            "\nusage: cat sample | ./fm_predict [<options>]"
            "\n"
            "\n"
            "options:\n"
            "-m <model_path>: set the model path\n"
            "-dim <factor_num>: dim of 2-way interactions\tdefault:8\n"
            "-core <threads_num>: set the number of threads\tdefault:1\n"
            "-out <predict_path>: set the predict path\n"
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

Option parse_option(const vector<string>& args)
{
    int argc = args.size();
    if(0 == argc) 
        throw invalid_argument("invalid command\n");
    Option opt;

    for(int i = 0; i < argc; ++i) 
    {
        if(args[i].compare("-m") == 0) 
        {
            if(i == argc - 1)
                throw invalid_argument("invalid command\n");
            opt.model_path = args[++i];
        }
        else if(args[i].compare("-dim") == 0)
        {
            if(i == argc - 1)
                throw invalid_argument("invalid command\n");
            opt.dim = stoi(args[++i]);
        }
        else if(args[i].compare("-core") == 0)
        {
            if(i == argc - 1)
                throw invalid_argument("invalid command\n");
            opt.threads_num = stoi(args[++i]);
        }
        else if(args[i].compare("-out") == 0)
        {
            if(i == argc - 1)
                throw invalid_argument("invalid command\n");
            opt.predict_path = args[++i];
        }
        else
        {
            break;
        }
    }
    return opt;
}


int main(int argc, char* argv[])
{
    cin.sync_with_stdio(false);
    cout.sync_with_stdio(false);
    Option opt;
    try
    {
        opt = parse_option(argv_to_args(argc, argv));
    }
    catch(const invalid_argument& e)
    {
        cout << e.what() << endl;
        cout << predict_help() << endl;
        return EXIT_FAILURE;
    }


    ifstream f_model(opt.model_path.c_str());
    ofstream f_predict(opt.predict_path.c_str(), ofstream::out);

    ftrl_predictor predictor(opt.dim, f_model, f_predict);

    pc_frame frame;
    frame.init(predictor, opt.threads_num);
    frame.run();

    f_model.close();
    f_predict.close();
    return 0;
}

