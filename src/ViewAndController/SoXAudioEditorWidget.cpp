/**
 * @file
 * The <C>SoXAudioEditorWidget</C> body implements an audio editor
 * widget (a slider or a combo box) in a SoX audio editor for editing
 * a single discrete value parameter.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*====================*/

#include "SoXAudioEditorWidget.h"

#include "Integer.h"
#include "Logging.h"
#include "Real.h"
#include "StringUtil.h"
#include "SoXAudioEditor.h"

/*====================*/

using SoXPlugins::CommonAudio::audioParameterKindToString;

using SoXPlugins::BaseTypes::Primitives::Integer;
using SoXPlugins::BaseTypes::Primitives::Real;
using SoXPlugins::CommonAudio::SoXAudioParameterKind;
using SoXPlugins::ViewAndController::SoXAudioEditor;
using SoXPlugins::ViewAndController::SoXAudioEditorWidget;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;

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
            const Integer intValue{(int) (double) realValue};
            const String value =(realValue == (Real) intValue
                                 ? intValue.toString()
                                 : TOSTRING(realValue));
            _editor->setValue(parameterName, value);
        }

        /*--------------------*/
        /*--------------------*/

        private:

            /** affected audio editor */
            SoXAudioEditor* _editor;

    };

    /*====================*/

    /** the JUCE notification kind that describes that no notifications
     * should be sent by some widget */
    static const juce::NotificationType noNotification =
        juce::NotificationType::dontSendNotification;

    /*--------------------*/
    /*--------------------*/

    /**
     * Constructs a JUCE widget from <C>map</C> for parameter named
     * <C>parameterName</C> with <C>kind</C> defining either a slider or a
     * combo box; <C>eventDispatcher</C> defines the event handler for the
     * editor.
     *
     * @param[in] map              audio parameter map for this editor
     * @param[in] parameterName    the parameter associated with new widget
     * @param[in] kind             the parameter kind of widget
     * @param[in] eventDispatcher  the associated event dispatcher for this
     *                             editor
     */
    static juce::Component* _makeWidget (IN SoXAudioParameterMap& map,
                                         IN String& parameterName,
                                         IN SoXAudioParameterKind& kind,
                                         EventDispatcher* eventDispatcher)
    {
        Logging_trace2(">>: name = %1, kind = %2",
                       parameterName, audioParameterKindToString(kind));

        juce::Component* result;
        const juce::String componentName = juce::String(parameterName);
        const String currentValue = map.value(parameterName);

        if (kind == SoXAudioParameterKind::unknownKind) {
            const juce::String lText("????");
            juce::Label* label = new juce::Label(componentName);
            label->setText(lText, noNotification);
            result = label;
        } else if (kind == SoXAudioParameterKind::enumKind) {
            StringList valueList;
            juce::ComboBox* comboBox = new juce::ComboBox(componentName);
            map.valueRangeEnum(parameterName, valueList);
            Natural selectedId = 0;

            for (Natural i = 1;  i <= valueList.size();  i++) {
                const String st = valueList[i - 1];
                const juce::String itemText(st);
                comboBox->addItem(itemText, (int) i);
                selectedId = (st == currentValue ? i : selectedId);
            }

            comboBox->setSelectedId((int) selectedId, noNotification);
            comboBox->addListener(eventDispatcher);
            result = comboBox;
        } else {
            Real lowValue, highValue, delta;
            juce::Slider* slider = new juce::Slider(componentName);

            if (kind == SoXAudioParameterKind::realKind) {
                map.valueRangeReal(parameterName,
                                   lowValue, highValue, delta);
            } else {
                Integer l, h, d;
                map.valueRangeInt(parameterName, l, h, d);
                lowValue  = (Real) l;
                highValue = (Real) h;
                delta     = (Real) d;
            }

            slider->setRange((double) lowValue, (double) highValue, (double) delta);
            slider->setValue((double) StringUtil::toReal(currentValue),
                             noNotification);
            slider->setVelocityBasedMode(true);
            slider->addListener(eventDispatcher);
            result = slider;
        }

        Logging_trace("<<");
        return result;
    }

}

/*============================================================*/

/*--------------------*/
/* setup              */    
/*--------------------*/

