#ifndef  __RGUILIB_H__
#define  __RGUILIB_H__
#include "RGUI.h"

class RGUILib {
public:
	/*
	static method
	*/
	static void		Init			( plugin_ui_env_t env  );
	static void		ShutDown		( plugin_ui_env_t env );
};

#endif
