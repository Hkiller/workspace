#ifndef DROW_RGUI_ANIM_CONTROL_MOUSE_PROCESSOR_INCLEDED
#define DROW_RGUI_ANIM_CONTROL_MOUSE_PROCESSOR_INCLEDED
#include <vector>
#include "cpepp/utils/TypeUtils.hpp"
#include "render/utils/ui_vector_2.h"
#include "RGUI.h"

namespace Drow {

class MouseProcessor {
public:
    virtual void move(plugin_ui_touch_track_t track) = 0;
    virtual void rise(plugin_ui_touch_track_t track) = 0;
    virtual ~MouseProcessor();
};

/*拖拽操作 */
class DragProcessor : public MouseProcessor {
public:
    DragProcessor(RGUIControl & control, plugin_ui_touch_track_t track, ui_vector_2 const & arg);
    
    virtual void move(plugin_ui_touch_track_t track);
    virtual void rise(plugin_ui_touch_track_t track);

protected:
    virtual void doMove(
        plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg) = 0;
    virtual void doRise(
        plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg,
        ui_vector_2 const & speed) = 0;

    RGUIControl & m_control;
    ui_vector_2 m_down_pt;
    ui_vector_2 m_down_arg;
};

template<typename OuterT>
class DragProcessorT : public DragProcessor {
public:
    typedef void (OuterT::*MoveFunT)(
    plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg);
    typedef void (OuterT::*RiseFunT)(
    plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg,
    ui_vector_2 const & speed);

    DragProcessorT(
        OuterT & control, plugin_ui_touch_track_t track, float arg,
        MoveFunT move, RiseFunT rise)
        : DragProcessor(control, track, mk_ui_vector_2(arg, 0.0f))
        , m_move(move)
        , m_rise(rise)
    {
    }
    
    DragProcessorT(
        OuterT & control, plugin_ui_touch_track_t track, ui_vector_2 const & arg,
        MoveFunT move, RiseFunT rise)
        : DragProcessor(control, track, arg)
        , m_move(move)
        , m_rise(rise)
    {
    }

protected:
    virtual void doMove(plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg) {
        OuterT & o = Cpe::Utils::calc_cast<OuterT>(m_control);
        (o.*m_move)(track, down_pt, cur_pt, down_arg);
    }
    
    virtual void doRise(
        plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg,
        ui_vector_2 const & speed)
    {
        OuterT & o = Cpe::Utils::calc_cast<OuterT>(m_control);
        (o.*m_rise)(track, down_pt, cur_pt, down_arg, speed);
    }
    
    MoveFunT m_move;
    RiseFunT m_rise;
};

}

#endif

