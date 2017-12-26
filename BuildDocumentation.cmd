doxygen Doxyfile
call D:"\Program Files\nodejs\nodevars.bat"
call apidoc -o docs/html/garfieldclarendon.github.io/apidoc/ -i LCSServer/ -f ".*.h$"
Pause
