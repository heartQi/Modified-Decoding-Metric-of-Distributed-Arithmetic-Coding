//�ļ�: sconfig.c & sconfig.h
//����: �������ļ���д������(for Windows)
//ʱ��: 2009-02-27

#include "string.h"
#include "process.h"
#include "sconfig.h"

static char newline[4] = {'\r','\n',0,0};

#define SCR_TMP_DESDIR "."

//����: sc_copy
//����: �����ļ�
//����: src  Դ�ļ�·�����ļ���
//      des  Ŀ���ļ�·�����ļ���
//����: �ɹ�����0, ʧ�ܷ���-1��
int sc_copy(const char * src, const char * des)
{
   int rest = -1;
   size_t iz;
   FILE * fs = 0;
   FILE * fd = 0;

   char gcvar[SCR_VAR_MAXLEN + 1];

   if(src == 0) return -1;
   if(des == 0) return -1;

   fs = fopen(src, "rb");
   if(fs == 0) goto fun_end;

   fd = fopen(des, "wb");
   if(fd == 0) goto fun_end;

   for(;;)
   {
      iz = fread(gcvar, 1, sizeof(gcvar), fs);
      if(iz <= 0) break;
      if(fwrite(gcvar, 1, iz, fd) != iz) goto fun_end;
   }

   rest = 0;

   fun_end:
   {
      if(fs != 0) fclose(fs);
      if(fd != 0) fclose(fd);
      return rest;
   }
}

