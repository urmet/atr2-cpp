/*
 * File:   atr2.cpp
 * Author: urmet
 *
 * Created on teisipäev, 1. november 2011. a, 19:44
 */

#include "atr2func.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <ctime>

#include <boost/ptr_container/ptr_vector.hpp>


#include "atr2.h"

#include "filelib.h"
#include "compiler.h"
#include "missile.h"
#include "opcodes.h"
#include "robot.h"


using namespace std;
/* GLOBALS!! */

/* constants */
const char*  main_filename;
const char*  robot_ext      =".at2";
const char*  locked_ext     =".atl";
const char*  config_ext     =".ats";
const char*  compile_ext    =".cmp";
const char*  report_ext     =".rep";

/*robot variables*/
unsigned int num_robots;
boost::ptr_vector<Robot> robot;

bool testing;

bool insane_missiles;
int lice, insanity, update_timer, max_gx, max_gy, stats_mode,time_slice;
long game_limit, game_cycle;
long matches, played, executed;



/*{--general settings--}*/
int kill_count;

int getch()
{
    int r;
    unsigned char c;
    if ( ( r = read ( 0, &c, sizeof ( c ) ) ) < 0 ) {
        return r;
    } else {
        return c;
    }
}



std::string operand ( int n, int m )
{
    std::string s,s2;
    std::stringstream ss;

    /*
     Microcode:
     0 = instruction, number, constant
     1 = variable, memory access
     2 = :label
     3 = !label (unresolved)
     4 = !label (resolved)
     8h mask = inderect addressing (enclosed in [])
     */
    switch ( m & 7 ) {
    case 1:
        ss<<"@"<<n;
        break;
    case 2:
        ss<<":"<<n;
        break;
    case 3:
        ss<<"$"<<n;
        break;
    case 4:
        ss<<"!"<<n;
        break;
    default:
        ss<<n;
    }
    s2 = ss.str();
    s = s2;
    if ( m & 8 ) {
        s ="["+s2+"]";
    }
    return s;
}

std::string mnemonic ( int n, int m )
{
    std::string s;
    if ( 0 != m ) {
        return operand ( n, m );
    }
    switch ( n ) {
    case NOP:
        s = "NOP";
        break;
    case ADD:
        s = "ADD";
        break;
    case SUB:
        s = "SUB";
        break;
    case OR:
        s = "OR";
        break;
    case 4:
        s = "AND";
        break;
    case 5:
        s = "XOR";
        break;
    case 6:
        s = "NOT";
        break;
    case 7:
        s = "MPY";
        break;
    case 8:
        s = "DIV";
        break;
    case 9:
        s = "MOD";
        break;
    case 10:
        s = "RET";
        break;
    case 11:
        s = "CALL";
        break;
    case JMP:
        s = "JMP";
        break;
    case 13:
        s = "JLS";
        break;
    case 14:
        s = "JGR";
        break;
    case 15:
        s = "JNE";
        break;
    case 16:
        s = "JE";
        break;
    case 17:
        s = "SWAP";
        break;
    case 18:
        s = "DO";
        break;
    case 19:
        s = "LOOP";
        break;
    case 20:
        s = "CMP";
        break;
    case 21:
        s = "TEST";
        break;
    case 22:
        s = "MOV";
        break;
    case 23:
        s = "LOC";
        break;
    case 24:
        s = "GET";
        break;
    case 25:
        s = "PUT";
        break;
    case 26:
        s = "INT";
        break;
    case 27:
        s = "IPO";
        break;
    case 28:
        s = "OPO";
        break;
    case 29:
        s = "DELAY";
        break;
    case 30:
        s = "PUSH";
        break;
    case 31:
        s = "POP";
        break;
    case 32:
        s = "ERR";
        break;
    case 33:
        s = "INC";
        break;
    case 34:
        s = "DEC";
        break;
    case 35:
        s = "SHL";
        break;
    case 36:
        s = "SHR";
        break;
    case 37:
        s = "ROL";
        break;
    case 38:
        s = "ROR";
        break;
    case 39:
        s = "JZ";
        break;
    case 40:
        s = "JNZ";
        break;
    case 41:
        s = "JGE";
        break;
    case 42:
        s = "JLE";
        break;
    case 43:
        s = "SAL";
        break;
    case 44:
        s = "SAR";
        break;
    case 45:
        s = "NEG";
        break;
    case 46:
        s = "JLT";
        break;
    default:
        s = "XXX";
        break;
    }
    return s;
}

