#########################################
# CMAKE file for the SoXPlugins project #
#                                       #
# Dr. Thomas Tensi, 2022-09             #
#########################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(SoXPlugins
        VERSION 1.0
        LANGUAGES CXX)

# =============================
# === program configuration ===
# =============================

SET(WINDOWS ${WIN32})
SET(MACOSX  ${APPLE})

IF(UNIX AND NOT APPLE)
    SET(LINUX 1)
ELSE()
    SET(LINUX 0)
ENDIF()

SET(platformName ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})

# --- compiler ---
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
INCLUDE(CPPCompilerConfiguration.cmake)

# --- LaTeX ---
FIND_PACKAGE(LATEX QUIET OPTIONAL_COMPONENTS PDFLATEX)

# --- DoxyGen ---
FIND_PACKAGE(Doxygen QUIET)

# --- JUCE ---
FIND_PACKAGE(juce QUIET)

# ===========================
# === local configuration ===
# ===========================

# local configuration settings
INCLUDE(LocalConfiguration.cmake)

# list of all effect names available as plugins
SET(effectNameList
    Compander Filter Gain Overdrive PhaserAndTremolo Reverb)

# a command line test program for optimization
SET(testProgramName "ZZZ_Test-SoXPlugins")

# the subdirectory for the configuration used
SET(configurationSubdirectory $<$<PLATFORM_ID:Windows>:/$<CONFIG>>)

# ===================
# === directories ===
# ===================

SET(buildDirectory ${CMAKE_CURRENT_BINARY_DIR})
SET(targetDirectory ${CMAKE_CURRENT_SOURCE_DIR}/../_DISTRIBUTION)
SET(platformTargetDirectory ${targetDirectory}/targetPlatforms)

# --- source directories ---
SET(srcDirectory ${CMAKE_CURRENT_SOURCE_DIR}/../src)
SET(srcAudioDirectory              ${srcDirectory}/Audio)
SET(srcBaseModulesDirectory        ${srcDirectory}/BaseModules)
SET(srcBaseTypesDirectory          ${srcDirectory}/BaseTypes)
SET(srcHelpersDirectory            ${srcDirectory}/Helpers)
SET(srcContainersDirectory         ${srcBaseTypesDirectory}/Containers)
SET(srcGenericTypesDirectory       ${srcBaseTypesDirectory}/GenericTypes)
SET(srcPrimitivesDirectory         ${srcBaseTypesDirectory}/Primitives)
SET(srcJuceFacadeDirectory         ${srcDirectory}/Libraries/JuceFacade)
SET(srcViewAndControllerDirectory  ${srcDirectory}/ViewAndController)
SET(srcEffectsDirectory            ${srcDirectory}/Effects)
SET(srcEffectsTestDirectory        ${srcEffectsDirectory}/SoX-Test)

FOREACH(effectName ${effectNameList})
    SET(srcEffect${effectName}Directory
        ${srcEffectsDirectory}/SoX${effectName})
ENDFOREACH(effectName)

SET(docDirectory ${CMAKE_CURRENT_SOURCE_DIR}/../doc)
SET(docLaTeXDirectory ${docDirectory}/latex)
SET(doxygenTargetDirectory
    ${CMAKE_CURRENT_SOURCE_DIR}/../internalDocumentation)
SET(testFileDirectory ${CMAKE_CURRENT_SOURCE_DIR}/../test)

# --- include directories ---

SET(juceModulesDirectory ${JUCE_DIR}/modules)

SET(standardIncludeDirectoryList
    ${srcAudioDirectory}
    ${srcBaseModulesDirectory}
    ${srcContainersDirectory}
    ${srcGenericTypesDirectory}
    ${srcPrimitivesDirectory}
    ${srcEffectsDirectory}
    ${srcHelpersDirectory}
    ${srcViewAndControllerDirectory})

SET(additionalIncludeDirectoryList
    ${srcJuceFacadeDirectory}
    ${juceModulesDirectory}
    ${juceModulesDirectory}/juce_audio_processors/format_types/VST3_SDK)

IF(MACOSX)
    SET(additionalIncludeDirectoryList
        ${additionalIncludeDirectoryList}
        /Developer/Headers/FlatCarbon)
ENDIF(MACOSX)

# --- additional JUCE settings ---

SET(jucePrefix ${srcJuceFacadeDirectory}/include_juce)

IF(MACOSX)
    SET(juceFileExtension mm)
ELSE()
    SET(juceFileExtension cpp)
ENDIF()

