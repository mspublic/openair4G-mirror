#include "em1_drv.h"

static int em1_ioctl_fifo_write(struct em1_private_s *pv,
				struct em1_ioctl_fifo_params *p)
{
	DECLARE_WAITQUEUE(wait, current);
	int res;

	if (!em1_user_op_enter(pv, &wait, &pv->rq_wait_fifo_w,
			       EM1_BUSY_FIFO_W, TASK_INTERRUPTIBLE))
	{
		res = em1_fifo_write(pv, p->words, p->count);
		pv->busy &= ~EM1_BUSY_FIFO_W;
	}

	em1_user_op_leave(pv, &wait, &pv->rq_wait_fifo_w);

	return res;
}

static int em1_ioctl_fifo_read(struct em1_private_s *pv,
			       struct em1_ioctl_fifo_params *p)
{
	DECLARE_WAITQUEUE(wait, current);
	int res;

	if (!em1_user_op_enter(pv, &wait, &pv->rq_wait_fifo_r,
			       EM1_BUSY_FIFO_R, TASK_INTERRUPTIBLE))
	{
		res = em1_fifo_read(pv, p->words, p->count);
		pv->busy &= ~EM1_BUSY_FIFO_R;
	}

	em1_user_op_leave(pv, &wait, &pv->rq_wait_fifo_r);

	return res;
}


int em1_ioctl(struct inode *inode, struct file *file,
	      unsigned int cmd, unsigned long arg)
{
	struct em1_private_s *pv = file->private_data;

	switch ((enum em1_ioctl_cmd)cmd)
	{
          case EM1_IOCTL_FIFO_WRITE: {
		struct em1_ioctl_fifo_params p;
		if (copy_from_user(&p, (void*)arg, sizeof(p)))
			return -EFAULT;
		return em1_ioctl_fifo_write(pv, &p);
	  }

	  case EM1_IOCTL_FIFO_READ: {
		struct em1_ioctl_fifo_params p;
		if (copy_from_user(&p, (void*)arg, sizeof(p)))
			return -EFAULT;		
		return em1_ioctl_fifo_read(pv, &p);
	  }
        } 
	return 0; 
}


/*
 * Local Variables:
 * c-file-style: "linux"
 * indent-tabs-mode: t
 * tab-width: 8
 * End:
 */

