#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#define DEFAULT_ECHO_PORT 7
char bufser[1024];
void swap(int* a,int* b) {
    int vsi=*a;
    *a=*b;
    *b=vsi;
}
int part(int mas[],int left,int right) {
    int end=mas[right];
    int i=left-1;
    for (int j=left;j<right;j++)
        if (mas[j]<=end) {
            i++;
            swap(&mas[i],&mas[j]);
        }
    swap(&mas[i+1],&mas[right]);
    return i+1;
}
void sort_mas(int mas[],int left,int right) {
    if (left<right) {
        int opora=part(mas,left,right);
        sort_mas(mas,left,opora-1);
        sort_mas(mas,opora+1,right);
    }
}
int main(int argc,char** argv) {
    struct sockaddr_in sock_server,sock_client;
    SOCKET sock_listen_id=INVALID_SOCKET;
    unsigned short num_port=DEFAULT_ECHO_PORT;
    int addr_sock_size,kol_byte;
    WSADATA WSAData;
    WORD wWSAVer;
    int *mas=NULL,size=-1,max_size=0;
    char *result_int;
    char int_to_str[20];
    if (argc>1) {
        if (sscanf(argv[1],"%u",&num_port)<1)
            fprintf(stderr,"=== Invalid port value ===");
    }
    wWSAVer=MAKEWORD(1,1);
    if (WSAStartup(wWSAVer,&WSAData)!=0) {
        puts("=== WinSocket initialization error ===");
        return -1;
    }
    sock_listen_id=socket(PF_INET,SOCK_DGRAM,0);
    if (sock_listen_id==INVALID_SOCKET) {
        fputs("=== Socket creating error ===\n",stderr);
        return -1;
    }
    memset(&sock_server,0,sizeof(sock_server));
    sock_server.sin_family=AF_INET;
    sock_server.sin_addr.S_un.S_addr=INADDR_ANY;
    sock_server.sin_port=htons(num_port);
    if (bind(sock_listen_id,(struct sockaddr*)&sock_server,sizeof(sock_server))!=0) {
        fprintf(stdout,"=== Error bind socket, port: %u ===\n",ntohs(sock_server.sin_port));
        return -1;
    }
    fprintf(stderr,"+++ Server ON, port: %u +++\n",ntohs(sock_server.sin_port));
    while (1) {
        addr_sock_size=sizeof(sock_client);
        kol_byte=recvfrom(sock_listen_id,bufser,sizeof(bufser)-1,0,(struct sockaddr*)&sock_client,&addr_sock_size);
        if (kol_byte<0) {
            fputs("=== Error input message! ===\n",stderr);
            continue;
        } else if (strcmp(bufser,"\0")!=0) {
            long val=strtol(bufser,&result_int,10);
            if ((*result_int=='\0')&&(val<INT_MAX)&&(val>INT_MIN)) {
                if (size==-1) size=0;
                size++;
                if (size>max_size) {
                    int *new_mas=(int*)realloc(mas,size*sizeof(int));
                    mas=new_mas;
                }
                mas[size-1]=(int)val;
            }
        } else if (size!=0) {
            if (size>0) {
                sort_mas(mas,0,size-1);
                printf("Sorted array: ");
                for (int i=0;i<size;i++) {
                    snprintf(int_to_str,sizeof(int_to_str),"%d",mas[i]);
                    printf("%s ",int_to_str);
                    sendto(sock_listen_id,int_to_str,sizeof(int_to_str),0,(struct sockaddr*)&sock_client,sizeof(sock_client));
                } printf("\n");
            }
            int_to_str[0]='\0';
            for (int i=0;i<10;i++) sendto(sock_listen_id,int_to_str,sizeof(int_to_str[0]),0,(struct sockaddr*)&sock_client,sizeof(sock_client));
            size=-1;
        }
    }
    shutdown(sock_listen_id,2);
    Sleep(100);
    closesocket(sock_listen_id);
    sock_listen_id=INVALID_SOCKET;
    WSACleanup();
    return 0;
}