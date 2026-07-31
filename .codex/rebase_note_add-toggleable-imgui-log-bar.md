Rebase conflicts are fully resolved in the main watchlist checkout.
- Rebased three commits onto eb56456.
- Preserved hardened logger, GUI lifecycle, MQTT console, and explicit CMake targets.
- Integrated the detachable workspace with the real device monitor and MQTT UI.
- Strict MSVC build passed for Application, Client, Server, and LoggerUnitTests.
- All 116 configured tests passed.
- Tracked working tree is clean; no conflict markers or rebase metadata remain.
- Updated [PR notes](C:/Users/patrik.maraczek/Development/watchlist/.codex/PR_descriptions/codex_add-toggleable-imgui-log-bar.md).
- Aligned the ignored local CMakeUserPresets.json; preset discovery works again.
Docker/Mosquitto tests were not run. No push was performed; the rewritten branch will require a force-with-lease push.