@echo off
setlocal

chcp 65001 

set src=\physics\converted\site
set dst=\physics\svg_ver

set field=analytic chemistry circuit dynamics electromag elementary fluid math quantum relativity statistic thermo

for %%a in (%field%) do (
  for %%i in (%src%\%%a\*.html) do (
    rem この次の for 文は変数 o に出力ファイル名を入れて、あとでタイムスタンプを取り出しやすくするためだけに入れているよ
    for %%o in (%dst%\%%a\%%~ni.html) do (
      if not exist %%o (
        echo %%a : %%~ni
        \physics\svg_changer %%a %%~ni noimg
      ) else if %%~ti gtr %%~to (
        echo %%a : %%~ni
        \physics\svg_changer %%a %%~ni noimg
      )
    )
  )
)

pause

