#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <boost/ptr_container/ptr_vector.hpp>

/* robots */
static const unsigned int MAX_ROBOTS = 32;
static const unsigned int MAX_CODE = 1024;
static const unsigned int MAX_STACK = 256;
static const unsigned int MAX_RAM = 1024;
static const unsigned int MAX_VARS = 256;
static const unsigned int MAX_LABELS = 256;

#include "opcodes.h"

/*compiler variables*/
extern unsigned numvars, numlabels, maxcode, lock_pos, lock_dat;
extern std::string varname[MAX_VARS];
extern int varloc[MAX_VARS];
extern std::string labelname[MAX_VARS];
extern int labelnum[MAX_LABELS];
extern bool debugging_compiler, compile_by_line, show_code;
extern bool show_source, compile_only;
extern std::string lock_code;

class Robot;

#define MAX_VAR_LEN 16

void compile ( Robot& r, std::string filename );
void robot_config ( int n );

void print_code ( Robot& r, int p );

#endif
