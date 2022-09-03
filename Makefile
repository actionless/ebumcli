all: ebumcli
ebumcli:
	gcc -I /usr/local/Cellar/libebur128/1.2.6/include/ -l ebur128 -l sndfile ebumcli.c -o ebumcli
