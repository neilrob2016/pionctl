# An example command file that can be run using the -f option or if renamed
# to ~/.pionrc will run automatically at startup.

#on_error halt
cls
echo "~BM*** Example command file ***"

# Skip over connected message and return on fail
on_error skip 2

# Don't print standard error message
on_error noprint

echo "Trying 127.0.0.1 ..."
timeout 2
#connect  127.0.0.1
connect 
echo "~FG~LI*** CONNECTED ***"

# Exit this command file and any command sequence, macro or file that called it
#halt

# Only exit this command file
return

echo "~FR~LIFAILED"
echo "Trying 127.0.0.2 ..."
connect 127.0.0.2
echo "~FG~LI*** CONNECTED ***"
return

echo "~FR~LIFAILED"
