#
# Local Settings for the C++-Compiler in CMAKE for the
# SoXPlugins
#

# --- list of defines for compiler ---
SET(cppDefineClauseList
    _CRT_SECURE_NO_WARNINGS
    _LIB
    _UNICODE
    _WINDOWS
    _WINDLL
    JUCE_APP_VERSION=1.0.0
    JUCE_APP_VERSION_HEX=0x10000
    JUCE_SHARED_CODE=1
    JucePlugin_Build_AAX=0
    JucePlugin_Build_AU=0
    JucePlugin_Build_AUv3=0
    JucePlugin_Build_RTAS=0
    JucePlugin_Build_Standalone=1
    JucePlugin_Build_Unity=0
    JucePlugin_Build_VST3=1
    JucePlugin_Build_VST=0
    JUCER_VS2017_78A5024=1 
    PRIMITIVE_TYPES_ARE_INLINED
    UNICODE
    WIN32
)


IF(MSVC)
    # --- list of warning number to be ignored
    SET(warningNumberList
        4100 4244 4505 4723 5105 26439 26451 26495 26498 26812 28182)
  
    STRING(JOIN " " cppFlagsCommon
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
        STRING(APPEND cppFlagsCommon " /wd" ${warningNumber})
    ENDFOREACH()         
  
    # ---  add all clauses in cppDefineClauseList ---
    FOREACH(defineClause ${cppDefineClauseList})
        STRING(APPEND cppFlagsCommon " /D" ${defineClause})
    ENDFOREACH()         
  
    STRING(JOIN " " cppFlagsRelease
           /DNDEBUG      # no debugging
           /O2           # generate fast code
           /fp:fast      # fast floating point calculation
    )
ELSE()
    STRING(JOIN " " cppFlagsCommon
           -O2                  # any suitable inline expansion
           -Ofast               # favors fast code
           -pedantic            # set strict standard conformance
           -Wall                # warning level: all
    )

    # ---  add all clauses in cppDefineClauseList ---
    FOREACH(defineClause ${cppDefineClauseList})
        STRING(APPEND cppFlagsCommon " -D" ${defineClause})
    ENDFOREACH()         
  
    STRING(JOIN " " cppFlagsRelease
           -DNDEBUG      # no debugging
           -Ofast        # generate fast code
           /ffast-math   # fast floating point calculation
    )
ENDIF()

SET(CMAKE_CXX_FLAGS ${cppFlagsCommon} CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS_RELEASE ${cppFlagsRelease} CACHE STRING "" FORCE)
