#include "coff.h"
#include "tok.h"

#define _OUTPE_

/* -----------------------------------------------------------------------
 Создание PE формата
 ------------------------------------------------------------------------ */
#define SIZESTUB 96
#define STRVERS 0x20 //смещение текста с номером версии

unsigned char stub[] = {
    0x4D, 0x5A, 0x50, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0F,
    0x00, 0xFF, 0xFF, 0x00, 0x00, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x40, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00,

    0xBA, 0x0E, 0x00, 0x0E, 0x1F, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01,
    0x4C, 0xCD, 0x21, 0x46, 0x6F, 0x72, 0x20, 0x57, 0x69, 0x6E, 0x33,
    0x32, 0x20, 0x6F, 0x6E, 0x6C, 0x79, 0x21, 0x0D, 0x0A, 0x24};

#define SIZESTUB2 12
unsigned char stub2[] = {0x4D, 0x5A, 0x53, 0x50, 0x48, 0x49,
                         0x4E, 0x58, 0x20, 0x43, 0x2D, 0x2D};
unsigned int numdll, numapi;
unsigned char FixUpTable =
    FALSE; //запретить создание таблици Fix UP for Windows
unsigned char WinMonoBlock = TRUE;
int numexport = 0;
unsigned long ImageBase = 0x400000;
unsigned long vsizeheader = 0x1000; //виртуальный размер заголовка.
unsigned long FILEALIGN = 0; // 512;	// выравнивание секций в файле
int filingzerope;

listexport *lexport = NULL;
static unsigned long sizestub;
static unsigned long numrs = 1;

unsigned long Align(unsigned long size, unsigned long val) {
  if (val < 2)
    return size;
  val--;
  return (size + val) & (~val);
}

/*
#include <conio.h>

void PrintMem(void *mem)
{
unsigned char *m,*n;
int i,j;
        m=(unsigned char *)mem;
        for(i=0;i<192;){
                n=m;
                for(j=0;j<16;j++,i++){
                        printf("%02X ",*m);
                        m++;
                }
                for(j=0;j<16;j++){
                        char c=*n;
                        n++;
                        if(c<0x20||c>0x7e)c='.';
                        putch(c);
                }
                puts("");
        }
}

void CheckMem()
{
        if(listdll!=NULL){	//есть api-проц
DLLLIST *newdll=listdll;	//начало списка DLL
APIPROC *listapi=newdll->list;
idrec *rec=listapi->recapi;
                PrintMem(rec);
        }
} */

void AddJmpApi() {
  //поиск api процедур
  //скорректировать адреса из таблицы перемещений
  alignersize += AlignCD(CS, 4);
  if (listdll != NULL) {       //есть api-проц
    DLLLIST *newdll = listdll; //начало списка DLL
    numdll = numapi = 0;
    for (APIPROC *listapi = newdll->list;;) {
      unsigned short numapiprocdll =
          0; //число используемых в библиотеке процедур
      for (short i = 0; i < newdll->num; i++) { //проверить все процедуры
        idrec *rec = (listapi + i)->recapi;
        unsigned int idnum = rec->recnumber; //идентификатор списка
        char useapi = FALSE; //флаг использования
        for (unsigned int j = 0; j < posts;
             j++) { //поиск использования процедуры
          if ((postbuf + j)->num == idnum) {
            if ((postbuf + j)->type == CALL_32) { //нашли
              useapi = API_JMP;                   //флаг взведем
              unsigned long hold =
                  outptr - ((postbuf + j)->loc + 4); //растояние до вызова
              *(long *)&output[(postbuf + j)->loc] = hold; //исправить
            } else if ((postbuf + j)->type == CALL_32I) {
              useapi = API_FAST; //флаг взведем
              numrel++;
            }
          }
        }
        if (useapi == API_JMP) { // процедура вызывалась
          *(short *)&output[outptr] = 0x25ff; //генерация JMP
          outptr += 2;
          rec->recnumber = outptr; //теперь вместо идент. точка входа
          AddReloc(CS); //добавить ее в табл перемещений
          *(long *)&output[outptr] = 0; //адрес вызова
          outptr += 4;
          numapi++; //общее число использованых api-процедур
          numapiprocdll++; //число использованых процедур в этой DLL
        } else if (useapi == API_FAST) {
          numapi++; //общее число использованых api-процедур
          numapiprocdll++; //число использованых процедур в этой DLL
        }
        rec->recrm = useapi; //тип вызова api; 0 - not used
        if (rec->newid != NULL) {
          free(rec->newid); //список параметров больше не нужен
          rec->newid = NULL;
        }
      }
      newdll->num = numapiprocdll; //исправить число реально использ процедур
      if (numapiprocdll == 0) { //в этой библиотеке не использ ни один вызов
        free(newdll->list); //удалить список процедур.
      } else
        numdll++;
      if (newdll->next == NULL)
        break; //конец списка
      newdll = newdll->next;
      listapi = newdll->list;
    }
  }
  outptrdata = outptr;
}

