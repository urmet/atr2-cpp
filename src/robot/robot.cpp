// Robot implementation

#include <boost/ptr_container/ptr_vector.hpp>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <cstdint>

#include "atr2.h"
#include "atr2func.h"
#include "filelib.h"
#include "opcodes.h"
#include "robot.h"

extern boost::ptr_vector<Robot> robot;
extern unsigned int num_robots;
extern long executed;

using std::cout;
using std::endl;

Robot::Robot ( int n1, std::string filename )
{
    if ( testing && n1==0 ) {
        debug = true;
        cout<<"debugging robot #1"<<endl;
    }
    config_rec *c = &config;
    n=n1;
    wins= 0;
    trials=0;
    kills=0;
    deaths=0;
    shots_fired=0;
    match_shots=0;
    hits=0;
    damage_total=0;
    cycles_lived=0;
    error_count=0;
    plen=0;
    max_time=0;
    name="";
    speed=0;
    arc_count=0;
    sonar_count=0;
    robot_time_limit=0;
    scanrange=1500;
    shotstrength=1;
    damageadj=1;
    speedadj=1;
    mines=0;
    c->scanner=2;
    c->weapon=2;
    c->armor=2;
    c->engine=2;
    c->heatsinks=1;
    c->shield=0;
    c->mines=0;
    for (unsigned i=0; i<MAX_RAM; ++i ) {
        ram[i]=0;
    }
    for (unsigned i=0; i<MAX_STACK; ++i ) {
        stack[i]=0;
    }
    for (unsigned i=0; i<MAX_CODE; ++i ) {
        code[i].op=NOP;
        code[i].arg1=0;
        code[i].arg2=0;
        code[i].microcode=0;
    }
    reset_hardware();
    reset_software();

    fn = base_name ( no_path ( filename ) );
    compile ( *this, filename );
    robot_config();
    int k = c->scanner+c->armor+c->weapon+c->engine+c->heatsinks+c->shield+c->mines;
    if ( k>max_config_points ) {
        prog_error ( 21,cstr ( k ) +"/"+cstr ( max_config_points ) );
    }

    return;
}

void Robot::reset_software()
{
    unsigned i;
    for ( i=0; i<MAX_RAM; ++i ) {
        ram[i] = 0;
    }
    for ( i=0; i<MAX_STACK; ++i ) {
        stack[i] = 0;
    }
    thd = hd;
    tspd = 0;
    scanarc=8;
    shift=0;
    err=0;
    overburn=false;
    keepshift=false;
    ip=0;
    accuracy=0;
    meters=0;
    delay_left=0;
    time_left=0;
    shields_up=false;
    return;
}

void Robot::reset_hardware()
{
    unsigned i;
    double d,dd;

    do {
        x= rand() %1000;
        y= rand() %1000;
//         cout<<"placed robot #"<<n+1<<" at "<<x<<"/"<<y<<endl;
        dd= 1000;
        for ( i=0; i<robot.size(); ++i ) {
            if ( ( robot[i].x ) <0 ) {
                robot[i].x=0;
            }
            if ( ( robot[i].x ) >1000 ) {
                robot[i].x=1000;
            }
            if ( ( robot[i].y ) <0 ) {
                robot[i].y=0;
            }
            if ( ( robot[i].y ) >1000 ) {
                robot[i].y=1000;
            }
            d=distance ( x,y,robot[i].x,robot[i].y );
            if ( ( ( robot[i].armor ) >0 ) && ( & ( robot[i] ) != this ) && ( d<dd ) ) {
                dd=d;
            }
        }
    } while ( dd<32 );

    for ( i=0; i<MAX_MINES; ++i ) {
        mine_rec *m = & ( mine[i] );
        m->x=-1;
        m->y=-1;
        m->yield=0;
        m->detonate=false;
        m->detect=0;
    }
    lx=-1;
    ly=-1;
    hd=rand() %256;
    shift=0;
    lhd=hd+1;
    lshift=shift+1;
    spd=0;
    speed=0;
    cooling=false;
    armor=100;
    larmor=0;
    heat=0;
    lheat=1;
    match_shots=0;
    won=false;
    last_damage=0;
    last_hit=0;
    transponder=n+1;
    meters=0;
    shutdown=400;
    shields_up=false;
    channel=transponder;
    startkills=kills;
    robot_config();
    return;
}

void Robot::robot_config()
{
    config_rec *c = & ( config );
    switch ( c->scanner ) {
    case 5:
        scanrange = 1500;
        break;
    case 4:
        scanrange = 1000;
        break;
    case 3:
        scanrange = 700;
        break;
    case 2:
        scanrange = 500;
        break;
    case 1:
        scanrange = 350;
        break;
    default:
        scanrange = 250;
    }
    switch ( c->weapon ) {
    case 5:
        shotstrength = 1.5;
        break;
    case 4:
        shotstrength = 1.35;
        break;
    case 3:
        shotstrength = 1.2;
        break;
    case 2:
        shotstrength = 1.0;
        break;
    case 1:
        shotstrength = 0.8;
        break;
    default:
        shotstrength = 0.5;
    }
    switch ( c->armor ) {
    case  5:
        damageadj = 0.66;
        speedadj = 0.66;
        break;
    case  4:
        damageadj = 0.77;
        speedadj = 0.75;
        break;
    case  3:
        damageadj = 0.83;
        speedadj = 0.85;
        break;
    case  2:
        damageadj = 1.00;
        speedadj = 1.00;
        break;
    case  1:
        damageadj = 1.50;
        speedadj = 1.20;
        break;
    default:
        damageadj = 2.00;
        speedadj = 1.33;
    }
    switch ( c->engine ) {
    case 5:
        speedadj = speedadj*1.5;
        break;
    case 4:
        speedadj = speedadj*1.35;
        break;
    case 3:
        speedadj = speedadj*1.2;
        break;
    case 2:
        speedadj = speedadj*1.0;
        break;
    case 1:
        speedadj = speedadj*0.8;
        break;
    default:
        speedadj = speedadj*0.5;
    }
    switch ( c->mines ) {
    case 5:
        mines = 24;
        break;
    case 4:
        mines = 16;
        break;
    case 3:
        mines = 10;
        break;
    case 2:
        mines = 6;
        break;
    case 1:
        mines = 4;
        break;
    default:
        mines = 2;
        c->mines=0;
    }
    shields_up=false;
    if ( c->shield<3 || c->shield>5 ) {
        c->shield=0;
    }
    if ( c->heatsinks<0 || c->heatsinks>5 ) {
        c->heatsinks=0;
    }
    return;

}

