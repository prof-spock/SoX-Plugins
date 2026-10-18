#########################################
# CMAKE file for the SoXPlugins project #
#                                       #
# Dr. Thomas Tensi, 2022-09             #
#########################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(SoXPlugins
        VERSION 1.0
        LANGUAGES C CXX)

# =============================
# === program configuration ===
# =============================

INCLUDE(_UtilityFunctions.cmake)
INCLUDE(_GlobalConfiguration.cmake)
INCLUDE(_GlobalConfiguration_Plugin.cmake)

# --- compiler ---
INCLUDE(_PluginSpecificCompilerConfiguration.cmake)
# first JUCE plugin settings, then global compiler settings
INCLUDE(_CPPCompilerConfiguration_Plugin.cmake)
INCLUDE(_CPPCompilerConfiguration.cmake)

# --- LaTeX ---
FIND_PACKAGE(LATEX QUIET OPTIONAL_COMPONENTS PDFLATEX)

# --- DoxyGen ---
FIND_PACKAGE(Doxygen QUIET)

# --- JUCE ---
FIND_PACKAGE(juce QUIET)

# local configuration settings
INCLUDE(_LocalConfiguration.cmake)

# define whether programs are strictly compatible to SoX_ng
SET(isStrictlyCompatibleToSoX TRUE)

# ===========================

# list of all plugin names available as plugins
SET(pluginNameList
    ChorusAndEcho Compander Filter FlangerPhaserAndTremolo Gain
    Overdrive Reverb)

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
CMAKE_PATH(SET srcStdCFacadeDirectory NORMALIZE
           ${srcDirectory}/Libraries/StandardCFacade)
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

SET(standardIncludeDirectoryList
    ${srcAudioSupportDirectory}
    ${srcBaseModulesDirectory}
    ${srcContainersDirectory}
    ${srcGenericTypesDirectory}
    ${srcPrimitivesDirectory}
    ${srcEffectsDirectory}
    ${srcHelpersDirectory}
    ${srcStdCFacadeDirectory}
    ${srcViewAndControllerDirectory}
)

SET(additionalIncludeDirectoryList
    ${srcJuceFacadeDirectory}
)

GLOBP_appendJuceIncludeDirectories(additionalIncludeDirectoryList
                                   "${JUCE_DIR}")

# --- add include directories and libraries for Unix ---
IF(LINUX)
    FIND_PACKAGE(PkgConfig REQUIRED) 
    SET(relevantPackageList freetype2)
    PKG_CHECK_MODULES(linuxPackages ${relevantPackageList})
    SET(unixIncludeDirectoryList /usr/include/freetype2)
    SET(unixLibraryList ${linuxPackages_LINK_LIBRARIES})

    LIST(APPEND additionalIncludeDirectoryList
         ${unixIncludeDirectoryList}
    )
ENDIF(LINUX)

# --- Windows specific files ---
IF(MSVC)
    CMAKE_PATH(SET debuggerVisualizationFileDirectory
               ${CMAKE_CURRENT_SOURCE_DIR}/Windows)
    SET(debuggerVisualizationFileNameList
        ${debuggerVisualizationFileDirectory}/audio.natvis
        ${debuggerVisualizationFileDirectory}/basemodules.natvis
        ${debuggerVisualizationFileDirectory}/basetypes.natvis)
ELSE()
    SET(debuggerVisualizationFileNameList )
ENDIF()

# ====================
# === source files ===
# ====================

SET(srcCommonAudioFileList
    ${srcAudioSupportDirectory}/AudioDataPoint.cpp
    ${srcAudioSupportDirectory}/AudioDataPointList.cpp
    ${srcAudioSupportDirectory}/AudioDataPointListVector.cpp
    ${srcAudioSupportDirectory}/AudioDataPointRingBuffer.cpp
    ${srcAudioSupportDirectory}/AudioDataPointRingBufferVector.cpp
    ${srcAudioSupportDirectory}/DelayLine.cpp
    ${srcAudioSupportDirectory}/DelayLineList.cpp
    ${srcAudioSupportDirectory}/IIRFilter.cpp
    ${srcAudioSupportDirectory}/WaveForm.cpp
)

SET(srcBaseModulesFileList
    ${srcBaseModulesDirectory}/Configuration.cpp
    ${srcBaseModulesDirectory}/File.cpp
    ${srcBaseModulesDirectory}/LoggingSupport.cpp
    ${srcBaseModulesDirectory}/OperatingSystem.cpp
    ${srcBaseModulesDirectory}/StringUtil.cpp
)

SET(srcContainersFileList
    ${srcContainersDirectory}/ByteList.cpp
    ${srcContainersDirectory}/Dictionary.cpp
    ${srcContainersDirectory}/IntegerList.cpp
    ${srcContainersDirectory}/NaturalList.cpp
    ${srcContainersDirectory}/RealList.cpp
    ${srcContainersDirectory}/StringList.cpp
    ${srcContainersDirectory}/StringSet.cpp
)

SET(srcEffectsBaseFileList
    ${srcEffectsDirectory}/SoXAudioEffect.cpp
)

