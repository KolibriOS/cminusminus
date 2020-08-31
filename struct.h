typedef struct _BIT_ {
  unsigned int siz : 8;
  unsigned int ofs : 24;
} BIT;

typedef struct _PROCINFO_ {
  char *buf; //адрес текста процедуры
  void *classteg; //адрес тега, где определена процедура
  unsigned int warn : 1;
  unsigned int speed : 1;
  unsigned int lst : 1;
  unsigned int typestring : 2;
  unsigned int inlinest : 1;
  unsigned int code32 : 1;
  unsigned int align : 1;
  unsigned int acycle : 1;
  unsigned int idasm : 1;
  unsigned int opnum : 1;
  unsigned int de : 1;
  unsigned int ostring : 1;
  unsigned int uselea : 1;
  unsigned int regoverstack : 1;
  unsigned int sizeacycle;
  char chip;
} PROCINFO;

typedef struct _idrec_ {
  union {
    struct _idrec_ *left;
    struct _localrec_ *next;
  };
  struct _idrec_ *right;  //предыдущ и следующ запись
  char recid[IDLENGTH]; //имя
  unsigned int flag;
  char *newid; //блок с даными, для структур адрес тега,для процедур параметры
  int rectok;  //тип
  int recrm;   //для структур число копий
  int recsegm;
  int recpost;
  int recsize;
  int recsib;
  int line;  //номер линии
  int file;  //файл
  int count; //счетчик использования
  unsigned short type;
  unsigned short npointr;
  union {
    char *sbuf; //указатель на блок исходного текста
    PROCINFO *pinfo;
  };
  union {
    long recnumber;
    long long reclnumber;
    double recdnumber;
    float recfnumber;
  };
} idrec;

typedef struct _localinfo_ {
  int usedfirst;
  int usedlast;
  int start;
  int end;
  int level;
  int count;
} localinfo;

typedef struct _localrec_ {
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
          unsigned char fuse;	//флаг использования
          unsigned char flag;	//флаг static*/
  idrec rec;
  localinfo li;
  unsigned char fuse; //флаг использования
} localrec;

#define INITBPPAR 1 //инициализация BP после параметров
#define INITBPLOC 2 //инициализация BP после локальных
#define INITBPENTER 4
#define INITBPADDESP 8

typedef struct _HEADLOC_ {
  int type;         //тип заголовка
  unsigned int ofs; //адрес значения
  unsigned int num; //величина значения
} HEADLOC;

typedef struct _treelocalrec_ {
  struct _treelocalrec_ *next;
  localrec *lrec;
  int initbp;
  int level;
  unsigned int addesp;
  int endline;
} treelocalrec;

typedef struct _ITOK_ {
  int rm;
  int segm;
  int post;
  int sib;
  union {
    long number;
    long long lnumber;
    double dnumber;
    float fnumber;
  };
  union {
    int size;
    BIT bit;
  };
  unsigned short type;
  unsigned short npointr;
  union {
    idrec *rec;
    localrec *locrec;
  };
  char name[IDLENGTH];
  unsigned int flag;
} ITOK;

typedef struct _elementteg_ {
  union {
    void *nteg; //адрес тега вложенной структуры
    idrec *rec;
  };
  int tok;
  union {
    unsigned int numel; //число элементов этого типа
    BIT bit;
  };
  unsigned int ofs; //смещение от начала структуры
  char name[IDLENGTH];
} elementteg;

typedef struct _structteg_ {
  struct _structteg_ *left;  //следующий тег
  struct _structteg_ *right; //следующий тег
  unsigned int size;       //размер тега
  unsigned int numoper;    //число операндов структуры
  elementteg *baza; //адрес с описанием элементов тега
  unsigned int flag;
  char name[IDLENGTH];
} structteg;

typedef struct _listexport_ {
  long address;
  char name[IDLENGTH];
} listexport;

