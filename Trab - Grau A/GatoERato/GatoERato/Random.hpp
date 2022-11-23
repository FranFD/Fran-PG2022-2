#pragma once

class Random
{
private:
    friend int main(void);
    static void Init();
    static void Shutdown();
public:
    static float Float();
    static float Float(float min, float max);
};
