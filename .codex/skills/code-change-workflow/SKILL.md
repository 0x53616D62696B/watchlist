---
name: code-change-workflow
description: Prepare watchlist repo code changes. Use only when Codex is about to change source code or code-adjacent implementation files in the watchlist repo, so it asks which worktree to use, initializes submodules after worktree swaps, and invokes $branch-pr-description after each code change.
---

# Code Change Workflow

## Overview

Apply this workflow only before making code changes in the watchlist repo. It is not required for ordinary analysis, read-only review, or non-code documentation edits unless they are part of an implementation change.

## Before Code Changes

1. Ask the user which worktree to use before changing code.
2. If the user chooses a different worktree or asks to swap worktrees, switch to that worktree first.
3. If creating new worktree, place it into folder `.worktrees/<branch_name>`.
3a. Place this new worktree in currently used vscode workspace.
4. After swapping to a new worktree, initialize submodules before editing.
5. Continue only after the worktree context is clear.

## After Code Changes

Use `$branch-pr-description` to document what happened on the current branch after each code change.
