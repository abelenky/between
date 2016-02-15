@echo off

call :RunTest 1 ::Libraries         :: Simple search between Opening marker and ending marker.
call :RunTest 2 ::Libraries         :: Opening marker, but no ending marker.
call :RunTest 3 ::Missing           :: No Marker at all
call :RunTest 4 ::Multiple          :: Marker appears multiple times.

call :TestLines 1 -l0 -l4           :: Lines starting at the beginning.
call :TestLines 2 -l1 -l4           :: Exclude the first line
call :TestLines 3 -l4 -l1           :: Lines specificed in reverse
call :TestLines 4 -l-5 -l10         :: Negative Value Test

pause
exit /B

:RunTest	
between %2 Tests\Test%1.txt > Result%1.txt
fc Result%1.txt Tests\Answer%1.txt > NUL
if %errorlevel% neq 0 (echo Test %1 ^(File^) Failed && exit /B)
del Result%1.txt

between %2 < Tests\Test%1.txt > Result%1.txt
fc Result%1.txt Tests\Answer%1.txt > NUL
if %errorlevel% neq 0 (echo "Test %1 ^(std::in^) Failed" && exit /B)
del Result%1.txt

type Tests\Test%1.txt | between %2 > Result%1.txt
fc Result%1.txt Tests\Answer%1.txt > NUL
if %errorlevel% neq 0 (echo "Test %1 ^(pipe^) Failed" && exit /B)
del Result%1.txt

echo Test %1 Passed
exit /B




:TestLines
between %2 %3 Tests\LineTest%1.txt > Result%1.txt
fc Result%1.txt Tests\LineAnswer%1.txt > NUL
if %errorlevel% neq 0 (echo "LineTest %1 ^(File^) Failed" && exit /B)
del Result%1.txt

between %2 %3 < Tests\LineTest%1.txt > Result%1.txt
fc Result%1.txt Tests\LineAnswer%1.txt > NUL
if %errorlevel% neq 0 (echo "LineTest %1 ^(std::in^) Failed" && exit /B)
del Result%1.txt

type Tests\LineTest%1.txt | between %2 %3 > Result%1.txt
fc Result%1.txt Tests\LineAnswer%1.txt > NUL
if %errorlevel% neq 0 (echo "LineTest %1 ^(File^) Failed" && exit /B)
del Result%1.txt

echo LineTest %1 Passed
exit /B
