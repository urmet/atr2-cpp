/*
 * File:   filelib.h
 * Author: urmet
 *
 * Created on teisipäev, 1. november 2011. a, 21:11
 */

#ifndef FILELIB_H
#define	FILELIB_H

#include <string>

bool exist ( std::string thisfile );
bool valid ( std::string thisfile );
std::string name_form ( std::string name );
std::string exten ( std::string name );
std::string base_name ( std::string name );
std::string attribs ( char b );
std::string path ( std::string fn );
std::string no_path ( std::string fn );
long file_length ( std::string fn );
void rewrite ( std::string fn );

#endif	/* FILELIB_H */

