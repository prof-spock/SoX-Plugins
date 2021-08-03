/**
 * @file
 * The <C>LoggingSupport</C> body implements a class for simple
 * entry/exit logging to a file; this logging relies on trace calls at
 * the beginning or end of a function with prefices ">>" and "<<" as
 * well as intermediate log lines with prefix "--"; the name of the
 * function is also logged to give a fully bracketed log.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-01
 */

/*====================*/

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <ostream>

/*====================*/

#include "LoggingSupport.h"
#include "StringUtil.h"
#include "StringList.h"

/*====================*/

using namespace std::chrono;

using std::ios_base;
using std::ofstream;

using SoXPlugins::BaseTypes::Logging;
using SoXPlugins::BaseTypes::Containers::StringList;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;

using std::getenv;
using StringUtil::expand;
using StringUtil::prefix;
using StringUtil::replace;
using StringUtil::startsWith;
using StringUtil::substring;

/*====================*/

/** the timestamp type as provided by the system call */
using Timestamp = uint64_t;

/*--------------------*/

/** number of milliseconds per day */
static const Natural _millisecondsPerDay = 86400000;

/*--------------------*/

/** time zone offset in milliseconds */
static const Natural _timeOffsetInMilliseconds{3600000};

/*--------------------*/

/** offset for DST in milliseconds */
static const Natural _dstOffsetInMilliseconds{3600000};

/*====================*/

namespace SoXPlugins::BaseTypes {

    /**
     * A <C>_LoggingTime</C> object provides simple time related
     * services like caching the current time string to speed up
     * tracing output.
     */
    class _LoggingTime {

        private:

            /** the number of digits after decimal point for time
             * indications */
            Natural _timeFractionalDigitCount;

            /*--------------------*/

            /** factor 10^<C>timeFractionalDigitCount</C> */
            Natural _timeFactor;

            /*--------------------*/

            /** the effective offset in milliseconds for DST (either 0 or
             * 1h) */
            Natural _effectiveDSTOffsetInMilliseconds;

            /*--------------------*/

            /** the time stamp of the previous log line (with a seconds
             * resolution). */
            Timestamp _previousTimestamp;

            /*--------------------*/

            /** the time of day formatted as a string. */
            String _timeOfDayString;

            /*--------------------*/
            /*--------------------*/

            /**
             * Returns current time with millisecond resolution
             *
             * @return  current time in milliseconds
             */
            Timestamp _getCurrentTime () {
                const system_clock::duration d =
                    system_clock::now().time_since_epoch();
                Timestamp result = duration_cast<milliseconds>(d).count();
                result %= (size_t) _millisecondsPerDay;
                result += (size_t) _timeOffsetInMilliseconds;
                result += (size_t) _effectiveDSTOffsetInMilliseconds;
                result /= (1000 / (size_t) _timeFactor);
                return result;
            }

        public:

            /**
             * Initializes logging time object.
             */
            _LoggingTime () {
                _timeFractionalDigitCount = 0;
                _timeFactor = 1;
                _previousTimestamp = 0;
                _timeOfDayString = "";
                const bool isDST = true;
                _effectiveDSTOffsetInMilliseconds =
                    (isDST ? _dstOffsetInMilliseconds : Natural{0});
            }

            /*--------------------*/

            /**
             * Sets the number of digits after decimal point for
             * time indications to <C>fractionalDigitCount</C>.
             *
             * @param[in] fractionalDigitCount number of digits
             *                                 after decimal point
             *                                 in time indications
             */
            void setFractionalDigitCount (IN Natural fractionalDigitCount)
            {
                _timeFractionalDigitCount =
                    Natural::minimum(3,
                                     Natural::maximum(0,
                                                      fractionalDigitCount));
                _timeFactor = Natural::power(10, _timeFractionalDigitCount);
            }

            /*--------------------*/

