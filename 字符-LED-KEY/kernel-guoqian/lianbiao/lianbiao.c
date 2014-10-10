#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/timer.h>
#include<asm/uaccess.h>
#include<linux/slab.h> //kmalloc
#include<linux/list.h>

struct student
{
	char name[100];
	int num;
	struct list_head list;

};

struct student *pstudent;
struct student *tmp_student;
struct list_head student_list;
struct list_head *pos;

int mylist_init(void)
{
	int i = 0;
	INIT_LIST_HEAD(&student_list);//student_listÊÇÁ´±íÍ·£¬È¡µØÖ·±ä³ÉÖ¸Õë

	pstudent = kmalloc(sizeof(struct student)*5,GFP_KERNEL);//·ÖÅä¿Õ¼ä//pstudent[i]½á¹¹Êı×éÓĞ5¸ö³ÉÔ±£
	memset(pstudent,0,sizeof(struct student)*5);//³õÊ¼»¯

	for(i=0;i<5;i++)
	{
		sprintf(pstudent[i].name,"your score is %d",i+90);/*°ÑÕûÊı123 ´òÓ¡³ÉÒ»¸ö×Ö·û´®±£´æÔÚs ÖĞ,
		                                                                                                sprintf(s, "%d", 123); //²úÉú"123" */
		pstudent[i].num = i+1;
		list_add(&(pstudent[i].list),&student_list);//´Óstudent_list¿ªÊ¼¼Ó
	}
	list_for_each(pos,&student_list)
	{
		tmp_student = list_entry(pos,struct student,list);
		printk("<0>student %d name:%s\n",tmp_student->num,tmp_student->name);
	}
	return 0;
}
void mylist_exit(void)
{
	int i;
	for(i=0;i<5;i++)
	{
		list_del(&(pstudent[i].list));
	}
	kfree(pstudent);
}
module_init(mylist_init);
module_exit(mylist_exit);
MODULE_LICENSE("GPL");
	


