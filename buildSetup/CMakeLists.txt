#########################################
# CMAKE file for the SoXPlugins project #
#                                       #
# Dr. Thomas Tensi, 2022-09             #
#########################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(SoXPlugins
        VERSION 1.0
        LANGUAGES CXX)

# =============================
# === program configuration ===
# =============================

INCLUDE(GlobalSettings.cmake)

# --- compiler ---
INCLUDE(PluginCPPCompilerSettings.cmake)
INCLUDE(CPPCompilerConfiguration.cmake)

# --- LaTeX ---
FIND_PACKAGE(LATEX QUIET OPTIONAL_COMPONENTS PDFLATEX)

# --- DoxyGen ---
FIND_PACKAGE(Doxygen QUIET)

# --- JUCE ---
FIND_PACKAGE(juce QUIET)

# local configuration settings
INCLUDE(LocalConfiguration.cmake)

# ===========================

# list of all plugin names available as plugins
LIST(APPEND pluginNameList
     Compander Filter Gain Overdrive PhaserAndTremolo Reverb)

# a command line test program for optimization
SET(testProgramName "ZZZ_Test-SoXPlugins")

# ===================
# === directories ===
# ===================

# --- source directories ---
CMAKE_PATH(SET srcDirectory NORMALIZE
           ${CMAKE_CURRENT_SOURCE_DIR}/../src)
CMAKE_PATH(SET srcAudioSupportDirectory NORMALIZE
           ${srcDirectory}/Audio)
CMAKE_PATH(SET srcBaseModulesDirectory NORMALIZE
           ${srcDirectory}/BaseModules)
CMAKE_PATH(SET srcBaseTypesDirectory NORMALIZE
           ${srcDirectory}/BaseTypes)
CMAKE_PATH(SET srcHelpersDirectory NORMALIZE
           ${srcDirectory}/Helpers)
CMAKE_PATH(SET srcContainersDirectory NORMALIZE
           ${srcBaseTypesDirectory}/Containers)
CMAKE_PATH(SET srcGenericTypesDirectory NORMALIZE
           ${srcBaseTypesDirectory}/GenericTypes)
CMAKE_PATH(SET srcPrimitivesDirectory NORMALIZE
           ${srcBaseTypesDirectory}/Primitives)
CMAKE_PATH(SET srcJuceFacadeDirectory NORMALIZE
           ${srcDirectory}/Libraries/JuceFacade)
CMAKE_PATH(SET srcViewAndControllerDirectory NORMALIZE
           ${srcDirectory}/ViewAndController)
CMAKE_PATH(SET srcEffectsDirectory NORMALIZE
           ${srcDirectory}/Effects)
CMAKE_PATH(SET srcEffectsTestDirectory NORMALIZE
           ${srcEffectsDirectory}/SoX-Test)

FOREACH(pluginName ${pluginNameList})
    CMAKE_PATH(SET srcEffect${pluginName}Directory NORMALIZE
               ${srcEffectsDirectory}/SoX${pluginName})
ENDFOREACH(pluginName)

CMAKE_PATH(SET testFileDirectory NORMALIZE
           ${CMAKE_CURRENT_SOURCE_DIR}/../test)

# --- include directories ---

CMAKE_PATH(SET juceModulesDirectory NORMALIZE
           ${JUCE_DIR}/modules)

LIST(APPEND standardIncludeDirectoryList
     ${srcAudioSupportDirectory}
     ${srcBaseModulesDirectory}
     ${srcContainersDirectory}
     ${srcGenericTypesDirectory}
     ${srcPrimitivesDirectory}
     ${srcEffectsDirectory}
     ${srcHelpersDirectory}
     ${srcViewAndControllerDirectory}
)

LIST(APPEND additionalIncludeDirectoryList
     ${srcJuceFacadeDirectory}
     ${juceModulesDirectory}
     ${juceModulesDirectory}/juce_audio_plugin_client/AU
     ${juceModulesDirectory}/juce_audio_processors/format_types/VST3_SDK
)

# --- add include directories and libraries for Unix ---
IF(LINUX)
    FIND_PACKAGE(PkgConfig REQUIRED) 
    LIST(APPEND relevantPackageList freetype2)
    PKG_CHECK_MODULES(linuxPackages ${relevantPackageList})
    LIST(APPEND unixIncludeDirectoryList /usr/include/freetype2)
    LIST(APPEND unixLibraryList ${linuxPackages_LINK_LIBRARIES})

    LIST(APPEND additionalIncludeDirectoryList
         ${unixIncludeDirectoryList}
    )
ENDIF(LINUX)

# ====================
# === source files ===
# ====================

