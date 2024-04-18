/*
  ==============================================================================

  This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#define JUCE_UNIT_TESTS (0)

#include <JuceHeader.h>
#include "SynthAudioSource.cpp"

#define USE_PGM (1)

//==============================================================================
/**
 */
class PluginProcessor  : public
#if USE_PGM == 1
foleys::MagicProcessor
#else
juce::AudioProcessor
#endif
{
public:
  //==============================================================================
  PluginProcessor();
  ~PluginProcessor() override;

  //==============================================================================
  juce::MidiBuffer generateMidiBuffer(const juce::MidiMessageSequence& midiMessageSequence, double sampleRate, int blockSize);
  void prepareToPlay (double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

  void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  //==============================================================================
#if USE_PGM == 0
  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;
  //==============================================================================
  void getStateInformation (juce::MemoryBlock& destData) override;
  void setStateInformation (const void* data, int sizeInBytes) override;
#endif

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram (int index) override;
  const juce::String getProgramName (int index) override;
  void changeProgramName (int index, const juce::String& newName) override;

  //==============================================================================
  // For our example:
  std::array<int,12> getPitchClassesPresent() {
    return pitchClassesPresent;
  }

  juce::MidiKeyboardState& getMidiKeyboardState() {
    return midiKeyboardState;
  }

private:

  juce::String currentChord = "no Chord in Processor";
  std::array<int,12> pitchClassesPresent { 0 };

  void updatePitchClassesPresent(int noteNumber);
  juce::MidiKeyboardState midiKeyboardState;

  void runUnitTests(bool runAll = false);

  std::vector<juce::MidiBuffer> recordedMidi;
  std::vector<juce::MidiBuffer> prevPredictions;
  std::vector<juce::MidiBuffer> liveMidi;
  juce::MidiMessageSequence recordedMidiSequence;
  int currentBufferIndexRec;
  int currentBufferIndexLive;
  int currentPositionRec;
  int currentPositionRecSamples;
  juce::MidiMessage lastEvent;
  juce::MidiMessage nextEvent;
  int lag; // in number of blocks
  int predictionBufferIndex;
  float tempo_prac_prev;
  float num_notes_recorded;
  float num_notes_network;
  float alpha; // alpha fo rtempo estimation
    
//    juce::AudioProcessorValueTreeState treeState;
//    juce::Synthesiser      synthesiser;
    SynthAudioSource synthAudioSource;
//    juce::ValueTree  presetNode;
    // GUI MAGIC: define that as last member of your AudioProcessor
//    foleys::MagicLevelSource*   outputMeter  = nullptr;
//    foleys::MagicPlotSource*    oscilloscope = nullptr;
//    foleys::MagicPlotSource*    analyser     = nullptr;

//    PresetListBox*              presetList   = nullptr;

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
