@echo off
setlocal
cd /d "%~dp0"

echo ============================================================
echo   推送 Compositor 中文版源码到你的 GitHub 仓库
echo ============================================================
echo.
echo  提示：请先在 https://github.com/new 新建一个空的仓库，
echo        然后把仓库地址（以 .git 结尾）粘贴到下面。
echo.
set /p REPO=请粘贴你的仓库地址:
if "%REPO%"=="" goto cancel

echo.
echo [1/4] 配置远端 origin ...
git remote remove origin >nul 2>&1
git remote add origin "%REPO%"

echo [2/4] 设置分支为 main ...
git branch -M main

echo [3/4] 正在推送到 GitHub，首次会弹出登录窗口，请按提示授权 ...
git push -u origin main
if errorlevel 1 goto failed

echo.
echo [4/4] 推送成功。
echo 请打开你的 GitHub 仓库，进入 Actions 页面等待编译完成，
echo 然后在页面底部 Artifacts 下载 Compositor-zh-CN-windows-x64。
goto done

:cancel
echo.
echo 未输入地址，已取消。
goto done

:failed
echo.
echo ------------------------------------------------------------
echo 推送失败，常见原因：
echo   1 本机未安装 Git，或未登录 GitHub
echo   2 仓库地址填错
echo   3 目标仓库不是空仓库，里面已经有文件
echo 可到 https://git-scm.com/download/win 安装 Git 后重试
echo ------------------------------------------------------------
goto done

:done
echo.
pause
