#include "src/Gui/ApplicationLogWindow.hpp"
#include "src/Gui/Workspace/DetachableAppWorkspace.hpp"

#include "src/Utils/Logger/Logger.hpp"

#include <imgui.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <format>
#include <string>
#include <utility>
#include <vector>

namespace
{
constexpr std::size_t MaxDisplayedEntries = 10'000;

struct SeverityDisplay
{
    LogLevel level;
    char const* name;
    ImVec4 color;
};

struct DisplayEntry
{
    LogEntry entry;
    std::vector<std::string> lines;
};

struct DisplayRow
{
    std::size_t entryIndex;
    std::size_t lineIndex;
};

constexpr std::array SeverityDisplays{
    SeverityDisplay{LogLevel::Fatal, "Fatal", ImVec4(1.0f, 0.2f, 0.2f, 1.0f)},
    SeverityDisplay{LogLevel::Error, "Error", ImVec4(1.0f, 0.35f, 0.35f, 1.0f)},
    SeverityDisplay{LogLevel::Warning, "Warning", ImVec4(1.0f, 0.85f, 0.25f, 1.0f)},
    SeverityDisplay{LogLevel::Info, "Info", ImVec4(1.0f, 1.0f, 1.0f, 1.0f)},
    SeverityDisplay{LogLevel::Debug, "Debug", ImVec4(0.3f, 0.9f, 1.0f, 1.0f)},
    SeverityDisplay{LogLevel::Trace, "Trace", ImVec4(0.65f, 0.65f, 0.65f, 1.0f)},
};

std::size_t SeverityIndex(LogLevel const level)
{
    for (std::size_t index = 0; index < SeverityDisplays.size(); ++index)
    {
        if (SeverityDisplays[index].level == level)
            return index;
    }
    return SeverityDisplays.size() - 1;
}

std::vector<std::string> FormatLogEntryLines(LogEntry const& entry)
{
    auto const formattedEntry = std::format(
        "[{}] {} | {} | {}", static_cast<char>(entry.level), entry.timestamp, entry.source, entry.message);

    std::vector<std::string> lines;
    std::size_t lineStart = 0;
    while (lineStart <= formattedEntry.size())
    {
        auto const lineEnd = formattedEntry.find('\n', lineStart);
        if (lineEnd == std::string::npos)
        {
            lines.push_back(formattedEntry.substr(lineStart));
            break;
        }

        lines.push_back(formattedEntry.substr(lineStart, lineEnd - lineStart));
        lineStart = lineEnd + 1;
    }
    return lines;
}
}

void Watchlist::Gui::ShowApplicationLog(bool* p_open)
{
    static std::deque<DisplayEntry> displayedEntries;
    static std::uint64_t lastSequence = 0;
    static std::array<bool, SeverityDisplays.size()> severityEnabled{true, true, true, true, true, true};
    static bool autoScroll = true;

    auto newEntries = GetLogEntriesSince(lastSequence);
    bool const receivedNewEntries = !newEntries.empty();
    if (receivedNewEntries)
    {
        lastSequence = newEntries.back().sequence;
        for (auto& entry : newEntries)
        {
            auto lines = FormatLogEntryLines(entry);
            displayedEntries.push_back(DisplayEntry{std::move(entry), std::move(lines)});
        }
        while (displayedEntries.size() > MaxDisplayedEntries)
            displayedEntries.pop_front();
    }

    if (!ImGui::Begin(Watchlist::Gui::ApplicationConsoleWindowName, p_open))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear"))
    {
        ClearLogEntries();
        displayedEntries.clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &autoScroll);
    ImGui::SameLine();
    ImGui::TextUnformatted("Filters:");

    for (std::size_t index = 0; index < SeverityDisplays.size(); ++index)
    {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, SeverityDisplays[index].color);
        ImGui::Checkbox(SeverityDisplays[index].name, &severityEnabled[index]);
        ImGui::PopStyleColor();
    }

    std::vector<DisplayRow> visibleRows;
    visibleRows.reserve(displayedEntries.size());
    for (std::size_t index = 0; index < displayedEntries.size(); ++index)
    {
        auto const& displayedEntry = displayedEntries[index];
        if (!severityEnabled[SeverityIndex(displayedEntry.entry.level)])
            continue;

        for (std::size_t lineIndex = 0; lineIndex < displayedEntry.lines.size(); ++lineIndex)
            visibleRows.push_back(DisplayRow{index, lineIndex});
    }

    ImGui::Separator();
    if (ImGui::BeginChild("ApplicationLogEntries", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        bool const wasAtBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(visibleRows.size()));
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                auto const displayRow = visibleRows[static_cast<std::size_t>(row)];
                auto const& displayedEntry = displayedEntries[displayRow.entryIndex];
                ImGui::PushStyleColor(
                    ImGuiCol_Text, SeverityDisplays[SeverityIndex(displayedEntry.entry.level)].color);
                ImGui::TextUnformatted(displayedEntry.lines[displayRow.lineIndex].c_str());
                ImGui::PopStyleColor();
            }
        }

        if (autoScroll && wasAtBottom && receivedNewEntries && !visibleRows.empty())
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::End();
}
