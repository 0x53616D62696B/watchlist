# GitVersion

This repository uses [GitVersion](https://gitversion.net/) to calculate semantic versions from Git history.

The active configuration is `GitVersion.yml` in the repository root. The CMake integration uses that file during configure and generates `src/Common/Version.hpp`.

## Version Format

The project follows [Semantic Versioning 2.0.0](https://semver.org/) with prerelease identifiers and build metadata:

```text
{major}.{minor}.{patch}-{prerelease}+{buildmetadata}
```

Examples:

```text
1.2.4-1
1.2.4-search.1
1.2.4-PullRequest123.1
1.2.4-random-random-name.1
```

- `MAJOR` changes for incompatible API changes.
- `MINOR` changes for backward-compatible new functionality.
- `PATCH` changes for backward-compatible fixes and normal mainline changes.
- `PRERELEASE` identifies untagged builds or branch builds.
- Build metadata can include branch and commit information.

## Active Workflow

The active workflow is:

```yaml
workflow: TrunkBased/preview1
```

In GitVersion 6 this enables the `Mainline` strategy. Mainline walks the commits on the main branch and applies version increments from merge commits and commit messages.

The main branch uses the trunk-based workflow default, which is `ContinuousDeployment`:

```yaml
branches:
  main:
    label: ''
    increment: Patch
```

This means `main` shows the inferred mainline version without a prerelease suffix, for example `1.2.4`. A tag still marks the stable released version explicitly, for example `v1.2.4`.

Release branches use `ContinuousDelivery` with a `beta` label:

```yaml
branches:
  release:
    mode: ContinuousDelivery
    label: beta
    increment: None
```

This is where prerelease numbers are useful. A release branch keeps the target version stable while each commit produces the next beta build, for example `1.2.4-beta.1`, `1.2.4-beta.2`, and `1.2.4-beta.3`.

## Branch Strategy

- `main` / `master`: trunk-based mainline branch, patch by default, no prerelease suffix.
- `feature/...`: continuous delivery branch with the branch name as the prerelease label.
- `release/...`: continuous delivery branch with the `beta` prerelease label and no automatic version increment.
- `pr/...`, `pull/...`, `pull-request/...`: continuous delivery branch with a `PullRequest{Number}` label.
- Any other branch name: handled by the `unknown` fallback rule.

Branch names matter because they select the branch rule in `GitVersion.yml`.

## Version Increments

`main` uses `increment: Patch`, so normal commits and merge commits request a patch bump.

Use commit messages or PR merge messages when a larger increment is needed:

- `+semver: major` or `+semver: breaking`
- `+semver: minor` or `+semver: feature`
- `+semver: patch` or `+semver: fix`
- `+semver: none` or `+semver: skip`

`+semver: patch` does not add another patch bump when the branch already increments by patch. It only requests the patch increment, which is already the default on `main`.

## Tags

Git tags are release markers and version sources.

```powershell
git tag v1.2.4
```

On the tagged commit, GitVersion reports the stable version:

```text
1.2.4
```

The next main commit then starts the next inferred mainline version:

```text
1.2.5
```

Without a tag, GitVersion continues to infer mainline versions from the latest version source and mainline history.

## Example Version Flow

Assume the latest released tag is `v1.2.3`.

Commits or merge commits on `main`:

```text
main commit: Fix typo
version: 1.2.4

main commit: Add cache +semver: minor
version: 1.3.0

main commit: Fix cache invalidation
version: 1.3.1

tag: v1.3.1
version on tagged commit: 1.3.1

next main commit: Update docs
version: 1.3.2
```

Commits on `release/*` branches:

```text
branch: release/1.3.1
commit: Prepare release notes
version: 1.3.1-beta.1

commit: Fix package metadata
version: 1.3.1-beta.2

tag: v1.3.1
version on tagged commit: 1.3.1
```

Commits on `feature/*` branches:

```text
branch: feature/search
commit: Start search screen
version: 1.2.4-search.1

commit: Add fuzzy matching +semver: minor
version: 1.3.0-search.2
```

The feature branch label comes from the part after `feature/`. The prerelease number increases automatically as more commits are added to that branch.

Commits on branches that do not match a named branch rule, such as `random/random_name`:

```text
branch: random/random_name
commit: Try local experiment
version: 1.2.4-random-random-name.1

commit: Continue experiment
version: 1.2.4-random-random-name.1
```

These branches use the `unknown` rule, which is `ManualDeployment`. The full informational version still includes branch and commit metadata, but the prerelease number is not meant to behave like a normal feature-branch build counter.

Prefer `feature/...`, `release/...`, or `pr/...` names when the branch is intended for normal project work.

## Unknown Branch Rule

The `unknown` rule is the fallback for branch names that do not match any earlier rule:

```yaml
unknown:
  mode: ManualDeployment
  label: '{BranchName}'
  increment: Inherit
  regex: (?<BranchName>.+)
```

For example, `random/random_name` does not match `main`, `feature/...`, `release/...`, or `pr/...`, so GitVersion handles it as `unknown`.

The regular expression captures the whole branch name as `BranchName`. GitVersion sanitizes it for SemVer output:

```text
random/random_name -> random-random-name
```

The fallback rule is useful because unusual branch names still get a predictable version instead of failing or behaving unpredictably.

## Source Branches

`source-branches` tells GitVersion which branch types a branch is allowed to have been created from.

For `unknown`:

```yaml
unknown:
  increment: Inherit
  source-branches:
  - main
  - release
  - feature
  - pull-request
```

Because `unknown` uses `increment: Inherit`, GitVersion needs to answer:

```text
Inherit from what?
```

`source-branches` narrows the possible answers. It tells GitVersion that an unmatched branch may have been branched from `main`, `release`, `feature`, or `pull-request`.

Example:

```powershell
git switch main
git switch -c random/test
```

`random/test` matches the `unknown` rule. GitVersion can infer that it probably came from `main`, then inherit the main branch increment behavior.

`source-branches` is about where a branch may have branched from. It is not about where the branch may merge to.

## Checking The Version

Use GitVersion directly:

```powershell
dotnet-gitversion /output json
```

Or show the effective configuration:

```powershell
dotnet-gitversion /showconfig
```

After changing branches, adding tags, or changing GitVersion configuration, re-run CMake configure so `src/Common/Version.hpp` is regenerated:

```powershell
cmake -S . -B build
```
