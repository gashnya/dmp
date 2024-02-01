#include <linux/bio.h>
#include <linux/init.h>
#include "dmp.h"

struct dmp_stats *stats;

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

	DMINFO("new device was added: %s", argv[0]);

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

	write_lock(&stats->lock);

	if (bio_data_dir(bio) == READ) {
		stats->read_req_cnt++;
		stats->read_blk_cnt += bio_sectors(bio);
	} else {
		stats->write_req_cnt++;
		stats->write_blk_cnt += bio_sectors(bio);
	}

	write_unlock(&stats->lock);

	return DM_MAPIO_REMAPPED;
}

static struct target_type dmp_target = {
	.name     = "dmp",
	.version  = {1, 0, 0},
	/* I think we do not want to combine dmp with
	 * other targets, so make it a singleton target.
	 */
	.features = DM_TARGET_SINGLETON,
	.module   = THIS_MODULE,
	.ctr      = dmp_ctr,
	.dtr      = dmp_dtr,
	.map      = dmp_map,
};

static int __init dmp_init(void)
{
	stats = kzalloc(sizeof(struct dmp_stats), GFP_KERNEL);
	if (!stats) {
		DMERR("failed to allocate memory for stats");
		return -ENOMEM;
	}

	rwlock_init(&stats->lock);

	int ret = dmp_extend_sysfs(dmp_target.module);
	if (ret) {
		DMERR("failed to extend module's sysfs, ret %d", ret);
		goto free_stats;
	}

	ret = dm_register_target(&dmp_target);
	if (ret) {
		DMERR("failed to register target, ret %d", ret);
		goto clean_sysfs;
	}

	return 0;

clean_sysfs:
	dmp_clean_sysfs(dmp_target.module);
free_stats:
	kfree(stats);
	return ret;
}

static __exit void dmp_exit(void)
{
	dm_unregister_target(&dmp_target);
	dmp_clean_sysfs(dmp_target.module);
	kfree(stats);
}

module_init(dmp_init)
module_exit(dmp_exit)

MODULE_LICENSE("GPL");