int MakePE() {
  unsigned short numobj = 1;
  char *importblock = NULL;
  char *relocblock = NULL;
  char *exportblock = NULL;
  unsigned long psize, vsize = 0, sizereloc = 0, sizeReloc = 0, sizeImport = 0,
                       sizeExport = 0, sizebss = 0, sizeRes = 0;
  unsigned long startsec = 0, startsecr = 0, startsece = 0, startsecres = 0;
  unsigned int posrel = 0, sizeimport = 0, startimportname = 0, sizeexport = 0,
               sizeres = 0, startexportname = 0;
  unsigned int sizehead;      //размер заголовка
  unsigned int exportnum = 0; //номер секции экспорта
  unsigned int relocnum = 0;  //номер секции перемещений
  unsigned int importnum = 0; //номер секции импорта
  unsigned int codenum = 0;   //номер секции кода
  unsigned int resnum = 0;    //номер секции ресурсов
  if (hout == NULL)
    return -1;
  if (WinMonoBlock == FALSE) {
    vsize = Align(outptr + (wbss == FALSE ? postsize : 0),
                  OBJECTALIGN); //виртуальный размер секции кода
    psize = Align(outptr, FILEALIGN); //физический размер секции кода
  } else
    vsize = outptr;
  //	sizehead=((postsize&&wbss)?2:1)*sizeof(OBJECT_ENTRY);
  sizehead = numrs * sizeof(OBJECT_ENTRY);
  OBJECT_ENTRY *objentry = (OBJECT_ENTRY *)MALLOC(sizehead); //тавлица объектов
  memset(objentry, 0, sizehead); //очистить таблицу объектов
                                 //секция .bss
  if (wbss) {    //есть post переменные
    numobj++;    //увеличиваем число объектов
    codenum = 1; //номер секции кода
    strcpy(objentry->name, ".bss"); //имя секции
    objentry->vsize = sizebss = Align(postsize, OBJECTALIGN);
    objentry->pOffset = objentry->psize = 0;
    objentry->flags = 0xC0000080;
    objentry->sectionRVA = vsizeheader;
  }
  strcpy((objentry + codenum)->name, "CODE"); //ее имя
  (objentry + codenum)->vsize = vsize; //размер секции в памяти
  (objentry + codenum)->psize = psize; //размер секции в файле
  (objentry + codenum)->flags = 0xe0000060; //флаг секции
  (objentry + codenum)->sectionRVA =
      vsizeheader + sizebss; //виртуальный адрес секции в памяти
                             //секция импорта
  if (numapi != 0) { //есть вызовы api-процедур созд секцию импорта
    if (!WinMonoBlock) { //если не единый блок
      importnum = numobj;
      numobj++; //увеличиваем число объектов
    }
    startsec = vsizeheader + vsize + sizebss; //начало секции в памяти
                                              //реальный размер секции
    startimportname =
        (numdll + 1) * 20 + (numapi + numdll) * (shortimport == 0 ? 8 : 4);
    sizeimport = Align(startimportname, FILEALIGN); //размер секции в памяти
    importblock = (char *)MALLOC(sizeimport); //память под нее
    memset(importblock, 0, sizeimport);       //очистить ее
    DLLLIST *newdll = listdll; //табличка dll с импортируемыми процедурами
    unsigned long sn, sn1;
    sn1 = sn = (numdll + 1) * 20;
    for (int i = 0;; i++) {
      while (newdll->num == 0)
        if ((newdll = newdll->next) == NULL)
          break; //пропуск неиспользуемых
      if (newdll == NULL)
        break; //завершить цикл если список dll пуст
      APIPROC *listapi = newdll->list; //табличка процедур из текущей dll
      *(long *)&importblock[i * 20 + 12] = startsec + startimportname;
      *(long *)&importblock[i * 20] = (shortimport == 0 ? startsec + sn : 0);
      *(long *)&importblock[i * 20 + 16] =
          startsec + sn + (shortimport == 0 ? (numdll + numapi) * 4 : 0);
      sn += (newdll->num + 1) * 4;
      unsigned int lenn = strlen(newdll->name) + 1;
      if ((lenn + startimportname + 1) >= sizeimport) {
        sizeimport += FILEALIGN; //увеличить размер секции
        importblock = (char *)REALLOC(importblock, sizeimport);
        memset(importblock + sizeimport - FILEALIGN, 0, FILEALIGN);
      }
      strcpy(&importblock[startimportname], newdll->name);
      startimportname += lenn;
      for (int n = 0, t = 0; n < newdll->num; n++, t++) {
        while ((listapi + t)->recapi->recrm == 0)
          t++;
        idrec *rec = (listapi + t)->recapi;
        unsigned long newadr;
        newadr = ImageBase + startsec + sn1 +
                 (shortimport == 0 ? (numdll + numapi) * 4 : 0);
        if (rec->recrm == API_JMP)
          *(long *)&output[rec->recnumber] = newadr;
        else {
          for (unsigned int j = 0; j < posts;
               j++) { //поиск использования процедуры
            if ((postbuf + j)->num == (unsigned long)rec->recnumber &&
                (postbuf + j)->type == CALL_32I) {
              *(long *)&output[(postbuf + j)->loc] = newadr; //исправить
            }
          }
        }
        if (useordinal && rec->recsib != -1) {
          *(long *)&importblock[sn1] = rec->recsib | 0x80000000;
          if (shortimport == 0)
            *(long *)&importblock[sn1 + (numdll + numapi) * 4] =
                rec->recsib | 0x80000000;
        } else {
          if ((startimportname % 2) == 1)
            importblock[startimportname++] = 0;
          *(long *)&importblock[sn1] = startsec + startimportname;
          if (shortimport == 0)
            *(long *)&importblock[sn1 + (numdll + numapi) * 4] =
                startsec + startimportname;
          if (rec->recsize != -1)
            sprintf((char *)string2, "%s@%u", rec->recid, rec->recsize);
          else
            strcpy((char *)string2, rec->recid);
          lenn = strlen((char *)string2) + 1;
          if ((lenn + startimportname + 4) >= sizeimport) {
            sizeimport += FILEALIGN;
            importblock = (char *)REALLOC(importblock, sizeimport);
            memset(importblock + sizeimport - FILEALIGN, 0, FILEALIGN);
          }
          *(short *)&importblock[startimportname] = 0;
          startimportname += 2;
          strcpy(&importblock[startimportname], (char *)string2);
          startimportname += lenn;
        }
        sn1 += 4;
      }
      if (newdll->next == NULL)
        break;
      newdll = newdll->next;
      sn1 += 4;
    }
    importblock[startimportname++] = 0;

    if (!WinMonoBlock) { //если не единый блок
      strcpy((objentry + importnum)->name, ".idata"); //имя секции
      (objentry + importnum)->vsize = sizeImport =
          Align(sizeimport, OBJECTALIGN);
      (objentry + importnum)->psize = sizeimport;
      (objentry + importnum)->flags = 0xC0000040;
      (objentry + importnum)->sectionRVA = startsec;
    } else
      sizeImport = sizeimport = Align(startimportname, 4);
  }
  //секция экспорта
  if (numexport != 0) {
    if (!WinMonoBlock) { //если не единый блок
      exportnum = numobj;
      numobj++; //увеличиваем число объектов
    }
    startsece =
        vsizeheader + vsize + sizeImport + sizebss; //начало секции в памяти
    startexportname =
        sizeof(EXPORT_TABLE) + numexport * 10; //реальный размер секции
    sizeexport = Align(startexportname, FILEALIGN); //размер секции в памяти
    exportblock = (char *)MALLOC(sizeexport); //память под нее
    memset(exportblock, 0, sizeexport);       //очистить ее
    *(long *)&exportblock[12] = startsece + startexportname; //адрес имени файла
    *(long *)&exportblock[16] = 1;                           // Ordinal Base
    *(long *)&exportblock[20] = numexport;                   // Num of Functions
    *(long *)&exportblock[24] = numexport; // Num of Name Pointer
    *(long *)&exportblock[28] =
        startsece + sizeof(EXPORT_TABLE); // Address Table RVA
    *(long *)&exportblock[32] =
        startsece + sizeof(EXPORT_TABLE) + numexport * 4; // Name Pointers RVA
    *(long *)&exportblock[36] =
        startsece + sizeof(EXPORT_TABLE) + numexport * 8; // Ordinal Table RVA
    char *oname;
    strcpy((char *)string2, (char *)rawfilename);
    oname = strrchr((char *)string2, '\\');
    if (oname == NULL)
      oname = (char *)string2;
    else
      oname = oname + 1;
    sprintf((char *)string, "%s.%s", oname, outext);
    unsigned int lenn = strlen((char *)string) + 1;
    if ((lenn + startexportname + 1) >= sizeexport) {
      sizeexport += FILEALIGN; //увеличить размер секции
      exportblock = (char *)REALLOC(exportblock, sizeexport);
      memset(exportblock + sizeexport - FILEALIGN, 0, FILEALIGN);
    }
    strcpy(&exportblock[startexportname], (char *)string);
    startexportname += lenn;
    for (int i = 0; i < numexport; i++) {
      *(long *)&exportblock[sizeof(EXPORT_TABLE) + i * 4] =
          (lexport + i)->address + vsizeheader + sizebss; //адреса функций
      *(long *)&exportblock[sizeof(EXPORT_TABLE) + (numexport + i) * 4] =
          startsece + startexportname; //адреса имен
      *(short *)&exportblock[sizeof(EXPORT_TABLE) + numexport * 8 + i * 2] =
          (short)i; //ординалы имен
      lenn = strlen((lexport + i)->name) + 1;
      if ((lenn + startexportname + 1) >= sizeexport) {
        sizeexport += FILEALIGN; //увеличить размер секции
        exportblock = (char *)REALLOC(exportblock, sizeexport);
        memset(exportblock + sizeexport - FILEALIGN, 0, FILEALIGN);
      }
      strcpy(&exportblock[startexportname], (lexport + i)->name);
      startexportname += lenn;
    }
    free(lexport);       //освободим уже не нужный блок
    if (!WinMonoBlock) { //если не единый блок
      strcpy((objentry + exportnum)->name, ".edata"); //имя секции
      (objentry + exportnum)->vsize = sizeExport =
          Align(sizeexport, OBJECTALIGN);
      (objentry + exportnum)->psize = sizeexport;
      (objentry + exportnum)->flags = 0x40000040;
      (objentry + exportnum)->sectionRVA = startsece;
    } else
      sizeexport = sizeExport = Align(startexportname, 4);
  }

  if (numres) {                  //секция ресурсов
    if (WinMonoBlock == FALSE) { //если не единый блок
      resnum = numobj;
      numobj++; //увеличить число объектов
    }
    startsecres = vsizeheader + vsize + sizeImport + sizebss +
                  sizeExport; //начало секции в памяти
    LISTRELOC *resrel;
    if (MakeRes(startsecres, &resrel))
      free(resrel);
    if (!WinMonoBlock) { //если не единый блок
      strcpy((objentry + resnum)->name, ".rsrc"); //имя секции
      (objentry + resnum)->vsize = sizeRes = Align(curposbuf, OBJECTALIGN);
      (objentry + resnum)->psize = sizeres = Align(curposbuf, FILEALIGN);
      (objentry + resnum)->flags = 0x40000040;
      (objentry + resnum)->sectionRVA = startsecres;
    } else
      sizeres = Align(curposbuf, 4);
  }

  //секция таблиц перемещения
  if ((FixUpTable == TRUE &&
       numrel != 0) /*||numexport!=0*/) { //создать секцию перемещения
    if (WinMonoBlock == FALSE ||
        dllflag == TRUE) { //если не единый блок и это DLL
      relocnum = numobj;
      numobj++; //увеличить число объектов
    }
    if (WinMonoBlock && dllflag)
      startsecr = vsizeheader +
                  Align(sizeimport + sizeexport + outptr +
                            (wbss == FALSE ? postsize : 0) + sizebss + sizeres,
                        OBJECTALIGN);
    else
      startsecr = vsizeheader + vsize + sizeImport + sizeExport + sizebss +
                  sizeres; //виртуальный адрес секции в памяти
    //физический размер секции таблицы перемещений
    sizereloc = Align(numrel * 2 + (outptr / 4096 + 1) * 10, FILEALIGN);
    sizeReloc = Align(sizereloc, OBJECTALIGN); //виртуальный размер этой секции
    relocblock = (char *)MALLOC(sizereloc); //память под эту секцию
    memset(relocblock, 0, sizereloc);       //очистить ее
    //заполняем секцию перемещения
    unsigned int startrsec = 0; //адрес начала блока в секции перемещения
    unsigned int startblc = 0; //адрес первого блока
    posrel = 8;
    do {
      unsigned char fr = FALSE;                  //флаг элемента
      for (unsigned int i = 0; i < posts; i++) { //обходим всю таблицу post
        if (((postbuf + i)->type == CALL_32I ||
             ((postbuf + i)->type >= POST_VAR32 &&
              (postbuf + i)->type <= FIX_CODE32)) &&
            (postbuf + i)->loc >= startblc &&
            (postbuf + i)->loc < (startblc + 4096)) {
          *(short *)&relocblock[posrel] =
              (short)((postbuf + i)->loc % 4096 | 0x3000);
          posrel += 2;
          fr = TRUE;
        }
      }
      if (fr != FALSE) { //если были перемещаемые адреса
        posrel += posrel % 4; //выравниваем
        *(long *)&relocblock[startrsec] = vsizeheader + sizebss + startblc;
        *(long *)&relocblock[startrsec + 4] = posrel - startrsec; //размер куска
        startrsec = posrel;
        posrel += 8;
      }
      startblc += 4096;
    } while (startblc < vsize);
    posrel -= 8;
    if (WinMonoBlock == FALSE || dllflag == TRUE) { //если не единый блок
      strcpy((objentry + relocnum)->name, ".reloc"); //имя секции
      (objentry + relocnum)->vsize = sizeReloc; //размер секции в памяти
      (objentry + relocnum)->psize = sizereloc; //размер секции в файле
      (objentry + relocnum)->flags = 0x52000040; //флаг секции
      (objentry + relocnum)->sectionRVA =
          startsecr; //виртуальный адрес секции в памяти
    } else
      sizereloc = Align(posrel, 4);
  }
  if (WinMonoBlock) {
    psize = sizeimport + sizeexport + (dllflag == FALSE ? sizereloc : 0) +
            sizeres; //размер дополнительных данных
    if (wbss == 0) {
      for (unsigned int i = 0; i < posts; i++) {
        if ((postbuf + i)->type == POST_VAR32)
          *(long *)&output[(postbuf + i)->loc] += psize;
      }
    }
    psize += outptr;
    (objentry + codenum)->vsize = vsize =
        Align(psize + (wbss == FALSE ? postsize : 0),
              OBJECTALIGN); //виртуальный размер секции кода
    filingzerope = (objentry + codenum)->psize =
        Align(psize, FILEALIGN); //физический размер секции кода
    filingzerope -= psize;
    psize = (objentry + codenum)->psize;
    sizeImport = sizeExport = 0;
    if (dllflag == FALSE)
      sizeReloc = 0;
  }
  PE_HEADER *peheader = (PE_HEADER *)MALLOC(sizeof(PE_HEADER));
  memset(peheader, 0, sizeof(PE_HEADER));
  sizehead = Align(sizeof(PE_HEADER) + sizestub +
                       (numobj + (numres != 0 ? 0 : 1)) * sizeof(OBJECT_ENTRY),
                   FILEALIGN);
  peheader->sign[0] = 0x50; // 'P'
  peheader->sign[1] = 0x45; // 'E'
  peheader->cpu = 0x14c;    //(chip>=4?(chip>=5?0x14e:0x14D):0x14c);
                            //	peheader->date_time=0;
  peheader->DLLflag = dllflag;
  peheader->numobj = numobj;
  peheader->NTheadsize = 0xe0;
  peheader->flags = (short)(0x818e | (dllflag == 0 ? 0 : 0x2000));
  peheader->Magic = 0x10b;
  peheader->LinkVer = (short)((short)ver2 * 256 + ver1);
  peheader->sizecode = psize;
  peheader->sizeuninitdata = postsize;
  {
    unsigned int temp;
    temp = EntryPoint();
    if (temp == 0xffffffff)
      peheader->EntryRVA = 0;
    else
      peheader->EntryRVA = vsizeheader + sizebss + temp;
  }
  peheader->basecode = vsizeheader + sizebss;
  peheader->objAlig = OBJECTALIGN;
  peheader->fileAlig = FILEALIGN;
  peheader->OSver = 1;
  peheader->SubSysVer = 4;
  peheader->ImageBase = ImageBase;
  peheader->headsize = sizehead;
  peheader->imagesize = vsizeheader + //размер заголовка
                        vsize +       //размер кода
                        sizebss +     //размер post блока
                        sizeReloc + //размер таблицы перемещения
                        sizeImport + //размер таблицы импорта
                        sizeRes + //размер таблицы ресурсов
                        sizeExport; //размер таблицы экспорта
  peheader->SubSys = (short)(2 + wconsole); // GUIWIN
  peheader->stackRezSize = stacksize * 0x10;
  peheader->stackComSize = stacksize;
  peheader->heapRezSize = 0x10000;
  peheader->heapComSize = 0x1000; // postsize;	//????
  peheader->numRVA = 0x10;
  if (!usestub) {
    peheader->basedata = 12;
    peheader->pCOFF = 0x40;
  }
  (objentry + codenum)->pOffset = sizehead;
  if (numapi) {
    if (!WinMonoBlock)
      (objentry + importnum)->pOffset = sizehead + psize;
    peheader->importRVA = startsec;
    peheader->importSize = startimportname;
  }
  if (numexport) {
    if (!WinMonoBlock)
      (objentry + exportnum)->pOffset = sizehead + psize + sizeimport;
    peheader->exportRVA = startsece;
    peheader->exportSize = startexportname;
  }
  if (numres) {
    if (!WinMonoBlock)
      (objentry + resnum)->pOffset = sizehead + psize + sizeimport + sizeexport;
    peheader->resourRVA = startsecres;
    peheader->resourSize = curposbuf;
  }
  if (posrel) {
    if (!WinMonoBlock)
      (objentry + relocnum)->pOffset =
          sizehead + psize + sizeimport + sizeexport + sizeres;
    else if (dllflag)
      (objentry + relocnum)->pOffset = sizehead + psize;
    peheader->fixupRVA = startsecr;
    peheader->fixupSize = posrel;
  }
  if (fwrite(peheader, sizeof(PE_HEADER), 1, hout) != 1) {
  errwrite:
    ErrWrite();
    fclose(hout);
    hout = NULL;
    return (-1);
  }
  if (fwrite(objentry, sizeof(OBJECT_ENTRY) * numobj, 1, hout) != 1)
    goto errwrite;
  ChSize(sizehead);
  runfilesize = sizehead + psize;
  outputcodestart = ftell(hout);
  if (fwrite(output, outptr, 1, hout) != 1)
    goto errwrite; //блок кода
  if (!WinMonoBlock) {
    filingzerope = psize - outptr;
    ChSize(runfilesize);
  }
  if (numapi) {
    if (fwrite(importblock, sizeimport, 1, hout) != 1)
      goto errwrite;
    free(importblock);
  }
  if (numexport) {
    if (fwrite(exportblock, sizeexport, 1, hout) != 1)
      goto errwrite;
    free(exportblock);
  }
  if (numres) {
    if (fwrite(resbuf, sizeres, 1, hout) != 1)
      goto errwrite;
    free(resbuf);
  }
  if (posrel) {
    if (WinMonoBlock && dllflag)
      ChSize(runfilesize);
    if (fwrite(relocblock, sizereloc, 1, hout) != 1)
      goto errwrite;
    free(relocblock);
  }
  if (WinMonoBlock) {
    if (dllflag)
      runfilesize += sizereloc;
  } else
    runfilesize += sizereloc + sizeimport + sizeexport + sizeres;
  ChSize(runfilesize);
  free(peheader);
  free(objentry);
  fclose(hout);
  hout = NULL;
  ImageBase += vsizeheader + sizebss; //изм размер для листинга
  return 0;
}

