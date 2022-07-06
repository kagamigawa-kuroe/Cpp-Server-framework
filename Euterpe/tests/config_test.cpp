//
// Created by 王泓哲 on 06/07/2022.
//

#include "../src/config/config.h"
#include "../src/Log/log.h"
#include <iostream>

void base_test(){

}


euterpe::ConfigVar<int>::ptr int_value_1 = euterpe::Config::Lookup("systemport",(int)8080,"ststemport");

int main(){
    EUTERPE_LOG_INFO(EUTERPE_LOG_ROOT()) << int_value_1->getValue();

}