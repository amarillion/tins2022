#ifndef HISCORE_H
#define HISCORE_H

#include <string>
#include <memory>
#include "state.h"

// abstract
class HiScore : public State
{
public:
    virtual void init() = 0;
    virtual bool isHiscore (int score) = 0;
    virtual void add(int newScore) = 0;

    // factory
    static std::shared_ptr<HiScore> newInstance();
};

#endif
