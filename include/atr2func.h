#ifndef ATR2FUNC_H
#define ATR2FUNC_H

#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <string>

extern bool registered, graphix,sound_on,silent;
extern float sint[256], cost[256];
extern const double pi;

const int16_t maxint = INT16_MAX;
const int16_t minint = INT16_MIN;

void textxy ( int x, int y, std::string s );
void coltextxy ( int x, int y, std::string s, char c );
char hexnum ( const char num );
std::string hexb ( const unsigned char num );
std::string hex ( unsigned int num );
float valuer ( std::string i );
long value ( std::string i );
std::string cstrr ( float i );
std::string cstr ( const long int i );
std::string zero_pad ( long n, long l );
std::string zero_pads ( std::string s, long l );
std::string addfront ( const std::string& b, const int l );
std::string addrear ( std::string b, unsigned int l );
std::string ucase ( std::string s );
std::string lcase ( std::string s );
std::string space ( char i );
std::string repchar ( char c, char i );
std::string ltrim ( std::string s1 );
std::string rtrim ( std::string s1 );
std::string btrim ( std::string s1 );
std::string lstr ( std::string s1, int l );
std::string rstr ( std::string s1, int l );
void FlushKey();
void calibrate_timing();

void time_delay ( int n );

int16_t sar ( int16_t n, uint k );
int16_t sal ( int16_t n, int16_t k );
uint16_t rol ( const uint16_t value, uint16_t shift );
uint16_t ror ( const uint16_t value, uint16_t shift );
void viewport ( int x1, int y1, int x2, int y2 );
void main_viewport();
void make_tables();
int robot_color ( int n );
void box ( int x1, int y1, int x2, int y2 );
void hole ( int x1, int y1, int x2, int y2 );
void chirp();
void click();
int hex2int ( std::string s );
int str2int ( std::string s );
double distance ( double x1, double y1, double x2, double y2 );
float find_angle ( float xx, float yy, float tx, float ty );

int find_anglei ( float xx, float yy, float tx, float ty );

std::string bin ( int n );

std::string decimal ( int num, int length );

char *atr2date();
char *atr2time();

void textcolor ( char );

#endif  /* ATR2FUNC_H */

