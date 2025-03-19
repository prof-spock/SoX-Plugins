/**
 * @file
 * The <C>SoXAudioEditorWidget</C> body implements an audio editor
 * widget (a slider or a combo box) in a SoX audio editor for editing
 * a single discrete value parameter.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "Logging.h"
#include "SoXAudioEditor.h"

/*--------------------*/

using SoXPlugins::Helpers::SoXEffectParameterKind;
using SoXPlugins::ViewAndController::SoXAudioEditor;
using SoXPlugins::Helpers::effectParameterKindToString;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/*============================================================*/

/*--------------------*/
/* static data        */
/*--------------------*/

/** the index of the active page */
static Natural _currentPageNumber = 1;

/* the width percentages within a row for the different
 * parts */

/** percentage of full editor width of left margin before
 * label */
static Percentage _leftMarginWidthPercentage = 3.0;

/** percentage of full editor width of label width */
static Percentage _labelWidthPercentage = 20.0;

/** percentage of full editor width of separation
 * between label and widget or widget and value */
static Percentage _separationWidthPercentage = 3.0;

/** percentage of full editor width of widget width */
static Percentage _widgetWidthPercentage = 50.0;

/** percentage of full editor width of value field
 * width */
static Percentage _valueFieldWidthPercentage = 15.0;

/* the height percentages of the top margin, a single row
 * and the separation to the next row */

/** percentage of full editor height for top and
 * bottom margin */
static Percentage _topMarginHeightPercentage = 10.0;

/** percentage of full editor height of height of
 * widget */
static Percentage _rowHeightPercentage = 20.0;

/** percentage of full editor height of gap between
 * widgets */
static Percentage _rowGapHeightPercentage = 5.0;


/*============================================================*/

static void _updateWidgetRange (INOUT juce::Component& widget,
                                IN SoXEffectParameterMap& map,
                                IN String& parameterName);

/*============================================================*/

namespace SoXPlugins::ViewAndController {

    /** tells whether current widget is active because it is on the
     * displayed page or on (always displayed) page 0 */
    #define _widgetIsActive (_pageNumber == _currentPageNumber \
                             || _pageNumber == 0)

    /*--------------------*/

    /**
     * An <C>EventDispatcher</C> is a helper type for catching events from
     * embedded sliders and combo boxes and dispatching them to associated
     * audio editor of this widget.
     */
    struct EventDispatcher :
        public juce::ComboBox::Listener,
        public juce::Slider::Listener
    {

        /**
         * Constructor for event dispatcher for <C>editor</C>.
         *
         * @param editor  target editor of new event dispatcher
         */
        EventDispatcher (SoXAudioEditor* editor)
            : juce::ComboBox::Listener(), juce::Slider::Listener()
        {
            _editor = editor;
        }

        /*--------------------*/

        /**
         * Handles a change in a combo box widget.
         *
         * @param comboBox  combo box that has changed
         */
        void comboBoxChanged (juce::ComboBox* comboBox)
        {
            const String parameterName = comboBox->getName().toStdString();
            const String value = comboBox->getText().toStdString();
            _editor->setValue(parameterName, value);
        }

        /*--------------------*/

        /**
         * Handles a change in a slider widget.
         *
         * @param slider  slider that has changed
         */
        void sliderValueChanged (juce::Slider* slider)
        {
            const String parameterName = slider->getName().toStdString();
            const Real realValue = slider->getValue();
            const Integer intValue{realValue};
            const String value =(realValue == (Real) intValue
                                 ? TOSTRING(intValue)
                                 : TOSTRING(realValue));
            _editor->setValue(parameterName, value);
        }

        /*--------------------*/
        /*--------------------*/

        private:

            /** affected audio editor */
            SoXAudioEditor* _editor;

    };
}

/*============================================================*/

using SoXPlugins::ViewAndController::EventDispatcher;

/*============================================================*/

/** the JUCE notification kind that describes that no notifications
 * should be sent by some widget */
static const juce::NotificationType _noNotification =
    juce::NotificationType::dontSendNotification;

/*--------------------*/
/*--------------------*/

/**
 * Constructs a JUCE widget from <C>map</C> for parameter named
 * <C>parameterName</C> defining either a slider or a combo box;
 * <C>eventDispatcher</C> defines the event handler for the editor.
 *
 * @param[in] map              audio parameter map for this editor
 * @param[in] parameterName    the parameter associated with new widget
 * @param[in] eventDispatcher  the associated event dispatcher for this
 *                             editor
 */
