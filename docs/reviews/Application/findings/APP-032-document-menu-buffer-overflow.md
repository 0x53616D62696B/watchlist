# APP-032: Document menu formatting can overflow a fixed buffer

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Status:** Resolved
- **Subsystem:** GUI
- **Location:** [`src/Gui/MyApp.cpp`](../../../../src/Gui/MyApp.cpp), lines 355-368
- **Dependencies:** APP-033

## Observation

`DisplayContextMenu` formats `doc->Name` into `char buf[256]` with unbounded `sprintf`.

## Reasoning and impact

Any document name longer than the remaining buffer capacity writes past the stack buffer. Current sample names are short, but the document abstraction is intended to evolve toward application data, where names are not inherently bounded.

## Recommended improvement

Avoid the intermediate C buffer by using `std::format`/`std::string`, or use a bounded formatter and a documented maximum validated at the domain boundary.

## Acceptance criteria

- Arbitrarily long valid names cannot write outside owned memory.
- The displayed label is either complete or predictably truncated.
- No format-string interpretation comes from document data.

## Suggested tests

Render context menus for empty, 255-byte, 256-byte, multi-kilobyte, and UTF-8 names under ASan.

## Resolution and validation

The demo document/context-menu implementation and its unbounded `sprintf` buffer were removed from the production target. Device labels are held in `std::string` and passed to ImGui as data through `TextUnformatted` or fixed format strings, so device-provided text is neither copied into a fixed buffer nor interpreted as a format string. The Application builds cleanly with MSVC warnings treated as errors.
