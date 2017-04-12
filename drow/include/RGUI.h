#ifndef DROW_UI_TYPES_H
#define DROW_UI_TYPES_H
#include "plugin/ui/plugin_ui_types.h"
#include "RColor.h"
#include "RPCH.h"

//toggle dock
enum RToggleDockStyle
{
	TD_TP,
	TD_LT,
	TD_RT,
	TD_BM
};

//进度条模式  
enum RProgressMode
{
	PM_XINC,
	PM_XINC_R,
	PM_XDEC,
	PM_XDEC_R,
	PM_YINC,
	PM_YINC_R,
	PM_YDEC,
	PM_YDEC_R,
};

enum RProgressType
{
	PT_CLIP,
	PT_SCALE,
};

namespace Drow {
class Popup;
class Slot;
class PkgLoadTask;
class MouseProcessor;
class ActionBuilder;
class Animation;
class ControlScale;
class ControlSroll;
class ControlMoveIn;
class ControlMoveOut;
class ControlAlphaIn;
class ControlAlphaOut;
class ControlMove;
class ControlFrameMove;
class ControlFrameScale;
class LabelTimeDuration;
}

class RGUIControlRepo;
class RGUIUnit;
class RGUICaret;
class RGUIControl;
class RGUIAnimation;
class RGUIProgressBar;
class RGUIProgressTimer;
class RGUIPicture;
class RGUIPictureCondition;
class RGUILabel;
class RGUILabelCondition;
class RGUIRichText;
class RGUIButton;
class RGUIToggle;
class RGUISwitch;
class RGUISlider;
class RGUISwiper;
class RGUIRadioBox;
class RGUICheckBox;
class RGUIEditBox;
class RGUIMultiEditBox;
class RGUIScrollable;
class RGUIListBoxBase;
class RGUIListBox;
class RGUIListBoxRow;
class RGUIListBoxCol;
class RGUIListBoxAdv;
class RGUIListBoxAdvItem;
class RGUIComboBoxDropList;
class RGUIComboBox;
class RGUIPanel;
class RGUIScrollPanel;
class RGUITabPage;
class RGUITab;
class RGUIRichLabel;
class RGUIWindow;

#endif
