#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/spi/spi.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/regulator/consumer.h>
#include <linux/ioctl.h>
#ifdef CONFIG_OF
#include <linux/of_gpio.h>
#endif
#include <linux/ese_p3.h>

#include <linux/pm_runtime.h>
#include <linux/spi/spidev.h>
#include <linux/clk.h>
#include <linux/wakelock.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/spi/spidev.h>
#include <asm/uaccess.h>

#ifdef CONFIG_SEC_FACTORY
#undef CONFIG_ESE_SECURE
#endif

#define SPI_DEFAULT_SPEED 6500000L

/* size of maximum read/write buffer supported by driver */
#define MAX_BUFFER_SIZE   259U

/* Different driver debug lever */
enum P3_DEBUG_LEVEL {
	P3_DEBUG_OFF,
	P3_FULL_DEBUG
};

/* Variable to store current debug level request by ioctl */
static unsigned char debug_level = P3_FULL_DEBUG;

#define P3_DBG_MSG(msg...) do { \
		switch (debug_level) { \
		case P3_DEBUG_OFF: \
			break; \
		case P3_FULL_DEBUG: \
			pr_info("[ESE-P3] :  " msg); \
			break; \
		default: \
			pr_err("[ESE-P3] : debug level %d", debug_level);\
			break; \
		}; \
	} while (0);

#define P3_ERR_MSG(msg...) pr_err("[ESE-P3] : " msg);
#define P3_INFO_MSG(msg...) pr_info("[ESE-P3] : " msg);

#ifdef CONFIG_ESE_SECURE
#define TO_STR2(s) #s
#define TO_STR(s) TO_STR2(s)
/* SPI_CLK(s) produces "spi3-sclk" */
#define SPI_CLK(x) "spi" TO_STR(CONFIG_ESE_SECURE_SPI_PORT) "-" #x "clk"
#endif

/* Device specific macro and structure */
struct p3_data {
	wait_queue_head_t read_wq; /* wait queue for read interrupt */
	struct mutex buffer_mutex; /* buffer mutex */
	struct spi_device *spi;  /* spi device structure */
	struct miscdevice p3_device; /* char device as misc driver */

	bool device_opened;
#ifdef FEATURE_ESE_WAKELOCK
	struct wake_lock ese_lock;
#endif
	unsigned long speed;
	unsigned int nfc_ese_pwr_req;
};

#ifdef CONFIG_ESE_SECURE
static int p3_suspend(void)
{
#if 0
	u64 r0 = 0, r1 = 0, r2 = 0, r3 = 0;
	int ret = 0;

	r0 = (0x83000032);
	ret = exynos_smc(r0, r1, r2, r3);

	if (ret)
		P3_ERR_MSG("P3 check suspend status! 0x%X\n", ret);
#endif

	return 0;
}

static int p3_resume(void)
{
#if 0
	u64 r0 = 0, r1 = 0, r2 = 0, r3 = 0;
	int ret = 0;

	r0 = (0x83000033);
	ret = exynos_smc(r0, r1, r2, r3);

	if (ret)
		P3_ERR_MSG("P3 check resume status! 0x%X\n", ret);
#endif
	return 0;
}

static int p3_clk_prepare(struct p3_data *data)
{
	int ret_val = 0;
	struct spi_device *spidev = NULL;

	spidev = spi_dev_get(data->spi);

#ifndef CONFIG_ESE_SECURE
	/* Qcom spi active pinctrl */
	ret_val = ese_spi_request_gpios(spidev);
	if (ret_val < 0)
		pr_err("%s: couldn't config spi gpio\n", __func__);
#endif
	usleep_range(200, 230);

	spidev->max_speed_hz = SPI_DEFAULT_SPEED;
	ret_val = ese_spi_clock_enable(spidev);
	if (ret_val < 0)
		pr_err("%s: Unable to enable spi clk\n",
				__func__);
	else {
		ret_val = ese_spi_clock_set_rate(spidev);
		if (ret_val < 0)
			pr_err("%s: Unable to set spi clk rate\n",
					__func__);
}

	return 0;
}

static int p3_clk_unprepare(struct p3_data *data)
{
	int ret_val = 0;
	struct spi_device *spidev = NULL;

	spidev = spi_dev_get(data->spi);

	ret_val = ese_spi_clock_disable(spidev);
	if (ret_val < 0)
		pr_err("%s: couldn't disable spi clks\n", __func__);

	return 0;
}
#endif

