#include <windows.h>
#include <stdio.h>
#include <winsock.h>	//подключение библиотеки Winsock 1.1
#define DEFAULT_ECHO_PORT 7	//порт по умолчанию
char bufser[1024],int_to_str[20];	//глобальный буффер
void swap(int* a,int* b) {	//функция перестановки элементов для алгоритма сортировки
    int vsi=*a;
    *a=*b;
    *b=vsi;
}
int part(int mas[],int left,int right) {	//функция разделения массива для быстрой сортировки
    int end=mas[right];	//в качестве опорного элемента берем последний
    int i=left-1;	//индекс меньшего элемента
    for (int j=left;j<right;j++)
        if (mas[j]<=end) {	//если текущий элемент не больше опорного
            i++;
            swap(&mas[i],&mas[j]);	//меняем элементы местами
        }
    swap(&mas[i+1],&mas[right]);	//ставим опорник на правильное место
    return i+1;	//возвращаем индекс опорного элемента
}
void sort_mas(int mas[],int left,int right) {	//рекурсивная функция быстрой сортировки
    if (left<right) {
        int opora=part(mas,left,right);	//нахождение опорного элемента
        sort_mas(mas,left,opora-1);	//рекурсивная сортировка левой части
        sort_mas(mas,opora+1,right);	//рекурсивная сортировка правой части
    }
}
int main(int argc,char** argv) {
	struct sockaddr_in SockAddrServer,SockAddrClient;	//стурктуры для хранения сетевых адресов сервера и клиента
	SOCKET SockListen,SockData;	//сокеты для прослушивания и для работы с клиентом
	unsigned short port=DEFAULT_ECHO_PORT;	//порт (по умолчанию 7)
	int size_addr,kol_byte,cur_byte;	
	WSADATA WSAData;
	WORD wWSAVer;
	int *mas=NULL,size=-1,max_size=0;	//переменные для работы с динамическим массивом чисел
	char* result_int;
	if (argc>1)	//если есть аргумент, то это порт
		if (sscanf(argv[1],"%u",&port)<1)	//считываем порт и пытаемся записать в соответствующую переменную
			fprintf(stderr,"=== Error of number port: %s, use default ===\n",argv[1]);	//при некорректном значении выводим сообщение
	wWSAVer=MAKEWORD(1,1);	//получение версии Winsock
	if (WSAStartup(wWSAVer,&WSAData)!=0) {	//инициализация библиотеки Winsock
		puts("=== Error of initialization WinSocket! ===\n");	//в случае неудачи - вывод сообщения об ошибке
		return -1;	//и завершение работы программы
	}
	SockListen=socket(PF_INET,SOCK_STREAM,0);	//создание потокового сокета
	if (SockListen==INVALID_SOCKET) {	//если сокет не создан
		fputs("=== Socket creation error! ===\n",stderr);	//вывод ошибки
		return -1;	//и завершение работы программы
	}
	memset(&SockAddrServer,0,sizeof(SockAddrServer));	//очистка структуры адреса сервера
	SockAddrServer.sin_family=AF_INET;	//заполнение данных локального адреса сервера
	SockAddrServer.sin_addr.S_un.S_addr=INADDR_ANY;	//слушать все доступные интерфейсы
	SockAddrServer.sin_port=htons(port);	//записываем порт в нужном формате
	if (bind(SockListen,(struct sockaddr*)&SockAddrServer,sizeof(SockAddrServer))!=0) {	//привязка сокета к адресу и порту
		fprintf(stderr,"=== Error of bind, port: %u ===\n",ntohs(SockAddrServer.sin_port));	//в случае неудачи - сообщение об ошибке
		return -1;	//и завершение работы программы
	}
	if (listen(SockListen,2)!=0) {	//сокет в режиме прослушивания (максимальная очередь клиентов - 2)
		closesocket(SockListen);	//закрываем сокет в случае неудачи
		fputs("=== Error of listen ===\n",stderr);	//вывод соответствущего сообщения об ошибке
		return -1;	//и завершение работы программы
	}
	fprintf(stderr,"Server start, port: %u\n",ntohs(SockAddrServer.sin_port));	//после успешной настройки и запуска сервера выводим оповещение
	while (1) {	//бесконечный цикл обработки подключений клиентов
		size_addr=sizeof(SockAddrClient);
		SockData=accept(SockListen,(struct sockaddr*)&SockAddrClient,&size_addr);	//ожидание подключения клиента
		if (SockData==INVALID_SOCKET) {	//если ошибка при подключении
			fputs("=== Connection error ===\n",stderr);	//вывод сообщения об ошибке
			continue;	//программа не завершается, продолжаем дальше ожидать клиентов
		}
		size=-1; max_size=0; mas=NULL;	//при подключении клиента сбрасываем данные динамического массива
		while (1) {	//внутренний бесконечный цикл для обработки входящих данных от конкретного клиента
			memset(bufser,0,sizeof(bufser));	//очистка буфера
			kol_byte=recv(SockData,bufser,sizeof(bufser)-1,0);	//прием данных
			if (kol_byte<=0) {	//если клиент закрыл соединение или произошла ошибка
				if (size>0) {	//проверка массива чисел на наличие данных
					sort_mas(mas,0,size-1);	//сортировка данного массива
					printf("Sorted array: ");	//шапка вывода отсортированного массива
					for (int i=0;i<size;i++) {	//работаем с каждым элементом
						snprintf(int_to_str,sizeof(int_to_str)+1,"%d ",mas[i]);	//переводим целое значение в строковое представление
						printf("%s ",int_to_str);	//выводим данное значение на экран
						send(SockData,int_to_str,strlen(int_to_str),0);	//отправляем значение по сети клиенту
					} 
					printf("\n");	//перенос строки в конце вывода
					size=-1;	//сброс размера массива
					free(mas);	//освобождение ресурсов, занятых под массив
				}
				break;	//выход из внутреннего цикла
			}
			cur_byte=0;	//если данные от клиента пришли не пустые, разбираем входящие данные и извлекаем все элементы
			for (int i=0;i<kol_byte;i++) {	//посимвольно считываем данные
				if (bufser[i]=='\0') {	//если встречаем нулевой символ
					char cur_value[256];
					strcpy(cur_value,bufser+cur_byte);	//записываем элемент строки
					long val=strtol(cur_value,&result_int,10);	//пытаемся представить его целым числом
					if ((*result_int=='\0')&&(val<INT_MAX)&&(val>INT_MIN)) {	//если полученный элемент является целым числом
						if (size==-1) size=0;	//при первом добавлении элемента в массив активируем данный массив
						size++;	//увеличиваем размер массива на единицу
						if (size>max_size) {	//динамическое предоставление памяти для нового элемента
							int *new_mas=(int*)realloc(mas,size*sizeof(int));
							mas=new_mas;
							max_size=size;
						}
						mas[size-1]=(int)val;	//записываем новое значение в массив чисел
					}
					cur_byte=i+1;	//переходим к следующему символу входной строки
				}
			}			
		}
		shutdown(SockData,2);	//запрет на отправку и получение данных
		closesocket(SockData);	//закрытие клиентского сокета
		SockData=INVALID_SOCKET;	//обнуление сокета клиента
	}
	//сюда выполнение программно не дойдет, код написан для соблюдения стандартов программирования
	shutdown(SockListen,2);	//закрытие каналов слушающего сокета
	Sleep(100);
	closesocket(SockListen);	//закрытие сокета сервера
	SockListen=INVALID_SOCKET;
	WSACleanup();	//освобождение ресурсов
	return 0;	//успешное завершение работы программы
}
