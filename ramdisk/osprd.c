#include <linux/version.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>  /* printk() */
#include <linux/errno.h>   /* error codes */
#include <linux/types.h>   /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/wait.h>
#include <linux/file.h>


// USED FOR CRYPTO STUFF //
// ********************* //
#include <linux/crypto.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>

#define FILL_SG(sg,ptr,len) do { (sg)->page = virt_to_page(ptr); (sg)->offset = offset_in_page(ptr); (sg)->length = len; } while (0)

enum crypto_op {
	CRYPTO_ENCRYPT = 0,
	CRYPTO_DECRYPT = 1
};

const static char *crypto_algo = "aes";

// Used for debugging
static void hexdump(unsigned char *buf, unsigned int len)
{
	while (len--)
		printk("%02x", *buf++);
	printk("\n");
}

// ********************* //


#include "spinlock.h"
#include "osprd.h"

/* The size of an OSPRD sector. */
#define SECTOR_SIZE	512

/* This flag is added to an OSPRD file's f_flags to indicate that the file
 * is locked. */
#define F_OSPRD_LOCKED	0x80000

/* eprintk() prints messages to the console.
 * (If working on a real Linux machine, change KERN_NOTICE to KERN_ALERT or
 * KERN_EMERG so that you are sure to see the messages.  By default, the
 * kernel does not print all messages to the console.  Levels like KERN_ALERT
 * and KERN_EMERG will make sure that you will see messages.) */
#define eprintk(format, ...) printk(KERN_NOTICE format, ## __VA_ARGS__)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("CS 111 RAM Disk");
MODULE_AUTHOR("Georgi Baghdasaryan");

#define OSPRD_MAJOR	222

/* This module parameter controls how big the disk will be.
 * You can specify module parameters when you load the module,
 * as an argument to insmod: "insmod osprd.ko nsectors=4096" */
static int nsectors = 32;
module_param(nsectors, int, 0);


typedef struct pid_list {
	pid_t curr;
	struct pid_list *next;
} *pid_list_t;

/* The internal representation of our device. */
typedef struct osprd_info {
	char key[16];			// A 16 bit ramdisk
					//   encryption/decryption key

	uint8_t *data;                  // The data array. Its size is
	                                // (nsectors * SECTOR_SIZE) bytes

	osp_spinlock_t mutex;           // Mutex for synchronizing access to
					// this block device

	unsigned ticket_head;		// Currently running ticket for
					// the device lock

	unsigned ticket_tail;		// Next available ticket for
					// the device lock

	wait_queue_head_t blockq;       // Wait queue for tasks blocked on
					// the device lock

	pid_t write_lock_holder; 	// A data structure that holds current
					// write lock process'  pid

	pid_list_t read_lock_holder;
	unsigned num_read_locks;

	struct request_queue *queue;    // The device request queue.
	spinlock_t qlock;		// Used internally for mutual
	                                //   exclusion in the 'queue'.
	struct gendisk *gd;             // The generic disk.
} osprd_info_t;

#define NOSPRD 4
static osprd_info_t osprds[NOSPRD];


// Declare useful helper functions

/*
 * file2osprd(filp)
 *   Given an open file, check whether that file corresponds to an OSP ramdisk.
 *   If so, return a pointer to the ramdisk's osprd_info_t.
 *   If not, return NULL.
 */
static osprd_info_t *file2osprd(struct file *filp);






static ssize_t crypto(osprd_info_t *d, char **data, int data_size, enum crypto_op op);
/*
 * for_each_open_file(task, callback, user_data)
 *   Given a task, call the function 'callback' once for each of 'task's open
 *   files.  'callback' is called as 'callback(filp, user_data)'; 'filp' is
 *   the open file, and 'user_data' is copied from for_each_open_file's third
 *   argument.
 */
static void for_each_open_file(struct task_struct *task,
			       void (*callback)(struct file *filp,
						osprd_info_t *user_data),
			       osprd_info_t *user_data);


/*
 * osprd_process_request(d, req)
 *   Called when the user reads or writes a sector.
 *   Should perform the read or write, as appropriate.
 */
