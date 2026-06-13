---
name: branch-pr-description
description: Document what happened on the current branch for a pull request. Use when the user asks to document, summarize, refresh, or update the PR description for the current branch, or when another workflow needs the branch-specific PR notes updated after code changes.
---

# Branch PR Description

## Overview

Update the branch-specific PR description file for the current branch. Keep the notes scoped to the current branch so work from different branches is not mixed.

## Workflow

1. Determine the current branch with `git branch --show-current`.
2. Convert `/` or `\` in the branch name to `_` for the PR description filename.
3. Update `.codex/PR_descriptions/<branch>.md`.
4. Include a suitable PR title.
5. Add a concise description of what has been done on the current branch.
6. Preserve relevant existing branch notes, and revise stale content when the current branch story has changed.
