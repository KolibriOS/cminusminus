typedef struct _BIT_{
	unsigned int siz:8;
	unsigned int ofs:24;
}BIT;

struct _PROCINFO_
{
	char *buf;	//���� ⥪�� ��楤���
	void *classteg;	//���� ⥣�, ��� ��।����� ��楤��
	unsigned int warn:1;
	unsigned int speed:1;
	unsigned int lst:1;
	unsigned int typestring:2;
	unsigned int inlinest:1;
	unsigned int code32:1;
	unsigned int align:1;
	unsigned int acycle:1;
	unsigned int idasm:1;
	unsigned int opnum:1;
	unsigned int de:1;
	unsigned int ostring:1;
	unsigned int uselea:1;
	unsigned int regoverstack:1;
	unsigned int sizeacycle;
	char chip;
};

struct idrec
{
	union{
		struct idrec *left;
		struct localrec *next;
	};
	struct idrec *right;	//�।��� � ᫥���� ������
	char recid[IDLENGTH];	//���
	unsigned int flag;
	char *newid;  //���� � ���묨, ��� ������� ���� ⥣�,��� ��楤�� ��ࠬ����
	int rectok;		//⨯
	int recrm;    //��� ������� �᫮ �����
	int recsegm;
	int recpost;
	int recsize;
	int recsib;
	int line;	//����� �����
	int file;	//䠩�
	int count;	//���稪 �ᯮ�짮�����
	unsigned short type;
	unsigned short npointr;
	union{
		char *sbuf;	//㪠��⥫� �� ���� ��室���� ⥪��
		_PROCINFO_ *pinfo;
	};
	union{
		long recnumber;
		long long reclnumber;
		double recdnumber;
		float recfnumber;
	};
};

struct localinfo
{
	int usedfirst;
	int usedlast;
	int start;
	int end;
	int level;
	int count;
};

struct localrec
{
/*	struct localrec *next;
	int localtok;
	unsigned short type;
	unsigned short npointr;
	union{
		unsigned int localnumber;
		idrec *rec;
	};
	int locsize;
	char localid[IDLENGTH];
	unsigned char fuse;	//䫠� �ᯮ�짮�����
	unsigned char flag;	//䫠� static*/
	idrec rec;
	localinfo li;
	unsigned char fuse;	//䫠� �ᯮ�짮�����
};

#define INITBPPAR 1	//���樠������ BP ��᫥ ��ࠬ��஢
#define INITBPLOC 2 //���樠������ BP ��᫥ ��������
#define INITBPENTER 4
#define INITBPADDESP 8

struct HEADLOC
{
	int type;	//⨯ ���������
	unsigned int ofs; //���� ���祭��
	unsigned int num;	//����稭� ���祭��
};

struct treelocalrec
{
	treelocalrec *next;
	localrec *lrec;
	int initbp;
	int level;
	unsigned int addesp;
	int endline;
};

typedef struct _ITOK_
{
	int rm;
	int segm;
	int post;
	int sib;
	union{
		long number;
		long long lnumber;
		double dnumber;
		float fnumber;
	};
	union{
		int size;
		BIT bit;
	};
	unsigned short type;
	unsigned short npointr;
union{
 		idrec *rec;
		localrec *locrec;
	};
	char name[IDLENGTH];
	unsigned int flag;
}ITOK;

struct elementteg
{
	union{
		void *nteg;	//���� ⥣� ��������� ��������
		idrec *rec;
	};
	int tok;
	union{
		unsigned int numel;	//�᫮ ����⮢ �⮣� ⨯�
		BIT bit;
	};
	unsigned int ofs;	//ᬥ饭�� �� ��砫� ��������
	char name[IDLENGTH];
};

struct structteg
{
	struct structteg *left;	//᫥���騩 ⥣
	struct structteg *right;	//᫥���騩 ⥣
	unsigned int size;	//ࠧ��� ⥣�
	unsigned int numoper;	//�᫮ ���࠭��� ��������
	struct elementteg *baza;	//���� � ���ᠭ��� ����⮢ ⥣�
	unsigned int flag;
	char name[IDLENGTH];
};

struct listexport
{
	long address;
	char name[IDLENGTH];
};

typedef struct _IOFS_
{
	unsigned int ofs;
	unsigned int line;	//����� �����
	unsigned int file;	//䠩�
	unsigned char dataseg;
}IOFS;

typedef struct _UNDEFOFF_
{
	struct _UNDEFOFF_ *next;
	IOFS *pos;	//���� � ���ᠬ� ��㤠 ��뫪�
	int num;	//�᫮ ��뫮� �� ��� ����
	char name[IDLENGTH];
}UNDEFOFF;

typedef struct _LISTCOM_
{
	char name[IDLENGTH];
}LISTCOM;

typedef struct _SINFO_
{
	char *bufstr;
	int size;
}SINFO;

