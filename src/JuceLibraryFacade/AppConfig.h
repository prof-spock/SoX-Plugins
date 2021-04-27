#pragma once

#define JUCE_DISPLAY_SPLASH_SCREEN 0
#define JUCE_REPORT_APP_USAGE 0

//===============
//=== MODULES ===
//===============

#define JUCE_MODULE_AVAILABLE_juce_analytics              1
#define JUCE_MODULE_AVAILABLE_juce_audio_basics           1
#define JUCE_MODULE_AVAILABLE_juce_audio_devices          1
#define JUCE_MODULE_AVAILABLE_juce_audio_formats          1
#define JUCE_MODULE_AVAILABLE_juce_audio_plugin_client    1
#define JUCE_MODULE_AVAILABLE_juce_audio_processors       1
#define JUCE_MODULE_AVAILABLE_juce_audio_utils            1
#define JUCE_MODULE_AVAILABLE_juce_core                   1
#define JUCE_MODULE_AVAILABLE_juce_cryptography           1
#define JUCE_MODULE_AVAILABLE_juce_data_structures        1
#define JUCE_MODULE_AVAILABLE_juce_events                 1
#define JUCE_MODULE_AVAILABLE_juce_graphics               1
#define JUCE_MODULE_AVAILABLE_juce_gui_basics             1
#define JUCE_MODULE_AVAILABLE_juce_gui_extra              1
#define JUCE_MODULE_AVAILABLE_juce_opengl                 1

#define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 1

//============================================================

#define JUCE_VST3_CAN_REPLACE_VST2 0
#define JUCE_ALLOW_STATIC_NULL_VARIABLES 0
#define JUCE_STRICT_REFCOUNTEDPOINTER 1
#define JUCE_STANDALONE_APPLICATION 1

#define JucePlugin_Build_VST3        1

#define JucePlugin_IsSynth              0
#define JucePlugin_WantsMidiInput       0
#define JucePlugin_ProducesMidiOutput   0
#define JucePlugin_IsMidiEffect         0

#define JucePlugin_EditorRequiresKeyboardFocus 0

//=============================
//=== MANUFACTURER SETTINGS ===
//=============================

#define JucePlugin_Manufacturer         "SoX Plugins"
#define JucePlugin_ManufacturerEmail    ""
#define JucePlugin_ManufacturerWebsite  ""
#define JucePlugin_ManufacturerCode     1234
#define JucePlugin_VersionString        "1.0.0"
#define JucePlugin_Version              0x10100