void ChSize(long size) {
  char buf[256];
  long delta, ssave;
  memset(buf, 0, 256);
  delta = size - ftell(hout);
  while (delta > 0) {
    ssave = 256;
    if (delta < 256)
      ssave = delta;
    fwrite(buf, ssave, 1, hout);
    delta -= ssave;
  }
}

int AlignCD(char segm, int val) //выравнять данные или код
{
  unsigned int a;
  a = (segm == DS ? outptrdata : outptr) % val;
  if (a == 0)
    val = 0;
  else
    val -= a;
  a = 0;
  while (val != 0) {
    segm == DS ? opd(aligner) : op(0x90);
    val--;
    a++;
  }
  return a;
}

void CreatWinStub() {
  if (!usestub) {
    sizestub = SIZESTUB2;
    hout = CreateOutPut(outext, "wb");
    if (fwrite(stub2, SIZESTUB2, 1, hout) != 1) {
      ErrWrite();
      return;
    }
  } else
    CreatStub(winstub);
  //подсчитать число секций
  if (wbss) {
    if (postsize)
      numrs++;
    else
      wbss = FALSE;
  }
  if (WinMonoBlock == FALSE) { //если не единый блок
    if (numapi)
      numrs++; //есть вызовы api-процедур
    if (numexport)
      numrs++; //создать секцию импорта
    if ((FixUpTable == TRUE && posts) /*||numexport!=0*/)
      numrs++; //создать секцию перемещения
    if (numres)
      numrs++; //ресурсы
  } else if (dllflag && FixUpTable == TRUE && posts != 0)
    numrs++; //создать секцию перемещения
             //размер загрузочного образа
  vsizeheader = Align(
      numrs * sizeof(OBJECT_ENTRY) + sizeof(PE_HEADER) + sizestub, 0x1000);
}