static void osprd_process_request(osprd_info_t *d, struct request *req)
{
	uint8_t *start_loc = NULL;
	unsigned num_bytes;

	if (!blk_fs_request(req)) {
		end_request(req, 0);
		return;
	}

	start_loc = d->data + SECTOR_SIZE * req->sector;
	num_bytes = SECTOR_SIZE * req->current_nr_sectors;

	if (start_loc + num_bytes > d->data + SECTOR_SIZE * nsectors) {
		printk(KERN_WARNING "Out of bound write avoided.\n");
		end_request(req, 0);
	}

	osp_spin_lock(&d->mutex);
	if (rq_data_dir(req) == READ) {
		memcpy(req->buffer, start_loc, num_bytes);
	} else if (rq_data_dir(req) == WRITE) {
		memcpy(start_loc, req->buffer, num_bytes);
	} else {
		osp_spin_unlock(&d->mutex);
		eprintk("Unknown request.\n");
		end_request(req, 0);
		return;
	}
	osp_spin_unlock(&d->mutex);

	end_request(req, 1);
}


// This function is called when a /dev/osprdX file is opened.
// You aren't likely to need to change this.
static int osprd_open(struct inode *inode, struct file *filp)
{
	// Always set the O_SYNC flag. That way, we will get writes immediately
	// instead of waiting for them to get through write-back caches.
	filp->f_flags |= O_SYNC;
	return 0;
}


// This function is called when a /dev/osprdX file is finally closed.
// (If the file descriptor was dup2ed, this function is called only when the
// last copy is closed.)
static int osprd_close_last(struct inode *inode, struct file *filp)
{
	if (filp) {
		osprd_info_t *d = file2osprd(filp);
		int filp_writable = filp->f_mode & FMODE_WRITE;
		pid_list_t curr = NULL,
			   prev = NULL;

		if ((filp->f_flags & F_OSPRD_LOCKED) == 0)
			return 0;

		osp_spin_lock(&d->mutex);
		if (filp_writable) {
			if (d->write_lock_holder != 0)
				d->write_lock_holder = -1;
			goto out;
		}
		curr = d->read_lock_holder;
		prev = d->read_lock_holder;
		d->num_read_locks--;

		while (curr != NULL) {
			if (curr->curr == current->pid) {
				if (prev == NULL) {
					d->read_lock_holder = curr->next;
					continue;
				}
				prev->next = curr->next;
				break;
			}
			prev = curr;
			curr = curr->next;
		}

	out:
		filp->f_flags &= ~F_OSPRD_LOCKED;
		osp_spin_unlock(&d->mutex);

		wake_up_all(&d->blockq);

		// This line avoids compiler warnings; you may remove it.
		(void) filp_writable, (void) d;
	}

	return 0;
}


/*
 * osprd_lock
 */

/*
 * osprd_ioctl(inode, filp, cmd, arg)
 *   Called to perform an ioctl on the named file.
 */
