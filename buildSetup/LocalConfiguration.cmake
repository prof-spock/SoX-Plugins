#
# Local Settings for CMAKE for the SoXPlugins
#

SET(programDirectory "/Program Files")

# the directory where the JUCE modules are located
SET(juceModulesDirectory ${programDirectory}/JUCE/modules)

# the directory where doxygen is located
IF(NOT DOXYGEN_FOUND)
    SET(DOXYGEN_EXECUTABLE ${programDirectory}/DoxyGen/doxygen)
    SET(DOXYGEN_FOUND 1)
ENDIF()
