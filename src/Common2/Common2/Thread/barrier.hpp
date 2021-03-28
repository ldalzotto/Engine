#pragma once

enum class BarrierTwoStep_State
{
    OPENED = 0,
    ASKING_FOR_SYNC_1 = 1,
    SYNC_1_REACHED = 2,
    ASKING_FOR_SYNC_2 = 3,
    SYNC_2_REACHED = 4
};

/*
    A BarrierTwoStep is a functional object that allows executing a piece piece of code when the _SYNC_1 point has been reached.
    Once the _SYNC_1 point is reached, further execution of the triggering thread is holded until _SYNC_2 point is reached.
*/
struct BarrierTwoStep
{
    BarrierTwoStep_State state;
};

inline void BarrierTwoStep_ask_and_wait_for_sync_1(BarrierTwoStep* thiz)
{
    while (thiz->state != BarrierTwoStep_State::OPENED)
    {
    }
    thiz->state = BarrierTwoStep_State::ASKING_FOR_SYNC_1;
    while (thiz->state != BarrierTwoStep_State::SYNC_1_REACHED)
    {
    }
};

inline void BarrierTwoStep_ask_for_sync_1(BarrierTwoStep* thiz)
{
    while (thiz->state != BarrierTwoStep_State::OPENED)
    {
    }
    thiz->state = BarrierTwoStep_State::ASKING_FOR_SYNC_1;
};

inline void BarrierTwoStep_wait_for_sync_1(BarrierTwoStep* thiz)
{
    while (thiz->state != BarrierTwoStep_State::SYNC_1_REACHED)
    {
    }
};

inline void BarrierTwoStep_notify_sync_1_and_wait_for_sync_2(BarrierTwoStep* thiz)
{
    thiz->state = BarrierTwoStep_State::SYNC_1_REACHED;
    while (thiz->state != BarrierTwoStep_State::SYNC_2_REACHED)
    {
    }
    thiz->state = BarrierTwoStep_State::OPENED;
};

inline void BarrierTwoStep_notify_sync_2(BarrierTwoStep* thiz)
{
    thiz->state = BarrierTwoStep_State::SYNC_2_REACHED;
};

inline int8 BarrierTwoStep_is_opened(BarrierTwoStep* thiz)
{
    return thiz->state == BarrierTwoStep_State::OPENED;
};