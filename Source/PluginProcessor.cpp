/*
  ==============================================================================

  This file contains the basic framework code for a JUCE plugin processor.
  Created from Music 320c starter code, which builds on JUCE examples.

  ==============================================================================
*/

#include "PluginProcessor.h"
//#include "PresetListBox.h"

#if USE_PGM == 0
#include "PluginEditor.h"
#endif

//==============================================================================

//static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
//{
//    juce::AudioProcessorValueTreeState::ParameterLayout layout;
//    FoleysSynth::addADSRParameters (layout);
//    FoleysSynth::addOvertoneParameters (layout);
//    FoleysSynth::addGainParameters (layout);
//    return layout;
//}

//==============================================================================


void bufferVals(juce::MidiBuffer& a, juce::String name = "") {
    std::cout << name << ":" << std::endl;
    for(const auto meta: a) {
        std::cout << "MIDI Event: " << meta.getMessage().getDescription() << meta.getMessage().getTimeStamp() << std::endl;
    }
    std::cout << "end " << name << std::endl;
}

void seqVals(juce::MidiMessageSequence& a, juce::String name = "") {
    std::cout << name << ":" << std::endl;
    for(const auto meta: a) {
        std::cout << "MIDI Event: " << meta->message.getDescription() << meta->message.getTimeStamp() << std::endl;
    }
    std::cout << "end " << name << std::endl;
}

void p50s(juce::MidiMessageSequence& a, juce::String name = "", int n = 50) {
    std::cout << name << ":" << std::endl;
    for (int i=0; i<n; i++) {
        std::cout << "MIDI Event: "<< a.getEventPointer(i)->message.getDescription() << a.getEventPointer(i)->message.getTimeStamp() << std::endl;
    }
    std::cout << "end " << name << std::endl;
}

void p50b(std::vector<juce::MidiBuffer>& a, juce::String name = "", int n = 50, bool tsAbs = true) {
    std::cout << name << ":" << std::endl;
    int count = 0;
    int buffNum = 0;
    for (const auto& buff: a) {
        for(const auto meta: buff) {
            std::cout << "MIDI Event: "<< meta.getMessage().getDescription() << (meta.getMessage().getTimeStamp())+(512*buffNum*tsAbs) << std::endl; // 512 block size
            count++;
            if(count>=n)
                break;
        }
        buffNum++;
        if(count>=n)
            break;
    }
    std::cout << "end " << name << std::endl;
}

PluginProcessor::PluginProcessor() // xtor
#ifndef JucePlugin_PreferredChannelConfigurations
  :
#if USE_PGM == 1
  MagicProcessor
#else
  AudioProcessor
#endif
  (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                    .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                    .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                    )
#endif
{
#if USE_PGM == 1
  magicState.setGuiValueTree (BinaryData::MidiPredict_xml, BinaryData::MidiPredict_xmlSize);
#endif

#if JUCE_UNIT_TESTS
  runUnitTests();
#endif
  
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
  return JucePlugin_Name;
}

// Set these in Projucer:

bool PluginProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool PluginProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool PluginProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const
{
  return 0.0;
}

int PluginProcessor::getNumPrograms()
{
  return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
  // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
  return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
}

