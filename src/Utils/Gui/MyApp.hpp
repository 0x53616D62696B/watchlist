/*********************
 *
 * @Brief: Implementation of GUI Layouts.
 *
 * License GPL version 3 or later
 * *****
 * ****
 * ***
 * **
 * *
 */

#pragma once

namespace MyApp
{
// Windows (apps) - accessible from the "Examples" menu
struct WindowAppsFlags
{
    bool show_app_documents = true;
};

const std::string windowTitle = "CookBook";
void ShowWindow(bool* p_open = nullptr);
// static void ShowAppMainMenuBar(); // TODO: remove this fcn
static void ShowAppMenuBar(struct MyApp::WindowAppsFlags& sWindowAppsFlags);
static void ShowMenuFile();
void ShowAppDocuments(bool* p_open);
struct AppDocuments;
struct MyDocument;
static void NotifyOfDocumentsClosedElsewhere(MyApp::AppDocuments& app);
static void ShowDockingDisabledMessage();

}
