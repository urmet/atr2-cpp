#include <iostream>
#include <boost/ptr_container/ptr_vector.hpp>

#include "atr2.h"
#include "missile.h"
#include "robot.h"
#include "atr2func.h"

boost::ptr_vector<missile> missiles;

using std::cout;
using std::endl;

static void remove_inactive()
{
//    std::cout<<"removing missiles - "<<missiles.size() <<"left"<<std::endl;
    for ( boost::ptr_vector<missile>::iterator m=missiles.begin(); m!=missiles.end(); ) {
        if ( m->active == 0 ) {
            m = missiles.erase ( m );
        } else {
            ++m;
        }
    }
}

void print_missiles()
{
    for ( boost::ptr_vector<missile>::iterator m=missiles.begin(), end=missiles.end();
            m!=end; ++m ) {
        std::cout<<"missile: "<<m->hd<<" a="<<m->active<<std::endl;
    }
}

void do_missiles()
{
    if ( !missiles.size() ) {
        return;
    }
    if ( game_cycle&0xff ) {
        remove_inactive();
    }
    for ( boost::ptr_vector<missile>::iterator m=missiles.begin(), end=missiles.end();
            m!=end; ++m ) {
//     cout<<"do_missile("<<n<<");"<<endl;
        float range,d,xv,yv;
        int k,l,xx,yy,tx,ty,dd,dam;
        bool source_alive;
        if ( m->active==1 ) { // normal missile
            //wall collision
            if ( ( ( m->x ) < ( -20.0 ) ) || ( ( m->x ) > ( 1020.0 ) )
                    || ( ( m->y ) < ( -20.0 ) ) || ( ( m->y ) > ( 1020.0 ) ) ) {
                m->active=0;
            }

            //move missile
            m->lx=m->x;
            m->ly=m->y;
            if ( m->active>0 ) { // did not collide to wall
                m->hd= ( m->hd+256 ) &255; // normalize
                xv = ( sint[m->hd] ) * ( m->mspd );
                yv = ( -cost[m->hd] ) * ( m->mspd );
//             cout<<"location: "<<m->x<<","<<m->y<<endl;
                ( m->x ) += xv;
                ( m->y ) += yv;
//             cout<<"location: "<<m->x<<","<<m->y<<endl;
            }

            // look for hit on a robot
            k=-1;
            l=maxint;
            for ( boost::ptr_vector<Robot>::iterator r = robot.begin(), end=robot.end();
                    r!=end; ++r ) {
                if ( r->armor>0 && r->n != m->source ) { // not dead and not the shooter
                    d = distance ( m->lx,m->ly,r->x,r->y );
                    xx = lrint ( sint[m->hd]*d+m->lx ); // ??
                    yy = lrint ( -cost[m->hd]*d+m->ly ); // ??
                    range = distance ( xx,yy,r->x,r->y );
                    if ( ( d<= ( m->mspd ) ) && ( range<hit_range ) && ( d<=l ) ) {
                        k=r->n;
                        l=lrint ( d );
                        dd=lrint ( range );
                        tx=xx;
                        ty=yy;
                    }
                }
            }

            if ( k>=0 ) { // boom!
//              cout<<"boom!"<<endl;
                m->x=tx; // move to place
                m->y=ty;
                m->active=2; // animation mode
                m->rad=0; // reset animation radius
                m->lrad=0;
                if ( ( m->source<=robot.size() ) ) {
                    robot[m->source].last_hit=0;
                    ++ ( robot[m->source].hits );
                }
                for ( boost::ptr_vector<Robot>::iterator r = robot.begin(), end=robot.end();
                        r!=end; ++r ) {
                    dd=lrint ( distance ( m->x,m->y,r->x,r->y ) );
                    if ( dd<=hit_range ) {
                        dam=lrint ( fabs ( ( float ) hit_range-dd ) * ( m->mult ) );
                        if ( dam<=0 ) {
                            dam=1;
                        }
                        kill_count=0;
                        if ( robot[m->source].armor>0 ) {
                            source_alive=true;
                        } else {
                            source_alive=false;
                        }
                        r->damage ( dam,false );
                        if ( ( m->source<robot.size() ) && ( m->source!=r->n ) ) {
                            robot[m->source].damage_total += dam;
                        }
                        if ( kill_count>0 && source_alive && ( robot[m->source].armor<=0 ) ) {
                            --kill_count;
                        }
                        if ( kill_count>0 ) {
                            robot[m->source].kills += kill_count;
                        }
                    }
                }
            }
        }
        if ( m->active==2 ) { // explosion animation. useless?
            m->lrad=m->rad;
            ++ ( m->rad );
            if ( ( m->rad ) > ( m->max_rad ) ) {
                m->active=0;
            }
        }
    }
}

void reset_missiles()
{
    missiles.clear();
}

