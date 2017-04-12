#import <UIKit/UIKit.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

@class EAGLContext;

@interface EAGLView : UIView {
}

@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, readonly) GLuint colorBuffer;
@property (nonatomic, readonly) GLuint frameBuffer;

- (id) initWithFrame:(CGRect)frame;

@end
