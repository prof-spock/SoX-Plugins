#########################################
# CMAKE file for the SoXPlugins project #
#                                       #
# Dr. Thomas Tensi, 2021-04             #
#########################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(SoXPlugins
        VERSION 1.0
        LANGUAGES CXX)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
ENDIF()

MESSAGE(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# =============================
# === program configuration ===
# =============================

SET(platformName ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
SET(extendedPlatformName ${platformName}-${CMAKE_BUILD_TYPE})

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

# ===================
# === directories ===
# ===================

SET(buildDirectory ${CMAKE_CURRENT_BINARY_DIR})
SET(targetDirectory ${buildDirectory}/../_DISTRIBUTION)

# --- source directories ---
SET(srcDirectory ../src)
SET(srcBaseTypesDirectory          ${srcDirectory}/BaseTypes)
SET(srcPrimitivesDirectory         ${srcBaseTypesDirectory}/Primitives)
SET(srcContainersDirectory         ${srcBaseTypesDirectory}/Containers)
SET(srcCommonAudioDirectory        ${srcDirectory}/CommonAudio)
SET(srcJuceFacadeDirectory         ${srcDirectory}/JuceLibraryFacade)
SET(srcViewAndControllerDirectory  ${srcDirectory}/ViewAndController)
SET(srcEffectsDirectory            ${srcDirectory}/Effects)
SET(srcEffectsTestDirectory        ${srcEffectsDirectory}/SoX-Test)

FOREACH(effectName ${effectNameList})
    SET(srcEffect${effectName}Directory
        ${srcEffectsDirectory}/SoX${effectName})
ENDFOREACH(effectName)

SET(docDirectory ../doc)
SET(docLatexDirectory ../doc/latex)
SET(doxygenTargetDirectory ../internalDocumentation)
SET(testFileDirectory ../test)

# --- include directories ---

SET(juceModulesDirectory ${JUCE_DIR}/modules)

SET(standardIncludeDirectories
    ${srcDirectory}/BaseTypes
    ${srcBaseTypesDirectory}/Primitives
    ${srcBaseTypesDirectory}/Containers
    ${srcDirectory}/CommonAudio
    ${srcDirectory}/ViewAndController)

SET(additionalIncludeDirectories
    ${srcDirectory}/JuceLibraryFacade
    ${juceModulesDirectory}
    ${juceModulesDirectory}/juce_audio_processors/format_types/VST3_SDK)

IF(APPLE)
    SET(additionalIncludeDirectories
        ${additionalIncludeDirectories}
        /Developer/Headers/FlatCarbon)
ENDIF(APPLE)

# --- additional JUCE settings ---

SET(jucePrefix ${srcJuceFacadeDirectory}/include_juce)

IF(APPLE)
    SET(juceSuffix mm)
ELSE()
    SET(juceSuffix cpp)
ENDIF()

# ====================
# === source files ===
# ====================

SET(srcBaseTypesFiles
    ${srcBaseTypesDirectory}/LoggingSupport.cpp
    ${srcBaseTypesDirectory}/StringUtil.cpp)

SET(srcCommonAudioFiles
    ${srcCommonAudioDirectory}/SoXAudioEffect.cpp
    ${srcCommonAudioDirectory}/SoXAudioParameterMap.cpp
    ${srcCommonAudioDirectory}/SoXAudioSampleBuffer.cpp
    ${srcCommonAudioDirectory}/SoXAudioSampleList.cpp
    ${srcCommonAudioDirectory}/SoXAudioSampleMatrix.cpp
    ${srcCommonAudioDirectory}/SoXAudioSampleQueue.cpp
    ${srcCommonAudioDirectory}/SoXIIRFilter.cpp
    ${srcCommonAudioDirectory}/SoXWaveForm.cpp)

SET(srcContainersFiles
    ${srcContainersDirectory}/Dictionary.cpp
    ${srcContainersDirectory}/IntegerList.cpp
    ${srcContainersDirectory}/NaturalList.cpp
    ${srcContainersDirectory}/RealList.cpp
    ${srcContainersDirectory}/StringList.cpp
    ${srcContainersDirectory}/StringSet.cpp)

SET(srcPrimitivesFiles ${srcPrimitivesDirectory}/MyString.cpp)

SET(srcViewAndControllerFiles
    ${srcViewAndControllerDirectory}/SoXAudioEditor.cpp
    ${srcViewAndControllerDirectory}/SoXAudioEditorWidget.cpp
    ${srcViewAndControllerDirectory}/SoXAudioProcessor.cpp)

SET(allSrcFiles
    ${srcBaseTypesFiles} ${srcCommonAudioFiles} ${srcContainersFiles}
    ${srcPrimitivesFiles} ${srcViewAndControllerFiles})

# -------------------------------------------------------------------
# --- XXXFilesSTD is the file name list for the plain audio       ---
# --- effect library of some SoX effect                           ---
# -------------------------------------------------------------------

FOREACH(effectName Filter Gain Overdrive PhaserAndTremolo)
    SET(fileListName srcEffect${effectName}FilesSTD)

    SET(${fileListName}
        ${srcEffect${effectName}Directory}/SoX${effectName}_AudioEffect.cpp)

    SET(allSrcFiles ${allSrcFiles} ${${fileListName}})
ENDFOREACH(effectName)

FOREACH(effectName Compander Reverb)
    SET(effectDirectory ${srcEffect${effectName}Directory})

    SET(fileListName srcEffect${effectName}FilesSTD)

    SET(${fileListName}
        ${effectDirectory}/SoX${effectName}_AudioEffect.cpp
        ${effectDirectory}/SoX${effectName}Support.cpp)

    SET(allSrcFiles ${allSrcFiles} ${${fileListName}})
ENDFOREACH(effectName)

# -------------------------------------------------------------------
# --- XXXFiles_VST is the file name list for the VST3 library     ---
# --- for some SoX effect                                         ---
# -------------------------------------------------------------------

FOREACH(effectName ${effectNameList})
    SET(effectDirectory ${srcEffect${effectName}Directory})

    SET(fileListName srcEffect${effectName}Files_VST)

    SET(${fileListName}
        ${effectDirectory}/SoX${effectName}_AudioProcessor.cpp
        ${effectDirectory}/SoXPlugin-util.cpp
        ${effectDirectory}/SoXPlugin-VST_1.cpp)

    IF(WINDOWS)
        SET(${fileListName}
            ${${fileListName}}
            ${effectDirectory}/resources.rc)
    ENDIF(WINDOWS)

    IF(APPLE)
        SET(${fileListName}
            ${${fileListName}}
            ${effectDirectory}/SoXPlugin-VST_2.${juceSuffix}
            ${effectDirectory}/SoXPlugin-Standalone.cpp)
    ENDIF(APPLE)

    SET(allSrcFiles ${allSrcFiles} ${${fileListName}})
ENDFOREACH(effectName)

# -------------------------------------------------------------------
# --- XXXFiles_AU is the file name list for the AUv3 library for  ---
# --- some SoX effect                                             ---
# -------------------------------------------------------------------

IF(APPLE)
    FOREACH(effectName ${effectNameList})
        SET(effectDirectory ${srcEffect${effectName}Directory})

        SET(fileListName srcEffect${effectName}Files_AU)

        SET(${fileListName}
            ${effectDirectory}/SoX${effectName}_AudioProcessor.cpp
            ${effectDirectory}/SoXPlugin-util.cpp
            ${effectDirectory}/SoXPlugin-AU_1.mm
            ${effectDirectory}/SoXPlugin-AU_2.mm
            ${effectDirectory}/SoXPlugin-Standalone.cpp)

        SET(allSrcFiles ${allSrcFiles} ${${fileListName}})
    ENDFOREACH(effectName)
ENDIF(APPLE)

# -------------------------------------------------------------------
# --- a simple command line test for some of the engines          ---
# -------------------------------------------------------------------

SET(srcEffectsTestFiles
    ${srcEffectsTestDirectory}/SoX-TEST_main-std.cpp)

SET(allSrcFiles ${allSrcFiles} ${srcEffectsTestFiles})

# -------------------------------------------------------------------
# --- the file name list of facade files for JUCE; those files    ---
# --- reference real implementations in JUCE; note that on        ---
# --- Apple platforms the Objective-C++ (.mm) files are used      ---
# --- of the standard C++ (.cpp) files                            ---
# -------------------------------------------------------------------

SET(srcJuceFacadeFilesSTD
    ${jucePrefix}_audio_basics.${juceSuffix}
    ${jucePrefix}_audio_devices.${juceSuffix}
    ${jucePrefix}_audio_formats.${juceSuffix}
    ${jucePrefix}_audio_processors.${juceSuffix}
    ${jucePrefix}_audio_utils.${juceSuffix}
    ${jucePrefix}_core.${juceSuffix}
    ${jucePrefix}_data_structures.${juceSuffix}
    ${jucePrefix}_events.${juceSuffix}
    ${jucePrefix}_graphics.${juceSuffix}
    ${jucePrefix}_gui_basics.${juceSuffix}
    ${jucePrefix}_gui_extra.${juceSuffix})

SET(srcJuceFacadeFiles_VST
    ${jucePrefix}_audio_plugin_client_utils.cpp
    ${jucePrefix}_audio_plugin_client_VST_utils.${juceSuffix})

SET(srcJuceFacadeFiles_AU
    #${jucePrefix}_audio_plugin_client_utils.cpp
    #${jucePrefix}_audio_plugin_client_AU_1.mm
    #${jucePrefix}_audio_plugin_client_AU_2.mm
    )

# --- documentation sources ---
SET(docLatexFileNameStem SoXPlugins-documentation)
SET(docLatexFile ${docLatexDirectory}/${docLatexFileNameStem}.ltx)
SET(docDoxygenFile ${docDirectory}/doxygen/SoXPlugins-doxygen-FULL.cfg)

# --- regression test sources ---
SET(regressionTestFileList
    ${testFileDirectory}/makeTestFiles.bat
    ${testFileDirectory}/makeTestFiles.sh
    "${testFileDirectory}/testSoxPlugins-C++.rpp")

# -------------------------------------------------
# --- Apple framework files and bundle settings ---
# -------------------------------------------------

IF(NOT APPLE)
    SET(frameworkNameList '')
ELSE()
    SET(frameworkNameList
        AVFoundation AVKit Accelerate AudioToolbox AudioUnit Carbon
        Cocoa CoreAudio CoreAudioKit CoreGraphics CoreImage CoreMIDI
        CoreMedia CoreServices CoreText DiscRecording Foundation
        IOKit ImageIO OpenGL QTKit QuartzCore WebKit)

    SET(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    SET(bundlePrefix "eu.tensi.thomas.SoXPlugins")
ENDIF(NOT APPLE)

###########
# TARGETS #
###########

INCLUDE_DIRECTORIES(${standardIncludeDirectories})

# ==================================
# === intermediate library files ===
# ==================================

# --- juce library with classes from JUCE ---
ADD_LIBRARY(JuceFramework STATIC EXCLUDE_FROM_ALL
            ${srcJuceFacadeFilesSTD})

TARGET_INCLUDE_DIRECTORIES(JuceFramework PUBLIC
                           ${additionalIncludeDirectories})

SET_TARGET_PROPERTIES(JuceFramework PROPERTIES
        XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME NO
        XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES YES
        POSITION_INDEPENDENT_CODE TRUE
        C_VISIBILITY_PRESET hidden)

# --- common services library (primitives containers and common audio) ---
ADD_LIBRARY(SoXCommon STATIC EXCLUDE_FROM_ALL
            ${srcBaseTypesFiles} ${srcPrimitivesFiles}
            ${srcContainersFiles} ${srcCommonAudioFiles})

# --- view and controller library ---
ADD_LIBRARY(SoXViewAndController STATIC EXCLUDE_FROM_ALL
            ${srcViewAndControllerFiles})

TARGET_INCLUDE_DIRECTORIES(SoXViewAndController PUBLIC
                           ${srcJuceFacadeDirectory}
                           ${additionalIncludeDirectories})

ADD_CUSTOM_TARGET(SupportLibraries ALL)
ADD_DEPENDENCIES(SupportLibraries
                 JuceFramework SoXCommon SoXViewAndController)

# ==============================
# === effective target files ===
# ==============================

ADD_CUSTOM_TARGET(SoXPlugins ALL)

# -------------------------------------------------------
# --- a target combining the plugins for the platform ---
# --- for each effect                                 ---
# -------------------------------------------------------

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

    ADD_LIBRARY(${targetName} STATIC
                ${srcEffect${effectName}FilesSTD})

    TARGET_INCLUDE_DIRECTORIES(${targetName}
                               PUBLIC ${srcEffect${effectName}Directory})

    ADD_DEPENDENCIES(SoXPlugins_Effect ${targetName})
ENDFOREACH(effectName)

# -----------------------------------------------------------------
# --- build a dynamic library for each effect with Juce GUI and ---
# --- VST client plus the static library of that SoX effect     ---
# -----------------------------------------------------------------

ADD_CUSTOM_TARGET(SoXPlugins_VST ALL)

FOREACH(effectName ${effectNameList})
    SET(libraryName SoX${effectName})
    SET(targetName ${libraryName}_VST)

    IF(NOT APPLE)
        ADD_LIBRARY(${targetName} SHARED
                    ${srcEffect${effectName}Files_VST})
    ELSE()
        ADD_EXECUTABLE(${targetName} MACOSX_BUNDLE
                       ${srcEffect${effectName}Files_VST})
        FOREACH(frameworkName ${frameworkNameList})
            TARGET_LINK_LIBRARIES(${targetName} "-framework ${frameworkName}")
        ENDFOREACH(frameworkName)
    ENDIF(NOT APPLE)

    TARGET_INCLUDE_DIRECTORIES(${targetName} PUBLIC
                               ${additionalIncludeDirectories})

    TARGET_LINK_LIBRARIES(${targetName}
                          SoXCommon SoXViewAndController JuceFramework
                          SoX${effectName}_Effect)

    SET_TARGET_PROPERTIES(${targetName} PROPERTIES
                          OUTPUT_NAME ${libraryName}
                          SUFFIX ".vst3")

    ADD_DEPENDENCIES(SoXPlugins_VST ${targetName})
    ADD_DEPENDENCIES(SoX${effectName} ${targetName})
ENDFOREACH(effectName)

# -----------------------------------------------------------------
# --- build an audio unit app for each effect with Juce GUI and ---
# --- AUv3 client plus the static library of that SoX effect     ---
# -----------------------------------------------------------------

IF(APPLE)
    ADD_CUSTOM_TARGET(SoXPlugins_AU ALL)

    FOREACH(effectName ${effectNameList})
        SET(libraryName SoX${effectName})
        SET(targetName ${libraryName}_AU)

        ADD_EXECUTABLE(${targetName} MACOSX_BUNDLE
                       ${srcEffect${effectName}Files_AU})

        TARGET_INCLUDE_DIRECTORIES(${targetName} PUBLIC
                                   ${additionalIncludeDirectories})

        TARGET_LINK_LIBRARIES(${targetName}
                              SoXCommon SoXViewAndController JuceFramework
                              SoXEffect${effectName})

        FOREACH(frameworkName ${frameworkNameList})
            TARGET_LINK_LIBRARIES(${targetName} "-framework ${frameworkName}")
        ENDFOREACH(frameworkName)

        SET_TARGET_PROPERTIES(${targetName} PROPERTIES
            BUNDLE True
            MACOSX_BUNDLE_BUNDLE_NAME ${libraryName}
            MACOSX_BUNDLE_GUI_IDENTIFIER ${bundlePrefix}.${libraryName}
            OUTPUT_NAME ${libraryName}
            SUFFIX ".au")

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
    # do a two phase compilation to get the TOC and AUX files
    # correct
    SET(pdflatexCommandA
        ${PDFLATEX_COMPILER} -draftmode -interaction=nonstopmode
        --output-directory=${buildDirectory})

    SET(pdflatexCommandB
        ${PDFLATEX_COMPILER} -interaction=nonstopmode
        --output-directory=${buildDirectory})

    SET(docLatexFileNameStemBuildPath
        ${buildDirectory}/${docLatexFileNameStem})

    SET(docLatexPdfFile ${docLatexFileNameStemBuildPath}.pdf)

    ADD_CUSTOM_TARGET(pdfDocumentation
                      DEPENDS ${docLatexFile}
                      COMMAND ${pdflatexCommandA}
                              ${docLatexFileNameStem}.ltx
                      COMMAND ${pdflatexCommandB}
                              ${docLatexFileNameStem}.ltx
                      COMMENT "Generating Manual."
                      WORKING_DIRECTORY ${docLatexDirectory})

    ADD_DEPENDENCIES(documentation pdfDocumentation)
ENDIF()

# -----------------------------
# --- DoxyGen Documentation ---
# -----------------------------

IF(NOT DOXYGEN_FOUND)
    MESSAGE(STATUS "No DoxyGen or GraphViz found --> skipping.")
ELSE()
    SET(doxygenRootFile ${doxygenTargetDirectory}/html/index.html)
    CONFIGURE_FILE(${docDoxygenFile}.in ${docDoxygenFile})

    ADD_CUSTOM_COMMAND(OUTPUT ${doxygenRootFile}
                       COMMAND ${DOXYGEN_EXECUTABLE} ${docDoxygenFile}
                       WORKING_DIRECTORY ${buildDirectory}
                       COMMENT "Building DoxyGen API Documentation."
                       DEPENDS ${allSrcFiles})

    ADD_CUSTOM_TARGET(internalDocumentation
                      DEPENDS ${doxygenRootFile})

    ADD_DEPENDENCIES(documentation internalDocumentation)
ENDIF()

# ====================
# === INSTALLATION ===
# ====================

# --- install the documentation ---
IF(LATEX_PDFLATEX_FOUND)
    # WORKAROUND: find the path of the PDF file from some target
    INSTALL(FILES ${buildDirectory}/${docLatexFileNameStem}.pdf
            DESTINATION ${targetDirectory}/doc)
ENDIF()

# --- install the regression test files ---
INSTALL(FILES ${regressionTestFileList} DESTINATION ${targetDirectory}/test)

# --- install the VST3 plugins to platform directory ---
FOREACH(effectName ${effectNameList})
    SET(libraryName SoX${effectName})
    SET(targetName ${libraryName}_VST)

    INSTALL(TARGETS ${targetName}
            DESTINATION ${targetDirectory}/${platformName})

    # WORKAROUND: get rid of extra static effect lib files
    SET(staticLibFilePath
        ${targetDirectory}/${platformName}/${libraryName}.lib)
    INSTALL(CODE "FILE(REMOVE \"${staticLibFilePath}\")")
ENDFOREACH(effectName)

# --- install the AUv3 plugins to platform directory ---
IF(APPLE)
    FOREACH(effectName ${effectNameList})
        SET(libraryName SoX${effectName})
        SET(targetName ${libraryName}_AU)

        INSTALL(TARGETS ${targetName}
                DESTINATION ${targetDirectory}/${platformName})
    ENDFOREACH(effectName)
ENDIF()

# --- install the regression test files ---
INSTALL(FILES ${regressionTestFileList} DESTINATION ${targetDirectory}/test)
