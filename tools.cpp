
#include "tools.h"
#include "string.h"

//���㲻ͬ��bit��
int bitdiff(const char* left,const char* right,int count)
{
	int diffcount=0;
	for(int icount=0;icount<count;icount++)
	{
		if (left[icount]!=right[icount])
		{
			diffcount++;
		}
	}
	return diffcount;
}