#pragma once
#ifndef SERVER_H__
#define SERVER_H__
#include <stdint.h>

#pragma pack(1)
typedef struct PacketHeader
{
    int16_t DataSize;
    int16_t PacketID;
    char    Type;
} PHeader;

typedef struct ResponsePacket {

    int16_t         DataSize;
    int16_t         PacketID;
    char            Type;
    unsigned char   Data[4];
} Response;

typedef struct BinaryRequest {
    int     N1;
    short   Op1;
    int     N2;
} BData;

typedef struct TernaryRequest {
    int     N1;
    short   Op1;
    int     N2;
    short   Op2;
    int     N3;
} TData;


void    err_quit(const wchar_t* msg);
void    err_display(const wchar_t* msg);
void	binaryCalculus(char* buf, int* response);
void	ternaryCalculus(char* buf, int* response);

#endif // !SERVER_H__
