#include <malloc.h>
#include <memory.h>
#include <string.h>
#ifndef UNIX
#include <windows.h>
#include <pthread.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include "VKF.h"

#ifdef UNIX
#define scanf_s scanf
#define sprintf_s snprintf
#define errno_t int
#endif

static int iCounts = 200000;
unsigned char pucPubKey[64] =
{
    0x3E, 0x3A, 0xDD, 0x4B, 0x3C, 0x32, 0x29, 0x85, 0x2D, 0xC4, 0xC6, 0xA1, 0x5B, 0x21, 0x1A, 0x88,
    0x9C, 0xD0, 0xDB, 0x04, 0x6E, 0x93, 0x95, 0xBA, 0xEB, 0x35, 0xE0, 0x72, 0xC9, 0x60, 0x71, 0xE2,
    0x5D, 0xFD, 0xEE, 0x2C, 0x21, 0x87, 0x97, 0xBB, 0x6F, 0x01, 0xF9, 0xE0, 0x1E, 0x53, 0x32, 0x76,
    0x49, 0x43, 0xAA, 0x1C, 0x87, 0x3F, 0xE5, 0xD0, 0xB8, 0x93, 0xBF, 0xAB, 0xDB, 0xA1, 0x13, 0x14
};

void bin2hex(const char *in, int inLen, char *out)
{
    unsigned char CC;
    int iLoop;

    for (iLoop = 0; iLoop < inLen; iLoop++)
    {
        CC = (unsigned char)((in[iLoop] >> 4) & 0x0f);
        out[iLoop * 2] = (char)((CC < 0x0A) ? CC + '0' : CC + 'A' - 0x0A);
        CC = (unsigned char)(in[iLoop] & 0x0f);
        out[iLoop * 2 + 1] = (char)((CC < 0x0A) ? CC + '0' : CC + 'A' - 0x0A);

    }
}

int PackBCD( char *InBuf, unsigned char *OutBuf, int Len )
{
    int        rc;                    /* Return Value */
    register int     ActiveNibble;    /* Active Nibble Flag */
    char     CharIn;                  /* Character from source buffer */
    unsigned char   CharOut;          /* Character from target buffer */
    rc = 0;                           /* Assume everything OK. */

    ActiveNibble = 0;                 /* Set Most Sign Nibble (MSN) */

    /* to Active Nibble. */
    for ( ; (Len > 0); Len--, InBuf++ )
    {
        CharIn = *InBuf;
        if ( !isxdigit ( CharIn ) )    /* validate character */
        {
            rc = -1;
        }
        else
        {
            if ( CharIn > '9')
            {
                CharIn += 9;           /* Adjust Nibble for A-F */
            }
        }

        if ( rc == 0 )
        {
            CharOut = *OutBuf;
            if ( ActiveNibble )
            {
                *OutBuf++ = (unsigned char)( ( CharOut & 0xF0) |
                                             ( CharIn  & 0x0F)   );
            }
            else
            {
                *OutBuf = (unsigned char)( ( CharOut & 0x0F)   |
                                           ( (CharIn & 0x0F) << 4)   );
            }
            ActiveNibble ^= 1;        /* Change Active Nibble */
        }
    }
    return rc;
}


int UnpackBCD( unsigned char *InBuf,  char *OutBuf, int Len )
{
    int rc = 0;
    unsigned char ch;
    register int i, active = 0;

    for ( i = 0; i < Len; i++ )
    {
        ch = *InBuf;
        if ( active )
        {
            (*OutBuf = (ch & 0xF)) < 10 ? (*OutBuf += '0') : (*OutBuf += ('A' - 10));
            InBuf++;
        }
        else
        {
            (*OutBuf = (ch & 0xF0) >> 4) < 10 ? (*OutBuf += '0') : (*OutBuf += ('A' - 10));
        }

        active ^= 1;

        if ( !isxdigit ( *OutBuf ) )    /* validate character */
        {
            rc = -1;
            break;
        }
        OutBuf++;
    }

    *OutBuf = 0;
    return ( rc );
}