# --- add include directories and libraries for Unix ---
IF(LINUX)
    FIND_PACKAGE(PkgConfig REQUIRED) 
    SET(relevantPackageList freetype2)
    PKG_CHECK_MODULES(linuxPackages ${relevantPackageList})
    SET(unixIncludeDirectoryList /usr/include/freetype2)
    SET(unixLibraryList ${linuxPackages_LINK_LIBRARIES})

    SET(additionalIncludeDirectoryList
        ${additionalIncludeDirectoryList}
        ${unixIncludeDirectoryList})
ENDIF(LINUX)

# ====================
# === source files ===
# ====================

SET(srcBaseModulesFileList
    ${srcBaseModulesDirectory}/File.cpp
    ${srcBaseModulesDirectory}/LoggingSupport.cpp
    ${srcBaseModulesDirectory}/OperatingSystem.cpp
    ${srcBaseModulesDirectory}/StringUtil.cpp)

SET(srcCommonAudioFileList
    ${srcAudioDirectory}/AudioSampleList.cpp
    ${srcAudioDirectory}/AudioSampleListVector.cpp
    ${srcAudioDirectory}/AudioSampleRingBuffer.cpp
    ${srcAudioDirectory}/AudioSampleRingBufferVector.cpp
    ${srcAudioDirectory}/IIRFilter.cpp
    ${srcAudioDirectory}/WaveForm.cpp)

SET(srcContainersFileList
    ${srcContainersDirectory}/ByteList.cpp
    ${srcContainersDirectory}/Dictionary.cpp
    ${srcContainersDirectory}/IntegerList.cpp
    ${srcContainersDirectory}/NaturalList.cpp
    ${srcContainersDirectory}/RealList.cpp
    ${srcContainersDirectory}/StringList.cpp)

SET(srcEffectsBaseFileList
    ${srcEffectsDirectory}/SoXAudioEffect.cpp)

SET(srcHelpersFileList
    ${srcHelpersDirectory}/SoXEffectParameterMap.cpp)

SET(srcPrimitivesFileList ${srcPrimitivesDirectory}/MyString.cpp)

SET(srcViewAndControllerFileList
    ${srcViewAndControllerDirectory}/SoXAudioEditor.cpp
    ${srcViewAndControllerDirectory}/SoXAudioEditorWidget.cpp
    ${srcViewAndControllerDirectory}/SoXAudioProcessor.cpp)

SET(allSrcFileList
    ${srcBaseModulesFileList} ${srcCommonAudioFileList}
    ${srcContainersFileList} ${srcEffectsBaseFileList}
    ${srcHelpersFileList} ${srcPrimitivesFileList}
    ${srcViewAndControllerFiles})

# -------------------------------------------------------------------
# --- XXXFileListSTD is the file name list for the plain audio    ---
# --- effect library of some SoX effect                           ---
# -------------------------------------------------------------------

FOREACH(effectName Filter Gain Overdrive PhaserAndTremolo)
    SET(fileListName srcEffect${effectName}FileListSTD)

    SET(${fileListName}
        ${srcEffect${effectName}Directory}/SoX${effectName}_AudioEffect.cpp)

    SET(allSrcFileList ${allSrcFileList} ${${fileListName}})
ENDFOREACH(effectName)

FOREACH(effectName Compander Reverb)
    SET(effectDirectory ${srcEffect${effectName}Directory})

    SET(fileListName srcEffect${effectName}FileListSTD)

    SET(${fileListName}
        ${effectDirectory}/SoX${effectName}_AudioEffect.cpp
        ${effectDirectory}/SoX${effectName}Support.cpp)

    SET(allSrcFileList ${allSrcFileList} ${${fileListName}})
ENDFOREACH(effectName)

# -----------------------------------------------------------------
# --- XXXFileList_AU is the file name list for the AU library   ---
# --- for some SoX effect                                       ---
# -----------------------------------------------------------------

IF(MACOSX)
    FOREACH(effectName ${effectNameList})
        SET(effectDirectory ${srcEffect${effectName}Directory})

        SET(fileListName srcEffect${effectName}FileList_AU)

        SET(${fileListName}
            ${effectDirectory}/SoX${effectName}_AudioProcessor.cpp
            ${effectDirectory}/SoXPlugin-util.cpp
            ${effectDirectory}/SoXPlugin-AU_1.mm
            ${effectDirectory}/SoXPlugin-AU_2.mm
            ${srcJuceFacadeDirectory}/include_juce_audio_plugin_client_AU.r
            ${effectDirectory}/SoXPlugin-Standalone.cpp)

        SET(allSrcFileList ${allSrcFileList} ${${fileListName}})
    ENDFOREACH(effectName)
ENDIF(MACOSX)

