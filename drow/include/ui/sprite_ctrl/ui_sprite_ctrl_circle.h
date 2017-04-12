#ifndef UI_SPRITE_CTRL_CIRCLE_H
#define UI_SPRITE_CTRL_CIRCLE_H
#include "gd/app/app_types.h"
#include "ui_sprite_ctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CTRL_CIRCLE_NAME;

ui_sprite_ctrl_circle_t ui_sprite_ctrl_circle_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ctrl_circle_free(ui_sprite_ctrl_circle_t ctrl);

const char * ui_sprite_ctrl_circle_on_begin(ui_sprite_ctrl_circle_t ctrl);
int ui_sprite_ctrl_circle_set_on_begin(ui_sprite_ctrl_circle_t ctrl, const char * on_begin);

const char * ui_sprite_ctrl_circle_on_move(ui_sprite_ctrl_circle_t ctrl);
int ui_sprite_ctrl_circle_set_on_move(ui_sprite_ctrl_circle_t ctrl, const char * on_ctrl);

const char * ui_sprite_ctrl_circle_on_done(ui_sprite_ctrl_circle_t ctrl);
int ui_sprite_ctrl_circle_set_on_done(ui_sprite_ctrl_circle_t ctrl, const char * on_done);

const char * ui_sprite_ctrl_circle_on_cancel(ui_sprite_ctrl_circle_t ctrl);
int ui_sprite_ctrl_circle_set_on_cancel(ui_sprite_ctrl_circle_t ctrl, const char * on_cancel);

float ui_sprite_ctrl_circle_keep_send_span(ui_sprite_ctrl_circle_t ctrl);
void ui_sprite_ctrl_circle_set_keep_send_span(ui_sprite_ctrl_circle_t ctrl, float span);

/*是否反方向 */
uint8_t ui_sprite_ctrl_circle_negative(ui_sprite_ctrl_circle_t ctrl);
void ui_sprite_ctrl_circle_set_negative(ui_sprite_ctrl_circle_t ctrl, uint8_t negative);

/*是否scale图片 */
uint8_t ui_sprite_ctrl_circle_do_scale(ui_sprite_ctrl_circle_t ctrl);
void ui_sprite_ctrl_circle_set_do_scale(ui_sprite_ctrl_circle_t ctrl, uint8_t do_scale);

/*是否旋转图片 */
uint8_t ui_sprite_ctrl_circle_do_rotate(ui_sprite_ctrl_circle_t ctrl);
void ui_sprite_ctrl_circle_set_do_rotate(ui_sprite_ctrl_circle_t ctrl, uint8_t do_rotate);

/*最小屏幕距离，小于这个不改变数值 */
float ui_sprite_ctrl_circle_screen_min(ui_sprite_ctrl_circle_t ctrl);
float ui_sprite_ctrl_circle_screen_max(ui_sprite_ctrl_circle_t ctrl);
int ui_sprite_ctrl_circle_set_screen_range(ui_sprite_ctrl_circle_t ctrl, float _min, float _max);

/*角度限制 */
float ui_sprite_ctrl_circle_angle_min(ui_sprite_ctrl_circle_t ctrl);
float ui_sprite_ctrl_circle_angle_max(ui_sprite_ctrl_circle_t ctrl);
int ui_sprite_ctrl_circle_set_angle_range(ui_sprite_ctrl_circle_t ctrl, float _min, float _max);

/*最大  */
float ui_sprite_ctrl_circle_logic_min(ui_sprite_ctrl_circle_t ctrl);
float ui_sprite_ctrl_circle_logic_max(ui_sprite_ctrl_circle_t ctrl);
int ui_sprite_ctrl_circle_set_logic_range(ui_sprite_ctrl_circle_t ctrl, float _min, float _max);

/*逻辑距离的scale */
float ui_sprite_ctrl_circle_logic_base(ui_sprite_ctrl_circle_t ctrl);
void ui_sprite_ctrl_circle_set_logic_base(ui_sprite_ctrl_circle_t ctrl, float base_value);

/*逆向取消的距离 */
float ui_sprite_ctrl_circle_cancel_distance(ui_sprite_ctrl_circle_t ctrl);
void ui_sprite_ctrl_circle_set_cancel_distance(ui_sprite_ctrl_circle_t ctrl, float cancel_distance);

#ifdef __cplusplus
}
#endif

#endif