            /**
             * Returns current time of day (in seconds) as string;
             * uses caching to speed up processing.
             *
             * @return time of day as string
             */
            String currentTimeOfDay () {
                Timestamp currentTimestamp = _getCurrentTime();

                if (currentTimestamp != _previousTimestamp) {
                    _previousTimestamp = currentTimestamp;
                    const Natural fractionalPart =
                        (Natural) currentTimestamp % _timeFactor;
                    currentTimestamp /= (int) _timeFactor;
                    const Natural seconds =
                        (Natural) currentTimestamp % 60;
                    const Natural minutes =
                        (Natural) (currentTimestamp / 60) % 60;
                    const Natural hours   =
                        (Natural) (currentTimestamp / 3600) % 24;
                    _timeOfDayString = (TOSTRING(hours, 2)
                                        + TOSTRING(minutes, 2)
                                        + TOSTRING(seconds, 2));

                    if (_timeFractionalDigitCount > 0) {
                        _timeOfDayString +=
                            ("." +
                             TOSTRING(fractionalPart,
                                      _timeFractionalDigitCount));
                    }
                }

                return _timeOfDayString;
            }

    };
}

/*====================*/

using SoXPlugins::BaseTypes::_LoggingTime;

/*--------------------*/
/* PRIVATE FEATURES   */
/*--------------------*/

/** the name prefix to be ignored in function names */
static String _ignoredFunctionNamePrefix = "";

/*--------------------*/

/** length of identification prefix for trace messages */
static const Natural _prefixLength = 2;

/*--------------------*/

/** list of allowed two-character prefices in trace template */
static StringList _standardPrefixList =
    StringList::makeBySplit(">>,<<,--", ",");

/*--------------------*/

/** the buffer logs data before log file is opened, otherwise a
    write-through to target file will be done */
static StringList _buffer = StringList();

/*--------------------*/

/** flag to tell whether time is logged or omitted */
static bool _timeIsLogged = false;

/*--------------------*/

/** flag to tell whether target logging file shall be kept open (or
    closed and reopened after each message output) */
static bool _fileIsKeptOpen = false;

/*--------------------*/

/** the file name of the logging file */
static String _fileName = "";

/*--------------------*/

/** the logging target file */
static ofstream _file = ofstream();

/*--------------------*/

/** the current logging time handler */
static _LoggingTime _loggingTime = _LoggingTime();

/*--------------------*/
/*--------------------*/

/**
 * Closes logging file if still open.
 */
static void _closeFileConditionally () {
    if (_file.is_open()) {
        _file.close();
    }
}

/*--------------------*/

/**
 * Calculates function name from <C>functionSignature</C>.
 *
 * @param[in} functionSignature          signature of function as string
 * @param[in} ignoredFunctionNamePrefix  prefix to be stripped from
 *                                       function name
 * @return  function name
 */
static
String _functionNameFromSignature (IN String& functionSignature,
                                   IN String& ignoredFunctionNamePrefix) {
    String functionName = functionSignature;

    // sanitize function name    
    Natural position = functionName.find("(");
    functionName = prefix(functionName, position);
    replace(functionName, " *__cdecl ", "::");
    replace(functionName, "*", "");
    position = functionName.rfind(" ");
    functionName = substring(functionName, position + 1);
    replace(functionName, "::", ".");

    const bool hasPrefix =
        (ignoredFunctionNamePrefix > ""
         && startsWith(functionName, ignoredFunctionNamePrefix));

    if (hasPrefix) {
        const Natural prefixLength = ignoredFunctionNamePrefix.size();
        functionName = substring(functionName, prefixLength);
    }

    return functionName;
}

/*--------------------*/

/**
 * Creates or reopens logging file depending on value of
 * <C>isNew</C>.
 *
 * @param[in] isNew  flag to tell whether file must be created
 */
static void _openOrCreateFile (IN bool isNew) {
    if (_fileName == "" || _fileName == "STDERR") {
        _closeFileConditionally();
    } else {
        ios_base::openmode mode = ios_base::out;
        mode = (isNew ? mode : mode | ios_base::app);
        _file.open(_fileName, mode);
    }
}

/*--------------------*/

/**
 * Returns name of temporary directory as string.
 *
 * @return  name of temporary directory
 */
static String _temporaryDirectoryName ()
{
    char* environmentPath = (char*) getenv("tmp");
    environmentPath = (environmentPath != nullptr ? environmentPath
                       : (char*) getenv("temp"));
    environmentPath = (environmentPath != nullptr ? environmentPath
                       : (char*) "/tmp");
    String result = String(environmentPath);
    return result;
}

/*--------------------*/

