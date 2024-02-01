#include <linux/bio.h>
#include <linux/device-mapper.h>
#include <linux/init.h>
#include <linux/module.h>

#define DM_MSG_PREFIX "dmp"

static int dmp_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
	struct dm_dev *dev;

	if (argc != 1) {
		ti->error = "invalid number of arguments";
		return -EINVAL;
	}

	int ret = dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &dev);
	if (ret) {
		ti->error = "device lookup failed";
		return ret;
	}

	ti->num_flush_bios        = 1;
	ti->num_discard_bios      = 1;
	ti->num_secure_erase_bios = 1;
	ti->num_write_zeroes_bios = 1;

	ti->private = dev;

	return 0;
}

static void dmp_dtr(struct dm_target *ti)
{
	dm_put_device(ti, ti->private);
}

static int dmp_map(struct dm_target *ti, struct bio *bio)
{
	struct dm_dev *dev = ti->private;

	bio_set_dev(bio, dev->bdev);

	return DM_MAPIO_REMAPPED;
}

static struct target_type dmp_target = {
	.name     = "dmp",
	.version  = {1, 0, 0},
	/* .features = DM_TARGET_SINGLETON, */
	.module   = THIS_MODULE,
	.ctr      = dmp_ctr,
	.dtr      = dmp_dtr,
	.map      = dmp_map,
};

static int __init dmp_init(void)
{
	int ret = dm_register_target(&dmp_target);

	if (ret < 0) {
		DMERR("register failed %d", ret);
	}

	return ret;
}

static __exit void dmp_exit(void)
{
	dm_unregister_target(&dmp_target);
}

module_init(dmp_init)
module_exit(dmp_exit)

MODULE_LICENSE("GPL");