//����: sc_read
//����: �������ļ���ȡ�ַ�������
//����: name  �����ļ�·��������
//      pkey  ����(\0��β���ִ�)
//      pdes  �������ݵ��ڴ��ַ
//      inum  �������ݵ��ڴ��С
//����: ��ȡ���ַ�����(������\0������)ֵ���ڵ�����, ʧ���򷵻ظ�ֵ��
//��ע: ������ȡ�����浽����pdesָ����ڴ��е�����Ϊ\0��β���ַ�����
//      ����pdesָ����ڴ��Ӧ�㹻��Ű���\0��β�����ڵ������ַ�����
//      ���������ݵ��ڴ��СС���ַ������ݴ�С(����\0)��������-1��
//      ������pdes��inumΪ��, ��ú���ֻ�������ݵĳ���, ����ȡ���ݡ�
//����: -1=��������ȷ, -2=�޷����ļ�, -3=��������, -4=δ֪�Ĵ���
int sc_read(const char * name, const char * pkey, void * pdes, int inum)
{
   int iv, i, rst = -4;
   unsigned char ucvar;
   unsigned char * cp;
   FILE * fp = 0;

   //������
   if(name == 0) return -1;
   if(pkey == 0) return -1;
   if(inum < 0) return -1;

   cp = (unsigned char *)pkey;
   iv = 0;

   for(i=0; i<=SCR_KEY_MAXLEN+1; i++,cp++)
   {
      if(*cp == 0)
      {
         iv = i;
         break;
      }

      if(!((*cp>='0' && *cp<='9') || *cp==' ' || *cp=='-' || *cp=='_'
         || (*cp>='a' && *cp<='z') || *cp=='/' || *cp=='\\' || *cp=='.'
         || (*cp>='A' && *cp<='Z') || *cp==':' || *cp>127)) return -1;
   }

   if(iv > SCR_KEY_MAXLEN || iv < 1) return -1;

   //�������ļ�
   fp = fopen(name, "rb");
   if(fp == 0)
   {
      rst = -2;
      goto read_exit;
   }

   //��������
   search_key:
   {
      find_keyjp:

      //���Ҽ������
      for(;;)
      {
         iv = (int)fread(&ucvar, 1, sizeof(ucvar), fp);
         if(iv != sizeof(ucvar))
         {
            rst = -3;
            goto read_exit;
         }
         if(ucvar == '<') break;
         if(ucvar!=' ' && ucvar!='\t' && ucvar!=0xD && ucvar!=0xA) goto find_lend;
      }

      //�Ƚϼ���
      for(cp=(unsigned char *)pkey ;; cp++)
      {
         iv = (int)fread(&ucvar, 1, sizeof(ucvar), fp);
         if(iv != sizeof(ucvar))
         {
            rst = -3;
            goto read_exit;
         }
         if(ucvar=='>' && *cp==0) goto search_equal;
         if(ucvar==0xD || ucvar==0xA) goto find_keyjp;
         if(*cp != ucvar) break;
      }

      find_lend:

      //�����н���
      for(;;)
      {
         iv = (int)fread(&ucvar, 1, sizeof(ucvar), fp);
         if(iv != sizeof(ucvar))
         {
            rst = -3;
            goto read_exit;
         }
         if(ucvar==0xD || ucvar==0xA) break;
      }

      goto search_key;
   }

   search_equal:

   //�����Ⱥ�
   for(;;)
   {
      iv = (int)fread(&ucvar, 1, sizeof(ucvar), fp);
      if(iv != sizeof(ucvar))
      {
         rst = -4;
         goto read_exit;
      }
      if(ucvar == '=') break;
      if(ucvar!=' ' && ucvar!='\t')
      {
         rst = -4;
         goto read_exit;
      }
   }

   //����ֵ�����
   for(;;)
   {
      iv = (int)fread(&ucvar, 1, sizeof(ucvar), fp);
      if(iv != sizeof(ucvar))
      {
         rst = -4;
         goto read_exit;
      }
      if(ucvar == '"') break;
      if(ucvar!=' ' && ucvar!='\t')
      {
         rst = -4;
         goto read_exit;
      }
   }

   //��ȡֵ�ַ���
   for(i=0,cp=(unsigned char*)pdes ;; i++,cp++)
   {
      if(inum!=0 && i>=inum)
      {
         rst = -1;
         goto read_exit;
      }
      if(i > SCR_VAR_MAXLEN)
      {
         rst = -4;
         goto read_exit;
      }
      iv = (int)fread(&ucvar, 1, sizeof(ucvar), fp);
      if(iv != sizeof(ucvar))
      {
         rst = -4;
         goto read_exit;
      }
      if(ucvar == '"')
      {
         if(pdes!=0 && inum!=0) *cp = 0;
         rst = i;
         goto read_exit;
      }
      if(ucvar < 32)
      {
         rst = -4;
         goto read_exit;
      }
      if(pdes!=0 && inum!=0) *cp = ucvar;
   }

   read_exit:
   if(fp != 0) fclose(fp);
   return rst;
}
//����: sc_read_int
//����: ��ȡint���Ͳ���
//����: ��sc_read ����
int sc_read_int(const char * name, const char * pkey, int * pdes)
{
	char value[50];
	sc_read(name, pkey, value, 50);
	*pdes=atoi(value);
	return 1;
}
//����: sc_read_int
//����: ��ȡint���Ͳ���
//����: ��sc_read ����
unsigned long sc_read_long(const char * name, const char * pkey, unsigned long * pdes)
{
	char value[50];
	sc_read(name, pkey, value, 50);
	*pdes=(unsigned long)atoi(value);
	return 1;
}
//����: sc_read_double
//����: ��ȡdouble���Ͳ���
//����: ��sc_read ����
int sc_read_double(const char * name, const char * pkey, double * pdes)
{
	char value[50];
	sc_read(name, pkey,value,50);
	*pdes=atof(value);
	return 1;
}
//����: sc_newline
//����: �����ļ����и�ʽ
//����: SCL_PC��SCL_UNIX
void sc_newline(int iv)
{
   if(iv == SCL_PC) _snprintf(newline, sizeof(newline), "\r\n");
   else _snprintf(newline, sizeof(newline), "\n");
   return;
}

//����: sc_open
//����: Ϊ��ȡ���������ļ�
//����: �����ļ�·�����ļ���
//����: �����ļ����, ʧ�ܷ���0��
FILE * sc_open(const char * name)
{
   if(name == 0) return 0;
   return fopen(name, "rb");
}

//����: sc_close
//����: �ر������ļ����
//����: �����ļ����
void sc_close(FILE * vp)
{
   if(vp != 0) fclose(vp);
   return;
}

//����: sc_reset
//����: �����������ö�ָ��λ��
//����: �����ļ����
//����: �ɹ�����0, ʧ�ܷ��ط�0��
int sc_reset(FILE * vp)
{
   if(vp == 0) return -1;
   return fseek(vp, 0, SEEK_SET);
}

