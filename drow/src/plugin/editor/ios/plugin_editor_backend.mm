#include <assert.h>
#include "plugin/layout/plugin_layout_render.h"
#include "plugin/layout/plugin_layout_render_node.h"
#include "plugin/layout/plugin_layout_animation_selection.h"
#include "plugin/layout/plugin_layout_animation_caret.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "plugin/app_env/ios/plugin_app_env_ios.h"
#include "plugin_editor_backend_i.h"
#include "../plugin_editor_editing_i.h"

int plugin_editor_backend_init(plugin_editor_module_t module) {
    plugin_editor_backend_t backend;
    UIWindow * window = (UIWindow*)plugin_app_env_ios_window(plugin_app_env_module_find_nc(module->m_app, NULL));

    assert(window);
    
    backend = (plugin_editor_backend_t)mem_alloc(module->m_alloc, sizeof(struct plugin_editor_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "%s: backend: alloc fail", plugin_editor_module_name(module));
        return -1;
    }

    backend->m_delegate = [[plugin_editor_delegate alloc] init];
    if (backend->m_delegate == nil) {
        CPE_ERROR(module->m_em, "%s: backend: alloc delegate fail", plugin_editor_module_name(module));
        mem_free(module->m_alloc, backend);
        return -1;
    }
    backend->m_delegate.module = module;
    
    [window addSubview: backend->m_delegate.view];
    
    module->m_backend = backend;
    return 0;
}

void plugin_editor_backend_fini(plugin_editor_module_t module) {

    [module->m_backend->m_delegate release];

    mem_free(module->m_alloc, module->m_backend);
    module->m_backend = NULL;
}

void plugin_editor_backend_update_content(plugin_editor_module_t module, plugin_layout_render_t render) {
    plugin_editor_backend_t backend = module->m_backend;
    plugin_editor_delegate * delegate = backend->m_delegate;

    assert(module->m_active_editing);
    
    if(delegate.accessoryView.hidden != NO) {
        delegate.accessoryView.hidden = NO;
    }

    delegate.inputView.keyboardType =
        module->m_active_editing->m_number_only
        ? UIKeyboardTypeNumbersAndPunctuation
        : UIKeyboardTypeDefault;

    delegate.inputView.secureTextEntry = module->m_active_editing->m_is_passwd ? TRUE : FALSE;

    char * c_data = plugin_layout_render_text_utf8(module->m_alloc, render);
    delegate.inputView.text = [NSString stringWithUTF8String: c_data];
    mem_free(module->m_alloc, c_data);

    [delegate.inputView becomeFirstResponder];
}

char * plugin_editor_backend_clipboard_get(plugin_editor_module_t module) {
    //UIPasteboard *pb = [UIPasteboardNameGeneral generalPasteboard];

    //[pb dataForPasteboardType: kUTTypePlainText];
    
    return NULL;
}

int plugin_editor_backend_clipboard_put(plugin_editor_module_t module, const char * data) {
    //UIPasteboard *pb = [UIPasteboardNameGeneral generalPasteboard];
    return 0;
}

void plugin_editor_backend_close(plugin_editor_module_t module) {
    plugin_editor_backend_t backend = module->m_backend;
    plugin_editor_delegate * delegate = backend->m_delegate;

    assert(module->m_active_editing);

    [delegate.inputView resignFirstResponder];
    if(delegate.accessoryView.hidden != YES) {
        delegate.accessoryView.hidden = YES;
        plugin_editor_backend_commit_text(module);
        plugin_editor_backend_commit_selection(module);
    }
}

void plugin_editor_backend_update_selection(plugin_editor_module_t module, plugin_layout_render_t render) {
    plugin_editor_delegate * delegate = module->m_backend->m_delegate;
    plugin_layout_animation_selection_t selection;
    plugin_layout_animation_caret_t caret;
    
    NSRange range ;
    if ((selection = plugin_layout_animation_selection_find_first(render))) {
        int begin_pos = plugin_layout_animation_selection_begin_pos(selection);
        int end_pos = plugin_layout_animation_selection_end_pos(selection);

        range.location = begin_pos;
        range.length = end_pos >= begin_pos ? (end_pos - begin_pos) : 0;
        delegate.inputView.selectedRange  = range;
    }
    else if (caret = plugin_layout_animation_caret_find_first(render)) {
        range.location = plugin_layout_animation_caret_pos(caret);
        range.length = 0;
        delegate.inputView.selectedRange  = range;
    }
}

void plugin_editor_backend_commit_text(plugin_editor_module_t module) {
    plugin_editor_delegate * delegate = module->m_backend->m_delegate;
    
    const char * text = [[delegate.inputView text] UTF8String];
    if (text == NULL) text = "";

    plugin_editor_editing_commit_data_utf8(module->m_active_editing, text);
}

void plugin_editor_backend_commit_selection(plugin_editor_module_t module) {
    plugin_editor_delegate * delegate = module->m_backend->m_delegate;
    
    plugin_editor_editing_commit_selection(
        module->m_active_editing,
        delegate.inputView.selectedRange.location,
        delegate.inputView.selectedRange.location + delegate.inputView.selectedRange.length);
}