# #--------------------------------------------------------------------
# #--- XXXFileList_LV2 is the file name list for the LV2 library    ---
# #--- for some SoX effect                                          ---
# #--------------------------------------------------------------------
#
# SET(localEffectNameList Gain)
#
# FOREACH(effectName ${localEffectNameList})
#     SET(effectDirectory ${srcEffect${effectName}Directory})
#
#     SET(fileListName srcEffect${effectName}FileList_LV2)
#
#     SET(${fileListName}
#         ${effectDirectory}/SoX${effectName}_AudioProcessor.cpp
#         ${effectDirectory}/SoXPlugin-LV2.cpp)
#
#     IF(MACOSX)
#         SET(${fileListName}
#             ${${fileListName}}
#             ${effectDirectory}/SoXPlugin-VST_2.${juceFileExtension})
#     ENDIF(MACOSX)
#
#     SET(allSrcFileList ${allSrcFileList} ${${fileListName}})
# ENDFOREACH(effectName)

# --------------------------------------------------------------------
# --- XXXFileList_VST is the file name list for the VST3 library   ---
# --- for some SoX effect                                          ---
# --------------------------------------------------------------------

FOREACH(effectName ${effectNameList})
    SET(effectDirectory ${srcEffect${effectName}Directory})

    SET(fileListName srcEffect${effectName}FileList_VST)

    SET(${fileListName}
        ${effectDirectory}/SoX${effectName}_AudioProcessor.cpp
        ${effectDirectory}/SoXPlugin-VST_1.cpp)

    IF(WINDOWS)
        SET(${fileListName}
            ${${fileListName}}
            ${effectDirectory}/resources.rc)
    ENDIF(WINDOWS)

    IF(MACOSX)
        SET(${fileListName}
            ${${fileListName}}
            ${effectDirectory}/SoXPlugin-VST_2.${juceFileExtension})
    ENDIF(MACOSX)

    SET(allSrcFileList ${allSrcFileList} ${${fileListName}})
ENDFOREACH(effectName)

# -------------------------------------------------------------------
# --- a simple command line test for some of the engines          ---
# -------------------------------------------------------------------

SET(srcEffectsTestFileList
    ${srcEffectsTestDirectory}/SoX-TEST_main-std.cpp)

SET(allSrcFileList ${allSrcFileList} ${srcEffectsTestFileList})

# -------------------------------------------------------------------
# --- the file name list of facade files for JUCE; those files    ---
# --- reference real implementations in JUCE; note that on        ---
# --- Apple platforms the Objective-C++ (.mm) files are used      ---
# --- of the standard C++ (.cpp) files                            ---
# -------------------------------------------------------------------

SET(srcJuceFacadeFileListSTD
    ${jucePrefix}_audio_basics.${juceFileExtension}
    ${jucePrefix}_audio_devices.${juceFileExtension}
    ${jucePrefix}_audio_formats.${juceFileExtension}
    ${jucePrefix}_audio_processors.${juceFileExtension}
    ${jucePrefix}_audio_utils.${juceFileExtension}
    ${jucePrefix}_core.${juceFileExtension}
    ${jucePrefix}_data_structures.${juceFileExtension}
    ${jucePrefix}_events.${juceFileExtension}
    ${jucePrefix}_graphics.${juceFileExtension}
    ${jucePrefix}_gui_basics.${juceFileExtension}
    ${jucePrefix}_gui_extra.${juceFileExtension} )

# --- documentation sources ---
SET(docLaTeXFileNameStem SoXPlugins-documentation)
SET(docLaTeXFileName ${docLaTeXDirectory}/${docLaTeXFileNameStem}.ltx)
SET(docLaTeXLocalFileName ${buildDirectory}/${docLaTeXFileNameStem}.ltx)
SET(docDoxygenFileTemplate
    ${docDirectory}/doxygen/SoXPlugins-doxygen-FULL.cfg.in)
SET(docDoxygenFileName SoXPlugins-doxygen-FULL.cfg)

# --- regression test sources ---
SET(regressionTestFileList
    ${testFileDirectory}/makeTestFiles.bat
    ${testFileDirectory}/makeTestFiles.sh
    "${testFileDirectory}/testSoxPlugins-C++.RPP")

# --- Windows specific files ---
IF(MSVC)
    SET(debuggerVisualizationFileName
        ${CMAKE_CURRENT_SOURCE_DIR}/Windows/datastructures.natvis)
ELSE()
    SET(debuggerVisualizationFileName )
ENDIF()

# --- MacOSX specific files ---
SET(miscDirectory ${CMAKE_CURRENT_SOURCE_DIR}/MacOSX)
SET(plistFileTemplate ${miscDirectory}/Info.plist.in)
SET(nibFileName RecentFilesMenuTemplate.nib)
SET(nibFileSourcePath ${miscDirectory}/${nibFileName})