void prog_error ( int n, std::string ss )
{
    std::stringstream s;
    cout<<"Error #"<<n<<": ";
    switch ( n ) {
    case  0:
        s<<ss;
        break;
    case  1:
        s<<"Invalid :label - \""<<ss<<"\", silly mortal.";
        break;
    case  2:
        s<<"Undefined identifier - \""<<ss<<"\". A typo perhaps?";
        break;
    case  3:
        s<<"Memory access out of range - \""<<ss<<"\"";
        break;
    case  4:
        s<<"Not enough robots for combat. Maybe we should just drive in circles.";
        break;
    case  5:
        s<<"Robot names and settings must be specified. An empty arena is no fun.";
        break;
    case  6:
        s<<"Config file not found - \""<<ss<<"\"";
        break;
    case  7:
        s<<"Cannot access a config file from a config file - \""<<ss<<"\"";
        break;
    case  8:
        s<<"Robot not found \""<<ss<<"\". Perhaps you mistyped it?";
        break;
    case  9:
        s<<"Insufficient RAM to load robot: \""<<ss<<"\"... This is not good.";
        break;
    case 10:
        s<<"Too many robots! We can only handle "<<MAX_ROBOTS<<"! Blah.. limits are limits.";
        break;
    case 11:
        s<<"You already have a perfectly good #def for \""<<ss<<"\", silly.";
        break;
    case 12:
        s<<"Variable name too long! Boldly we simplify, simplify along..."<<ss;
        break;
    case 13:
        s<<"!Label already defined \""<<ss<<"\", silly.";
        break;
    case 14:
        s<<"Too many variables! (Var limit: "<<MAX_VARS<<")";
        break;
    case 15:
        s<<"Too many !labels (!label limit: "<<MAX_LABELS<<")";
        break;
    case 16:
        s<<"Robot program too long! Boldly we simplify, simplify along..."<<ss;
        break;
    case 17:
        s<<"!Label missing error. !Label #"<<ss<<".";
        break;
    case 18:
        s<<"!Label out of range: "<<ss;
        break;
    case 19:
        s<<"!Label not found. "<<ss;
        break;
    case 20:
        s<<"Invalid config option: \""<<ss<<"\". Inventing a new device?";
        break;
    case 21:
        s<<"Robot is attempting to cheat; Too many config points ("<<ss<<")";
        break;
    case 22:
        s<<"Insufficient data in data statement: \""<<ss<<"\"";
        break;
    case 23:
        s<<"Too many asterisks: \""<<ss<<"\"";
        break;
    case 24:
        s<<"Invalid step count: \""<<ss<<"\". 1-9 are valid confditions.";
        break;
    case 25:
        s<<"\""<<ss<<"\"";
        break;
    default:
        s<<ss;
    }

    cout << s.str() <<endl<<endl;
    abort();
}






void create_robot ( int n, std::string filename )
{
    if ( filename == base_name ( filename ) ) {
        if ( filename[0]=='?' ) {
            filename = filename.append ( locked_ext );
        } else {
            filename = filename.append ( robot_ext );
        }
    }
    if ( filename[0]=='?' ) {
        filename = rstr ( filename,filename.length()-1 );
    }
    Robot *r = new Robot ( n,filename );
    robot.push_back ( r );
//    compile ( r, filename );
//    r.robot_config();

    return;
}
bool gameover()
{
    if ( game_cycle>=game_limit && game_limit>0 ) {
        return true;
    }
    if ( ( game_cycle&31 ) ==0 ) {
        int k=0;
        for ( int n=0; n<num_robots; ++n )
            if ( robot[n].armor>0 ) {
                ++k;
            }
        if ( k<=1 ) {
            return true;
        }
    } else {
        return false;
    }
    return false;
}

bool invalid_microcode ( Robot& r, int ip )
{
    bool invalid = false;
    int i,k;
    for ( i=0; i<=1; ++i ) {
//         cout<<"checking invalid: "<<robot[n]->code[ip].op[MAX_OP]<<endl;
        k= ( r.code[ip].microcode>> ( i<<2 ) ) &7;
        if ( ! ( ( k==0 ) || ( k==1 ) || ( k==2 ) || ( k==4 ) ) ) {
            invalid=true;
        }
    }
    return invalid;
}


