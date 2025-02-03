/*

 FTP server in simplest old C style for TinyC compiler for very old Windows PCs.

 ChatGPT made almost everything

*/

#define _WIN32_WINNT 0x0501 // Ensure compatibility with Windows XP
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 100*1024

char buffer[BUFFER_SIZE];
char filenm[BUFFER_SIZE];
char list_buffer[BUFFER_SIZE];
char entry[BUFFER_SIZE];
char pathbuf[BUFFER_SIZE];
char server_ip[256];
char client_ip[256];
int client_len;
struct sockaddr_in server_addr = {0}, client_addr = {0};
struct sockaddr_in active_addr = {0}, pasv_addr = {0};
FILE *file;
int control_port = 21;
int data_port = 20;
int passive_mode = 1;
int transfer_mode = 0; // 0 = ASCII, 1 = Binary
WSADATA wsa_data;
SOCKET server_sock = INVALID_SOCKET, client_sock = INVALID_SOCKET;
SOCKET data_sock = INVALID_SOCKET, pasv_sock = INVALID_SOCKET;
char working_directory[MAX_PATH] = "C:\\";

void get_server_ip(char *ip, size_t size) {
    char host_name[256];
    if (gethostname(host_name, sizeof(host_name)) == 0) {
        struct hostent *host_entry = gethostbyname(host_name);
        if (host_entry) {
            struct in_addr **addr_list = (struct in_addr **)host_entry->h_addr_list;
            if (addr_list[0]) {
                strncpy(ip, inet_ntoa(*addr_list[0]), size);
                return;
            }
        }
    }
    strncpy(ip, "127.0.0.1", size);
}

void send_response(const char *message) {
    send(client_sock, message, strlen(message), 0);
}

SOCKET get_data_socket() {
    if (passive_mode) {
        struct sockaddr_in client;
        int client_len = sizeof(client);
        return accept(pasv_sock, (struct sockaddr*)&client, &client_len);
    }
    return data_sock;
}

void removecharat( char *s ) {
	char *T = s, *Q = s;
	while(*Q != 0) *(Q++) = *(++T);
}

void norm_filename( char *filename ) {
	char *C = filename;
	while(*C == ' ') removecharat(C);
	while(*C != 0) {
		if(*C == '/') *C = '\\';
		if(*C < 14 || *C == ' ') { *C = '\0'; break; }
		C++;
	}
}

void removelastslash( char *filename ) {
	char *slash = strrchr(filename, '\\');
	if(slash != NULL) {
        if( *(++slash) == 0 ) *(--slash) = '\0';
	}
}

void removeslashes( char *filename ) {
	char *C = filename;
	while(*C != 0) {
		if(*C == '/' || *C == '\\' || *C == ':') removecharat(C);
		else C++;
	}
}

void removestartdots( char *filename ) {
	char *C = filename;
	while(*C == '.' || *C == ' ') removecharat(C);
}

void safefilename(char *path) {
	norm_filename(path);
	removeslashes(path);
	removestartdots(path);
	GetCurrentDirectory(MAX_PATH, pathbuf);
	removelastslash(pathbuf);	
	strcat(pathbuf,"/");
	strcat(pathbuf,path);	
	norm_filename(pathbuf);	
	strcpy(path, pathbuf);
}

void remove_till_parent_folder(char *path) {
    size_t len = strlen(path);
    if (len == 0) return;
    while (len > 0 && path[len-1] == '\\') path[--len] = '\0';
    while (len > 0 && path[len-1] != '\\') path[--len] = '\0';
}

