/**
 *
 * Copyright (c) 2013 Pascal Gauthier.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "msfa/controllers.h"
#include "msfa/dx7note.h"
#include "msfa/lfo.h"
#include "msfa/synth.h"
#include "PluginParam.h"
#include "PluginData.h"
#include "PluginFx.h"
#include "SysexComm.h"

struct ProcessorVoice {
    int midi_note;
    bool keydown;
    bool sustained;
    bool live;
    Dx7Note *dx7_note;
};

enum DexedEngineResolution {
    DEXED_RESO_MODERN,
    DEXED_RESO_MARKI,
    DEXED_RESO_OPL
};

//==============================================================================
/**
*/
class DexedAudioProcessor  : public AudioProcessor, public AsyncUpdater, public MidiInputCallback
{
    static const int MAX_ACTIVE_NOTES = 16;
    ProcessorVoice voices[MAX_ACTIVE_NOTES];
    int currentNote;

    // The original DX7 had one single LFO. Later units had an LFO per note.
    Lfo lfo;

    bool sustain;

    // Extra buffering for when GetSamples wants a buffer not a multiple of N
    float extra_buf[N];
    int extra_buf_size;

    int currentProgram;
    
    /**
     * The last time the state was save, to be able to bypass a VST host bug.
     */
    long lastStateSave;
    
    /**
     * PlugFX
     */
    PluginFx fx;

    /**
     * This flag is used in the audio thread to know if the voice has changed
     * and needs to be updated.
     */
    bool refreshVoice;

    bool normalizeDxVelocity;
    bool sendSysexChange;
    
    void processMidiMessage(const MidiMessage *msg);
    void keydown(uint8_t pitch, uint8_t velo);
    void keyup(uint8_t pitch);
    
    /**
     * this is called from the Audio thread to tell
     * to update the UI / hostdata 
     */
    void handleAsyncUpdate();
    void initCtrl();

    PropertiesFile::Options prefOptions;

	MidiMessage* nextMidi,*midiMsg;
	bool hasMidiMessage;
    int midiEventPos;
	bool getNextEvent(MidiBuffer::Iterator* iter,const int samplePos);
    
    void handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message);
    uint32_t engineResolution;
    
public :
    // in MIDI units (0x4000 is neutral)
    Controllers controllers;
    StringArray programNames;    
    char sysex[4096];    
    char data[161];
    
    CartridgeManager cartManager;
    SysexComm sysexComm;
    
    VoiceStatus voiceStatus;
    
    bool forceRefreshUI;
    
    float vuSignal;

    int getEngineResolution();
    void setEngineResolution(int rs);
    
    Array<Ctrl*> ctrl;

    OperatorCtrl opCtrl[6];
    ScopedPointer<CtrlDX> pitchEgRate[4];
    ScopedPointer<CtrlDX> pitchEgLevel[4];
    ScopedPointer<CtrlDX> pitchModSens;
    ScopedPointer<CtrlDX> algo;
    ScopedPointer<CtrlDX> oscSync;
    ScopedPointer<CtrlDX> feedback;
    ScopedPointer<CtrlDX> lfoRate;
    ScopedPointer<CtrlDX> lfoDelay;
    ScopedPointer<CtrlDX> lfoAmpDepth;
    ScopedPointer<CtrlDX> lfoPitchDepth;
    ScopedPointer<CtrlDX> lfoWaveform;
    ScopedPointer<CtrlDX> lfoSync;
    ScopedPointer<CtrlDX> transpose;

    ScopedPointer<CtrlFloat> fxCutoff;
    ScopedPointer<CtrlFloat> fxReso;
    ScopedPointer<CtrlFloat> output;

    int importSysex(const char *imported);
    void setDxValue(int offset, int v);

    //==============================================================================
    DexedAudioProcessor();
    ~DexedAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void panic();
    
    //==============================================================================
    AudioProcessorEditor* createEditor();
    bool hasEditor() const;
    void updateUI();
    bool peekVoiceStatus();
    void unpackProgram(int idx);
    void updateProgramFromSysex(const uint8 *rawdata);
    void loadBuiltin(int idx);
    //==============================================================================
    const String getName() const;
    int getNumParameters();
    float getParameter (int index);
    void setParameter (int index, float newValue);
    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;

    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);
    
    //void getCurrentProgramStateInformation (MemoryBlock& destData);
    //void setCurrentProgramStateInformation (const void* data, int sizeInBytes);
    //==============================================================================
    
    // this is kept up to date with the midi messages that arrive, and the UI component
    // registers with it so it can represent the incoming messages
    MidiKeyboardState keyboardState;
    void unbindUI();

    void loadPreference();
    void savePreference();
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DexedAudioProcessor)

};

#endif  // PLUGINPROCESSOR_H_INCLUDED
