#import "UncaughtExceptionHandler.h"
#include <libkern/OSAtomic.h>
#include <execinfo.h>

NSString * const UncaughtExceptionHandlerSignalExceptionName = @"UncaughtExceptionHandlerSignalExceptionName";
NSString * const UncaughtExceptionHandlerSignalKey = @"UncaughtExceptionHandlerSignalKey";
NSString * const UncaughtExceptionHandlerAddressesKey = @"UncaughtExceptionHandlerAddressesKey";
volatile int32_t UncaughtExceptionCount = 0;
const int32_t UncaughtExceptionMaximum = 10;
const NSInteger UncaughtExceptionHandlerSkipAddressCount = 0;
const NSInteger UncaughtExceptionHandlerReportAddressCount = 15;

@implementation UncaughtExceptionHandler
+ (NSArray *)backtrace
{
    static const int length = 256;
    
    void* callstack[length];
    int frames = backtrace(callstack, length);    
    char **strs = backtrace_symbols(callstack, frames);    

    NSMutableArray *backtrace = [NSMutableArray arrayWithCapacity:frames];
    for (int i = UncaughtExceptionHandlerSkipAddressCount; (i < UncaughtExceptionHandlerSkipAddressCount + UncaughtExceptionHandlerReportAddressCount) && i < frames; i++)
    {
	 	[backtrace addObject:[NSString stringWithUTF8String:strs[i]]];
    }
    
    free(strs);
    return backtrace;
}

- (void)handleException:(NSException *)exception
{
    NSSetUncaughtExceptionHandler(NULL);
    
	signal(SIGABRT, SIG_DFL);
	signal(SIGILL, SIG_DFL);    
	signal(SIGSEGV, SIG_DFL);    
	signal(SIGFPE, SIG_DFL);    
	signal(SIGBUS, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);   
    
    //Xiaoxin::ExceptionHandler::OnException();
    
	NSString *reason = [exception reason];
	NSString *name = [exception name];
    NSDictionary *userinfo = [exception userInfo];
    NSArray* callstackArray = [userinfo objectForKey:UncaughtExceptionHandlerAddressesKey];

    NSString *platform= [NSString stringWithUTF8String:"TODO: config in env"];

	NSString *urlStr = [NSString stringWithFormat:@"mailto:business@drow-games.cn?subject=自动Bug报告&body=感谢您的配合!<br><br><br>"
						"错误详情:<br>%@<br>------------------<br>%@<br>----------------<br>%@<br>--------------<br>%@",platform,
						name,reason,[callstackArray componentsJoinedByString:@"<br>"]];
    
	NSURL *url = [NSURL URLWithString:[urlStr stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
	[[UIApplication sharedApplication] openURL:url];
    
	if ([[exception name] isEqual:UncaughtExceptionHandlerSignalExceptionName])        
	{        
		kill(getpid(), [[[exception userInfo] objectForKey:UncaughtExceptionHandlerSignalKey] intValue]);        
	}
	else        
	{        
		[exception raise];        
	}
    
}
@end

NSString* getAppInfo()
{
    // SVR_VERSION_VERSION const & ver = Xiaoxin::GameConfig::Instance()->GetVersionCode();
    
    // char buf[128];
    // snprintf(buf, sizeof(buf), "%d.%d.%d", ver.parts[0], ver.parts[1], ver.parts[2]);
    //TODO: version
    char buf[128];
    snprintf(buf, sizeof(buf), "1.0.0");

    NSString *appInfo = [NSString stringWithFormat:@"App : %@(%@)\nDevice : %@\nOS Version : %@ %@\n",
                         [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"],
                         [NSString stringWithUTF8String:buf],
                         [UIDevice currentDevice].model,                         
                         [UIDevice currentDevice].systemName,
                         [UIDevice currentDevice].systemVersion];

    return appInfo;
}

/*
 SYS
 */
void UncaughtException(NSException *exception)
{
    /*
	NSArray *arr = [exception callStackSymbols];
	NSString *reason = [exception reason];
	NSString *name = [exception name];
    NSString *platform= [NSString stringWithUTF8String:GetReportSubject().c_str()];
    
	NSString *urlStr = [NSString stringWithFormat:@"mailto://Xiaoxin@126.com?subject=自动Bug报告&body=感谢您的配合!<br><br><br>"
						"错误详情:<br>%@<br>-------------------<br>%@<br>----------------<br>%@<br>------------------<br>%@",platform,
						name,reason,[arr componentsJoinedByString:@"<br>"]];
	NSURL *url = [NSURL URLWithString:[urlStr stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
	[UIApplication sharedApplication] openURL:url];
    */
    //Xiaoxin::ExceptionHandler::OnException();
}

/*
 SIG
 */
void MySignalHandler(int signal)
{
	int32_t exceptionCount = OSAtomicIncrement32(&UncaughtExceptionCount);    
	if (exceptionCount > UncaughtExceptionMaximum)
	{
		return;
	}
    
	NSMutableDictionary *userInfo =[NSMutableDictionary dictionaryWithObject:[NSNumber numberWithInt:signal] forKey:UncaughtExceptionHandlerSignalKey];
	NSArray *callStack = [UncaughtExceptionHandler backtrace];    
	[userInfo setObject:callStack forKey:UncaughtExceptionHandlerAddressesKey];
	[userInfo setObject:[NSNumber numberWithInt:signal] forKey:UncaughtExceptionHandlerSignalKey];
    
	[[[[UncaughtExceptionHandler alloc] init] autorelease] performSelectorOnMainThread:@selector(handleException:) withObject:
     
    [NSException exceptionWithName:UncaughtExceptionHandlerSignalExceptionName reason:[NSString stringWithFormat: NSLocalizedString(@"Signal %d was raised.\n"@"%@", nil),signal, getAppInfo()]
      userInfo:userInfo] waitUntilDone:YES];
}

void InstallUncaughtExceptionHandler()
{
    NSSetUncaughtExceptionHandler(&UncaughtException);
	
    signal(SIGABRT, MySignalHandler);
	signal(SIGILL, MySignalHandler);    
	signal(SIGSEGV, MySignalHandler);    
	signal(SIGFPE, MySignalHandler);    
	signal(SIGBUS, MySignalHandler);    
	signal(SIGPIPE, MySignalHandler);

}