void get_file_attributes(char *output, WIN32_FIND_DATA *find_data) {
    char permissions[11] = "-rwxrwxrwx";
    if (find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        permissions[0] = 'd';
    }
    SYSTEMTIME stUTC, stLocal;
    FileTimeToSystemTime(&find_data->ftLastWriteTime, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
    char month[4];
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    snprintf(month, sizeof(month), "%s", months[stLocal.wMonth - 1]);
    char date[20];
    snprintf(date, sizeof(date), "%s %02d %02d:%02d", month, stLocal.wDay, stLocal.wHour, stLocal.wMinute);
	//unsigned long long size = ((unsigned long long)find_data->nFileSizeHigh << 32) | find_data->nFileSizeLow;
	DWORD size = find_data->nFileSizeLow;	// 4GB
    sprintf(output, "%s   1 user    group    %12llu %s %s\r\n", permissions, size, date, find_data->cFileName);
}

void list_directory() {
    SOCKET transfer_sock = get_data_socket();
    if (transfer_sock == INVALID_SOCKET) {
        send_response("425 Can't open data connection\r\n");
        return;
    }
    WIN32_FIND_DATA find_data;
    HANDLE hFind = FindFirstFile("*", &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        send_response("550 Directory listing failed\r\n");
        return;
    }
    send_response("150 Here comes the directory listing\r\n");
    strcpy(list_buffer, "");
    do {
        if (strcmp(find_data.cFileName, ".") != 0 && strcmp(find_data.cFileName, "..") != 0) {
			get_file_attributes(entry, &find_data);
			strcat(list_buffer, entry);
        }
    } while (FindNextFile(hFind, &find_data));
    FindClose(hFind);
    strcat(list_buffer, "\r\n"); // Ensure proper termination
	send(transfer_sock, list_buffer, strlen(list_buffer), 0);
    closesocket(transfer_sock);
    send_response("226 Directory send OK.\r\n");
}

void nlst() {
	list_directory();
}

void pwd() {
    char current_path[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, current_path);
	norm_filename(current_path);
    sprintf(buffer, "257 \"%s\" is the current directory\r\n", current_path);
    send_response(buffer);
}

void retr(char *filename) {
	strcpy( filenm, filename );
	norm_filename(filenm);
	removeslashes(filenm);
    SOCKET transfer_sock = get_data_socket();
    if (transfer_sock == INVALID_SOCKET) {
        send_response("425 Can't open data connection\r\n");
        return;
    }
    file = fopen(filenm, transfer_mode ? "rb" : "rt");
    if (!file) {
        send_response("550 File not found\r\n");
    } else {
        send_response("150 Opening data connection\r\n");
        int bytes_read;
        while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            send(transfer_sock, buffer, bytes_read, 0);
        }
        fclose(file);
		printf("[INFO] Sent file: %s\n", (char *)filenm);
        closesocket(transfer_sock);
        send_response("226 Transfer complete\r\n");
    }
}

void stor(char *filename) {
	strcpy( filenm, filename );
	norm_filename(filenm);
	removeslashes(filenm);
    SOCKET transfer_sock = get_data_socket();
    if (transfer_sock == INVALID_SOCKET) {
        send_response("425 Can't open data connection\r\n");
        return;
    }
    file = fopen(filenm, transfer_mode ? "wb" : "wt");
    if (!file) {
        send_response("550 Failed to open file for writing\r\n");
    } else {
        send_response("150 Ready to receive file\r\n");
        int bytes_received;
        while ((bytes_received = recv(transfer_sock, buffer, BUFFER_SIZE, 0)) > 0) {
            fwrite(buffer, 1, bytes_received, file);
        }
        fclose(file);
		printf("[INFO] Received file: %s\n", (char *)filenm);
        closesocket(transfer_sock);
        send_response("226 Transfer complete\r\n");
    }
}
			
void pasv() {
	int h1, h2, h3, h4;
	sscanf(client_ip, "%d.%d.%d.%d", &h1, &h2, &h3, &h4);
    int port = 50000 + (rand() % 1000);
    pasv_sock = socket(AF_INET, SOCK_STREAM, 0);
    pasv_addr.sin_family = AF_INET;
    pasv_addr.sin_addr.s_addr = INADDR_ANY;
    pasv_addr.sin_port = htons(port);
    bind(pasv_sock, (struct sockaddr*)&pasv_addr, sizeof(pasv_addr));
    listen(pasv_sock, 1);
    sprintf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n", h1, h2, h3, h4, port / 256, port % 256);
	printf("port:%ld\n",port);
    send_response(buffer);
}