void ImportName(char *name) {
  FILE *infile;
  union {
    PE_HEADER pe;
    OBJECT_ENTRY obj;
  };
  unsigned long temp;
  unsigned long exportdir; //секция с експортом
  unsigned long numobj;    //число объектов
  unsigned long posdll;    //позиция секции в файле
  unsigned long nameadr;   //таблица адресов имен
  unsigned long startname; //начало имени
  unsigned long ordinallist, ordinalbase;
  unsigned int i, j;
  DLLLIST *newdll;
  APIPROC *listapi;
  if ((infile = fopen(name, "rb")) == NULL) {
    ErrOpenFile(name);
    return;
  }
  fseek(infile, 0x3C, SEEK_SET);
  if (fread(&temp, 4, 1, infile) != 1) {
  errread:
    fprintf(stderr, "Unable to read from file %s.\n", name);
    fclose(infile);
    return;
  }
  fseek(infile, 0, SEEK_END);
  if ((unsigned long)ftell(infile) <= temp) {
    fprintf(stderr, "Bad file %s.\n", name);
    fclose(infile);
    return;
  }
  fseek(infile, temp, SEEK_SET);
  if (fread(&pe, sizeof(PE_HEADER), 1, infile) != 1)
    goto errread;
  if ((pe.sign[0] != 0x50) && (pe.sign[1] != 0x45)) {
    fprintf(stderr, "For DLL support only format PE.\n");
    fclose(infile);
    return;
  }
  if ((exportdir = pe.exportRVA) == 0) {
    fprintf(stderr, "No export directory on %s.\n", name);
    fclose(infile);
    return;
  }
  numobj = pe.numobj;
  temp = pe.objAlig;
  while (numobj != 0) {
    if (fread(&obj, sizeof(OBJECT_ENTRY), 1, infile) != 1)
      goto errread;
    if ((obj.sectionRVA + Align(obj.psize, temp)) > exportdir)
      break;
    numobj--;
  }
  if (numobj == 0) {
    fprintf(stderr, "Bad object table in %s.\n", name);
    fclose(infile);
    return;
  }
  posdll = obj.pOffset + exportdir - obj.sectionRVA;
  fseek(infile, posdll + 24, SEEK_SET);
  if (fread(&numobj, 4, 1, infile) != 1)
    goto errread;
  fseek(infile, posdll + 32, SEEK_SET);
  if (fread(&nameadr, 4, 1, infile) != 1)
    goto errread;
  if (fread(&ordinallist, 4, 1, infile) != 1)
    goto errread;
  nameadr -= exportdir;
  ordinallist -= exportdir;
  fseek(infile, posdll + 12, SEEK_SET);
  if (fread(&startname, 4, 1, infile) != 1)
    goto errread;
  if (fread(&ordinalbase, 4, 1, infile) != 1)
    goto errread;
  fseek(infile, posdll + startname - exportdir, SEEK_SET);
  j = 0;
  do {
    if (fread(&string[j], 1, 1, infile) != 1)
      goto errread;
  } while (string[j++] != 0); //имя библиотеки
  newdll = FindDLL();
  listapi = newdll->list;

  for (i = 0; i < numobj; i++) {
    fseek(infile, posdll + nameadr, SEEK_SET);
    if (fread(&startname, 4, 1, infile) != 1)
      goto errread;
    fseek(infile, posdll + startname - exportdir, SEEK_SET);
    itok.size = -1;
    j = 0;
    unsigned char c;
    do {
      if (fread(&c, 1, 1, infile) != 1)
        goto errread;
      if (c == '@') {
        itok.name[j] = 0;
        break;
      }
      itok.name[j] = c;
      j++;
    } while (j <= IDLENGTH && c != 0);
    if (c == '@') {
      j = 0;
      do {
        if (fread(&c, 1, 1, infile) != 1)
          goto errread;
        string[j++] = c;
      } while (isdigit(c));
      itok.size = getnumber(string);
    }
    tok = tk_id;
    searchvar(itok.name);
    if (tok == tk_id) {
      fseek(infile, posdll + ordinallist, SEEK_SET);
      itok.sib = 0;
      if (fread(&itok.sib, 2, 1, infile) != 1)
        goto errread;
      itok.sib += ordinalbase;
      tok = tk_apiproc;
      itok.number = secondcallnum++;
      itok.segm = NOT_DYNAMIC;
      string[0] = 0;
      if (newdll->num == 0)
        listapi = (APIPROC *)MALLOC(sizeof(APIPROC)); //первая в списке
      else
        listapi =
            (APIPROC *)REALLOC(listapi, sizeof(APIPROC) * (newdll->num + 1));
      (listapi + newdll->num)->recapi = addtotree(itok.name);
      newdll->num++;
    }
    nameadr += 4;
    ordinallist += 2;
  }
  newdll->list = listapi;
  fclose(infile);
}

