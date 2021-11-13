
SYSDIR = ../systemd_services

all: user_service goes_service nginx_server
CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11 -g

user_service: user_service.c
	gcc $(CFLAGS) -o user_service user_service.c -lulfius -ljansson
	sudo cp user_service /usr/bin/
	#cd $(SYSDIR) ESTE ESTABA COMENTADO
	sudo cp $(SYSDIR)/user_service.service /etc/systemd/system
	sudo systemctl enable user_service.service
	sudo systemctl start user_service.service

goes_service: goes_service.o utilidades.o
	gcc $(CFLAGS) -o goes_service goes_service.o utilidades.o -lulfius -ljansson -lpthread
	sudo cp goes_service /usr/bin/
	sudo cp $(SYSDIR)/goes_service.service /etc/systemd/system
	sudo systemctl enable goes_service.service
	sudo systemctl start goes_service.service

nginx_server:
	sudo cp $(SYSDIR)/server_nginx /etc/nginx/sites-available
	sudo ln -s /etc/nginx/sites-available/server_nginx /etc/nginx/sites-enabled/server_nginx
	sudo nginx -s reload

goes_service.o: goes_service.c utilidades.h
	gcc $(CFLAGS) -c goes_service.c

utilidades.o: utilidades.c utilidades.h
	gcc $(CFLAGS) -c utilidades.c



clean:
	rm -f user_service goes_service *.o log.txt *.txt *.nc
	sudo systemctl stop user_service.service
	sudo systemctl disable user_service.service
	sudo systemctl stop goes_service.service
	sudo systemctl disable goes_service.service