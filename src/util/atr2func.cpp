#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <cstdio>


#include <boost/lexical_cast.hpp>
#include "atr2func.h"

const double pi = 3.1415926535897932384626433832795;

using std::string;
using std::cout;
using std::endl;

float sint[256], cost[256];
long delay_per_sec;
bool registered, graphix = false, sound_on, silent;
string reg_name;
int reg_num;

string cstr ( const long int i )
{
    string r = boost::lexical_cast<std::string> ( i );
    return r;
}


float valuer ( std::string i )
{
    float r;
    std::stringstream ss ( i );
    ss>>r;
    return r;
}

long value ( std::string i )
{
    return boost::lexical_cast<long> ( i );
}

std::string addfront ( const std::string &b, const int l )
{
    std::stringstream ss;
    ss<<std::setw ( l );
    ss<<b.c_str();
    return string ( ss.str() );
}

std::string addrear ( std::string b, unsigned int l )
{
    while ( b.length() <l ) {
        b.append ( " " );
    }
    return b;
}

void main_viewport()
{

}


int memavailz()
{
    return 0;
}

string date()
{
    char tmp[25];
    std::time_t tim;
    std::tm *s_tm;
    tim = std::time ( NULL );
    s_tm = std::localtime ( &tim );
    std::strftime ( tmp, 24, "%F", s_tm );
    return std::string ( tmp );
}

void make_tables()
{
    for ( int i = 0; i < 256; ++i ) {
        sint[i] = sin ( ( double ) i/128*pi );
        cost[i] = cos ( ( double ) i/128*pi );
    }
}
std::string ucase ( std::string s )
{
    std::string ret ( s );
    std::transform ( ret.begin(), ret.end(),ret.begin(), ::toupper );
    return ret;
}

int robot_color ( int /*n*/ )
{
    // cout << "STUB: " << __FUNCTION__ << endl;
    return 0; /* stub */
}
std::string zero_pad ( long n, long l )
{
    std::string s = cstr ( n );
    std::stringstream ss;
    for ( int i=l-s.length(); i>0; --i ) {
        ss<<"0";
    }
    ss<<s;
    return ss.str();
}

std::string ltrim ( std::string s1 )
{
    std::string tmp = s1;
    while ( tmp.length() >0 &&
            ( ( tmp.substr ( 0,1 ) ==" " ) ||
              ( tmp.substr ( 0,1 ) =="\b" ) ||
              ( tmp.substr ( 0,1 ) =="\t" ) ) ) {
        tmp = tmp.substr ( 1 );
    }

    return tmp;
}

std::string rtrim ( std::string s1 )
{
    std::string tmp = s1;
    while ( tmp.length() >0 &&
            ( ( tmp.substr ( tmp.length()-1,1 ) ==" " ) ||
              ( tmp.substr ( tmp.length()-1,1 ) =="\b" ) ||
              ( tmp.substr ( tmp.length()-1,1 ) =="\t" ) ) ) {
        tmp = tmp.substr ( 0,tmp.length()-1 );
    }

    return tmp;
}

std::string btrim ( std::string s1 )
{
    return ltrim ( rtrim ( s1 ) );
}

std::string lstr ( std::string s1, int l )
{
    std::string tmp;
    if ( s1.length() <=1 ) {
        tmp = s1;
    } else {
        tmp = s1.substr ( 0,l );
    }

    return tmp;
}

std::string rstr ( std::string s1, int l )
{
    std::string tmp;
    if ( s1.length() <=1 ) {
        tmp = s1;
    } else {
        tmp = s1.substr ( s1.length()-l );
    }

    return tmp;
}

double distance ( double x1, double y1, double x2, double y2 )
{
//   cout<<"distance between "<<x1<<","<<y1<<" "<<x2<<","<<y2<<" is:";
    double ret = fabs ( sqrt ( pow ( y1-y2,2 ) + pow ( x1-x2,2 ) ) );
//     cout<<ret<<endl;
    return ret;
}

int hex2int ( string s )
{
    std::stringstream ss;
    int ret;
    ss<<std::hex<<s;
    ss>>ret;
    return ret;
}


