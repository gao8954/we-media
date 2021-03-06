//
// Created by gaodong on 2018/9/19.
//

#include "SimpleHandler.h"
#include <sstream>
#include <string>
#include <iostream>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

namespace {

    SimpleHandler *g_instance = NULL;

}  // namespace

SimpleHandler::SimpleHandler(bool use_views)
        : use_views_(use_views), is_closing_(false) {
    DCHECK(!g_instance);
    g_instance = this;
}

SimpleHandler::~SimpleHandler() {
    g_instance = NULL;
}

// static
SimpleHandler *SimpleHandler::GetInstance() {
    return g_instance;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString &title) {
    CEF_REQUIRE_UI_THREAD();

    if (use_views_) {
        // Set the title of the window using the Views framework.
        CefRefPtr<CefBrowserView> browser_view =
                CefBrowserView::GetForBrowser(browser);
        if (browser_view) {
            CefRefPtr<CefWindow> window = browser_view->GetWindow();
            if (window)
                window->SetTitle(title);
        }
    } else {
        // Set the title of the window using platform APIs.
        PlatformTitleChange(browser, title);
    }
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    // Add to the list of existing browsers.
    browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed destription of this
    // process.
    if (browser_list_.size() == 1) {
        // Set a flag to indicate that the window close should be allowed.
        is_closing_ = true;
    }

    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    // Remove from the list of existing browsers.
    BrowserList::iterator bit = browser_list_.begin();
    for (; bit != browser_list_.end(); ++bit) {
        if ((*bit)->IsSame(browser)) {
            browser_list_.erase(bit);
            break;
        }
    }

    if (browser_list_.empty()) {
        // All browser windows have closed. Quit the application message loop.
        CefQuitMessageLoop();
    }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString &errorText,
                                const CefString &failedUrl) {
    CEF_REQUIRE_UI_THREAD();

    // Don't display an error for downloaded files.
    if (errorCode == ERR_ABORTED)
        return;

    // Display a load error message.
    std::stringstream ss;
    ss << "<html><body bgcolor=\"white\">"
            "<h2>Failed to load URL "
       << std::string(failedUrl) << " with error " << std::string(errorText)
       << " (" << errorCode << ").</h2></body></html>";
    frame->LoadString(ss.str(), failedUrl);
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
    if (!CefCurrentlyOn(TID_UI)) {
        // Execute on the UI thread.
        CefPostTask(TID_UI, base::Bind(&SimpleHandler::CloseAllBrowsers, this,
                                       force_close));
        return;
    }

    if (browser_list_.empty())
        return;

    BrowserList::const_iterator it = browser_list_.begin();
    for (; it != browser_list_.end(); ++it)
        (*it)->GetHost()->CloseBrowser(force_close);
}

void SimpleHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefContextMenuParams> params,
                                        CefRefPtr<CefMenuModel> model) {
    model->Clear();

    std::cout << model.get() << std::endl;
    std::cout << model->GetCount() << std::endl;
    std::cout << "*********" << std::endl;

    //在这里，我添加了自己想要的菜单
    /*cef_context_menu_type_flags_t flag =   params->GetTypeFlags();
    if ( flag & CM_TYPEFLAG_PAGE )
    {//普通页面的右键消息
        model->SetLabel(MENU_ID_BACK, L"后退");
        model->SetLabel(MENU_ID_FORWARD, L"前进");
        model->SetLabel(MENU_ID_VIEW_SOURCE, L"查看源代码");
        model->SetLabel(MENU_ID_PRINT, L"打印");
        model->SetLabel(MENU_ID_RELOAD, L"刷新");
        model->SetLabel(MENU_ID_RELOAD_NOCACHE, L"强制刷新");
        model->SetLabel(MENU_ID_STOPLOAD, L"停止加载");
        model->SetLabel(MENU_ID_REDO, L"重复");
    }
    if ( flag & CM_TYPEFLAG_EDITABLE)
    {//编辑框的右键消息
        model->SetLabel(MENU_ID_UNDO, L"撤销");
        model->SetLabel(MENU_ID_REDO, L"重做");
        model->SetLabel(MENU_ID_CUT, L"剪切");
        model->SetLabel(MENU_ID_COPY, L"复制");
        model->SetLabel(MENU_ID_PASTE, L"粘贴");
        model->SetLabel(MENU_ID_DELETE, L"删除");
        model->SetLabel(MENU_ID_SELECT_ALL, L"全选");
    }*/

}

bool SimpleHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefRequest> request,
                                   bool user_gesture,
                                   bool is_redirect) {
    std::string url = request->GetURL();
    if (url.find("login/password") != std::string::npos) {
        browser->GetHost();
        std::cout << url << std::endl;
        CefWindowInfo window_info;
        CefBrowserSettings browser_settings;
        std::string url;
        url = "http://www.baidu.com";
        browser->GetHost()->CreateBrowser(window_info, browser->GetHost()->GetClient(), url, browser_settings, NULL);
    }

    return false;

}