LIST(APPEND srcCommonAudioFileList
     ${srcAudioSupportDirectory}/AudioSampleList.cpp
     ${srcAudioSupportDirectory}/AudioSampleListVector.cpp
     ${srcAudioSupportDirectory}/AudioSampleRingBuffer.cpp
     ${srcAudioSupportDirectory}/AudioSampleRingBufferVector.cpp
     ${srcAudioSupportDirectory}/IIRFilter.cpp
     ${srcAudioSupportDirectory}/WaveForm.cpp
)

LIST(APPEND srcBaseModulesFileList
     ${srcBaseModulesDirectory}/File.cpp
     ${srcBaseModulesDirectory}/LoggingSupport.cpp
     ${srcBaseModulesDirectory}/OperatingSystem.cpp
     ${srcBaseModulesDirectory}/StringUtil.cpp
)

LIST(APPEND srcContainersFileList
     ${srcContainersDirectory}/ByteList.cpp
     ${srcContainersDirectory}/Dictionary.cpp
     ${srcContainersDirectory}/IntegerList.cpp
     ${srcContainersDirectory}/NaturalList.cpp
     ${srcContainersDirectory}/RealList.cpp
     ${srcContainersDirectory}/StringList.cpp
     ${srcContainersDirectory}/StringSet.cpp
)

LIST(APPEND srcEffectsBaseFileList
     ${srcEffectsDirectory}/SoXAudioEffect.cpp
)

LIST(APPEND srcHelpersFileList
    ${srcHelpersDirectory}/SoXEffectParameterMap.cpp
)

LIST(APPEND srcPrimitivesFileList
     ${srcPrimitivesDirectory}/MyString.cpp
     ${srcPrimitivesDirectory}/Real.cpp
)

LIST(APPEND srcViewAndControllerFileList
     ${srcViewAndControllerDirectory}/SoXAudioEditor.cpp
     ${srcViewAndControllerDirectory}/SoXAudioEditorWidget.cpp
     ${srcViewAndControllerDirectory}/SoXAudioProcessor.cpp
)

LIST(APPEND allSrcFileList
     ${srcBaseModulesFileList} ${srcCommonAudioFileList}
     ${srcContainersFileList} ${srcEffectsBaseFileList}
     ${srcHelpersFileList} ${srcPrimitivesFileList}
     ${srcViewAndControllerFiles}
)

# -------------------------------------------------------------------
# --- XXXFileListSTD is the file name list for the plain audio    ---
# --- plugin library of some SoX effect                           ---
# -------------------------------------------------------------------

LIST(APPEND complexPluginsList
    Compander Reverb
)

FOREACH(pluginName ${pluginNameList})
    CMAKE_PATH(SET effectDirectory NORMALIZE
               ${srcEffect${pluginName}Directory})
    SET(fileListName srcEffect${pluginName}FileListSTD)

    LIST(APPEND ${fileListName}
         ${effectDirectory}/SoX${pluginName}_AudioEffect.cpp
    )

    IF(pluginName IN_LIST complexPluginsList)
        LIST(APPEND ${fileListName}
             ${effectDirectory}/SoX${pluginName}Support.cpp
        )
    ENDIF()

    SET(allSrcFileList ${allSrcFileList} ${${fileListName}})
ENDFOREACH(pluginName)

# -----------------------------------------------------------------
# --- XXXFileList_AU is the file name list for the AU library   ---
# --- for some plugin                                           ---
# -----------------------------------------------------------------

IF(MACOSX)
    FOREACH(pluginName ${pluginNameList})
        CMAKE_PATH(SET effectDirectory NORMALIZE
                   ${srcEffect${pluginName}Directory})

        SET(fileListName srcEffect${pluginName}FileList_AU)

        LIST(APPEND ${fileListName}
             ${effectDirectory}/SoX${pluginName}_AudioProcessor.cpp
             ${effectDirectory}/SoXPlugin-util.cpp
             ${effectDirectory}/SoXPlugin-AU_1.mm
             ${effectDirectory}/SoXPlugin-Standalone.cpp
        )

        LIST(APPEND allSrcFileList ${${fileListName}})
    ENDFOREACH(pluginName)
ENDIF(MACOSX)

# --------------------------------------------------------------------
# --- XXXFileList_VST is the file name list for the VST3 library   ---
# --- for some SoX effect                                          ---
# --------------------------------------------------------------------