void CreatStub(char *name) {
  sizestub = SIZESTUB;
  hout = CreateOutPut(outext, "wb");
  sprintf((char *)&stub[STRVERS], "%s%s", compilerstr, __DATE__);
  if (name == NULL) {
  stdstub:
    if (fwrite(stub, SIZESTUB, 1, hout) != 1) {
    errwrite:
      ErrWrite();
      return;
    }
  } else {
    EXE_DOS_HEADER exeheader; // header for EXE format
    FILE *stubin;
    if ((stubin = fopen(name, "rb")) == NULL) {
      ErrOpenFile(name);
      goto stdstub;
    }
    if (fread(&exeheader, sizeof(EXE_DOS_HEADER), 1, stubin) != 1) {
    errread:
      ErrReadStub();
      fclose(stubin);
      goto stdstub;
    }
    if (exeheader.sign != 0x5A4D) {
    errstub:
      fprintf(stderr, "File %s can not be stub file.\n", name);
      fclose(stubin);
      goto stdstub;
    }
    fseek(stubin, 0, SEEK_END);
    sizestub = ftell(stubin);
    unsigned long temp;
    if (exeheader.ofsreloc >= 0x40) { //проверка что это не 32-битный файл
      fseek(stubin, 0x3c, SEEK_SET);
      if (fread(&temp, 4, 1, stubin) != 1)
        goto errread;
      if (temp < sizestub) {
        fseek(stubin, temp, SEEK_SET);
        if (fread(&temp, 4, 1, stubin) != 1)
          goto errread;
        unsigned char *tmpsign = (unsigned char *)&temp;
        if ((((tmpsign[0] == 0x50) || (tmpsign[0] == 0x4e) ||
              (tmpsign[0] == 0x4c)) &&
             (tmpsign[1] == 0x45))                            /* PE, NE, LE */
            || ((tmpsign[0] == 0x4c) && (tmpsign[1] = 0x58))) /* LX */
          goto errstub;
      }
      exeheader.ofsreloc += (unsigned short)0x20;
    } else {
      exeheader.ofsreloc = 0x40;
    }
    //размер файла
    sizestub = Align(sizestub + 32, 8);
    fseek(stubin, 0x20, SEEK_SET);
    exeheader.headsize += (unsigned short)2;
    if (fwrite(&exeheader, sizeof(EXE_DOS_HEADER), 1, hout) != 1)
      goto errwrite;
    *(unsigned long *)&stub[STRVERS + 28] = sizestub;
    if (fwrite(&stub[STRVERS], 32, 1, hout) != 1)
      goto errwrite;
    CopyFile(stubin, hout);
    ChSize(sizestub);
  }
}

