/*
 * pid.c
 *
 *  Created on: 05.09.2019
 *      Author: damian
 */
#include "pid.h"

int32_t Calc(uint32_t zadana,uint32_t zmierzona)
{
//    static int32_t poz_last=0,e_last=0;
//    static int64_t err=0;
//    int32_t ans,e,ev;
//    uint32_t delta=ENCODER_MAX_VAL/100;
//
//    //POZ
//    if(zadana>ENCODER_MAX_VAL-delta) {zadana=ENCODER_MAX_VAL-delta;} //ograniczenie zeby nie wszedl w rejony bliskie 0 i max
//    else if(zadana<delta) {zadana=delta;}
//
//    e = zadana-zmierzona; //uchyb
////    if(e<200 && e>-200)
////    {
////        err += e;
////        if(err>10000000) {err=10000000;}
////        else if(err<-10000000) {err=-10000000;}
////    }
////    else {err = 0;}
//
//    ans = e*1 + (e-e_last)*250;// + err/10000;//
//
//    e_last = e;
//
//    //VEL
//    if(ans>config.setvel) {ans=config.setvel;} //3000
//    else if(ans<-config.setvel) {ans=-config.setvel;} //3000
//
//    ev = ans-(zmierzona-poz_last);
//
//    ans = ev*1;
//
//    if(ans>0x7FFF) ans=0x7FFF;
//    else if(ans<-0x7FFF) ans=-0x7FFF;
//
//    poz_last = zmierzona;
//    return ans;
}


