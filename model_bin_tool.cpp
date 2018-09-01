#include <iostream>
#include "src/FTRL/ftrl_model.h"
#include "src/FTRL/model_bin_file.h"

using namespace std;


struct bin_tool_option
{
    bin_tool_option() : task(-1), factor_num(-1), model_number_type("double") {}
    string input_model_path, output_model_path, model_number_type;
    int task, factor_num;
    
    void parse_option(const vector<string>& args)
    {
        int argc = args.size();
        if(0 == argc)
            throw invalid_argument("invalid command\n");
        for(int i = 0; i < argc; ++i)
        {
            if(args[i].compare("-task") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                task = stoi(args[++i]);
                if(task < 1 || task > 4)
                    throw invalid_argument("invalid command\n");
            }
            else if(args[i].compare("-dim") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                factor_num = stoi(args[++i]);
                if(factor_num < 0)
                    throw invalid_argument("invalid command\n");
            }
            else if(args[i].compare("-im") == 0) 
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                input_model_path = args[++i];
            }
            else if(args[i].compare("-om") == 0) 
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                output_model_path = args[++i];
            }
            else if(args[i].compare("-mnt") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                model_number_type = args[++i];
                if("double" != model_number_type && "float" != model_number_type)
                    throw invalid_argument("invalid command\n");
            }
            else
            {
                throw invalid_argument("invalid command\n");
                break;
            }
        }
    }
};

string bin_tool_help()
{
    return string(
            "\nusage: ./model_bin_tool [<options>]"
            "\n"
            "\n"
            "options:\n"
            "-task <task_type>: 1-print bin model info\n"
            "                   2-transfer format, bin to txt\n"
            "                   3-transfer format, bin to txt, only nonzero features\n"
            "                   4-transfer format, txt to bin\n"
            "-im <input_model_path>: set the intput model path\n"
            "-om <output_model_path>: set the output model path for task 2,3 and 4, otherwise, it writes to standard output for task 2 and 3\n"
            "-dim <factor_num>: dim of 2-way interactions, for task 4\n"
            "-mnt <model_number_type>: set the number type of the bin model for task 4, double or float\tdefault:double\n"
    );
}


int print_info(const bin_tool_option& opt)
{
    model_bin_file mbf;
    if(!mbf.read_info(opt.input_model_path))
    {
        cerr << "read input file error" << endl;
        return 1;
    }
    mbf.print_info();
    return 0;
}


template<typename T>
bool bin_to_txt(int factorNum, const model_bin_file& mbf, ostream& outFile, bool isFullFormat = true)
{
    ftrl_model<T>* pModel = new ftrl_model<T>(factorNum, 0, 0);
    if(mbf.get_unit_len() != pModel->get_unit_mem_size())
    {
        cerr << "input file data error" << endl;
        return false;
    }
    char* buffer = new char[64*1024];
    unsigned short feaLen;
    if(!mbf.read_one_fea(buffer, feaLen))
    {
        cerr << "read input file error" << endl;
        return false;
    }
    buffer[feaLen] = 0;
    if(ftrl_model<T>::get_bias_fea_name() != string(buffer))
    {
        cerr << "error: bias feature not found" << endl;
        return false;
    }
    ftrl_model_unit<T>* pMu = ftrl_model_unit<T>::create_instance(0, 0.0, 0.0);
    if(!mbf.read_one_unit(pMu))
    {
        cerr << "read input file error" << endl;
        return false;
    }
    pModel->output_model_one_line(outFile, buffer, pMu, true);
    for(size_t i = 1; i < mbf.get_fea_num(); ++i)
    {
        if(!mbf.read_one_fea(buffer, feaLen))
        {
            cerr << "read input file error" << endl;
            return false;
        }
        buffer[feaLen] = 0;
        if(!mbf.read_one_unit(pMu))
        {
            cerr << "read input file error" << endl;
            return false;
        }
        if(isFullFormat || pMu->is_none_zero())
        {
            pModel->output_model_one_line(outFile, buffer, pMu);
        }
    }
    return true;
}