static int p3_power_onoff(struct p3_data *data, int onoff)
{
	gpio_direction_output(data->nfc_ese_pwr_req, onoff);

	return 0;
}

#ifndef CONFIG_ESE_SECURE
static int p3_xfer(struct p3_data *p3_device, struct p3_ioctl_transfer *tr)
{
        int    status = 0;
        struct spi_message m;
        struct spi_transfer t;
	unsigned char tx_buffer[MAX_BUFFER_SIZE] = {0x0, };
	unsigned char rx_buffer[MAX_BUFFER_SIZE] = {0x0, };

	P3_DBG_MSG("%s\n", __func__);

	if (p3_device == NULL || tr == NULL)
		return -EFAULT;

	if (tr->len > DEFAULT_BUFFER_SIZE || !tr->len)
		return -EMSGSIZE;

	if (copy_from_user(tx_buffer,
			tr->tx_buffer, tr->len) != 0)
		return -EFAULT;

        spi_message_init(&m);
        memset(&t, 0, sizeof(t));

	t.tx_buf = tx_buffer;
	t.rx_buf = rx_buffer;
	t.len = tr->len;

        spi_message_add_tail(&t, &m);

	status = spi_sync(p3_device->spi, &m);

	if (copy_to_user(tr->rx_buffer, rx_buffer, tr->len)) {
		P3_ERR_MSG("%s : failed to copy to user space\n", __func__);
		return -EFAULT;
}

	P3_DBG_MSG("%s p3_xfer,length=%d\n", __func__, tr->len);

	return status;
}

static int p3_rw_spi_message(struct p3_data *p3_device, unsigned long arg)
{
	struct p3_ioctl_transfer dup;
	int err = 0;

	if (copy_from_user(&dup, (void *)arg,
			   sizeof(struct p3_ioctl_transfer)) != 0) {
			P3_ERR_MSG("%s copy_from_user fail!\n", __func__);
		return -EFAULT;
	} else {
		err = p3_xfer(p3_device, &dup);
		if (err != 0) {
			P3_ERR_MSG("%s xfer failed!\n", __func__);
			return err;
        }
}

	return 0;
	}

#endif

static int spip3_open(struct inode *inode, struct file *filp)
{
	struct p3_data *p3_dev = container_of(filp->private_data,
			struct p3_data, p3_device);
	int ret = 0;

	/* for defence MULTI-OPEN */
	if (p3_dev->device_opened) {
		P3_ERR_MSG("%s - ALREADY opened!\n", __func__);
		return -EBUSY;
	}
	p3_dev->device_opened = true;
	P3_INFO_MSG("open\n");

#ifdef FEATURE_ESE_WAKELOCK
	wake_lock(&p3_dev->ese_lock);
#endif
	ret = p3_power_onoff(p3_dev, 1);
	if (ret < 0)
		P3_ERR_MSG(" test: failed to turn on LDO()\n");
	usleep_range(5000, 5500);

#ifdef CONFIG_ESE_SECURE
	p3_clk_prepare(p3_dev);
	p3_resume();
#endif

	filp->private_data = p3_dev;

	return 0;
}

static int spip3_release(struct inode *inode, struct file *filp)
{
	struct p3_data *p3_dev = filp->private_data;
	int ret = 0;

	P3_INFO_MSG("release\n");

	ret = p3_power_onoff(p3_dev, 0);
	if (ret < 0)
		P3_ERR_MSG(" test: failed to turn off LDO()\n");

#ifdef CONFIG_ESE_SECURE
	p3_clk_unprepare(p3_dev);
	p3_suspend();
#endif
#ifdef FEATURE_ESE_WAKELOCK
	if (wake_lock_active(&p3_dev->ese_lock)) {
		P3_DBG_MSG("%s: wake unlock at release!!\n", __func__);
		wake_unlock(&p3_dev->ese_lock);
	}
#endif

	p3_dev->device_opened = false;
	P3_DBG_MSG("%s : Major No: %d, Minor No: %d\n", __func__,
			imajor(inode), iminor(inode));
	return 0;
}