int osprd_ioctl(struct inode *inode, struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	osprd_info_t *d = file2osprd(filp);	// device info

	// is file open for writing?
	int filp_writable = (filp->f_mode & FMODE_WRITE) != 0;

	// This line avoids compiler warnings; you may remove it.
	(void) filp_writable, (void) d;

	// Set 'r' to the ioctl's return value: 0 on success, negative on error

	if (d == NULL)
		return -1;

	if (cmd == OSPRDIOCACQUIRE) {
		unsigned local_ticket;
		pid_list_t prev = NULL,
			   curr = NULL;

		// USED FOR TESTING crypto(...)
		// ****************************
		printk("Data encryption started...\n");
		char *data = kmalloc(512 * sizeof(char), GFP_KERNEL);
		if (!data)
			return -ENOMEM;
		strcpy(data, "hello, world!");
		// NOTE: Data and data_size should be 2^N bytes long
		printk("DATA: %s\n", data);
		crypto(d, &data, 512, 0);
		printk("ENCRYPTED: %s\n", data);
		crypto(d, &data, 512, 1);
		printk("DECRYPTED: %s\n", data);
		printk("Data encryption finished...\n");
		// ****************************

		osp_spin_lock(&d->mutex);
		if (d->write_lock_holder == current->pid) {
			osp_spin_unlock(&d->mutex);
			return -EDEADLK;
		}

		local_ticket = d->ticket_head;
		d->ticket_head++;
		osp_spin_unlock(&d->mutex);

		while (d->write_lock_holder != -1
		    || (filp_writable && d->num_read_locks != 0)
		    || local_ticket != d->ticket_tail) {
			if (wait_event_interruptible(d->blockq,1) == -ERESTARTSYS)
				return -ERESTARTSYS;

			schedule();
		}

		if (filp_writable) {
			osp_spin_lock(&d->mutex);

			curr = d->read_lock_holder;
			while (curr != NULL) {
				if (curr->curr == current->pid) {
					osp_spin_unlock(&d->mutex);
					return -EDEADLK;
				} else {
					prev = curr;
					curr = curr->next;
				}
			}
			
			filp->f_flags |= F_OSPRD_LOCKED;
			d->write_lock_holder = current->pid;
			d->ticket_tail++;
			osp_spin_unlock(&d->mutex);
		} else {
			osp_spin_lock(&d->mutex);

			filp->f_flags |= F_OSPRD_LOCKED;
			d->num_read_locks++;

			curr = d->read_lock_holder;
			while (curr != NULL) {
				prev = curr;
				curr = curr->next;
			}

			if (prev == NULL) {
				d->read_lock_holder = kmalloc(sizeof(pid_list_t), GFP_ATOMIC);
				if (!d->read_lock_holder)
					return -ENOMEM;
				d->read_lock_holder->curr = current->pid;
				d->read_lock_holder->next = NULL;
			} else {
				prev->next = kmalloc(sizeof(pid_list_t), GFP_ATOMIC);
				if (!prev->next)
					return -ENOMEM;
				prev->next->curr = current->pid;
				prev->next->next = NULL;
			}

			d->ticket_tail++;
			osp_spin_unlock(&d->mutex);
		}
	} else if (cmd == OSPRDIOCTRYACQUIRE) {
		osp_spin_lock(&d->mutex);

		if (d->write_lock_holder != -1
		 || (filp_writable && d->num_read_locks != 0)
		 || d->ticket_head != d->ticket_tail
		 || d->write_lock_holder == current->pid) {
			osp_spin_unlock(&d->mutex);
			return -EBUSY;	
		}

		if (filp_writable) {
			d->write_lock_holder = current->pid;
		} else {
			d->num_read_locks++;
		}

		d->ticket_head++;
		if (d->ticket_tail < d->ticket_head)
			d->ticket_tail++;

		filp->f_flags |= F_OSPRD_LOCKED;
		osp_spin_unlock(&d->mutex);

		wake_up_all(&d->blockq);
	} else if (cmd == OSPRDIOCRELEASE) {
		if ((filp->f_flags & F_OSPRD_LOCKED) == 0)
			return -EINVAL;

		osp_spin_lock(&d->mutex);
		if (filp_writable)
			d->write_lock_holder = -1;
		else
			d->num_read_locks--;
		filp->f_flags &= ~F_OSPRD_LOCKED;
		osp_spin_unlock(&d->mutex);

		wake_up_all(&d->blockq);
	} else
		return -ENOTTY; /* unknown command */

	return 0;
}


// Initialize internal fields for an osprd_info_t.

static void osprd_setup(osprd_info_t *d)
{
	/* Initialize the wait queue. */
	init_waitqueue_head(&d->blockq);
	osp_spin_lock_init(&d->mutex);
	d->ticket_head = d->ticket_tail = 0;
	d->write_lock_holder = -1;
	d->num_read_locks = 0;
	memset(d->key, 0, sizeof(d->key));
}

