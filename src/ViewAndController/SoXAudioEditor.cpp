/**
 * @file
 * The <C>SoXAudioEditor</C> body implements the audio editor
 * for a plugin, represented by a display window containing the
 * parameters.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*====================*/

#include "SoXAudioEditor.h"

#include <map>
#include "Integer.h"
#include "Logging.h"
#include "NaturalList.h"
#include "StringSet.h"
#include "StringUtil.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Integer;
using SoXPlugins::BaseTypes::Containers::NaturalList;
using SoXPlugins::BaseTypes::Containers::StringSet;
using SoXPlugins::ViewAndController::SoXAudioEditor;
using SoXPlugins::ViewAndController::SoXAudioEditorWidgetPtrList;
using SoXPlugins::ViewAndController::SoXAudioProcessor;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;
using StringUtil::toNatural;

/*====================*/

// standard colors
/** black color */
static const juce::Colour _black(0, 0, 0);
/** blue color */
static const juce::Colour _blue(0, 0, 255);
/** green color */
static const juce::Colour _green(0, 255, 0);
/** cyan color */
static const juce::Colour _cyan(0, 255, 0);
/** red color */
static const juce::Colour _red(255, 0, 0);
/** magenta color */
static const juce::Colour _magenta(255, 0, 255);
/** yellow color */
static const juce::Colour _yellow(255, 255, 0);
/** white color */
static const juce::Colour _white(255, 255, 255);
/** fully transparent color */
static const juce::Colour _transparent = juce::Colour();

// levels of grey
/** grey colour with  1/16th blackness */
static const juce::Colour _grey1(240, 240, 240);
/** grey colour with  2/16th blackness */
static const juce::Colour _grey2(224, 224, 224);
/** grey colour with  3/16th blackness */
static const juce::Colour _grey3(208, 208, 208);
/** grey colour with  4/16th blackness */
static const juce::Colour _grey4(192, 192, 192);
/** grey colour with  5/16th blackness */
static const juce::Colour _grey5(176, 176, 176);
/** grey colour with  6/16th blackness */
static const juce::Colour _grey6(160, 160, 160);
/** grey colour with  7/16th blackness */
static const juce::Colour _grey7(144, 144, 144);
/** grey colour with  8/16th blackness */
static const juce::Colour _grey8(128, 128, 128);
/** grey colour with  9/16th blackness */
static const juce::Colour _grey9(112, 112, 112);
/** grey colour with 10/16th blackness */
static const juce::Colour _greyA( 96,  96,  96);
/** grey colour with 11/16th blackness */
static const juce::Colour _greyB( 80,  80,  80);
/** grey colour with 12/16th blackness */
static const juce::Colour _greyC( 64,  64,  64);
/** grey colour with 13/16th blackness */
static const juce::Colour _greyD( 48,  48,  48);
/** grey colour with 14/16th blackness */
static const juce::Colour _greyE( 32,  32,  32);
/** grey colour with 15/16th blackness */
static const juce::Colour _greyF( 16,  16,  16);

// special colors
/** a red colour with 10/16th blackness */
static const juce::Colour _redA(96, 64, 64);
/** a grey colour with 5/16th blackness and half transparent */
static const juce::Colour _semiGrey5 = _grey5.withAlpha(0.5f);

// logical colors
/** the default editor background color */
static const juce::Colour _defaultBackgroundColor = _grey2;
/** the editor background color for the active page */
static const juce::Colour _activePageColor        = _grey4;
/** the editor background color for the inactive page */
static const juce::Colour _inactivePageColor      = _redA;
/** a dimmed down background color */
static const juce::Colour _dimmedBackgroundColor  = _grey8;

/** the default width of an SoXPlugins editor in pixels */
static const Natural _defaultWidth    = 500;
/** the default height of an SoXPlugin widget in pixels */
static const Natural _heightPerWidget =  40;

/*--------------------*/
/* auxiliary routines */
/*--------------------*/

/**
 * Resets the widget list
 *
 * @param[inout] widgetList  the list of widgets in an editor
 */
