#import <QuartzCore/QuartzCore.h>
#include "cpe/pal/pal_socket.h" /*for select (sleep)*/
#include "cpe/utils/time_utils.h"
#include "render/runtime/ui_runtime_render.h"
#include "plugin/ui/plugin_ui_phase.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#import "AppDelegate.h"
#import "ViewController.h"
#import "EAGLView.h"
#include "../EnvExt.hpp"
#include "../RuningExt.hpp"

@interface ViewController ()

@property (nonatomic, assign) CADisplayLink *displayLink;

@end

@implementation ViewController

@synthesize displayLink;
@synthesize animationFrameInterval;
@synthesize animating;
@synthesize textureContext;

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    EAGLContext * context = [[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2] autorelease];
    if (!context) 
    {
        NSLog(@"Failed to create ES context");
    }
    
    if ([context respondsToSelector:@selector(setMultiThreaded:)]) {
        context.multiThreaded = YES;
    }

    textureContext = [[EAGLContext alloc] initWithAPI:[context API] sharegroup: [context sharegroup]];
    if ([textureContext respondsToSelector:@selector(setMultiThreaded:)]) {
        textureContext.multiThreaded = YES;
    }

    EAGLView * glView = (EAGLView *)self.view;
    glView.context = context;
    
    animating = FALSE;
    animationFrameInterval = 1;
    
    self.displayLink = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
    [self startAnimation];
    
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self stopAnimation];
    
    [super viewWillDisappear:animated];
}


- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    /*
	 Frame interval defines how many display frames must pass between each time the display link fires.
	 The display link will only fire 30 times a second when the frame internal is two on a display that refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second when the display refreshes at 60 times a second. A frame interval setting of less than one results in undefined behavior.
	 */
    if (frameInterval >= 1) {
        animationFrameInterval = frameInterval;
        
        if (animating) {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

-(void)ImpLoop
{
    Gd::App::Application & app = Gd::App::Application::instance();
    UI::App::EnvExt & env = UI::App::EnvExt::instance(app);
    plugin_ui_env_t ui_env = env.uiCenter().uiEnv();
    
    EAGLView * glView = (EAGLView*)self.view;

    UIApplication * application = [UIApplication sharedApplication];
    AppDelegate * appDelegate = application.delegate;
    
    if (gd_app_state(app) == gd_app_done) {
        [self stopAnimation];
        [appDelegate applicationWillTerminate: application];
        exit(0);
        return;
    }

    plugin_ui_phase_node_t phase_node = plugin_ui_phase_node_current(ui_env);
    plugin_ui_phase_t phase = phase_node ? plugin_ui_phase_node_current_phase(phase_node) : NULL;
    uint8_t fps = phase ? plugin_ui_phase_fps(phase) : 60;
    if (fps < 60) {
        uint16_t fix_frame_time = 1000 / fps;
        int64_t cur_time = cur_time_ms();
        int64_t last_time = env.runing().lastUpdateTime();
        int64_t diff_time = cur_time - last_time;
        
        if (diff_time > 0 && diff_time < fix_frame_time) {
            fd_set rfds;
            struct timeval tv;
            int fd = 1;

            FD_ZERO (&rfds);
            FD_SET(fd, &rfds);
            tv.tv_sec = 0;
            tv.tv_usec = (fix_frame_time - diff_time) * 1000;
            if (select(0, NULL, NULL, NULL, &tv) == -1) {
                APP_CTX_INFO(
                    app, "ViewController: sleep %d ms, select error, error=%d (%s)!",
                    (fix_frame_time - diff_time), errno, strerror(errno));
            }
            
        }
    }
    
    /*update */
    [EAGLContext setCurrentContext: textureContext];
    env.runing().update();
    [EAGLContext setCurrentContext: nil];

    /*绘制*/
    [EAGLContext setCurrentContext: glView.context];
    ui_runtime_render_t render = env.context();

    /*    开始绘制，上一次绘制没有完成会阻塞 */
    uint8_t have_pre_frame;
    ui_runtime_render_begin(render, &have_pre_frame);

    /*    有上一帧绘制数据则需要提交 */
    if (have_pre_frame) {
        glBindRenderbuffer(GL_RENDERBUFFER, glView.colorBuffer);
        [glView.context presentRenderbuffer:GL_RENDERBUFFER];
    }
    
    /*    开始绘制当前帧 */
    glBindFramebuffer(GL_FRAMEBUFFER, glView.frameBuffer);
    plugin_ui_env_render(ui_env, render);

    ui_runtime_render_done(render);
    
    [EAGLContext setCurrentContext: nil];

        
    if ([appDelegate respondsToSelector:@selector(ShowFrame)]) {
        [appDelegate performSelector:@selector(ShowFrame)];
    }
    
}

- (void)startAnimation
{
    if (!animating) {
        /*
         CADisplayLink is API new in iOS 3.1. Compiling against earlier versions will result in a warning, but can be dismissed if the system version runtime check for CADisplayLink exists in -awakeFromNib. The runtime check ensures this code will not be called in system versions earlier than 3.1.
         */
        displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(ImpLoop)];
        [displayLink setFrameInterval:animationFrameInterval];
        
        // The run loop will retain the display link on add.
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating) {
        [self.displayLink invalidate];
        self.displayLink = nil;
        
        animating = FALSE;
    }
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    if (interfaceOrientation == UIDeviceOrientationLandscapeRight || interfaceOrientation == UIDeviceOrientationLandscapeLeft)
    {
        return YES;
    }
	
	return NO;
}

-(void)touchedProcess:(NSSet*)touches withEvent:(UIEvent*)event
{
    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());

    int curTouchID = -1;
    for(UITouch *myTouch in touches)
    {
        CGPoint newpoint = [myTouch locationInView:self.view];
        
        float scale = [[UIScreen mainScreen] scale];
        newpoint.x *= scale; newpoint.y *= scale;
        
        curTouchID = (ptr_int_t)myTouch;
        if(UITouchPhaseBegan == myTouch.phase)
        {
            env.runing().processInput(UI::App::RuningExt::TouchBegin, curTouchID, newpoint.x, newpoint.y);
        }
        else if(UITouchPhaseMoved == myTouch.phase)
        {
            env.runing().processInput(UI::App::RuningExt::TouchMove, curTouchID, newpoint.x, newpoint.y);
        }
        else if(UITouchPhaseEnded == myTouch.phase || UITouchPhaseCancelled == myTouch.phase)
        {
            env.runing().processInput(UI::App::RuningExt::TouchEnd, curTouchID, newpoint.x, newpoint.y);
        }
    }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchedProcess:touches withEvent:event];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchedProcess:touches withEvent:event];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchedProcess:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchedProcess:touches withEvent:event];
}

#pragma mark - Releasing

- (void)viewDidUnload
{    
    [super viewDidUnload];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc. that aren't in use.
}

- (void)dealloc 
{
    if (textureContext) [textureContext release];
    [super dealloc];
}

@end