const juce::String PluginProcessor::getProgramName (int index)
{
  return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================

std::vector<juce::MidiBuffer> readMIDIFile(const juce::File& midiFile, double sampleRate, int blockSize)
{
    juce::MidiFile midiFileData;
    juce::MidiBuffer loadedMidiBuffer;

    juce::FileInputStream fileInputStream(midiFile);

    if (fileInputStream.openedOk())
    {
        if (midiFileData.readFrom(fileInputStream))
        {
            midiFileData.convertTimestampTicksToSeconds();
            int numBlocks = static_cast<int>(midiFileData.getLastTimestamp() * sampleRate / blockSize) + 1;
            std::vector<juce::MidiBuffer> midiBuffers(numBlocks);

            for (int trackIndex = 0; trackIndex < midiFileData.getNumTracks(); ++trackIndex)
            {
                const juce::MidiMessageSequence& track = *midiFileData.getTrack(trackIndex);

                for (int eventIndex = 0; eventIndex < track.getNumEvents(); ++eventIndex)
                {
                    const juce::MidiMessage& midiMessage = track.getEventPointer(eventIndex)->message;
                    double timeStamp_sec = midiMessage.getTimeStamp(); // seconds
                    
                    int blockIndex = static_cast<int>(timeStamp_sec * sampleRate / blockSize);

                    // Create a buffer for this block if not created yet
                    if (midiBuffers[blockIndex].isEmpty())
                    {
                        midiBuffers[blockIndex] = juce::MidiBuffer();
                    }

                    // Add the MIDI message to the buffer for the corresponding block
                    midiBuffers[blockIndex].addEvent(midiMessage, static_cast<int>(timeStamp_sec * sampleRate) % blockSize); // timeStamp in samples
                }
            }

            return midiBuffers;
        }
        else
        {
            juce::Logger::writeToLog("Error reading MIDI file: " + midiFile.getFullPathName());
        }
    }
    else
    {
        juce::Logger::writeToLog("Error opening MIDI file: " + midiFile.getFullPathName());
    }

    // Return an empty vector in case of an error
    return {};
}


juce::MidiMessageSequence readMIDIFile(const juce::File& midiFile, double sampleRate, double speedShift = 1)
{
    juce::MidiFile midiFileData;
    juce::FileInputStream fileInputStream(midiFile);

    if (fileInputStream.openedOk())
    {
        if (midiFileData.readFrom(fileInputStream))
        {
            midiFileData.convertTimestampTicksToSeconds();
            juce::MidiMessageSequence loadedMidiSequence;

            for (int trackIndex = 0; trackIndex < midiFileData.getNumTracks(); ++trackIndex)
            {
                const juce::MidiMessageSequence& track = *midiFileData.getTrack(trackIndex);

                for (int eventIndex = 0; eventIndex < track.getNumEvents(); ++eventIndex)
                {
                    const juce::MidiMessage& midiMessage = track.getEventPointer(eventIndex)->message;
                    double timeStamp_sec = midiMessage.getTimeStamp(); // seconds
                    int timeStamp_samples = static_cast<int>(timeStamp_sec * sampleRate);

                    // Add the MIDI message to the sequence
                    loadedMidiSequence.addEvent(midiMessage, speedShift*timeStamp_samples);
//                    loadedMidiSequence.addEvent(midiMessage, (speedShift - 1.0) * timeStamp_samples);
                }
            }
            return loadedMidiSequence;
        }
        else
        {
            juce::Logger::writeToLog("Error reading MIDI file: " + midiFile.getFullPathName());
        }
    }
    else
    {
        juce::Logger::writeToLog("Error opening MIDI file: " + midiFile.getFullPathName());
    }

    // Return an empty sequence in case of an error
    return juce::MidiMessageSequence();
}

juce::MidiBuffer PluginProcessor::generateMidiBuffer(const juce::MidiMessageSequence& midiMessageSequence, double sampleRate, int blockSize)
{
    juce::MidiBuffer midiBuffer;

    // Iterate over the MIDI events in the sequence
    for (int eventIndex = currentPositionRecMidi; eventIndex < midiMessageSequence.getNumEvents(); ++eventIndex)
    {
        const juce::MidiMessage& midiMessage = midiMessageSequence.getEventPointer(eventIndex)->message;
        int timeStamp_samples = midiMessage.getTimeStamp();

        // Check if the event is within the range of the current sample position and two blocks ahead
        if (timeStamp_samples < currentPositionRecSamples + 2 * blockSize)
        {
            // Add the MIDI message to the buffer
            midiBuffer.addEvent(midiMessage, (timeStamp_samples - currentPositionRecSamples));
        }
    }

    return midiBuffer;
}


void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
    
    if (DEBUG_FLAG) {
        std::cout << "Samples per Block = " << samplesPerBlock << "\n";
        std::cout << "SampleRate = " << sampleRate << "\n";
        std::cout << "Number of blocks per second = " << sampleRate/samplesPerBlock << "\n";
    }
    
    // First reading midi file for recorded practice performance
    auto myMidiFile = juce::File::getSpecialLocation (juce::File::currentApplicationFile)
      .getChildFile ("Contents")
      .getChildFile ("Resources")
      .getChildFile ("ladispute_1.mid");
    jassert(myMidiFile.existsAsFile());
    std::vector<juce::MidiBuffer> recordedMidi = readMIDIFile(myMidiFile, sampleRate, samplesPerBlock); // TO DO: only read lag number of blocks
    int currentBufferIndexRec = 0;
    currentPositionRecMidi = 0;
    currentPositionRecSamples = 0;
    
    // Setting lag and processing outputs till then?
    lag = 20; // num blocks
    for(int i=0; i<lag; i++)
    {
        prevPredictions.push_back(recordedMidi[currentBufferIndexRec]);
        currentPositionRecMidi += recordedMidi[currentBufferIndexRec].getNumEvents();
        currentPositionRecSamples += samplesPerBlock;
        ++currentBufferIndexRec;
        if (DEBUG_FLAG) {
//            std::cout << "\nInitializing at i=" << i << std::endl;
//            bufferVals(recordedMidi[currentBufferIndexRec-1], "Current buffer");
//            p50b(prevPredictions, "prevpred", 20);
        }
    }
    predictionBufferIndex = 0;
    
    recordedMidiSequence = readMIDIFile(myMidiFile, sampleRate, 0.8);
//    if(currentPositionRecMidi>0)
//        lastEvent = recordedMidiSequence.getEventPointer(currentPositionRecMidi-1)->message;
//    nextEvent = recordedMidiSequence.getEventPointer(currentPositionRecMidi)->message;
    unmatchedNotes_pred.clear();
    unmatchedNotes_live.clear();
    
    auto myMidiFile2 = juce::File::getSpecialLocation (juce::File::currentApplicationFile)
      .getChildFile ("Contents")
      .getChildFile ("Resources")
      .getChildFile ("ladispute_1.mid");
    jassert(myMidiFile2.existsAsFile());
    liveMidi = readMIDIFile(myMidiFile2, sampleRate, samplesPerBlock);
    currentBufferIndexLive = 0;
    
//    synthesiser.setCurrentPlaybackSampleRate(sampleRate);
    synthAudioSource.prepareToPlay(samplesPerBlock, sampleRate);
    liveBufferIndex = nullptr;
    predBufferIndex = nullptr;
    
    if (DEBUG_FLAG) {
//        p50b(liveMidi, "50 events of Live MIDI");
//        p50b(recordedMidi, "50 events of Rec MIDI");
    }

    sampleRate_ = sampleRate;
    
}