#define MAXLISTNAME 1024
#define MAXNUMSYMBOL 128
#define MAXSIZESYMBOL MAXNUMSYMBOL * sizeof(IMAGE_SYMBOL)
#define MAXNUMRELOC 256

char *ListName;
unsigned long sizelistName, maxsizelistname, textnum, bssnum;
IMAGE_SYMBOL *isymbol;
unsigned long numsymbol, maxnumsymbol, maxnumnameid, maxnumreloc, numreloc;
int segtext, segbss;
extern int numextern;
IMAGE_RELOCATION *treloc;

struct NAMEID {
  int num;
  int id;
} * NameId;

#define MAXSIZENAMEID MAXNUMSYMBOL * sizeof(NAMEID)

void AddName(char *name, unsigned long size) {
  if (sizelistName + size + 1 >= maxsizelistname) {
    maxsizelistname += MAXLISTNAME;
    ListName = (char *)REALLOC(ListName, maxsizelistname);
  }
  strcpy(ListName + sizelistName, name);
  sizelistName += size + 1;
}

void CreatSymbolTable(idrec *ptr) {
  unsigned int i;
  if (ptr != NULL) {
    CreatSymbolTable(ptr->right);
    if (ptr->rectok == tk_apiproc) {
      for (unsigned int j = 0; j < posts; j++) { //поиск использования процедуры
        if ((postbuf + j)->num == (unsigned long)ptr->recnumber &&
            ((postbuf + j)->type == CALL_32I ||
             (postbuf + j)->type == CALL_32)) {
          goto nameext;
        }
      }
    } else if ((externnum != 0 && ptr->rectok == tk_undefproc &&
                (ptr->flag & f_extern) != 0) ||
               ((ptr->rectok == tk_proc || ptr->rectok == tk_interruptproc) &&
                ptr->recsegm >= NOT_DYNAMIC) ||
               ((ptr->rectok >= tk_bits && ptr->rectok <= tk_doublevar) ||
                ptr->rectok == tk_structvar)) {
    nameext:
      if (numsymbol + 1 >= maxnumsymbol) {
        maxnumsymbol += MAXNUMSYMBOL;
        isymbol = (IMAGE_SYMBOL *)REALLOC(isymbol,
                                          maxnumsymbol * sizeof(IMAGE_SYMBOL));
        memset(isymbol + maxnumsymbol * sizeof(IMAGE_SYMBOL) - MAXSIZESYMBOL, 0,
               MAXSIZESYMBOL); //очистить ее
      }
      if (ptr->rectok == tk_apiproc || ptr->rectok == tk_undefproc ||
          ptr->rectok == tk_proc || ptr->rectok == tk_interruptproc)
        (isymbol + numsymbol)->Type = 32;
      i = strlen(ptr->recid);
      if (i > 8) {
        (isymbol + numsymbol)->N.Name.Short = 0;
        (isymbol + numsymbol)->N.Name.Long = sizelistName + 4;
        AddName(ptr->recid, i);
      } else
        strncpy((isymbol + numsymbol)->N.sname, ptr->recid, i);
      if (ptr->rectok != tk_apiproc && ptr->rectok != tk_undefproc &&
          (ptr->flag & f_extern) == 0) {
        (isymbol + numsymbol)->Value = ptr->recnumber;
        if (ptr->rectok == tk_proc || ptr->rectok == tk_interruptproc)
          (isymbol + numsymbol)->SectionNumber = (short)(textnum + 1);
        else
          (isymbol + numsymbol)->SectionNumber =
              (short)((ptr->recpost == 0 ? textnum : bssnum) + 1);
      } else {
        (NameId + numextern)->num = ptr->recnumber;
        (NameId + numextern)->id = numsymbol;
        numextern++;
        if (numextern >= maxnumnameid) {
          maxnumnameid += MAXNUMSYMBOL;
          NameId = (NAMEID *)REALLOC(NameId, maxnumnameid * sizeof(NAMEID));
        }
      }
      (isymbol + numsymbol)->StorageClass = 2;
      numsymbol++;
    }
    CreatSymbolTable(ptr->left);
  }
}