void Robot::execute()
{
    double heat_mult;
    if ( armor<0 ) { // already dead
        return;
    }

    //time slice
    time_left=time_slice;
    if ( ( time_left>robot_time_limit ) && ( robot_time_limit>0 ) ) {
        time_left=robot_time_limit;
    }
    if ( ( time_left>max_time ) && ( max_time>0 ) ) {
        time_left=max_time;
    }
    executed=0;

    // execute timeslice
    while ( time_left && ! ( cooling ) && ( executed< ( 20+time_slice ) ) && ( armor>0 ) ) {
        if ( delay_left>0 ) {
            -- ( delay_left );
            -- ( time_left );
        }
        if ( ( delay_left==0 ) ) {
            execute_instruction();
        }
        if ( heat>=shutdown ) {
            cooling=true;
            shields_up=false;
        }
        if ( heat>=500 ) { // self destruct
            damage ( 1000,true );
        }
    }

    // fix up variables
    thd = ( thd+1024 ) &255;
    hd = ( hd+1024 ) &255;
    shift= ( shift+1024 ) &255;

    if ( tspd<-75 ) {
        tspd=-75;
    }
    if ( tspd>100 ) {
        tspd=100;
    }
    if ( spd<-75 ) {
        spd=-75;
    }
    if ( spd>100 ) {
        spd=100;
    }
    if ( last_damage< ( unsigned ) maxint ) {
        ++ ( last_damage );
    }
    if ( last_hit< ( unsigned ) maxint ) {
        ++ ( last_hit );
    }

    // heat up every fourth cycle
    if ( shields_up && ( ( game_cycle&3 ) ==0 ) ) {
        ++ ( heat );
    }
    if ( ! ( shields_up ) ) {
        if ( heat>0 ) {
            switch ( config.heatsinks ) {
            case 5:
                if ( ( game_cycle&1 ) == 0 ) {
                    -- ( heat );
                }
                break;
            case 4:
                if ( ( game_cycle%3 ) == 0 ) {
                    -- ( heat );
                }
                break;
            case 3:
                if ( ( game_cycle&3 ) == 0 ) {
                    -- ( heat );
                }
                break;
            case 2:
                if ( ( game_cycle&7 ) == 0 ) {
                    -- ( heat );
                }
                break;
            case 1:
                break;
            default:
                // no heatsinks - heat up every fourth cycle
                if ( ( game_cycle&3 ) ==0 ) {
                    ++ ( heat );
                }
            }
        }

        // overburn heats up every third cycle
        if ( overburn && ( game_cycle%3==0 ) ) {
            ++ ( heat );
        }

        // bit of normal cooling
        if ( heat>0 ) {
            -- ( heat );
        }

        // slow rolling ceeps you cool
        if ( ( heat>0 ) && ( ( game_cycle&7 ) ==0 ) && ( abs ( tspd ) <=25 ) ) {
            -- ( heat );
        }

        // cool enough
        if ( heat<= ( shutdown-50 ) ) {
            cooling=false;
        }
    } // if(!shields_up)

    // no moving while cooling
    if ( cooling ) {
        tspd=0;
    }

    // hot stuff moves slower
    heat_mult=1;
    if ( heat>=80 && heat<=99 ) {
        heat_mult=0.98;
    } else if ( heat>=100 && heat<=149 ) {
        heat_mult=0.95;
    } else if ( heat>=150 && heat<=199 ) {
        heat_mult=0.85;
    } else if ( heat>=200 && heat<=249 ) {
        heat_mult=0.75;
    } else if ( heat>=250 && heat<= maxint ) {
        heat_mult=0.50;
    }
    if ( overburn ) {
        heat_mult *= 1.30;
    }

    // hot stuff also burns
    if ( ( heat>=475 ) && ( ( game_cycle&3 ) ==0 ) ) {
        damage ( 1,true );
    } else if ( ( heat>=450 ) && ( ( game_cycle&7 ) ==0 ) ) {
        damage ( 1,true );
    } else if ( ( heat>=400 ) && ( ( game_cycle&15 ) ==0 ) ) {
        damage ( 1,true );
    } else if ( ( heat>=350 ) && ( ( game_cycle&31 ) ==0 ) ) {
        damage ( 1,true );
    } else if ( ( heat>=300 ) && ( ( game_cycle&63 ) ==0 ) ) {
        damage ( 1,true );
    }

    // update robot inp hysical world

    //acceleration
    if ( abs ( ( spd )- ( tspd ) ) <=acceleration ) {
        spd=tspd;
    } else {
        if ( ( tspd ) > ( spd ) ) {
            spd += acceleration;
        } else {
            spd -= acceleration;
        }
    }

    //turning

    // absolute turret direction
    int tthd = hd+shift;

    if ( ( abs ( hd-thd ) <=turn_rate ) || ( abs ( hd-thd ) >=256-turn_rate ) ) {
        hd=thd;
    } else if ( hd!=thd ) {
        int k=0;
        if ( ( ( ( thd ) > ( hd ) ) && ( abs ( ( hd )- ( thd ) ) <=128 ) ) ||
                ( ( thd<hd ) && ( abs ( hd-thd ) >=128 ) ) ) {
            k=1;
        }
        if ( k==1 ) {
            hd = ( hd+turn_rate ) &255;
        } else {
            hd = ( hd+256-turn_rate ) &255;
        }
    }
    hd=hd&255; // normalize

    // rotate turret with the tank
    if ( keepshift ) {
        shift= ( tthd- ( hd ) +1024 ) &255;
    }

    //moving
    // target positions
    double ttx,tty;
    speed= ( ( float ) spd/100.0* ( ( float ) max_vel*heat_mult* speedadj ) );
    xv=sint[hd]* ( speed );
    yv=cost[hd]* ( speed );
    if ( hd==0 || hd==128 ) {
        xv=0;
    }
    if ( hd==64 || hd==192 ) {
        yv=0;
    }
    if ( xv!=0 ) {
        ttx=x+xv;
    } else {
        ttx=x;
    }
    if ( yv!=0 ) {
        tty=y+yv;
    } else {
        tty=y;
    }
    if ( ttx<0 || tty<0 || ttx>1000 || tty>1000 ) {
        ++ ( ram[8] ); // increase collision count
        tspd=0;        // stop
        spd=0;
        if ( fabs ( speed ) >= ( ( double ) max_vel/2.0 ) ) {
            damage ( 1,true ); // collision damage
        }
    }
    for ( boost::ptr_vector<Robot>::iterator r=robot.begin(), end=robot.end(); r!=end; ++r ) {
        if ( ( r->n!=n ) && ( r->armor>0 ) && ( distance ( ttx,tty,r->x,r->y ) <crash_range ) ) {
            // Crash!!
            // stop both robots
            tspd=0;
            spd=0;
            ttx=x;
            tty=y;
            r->tspd=0;
            r->spd=0;

            // increase crash counts
            ++ram[8];
            ++ ( r->ram[8] );
            if ( fabs ( speed ) >= ( ( double ) max_vel/2.0 ) ) {
                damage ( 1,true );
                r->damage ( 1,true );
            }
        }
    }

    if ( ttx<0 ) {
        ttx=0;
    }
    if ( tty<0 ) {
        tty=0;
    }
    if ( ttx>1000 ) {
        ttx=1000;
    }
    if ( tty>1000 ) {
        tty=1000;
    }

    meters += distance ( x,y,ttx,tty );
    if ( meters>=maxint ) {
        meters -=maxint;
    }
    ram[9]=meters;
    x=ttx;
    y=tty;

    if ( armor<0 ) {
        armor =0;
    }
    if ( heat<0 ) {
        heat =0;
    }
    lheat=heat;
    larmor=armor;

    ++ ( cycles_lived );
    return;

}


