@echo off
setlocal enabledelayedexpansion

REM Initialize SOURCES variable
set "SOURCES="

REM Loop through all .cpp files recursively and add to SOURCES
for /R %%f in (*.cpp) do (
    set "SOURCES=!SOURCES! %%f"
)

REM Compile all sources and include headers folder
echo Compiling all .cpp files...
g++ -std=c++17 -Wall -Iheaders !SOURCES! -o myprogram.exe

REM Check if compilation was successful
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    pause
    exit /b
)

REM Run the program
echo Running program...
myprogram.exe

pause