static int crypto(osprd_info_t *d, char **data, int data_size, enum crypto_op op)
{
	int ret_code = 1;
	char iv[16];

	// Local variables
	struct crypto_tfm *tfm;
	struct scatterlist sg;
	int ret;

	//memset(key, 0, sizeof(key));
	memset(iv, 0, sizeof(iv));

	tfm = crypto_alloc_tfm(crypto_algo, 0);
	if (tfm == NULL) {
		eprintk("failed to load transform for %s\n", crypto_algo);
		return 0;
	}

	ret = crypto_cipher_setkey(tfm, d->key, sizeof(d->key));
	if (ret) {
		eprintk("setkey() failed flags=%x\n", tfm->crt_flags);
		ret_code = 0;
		goto out;
	}

	FILL_SG(&sg, *data, data_size);

	crypto_cipher_set_iv(tfm, iv, crypto_tfm_alg_ivsize (tfm));
	if (op == CRYPTO_ENCRYPT) {
		ret = crypto_cipher_encrypt(tfm, &sg, &sg, sg.length);
	} else if (op == CRYPTO_DECRYPT) {
		ret = crypto_cipher_decrypt(tfm, &sg, &sg, sg.length);
	} else {
		eprintk("unknown cryptographic operation specified\n");
		ret_code = 0;
		goto out;
	}

	if (ret) {
		eprintk("%s failed, flags=0x%x\n", op == CRYPTO_ENCRYPT ? "encryption" : "decryption", tfm->crt_flags);
		ret_code = 0;
		goto out;
	}

out:
	crypto_free_tfm(tfm);

	return ret_code;
}


/*****************************************************************************/
/*         THERE IS NO NEED TO UNDERSTAND ANY CODE BELOW THIS LINE!          */
/*                                                                           */
/*****************************************************************************/

// Process a list of requests for a osprd_info_t.
// Calls osprd_process_request for each element of the queue.

static void osprd_process_request_queue(request_queue_t *q)
{
	osprd_info_t *d = (osprd_info_t *) q->queuedata;
	struct request *req;

	while ((req = elv_next_request(q)) != NULL)
		osprd_process_request(d, req);
}


// Some particularly horrible stuff to get around some Linux issues:
// the Linux block device interface doesn't let a block device find out
// which file has been closed.  We need this information.

static ssize_t _osprd_read(struct file *file,
			   char *buffer,      /* The buffer to fill with data */
			   size_t count,      /* The length of the buffer     */
			   loff_t *offset)    /* Our offset in the file       */
{



	// READ CODE GOES HERE


	// returns NUM_READ_BYTES
}

static ssize_t _osprd_write(struct file *file,
			    char *buffer,      /* The buffer to fill with data */
			    size_t count,      /* The length of the buffer     */
			    loff_t *offset)    /* Our offset in the file       */
{



	// WRITE CODE GOES HERE


	// returns NUM_WRITTEN_BYTES
}

static struct file_operations osprd_blk_fops = {
// UNCOMMENT THE FOLLOWING TWO LINES WHEN DONE
//	.read = _osprd_read,
//	.write = _osprd_write
};

static int (*blkdev_release)(struct inode *, struct file *);

static int _osprd_release(struct inode *inode, struct file *filp)
{
	if (file2osprd(filp))
		osprd_close_last(inode, filp);
	return (*blkdev_release)(inode, filp);
}

static int _osprd_open(struct inode *inode, struct file *filp)
{
	if (!osprd_blk_fops.open) {
		memcpy(&osprd_blk_fops, filp->f_op, sizeof(osprd_blk_fops));
		blkdev_release = osprd_blk_fops.release;
		osprd_blk_fops.release = _osprd_release;
	}
	filp->f_op = &osprd_blk_fops;
	return osprd_open(inode, filp);
}


// The device operations structure.

static struct block_device_operations osprd_ops = {
	.owner = THIS_MODULE,
	.open = _osprd_open,
	// .release = osprd_release, // we must call our own release
	.ioctl = osprd_ioctl
};


// Given an open file, check whether that file corresponds to an OSP ramdisk.
// If so, return a pointer to the ramdisk's osprd_info_t.
// If not, return NULL.

