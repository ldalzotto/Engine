#pragma once

/*
    A Barrier is a functional object that act as a way to wait the current thread execution until the barrier is open
*/
struct Barrier
{
    int8 val;

    inline void open()
    {
        this->val = 0;
    };

    inline void close()
    {
        this->val = 1;
    };

    inline int8 is_closed()
    {
        return this->val == 1;
    };

    inline void wait_for_open()
    {
        while (this->val == 1)
        {
        }
    };
};

/*
    A BarrierTwoStep is a functional object that allows executing a piece piece of code when the _SYNC_1 point has been reached.
    Once the _SYNC_1 point is reached, further execution of the triggering thread is holded until _SYNC_2 point is reached.
*/
struct BarrierTwoStep
{
    enum class State
    {
        OPENED = 0,
        ASKING_FOR_SYNC_1 = 1,
        SYNC_1_REACHED = 2,
        ASKING_FOR_SYNC_2 = 3,
        SYNC_2_REACHED = 4
    } state;



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

    inline void ask_for_sync_1()
    {
        while (this->state != State::OPENED)
        {
        }
        this->state = State::ASKING_FOR_SYNC_1;
    };

    inline void wait_for_sync_1()
    {
        while (this->state != State::SYNC_1_REACHED)
        {
        }
    };

    inline void notify_sync_1_and_wait_for_sync_2()
    {
        this->state = State::SYNC_1_REACHED;
        while (this->state != State::SYNC_2_REACHED)
        {
        }
        this->state = State::OPENED;
    };

    inline void notify_sync_2()
    {
        this->state = State::SYNC_2_REACHED;
    };

    inline int8 is_opened()
    {
        return this->state == State::OPENED;
    };

};