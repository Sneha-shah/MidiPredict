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

  void combineEvents(juce::MidiBuffer& a, juce::MidiBuffer& b, int numSamples, int offset);
  bool checkIfPause(juce::MidiBuffer& predBuffer, juce::MidiBuffer& liveBuffer);
    bool searchLive(juce::MidiMessage m);
    void updateNoteDensity(juce::MidiBuffer& predBuffer, juce::MidiBuffer& liveBuffer);
    void getBuffers(int numSamples);
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

  bool DEBUG_FLAG = 1;
    
  juce::String currentChord = "no Chord in Processor";
  std::array<int,12> pitchClassesPresent { 0 };

  void updatePitchClassesPresent(int noteNumber);
  juce::MidiKeyboardState midiKeyboardState;

  void runUnitTests(bool runAll = false);
    
    volatile float sampleRate_;
    
    // For file reading and data storage
  std::vector<juce::MidiBuffer> prevPredictions;
    int predictionBufferIndex;
//  std::vector<juce::MidiBuffer> prevRecordedBlocks;
//    int prevRecordedBlocksIndex;
  std::vector<juce::MidiBuffer> liveMidi;
    int currentBufferIndexLive;
  juce::MidiMessageSequence recordedMidiSequence;
    int currentPositionRecMidi;
    int currentPositionRecSamples;
  juce::MidiBuffer recordedBuffer;
  juce::MidiBuffer liveBuffer;
    int lag; // in number of blocks
    
    // For PausePlay Prediction
    const juce::uint8* liveBufferIndex;
    const juce::uint8* predBufferIndex;
    juce::MidiMessageSequence unmatchedNotes_pred;
    juce::MidiMessageSequence unmatchedNotes_live;
    
    // For Note Density Prediction
  float noteDensity_pred;
  float num_notes_predicted;
  float num_notes_network;
  float alpha; // alpha for tempo estimation
    int numBlocksForDensity;
    std::vector<int> prev50Pred;
    std::vector<int> prev50Live;
    int prev50PredIndex;
    int prev50LiveIndex;
    
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
