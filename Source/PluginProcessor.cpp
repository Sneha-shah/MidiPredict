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


/**
 * @brief Prints the details of each MIDI event in a given MIDI buffer.
 *
 * This function iterates through all the MIDI events in the provided MIDI buffer and
 * prints their descriptions and timestamps. It also outputs the buffer's name if provided.
 *
 * @param a The MIDI buffer to iterate through and print event details.
 * @param name (Optional) The name of the buffer, used for labeling the output.
 *
 * @note The function uses standard output (std::cout) for printing the details.
 */
void bufferVals(juce::MidiBuffer& a, juce::String name = "")
{
    // Print the name of the buffer if provided.
    std::cout << name << ":" << std::endl;

    // Iterate through each MIDI event in the buffer.
    for (const auto meta : a)
    {
        // Print the description and timestamp of each MIDI event.
        std::cout << "MIDI Event: " << meta.getMessage().getDescription()
                  << ", " << meta.getMessage().getTimeStamp() << std::endl;
    }

    // Print the end marker for the buffer.
    std::cout << "end " << name << std::endl;
}

/**
 * @brief Prints the details of each MIDI event in a given MIDI message sequence.
 *
 * This function iterates through all the MIDI events in the provided MIDI message sequence and
 * prints their descriptions and timestamps. It also outputs the sequence's name if provided.
 *
 * @param a The MIDI message sequence to iterate through and print event details.
 * @param name (Optional) The name of the sequence, used for labeling the output.
 *
 * @note The function uses standard output (std::cout) for printing the details.
 */
void seqVals(juce::MidiMessageSequence& a, juce::String name = "") 
{
    // Print the name of the sequence if provided.
    std::cout << name << ":" << std::endl;
    
    // Iterate through each MIDI event in the sequence.
    for(const auto meta: a) 
    {
        // Print the description and timestamp of each MIDI event.
        std::cout << "MIDI Event: " << meta->message.getDescription() << ", " << meta->message.getTimeStamp() << std::endl;
    }
    
    // Print the end marker for the sequence.
    std::cout << "end " << name << std::endl;
}

/**
 * @brief Prints details of the first `n` MIDI events in a given MIDI message sequence.
 *
 * This function iterates through the first `n` MIDI events in the provided MIDI message sequence and
 * prints their descriptions and timestamps. It also outputs the sequence's name if provided.
 *
 * @param a The MIDI message sequence to iterate through and print event details.
 * @param name (Optional) The name of the sequence, used for labeling the output. Defaults to an empty string.
 * @param n The number of MIDI events to print from the start of the sequence. Defaults to 50.
 *
 * @note The function uses standard output (std::cout) for printing the details.
 * @warning Ensure `n` does not exceed the number of events in the sequence to avoid out-of-bounds access.
 */
void p50s(juce::MidiMessageSequence& a, juce::String name = "", int n = 50) 
{
    // Print the name of the sequence if provided.
    std::cout << name << ":" << std::endl;
    
    // Iterate through the first `n` MIDI events in the sequence.
    for (int i=0; i<n; i++) 
    {
        // Check if the event index is within the bounds of the sequence.
        if (i >= a.getNumEvents())
        {
            std::cout << "End of available MIDI events in the sequence." << std::endl;
            break;
        }
        
        // Access the event at index `i` and print its description and timestamp.
        auto eventPointer = a.getEventPointer(i);
        if (eventPointer != nullptr) // Ensure the event pointer is valid.
        {
            std::cout << "MIDI Event: "<< eventPointer->message.getDescription() << ", " << eventPointer->message.getTimeStamp() << std::endl;
        }
        else
        {
            std::cout << "Invalid MIDI event at index " << i << "." << std::endl;
        }
    }
    
    // Print the end marker for the sequence.
    std::cout << "end " << name << std::endl;
}

/**
 * @brief Prints details of the first `n` MIDI events from a vector of MIDI buffers.
 *
 * This function iterates through the provided vector of `juce::MidiBuffer` objects and prints the details
 * of the first `n` MIDI events, including their descriptions and timestamps. The timestamps are adjusted
 * to account for the buffer index and a block size of 512 samples.
 *
 * @param a A vector of `juce::MidiBuffer` objects from which to print MIDI event details.
 * @param name (Optional) The name of the collection, used for labeling the output. Defaults to an empty string.
 * @param n The number of MIDI events to print. Defaults to 50.
 *
 * @note The function outputs details using standard output (std::cout).
 * @warning Ensure that `n` does not exceed the total number of events across all buffers to avoid premature termination.
 */
