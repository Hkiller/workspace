#include "cpe/pal/pal_string.h"
#include "render/model/ui_data_utils.h"

uint8_t ui_pos_policy_from_str(const char * str_policy) {
	if(strcmp(str_policy, "top-center") == 0) {
		return ui_pos_policy_top_center;
	}
	else if(strcmp(str_policy, "top-left") == 0) {
		return ui_pos_policy_top_left;
	}
	else if(strcmp(str_policy, "top-right") == 0) {
		return ui_pos_policy_top_right;
	}
	else if(strcmp(str_policy, "center-left") == 0) {
		return ui_pos_policy_center_left;
	}
	else if(strcmp(str_policy, "center") == 0) {
		return ui_pos_policy_center;
	}
	else if(strcmp(str_policy, "center-right") == 0) {
		return ui_pos_policy_center_right;
	}
	else if(strcmp(str_policy, "bottom-left") == 0) {
		return ui_pos_policy_bottom_left;
	}
	else if(strcmp(str_policy, "bottom-center") == 0) {
		return ui_pos_policy_bottom_center;
	}
	else if(strcmp(str_policy, "bottom-right") == 0) {
		return ui_pos_policy_bottom_right;
	}
	else if(strcmp(str_policy, "pivot") == 0) {
		return ui_pos_policy_pivot;
	}
    else {
		return ui_pos_policy_center;
    }
}
