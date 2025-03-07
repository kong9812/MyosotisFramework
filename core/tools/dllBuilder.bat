@echo off

:: vswhere を使って msbuild のパスを取得
for /f "usebackq tokens=*" %%i in (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set MSBUILD_PATH=%%i
)

:: ソリューションファイルのパスを設定
set SOLUTION_PATH="..\..\bin\MyosotisFW.sln"

:: ビルド構成
set CONFIGURATION=FWDLL

:: ビルドしたいプロジェクトの名前を設定
set PROJECT_NAME=myosotis

:: 出力先ディレクトリを設定
set BUILD_DIR="..\core"

:: ビルド
"%MSBUILD_PATH%" %SOLUTION_PATH% /p:Configuration=%CONFIGURATION% /p:OutDir=%BUILD_DIR%\ /t:%PROJECT_NAME%