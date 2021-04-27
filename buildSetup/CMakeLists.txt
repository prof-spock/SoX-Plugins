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
FIND_PACKAGE(LATEX COMPONENTS PDFLATEX)

# --- DoxyGen ---
#FIND_PACKAGE(Doxygen REQUIRED dot)

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
# --- XXXFilesVST is the file name list for the VST3 library for  ---
# --- some SoX effect                                             ---
# -------------------------------------------------------------------

FOREACH(effectName ${effectNameList})
    SET(effectDirectory ${srcEffect${effectName}Directory})

    SET(fileListName srcEffect${effectName}FilesVST)

    SET(${fileListName}
        ${effectDirectory}/resources.rc
        ${effectDirectory}/SoX${effectName}_main-vst3.cpp
        ${effectDirectory}/SoX${effectName}_util.cpp)

    SET(allSrcFiles ${allSrcFiles} ${${fileListName}})
ENDFOREACH(effectName)

SET(srcEffectsTestFiles
    ${srcEffectsTestDirectory}/SoX-TEST_main-std.cpp)

SET(allSrcFiles ${allSrcFiles} ${srcEffectsTestFiles})

# -------------------------------------------------------------------
# --- the file name list of facade files for JUCE; those files    ---
# --- reference real implementations in JUCE                      ---
# -------------------------------------------------------------------

SET(srcJuceFacadeFilesSTD
    ${srcJuceFacadeDirectory}/include_juce_audio_basics.cpp
    ${srcJuceFacadeDirectory}/include_juce_audio_devices.cpp
    ${srcJuceFacadeDirectory}/include_juce_audio_formats.cpp
    ${srcJuceFacadeDirectory}/include_juce_audio_processors.cpp
    ${srcJuceFacadeDirectory}/include_juce_audio_utils.cpp
    ${srcJuceFacadeDirectory}/include_juce_core.cpp
    ${srcJuceFacadeDirectory}/include_juce_cryptography.cpp
    ${srcJuceFacadeDirectory}/include_juce_data_structures.cpp
    ${srcJuceFacadeDirectory}/include_juce_events.cpp
    ${srcJuceFacadeDirectory}/include_juce_graphics.cpp
    ${srcJuceFacadeDirectory}/include_juce_gui_basics.cpp
    ${srcJuceFacadeDirectory}/include_juce_gui_extra.cpp
    ${srcJuceFacadeDirectory}/include_juce_opengl.cpp)

SET(srcJuceFacadeFilesPLUGIN
    ${srcJuceFacadeDirectory}/include_juce_audio_plugin_client_RTAS_utils.cpp
    ${srcJuceFacadeDirectory}/include_juce_audio_plugin_client_Standalone.cpp
    ${srcJuceFacadeDirectory}/include_juce_audio_plugin_client_utils.cpp
    ${srcJuceFacadeDirectory}/include_juce_audio_plugin_client_VST2.cpp
    ${srcJuceFacadeDirectory}/include_juce_audio_plugin_client_VST3.cpp)

# --- documentation sources ---
SET(docLatexFileNameStem SoXPlugins-documentation)
SET(docLatexFile ${docLatexDirectory}/${docLatexFileNameStem}.ltx)
SET(docDoxygenFile ${docDirectory}/doxygen/SoXPlugins-doxygen-FULL.cfg)

# --- regression test sources ---
SET(regressionTestFileList
    ${testFileDirectory}/makeTestFiles.bat
    ${testFileDirectory}/makeTestFiles.sh
    "${testFileDirectory}/testSoxPlugins-C++.rpp")

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

# ------------------------------------------------------
# - build a static library for each effect without GUI -
# ------------------------------------------------------

FOREACH(effectName ${effectNameList})
    SET(libraryName SoXEffect${effectName})

    ADD_LIBRARY(${libraryName} STATIC
                ${srcEffect${effectName}FilesSTD})

    TARGET_INCLUDE_DIRECTORIES(${libraryName}
                               PUBLIC ${srcEffect${effectName}Directory})
ENDFOREACH(effectName)

# ==============================
# === effective target files ===
# ==============================

# -----------------------------------------------------------------
# --- build a DLL library for each effect with Juce GUI and VST ---
# --- client plus the static library of that SoX effect         ---
# -----------------------------------------------------------------

FOREACH(effectName ${effectNameList})
    SET(libraryName SoX${effectName})

    ADD_LIBRARY(${libraryName} SHARED
                ${srcEffect${effectName}FilesVST})

    TARGET_INCLUDE_DIRECTORIES(${libraryName} PUBLIC
                               ${additionalIncludeDirectories})

    TARGET_LINK_LIBRARIES(${libraryName}
                          SoXCommon SoXViewAndController JuceFramework
                          SoXEffect${effectName})

    SET_TARGET_PROPERTIES(${libraryName} PROPERTIES SUFFIX ".vst3")
ENDFOREACH(effectName)

# -------------------------------------------------
# --- a test executable for a simple smoke test ---
# -------------------------------------------------

ADD_EXECUTABLE(SoXTest ${srcEffectsTestFiles})

TARGET_INCLUDE_DIRECTORIES(SoXTest PUBLIC
    ${srcEffectsCompanderDirectory}
    ${srcEffectsPhaserAndTremoloDirectory}
    ${srcEffectsReverbDirectory})

TARGET_LINK_LIBRARIES(SoXTest
                      SoXCommon SoXEffectCompander
                      SoXEffectPhaserAndTremolo SoXEffectReverb)

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

    ADD_CUSTOM_TARGET(finalPDFManual
                      DEPENDS ${docLatexFile}
                      COMMAND ${pdflatexCommandA}
                              ${docLatexFileNameStem}.ltx
                      COMMAND ${pdflatexCommandB}
                              ${docLatexFileNameStem}.ltx
                      COMMENT "Generating Manual."
                      WORKING_DIRECTORY ${docLatexDirectory})

    ADD_DEPENDENCIES(documentation finalPDFManual)
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

# --- install the VST3 plugins to platform directory ---
FOREACH(effectName ${effectNameList})
    SET(libraryName SoX${effectName})

    INSTALL(TARGETS ${libraryName}
            DESTINATION ${targetDirectory}/${platformName})

    # WORKAROUND: get rid of extra static effect lib files
    SET(staticLibFilePath
        ${targetDirectory}/${platformName}/${libraryName}.lib)
    INSTALL(CODE "FILE(REMOVE \"${staticLibFilePath}\")")
ENDFOREACH(effectName)

# --- install the documentation ---
IF(LATEX_PDFLATEX_FOUND)
    # WORKAROUND: find the path of the PDF file from some target
    INSTALL(FILES ${buildDirectory}/${docLatexFileNameStem}.pdf
            DESTINATION ${targetDirectory}/doc)
ENDIF()

# --- install the regression test files ---
INSTALL(FILES ${regressionTestFileList} DESTINATION ${targetDirectory}/test)
