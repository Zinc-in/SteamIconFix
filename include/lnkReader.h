
#include <windows.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <unordered_map>

#define         LNK_SEEK_BEG            1
#define         LNK_SEEK_END            2
#define         LNK_SEEK_CUR            3

using namespace std;

class lnkReader {
public:

    typedef struct _tagLinkFileHeader {
        DWORD       HeaderSize;     //�ļ�ͷ�Ĵ�С
        GUID        LinkCLSID;      //CLSID
        DWORD       Flags;          //lnk�ļ���־
        DWORD       FileAttributes; //�ļ�����(Ŀ���ļ���)
        FILETIME    CreationTime;   //����ʱ��(Ŀ���ļ���)
        FILETIME    AccessTime;     //����ʱ��
        FILETIME    WriteTime;      //�޸�ʱ��
        DWORD       FileSize;       //�ļ���С
        DWORD       IconIndex;
        DWORD       ShowCommand;    //��ʾ��ʽ
        WORD        Hotkey;         //�ȼ�
        BYTE        retain[10];     //������10byte����
    }LINKFILE_HEADER, * LPLINKFILE_HEADER;

    typedef struct _tagLinkItemID {
        WORD        wSize;
        BYTE        bType;
        BYTE* bData;

        inline int getTypeData() { return (bType & 0xFL); };
        inline int getListType() { return ((bType & 0xF0L) >> 4); };

    }ITEMID, * LPITEMID;

    typedef struct _tagItemType {
        static const BYTE      ROOT     = 1;
        static const BYTE      VOLUME   = 2;
        static const BYTE      FILE     = 3;
    }ITEM_TYPE;

public:

    lnkReader() = default;

    int run(string stLnkPath)
    {
        lnkFile.open(stLnkPath, ios::in | ios::binary);     //�Զ�����ֻ���ķ�ʽ���ļ���
        if (!lnkFile.is_open())
            return -1;      //��ʧ��,����-1

        //��ȡ�ļ���С
        seek(LNK_SEEK_END, 0);  //������������λ��
        tagFileSize = tell();   //ͨ��tell��ȡ�ļ���С
        seek(LNK_SEEK_BEG, 0);   //��������ͷ

        Assert(tagFileSize < (LONGLONG)sizeof(LINKFILE_HEADER), "The file is too small.");  //�ļ���С����û��һ��lnk�ļ�ͷ��, �������ļ�����

        //��ȡ4���ֽ��ж��Ƿ���lnk�����ļ�
        read(&lnkHeader.HeaderSize, 4);
        if (lnkHeader.HeaderSize != 0x4c)//������lnk�ļ��򷵻� -2
        {
            return -2;
        }
        //�ƶ����ݵ��ļ���ͷ��λ��, ��Ϊ��ȡ�ļ�ͷ4���ֽ�ʱ�ı����ļ���ȡ��λ��
        seek(LNK_SEEK_BEG, 0);

        //��ȡ�ļ�ͷ
        read(&lnkHeader, sizeof(LINKFILE_HEADER));

        //��ȡ��ݷ�ʽָ����ļ�·��
        getLinkTargetIDList();



        return 0;
    }
    string getPath() { return tagFilePath; }

private:
    fstream lnkFile;    //�ļ���
    LONGLONG tagFileSize;   //�ļ���С
    LINKFILE_HEADER lnkHeader;  //lnk�ļ�ͷ

    string tagFilePath;