int str2int ( std::string s )
{
    long k;
    bool neg =false;
    std::string s1 = btrim ( ucase ( s ) );
    if ( s1.empty() ) {
        k=0;
    } else {
        if ( s[0]=='-' ) {
            neg = true;
            s1=rstr ( s1,s1.length()-1 );
        }
        k=0;
        if ( lstr ( s1,2 ) =="0X" ) {
            k=hex2int ( rstr ( s1,s1.length()-2 ) );
        } else if ( rstr ( s1,1 ) =="H" ) {
            k=hex2int ( lstr ( s,s.length()-1 ) );
        } else {
            k=value ( s1 );
        }
        if ( neg ) {
            k=0-k;
        }
    }
    return k;
}

void textcolor ( char )
{
    return;
}

char hexnum ( const char num )
{
    char h;
    switch ( num ) {
    case  0:
        h='0';
        break;
    case  1:
        h='1';
        break;
    case  2:
        h='2';
        break;
    case  3:
        h='3';
        break;
    case  4:
        h='4';
        break;
    case  5:
        h='5';
        break;
    case  6:
        h='6';
        break;
    case  7:
        h='7';
        break;
    case  8:
        h='8';
        break;
    case  9:
        h='9';
        break;
    case 10:
        h='A';
        break;
    case 11:
        h='B';
        break;
    case 12:
        h='C';
        break;
    case 13:
        h='D';
        break;
    case 14:
        h='E';
        break;
    case  15:
        h='F';
        break;
    default:
        h='X';

    }
    return h;
}


string hexb ( const unsigned char num )
{
    std::stringstream ss;
    ss<<hexnum ( num>>4 ) <<hexnum ( num&15 );
    return ss.str();
}


string hex ( unsigned int num )
{
    return hexb ( ( unsigned char ) num>>8 ) +hexb ( num&255 );
}

void chirp()
{

}

void click()
{

}

void calibrate_timing()
{
    return;
}

int find_anglei ( float xx, float yy, float tx, float ty )
{
    int i = rint ( find_angle ( xx,yy,tx,ty ) /pi*128+256 );
    while ( i<0 ) {
        i += 256;
    }
    i &= 255;
    return i;
}


float find_angle ( float xx, float yy, float tx, float ty )
{
    double v,z;
    double q=0;

    v=fabs ( ( tx-xx ) ); //horisontaalne kaugus
    if ( v==0 ) { // asuvad samal vertikaaljoonel
        if ( /*(tx==xx)&&*/ ( ty>yy ) ) { // üleval
            q=pi;
        }
        if ( /*(tx==xx)&&*/ ( ty<=yy ) ) { // all
            q=0;
        }
    } else {
        z=fabs ( ( ty-yy ) ); // kõrguste vahe
//         cout<<"v="<<v<<" z="<<z;
        q=fabs ( atan ( z/v ) );
        if ( ( tx>xx ) && ( ty>yy ) ) {
            q=pi/2+q;
        } else if ( ( tx>xx ) && ( ty<yy ) ) {
            q=pi/2-q;
        } else if ( ( tx<xx ) && ( ty<yy ) ) {
            q=pi+pi/2+q;
        } else if ( ( tx<xx ) && ( ty>yy ) ) {
            q=pi+pi/2-q;
        } else if ( ( tx=xx ) && ( ty>yy ) ) {
            q=pi/2;
        } else if ( ( tx=xx ) && ( ty<yy ) ) {
            q=0;
        } else if ( ( tx<xx ) && ( ty=yy ) ) {
            q=pi+pi/2;
        } else if ( ( tx>xx ) && ( ty=yy ) ) {
            q=pi/2;
        }
    }
//     cout<<"arc = "<<q<<endl;
    return q;
}

int16_t sar ( int16_t n, uint k )
{
    unsigned mask=1<<15;
    unsigned ret= ( unsigned ) n>>k;
    ret |= ( mask&n );
    return ret;
}

uint16_t rol ( const uint16_t value, uint16_t shift )
{
    if ( ( shift &= sizeof ( value ) *8 - 1 ) == 0 ) {
        return value;
    }
    return ( value << shift ) | ( value >> ( sizeof ( value ) *8 - shift ) );

}

uint16_t ror ( const uint16_t value, uint16_t shift )
{
    if ( ( shift &= sizeof ( value ) *8 - 1 ) == 0 ) {
        return value;
    }
    return ( value >> shift ) | ( value << ( sizeof ( value ) *8 - shift ) );
}

int16_t sal ( int16_t n, int16_t k )
{
    return n<<k;
}

