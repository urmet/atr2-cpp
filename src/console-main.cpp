#include <iostream>
#include <ctime>

#include "atr2.h"
#include "atr2func.h"
#include "filelib.h"
#include "compiler.h"
#include "robot.h"

using std::cout;
using std::endl;

static bool report, show_cnotice;
static int report_type;

static const char*  progname       ="ATR2-cpp";
static const char*   version        ="0.0";
static const char*  cnotice1       ="Original idea by Ed T. Toton III";
// static const char*  main_filename  ="atr2-cpp";

static void init ( int argc, char **argv )
{
//     logging_errors = true;
    insane_missiles = false;
    insanity = 0;
    graphix = true;
    played = 0;
    show_cnotice = false;
    report = false;
    kill_count = 0;
    make_tables();
//     srand(0);
    srand ( time ( NULL ) );
    game_limit = 100000;
    game_cycle = 0;
    time_slice = default_slice;

    if ( argc > 1 ) {
        for ( int i = 1; i < argc; ++i ) {
            cout<<"parsing cmd param: "<<argv[i]<<endl;
            parse_param ( argv[i] );
        }
    } else {
        prog_error ( 5,"" );
    }
    if ( num_robots <= 1 && !testing ) {
        prog_error ( 4,"" );
    }
    // fixups
    if ( time_slice>100 ) {
        time_slice=100;
    }
    if ( time_slice<1 ) {
        time_slice=1;
    }
    if ( game_limit<0 ) {
        game_limit = 0;
    }
    if ( game_limit>100000 ) {
        game_limit = 100000;
    }
    if ( maxcode<1 ) {
        maxcode =1;
    }
    if ( maxcode>MAX_CODE ) {
        maxcode = MAX_CODE;
    }

    return;
}


void shutdown()
{
    if ( show_cnotice ) {
        textcolor ( 3 );
        cout<<progname<<" "<<version<<" ";
        cout<<cnotice1<<endl;
    }
    textcolor ( 7 );
    cout<<endl;
//     if ( logging_errors ) {
//         for ( int i=0; i<num_robots; ++i ) {
//             cout<<"Robot error-log created: "<<base_name ( robot[i].fn ) <<".ERR"<<endl;
//         }
//     }
    return;
}



void parse_param ( std::string s )
{
    std::ifstream f;
    std::string fn,s1,s2;
    bool found = false;
    std::string tmp = btrim ( s );
    if ( tmp == "" ) {
        return;
    }
    if ( tmp[0]=='#' ) {
        fn = rstr ( tmp, tmp.length()-1 );
        if ( fn==base_name ( fn ) ) {
            fn.append ( config_ext );
        }
        if ( !exist ( fn ) ) {
            prog_error ( 6,fn.c_str() );
        }
        f.open ( fn.c_str() );
        while ( !f.eof() ) {
            f>>s1;
            s2 = s1;
            s1 = ucase ( btrim ( s1 ) );
            if ( s1[1]=='#' ) {
                prog_error ( 7,s1.c_str() );
            } else {
                parse_param ( s2 );
            }
        }
        f.close();
        found = true;
    } else if ( ( tmp[0]=='/' ) || ( tmp[0]=='-' ) || ( tmp[0]=='=' ) ) {
        tmp = ucase ( rstr ( s,s.length()-1 ) );
        if ( tmp[0]=='T' ) {
            time_slice = value ( rstr ( tmp,tmp.length()-1 ) );
            found = true;
        }
        if ( tmp[0]=='L' ) {
            game_limit = value ( rstr ( tmp,tmp.length()-1 ) ) *1000;
            found = true;
        }
        if ( tmp[0]=='Q' ) {
            sound_on = false;
            found = true;
        }
        if ( tmp[0]=='N' ) {
            silent = true;
            found = true;
        }
        if ( tmp[0]=='M' ) {
            set_matches ( value ( rstr ( tmp,tmp.length()-1 ) ) );
            found = true;
        }
        if ( tmp[0]=='S' ) {
            show_source = false;
            found = true;
        }
        if ( tmp[0]=='R' ) {
            report = true;
            if ( tmp.length() >1 ) {
                report_type= value ( rstr ( tmp,tmp.length()-1 ) );
            }
            found = true;
        }
        if ( tmp[0]=='C' ) {
            compile_only=true;
            found = true;
        }
        if ( tmp[0]=='^' ) {
            show_cnotice = false;
            found = true;
        }
        if ( tmp[0]=='!' ) {
            insane_missiles = true;
            if ( tmp.length() >1 ) {
                insanity= value ( rstr ( tmp,tmp.length()-1 ) );
            }
            found = true;
        }

//         if ( tmp[0]=='E' ) {
//             logging_errors = true;
//             found = true;
//         }
        if ( tmp[0]=='Z' ) {
            testing = true;
            found = true;
        }
        if ( insanity<0 ) {
            insanity=0;
        }
        if ( insanity>15 ) {
            insanity = 15;
        }
    } else if ( tmp[0]==';' ) {
        found = true;
    } else if ( num_robots<MAX_ROBOTS && tmp.length() ) {
        ++num_robots;
        create_robot ( num_robots-1,tmp );
        found=true;
        if ( num_robots==MAX_ROBOTS ) {
            cout<<"Maximum number of robots reached."<<endl;
        }
    } else {
        prog_error ( 10,"" );
    }

    if ( !found ) {
        prog_error ( 8,tmp );
    }

    return;
}

int main ( int argc, char **argv )
{
    int leaders=0, n;
    unsigned wins=0;

    cout << progname << " " << version << endl;
    cout << cnotice1 << endl;
    init ( argc, argv );

    if ( matches>0 ) {
        for ( int i=0; i<matches; ++i ) {
            bout();
        }
    }
    cout<<endl;
    if ( matches>1 ) {
        cout<<endl<<endl;
        cout<<"Bout complete! ("<<matches<<" matches)"<<endl;
        cout<<endl;
        
        // determine the winner(s)
        for ( robot_it r=robot.begin(),end=robot.end(); r!=end; ++r ) {
            if ( r->wins==wins ) {
                ++leaders;
            }
            if ( r->wins>wins ) {
                leaders=1;
                n=r->n;
                wins=r->wins;
            }
        }
        
        cout<<"Robot           Wins  Matches  Kills  Deaths    Shots"<<endl;
        cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
        for ( robot_it r=robot.begin(),end=robot.end(); r!=end; ++r ) {
            textcolor ( robot_color ( r->n ) );
            cout<<addfront ( cstr ( r->n+1 ),2 ) <<" - "<<addrear ( r->fn,8 );
            cout<<addfront ( cstr ( r->wins ),7 ) <<addfront ( cstr ( r->trials ),8 );
            cout<<addfront ( cstr ( r->kills ),8 ) <<addfront ( cstr ( r->deaths ),8 );
            cout<<addfront ( cstr ( r->shots_fired ),9 ) <<endl;
        }


        textcolor ( 15 );
        cout<<endl;
        if ( leaders==1 ) {
            cout<<"Robot #"<<n+1<<" ("<<robot[n].fn<<") wins the bout!";
            cout<<" (score: "<<wins<<"/"<<matches<<")"<<endl;
        } else {
            cout<<"There is no clear victor!"<<endl<<endl;
        }
    }

    cout<<"Shutting down."<<endl;
    shutdown();
    return 0;
}