# -------------------------------------------------
# --- Apple framework files and bundle settings ---
# -------------------------------------------------

IF(NOT MACOSX)
    SET(frameworkNameList '')
ELSE()
    # included frameworks
    SET(frameworkNameList
        Accelerate AudioToolbox AudioUnit Carbon
        Cocoa CoreAudio CoreAudioKit CoreMIDI
        DiscRecording Foundation IOKit QuartzCore WebKit)

    # bundle settings
    SET(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    SET(bundlePrefix "eu.tensi.thomas.SoXPlugins")

    # directories and includes for resource compiler Rez
    SET(appDevelopDirectory
        "/Applications/Xcode.app/Contents/Developer")
    SET(appSdkDirectory
        ${appDevelopDirectory}/Platforms/MacOSX.platform/Developer/SDKs)

    # make definition string from elements in
    # manufacturerAndSuiteDefineClauseList
    SET(rezDefines )
    FOREACH(defineClause ${manufacturerAndSuiteDefineClauseList})
        STRING(REPLACE "§" "\\\"" definitionFlag ${defineClause})
        SET(rezDefines ${rezDefines} "-d" ${definitionFlag})
    ENDFOREACH(defineClause)

    SET(rezFlags -useDF -script Roman)
    SET(rezIncludeDirectoryList
        -i "/System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
        -i ${appDevelopDirectory}/CoreAudio/AudioUnits/AUPublic/AUBase
        -i ${appSdkDirectory}/MacOSX.sdk/System/Library/Frameworks/AudioUnit.framework/Headers
        -i ${juceModulesDirectory}/juce_audio_processors/format_types/VST3_SDK
        -i ${juceModulesDirectory}
        -i ${srcJuceFacadeDirectory}
        -i ${juceModulesDirectory}/juce_audio_plugin_client
        -isysroot ${appSdkDirectory}/MacOSX10.15.sdk)
ENDIF(NOT MACOSX)

# #################
# ### FUNCTIONS ###
# #################

FUNCTION(addEffectNibFileForAUTarget targetName resourcePath)
    # adds a NIB file into resource folder <resourcePath> for target
    # <targetName> representing an audio unit library

    SET(targetNibFileName ${resourcePath}/${nibFileName})
    ADD_CUSTOM_COMMAND(TARGET ${targetName} POST_BUILD
                       COMMAND ibtool
                               --strip ${targetNibFileName}
                               --output-format human-readable-text
                               ${nibFileSourcePath})
ENDFUNCTION(addEffectNibFileForAUTarget)

#--------------------

FUNCTION(addEffectRsrcFileForAUTarget
         targetName libraryName resourcePath)
    # adds a resource rsrc file into resource folder <resourcePath>
    # for target <targetName> representing an audio unit library
    # effect with <libraryName>

    SET(targetRsrcFileName ${resourcePath}/${libraryName}.rsrc)
    SET(resourceFileName
        ${srcJuceFacadeDirectory}/include_juce_audio_plugin_client_AU.r)

    ADD_CUSTOM_COMMAND(TARGET ${targetName} POST_BUILD
                       COMMAND rez ${resourceFileName}
                               ${rezFlags} ${rezDefines}
                               ${rezIncludeDirectoryList}
                               -i ${srcDirectory}/Effects/${libraryName}
                               -o ${targetRsrcFileName})
ENDFUNCTION(addEffectRsrcFileForAUTarget)

#--------------------

FUNCTION(makeEffectAUTarget effectName libraryName targetName)
    # defines an effect AU plugin as target <targetName> with
    # the AU wrapper classes combined with the static library for
    # given effect named <effectName>

    SET(plistFileName ${effectName}_AppleInfo.plist)
    SET(principalClassName "NSApplication")

    ADD_LIBRARY(${targetName}
                MODULE ${srcEffect${effectName}FileList_AU})

    TARGET_INCLUDE_DIRECTORIES(${targetName} PUBLIC
                               ${additionalIncludeDirectoryList})

    TARGET_LINK_LIBRARIES(${targetName}
                          SoX${effectName}_Effect SoXViewAndController
                          SoXCommon JuceFramework)

    # add Apple frameworks to library list
    FOREACH(frameworkName ${frameworkNameList})
      TARGET_LINK_LIBRARIES(${targetName}
                            "-framework ${frameworkName}")
    ENDFOREACH(frameworkName)

    # make the specific PLIST file
    SET(JUCE_manufacturerName "DrTT")
    SET(JUCE_auMainType "aufx")
    shortNameForEffect(${effectName})
    SET(JUCE_auSubType ${shortName})
    CONFIGURE_FILE(${plistFileTemplate} ${plistFileName})

    SET_TARGET_PROPERTIES(${targetName} PROPERTIES
        BUNDLE_EXTENSION component
        BUNDLE TRUE
        XCODE_ATTRIBUTE_WRAPPER_EXTENSION component
        XCODE_ATTRIBUTE_LIBRARY_STYLE Bundle
        XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE YES
        MACOSX_BUNDLE_BUNDLE_NAME ${libraryName}
        MACOSX_BUNDLE_GUI_IDENTIFIER ${bundlePrefix}.${libraryName}
        MACOSX_BUNDLE_INFO_PLIST ${plistFileName}
        OUTPUT_NAME ${libraryName})

    # add resources for audio unit library
    SET(resourcePath ${libraryName}.component/Contents/Resources)
    ADD_CUSTOM_COMMAND(TARGET ${targetName} POST_BUILD
                       COMMAND mkdir -p ${resourcePath})
    addEffectNibFileForAUTarget(${targetName} ${resourcePath})
    addEffectRsrcFileForAUTarget(${targetName} ${libraryName}
                                 ${resourcePath})
ENDFUNCTION(makeEffectAUTarget)

# #--------------------
# 
# FUNCTION(makeEffectLV2Target effectName libraryName targetName)
#     # defines an effect LV2 plugin as target <targetName> with the AU
#     # wrapper classes combined with the static library for given
#     # effect named <effectName>
#
#     ADD_LIBRARY(${targetName} SHARED
#                 MODULE ${srcEffect${effectName}FileList_LV2})
#
#     TARGET_INCLUDE_DIRECTORIES(${targetName} PUBLIC
#                                ${additionalIncludeDirectoryList})
# 
#     TARGET_LINK_LIBRARIES(${targetName}
#                           SoX${effectName}_Effect SoXViewAndController
#                           SoXCommon JuceFramework)
# 
# ENDFUNCTION(makeEffectLV2Target)

#--------------------

FUNCTION(makeEffectStaticLibraryTarget effectName targetName)
    # defines a static effect library as target <targetName> with
    # the engine classes for given effect named <effectName>

    ADD_LIBRARY(${targetName} STATIC
                ${srcEffect${effectName}FileListSTD}
                ${debuggerVisualizationFileName})

    TARGET_INCLUDE_DIRECTORIES(${targetName}
                               PUBLIC ${srcEffect${effectName}Directory})

    SET_TARGET_PROPERTIES(${targetName} PROPERTIES
                          POSITION_INDEPENDENT_CODE TRUE
                          INTERFACE_POSITION_INDEPENDENT_CODE TRUE)
ENDFUNCTION(makeEffectStaticLibraryTarget)

#--------------------

FUNCTION(makeEffectVSTTarget effectName libraryName targetName)
    # defines an effect VST plugin as target <targetName> with
    # the VST wrapper classes combined with the static library for
    # given effect named <effectName>

    IF(MACOSX)
        # on Apple platform a VST plugin is a library bundle
        ADD_LIBRARY(${targetName} MODULE
                    ${debuggerVisualizationFileName}
                    ${srcEffect${effectName}FileList_VST})
    ELSE()
        # VST plugin is a dynamic library
        ADD_LIBRARY(${targetName} SHARED
                    ${debuggerVisualizationFileName}
                    ${srcEffect${effectName}FileList_VST})
    ENDIF()

    TARGET_INCLUDE_DIRECTORIES(${targetName} PUBLIC
                               ${additionalIncludeDirectoryList})

    TARGET_LINK_LIBRARIES(${targetName}
                          SoX${effectName}_Effect SoXViewAndController
                          SoXCommon JuceFramework)

    IF(MACOSX)
        # add framework libraries
        FOREACH(frameworkName ${frameworkNameList})
            TARGET_LINK_LIBRARIES(${targetName}
                                  "-framework ${frameworkName}")
        ENDFOREACH(frameworkName)
    ENDIF(MACOSX)

    IF(LINUX)
        TARGET_LINK_LIBRARIES(${targetName} ${unixLibraryList})
        TARGET_LINK_LIBRARIES(${targetName} dl pthread rt)
    ENDIF(LINUX)

    # put library into a bundle structure regardless of operating
    # system
    IF(MACOSX)
        SET_TARGET_PROPERTIES(${targetName} PROPERTIES
                              BUNDLE TRUE
                              BUNDLE_EXTENSION vst3
                              OUTPUT_NAME ${libraryName}
                              SUFFIX "")
    ELSE()
        IF(WINDOWS)
            SET(libraryNameExtension ".vst3")
            SET(vst3PlatformName "x86_64-win")
        ELSE()
            SET(libraryNameExtension ".so")
            SET(vst3PlatformName "x86_64-linux")
        ENDIF()

        SET_TARGET_PROPERTIES(${targetName} PROPERTIES
                              OUTPUT_NAME ${libraryName}
                              PREFIX ""
                              SUFFIX ${libraryNameExtension})

        SET(libraryFileName ${libraryName}${libraryNameExtension})
        SET(libraryFileDirectory
            ${buildDirectory}${configurationSubdirectory})
        SET(vst3Directory
            ${buildDirectory}/${libraryName}/Contents/${vst3PlatformName})
        
        ADD_CUSTOM_COMMAND(TARGET ${targetName} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E make_directory
                                   ${vst3Directory}
                           COMMAND ${CMAKE_COMMAND} -E copy
                                   ${libraryFileDirectory}/${libraryFileName}
                                   ${vst3Directory}
                                   COMMENT "Bundling ${libraryName}.")
    ENDIF()
ENDFUNCTION(makeEffectVSTTarget)

#--------------------

FUNCTION(makeInternalDocumentationTarget targetName)
    # defines the target for the DoxyGen documentation as HTML files

    SET(doxygenRootFile ${doxygenTargetDirectory}/html/index.html)
    CONFIGURE_FILE(${docDoxygenFileTemplate} ${docDoxygenFileName})

    ADD_CUSTOM_COMMAND(OUTPUT ${doxygenRootFile}
                       COMMAND ${DOXYGEN_EXECUTABLE} ${docDoxygenFileName}
                       WORKING_DIRECTORY ${buildDirectory}
                       COMMENT "Building DoxyGen API Documentation."
                       DEPENDS ${allSrcFileList})

    ADD_CUSTOM_TARGET(${targetName} DEPENDS ${doxygenRootFile})
ENDFUNCTION(makeInternalDocumentationTarget)

#--------------------

FUNCTION(makeJuceFrameworkTarget)
    # defines a JUCE static library as target <JuceFrameWork> with
    # classes from the JUCE modules directory

    SET(targetName JuceFramework)

    ADD_LIBRARY(${targetName} STATIC EXCLUDE_FROM_ALL
                ${srcJuceFacadeFileListSTD})

    TARGET_INCLUDE_DIRECTORIES(${targetName} PUBLIC
                               ${additionalIncludeDirectoryList})

    SET_TARGET_PROPERTIES(${targetName} PROPERTIES
                          POSITION_INDEPENDENT_CODE TRUE
                          INTERFACE_POSITION_INDEPENDENT_CODE TRUE)
ENDFUNCTION(makeJuceFrameworkTarget)

#--------------------

FUNCTION(makeLaTeXDocumentationTarget targetName)
    # defines the target for the LaTeX documentation as a PDF manual;
    # does a two phase compilation to get the TOC and AUX files
    # correct

    SET(commonLaTeXParameters
        -c-style-errors
        -interaction=nonstopmode
        --output-directory=${buildDirectory})

    SET(pdfLaTeXCommandA
        ${PDFLATEX_COMPILER} -draftmode ${commonLaTeXParameters})

    SET(pdfLaTeXCommandB
        ${PDFLATEX_COMPILER} ${commonLaTeXParameters})

    SET(docLaTeXFileNameStemBuildPath
        ${buildDirectory}/${docLaTeXFileNameStem})

    SET(docLaTeXPdfFileName ${docLaTeXFileNameStemBuildPath}.pdf)

    # make a temporary copy of documentation and figures into build
    # directory and then process files there via LaTeX compiler
    ADD_CUSTOM_COMMAND(OUTPUT  ${docLaTeXLocalFileName}
                       DEPENDS ${docLaTeXFileName}
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                               ${docLaTeXDirectory}/figures
                               ${buildDirectory}/figures
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different
                               ${docLaTeXFileName}
                               ${docLaTeXLocalFileName}
                      COMMENT "Copying LaTeX Files.")

    ADD_CUSTOM_COMMAND(OUTPUT  ${docLaTeXPdfFileName}
                       DEPENDS ${docLaTeXLocalFileName}
                       COMMAND ${pdfLaTeXCommandA}
                               ${docLaTeXFileNameStem}.ltx
                       COMMAND ${pdfLaTeXCommandB}
                               ${docLaTeXFileNameStem}.ltx
                       COMMENT "Generating Manual.")

    ADD_CUSTOM_TARGET(${targetName}
                      DEPENDS ${docLaTeXPdfFileName})
ENDFUNCTION(makeLaTeXDocumentationTarget)

#--------------------

FUNCTION(makeSoXCommonTarget)
    SET(targetName SoXCommon)

    ADD_LIBRARY(${targetName} STATIC EXCLUDE_FROM_ALL
                ${srcBaseModulesFileList} ${srcPrimitivesFileList}
                ${srcContainersFileList} ${srcCommonAudioFileList}
                ${srcEffectsBaseFileList} ${srcHelpersFileList})

    SET_TARGET_PROPERTIES(${targetName} PROPERTIES
                          POSITION_INDEPENDENT_CODE TRUE
                          INTERFACE_POSITION_INDEPENDENT_CODE TRUE)
ENDFUNCTION(makeSoXCommonTarget)

#--------------------

FUNCTION(makeSoXViewAndControllerTarget)
    # defines a static library as target <SoXViewAndController> with
    # classes interfacing to JUCE

    SET(targetName SoXViewAndController)

    ADD_LIBRARY(${targetName} STATIC EXCLUDE_FROM_ALL
                ${srcViewAndControllerFileList})

    TARGET_INCLUDE_DIRECTORIES(${targetName} PUBLIC
                               ${srcJuceFacadeDirectory}
                               ${additionalIncludeDirectoryList})

    SET_TARGET_PROPERTIES(${targetName} PROPERTIES
                          POSITION_INDEPENDENT_CODE TRUE
                          INTERFACE_POSITION_INDEPENDENT_CODE TRUE)
ENDFUNCTION(makeSoXViewAndControllerTarget)

#--------------------

FUNCTION(shortNameForEffect effectName)
    # returns Apple four character AU subtype name for given
    # <effectName>

    IF(${effectName} STREQUAL "Compander")
        SET(result mcpd)
    ELSEIF(${effectName} STREQUAL "Filter")
        SET(result filt)
    ELSEIF(${effectName} STREQUAL "Gain")
        SET(result gain)
    ELSEIF(${effectName} STREQUAL "Overdrive")
        SET(result ovdr)
    ELSEIF(${effectName} STREQUAL "PhaserAndTremolo")
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

# compile with logging enabled globally for a debug configuration
# ADD_COMPILE_DEFINITIONS($<$<CONFIG:Debug>:LOGGING_IS_ACTIVE>)

# ==================================
# === intermediate library files ===
# ==================================

# --- juce library with classes from JUCE ---
makeJuceFrameworkTarget()

# --- common services library (primitives containers and common audio) ---
makeSoXCommonTarget()

# --- view and controller library ---
makeSoXViewAndControllerTarget()

ADD_CUSTOM_TARGET(SupportLibraries ALL)
ADD_DEPENDENCIES(SupportLibraries
                 JuceFramework SoXCommon SoXViewAndController)

# ==============================
# === effective target files ===
# ==============================

ADD_CUSTOM_TARGET(SoXPlugins ALL)

# ---------------------------------------------------------
# --- define a target SoX<Effect> combining the plugins ---
# --- for the platform for each effect                  ---
# ---------------------------------------------------------

FOREACH(effectName ${effectNameList})
    SET(targetName SoX${effectName})
    ADD_CUSTOM_TARGET(${targetName} ALL)
    ADD_DEPENDENCIES(SoXPlugins ${targetName})
ENDFOREACH(effectName)

# ------------------------------------------------------
# - build a static library for each effect without GUI -
# ------------------------------------------------------

ADD_CUSTOM_TARGET(SoXPlugins_Effect ALL)

FOREACH(effectName ${effectNameList})
    SET(targetName SoX${effectName}_Effect)
    makeEffectStaticLibraryTarget(${effectName} ${targetName})
    ADD_DEPENDENCIES(SoXPlugins_Effect ${targetName})
ENDFOREACH(effectName)

# ---------------------------------------------------------
# --- build a simple program for optimization tests     ---
# ---------------------------------------------------------

SET(targetName ${testProgramName})

SET(testProgramFileList
    ${commonSrcFileList}
    ${srcEffectsDirectory}/SoX-Test/SoX-TEST_main-std.cpp
    ${debuggerVisualizationFileName})

ADD_EXECUTABLE(${targetName} ${testProgramFileList})

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
                      SoXCommon)

