#include "MyApp.hpp"

// TODO: set clang-format option to not to format one line function calls
// clang-format off

// TODO: Remove this function
/*
static void MyApp::ShowAppMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("TODO", nullptr, false, false)) {}
            MyApp::ShowMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("TODO", nullptr, false, false)) {}
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options"))
        {
            if (ImGui::MenuItem("TODO", nullptr, false, false)) {}
            if (ImGui::MenuItem("Undo", nullptr)) {}
            if (ImGui::MenuItem("Redo", nullptr, false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", nullptr)) {}
            if (ImGui::MenuItem("Copy", nullptr)) {}
            if (ImGui::MenuItem("Paste", nullptr)) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
*/
static void MyApp::ShowAppMenuBar(struct MyApp::WindowAppsFlags& sWindowAppsFlags)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("TODO", nullptr, false, false)) {}
            MyApp::ShowMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("TODO", nullptr, false, false)) {}
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Features/Tools"))
        {
            //TODO: this add the possibility to open/reopen features window (basicly other windows) 
            if (ImGui::MenuItem("TODO", nullptr, false, false)) {}
            //std::cout << &(sWindowAppsFlags.show_app_documents) << std::endl; 
            if (ImGui::MenuItem("Vizual workplace", nullptr, &(sWindowAppsFlags.show_app_documents))) {}


            //ImGui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
            //ImGui::MenuItem("Console", NULL, &show_app_console);
            //ImGui::MenuItem("Log", NULL, &show_app_log);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options"))
        {
            if (ImGui::MenuItem("TODO", nullptr, false, false)) {}
            if (ImGui::MenuItem("Undo", nullptr)) {}
            if (ImGui::MenuItem("Redo", nullptr, false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", nullptr)) {}
            if (ImGui::MenuItem("Copy", nullptr)) {}
            if (ImGui::MenuItem("Paste", nullptr)) {}
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

static void MyApp::ShowMenuFile()
{
    //IMGUI_DEMO_MARKER("Examples/Menu");
    ImGui::MenuItem("(demo menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                MyApp::ShowMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();
    //IMGUI_DEMO_MARKER("Examples/Menu/Options");
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    //IMGUI_DEMO_MARKER("Examples/Menu/Colors");
    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (ImGui::BeginMenu("Options")) // <-- Append!
    {
        //IMGUI_DEMO_MARKER("Examples/Menu/Append to an existing menu");
        static bool b = true;
        ImGui::Checkbox("SomeOption", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}


// TODO: create generic class from this function to reuse it with only few changes
void MyApp::ShowWindow(bool* p_open)
{
    // LOG_DEBUG("Running my app layout");

    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");

    static struct MyApp::WindowAppsFlags sWindowAppsFlags;    

    //if (show_app_main_menu_bar)     MyApp::ShowAppMenuBar();
    if (sWindowAppsFlags.show_app_documents)         MyApp::ShowAppDocuments(&sWindowAppsFlags.show_app_documents);

    // Demonstrate the various window flags. Typically you would just use the default!
    static bool no_titlebar = false;
    static bool no_scrollbar = true;
    static bool no_menu = false;
    static bool no_move = false;
    static bool no_resize = false;
    static bool no_collapse = false;
    static bool no_close = false;
    static bool no_nav = false;
    static bool no_background = false;
    static bool no_bring_to_front = false;
    static bool no_docking = false;
    static bool unsaved_document = false;

    ImGuiWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
    if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
    if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
    if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (no_docking)         window_flags |= ImGuiWindowFlags_NoDocking;
    if (unsaved_document)   window_flags |= ImGuiWindowFlags_UnsavedDocument;
    if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

/*
    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
*/

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("CookBook", p_open, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

    // Menu Bar
    MyApp::ShowAppMenuBar(sWindowAppsFlags);
    /*
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            //IMGUI_DEMO_MARKER("Menu/File");
            MyApp::ShowMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Examples"))
        {
            //IMGUI_DEMO_MARKER("Menu/Examples");
            ImGui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
            //ImGui::MenuItem("Console", NULL, &show_app_console);
            //ImGui::MenuItem("Log", NULL, &show_app_log);
            ImGui::EndMenu();
        }
        //if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
        if (ImGui::BeginMenu("Tools"))
        {
        
        // TODO: Disable debug tools!
  #ifndef IMGUI_DISABLE_DEBUG_TOOLS
            const bool has_debug_tools = true;
  #else
            const bool has_debug_tools = false;
  #endif
            //ImGui::MenuItem("Metrics/Debugger", NULL, &show_app_metrics, has_debug_tools);
            //ImGui::MenuItem("Debug Log", NULL, &show_app_debug_log, has_debug_tools);
            //ImGui::MenuItem("Stack Tool", NULL, &show_app_stack_tool, has_debug_tools);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    */

    ImGui::Text("dear imgui says hello! (%s) (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
    ImGui::Spacing();


    if (ImGui::CollapsingHeader("Help"))
    {
        ImGui::Text("ABOUT THIS DEMO:");
        ImGui::BulletText("Sections below are demonstrating many aspects of the library.");
        ImGui::BulletText("The \"Examples\" menu above leads to more demo contents.");
        ImGui::BulletText("The \"Tools\" menu above gives access to: About Box, Style Editor,\n"
                          "and Metrics/Debugger (general purpose Dear ImGui debugging tool).");
        ImGui::Separator();

        ImGui::Text("PROGRAMMER GUIDE:");
        ImGui::BulletText("See the ShowDemoWindow() code in imgui_demo.cpp. <- you are here!");
        ImGui::BulletText("See comments in imgui.cpp.");
        ImGui::BulletText("See example applications in the examples/ folder.");
        ImGui::BulletText("Read the FAQ at http://www.dearimgui.org/faq/");
        ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
        ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");
        ImGui::Separator();

        ImGui::Text("USER GUIDE:");
        ImGui::ShowUserGuide();
    }

    // All demo contents
    //ShowDemoWindowWidgets();
    //ShowDemoWindowLayout();
    //ShowDemoWindowPopups();
    //ShowDemoWindowTables();
    //ShowDemoWindowMisc();

    // End of ShowDemoWindow()
    ImGui::PopItemWidth();
    ImGui::End();


}


// Simplified structure to mimic a Document model
struct MyApp::MyDocument
{
    const char* Name;       // Document title
    bool        Open;       // Set when open (we keep an array of all available documents to simplify demo code!)
    bool        OpenPrev;   // Copy of Open from last update.
    bool        Dirty;      // Set when the document has been modified
    bool        WantClose;  // Set when the document
    ImVec4      Color;      // An arbitrary variable associated to the document

    MyApp::MyDocument(const char* name, bool open = true, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
    {
        Name = name;
        Open = OpenPrev = open;
        Dirty = false;
        WantClose = false;
        Color = color;
    }
    void DoOpen()       { Open = true; }
    void DoQueueClose() { WantClose = true; }
    void DoForceClose() { Open = false; Dirty = false; }
    void DoSave()       { Dirty = false; }

    // Display placeholder contents for the Document
    static void DisplayContents(MyApp::MyDocument* doc)
    {
        ImGui::PushID(doc);
        ImGui::Text("Document \"%s\"", doc->Name);
        ImGui::PushStyleColor(ImGuiCol_Text, doc->Color);
        ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
        ImGui::PopStyleColor();
        if (ImGui::Button("Modify", ImVec2(100, 0)))
            doc->Dirty = true;
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(100, 0)))
            doc->DoSave();
        ImGui::ColorEdit3("color", &doc->Color.x);  // Useful to test drag and drop and hold-dragged-to-open-tab behavior.
        ImGui::PopID();
    }

    // Display context menu for the Document
    static void DisplayContextMenu(MyApp::MyDocument* doc)
    {
        if (!ImGui::BeginPopupContextItem())
            return;

        char buf[256];
        sprintf(buf, "Save %s", doc->Name);
        if (ImGui::MenuItem(buf, "CTRL+S", false, doc->Open))
            doc->DoSave();
        if (ImGui::MenuItem("Close", "CTRL+W", false, doc->Open))
            doc->DoQueueClose();
        ImGui::EndPopup();
    }
};

struct MyApp::AppDocuments
{
    ImVector<MyApp::MyDocument> Documents;

    MyApp::AppDocuments()
    {
        Documents.push_back(MyApp::MyDocument("Lettuce",             true,  ImVec4(0.4f, 0.8f, 0.4f, 1.0f)));
        Documents.push_back(MyApp::MyDocument("Eggplant",            true,  ImVec4(0.8f, 0.5f, 1.0f, 1.0f)));
        Documents.push_back(MyApp::MyDocument("Carrot",              true,  ImVec4(1.0f, 0.8f, 0.5f, 1.0f)));
        Documents.push_back(MyApp::MyDocument("Tomato",              false, ImVec4(1.0f, 0.3f, 0.4f, 1.0f)));
        Documents.push_back(MyApp::MyDocument("A Rather Long Title", false));
        Documents.push_back(MyApp::MyDocument("Some Document",       false));
    }
};

void MyApp::ShowAppDocuments(bool* p_open)
{
    static MyApp::AppDocuments app;

    // Options
    enum Target
    {
        Target_None,
        Target_Tab,                 // Create documents as local tab into a local tab bar
        Target_DockSpaceAndWindow   // Create documents as regular windows, and create an embedded dockspace
    };
    static Target opt_target = Target_Tab;
    static bool opt_reorderable = true;
    static ImGuiTabBarFlags opt_fitting_flags = ImGuiTabBarFlags_FittingPolicyDefault_;

    // When (opt_target == Target_DockSpaceAndWindow) there is the possibily that one of our child Document window (e.g. "Eggplant")
    // that we emit gets docked into the same spot as the parent window ("Example: Documents").
    // This would create a problematic feedback loop because selecting the "Eggplant" tab would make the "Example: Documents" tab
    // not visible, which in turn would stop submitting the "Eggplant" window.
    // We avoid this problem by submitting our documents window even if our parent window is not currently visible.
    // Another solution may be to make the "Example: Documents" window use the ImGuiWindowFlags_NoDocking.

    bool window_contents_visible = ImGui::Begin("Example: Documents", p_open, ImGuiWindowFlags_MenuBar);
    if (!window_contents_visible && opt_target != Target_DockSpaceAndWindow)
    {
        ImGui::End();
        return;
    }

    // Menu
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            int open_count = 0;
            for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
                open_count += app.Documents[doc_n].Open ? 1 : 0;

            if (ImGui::BeginMenu("Open", open_count < app.Documents.Size))
            {
                for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
                {
                    MyApp::MyDocument* doc = &app.Documents[doc_n];
                    if (!doc->Open)
                        if (ImGui::MenuItem(doc->Name))
                            doc->DoOpen();
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Close All Documents", NULL, false, open_count > 0))
                for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
                    app.Documents[doc_n].DoQueueClose();
            if (ImGui::MenuItem("Exit", "Ctrl+F4") && p_open)
                *p_open = false;
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // [Debug] List documents with one checkbox for each
    for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
    {
        MyApp::MyDocument* doc = &app.Documents[doc_n];
        if (doc_n > 0)
            ImGui::SameLine();
        ImGui::PushID(doc);
        if (ImGui::Checkbox(doc->Name, &doc->Open))
            if (!doc->Open)
                doc->DoForceClose();
        ImGui::PopID();
    }
    ImGui::PushItemWidth(ImGui::GetFontSize() * 12);
    ImGui::Combo("Output", (int*)&opt_target, "None\0TabBar+Tabs\0DockSpace+Window\0");
    ImGui::PopItemWidth();
    bool redock_all = false;
    if (opt_target == Target_Tab)                { ImGui::SameLine(); ImGui::Checkbox("Reorderable Tabs", &opt_reorderable); }
    if (opt_target == Target_DockSpaceAndWindow) { ImGui::SameLine(); redock_all = ImGui::Button("Redock all"); }

    ImGui::Separator();

    // About the ImGuiWindowFlags_UnsavedDocument / ImGuiTabItemFlags_UnsavedDocument flags.
    // They have multiple effects:
    // - Display a dot next to the title.
    // - Tab is selected when clicking the X close button.
    // - Closure is not assumed (will wait for user to stop submitting the tab).
    //   Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    //   We need to assume closure by default otherwise waiting for "lack of submission" on the next frame would leave an empty
    //   hole for one-frame, both in the tab-bar and in tab-contents when closing a tab/window.
    //   The rarely used SetTabItemClosed() function is a way to notify of programmatic closure to avoid the one-frame hole.

    // Tabs
    if (opt_target == Target_Tab)
    {
        ImGuiTabBarFlags tab_bar_flags = (opt_fitting_flags) | (opt_reorderable ? ImGuiTabBarFlags_Reorderable : 0);
        if (ImGui::BeginTabBar("##tabs", tab_bar_flags))
        {
            if (opt_reorderable)
                MyApp::NotifyOfDocumentsClosedElsewhere(app);

            // [DEBUG] Stress tests
            //if ((ImGui::GetFrameCount() % 30) == 0) docs[1].Open ^= 1;            // [DEBUG] Automatically show/hide a tab. Test various interactions e.g. dragging with this on.
            //if (ImGui::GetIO().KeyCtrl) ImGui::SetTabItemSelected(docs[1].Name);  // [DEBUG] Test SetTabItemSelected(), probably not very useful as-is anyway..

            // Submit Tabs
            for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
            {
                MyApp::MyDocument* doc = &app.Documents[doc_n];
                if (!doc->Open)
                    continue;

                ImGuiTabItemFlags tab_flags = (doc->Dirty ? ImGuiTabItemFlags_UnsavedDocument : 0);
                bool visible = ImGui::BeginTabItem(doc->Name, &doc->Open, tab_flags);

                // Cancel attempt to close when unsaved add to save queue so we can display a popup.
                if (!doc->Open && doc->Dirty)
                {
                    doc->Open = true;
                    doc->DoQueueClose();
                }

                MyApp::MyDocument::DisplayContextMenu(doc);
                if (visible)
                {
                    MyApp::MyDocument::DisplayContents(doc);
                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }
    }
    else if (opt_target == Target_DockSpaceAndWindow)
    {
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            MyApp::NotifyOfDocumentsClosedElsewhere(app);

            // Create a DockSpace node where any window can be docked
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id);

            // Create Windows
            for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
            {
                MyApp::MyDocument* doc = &app.Documents[doc_n];
                if (!doc->Open)
                    continue;

                ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
                ImGuiWindowFlags window_flags = (doc->Dirty ? ImGuiWindowFlags_UnsavedDocument : 0);
                bool visible = ImGui::Begin(doc->Name, &doc->Open, window_flags);

                // Cancel attempt to close when unsaved add to save queue so we can display a popup.
                if (!doc->Open && doc->Dirty)
                {
                    doc->Open = true;
                    doc->DoQueueClose();
                }

                MyApp::MyDocument::DisplayContextMenu(doc);
                if (visible)
                    MyApp::MyDocument::DisplayContents(doc);

                ImGui::End();
            }
        }
        else
        {
            MyApp::ShowDockingDisabledMessage();
        }
    }

    // Early out other contents
    if (!window_contents_visible)
    {
        ImGui::End();
        return;
    }

    // Update closing queue
    static ImVector<MyApp::MyDocument*> close_queue;
    if (close_queue.empty())
    {
        // Close queue is locked once we started a popup
        for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
        {
            MyApp::MyDocument* doc = &app.Documents[doc_n];
            if (doc->WantClose)
            {
                doc->WantClose = false;
                close_queue.push_back(doc);
            }
        }
    }

    // Display closing confirmation UI
    if (!close_queue.empty())
    {
        int close_queue_unsaved_documents = 0;
        for (int n = 0; n < close_queue.Size; n++)
            if (close_queue[n]->Dirty)
                close_queue_unsaved_documents++;

        if (close_queue_unsaved_documents == 0)
        {
            // Close documents when all are unsaved
            for (int n = 0; n < close_queue.Size; n++)
                close_queue[n]->DoForceClose();
            close_queue.clear();
        }
        else
        {
            if (!ImGui::IsPopupOpen("Save?"))
                ImGui::OpenPopup("Save?");
            if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Save change to the following items?");
                float item_height = ImGui::GetTextLineHeightWithSpacing();
                if (ImGui::BeginChildFrame(ImGui::GetID("frame"), ImVec2(-FLT_MIN, 6.25f * item_height)))
                {
                    for (int n = 0; n < close_queue.Size; n++)
                        if (close_queue[n]->Dirty)
                            ImGui::Text("%s", close_queue[n]->Name);
                    ImGui::EndChildFrame();
                }

                ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
                if (ImGui::Button("Yes", button_size))
                {
                    for (int n = 0; n < close_queue.Size; n++)
                    {
                        if (close_queue[n]->Dirty)
                            close_queue[n]->DoSave();
                        close_queue[n]->DoForceClose();
                    }
                    close_queue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("No", button_size))
                {
                    for (int n = 0; n < close_queue.Size; n++)
                        close_queue[n]->DoForceClose();
                    close_queue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", button_size))
                {
                    close_queue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }

    ImGui::End();
}

// [Optional] Notify the system of Tabs/Windows closure that happened outside the regular tab interface.
// If a tab has been closed programmatically (aka closed from another source such as the Checkbox() in the demo,
// as opposed to clicking on the regular tab closing button) and stops being submitted, it will take a frame for
// the tab bar to notice its absence. During this frame there will be a gap in the tab bar, and if the tab that has
// disappeared was the selected one, the tab bar will report no selected tab during the frame. This will effectively
// give the impression of a flicker for one frame.
// We call SetTabItemClosed() to manually notify the Tab Bar or Docking system of removed tabs to avoid this glitch.
// Note that this completely optional, and only affect tab bars with the ImGuiTabBarFlags_Reorderable flag.
static void MyApp::NotifyOfDocumentsClosedElsewhere(MyApp::AppDocuments& app)
{
    for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
    {
        MyApp::MyDocument* doc = &app.Documents[doc_n];
        if (!doc->Open && doc->OpenPrev)
            ImGui::SetTabItemClosed(doc->Name);
        doc->OpenPrev = doc->Open;
    }
}


static void MyApp::ShowDockingDisabledMessage()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
    ImGui::Text("Set io.ConfigFlags |= ImGuiConfigFlags_DockingEnable in your code, or ");
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::SmallButton("click here"))
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

// clang-format on