void Robot::execute_instruction()
{
    int j,k;
    unsigned time_used;
    int loc;
    bool inc_ip;

    ram[0]=tspd;
    ram[1]=thd;
    ram[2]=shift;
    ram[3]=accuracy;
    time_used=1;
    inc_ip=true;
    loc=0;
    if ( ( ip ) >plen )  {
        ip=0;
    }
    if ( invalid_microcode ( *this,ip ) ) {
        time_used=1;
        robot_error ( 16,hex ( code[ip].microcode ) );
    }
    if ( ! ( ( ( code[ip].microcode&7 ) ==1 ) ||
             ( ( code[ip].microcode&7 ) ==0 ) ) ) {
        time_used=0;
    } else {
        if ( debug ) {
            cout<<"executing instruction: ip:"<<ip<<":"
                <<mnemonic(code[ip].op,code[ip].microcode&15) <<" "
                <<(code[ip].arg1) <<" "
                <<(code[ip].arg2)<<endl;
                
        }
        switch ( get_val ( ip,0 ) ) {
        case 0: // NOP
            ++executed;
            break;
        case 1: // ADD
            put_val ( ip,1,get_val ( ip,1 ) +get_val ( ip,2 ) );
            ++executed;
            break;
        case 2: // SUB
            put_val ( ip,1,get_val ( ip,1 )-get_val ( ip,2 ) );
            ++executed;
            break;
        case 3: // OR
            put_val ( ip,1, ( get_val ( ip,1 ) |get_val ( ip,2 ) ) );
            ++executed;
            break;
        case 4: // AND
            put_val ( ip,1, ( get_val ( ip,1 ) &get_val ( ip,2 ) ) );
            ++executed;
            break;
        case 5: // XOR
            put_val ( ip,1, ( get_val ( ip,1 ) ^ get_val ( ip,2 ) ) );
            ++executed;
            break;
        case NOT:
            put_val ( ip,1, ~ ( get_val ( ip,1 ) ) );
            ++executed;
            break;
        case 7: // MPY
            put_val ( ip,1,get_val ( ip,1 ) *get_val ( ip,2 ) );
            time_used=10;
            ++executed;
            break;
        case DIV:
            j = get_val ( ip,2 );
            if ( j==0 ) {
                robot_error ( 8,"" );
            } else {
                put_val ( ip,1, ( get_val ( ip,1 ) /j ) );
            }
            time_used=10;
            ++executed;
            break;
        case MOD:
            j = get_val ( ip,2 );
            if ( j==0 ) {
                robot_error ( 8,"" );
            } else {
                put_val ( ip,1, ( get_val ( ip,1 ) %j ) );
            }
            time_used=10;
            ++executed;
            break;
        case 10: // RET
            ip = pop ();
            if ( ip>plen ) {
                robot_error ( 7,cstr ( ip ) );
            }
            ++executed;
            break;
        case 11: // GSB
            loc=find_label ( get_val ( ip,1 ), ( code[ip].microcode>>4 ) );
            if ( loc>=0 ) {
                push ( ip );
                inc_ip=false;
//            cout<<"GSB: old ip: "<<ip<<" new ip: "<<loc<<endl;
                ip=loc;
            } else {
                robot_error ( 2,cstr ( get_val ( ip,1 ) ) );
            }
            ++executed;
            break;
        case 12: // JMP
            jump ( 1,inc_ip );
            ++executed;
            break;
        case 13: // JLS
            if ( ( ram[64] ) &2 ) {
                jump ( 1,inc_ip );
            }
            time_used=0;
            ++executed;
            break;
        case 14: // JGR
            if ( ( ram[64] ) &4 )  {
                jump ( 1,inc_ip );
            }
            time_used=0;
            ++executed;
            break;
        case 15: // JNE
            if ( ( ( ram[64] ) &1 ) ==0 ) {
                jump ( 1,inc_ip );
            }
            time_used=0;
            ++executed;
            break;
        case 16: // JEQ / JE
            if ( ram[64]&1 ) {
                jump ( 1,inc_ip );
            }
            time_used=0;
            ++executed;
            break;
        case 17: // SWAP/XCHG
            ram[4]=get_val ( ip,1 );
            put_val ( ip,1,get_val ( ip,2 ) );
            put_val ( ip,2,ram[4] );
            time_used=3;
            ++executed;
            break;
        case DO:
            ram[67]=get_val ( ip,1 );
            ++executed;
            break;
        case 19: // LOOP
            -- ( ram[67] );
            if ( ram[67]>0 ) {
                jump ( 1, inc_ip );
            }
            ++executed;
            break;
        case CMP: // CMP
            k=get_val ( ip,1 )-get_val ( ip,2 );
//             cout<<"CMP "<<get_val(ip,1)<<" "<<get_val(ip,2)<<endl;
            ram[64] &= 0xff00; // zero flags
            if ( k==0 ) {
                ram[64] |= 1;
            }
            if ( k<0 ) {
                ram[64] |= 2;
            }
            if ( k>0 ) {
                ram[64] |= 4;
            }
            if ( ( get_val ( ip,2 ) ==0 ) && ( k==0 ) ) {
                ram[64] |= 8;
            }
            ++executed;
            break;
        case TEST:
            k=get_val ( ip,1 ) &get_val ( ip,2 );
            ram[64] &= 0xFFF0;
            if ( k==get_val ( ip,2 ) ) {
                ram[64] |= 1;
            }
            if ( k==0 ) {
                ram[64] |= 8;
            }
            ++executed;
            break;
        case 22: // MOV / SET
            put_val ( ip,1,get_val ( ip,2 ) );
            ++executed;
            break;
        case 23: // LOC
            put_val ( ip,1,code[ip].arg2 );
            time_used=2;
            ++executed;
            break;
        case 24: // GET
            k=get_val ( ip,2 );
            if ( k>=0 && k<=MAX_RAM ) {
                put_val ( ip,1,ram[k] );
            } else if ( ( k>MAX_RAM ) && ( k<= ( MAX_RAM+1+ ( ( MAX_CODE ) >>3 )-1 ) ) ) {
                j=k-MAX_RAM-1;
                switch ( j&3 ) {
                case 0:
                    put_val ( ip,1,code[j>>2].op );
                    break;
                case 1:
                    put_val ( ip,1,code[j>>2].arg1 );
                    break;
                case 2:
                    put_val ( ip,1,code[j>>2].arg2 );
                    break;
                case 3:
                    put_val ( ip,1,code[j>>2].microcode );
                    break;
                }
            } else {
                robot_error ( 4,cstr ( k ) );
            }
            time_used=2;


            ++executed;
            break;
        case 25: // PUT
            k=get_val ( ip,2 );
            if ( k<0 || k> MAX_RAM ) {
                robot_error ( 4,cstr ( k ) );
            } else {
                ram[k] =get_val ( ip,1 );
            }
            time_used=2;
            ++executed;
            break;
        case 26: // INT
            call_int ( get_val ( ip,1 ),time_used );
            ++executed;
            break;
        case 27: // IPO / IN
            time_used=4;
            put_val ( ip,2,in_port ( get_val ( ip,1 ),time_used ) );
            ++executed;
            break;
        case 28: // OPO / OUT
            time_used=4;
            out_port ( get_val ( ip,1 ),get_val ( ip,2 ),time_used );
            ++executed;
            break;
        case 29: // DEL / DELAY
            time_used = get_val ( ip,1 );
            ++executed;
            break;
        case PUSH:
            push ( get_val ( ip,1 ) );
            ++executed;
            break;
        case 31: // POP
            put_val ( ip,1,pop() );
            ++executed;
            break;
        case 32: // ERR
            robot_error ( get_val ( ip,1 ),"" );
            time_used=0;
            ++executed;
            break;
        case 33: // INC
            put_val ( ip,1,get_val ( ip,1 ) +1 );
            ++executed;
            break;
        case 34: // DEC
            put_val ( ip,1,get_val ( ip,1 )-1 );
            ++executed;
            break;
        case 35: // SHL
            put_val ( ip,1,get_val ( ip,1 ) <<get_val ( ip,2 ) );
            ++executed;
            break;
        case SHR: // SHR
            put_val ( ip,1, ( get_val ( ip,1 ) >>get_val ( ip,2 ) ) );
//             cout<<"SHR "<<get_val ( ip,1 ) <<" "<<get_val ( ip,2 ) <<endl;
            ++executed;
            break;
        case 37: //ROL
            put_val ( ip,1,rol ( get_val ( ip,1 ),get_val ( ip,2 ) ) );
            ++executed;
            break;
        case 38: // ROR
            put_val ( ip,1,ror ( get_val ( ip,1 ),get_val ( ip,2 ) ) );
            ++executed;
            break;
        case 39: // JZ
            time_used=0;
            if ( ram[64]&8 ) {
                jump ( 1,inc_ip );
            }
            ++executed;
            break;
        case 40: // JNZ
            time_used=0;
            if ( ! ( ram[64]&8 ) ) {
                jump ( 1,inc_ip );
            }
            ++executed;
            break;
        case 41: // JAE / JGE
            if ( ram[64]&1 || ram[64]&4 ) {
                jump ( 1,inc_ip );
            }
            time_used=0;
            ++executed;
            break;
        case 42: // JBE / LJE
            if ( ram[64]&1 || ram[64]&2 ) {
                jump ( 1,inc_ip );
            }
            time_used=0;
            ++executed;
            break;
        case 43: // SAL
            put_val ( ip,1,sal ( get_val ( ip,1 ),get_val ( ip,2 ) ) );
            ++executed;
            break;
        case 44: //SAR
            put_val ( ip,1,sar ( get_val ( ip,1 ),get_val ( ip,2 ) ) );
            ++executed;
            break;
        case 45: // NEG
            put_val ( ip,1,0-get_val ( ip,1 ) );
            ++executed;
            break;
        case 46:
            loc=get_val ( ip,1 );
            if ( ( loc>=0 ) && ( ( unsigned ) loc<=plen ) ) {
                inc_ip=false;
                ip=loc;
            } else {
                robot_error ( 2,cstr ( loc ) );
            }
            break;

        default:
            cout<<"unknown opcode: "<<get_val ( ip,0 ) <<endl;
            cout<<"ip = "<<ip<<endl;
            robot_error ( 6,"" );
        }
    }
    delay_left +=time_used;
    if ( inc_ip ) {
        ++ ( ip );
    }

    return;
}

