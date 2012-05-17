#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>

#include "compiler.h"
#include "robot.h"
#include "atr2func.h"
#include "atr2.h"
#include "filelib.h"
#include "opcodes.h"

using namespace std;

unsigned numvars, numlabels, maxcode=MAX_CODE, lock_pos, lock_dat;
std::string varname[MAX_VARS];
int varloc[MAX_VARS];
std::string labelname[MAX_VARS];
int labelnum[MAX_LABELS];
bool debugging_compiler = false, compile_by_line = false, show_code = false;
bool show_source, compile_only;
std::string lock_code;

struct parsed_token {
  union {
    int opcode;
    Opcode op;
  };
    int microcode;
};

//teh labelmap
static std::map<std::string,int> labelmap;

static const unsigned int MAX_OP=3;
typedef std::string parsetype[MAX_OP];

static parsed_token parse_token ( string s );

static void check_plen ( unsigned );

static std::map<std::string, int> init_map()
{
    std::map<std::string,int> ret;
    ret["MININT"]=-32768;
    ret["MAXINT"]=32767;
    ret["NOP"]=0;
    ret["I_DESTRUCT"]=0;
    ret["ADD"]=1;
    ret["P_SPEDOMETER"]=1;
    ret["I_RESET"]=1;
    ret["SUB"]=2;
    ret["P_HEAT"]=2;
    ret["I_LOCATE"]=2;
    ret["OR"]=3;
    ret["P_COMPASS"]=3;
    ret["I_KEEPSHIFT"]=3;
    ret["AND"]=4;
    ret["P_TURRET_OFS"]=4;
    ret["I_OVERBURN"]=4;
    ret["XOR"]=5;
    ret["P_TURRET_ABS"]=5;
    ret["I_ID"]=5;
    ret["NOT"]=6;
    ret["P_DAMAGE"]=6;
    ret["P_ARMOR"]=6;
    ret["I_TIMER"]=6;
    ret["MPY"]=7;
    ret["P_SCAN"]=7;
    ret["I_ANGLE"]=7;
    ret["DIV"]=8;
    ret["P_ACCURACY"]=8;
    ret["I_TID"]=8;
    ret["I_TARGETID"]=8;
    ret["MOD"]=9;
    ret["P_RADAR"]=9;
    ret["I_TINFO"]=9;
    ret["I_TARGETINFO"]=9;
    ret["RET"]=10;
    ret["P_RANDOM"]=10;
    ret["P_RAND"]=10;
    ret["I_GINFO"]=10;
    ret["I_GAMEINFO"]=10;
    ret["GSB"]=11;
    ret["CALL"]=11;
    ret["P_THROTTLE"]=11;
    ret["I_RINFO"]=11;
    ret["I_ROBOTINFO"]=11;
    ret["JMP"]=12;
    ret["GOTO"]=12;
    ret["P_OFS_TURRET"]=12;
    ret["P_TROTATE"]=12;
    ret["I_COLLISIONS"]=12;
    ret["JLS"]=13;
    ret["JB"]=13;
    ret["P_ABS_TURRET"]=13;
    ret["P_TAIM"]=13;
    ret["I_RESETCOLCNT"]=13;
    ret["JGR"]=14;
    ret["JA"]=14;
    ret["P_STEERING"]=14;
    ret["I_TRANSMIT"]=14;
    ret["JNE"]=15;
    ret["P_WEAP"]=15;
    ret["P_WEAPON"]=15;
    ret["P_FIRE"]=15;
    ret["I_RECEIVE"]=15;
    ret["JEQ"]=16;
    ret["JE"]=16;
    ret["P_SONAR"]=16;
    ret["I_DATAREADY"]=16;
    ret["XCHG"]=17;
    ret["SWAP"]=17;
    ret["P_ARC"]=17;
    ret["P_SCANARC"]=17;
    ret["I_CLEARCOM"]=17;
    ret["DO"]=18;
    ret["P_OVERBURN"]=18;
    ret["I_KILLS"]=18;
    ret["I_DEATHS"]=18;
    ret["LOOP"]=19;
    ret["P_TRANSPONDER"]=19;
    ret["I_CLEARMETERS"]=19;
    ret["CMP"]=20;
    ret["P_SHUTDOWN"]=20;
    ret["TEST"]=21;
    ret["P_CHANNEL"]=21;
    ret["SET"]=22;
    ret["MOV"]=22;
    ret["P_MINELAYER"]=22;
    ret["LOC"]=23;
    ret["P_MINETRIGGER"]=23;
    ret["GET"]=24;
    ret["P_SHIELD"]=24;
    ret["P_SHIELDS"]=24;
    ret["PUT"]=25;
    ret["INT"]=26;
    ret["IPO"]=27;
    ret["IN"]=27;
    ret["OPO"]=28;
    ret["OUT"]=28;
    ret["DEL"]=29;
    ret["DELAY"]=29;
    ret["PUSH"]=30;
    ret["POP"]=31;
    ret["ERR"]=32;
    ret["ERROR"]=32;
    ret["INC"]=33;
    ret["DEC"]=34;
    ret["SHL"]=35;
    ret["SHR"]=36;
    ret["ROL"]=37;
    ret["ROR"]=38;
    ret["JZ"]=39;
    ret["JNZ"]=40;
    ret["JAE"]=41;
    ret["JGE"]=41;
    ret["JBE"]=42;
    ret["JLE"]=42;
    ret["SAL"]=43;
    ret["SAR"]=44;
    ret["NEG"]=45;
    ret["JTL"]=46;
    ret["I_SUCCESS"]=98;
    ret["I_ERROR"]=99;
    return ret;
}



