signtool.exe sign /f c:\projects\gtest-runner\resources\certificates\certum.pfx /p %PWD% C:\projects\gtest-runner\build\Release\gtest-runner-%APPVEYOR_REPO_TAG_NAME%-%EXT%.exe 
exit 0