    //�Լ�д�Ķ��Ժ���
    void Assert(bool condition, string description)
    {
        if (condition)
        {
            MessageBoxA(0, description.c_str(), "Error", MB_ICONERROR);
            throw "assert.";
        }
    }
    //��������
    void seek(DWORD point, int offset)
    {
        switch (point)
        {
        case LNK_SEEK_BEG:        //������ͷ����ƫ��
            lnkFile.seekg(offset, ios::beg);
            break;
        case LNK_SEEK_CUR:        //��ǰλ�ý���ƫ��
            lnkFile.seekg(offset, ios::cur);
            break;
        case LNK_SEEK_END:        //����λ�ý���ƫ��
            lnkFile.seekg(offset, ios::end);

        default:
            break;
        }
    }
    //��ȡָ�뵱ǰ��λ��
    LONGLONG tell()
    {
        return lnkFile.tellg();
    }
    //��ȡ����
    void read(PVOID pData, int szCount)
    {
        if (pData == nullptr)
            seek(LNK_SEEK_CUR, szCount);    //�����ֽ�
        else
            lnkFile.read((char*)pData, szCount);
    }
    //��ʼ���ڴ�
    void memzero(PVOID pDst, int iSize)
    {
        ZeroMemory(pDst, iSize);
    }
    //���������л�ȡһ���������ַ���, �����ַ�����(����������)
    int getstring(string& src)
    {
        src = "";
        int n = 0;
        for (char ch[2] = "";; n++)
        {
            read(ch, 1);
            if (ch[0] == '\0')
                break;

            src += string(ch);
        }

        return n + 1;
    }

    void getLinkTargetIDList()
    {
        Assert(!(lnkHeader.Flags & 0x1), "The .lnk file doesn't have the HasLinkTargetIDList flag, so we cannot continue.");

        int szIDList = 0;
        char* buf = NULL;
        string path = "";

        //��ȡIDList��С
        read(&szIDList, 2);

        Assert(szIDList == 0, "Failed to read the IDList size."); //��ȡIDList��С������ļ�����

        for (int szCurrent = 0; szCurrent < szIDList - 2;)
        {
            ITEMID ItemID;

            read(&ItemID.wSize, 2);      //��ȡ��С
            if (ItemID.wSize == 0)       //�ж��Ƿ��� TerminallID
                break;

            read(&ItemID.bType, 1); //��ȡ����

            //�ж�ItemID����
            switch (ItemID.getListType())
            {
            case ITEM_TYPE::ROOT:
            {
                //����
                read(nullptr, ItemID.wSize - 3);

                break;
            }
            case ITEM_TYPE::VOLUME:
            {
                //��ȡ�̷�
                char* buf = new char[ItemID.wSize - 3];
                memzero(buf, ItemID.wSize - 3);

                read(buf, ItemID.wSize - 3);
                Assert(!strcmp(buf, ""), "Failed to read dirver letter.");   //��ȡ�̷�ʧ��

                path += string(buf);

                break;
            }
            case ITEM_TYPE::FILE:
            {
                int iFileSize = 0;
                WORD DosData = 0, DosTime = 0, FileAttributes = 0;
                string str = "";

                read(nullptr, 1);  //����δ֪�ֽ�
                /* ע:read������һ��������NULL��ʾ������� szCount ���ֽ�, �൱�� seek(LNK_SEEK_CUR, szCount) ,���Կ���read��������ϸ��*/
                read(&iFileSize, 4);  //��ȡ�ļ���С(��ʱ����Ҫ)

                //��ȡ�ļ�(�ļ���)�������ں�ʱ��
                read(&DosData, 2);
                read(&DosTime, 2);

                read(&FileAttributes, 2);           //��ȡ�ļ�(�ļ���)����
                int reads = getstring(str);         //��ȡ�ļ�����

                Assert(str == "", "Failed to read file name.");  //��ȡ�ļ�����ʧ��

                path += str + R"(\)";

                //read(nullptr, ItemID.wSize - 1 - 4 - 2 - 2 - 2 - reads - 3)
                read(nullptr, ItemID.wSize - reads - 14);   //��ʱ����Ҫ������Ϣ

                break;
            }
            default:
                Assert(1, "Failed to read ItemID.");  //ItemID����δ֪
            }

            szCurrent += ItemID.wSize;
        }

        path = path.substr(0, path.size() - 1); //ɾȥ���һ�������"\"
        tagFilePath = path;

    }


};



