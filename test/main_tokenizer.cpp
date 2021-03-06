#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>
#include "tokenizer.h"
#include "fr_tokenizer.h"
#include "en_tokenizer.h"
#include "char_tokenizer.h"
#include "stemmer.h"
#include "stopwords.h"
#include "utils.h"
#include "generalization.h"


using namespace std;
using namespace qnlp;


bool l_aggressive=false;
bool l_no_punct=false;
bool l_underscore=false;
bool l_cased=false;
bool l_dash=false;
bool l_stem=false;
bool l_generalize=false;
bool l_stopwords=false;
string l_lang="";
int l_threads=4;

void usage()
{
    cout << 
            "--stem (-s)              stem outputs (default false)\n"
            "--dash (-d)              split using dashes (default false)\n"
            "--lowercased (-c)        put the sequence in lowercase (default false)\n"
            "--underscore (-u)        split using underscores (default false)\n"
            "--stopwords (-w)         remove stopwords (default false)\n"
            "--aggressive (-a)        equivalent to --dash and --underscore and every separators\n"
            "--no-punct (-p)          remove punctuation from tokenization\n"
            "--generalize (-g)        remove numbers and replace them with a tag XNUMBER\n"
            "--lang <val> (-l)        set language (required)\n"
            "--help (-h)             Show this message\n";
    exit(1);
}

void ProcessArgs(int argc, char** argv)
{
    const char* const short_opts = "gspvwqdcual:e:t:h";
    const option long_opts[] = {
            {"stem", 0, nullptr, 's'},
            {"generalize", 0, nullptr, 'g'},
            {"dash", 0, nullptr, 'd'},
            {"lowercased", 0, nullptr, 'c'},
            {"underscore", 0, nullptr, 'u'},
            {"aggressive", 0, nullptr, 'a'},
            {"no-punct", 0, nullptr, 'p'},
            {"stopwords", 0, nullptr, 'w'},
            {"lang", 1, nullptr, 'l'},
            {"embmodel", 1, nullptr, 'e'},
            {"help", 0, nullptr, 'h'},
            {nullptr, 0, nullptr, 0}
    };

    while (true)
    {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (-1 == opt)
            break;

        switch (opt)
        {
        case 's':
            l_stem = true;
            break;

        case 'g':
            l_generalize = true;
            break;

        case 'd':
            l_dash = true;
            break;

        case 'c':
            l_cased = true;
            break;

        case 'u':
            l_underscore = true;
            break;

        case 'w':
            l_stopwords = true;
            break;

        case 'a':
            l_dash = true;
            l_underscore = true;
            l_aggressive = true;
            break;

        case 'p':
            l_no_punct = true;
            break;

        case 'l':
            l_lang = optarg;
            break;

        case 'h': // -h or --help
        case '?': // Unrecognized option
        default:
            usage();
            break;
        }
    }
    if (l_lang == "")
    {
        cerr << "Error, you must set a lang" <<endl;
        usage();
        exit(1);
    }
}



string stemming(string input , bool l_stemming)
{
    vector<string> line_splited;
    string tmp_output;
    if (l_stemming)
    {
        Stemmer s(l_lang.c_str());
        return s.stem_sentence(input);
    }
    return input;
}
vector<string> stemming(vector<string> &input , Stemmer& s)
{
        return s.stem_sentence_vector(input);
}

vector<string> filtering_stopwords(vector<string> &input , Stopwords & sw)
{
        return sw.filter_stopwords(input,l_lang);
}

vector<string> generalize(vector<string> &input , bool l_generalize)
{
    vector<string> tmp;
    if (l_generalize)
    {
        Generalization gen;
        tmp=gen.filter_numbers(input);
        return gen.filter_hours(tmp);
    }
    return input;
}

int main ( int argc, char *argv[] )
{
    ProcessArgs(argc, argv);
    string l_output;
    vector<string> l_output_vec;
    string l_token_pred;
    stringstream l_out;
    string line;
    Tokenizer* l_tokenizer;
    Stopwords sw;
    Stemmer stem(l_lang.c_str());
    if (l_lang.compare("fr") == 0) 
    {
        l_tokenizer = new Tokenizer_fr(l_cased,l_underscore,l_dash, l_aggressive);
    }
    else if (l_lang.compare("en") == 0) 
    {
        l_tokenizer = new Tokenizer_en(l_cased,l_underscore,l_dash, l_aggressive);
    }
    else if (l_lang.compare("char") == 0) 
    {
        l_tokenizer = new Tokenizer_char(l_cased,l_underscore,l_dash, l_aggressive);
    }
    else
    {
        l_tokenizer = new Tokenizer(l_cased,l_underscore,l_dash, l_aggressive);
    }
    l_tokenizer->setNoPunct(l_no_punct);
    while (std::getline(std::cin, line))
    {
        string to_tokenize=line;
        l_output_vec = l_tokenizer->tokenize(to_tokenize);
        if (l_stem)  
            l_output_vec = stemming(l_output_vec,stem);
        if (l_stopwords) 
            l_output_vec = filtering_stopwords(l_output_vec,sw);
        if (l_generalize) 
            l_output_vec  = generalize(l_output_vec,l_generalize);
        else
        {
            l_output=qnlp::Join(l_output_vec," ");      
        }
        cout << l_output << endl;
//         cout << "ORI: " << line << endl;
//         cout << "TOK: " << l_output << endl;
//         cout << "NTK: " << qnlp::Join(l_output_vec_ntk," ") << endl;
    }
    delete(l_tokenizer);
    return EXIT_SUCCESS;
}