void do_mine ( Robot &ro , int mi )
{
    int k;
    double d;
    bool source_alive;
    mine_rec *m = & ( ro.mine[mi] );
    if ( ( m->x>=0 ) && ( m->x<=1000 ) && ( m->y>=0 ) && ( m->y<=1000 )
            && ( m->yield>0 ) ) {
        for ( boost::ptr_vector<Robot>::iterator r=robot.begin(),end=robot.end();
                r != end; ++r ) {
            if ( ( r->armor>0 ) && ( r->n != ro.n ) ) {
                d=distance ( m->x,m->y,r->x,r->y );
                if ( d<m->detect ) {
                    m->detonate=true;
                }
            }
        }// for
        if ( m->detonate ) { // boom!
//             ro.shoot_missile(m->x,m->y); // was used just for animation
            kill_count=0;
            if ( ro.armor>0 ) {
                source_alive=true;
            } else {
                source_alive = false;
            }
            for ( boost::ptr_vector<Robot>::iterator r=robot.begin(),end=robot.end();
                    r != end; ++r ) {
                if ( r->armor >0 ) {
                    k=lrint ( distance ( m->x,m->y,r->x,r->y ) );
                    if ( k<m->yield ) {
                        r->damage ( lrint ( fabs ( m->yield-k ) ),false );
                        if ( r->n != ro.n ) {
                            ro.damage_total += lrint ( fabs ( m->yield-k ) );
                        }
                    }
                }
                if ( kill_count>0 && source_alive && ro.armor <=0 ) {
                    --kill_count;
                }
                if ( kill_count>0 ) {
                    ro.kills += kill_count;
                }
            }
        }
    }
}


std::string victor_string ( int k, int n )
{
    std::stringstream ss;
    if ( k==1 ) {
        ss<<"Robot #"<<cstr ( n+1 ) <<" ("<<robot[n].fn<<") wins!";
    } else if ( k==0 ) {
        ss<<"Simulanteous destruction, match is a tie.";
    } else if ( k>1 ) {
        ss<<"No clear victor, match is a tie.";
    }
    return ss.str();
}

void show_statistics()
{
    int j,k;
    unsigned n;
    {
        textcolor ( 15 );
        cout<<"Match "<<played<<"/"<<matches<<" results:"<<endl<<endl;
        cout<<"Robot            Scored   Wins  Matches  Armor  Kills  Deaths    Shots"<<endl;
        cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
        n=-1;
        k=0;
        for ( unsigned i=0; i<robot.size(); ++i ) {
            if ( ( robot[i].armor>0 ) || ( robot[i].won ) ) {
                ++k;
                n=i;
            }
        }
        for ( unsigned i=0; i<robot.size(); ++i ) {
            Robot &r=robot[i];
            textcolor ( robot_color ( i ) );
            if ( ( k==1 ) && ( n==i ) ) {
                j=1;
            } else {
                j=0;
            }
            cout<<addfront ( cstr ( i+1 ),2 ) <<" - "<<addrear ( r.fn,15 ) <<cstr ( j );
            cout<<addfront ( cstr ( r.wins ),8 ) <<addfront ( cstr ( r.trials ),8 );
            cout<<addfront ( cstr ( r.armor ) +"%",9 ) <<addfront ( cstr ( r.kills ),7 );
            cout<<addfront ( cstr ( r.deaths ),8 ) <<addfront ( cstr ( r.match_shots ),9 );
            cout<<endl;
        }
        textcolor ( 15 );
        cout<<endl<<victor_string ( k,n ) <<endl<<endl;
        cout<<"Cycles played: "<<game_cycle<<endl;
    }
}

void score_robots()
{
    int n,i,k;
    k=0;
    n=-1;
    for ( i=0; i<num_robots; ++i ) {
        ++ ( robot[i].trials );
        if ( robot[i].armor>0 ) {
            ++k;
            n=i;
        }
    }
    if ( ( k==1 ) && ( n>=0 ) ) {
        ++ ( robot[n].wins );
        robot[n].won=true;
    }
    return;
}

void init_bout()
{
    unsigned int i;
    game_cycle=0;
    for ( i=0; i<robot.size(); ++i ) {
        Robot &r = robot[i];
        r.reset_hardware();
        r.reset_software();
    }
    
    reset_missiles();
    
    /*
     *    {writeln(#13+'Match ',played,'/',matches,', Battle in progress...');
     * ´  writeln;}
     */
    return;
}

void bout()
{
    ++played;
    init_bout();

    do {
        ++game_cycle;
        do_game_cycle();
    } while ( !  gameover() );
    score_robots();
    if ( !silent ) {
        show_statistics();
    }
    return;
}

void do_game_cycle()
{
    for ( boost::ptr_vector<Robot>::iterator r = robot.begin(), end=robot.end(); r!=end; ++r ) {
        if ( r->armor>0 ) {
            //cout<<"doing robot #"<<i<<endl;
            r->execute();
        }
    }
    do_missiles ();
    for ( boost::ptr_vector<Robot>::iterator r = robot.begin(), end=robot.end(); r!=end; ++r ) {
        for ( int k=0; k<MAX_MINES; ++k ) {
            if ( r->mine[k].yield>0 ) {
                do_mine ( *r,k );
            }
        }
    }
}

void set_matches ( const int m )
{
    matches=m;
    if ( matches > 100000 ) {
        matches = 100000;
    }
    if ( matches < 1 ) {
        matches = 1;
    }
}
