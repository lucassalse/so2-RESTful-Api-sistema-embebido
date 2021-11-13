all: user_service
CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11 -g

user_service: user_service.c
	gcc $(CFLAGS) -o user_service user_service.c -lulfius -ljansson
	sudo cp user_service /usr/bin/
	sudo cp user_service.service /etc/systemd/system/
	sudo systemctl enable user_service.service
	sudo systemctl start user_service.service



clean:
	rm -f user_service *.o log.txt
	sudo systemctl stop user_service.service
	sudo systemctl disable user_service.service