SET(srcGenericTypesFileList
    ${srcGenericTypesDirectory}/GenericListInterpolator.cpp
)

SET(srcHelpersFileList
    ${srcHelpersDirectory}/SoXEffectParameterMap.cpp
    ${srcHelpersDirectory}/SoXValueChangeObserver.cpp
)

SET(srcPrimitivesFileList
    ${srcPrimitivesDirectory}/MyString.cpp
    ${srcPrimitivesDirectory}/Real.cpp
    ${srcPrimitivesDirectory}/TimeAndDuration.cpp
)

SET(srcViewAndControllerFileList
    ${srcViewAndControllerDirectory}/SoXAudioEditor.cpp
    ${srcViewAndControllerDirectory}/SoXAudioEditorWidget.cpp
    ${srcViewAndControllerDirectory}/SoXAudioProcessor.cpp
)

SET(allSrcFileList
    ${srcBaseModulesFileList}   ${srcCommonAudioFileList}
    ${srcContainersFileList}    ${srcEffectsBaseFileList}
    ${srcGenericTypesFileList}  ${srcHelpersFileList}
    ${srcPrimitivesFileList}    ${srcViewAndControllerFiles}
)

# -------------------------------------------------------------------
# --- XXXFileListSTD is the file name list for the plain audio    ---
# --- plugin library of some SoX effect                           ---
# -------------------------------------------------------------------

SET(complexPluginsList Compander Reverb)

FOREACH(pluginName ${pluginNameList})
    CMAKE_PATH(SET effectDirectory NORMALIZE
               ${srcEffect${pluginName}Directory})
    SET(fileListName srcEffect${pluginName}FileListSTD)

    SET(${fileListName}
        ${effectDirectory}/SoX${pluginName}_AudioEffect.cpp)

    IF(pluginName IN_LIST complexPluginsList)
        LIST(APPEND ${fileListName}
             ${effectDirectory}/SoX${pluginName}Support.cpp
        )
    ENDIF()

    LIST(APPEND allSrcFileList ${${fileListName}})
ENDFOREACH(pluginName)

# -----------------------------------------------------------------
# --- XXXFileList_AU is the file name list for the AU library   ---
# --- for some plugin                                           ---
# -----------------------------------------------------------------

IF(MACOS)
    FOREACH(pluginName ${pluginNameList})
        CMAKE_PATH(SET effectDirectory NORMALIZE
                   ${srcEffect${pluginName}Directory})

        SET(fileListName srcEffect${pluginName}FileList_AU)

        LIST(APPEND ${fileListName}
             ${effectDirectory}/SoX${pluginName}_AudioProcessor.cpp
             ${srcJuceFacadeDirectory}/JucePlugin-util.cpp
             ${srcJuceFacadeDirectory}/JucePlugin-AU_1.mm
             ${srcJuceFacadeDirectory}/JucePlugin-Standalone.cpp
        )

        LIST(APPEND allSrcFileList ${${fileListName}})
    ENDFOREACH(pluginName)
ENDIF(MACOS)

# --------------------------------------------------------------------
# --- XXXFileList_VST is the file name list for the VST3 library   ---
# --- for some SoX effect                                          ---
# --------------------------------------------------------------------

