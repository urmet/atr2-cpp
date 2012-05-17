#ifndef ROBOT_H
#define ROBOT_H

#include <fstream>
#include <map>

#include "missile.h"
#include "opcodes.h"
#include "compiler.h"

const int acceleration   =4;
const int turn_rate      =8;
const int max_vel        =4;
const int missile_spd    =32;
const int hit_range      =14;
const int blast_radius   =25;
const int crash_range    =8;
const int max_sonar      =250;
const int com_queue      =512;
const int max_queue      =255;
const int max_config_points=12;
#define MAX_MINES 64
const int mine_blast     =35;

struct op_rec{
    Opcode op;
    int arg1,arg2;
    int microcode;
};

typedef op_rec prog_type[MAX_CODE];

struct config_rec{
    int scanner,weapon,armor,engine,heatsinks,shield,mines;
};

typedef struct {
    float x,y;
    int detect, yield;
    bool detonate;
} mine_rec;

class Robot
{
    // functions that poke around robots' insides
    friend void compile ( Robot& r, std::string filename );
    friend void do_missiles();
    friend void do_mine ( Robot &ro , int mi );
    
public:
    Robot ( int n, std::string fn );
    void reset_software();
    void reset_hardware();
    void robot_config();
    unsigned n;
    std::string fn;
    mine_rec mine[MAX_MINES];
    int armor;
    bool won;
    unsigned int wins;
    unsigned int trials;
    unsigned int kills;
    unsigned int deaths;
    unsigned int match_shots;
    unsigned int shots_fired;

    prog_type code;
    /**
    * execute one timeslice
    */

    void execute();
    void damage ( int d, bool physical );
private:
    
    bool debug; // !!!!
    
int com_recieve ();    
void com_transmit ( int chan, int data );
    
    void init_mine (int, int );
    void shoot_missile ( float xx, float yy, int dir, bool ob );
    void log_error ( int i, std::string ov );
    /**
     * execute one instruction
     */
    void execute_instruction();

    /**
     * get value from code
     *
     * @param c instruction address
     * @param o index in instruction
     */
    int get_val ( const int c, const int o );

    /**
    * @param i ram address
    * @param j unused
    */
    int get_from_ram ( int i );

    void put_val ( int c, int o, int v );

    /**
     * scan for enemies
     *
     * @return distance to nearest enemy
     */
    int scan ();

    /**
     * @return value from stack
     */
    int pop ();

    /**
     * get the ip label points to
     * @param label label code
     * @param mc microcode
     */
    int find_label ( int label, int m );

    void push ( int v );
    /**
     *
     */
    void jump ( int o, bool& inc_ip );

    /**
     *
     */
    int in_port ( int p,unsigned &time_used );

    void out_port ( int p, int v, unsigned &time_used );
    void call_int ( int int_num, unsigned& time_used );

    /**
     *
     */
    void robot_error ( const int i, const std::string ov );

    unsigned plen; // program lenght
    unsigned ip;
    config_rec config;
    bool is_locked;
    std::string name;
    unsigned robot_time_limit;
    unsigned startkills;
    int mem_watch;
    unsigned error_count;
    std::ofstream errorlog;
    int16_t stack[MAX_STACK];
    int16_t ram[MAX_RAM];
    float x,y,xv,yv;
    float meters,speed,speedadj;
    int spd,tspd,hd,thd;

    /**
     * turret direction relative to the tank
     */
    int shift;

    float shotstrength,damageadj;
    int accuracy;
    unsigned damage_total,last_damage,hits,last_hit,cycles_lived,mines;
    unsigned larmor;
    int heat,lheat;
    bool shields_up,overburn,keepshift,cooling;
    int shutdown;
    unsigned delay_left,time_left,max_time;
    int transponder,channel;
    int scanarc;

    float lx,ly;
    int lhd;
    int err;
    int lshift,arc_count,sonar_count,scanrange;
    int lendarc,endarc,lstartarc,startarc;

    bool lshields;



protected:
    Robot ( const Robot * )  {}
    void operator= ( const Robot );
};

typedef Robot *robot_ptr;
typedef boost::ptr_vector<Robot>::iterator  Robot_it;

#endif // ROBOT_H
