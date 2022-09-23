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
const std::string windowTitle = "CookBook";
void ShowWindow(bool* p_open = nullptr);
static void ShowAppMainMenuBar();
static void ShowMenuFile();
void ShowAppDocuments(bool* p_open);
struct AppDocuments;
struct MyDocument;
static void NotifyOfDocumentsClosedElsewhere(MyApp::AppDocuments& app);
static void ShowDockingDisabledMessage();
}