static void check_plen ( unsigned plen )
{

    char err[100];
    if ( plen > maxcode ) {
        sprintf ( err, "Maximum program lenght exceeded, (Limit: '%d' compiled lines)", maxcode );
        prog_error ( 16, err );
    }
}


void compile ( Robot &r, std::string filename )
{
    parsetype pp;
    std::string s,s1,s2,s3,orig_s,msg;
    unsigned i,j;
    int k,linecount,mask,locktype,l;
    char c,lc=0;
    lock_code="";
    lock_pos=0;
    locktype=0;
    lock_dat=0;
    if ( !exist ( filename ) ) {
        prog_error ( 8,filename );
    }
//     textcolor(robot_color(n));
    cout<<"Compiling robot #"<<r.n+1<<": "<<filename<<endl;
    r.is_locked=false;
//     textcolor(robot_color(n));
    numvars=0;
    numlabels=0;
    labelmap.clear();
    for ( k=0; k<MAX_CODE; ++k ) {
        for ( i=0; i<=MAX_OP; ++i ) {
            r.code[k].op=NOP;
            r.code[k].arg1=0;
            r.code[k].arg2=0;
            r.code[k].microcode=0;
        }
    }
    r.plen=0;
    std::ifstream f;
    f.open ( filename.c_str() );
    s="";
    linecount=0;

    //first pass - compile

    while ( !f.eof() && s!="#END" ) {
        char buf[1024];
        f.getline ( buf,1023 );
        s=buf;
        ++linecount;
        s=btrim ( s );
        orig_s = s;
        for ( i=0; i<s.length(); ++i ) {
            unsigned char tmp = s.at ( i );
            if ( ( tmp<=32 ) || ( tmp>128 ) ) {
                s[i]=' ';
            }
        }
        if ( !silent ) {
            if ( show_source && ( ( lock_code=="" ) ||debugging_compiler ) ) {
                cout<<zero_pad ( linecount,3 ) <<":"<<zero_pad ( r.plen,3 ) <<" "<<s<<endl;
            }
        }
        k=0;
        for ( i=s.length(); i>0; --i ) {
            if ( s[i-1]==';' ) {
                k=i-1;
            }
        }
        if ( k>0 ) {
            s= lstr ( s,k-1 ); // cut off comment, but leave the ';'
        }
        s=btrim ( ucase ( s ) );
        for ( i=0; i<MAX_OP; ++i ) {
            pp[i]="";
        }
        if ( s.length() >0 && s[0]!=';' ) { // line has more than just a comment
            if ( s[0]=='#' ) {
                s1=ucase ( btrim ( rstr ( s,s.length()-1 ) ) );
                msg=btrim ( rstr ( orig_s,orig_s.length()-5 ) );
                k=0;

                // search for first space on line
                if ( ( k = s1.find ( ' ' ) ) ==string::npos ) {
                    k=0;
                } else {
                    ++k;
                }
                // --k;
                if ( k>0 ) {
                    s2=lstr ( s1,k-1 );
                    s3=ucase ( btrim ( rstr ( s1,s1.length()-k ) ) );
                    k=0;
                    if ( numvars>0 ) {
                        for ( i=0; i<numvars; ++i ) {
                            if ( s3==varname[i] ) {
                                k=i;
                            }
                        }
                    }
                    if ( s2=="DEF" && ( numvars<MAX_VARS ) ) {
                        if ( s3.length() >MAX_VAR_LEN ) {
                            prog_error ( 12,s3 );
                        } else if ( k>0 ) {
                            prog_error ( 11,s3 );
                        } else {
                            ++numvars;
                            if ( numvars>MAX_VARS ) {
                                prog_error ( 14,"" );
                            } else {
//                            cout<<"adding variable #"<<numvars<<" "<<s3<<endl;
                                varname[numvars-1]=s3;
                                varloc[numvars-1]= ( 127+numvars );
                            }
                        }
                    } else if ( lstr ( s2,4 ) =="LOCK" ) {
                        r.is_locked=true;
                        if ( s2.length() >4 ) {
                            locktype=value ( rstr ( s2,s2.length()-4 ) );
                            lock_code=btrim ( ucase ( s3 ) );
                            cout<<"Robot is of LOCKed format from this point forward. ["<<locktype<<"]"<<endl;
                            cout<<"Using key: \""<<lock_code<<"\""<<endl;
                            for ( i=0; i<lock_code.length(); ++i ) {
                                lock_code[i]= ( ( char ) ( lock_code[i] )-65 );
                            }

                        }
                    } else if ( s2=="MSG" ) {
                        r.name=msg;
                    } else if ( s2=="TIME" ) {
                        int tmp= value ( s3 );
                        r.robot_time_limit= ( ( tmp>=0 ) ?tmp:0 );


                    } else if ( s2=="CONFIG" ) {

                        if ( lstr ( s3,8 ) =="SCANNER=" ) {
                            r.config.scanner = value ( rstr ( s3,s3.length()-8 ) );
                        } else if ( lstr ( s3,7 ) =="SHIELD=" ) {
                            r.config.shield = value ( rstr ( s3,s3.length()-7 ) );
                        } else if ( lstr ( s3,7 ) =="WEAPON=" ) {
                            r.config.weapon = value ( rstr ( s3,s3.length()-7 ) );
                        } else if ( lstr ( s3,6 ) =="ARMOR=" ) {
                            r.config.armor = value ( rstr ( s3,s3.length()-6 ) );
                        } else if ( lstr ( s3,7 ) =="ENGINE=" ) {
                            r.config.engine = value ( rstr ( s3,s3.length()-7 ) );
                        } else if ( lstr ( s3,10 ) =="HEATSINKS=" ) {
                            r.config.heatsinks = value ( rstr ( s3,s3.length()-10 ) );
                        } else if ( lstr ( s3,6 ) =="MINES=" ) {
                            r.config.mines = value ( rstr ( s3,s3.length()-6 ) );
                        } else {
                            prog_error ( 20,s3 );
                        }

                        config_rec *c = & ( r.config );
                        if ( c->scanner<0 ) {
                            c->scanner=0;
                        }
                        if ( c->scanner>5 ) {
                            c->scanner=5;
                        }
                        if ( c->shield<0 ) {
                            c->shield=0;
                        }
                        if ( c->shield>5 ) {
                            c->shield=5;
                        }
                        if ( c->weapon<0 ) {
                            c->weapon=0;
                        }
                        if ( c->weapon>5 ) {
                            c->weapon=5;
                        }
                        if ( c->armor<0 ) {
                            c->armor=0;
                        }
                        if ( c->armor>5 ) {
                            c->armor=5;
                        }
                        if ( c->engine<0 ) {
                            c->engine=0;
                        }
                        if ( c->engine>5 ) {
                            c->engine=5;
                        }
                        if ( c->heatsinks<0 ) {
                            c->heatsinks=0;
                        }
                        if ( c->heatsinks>5 ) {
                            c->heatsinks=5;
                        }
                        if ( c->mines<0 ) {
                            c->mines=0;
                        }
                        if ( c->mines>5 ) {
                            c->mines=5;
                        }
                    } else {
                        cout<<"Warning: unknown directive \""<<s2<<"\""<<endl;
                    }

                }
            } // if("#")
            else if ( s[0]=='*' ) { // inline pre-compiled machine code
                check_plen ( r.plen );
                for ( i=0; i<MAX_OP; ++i ) {
                    pp[i]="";
                }
                for ( i=1; i<s.length(); ++i )
                    if ( s[i]=='*' ) {
                        prog_error ( 23,s );
                    }
                k=0;
                i=0;
                s1="";
                if ( s.length() >=2 ) {
                    prog_error ( 22,s );
                }
                while ( i<s.length() ) {
                    ++i;
                    unsigned char tmp = s[i];
                    unsigned char tmp2 =s[i-1];
                    if ( ( tmp>=33&&tmp<=41 ) || ( tmp>=43&&tmp<=127 ) ) {
                        pp[k]=pp[k]+s[i];
                    } else if ( ( ( tmp<=32 ) || ( tmp>=128 ) ) &&
                                ( ( tmp2>=33&&tmp2<=41 ) || ( tmp2>=43&&tmp2<=127 ) ) ) {
                        ++k;
                    }
                }
                r.code[r.plen].op=Opcode ( str2int ( pp[0] ) );
                r.code[r.plen].arg1=str2int ( pp[1] );
                r.code[r.plen].arg2=str2int ( pp[2] );
                ++ ( r.plen );
            } // if("*")
            else if ( s[0]==':' ) { // labels
                check_plen ( r.plen );
                s1=rstr ( s,s.length()-1 );

                for ( i=0; i<s1.length(); ++i ) {
                    if ( ! ( s1[i]>='0' ) && ( s1[i]<='9' ) ) {
                        prog_error ( 1,s );
                    }
                }
                r.code[r.plen].op= ( Opcode ) str2int ( s1 );
//                 cout<<"adding :label - str2int(s1)="<<str2int ( s1 ) <<endl;
                r.code[r.plen].microcode=2;
                if ( show_code ) {
                    print_code ( r,r.plen );
                }
                ++ ( r.plen );
            } // if(":")
            else if ( s[0]=='!' ) {
                check_plen ( r.plen );
                s1=btrim ( rstr ( s,s.length()-1 ) ); // ucase and remove the !
                k=0;
                for ( i=s.length(); i>0; --i ) {
                    switch ( s1[i-1] ) {
                    case ';':
                    case 8:
                    case 9:
                    case 10:
                    case ' ':
                    case ',':
                        k=i-1;
                        break;
                    }
                }
                if ( k>0 ) {
                    s1=lstr ( s1,k-1 );
                }
                k=0;

                if ( labelmap.count ( s1 ) ) {
                    if ( labelmap[s1]>=0 ) { // label already defined!
                        prog_error ( 13,std::string ( "\"!" ) +s1+" ("+cstr ( labelmap[s1] ) +")" );
                    }
                }
                labelmap[s1]=r.plen;
//                 cout<<"inserrrt "<<s1<<endl;

            }// if("!")
            else { // instructions/numbers
                check_plen ( r.plen );
                //parse instruction
                //remove comments
                k=0;
                for ( i=s.length(); i>0; --i ) {
                    if ( s[i-1]==';' ) {
                        k=i-1;
                    }
                }

                if ( k>0 ) {
                    s=lstr ( s,k-1 );
                }

                //setup variables for parsing
                k=0;
                for ( j=0; j<MAX_OP; ++j ) {
                    pp[j]="";
                }
                for ( j=0; j<s.length(); ++j ) {
                    c=s[j];
                    if ( ( ! ( c==' '||c==8||c==9||c==10||c==',' ) ) && k<MAX_OP ) {
                        pp[k]=pp[k]+c;
                    } else if ( ! ( lc==' '||lc==8||lc==9||lc==10||lc==',' ) ) {
                        ++k;
                    }
                    lc=c;
                }
                parsed_token tmp = parse_token ( pp[0] );
                r.code[r.plen].op = tmp.op;
                r.code[r.plen].microcode = tmp.microcode;
                tmp = parse_token ( pp[1] );
                r.code[r.plen].arg1 = tmp.opcode;
                r.code[r.plen].microcode |= tmp.microcode<<4;
                tmp = parse_token ( pp[2] );
                r.code[r.plen].arg2 = tmp.opcode;
                r.code[r.plen].microcode |= tmp.microcode<<8;

                if ( show_code ) {
                    print_code ( r,r.plen );
                }

                ++ ( r.plen );

            }
        }
    } // while(!f.eof())
    f.close();

    // add a NOP at program end, if there's room
    if ( r.plen<=MAX_CODE ) {
        r.code[r.plen].op=NOP;
    } else {
        -- ( r.plen );
    }

    // second pass. resolving !labels
    if ( labelmap.size() >0 ) {
        for ( i=0; i<=r.plen; ++i ) {
            for ( j=0; j<MAX_OP; ++j ) {
                if ( ( ( r.code[i].microcode ) >> ( j*4 ) ) ==3 ) { //unresolved label
                    if(j==0) k=r.code[i].op;
                    if(j==1) k=r.code[i].arg1;
                    if(j==2) k=r.code[i].arg2;
                    if ( k>0 ) {
                        l=labelnum[k+1];
                        if ( l<0 ) {
                            prog_error ( 19,std::string ( "\"!" ) +labelname[k]+"\" ("+cstr ( l ) +")" );
                        }
                        if ( l<0 || ( unsigned ) l>maxcode ) {
                            prog_error ( 18,std::string ( "\"!" ) +labelname[k]+" ("+cstr ( l ) +")" );
                        } else {
                            r.code[i].op= ( Opcode ) l;
                            mask= ~ ( 0xF<< ( j*4 ) );
                            r.code[i].microcode= ( ( ( r.code[i].microcode ) & mask ) | ( 4<< ( j*4 ) ) );
                        }
                    } else {
                        std::stringstream ss;
                        ss<<k;
                        prog_error ( 17,ss.str() );
                    }
                }
            }
        }
        textcolor ( 7 );
    }
//     for(int p=0; p<r->plen;i++)
//     {
//       cout<<hex(r->code[p].op[0])<<" "<<hex(r->code[p].op[1])<<" ";
//       cout<<hex(r->code[p].op[2])<<" "<<hex(r->code[p].op[3])<<" "<<endl;
//     }
    return;
}