static
void _clearWidgetList (INOUT SoXAudioEditorWidgetPtrList& widgetList)
{
    Logging_trace(">>");

    for (Natural i = 0;  i < widgetList.size();  i++) {
        delete widgetList[i];
    }

    widgetList.clear();

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Adds widgets to <C>widgetList</C> for each parameter in
 * <C>parameterMap</C>; also takes into account the page setting of
 * each widget; returns pair of fixed widget count on page and maximum
 * widget count on a page
 *
 * @param[inout] currentEditor  audio editor to be changed
 * @param[inout] widgetList     list of associated editor widgets
 * @return  pair of fixed widget count on each page and maximum widget
 *          count on a page
 */
static NaturalList
_addWidgetsToList (INOUT SoXAudioEditor* currentEditor,
                   INOUT SoXAudioEditorWidgetPtrList& widgetList)
{
    Logging_trace(">>");

    const SoXAudioParameterMap& parameterMap =
        currentEditor->audioParameterMap();
    const StringList parameterNameList =
        parameterMap.parameterNameList();

    // find fixed count of widgets
    Natural fixedWidgetCount = 0;

    for (const String& parameterName : parameterNameList) {
        Natural pageNumber;
        String labelName;
        SoXAudioParameterMap::splitParameterName(parameterName,
                                                 labelName, pageNumber);
        fixedWidgetCount += (pageNumber == 0 ? 1 : 0);
    }

    // generate widgets
    Natural maximumWidgetCountOnPage = fixedWidgetCount;
    NaturalList pageNumberToWidgetCountMap;
    Natural pageCount = 0;
    _clearWidgetList(widgetList);
    Logging_trace1("--: fixedWidgetCount = %1", TOSTRING(fixedWidgetCount));

    for (const String& parameterName : parameterNameList) {
        Natural pageNumber;
        String labelName;
        SoXAudioParameterMap::splitParameterName(parameterName,
                                                 labelName, pageNumber);

        if (pageNumber >= pageCount) {
            pageCount = pageNumber + 1;
            pageNumberToWidgetCountMap.setLength(pageCount, Natural{0});
        }

        SoXAudioEditorWidget* widget =
            new SoXAudioEditorWidget(currentEditor, parameterMap,
                                     parameterName, labelName);
        widgetList.append(widget);
        const Natural positionInPage =
            (pageNumberToWidgetCountMap[pageNumber]++
             + (pageNumber > 0 ? fixedWidgetCount : Natural{0}));
        widget->setPageAndRow(pageNumber, positionInPage);
        maximumWidgetCountOnPage =
            Natural::maximum(positionInPage + 1, maximumWidgetCountOnPage);
    }

    NaturalList result = NaturalList::fromList({fixedWidgetCount,
                                                maximumWidgetCountOnPage});

    Logging_trace1("<<: %1", result.toString());
    return result;
}

/*--------------------*/

/**
 * Sets the generic look and feel for the SoX audio editors.
 *
 * @param[inout] lookAndFeel  the JUCE look and feel object for this
 *                            editor
 */
static void _changeLookAndFeel (INOUT juce::LookAndFeel& lookAndFeel)
{
    Logging_trace(">>");

    lookAndFeel.setDefaultSansSerifTypefaceName("Arial");

    // combo boxes
    lookAndFeel.setColour(juce::ComboBox::backgroundColourId,
                          _grey5);
    lookAndFeel.setColour(juce::ComboBox::arrowColourId, _black);
    lookAndFeel.setColour(juce::ComboBox::buttonColourId, _green);
    lookAndFeel.setColour(juce::ComboBox::textColourId, _black);

    // labels
    /// lookAndFeel.setColour(juce::Label::backgroundColourId, _grey5);
    lookAndFeel.setColour(juce::Label::backgroundColourId, _transparent);
    lookAndFeel.setColour(juce::Label::textColourId, _black);

    // sliders
    lookAndFeel.setColour(juce::Slider::backgroundColourId,
                          _grey5);
    lookAndFeel.setColour(juce::Slider::thumbColourId, _greyC);
    lookAndFeel.setColour(juce::Slider::trackColourId, _grey9);
    lookAndFeel.setColour(juce::Slider::textBoxTextColourId, _black);
    lookAndFeel.setColour(juce::Slider::textBoxBackgroundColourId,
                          _semiGrey5);
    lookAndFeel.setColour(juce::Slider::textBoxOutlineColourId,
                          _dimmedBackgroundColor);

    // text entry
    lookAndFeel.setColour(juce::TextEditor::highlightColourId,
                          _black);
    lookAndFeel.setColour(juce::TextEditor::highlightedTextColourId,
                          _white);
    Logging_trace("<<");
}

/*--------------------*/

/**
 * Finds values for page count and page index (if available) from
 * parameter map and returns them as a pair.
 *
 * @param[inout] currentEditor  audio editor to be changed
 * @return  pair of page count and page index
 */
static NaturalList _findPageIndices (INOUT SoXAudioEditor* currentEditor)
{
    Logging_trace(">>");

    const SoXAudioParameterMap& parameterMap =
        currentEditor->audioParameterMap();
    const StringList parameterNameList =
        parameterMap.parameterNameList();

    Natural pageIndex = 0;
    Natural pageCount = 0;

    for (const String& parameterName : parameterNameList) {
        Natural pageNumber;
        String labelName;
        Integer nominalPageNumber;
        SoXAudioParameterMap::splitParameterName(parameterName,
                                                 labelName, pageNumber,
                                                 nominalPageNumber);
        if (nominalPageNumber < 0) {
            const String value = parameterMap.value(parameterName);
            Logging_trace2("--: nominalPageNumber = %1, value = %2",
                           nominalPageNumber.toString(), value);
            const Natural v =
                Natural::maximum(1, toNatural(value, 1));

            if (nominalPageNumber == -1) {
                pageIndex = v;
            } else {
                pageCount = v;
            }
        }
    }

    const NaturalList result = NaturalList::fromList({pageCount, pageIndex});
    Logging_trace1("<<: %1", result.toString());
    return result;
}

/*--------------------*/

/**
 * Tells each widget in <C>widgetList</C> about new page
 * <C>currentEditorPage</C>.
 *
 * @param[in] widgetList  widget list of current editor
 * @param[in] pageIndex   current editor page
 */
static void
_notifyWidgetsAboutPageChange (INOUT SoXAudioEditorWidgetPtrList& widgetList,
                               IN Natural pageIndex)
{
    Logging_trace1(">>: page = %1", TOSTRING(pageIndex));

    for (SoXAudioEditorWidget* widget : widgetList) {
        widget->handlePageChange(pageIndex);
    }

    Logging_trace("<<");
}

/*--------------------*/
/* EXPORTED FEATURES  */
/*--------------------*/

SoXAudioEditor::SoXAudioEditor (INOUT SoXAudioProcessor& processor)
    : juce::AudioProcessorEditor(&processor),
      _processor(processor),
      _currentEditorPageIndex(1),
      _lastEditorPageIndex(1),
      _fixedWidgetPercentage(Percentage{100.0})
{
    Logging_trace(">>");

    _changeLookAndFeel(getLookAndFeel());

    // define some default size
    setSize(50, 50);

    // default width distribution in a widget row
    SoXAudioEditorWidget::setPartWidthsInRow(Percentage{3},
                                             Percentage{20},
                                             Percentage{3},
                                             Percentage{50},
                                             Percentage{15});

    // default height distribution in an editor page
    SoXAudioEditorWidget::setPartHeightsInPage(Percentage{10},
                                               Percentage{20},
                                               Percentage{10});

    // initialize the page indices
    NaturalList pageIndices = _findPageIndices(this);
    _lastEditorPageIndex    = pageIndices[0];
    _currentEditorPageIndex = pageIndices[1];

    // register at audio processor for change notification
    _processor.registerObserver(this);
    _resetAppearance();

    Logging_trace("<<");
}

/*--------------------*/

SoXAudioEditor::~SoXAudioEditor ()
{
    Logging_trace(">>");
    _processor.unregisterObserver(this);
    _clearWidgetList(_widgetList);
    Logging_trace("<<");
}

/*--------------------*/
/* graphics events    */
/*--------------------*/

void SoXAudioEditor::paint (juce::Graphics& graphics)
{
    Logging_trace(">>");

    juce::Rectangle<int> rectangle = getLocalBounds();
    const Integer height{rectangle.getHeight()};
    const Integer y =
        Integer::floor((double) _fixedWidgetPercentage.of(height));
    Logging_trace2("--: height = %1, y = %2",
                   height.toString(), y.toString());

    // fill fixed part with default background color
    rectangle.setHeight((int) y);
    graphics.setColour(_defaultBackgroundColor);
    graphics.fillRect(rectangle);

    // fill page part with color depending on activeness of page
    const bool isActivePage =
        (_currentEditorPageIndex <= _lastEditorPageIndex);
    const juce::Colour pageColor = (isActivePage
                                    ? _activePageColor
                                    : _inactivePageColor);
    rectangle.setY((int) y);
    rectangle.setHeight((int) (height - y));
    graphics.setColour(pageColor);
    graphics.fillRect(rectangle);

    _notifyWidgetsAboutPageChange(_widgetList, _currentEditorPageIndex);
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEditor::resized ()
{
    Logging_trace(">>");
    for (SoXAudioEditorWidget* widget : _widgetList) {
        widget->resized();
    }
    Logging_trace("<<");
}

/*--------------------*/

const SoXAudioParameterMap& SoXAudioEditor::audioParameterMap () const
{
    return _processor.audioParameterMap();
}

/*--------------------*/

void SoXAudioEditor::setValue (IN String& parameterName,
                               IN String& value)
{
    Logging_trace(">>");
    _processor.setValue(parameterName, value);
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEditor::notifyAboutChange (IN SoXAudioValueChangeKind kind,
                                        IN String& data)
{
    Logging_trace2(">>: kind = %1, data = %2",
                   SoXAudioValueChangeKind_toString(kind), data);
    bool repaintIsNecessary = false;

    if (kind == SoXAudioValueChangeKind::globalChange) {
        _resetAppearance();
    } else if (kind == SoXAudioValueChangeKind::pageChange) {
        repaintIsNecessary = true;
    } else {
        const String parameterName = data;
        const String value = audioParameterMap().value(parameterName);
        Logging_trace2("--: parameterName = %1, value = %2",
                       parameterName, value);

        for (SoXAudioEditorWidget* widget : _widgetList) {
            if (widget->parameterName() == parameterName) {
                widget->setValue(value);
            }
        }

        if (kind == SoXAudioValueChangeKind::pageCountChange) {
            _lastEditorPageIndex = Natural::maximum(1, toNatural(value, 1));
            repaintIsNecessary = true;
        } else if (SoXAudioParameterMap::isPageSelector(parameterName)) {
            _currentEditorPageIndex =
                Natural::maximum(1, toNatural(value, 1));
            repaintIsNecessary = true;
        }
    }

    if (repaintIsNecessary) {
        repaint();
    }

    Logging_trace("<<");
}

/*--------------------*/
/*--------------------*/

void SoXAudioEditor::_resetAppearance ()
{
    Logging_trace(">>");

    NaturalList widgetCountList = _addWidgetsToList(this, _widgetList);
    const Natural fixedWidgetCount = widgetCountList[0];
    const Natural maximumWidgetCount =
        Natural::maximum(1, widgetCountList[1]);
    setSize((int) _defaultWidth,
            (int) (_heightPerWidget * maximumWidgetCount));

    // initialize page setup
    const Real rowSpaceFactor = 0.25;
    const Real borderFactor   = 0.5;
    const Percentage rowHeight =
        100.0 / (((size_t) maximumWidgetCount - 1)
                 * (1 + (double) rowSpaceFactor)
                 + 2 * (double) borderFactor + 1);
    const Percentage topHeight      = Percentage{(double) rowHeight / 2};
    const Percentage rowSpaceHeight = Percentage{(double) topHeight / 2};
    SoXAudioEditorWidget::setPartHeightsInPage(topHeight,
                                               rowHeight,
                                               rowSpaceHeight);
    _fixedWidgetPercentage =
        Percentage{(double) (fixedWidgetCount == maximumWidgetCount
                    ? 100.0
                    : (topHeight
                       + ((double) (rowHeight + rowSpaceHeight)
                          * (double) fixedWidgetCount)
                       - rowSpaceHeight / 2))};

    Logging_trace("<<");
}
