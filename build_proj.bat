call setup_env.bat
set PYTHONPATH=%PYTHONPATH%;%CD%\tools\grit
tools\gyp\gyp.bat --depth . --no-circular-check -G msvs_version=2008 -Ibuild/common.gypi build_add/all.gyp