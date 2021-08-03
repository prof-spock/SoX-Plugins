# -*- coding: utf-8 -*-
#
# Local Settings for the C++-Compiler in CMAKE for the
# SoXPlugins
#

# --- list of defines for compiler ---

# manufacturer settings
SET(manufacturerAndSuiteDefineClauseList
    JucePlugin_Manufacturer=§DrTT§
    JucePlugin_ManufacturerEmail=§§
    JucePlugin_ManufacturerWebsite=§https://github.com/prof-spock/SoX-Plugins§
    JucePlugin_ManufacturerCode=0x44725454
    JucePlugin_AUManufacturerCode=JucePlugin_ManufacturerCode)

# plugin suite settings
SET(manufacturerAndSuiteDefineClauseList
    ${manufacturerAndSuiteDefineClauseList}
    JucePlugin_Version=1.0.0
    JucePlugin_VersionCode=0x10000
    JucePlugin_VersionString=§1.0.0§)
  
SET(cppDefineClauseList
    _CRT_SECURE_NO_WARNINGS
    _HAS_ITERATOR_DEBUGGING=0
    _LIB
    _UNICODE
    JUCE_ALLOW_STATIC_NULL_VARIABLES=0
    JUCE_APP_VERSION=1.0.0
    JUCE_APP_VERSION_HEX=0x10000
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_DONT_DECLARE_PROJECTINFO=1
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
    JUCE_MODULE_AVAILABLE_juce_audio_basics=1
    JUCE_MODULE_AVAILABLE_juce_audio_devices=1
    JUCE_MODULE_AVAILABLE_juce_audio_formats=1
    JUCE_MODULE_AVAILABLE_juce_audio_plugin_client=1
    JUCE_MODULE_AVAILABLE_juce_audio_processors=1
    JUCE_MODULE_AVAILABLE_juce_audio_utils=1
    JUCE_MODULE_AVAILABLE_juce_core=1
    JUCE_MODULE_AVAILABLE_juce_data_structures=1
    JUCE_MODULE_AVAILABLE_juce_events=1
    JUCE_MODULE_AVAILABLE_juce_graphics=1
    JUCE_MODULE_AVAILABLE_juce_gui_basics=1
    JUCE_MODULE_AVAILABLE_juce_gui_extra=1
    JUCE_REPORT_APP_USAGE=0
    JUCE_SHARED_CODE=1
    JUCE_STANDALONE_APPLICATION=JucePlugin_Build_Standalone
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JucePlugin_AAXCategory=0
    JucePlugin_AAXDisableBypass=0
    JucePlugin_AAXDisableMultiMono=0
    JucePlugin_Build_AAX=0
    JucePlugin_Build_AUv3=0
    JucePlugin_Build_RTAS=0
    JucePlugin_Build_Standalone=0
    JucePlugin_Build_Unity=0
    JucePlugin_Build_VST=0
    JucePlugin_Build_VST3=1
    JucePlugin_EditorRequiresKeyboardFocus=0
    JucePlugin_Enable_IAA=0
    JucePlugin_IAAType=0x61757278
    JucePlugin_IsMidiEffect=0
    JucePlugin_IsSynth=0
    JucePlugin_ProducesMidiOutput=0
    JucePlugin_RTASCategory=0
    JucePlugin_RTASDisableBypass=0
    JucePlugin_RTASDisableMultiMono=0
    JucePlugin_VSTCategory=kPlugCategEffect
    JucePlugin_Vst3Category=§Fx§
    JucePlugin_VSTNumMidiInputs=16
    JucePlugin_VSTNumMidiOutputs=16
    JucePlugin_WantsMidiInput=0
    PRIMITIVE_TYPES_ARE_INLINED
    UNICODE)

# --- add specific settings per platform ---
IF(WINDOWS)
    SET(cppDefineClauseList
        ${cppDefineClauseList}
        JucePlugin_Build_AU=0
        JUCER_VS2017_78A5024=1 
        _WINDOWS
        _WINDLL
        WIN32)
ENDIF(WINDOWS)

IF(MACOSX)
    SET(cppDefineClauseList
        ${cppDefineClauseList}
        JucePlugin_Build_AU=1
        JUCER_XCODE_MAC_F6D2F4CF=1
        APPLE)

    # add 'aufx' as AU plugin type for all effects
    SET(manufacturerAndSuiteDefineClauseList
        ${manufacturerAndSuiteDefineClauseList}
        JucePlugin_AUMainType=0x61756678)

    ENABLE_LANGUAGE(OBJC)
    SET(CMAKE_CXX_STANDARD_REQUIRED False)