FOREACH(pluginName ${pluginNameList})
    CMAKE_PATH(SET effectDirectory NORMALIZE
               ${srcEffect${pluginName}Directory})

    SET(fileListName srcEffect${pluginName}FileList_VST)

    SET(${fileListName}
        ${effectDirectory}/SoX${pluginName}_AudioProcessor.cpp
        ${srcJuceFacadeDirectory}/JucePlugin-VST_1.${GLOBP_juceFileExtension}
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

SET(srcEffectsTestFileList
    ${srcEffectsTestDirectory}/SoX-Test_main-std.cpp
)

LIST(APPEND allSrcFileList ${srcEffectsTestFileList})

# --- regression test sources ---
SET(regressionTestFileList
    ${testFileDirectory}/makeTestFiles.${GLOB_batchFileExtension}
    "${testFileDirectory}/testSoXPlugins-VST3.rpp"
)

IF(MACOS)
    LIST(APPEND regressionTestFileList
         "${testFileDirectory}/testSoXPlugins-AU.rpp"
    )
ENDIF(MACOS)

# #################
# ### FUNCTIONS ###
# #################

FUNCTION(makeSoXViewAndControllerTarget)
    # defines a static library as target <SoXViewAndController> with
    # classes interfacing to JUCE

    SET(targetName SoXViewAndController)

    ADD_LIBRARY(${targetName} STATIC EXCLUDE_FROM_ALL
                ${srcViewAndControllerFileList})

    CMPCONF_addCompilerFlags(${targetName} TRUE)

    TARGET_INCLUDE_DIRECTORIES(${targetName} PUBLIC
                               ${additionalIncludeDirectoryList})

    SET_TARGET_PROPERTIES(${targetName} PROPERTIES
                          POSITION_INDEPENDENT_CODE TRUE
                          INTERFACE_POSITION_INDEPENDENT_CODE TRUE)
ENDFUNCTION(makeSoXViewAndControllerTarget)

#--------------------

FUNCTION(shortNameForEffect pluginName)
    # returns Apple four character AU subtype name for given
    # <pluginName>

    IF(${pluginName} STREQUAL "ChorusAndEcho")
        SET(result "cech")
    ELSEIF(${pluginName} STREQUAL "Compander")
        SET(result "mcpd")
    ELSEIF(${pluginName} STREQUAL "Filter")
        SET(result "filt")
    ELSEIF(${pluginName} STREQUAL "FlangerPhaserAndTremolo")
        SET(result "fptr")
    ELSEIF(${pluginName} STREQUAL "Gain")
        SET(result "gain")
    ELSEIF(${pluginName} STREQUAL "Overdrive")
        SET(result "ovdr")
    ELSE()
        SET(result "rvrb")
    ENDIF()

    SET(shortName ${result} PARENT_SCOPE)
ENDFUNCTION(shortNameForEffect)

###########
# TARGETS #
###########

MESSAGE(STATUS "standardIncludeDirectories " ${standardIncludeDirectoryList})
INCLUDE_DIRECTORIES(${standardIncludeDirectoryList})

# ==================================
# === intermediate library files ===
# ==================================

# --- common services library (primitives containers and common audio) ---
SET(sourceFileList
    ${srcBaseModulesFileList}   ${srcContainersFileList}
    ${srcPrimitivesFileList}    ${srcCommonAudioFileList}
    ${srcEffectsBaseFileList}   ${srcGenericTypesFileList}
    ${srcHelpersFileList}
)

GLOB_makeTarget_commonProjectLibrary("${sourceFileList}")

# --- juce library with classes from JUCE ---
CMAKE_PATH(SET jucePrefix NORMALIZE
           ${srcJuceFacadeDirectory}/include_juce)

GLOBP_makeTarget_juceFramework(${jucePrefix})

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
    SET(fileList
        ${srcEffect${pluginName}FileListSTD}
        ${debuggerVisualizationFileNameList}
    )
    
    GLOBP_makeTarget_pluginStaticLib(${targetName}
                                     ${pluginDirectory}
                                     "${fileList}")
    ADD_DEPENDENCIES(SoXPlugins_Effect ${targetName})
ENDFOREACH(pluginName)

# ---------------------------------------------------------
# --- build a simple program for optimization tests     ---
# ---------------------------------------------------------

SET(targetName ${testProgramName})

SET(testProgramFileList
    ${commonSrcFileList}
    ${srcEffectsDirectory}/SoX-Test/SoX-Test_main-std.cpp
    ${debuggerVisualizationFileNameList}
)

ADD_EXECUTABLE(${targetName} ${testProgramFileList})

CMPCONF_addCompilerFlags(${targetName} TRUE)

TARGET_INCLUDE_DIRECTORIES(${targetName} PUBLIC
                           ${srcEffectsDirectory}/SoXChorusAndEcho
                           ${srcEffectsDirectory}/SoXCompander
                           ${srcEffectsDirectory}/SoXFilter
                           ${srcEffectsDirectory}/SoXFlangerPhaserAndTremolo
                           ${srcEffectsDirectory}/SoXGain
                           ${srcEffectsDirectory}/SoXOverdrive
                           ${srcEffectsDirectory}/SoXReverb)

TARGET_LINK_LIBRARIES(${targetName}
                      SoXChorusAndEcho_Effect
                      SoXCompander_Effect
                      SoXFilter_Effect
                      SoXFlangerPhaserAndTremolo_Effect
                      SoXGain_Effect
                      SoXOverdrive_Effect
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
    SET(effectTargetName SoX${pluginName}_Effect)
    SET(libraryList
        ${effectTargetName} SoXViewAndController
        CommonProjectLibrary JuceFramework
    )

    GLOBP_makeTarget_pluginVST(
        ${targetName} ${pluginName} ${libraryName}
        "${sourceFileList}"
        "${additionalIncludeDirectoryList}"
        "${libraryList}"
        "${unixLibraryList}"
    )

    IF(${isStrictlyCompatibleToSoX})
        TARGET_COMPILE_DEFINITIONS(${effectTargetName} PUBLIC
                                   "EMULATE_SOX_STRICTLY")
    ENDIF()

    ADD_DEPENDENCIES(SoXPlugins_VST ${targetName})
    ADD_DEPENDENCIES(SoX${pluginName} ${targetName})
ENDFOREACH(pluginName)

# -----------------------------------------------------------------
# --- build an audio unit app for each effect with Juce GUI and ---
# --- AUv3 client plus the static library of that SoX effect     ---
# -----------------------------------------------------------------

IF(MACOS)
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

        GLOBP_makeTarget_pluginAU(${targetName} ${pluginName} ${libraryName}
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

IF(CMPCONF_compilerIsMSVC)
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
    GLOBP_install_pluginVST(${libraryName} "")
    GLOBP_install_pluginAU(${libraryName} "")
ENDFOREACH(pluginName)
