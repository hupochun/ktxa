cd /d %~dp0
"G:/Qt/5.15.2/msvc2019_64/bin/lrelease.exe" ./zh_CN.ts -qm zh_CN.qm
copy /y zh_CN.qm "../build-ktxaAIQt-Replacement_for_Desktop_Qt_5_14_2_MSVC2017_64bit-Debug/debug/zh_CN.qm"
copy /y zh_CN.qm "../build-ktxaAIQt-Replacement_for_Desktop_Qt_5_14_2_MSVC2017_64bit-Release/release/zh_CN.qm"
copy /y zh_CN.qm "../bin/zh_CN.qm"

pause
