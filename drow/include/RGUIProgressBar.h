#ifndef DROW_UI_CONTROL_PROGRESSBAR_H
#define DROW_UI_CONTROL_PROGRESSBAR_H
#include "RGUILabel.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIProgressBar, ui_control_type_progress);

class RGUIProgressBar : public RGUILabel {
public:
	RGUIProgressBar();

	uint8_t GetType(void) const { return mType; }
	void SetType(uint8_t type) { mType = type; }

	uint8_t GetMode(void) const { return mMode; }
	void SetMode(uint8_t mode) { mMode = mode; }

	void SetSpeed(float speed) { mSpeed = speed; mMoveDuration = 0.0f; }
	float GetSpeed(void) const { return mSpeed; }

	void SetProgress(float value);
    void SetToProgress(float value);
	float GetProgress(void) const { return mCurrProgress; }

	float GetLastProgress(void) { return mLastProgress; }
	
	void SetAlphaMove(bool alphaMove) { mAlphaMove = alphaMove; }
    void SetMoveDuration(float duration) { mMoveDuration = duration; mSpeed = 0.0f; }

    virtual void Load(ui_data_control_t control);

protected:
	virtual void UpdateSelf(float deltaTime);
    bool updateProgress(float deltaTime);
    void updateFrames(void);
    void updateFrames(float progress, float alpha);
    
    bool isMoveInc(void) const;

    static int mode_setter(plugin_ui_control_t control, dr_value_t value);
    static int mode_getter(plugin_ui_control_t control, dr_value_t data);    
    static int progress_setter(plugin_ui_control_t control, dr_value_t value);
    static int progress_getter(plugin_ui_control_t control, dr_value_t data);    
    static void setup(plugin_ui_control_meta_t meta);

protected:
	uint8_t	        mType;
	uint8_t	        mMode;
    float           nMoveDuration;
    float           mMovedTime;
    float           mMoveStartProgress;
	float	        mSpeed;
	float			mCurrProgress;
	float			mLastProgress;
	bool			mAlphaMove;
    float           mMoveDuration;

    ~RGUIProgressBar();
    friend class RGUIControlRepo; 
};

#endif
