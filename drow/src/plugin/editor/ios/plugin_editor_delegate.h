#ifndef PLUGIN_EDITOR_DELEGATE_I_H
#define PLUGIN_EDITOR_DELEGATE_I_H
#import <UIKit/UIKit.h>
#include "../plugin_editor_module_i.h"

@interface plugin_editor_delegate : UIViewController<UITextViewDelegate> {
    plugin_editor_module_t module;
    UIView*             accessoryView;
    UITextView*         inputView;
    CGFloat             deviceWidth;
    CGFloat             deviceHeight;
};

@property(nonatomic , assign) plugin_editor_module_t module;
@property(nonatomic , retain) UIView*       accessoryView;
@property(nonatomic , retain) UITextView*   inputView;

@end

#endif
