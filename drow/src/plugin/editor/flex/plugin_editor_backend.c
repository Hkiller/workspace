#include "gd/app/app_context.h"
#include "../plugin_editor_module_i.h"
#include "../plugin_editor_editing_i.h"

int plugin_editor_backend_init(plugin_editor_module_t module) {
    return 0;
}

void plugin_editor_backend_fini(plugin_editor_module_t module) {
}

void plugin_editor_backend_update_content(plugin_editor_module_t module, plugin_layout_render_t render) {
}

void plugin_editor_backend_update_selection(plugin_editor_module_t module, plugin_layout_render_t render) {
}

char * plugin_editor_backend_clipboard_get(plugin_editor_module_t module) {
    return NULL;
}

int plugin_editor_backend_clipboard_put(plugin_editor_module_t module, const char * data) {
    return 0;
}

void plugin_editor_backend_close(plugin_editor_module_t module) {
}

static void plugin_editor_on_back(plugin_editor_module_t module, plugin_editor_editing_t editing) {
/* 	//if (mTextField->GetSelectCount()) */
/* 	//{ */
/* 	//	mTextField->DelSelection(); */
/* 	//} */
/* 	//else */
/* 	//{ */
/* 	//	uint32_t caretIndex = mTextField->GetCaretIndex(); */
/* 	//	if (caretIndex > 0) */
/* 	//	{ */
/* 	//		UString temp = mTextField->GetTextW(); */
/* 	//		temp.erase( */
/* 	//			caretIndex - 1, */
/* 	//			1); */

/* 	//		mTextField->SetTextW(temp); */
/* 	//		mTextField->SetCaretIndex(caretIndex - 1); */
/* 	//	} */
/* 	//} */
/* } */
}

static void plugin_editor_on_enter(plugin_editor_module_t module, plugin_editor_editing_t editing) {
/* 	//uint32_t caretIndex = mTextField->GetCaretIndex(); */

/* 	//UString temp = mTextField->GetTextW(); */
/* 	//temp.insert( */
/* 	//	caretIndex, */
/* 	//	1, */
/* 	//	_WC('\n')); */

/* 	//mTextField->SetTextW(temp); */
/* 	//mTextField->SetCaretIndex(caretIndex + 1); */
}

static void plugin_editor_on_left(plugin_editor_module_t module, plugin_editor_editing_t editing) {
//     if (editing->m_caret > 0) {
//         plugin_editor_editing_commit_caret(editing, plugin_layout_animation_caret_pos(editing->m_caret) - 1);
//     }
}

static void plugin_editor_on_right(plugin_editor_module_t module, plugin_editor_editing_t editing) {
/* 	//if (mTextField->GetCaretIndex() < mTextField->GetTextW().length()) */
/* 	//	mTextField->SetCaretIndex(mTextField->GetCaretIndex() + 1); */
}

static void plugin_editor_on_delete(plugin_editor_module_t module, plugin_editor_editing_t editing) {
	//if (mTextField->GetSelectCount())
	//{
	//	mTextField->DelSelection();
	//}
	//else
	//{
	//	uint32_t caretIndex = mTextField->GetCaretIndex();
	//	if (caretIndex < mTextField->GetTextW().length())
	//	{
	//		UString temp = mTextField->GetTextW();
	//		temp.erase(
	//			caretIndex,
	//			1);

	//		//设置
	//		mTextField->SetTextW(temp);
	//	}
	//}
}

/* void plugin_editor_on_keydown(WPARAM wParam, LPARAM lParam) { */
/*     plugin_editor_module_t module; */

/*     module = plugin_editor_module_find_nc(gd_app_ins(), NULL); */
/*     if (module == NULL) return; */

/*     if (module->m_active_editing == NULL) return; */

/* 	switch(wParam) { */
/* 	case VK_LEFT: */
/*         plugin_editor_on_left(module, module->m_active_editing); */
/*         break; */
/* 	case VK_RIGHT: */
/*         plugin_editor_on_right(module, module->m_active_editing); */
/*         break; */
/* 	case VK_BACK: */
/*         plugin_editor_on_back(module, module->m_active_editing); */
/*         break; */
/* 	case VK_RETURN: */
/*         plugin_editor_on_enter(module, module->m_active_editing); */
/*         break; */
/* 	case VK_DELETE: */
/*         plugin_editor_on_delete(module, module->m_active_editing); */
/*         break; */
/* 	} */
/* } */

/* void plugin_editor_on_char(WPARAM wParam, LPARAM lParam) { */
/*     plugin_editor_module_t module; */

/*     module = plugin_editor_module_find_nc(gd_app_ins(), NULL); */
/*     if (module == NULL) return; */

/*     if (module->m_active_editing == NULL) return; */

/* 	/\* UString temp; *\/ */
/* 	/\* temp.append(1, (WChar)wParam); *\/ */
/* 	/\* SetText(temp); *\/ */
/* } */