## if (CMAKE_BUILD_TYPE STREQUAL "Debug")
##     TARGET_COMPILE_DEFINITIONS(${targetName} PUBLIC -DLOGGING_IS_ACTIVE)
## ENDIF()

# -----------------------------------------------------------------
# --- build a dynamic library for each effect with Juce GUI and ---
# --- VST client plus the static library of that SoX effect     ---
# -----------------------------------------------------------------

ADD_CUSTOM_TARGET(SoXPlugins_VST ALL)

FOREACH(effectName ${effectNameList})
    SET(libraryName SoX${effectName})
    SET(targetName ${libraryName}_VST)
    makeEffectVSTTarget(${effectName} ${libraryName} ${targetName})
    ADD_DEPENDENCIES(SoXPlugins_VST ${targetName})
    ADD_DEPENDENCIES(SoX${effectName} ${targetName})
ENDFOREACH(effectName)

# # -----------------------------------------------------------------
# # --- build a dynamic library for each effect with Juce GUI and ---
# # --- LV2 client plus the static library of that SoX effect     ---
# # -----------------------------------------------------------------

# makeEffectLV2Target(Gain SoXGain SoXGain_LV2)

# -----------------------------------------------------------------
# --- build an audio unit app for each effect with Juce GUI and ---
# --- AUv3 client plus the static library of that SoX effect     ---
# -----------------------------------------------------------------