typedef struct _IOFS_ {
  unsigned int ofs;
  unsigned int line; //номер линии
  unsigned int file; //файл
  unsigned char dataseg;
} IOFS;

typedef struct _UNDEFOFF_ {
  struct _UNDEFOFF_ *next;
  IOFS *pos; //буфер с адресами откуда ссылки
  int num;   //число ссылок на эту метку
  char name[IDLENGTH];
} UNDEFOFF;

typedef struct _LISTCOM_ {
  char name[IDLENGTH];
} LISTCOM;

typedef struct _SINFO_ {
  char *bufstr;
  int size;
} SINFO;

//структура списка api-процедур
typedef struct _APIPROC_ {
  idrec *recapi;
} APIPROC;

//
typedef struct _DLLLIST_ {
  struct _DLLLIST_ *next; //следующая DLL
  struct _APIPROC_ *list; //список процедур
  unsigned short num;     //число процедур
  char name[IDLENGTH];    //имя DLL
} DLLLIST;

typedef struct _PE_HEADER_ {
  long sign;      //сигнатура - всегда  'PE'
  short cpu;      //мин тип CPU - всегда 0x14C
  short numobj;   //число входов в таблицу объектов
  long date_time; //дата модификации линкером
  long pCOFF;
  long COFFsize;
  short NTheadsize; //размер заголовка PE от MAGIC - всегда 0xE0
  short flags;
  short Magic;   //назначение прграммы
  short LinkVer; //версия линкера
  long sizecode;
  long sizeinitdata;
  long sizeuninitdata;
  long EntryRVA; //адрес относит IMAGE BASE по которому передается управление
  long basecode; // RVA секция, которая содержит программный код
  long basedata; // RVA секция,содержащая данные
  long ImageBase; //виртуальный начальный адрес загрузки программы
  long objAlig;   //выравнивание программных секций
  long fileAlig;  //Выравнивание секций в файле
  long OSver; //номер версии опер системы необх программе
  long userver;
  long SubSysVer;
  long rez;
  long imagesize; //размер в байтах загружаемого образа с заголовками выравненый
  long headsize;  //разм всех заголовков stub+PE+objtabl
  long checksum;
  short SubSys; //операционная сист необх для запуска
  short DLLflag;
  long stackRezSize;
  long stackComSize;
  long heapRezSize;
  long heapComSize;
  long loaderFlag;
  long numRVA; //всегда 10
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
} PE_HEADER;

typedef struct _OBJECT_ENTRY_ {
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
} OBJECT_ENTRY;

typedef struct _EXPORT_TABLE_ {
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
} EXPORT_TABLE;

struct ftime {
  unsigned ft_tsec : 5;  /* две секунды */
  unsigned ft_min : 6;   /* минуты */
  unsigned ft_hour : 5;  /* часы */
  unsigned ft_day : 5;   /* день */
  unsigned ft_month : 4; /* месяц */
  unsigned ft_year : 7;  /* год-1980 */
};

typedef struct _STRING_LIST_ {
  void *next;          //следующая структура
  unsigned int len;    //длина строки
  unsigned int ofs;    //адрес в выходном файле
  unsigned char type;  //тип терминатора
  unsigned char plase; //где сейчас строка - post or data
} STRING_LIST;

typedef struct _FILEINFO_ {
  char *filename;
  int numdline;
  idrec *stlist;
  union {
    struct ftime time;
    unsigned short lineidx[2];
  };
} FILEINFO;

typedef struct _EWAR_ {
  FILE *file;
  char *name;
} EWAR;

typedef struct _ICOMP_ {
  unsigned int type;
  unsigned int loc;
  unsigned int use_cxz;
} ICOMP;

typedef struct _RETLIST_ {
  unsigned int line;
  unsigned int loc;
  unsigned int type;
  //	int use;
} RETLIST;

enum { singlcase, startmulti, endmulti };

