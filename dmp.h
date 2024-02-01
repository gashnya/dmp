#ifndef DMP_H
#define DMP_H

#include <linux/device-mapper.h>
#include <linux/module.h>

#include <linux/spinlock.h>
#include <linux/types.h>

#define DM_MSG_PREFIX "dmp"

struct dmp_stats {
	/* Unlikely to overflow */
	uint64_t read_req_cnt;
	uint64_t read_blk_cnt;

	uint64_t write_req_cnt;
	uint64_t write_blk_cnt;

	rwlock_t lock;
};

extern struct dmp_stats *stats;

int dmp_extend_sysfs(struct module *module);
void dmp_clean_sysfs(struct module *module);

#endif /* DMP_H */