IF(MACOSX)
    ADD_CUSTOM_TARGET(SoXPlugins_AU ALL)

    FOREACH(effectName ${effectNameList})
        SET(libraryName SoX${effectName})
        SET(targetName ${libraryName}_AU)
        makeEffectAUTarget(${effectName} ${libraryName} ${targetName})
        ADD_DEPENDENCIES(SoXPlugins_AU ${targetName})
        ADD_DEPENDENCIES(SoX${effectName} ${targetName})
    ENDFOREACH(effectName)
ENDIF()

# ---------------------------
# --- LaTeX Documentation ---
# ---------------------------

ADD_CUSTOM_TARGET(documentation ALL)

IF(NOT LATEX_PDFLATEX_FOUND)
    MESSAGE(STATUS "No PDFLaTeX compiler found --> skipping.")
ELSE()
    SET(targetName pdfDocumentation)
    makeLaTeXDocumentationTarget(${targetName})
    ADD_DEPENDENCIES(documentation ${targetName})
ENDIF()

# -----------------------------
# --- DoxyGen Documentation ---
# -----------------------------

IF(NOT DOXYGEN_FOUND)
    MESSAGE(STATUS "No DoxyGen or GraphViz found --> skipping.")
ELSE()
    SET(targetName internalDocumentation)
    makeInternalDocumentationTarget(${targetName})
    ADD_DEPENDENCIES(documentation ${targetName})