void T_SearchDevice()
{
    int rv = 0;
    char strDeviceID[64] = {0};
    DeviceInfo  pstOutDeviceInfo;

    memset(&pstOutDeviceInfo, 0x00, sizeof(pstOutDeviceInfo));

    printf("please input Device ID [nH]:\n");
    scanf("%s", strDeviceID);

    rv = VKF_SearchDevice( strDeviceID, &pstOutDeviceInfo/*out*/);
    if ( rv )
    {
        printf("VKF_SearchDevice Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }

    printf("Status = [%d]\n", pstOutDeviceInfo.uiStatus);
    printf("Type = [%d]\n", pstOutDeviceInfo.uiType);
    printf("SecurityLevel = [%d]\n", pstOutDeviceInfo.uiSecurityLevel);
    //printf("IPC设备ID = [%s]\n", pstOutDeviceInfo.strIpcId);
    //printf("NVR设备ID = [%s]\n", pstOutDeviceInfo.strNvrId);
    //strDeviceId
    //strParentDevId
    printf("Device Id = [%s]\n", pstOutDeviceInfo.strDeviceId);
    printf("Parent DevId = [%s]\n", pstOutDeviceInfo.strParentDevId);
    printf("Create Time = [%s]\n", pstOutDeviceInfo.strCreateTime);
    printf("Last Register Time = [%s]\n", pstOutDeviceInfo.strLastRegisterTime);
    printf("Delete Time = [%s]\n", pstOutDeviceInfo.strDeleteTime);
}

void T_GenerateVEMK()
{
    int rv = 0;
    char strIpcId[64] = {0};
    char strNvrId[64] = {0};
    char strTimer[32] = {0};
    unsigned int uiAlgorithmVemk = 0;
    unsigned int uiType = 0;//0 nvr device , 1 ipc device

    printf("please input Device Type (0:NVR 1:IPC):\n");
    scanf("%ud", &uiType);

    if(uiType < 1)
    {
        //nvr device
        printf("please input NVR ID:\n");
        scanf("%s", strNvrId);
    }
    else
    {
        printf("Use NVR input NVR ID OR Not Use NVR input 0:\n");
        scanf("%s", strNvrId);

        printf("please input IPC ID:\n");
        scanf("%s", strIpcId);
    }


    printf("please input uiAlgorithmVemk(1:SM1 2:SM4):\n");
    scanf("%ud", &uiAlgorithmVemk);

    if(uiAlgorithmVemk == 1)
    {
        uiAlgorithmVemk = VGD_SM1_ECB;
    }
    else
    {
        uiAlgorithmVemk = VGD_SM4_ECB;
    }

    /*
    printf("please input time:\n");
    scanf("%s", strTimer);
    */
    strcpy(strTimer, "2014-10-03 15:36:00");

    if(uiType < 1)
    {
        rv = VKF_GenerateVEMK( strNvrId, NULL, uiAlgorithmVemk, strTimer, 1);
    }
    else
    {
        if(strlen(strNvrId) < 5)
        {
            rv = VKF_GenerateVEMK( strIpcId, NULL, uiAlgorithmVemk, strTimer, 1);
        }
        else
        {
            rv = VKF_GenerateVEMK( strIpcId, strNvrId, uiAlgorithmVemk, strTimer, 1);
        }

    }
    //rv = VKF_GenerateVEMK( strIpcId, strNvrId, uiAlgorithmVemk, strTimer, 1);
    //rv = VKF_GenerateVEMK( strIpcId, NULL, uiAlgorithmVemk, strTimer, 1);
    if ( rv )
    {
        printf("VKF_GenerateVEMK Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("VKF_GenerateVEMK Success!!!\r\n");
}
void T_GenerateVEMKOld()
{
    int rv = 0;
    char strIpcId[64] = {0};
    char strNvrId[64] = {0};
    char strTimer[32] = {0};
    unsigned int uiAlgorithmVemk = 0;
    unsigned int uiType = 0;//0 nvr device , 1 ipc device

    printf("please input Device Type (0:NVR 1:IPC):\n");
    scanf("%ud", &uiType);

    if(uiType < 1)
    {
        //nvr device
        printf("please input NVR ID:\n");
        scanf("%s", strNvrId);
    }
    else
    {
        printf("Use NVR input NVR ID OR Not Use NVR input 0:\n");
        scanf("%s", strNvrId);

        printf("please input IPC ID:\n");
        scanf("%s", strIpcId);
    }


    printf("please input uiAlgorithmVemk(1:SM1 2:SM4):\n");
    scanf("%ud", &uiAlgorithmVemk);

    if(uiAlgorithmVemk == 1)
    {
        uiAlgorithmVemk = VGD_SM1_ECB;
    }
    else
    {
        uiAlgorithmVemk = VGD_SM4_ECB;
    }

    /*
    printf("please input time:\n");
    scanf("%s", strTimer);
    */
    strcpy(strTimer, "2014-10-03 15:36:00");

    if(uiType < 1)
    {
        rv = VKF_GenerateVEMKOld( strNvrId, NULL, uiAlgorithmVemk, strTimer, 1);
    }
    else
    {
        if(strlen(strNvrId) < 5)
        {
            rv = VKF_GenerateVEMKOld( strIpcId, NULL, uiAlgorithmVemk, strTimer, 1);
        }
        else
        {
            rv = VKF_GenerateVEMKOld( strIpcId, strNvrId, uiAlgorithmVemk, strTimer, 1);
        }

    }
    //rv = VKF_GenerateVEMK( strIpcId, strNvrId, uiAlgorithmVemk, strTimer, 1);
    //rv = VKF_GenerateVEMK( strIpcId, NULL, uiAlgorithmVemk, strTimer, 1);
    if ( rv )
    {
        printf("VKF_GenerateVEMKOld Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("VKF_GenerateVEMKOld Success!!!\r\n");
}
void T_DeleteVEMK()
{
    int rv = 0;
    char strIpcId[64] = {0};
    char strNvrId[64] = {0};
    char strTimer[32] = {0};

    printf("please input IPC ID:\n");
    scanf("%s", strIpcId);

    //printf("please input time:\n");
    //scanf("%s", strTimer);
    strcpy(strTimer, "2014-10-03 17:36:00");

    rv = VKF_DeleteVEMK( strIpcId, strTimer );
    if ( rv )
    {
        printf("VKF_DeleteVEMK Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("VKF_DeleteVEMK Success!!!\n");
}

void T_GenerateIpcVKEK()
{
    int rv = 0;
    unsigned int uiAlgorithm = 0;
    char strIpcId[64] = {0};
    char strVkekVersion[64] = {0};
    char strRegisterTime[32] = {0};
    char strOutCipherVKEK[1024] = {0};
    char strPubKey_HEX[256] = {0};
    unsigned char pucPubKey[256] = {0};
    unsigned int uiPubKeyL = 0;

     // printf("please input IPC ID:\n");
     // scanf("%s", strIpcId);

     // printf("please input Vkek Version:\n");
     // scanf("%s", strVkekVersion);

     // printf("please input uiAlgorithm(1:SM1 2:SM4):\n");
     // scanf("%ud", &uiAlgorithm);

     // if(uiAlgorithm == 1)
    // {
         // uiAlgorithm = VGD_SM1_ECB;
     // }
     // else
     // {
         // uiAlgorithm = VGD_SM4_ECB;
     // }

     // printf("please input public key (HEX string):\n");
     // scanf("%s", strPubKey_HEX);

     // //printf("please input Register Time:\n");
     // //scanf("%s", strRegisterTime);
     // strcpy(strRegisterTime, "2014-10-03 19:36:00");

     // PackBCD(strPubKey_HEX, pucPubKey, (int)strlen(strPubKey_HEX));
     // uiPubKeyL = strlen(strPubKey_HEX) / 2;s
	 unsigned int puiOutVkekLen;
     rv = VKF_GenerateIpcVKEK( "00000000001321000001",
                               "2020-12-07-15:20:02",
                               VGD_SM4_ECB,
                               "\x51\xBC\x78\x27\x73\xCD\x4A\xB4\xCC\xB5\x0C\x69\x5C\xBC\x2C\x83\x38\x30\xF7\x66\x3E\x10\x89\xDE\xD3\x1C\x53\x8D\xE1\x6E\x4B\xD9\xD7\x27\xEB\x62\x23\x4B\x55\x77\xB8\x57\xCF\xAE\x27\x83\x13\x27\x78\x1D\xDC\x7D\x38\xD4\x7F\x12\x60\x76\x41\x7C\x32\xB2\x12\xA6",
								64,
                               "2020-12-07-15:20:00",
                               strOutCipherVKEK/*out*/,
							   &puiOutVkekLen);
    if ( rv )
    {
        printf("VKF_GenerateVKEK Failed!!! Error: %#010X!\r\n", rv);
        //return ;
    }
	char out[256];
	memset(out, 0x00, 256);
	UnpackBCD( strOutCipherVKEK,  out, puiOutVkekLen * 2 );
    printf("strOutCipherVKEK = [%s]\n", out);
	printf("puiOutVkekLen = [%d]\n", puiOutVkekLen);
}

void *task ()
{
    int rv = 0;
    int nu;
    unsigned int uiAlgorithm = VGD_SM1_ECB;
    char strRegisterTime[32] = "2014-10-03 19:36:00";
    char strOutCipherVKEK[1024] = {0};
    char strIpcId[64] = "65010000001320001001";
    char strVkekVersion[64] = { 0 };

    srand((int) time(0));
    while(iCounts > 0)
    {
        nu = 0 + (int)( 100000000.0 * rand() / (RAND_MAX + 1.0));
        sprintf(strVkekVersion, "%019d", nu);
		unsigned int puiOutVkekLen;	
        rv = VKF_GenerateIpcVKEK("00000000001320000001", "2020-05-22 13:55:35", VGD_SM1_ECB,
		"\x51\xBC\x78\x27\x73\xCD\x4A\xB4\xCC\xB5\x0C\x69\x5C\xBC\x2C\x83\x38\x30\xF7\x66\x3E\x10\x89\xDE\xD3\x1C\x53\x8D\xE1\x6E\x4B\xD9\xD7\x27\xEB\x62\x23\x4B\x55\x77\xB8\x57\xCF\xAE\x27\x83\x13\x27\x78\x1D\xDC\x7D\x38\xD4\x7F\x12\x60\x76\x41\x7C\x32\xB2\x12\xA6",
		64, "2020-05-21 12:37:07", strOutCipherVKEK, &puiOutVkekLen);
        if(rv)
        {
            ;//printf("GenVKEK rv = %d\n", rv);
        }
        else
        {
            iCounts--;
        }

    }
    return;

}

void T_BatVKEK_Dates()
{
    int i, res;
    pthread_t a_thread[32];
    printf("Bat GenVKEK begin\n");

    for(i = 0; i < 32; i++)
    {
        res = pthread_create(&a_thread[i], NULL, task, NULL);
        if ( res != 0 )
        {
            fprintf(stderr, "pthred_create() failed(%d)\r\n", res);
        }
    }
    while(iCounts > 0)
    {
        if((200000 - iCounts) % 400 == 0)
        {
            printf("GenVKEK Counts = [%d]\n", (200000 - iCounts));
        }
        //sleep(1);
    }
    printf("Bat GenVKEK 20w End\n");
    return;
}

/*
void T_BatVKEK_Dates()
{
	int rv = 0;
	int i = 200000, nu;
	unsigned int uiAlgorithm = VGD_SM1_ECB;
	char strRegisterTime[32] = "2014-10-03 19:36:00";
    char strOutCipherVKEK[1024] = {0};
	unsigned char pucPubKey[64] = {
	0x3E, 0x3A, 0xDD, 0x4B, 0x3C, 0x32, 0x29, 0x85, 0x2D, 0xC4, 0xC6, 0xA1, 0x5B, 0x21, 0x1A, 0x88,
	0x9C, 0xD0, 0xDB, 0x04, 0x6E, 0x93, 0x95, 0xBA, 0xEB, 0x35, 0xE0, 0x72, 0xC9, 0x60, 0x71, 0xE2,
	0x5D, 0xFD, 0xEE, 0x2C, 0x21, 0x87, 0x97, 0xBB, 0x6F, 0x01, 0xF9, 0xE0, 0x1E, 0x53, 0x32, 0x76,
	0x49, 0x43, 0xAA, 0x1C, 0x87, 0x3F, 0xE5, 0xD0, 0xB8, 0x93, 0xBF, 0xAB, 0xDB, 0xA1, 0x13, 0x14
	};
	char strIpcId[64] = "00000000001320000001";
	char strVkekVersion[64] = { 0 };

	srand((int) time(0));

	printf("Bat GenVKEK begin\n");

	while(i>=0){
		nu = 0 + (int)( 100000000.0 *rand()/(RAND_MAX + 1.0));
		sprintf(strVkekVersion, "%019d", nu);
		rv = VKF_GenerateIpcVKEK( strIpcId,
			strVkekVersion,
			uiAlgorithm,
			pucPubKey,
			64,
			strRegisterTime,
			strOutCipherVKEK );
		if(rv){
			printf("GenVKEK rv = %d\n", rv);
		}else{
			i--;
		}
		if((200000-i)%50==0){
			printf("GenVKEK Counts = [%d]\n", (200000-i));
		}
	}
    printf("Bat GenVKEK end\n");
	return;
}
*/
void T_GenerateNvrVKEK()
{
    int rv = 0;
    unsigned int uiAlgorithmVkek = 0;
    char strNvrId[64] = {0};
    char strVkekVersion[64] = {0};

    char strPubKey_HEX[256] = {0};
    unsigned char pucPubKey[64] = {0};
    unsigned int    uiPubKeyL = 0;
    char strRegisterTime[64] = {0};
    char strOutCipherVkek[128] = {0};
    unsigned int    puiVemkVer = 0;
    char strOutCipoherVemk[128] = {0};

    printf("please input strNvrId:\n");
    scanf("%s", strNvrId);

    printf("please input strVkekVersion:\n");
    scanf("%s", strVkekVersion);

    printf("please input uiAlgorithmVkek(1:SM1 2:SM4):\n");
    scanf("%ud", &uiAlgorithmVkek);

    if(uiAlgorithmVkek == 1)
    {
        uiAlgorithmVkek = VGD_SM1_ECB;
    }
    else
    {
        uiAlgorithmVkek = VGD_SM4_ECB;
    }

    printf("please input public key (HEX string):\n");
    scanf("%s", strPubKey_HEX);

    strcpy(strRegisterTime, "2014-10-03 19:36:00");

    PackBCD(strPubKey_HEX, pucPubKey, (int)strlen(strPubKey_HEX));
    uiPubKeyL = strlen(strPubKey_HEX) / 2;

    // rv = VKF_GenerateNvrVKEK( strNvrId,
                              // strVkekVersion,
                              // uiAlgorithmVkek,
                              // pucPubKey,
                              // uiPubKeyL,
                              // strRegisterTime,
                              // strOutCipherVkek,
                              // &puiVemkVer,
                              // strOutCipoherVemk);
    if(rv)
    {
        printf("return code = [0X%010X]\n", rv);
        return;
    }
    printf("VKEK密文数据 = [%s]\n", strOutCipherVkek);
    if(puiVemkVer)
    {
        printf("更新后的VEMK = [%s]\n", strOutCipoherVemk);
    }
}



void T_RegisterVKEKRecord()
{
    int rv = 0;
    unsigned int uiAlgorithm = 0;
    unsigned int uiVemkVer = 0;
    char strIpcId[64] = {0};
    char strVkekVersion[64] = {0};
    char strNVRId[32] = {0};
    char strRegisterTime[32] = {0};

    printf("please input IPC ID(20H):\n");
    scanf("%s", strIpcId);

    printf("please input NVR ID(20H):\n");
    scanf("%s", strNVRId);

    printf("please input Vkek Version(20H):\n");
    scanf("%s", strVkekVersion);

    printf("please input Algorithm(1:SM1 2:SM4):\n");
    scanf("%ud", &uiAlgorithm);

    if(uiAlgorithm == 1)
    {
        uiAlgorithm = VGD_SM1_ECB;
    }
    else
    {
        uiAlgorithm = VGD_SM4_ECB;
    }

    //printf("please input Register Time:\n");
    //scanf("%s", strRegisterTime);
    strcpy(strRegisterTime, "2014-10-03 19:36:00");

    printf("please input uiVemkVer:\n");
    scanf("%ud", &uiVemkVer);

    rv = VKF_RegisterVKEKRecord( strIpcId,
                                 strNVRId,
                                 strVkekVersion,
                                 uiAlgorithm,
                                 uiVemkVer,
                                 strRegisterTime);
    if ( rv )
    {
        printf("VKF_RegisterVKEKRecord Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("VKF_RegisterVKEKRecord Success!!!\n");
}


void T_SearchVKEK_Version()
{
    int rv = 0;
    char strIpcId[64] = {0};
    char strVkekVersion[64] = {0};
    VKEKRecord pstOutVkek;

    memset(&pstOutVkek, 0x00, sizeof(VKEKRecord));

    printf("please input IPC ID:\n");
    scanf("%s", strIpcId);

    printf("please input Vkek Version:\n");
    scanf("%s", strVkekVersion);

    rv = VKF_SearchVKEK_Version( strIpcId,
                                 strVkekVersion,
                                 &pstOutVkek/*out*/);
    if ( rv )
    {
        printf("T_SearchVKEK_Version Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("设备编号 = [%s]\n", pstOutVkek.strDeviceId);
    printf("VKEK版本 = [%s]\n", pstOutVkek.strVkekVersion);
    printf("VKEK生成时间 = [%s]\n", pstOutVkek.strRegisterTime);
    printf("密钥算法标识 = [%d]\n", pstOutVkek.uiAlgorithm);
}

void T_ExportVKEK_Version()
{
    int rv = 0;
    char strIpcId[64] = {0};
    char strKEKVersion[64] = {0};
    VKEKRecord pstOutVkek;
    int iVkekLen = sizeof(VKEKRecord);

    char strPubKey_HEX[256] = {0};
    unsigned char pucPubKey[256] = {0};
    unsigned int uiPubKeyL = 0;

    memset(&pstOutVkek, 0x00, sizeof(VKEKRecord));

    printf("please input IPC ID:\n");
    scanf("%s", strIpcId);

	printf("please input public key (HEX string):\n");
    scanf("%s", strPubKey_HEX);
	
    printf("please input Vkek Version:\n");
    scanf("%s", strKEKVersion);

  

    PackBCD(strPubKey_HEX, pucPubKey, (int)strlen(strPubKey_HEX));
    uiPubKeyL = strlen(strPubKey_HEX) / 2;
    
    rv = VKF_ExportVKEK_Version( strIpcId,
        strKEKVersion,
        pucPubKey,
        uiPubKeyL,
		&iVkekLen,
        &pstOutVkek
    	);
    if ( rv ) {
        printf("T_SearchVKEK_Version Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
	
    printf("设备编号 = [%s]\n", pstOutVkek.strDeviceId);
    printf("记录时间 = [%s]\n", pstOutVkek.strRegisterTime);
    printf("密钥版本 = [%s]\n", pstOutVkek.strVkekVersion);
    printf("密钥算法标识 = [%d]\n", pstOutVkek.uiCipherVkeyLen);
	
	char out[256];
	memset(out, 0x00, 256);
	UnpackBCD( pstOutVkek.pucCipherVkey,  out, pstOutVkek.uiCipherVkeyLen * 2 );
    printf("密钥密文 = [%s]\n", out);
    printf("校验值 = [%s]\n", pstOutVkek.strKcv);
    
}


void T_DecryptVideo()
{
    int rv = 0;
    unsigned int uiAlgorithmVek = 0;
    char strIpcId[64] = {0};
    char strVkekVersion[64] = {0};
    char strCipherVek[128] = {0};
    char pucIV[33] = {0};
    char pucIV_BCD[16] = {0};

    unsigned char pucCipherData[512] = {0};
    unsigned char pucCipherData_BCD[512] = {0};
    unsigned int  uiCipherDataL = 0;
    unsigned char pucOutPlainData[512] = {0};
    unsigned int puiOutPlainDataL = 0;
    char OutBuf[512] = {0};
    char OutIV[33] = {0};
    /*
        printf("please input IPC ID:\n");
        scanf("%s", strIpcId);

        printf("please input Cipher Vek:\n");
        scanf("%s", strCipherVek);

        printf("please input Vkek Version:\n");
        scanf("%s", strVkekVersion);

        printf("please input uiAlgorithmVek(1:SM1 2:SM4):\n");
        scanf("%ud", &uiAlgorithmVek);

        if(uiAlgorithmVek == 1)
        {
            uiAlgorithmVek = VGD_SM1_ECB;
        }
        else
        {
            uiAlgorithmVek = VGD_SM4_ECB;
        }

        printf("please input pucIV:\n");
        scanf("%s", pucIV);

        printf("please input pucCipherData:\n");
        scanf("%s", pucCipherData);
    */
    memcpy(strIpcId, "00000000000000000006", 20);
    memcpy(strVkekVersion, "12000000000000000036", 20);
    memcpy(strCipherVek, "AQIDBAUGBwgJCgsMDQ4PEA==", 24);
    uiAlgorithmVek = VGD_SM1_ECB;
    memcpy(pucIV, "00000000000000000000000000000000", 32);
    memcpy(pucCipherData, "F06F1E86522C4655DE1C2C65DE017C23EA005C6786941A7AE9B3F3CFDC09B462", 64);

    PackBCD(pucCipherData, pucCipherData_BCD, (int)strlen(pucCipherData));
    uiCipherDataL = strlen(pucCipherData) / 2;

    PackBCD(pucIV, pucIV_BCD, (int)strlen(pucIV));

    rv = VKF_DecryptVideo( strIpcId,
                           strVkekVersion,
                           strCipherVek,
                           uiAlgorithmVek,
                           pucIV_BCD/*in&out*/,
                           pucCipherData_BCD,
                           uiCipherDataL,
                           pucOutPlainData/*out*/,
                           &puiOutPlainDataL/*out*/);
    if ( rv )
    {
        printf("VKF_DecryptVideo Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }

    UnpackBCD( pucOutPlainData,  OutBuf, puiOutPlainDataL * 2 );
    UnpackBCD( pucIV_BCD,  OutIV, 32 );

    printf("数据明文 = [%s]\n", OutBuf);
}

int get_xml_data(char* datafile, char** xml_data)
{
	int rv = 0;
	int retcode = 0;
	FILE* fp = NULL;
	unsigned int size = 0, filesize = 0;
	struct stat statbuff;
	char* data = NULL;

	retcode = stat(datafile, &statbuff);
	if (retcode < 0 || statbuff.st_size == 0)
	{
		rv = -1;
		goto END;
	}

	filesize = statbuff.st_size;
	data = malloc(filesize);
	if (data == NULL)
	{
		rv = -2;
		goto END;
	}

	fp = fopen(datafile, "rb");
	if (fp == NULL)
	{
		free(data);
		rv = -3;
		goto END;
	}

	size = fread(data, 1, filesize, fp);
	if (size <= 0 || size != filesize)
	{
		fclose(fp);
		free(data);
		rv = -4;
		goto END;
	}
	fclose(fp);

	*xml_data = data;
	data = NULL;
END:
	return rv;
}

void T_GenerateSignalCheckData()
{
    int rv = 0;
    char strIpcId[64] = {0};
    char strSignalData[512] = "";
    char strOutCheckData[512] = "";
	char strMessagebody[1024] = "<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode></CivilCode>\n<Block></Block>\n<Address></Address>\n<Parental>0</Parental>\n<ParentID>32050000002090000204</ParentID>\n<SafetyWay>0</SafetyWay>\n<RegisterWay>1</RegisterWay>\n<Secrecy>0</Secrecy>\n<IPAddress>192.168.5.169</IPAddress>\n<Port>5231</Port>\n<Status>ON</Status>\n<Longitude>121.290</Longitude>\n<Latitude>31.110</Latitude>\n</Item>\n</DeviceList>\n</Response>\n";


    rv = VKF_GenerateSignalCheckData( "00000000001321000001",
                                      "1234567890",
									 NULL,
                                     strOutCheckData/*out*/);
    if ( rv )
    {
        printf("VKF_GenerateSignalCheckData Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("信令验证数据 = %s\n", strOutCheckData);
}



void T_GenerateSignalCheckData_Ext()
{
    int rv = 0;
	char strPlatformId[64] = {0};
    char strIpcId[64] = {0};
    char strSignalData[512] = {0};
    char strOutCheckData[512] = {0};

	printf("please input Platform ID:\n");
    scanf("%s", strPlatformId);
	
    printf("please input IPC ID:\n");
    scanf("%s", strIpcId);

    printf("please input Signal Data:\n");
    scanf("%s", strSignalData);

    rv = VKF_GenerateSignalCheckData_Ext(strPlatformId, strIpcId, strSignalData, strOutCheckData/*out*/);
    if ( rv )
    {
        printf("VKF_GenerateSignalCheckData_Ext Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("信令验证数据 = (%s)\n", strOutCheckData);
}


void T_VeriySignalCheckData()
{
    int rv = 0;
    char strIpcId[64] = {0};
    char strSignalData[512] = "";
    char strCheckData[512] = {0};
	//char strMessagebody[512] = {0};
		char strMessagebody[1024] = "<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode></CivilCode>\n<Block></Block>\n<Address></Address>\n<Parental>0</Parental>\n<ParentID>32050000002090000204</ParentID>\n<SafetyWay>0</SafetyWay>\n<RegisterWay>1</RegisterWay>\n<Secrecy>0</Secrecy>\n<IPAddress>192.168.5.169</IPAddress>\n<Port>5231</Port>\n<Status>ON</Status>\n<Longitude>121.290</Longitude>\n<Latitude>31.110</Latitude>\n</Item>\n</DeviceList>\n</Response>\n";

 //printf("please input ipc id:\n");
//scanf("%s", strIpcId);

    // printf("please input Signal Data:\n");
    // scanf("%s", strSignalData);
	
	// printf("please input Messagebody Data:\n");
    // scanf("%s", strMessagebody);
	
printf("please input check data:\n");
scanf("%s", strCheckData);

    rv = VKF_VeriySignalCheckData( "00000000001321000001",
                                    "1234567890",
									NULL,
                                   strCheckData);
    if ( rv )
    {
        printf("VKF_VeriySignalCheckData Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("VKF_VeriySignalCheckData success!!\n");
}

int CipherTranse( unsigned char *derCipher, unsigned int derCipherLen, unsigned char *pucCipher_sm2, unsigned int uiCipher_sm2L, unsigned char **pucVkek_rank, unsigned int *uiAlgorithmTk)
{
    int rv = 0;

    return rv;

}

void T_ImportSipVKEK_WithSM2()
{

    return;
}


void T_SearchVKEK_Dates()
{
    int rv = 0;
    int i = 0;
    char strIpcId[64] = {0};
    char strStartDate[64] = {0};
    char strEndDate[128] = {0};

    char strOutId[64] = {0};
    char strDate[64] = {0};

    unsigned int puiOutCount = 10;
    //VKEKRecord pstOutVkeks[10] = {0};
    VKEKRecord *pstOutVkeks;

    printf("please input IPC ID:\n");
    scanf("%s", strIpcId);

    /*
    printf("please input Start Date:\n");
    scanf("%s", strStartDate);

    printf("please input End Date:\n");
    scanf("%s", strEndDate);
    */
    strcpy(strStartDate, "2014-09-03 19:36:00");
    strcpy(strEndDate, "2014-11-03 19:36:00");

    //printf("please input Out Count:\n");
    //scanf("%d", &puiOutCount);

    rv = VKF_SearchVKEK_Dates( strIpcId,
                               strStartDate,
                               strEndDate,
                               &puiOutCount/*in&out*/,
                               &pstOutVkeks/*out*/);
    if ( rv )
    {
        printf("VKF_SearchVKEK_Dates Failed!!! Error: %#10X!\r\n", rv);
        return ;
    }

    for(i = 0; i < puiOutCount; i++)
    {
        printf("---------  第 %d 条密钥信息 ----------\n", i + 1);
        memcpy(strOutId, (pstOutVkeks + i)->strDeviceId, 20);
        memcpy(strDate, (pstOutVkeks + i)->strRegisterTime, 19);
        printf("设备编号 = [%s]\n", strOutId);
        printf("记录时间 = [%s]\n", strDate);
    }
    free(pstOutVkeks);
}

void T_ImportVKEK()
{
    int rv = 0;
    unsigned int uiAlgorithm = 0;
    char strIpcId[64] = {0};
    char strVkekVersion[64] = {0};
    char strRegisterTime[32] = {0};
	char strVkekWithSM2[512] = {0};
	
    printf("please input IPC ID:\n");
    scanf_s("%s", strIpcId, 64);

    printf("please input uiAlgorithm(1:SM1 2:SM4):\n");
    scanf_s("%ud", &uiAlgorithm);

    printf("please input Vkek Version:\n");
    scanf_s("%s", strVkekVersion, 64);

    if(uiAlgorithm == 1)
    {
        uiAlgorithm = VGD_SM1_ECB;
    }
    else
    {
        uiAlgorithm = VGD_SM4_ECB;
    }

    strcpy(strRegisterTime, "2018-12-17 19:36:00");

    rv = VKF_ImportVKEK(strIpcId,
	strIpcId,
	strVkekVersion,
	uiAlgorithm,
	strRegisterTime,
	strVkekWithSM2);
    if ( rv )
    {
        printf("VKF_ImportVKEK Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("VKF_ImportVKEK Success. \n");
	printf("strVkekWithSM2=%s \n", strVkekWithSM2);
}


void T_VeriySignalCheckData_Ext()
{
	int rv = 0;
    char strIpcId[64] = {0};
    char strSignalData[512] = {0};
    char strCheckData[512] = {0};
	char strPlatformId[128] = {0};

	printf("please input Platform ID:\n");
    scanf("%s", strPlatformId);
	
    printf("please input IPC ID:\n");
    scanf("%s", strIpcId);

    printf("please input Signal Data:\n");
    scanf("%s", strSignalData);

    printf("please input Check Data:\n");
    scanf("%s", strCheckData);

    rv = VKF_VeriySignalCheckData_Ext( strPlatformId,
									strIpcId,
                                   strSignalData,
                                   strCheckData);
    if ( rv )
    {
        printf("VKF_VeriySignalCheckData_Ext Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("VKF_VeriySignalCheckData_Ext success!!\n");

}

void T_GetDevSignPubKey()
{
	int rv;
	char strSignPubKey[256];

	memset(strSignPubKey, 0x00, 256);
	
	rv = VKF_ExportSignPubKey(strSignPubKey);
	if ( rv )
    {
        printf("VKF_ExportSignPubKey Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("VKF_ExportSignPubKey success!!\n");
	printf("pubkey x|y:%s\n",strSignPubKey);

}

void T_ExportVKEKDatas()
{
	int rv;
	unsigned int uiTableIndex = 0;
	unsigned int uiSplitBegin = 0; 
	unsigned int uiDataCount = 0; 
	unsigned int uiVkekDataLen = 0; 
	unsigned char ucVkekData[4096];

	memset(ucVkekData, 0x00, 4096);

	printf("please input uiTableIndex[1,2,3,4]:\n");
    scanf_s("%ud", &uiTableIndex);

	printf("please input uiSplitBegin:\n");
    scanf_s("%ud", &uiSplitBegin);

	
	rv = VKF_ExportVKEKDatas(
		uiTableIndex, 
		uiSplitBegin, 
		&uiDataCount, 
		&uiVkekDataLen, 
		ucVkekData);
	if ( rv )
    {
        printf("VKF_ExportSignPubKey Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("VKF_ExportSignPubKey success!!\n");
	printf("uiDataCount = %d, uiVkekDataLen = %d\n",uiDataCount, uiVkekDataLen);
	printf("ucVkekData:%s\n",ucVkekData);
}


void T_SignDatas()
{
	int rv;
	char strSignRS[256], strData[33];
	char out[256];

	memset(strSignRS, 0x00, 256);
	memset(strData, 0x00, 33);
	memset(strData, 0x02, 32);
	memset(out, 0x00, 256);
	
	rv = VKF_DoSign(strData, strSignRS);
	if ( rv )
    {
        printf("VKF_DoSign Failed!!! Error: %#010X!\r\n", rv);
        return ;
    }
    printf("VKF_DoSign success!!\n");
	bin2hex(strSignRS, 64,out);
	printf("strSignRS :[%s]\n",out);
}

void T_VKF_ImportVKEK()
{
	int rv;
    char strPlatformId[128] = "province";
    //char strPlatformId[128] = "123456";
    char strDeviceId[128] = "00000000001320002020";
    char strVkekVersion[128] = "2020-05-23-14:56:38";
    unsigned int uiAlgorithmVkek = VGD_SM4_ECB;
    char strRegisterTime[20] = "2020-05-21 12:37:07";
    char strVkekWithSM2[1024] = "tbd28t/wrGx6k5fmMLvl7kFvZoVT+a1+Yam+lDMA9Xqf1Rc0gTQrDHcIN8vZathCvJpGqDxJaYxqdV8RG0URmDeyQZAoBeJRgRL0ud/NmcQKYNIKozfqtfYksKfup2LSzpP38umkeLuij/iABBZyFQ==";

	rv = VKF_ImportVKEK(strPlatformId, strDeviceId, strVkekVersion, uiAlgorithmVkek, strRegisterTime, strVkekWithSM2);
	if(rv){
		printf("VKF_ImportVKEK err rv=%d\n", rv);
	}else{
		printf("VKF_ImportVKEK success\n");
	}

	return;
}


void T_MultiPacketSignalCheckandVeriy()
{
	int rv;
	char strOutCheckData[512] = { 0 };
	rv = VKF_MultiPacketGenerateSignalCheckData("00000000001320000001", "<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode></CivilCode>\n<Block></Block>\n<Address></Address>\n<Parental>0</Parental>\n<ParentID>32050000002090000204</ParentID>\n<SafetyWay>0</SafetyWay>\n<RegisterWay>1</RegisterWay>\n<Secrecy>0</Secrecy>\n<IPAddress>192.168.5.169</IPAddress>\n<Port>5231</Port>\n<Status>ON</Status>\n<Longitude>121.290</Longitude>\n<Latitude>31.110</Latitude>\n</Item>\n</DeviceList>\n</Response>\n<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode><<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode></CivilCode>\n<Block></Block>\n<Address></Address>\n<Parental>0</Parental>\n<ParentID>32050000002090000204</ParentID>\n<SafetyWay>0</SafetyWay>\n<RegisterWay>1</RegisterWay>\n<Secrecy>0</Secrecy>\n<IPAddress>192.168.5.169</IPAddress>\n<Port>5231</Port>\n<Status>ON</Status>\n<Longitude>121.290</Longitude>\n<Latitude>31.110</Latitude>\n</Item>\n</DeviceList>\n</Response>\n<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode><" ,"<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode></CivilCode>\n<Block></Block>\n<Address></Address>\n<Parental>0</Parental>\n<ParentID>32050000002090000204</ParentID>\n<SafetyWay>0</SafetyWay>\n<RegisterWay>1</RegisterWay>\n<Secrecy>0</Secrecy>\n<IPAddress>192.168.5.169</IPAddress>\n<Port>5231</Port>\n<Status>ON</Status>\n<Longitude>121.290</Longitude>\n<Latitude>31.110</Latitude>\n</Item>\n</DeviceList>\n</Response>\n<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode><<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode></CivilCode>\n<Block></Block>\n<Address></Address>\n<Parental>0</Parental>\n<ParentID>32050000002090000204</ParentID>\n<SafetyWay>0</SafetyWay>\n<RegisterWay>1</RegisterWay>\n<Secrecy>0</Secrecy>\n<IPAddress>192.168.5.169</IPAddress>\n<Port>5231</Port>\n<Status>ON</Status>\n<Longitude>121.290</Longitude>\n<Latitude>31.110</Latitude>\n</Item>\n</DeviceList>\n</Response>\n<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode><", strOutCheckData);
	if (rv != VKR_OK)
	{
		printf("VKF_MultiPacketGenerateSignalCheckData   Failed!!! Error: %#x!\r\n", rv);
		return -1;
	}
	else
	{
		printf("VKF_MultiPacketGenerateSignalCheckData success。\r\n\r\n");
	}




	rv = VKF_MultiPacketVeriySignalCheckData("00000000001320000001", "<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode></CivilCode>\n<Block></Block>\n<Address></Address>\n<Parental>0</Parental>\n<ParentID>32050000002090000204</ParentID>\n<SafetyWay>0</SafetyWay>\n<RegisterWay>1</RegisterWay>\n<Secrecy>0</Secrecy>\n<IPAddress>192.168.5.169</IPAddress>\n<Port>5231</Port>\n<Status>ON</Status>\n<Longitude>121.290</Longitude>\n<Latitude>31.110</Latitude>\n</Item>\n</DeviceList>\n</Response>\n<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode><<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode></CivilCode>\n<Block></Block>\n<Address></Address>\n<Parental>0</Parental>\n<ParentID>32050000002090000204</ParentID>\n<SafetyWay>0</SafetyWay>\n<RegisterWay>1</RegisterWay>\n<Secrecy>0</Secrecy>\n<IPAddress>192.168.5.169</IPAddress>\n<Port>5231</Port>\n<Status>ON</Status>\n<Longitude>121.290</Longitude>\n<Latitude>31.110</Latitude>\n</Item>\n</DeviceList>\n</Response>\n<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode><", "<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode></CivilCode>\n<Block></Block>\n<Address></Address>\n<Parental>0</Parental>\n<ParentID>32050000002090000204</ParentID>\n<SafetyWay>0</SafetyWay>\n<RegisterWay>1</RegisterWay>\n<Secrecy>0</Secrecy>\n<IPAddress>192.168.5.169</IPAddress>\n<Port>5231</Port>\n<Status>ON</Status>\n<Longitude>121.290</Longitude>\n<Latitude>31.110</Latitude>\n</Item>\n</DeviceList>\n</Response>\n<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode><<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode></CivilCode>\n<Block></Block>\n<Address></Address>\n<Parental>0</Parental>\n<ParentID>32050000002090000204</ParentID>\n<SafetyWay>0</SafetyWay>\n<RegisterWay>1</RegisterWay>\n<Secrecy>0</Secrecy>\n<IPAddress>192.168.5.169</IPAddress>\n<Port>5231</Port>\n<Status>ON</Status>\n<Longitude>121.290</Longitude>\n<Latitude>31.110</Latitude>\n</Item>\n</DeviceList>\n</Response>\n<?xml version=\"1.0\"?>\n<Response>\n<CmdType>Catalog</CmdType>\n<SN>7</SN>\n<DeviceID>32050000001320000169</DeviceID>\n<SumNum>1</SumNum>\n<DeviceList Num=\"1\">\n<Item>\n<DeviceID>32050000001320000169</DeviceID>\n<Name>enc</Name>\n<Manufacturer>KEDACOM</Manufacturer>\n<Model>ZD5920-GN</Model>\n<Owner></Owner>\n<CivilCode><", strOutCheckData);
	if (rv != VKR_OK)
	{
		printf("VKF_MultiPacketVeriySignalCheckData  Failed!!! Error: %#x!\r\n", rv);
		return -1;
	}
	else
	{
		printf("VKF_MultiPacketVeriySignalCheckData  success。\r\n\r\n");
	}
	
}


int main(void)
{
    int rv, sel;
    srand( (unsigned int)time(NULL) );
    char pcConfigFilePath[100] = "./cacipher.ini";

    rv = VKF_Initialize(pcConfigFilePath);
    if (rv != VKR_OK)
    {
        printf("设备接口初始化失败!!! Error: %#x!\r\n", rv);
        return -1;
    }
    else
    {
        printf("设备接口初始化成功。\r\n\r\n");
    }
	
	char strOutCheckData[512] = { 0 };

	
    printf("**************************************************************************\r\n\r\n");

    for ( ; ; )
    {
        printf( "-------------    VKF API test    ------------\n" );
        printf( "|                                           |\n" );
        printf( "|        1.  Search Device.                 |\n" );
        printf( "|        2.  Generate VEMK.                 |\n" );
        printf( "|        3.  Delete VEMK.                   |\n" );
        printf( "|        4.  Generate IPC VKEK.             |\n" );
        printf( "|        5.  Generate NVR VKEK.             |\n" );
        printf( "|        6.  Record Register VKEK.          |\n" );
        printf( "|        7.  SearchVKEK by Version.         |\n" );
        printf( "|        8.  Export VKEK by Version.        |\n" );
        printf( "|        9.  Decrypt Video.   	             |\n" );
        printf( "|        10. Generate Signal Check Data.    |\n" );
        printf( "|        11. Veriy Signal CheckData.        |\n" );
        printf( "|        12. Search VKEK by Dates.          |\n" );
        printf( "|        13. bat 20w VKEK Datas.            |\n" );
	    printf( "|        15. VKF_ImportVKEK.                |\n" );
        printf( "|        16. Veriy Signal CheckData Ext.    |\n" );
		printf( "|        17. Get Dev Sign PubKey.           |\n" );
		printf( "|        18. Get VKEK Parms List.           |\n" );
		printf( "|        19. dev signkey sign data.         |\n" );
		printf( "|        20. pri key descrypto input vkek.  |\n" );
		printf( "|        21. Generate Signal Check Data Ext.|\n" );
		printf( "|        22. T_GenerateVEMKOld.         	 |\n" );
		printf( "|        23. T_MultiPacketSignalCheckandVeriy.|\n" );
        printf( "|        0.  Exit.                          |\n" );
        printf( "|                                           |\n" );
        printf( "---------------------------------------------\n" );
        printf( "\nPlease Select:" );
        scanf_s( "%d", &sel );

        switch( sel )
        {
        case 1:
            T_SearchDevice();
            break;
        case 2:
            T_GenerateVEMK();
            break;
        case 3:
            T_DeleteVEMK();
            break;
        case 4:
            T_GenerateIpcVKEK();
            break;
        case 5:
            T_GenerateNvrVKEK();
            break;
        case 6:
            T_RegisterVKEKRecord();
            break;
        case 7:
            T_SearchVKEK_Version();
            break;
        case 8:
            T_ExportVKEK_Version();
            break;
        case 9:
            T_DecryptVideo();
            break;
        case 10:
            T_GenerateSignalCheckData();
            break;
        case 11:
            T_VeriySignalCheckData();
            break;
        case 12:
            T_SearchVKEK_Dates();
            break;
        case 13:
            T_BatVKEK_Dates();
            break;
        case 14:
            T_ImportSipVKEK_WithSM2();
            break;
		case 15:
            T_ImportVKEK();
            break;
		case 16:
            T_VeriySignalCheckData_Ext();
            break;
		case 17:
            T_GetDevSignPubKey();
            break;
		case 18:
            T_ExportVKEKDatas();
            break;
		case 19:
            T_SignDatas();
            break;
		case 20:
            T_VKF_ImportVKEK();
            break;
		case 21:
            T_GenerateSignalCheckData_Ext();
            break;
		case 22:
            T_GenerateVEMKOld();
            break;
		case 23:
			while(1)
			{
				T_MultiPacketSignalCheckandVeriy();
			}
			break;
        case 0:
            VKF_UnInitialize();
            printf( "Test Finished.\n" );
            return 0;
        default:
            printf( "Invalid Input, Press <Enter> Key to Continue...\n" );
            break;
        }
        getchar();
        getchar();
    }

    return 0;
}

