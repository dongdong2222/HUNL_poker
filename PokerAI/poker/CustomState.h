#pragma once
#include "State.h"


class CustomPokerstate: public Pokerstate{
public:
    CustomPokerstate(PokerTable _table, Engine* _engine=NULL, int _small_blind = 50, int _big_blind = 100)
    : Pokerstate(_table, _engine, _small_blind, _big_blind)
    {

    }
    

};