ENDIF()

# ====================
# === INSTALLATION ===
# ====================

SET(platformTargetSubdirectory
    ${platformTargetDirectory}/${platformName}${configurationSubdirectory})

IF(MSVC)
    # make installation part of build in MSVC
    SET(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)
ENDIF()

# --- install the documentation ---
IF(LATEX_PDFLATEX_FOUND)
    # WORKAROUND: find the path of the PDF file from some target
    SET(specificTargetDirectory ${targetDirectory}/doc)
    FILE(REMOVE_RECURSE ${specificTargetDirectory})
    INSTALL(FILES ${buildDirectory}/${docLaTeXFileNameStem}.pdf
            DESTINATION ${specificTargetDirectory})
ENDIF()

# --- install the regression test files ---
SET(specificTargetDirectory ${targetDirectory}/test)
FILE(REMOVE_RECURSE ${specificTargetDirectory})
INSTALL(FILES ${regressionTestFileList}
        DESTINATION ${specificTargetDirectory})

# --- install the VST3 plugins to platform directory ---
FOREACH(effectName ${effectNameList})
    SET(libraryName SoX${effectName})
    SET(targetVST3Directory ${platformTargetSubdirectory}/VST3)
    FILE(REMOVE_RECURSE ${targetVST3Directory})

    IF(NOT MACOSX)
        SET(targetVST3Path ${targetVST3Directory}/${libraryName}.vst3)
        INSTALL(DIRECTORY ${buildDirectory}/${libraryName}/Contents
                DESTINATION ${targetVST3Path})
    ELSE()
        SET(targetName ${libraryName}_VST)
        INSTALL(TARGETS ${targetName}
                DESTINATION ${targetVST3Directory})
    ENDIF()
ENDFOREACH(effectName)

# --- install the AU plugins to platform directory ---
IF(MACOSX)
    FOREACH(effectName ${effectNameList})
        SET(libraryName SoX${effectName})
        SET(targetName ${libraryName}_AU)
        SET(specificTargetDirectory
            ${platformTargetDirectory}/${platformName}/${cmakeBuildType}/AU)
        FILE(REMOVE_RECURSE ${specificTargetDirectory})
        INSTALL(TARGETS ${targetName}
                DESTINATION ${specificTargetDirectory})
    ENDFOREACH(effectName)
ENDIF(MACOSX)
