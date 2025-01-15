#pragma once

// core sequencer API for Electribe 3 (and hopefully more synths)
// copyright: Korg Inc.
// created by Shijie Xia on 2025/1/14
// note: platform specific APIs (e.g. JUCE) should not be used here

// TODO: Doxygene documentation

class E3Sequencer {
public:
    // set up tick time in seconds
    // for most embedded system, this is 0.001 (1 ms)
    // can also be tailored to desktop/RPI systems
    // for example, for a system of sample rate 48k and block size 64 samples
    // and tick once per audio processing block
    // the tick rate is 48k/64 = 750, thus tick time should be 1/750

    // the timing resolution of the sequencer is a 1/384 of one bar
    // (or 1/24 of a quarter note. same as Elektron)
    // so 1 micro step = 2 seconds / 384 = 5.208 ms for BPM 120
    // ideally tick interval should be shorter than 1/10 of a micro step (roughly over 2000Hz)
    // otherwise timing precision suffers
    E3Sequencer(float tickTime) {
        juce::ignoreUnused(tickTime);
    }

    // transport-related
    void Start();
    void Stop();
    void Resume();

    void Tick();

private:
    // put all seqencer data here

};