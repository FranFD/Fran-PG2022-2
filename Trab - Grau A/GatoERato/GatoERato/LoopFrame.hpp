#pragma once

#include <chrono>

class LoopFrame
{
public:
    typedef std::chrono::high_resolution_clock LFClock;

    inline void StartFrame()
    {
        frameStart = LFClock::now().time_since_epoch();
    }

    inline void EndFrame()
    {
        frameEnd = LFClock::now().time_since_epoch();
    }

    inline float FrameDelta()
    {
        return (frameEnd - frameStart).count() / (float)1e9;
    }

    inline float NextFrame()
    {
        EndFrame();
        float ds = FrameDelta();
        StartFrame();
        return ds;
    }
private:
    LFClock::duration frameStart;
    LFClock::duration frameEnd;
};
