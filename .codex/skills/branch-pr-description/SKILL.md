---
name: branch-pr-description
description: Document what happened on the current branch for a pull request. Use when the user asks to document, summarize, refresh, or update the PR description for the current branch, or when another workflow needs the branch-specific PR notes updated after code changes.
---

# Branch PR Description

## Overview

Update the current worktree's branch-specific PR description without mixing notes from other worktrees or branches.

## Workflow

1. Treat the current working directory as the git worktree; do not switch checkouts unless explicitly asked.
2. Get the branch with `git branch --show-current`, replace `/` or `\` with `_`, and update `.codex/PR_descriptions/<branch>.md` in this worktree.
3. Include a suitable title, concise summary of branch work, and any relevant existing notes; revise stale content.
