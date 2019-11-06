#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<string>

#include "wavelet_tree.h"


int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        puts("Invalid command.");
        exit(1);
    }


    if(!strcmp(argv[1], "build"))
    {
        std::string inputFile(argv[2]);
        std::string outputFile(argv[3]);

        wavelet_tree(inputFile, outputFile);
    }
    else if(!strcmp(argv[1], "access"))
    {
        std::string wtFile(argv[2]);
        std::string indicesFile(argv[3]);

        wavelet_tree::access_queries(wtFile, indicesFile);
    }
    else if(!strcmp(argv[1], "rank"))
    {
        std::string wtFile(argv[2]);
        std::string queriesFile(argv[3]);

        wavelet_tree::rank_queries(wtFile, queriesFile);
    }
    else if(!strcmp(argv[1], "select"))
    {
        std::string wtFile(argv[2]);
        std::string queriesFile(argv[3]);

        wavelet_tree::select_queries(wtFile, queriesFile);
    }
    else
        puts("Invalid command.");
    

    return 0;
}