void PluginProcessor::releaseResources()
{
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
    synthAudioSource.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  std::cout << "ndef JucePlugin_PreferredChannelConfigurations";
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused (layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
      && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void PluginProcessor::combineEvents(juce::MidiBuffer& a, juce::MidiBuffer& b, int numSamples = -1)
{
    a.addEvents(b, 0, numSamples, 0);
    if (DEBUG_FLAG) {
        if(! b.isEmpty())
        {
//            std::cout << "Found an event";
        }
    }
}

bool seachLive();

bool PluginProcessor::checkIfPause(juce::MidiBuffer& predBuffer, juce::MidiBuffer& liveBuffer)
{
    // What to do if live buffer gets ahead?
    
    juce::MidiMessage m;
    bool pause = false;
    int option = 2; // 1 is pointers, 2 is MidiSequence
    
    if (option == 1) {
        
        /** OPTION 1 STARTS HERE - Using MidiBufferIterators */
        if (predBufferIndex == nullptr) {
            auto iter = predBuffer.begin();
            if (iter == predBuffer.end()) {
                pause = false;
                return pause;
            }
            predBufferIndex = (*iter).getMessage().getRawData();
            //        auto temp = (*iter).getMessage().getRawData();
            //        predBufferIndex = iter.getData();
            //        if (DEBUG_FLAG)
            //            std::cout << "predBufferIndex==(*iter).getMessage().getRawData():" << (predBufferIndex==temp)<<std::endl;
        }
        if (liveBufferIndex == nullptr) {
            auto iter = liveBuffer.begin();
            if (iter == liveBuffer.end()) {
                pause = true;
                return pause;
            }
            liveBufferIndex = (*iter).getMessage().getRawData();
            //        const juce::uint8* temp = (*iter).getMessage().getRawData();
            //        liveBufferIndex = iter.getData();
            if (DEBUG_FLAG) {
                //            std::cout << "liveBufferIndex==(*iter).getMessage().getRawData():" << (liveBufferIndex==temp)<<std::endl;
//                std::cout << liveBuffer.
            }
        }
        
        // Loop through unmatchedNotes and compare MIDI events to buffer b
        
        for (juce::MidiBufferIterator iterP(predBufferIndex); iterP != predBuffer.end(); ++iterP)
        {
            m = (*iterP).getMessage();
            if (! m.isNoteOn())
                continue;
            predBufferIndex = reinterpret_cast<const juce::uint8*>(&m); // ????
            
            if(!pause) {
                bool found = false; // is this really the best way to check missing notes?
                for (juce::MidiBufferIterator iterL(liveBufferIndex); iterL != liveBuffer.end(); ++iterL)
                {
                    const juce::MidiMessage& m2 = (*iterL).getMessage();
                    if (! m2.isNoteOn())
                        continue;
                    liveBufferIndex = reinterpret_cast<const juce::uint8*>(&m2); // ????
                    if ((*iterL).getMessage().getNoteNumber() == m.getNoteNumber())
                    {
                        found = true;
                        break;
                    }
                }
                if (!found) // pause and wait till note is found
                {
                    pause = true;
                    break;
                }
            }
        }
        /** OPTION 1 ENDS HERE */
        
    } else if (option == 2) {
        
        /** OPTION 2 STARTS HERE - Using MidiSequences */
            liveBufferIndex = nullptr;
            predBufferIndex = nullptr;
            int timeAdj = 0;
            if (unmatchedNotes_live.getNumEvents() > 0)
                timeAdj = unmatchedNotes_live.getEventTime(unmatchedNotes_live.getNumEvents() - 1)+2;
            for (auto metaB : liveBuffer)
            {
                if (metaB.getMessage().isNoteOnOrOff())
                {
                    unmatchedNotes_live.addEvent(metaB.getMessage(), timeAdj);
                }
            }
            while (unmatchedNotes_pred.getNumEvents() > 0)
            {
                m = unmatchedNotes_pred.getEventPointer(0)->message;
                if (!m.isNoteOnOrOff()) { // Skip non-note events
                    unmatchedNotes_pred.deleteEvent(0,0);
                    continue;
                }
        
                bool found = false;
                for (int i=0; i<1 && i<unmatchedNotes_live.getNumEvents(); i++) {
                    if (unmatchedNotes_live.getEventPointer(i)->message.getNoteNumber() == m.getNoteNumber())
                    {
                        found = true;
                        unmatchedNotes_live.deleteEvent(i,0);
                        break;
                    }
                }
                // check buffer
                if (!found) // If note not found in buffer b, set pause flag
                {
                    pause = true;
                    break;
                } else {
                    unmatchedNotes_pred.deleteEvent(0,0);
                }
            }
            for (const auto meta : predBuffer)
            {
                if (DEBUG_FLAG && unmatchedNotes_live.getNumEvents() > 0)
                    std::cout << "\nCatching up to live now...\n\n";
                m = meta.getMessage();
                if (! m.isNoteOnOrOff())
                    continue;
        
                if(!pause) {
                    bool found = false; // is this really the best way to check missing notes?
                    for (int i=0; i<1 && i<unmatchedNotes_live.getNumEvents(); i++) {
                        if (unmatchedNotes_live.getEventPointer(i)->message.getNoteNumber() == m.getNoteNumber())
                        {
                            found = true;
                            unmatchedNotes_live.deleteEvent(i,0);
        //                        i--;
        //                        break;
                        }
                    }
                    if (!found) // If note not found in buffer b, set pause flag
                    {
                        pause = true;
                    }
                }
                if (pause) { // save notes to search later
                    unmatchedNotes_pred.addEvent(m);
        //            unmatchedNotes.addEvent(m, currentPositionRecSamples);
        //            unmatchedNotes.updateMatchedPairs();
                }
            }
        /** OPTION 2 ENDS HERE */
    }
    
    if (DEBUG_FLAG) {
        bufferVals(liveBuffer, "Live");
        bufferVals(predBuffer, "Pred");
        seqVals(unmatchedNotes_pred, "unmatched_pred");
        seqVals(unmatchedNotes_live, "unmatched_live");
    }
    if(!pause) {
        predBufferIndex = nullptr;
//        liveBufferIndex = nullptr;
        return false;
    }
    else {
        return true;// pause and wait till note is found
    }
    return false;
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (sampleRate_ == 0.0) {
        // Sample rate not set, return without processing
        return;
    }
    // PROCESS MIDI DATA
//    juce::MidiBuffer processedMidi;
    int time_samp;
    juce::MidiMessage m;
    
    // Source 1 (history) midiRecorded
    // 1) Process it block by block here :
//    juce::MidiBuffer& recordedBuffer = recordedMidi[currentBufferIndexRec];
//    ++currentBufferIndexRec;
    if (predBufferIndex == nullptr) {
        recordedBuffer2 = generateMidiBuffer(recordedMidiSequence, getSampleRate(), buffer.getNumSamples()); // QUES: Is this an expensive operation to copy? Can it be avoided? (using rn so pointers access to same memory location...)
    } else if ( DEBUG_FLAG ) {
//        std::cout << "\nNON-NULL PTR predBufferIndex\n\n";
    }
//    int prevPositionRecSamples = currentPositionRecSamples;
    
//    if (currentBufferIndex >= recordedMidi.size())
//    {
//        recordedBuffer = midiMessages;
//        --currentBufferIndex;
//    }
    
    // 2) Send processed prediction for playback : midiPrediction
    juce::MidiBuffer midiPrediction {};
    
    // Temporarily, Source 2 from file:
    // 1) Create a delayed input at some latency (say 50ms) : delayed_realtime
    // 2) Use as is for playback
    if (liveBufferIndex == nullptr) {
        liveBuffer = liveMidi[currentBufferIndexLive]; // QUES: Is this an expensive operation to copy? Can it be avoided? (using rn so pointers access to same memory location...)
    } else {
        liveBuffer.addEvents(liveMidi[currentBufferIndexLive], 0, -1, buffer.getNumSamples());
        // TO DO: Also somehow delete matched events
//        if ( DEBUG_FLAG ) {
//            std::cout << "\nNON-NULL PTR liveBufferIndex\n\n";
//        }
    }
    ++currentBufferIndexLive;
    
    if ( DEBUG_FLAG ) {
        if (! (recordedBuffer2.isEmpty() && liveBuffer.isEmpty())) {
            bufferVals(recordedBuffer2, "recordedBuffer2");
            bufferVals(liveBuffer,"Live");
        }
    }
    
    // MAGIC GUI: send midi messages to the keyboard state and MidiLearn
    magicState.processMidiBuffer (midiMessages, buffer.getNumSamples(), true);
    // MAGIC GUI: send playhead information to the GUI
    magicState.updatePlayheadInformation (getPlayHead());
    
    // Predictions:
    // 1. Playback midi file as is DONE
    // 2. Playback midi file, and if delayed input, pause playback. Add a 1 block speedup when live is ahead
    // 3. Implement tempo tracking:
    //     tempo_prac(n) = a*tempo_prac(n-1) + (1-a)*tempo_network(n-lag)
    // 4.
    
    int predictionCase = 2; // 1 - Playback recording as is
//    int PLAYBACK = 1;
//    int PAUSE = 2;
//    int TEMPO_EXP = 3;
    bool isPaused = false;
    if (predictionCase == 2) {
        isPaused = checkIfPause(prevPredictions[predictionBufferIndex], liveBuffer);
        if (DEBUG_FLAG)
            std::cout << "isPaused: " << isPaused << std::endl;
    }

    // Obsolete API (which still works): for (MidiBuffer::Iterator i (midiMessages); i.getNextEvent (m, time);)
    for (const auto meta : recordedBuffer2)
    {
        m = meta.getMessage();
//        m.setNoteNumber(m.getNoteNumber()+7); // Changing key/ octave to differentiate from live performance
        midiKeyboardState.processNextMidiEvent(m); // Let PGM display current note
        time_samp = m.getTimeStamp();
        auto description = m.getDescription();
        if (DEBUG_FLAG) {
            std::cout << "Iterating rb2 --- MIDI EVENT:" << description << "\n";
        }
        
        if (predictionCase == 1) {
            // 1. Simplest prediction case - playback recording as is
            // do nothing
        }
        else if (predictionCase == 2) {
            // 2. Pause when no input seen
            // We need to compare already played recBuffer (aka current prediction - lag) 
            // With the liveBuffer (current input)
            // If for any note in recBuffer, there is no corresponding note in liveBuffer, pause
            // If there is a note in liveBuffer that is not in recBuffer, play it
            // For every m in prevPredictions[predictionBufferIndex], check if it is in liveBuffer
            // If not, pause

            // Initially pause the whole block only
            if (isPaused)
            {
                if (DEBUG_FLAG)
                    std::cout << "pausing !!!!!!!!!" << std::endl;
                break;
            }
            
        }
        else if (predictionCase == 3) {
            // 3. Implement rough tempo tracking pt 1: (calc note density using number of notes played so far)
            // noteDensity_prac(n) = a*noteDensity_prac(n-1) + (1-a)*noteDensity_network(n-lag)
            num_notes_recorded = 0; // TO DO: update with prediction buffer (prevPredictions[predictionBufferIndex])
            num_notes_network = 0; // TO DO: update with curr buffer (midiMessages)
            float noteDensity_prac;
            float noteDensity_network = num_notes_network/ num_notes_recorded; // tempo estimation using number of notes played so far as compared to recording.
            noteDensity_prac = alpha*noteDensity_prac_prev + (1-alpha) * noteDensity_network;
            noteDensity_prac_prev = noteDensity_prac;
            // next process m accordingly
        }
        else {
            std::cout << "Invalid value for predictionCase. Defaulting to do nothing case." << std::endl;
        }
        
        // Add processed midi event to prediction buffer
        midiPrediction.addEvent (m, time_samp);
        currentPositionRecMidi += 1;
        if(m.isNoteOn())
        {
//            if(lastEvent.isNoteOn())
//                currentPositionRecSamples += nextEvent.getTimeStamp() - lastEvent.getTimeStamp(); //FIX THIS!!!
//            else
//                currentPositionRecSamples += nextEvent.getTimeStamp();
//            lastEvent = nextEvent;
//            nextEvent = m.withTimeStamp(m.getTimeStamp()+prevPositionRecSamples);
        }
        if(m.getTimeStamp() >= buffer.getNumSamples())
            break;
        
        if (m.isNoteOn()) { // exercise MIDI out
        } else if (m.isNoteOff()) {
            // Cancel pitchbend?
        } else if (m.isAftertouch()) {
            // Do something with aftertouch?
        } else if (m.isPitchWheel()) {
            // You could save the last note number and add pitchbend to that and convert according to your transformation
        }
    }
    
    // play prediction notes using synthesizer
    juce::AudioSourceChannelInfo bufferInfo;
    bufferInfo.buffer = &buffer;  // Assuming buffer is your AudioBuffer<float>
    bufferInfo.startSample = 0;
    bufferInfo.numSamples = buffer.getNumSamples();
    // combine prevPredictions[predictionBufferIndex] and midiMessages
    juce::MidiBuffer midiCombined; // midi file + current keyboard
    //    combineEvents(midiCombined, recordedBuffer, buffer.getNumSamples());
    //    combineEvents(midiCombined, recordedBuffer2, buffer.getNumSamples());
//    if (!isPaused)
        combineEvents(midiCombined, prevPredictions[predictionBufferIndex]); // Uses addEvents with MidiBuffer&
    combineEvents(midiCombined, liveBuffer);
    combineEvents(midiCombined, midiMessages);
    
    synthAudioSource.getNextAudioBlock(bufferInfo,
                                       midiCombined);
    // JOS (used for debugging): midiPrediction);

  // midiMessages.swapWith (prevPredictions[predictionBufferIndex]); // need to do this?
  // JOS: Yes, when we want the plugin to forward it (Midi Filter Plugin case)
    if (isPaused) {
        prevPredictions[predictionBufferIndex] = midiPrediction;
    } else {
        prevPredictions[predictionBufferIndex] = midiPrediction;
        predictionBufferIndex = (predictionBufferIndex+1) % prevPredictions.size();
        currentPositionRecSamples += buffer.getNumSamples(); // FIX THIS
    }
    if (DEBUG_FLAG)
        p50b(prevPredictions, "\nprevpred", 20);
    
    
//    for (int i = 1; i < buffer.getNumChannels(); ++i)
//        buffer.copyFrom (i, 0, buffer.getReadPointer (0), buffer.getNumSamples());
//
//    // MAGIC GUI: send the finished buffer to the level meter
//    outputMeter->pushSamples (buffer);
//    oscilloscope->pushSamples (buffer);
//    analyser->pushSamples (buffer);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if USE_PGM == 0

//==============================================================================

bool PluginProcessor::hasEditor() const
{
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
  return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
  // You should use this method to restore your parameters from this memory block,
  // whose contents will have been created by the getStateInformation() call.
}

#endif

//==============================================================================

void PluginProcessor::runUnitTests(bool runAll) {
  std::cout << "GeoM.cpp: Out of the available UNIT-TEST categories:\n  ";
  for (auto c : juce::UnitTest::getAllCategories()) {
    std::cout << " " << c;
  }
  DBG ("\n  running unit tests \"MIDI\":");
  juce::UnitTestRunner testRunner;
  if (runAll) {
    testRunner.runAllTests();
  } else { // just run what we have below
    auto tests = juce::UnitTest::getAllTests();
    for (auto t : tests) {
      if (t->getCategory() == "MIDI") { // ~/JUCE/modules/juce_audio_basics/midi/juce_MidiFile.cpp:531
        DBG(" MIDI / " << t->getName());
        t->performTest(&testRunner);
      } else if (t->getCategory() == "MidiFile") { // this file
        DBG("  MidiPredict / " << t->getName());
        t->performTest(&testRunner);
      } else {
        DBG("SKIPPED unit-test " << t->getCategory() << " / " << t->getName());
      }
    }
  }
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new PluginProcessor();
}

//==============================================================================
// Copied from /l/j/modules/juce_audio_basics/midi/juce_MidiFile.cpp
// purely to provide reference code for adding MidiFile support.
//==============================================================================
#if JUCE_UNIT_TESTS

#pragma message("JUCE_UNIT_TESTS is SET")

using namespace juce;

struct MidiPredictTest  : public UnitTest
{
  MidiPredictTest() : UnitTest ("MidiPredict", UnitTestCategories::midi)
  {}
  
  void runTest() override
  {
    beginTest ("First MidiPredict test series");
    {
      std::cout << "My first MidiPredict test\n";
      expect(1 == 1); // test 1
      expect(0 == 0); // test 2
      // expect(0 == 1); // Try it!
      // etc.
    }
  }
  };

static MidiPredictTest midiPredictTests;

//==============================================================================

namespace MidiFileHelpers
{

    static void writeVariableLengthInt (OutputStream& out, uint32 v)
    {
        auto buffer = v & 0x7f;

        while ((v >>= 7) != 0)
        {
            buffer <<= 8;
            buffer |= ((v & 0x7f) | 0x80);
        }

        for (;;)
        {
            out.writeByte ((char) buffer);

            if (buffer & 0x80)
                buffer >>= 8;
            else
                break;
        }
    }

    template <typename Integral>
    struct ReadTrait;

    template <>
    struct ReadTrait<uint32> { static constexpr auto read = ByteOrder::bigEndianInt; };

    template <>
    struct ReadTrait<uint16> { static constexpr auto read = ByteOrder::bigEndianShort; };

    template <typename Integral>
    Optional<Integral> tryRead (const uint8*& data, size_t& remaining)
    {
        using Trait = ReadTrait<Integral>;
        constexpr auto size = sizeof (Integral);

        if (remaining < size)
            return {};

        const Optional<Integral> result { Trait::read (data) };

        data += size;
        remaining -= size;

        return result;
    }

    struct HeaderDetails
    {
        size_t bytesRead = 0;
        short timeFormat = 0;
        short fileType = 0;
        short numberOfTracks = 0;
    };

    static Optional<HeaderDetails> parseMidiHeader (const uint8* const initialData,
                                                    const size_t maxSize)
    {
        auto* data = initialData;
        auto remaining = maxSize;

        auto ch = tryRead<uint32> (data, remaining);

        if (! ch.hasValue())
            return {};

        if (*ch != ByteOrder::bigEndianInt ("MThd"))
        {
            auto ok = false;

            if (*ch == ByteOrder::bigEndianInt ("RIFF"))
            {
                for (int i = 0; i < 8; ++i)
                {
                    ch = tryRead<uint32> (data, remaining);

                    if (! ch.hasValue())
                        return {};

                    if (*ch == ByteOrder::bigEndianInt ("MThd"))
                    {
                        ok = true;
                        break;
                    }
                }
            }

            if (! ok)
                return {};
        }

        const auto bytesRemaining = tryRead<uint32> (data, remaining);

        if (! bytesRemaining.hasValue() || *bytesRemaining > remaining)
            return {};

        const auto optFileType = tryRead<uint16> (data, remaining);

        if (! optFileType.hasValue() || 2 < *optFileType)
            return {};

        const auto optNumTracks = tryRead<uint16> (data, remaining);

        if (! optNumTracks.hasValue() || (*optFileType == 0 && *optNumTracks != 1))
            return {};

        const auto optTimeFormat = tryRead<uint16> (data, remaining);

        if (! optTimeFormat.hasValue())
            return {};

        HeaderDetails result;

        result.fileType = (short) *optFileType;
        result.timeFormat = (short) *optTimeFormat;
        result.numberOfTracks = (short) *optNumTracks;
        result.bytesRead = maxSize - remaining;

        return { result };
    }

    template <typename Fn>
    static Optional<MidiFileHelpers::HeaderDetails> parseHeader (Fn&& fn)
    {
        MemoryOutputStream os;
        fn (os);

        return MidiFileHelpers::parseMidiHeader (reinterpret_cast<const uint8*> (os.getData()),
                                                 os.getDataSize());
    }

    static MidiMessageSequence readTrack (const uint8* data, int size)
    {
        double time = 0;
        uint8 lastStatusByte = 0;

        MidiMessageSequence result;

        while (size > 0)
        {
            const auto delay = MidiMessage::readVariableLengthValue (data, (int) size);

            if (! delay.isValid())
                break;

            data += delay.bytesUsed;
            size -= delay.bytesUsed;
            time += delay.value;

            if (size <= 0)
                break;

            int messSize = 0;
            const MidiMessage mm (data, size, messSize, lastStatusByte, time);

            if (messSize <= 0)
                break;

            size -= messSize;
            data += messSize;

            result.addEvent (mm);

            auto firstByte = *(mm.getRawData());

            if ((firstByte & 0xf0) != 0xf0)
                lastStatusByte = firstByte;
        }

        return result;
    }

  //-==---------------- End MidiFile Helpers -------------------------

struct MidiFileTest final : public UnitTest
{
    MidiFileTest()
        : UnitTest ("MidiFile", UnitTestCategories::midi)
    {
      printf("RUNNING UNIT TESTS\n");
    }

    void runTest() override
    {
        beginTest ("ReadTrack respects running status");
        {
            const auto sequence = parseSequence ([] (OutputStream& os)
            {
                MidiFileHelpers::writeVariableLengthInt (os, 100);
                writeBytes (os, { 0x90, 0x40, 0x40 });
                MidiFileHelpers::writeVariableLengthInt (os, 200);
                writeBytes (os, { 0x40, 0x40 });
                MidiFileHelpers::writeVariableLengthInt (os, 300);
                writeBytes (os, { 0xff, 0x2f, 0x00 });
            });

            expectEquals (sequence.getNumEvents(), 3);
            expect (sequence.getEventPointer (0)->message.isNoteOn());
            expect (sequence.getEventPointer (1)->message.isNoteOn());
            expect (sequence.getEventPointer (2)->message.isEndOfTrackMetaEvent());
        }

        beginTest ("ReadTrack returns available messages if input is truncated");
        {
            {
                const auto sequence = parseSequence ([] (OutputStream& os)
                {
                    // Incomplete delta time
                    writeBytes (os, { 0xff });
                });

                expectEquals (sequence.getNumEvents(), 0);
            }

            {
                const auto sequence = parseSequence ([] (OutputStream& os)
                {
                    // Complete delta with no following event
                    MidiFileHelpers::writeVariableLengthInt (os, 0xffff);
                });

                expectEquals (sequence.getNumEvents(), 0);
            }

            {
                const auto sequence = parseSequence ([] (OutputStream& os)
                {
                    // Complete delta with malformed following event
                    MidiFileHelpers::writeVariableLengthInt (os, 0xffff);
                    writeBytes (os, { 0x90, 0x40 });
                });

                expectEquals (sequence.getNumEvents(), 1);
                expect (sequence.getEventPointer (0)->message.isNoteOff());
                expectEquals (sequence.getEventPointer (0)->message.getNoteNumber(), 0x40);
                expectEquals (sequence.getEventPointer (0)->message.getVelocity(), (uint8) 0x00);
            }
        }

        beginTest ("Header parsing works");
        {
            {
                // No data
                const auto header = parseHeader ([] (OutputStream&) {});
                expect (! header.hasValue());
            }

            {
                // Invalid initial byte
                const auto header = parseHeader ([] (OutputStream& os)
                {
                    writeBytes (os, { 0xff });
                });

                expect (! header.hasValue());
            }

            {
                // Type block, but no header data
                const auto header = parseHeader ([] (OutputStream& os)
                {
                    writeBytes (os, { 'M', 'T', 'h', 'd' });
                });

                expect (! header.hasValue());
            }

            {
                // We (ll-formed header, but track type is 0 and channels != 1
                const auto header = parseHeader ([] (OutputStream& os)
                {
                    writeBytes (os, { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 0, 0, 16, 0, 1 });
                });

                expect (! header.hasValue());
            }

            {
                // Well-formed header, but track type is 5
                const auto header = parseHeader ([] (OutputStream& os)
                {
                    writeBytes (os, { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 5, 0, 16, 0, 1 });
                });

                expect (! header.hasValue());
            }

            {
                // Well-formed header
                const auto header = parseHeader ([] (OutputStream& os)
                {
                    writeBytes (os, { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 16, 0, 1 });
                });

                expect (header.hasValue());

                expectEquals (header->fileType, (short) 1);
                expectEquals (header->numberOfTracks, (short) 16);
                expectEquals (header->timeFormat, (short) 1);
                expectEquals ((int) header->bytesRead, 14);
            }
        }

        beginTest ("Read from stream");
        {
            {
                // Empty input
                const auto file = parseFile ([] (OutputStream&) {});
                expect (! file.hasValue());
            }

            {
                // Malformed header
                const auto file = parseFile ([] (OutputStream& os)
                {
                    writeBytes (os, { 'M', 'T', 'h', 'd' });
                });

                expect (! file.hasValue());
            }

            {
                // Header, no channels
                const auto file = parseFile ([] (OutputStream& os)
                {
                    writeBytes (os, { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 0, 0, 1 });
                });

                expect (file.hasValue());
                expectEquals (file->getNumTracks(), 0);
            }

            {
                // Header, one malformed channel
                const auto file = parseFile ([] (OutputStream& os)
                {
                    writeBytes (os, { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 1, 0, 1 });
                    writeBytes (os, { 'M', 'T', 'r', '?' });
                });

                expect (! file.hasValue());
            }

            {
                // Header, one channel with malformed message
                const auto file = parseFile ([] (OutputStream& os)
                {
                    writeBytes (os, { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 1, 0, 1 });
                    writeBytes (os, { 'M', 'T', 'r', 'k', 0, 0, 0, 1, 0xff });
                });

                expect (file.hasValue());
                expectEquals (file->getNumTracks(), 1);
                expectEquals (file->getTrack (0)->getNumEvents(), 0);
            }

            {
                // Header, one channel with incorrect length message
                const auto file = parseFile ([] (OutputStream& os)
                {
                    writeBytes (os, { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 1, 0, 1 });
                    writeBytes (os, { 'M', 'T', 'r', 'k', 0x0f, 0, 0, 0, 0xff });
                });

                expect (! file.hasValue());
            }

            {
                // Header, one channel, all well-formed
                const auto file = parseFile ([] (OutputStream& os)
                {
                    writeBytes (os, { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 1, 0, 1 });
                    writeBytes (os, { 'M', 'T', 'r', 'k', 0, 0, 0, 4 });

                    MidiFileHelpers::writeVariableLengthInt (os, 0x0f);
                    writeBytes (os, { 0x80, 0x00, 0x00 });
                });

                expect (file.hasValue());
                expectEquals (file->getNumTracks(), 1);

                auto& track = *file->getTrack (0);
                expectEquals (track.getNumEvents(), 1);
                expect (track.getEventPointer (0)->message.isNoteOff());
                expectEquals (track.getEventPointer (0)->message.getTimeStamp(), (double) 0x0f);
            }
        }
    }

    template <typename Fn>
    static MidiMessageSequence parseSequence (Fn&& fn)
    {
        MemoryOutputStream os;
        fn (os);

        return MidiFileHelpers::readTrack (reinterpret_cast<const uint8*> (os.getData()),
                                           (int) os.getDataSize());
    }

    template <typename Fn>
    static Optional<MidiFileHelpers::HeaderDetails> parseHeader (Fn&& fn)
    {
        MemoryOutputStream os;
        fn (os);

        return MidiFileHelpers::parseMidiHeader (reinterpret_cast<const uint8*> (os.getData()),
                                                 os.getDataSize());
    }

    template <typename Fn>
    static Optional<MidiFile> parseFile (Fn&& fn)
    {
        MemoryOutputStream os;
        fn (os);

        MemoryInputStream is (os.getData(), os.getDataSize(), false);
        MidiFile mf;

        int fileType = 0;

        if (mf.readFrom (is, true, &fileType))
            return mf;

        return {};
    }

    static void writeBytes (OutputStream& os, const std::vector<uint8>& bytes)
    {
        for (const auto& byte : bytes)
            os.writeByte ((char) byte);
    }
};

static MidiFileTest midiFileTests;
} // namespace MidiFileHelpers

#endif // JUCE_UNIT_TESTS
