#pragma once

#define JUCE_DISPLAY_SPLASH_SCREEN 0
#define JUCE_REPORT_APP_USAGE 0
#define JUCE_USE_DARK_SPLASH_SCREEN 1

//#define JUCE_PROJUCER_VERSION 0x50407

//==============================================================================
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

//========================== 
// juce_audio_devices flags:
//========================== 

//#define JUCE_USE_WINRT_MIDI 0
//#define JUCE_ASIO 0
//#define JUCE_WASAPI 1
//#define JUCE_WASAPI_EXCLUSIVE 0
//#define JUCE_DIRECTSOUND 1
//#define JUCE_ALSA 1
//#define JUCE_JACK 0
//#define JUCE_BELA 0
//#define JUCE_USE_ANDROID_OBOE 0
//#define JUCE_USE_ANDROID_OPENSLES 0
//#define JUCE_DISABLE_AUDIO_MIXING_WITH_OTHER_APPS 0

//==========================
// juce_audio_formats flags:
//==========================

//#define JUCE_USE_FLAC 1
//#define JUCE_USE_OGGVORBIS 1
//#define JUCE_USE_MP3AUDIOFORMAT 0
//#define JUCE_USE_LAME_AUDIO_FORMAT 0
//#define JUCE_USE_WINDOWS_MEDIA_FORMAT 1

//================================
// juce_audio_plugin_client flags:
//================================

#define   JUCE_VST3_CAN_REPLACE_VST2 0
//#define JUCE_FORCE_USE_LEGACY_PARAM_IDS 0
//#define JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE 0
//#define JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS 1
//#define JUCE_AU_WRAPPERS_SAVE_PROGRAM_STATES 0
//#define JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE 0

//=============================
// juce_audio_processors flags:
//=============================

//#define JUCE_PLUGINHOST_VST 0
//#define JUCE_PLUGINHOST_VST3 0
//#define JUCE_PLUGINHOST_AU 0
//#define JUCE_PLUGINHOST_LADSPA 0

//========================
// juce_audio_utils flags:
//========================

//#define JUCE_USE_CDREADER 0
//#define JUCE_USE_CDBURNER 0

//=================
// juce_core flags:
//=================

//#define JUCE_FORCE_DEBUG 0
//#define JUCE_LOG_ASSERTIONS 0
//#define JUCE_CHECK_MEMORY_LEAKS 1
//#define JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES 0
//#define JUCE_INCLUDE_ZLIB_CODE 1
//#define JUCE_USE_CURL 1
//#define JUCE_LOAD_CURL_SYMBOLS_LAZILY 0
//#define JUCE_CATCH_UNHANDLED_EXCEPTIONS 0
#define JUCE_ALLOW_STATIC_NULL_VARIABLES 0
#define JUCE_STRICT_REFCOUNTEDPOINTER 1

//===================
// juce_events flags:
//===================

//#define JUCE_EXECUTE_APP_SUSPEND_ON_BACKGROUND_TASK 0

//=====================
// juce_graphics flags:
//=====================

//#define JUCE_USE_COREIMAGE_LOADER 1
//#define JUCE_USE_DIRECTWRITE 1
//#define JUCE_DISABLE_COREGRAPHICS_FONT_SMOOTHING 0

//=======================
// juce_gui_basics flags:
//=======================

//#define JUCE_ENABLE_REPAINT_DEBUGGING 0
//#define JUCE_USE_XRANDR 1
//#define JUCE_USE_XINERAMA 1
//#define JUCE_USE_XSHM 1
//#define JUCE_USE_XRENDER 0
//#define JUCE_USE_XCURSOR 1
//#define JUCE_WIN_PER_MONITOR_DPI_AWARE 1

//======================
// juce_gui_extra flags:
//======================

//#define JUCE_WEB_BROWSER 1
//#define JUCE_ENABLE_LIVE_CONSTANT_EDITOR 0

//==============================================================================
#ifndef    JUCE_STANDALONE_APPLICATION
    #if defined(JucePlugin_Name) && defined(JucePlugin_Build_Standalone)
        #define  JUCE_STANDALONE_APPLICATION JucePlugin_Build_Standalone
    #else
        #define  JUCE_STANDALONE_APPLICATION 1
    #endif
#endif

//==============================================================================

#define JucePlugin_Build_VST3        1
#define JucePlugin_Build_Standalone  1

#define JucePlugin_IsSynth              0
#define JucePlugin_WantsMidiInput       0
#define JucePlugin_ProducesMidiOutput   0
#define JucePlugin_IsMidiEffect         0

#define JucePlugin_ManufacturerCode     1234
#define JucePlugin_EditorRequiresKeyboardFocus 0