typedef struct _ISW_ {
  unsigned char type;
  unsigned int postcase;
  unsigned long value;
} ISW;

typedef struct _postinfo_ {
  unsigned int loc;
  unsigned int num;
  unsigned short type;
  unsigned short line;
  unsigned short file;
} postinfo;

typedef struct _EXE_DOS_HEADER_ {
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
  unsigned long fullsize;
} EXE_DOS_HEADER;

typedef struct _FSWI_ {
  ISW *info;
  int sizetab; //число элементов
  int type;    //разрядность
  int numcase; //число используемых элементов
  int defal;   //значение по умолчанию.
  int ptb; //адрес указателя на эту таблицу в блоке кода
  int ptv; //адрес тавлицы величин
  int mode; //тип switch
  int razr; //разрядность величин
} FSWI;

struct paraminfo {
  unsigned int ofspar;
  unsigned char type[8];
};

struct MEOSheader {
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

typedef struct _LVIC_ {
  idrec *rec;
  //	int blocks;
  int typevar;
  int contype; //тип содержимого
  union {
    long number;
    long long lnumber;
    double dnumber;
    float fnumber;
  };
} LVIC;

typedef struct _BLVIC_ {
  int sizevic;
  LVIC *listvic;
} BLVIC;

#endif

#define SIZEIDREG 256
#define NOINREG 8
#define SKIPREG 9

typedef struct _REGEQVAR_ {
  struct _REGEQVAR_ *next;
  char name[IDLENGTH];
  unsigned char razr;
} REGEQVAR;

typedef struct _REGISTERSTAT_ {
  union {
    REGEQVAR *next;
#ifdef OPTVARCONST
    BLVIC *bakvic;
#endif
  };
  union {
    char id[SIZEIDREG];
    void *stringpar;
    unsigned long number;
  };
  unsigned char type;
  unsigned char razr;
} REGISTERSTAT;

typedef struct _SAVEREG_ {
  unsigned int size;    //размер памяти для регистров
  unsigned char all;    //все регистры
  unsigned char reg[8]; //карта регистров
} SAVEREG;

typedef struct _SAVEPAR_ {
  unsigned char ooptimizespeed;
  unsigned char owarning;
  unsigned char odbg;
  unsigned char odosstring;
  unsigned char ouseinline;
  unsigned char oam32; // режим 32 битной адресации
  unsigned char oalignword;
  unsigned char oAlignCycle; //выравнивать начала циклов
  unsigned char oidasm; //ассемблерные инструкции считать идентификаторами
  int ooptnumber;
  int odivexpand;
  unsigned char ooptstr; //оптимизация строковых констант
  unsigned char ochip;
  int oaligncycle;
  unsigned char ouselea;
  unsigned char oregoverstack;
} SAVEPAR;

typedef struct _COM_MOD_ {
  struct _COM_MOD_ *next;
  unsigned char *input;  /* dynamic input buffer */
  unsigned int endinptr; /* end index of input array */
  unsigned int inptr;    /* index in input buffer */
  unsigned int inptr2;   /* index in input buffer */
  unsigned int linenumber;
  unsigned int currentfileinfo;
  int numparamdef; //число параметров в текущем define
  char *declareparamdef; //список объявленых параметров define
  char *paramdef; //список новых параметров
  int freze; //флаг запрещения удаления структуры
} COM_MOD;

typedef struct _LISTRELOC_ {
  unsigned int val;
} LISTRELOC;

typedef struct _LISTFLOAT_ {
  union {
    float fnum;
    double dnum;
    unsigned long num[2];
  };
  int type;
  unsigned int ofs;
} LISTFLOAT;

typedef struct _LILV_ {
  unsigned int ofs;
  int size;
  localrec *rec;
} LILV;

typedef struct _WARNACT_ {
  void (*fwarn)(const char *str, unsigned int line, unsigned int file);
  unsigned char usewarn;
} WARNACT;
