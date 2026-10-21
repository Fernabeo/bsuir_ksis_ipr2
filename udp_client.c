#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
char bufcli [1024];
int main(int argc,char** argv) {
    struct sockaddr_in sock_addr_local,sock_addr_ser,sock_addr_ser2;
    SOCKET sock_local=INVALID_SOCKET;
    struct hostent* pDNStoIP;
    int flagBC,nAddrSize,port_ser,size_message,i,vsi;
    WSADATA WSAData;
    WORD wWSAVer;
    char end_mas[5];
    if (argc<3) {	//проверка минимального набора аргументов
        puts("=== Not enough arguments: [addr_server] [port] [message1]... ===\n");
        return -1;
    }
    wWSAVer=MAKEWORD(1,1);	//получение версии WinSock
    if (WSAStartup(wWSAVer,&WSAData)!=0) {	//инициализация WinSock
        puts("=== WinSocket initialization error ===");	//если неудача, вывод ошибки
        return -1;
    }
    sock_local=socket(PF_INET,SOCK_DGRAM,0);	//создание сокета
    if (sock_local==INVALID_SOCKET) {	//если сокет не создан
        fputs("=== Socket creating error ===\n",stderr);	//вывод ошибки
        return -1;
    }
    flagBC=1;
    setsockopt(sock_local,SOL_SOCKET,SO_BROADCAST,(char*)(&flagBC),sizeof(flagBC));	//настройка широковещательного сообщения
    memset(&sock_addr_local,0,sizeof(sock_addr_local));
    sock_addr_local.sin_family=AF_INET;	//заполнение данных локального адреса
    sock_addr_local.sin_addr.S_un.S_addr=INADDR_ANY;
    sock_addr_local.sin_port=0;
    if (bind(sock_local,(struct sockaddr*)&sock_addr_local,sizeof(sock_addr_local))!=0) {	//привязка сокета
        fputs("=== Error bind to local address ===\n",stderr);	//вывод ошибки в случае ошибки привязки
        return -1;
    }
    int timeout = 3000;	//установка тайм-аута на случай неработающего сервера
    if (setsockopt(sock_local, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
        fprintf(stderr, "=== Error setting timeout: %d ===\n", WSAGetLastError());
        closesocket(sock_local);
        WSACleanup();
        return -1;
    }
    memset(&sock_addr_ser,0,sizeof(sock_addr_ser));	//подготовка структуры адреса сервера
    sock_addr_ser.sin_family=AF_INET;
    if (strcmp(argv[1],"255.255.255.255")==0)
        sock_addr_ser.sin_addr.S_un.S_addr=INADDR_BROADCAST;
    else {
        sock_addr_ser.sin_addr.S_un.S_addr=inet_addr(argv[1]);	//преобразование IP-адреса в нужный формат
        if (sock_addr_ser.sin_addr.S_un.S_addr==INADDR_NONE) {	//если передается не IP,
            if ((pDNStoIP=gethostbyname(argv[1]))==NULL) {		//пытаемся получить IP через доменное имя
                fprintf(stderr,"=== Host not found: %s ===\n",argv[1]);	//если не получилось получить IP, то ошибка с завершением работы
                return -1;
            }
            sock_addr_ser.sin_addr=*(struct in_addr*)(pDNStoIP->h_addr_list[0]);	//записываем полученный IP
        }
    }
    if (sscanf(argv[2],"%u",&port_ser)<1) {	//считываем порт из командной строки
        fprintf(stderr,"=== Error port %s ===\n",argv[2]);	//ошибка в случае некорректного порта
        return -1;
    }
    sock_addr_ser.sin_port=htons((unsigned short)port_ser);	//записываем в структура адреса сервера считанный порт
    for (i=3;i<argc;i++) {	//передаем сообщения по сети из командной строки
        size_message=strlen(argv[i])+1;	//определяем длину сообщения
        if (sendto(sock_local,argv[i],size_message,0,(struct sockaddr*)&sock_addr_ser,sizeof(sock_addr_ser))==SOCKET_ERROR) {	//если ошибка отправки
            fprintf(stderr,"=== Error of send: [%s] ===\n",argv[i]);	//выводим сообщение об ошибке
            continue;	//и переходим к отправке следующего сообщения
        }
    }
    for (i=1;i<=10;i++) { //после отправки всех сообщение отправляем нулевой символ для обозначения конца передачи
        if (sendto(sock_local,"\0",sizeof("\0"),0,(struct sockaddr*)&sock_addr_ser,sizeof(sock_addr_ser))==SOCKET_ERROR) {
            fprintf(stderr,"=== Error of send END ===\n");	//вывод сообщения в случае неудачи
            continue;
        }
    }
    while(1) {	//запускаем бесконечный цикл приема данных
        nAddrSize=sizeof(sock_addr_ser2);
        size_message=recvfrom(sock_local,bufcli,sizeof(bufcli)-1,0,(struct sockaddr*)&sock_addr_ser2,&nAddrSize);	//получение данных
        if (size_message == SOCKET_ERROR) {	//если ошибка приема
            int err = WSAGetLastError();
            if (err == WSAETIMEDOUT) {	//если сервер не ответил в течение таймаута
                fputs("=== Timeout exceeded. The server did not respond. ===\n", stderr);
            } else {
                fprintf(stderr, "=== Error answer ===\n");	//другая ошибка
            }
            break; 
        }
        if (bufcli[0]=='\0') break;	//если получили нулевой символ, то конец приема и выход из цикла
        bufcli[size_message]='\0';	//к полученному сообщения добавляем нулевой символ для корректного вывода на экран
        fprintf(stdout,"Answer from %s:%u [%s]\n",inet_ntoa(sock_addr_ser2.sin_addr),ntohs(sock_addr_ser2.sin_port),bufcli);	//вывод информации
    }
    shutdown(sock_local,2);	//освобождение ресурсов и завершение работы
    Sleep(100);
    closesocket(sock_local);
    sock_local=INVALID_SOCKET;
    WSACleanup();
    return 0;
}