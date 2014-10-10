
#ifndef _SEM_H_
#define _SEM_H_

#ifndef MEMDEV_MAJOR
#define MEMDEV_MAJOR 0  /*预设的mem的主设备号*/
#endif

#ifndef MEMDEV_NR_DEVS
#define MEMDEV_NR_DEVS 2    /*设备数*/
#endif

#ifndef MEMDEV_SIZE
#define MEMDEV_SIZE 4096
#endif

/*mem设备描述结构体*/
struct mem_dev                                     
{                                                        
  char *data;                      
  unsigned long size;    
  wait_queue_head_t inq;   
};

#endif /* _MEMDEV_H_ */
