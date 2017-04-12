#import <UIKit/UIKit.h>
#import "cpepp/dr/Utils.hpp"
#import "gd/app/app_context.h"
#import "gd/app/app_module.h"
#import "gdpp/app/Log.hpp"
#import "gdpp/app/Application.hpp"
#import "AppDelegate.h"
#import "ViewController.h"
#import "plugin/app_env/plugin_app_env_module.h"
#import "plugin/app_env/ios/plugin_app_env_ios.h"
#import "protocol/plugin/app_env/app_env_pro.h"
#import "render/runtime/ui_runtime_render.h"
#import "render/runtime/ui_runtime_render_worker.h"
#import "UncaughtExceptionHandler.h"
#import "EAGLView.h"
#import "../EnvExt.hpp"

extern "C" {
    int plugin_app_env_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg);
    void plugin_app_env_module_app_fini(gd_app_context_t app, gd_app_module_t module);
}

@implementation AppDelegate

gd_app_context_t _app = NULL;

@synthesize window = _window;
@synthesize viewController = _viewController;
@synthesize app = _app;

-(id) init
{
    if ( self = [super init] ) {
    }
    return self;
}

- (void) dealloc
{
    [_window release];
    [_viewController release];
    [super dealloc];
}

- (void) initAppEnv: (UIApplication *)application options: (NSDictionary *)launchOptions
{
    char prog_name[] = "PlayerShareData";
    char * argv[] = { prog_name};

    _app = gd_app_context_create_main(NULL, 0, 1, argv);
    if (_app == NULL) {
        assert(false);
        throw ::std::runtime_error("create app fail!");
    }

    gd_app_set_debug(_app, 1);
    gd_app_ins_set(_app);
    if (gd_app_add_tag(_app, "ios") != 0) {
        APP_CTX_ERROR(_app, "AppContext: set app_env module tag fail!");
        gd_app_context_free(_app);
        _app = NULL;
        throw ::std::runtime_error("create app fail!");
    }

    if (gd_app_module_type_init(
            "plugin_app_env_module",
            plugin_app_env_module_app_init, plugin_app_env_module_app_fini, NULL, NULL, gd_app_em(_app))
        != 0
        || gd_app_install_module(_app, "plugin_app_env_module", "plugin_app_env_module", NULL, NULL)
        == NULL)
    {
        APP_CTX_ERROR(_app, "AppContext: create app_env module fail!");
        gd_app_context_free(_app);
        _app = NULL;
        throw ::std::runtime_error("create app fail!");
    }

    plugin_app_env_module_t app_env = plugin_app_env_module_find_nc(_app, NULL);
    assert(app_env);

    plugin_app_env_ios_set_window(app_env, self.window);
    plugin_app_env_ios_set_application(app_env, application);

    if (gd_app_cfg_reload(_app) != 0) {
        APP_CTX_ERROR(_app, "AppContext: load cfg fail!");
        gd_app_context_free(_app);
        _app = NULL;
        assert(false);
        throw ::std::runtime_error("create app fail!");
    }

    if (gd_app_modules_load(_app) != 0) {
        gd_app_context_free(_app);
        _app = NULL;
        APP_CTX_ERROR(_app, "AppContext: create app load module fail!");
        assert(false);
        throw ::std::runtime_error("create app fail!");
    }
}