static long spip3_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	int ret = 0;
	struct p3_data *data = NULL;

	if (_IOC_TYPE(cmd) != P3_MAGIC) {
		P3_ERR_MSG("%s invalid magic. cmd=0x%X Received=0x%X \
		 Expected=0x%X\n", __func__, cmd, _IOC_TYPE(cmd), P3_MAGIC);
		return -ENOTTY;
	}

	data = filp->private_data;

	mutex_lock(&data->buffer_mutex);
	switch (cmd) {
		case P3_SET_DBG:
			debug_level = (unsigned char)arg;
			P3_DBG_MSG(KERN_INFO"[NXP-P3] -  Debug level %d", debug_level);
			break;
		case  P3_ENABLE_SPI_CLK:
		P3_DBG_MSG("%s P3_ENABLE_SPI_CLK\n", __func__);
#ifdef CONFIG_ESE_SECURE
		ret = p3_clk_prepare(data);
#endif
			break;
		case P3_DISABLE_SPI_CLK :
			P3_DBG_MSG("%s P3_DISABLE_SPI_CLK\n", __func__);
#ifdef CONFIG_ESE_SECURE
			ret = p3_clk_unprepare(data);
#endif
			break;
	/*non TZ */
#ifndef CONFIG_ESE_SECURE
		case P3_RW_SPI_DATA :
			ret = p3_rw_spi_message(data, arg);
			if (ret < 0)
				P3_ERR_MSG("%s P3_RW_SPI_DATA failed [%d].\n",
					__func__, ret);
			break;
#endif
		case P3_SET_PWR:
		case P3_SET_POLL:
		case P3_SET_SPI_CONFIG:
		case P3_ENABLE_SPI_CS :
		case P3_DISABLE_SPI_CS :
		case P3_ENABLE_CLK_CS :
		case  P3_DISABLE_CLK_CS:
		case P3_SWING_CS :
		case P3_SET_SPI_CLK :
			P3_ERR_MSG("%s deprecated IOCTL:0x%X\n", __func__, cmd);
			break;

		default:
		P3_DBG_MSG("%s no matching ioctl! 0x%X\n", __func__, cmd);
		ret = -EINVAL;
	}
	mutex_unlock(&data->buffer_mutex);

	return ret;
}

#ifndef CONFIG_ESE_SECURE
static ssize_t spip3_write(struct file *filp, const char *buf, size_t count,
		loff_t *offset)
{
	int ret = -1;
	struct p3_data *p3_dev;
	unsigned char tx_buffer[MAX_BUFFER_SIZE] = {0x0, };

	P3_DBG_MSG("spip3_write -Enter count %zu\n", count);

	p3_dev = filp->private_data;

	mutex_lock(&p3_dev->buffer_mutex);
	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	if (copy_from_user(&tx_buffer[0], &buf[0], count)) {
		P3_ERR_MSG("%s : failed to copy from user space\n", __func__);
		mutex_unlock(&p3_dev->buffer_mutex);
		return -EFAULT;
	}

	/* Write data */
	ret = spi_write(p3_dev->spi, &tx_buffer[0], count);
	if (ret < 0)
		ret = -EIO;
	else
		ret = count;

	mutex_unlock(&p3_dev->buffer_mutex);
	P3_DBG_MSG(KERN_ALERT "spip3_write ret %d- Exit\n", ret);

	return ret;
}

static ssize_t spip3_read(struct file *filp, char *buf, size_t count,
		loff_t *offset)
{
	int ret = -EIO;
	struct p3_data *p3_dev = filp->private_data;
	unsigned char rx_buffer[MAX_BUFFER_SIZE] = {0x0, };

	P3_DBG_MSG("spip3_read count %zu - Enter\n", count);

	mutex_lock(&p3_dev->buffer_mutex);

	/* Read the availabe data along with one byte LRC */
	ret = spi_read(p3_dev->spi, (void *)rx_buffer, count);
	if (ret < 0) {
		P3_ERR_MSG("spi_read failed\n");
		ret = -EIO;
		goto fail;
	}

	if (copy_to_user(buf, &rx_buffer[0], count)) {
		P3_ERR_MSG("%s : failed to copy to user space\n", __func__);
		ret = -EFAULT;
		goto fail;
	}
	ret = count;

fail:
	P3_DBG_MSG("%s ret %d Exit\n", __func__, ret);
	mutex_unlock(&p3_dev->buffer_mutex);

	return ret;
}
#endif

/* possible fops on the p3 device */
static const struct file_operations spip3_dev_fops = {
	.owner =        THIS_MODULE,
#ifndef CONFIG_ESE_SECURE
	.read =         spip3_read,
	.write =        spip3_write,
#endif
	.open =         spip3_open,
	.release =      spip3_release,
	.unlocked_ioctl = spip3_ioctl,
};