void FreeCoffBuf() {
  free(ListName);
  free(isymbol);
  free(NameId);
  free(treloc);
  fclose(hout);
  hout = NULL;
}

void IncReloc() {
  numreloc++;
  if (numreloc >= maxnumreloc) {
    maxnumreloc += MAXNUMRELOC;
    treloc = (IMAGE_RELOCATION *)REALLOC(treloc, maxnumreloc *
                                                     sizeof(IMAGE_RELOCATION));
  }
}

void CreatRelocTable() {
  for (int j = 0; j < posts; j++) {
    (treloc + numreloc)->VirtualAddress = (postbuf + j)->loc - startptr;
    if ((postbuf + j)->type >= CALL_32I && (postbuf + j)->type <= FIX_CODE32) {
      switch ((postbuf + j)->type) {
      case POST_VAR:
      case POST_VAR32:
        (treloc + numreloc)->Type = IMAGE_REL_I386_DIR32;
        (treloc + numreloc)->SymbolTableIndex = segbss;
        break;
      case FIX_VAR:
      case FIX_VAR32:
      case FIX_CODE:
      case FIX_CODE32:
        (treloc + numreloc)->Type = IMAGE_REL_I386_DIR32;
        (treloc + numreloc)->SymbolTableIndex = segtext;
        break;
      }
      IncReloc();
    } else {
      for (int i = 0; i < numextern; i++) {
        if ((NameId + i)->num == (postbuf + j)->num) {
          if ((postbuf + j)->type == EXT_VAR)
            (treloc + numreloc)->Type = IMAGE_REL_I386_DIR32;
          else
            (treloc + numreloc)->Type = IMAGE_REL_I386_REL32;
          (treloc + numreloc)->SymbolTableIndex = (NameId + i)->id;
          IncReloc();
          break;
        }
      }
    }
  }
}

