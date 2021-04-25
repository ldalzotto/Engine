#pragma once

/*
    A BarrierTwoStep is a functional object that allows executing a piece piece of code when the _SYNC_1 point has been reached.
    Once the _SYNC_1 point is reached, further execution of the triggering thread is holded until _SYNC_2 point is reached.
*/
struct BarrierTwoStep
{
    enum class State : int8
    {
        OPENED = 0,
        ASKING_FOR_SYNC_1 = 1,
        SYNC_1_REACHED = 2,
        ASKING_FOR_SYNC_2 = 3,
        SYNC_2_REACHED = 4
    };

    volatile State state;

#if 1
    inline BarrierTwoStep()
    {
        this->state = State::OPENED;
    };
    inline BarrierTwoStep(const BarrierTwoStep& p_other)
    {
        this->state = p_other.state;
    };

    inline BarrierTwoStep(const volatile BarrierTwoStep& p_other)
    {
        this->state = p_other.state;
    };
#endif

    inline volatile BarrierTwoStep& operator=(const volatile BarrierTwoStep& p_other) volatile
    {
        this->state = p_other.state;
        return *this;
    };

    inline void ask_and_wait_for_sync_1()
    {
        while (this->state != State::OPENED)
        {
        }
        this->state = State::ASKING_FOR_SYNC_1;
        while (this->state != State::SYNC_1_REACHED)
        {
        }
    };

    inline void ask_and_wait_for_sync_1() volatile
    {
        ((BarrierTwoStep*)this)->ask_and_wait_for_sync_1();
    };

    inline void ask_for_sync_1()
    {
        while (this->state != State::OPENED)
        {
        }
        this->state = State::ASKING_FOR_SYNC_1;
    };

    inline void ask_for_sync_1() volatile
    {
        ((BarrierTwoStep*)this)->ask_for_sync_1();
    };

    inline void wait_for_sync_1()
    {
        while (this->state != State::SYNC_1_REACHED)
        {
        }
    };

    inline void wait_for_sync_1() volatile
    {
        ((BarrierTwoStep*)this)->wait_for_sync_1();
    };

    inline void notify_sync_1_and_wait_for_sync_2()
    {
        this->state = State::SYNC_1_REACHED;
        while (this->state != State::SYNC_2_REACHED)
        {
        }
        this->state = State::OPENED;
    };

    inline void notify_sync_1_and_wait_for_sync_2() volatile
    {
        ((BarrierTwoStep*)this)->notify_sync_1_and_wait_for_sync_2();
    };

    inline void notify_sync_2()
    {
        this->state = State::SYNC_2_REACHED;
    };

    inline void notify_sync_2() volatile
    {
        ((BarrierTwoStep*)this)->notify_sync_2();
    };

    inline int8 is_opened()
    {
        return this->state == State::OPENED;
    };

    inline int8 is_opened() volatile
    {
        return ((BarrierTwoStep*)this)->is_opened();
    };
};