int Robot::get_val ( const int c, const int o )
{
//    cout<<"get_val("<<n<<","<<c<<","<<o<<")"<<endl;
    int i=0,j,k;
    j= ( ( code[c].microcode ) >> ( 4*o ) ) &15;
//     cout<<"microcode="<<r.code[c].microcode<<endl;
    if ( o==0 ) {
        i=code[c].op;
    }
    else if ( o==1 ) {
        i=code[c].arg1;
    }
    else if ( o==2 ) {
        i=code[c].arg2;
    }
    if ( ( j&7 ) ==1 ) {
        k=get_from_ram ( i );
    } else {
        k=i;
    }
    if ( ( j&8 ) >0 ) {
        k=get_from_ram ( k );
    }
//   cout<<"get_val returns "<<k<<" i="<<i<<" j="<<j<<endl;
    return k;
}

void Robot::put_val ( int c, int o, int v )
{
    int i=0,j=0;
    j= ( code[c].microcode >> ( 4*o ) ) &15;
    if ( o==0 ) {
        i=code[c].op;
    } else if ( o==1 ) {
        i=code[c].arg1;
    } else if ( o==2 ) {
        i=code[c].arg2;
    }
//     cout<<"put_val("<<c<<","<<o<<","<<v<<")"<<endl;
    if ( ( j&7 ) ==1 ) {
        if ( ( i<0 ) || ( i>=MAX_RAM ) ) {
            robot_error ( 4,cstr ( i ) );
        } else if ( ( j&8 ) >0 ) {
            i=ram[i];
            if ( ( i<0 ) || ( i> ( MAX_RAM-1 ) ) ) {
                robot_error ( 4,cstr ( i ) );
            } else {
//            cout<<"putting "<<v<<" in robot["<<n<<"]->ram["<<i<<"]"<<endl;
                ram[i]=v;
            }
        } else {
//            if(i==65||i==66)
//               cout<<"putting "<<v<<" in robot["<<n<<"]->ram["<<i<<"]"<<endl;
            ram[i]=v;
        }
    } else {
        robot_error ( 3,"" );
    }
}

