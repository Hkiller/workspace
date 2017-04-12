#import "plugin_editor_delegate.h"
#include "plugin_editor_backend_i.h"
#include "../plugin_editor_editing_i.h"

@implementation plugin_editor_delegate

@synthesize module;
@synthesize accessoryView;
@synthesize inputView;

static const CGFloat cTextPlanHeight = 60;

- (id)init {
    self = [super init];
    [self viewDidLoad];
    return self;
}

-(void)createAccessView
{
    int heigth = deviceHeight > deviceWidth ? deviceHeight : deviceWidth;
    int width = 100;
    
    self.accessoryView = [[UIView alloc]initWithFrame:CGRectMake(0,  heigth, width, cTextPlanHeight)];
    self.accessoryView.backgroundColor = [UIColor grayColor];  
    self.accessoryView.userInteractionEnabled = YES;
    self.accessoryView.hidden = YES;
    
    self.view = self.accessoryView;
    
    self.inputView = [[UITextView alloc]initWithFrame:CGRectMake(2 ,2, width - 2, cTextPlanHeight - 4)];
 
    self.inputView.keyboardType = UIKeyboardTypeDefault;
    self.inputView.textColor    = [UIColor blackColor];
    self.inputView.font         = [UIFont systemFontOfSize:20.0];
    self.inputView.secureTextEntry = YES;
    self.inputView.autocorrectionType = UITextAutocorrectionTypeNo;
    self.inputView.returnKeyType   = UIReturnKeyDone;
 
    self.inputView.scrollEnabled = NO;
    self.inputView.delegate = self;
    self.inputView.hidden = NO;  
    
    [self.accessoryView addSubview:self.inputView];
}

-(void)updateDeviceInfo
{
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    CGSize screenSize = screenRect.size;

	UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
    if(orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown) {
		deviceWidth = screenSize.width;
		deviceHeight = screenSize.height;
	}
	else if(orientation == UIInterfaceOrientationLandscapeRight || orientation == UIInterfaceOrientationLandscapeLeft) {
   		deviceWidth = screenSize.height;
		deviceHeight = screenSize.width;
	}
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self updateDeviceInfo];
    [self createAccessView];
}

- (void)textViewDidBeginEditing:(UITextView *)textView
{
    /*
    if(caretChange)
    {
        caretChange = false;
        [self UpdateCaretIndex];
    }
     */
    if (self.module->m_debug) {
        CPE_INFO(self.module->m_em, "%s: textViewDidBeginEditing", plugin_editor_module_name(self.module));
    }
}


- (void)textViewDidChange:(UITextView *)textView
{
    if (self.module->m_debug) {
        CPE_INFO(self.module->m_em, "%s: textViewDidChange", plugin_editor_module_name(self.module));
    }
    
    plugin_editor_backend_commit_text(module);
    plugin_editor_backend_commit_selection(module);
}

- (BOOL)textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text
{
    plugin_editor_editing_t editing = self.module->m_active_editing;
    assert(editing);
    
    if([text isEqualToString : @"\n"]) {
        plugin_editor_module_set_active_editing(self.module, NULL);
        return NO;
    }

    if (editing->m_max_len) {
        if ([[textView text] length ] + (text.length - range.length) >= editing->m_max_len + 1) return NO;
    }

    //return ([[textView text] length ] + (text.length - range.length) < self.maxLength +1 );
    return YES;
}

- (void)textViewDidEndEditing:(UITextView *)textView
{
    if (self.module->m_debug) {
        CPE_INFO(self.module->m_em, "%s: textViewDidEndEditing", plugin_editor_module_name(self.module));
    }

    plugin_editor_module_set_active_editing(self.module, NULL);
}

- (void)dealloc
{
    [inputView release];
    [accessoryView release];
    [super dealloc];
}

@end