int MakeCoff() {
  COFF_HEADER chead;
  unsigned long sizehead, curobj, resnum, numresrel, segres, lastoffset,
      headernum;
  OBJECT_ENTRY *objentry;
  int i;
  LISTRELOC *resrel = NULL;
  char *codesecname;
  hout = CreateOutPut("obj", "wb");
  chead.cpu = 0x14c;
  chead.SizeOfOptionalHeader = 0;
  chead.date_time = 0;
  chead.Characteristics = 0x100;
  /*if(header)*/ numrs = 2;
  //подсчитать число секций
  if (wbss) {
    if (postsize)
      numrs++;
    else
      wbss = FALSE;
  }
  if (numres)
    numrs++; //ресурсы
  chead.numobj = numrs;
  sizehead = numrs * sizeof(OBJECT_ENTRY);
  objentry = (OBJECT_ENTRY *)MALLOC(sizehead); //тавлица объектов
  memset(objentry, 0, sizehead); //очистить таблицу объектов
  curobj = 0;
  lastoffset = sizehead + sizeof(COFF_HEADER);
  //	if(header){
  strcpy((objentry + curobj)->name, ".version");
  sprintf((char *)&stub[STRVERS], "%s%s", compilerstr, __DATE__);
  (objentry + curobj)->psize = strlen((char *)&stub[STRVERS]) + 1;
  (objentry + curobj)->pOffset = lastoffset;
  (objentry + curobj)->flags = 0x100A00;
  headernum = curobj;
  lastoffset += (objentry + curobj)->psize;
  curobj++;
  //	}
  codesecname = ".text";
  if (splitdata == FALSE)
    codesecname = ".codedat";
  strcpy((objentry + curobj)->name, codesecname);
  (objentry + curobj)->psize = outptr;
  (objentry + curobj)->pOffset = lastoffset;
  (objentry + curobj)->flags = 0xE0300060;
  lastoffset += outptr;
  textnum = curobj;
  curobj++;
  if (wbss) {
    strcpy((objentry + curobj)->name, ".bss");
    (objentry + curobj)->psize = postsize;
    (objentry + curobj)->flags = 0xC0300080;
    bssnum = curobj;
    curobj++;
  }
  if (numres) {
    strcpy((objentry + curobj)->name, ".rsrc$01");
    numresrel = (objentry + curobj)->NumberOfRelocations = MakeRes(0, &resrel);
    (objentry + curobj)->psize = curposbuf;
    (objentry + curobj)->flags = 0x40000040;
    resnum = curobj;
  }
  sizelistName = 0;
  numsymbol = 0;
  ListName = (char *)MALLOC(MAXLISTNAME);
  isymbol = (IMAGE_SYMBOL *)MALLOC(MAXSIZESYMBOL);
  memset(isymbol, 0, MAXSIZESYMBOL); //очистить ее
  maxsizelistname = MAXLISTNAME;
  maxnumnameid = maxnumsymbol = MAXNUMSYMBOL;
  NameId = (NAMEID *)MALLOC(MAXSIZENAMEID);
  treloc = (IMAGE_RELOCATION *)MALLOC(sizeof(IMAGE_RELOCATION) * MAXNUMRELOC);
  maxnumreloc = MAXNUMRELOC;
  numreloc = 0;
  strcpy(isymbol->N.sname, "@comp.id");
  isymbol->Value = 0x141F8E;
  isymbol->SectionNumber = -1;
  isymbol->StorageClass = 3;
  strcpy((isymbol + 1)->N.sname, ".file");
  (isymbol + 1)->Value = 1;
  (isymbol + 1)->SectionNumber = -2;
  (isymbol + 1)->StorageClass = 0x67;
  i = (strlen(startfileinfo->filename) - 1) / sizeof(IMAGE_SYMBOL) + 1;
  (isymbol + 1)->NumberOfAuxSymbols = i;
  strcpy((isymbol + 2)->N.sname, startfileinfo->filename);
  numsymbol = i + 2;
  segtext = numsymbol;
  strcpy((isymbol + numsymbol)->N.sname, codesecname);
  (isymbol + numsymbol)->SectionNumber = textnum + 1;
  (isymbol + numsymbol)->StorageClass = 3;
  (isymbol + numsymbol)->NumberOfAuxSymbols = 1;
  numsymbol++;
  (isymbol + numsymbol)->N.Name.Short = outptr;
  numsymbol++;
  if (wbss) {
    segbss = numsymbol;
    strcpy((isymbol + numsymbol)->N.sname, ".bss");
    (isymbol + numsymbol)->SectionNumber = bssnum + 1;
    (isymbol + numsymbol)->StorageClass = 3;
    (isymbol + numsymbol)->NumberOfAuxSymbols = 1;
    numsymbol++;
    (isymbol + numsymbol)->N.Name.Short = postsize;
    numsymbol++;
    strcpy((isymbol + numsymbol)->N.sname, "DGROUP");
    (isymbol + numsymbol)->SectionNumber = bssnum + 1;
    (isymbol + numsymbol)->StorageClass = 3;
  }
  strcpy((isymbol + numsymbol)->N.sname, "FLAT");
  (isymbol + numsymbol)->SectionNumber = -1;
  (isymbol + numsymbol)->StorageClass = 3;
  numsymbol++;
  if (numres) {
    segres = numsymbol;
    strcpy((isymbol + numsymbol)->N.sname, ".rsrc$01");
    (isymbol + numsymbol)->StorageClass = 3;
    (isymbol + numsymbol)->SectionNumber = resnum + 1;
    numsymbol++;
  }
  //	if(header){
  strcpy((isymbol + numsymbol)->N.sname, ".version");
  (isymbol + numsymbol)->SectionNumber = headernum + 1;
  (isymbol + numsymbol)->StorageClass = 3;
  numsymbol++;
  //	}
  CreatSymbolTable(treestart);
  CreatRelocTable();
  (isymbol + segtext + 1)->N.Name.Long = numreloc;
  (objentry + textnum)->NumberOfRelocations = numreloc;
  if (numreloc) {
    (objentry + textnum)->PointerToRelocations = lastoffset;
    lastoffset += sizeof(IMAGE_RELOCATION) * numreloc;
  }
  if (numres) {
    (objentry + resnum)->pOffset = lastoffset;
    lastoffset += curposbuf;
    if (numresrel) {
      (objentry + resnum)->PointerToRelocations = lastoffset;
      lastoffset += sizeof(IMAGE_RELOCATION) * numresrel;
    }
  }
  chead.COFFsize = numsymbol;
  if (numsymbol) {
    chead.pCOFF = lastoffset;
  }
  if (fwrite(&chead, sizeof(COFF_HEADER), 1, hout) != 1) {
  errwrite:
    ErrWrite();
    free(objentry);
    if (resrel)
      free(resrel);
    FreeCoffBuf();
    return (-1);
  }
  if (fwrite(objentry, sizehead, 1, hout) != 1)
    goto errwrite;
  //	if(header){
  if (fwrite(&stub[STRVERS], (objentry + headernum)->psize, 1, hout) != 1)
    goto errwrite;
  //	}
  if (fwrite(output, outptr, 1, hout) != 1)
    goto errwrite; //блок кода
  if (numreloc) {
    if (fwrite(treloc, numreloc * sizeof(IMAGE_RELOCATION), 1, hout) != 1)
      goto errwrite;
  }
  if (numres) {
    if (fwrite(resbuf, curposbuf, 1, hout) != 1)
      goto errwrite;
    free(resbuf);
    if (numresrel) {
      IMAGE_RELOCATION *rrel;
      rrel = (IMAGE_RELOCATION *)MALLOC(sizeof(IMAGE_RELOCATION) * numresrel);
      for (i = 0; i < numresrel; i++) {
        (rrel + i)->VirtualAddress = (resrel + i)->val;
        (rrel + i)->Type = IMAGE_REL_I386_DIR32NB;
        (rrel + i)->SymbolTableIndex = segres;
      }
      if (fwrite(rrel, sizeof(IMAGE_RELOCATION) * numresrel, 1, hout) != 1)
        goto errwrite;
      free(rrel);
    }
  }
  if (numsymbol) {
    if (fwrite(isymbol, numsymbol * sizeof(IMAGE_SYMBOL), 1, hout) != 1)
      goto errwrite;
    if (sizelistName) {
      sizelistName += 4;
      if (fwrite(&sizelistName, 4, 1, hout) != 1)
        goto errwrite;
      if (fwrite(ListName, sizelistName - 4, 1, hout) != 1)
        goto errwrite;
    } else {
      if (fwrite(&sizelistName, 4, 1, hout) != 1)
        goto errwrite;
      sizelistName += 4;
    }
  }
  runfilesize = lastoffset + sizelistName;
  free(objentry);
  if (resrel)
    free(resrel);
  FreeCoffBuf();
  return 0;
}
