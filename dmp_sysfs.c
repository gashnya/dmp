#include <linux/sysfs.h>
#include "dmp.h"

static ssize_t volumes_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	uint64_t read_req_cnt;
	uint64_t read_blk_cnt;
	uint64_t write_req_cnt;
	uint64_t write_blk_cnt;

	uint64_t total_req_cnt;
	uint64_t avg_read_blk_size  = 0;
	uint64_t avg_write_blk_size = 0;
	uint64_t avg_total_blk_size = 0;

	read_lock(&stats->lock);

	read_req_cnt  = stats->read_req_cnt;
	write_req_cnt = stats->write_req_cnt;

	read_blk_cnt  = stats->read_blk_cnt;
	write_blk_cnt = stats->write_blk_cnt;

	read_unlock(&stats->lock);

	if (read_req_cnt != 0) {
		avg_read_blk_size = (read_blk_cnt * SECTOR_SIZE) / read_req_cnt;
	}

	if (write_req_cnt != 0) {
		avg_write_blk_size = (write_blk_cnt * SECTOR_SIZE) / write_req_cnt;
	}

	total_req_cnt = read_req_cnt + write_req_cnt;

	if (total_req_cnt != 0) {
		avg_total_blk_size = ((read_blk_cnt + write_blk_cnt) * SECTOR_SIZE) / total_req_cnt;
	}

	return scnprintf(
		buf, PAGE_SIZE,
		"read:\n"
		" reqs: %llu\n"
		" avg size: %llu\n"
		"write:\n"
		" reqs: %llu\n"
		" avg size: %llu\n"
		"total:\n"
		" reqs: %llu\n"
		" avg size: %llu\n",
		read_req_cnt, avg_read_blk_size,
		write_req_cnt, avg_write_blk_size,
		total_req_cnt, avg_total_blk_size
		);
}

static ssize_t reset_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t size)
{
	int val;
	if (sscanf(buf, "%d", &val) != 1 || val != 1) {
		DMERR("reset can only take the value of 1");
		return -EINVAL;
	}

	write_lock(&stats->lock);

	stats->read_req_cnt  = 0;
	stats->read_blk_cnt  = 0;
	stats->write_req_cnt = 0;
	stats->write_blk_cnt = 0;

	write_unlock(&stats->lock);

	return size;
}

static struct kobj_attribute dmp_stat_volumes = __ATTR_RO(volumes);
static struct kobj_attribute dmp_stat_reset   = __ATTR_WO(reset);

static struct attribute *dmp_stat_attrs[] = {
	&dmp_stat_volumes.attr,
	&dmp_stat_reset.attr,

	NULL
};

static struct attribute_group dmp_stat_attr_grp = {
	.name  = "stat",
	.attrs = dmp_stat_attrs,
};

int dmp_extend_sysfs(struct module *module)
{
	int ret = sysfs_create_group(&module->mkobj.kobj, &dmp_stat_attr_grp);
	if (ret) {
		DMERR("failed to create sysfs group for stat, ret %d", ret);
		return ret;
	}

	return 0;
}

void dmp_clean_sysfs(struct module *module)
{
	sysfs_remove_group(&module->mkobj.kobj, &dmp_stat_attr_grp);
}
