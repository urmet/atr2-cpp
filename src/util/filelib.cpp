#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>

#include "filelib.h"

using std::cout;
using std::endl;

bool exist ( std::string thisfile )
{
    std::ifstream ifile ( thisfile.c_str() );
    return ifile;

}

bool valid ( std::string thisfile )
{
    cout << "STUB: " << __FUNCTION__ << endl;
    cout << thisfile << endl;
    return true;
}

std::string name_form ( std::string /*name*/ )
{
    cout << "STUB: " << __FUNCTION__ << endl;
    return "";
}

std::string exten ( std::string /*name*/ )
{
    cout << "STUB: " << __FUNCTION__ << endl;
    return "";
}

std::string base_name ( std::string name )
{
    int k;
    k=name.rfind ( '.' );
    return name.substr ( 0,k );
}

std::string attribs ( char /*b*/ )
{
    cout << "STUB: " << __FUNCTION__ << endl;
    return "";
}

std::string path ( std::string /*fn*/ )
{
    cout << "STUB: " << __FUNCTION__ << endl;
    return "";
}

std::string no_path ( std::string fn )
{
    if ( fn.rfind ( '/' ) ==std::string::npos ) {
        return fn;
    }
    return fn.substr ( fn.rfind ( '/' ) );
}

long file_length ( std::string /*fn*/ )
{
    cout << "STUB: " << __FUNCTION__ << endl;
    return 0;
}

void rewrite ( std::string fn )
{
    unlink(fn.c_str());
}