static juce::Component* _makeWidget (IN SoXEffectParameterMap& map,
                                     IN String& parameterName,
                                     EventDispatcher* eventDispatcher)
{
    Logging_trace1(">>: name = %1", parameterName);

    juce::Component* widget;
    const juce::String componentName = juce::String(parameterName);
    const SoXEffectParameterKind kind = map.kind(parameterName);

    /* make widget depending on kind of effect parameter */
    if (kind == SoXEffectParameterKind::unknownKind) {
        juce::Label* label = new juce::Label(componentName);
        widget = label;
    } else if (kind == SoXEffectParameterKind::enumKind) {
        StringList valueList;
        juce::ComboBox* comboBox = new juce::ComboBox(componentName);
        comboBox->addListener(eventDispatcher);
        widget = comboBox;
    } else {
        Real lowValue, highValue, delta;
        juce::Slider* slider = new juce::Slider(componentName);
        slider->setVelocityBasedMode(true);
        slider->addListener(eventDispatcher);
        widget = slider;
    }

    _updateWidgetRange(*widget, map, parameterName);

    Logging_trace("<<");
    return widget;
}

/*--------------------*/

/**
 * Sets bounds of <C>widget</C> to percentages <C>x</C>, <C>y</C>,
 * <C>width</C> and <C>height</C> of frame given by <C>parentWidth</C>
 * and <C>parentHeight</C>.
 *
 * @param[inout] widget        juce widget to be adapted
 * @param[in]    parentWidth   enclosing widget width
 * @param[in]    parentHeight  enclosing widget height
 * @param[in]    x             left margin of widget area as percentage
 *                             of parent widget area
 * @param[in]    y             top margin of widget area as percentage
 *                             of parent widget area
 * @param[in]    width         width of widget area as percentage
 *                             of parent widget area
 * @param[in]    height        height of widget area as percentage
 *                             of parent widget area
 */