/**
 * Writes all lines in <C>lineList</C> to logging file.
 *
 * @param lineList list of lines to be written to logging file
 */
static void _writeLineListToFile (IN StringList& lineList) {
    for (String line : lineList) {
        _file << line;
    }
}

/*--------------------*/

/**
 * Opens or reopens logging file and writes single line <C>st</C> with
 * a newline appended (depending on internal setting of
 * <C>_fileIsKeptOpen</C>.
 *
 * @param[in] st  string to write (to be followed by newline)
 */
static void _writeLine (IN String& st) {
    const String line = st + "\n";

    if (_fileName == "") {
        // no output file => put line into buffer
        _buffer.append(line);
    } else {
        if (!_fileIsKeptOpen) {
            _openOrCreateFile(false);
        }

        if (!_file.is_open()) {
            // output file cannot be accessed
            // => put line into buffer
            _buffer.append(line);
        } else {
            StringList list = StringList();
            list.append(line);
            _writeLineListToFile(list);
        }

        if (!_fileIsKeptOpen) {
            _closeFileConditionally();
        }
    }
}

/*--------------------*/
/* EXPORTED FEATURES  */
/*--------------------*/

void Logging::initialize ()
{
    _buffer.clear();
    _writeLine("START LOGGING -*- coding: utf-8 -*-");
}

/*--------------------*/

void
Logging::initializeWithDefaults (IN String& fileNameStem,
                                 IN String& ignoredFunctionNamePrefix)
{
    initialize();
    String fileName = fileNameStem + ".log";
    replace(fileName, " ", "");
    const String loggingFilePath =
        expand("%1/%2", _temporaryDirectoryName(), fileName);
    setFileName(loggingFilePath, false);
    setIgnoredFunctionNamePrefix(ignoredFunctionNamePrefix);
    setTracingWithTime(true, 2);
}

/*--------------------*/

void Logging::finalize () {
    _writeLine("END LOGGING");
    _closeFileConditionally();

    if (!_buffer.empty()) {
      // there are still lines in the log
      //TT: _file = std::cerr;
      _writeLineListToFile(_buffer);
    }
}

/*--------------------*/

void Logging::setFileName (IN String& fileName,
                           IN bool isKeptOpen) {
    if (_fileName == fileName) {
        const String message =
            expand("logging file %1 already open => skip", fileName);
        _writeLine(message);
    } else {
        _fileName       = fileName;
        _fileIsKeptOpen = isKeptOpen;
        _openOrCreateFile(true);

        if (!_file.is_open()) {
            _fileName = "";
        } else {
            _writeLineListToFile(_buffer);
            _buffer.clear();
        }

        if (!_fileIsKeptOpen) {
            _closeFileConditionally();
        }
    }
}

/*--------------------*/

void Logging::setIgnoredFunctionNamePrefix (IN String& namePrefix) {
    _ignoredFunctionNamePrefix = namePrefix;
}

/*--------------------*/

void Logging::setTracingWithTime (IN bool timeIsLogged,
                                  IN Natural fractionalDigitCount) {
    _timeIsLogged = timeIsLogged;
    _loggingTime.setFractionalDigitCount(fractionalDigitCount);
}

/*--------------------*/

void Logging::trace (IN String& functionSignature,
                     IN String& message) {
    const String functionName =
        _functionNameFromSignature(functionSignature,
                                   _ignoredFunctionNamePrefix);
    const Natural messageLength =
        (Natural) message.size();
    String newMessage = message;
    //functionName = functionName.replace("<init>", "CONSTRUCTOR");

    const String timeString =
        (!_timeIsLogged
         ? ""
         : " (" + _loggingTime.currentTimeOfDay() + ")");

    if (messageLength < _prefixLength
        || !_standardPrefixList.contains(prefix(message, _prefixLength))) {
      newMessage = ((messageLength > 0 ? "--:" : "--")
                   + message);
    }

    newMessage = (prefix(newMessage, _prefixLength) + functionName
                  + timeString + substring(newMessage, _prefixLength));
    _writeLine(newMessage);
}

/*--------------------*/

void Logging::traceError (IN String& functionSignature,
                          IN String& message) {
    trace(functionSignature, "--: ERROR - " + message);
}
