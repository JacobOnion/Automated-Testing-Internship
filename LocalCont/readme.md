Minimum viable container to run suspicious code locally


the command to run is:

podman build -t container:latest .

podman run     --rm     -it     --security-opt label=disable     -e DISPLAY=$DISPLAY -e XAUTHORITY=/root/.Xauthority -v /tmp/.X11-unix:/tmp/.X11-unix     container:latest