/**
 * @file
 * The <C>SoXValueChangeObserver</C> body implements observer
 * objects for value changes.
 *
 * @author Dr. Thomas Tensi
 * @date   2026-08
 */

/*====================*/

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXValueChangeObserver.h"

#include "Logging.h"

/*--------------------*/

using SoXPlugins::Helpers::SoXValueChangeNotification;
using SoXPlugins::Helpers::SoXValueChangeObserver;

/*====================*/

namespace SoXPlugins::Helpers {

    String SoXValueChangeKind_toString (IN SoXValueChangeKind kind)
    {
        String result;

        if (kind == SoXValueChangeKind::globalChange) {
            result = "globalChange";
        } else if (kind == SoXValueChangeKind::parameterChange) {
            result = "parameterChange";
        } else if (kind == SoXValueChangeKind::noChange) {
            result = "noChange";
        } else {
            result = "???";
        }

        return result;
    }
}

/*--------------------*/
/* creation           */
/*--------------------*/

SoXValueChangeObserver::SoXValueChangeObserver
    (Object valueChangeHandler,
     SoXValueChangeNotification notificationProc)
{
    Logging_trace(">>");
    set(valueChangeHandler, notificationProc);
    Logging_trace("<<");
}

/*--------------------*/
/* property change    */
/*--------------------*/

void
SoXValueChangeObserver::set (Object valueChangeHandler,
                             SoXValueChangeNotification notificationProc)
{
    Logging_trace(">>");
    _valueChangeHandler = valueChangeHandler;
    _notificationProc   = notificationProc;
    Logging_trace("<<");
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void
SoXValueChangeObserver::notifyAboutValueChange (IN Dictionary& dictionary,
                                                IN SoXValueChangeKind kind)
{
    Logging_trace2(">>: dictionary = %1, kind = %2",
                   dictionary.toString(),
                   SoXValueChangeKind_toString(kind));

    if (_valueChangeHandler != nullptr
        && _notificationProc != nullptr) {
        (*_notificationProc)(_valueChangeHandler, dictionary, kind);
    }

    Logging_trace("<<");
}