//����: sc_readline
//����: �������ļ��ж�ȡһ������
//����: hcfg  �����ļ����
//      pkey  ���Ľ��յ�ַ(��СSCR_KEY_MAXLEN+1�ֽ�)
//      pvar  ֵ�Ľ��յ�ַ(��СSCR_VAR_MAXLEN+1�ֽ�)
//����: �ɹ�����1, ���������ݷ���0, ��������-1��
//��ע: ���صļ���ֵ��ΪNULL��β���ַ������ݡ�
int sc_readline(FILE * hcfg, char * pkey, char * pvar)
{
   int iv, i, irst = 0;
   unsigned char ucvar;

   if(hcfg == 0) return -1;
   if(pkey == 0) return -1;
   if(pvar == 0) return -1;

   //���Ҽ������
   for(;;)
   {
      iv = (int)fread(&ucvar, 1, sizeof(ucvar), hcfg);
      if(iv != sizeof(ucvar)) return -1;
      if(ucvar == '<') break;
      if(ucvar == 0xD || ucvar == 0xA) return 0;
      if(ucvar != ' ' && ucvar != '\t') goto find_lend;
   }

   //��ȡ���ַ���
   for(i=0 ;; i++)
   {
      if(i > SCR_KEY_MAXLEN) goto find_lend;

      iv = (int)fread(&ucvar, 1, sizeof(ucvar), hcfg);
      if(iv != sizeof(ucvar)) return -1;

      if(ucvar == '>')
      {
         if(i == 0) goto find_lend;
         else{ pkey[i] = 0; break; }
      }

      if(ucvar == 0xD || ucvar == 0xA) return 0;

      if(!((ucvar>='0' && ucvar<='9') || ucvar==' ' || ucvar=='-' || ucvar=='_'
      || (ucvar>='a' && ucvar<='z') || ucvar=='/' || ucvar=='\\' || ucvar=='.'
      || (ucvar>='A' && ucvar<='Z') || ucvar==':' || ucvar>127)) goto find_lend;

      pkey[i] = (char)ucvar;
   }

   //�����Ⱥ�
   for(;;)
   {
      iv = (int)fread(&ucvar, 1, sizeof(ucvar), hcfg);
      if(iv != sizeof(ucvar)) return -1;
      if(ucvar == '=') break;
      if(ucvar == 0xD || ucvar == 0xA) return 0;
      if(ucvar != ' ' && ucvar != '\t') goto find_lend;
   }

   //����ֵ�����
   for(;;)
   {
      iv = (int)fread(&ucvar, 1, sizeof(ucvar), hcfg);
      if(iv != sizeof(ucvar)) return -1;
      if(ucvar == '"') break;
      if(ucvar == 0xD || ucvar == 0xA) return 0;
      if(ucvar != ' ' && ucvar != '\t') goto find_lend;
   }

   //��ȡֵ�ַ���
   for(i=0 ;; i++)
   {
      if(i > SCR_VAR_MAXLEN) goto find_lend;

      iv = (int)fread(&ucvar, 1, sizeof(ucvar), hcfg);
      if(iv != sizeof(ucvar)) return -1;

      if(ucvar == '"')
      {
         pvar[i] = 0;
         break;
      }

      if(ucvar == 0xD || ucvar == 0xA) return 0;
      if(ucvar < 32) goto find_lend;

      pvar[i] = (char)ucvar;
   }

   irst = 1;

   find_lend:

   //�����н���
   for(;;)
   {
      iv = (int)fread(&ucvar, 1, sizeof(ucvar), hcfg);
      if(iv != sizeof(ucvar)) break;
      if(ucvar == 0xD || ucvar == 0xA) break;
   }

   return irst;
}

