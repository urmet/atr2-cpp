/*
 * File:   atr2.h
 * Author: urmet
 *
 * Created on teisipäev, 1. november 2011. a, 19:44
 */

#ifndef ATR2_H
#define ATR2_H 1

#include <cstdio>
#include <string>

#include <boost/ptr_container/ptr_vector.hpp>

#include "opcodes.h"


extern const char*  main_filename  ;
extern const char*  robot_ext      ;
extern const char*  locked_ext     ;
extern const char*  config_ext     ;

extern bool insane_missiles;
extern int insanity;
extern int time_slice;
extern int kill_count;
extern long game_cycle;
extern long matches; // total matches
extern long played; // matches played

extern long game_limit;
extern unsigned int num_robots;


class Robot; // forward declaration
extern boost::ptr_vector<Robot> robot;

typedef boost::ptr_vector<Robot>::iterator robot_it;

extern bool testing;

/* simulator & graphics */
const double screen_scale   =0.46;
const int screen_x       =5;
const int screen_y       =5;
const int robot_scale    =06;
const int default_delay  =20;
const int default_slice  =05;
#define mine_circle (trunc(mine_blast*screen_scale)+1)
#define mis_radius (trunc(hit_range/2)+1)


void set_matches(const int m);


/* prototypess!! */
std::string operand ( int n, int m );
std::string mnemonic ( int n, int m );
void log_error ( Robot &r, int i, std::string ov );
void prog_error ( int n, std::string ss );

void create_robot ( int n, std::string filename );
void shutdown();

void parse_param ( std::string s ); // DONE

bool gameover();
bool invalid_microcode ( Robot &r, int ip );
void do_mine ( Robot& r, int );
std::string victor_string ( int k, int n );
void show_statistics();
void score_robots();
void init_bout();
void bout();
void write_report();

/**
 * advance game one cycle
 */
void do_game_cycle();

#endif  /* ATR2_H */