//������� ᯨ᪠ api-��楤��
typedef struct _APIPROC_
{
	struct idrec *recapi;
}APIPROC;

//
typedef struct _DLLLIST_
{
	struct _DLLLIST_ *next;	//᫥����� DLL
	struct _APIPROC_ *list;	//ᯨ᮪ ��楤��
	unsigned short num;     //�᫮ ��楤��
	char name[IDLENGTH];	//��� DLL
}DLLLIST;

typedef struct _PE_HEADER_
{
	long sign;	//ᨣ����� - �ᥣ��  'PE'
	short cpu;    //��� ⨯ CPU - �ᥣ�� 0x14C
	short numobj;	//�᫮ �室�� � ⠡���� ��ꥪ⮢
	long date_time;	//��� ����䨪�樨 �����஬
	long pCOFF;
	long COFFsize;
	short NTheadsize;	//ࠧ��� ��������� PE �� MAGIC - �ᥣ�� 0xE0
	short flags;
	short Magic;	//�����祭�� ��ࠬ��
	short LinkVer;	//����� ������
	long sizecode;
	long sizeinitdata;
	long sizeuninitdata;
	long EntryRVA;	//���� �⭮�� IMAGE BASE �� ���஬� ��।����� �ࠢ�����
	long basecode;	//RVA ᥪ��, ����� ᮤ�ন� �ணࠬ��� ���
	long basedata;	//RVA ᥪ��,ᮤ�ঠ�� �����
	long ImageBase;	//����㠫�� ��砫�� ���� ����㧪� �ணࠬ��
	long objAlig;	//��ࠢ������� �ணࠬ���� ᥪ権
	long fileAlig;	//��ࠢ������� ᥪ権 � 䠩��
	long OSver;	//����� ���ᨨ ���� ��⥬� ����� �ணࠬ��
	long userver;
	long SubSysVer;
	long rez;
	long imagesize;	//ࠧ��� � ����� ����㦠����� ��ࠧ� � ����������� ��ࠢ����
	long headsize;	//ࠧ� ��� ���������� stub+PE+objtabl
	long checksum;
	short SubSys;	//����樮���� ��� ����� ��� ����᪠
	short DLLflag;
	long stackRezSize;
	long stackComSize;
	long heapRezSize;
	long heapComSize;
	long loaderFlag;
	long numRVA;	//�ᥣ�� 10
	long exportRVA;
	long exportSize;
	long importRVA;
	long importSize;
	long resourRVA;
	long resourSize;
	long exceptRVA;
	long exceptSize;
	long securRVA;
	long securSize;
	long fixupRVA;
	long fixupSize;
	long debugRVA;
	long debugSize;
	long descripRVA;
	long descripSize;
	long machinRVA;
	long machinSize;
	long tlsRVA;
	long tlsSize;
	long loadConfRVA;
	long loadConfSize;
	long rez2[2];
	long iatRVA;
	long iatSize;
	long rez3[6];
}PE_HEADER;

typedef struct _OBJECT_ENTRY_
{
	char name[8];
	long vsize;
	long sectionRVA;
	long psize;
	long pOffset;
	unsigned long PointerToRelocations;
	unsigned long PointerToLinenumbers;
	unsigned short NumberOfRelocations;
	unsigned short NumberOfLinenumbers;
	long flags;
}OBJECT_ENTRY;

typedef struct _EXPORT_TABLE_
{
	unsigned long Flags;
	unsigned long Time;
	unsigned short Version[2];
	unsigned long NameRVA;
	unsigned long OriginalBase;
	unsigned long NumFunc;
	unsigned long NumName;
	unsigned long AddressRVA;
	unsigned long NamePRVA;
	unsigned long OrdinalRVA;
}EXPORT_TABLE;

#if !defined(__WIN32__)
struct ftime {
	unsigned ft_tsec:5;  /* ��� ᥪ㭤� */
	unsigned ft_min:6;   /* ������ */
	unsigned ft_hour:5;  /* ��� */
	unsigned ft_day:5;   /* ���� */
	unsigned ft_month:4; /* ����� */
	unsigned ft_year:7;  /* ���-1980 */
};
#else
#include <io.h>
#include <sys/stat.h>
#endif

typedef struct _STRING_LIST_
{
	void *next;	//᫥����� �������
	unsigned int len; //����� ��ப�
	unsigned int ofs;	//���� � ��室��� 䠩��
	unsigned char type;	//⨯ �ନ����
	unsigned char plase;	//��� ᥩ�� ��ப� - post or data
}STRING_LIST;

struct FILEINFO
{
	char *filename;
	int numdline;
	idrec *stlist;
	union{
		struct ftime time;
		unsigned short lineidx[2];
	};
};

struct EWAR{
	FILE *file;
	char *name;
};

typedef struct _ICOMP_
{
	unsigned int type;
	unsigned int loc;
	unsigned int use_cxz;
}ICOMP;