int Robot::scan()
{
    //   cout<<"scan("<<n<<")"<<endl;
    float r,d,acc;
    int dir,range,j,tmprange,nn,xx,yy,sign;
    nn=-1;
    range=maxint;

    if ( scanarc<0 ) {
        scanarc=0;
    }
    accuracy=0;
    nn=0;
    dir= ( shift+hd ) &255; // direction of turret
    
    // scan them robots!!
    for ( boost::ptr_vector<Robot>::iterator ro=robot.begin(), end=robot.end(); ro!=end; ++ro ) {
        
        if ( ( ( ro->n ) !=n ) && ( ro->armor>0 ) ) { // not self and not dead
            
            j=find_anglei ( x,y,ro->x,ro->y ); // heading to other robot
            d=distance ( x,y,ro->x,ro->y ); // distance to other robot
            if ( debug ) {
                cout<<"angle between robots: "<<j<<endl;
            }
            tmprange=lrint ( d );
            
            bool visible = ( ( abs ( j-dir ) <=scanarc ) || ( abs ( j-dir ) >= ( 256-scanarc ) ) ); 
            
            if ( ( tmprange<range ) && ( tmprange<=( scanrange ) )  && visible ){
                dir = ( dir+1024 ) &255;
                xx=rint ( sint[dir]*d+ ( x ) );  // rounded location of target
                yy=rint ( -cost[dir]*d+ ( y ) ); // rounded location of target
                r=distance ( xx,yy,ro->x,ro->y );
                if ( debug ) {
                    cout<<"SCAN HIT! Scan X,Y: "<<round ( xx ) <<","<<round ( yy ) <<" Robot X,Y: "
                        <<round ( ro->x ) <<","<<round ( ro->y ) <<" Dist="<<round ( r ) <<" n="<<ro->n<<endl;
                    cout<<"self X,Y: "<<round(x)<<","<<round(y)<<endl;
                }
                if ( ( scanarc>0 ) || ( r< ( hit_range-2 ) ) ) {
                    range=tmprange;
                    accuracy=0;
                    if ( scanarc>0 ) {
                        j= ( j+1024 ) &255;
                        dir= ( dir+1024 ) &255;
                        if ( j<dir ) {
                            sign=-1;
                        }
                        if ( j>dir ) {
                            sign=1;
                        }
                        if ( ( j>190 ) && ( dir<66 ) ) {
                            dir +=256;
                            sign=-1;
                        }
                        if ( ( dir>190 ) && ( j<66 ) ) {
                            j += 256;
                            sign = 1;
                        }
                        acc = ( abs ( j-dir ) / ( scanarc*2 ) );
                        if ( sign<0 ) {
                            accuracy = -abs ( rint ( acc ) );
                        } else {
                            accuracy =  abs ( rint ( acc ) );
                        }
                        if ( accuracy>2 ) {
                            accuracy=2;
                        }
                        if ( accuracy<-2 ) {
                            accuracy=-2;
                        }

                    }
                }
                nn=ro->n;
            }
        }
    }
    if ( nn>=0 && nn<num_robots ) {
        ram[5]=robot[nn].transponder;
        ram[6]= ( ( robot[nn].hd ) - ( hd+shift ) +1024 ) &255;
        ram[7]=robot[nn].spd;
        ram[13]=rint ( robot[nn].speed*100 );
    }
    return range;
}

int Robot::get_from_ram ( int i )
{

//   cout<<"get_from_ram for robot #"<<n<<" location;"<<i<<endl;
    int k=0,l;
    if ( ( i<0 ) or ( i> ( MAX_RAM+1 ) + ( ( ( MAX_CODE+1 ) << 3 )-1 ) ) ) {
        k=0;
        robot_error ( 4,cstr ( i ) );
    } else {
        if ( i<=MAX_RAM ) { // tavaline aadress.
            k= ram[i];
        } else {            // programmikoodi aadress
            l= ( i-MAX_RAM-1 );
            switch ( l&3 ) {
            case 0:
                k=code[l>>2].op;
                break;
            case 1:
                k=code[l>>2].arg1;
                break;
            case 2:
                k = code[l>>2].arg2;
            case 3:
                k=code[l>>2].microcode;
            }
        }
    }
//     cout<<"get_from_ram returns "<<k<<endl;
    return k;
}

int Robot::pop()
{
    int k=0;
    if ( ( ram[71]>0 ) && ( ram[71]<=MAX_STACK ) ) {
        k=stack[ram[71]];
        -- ( ram[71] );
    } else {
        robot_error ( 5,cstr ( ram[71] ) );
    }
    return k;
}

