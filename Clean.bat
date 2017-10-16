@forfiles /s /m *.d /c "cmd /c del @file"
@forfiles /s /m *.lst /c "cmd /c del @file"
@forfiles /s /m *.dep /c "cmd /c del @file"
@forfiles /s /m *.uvgui* /c "cmd /c del @file"
@forfiles /s /m *.plg /c "cmd /c del @file"
@forfiles /s /m *.sct /c "cmd /c del @file"
@forfiles /s /m *.map /c "cmd /c del @file"
@forfiles /s /m *.crf /c "cmd /c del @file"
@forfiles /s /m *.bak /c "cmd /c del @file"
@forfiles /s /m *.scr /c "cmd /c del @file"
@forfiles /s /m *.scvd /c "cmd /c del @file"

@forfiles /s /m JLinkLog.txt /c "cmd /c del @file"
@forfiles /s /m JLinkSettings.ini /c "cmd /c del @file"

for %%i in (
Output DebugConfig
) do (
if exist %%i\*.* del %%i\*.* /Q
)