static int p3_parse_dt(struct device *dev, struct p3_data *data)
{
	struct device_node *np = dev->of_node;
	int ret = -1;

	data->nfc_ese_pwr_req = of_get_named_gpio(np, "p3-ese_pwr_req", 0);

	if (data->nfc_ese_pwr_req < 0) {
		P3_ERR_MSG("%s - fail get nfc_ese_pwr_req\n", __func__);
		return -1;
	}

	ret = gpio_request(data->nfc_ese_pwr_req, "ese_pwr_req");
	if (ret) {
		P3_ERR_MSG("%s - failed to request ese_pwr_req\n", __func__);
		return -1;
	}

	P3_DBG_MSG("%s: nfc_ese_pwr_req=%d\n", __func__, data->nfc_ese_pwr_req);

	return ret;
}

static int spip3_probe(struct spi_device *spi)
{
	int ret = -1;
	struct p3_data *data = NULL;

	P3_INFO_MSG("%s chip select : %d , bus number = %d\n",
		__func__, spi->chip_select, spi->master->bus_num);

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (data == NULL) {
		P3_ERR_MSG("failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto err_exit;
	}

	ret = p3_parse_dt(&spi->dev, data);
	if (ret) {
		P3_ERR_MSG("%s - Failed to parse DT\n", __func__);
		goto p3_parse_dt_failed;
	}

#ifndef CONFIG_ESE_SECURE
	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0;
	spi->max_speed_hz = SPI_DEFAULT_SPEED;
	ret = spi_setup(spi);
	if (ret < 0) {
		P3_ERR_MSG("failed to do spi_setup()\n");
		goto p3_parse_dt_failed;
	}
#endif
	data->speed = SPI_DEFAULT_SPEED;
	data->spi = spi;
	data->p3_device.minor = MISC_DYNAMIC_MINOR;
	data->p3_device.name = "p3";
	data->p3_device.fops = &spip3_dev_fops;
	data->p3_device.parent = &spi->dev;

	dev_set_drvdata(&spi->dev, data);

	/* init mutex and queues */
	init_waitqueue_head(&data->read_wq);
	mutex_init(&data->buffer_mutex);
#ifdef FEATURE_ESE_WAKELOCK
	wake_lock_init(&data->ese_lock,
		WAKE_LOCK_SUSPEND, "ese_wake_lock");
#endif

	data->device_opened = false;

	ret = misc_register(&data->p3_device);
	if (ret < 0) {
		P3_ERR_MSG("misc_register failed! %d\n", ret);
		goto err_misc_regi;
	}

	P3_INFO_MSG("%s finished...\n", __func__);

	return ret;

err_misc_regi:
#ifdef FEATURE_ESE_WAKELOCK
	wake_lock_destroy(&data->ese_lock);
#endif
	mutex_destroy(&data->buffer_mutex);
p3_parse_dt_failed:
	kfree(data);
err_exit:
	P3_DBG_MSG("ERROR: Exit : %s ret %d\n", __func__, ret);
	return ret;
}

static int spip3_remove(struct spi_device *spi)
{
	struct p3_data *p3_dev = dev_get_drvdata(&spi->dev);

	P3_DBG_MSG("Entry : %s\n", __func__);
	if (p3_dev == NULL) {
		P3_ERR_MSG("%s p3_dev is null!\n", __func__);
	return 0;
}

#ifdef FEATURE_ESE_WAKELOCK
	wake_lock_destroy(&p3_dev->ese_lock);
#endif
	mutex_destroy(&p3_dev->buffer_mutex);
	misc_deregister(&p3_dev->p3_device);

	kfree(p3_dev);
	P3_DBG_MSG("Exit : %s\n", __func__);
	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id p3_match_table[] = {
	{ .compatible = "ese_p3",},
	{},
};
#else
#define ese_match_table NULL
#endif

static struct spi_driver spip3_driver = {
	.driver = {
		.name =         "p3",
		.bus = &spi_bus_type,
		.owner =        THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = p3_match_table,
#endif
	},
	.probe =        spip3_probe,
	.remove =        spip3_remove,
};

static int __init spip3_dev_init(void)
{
	P3_INFO_MSG("Entry : %s\n", __func__);
	return spi_register_driver(&spip3_driver);
	}

static void __exit spip3_dev_exit(void)
{
	P3_INFO_MSG("Entry : %s\n", __func__);
	spi_unregister_driver(&spip3_driver);
}

module_init(spip3_dev_init);
module_exit(spip3_dev_exit);

MODULE_AUTHOR("Sec");
MODULE_DESCRIPTION("ese SPI driver");
MODULE_LICENSE("GPL");
