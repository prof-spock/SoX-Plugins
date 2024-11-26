# -*- coding: utf-8 -*-
#
# Local Settings for the C++-Compiler in CMAKE for the
# SoXPlugins

# plugin suite settings
LIST(APPEND manufacturerAndSuiteDefineClauseList
    JucePlugin_Version=1.0.0
    JucePlugin_VersionCode=0x10000
    JucePlugin_VersionString="1.0.0"
    JucePlugin_ManufacturerWebsite="https://github.com/prof-spock/SoXPlugins"
)

IF(MACOSX)
    # add 'aufx' as AU plugin type
    LIST(APPEND manufacturerAndSuiteDefineClauseList
         JucePlugin_AUMainType=0x61756678
    )
ENDIF(MACOSX)

LIST(APPEND cppDefinitions_common
     _CRT_SECURE_NO_WARNINGS
     _HAS_ITERATOR_DEBUGGING=0
     JUCE_ALLOW_STATIC_NULL_VARIABLES=0
     JUCE_APP_VERSION=1.0.0
     JUCE_APP_VERSION_HEX=0x10000
     JucePlugin_AAXCategory=0
     JucePlugin_IAAType=0x61757278
     JucePlugin_IsMidiEffect=0
     JucePlugin_IsSynth=0
     JucePlugin_ProducesMidiOutput=0
     JucePlugin_RTASCategory=0
     JucePlugin_RTASDisableBypass=0
     JucePlugin_RTASDisableMultiMono=0
     JucePlugin_Vst3Category="Fx"
     JucePlugin_VSTCategory=kPlugCategEffect
     JucePlugin_VSTNumMidiInputs=16
     JucePlugin_VSTNumMidiOutputs=16
     JucePlugin_WantsMidiInput=0
)
