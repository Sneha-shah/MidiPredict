/*
  ==============================================================================

    SynthAudioSource.cpp
    Created: 3 Mar 2024 8:01:13pm
    Author:  Sneha Shah

  ==============================================================================
*/

#include <JuceHeader.h>
//#include "SineWaveSound.cpp"
#include "SineWaveVoice.cpp"

class SynthAudioSource   : public juce::AudioSource
{
public:
    SynthAudioSource (juce::MidiKeyboardState& keyState)
        : keyboardState (keyState)
    {
        for (auto i = 0; i < 4; ++i)                // [1]
            synth.addVoice (new SineWaveVoice());
 
        synth.addSound (new SineWaveSound());       // [2]
    }
    
    SynthAudioSource ()
        : keyboardState (initialMidiKeyboardState)
    {
        for (auto i = 0; i < 4; ++i)                // [1]
            synth.addVoice (new SineWaveVoice());
 
        synth.addSound (new SineWaveSound());       // [2]
    }
 
    void setUsingSineWaveSound()
    {
        synth.clearSounds();
    }
 
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        synth.setCurrentPlaybackSampleRate (sampleRate); // [3]
        midiCollector.reset (sampleRate); // [10]
    }
 
    void releaseResources() override {}
 
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();
 
        juce::MidiBuffer incomingMidi;
        keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample,
                                             bufferToFill.numSamples, true);       // [4]
 
        synth.renderNextBlock (*bufferToFill.buffer, incomingMidi,
                               bufferToFill.startSample, bufferToFill.numSamples); // [5]
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill, juce::MidiBuffer incomingMidi)
    {
        bufferToFill.clearActiveBufferRegion();

//        juce::MidiBuffer incomingMidi;
//        keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample,
//                                             bufferToFill.numSamples, true);       // [4]

        synth.renderNextBlock (*bufferToFill.buffer, incomingMidi,
                               bufferToFill.startSample, bufferToFill.numSamples); // [5]
    }
    
    juce::MidiMessageCollector* getMidiCollector()
    {
        return &midiCollector;
    }
 
private:
    juce::MidiKeyboardState initialMidiKeyboardState;
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
    juce::MidiMessageCollector midiCollector;
};



//#include "SynthAudioSource.h"
//
//class SynthAudioSource   : public juce::AudioSource
//{
//public:
//    SynthAudioSource (juce::MidiKeyboardState& keyState)
//        : keyboardState (keyState)
//    {
//        for (auto i = 0; i < 4; ++i)                // [1]
//            synth.addVoice (new SineWaveVoice());
//
//        synth.addSound (new SineWaveSound());       // [2]
//    }
//
//    void setUsingSineWaveSound()
//    {
//        synth.clearSounds();
//    }
//
//    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
//    {
//        synth.setCurrentPlaybackSampleRate (sampleRate); // [3]
//    }
//
//    void releaseResources() override {}
//
//    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
//    {
//        bufferToFill.clearActiveBufferRegion();
//
//        juce::MidiBuffer incomingMidi;
//        keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample,
//                                             bufferToFill.numSamples, true);       // [4]
//
//        synth.renderNextBlock (*bufferToFill.buffer, incomingMidi,
//                               bufferToFill.startSample, bufferToFill.numSamples); // [5]
//    }
//
//private:
//};