int Robot::find_label ( int l, int m )
{
    /**
     * find_label
     * @param n number of robot
     * @param l label identifier
     * @param m microcode
     */
// cout<<"find_label("<<l<<","<<m<<")"<<endl;
    int i,j,k;
    k=-1;
    if ( m==3 ) { // unresolved label
        robot_error ( 9,"" );
    } else if ( m==4 ) { // simple !label, already resolved
        k=l;
    } else {
//         cout<<"vastik :label l="<<l<<" m="<<m<<endl;
        for ( i= ( plen-1 ); i>=0; --i ) {
            j=code[i].microcode&15;
            if ( ( j==2 ) && ( code[i].op==l ) ) {
                k=i;
            }
        }
    }
    return k;
}

void Robot::push ( int v )
{
//   cout<<"push!"<<endl;
    if ( ( ram[71]>=0 ) && ( ram[71]<MAX_STACK ) ) {
        ++ ( ram[71] );
        stack[ram[71]] = v;
    } else {
        robot_error ( 1,cstr ( ram[71] ) );
    }
    return;
}

void Robot::jump ( int o, bool& inc_ip )
{
    int loc;
    loc = find_label ( get_val ( ip,o ), ( code[ip].microcode>> ( o*4 ) ) );

    if ( ( loc>=0 ) && ( loc<= ( plen ) ) ) {
        inc_ip=false;
        ip=loc;
    } else {
        cout<<"JMP failed to "<<loc<<endl;
        robot_error ( 2,cstr ( loc ) );
    }
    return;

}

void Robot::robot_error ( const int i, const std::string ov )
{
//     if ( logging_errors ) {
//         log_error ( i,ov );
//     }
    ++error_count;
    return;
}

int Robot::in_port ( int p, unsigned int& time_used )
{
//     cout<<"in_port("<<p<<")"<<endl;
    int v=0,j,k,l,nn;
    switch ( p ) {
    case 1:
        v=spd;
        break;
    case 2:
        v=heat;
        break;
    case 3:
        v=hd;
        break;
    case 4:
        v=shift;
        break;
    case 5:
        v= ( shift+hd ) &255;
        break;
    case 6:
        v=armor;
        break;
    case 7:
        v=scan();
//         if(v!=maxint) cout<<"scan returns: "<<v<<endl;
        ++time_used;
        //if(show_arcs) arc_count=2;
        break;
    case 8:
        v=accuracy;
        ++time_used;
        break;
    case 9:
        nn=-1;
        time_used +=3;
        k=maxint;
        for (Robot_it r=robot.begin(),end=robot.end(); r!=end;++r) {
            j=rint ( distance ( x,y,r->x,r->y ) );
            if ( ( n!=r->n ) && ( j<k ) && ( r->armor>0 ) ) {
                k=j;
                nn=r->n;
            }
        }
        v=k;
        if ( ( nn>=0 ) && ( nn<num_robots ) ) {
            ram[5]=robot[nn].transponder;
        }
        break;
    case 10:
        v=rand() %UINT16_MAX;
        break;
    case 16: // sonar. get heading to nearest target
        nn=-1;
        time_used += 40;
        l=-1;
        k=maxint;
        for ( auto r=robot.begin(), end=robot.end(); r!=end; ++r ) {
            j=lrint ( distance ( x,y,r->x,r->y ) );
            if ( ( n!=r->n ) && ( j<k ) && ( r->armor>0 ) && ( j<max_sonar ) ) {
                k=j;
                l=r->n;
                nn=r->n;
            }
        }
        if ( l>=0 ) {
            v= ( lrint ( find_angle ( x,y,robot[l].x,robot[l].y ) /pi*128.0
                         +1024.0+ ( float ) ( rand() %64 )-32 ) &255 );
        } else {
            v=minint;
        }
        if ( nn>=0 ) {
            ram[5] = robot[nn].transponder;
        }
        break;
    case 17:
        v=scanarc;
        break;
    case 18:
        v= overburn ? 1:0;
        break;
    case 19:
        v=transponder;
        break;
    case 20:
        v=shutdown;
        break;
    case 21:
        v=channel;
        break;
    case 22:
        v=mines;
        break;
    case 23:
        if ( config.mines>=0 ) {
            k=0;
            for ( int i=0; i<MAX_MINES; ++i ) {
                mine_rec *m = &mine[i];
                if ( m->x>=0 && m->x<=1000 && m->y>=0 && m->y<=1000 && m->yield>0 ) {
                    ++k;
                }
                v=k;
            }
        } else {
            v=0;
        }
        break;
    case 24:
        if ( config.shield>0 ) {
            if ( shields_up ) {
                v=1;
            } else {
                v=0;
            }
        } else {
            v=0;
            shields_up=false;
        }
        break;
    default:
        robot_error ( 11,cstr ( p ) );
    }
    return v;
}

void Robot::out_port ( int p, int v, unsigned int& time_used )
{
    if(debug)
    {cout<<"out_port("<<p<<", "<<v<<", "<<time_used<<")"<<endl;}
    int i;
    switch ( p ) {
    case 11:
        tspd=v;
        break;
    case 12: // P_OFS_TURRET
        shift= ( shift+v+1024 ) &255;
        break;
    case 13:
        shift= ( v+1024 ) &255;
        break;
    case 14:
        thd= ( thd+v+1024 ) &255;
        break;
    case 15:
        time_used += 3;
        if ( v>4 ) {
            v=4;
        }
        if ( v<-4 ) {
            v=-4;
        }
        shoot_missile ( x,y, ( ( hd+shift+v ) &255 ),overburn );
        break;
    case 17:
//         cout<<"setting scanarc to "<<v<<endl;
        scanarc=v;
        break;
    case 18:
        v?overburn=true:overburn=false;
        break;
    case 19:
        transponder=v;
        break;
    case 20:
        shutdown=v;
        break;
    case 21:
        channel=v;
        break;
    case 22:
        if ( config.mines>=0 ) {
            if ( mines>0 ) {
                init_mine ( v,mine_blast );
                -- ( mines );
            } else {
                robot_error ( 14,"" );
            }
        } else {
            robot_error ( 13,"" );
        }
        break;
    case 23:
        if ( config.mines>=0 ) {
            for ( i=0; i<MAX_MINES; ++i ) {
                mine[i].detonate=true;
            }
        } else {
            robot_error ( 13,"" );
        }
        break;
    case 24:
        if ( config.shield>=3 ) {
            if ( v==0 ) {
                shields_up=false;
            } else {
                shields_up=true;
            }
        } else {
            shields_up=false;
            robot_error ( 11,cstr ( p ) );
        }
        break;
    default:
        robot_error ( 11,cstr ( p ) );
    }
    if ( scanarc>64 ) {
        scanarc=64;
    }
    if ( scanarc<0 ) {
        scanarc=0;
    }
    return;
}

