# An example command file that can be run using the -f option or if renamed
# to ~/.pionrc will run automatically at startup.

cls
echo "~BM*** Example command file ***"
wait 1
# Won't connect but shows how to connect to your streamer immediately instead
# of waiting for EZPROZY packets. If this fails it'll try to listen anyway.
echo "Trying to connect..."
#on_error cont
connect 127.0.0.1
echo "~FG~LI*** CONNECTED ***"
