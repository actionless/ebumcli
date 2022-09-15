all: ebumcli-mac
ebumcli-linux:
	gcc -I /usr/include/ -l m -l ebur128 -l sndfile ebumcli.c -o ebumcli
ebumcli-mac:
	gcc -I /usr/local/Cellar/libebur128/1.2.6/include/ -l ebur128 -l sndfile ebumcli.c -o ebumcli
