copy /Y "Documents\LCSOverview.pdf" "docs\html\garfieldclarendon.github.io\html\Documents"
D:\"Program Files"\doxygen\bin\doxygen Doxyfile
call D:"\Program Files\nodejs\nodevars.bat"
call apidoc -o docs/html/garfieldclarendon.github.io/apidoc/ -i LCSServer/ -f ".*.h$"
Pause
