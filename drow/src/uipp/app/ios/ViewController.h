#ifndef Application_ViewController_h
#define Application_ViewController_h

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@interface ViewController : UIViewController

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;
@property (nonatomic, retain, readonly) EAGLContext * textureContext;

-(void) startAnimation;
-(void) stopAnimation;
@end


#endif
