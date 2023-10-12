//==============================================================================
//
//  hexembed - a simple utility to help embed files in C programs
//  
//============================================================================
//  Copyright (C) Guilaume Plante 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================



#include <windows.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>

using namespace std;

std::string base_name(std::string const & path)
{
  return path.substr(path.find_last_of("/\\") + 1);
}

inline std::string string_base_name(std::string const & path)
{
  return base_name(path);
}

typedef std::string::value_type char_t;

char_t up_char(char_t ch)
{
    return std::use_facet< std::ctype< char_t > >(std::locale()).toupper(ch);
}

std::string toupper(const std::string& src)
{
    std::string result;
    std::transform(src.begin(), src.end(), std::back_inserter(result), up_char);
    return result;
}

int main(int argc, char *argv[]) {

    HANDLE hCon = GetStdHandle( STD_OUTPUT_HANDLE );   // link to console to change colors
    if (argc != 2) {
        SetConsoleTextAttribute( hCon, 4 );
        cout << "Error: file name required." << endl;
        SetConsoleTextAttribute( hCon, 7 );
        return 1;
    }

    char fullFilename[MAX_PATH];
    const char *fname = argv[1];

    WIN32_FIND_DATA FindFileData;
    HANDLE handle = FindFirstFile(fname, &FindFileData) ;
    int found = handle != INVALID_HANDLE_VALUE;
    if(!found) 
    {
        SetConsoleTextAttribute( hCon, 4 );
        fprintf(stderr, "Cannot find file: %s.\n", fname);
        SetConsoleTextAttribute( hCon, 7 );
        return 2;
    }

    GetFullPathName(FindFileData.cFileName, MAX_PATH, fullFilename, nullptr);
    
    FindClose(handle);

    filesystem::path p(fullFilename);
    string sBasename = p.stem().string();
    string sBasenameUpper = toupper(sBasename);
    string sFilename = string_base_name(p.stem().string());
    
    

    FILE *fp = fopen(fname, "rb");
    if (!fp) {
        fprintf(stderr, "Error opening file: %s.\n", fname);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    const int fsize = ftell(fp);

    fseek(fp, 0, SEEK_SET);
    unsigned char *b = (unsigned char*)malloc(fsize);

    fread(b, fsize, 1, fp);
    fclose(fp);


    printf("/* Embedded file: %s */ \n", fullFilename);
    printf("const unsigned int %s_RAWDATA_SIZE = %d;\n", sBasenameUpper.c_str(), fsize);
    //printf("const int fsize = %d;\n", fsize);
    printf("const unsigned char %s_RAWDATA[%s_RAWDATA_SIZE] = {\n\t", sBasenameUpper.c_str(), sBasenameUpper.c_str());

    int i;
    for (i = 0; i < fsize; ++i) {
        printf("0x%02x%s",
                b[i],
                i == fsize-1 ? "" : ((i+1) % 16 == 0 ? ",\n\t" : ", "));
    }

    printf("\n};\n");

    free(b);
    return 0;
}
