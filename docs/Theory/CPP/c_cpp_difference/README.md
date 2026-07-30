# C and C++ Differences

This note set explains the main practical differences between C and C++, then lists what to check when moving code in either direction.

Files:

- [main-differences.md](main-differences.md): language and ecosystem differences.
- [porting-c-to-cpp.md](porting-c-to-cpp.md): checklist for compiling or redesigning C code as C++.
- [porting-cpp-to-c.md](porting-cpp-to-c.md): checklist for translating C++ code back to C.
- [quick-reference.md](quick-reference.md): compact comparison table and common gotchas.

Use these as engineering checklists, not as strict rules. C++ can compile much C-like code, but good C++ usually changes ownership, error handling, initialization, and abstraction style. C can represent many C++ ideas, but usually requires explicit lifetime management and manual interface design.
