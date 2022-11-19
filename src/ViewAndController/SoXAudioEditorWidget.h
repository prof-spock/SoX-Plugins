/**
 * @file
 * The <C>SoXAudioEditorWidget</C> specification defines an audio
 * editor widget (a slider or a combo box) in a SoX audio editor for
 * editing a single discrete value parameter.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "JuceHeaders.h"

#include "Object.h"
#include "Percentage.h"
#include "SoXEffectParameterMap.h"

/*--------------------*/

using BaseTypes::Primitives::Object;
using BaseTypes::Primitives::Percentage;
using SoXPlugins::Helpers::SoXEffectParameterMap;

/*====================*/

namespace SoXPlugins::ViewAndController {

    /**
     * A <C>SoXAudioEditorWidget</C> object models an audio editor
     * widget (a slider or a combo box) in a SoX audio editor for
     * editing a single discrete value parameter; within the editor
     * the widgets are layout out in a vertical manner as rows; note
     * that there is no subclassing for comboboxex or sliders: all
     * processing is done in a single class.
    */
    struct SoXAudioEditorWidget {

        /**
         * Creates a new slider or combobox widget for audio
         * parameter map <C>map</C> representing parameter named
         * <C>parameterName</C>; <C>parent</C> tells the
         * associated audio editor for this widget, the kind of
         * parameter defines whether it will be represented as a
         * combobox (for enums) or slider (for ints and real
         * values); <C>labelName</C> tells how the parameter will
         * be labelled in the GUI
         *
         * @param[inout] parent       JUCE component enclosing this
         *                            widget
         * @param[in] map             parameter map from audio
         *                            processor containing this
         *                            parameter
         * @param[in] parameterName   name of parameter represented by
         *                            this widget
         * @param[in] labelName       name of label to be used by
         *                            widget for this parameter
         */
        SoXAudioEditorWidget (INOUT juce::Component* parent,
                              IN SoXEffectParameterMap& map,
                              IN String& parameterName,
                              IN String& labelName);

        /*--------------------*/

        ~SoXAudioEditorWidget ();

        /*--------------------*/
        /* property access    */
        /*--------------------*/

        /**
         * Returns the associated parameter name of this widget.
         *
         * @return parameter name represented by this widget
         */
        String parameterName () const;

        /*--------------------*/
        /* event processing   */
        /*--------------------*/

        /**
         * Changes value of widget to <C>value</C> (if acceptable
         * for this widget); if not acceptable, nothing will happen
         *
         * @param[in] value  new value for widget
         */
        void setValue (IN String value);

        /*--------------------*/

        /**
         * Tells that this widget is resized; triggers automated
         * calculation of position and size of embedded widgets
         */
        void resized ();

        /*--------------------*/
        /* positioning/paging */
        /*--------------------*/

        /**
         * Tells that page with <C>pageNumber</C> is active (where
         * count starts at 1); the widget can decide whether it is
         * visible or not.
         *
         * @param pageNumber index of active page (starting at 1)
         */
        void handlePageChange (IN Natural pageNumber);

        /*--------------------*/

        /**
         * Sets the page and vertical position indices of this
         * widget to <C>pageNumber</C> and <C>rowNumber</C>.
         *
         * @param pageNumber  page of widget (where 0 means: it is
         *                    visible on all pages
         * @param rowNumber   position of widget on page (starting with
         *                    zero)
         */
        void setPageAndRow (IN Natural pageNumber,
                            IN Natural rowNumber);

        /*--------------------*/
        /* layout parameters  */
        /*--------------------*/

        /**
         * Gives percentages of the full row width used for left
         * gap, the label, the separation, the widget and the
         * value field (as <C>leftGapPercentage</C>,
         * <C>labelWidthPercentage</C>,
         * <C>separationPercentage</C>,
         * <C>widgetWidthPercentage</C> and
         * <C>valueFieldPercentage</C>); ideally
         * (2*<C>leftGapPercentage</C> +
         * <C>labelWidthPercentage</C> +
         * 2*<C>separationPercentage</C> +
         * <C>widgetWidthPercentage</C> +
         * <C>valueFieldPercentage</C>) == 100%
         *
         * @param[in] leftGapPercentage      percentage of width of left
         *                                   gap before label
         * @param[in] labelWidthPercentage   percentage of width of
         *                                   label width
         * @param[in] separationPercentage   percentage of width of
         *                                   separation between label
         *                                   and widget or widget and
         *                                   value
         * @param[in] widgetWidthPercentage  percentage of width of
         *                                   widget width
         * @param[in] valueFieldPercentage   percentage of width of
         *                                   value field width
         */
        static void setPartWidthsInRow
                        (IN Percentage leftGapPercentage,
                         IN Percentage labelWidthPercentage,
                         IN Percentage separationPercentage,
                         IN Percentage widgetWidthPercentage,
                         IN Percentage valueFieldPercentage);

        /*--------------------*/

        /**
         * Gives percentages of the full editor height used for
         * top gap, a single row and the distance between rows (as
         * <C>topGap</C>, <C>rowHeight</C> and <C>rowGap</C>).
         *
         * @param[in] topGapPercentage     percentage of height for top
         *                                 and bottom gap
         * @param[in] rowHeightPercentage  percentage of height of height
         *                                 of widget
         * @param[in] rowGapPercentage     percentage of height of gap
         *                                 between widgets
         */
        static void setPartHeightsInPage
                        (IN Percentage topGapPercentage,
                         IN Percentage rowHeightPercentage,
                         IN Percentage rowGapPercentage);

        /*--------------------*/
        /*--------------------*/

        protected:

            /** name of parameter displayed by this widget */
            String _parameterName;

            /** page position within editor (where zero means:
             * show on all pages) */
            Natural _pageNumber;

            /** row position on page within editor (starting at
             * zero) */
            Natural _rowNumber;

            /** label shown for widget */
            juce::Label*     _labelWidget;

            /** control widget: either combobox or slider */
            juce::Component* _controlWidget;

            /** object responsible for dispatching change events from
             * this widget to enclosing editor */
            Object _eventDispatcher;

            /** the underlying parameter map */
            const SoXEffectParameterMap& _parameterMap;

            /*====================*/
            /* class wide params  */
            /*====================*/

            /** the index of the active page */
            static Natural _currentPageNumber;

            /* the width percentages within a row for the different
             * parts */

            /** percentage of full editor width of left gap before
             * label */
            static Percentage _leftGapPercentage;

            /** percentage of full editor width of label width */
            static Percentage _labelWidthPercentage;

            /** percentage of full editor width of separation
             * between label and widget or widget and value */
            static Percentage _separationPercentage;

            /** percentage of full editor width of widget width */
            static Percentage _widgetWidthPercentage;

            /** percentage of full editor width of value field
             * width */
            static Percentage _valueFieldPercentage;

            /* the height percentages of the top gap, a single row
             * and the separation to the next row */

            /** percentage of full editor height for top and
             * bottom gap */
            static Percentage _topGapPercentage;

            /** percentage of full editor height of height of
             * widget */
            static Percentage _rowHeightPercentage;

            /** percentage of full editor height of gap between
             * widgets */
            static Percentage _rowGapPercentage;

    };

}