extern void InstallUncaughtExceptionHandler();
// static int app_gl_worker_begin_render(void * ctx);
// static void app_gl_worker_end_render(void * ctx);

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    @autoreleasepool {
        chdir([[[NSBundle mainBundle] bundlePath]UTF8String]);
    }
    
    InstallUncaughtExceptionHandler();
    
    self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
    
    EAGLView *glView = [[EAGLView alloc] initWithFrame: [self.window bounds] ];
    ViewController * glViewController = [[ViewController alloc] initWithNibName:nil bundle:nil];
    
    self.viewController = glViewController;
    self.viewController.wantsFullScreenLayout = YES;
    self.viewController.view = glView;
    [self.viewController viewDidLoad];

    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];
    
    //enable multi touch
    self.viewController.view.multipleTouchEnabled = YES;

    [EAGLContext setCurrentContext: glViewController.textureContext];
    assert(glGetError() == 0);
    glBindFramebuffer(GL_FRAMEBUFFER, glView.frameBuffer);
    assert(glGetError() == 0);
    
    [self initAppEnv: application
             options: launchOptions  ];
    assert(glGetError() == 0);

    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());

    CGSize sz;
    sz.width = [[UIScreen mainScreen] bounds].size.width;
    sz.height = [[UIScreen mainScreen] bounds].size.height;
    
    CGSize scale;
    scale.width = [UIScreen mainScreen].scale;
    scale.height = [UIScreen mainScreen].scale;
    
    ui_vector_2_t base_size = plugin_ui_env_origin_sz(env.uiCenter().uiEnv());
    if ((base_size->x > base_size->y && sz.width < sz.height)
        || (base_size->x < base_size->y && sz.width > sz.height))
    {
        int32_t t = sz.width;
        sz.width = sz.height ;
        sz.height = t;
    }

    env.runing().init();
    env.runing().setSize(sz.width * scale.width, sz.height * scale.height);

    [EAGLContext setCurrentContext: nil];
    assert(glGetError() == 0);

    //make screen always lighting in game
    [ UIApplication sharedApplication].idleTimerDisabled = YES;
    
    return YES;
}

// static int app_gl_worker_begin_render(void * ctx) {
//     AppDelegate * app_delegate = (AppDelegate * )ctx;
//     EAGLView* glView = (EAGLView*)app_delegate.viewController.view;
    
//     [EAGLContext setCurrentContext: glView.context];
//     glBindFramebuffer(GL_FRAMEBUFFER, glView.frameBuffer);
//     return 0;
// }

// static void app_gl_worker_end_render(void * ctx) {
//     [EAGLContext setCurrentContext: NULL];
// }


- (void)applicationWillResignActive:(UIApplication *)application
{
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */

    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());

    env.runing().setState(ui_runtime_pause);

    [self.viewController stopAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());
    env.runing().setState(ui_runtime_stop);
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());
    env.runing().setState(ui_runtime_pause);

    // ui_runtime_render_t ctx = env.context();
    // ui_runtime_render_worker_t worker = ui_runtime_render_worker_get(ctx);
    // if (worker) {
    //     ui_runtime_render_worker_free(worker);
    // }
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    [self.viewController startAnimation];

    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());
    env.runing().setState(ui_runtime_runing);

    // ui_runtime_render_t ctx = env.context();
    // if(ui_runtime_render_worker_get(ctx) == NULL) {
    //     ui_runtime_render_worker_create(ctx, self, app_gl_worker_begin_render, app_gl_worker_end_render, 1);
    // }
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    gd_app_context_free(_app);
    _app = NULL;
}

-(void) SetInteraction:(BOOL)allow onView:(UIView*) aView
{
    aView.userInteractionEnabled = allow;
    for( UIView* v in aView.subviews )
    {
        [self SetInteraction:allow onView:v];
    }
}

- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)url
{
    plugin_app_env_module_t app_env = plugin_app_env_module_find_nc(_app, NULL);
    return plugin_app_env_ios_dispatch_open_url(app_env, application, url, NULL, NULL) ? TRUE : FALSE;
}

- (BOOL)application:(UIApplication *)application
            openURL:(NSURL *)url
            options:(nonnull NSDictionary<UIApplicationOpenURLOptionsKey,id> *)options
{
    return [self application:application
                     openURL:url
           sourceApplication:options[UIApplicationOpenURLOptionsSourceApplicationKey]
                  annotation:options[UIApplicationOpenURLOptionsAnnotationKey]];
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
    plugin_app_env_module_t app_env = plugin_app_env_module_find_nc(_app, NULL);
    return plugin_app_env_ios_dispatch_open_url(app_env, application, url, sourceApplication, annotation) ? TRUE : FALSE;
}

@end
