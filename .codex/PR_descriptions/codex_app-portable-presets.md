# Make tracked CMake presets portable

## Summary

- remove the mandatory `CMakeUserPresets.json` configure gate
- expose portable tracked configure, build, and test presets for default and Tracy builds
- retain optional machine-specific MSVC overrides under non-conflicting `local-*` names
- update setup guidance and mark APP-018 resolved with validation evidence

## Validation

- `cmake --list-presets=all` without `CMakeUserPresets.json`
- `cmake --fresh --preset default` with installed Ninja/MSVC tools active in the environment
- `cmake --build --preset default --target help`
- loaded a minimal inherited user override and inspected its resolved variables
- copied `CMakeUserPresetsExample.json` to the ignored user file and listed all tracked and local presets together
- confirmed absent Ninja/compiler tools produce standard CMake discovery diagnostics
- `git diff --check`
