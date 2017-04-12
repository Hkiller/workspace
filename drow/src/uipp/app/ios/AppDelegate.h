#import <UIKit/UIKit.h>
#include "gd/app/app_context.h"

@class ViewController;
@class EAGLContext;

@interface AppDelegate : UIResponder<UIApplicationDelegate> {
}

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) ViewController *viewController;
@property (nonatomic, readonly) gd_app_context_t app;
@property (retain, nonatomic, readonly) EAGLContext * workerContext;

-(void)SetInteraction:(BOOL)allow onView:(UIView*)aView;
-(void)sendMail:(NSString*)subject body:(NSString *)messageStr sender:(NSString*)sender recipient:(NSString*)recipients;

@end