void Robot::call_int ( int int_num, unsigned &time_used )
{
    int i,j,k;
    switch ( int_num ) {
    case 0: // self destruct
        damage ( 1000,true );
        break;
    case 1: // reset
        reset_software();
        time_used=10;
        break;
    case 2: // EX, FX = self x,y
        time_used=5;
        ram[69] = rint ( x );
        ram[70] = rint ( y );
        break;
    case 3:
        time_used=2;
        if ( ram[65]==0 ) {
            keepshift=false;
        } else {
            keepshift=true;
        }
        ram[70]= ( shift ) &255;
        break;
    case 4:
        if ( ram[65]==0 ) {
            overburn=false;
        } else {
            overburn=true;
        }
        break;
    case 5:
        time_used=2;
        ram[70]=transponder;
        break;
    case 6:
        time_used=2;
        ram[69]=game_cycle>>16;
        ram[70]=game_cycle&65535;
        break;
    case 7: // Find angle to point in EX:FX
        j=ram[69];
        k=ram[70];
        if ( j<0 ) {
            j=0;
        }
        if ( j>1000 ) {
            j=1000;
        }
        if ( k<0 ) {
            k=0;
        }
        if ( k>1000 ) {
            k=1000;
        }
        ram[65]= ( ( int ) rint ( find_angle ( x,y,j,k ) /pi*128.0+256 ) ) &255;
        time_used=32;
        break;
    case 8:
        ram[70]=ram[5];
        time_used=1;
        break;
    case 9:
        ram[69]=ram[6];
        ram[70]=ram[7];
        time_used=2;
        break;
    case 10:
        k=0;
        for ( i=0; i<num_robots; ++i )
            if ( robot[i].armor>0 ) {
                ++k;
            }
        ram[68]=k;
        ram[69]=played;
        ram[69]=matches;
        time_used=4;
        break;
    case 11:
        ram[68]=rint ( speed*100 );
        ram[69]=last_damage;
        ram[69]=last_hit;
        time_used=5;

        break;
    case 12:
        ram[70]=ram[8];
        time_used=1;
        break;
    case 13:
        ram[8]=0;
        time_used=1;
        break;
    case 14:
        com_transmit ( channel,ram[65] );
        time_used=1;
        break;
    case 15:
        if ( ( ram[10] ) != ( ram[11] ) ) {
            ram[70]=com_recieve ();
        } else {
            robot_error ( 12,"" );
        }
        time_used=1;
        break;
    case 16:
        if ( ( ram[10] ) >= ( ram[11] ) ) {
            ram[70]= ( ram[11]-ram[10] );
        } else {
            ram[70]=max_queue+1- ( ram[10] ) + ( ram[11] );
        }
        time_used=1;
        break;
    case 17:
        ram[10]=0;
        ram[11]=0;
        time_used=1;
        break;
    case 18:
        ram[68]=kills;
        ram[69]= ( kills )- ( startkills );
        ram[70]=deaths;
        time_used=3;
        break;
    case 19:
        ram[9]=0;
        meters=0;
        break;
    case 98: // test SUCCESS
        exit(0);
        break;
    case 99: // test FAIL
        exit(99);
        break;
    default:
        robot_error ( 10,cstr ( int_num ) );
    }
}

void Robot::shoot_missile ( float xx, float yy, int dir, bool ob )
{
    double multiplier;
    missile *m = new missile;

    if ( debug ) {
        cout<<"firing missile from "<<xx<<"/"<<yy<<" at hd: "<<dir<<endl;
    }

    m->source=n;
    m->x=xx;
    m->lx=m->x;
    m->y=yy;
    m->ly=m->y;
    m->rad=0;
    m->lrad=0;
    if ( ob ) {
        m->mult=1.25;
    } else {
        m->mult=1;
    }


    m->mult *= shotstrength;
    multiplier=m->mult;
    if ( ob ) {
        n+=0.25;
    }
    m->mspd=missile_spd* ( m->mult );
    if ( insane_missiles ) {
        m->mspd=100+ ( 50*insanity ) *m->mult;
    }
    heat += rint ( multiplier*20.0 );
    shots_fired += 1;
    match_shots += 1;
    m->active=1;
    m->hd=dir;
    m->max_rad=mis_radius;

    missiles.push_back ( m );
}

