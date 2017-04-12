#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_data_value.h"
#include "plugin/ui/plugin_ui_control_attr_meta.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "RGUIWindow.h"
#include "RGUIProgressBar.h"

RGUIProgressBar::RGUIProgressBar() {
	mType = PT_CLIP;
	mMode = PM_XDEC;
	mSpeed = 1.0f;
    mMoveDuration = 0.0f;
	mCurrProgress = 0.0f;
	mLastProgress = 0.0f;
	mAlphaMove = false;
    mMovedTime = 0.0f;
    mMoveStartProgress = 0.0f;
}

RGUIProgressBar::~RGUIProgressBar() {
}

void RGUIProgressBar::Load( ui_data_control_t control ) {
    RGUILabel::Load(control);

    if (type() == ui_control_type_progress) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUILabel::Load(data.data.progress.text);
		mType = data.data.progress.type;
        mMode = data.data.progress.mode;
        mSpeed = data.data.progress.speed;
        mMoveDuration = 0.0f;
        mCurrProgress = data.data.progress.cur_progress;
        mMoveStartProgress = mCurrProgress;
    }
}

bool RGUIProgressBar::updateProgress(float deltaTime) {
    if (mLastProgress == mCurrProgress) return false;
    
    if (mMoveDuration == 0.0f) { /*by speed*/
        if (isMoveInc()) {
            mLastProgress += mSpeed;
            if (mLastProgress >  mCurrProgress) {
                mLastProgress =  mCurrProgress;
            }
        }
        else {
			mLastProgress -= mSpeed;
			if (mLastProgress <  mCurrProgress) {
				mLastProgress =  mCurrProgress;
            }
        }
    }
    else {
        float percent;
        mMovedTime += deltaTime;
        if (mMovedTime > mMoveDuration) {
            percent = 1.0f;
        }
        else {
            percent = mMovedTime / mMoveDuration;
        }

        mLastProgress = mMoveStartProgress + (mCurrProgress - mMoveStartProgress) * percent;
    }

    if (mLastProgress == mCurrProgress) {
        plugin_ui_control_dispatch_event(
            control(), control(),
            plugin_ui_event_progress_done, plugin_ui_event_dispatch_to_self_and_parent);
    }

    return true;
}

void RGUIProgressBar::UpdateSelf(float deltaTime) {
    if (updateProgress(deltaTime) || frameChanged()) {
        updateFrames();
    }
}

void RGUIProgressBar::updateFrames(void) {
    updateFrames(mLastProgress, 1.0f);
}

void RGUIProgressBar::updateFrames(float progress, float alpha) {
	ui_rect clip_rt = GetRenderRealRTAbs();
    ui_vector_2 offset = UI_VECTOR_2_ZERO;
    ui_vector_2 scale = UI_VECTOR_2_IDENTITY;
    
	switch ((uint8_t)mMode) {
	case PM_XDEC:
	case PM_XINC:
        scale.x = progress;
		break;
	case PM_YDEC:
	case PM_YINC:
        scale.y = progress;
		break;
	case PM_XDEC_R:
	case PM_XINC_R:
        scale.x = progress;
        offset.x += ui_rect_width(&clip_rt) * (1.0f - progress);
		break;
	case PM_YDEC_R:
	case PM_YINC_R:
        scale.y = progress;
        offset.y += ui_rect_height(&clip_rt) * (1.0f - progress);
		break;
	}

	switch ((uint8_t)mType) {
	case PT_CLIP: {
        plugin_ui_control_frame_it frame_it;
        plugin_ui_control_frames_in_layer(control(), &frame_it, plugin_ui_control_frame_layer_back);
        while(plugin_ui_control_frame_t frame = plugin_ui_control_frame_it_next(&frame_it)) {
            plugin_ui_control_frame_set_local_pos(frame, &offset);
            plugin_ui_control_frame_set_scale(frame, &scale);
        }
		break;
    }
	case PT_SCALE: {
        plugin_ui_control_frame_it frame_it;
        plugin_ui_control_frames_in_layer(control(), &frame_it, plugin_ui_control_frame_layer_back);
        while(plugin_ui_control_frame_t frame = plugin_ui_control_frame_it_next(&frame_it)) {
            plugin_ui_control_frame_set_local_pos(frame, &offset);
            plugin_ui_control_frame_set_scale(frame, &scale);
        }
        break;
		}
	}
}

