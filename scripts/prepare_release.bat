echo off
IF "%1"=="" GOTO:arg_not_exists
IF "%2"=="" GOTO:arg_not_exists
set version=%1
set versionName=%2
node update_API_version.js
node update_Plugin_version.js %version% %versionName% 
:arg_not_exists
echo Please run this command with version and versionName. For example: prepare_release.bat 46 46.0.1