static osprd_info_t *file2osprd(struct file *filp)
{
	if (filp) {
		struct inode *ino = filp->f_dentry->d_inode;
		if (ino->i_bdev
		    && ino->i_bdev->bd_disk
		    && ino->i_bdev->bd_disk->major == OSPRD_MAJOR
		    && ino->i_bdev->bd_disk->fops == &osprd_ops)
			return (osprd_info_t *) ino->i_bdev->bd_disk->private_data;
	}
	return NULL;
}


// Call the function 'callback' with data 'user_data' for each of 'task's
// open files.

static void for_each_open_file(struct task_struct *task,
		  void (*callback)(struct file *filp, osprd_info_t *user_data),
		  osprd_info_t *user_data)
{
	int fd;
	task_lock(task);
	spin_lock(&task->files->file_lock);
	{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 13)
		struct files_struct *f = task->files;
#else
		struct fdtable *f = task->files->fdt;
#endif
		for (fd = 0; fd < f->max_fds; fd++)
			if (f->fd[fd])
				(*callback)(f->fd[fd], user_data);
	}
	spin_unlock(&task->files->file_lock);
	task_unlock(task);
}


// Destroy a osprd_info_t.

static void cleanup_device(osprd_info_t *d)
{
	wake_up_all(&d->blockq);
	if (d->gd) {
		del_gendisk(d->gd);
		put_disk(d->gd);
	}
	if (d->queue)
		blk_cleanup_queue(d->queue);
	if (d->data)
		vfree(d->data);
}


// Initialize a osprd_info_t.

static int setup_device(osprd_info_t *d, int which)
{
	memset(d, 0, sizeof(osprd_info_t));

	/* Get memory to store the actual block data. */
	if (!(d->data = vmalloc(nsectors * SECTOR_SIZE)))
		return -1;
	memset(d->data, 0, nsectors * SECTOR_SIZE);

	/* Set up the I/O queue. */
	spin_lock_init(&d->qlock);
	if (!(d->queue = blk_init_queue(osprd_process_request_queue, &d->qlock)))
		return -1;
	blk_queue_hardsect_size(d->queue, SECTOR_SIZE);
	d->queue->queuedata = d;

	/* The gendisk structure. */
	if (!(d->gd = alloc_disk(1)))
		return -1;
	d->gd->major = OSPRD_MAJOR;
	d->gd->first_minor = which;
	d->gd->fops = &osprd_ops;
	d->gd->queue = d->queue;
	d->gd->private_data = d;
	snprintf(d->gd->disk_name, 32, "osprd%c", which + 'a');
	set_capacity(d->gd, nsectors);
	add_disk(d->gd);

	/* Call the setup function. */
	osprd_setup(d);

	return 0;
}

static void osprd_exit(void);


// The kernel calls this function when the module is loaded.
// It initializes the 4 osprd block devices.

static int __init osprd_init(void)
{
	int i, r;

	// shut up the compiler
	(void) for_each_open_file;
#ifndef osp_spin_lock
	(void) osp_spin_lock;
	(void) osp_spin_unlock;
#endif

	/* Register the block device name. */
	if (register_blkdev(OSPRD_MAJOR, "osprd") < 0) {
		printk(KERN_WARNING "osprd: unable to get major number\n");
		return -EBUSY;
	}

	/* Initialize the device structures. */
	for (i = r = 0; i < NOSPRD; i++)
		if (setup_device(&osprds[i], i) < 0)
			r = -EINVAL;

	if (r < 0) {
		printk(KERN_EMERG "osprd: can't set up device structures\n");
		osprd_exit();
		return -EBUSY;
	} else
		return 0;
}


// The kernel calls this function to unload the osprd module.
// It destroys the osprd devices.

static void osprd_exit(void)
{
	int i;
	for (i = 0; i < NOSPRD; i++)
		cleanup_device(&osprds[i]);
	unregister_blkdev(OSPRD_MAJOR, "osprd");
}


// Tell Linux to call those functions at init and exit time.
module_init(osprd_init);
module_exit(osprd_exit);
