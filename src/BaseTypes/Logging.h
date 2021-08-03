/**
 * @file
 * The <C>Logging</C> specification defines macros for entry-, exit-
 * and intermediate logging of functions into a file; this file
 * provides a conditional facade that expands all logging functions to
 * empty for production code and expands to concrete logging
 * implementations for debugging code.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-02
 */

#pragma once

/*====================*/

#include <cstdarg>

#include "LoggingSupport.h"
#include "StringUtil.h"

/*====================*/

using SoXPlugins::BaseTypes::Logging;
using SoXPlugins::BaseTypes::StringUtil::expand;

/*====================*/

#ifdef __FUNCSIG__
    #define signatureOfFunction __FUNCSIG__
#else
    #define signatureOfFunction __PRETTY_FUNCTION__
#endif

/*--------------------*/

#ifdef LOGGING_IS_ACTIVE
    // for active logging routines are redirected to corresponding
    // routines in logging class

    #define Logging_initialize()  Logging::initialize()

    #define Logging_initializeWithDefaults(fileNameStem, \
                                           ignoredFunctionNamePrefix) \
            Logging::initializeWithDefaults(fileNameStem, \
                                            ignoredFunctionNamePrefix)

    #define Logging_finalize()    Logging::finalize()

    #define Logging_setFileName(fileName, isKeptOpen)  \
                Logging::setFileName(fileName, isKeptOpen)

    #define Logging_setIgnoredFunctionNamePrefix(prefix) \
                Logging::setIgnoredFunctionNamePrefix(prefix)

    #define Logging_setTracingWithTime(timeIsLogged, fractionalDigitCount) \
                Logging::setTracingWithTime(timeIsLogged, \
                                            fractionalDigitCount)

    #define _Logging_trace(message) \
                Logging::trace(signatureOfFunction, message)

    #define _Logging_traceError(message) \
                Logging::traceError(signatureOfFunction, message)

#else
    // for inactive logging routines are simply empty

    #define Logging_initialize()

    #define Logging_initializeWithDefaults(fileNameStem, \
                                           ignoredFunctionNamePrefix)

    #define Logging_finalize()

    #define Logging_setFileName(fileName, isKeptOpen)

    #define Logging_setIgnoredFunctionNamePrefix(prefix)

    #define Logging_setTracingWithTime(timeIsLogged, fractionalDigitCount)

    #define _Logging_trace(message)

    #define _Logging_traceError(message)

#endif

/*--------------------*/

#define Logging_trace(formattingTemplate)   \
            _Logging_trace(formattingTemplate)
#define Logging_trace1(formattingTemplate, st1) \
            _Logging_trace(expand(formattingTemplate, st1))
#define Logging_trace2(formattingTemplate, st1, st2) \
            _Logging_trace(expand(formattingTemplate, st1, st2))
#define Logging_trace3(formattingTemplate, st1, st2, st3) \
            _Logging_trace(expand(formattingTemplate, st1, st2, st3))
#define Logging_trace4(formattingTemplate, st1, st2, st3, st4)  \
            _Logging_trace(expand(formattingTemplate, \
                                  st1, st2, st3, st4))
#define Logging_trace5(formattingTemplate, st1, st2, st3, st4, \
                       st5) \
            _Logging_trace(expand(formattingTemplate, \
                                  st1, st2, st3, st4, st5))
#define Logging_trace6(formattingTemplate, st1, st2, st3, st4, \
                       st5, st6) \
            _Logging_trace(expand(formattingTemplate, \
                                  st1, st2, st3, st4, st5, \
                                   st6))
#define Logging_trace7(formattingTemplate, st1, st2, st3, st4, \
                       st5, st6, st7) \
            _Logging_trace(expand(formattingTemplate, \
                                  st1, st2, st3, st4, st5, \
                                  st6, st7))
#define Logging_trace8(formattingTemplate, st1, st2, st3, st4, \
                       st5, st6, st7, st8) \
            _Logging_trace(expand(formattingTemplate, \
                                  st1, st2, st3, st4, st5, \
                                  st6, st7, st8))
#define Logging_trace9(formattingTemplate, st1, st2, st3, st4, \
                       st5, st6, st7, st8, st9) \
            _Logging_trace(expand(formattingTemplate, \
                                  st1, st2, st3, st4, st5, \
                                  st6, st7, st8, st9))
#define Logging_trace10(formattingTemplate, st1, st2, st3, st4, \
                        st5, st6, st7, st8, st9, st10) \
            _Logging_trace(expand(formattingTemplate, \
                                  st1, st2, st3, st4, st5, \
                                  st6, st7, st8, st9, st10))
#define Logging_traceError(formattingTemplate)   \
            _Logging_traceError(formattingTemplate)
#define Logging_traceError1(formattingTemplate, st1) \
            _Logging_traceError(expand(formattingTemplate, st1))
#define Logging_traceError2(formattingTemplate, st1, st2) \
            _Logging_traceError(expand(formattingTemplate, st1, st2))
#define Logging_traceError3(formattingTemplate, st1, st2, st3) \
            _Logging_traceError(expand(formattingTemplate, st1, st2, st3))
#define Logging_traceError4(formattingTemplate, st1, st2, st3, st4)  \
            _Logging_traceError(expand(formattingTemplate, \
                                       st1, st2, st3, st4))
#define Logging_traceError5(formattingTemplate, st1, st2, st3, st4, \
                            st5) \
            _Logging_traceError(expand(formattingTemplate, \
                                       st1, st2, st3, st4, st5))
#define Logging_traceError6(formattingTemplate, st1, st2, st3, st4, \
                            st5, st6) \
            _Logging_traceError(expand(formattingTemplate, \
                                       st1, st2, st3, st4, st5, \
                                       st6))
#define Logging_traceError7(formattingTemplate, st1, st2, st3, st4, \
                            st5, st6, st7) \
            _Logging_traceError(expand(formattingTemplate, \
                                       st1, st2, st3, st4, st5, \
                                       st6, st7))
#define Logging_traceError8(formattingTemplate, st1, st2, st3, st4, \
                            st5, st6, st7, st8) \
            _Logging_traceError(expand(formattingTemplate, \
                                       st1, st2, st3, st4, st5, \
                                       st6, st7, st8))
#define Logging_traceError9(formattingTemplate, st1, st2, st3, st4, \
                            st5, st6, st7, st8, st9) \
            _Logging_traceError(expand(formattingTemplate, \
                                       st1, st2, st3, st4, st5, \
                                       st6, st7, st8, st9))
#define Logging_traceError10(formattingTemplate, st1, st2, st3, st4, \
                             st5, st6, st7, st8, st9, st10)                  \
            _Logging_traceError(expand(formattingTemplate, \
                                       st1, st2, st3, st4, st5, \
                                       st6, st7, st8, st9, st10))