int transfer_bin_to_txt(const bin_tool_option& opt, bool isFullFormat = true)
{
    model_bin_file mbf;
    if(!mbf.open_file_for_read(opt.input_model_path))
    {
        cerr << "open input file error" << endl;
        return 1;
    }
    ostream* pOut = &cout;
    if(!opt.output_model_path.empty())
    {
        pOut = new ofstream(opt.output_model_path);
        if(NULL == pOut || !(*pOut))
        {
            cerr << "open output file error" << endl;
            return 1;
        }
    }
    if(mbf.get_num_byte_len() == sizeof(double))
    {
        if(!bin_to_txt<double>(mbf.get_factor_num(), mbf, *pOut, isFullFormat))
        {
            return 1;
        }
    }
    else if(mbf.get_num_byte_len() == sizeof(float))
    {
        if(!bin_to_txt<float>(mbf.get_factor_num(), mbf, *pOut, isFullFormat))
        {
            return 1;
        }
    }
    else
    {
        cerr << "read input file error" << endl;
        return 1;
    }
    if(!opt.output_model_path.empty())
    {
        ((ofstream*)pOut)->close();
        delete pOut;
    }
    if(!mbf.close_file())
    {
        cerr << "close input file error" << endl;
        return 1;
    }
    return 0;
}


template<typename T>
bool txt_to_bin(const bin_tool_option& opt, ifstream& inFile)
{
    model_bin_file mbf;
    ftrl_model<T>* pModel = new ftrl_model<T>(opt.factor_num, 0, 0);
    if(!mbf.open_file_for_write(opt.output_model_path, sizeof(T), opt.factor_num, pModel->get_unit_mem_size()))
    {
        cerr << "open output file error" << endl;
        return 1;
    }
    
    vector<string> strVec;
    bool dataFmtErr;
    if(!pModel->convert_one_line_of_txt_model_to_vec(inFile, strVec, dataFmtErr, true))
    {
        cerr << "read file error" << endl;
        return 1;
    }
    ftrl_model_unit<T>* pMu = ftrl_model_unit<T>::create_instance(0, strVec);

    if(!mbf.write_one_fea_unit(ftrl_model<T>::get_bias_fea_name().c_str(), pMu, true))// write bias
    {
        cerr << "write file error" << endl;
        return 1;
    }
    
    while(pModel->convert_one_line_of_txt_model_to_vec(inFile, strVec, dataFmtErr))
    {
        string& index = strVec[0];
        pMu->instance_init(opt.factor_num, strVec);
        if(!mbf.write_one_fea_unit(index.c_str(), pMu, pMu->is_none_zero()))
        {
            cerr << "write file error" << endl;
            return 1;
        }
    }
    if(dataFmtErr)
    {
        cerr << "input file format error" << endl;
        return 1;
    }
    if(!mbf.close_file())
    {
        cerr << "close output file error" << endl;
        return 1;
    }
    return 0;
}


int transfer_txt_to_bin(const bin_tool_option& opt)
{
    ifstream inFile(opt.input_model_path);
    if(!inFile)
    {
        cerr << "open input file error" << endl;
        return 1;
    }
    if("double" == opt.model_number_type)
    {
        if(!txt_to_bin<double>(opt, inFile)) return 1;
    }
    else if("float" == opt.model_number_type)
    {
        if(!txt_to_bin<float>(opt, inFile)) return 1;
    }
    else
    {
        cerr << "invalid argument: -mnt " << opt.model_number_type << endl;
        return 1;
    }
    inFile.close();
    return 0;
}


int main(int argc, char* argv[])
{
    static_assert(sizeof(void *) == 8, "only 64-bit code generation is supported.");
    cin.sync_with_stdio(false);
    cout.sync_with_stdio(false);
    bin_tool_option opt;
    try
    {
        opt.parse_option(utils::argv_to_args(argc, argv));
    }
    catch(const invalid_argument& e)
    {
        cerr << "invalid_argument:" << e.what() << endl;
        cerr << bin_tool_help() << endl;
        return 1;
    }
    if(1 == opt.task)
    {
        return print_info(opt);
    }
    else if(2 == opt.task)
    {
        return transfer_bin_to_txt(opt, true);
    }
    else if(3 == opt.task)
    {
        return transfer_bin_to_txt(opt, false);
    }
    else if(4 == opt.task)
    {
        if(opt.factor_num < 0)
        {
            cerr << "invalid argument: -dim" << endl;
            cerr << bin_tool_help() << endl;
            return 1;
        }
        return transfer_txt_to_bin(opt);
    }
    else
    {
        cerr << "invalid argument: -task" << endl;
        cerr << bin_tool_help() << endl;
        return 1;
    }
}

