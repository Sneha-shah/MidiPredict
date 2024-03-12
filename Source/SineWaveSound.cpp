/*
  ==============================================================================

    SineWaveSound.cpp
    Created: 7 Mar 2024 4:11:36pm
    Author:  Sneha Shah

  ==============================================================================
*/

//#include "SineWaveSound.h"

#include <JuceHeader.h>

struct SineWaveSound   : public juce::SynthesiserSound
{
    SineWaveSound() {}
 
    bool appliesToNote    (int) override        { return true; }
    bool appliesToChannel (int) override        { return true; }
};