void Robot::log_error ( int i, std::string ov )
{
//     if ( !logging_errors ) {
//         return;
//     }
    std::string s;
    switch ( i ) {
    case 1:
        s="Stack full - Too many CALLs?";
        break;
    case 2:
        s="Label not found. Hmmm.";
        break;
    case 3:
        s="Can""t assign value - Tisk tisk.";
        break;
    case 4:
        s="Illegal memory reference";
        break;
    case 5:
        s="Stack empty - Too many RETs?";
        break;
    case 6:
        s="Illegal instruction. How bizarre.";
        break;
    case 7:
        s="Return out of range - Woops!";
        break;
    case 8:
        s="Divide by zero";
        break;
    case 9:
        s="Unresolved !label. WTF?";
        break;
    case 10:
        s="Invalid Interrupt Call";
        break;
    case 11:
        s="Invalid Port Access";
        break;
    case 12:
        s="Com Queue empty";
        break;
    case 13:
        s="No mine-layer, silly.";
        break;
    case 14:
        s="No mines left";
        break;
    case 15:
        s="No shield installed - Arm the photon torpedoes instead. :-)";
        break;
    case 16:
        s="Invalid Microcode in instruction.";
        break;
    default:
        s="Unkown error";
        break;
    }
    errorlog<<"<"<<i<<"> "<<s<<" (Line #"<<ip<<") [Cycle: "
            <<game_cycle<<", Match: "<<played<<"/"<<matches<<"]"<<endl;
    errorlog<<" "<<mnemonic ( code[ip].op,code[ip].microcode&15 )
            <<" "<<mnemonic ( code[ip].arg1, ( code[ip].microcode>>4 ) &15 )
            <<", "<<mnemonic ( code[ip].arg2, ( code[ip].microcode>>8 ) &15 );
    if ( !ov.empty() ) {
        errorlog<<"     (Values: "<<ov<<")"<<endl;
    } else {
        errorlog<<" AX="<<addrear ( ( cstr ( ram[65] ) +"," ),7 );
        errorlog<<" BX="<<addrear ( ( cstr ( ram[66] ) +"," ),7 );
        errorlog<<" CX="<<addrear ( ( cstr ( ram[67] ) +"," ),7 );
        errorlog<<" DX="<<addrear ( ( cstr ( ram[68] ) +"," ),7 );
        errorlog<<" EX="<<addrear ( ( cstr ( ram[69] ) +"," ),7 );
        errorlog<<" FX="<<addrear ( ( cstr ( ram[70] ) +"," ),7 );
        errorlog<<" Flags="<<ram[64]<<endl;
        errorlog<<" AX="<<addrear ( ( hex ( ram[65] ) +"," ),7 );
        errorlog<<" BX="<<addrear ( ( hex ( ram[66] ) +"," ),7 );
        errorlog<<" CX="<<addrear ( ( hex ( ram[67] ) +"," ),7 );
        errorlog<<" DX="<<addrear ( ( hex ( ram[68] ) +"," ),7 );
        errorlog<<" EX="<<addrear ( ( hex ( ram[69] ) +"," ),7 );
        errorlog<<" FX="<<addrear ( ( hex ( ram[70] ) +"," ),7 );
        errorlog<<" Flags="<<hex ( ram[64] ) <<endl<<endl;;
    }
}

void Robot::damage ( int d, bool physical )
{
//     cout<<"damage("<<n<<", "<<d<<", "<<physical<<");"<<endl;
    int k,h,dd;
    double m;
    if ( armor<=0 ) {
        return;
    }

    if ( config.shield  <3 ) {
        shields_up = false;
    }

    h=0;
    if ( shields_up && !physical ) {
        dd=d;

        switch ( config.shield ) {
        case 3:
            d=rint ( dd* ( 2.0/3.0 ) );
            if ( d<1 ) {
                d=1;
            }
            h=rint ( dd* ( 2.0/3.0 ) );
            break;
        case 4:
            h=dd/2;
            d=dd-h;
            break;
        case 5:
            d=rint ( dd* ( 1.0/3 ) );
            if ( d<1 ) {
                d=1;
            }
            h=rint ( dd* ( 1.0/3 ) );
            if ( h<1 ) {
                h=1;
            }
            break;
        }

    }
    if ( d<0 ) {
        d=0;
    }
    //if(debug_info)
    //begin writeln(#13,zero_pad(game_cycle,5),' D ',n,': ',armor,'-',d,'=',armor-d,'           ');
    if ( d>0 ) {
        d=rint ( damageadj*d );
        if ( d<1 ) {
            d=1;
        }
    }
    armor -= d;
    heat += h;
    last_damage=0;
    if ( armor<=0 ) {
        armor=0;
        heat=500;
        armor=0;
        kill_count += 1;
        deaths  += 1;
        heat=0;
//         update_heat(n);
//         int blast_circle = ( float ) blast_radius*screen_scale+1;
//         init_missile ( x,y,0,blast_circle,false );
        if ( overburn ) {
            m=1.3;
        } else {
            m=1.0;
        }
        for ( unsigned i=0; i<robot.size(); ++i ) {
            if ( i!=n && ( robot[i].armor>0 ) ) {
                k=rint ( distance ( x,y,robot[i].x,robot[i].y ) );
                if ( k<blast_radius ) {
                    robot[i].damage ( rint ( abs ( blast_radius-k ) *m ),false );
                }
            }
        }
    }
    return;
}

void Robot::init_mine ( int detectrange,int size )
{
    int i,k;
    k=-1;
    mine_rec *m;
    for ( i=0; i<MAX_MINES; ++i ) {
        m = &mine[i];
        if ( ( ( m->x<0 ) || ( m->x>1000 ) || ( m->y<0 ) || ( m->y>1000 )
                || ( m->yield<=0 ) ) && ( k<0 ) ) {
            k=i;
            break;
        }
    }
    if ( k>0 ) {
        m->x=x;
        m->y=y;
        m->detect=detectrange;
        m->yield = size;
        m->detonate=false;
    }
}

void Robot::com_transmit ( int chan, int data )
{
    for ( boost::ptr_vector<Robot>::iterator rr=robot.begin(),end=robot.end(); rr!=end; ++rr ) {
        if ( ( rr->n != n ) && ( rr->armor>0 ) && ( rr->channel = chan ) ) {
            if ( ( rr->ram[10]<0 ) || ( rr->ram[10]>max_queue ) ) {
                rr->ram[10]=0;
            }
            if ( ( rr->ram[11]<0 ) || ( rr->ram[11]>max_queue ) ) {
                rr->ram[11]=0;
            }
            rr->ram[rr->ram[11]+com_queue]=data;
            rr->ram[11] +=1;
            if ( rr->ram[11]>max_queue ) {
                rr->ram[11]=0;
            }
            if ( rr->ram[11]==rr->ram[10] ) {
                rr-ram[10] += 1;
            }
            if ( rr->ram[10]>max_queue ) {
                rr->ram[10]=0;
            }
        }
    }
}

int Robot::com_recieve()
{
    int k=0;
    if ( ram[10]!=ram[11] ) {
        if ( ( ram[10]<0 ) || ( ram[10]>max_queue ) ) {
            ram[10]=0;
        }
        if ( ( ram[11]<0 ) || ( ram[11]>max_queue ) ) {
            ram[11]=0;
        }
        k=ram[ram[10]+com_queue];
        ++ ( ram[10] );
        if ( ram[10]>max_queue ) {
            ram[10]=0;
        }
    } else {
        robot_error ( 12,"" );
    }
    return k;
}