void port(char *params) {
    int h1, h2, h3, h4, p1, p2;
    sscanf(params, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
    data_sock = socket(AF_INET, SOCK_STREAM, 0);
    active_addr.sin_family = AF_INET;
    active_addr.sin_addr.s_addr = client_addr.sin_addr;
    active_addr.sin_port = htons(p1 * 256 + p2);
    printf("port:%ld\n",active_addr.sin_port);
	connect(data_sock, (struct sockaddr*)&active_addr, sizeof(active_addr));
    send_response("200 PORT command successful\r\n");
}

void cdup() {
    GetCurrentDirectory(MAX_PATH, pathbuf);
	norm_filename(pathbuf);
    if (strcmp(pathbuf, working_directory) == 0) {
		send_response("550 Already at root directory\r\n");
        return;
    }
    remove_till_parent_folder(pathbuf);
    if (SetCurrentDirectory(pathbuf)) {
        send_response("250 Directory successfully changed\r\n");
    } else {
        send_response("550 Failed to change directory\r\n");
    }
}

void cwd(char *path) {
	norm_filename(path);
	char *C = path;
	// avoid exact pathnames in commands
	if(*C == '.' || *C == '\\' || strchr(path, ':') != NULL) {
		cdup();
		return;
	}
	safefilename(path);
    if (SetCurrentDirectory(path)) {
        send_response("250 Directory successfully changed\r\n");
    } else {
        send_response("550 Failed to change directory\r\n");
    }
}

void mkd(char *path) {
	safefilename(path);
    if (CreateDirectory(path, NULL)) {
        send_response("257 Directory created\r\n");
    } else {
        send_response("550 Failed to create directory\r\n");
    }
}

void rmd(char *path) {
	safefilename(path);
    if (RemoveDirectory(path)) {
        send_response("250 Directory removed\r\n");
    } else {
        send_response("550 Failed to remove directory\r\n");
    }
}

void dele(char *path) {
	safefilename(path);
    if (DeleteFile(path)) {
        send_response("250 File deleted\r\n");
    } else {
        send_response("550 Failed to delete file\r\n");
    }
}

void rnfr(char *old_name) {
	strcpy( filenm, old_name );
	safefilename( filenm );
    send_response("350 Ready for RNTO\r\n");
}

void rnto(char *old_name, char *new_name) {
	safefilename(new_name);
    if (MoveFile(old_name, new_name)) {
        send_response("250 File renamed successfully\r\n");
    } else {
        send_response("550 Failed to rename file\r\n");
    }
}

void site(char *cmd) {
    char mode[4];
    if (sscanf(cmd, "chmod %3s %s", mode, filenm) != 2) {
        send_response("500 Invalid STAT command format\r\n");
        return;
    }
    int attributes = FILE_ATTRIBUTE_ARCHIVE;
    if (strcmp(mode, "777") == 0) {
        attributes |= FILE_ATTRIBUTE_NORMAL;
    } else if (strcmp(mode, "444") == 0) {
        attributes |= FILE_ATTRIBUTE_READONLY;
    } else if (strcmp(mode, "000") == 0) {
        attributes |= FILE_ATTRIBUTE_HIDDEN;
    } else {
        send_response("500 Unsupported chmod mode\r\n");
        return;
    }
	safefilename(filenm);
    if (SetFileAttributes(filenm, attributes)) {
		send_response("200 File attributes changed\r\n");
    } else {
		send_response("550 Failed to change file attributes\r\n");
    }
}

void feat() {
	send_response("211-Features:\r\n211 End\r\n");
}

void help() {
	send_response("220 Help not there\r\n");
}

void stat() {
	sprintf(buffer, "211-FTP server status:\r\n Connected to %s\r\n TYPE: ", server_ip);
	if(transfer_mode == 0) {
		strcat(buffer, "ASCII");
	} else {
		strcat(buffer, "BINARY");
	}
	strcat(buffer, "\r\n File transfer MODE: Stream\r\n211 End of status\r\n");
	send_response(buffer);
}
			
void client() {
	strcpy(client_ip, inet_ntoa(client_addr.sin_addr));
    printf("[INFO] Client connected: %s\n", client_ip);
    send_response("220 Simple FTP Server\r\n");
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) break;
        
        printf("[CMD] %s", buffer);
		if (strncmp(buffer, "USER", 4) == 0) {
			send_response("230 Logged in\r\n");
        } else if (strncmp(buffer, "PASS", 4) == 0) {
            send_response("230 Logged in\r\n");
        } else if (strncmp(buffer, "LIST", 4) == 0) {
            list_directory();
        } else if (strncmp(buffer, "NLST", 4) == 0) {
			nlst();
        } else if (strncmp(buffer, "PORT", 4) == 0) {
			passive_mode = 0;
			port(buffer + 5);
        } else if (strncmp(buffer, "PASV", 4) == 0) {
			passive_mode = 1;
			pasv();
        } else if (strncmp(buffer, "CWD", 3) == 0) {
			cwd(buffer + 4);
        } else if (strncmp(buffer, "CDUP", 4) == 0) {
			cdup();			
        } else if (strncmp(buffer, "XCUP", 4) == 0) {
			cdup();
        } else if (strncmp(buffer, "SYST", 4) == 0) {
			send_response("215 Windows TinyC made ftp\r\n");
        } else if (strncmp(buffer, "FEAT", 4) == 0) {
			feat();
        } else if (strncmp(buffer, "PWD", 3) == 0) {
			pwd();
        } else if (strncmp(buffer, "XPWD", 4) == 0) {
			pwd();
        } else if (strncmp(buffer, "STAT", 4) == 0) {
			stat();
        } else if (strncmp(buffer, "QUIT", 4) == 0 ||
				strncmp(buffer, "BYE", 3) == 0) {
			send_response("221 Bye.\r\n");
        } else if (strncmp(buffer, "RETR ", 5) == 0) {
            retr(buffer + 5);
        } else if (strncmp(buffer, "STOR ", 5) == 0) {
			stor(buffer + 5);
        } else if (strncmp(buffer, "TYPE I", 6) == 0) {
            transfer_mode = 1;
            send_response("200 Binary mode set\r\n");
        } else if (strncmp(buffer, "TYPE A", 6) == 0) {
            transfer_mode = 0;
            send_response("200 ASCII mode set\r\n");
        } else if (strncmp(buffer, "MKD ", 4) == 0) {
            mkd(buffer + 4);
        } else if (strncmp(buffer, "XMKD ", 5) == 0) {
            mkd(buffer + 5);
        } else if (strncmp(buffer, "RMD ", 4) == 0) {
            rmd(buffer + 4);
        } else if (strncmp(buffer, "XRMD ", 5) == 0) {
            rmd(buffer + 5);
        } else if (strncmp(buffer, "DELE ", 5) == 0) {
            dele(buffer + 5);
        } else if (strncmp(buffer, "RNFR ", 5) == 0) {
            rnfr(buffer + 5);
        } else if (strncmp(buffer, "RNTO ", 5) == 0) {
            rnto(filenm, buffer + 5);
        } else if (strncmp(buffer, "SITE ", 5) == 0) {
            site(buffer + 5);
		} else if (strncmp(buffer, "HELP", 4) == 0) {
			help();
        } else {
			send_response("500 Unknown command\r\n");
		}
    }
    printf("[INFO] Client disconnected\n");
    closesocket(client_sock);
}

void hello() {
 static char *hello_txt[] = {
 (char *)" --------------------------------\n",
 (char *)" This is an ftp server as a tool\n",
 (char *)"   for very old windows PCs.\n",
 (char *)" Can connect and get or put a file.\n",
 (char *)" Inbuilt cmd.exe>ftp.exe works ok.\n",
 (char *)"\n",
 (char *)" ChatGPT, 2025\n",
 (char *)" --------------------------------\n",
 (char *)"\0",
 };
 for( int i = 0; hello_txt[i][0] != '\0' ; i++ ) {
	printf( hello_txt[i] );
 }
 }

int main() {
	hello();
    GetModuleFileNameA(NULL, working_directory, MAX_PATH);
    *(strrchr(working_directory, '\\')) = '\0';
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
	client_len = sizeof(client_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(control_port);
    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);
    get_server_ip(server_ip, sizeof(server_ip));
    printf("[INFO] FTP Server started at ftp://%s:%d\n", server_ip, control_port);
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock != INVALID_SOCKET) {
            client();
        }
    }
    closesocket(server_sock);
    WSACleanup();
    return 0;
}