void p50b(std::vector<juce::MidiBuffer>& a, juce::String name = "", int n = 50)
{
    // Print the name of the collection if provided.
    std::cout << name << ":" << std::endl;
    
    int count = 0;     // Counter for the number of events printed.
    int buffNum = 0;   // Counter for the current buffer index.
    
    // Iterate through each buffer in the vector.
    for (const auto& buff: a) 
    {
        // Iterate through each MIDI event in the buffer.
        for(const auto meta: buff)
        {
            // Print the MIDI event description and timestamps.
            std::cout << "MIDI Event: "<< meta.getMessage().getDescription() 
                      << ", " << (meta.getMessage().getTimeStamp()) << "  "
                      << (meta.getMessage().getTimeStamp())+(512*buffNum)
                      << std::endl; // assume512 block size
            count++; // Increment the event counter.
            
            // Break if the required number of events have been printed.
            if(count>=n)
                break;
        }
        
        buffNum++; // Increment the buffer index.
        
        // Break if the required number of events have been printed.
        if(count>=n)
            break;
    }
    
    // Print the end marker for the collection.
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

/**
 * @brief Prints the current state of the PluginProcessor class to the standard output.
 *
 * This function outputs various state information of the PluginProcessor class, including MIDI buffer
 * contents, sequence values, timing adjustments, buffer indices, and note density metrics. The information
 * is printed to the standard output (std::cout).
 *
 * The function helps in debugging by providing a snapshot of the internal state of the class at a given
 * moment.
 *
 * @note Ensure that this function is called during debugging to avoid cluttering the standard output in production.
 */
void PluginProcessor::printClassState() {
    std::cout << "Note density in curr block is: " << noteDensity_pred << std::endl;
    p50b(prevPredictions, "prevpred", 20);
    bufferVals(liveBuffer, "liveBuff");
    bufferVals(recordedBuffer, "recBuff");
//    bufferVals(predBuffer, "predBuff"); // not class variable
    
    seqVals(unmatchedNotes_pred, "unmatched_pred");
    seqVals(unmatchedNotes_live, "unmatched_live");
    
    std::cout << "timeAdjLive: " << timeAdjLive << std::endl;
    std::cout << "timeAdjPred: " << timeAdjPred << std::endl;
    std::cout << "timeBetween: " << timeBetween << std::endl;
    
    std::cout << "predictionBufferIndex: " << predictionBufferIndex << std::endl;
    std::cout << "predictionPlaybackIndex: " << predictionPlaybackIndex << std::endl;
    std::cout << "currentBufferIndexLive: " << currentBufferIndexLive << std::endl;
    std::cout << "currentPositionRecMidi: " << currentPositionRecMidi << std::endl;
    std::cout << "currentPositionRecSamples: " << currentPositionRecSamples << std::endl;
    std::cout << "lagPositionPredSamples: " << lagPositionPredSamples << std::endl;
      
    // For Note Density Prediction
    std::cout << "noteDensity_pred: " << noteDensity_pred << std::endl;
    std::cout << "num_notes_predicted: " << num_notes_predicted << std::endl;
    std::cout << "num_notes_network: " << num_notes_network << std::endl;
    
}


/**
 * @brief Reads a MIDI file and generates MIDI buffers for each block, returning them as a vector.
 *
 * This function reads a MIDI file and processes it into MIDI buffers, where each buffer corresponds
 * to a block of audio samples. The MIDI events within each buffer are timestamped in samples. The
 * function can handle speed adjustments and limit the number of blocks processed.
 *
 * @param midiFile The MIDI file to be read.
 * @param sampleRate The sample rate of the audio, used for timestamp calculations.
 * @param blockSize The size of each audio block in samples.
 * @param maxBlocks The maximum number of blocks to process. If set to a negative value, there is no limit.
 * @param speedShift A multiplier for the event timestamps. Values less than 1 speed up the MIDI, and values greater than 1 slow it down.
 * @return A vector of MIDI buffers. Each buffer contains MIDI events for a corresponding block of audio.
 *         Timestamps are stored in samples
 *         If an error occurs during file reading or processing, an empty vector is returned.
 */
std::vector<juce::MidiBuffer> readMIDIFile(const juce::File& midiFile, double sampleRate, int blockSize, int maxBlocks = -1, double speedShift = 1.0)
{
    juce::MidiFile midiFileData;
    juce::MidiBuffer loadedMidiBuffer;

    // Attempt to open the MIDI file for reading
    juce::FileInputStream fileInputStream(midiFile);

    if (fileInputStream.openedOk())
    {
        if (midiFileData.readFrom(fileInputStream))
        {
            // Convert units from ticks to seconds
            midiFileData.convertTimestampTicksToSeconds();
            
            // Calculate the total number of blocks needed
            int numBlocks = static_cast<int>(midiFileData.getLastTimestamp() * speedShift * sampleRate / blockSize) + 1;
            if (maxBlocks > 0) {
                numBlocks = std::min(numBlocks, maxBlocks);
            }
            
            // Initialize a vector of MIDI buffers with the calculated number of blocks
            std::vector<juce::MidiBuffer> midiBuffers(numBlocks); // Initialize vector

            // Process each track in the MIDI file
            for (int trackIndex = 0; trackIndex < midiFileData.getNumTracks(); ++trackIndex)
            {
                const juce::MidiMessageSequence& track = *midiFileData.getTrack(trackIndex);

                // For every MIDI event in the track, add to the corresponding buffer
                for (int eventIndex = 0; eventIndex < track.getNumEvents(); ++eventIndex)
                {
                    const juce::MidiMessage& midiMessage = track.getEventPointer(eventIndex)->message;
                    double timeStamp_sec = midiMessage.getTimeStamp(); // seconds
                    
                    // Calculate the block index for the current event
                    int blockIndex = static_cast<int>(speedShift * timeStamp_sec * sampleRate / blockSize);
                    
                    // If the block index exceeds the number of blocks, stop processing further
                    if (blockIndex >= numBlocks)
                        break;

                    // Create a buffer for this block if not created yet
                    if (midiBuffers[blockIndex].isEmpty())
                    {
                        midiBuffers[blockIndex] = juce::MidiBuffer();
                    }

                    // Add the MIDI message to the buffer for the corresponding block
                    midiBuffers[blockIndex].addEvent(midiMessage, static_cast<int>(speedShift * timeStamp_sec * sampleRate) % blockSize); // timeStamp in samples
                }
            }

            // Return the vector of MIDI buffers
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

/**
 * @brief Reads a MIDI file and generates a MIDI message sequence with adjusted timestamps.
 *
 * This function reads a MIDI file and creates a `juce::MidiMessageSequence` that contains all the
 * MIDI events from the file. The events' timestamps are adjusted according to the provided
 * sample rate and speed shift. The function handles errors gracefully, returning an empty
 * sequence and logging errors if they occur.
 *
 * @param midiFile The MIDI file to read. This should be a valid JUCE `File` object representing the path to the MIDI file.
 * @param sampleRate The sample rate of the audio, used to convert event timestamps from seconds to samples.
 * @param speedShift A multiplier for the event timestamps. A value greater than 1.0 slows down the MIDI playback, while a value less than 1.0 speeds it up.
 * @return A `juce::MidiMessageSequence` containing all the MIDI events from the file with adjusted timestamps.
 *         If an error occurs during file reading or processing, an empty sequence is returned.
 */
juce::MidiMessageSequence readMIDIFile(const juce::File& midiFile, double sampleRate, double speedShift = 1)
{
    juce::MidiFile midiFileData; // Object to hold the MIDI file data
    juce::FileInputStream fileInputStream(midiFile); // Input stream to read the MIDI file

    // Check if the file input stream opened successfully
    if (fileInputStream.openedOk())
    {
        // Attempt to read the MIDI file
        if (midiFileData.readFrom(fileInputStream))
        {
            // Convert MIDI timestamps from ticks to seconds for easier handling
            midiFileData.convertTimestampTicksToSeconds();
            
            juce::MidiMessageSequence loadedMidiSequence; // Object to store the loaded MIDI messages

            // Iterate through all tracks in the MIDI file
            for (int trackIndex = 0; trackIndex < midiFileData.getNumTracks(); ++trackIndex)
            {
                const juce::MidiMessageSequence& track = *midiFileData.getTrack(trackIndex);

                // Iterate through all events in the current track
                for (int eventIndex = 0; eventIndex < track.getNumEvents(); ++eventIndex)
                {
                    // Get the MIDI message for the current event
                    const juce::MidiMessage& midiMessage = track.getEventPointer(eventIndex)->message;
                    
                    // Get the timestamp of the event in seconds
                    double timeStamp_sec = midiMessage.getTimeStamp(); // seconds
                    
                    // Convert the timestamp to samples
                    int timeStamp_samples = static_cast<int>(timeStamp_sec * sampleRate);
                    
                    // Adjust the timestamp according to the speed shift and add the event to the sequence
                    loadedMidiSequence.addEvent(midiMessage, speedShift*timeStamp_samples - timeStamp_sec);
                }
            }
            
            // Return the loaded MIDI sequence
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

/**
 * @brief Generates a MIDI buffer from a given MIDI message sequence within a specified range of samples.
 *
 * This function creates a `juce::MidiBuffer` from the provided `juce::MidiMessageSequence`.
 * It adds MIDI messages to the buffer that occur within the current sample position up to the specified range.
 * If the timestamps of the MIDI messages are out of order, an error message is printed.
 *
 * @param midiMessageSequence The sequence of MIDI messages to convert to a buffer.
 * @param sampleRate The sample rate of the audio, used to interpret the message timestamps.
 * @param readSamples The number of samples to read ahead from the current sample position.
 * @return A `juce::MidiBuffer` containing the MIDI messages within the specified sample range.
 *         The timestamps in the buffer are relative to the `currentPositionRecSamples`.
 */
juce::MidiBuffer PluginProcessor::generateMidiBuffer(const juce::MidiMessageSequence& midiMessageSequence, double sampleRate, int readSamples)
{
    juce::MidiBuffer midiBuffer; // Initialize an empty MIDI buffer

    // Iterate over the MIDI events starting from the current position
    for (int eventIndex = currentPositionRecMidi; eventIndex < midiMessageSequence.getNumEvents(); ++eventIndex)
    {
        // Get the MIDI message and its timestamp in samples
        const juce::MidiMessage& midiMessage = midiMessageSequence.getEventPointer(eventIndex)->message;
        int timeStamp_samples = midiMessage.getTimeStamp(); // Assuming timestamp is in samples

        // Check for out-of-order timestamps
        if (timeStamp_samples < currentPositionRecSamples)
        {
            std::cerr << "\nERROR IN TIMESTAMP COUNTING!" << std::endl; // warn user
        }

        // Check if the timestamp is within the current block range
        if (timeStamp_samples < currentPositionRecSamples + readSamples)
        {
            // Add the MIDI message to the buffer with a relative timestamp
            midiBuffer.addEvent(midiMessage, timeStamp_samples - currentPositionRecSamples);
        }
        else
        {
            // If the timestamp exceeds the current block range, stop processing
            break;
        }
    }

    // Return the populated MIDI buffer
    return midiBuffer;
}

/**
 * @brief Prepares the processor for playback with the given sample rate and buffer size.
 *
 * This function initializes various parameters and resources required for playback.
 * It loads MIDI files, sets up the synthesizer, and initializes variables for predictions and note density calculations.
 *
 * @param sampleRate The sample rate of the audio.
 * @param samplesPerBlock The size of each audio block.
 */
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    if (DEBUG_FLAG) {
        std::cout << "Samples per Block = " << samplesPerBlock << "\n";
        std::cout << "SampleRate = " << sampleRate << "\n";
        std::cout << "Number of blocks per second = " << sampleRate / samplesPerBlock << "\n";
    }

    // For testing
    double speedChange = 0.5;

    // Initialize live MIDI buffer if in live mode
    if (MODE == 0) {
        auto myMidiFile_live = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
            .getChildFile("Contents")
            .getChildFile("Resources")
            .getChildFile("ladispute_paused.mid");
        jassert(myMidiFile_live.existsAsFile());
        liveMidi = readMIDIFile(myMidiFile_live, sampleRate, samplesPerBlock);
        currentBufferIndexLive = 0;
    } else if (MODE == 1) {
        currentBufferIndexLive = -1;
    }

    // Read recorded MIDI file for practice performance
    auto myMidiFile_rec = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
            .getChildFile("Contents")
            .getChildFile("Resources")
            .getChildFile("ladispute_1.mid");
    jassert(myMidiFile_rec.existsAsFile());
    recordedMidiSequence = readMIDIFile(myMidiFile_rec, sampleRate, speedChange);
    currentPositionRecMidi = 0;
    currentPositionRecSamples = 0;
    lagPositionPredSamples = 0;

    // Setting lag for predictions and processing outputs - for demonstrating predictions in time with live
    lag = 20; // number of blocks
    std::vector<juce::MidiBuffer> recordedMidi = readMIDIFile(myMidiFile_rec, sampleRate, samplesPerBlock, lag, speedChange);
    for (int i = 0; i < lag; i++) {
        prevPredictions.push_back(recordedMidi[i]);
        currentPositionRecMidi += recordedMidi[i].getNumEvents();
        currentPositionRecSamples += samplesPerBlock;
    }
    predictionBufferIndex = 0;
    predictionPlaybackIndex = 0;

    // Prepare Synthesizer
    synthAudioSource.prepareToPlay(samplesPerBlock, sampleRate);

    // Initialize parameters for PausePlay Predictions
    timeBetween = 0.2 * sampleRate; // in samples
    unmatchedNotes_pred.clear();
    unmatchedNotes_live.clear();

    // Initialize parameters for Note Density Prediction
    noteDensity_pred = 1;
    num_notes_predicted = 0;
    num_notes_network = 0;
    alpha = 0.99;
    numBlocksForDensity = 10 * sampleRate / samplesPerBlock; // Convert seconds to blocks
    prev50Pred = std::vector<int>(numBlocksForDensity);
    prev50Live = std::vector<int>(numBlocksForDensity);
    prev50PredIndex = 0;
    prev50LiveIndex = 0;

    if (DEBUG_FLAG) {
        p50b(recordedMidi, "recordedMidiBuffers", 50);
        p50b(liveMidi, "liveMidi", 50);
        std::cout << std::endl;
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

/**
 * @brief Combines MIDI events from buffer b into buffer a with an optional note offset.
 *
 * This function combines MIDI events from buffer b into buffer a. If a note offset is specified,
 * note on events in buffer b will be transposed by the specified offset before being added to buffer a.
 *
 * @param a The destination MIDI buffer where events will be added.
 * @param b The source MIDI buffer containing events to be combined.
 * @param numSamples The maximum number of samples to process from buffer b (-1 to process all).
 * @param noteOffset The offset to apply to note events from buffer b before adding them to buffer a.
 *                   If set to 0 (default), events are added without modification.
 */
void PluginProcessor::combineEvents(juce::MidiBuffer& a, juce::MidiBuffer& b, int numSamples = -1, int noteOffset = 0)
{
    // If noteOffset is 0, simply add events from buffer b to buffer a without modification
    if (noteOffset == 0) {
        a.addEvents(b, 0, numSamples, 0);
        return;
    }
    
    // Iterate over each event in buffer b
    for (const auto meta : b)
    {
        const juce::MidiMessage m = meta.getMessage();
        
        // Break loop if numSamples is specified and current event timestamp exceeds it
        if (numSamples > 0 && m.getTimeStamp() > numSamples)
            break;
        
        // Check if the message is a note on or note off event
        if (m.isNoteOnOrOff())
        {
            const juce::uint8 vel = m.getVelocity();
            // Create a new note on message with the note offset applied
            juce::MidiMessage newMsg = juce::MidiMessage::noteOn(m.getChannel(), m.getNoteNumber() + noteOffset, vel);
            
            // Add the new message to buffer a with the original timestamp
            a.addEvent(newMsg, m.getTimeStamp());
        }
        else
        {
            // For other types of MIDI events, simply add them to buffer a without modification
            a.addEvent(m, m.getTimeStamp());
        }
    }
}

/**
 * @brief Searches for a MIDI note in the live buffer and returns a found flag.
 *
 * This function searches for a MIDI note in the live buffer (`unmatchedNotes_live`).
 * If the note is found, it deletes the corresponding event from the buffer and returns true.
 * If the note is not found, it returns false.
 *
 * @param m The MIDI message representing the note to search for.
 * @return True if the note is found in the live buffer, false otherwise.
 */
bool PluginProcessor::searchLive(juce::MidiMessage m) {
    bool found = false; // Flag to indicate if the note is found
    juce::MidiMessage mLive;

    // Iterate over the events in the live buffer
    for (int i = 0; i < unmatchedNotes_live.getNumEvents(); i++) {
        mLive = unmatchedNotes_live.getEventPointer(i)->message;
        
        // Check if the live event timestamp is within the acceptable time range
        if (mLive.getTimeStamp() - timeAdjLive < m.getTimeStamp() - timeBetween - timeAdjPred) { // TO DO: Also need to take into account how much live and prediction are out of sync by
            // If the live event is too far ahead of the predicted event, stop searching
            
//            unmatchedNotes_live.deleteEvent(i,0);
//            i--;
//            std::cout << std::endl << "\nExtra Note found in LIVE!!" << std::endl;
//            std::cout << "DELETED EVENT" << std::endl;
//            break;
        }

        // Check if the live event timestamp is after the predicted event timestamp
        if (mLive.getTimeStamp() - timeAdjLive > m.getTimeStamp() - timeAdjPred) {
            // If the live event is beyond the predicted event, stop searching
            break;
        }

        // Check if the note numbers match
        if (mLive.getNoteNumber() == m.getNoteNumber()) {
            // If the note is found, delete the event from the live buffer and set found to true
            found = true;
            unmatchedNotes_live.deleteEvent(i, 0);
            break;
        }
    }

    // Return the result indicating whether the note is found
    return found;
}

/**
 * @brief Checks if the processing should be paused based on the MIDI buffers.
 *
 * This function compares the MIDI events in the prediction buffer (`predBuffer`) with
 * the events in the live buffer (`liveBuffer`). It determines whether the processing
 * should be paused based on the availability and matching of MIDI events between the two buffers.
 *
 * @param predBuffer The MIDI buffer containing predicted MIDI events.
 * @param liveBuffer The MIDI buffer containing live MIDI events.
 * @param blockSize The number of samples in every block.
 * @return True if the processing should be paused, false otherwise.
 */
bool PluginProcessor::checkIfPause(juce::MidiBuffer& predBuffer, juce::MidiBuffer& liveBuffer, int blockSize)
{
    // TO DO: Use timestamp difference between some section of music for more accurate
    // Assumptions: Live Buffer is at the same speed or slower than Prediction
    // Assumption: Order of predBuffer and liveBuffer is exactly same, even for simultaneous notes
    
    if (DEBUG_FLAG) {
        bufferVals(liveBuffer, "LiveBuff");
        bufferVals(predBuffer, "PredBuff");
        seqVals(unmatchedNotes_pred, "unmatched_pred");
        seqVals(unmatchedNotes_live, "unmatched_live");
        std::cout << "timeAdjLive: " << timeAdjLive << std::endl;
        std::cout << "timeAdjPred: " << timeAdjPred << std::endl;
    }
    
    juce::MidiMessage m; // QUES: use by reference? juce::MidiMessage&
    bool pause = false;
    
    // to maintain order of notes
    if (unmatchedNotes_live.getNumEvents() > 0)
        timeAdjLive += blockSize; // checkIfPause being called once every block
    else
        timeAdjLive = 0;
    // copying events from liveBuffer to unmatchedNotes_live
    for (auto metaB : liveBuffer)
    {
        if (metaB.getMessage().isNoteOnOrOff())
        {
            unmatchedNotes_live.addEvent(metaB.getMessage(), timeAdjLive);
        }
    }
    
    // for pred seq
//    if (unmatchedNotes_pred.getNumEvents() > 0)
//        timeAdjPred += blockSize; // checkIfPause being called once every block
//    else
        timeAdjPred = 0;

    // Loop through first unmatchedNotes_pred search in live events
    while (unmatchedNotes_pred.getNumEvents() > 0)
    {
        m = unmatchedNotes_pred.getEventPointer(0)->message;
        if (!m.isNoteOnOrOff()) { // Not required?
            unmatchedNotes_pred.deleteEvent(0,0);
            continue;
        }
        pause = !searchLive(m);
        if (pause) // If note not found in buffer b, set pause flag
            break;
        else
            unmatchedNotes_pred.deleteEvent(0,0);
    }
    // Then loop through prediction buffer and search for events in live events
    for (const auto meta : predBuffer)
    {
//            if (DEBUG_FLAG && unmatchedNotes_live.getNumEvents() > 0)
//                std::cout << "\nCatching up to live now...\n\n";
        m = meta.getMessage();
        if (! m.isNoteOnOrOff())
            continue;

        if(!pause)
            pause = !searchLive(m);

        if (pause) { // save notes to search later
            unmatchedNotes_pred.addEvent(m, timeAdjPred);
        }
    }
    
//    if (pause) {
//        timeAdjPred -= blockSize;
//    }
    return pause;
}

/**
 * @brief Updates the note density based on MIDI buffers.
 *
 * This function calculates the note density using MIDI events from prediction buffer (`predBuffer`)
 * and live buffer (`liveBuffer`). It maintains a moving window of MIDI events to track changes in note density over time.
 *
 * @param predBuffer The MIDI buffer containing predicted MIDI events.
 * @param liveBuffer The MIDI buffer containing live MIDI events.
 */
void PluginProcessor::updateNoteDensity(juce::MidiBuffer& predBuffer, juce::MidiBuffer& liveBuffer) {
    // TO DO: Use timestamp difference of x notes rather than number of notes in a timeframe
    // TO DO: Implement some kind of exponential weight decay in timestamp differences? Take into account current async with live, its improvement/ depreciation wrt previous predictions and also the rate of change (improvement)
    // For example, if noteDensity_pred changes dramatically, need to somehow account for that so it doesn't keep oscillating but reaches steady state faster
    
    // use noteDensity in upto numBlocksForDensity buffers
    num_notes_predicted += predBuffer.getNumEvents() - prev50Pred[prev50PredIndex];
    num_notes_network += liveBuffer.getNumEvents() - prev50Live[prev50LiveIndex];
    
    prev50Pred[prev50PredIndex] = predBuffer.getNumEvents();
    prev50Live[prev50LiveIndex] = liveBuffer.getNumEvents();
    prev50PredIndex = (prev50PredIndex + 1) % std::max<int>(numBlocksForDensity,1);
    prev50LiveIndex = (prev50LiveIndex + 1) % std::max<int>(numBlocksForDensity,1);
    
    float noteDensity_network = num_notes_network/ num_notes_predicted; // tempo estimation using number of notes played so far as compared to prediction
//    noteDensity_pred = alpha*noteDensity_pred + (1-alpha) * noteDensity_network;
    noteDensity_pred = alpha * noteDensity_pred + (1-alpha) * noteDensity_pred * noteDensity_network;
}

/**
 * @brief Updates the recorded and live MIDI buffers for processing.
 *
 * This function updates the recorded and live MIDI buffers used for processing.
 * It generates the recorded buffer based on the recorded MIDI sequence and current note density prediction.
 * The live buffer is updated based on the selected mode: either from pre-loaded MIDI data or real-time MIDI input.
 *
 * @param blockSize The size of each audio block.
 * @param midiMessages The MIDI buffer containing live MIDI events.
 */
void PluginProcessor::getBuffers(int blockSize, juce::MidiBuffer& midiMessages) {
    recordedBuffer = generateMidiBuffer(recordedMidiSequence, getSampleRate(), ((int)noteDensity_pred+1)*blockSize); // read req blocks of rec data
    if (MODE == 0)
        liveBuffer = liveMidi[currentBufferIndexLive];
    else if (MODE == 1)
        liveBuffer = midiMessages;

    ++currentBufferIndexLive;
    
    if (DEBUG_FLAG) {
        p50b(prevPredictions, "prevpred", 20);
        if (liveBuffer.getNumEvents() > 0)
            bufferVals(liveBuffer, "liveBuffer");
        if (recordedBuffer.getNumEvents() > 0)
            bufferVals(recordedBuffer, "recordedBuffer");
        std::cout << std::endl;
    }
    
    //    if (currentBufferIndex >= recordedMidi.size())
    //    {
    //        recordedBuffer = midiMessages;
    //        --currentBufferIndex;
    //    }
}

/**
 * @brief Sets prediction variables based on the prediction case.
 *
 * This function sets prediction variables based on the selected prediction case.
 * It handles different prediction scenarios such as simple playback, pausing when no input is seen,
 * and implementing rough tempo tracking.
 *
 * @param predictionCase The selected prediction case:
 *                      - 1: Simplest prediction case - playback recording as is.
 *                      - 2: Pause when no input seen.
 *                      - 3: Implement rough tempo tracking.
 * @param numSamples The number of samples in every block.
 * @return True if the processing should be paused, false otherwise.
 */
bool PluginProcessor::setPredictionVariables(int predictionCase, int numSamples) {

    bool paused = false;
    
    if (predictionCase == 1) {
        // 1. Simplest prediction case - playback recording as is
        // do nothing
    } else if (predictionCase == 2) {
        // 2. Pause when no input seen
        // If for any note in predBuffer (at current - lag), there is no corresponding note in liveBuffer, pause
        // If there is a note in liveBuffer that is not in recBuffer, play it

        paused = checkIfPause(prevPredictions[predictionBufferIndex], liveBuffer, numSamples);
        if (DEBUG_FLAG) {
            std::cout << "isPaused: " << paused << std::endl;
            if (paused) {
                std::cout << std::endl;
            }
        }
    } else if (predictionCase == 3) {
        // 3. Implement rough tempo tracking pt 1: (calc note density using number of notes played so far)
        // noteDensity_pred(n) = a*noteDensity_pred(n-1) + (1-a)*noteDensity_network(n-lag)
        paused = checkIfPause(prevPredictions[predictionBufferIndex], liveBuffer, numSamples);
        updateNoteDensity(prevPredictions[predictionBufferIndex], liveBuffer);
        if (DEBUG_FLAG) {
            std::cout << "isPaused: " << paused << std::endl;
            std::cout << "Note density in curr block is: " << noteDensity_pred << std::endl;
        }
    } else {
        std::cout << "Invalid value for predictionCase. Defaulting to prediction case 1." << std::endl;
    }
    return paused;
}

/**
 * @brief Generates MIDI prediction buffer based on the recorded buffer and current tempo.
 *
 * This function generates a MIDI prediction buffer based on the recorded MIDI buffer and the current tempo.
 * It processes each MIDI event from the recorded buffer according to the current tempo and adds it to the prediction buffer.
 * If the processing is paused, an empty buffer is returned.
 *
 * @param numSamples The number of samples in every block.
 * @param paused Flag indicating if processing is paused.
 * @return The generated MIDI prediction buffer.
 */
juce::MidiBuffer PluginProcessor::generate_prediction(int numSamples, bool paused) {
    int time_samp;
    juce::MidiMessage m;
    juce::MidiBuffer midiPrediction {};
    
    // Loop through recordedBuffer and add to prediction buffer according to conditions set above
    if (paused) {
        return {};
    }
    for (const auto meta : recordedBuffer)
    {
        m = meta.getMessage();
        midiKeyboardState.processNextMidiEvent(m); // Let PGM display current note
        auto description = m.getDescription();
        if (DEBUG_FLAG) {
            std::cout << "Iterating rb2 --- MIDI EVENT:" << description << "\n";
        }
        
        // process m according to new tempo
        time_samp = m.getTimeStamp()/noteDensity_pred;
        
        // Add processed midi event to prediction buffer
        midiPrediction.addEvent (m, time_samp);
        currentPositionRecMidi += 1;

        if(time_samp >= numSamples)
            break;
    }
    return midiPrediction;
}

/**
 * @brief Processes a block of audio.
 *
 * This function processes an audio block, handling MIDI messages and generating predictions for playback.
 * It combines recorded MIDI buffers with live MIDI events and prediction buffers to synthesize audio.
 * The processing behavior is determined based on the prediction case set.
 *
 * @param buffer The audio buffer to process.
 * @param midiMessages The MIDI buffer containing incoming MIDI events.
 */
void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (sampleRate_ == 0.0) {
        // Sample rate not set, return without processing
        return;
    }
    // Obsolete API (which still works): for (MidiBuffer::Iterator i (midiMessages); i.getNextEvent (m, time);)
    
    // MAGIC GUI: send midi messages to the keyboard state and MidiLearn
    magicState.processMidiBuffer (midiMessages, buffer.getNumSamples(), true);
    // MAGIC GUI: send playhead information to the GUI
    magicState.updatePlayheadInformation (getPlayHead());
    
    // Source 1 (history) recordedBuffer - 2 blocks (lag amount of time in the future of live)
    // Source 2 (rn from file) liveBuffer - 1 block
    getBuffers(buffer.getNumSamples(), midiMessages);
    
    int predictionCase = 3;
//    int PLAYBACK = 1; // Playback midi file as is DONE
//    int PAUSE = 2; // Playback midi file, and if delayed input, pause playback. Add a 1 block speedup when live is ahead
//    int TEMPO_EXP = 3; // Implement tempo tracking: tempo_prac(n) = a*tempo_prac(n-1) + (1-a)*tempo_network(n-lag)
    bool isPaused = setPredictionVariables(predictionCase, buffer.getNumSamples());
    
    // Use recordedBuffer to generate midiPrediction for playback
    // Sets isPaused through return and noteDensity_pred internally
    juce::MidiBuffer midiPrediction = generate_prediction(buffer.getNumSamples(), isPaused);
    
    // Process midi events and buffer for synthesizer
    juce::AudioSourceChannelInfo bufferInfo;
    bufferInfo.buffer = &buffer;  // Assuming buffer is your AudioBuffer<float>
    bufferInfo.startSample = 0;
    bufferInfo.numSamples = buffer.getNumSamples();
    
    // Combine prediction and live performance ofr playback (prevPredictions[predictionBufferIndex] and midiMessages)
    juce::MidiBuffer midiCombined; // midi file + current keyboard // ideally use different voices for each playback
    //    combineEvents(midiCombined, recordedBuffer, buffer.getNumSamples());
    //    combineEvents(midiCombined, recordedBuffer2, buffer.getNumSamples());
    combineEvents(midiCombined, prevPredictions[predictionBufferIndex], -1, 0); // Uses addEvents with MidiBuffer&
    combineEvents(midiCombined, liveBuffer);
    combineEvents(midiCombined, midiMessages);
    
    // play prediction notes using synthesizer
    synthAudioSource.getNextAudioBlock(bufferInfo, midiCombined);
    // TO DO: Have dual channel synthesize (eg. 2 voices or left and right ear) to avoid note on annd offs getting mixed up

    // For plugin to forward it (Midi Filter Plugin case)
    midiMessages.swapWith (prevPredictions[predictionBufferIndex]); 
    
    // Update prediction buffer vectorde
    prevPredictions[predictionBufferIndex] = midiPrediction;
    predictionBufferIndex = (predictionBufferIndex+1) % prevPredictions.size();
    lagPositionPredSamples += buffer.getNumSamples();
    if (!isPaused)
        currentPositionRecSamples += buffer.getNumSamples()*noteDensity_pred;
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
