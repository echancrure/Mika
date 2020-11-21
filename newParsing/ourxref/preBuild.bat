win_flex --wincompat ourxref.l
IF  %ERRORLEVEL% NEQ 0 (
	ECHO "win_flex failed"
	EXIT 1
	)
win_bison -d ourxref.y
IF  %ERRORLEVEL% NEQ 0 (
	ECHO "win_bison failed"
	EXIT 2
	)
EXIT 0