FOREACH(pluginName ${pluginNameList})
    CMAKE_PATH(SET effectDirectory NORMALIZE
               ${srcEffect${pluginName}Directory})

    SET(fileListName srcEffect${pluginName}FileList_VST)

    LIST(APPEND ${fileListName}
         ${effectDirectory}/SoX${pluginName}_AudioProcessor.cpp
         ${effectDirectory}/SoXPlugin-VST_1.${GLOB_juceFileExtension}
    )

    IF(WINDOWS)
        LIST(APPEND ${fileListName}
             ${effectDirectory}/resources.rc
        )
    ENDIF(WINDOWS)

    LIST(APPEND allSrcFileList ${${fileListName}})
ENDFOREACH(pluginName)

# -------------------------------------------------------------------
# --- a simple command line test for some of the engines          ---
# -------------------------------------------------------------------

LIST(APPEND srcEffectsTestFileList
     ${srcEffectsTestDirectory}/SoX-Test_main-std.cpp
)

LIST(APPEND allSrcFileList ${srcEffectsTestFileList})

# --- regression test sources ---
LIST(APPEND regressionTestFileList
     ${testFileDirectory}/makeTestFiles.${GLOB_batchFileExtension}
     "${testFileDirectory}/testSoXPlugins-VST3.rpp"
)

IF(MACOSX)
    LIST(APPEND regressionTestFileList
         "${testFileDirectory}/testSoXPlugins-AU.rpp"
    )
ENDIF(MACOSX)

# #################
# ### FUNCTIONS ###
# #################

FUNCTION(makeSoXViewAndControllerTarget)
    # defines a static library as target <SoXViewAndController> with
    # classes interfacing to JUCE

    SET(targetName SoXViewAndController)

    ADD_LIBRARY(${targetName} STATIC EXCLUDE_FROM_ALL
                ${srcViewAndControllerFileList})

    addCompilerFlags(${targetName} TRUE)

    TARGET_INCLUDE_DIRECTORIES(${targetName} PUBLIC
                               ${srcJuceFacadeDirectory}
                               ${additionalIncludeDirectoryList})

    SET_TARGET_PROPERTIES(${targetName} PROPERTIES
                          POSITION_INDEPENDENT_CODE TRUE
                          INTERFACE_POSITION_INDEPENDENT_CODE TRUE)
ENDFUNCTION(makeSoXViewAndControllerTarget)

#--------------------

FUNCTION(shortNameForEffect pluginName)
    # returns Apple four character AU subtype name for given
    # <pluginName>

    IF(${pluginName} STREQUAL "Compander")
        SET(result mcpd)
    ELSEIF(${pluginName} STREQUAL "Filter")
        SET(result filt)
    ELSEIF(${pluginName} STREQUAL "Gain")
        SET(result gain)
    ELSEIF(${pluginName} STREQUAL "Overdrive")
        SET(result ovdr)
    ELSEIF(${pluginName} STREQUAL "PhaserAndTremolo")
        SET(result phtr)
    ELSE()
        SET(result rvrb)
    ENDIF()

    SET(shortName ${result} PARENT_SCOPE)
ENDFUNCTION(shortNameForEffect)

###########
# TARGETS #
###########

INCLUDE_DIRECTORIES(${standardIncludeDirectoryList})

# ==================================
# === intermediate library files ===
# ==================================

# --- common services library (primitives containers and common audio) ---
LIST(APPEND sourceFileList
     ${srcBaseModulesFileList} ${srcPrimitivesFileList}
     ${srcContainersFileList} ${srcCommonAudioFileList}
     ${srcEffectsBaseFileList} ${srcHelpersFileList}
)

GLOB_makeTarget_commonProjectLibrary("${sourceFileList}")

# --- juce library with classes from JUCE ---
CMAKE_PATH(SET jucePrefix NORMALIZE
           ${srcJuceFacadeDirectory}/include_juce)

GLOB_makeTarget_juceFramework(${jucePrefix})

# --- view and controller library ---
makeSoXViewAndControllerTarget()

ADD_CUSTOM_TARGET(SupportLibraries ALL)
ADD_DEPENDENCIES(SupportLibraries
                 JuceFramework CommonProjectLibrary SoXViewAndController)

# ==============================
# === effective target files ===
# ==============================

ADD_CUSTOM_TARGET(SoXPlugins ALL)

# ---------------------------------------------------------
# --- define a target SoX<Effect> combining the plugins ---
# --- for the platform for each effect                  ---
# ---------------------------------------------------------

FOREACH(pluginName ${pluginNameList})
    SET(targetName SoX${pluginName})
    ADD_CUSTOM_TARGET(${targetName} ALL)
    ADD_DEPENDENCIES(SoXPlugins ${targetName})
ENDFOREACH(pluginName)

# ------------------------------------------------------
# - build a static library for each effect without GUI -
# ------------------------------------------------------

ADD_CUSTOM_TARGET(SoXPlugins_Effect ALL)