void RGUIProgressBar::SetProgress(float value) {
    cpe_assert_float_sane(value);
    
	if (mCurrProgress != value) {
        mMoveStartProgress = mCurrProgress;
        mMovedTime = 0.0f;
        mCurrProgress = value;
        
        // switch(mMode) {
        // case PM_XINC:
        // case PM_XINC_R:
        //     mMode = mCurrProgress > mMoveStartProgress ? PM_XINC : PM_XINC_R;
        //     break;
        // case PM_XDEC:
        // case PM_XDEC_R:
        //     break;
        // case PM_YINC:
        // case PM_YDEC:
        //     break;
        // case PM_YINC_R:
        // case PM_YDEC_R:
        //     break;
        // }
        
        // if (isMoveInc()) {
		// 	if (mLastProgress > mCurrProgress)
		// 		mLastProgress = mCurrProgress;
        // }
        // else {
		// 	if (mLastProgress < mCurrProgress)
		// 		mLastProgress = mCurrProgress;
		// }

        plugin_ui_control_dispatch_event(
            control(), control(),
            plugin_ui_event_progress_changed, plugin_ui_event_dispatch_to_self_and_parent);
	}
}

void RGUIProgressBar::SetToProgress( float value) {
    if (mCurrProgress != value) {
        mMoveStartProgress = mCurrProgress;
        mCurrProgress  = value;
        mMovedTime = 0.0f;
        
        mLastProgress = mCurrProgress;
        
        plugin_ui_control_dispatch_event(
            control(), control(),
            plugin_ui_event_progress_changed, plugin_ui_event_dispatch_to_self_and_parent);

        updateFrames();
    }
}

bool RGUIProgressBar::isMoveInc(void) const {
	switch ((uint8_t)mMode) {
	case PM_XINC:
	case PM_YINC:
	case PM_XINC_R:
	case PM_YINC_R:
        return true;
    default:
        return false;
    }
}

int RGUIProgressBar::progress_getter(plugin_ui_control_t control, dr_value_t data) {
    RGUIProgressBar * progress = cast<RGUIProgressBar>(control);

    data->m_type = CPE_DR_TYPE_FLOAT;
    data->m_meta = NULL;
    data->m_data = (void*)&progress->mCurrProgress;
    data->m_size = sizeof(progress->mCurrProgress);
    
    return 0;
}

int RGUIProgressBar::progress_setter(plugin_ui_control_t control, dr_value_t value) {
    float v;

    if (dr_value_try_read_float(&v, value, NULL) != 0) return -1;

    cast<RGUIProgressBar>(control)->SetProgress(v);

    return 0;
}

int RGUIProgressBar::mode_getter(plugin_ui_control_t control, dr_value_t data) {
    RGUIProgressBar * progress = cast<RGUIProgressBar>(control);
    
    data->m_type = CPE_DR_TYPE_UINT8;
    data->m_meta = NULL;
    data->m_data = (void*)&progress->mMode;
    data->m_size = sizeof(progress->mMode);
    
    return 0;
}

int RGUIProgressBar::mode_setter(plugin_ui_control_t control, dr_value_t value) {
    uint8_t v;

    if (dr_value_try_read_uint8(&v, value, NULL) != 0) return -1;

    RGUIProgressBar * progress = cast<RGUIProgressBar>(control);
    progress->mMode = v;
    
    return 0;
}

void RGUIProgressBar::setup(plugin_ui_control_meta_t meta) {
    RGUILabel::setup(meta);

    plugin_ui_control_attr_meta_create(meta, "progress", progress_setter, progress_getter);    
    plugin_ui_control_attr_meta_create(meta, "mode", mode_setter, mode_getter);    
}

