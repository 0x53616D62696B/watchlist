# Make version generation deterministic and release-ready

## Summary

- resolve versions from an explicit override, validated GitVersion JSON, or a deterministic development fallback
- add strict GitVersion and clean-provenance switches for release workflows
- generate escaped version headers and Windows resources only under each build directory
- link reviewed Watchlist metadata into `Watchlist.exe` while retaining the `Application` target name
- add CMake script fixtures and resolve APP-019, APP-020, APP-021, APP-047, and APP-048

## Validation

- 9/9 `VersionPipeline.*` CTest cases passed
- configured installed GitVersion, override, fallback, and strict failure paths
- configured two build directories with independent version overrides
- built the `Application` target as `Watchlist.exe` with the generated resource (`WATCHLIST_WARNINGS_AS_ERRORS=OFF` due the known cumulative nodiscard dependency)
- queried Windows `VersionInfo` for product, company, description, copyright, filename, and full version
- confirmed `src/Common/Version.hpp` remained ignored, unchanged, and absent from the Git diff
- `git diff --check`
