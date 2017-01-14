@echo off
copy /y "c:\proj1\se\aaa001\proj\vc9\Release\app13.exe" "C:\proj1\se\aaa001\proj\vc9\../../testfolder/app13.exe"
if errorlevel 1 goto VCReportError
goto VCEnd
:VCReportError
echo Project : error PRJ0019: A tool returned an error code from "Performing Post-Build Event..."
exit 1
:VCEnd