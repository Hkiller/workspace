#import "EAGLView.h"
#include "../EnvExt.hpp"

@interface EAGLView (PrivateMethods)
- (void)createBuffs;
- (void)deleteBuffs;
@end

@implementation EAGLView

GLuint _frameBuffer = nil;
GLuint _colorBuffer = nil;

@synthesize context;

// You must implement this method
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame: frame];

	if (self) {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:TRUE], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                        nil];
        eaglLayer.contentsScale = [[UIScreen mainScreen] scale];
    }

    return self;
}

//The EAGL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:.
- (id)initWithCoder:(NSCoder*)coder
{
    self = [super initWithCoder:coder];
	if (self) {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:TRUE], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                        nil];
        eaglLayer.contentsScale = [[UIScreen mainScreen] scale];
    }
    
    return self;
}

- (void)dealloc
{
    [self deleteBuffs];    
    [context release];
    context = nil;
    [super dealloc];
}

- (void)setContext:(EAGLContext *)newContext
{
    if (context != newContext) {
        if (context) {
            [EAGLContext setCurrentContext:context];
            [self deleteBuffs];
            [EAGLContext setCurrentContext:nil];
        
            [context release];
            context = nil;
        }
        
        context = [newContext retain];
    }
}

- (GLuint) colorBuffer
{
    if (!_colorBuffer) {
        [self createBuffs];
    }

    return _colorBuffer;
}

- (GLuint) frameBuffer
{
    if (!_frameBuffer) {
        [self createBuffs];
    }

    return _frameBuffer;
}

- (void)createBuffs
{
    assert(!_frameBuffer);
    [EAGLContext setCurrentContext:context];
    assert(glGetError() == 0);

    glGenFramebuffers(1, &_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
    assert(glGetError() == 0);

    glGenRenderbuffers(1, &_colorBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorBuffer);
    assert(glGetError() == 0);

    GLint framebufferWidth;
    GLint framebufferHeight;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &framebufferWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &framebufferHeight);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorBuffer);
    assert(glGetError() == 0);

    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.layer];

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
}

- (void)deleteBuffs
{
    if (context) {
        [EAGLContext setCurrentContext:context];
        
        if (_frameBuffer) {
            glDeleteFramebuffers(1, &_frameBuffer);
            _frameBuffer = 0;
        }
        
        if (_colorBuffer) {
            glDeleteRenderbuffers(1, &_colorBuffer);
            _colorBuffer = 0;
        }
    }
}

- (void)layoutSubviews
{
    [self deleteBuffs];
}

@end