static void _setWidgetBounds (INOUT juce::Component* widget,
                              IN Real& parentWidth,
                              IN Real& parentHeight,
                              IN Percentage x,
                              IN Percentage y,
                              IN Percentage width,
                              IN Percentage height)
{
    Logging_trace6(">>: parentSize = (%1, %2),"
                   " position = (%1, %2), size = (%3, %4)",
                   TOSTRING(parentWidth), TOSTRING(parentHeight),
                   TOSTRING(x), TOSTRING(y),
                   TOSTRING(width), TOSTRING(height));

    widget->setBounds((int) Real::round(x.of(parentWidth)),
                      (int) Real::round(y.of(parentHeight)),
                      (int) Real::round(width.of(parentWidth)),
                      (int) Real::round(height.of(parentHeight)));

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Updates range of JUCE <C>widget</C> from <C>map</C> for parameter
 * named <C>parameterName</C> referring to either a slider or a combo
 * box
 *
 * @param[inout] widget         juce widget to be adapted
 * @param[in]    map            audio parameter map for this editor
 * @param[in]    parameterName  the parameter associated with new widget
 */
static void _updateWidgetRange (INOUT juce::Component& widget,
                                IN SoXEffectParameterMap& map,
                                IN String& parameterName)
{
    Logging_trace1(">>: name = %1", parameterName);

    const String currentValue = map.value(parameterName);
    const SoXEffectParameterKind kind = map.kind(parameterName);

    if (kind == SoXEffectParameterKind::unknownKind) {
        const juce::String lText("????");
        juce::Label& label = dynamic_cast<juce::Label&>(widget);
        label.setText(lText, _noNotification);
    } else if (kind == SoXEffectParameterKind::enumKind) {
        StringList valueList;
        map.valueRangeEnum(parameterName, valueList);
        juce::ComboBox& comboBox = dynamic_cast<juce::ComboBox&>(widget);
        comboBox.clear(_noNotification);
        Natural selectedId = 0;

        for (Natural i = 1;  i <= valueList.size();  i++) {
            const String st = valueList[i - 1];
            const juce::String itemText(st);
            comboBox.addItem(itemText, (int) i);
            selectedId = (st == currentValue ? i : selectedId);
        }

        comboBox.setSelectedId((int) selectedId, _noNotification);
    } else {
        Real lowValue, highValue, delta;

        if (kind == SoXEffectParameterKind::realKind) {
            map.valueRangeReal(parameterName,
                               lowValue, highValue, delta);
        } else {
            Integer l, h, d;
            map.valueRangeInt(parameterName, l, h, d);
            lowValue  = (Real) l;
            highValue = (Real) h;
            delta     = (Real) d;
        }

        juce::Slider& slider = dynamic_cast<juce::Slider&>(widget);
        slider.setRange((double) lowValue, (double) highValue,
                        (double) delta);
        slider.setValue((double) STR::toReal(currentValue),
                        _noNotification);
    }

    Logging_trace("<<");
}

/*============================================================*/

/*--------------------*/
/* setup              */    
/*--------------------*/

SoXAudioEditorWidget::SoXAudioEditorWidget (INOUT juce::Component* parent,
                                            IN SoXEffectParameterMap& map,
                                            IN String& parameterName,
                                            IN String& labelName)
    : _parameterName(parameterName),
      _labelWidget(new juce::Label()),
      _parameterMap(map)
{
    Logging_trace2(">>: name = %1, label = %2", parameterName, labelName);

    const juce::String lText(labelName);
    SoXAudioEditor* editor = dynamic_cast<SoXAudioEditor*>(parent);
    EventDispatcher* eventDispatcher = new EventDispatcher(editor);

    _controlWidget   = _makeWidget(map, parameterName, eventDispatcher);
    _eventDispatcher = eventDispatcher;
    _labelWidget->setText(lText, _noNotification);

    /* add real widgets to parent */
    parent->addChildComponent(_labelWidget);
    parent->addChildComponent(_controlWidget);
    Logging_trace("<<");
}

/*--------------------*/

SoXAudioEditorWidget::~SoXAudioEditorWidget ()
{
    Logging_trace(">>");
    delete _labelWidget;
    delete _controlWidget;
    EventDispatcher* eventDispatcher =
      (EventDispatcher*) _eventDispatcher;
    delete eventDispatcher;
    Logging_trace("<<");
}

/*--------------------*/
/* property access    */
/*--------------------*/

String SoXAudioEditorWidget::parameterName () const
{
    return _parameterName;
}

/*--------------------*/
/* event processing   */
/*--------------------*/

void SoXAudioEditorWidget::setValue (IN String& value)
{
    Logging_trace1(">>: %1", value);

    if (_parameterMap.isAllowedValue(_parameterName, value)) {
        juce::Slider* slider =
            dynamic_cast<juce::Slider*>(_controlWidget);
        juce::ComboBox* comboBox =
            dynamic_cast<juce::ComboBox*>(_controlWidget);

        if (slider != nullptr) {
            slider->setValue((double) STR::toReal(value),
                             _noNotification);
        } else if (comboBox != nullptr) {
            StringList valueList;
            _parameterMap.valueRangeEnum(_parameterName, valueList);
            comboBox->setSelectedItemIndex((int) valueList.position(value),
                                           _noNotification);
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEditorWidget::resized ()
{
    Logging_trace(">>");

    /* only resize active widgets */
    if (_widgetIsActive) {
        const Real parentWidth{(double) _controlWidget->getParentWidth()};
        const Real parentHeight{(double) _controlWidget->getParentHeight()};
        Logging_trace2("--: parentWidth = %1, parentHeight = %2",
                       TOSTRING(parentWidth), TOSTRING(parentHeight));

        /* find row position and height */
        Percentage width;
        Percentage x = 0.0;
        Percentage y;

        y = ((_rowHeightPercentage + _rowGapHeightPercentage) * _rowNumber
             + _topMarginHeightPercentage);

        /* label */
        x += _leftMarginWidthPercentage;
        _setWidgetBounds(_labelWidget, parentWidth, parentHeight,
                         x, y, _labelWidthPercentage, _rowHeightPercentage);

        /* control widget */
        x += _labelWidthPercentage + _separationWidthPercentage;
        width = (_separationWidthPercentage
                 + _widgetWidthPercentage
                 + _valueFieldWidthPercentage);
        _setWidgetBounds(_controlWidget, parentWidth, parentHeight,
                         x, y, width, _rowHeightPercentage);

        /* when it is a slider, also adapt text box */
        juce::Slider* slider = dynamic_cast<juce::Slider*>(_controlWidget);

        if (slider != NULL) {
            Integer boxWidth =
                (Integer) Real::round(_valueFieldWidthPercentage
                                      .of(parentWidth));
            Integer boxHeight =
                (Integer) Real::round(_rowHeightPercentage.of(parentHeight));
            slider->setTextBoxStyle(juce::Slider::TextBoxRight,
                                    false, (int) boxWidth, (int) boxHeight);
            Logging_trace2("--: sliderBoxSize = (%1, %2)",
                           TOSTRING(_valueFieldWidthPercentage),
                           TOSTRING(_rowHeightPercentage));
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEditorWidget::updateRange ()
{
    Logging_trace(">>");
    _updateWidgetRange(*_controlWidget, _parameterMap, _parameterName);
    Logging_trace("<<");
}

/*--------------------*/
/* positioning/paging */
/*--------------------*/

void
SoXAudioEditorWidget::handlePageChange (IN Natural pageNumber)
{
    Logging_trace1(">>: %1", TOSTRING(pageNumber));
    _currentPageNumber = pageNumber;
    const Boolean isVisible = _widgetIsActive;
    _controlWidget->setVisible(isVisible);
    _labelWidget->setVisible(isVisible);
    /* redraw widgets upon page change */
    resized();
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEditorWidget::setPageAndRow (IN Natural pageNumber,
                                          IN Natural rowNumber)
{
    Logging_trace2(">>: page = %1, row = %2",
                   TOSTRING(pageNumber), TOSTRING(rowNumber));
    _pageNumber = pageNumber;
    _rowNumber  = rowNumber;
    Logging_trace("<<");
}

/*--------------------*/
/* layout parameters  */
/*--------------------*/

void SoXAudioEditorWidget::partHeightsInPage
                (OUT Percentage& topMarginHeightPercentage,
                 OUT Percentage& rowHeightPercentage,
                 OUT Percentage& rowGapHeightPercentage)
{
    Logging_trace(">>");

    topMarginHeightPercentage = _topMarginHeightPercentage;
    rowHeightPercentage       = _rowHeightPercentage;
    rowGapHeightPercentage    = _rowGapHeightPercentage;
    
    Logging_trace3("<<: marginHeight = %1%, rowHeight = %2%, rowGap = %3%",
                   TOSTRING(topMarginHeightPercentage),
                   TOSTRING(rowHeightPercentage),
                   TOSTRING(rowGapHeightPercentage));
}

/*--------------------*/

void SoXAudioEditorWidget::setPartWidthsInRow
                               (IN Percentage leftMarginWidthPercentage,
                                IN Percentage labelWidthPercentage,
                                IN Percentage separationWidthPercentage,
                                IN Percentage widgetWidthPercentage,
                                IN Percentage valueFieldWidthPercentage)
{
    Logging_trace5(">>: leftMargin = %1%, labelWidth = %2%, sepWidth = %3%,"
                   " widgetWidth = %4%, valueFieldWidth = %5%",
                   TOSTRING(leftMarginWidthPercentage),
                   TOSTRING(labelWidthPercentage),
                   TOSTRING(separationWidthPercentage),
                   TOSTRING(widgetWidthPercentage),
                   TOSTRING(valueFieldWidthPercentage));

    _leftMarginWidthPercentage = leftMarginWidthPercentage;
    _labelWidthPercentage      = labelWidthPercentage;
    _separationWidthPercentage = separationWidthPercentage;
    _widgetWidthPercentage     = widgetWidthPercentage;
    _valueFieldWidthPercentage = valueFieldWidthPercentage;

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEditorWidget::setPartHeightsInPage
                               (IN Percentage topMarginHeightPercentage,
                                IN Percentage rowHeightPercentage,
                                IN Percentage rowGapHeightPercentage)
{
    Logging_trace3(">>: marginHeight = %1%, rowHeight = %2%, rowGap = %3%",
                   TOSTRING(topMarginHeightPercentage),
                   TOSTRING(rowHeightPercentage),
                   TOSTRING(rowGapHeightPercentage));

    _topMarginHeightPercentage = topMarginHeightPercentage;
    _rowHeightPercentage       = rowHeightPercentage;
    _rowGapHeightPercentage    = rowGapHeightPercentage;

    Logging_trace("<<");
}
