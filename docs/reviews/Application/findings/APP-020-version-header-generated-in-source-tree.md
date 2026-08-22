# APP-020: Configure writes generated state into the source tree

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Build/versioning
- **Location:** [`CMakeLists.txt`](../../../../CMakeLists.txt), line 22; [`cmake/GitVersionConfig.cmake`](../../../../cmake/GitVersionConfig.cmake), lines 124-156
- **Dependencies:** APP-019

## Observation

The version header is generated at `src/Common/Version.hpp`, an ignored path inside the source tree, and included by startup code.

## Reasoning and impact

Configure mutates the checkout, and the ignored header can remain stale across branch changes or coexist across multiple build directories with different configurations. Concurrent builds race to overwrite one shared file.

## Recommended improvement

Generate the header under the build directory, expose that directory through target-scoped include paths, and declare the generated file as an Application source/dependency.

## Acceptance criteria

- Configuring leaves the source tree unchanged.
- Separate build directories produce independent version headers.
- Reconfiguration reliably refreshes the header consumed by that build.

## Suggested tests

Configure two build directories with different supplied versions and confirm each executable sees its own header while `git status` remains clean.