static parsed_token parse_token ( string s )
{

    static std::map<std::string, int> constant_map = init_map();

    parsed_token ret;
    unsigned j=0;
    bool found,indirect;
    string ss;
    found=false;
    ret.opcode=0;   //nop
    ret.microcode=0; //{instruction/constant}
    s=btrim ( ucase ( s ) ); // normalize input
    indirect=false;

    /*
    Microcode:
       0 = instruction, number, constant
       1 = variable, memory access
       2 = :label
       3 = !label (unresolved)
       4 = !label (resolved)
      8h mask = inderect addressing (enclosed in [])
    */

    if ( s=="" ) {
        ret.opcode=0;
        ret.microcode=0;
        return ret;
    }

    if ( ( lstr ( s,1 ) =="[" ) && ( rstr ( s,1 ) =="]" ) ) {
        s=s.substr ( 1 );
        s=s.substr ( 0,s.length()-1 );
        indirect=true;
    }

    // !labels
    if ( ( !found ) && ( s[0]=='!' ) ) {
        ss=s;
        ss=btrim ( rstr ( ss,ss.length()-1 ) );
        if ( labelmap.size() ) {
            found = labelmap.count ( ss );
            if ( found ) {
                if ( labelmap[ss]>=0 ) {
                    ret.opcode=labelmap[ss];
                    ret.microcode=4; // resolved
                } else {
                    ret.opcode=j;
                    ret.microcode=3; // unresolved
                }
                return ret;
            }
        }
        if ( !found ) {
            ++numlabels;
            if ( numlabels>MAX_LABELS ) {
                prog_error ( 15,"" );
            } else {
                labelmap[ss]=-1;
                ret.opcode=numlabels;
                ret.microcode=3; // {unresolved !label}
                found=true;
            }
        }
    }

    // {variables}
    if ( ( numvars>0 ) && ( !found ) )
        for ( j=0; j< numvars; ++j ) {
//            cout<<"variables s[i]==varname[j] "<<s[i]<<"=="<<varname[j]<<endl;
            if ( s==varname[j] ) {
                ret.opcode=varloc[j];
                ret.microcode=1; // {variable}
                found=true;
            }
        }
    // constants
    // as in opcodes and port/interrupt names
    if ( constant_map.count ( s ) ) {
        ret.opcode=constant_map[s];
        found=true;
    }

    //{registers}
    if ( s=="COLCNT" ) {
        ret.opcode=8;
        ret.microcode=01;
        found=true;
    }
    if ( s=="METERS" ) {
        ret.opcode=9;
        ret.microcode=01;
        found=true;
    }
    if ( s=="COMBASE" ) {
        ret.opcode=10;
        ret.microcode=01;
        found=true;
    }
    if ( s=="COMEND" ) {
        ret.opcode=11;
        ret.microcode=01;
        found=true;
    }
    if ( s=="FLAGS" ) {
        ret.opcode=64;
        ret.microcode=01;
        found=true;
    }
    if ( s=="AX" ) {
        ret.opcode=65;
        ret.microcode=01;
        found=true;
    }
    if ( s=="BX" ) {
        ret.opcode=66;
        ret.microcode=01;
        found=true;
    }
    if ( s=="CX" ) {
        ret.opcode=67;
        ret.microcode=01;
        found=true;
    }
    if ( s=="DX" ) {
        ret.opcode=68;
        ret.microcode=01;
        found=true;
    }
    if ( s=="EX" ) {
        ret.opcode=69;
        ret.microcode=01;
        found=true;
    }
    if ( s=="FX" ) {
        ret.opcode=70;
        ret.microcode=01;
        found=true;
    }
    if ( s=="SP" ) {
        ret.opcode=71;
        ret.microcode=01;
        found=true;
    }

    // {memory addresses}
    if ( ( !found ) && ( s[0]=='@' ) && ( s[1]>='0' && s[1]<='9' ) ) {
        ret.opcode=str2int ( rstr ( s,s.length()-1 ) );
        if ( ( ret.opcode<0 ) or ( ret.opcode> ( MAX_RAM+ ( ( MAX_CODE ) <<3 ) ) ) ) {
            prog_error ( 3,s );
        }
        ret.microcode=1; // {variable}
        found=true;
    }

    // {numbers}
    if ( ( !found ) && ( ( s[0]>='0' && s[0]<='9' ) || ( s[0]=='-' ) ) ) {
        ret.opcode=str2int ( s );
        found=true;
    }

    if ( found ) {
        if ( indirect ) {
            ret.microcode |= 8;
        }
    } else if ( !s.empty() ) {
        prog_error ( 2,s );
    }
    return ret;

}

void print_code ( Robot &r, int p )
{
    cout<<hex ( p ) <<": ";

    cout<<zero_pad ( r.code[p].op,5 ) <<" ";
    cout<<zero_pad ( r.code[p].arg1,5 ) <<" ";
    cout<<zero_pad ( r.code[p].arg2,5 ) <<" ";
    cout<<zero_pad ( r.code[p].microcode,5 ) <<" ";

    cout<<" = ";
    cout<<hex ( r.code[p].op ) <<"h ";
    cout<<hex ( r.code[p].arg1 ) <<"h ";
    cout<<hex ( r.code[p].arg2 ) <<"h ";
    cout<<hex ( r.code[p].microcode ) <<"h ";
    cout<<endl<<endl;
    return;
}

