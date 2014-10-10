#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/timer.h>
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/hdreg.h>	/* HDIO_GETGEO */
#include <linux/kdev_t.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>	/* invalidate_bdev */
#include <linux/bio.h>
#include <linux/version.h>


#define SIMP_BLKDEV_DEVICEMAJOR        COMPAQ_SMART2_MAJOR
#define SIMP_BLKDEV_DISKNAME        "simp_blkdev"
#define SIMP_BLKDEV_BYTES        (16*1024*1024)

static struct request_queue *simp_blkdev_queue;
static struct gendisk *simp_blkdev_disk;
unsigned char simp_blkdev_data[SIMP_BLKDEV_BYTES];

static int simp_blkdev_make_request(struct request_queue *q, struct bio *bio)
{
        struct bio_vec *bvec;
        int i;
        void *dsk_mem;

        if ((bio->bi_sector << 9) + bio->bi_size > SIMP_BLKDEV_BYTES) {
                printk(KERN_ERR SIMP_BLKDEV_DISKNAME
                        ": bad request: block=%llu, count=%u\n",
                        (unsigned long long)bio->bi_sector, bio->bi_size);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
                bio_endio(bio, 0, -EIO);
#else
                bio_endio(bio, -EIO);
#endif
                return 0;
        }

        dsk_mem = simp_blkdev_data + (bio->bi_sector << 9);

        bio_for_each_segment(bvec, bio, i) {
                void *iovec_mem;

                switch (bio_rw(bio)) {
                case READ:
                case READA:
                        iovec_mem = kmap(bvec->bv_page) + bvec->bv_offset;
                        memcpy(iovec_mem, dsk_mem, bvec->bv_len);
                        kunmap(bvec->bv_page);
                        break;
                case WRITE:
                        iovec_mem = kmap(bvec->bv_page) + bvec->bv_offset;
                        memcpy(dsk_mem, iovec_mem, bvec->bv_len);
                        kunmap(bvec->bv_page);
                        break;
                default:
                        printk(KERN_ERR SIMP_BLKDEV_DISKNAME
                                ": unknown value of bio_rw: %lu\n",
                                bio_rw(bio));
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
                        bio_endio(bio, 0, -EIO);
#else
                        bio_endio(bio, -EIO);
#endif
                        return 0;
                }
                dsk_mem += bvec->bv_len;
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
        bio_endio(bio, bio->bi_size, 0);
#else
        bio_endio(bio, 0);
#endif

        return 0;
}


struct block_device_operations simp_blkdev_fops = {
        .owner                = THIS_MODULE,
};

static int __init simp_blkdev_init(void)
{
        int ret;

        simp_blkdev_queue = blk_alloc_queue(GFP_KERNEL);
        if (!simp_blkdev_queue) {
                ret = -ENOMEM;
                goto err_alloc_queue;
        }
        blk_queue_make_request(simp_blkdev_queue, simp_blkdev_make_request);

        simp_blkdev_disk = alloc_disk(1);
        if (!simp_blkdev_disk) {
                ret = -ENOMEM;
                goto err_alloc_disk;
        }

        strcpy(simp_blkdev_disk->disk_name, SIMP_BLKDEV_DISKNAME);
        simp_blkdev_disk->major = SIMP_BLKDEV_DEVICEMAJOR;
        simp_blkdev_disk->first_minor = 0;
        simp_blkdev_disk->fops = &simp_blkdev_fops;
        simp_blkdev_disk->queue = simp_blkdev_queue;
        set_capacity(simp_blkdev_disk, SIMP_BLKDEV_BYTES>>9);
        add_disk(simp_blkdev_disk);

        return 0;

err_alloc_disk:
        blk_cleanup_queue(simp_blkdev_queue);
err_alloc_queue:
        return ret;
}


static void __exit simp_blkdev_exit(void)
{
        del_gendisk(simp_blkdev_disk);
        put_disk(simp_blkdev_disk);
        blk_cleanup_queue(simp_blkdev_queue);
}

module_init(simp_blkdev_init);
module_exit(simp_blkdev_exit);