#pragma once

struct EngineLoop
{
	// To avoid crazy amount of update if the elapsed time from last frame is huge
	static const uint8 MAX_UPDATE_CALL_PER_FRAME = 2;

	time_t timebetweenupdates_mics;

	time_t previousUpdateTime_mics;
	time_t accumulatedelapsedtime_mics;

	inline static EngineLoop allocate_default(const time_t p_timebetweenupdates_mics)
	{
		return EngineLoop
		{
			p_timebetweenupdates_mics,
			0,
			clock_currenttime_mics()
		};
	};

	template<class EngineLoopCallbacksFunc>
	inline void update(EngineLoopCallbacksFunc& p_callback)
	{
		time_t l_currentTime = clock_currenttime_mics();
		time_t l_elapsed = l_currentTime - this->previousUpdateTime_mics;

		if (l_elapsed > (time_t)(this->timebetweenupdates_mics) * EngineLoop::MAX_UPDATE_CALL_PER_FRAME)
		{
			l_elapsed = (time_t)(this->timebetweenupdates_mics) * EngineLoop::MAX_UPDATE_CALL_PER_FRAME;
		}

		this->previousUpdateTime_mics = l_currentTime;
		this->accumulatedelapsedtime_mics += l_elapsed;

		if (this->accumulatedelapsedtime_mics >= this->timebetweenupdates_mics) {

			this->update_internal(this->timebetweenupdates_mics * 0.000001f, p_callback);
		}
		else
		{
			Thread::wait((uimax)  ((this->timebetweenupdates_mics - this->accumulatedelapsedtime_mics) * 0.0009999));
		}
	};

	template<class EngineLoopCallbacksFunc>
	inline void update_forced_delta(const float32 p_delta, EngineLoopCallbacksFunc& p_callback)
	{
		time_t l_currentTime = clock_currenttime_mics();
		this->previousUpdateTime_mics = l_currentTime;
		this->accumulatedelapsedtime_mics += (time_t)p_delta;

		this->update_internal(p_delta, p_callback);
	};

private:
	template<class EngineLoopCallbacksFunc>
	inline void update_internal(const float32 p_delta, EngineLoopCallbacksFunc& p_callback)
	{
		p_callback.newframe();

		p_callback.update(p_delta);

		p_callback.render();

		//TODO -> having a more precise loop (while delta < max  {} delta -= max) but preventing some piece of code to run twice.
		this->accumulatedelapsedtime_mics = 0;

		p_callback.endofframe();
	}
};