typedef struct _RETLIST_
{
	unsigned int line;
	unsigned int loc;
	unsigned int type;
//	int use;
}RETLIST;

enum{
	singlcase,startmulti,endmulti};

typedef struct _ISW_
{
	unsigned char type;
	unsigned int postcase;
	unsigned long value;
}ISW;

struct postinfo
{
	unsigned int loc;
	unsigned int num;
	unsigned short type;
	unsigned short line;
	unsigned short file;

};

typedef struct _EXE_DOS_HEADER_
{
	unsigned short sign;
	unsigned short numlastbyte;
	unsigned short numpage;
	unsigned short numreloc;
	unsigned short headsize;
	unsigned short minmem;
	unsigned short maxmem;
	unsigned short initSS;
	unsigned short initSP;
	unsigned short checksum;
	unsigned short initIP;
	unsigned short initCS;
	unsigned short ofsreloc;
	unsigned short overlay;
	unsigned long  fullsize;
}EXE_DOS_HEADER;

typedef struct _FSWI_
{
	ISW *info;
	int sizetab;	//�᫮ ����⮢
	int type;	//ࠧ�來����
	int numcase;	//�᫮ �ᯮ��㥬�� ����⮢
	int defal;	//���祭�� �� 㬮�砭��.
	int ptb;	//���� 㪠��⥫� �� ��� ⠡���� � ����� ����
	int ptv;	//���� ⠢���� ����稭
	int mode;	//⨯ switch
	int razr;	//ࠧ�來���� ����稭
}FSWI;

struct paraminfo
{
	unsigned int ofspar;
	unsigned char type[8];
};

struct MEOSheader
{
	unsigned char sign[8];
	unsigned long vers;
	unsigned long start;
	unsigned long size;
	unsigned long alloc_mem;
	unsigned long esp;
	unsigned long I_Param;
	unsigned long I_Icon;
};

#ifdef OPTVARCONST

struct LVIC{
	idrec *rec;
//	int blocks;
	int typevar;
	int contype;	//⨯ ᮤ�ন����
	union{
		long number;
		long long lnumber;
		double dnumber;
		float fnumber;
	};
};

struct BLVIC
{
	int sizevic;
	LVIC *listvic;
};

#endif

#define SIZEIDREG 256
#define NOINREG 8
#define SKIPREG 9

struct REGEQVAR
{
	REGEQVAR *next;
	char name[IDLENGTH];
	unsigned char razr;
};

struct REGISTERSTAT
{
	union{
		REGEQVAR *next;
#ifdef OPTVARCONST
		BLVIC *bakvic;
#endif
	};
	union{
		char id[SIZEIDREG];
		void *stringpar;
		unsigned long number;
	};
	unsigned char type;
	unsigned char razr;
};

struct SAVEREG
{
	unsigned int size;	//ࠧ��� ����� ��� ॣ���஢
	unsigned char all;	//�� ॣ�����
	unsigned char reg[8];	//���� ॣ���஢
};

struct SAVEPAR
{
 unsigned char ooptimizespeed;
 unsigned char owarning;
 unsigned char odbg;
 unsigned char odosstring;
 unsigned char ouseinline;
 unsigned char oam32; 		      // ०�� 32 ��⭮� ����樨
 unsigned char oalignword;
 unsigned char oAlignCycle;       //��ࠢ������ ��砫� 横���
 unsigned char oidasm;	//��ᥬ����� ������樨 ����� �����䨪��ࠬ�
 int ooptnumber;
 int odivexpand;
 unsigned char ooptstr;	//��⨬����� ��ப���� ����⠭�
 unsigned char ochip;
 int           oaligncycle;
 unsigned char ouselea;
 unsigned char oregoverstack;
};

struct COM_MOD
{
	COM_MOD *next;
	unsigned char *input; 	 /* dynamic input buffer */
	unsigned int endinptr;		 /* end index of input array */
	unsigned int inptr; 		 /* index in input buffer */
	unsigned int inptr2; 		 /* index in input buffer */
	unsigned int linenumber;
	unsigned int currentfileinfo;
	int numparamdef;	//�᫮ ��ࠬ��஢ � ⥪�饬 define
	char *declareparamdef;	//ᯨ᮪ ������� ��ࠬ��஢ define
	char *paramdef;	//ᯨ᮪ ����� ��ࠬ��஢
	int freze;	//䫠� ����饭�� 㤠����� ��������
};

struct LISTRELOC {
	unsigned int val;
};

struct LISTFLOAT
{
	union{
		float fnum;
		double dnum;
		unsigned long num[2];
	};
	int type;
	unsigned int ofs;
};

struct LILV
{
	unsigned int ofs;
	int size;
	localrec *rec;
};

struct WARNACT
{
	void (*fwarn)(char *str,unsigned int line,unsigned int file);
	unsigned char usewarn;
};
