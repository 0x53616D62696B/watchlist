1. When updating CmakeUserPresets.json update CmakeUserPresetsExample.json the same way. This file is being tracked by git.

2. Do not update Version.hpp. It is updated automatically when cmake runs configuration.
You may rebuild, no problem that Version.hpp is changed during any build.

3. After each code change update .codex/PR_description.md with description of what have been done in current branch for Pull Request. Also create suitable title for PR.
