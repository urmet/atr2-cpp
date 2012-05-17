#ifndef MISSILE_H
#define MISSILE_H

#include <boost/ptr_container/ptr_vector.hpp>

#define MAX_MISSILES 1024

struct missile {
    float x,y,lx,ly,mult,mspd;
    unsigned source;
    int active,hd,rad,lrad,max_rad;
};

extern boost::ptr_vector<missile> missiles;


// activate a missile
// void init_missile ( float xx, float yy, int dir, int s, int blast, bool ob );

void do_missiles();
void reset_missiles();

#endif
