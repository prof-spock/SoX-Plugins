/**
 * @file
 * The <C>LoggingSupport</C> specification defines a class for simple
 * entry/exit logging to a file; this logging relies on trace calls at
 * the beginning or end of a function with prefices ">>" and "<<" as
 * well as intermediate log lines with prefix "--"; the name of the
 * function is also logged to give a fully bracketed log.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-02
 */

#pragma once

/*====================*/

#include "GlobalMacros.h"
#include "Natural.h"
#include "MyString.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::String;

/*====================*/

namespace SoXPlugins::BaseTypes {

    /**
     * A <C>Logging</C> object defines methods for entry/exit
     * logging to a file; this logging relies on trace calls at the
     * beginning or end of a function with prefices ">>" and "<<" as
     * well as intermediate log lines with prefix "--"; the name of
     * the function is also logged to give a fully bracketed log.
     */
    struct Logging {

        /**
         * Sets up logging configuration and starts logging.
         */
        static void initialize ();

        /*--------------------*/

        /**
         * Sets up logging configuration and starts logging with
         * hundredth seconds time collection onto a logging file
         * in the temp directory with <C>fileNameStem</C> as part
         * of the log file name; <C>ignoredFunctionNamePrefix</C>
         * tells what prefix shall be skipped from the full
         * function names; this is a convenience function
         * combining several elementary functions
         *
         * @param[in] fileNameStem               identifying part of
         *                                       logging file name
         * @param[in] ignoredFunctionNamePrefix  string to be stripped
         *                                       from function names
         */
        static void initializeWithDefaults
                        (IN String& fileNameStem,
                         IN String& ignoredFunctionNamePrefix);

        /*--------------------*/

        /**
         * Cleans up logging configuration and ends logging.
         */
        static void finalize ();

        /*--------------------*/

        /**
         * Sets file name for logging to <C>fileName</C>; if
         * <C>isKeptOpen</C> is set, the logging file is not
         * closed after each log entry.
         *
         * @param[in] fileName    name of logging file from now on
         * @param[in] isKeptOpen  flag to tell that logging file
         *                        should not be closed
         */
        static void setFileName (IN String& fileName,
                                 IN bool isKeptOpen=true);

        /*--------------------*/

        /**
         * Sets function name prefix to be ignored to {@code namePrefix}.
         *
         * @param[in] namePrefix  string to be stripped from function
         *                        names
         */
        static void setIgnoredFunctionNamePrefix (IN String& namePrefix);

        /*--------------------*/

        /**
         * Sets logging of time when tracing to active or inactive.
         *
         * @param[in] timeIsLogged          sets or disables time output
         *                                  in log
         * @param[in] fractionalDigitCount  number of digits after decimal
         *                                  point in timing data
         */
        static void setTracingWithTime (IN bool timeIsLogged,
                                        IN Natural fractionalDigitCount=0);

        /*--------------------*/

        /**
         * Writes <C>message</C> together with function name
         * derived from <C>functionSignature</C> to log file
         *
         * @param[in] functionSignature  signature of enclosing
         *                               function
         * @param[in] message            string to be logged
         */
        static void trace (IN String& functionSignature,
                           IN String& message);

        /*--------------------*/

        /**
         * Writes <C>message</C> together with function name
         * derived from <C>functionSignature</C> to log file as an
         * error entry.
         *
         * @param[in] functionSignature  signature of enclosing
         *                               function
         * @param[in] message            string to be logged
         */
        static void traceError (IN String& functionSignature,
                                IN String& message);

    };

};