ENDIF(MACOSX)

IF(LINUX)
    SET(cppDefineClauseList
        ${cppDefineClauseList}
        JucePlugin_Build_AU=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_USE_XCURSOR=0
        JUCE_USE_XRANDR=0
        JUCE_USE_XRENDER=0
        LINUX=1
        UNIX)
ENDIF(LINUX)

# --- combine defines into single list ---
SET(cppDefineClauseList
    ${cppDefineClauseList}
    ${manufacturerAndSuiteDefineClauseList})
  
# --- define flags per compiler ---
IF(MSVC)
    # --- list of warning number to be ignored
    SET(warningNumberList
        4100 4244 4505 4723 5105 26439 26451 26495 26498 26812 28182)
  
    STRING(JOIN " " cppFlagsCommon
           /arch:AVX            # enable AVX vectorization instructions
           /bigobj              # increases number of addressable sections
           /diagnostics:column  # format of diagnostics message
           /EHsc                # exception handling: stack unwinding
           /Gd                  # cdecl calling convention
           /GS                  # buffers security check
           /MDd                 # creates debug multithreaded DLL
           /MP                  # multi processor compilation
           /nologo              # suppresses display of banner
           /Ob2                 # any suitable inline expansion
           /Oi                  # generates intrinsic functions
           /Ot                  # favors fast code
           /Ox                  # subset of optimization
           /permissive-         # set strict standard conformance
           /W4                  # warning level 4
           /Zc:forScope         # standard conformance for scoping
           /Zc:inline           # remove unreferenced functions
           /Zc:preprocessor     # conforming preprocessor
           /Zc:wchar_t          # wchar is native
    )

    # --- disable all warnings in warningNumberList ---
    FOREACH(warningNumber ${warningNumberList})
        STRING(APPEND cppFlagsCommon " /wd${warningNumber}")
    ENDFOREACH()         

    # ---  add all clauses in cppDefineClauseList ---
    FOREACH(defineClause ${cppDefineClauseList})
        STRING(REPLACE "§" "\\\"" cppDefinitionFlag " /D" ${defineClause})
        STRING(APPEND cppFlagsCommon ${cppDefinitionFlag})
    ENDFOREACH()         
  
    STRING(JOIN " " cppFlagsRelease
           /DNDEBUG      # no debugging
           /O2           # generate fast code
           /fp:fast      # fast floating point calculation
    )

    STRING(JOIN " " cppFlagsDebug
           /DDEBUG       # debugging on
           /Z7           # debug information in object files
           /fp:fast      # fast floating point calculation
    )
ELSE()
    STRING(JOIN " " cppFlagsCommon
           -ffast-math             # fast floating point calculation
           -mavx                   # enable AVX vectorization instructions
           -O0                     # no optimization
           -Ofast                  # favors fast code
           -pedantic               # set strict standard conformance
           -Wall                   # warning level: all
           -Wno-delete-incomplete  # remove warning for void deletion
           -Wno-unused-function    # remove warning for unused function
           )

    # warn about undefined symbols when linking
    IF(MACOSX)
        STRING(JOIN " " cppLinkerFlagsCommon
               ${cppLinkerFlagsCommon}
               -Wl,-undefined,error)
    ELSE()
        STRING(JOIN " " cppLinkerFlagsCommon
               ${cppLinkerFlagsCommon}
               -Wl,-no-undefined)
    ENDIF(MACOSX)           

    # ---  add all clauses in cppDefineClauseList ---
    FOREACH(defineClause ${cppDefineClauseList})
        STRING(REPLACE "§" "\\\"" cppDefinitionFlag " -D" ${defineClause})
        STRING(APPEND cppFlagsCommon ${cppDefinitionFlag})
    ENDFOREACH()         
  
    STRING(JOIN " " cppFlagsRelease
           -DNDEBUG         # no debugging
    )

    STRING(JOIN " " cppFlagsDebug
           -DLOGGING_IS_ACTIVE  # logging on
           -DDEBUG              # debugging on
           -g                   # debug information in object files
    )
ENDIF()

SET(CMAKE_CXX_FLAGS ${cppFlagsCommon} CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS_RELEASE ${cppFlagsRelease} CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS_DEBUG ${cppFlagsDebug} CACHE STRING "" FORCE)

SET(CMAKE_SHARED_LINKER_FLAGS ${cppLinkerFlagsCommon} CACHE STRING "" FORCE)
