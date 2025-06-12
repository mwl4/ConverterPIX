@setlocal
cd /d %~dp0

mkdir build_GDeflate || goto :error
cd build_GDeflate || goto :error
git clone https://github.com/microsoft/DirectStorage.git ./ || goto :error
git submodule init || goto :error
git submodule update || goto :error
cd GDeflate\GDeflate || goto :error

cmake -A Win32 -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -B build_debug -G "Visual Studio 15" -T "v141_xp" || goto :error
cmake --build build_debug --config Debug || goto :error

cmake -A Win32 -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -B build_release -G "Visual Studio 15" -T "v141_xp" || goto :error
cmake --build build_release --config Release || goto :error

cmake -A x64 -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -B build_debug_x64 -G "Visual Studio 15" -T "v141_xp" || goto :error
cmake --build build_debug_x64 --config Debug || goto :error

cmake -A x64 -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -B build_release_x64 -G "Visual Studio 15" -T "v141_xp" || goto :error
cmake --build build_release_x64 --config Release || goto :error

@echo Build succeeded. Built libraries you will find in build_GDeflate\GDeflate\GDeflate\build_debug\Debug and build_GDeflate\GDeflate\GDeflate\build_release\Release\
timeout /t 10

@endlocal
goto :EOF
:error
@endlocal
echo Failed with error #%errorlevel%.
pause
exit /b %errorlevel%
