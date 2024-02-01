import subprocess
import re
import sys

READ_REQ_CNT_IDX  = 0
WRITE_REQ_CNT_IDX = 4
READ_BLK_CNT_IDX  = 2
WRITE_BLK_CNT_IDX = 6

DMP_READ_REQ_CNT_IDX       = 0
DMP_WRITE_REQ_CNT_IDX      = 2
DMP_AVG_READ_BLK_SIZE_IDX  = 1
DMP_AVG_WRITE_BLK_SIZE_IDX = 3
DMP_TOTAL_REQ_CNT_IDX      = 4
DMP_AVG_TOTAL_BLK_SIZE_IDX = 5

SECTOR_SIZE = 512

if len(sys.argv) < 2:
    print("invalid arguments")
    print("usage: python3 dmp_check.py <dmp device> [extra dmp devices...]")

devs = sys.argv[1:]

dmp_stat = subprocess.check_output("cat /sys/module/dmp/stat/volumes", shell=True, text=True)
print(dmp_stat)

dmp_stat = list(map(int, re.findall(r'\d+', dmp_stat)))

read_req_cnt = 0
read_blk_cnt = 0

write_req_cnt = 0
write_blk_cnt = 0

for dev in devs:
    dev_stat = subprocess.check_output(f"cat /sys/block/{dev}/stat", shell=True, text=True).split()

    read_req_cnt  += int(dev_stat[READ_REQ_CNT_IDX])
    write_req_cnt += int(dev_stat[WRITE_REQ_CNT_IDX])

    read_blk_cnt  += int(dev_stat[READ_BLK_CNT_IDX])
    write_blk_cnt += int(dev_stat[WRITE_BLK_CNT_IDX])

avg_read_blk_size  = (read_blk_cnt * SECTOR_SIZE) // read_req_cnt if read_req_cnt != 0 else 0
avg_write_blk_size = (write_blk_cnt * SECTOR_SIZE) // write_req_cnt if write_req_cnt != 0 else 0
total_req_cnt      = read_req_cnt + write_req_cnt
avg_total_blk_size = ((read_blk_cnt + write_blk_cnt) * SECTOR_SIZE) // total_req_cnt if total_req_cnt != 0 else 0

expected = [
    ("read_req_cnt", read_req_cnt),
    ("avg_read_blk_size", avg_read_blk_size),
    ("write_req_cnt", write_req_cnt),
    ("avg_write_blk_size", avg_write_blk_size),
    ("total_req_cnt", total_req_cnt),
    ("avg_total_blk_size", avg_total_blk_size),
]

matches = True

for i, (name, val) in enumerate(expected):
    if dmp_stat[i] != val:
        matches = False
        print(f"{name} does not match: expected {val}, actual {dmp_stat[i]}")

if matches:
    print("correct")