FOREACH(pluginName ${pluginNameList})
    SET(targetName SoX${pluginName}_Effect)
    SET(pluginDirectory ${srcEffect${pluginName}Directory})
    LIST(APPEND fileList
         ${srcEffect${pluginName}FileListSTD}
         ${GLOB_debuggerVisualizationFileName}
    )
    
    GLOB_makeTarget_pluginStaticLib(${targetName}
                                    ${pluginDirectory}
                                    "${fileList}")
    ADD_DEPENDENCIES(SoXPlugins_Effect ${targetName})
ENDFOREACH(pluginName)

# ---------------------------------------------------------
# --- build a simple program for optimization tests     ---
# ---------------------------------------------------------

SET(targetName ${testProgramName})

LIST(APPEND testProgramFileList
     ${commonSrcFileList}
     ${srcEffectsDirectory}/SoX-Test/SoX-Test_main-std.cpp
     ${GLOB_debuggerVisualizationFileName}
)

ADD_EXECUTABLE(${targetName} ${testProgramFileList})

addCompilerFlags(${targetName} TRUE)

TARGET_INCLUDE_DIRECTORIES(${targetName} PUBLIC
                           ${srcEffectsDirectory}/SoXCompander
                           ${srcEffectsDirectory}/SoXFilter
                           ${srcEffectsDirectory}/SoXPhaserAndTremolo
                           ${srcEffectsDirectory}/SoXReverb)

TARGET_LINK_LIBRARIES(${targetName}
                      SoXCompander_Effect
                      SoXFilter_Effect
                      SoXPhaserAndTremolo_Effect
                      SoXReverb_Effect
                      CommonProjectLibrary)

# -----------------------------------------------------------------
# --- build a dynamic library for each effect with Juce GUI and ---
# --- VST client plus the static library of that SoX effect     ---
# -----------------------------------------------------------------

ADD_CUSTOM_TARGET(SoXPlugins_VST ALL)

FOREACH(pluginName ${pluginNameList})
    SET(libraryName SoX${pluginName})
    SET(targetName ${libraryName}_VST)
    SET(sourceFileList ${srcEffect${pluginName}FileList_VST})
    SET(libraryList
        SoX${pluginName}_Effect SoXViewAndController
        CommonProjectLibrary JuceFramework
    )

    GLOB_makeTarget_pluginVST(
        ${targetName} ${pluginName} ${libraryName}
        "${sourceFileList}"
        "${additionalIncludeDirectoryList}"
        "${libraryList}"
        "${unixLibraryList}"
    )

    ADD_DEPENDENCIES(SoXPlugins_VST ${targetName})
    ADD_DEPENDENCIES(SoX${pluginName} ${targetName})
ENDFOREACH(pluginName)

# -----------------------------------------------------------------
# --- build an audio unit app for each effect with Juce GUI and ---
# --- AUv3 client plus the static library of that SoX effect     ---
# -----------------------------------------------------------------

IF(MACOSX)
    ADD_CUSTOM_TARGET(SoXPlugins_AU ALL)

    FOREACH(pluginName ${pluginNameList})
        SET(libraryName SoX${pluginName})
        SET(targetName ${libraryName}_AU)
        shortNameForEffect(${pluginName})
        SET(fullPluginName SoX${pluginName})
        SET(libraryList
            ${fullPluginName}_Effect SoXViewAndController
            CommonProjectLibrary JuceFramework
        )

        GLOB_makeTarget_pluginAU(${targetName} ${pluginName} ${libraryName}
                                 ${fullPluginName} "DrTT: SoX ${pluginName}"
                                 "aufx" ${shortName}
                                 "${srcEffect${pluginName}FileList_AU}"
                                 "${additionalIncludeDirectoryList}"
                                 "${libraryList}"
        )

        ADD_DEPENDENCIES(SoXPlugins_AU ${targetName})
        ADD_DEPENDENCIES(SoX${pluginName} ${targetName})
    ENDFOREACH(pluginName)
ENDIF()

# ---------------------------
# --- documentation ---
# ---------------------------

GLOB_makeTarget_documentation("documentation" "${allSrcFileList}")

# ====================
# === INSTALLATION ===
# ====================

IF(MSVC)
    # make installation part of build in MSVC
    SET(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)
ENDIF()

# --- install the documentation ---
GLOB_install_documentationPDF()

# --- install the regression test files ---
GLOB_install_testFiles("${regressionTestFileList}")

# --- install the VST3 and AU plugins to platform directory ---
FOREACH(pluginName ${pluginNameList})
    SET(libraryName SoX${pluginName})
    GLOB_install_pluginVST(${libraryName} "")
    GLOB_install_pluginAU(${libraryName} "")
ENDFOREACH(pluginName)