//����: sc_write
//����: ��ӻ��д��ֵ�������ļ�
//����: name  �����ļ�·��������
//      pkey  ����(\0��β���ִ�)
//      pvar  ��ֵ(\0��β���ִ�)
//      mode  ģʽ(0:��ӻ��滻,SCW_APPEND:���,SCW_REPLACE:�滻)
//����: �ɹ�����0, ʧ���򷵻�-1��
int sc_write(const char * name, const char * pkey, const char * pvar, int mode)
{
   int iv, i;
   int rst = -1;
   int iwr = -1;
   unsigned char * cp;
   FILE * hs;
   FILE * fd;

   char gckey[SCR_KEY_MAXLEN + 1];
   char gcvar[SCR_VAR_MAXLEN + 1];

   if(name == 0) return -1;
   if(pkey == 0) return -1;
   if(pvar == 0) return -1;

   cp = (unsigned char *)pkey;
   iv = 0;

   for(i=0; i<=SCR_KEY_MAXLEN+1; i++,cp++)
   {
      if(*cp==0)
      {
         iv = i;
         break;
      }

      if(!((*cp>='0' && *cp<='9') || *cp==' ' || *cp=='-' || *cp=='_'
         || (*cp>='a' && *cp<='z') || *cp=='/' || *cp=='\\' || *cp=='.'
         || (*cp>='A' && *cp<='Z') || *cp==':' || *cp>127)) return -1;
   }

   if(iv>SCR_KEY_MAXLEN || iv<1) return -1;

   cp = (unsigned char *)pvar;
   iv = 0;

   for(i=0; i<=SCR_VAR_MAXLEN+1; i++,cp++)
   {
      if(*cp == 0)
      {
         iv = i;
         break;
      }

      if(*cp < 32 || *cp == '"') return -1;
   }

   if(iv>SCR_VAR_MAXLEN || iv<0) return -1;

   _snprintf(gcvar, sizeof(gcvar) - 1,
   "%s\\~scw_%x_%x", SCR_TMP_DESDIR, _getpid(), (int)&hs);
   gcvar[sizeof(gcvar) - 1] = 0;
   fd = fopen(gcvar, "wb");
   if(fd == NULL) return -1;

   hs = sc_open(name);
   if(hs != 0) for(;;)
   {
      iv = sc_readline(hs, gckey, gcvar);
      if(iv < 0) break;
      if(iv == 0) continue;

      cp = (unsigned char *)gcvar;
      iv = strcmp(pkey, gckey);
      if(iv == 0 && mode == SCW_APPEND) goto fun_end;
      if(iv == 0){ cp = (unsigned char *)pvar; iwr = 0; }

      if(fputs("<", fd) == EOF) goto fun_end;
      if(fputs(gckey, fd) == EOF) goto fun_end;
      if(fputs("> = \"", fd) == EOF) goto fun_end;
      if(fputs((char *)cp, fd) == EOF) goto fun_end;
      if(fputs("\"", fd) == EOF) goto fun_end;
      if(fputs(newline, fd) == EOF) goto fun_end;
   }

   if(iwr != 0)
   {
      if(mode == SCW_REPLACE) goto fun_end;
      if(fputs("<", fd) == EOF) goto fun_end;
      if(fputs(pkey, fd) == EOF) goto fun_end;
      if(fputs("> = \"", fd) == EOF) goto fun_end;
      if(fputs(pvar, fd) == EOF) goto fun_end;
      if(fputs("\"", fd) == EOF) goto fun_end;
      if(fputs(newline, fd) == EOF) goto fun_end;
   }

   rst = 0;

   fun_end:
   {
      fclose(fd);
      sc_close(hs);

      if(rst == 0)
      {
         _snprintf(gcvar, sizeof(gcvar) - 1,
         "%s\\~scw_%x_%x", SCR_TMP_DESDIR, _getpid(), (int)&hs);
         gcvar[sizeof(gcvar) - 1] = 0;
         rst = sc_copy(gcvar, name);

         _snprintf(gcvar, sizeof(gcvar) - 1,
         "%s\\~scw_%x_%x", SCR_TMP_DESDIR, _getpid(), (int)&hs);
         gcvar[sizeof(gcvar) - 1] = 0;
         _unlink(gcvar);
      }
      else
      {
         _snprintf(gcvar, sizeof(gcvar) - 1,
         "%s\\~scw_%x_%x", SCR_TMP_DESDIR, _getpid(), (int)&hs);
         gcvar[sizeof(gcvar) - 1] = 0;
         _unlink(gcvar);
      }
   }
   return rst;
}
