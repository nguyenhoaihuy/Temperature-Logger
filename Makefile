default: lab4c_tcp.c lab4c_tls.c
	gcc tcp.c -o tcp -Wall -Wextra -lm -lmraa
	gcc tls.c -o tls -Wall -Wextra -lm -lmraa -lssl -lcrypto
