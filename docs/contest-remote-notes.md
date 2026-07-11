# Contest 远程仓库操作备忘

## 远程仓库地址

```
https://gitlab.eduxiji.net/T2026102462011149/compiler2026-vortex.git
```

## 凭据

- **用户名：** `T2026102462011149@eduxiji.net`
- **密码：** `Hzh187_0178`

> 已配置 `git config --global credential.helper store`，凭据已存入 `~/.git-credentials`，后续操作无需再次输入。

## 推送

| 场景 | 命令 |
|------|------|
| 推送当前分支到远程同名分支 | `git push contest` |
| 推送 local_branch 到远程 remote_branch | `git push contest local_branch:remote_branch` |
| 强制推送（覆盖远程分支） | `git push contest local_branch:remote_branch --force` |
| 推送 contest 分支到远程 main 分支 | `git push contest contest:main` |
| 强制推送 contest 到远程 main | `git push contest contest:main --force` |

### 拉取

| 场景 | 命令 |
|------|------|
| 拉取远程当前分支更新 | `git pull contest` |
| 拉取远程 main 分支 | `git pull contest main` |

### 查看远程仓库

```bash
git remote -v
git branch -a
```
