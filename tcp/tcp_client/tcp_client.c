#include <windows.h>
#include <stdio.h>
#include <winsock.h>	//подключение библиотеки Winsock
char buffer[1024];	//глобальный буфер для приема данных
int main(int argc,char** argv) {
	struct sockaddr_in SockAddrServer;	//структура для хранения сетевого адреса сервера
	SOCKET SockData=INVALID_SOCKET;	//сокет для подключения и обмена данными
	struct hostent* pDNStoIP;	//для преобразования имени в IP
	int port,size_msg,i,pos_buffer=0;
	WSADATA WSAData;
	WORD wWSAVer;
	if (argc<3) {	//если недостаточно аргументов (IP адрес и номер порта)
		puts("=== At least 2 arguments ===");	//вывод сообщения о типе ошибки
		puts("tcp_client.exe <ip/name> <port> [<message1> <message2> ...]"); //вывод формата консольной команды при запуске
		return -1;	//завершение работы программа
	}
	wWSAVer=MAKEWORD(1,1);
	if (WSAStartup(wWSAVer,&WSAData)!=0) {	//инициализация библиотеки Winsock. Если неудача, то
		puts("=== Error of initialization WinSocket ===");	//вывод ошибки
		return -1;	//и завершение программы
	}
	memset(&SockAddrServer,0,sizeof(SockAddrServer));	//очистка структуры адреса сервера
	SockAddrServer.sin_family=AF_INET;	//использование протокола IPv4
	if (strcmp(argv[1],"255.255.255.255")==0)	//если адрес широковещательный
		SockAddrServer.sin_addr.S_un.S_addr=INADDR_BROADCAST;	//записываем это в структуру
	else {	//в противном случае
		SockAddrServer.sin_addr.S_un.S_addr=inet_addr(argv[1]);	//получение IP адреса и аргумента консоли
		if (SockAddrServer.sin_addr.S_un.S_addr==INADDR_NONE) {	//если значение IP адреса не подходит
			if ((pDNStoIP=gethostbyname(argv[1]))==NULL) {	//получаем IP через службу DNS. Если неудача, то
				fprintf(stderr,"=== Host not found: %s ===\n",argv[1]);	//сообщение об ошибки
				return -1;	//и завершение работы программы
			}
			SockAddrServer.sin_addr=*(struct in_addr*)pDNStoIP->h_addr_list[0];	//из полученных IP адресов берем самый первый
		}
	}
	if (sscanf(argv[2],"%u",&port)<1) {	//извлекаем значение порта из консоли. Если неверный формат, то
		fprintf(stderr,"=== Error number of port: %s ===\n",argv[2]);	//выводим сообщение об ошибки
		return -1;	//и завершение работы программы
	}
	SockAddrServer.sin_port=htons((unsigned short)port);	//записываем номер порта в структура сетевого адреса сервера
	SockData=socket(PF_INET,SOCK_STREAM,0);	//создаем сокет
	if (SockData==INVALID_SOCKET) {	//если сокет не создался, то
		fputs("=== Socket creation error ===\n",stderr);	//вывод сообщения об ошибки
		return -1;	//и завершение работы программы
	}
	if (connect(SockData,(struct sockaddr*)&SockAddrServer,sizeof(SockAddrServer))!=0) {	//подключаемся к серверу. Если ошибка, то
		fprintf(stderr,"=== Error connection with %s:%u ===\n",inet_ntoa(SockAddrServer.sin_addr),ntohs(SockAddrServer.sin_port));	//вывод сообщения об ошибки
		closesocket(SockData);	//закрытие сокета
		return -1;	//и завершение работы программы
	}
	fprintf(stdout,"Connection to the server established: %s:%u\n",inet_ntoa(SockAddrServer.sin_addr),ntohs(SockAddrServer.sin_port));	//оповещение об успешном подключении к серверу
	for (i=3;i<argc;i++) {	//отправляем поэлементно данные на сервер из консоли
		size_msg=strlen(argv[i])+1; //определяем длину отправляемой строки
		if (send(SockData,argv[i],size_msg,0)<size_msg) {	//отправляем серверу. Если строка не отправилась полностью, то
			fprintf(stderr,"Error of send: \"%s\"\n",argv[i]);	//выводим об этом оповещение
			continue;	//и переходим к отправке следующей строки
		}		
	}
	shutdown(SockData,1);	//после отправки всех данных закрываем сокет на отправку данных
	printf("Sorted array: ");	//выводим шапку отсортированных данных
	while (1) {	//запускаем бесконечный цикл приема данных от сервера
		size_msg=recv(SockData,buffer,sizeof(buffer)-1,0);	//получаем данные от сервера
		if (size_msg<=0) break;	//если размер данных отсутствует, значит сервер передал все данные. Выходим из цикла приема
		buffer[size_msg]='\0';	//добавляем нулевой символ для корректного парсинга строки
		int cur_p=0;	//устанавливаем указатель парсинга в начало строки
		while (cur_p!=size_msg) {	//проходим строку до ее конца
			if (cur_p==0||buffer[cur_p-1]=='\0') printf("%s ",buffer+cur_p);	//при достижении нулевого символа считываем элемент строки и выводим его на экран
			cur_p++;	//сдвигаем указатель к следующему элементу строки
		}
	}
	printf("\n");	//переводим курсор на новую строку
	shutdown(SockData,2);	//закрываем клиент на прием
	Sleep(100);
	closesocket(SockData);	//закрываем сокет
	SockData=INVALID_SOCKET;	//освобождаем ресурсы
	WSACleanup();
	return 0;	//успешное завершение работы программы
}