SoXAudioEditorWidget::SoXAudioEditorWidget (INOUT juce::Component* parent,
                                            IN SoXAudioParameterMap& map,
                                            IN String& parameterName,
                                            IN String& labelText)
    : _parameterName(parameterName),
      _labelWidget(new juce::Label()),
      _parameterMap(map)
{
    Logging_trace2(">>: name = %1, label = %2", parameterName, labelText);

    const SoXAudioParameterKind kind = map.kind(parameterName);
    const juce::String lText(labelText);
    SoXAudioEditor* editor = dynamic_cast<SoXAudioEditor*>(parent);
    EventDispatcher* eventDispatcher = new EventDispatcher(editor);

    _controlWidget   = _makeWidget(map, parameterName, kind,
                                   eventDispatcher);
    _eventDispatcher = eventDispatcher;
    _labelWidget->setText(lText, noNotification);

    // add real widgets to parent
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

void SoXAudioEditorWidget::setValue (IN String value)
{
    Logging_trace1(">>: %1", value);

    if (_parameterMap.isAllowedValue(_parameterName, value)) {
        juce::Slider* slider =
            dynamic_cast<juce::Slider*>(_controlWidget);
        juce::ComboBox* comboBox =
            dynamic_cast<juce::ComboBox*>(_controlWidget);

        if (slider != nullptr) {
            slider->setValue((double) StringUtil::toReal(value),
                             noNotification);
        } else if (comboBox != nullptr) {
            StringList valueList;
            _parameterMap.valueRangeEnum(_parameterName, valueList);
            comboBox->setSelectedItemIndex((int) valueList.position(value),
                                           noNotification);
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEditorWidget::resized ()
{
    Logging_trace(">>");

    if (_widgetIsActive) {
        // only resize active widgets
        const Natural parentWidth  =
            _controlWidget->getParentWidth();
        const Natural parentHeight =
            _controlWidget->getParentHeight();
        Logging_trace2("--: parentWidth = %1, parentHeight = %2",
                       TOSTRING(parentWidth), TOSTRING(parentHeight));

        // find row position and height
        const Natural topGapHeight = _topGapPercentage.of(parentHeight);
        const Natural rowGapHeight = _rowGapPercentage.of(parentHeight);
        const Natural height = _rowHeightPercentage.of(parentHeight);

        const Natural labelWidth = _labelWidthPercentage.of(parentWidth);
        const Natural leftGapWidth =
            _leftGapPercentage.of(parentWidth);
        const Natural separatorWidth =
            _separationPercentage.of(parentWidth);
        const Natural controlWidgetWidth =
            _widgetWidthPercentage.of(parentWidth);
        const Natural valueFieldWidth =
            _valueFieldPercentage.of(parentWidth);

        Natural width;
        Natural x = 0;
        Natural y;

        y = ((height + rowGapHeight) * _rowNumber + topGapHeight);

        // label
        x += leftGapWidth;
        _labelWidget->setBounds((int) x, (int) y,
                                (int) labelWidth, (int) height);
        Logging_trace4("--: labelPos = (%1, %2), labelSize = (%3, %4)",
                       TOSTRING(x), TOSTRING(y),
                       TOSTRING(labelWidth), TOSTRING(height));

        // control widget
        x += labelWidth + separatorWidth;
        width = separatorWidth + controlWidgetWidth + valueFieldWidth;
        _controlWidget->setBounds((int) x, (int) y,
                                  (int) width, (int) height);
        Logging_trace4("--: controlPos = (%1, %2), controlSize = (%3, %4)",
                       TOSTRING(x), TOSTRING(y),
                       TOSTRING(labelWidth), TOSTRING(height));
        juce::Slider* slider = dynamic_cast<juce::Slider*>(_controlWidget);

        if (slider != NULL) {
            slider->setTextBoxStyle(juce::Slider::TextBoxRight,
                                    false,
                                    (int) valueFieldWidth, (int) height);
            Logging_trace2("--: sliderBoxSize = (%1, %2)",
                           TOSTRING(valueFieldWidth), TOSTRING(height));
        }
    }

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
    const bool isVisible = _widgetIsActive;
    _controlWidget->setVisible(isVisible);
    _labelWidget->setVisible(isVisible);
    // redraw widgets upon page change
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

void SoXAudioEditorWidget::setPartWidthsInRow
                               (IN Percentage leftGapPercentage,
                                IN Percentage labelWidthPercentage,
                                IN Percentage separationPercentage,
                                IN Percentage widgetWidthPercentage,
                                IN Percentage valueFieldPercentage)
{
    Logging_trace5(">>: leftGap = %1%, labelWidth = %2%, sepWidth = %3%,"
                   " widgetWidth = %4%, valueFieldWidth = %5%",
                   TOSTRING(leftGapPercentage),
                   TOSTRING(labelWidthPercentage),
                   TOSTRING(separationPercentage),
                   TOSTRING(widgetWidthPercentage),
                   TOSTRING(valueFieldPercentage));

    _leftGapPercentage     = leftGapPercentage;
    _labelWidthPercentage  = labelWidthPercentage;
    _separationPercentage  = separationPercentage;
    _widgetWidthPercentage = widgetWidthPercentage;
    _valueFieldPercentage  = valueFieldPercentage;

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEditorWidget::setPartHeightsInPage
                               (IN Percentage topGapPercentage,
                                IN Percentage rowHeightPercentage,
                                IN Percentage rowGapPercentage)
{
    Logging_trace3(">>: topGap = %1%, rowHeight = %2%, rowGap = %3%",
                   TOSTRING(topGapPercentage),
                   TOSTRING(rowHeightPercentage),
                   TOSTRING(rowGapPercentage));

    _topGapPercentage    = topGapPercentage;
    _rowHeightPercentage = rowHeightPercentage;
    _rowGapPercentage    = rowGapPercentage;

    Logging_trace("<<");
}

/*--------------------*/
/* static data        */
/*--------------------*/

Natural SoXAudioEditorWidget::_currentPageNumber = 1;

// initialize to meaningful values (to be overwritten by
// plugins)
Percentage SoXAudioEditorWidget::_leftGapPercentage     =  3.0;
Percentage SoXAudioEditorWidget::_labelWidthPercentage  = 20.0;
Percentage SoXAudioEditorWidget::_separationPercentage  =  3.0;
Percentage SoXAudioEditorWidget::_widgetWidthPercentage = 50.0;
Percentage SoXAudioEditorWidget::_valueFieldPercentage  = 15.0;

Percentage SoXAudioEditorWidget::_topGapPercentage      = 10.0;
Percentage SoXAudioEditorWidget::_rowHeightPercentage   = 20.0;
Percentage SoXAudioEditorWidget::_rowGapPercentage      =  5.0;
