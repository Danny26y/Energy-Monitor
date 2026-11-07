# GitHub handoff

The local repository is rooted in `EnergyMonitorPrototype`, not its parent folder. Remote `origin` points to `https://github.com/Danny26y/Energy-Monitor.git`.

After the prepared local commit, publish from PowerShell:

```powershell
cd "C:\Users\Daniel\Documents\ChatGPT\My_Engineering\EnergyMonitorPrototype"
git status
git push -u origin main
```

Complete GitHub authentication yourself if prompted. No push was performed by the assistant.

If GitHub rejects the push because the remote already contains commits, do not force-push. First inspect them:

```powershell
git fetch origin
git log --oneline --graph --decorate --all
```

Then merge the remote history deliberately (and resolve any conflicts) before pushing. If the remote was initialized separately with a README, merging may require `git merge origin/main --allow-unrelated-histories`; inspect the remote branch name first.

For later edits:

```powershell
git add .
git diff --cached --stat
git commit -m "Describe the change"
git push
```

The repository includes the native schematic, current firmware source, normal simulation HEX, screenshots and documentation. Proteus workspace files, original/automatic backup copies, release ZIPs and historical test builds remain local and ignored. The required third-party Arduino Proteus model is not bundled; no license is granted for it by this repository. No project license has been chosen yet.
