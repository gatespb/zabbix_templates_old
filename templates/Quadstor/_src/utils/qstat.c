#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <tlclntapi.h>

static void print_usage(void) {
	fprintf(stdout, "qstat usage [v0.1.14]: \n");
	fprintf(stdout, "qstat -l list vdisks\n    [output] VDisk ID;Name;Pool;Serial Number;Size;Status (D E C V);VDisk status (Offline, Deletion in progress, etc...)\n");
    fprintf(stdout, "qstat -d list vdisks for zabbix\n");
    fprintf(stdout, "qstat -f total size;used size - for monitoring free space\n");
	fprintf(stdout, "qstat -s <source vdisk name> show vdisk statistics for zabbix\n    [output] Uncompressed Size (bytes);Write Size (bytes);Write Ops;Read Size (bytes);Read Ops\n");
    fprintf(stdout, "qstat -S <source vdisk name> show vdisk extended statistics\n");
	fprintf(stdout, "Show VDisk statistics\n");
	exit(0);
}

uint32_t get_vdiskid(char *target_name) {
	int retval;
	struct tdisk_list tdisk_list;
	struct tdisk_info *tdisk_info;

	retval = tl_client_list_vdisks(&tdisk_list, MSG_ID_LIST_TDISK);
	if (retval != 0) {
		fprintf (stderr, "Getting VDisk list failed.\n");
	}

	if (TAILQ_EMPTY(&tdisk_list))
		goto skip;
        
	TAILQ_FOREACH(tdisk_info, &tdisk_list, q_entry) {
		if (tdisk_info->disabled == VDISK_DELETED)
			continue;
            
        if (strcmp(tdisk_info->name, target_name) != 0)
            continue;

        tdisk_list_free(&tdisk_list);
		return tdisk_info->target_id;
	}
skip:
	tdisk_list_free(&tdisk_list);
	return 0;
}

int get_vdiskstat(char *target_name) {
    FILE *fp;
	char tempfile[MKSTEMP_NAMELEN];
	int fd;
	int retval;
	struct tdisk_stats stats;
    uint64_t target_id;
    
    target_id = get_vdiskid(target_name);
    if (target_id < 1) {
        fprintf (stderr, "VDisk %s not found.\n", target_name);
        return 1;
    }
    strcpy(tempfile, MKSTEMP_PREFIX);
	fd = mkstemp(tempfile);
	if (fd == -1)
		fprintf (stderr, "Internal processing error.\n");
	close(fd);

	retval = tl_client_list_target_generic(target_id, tempfile, MSG_ID_TDISK_STATS);
	if (retval != 0) {
		remove(tempfile);
		fprintf (stderr, "Getting vdisk stats failed.\n");
	}

	fp = fopen(tempfile, "r");
	if (!fp) {
		remove(tempfile);
		fprintf (stderr, "Internal processing error.\n");
	}

	memset(&stats, 0, sizeof(stats));
	parse_tdisk_stats(fp, &stats);
	remove(tempfile);
    
	//printf ("Write Size: %" PRIu64 "\n", stats.write_size);
    printf ("%llu;%llu;%llu;%llu;%llu\n", (unsigned long long)stats.uncompressed_size, (unsigned long long)stats.write_size, (unsigned long long)stats.write_cmds, (unsigned long long)stats.read_size, (unsigned long long)stats.read_cmds);
    
    return 0;
}

// ----------------------------------------- viewtdisk ---------------------------------------- //

int viewtdisk(char *target_name) {
	FILE *fp;
	// llist entries;
	char tempfile[MKSTEMP_NAMELEN];
	int fd;
	int retval;
	struct tdisk_stats stats;
	// char *tmp;
	char databuf[64];
	double ratio;
    uint32_t target_id;
	uint64_t used_size;
#if 0
	uint64_t transfer_rate;
#endif

    target_id = get_vdiskid(target_name);
    if (target_id < 1) {
        fprintf (stderr, "VDisk %s not found.\n", target_name);
        return 1;
    }

	strcpy(tempfile, MKSTEMP_PREFIX);
	fd = mkstemp(tempfile);
	if (fd == -1)
		printf ("[error] Internal processing error\n");
	close(fd);

	retval = tl_client_list_target_generic(target_id, tempfile, MSG_ID_TDISK_STATS);
	if (retval != 0) {
		remove(tempfile);
		printf ("[error] Getting vdisk stats failed\n");
	}

	fp = fopen(tempfile, "r");
	if (!fp) {
		remove(tempfile);
		printf ("[error] Internal processing error\n");
	}

	memset(&stats, 0, sizeof(stats));
	parse_tdisk_stats(fp, &stats);
	remove(tempfile);

        get_data_str(stats.write_size, databuf);
	printf ("Write Size: %s\n", databuf);
        printf ("Write Ops: %llu\n", (unsigned long long)stats.write_cmds);
#if 0
	if (stats.write_ticks)
		transfer_rate = (stats.write_size * 1000)/ stats.write_ticks;
	else
		transfer_rate = 0;
	get_data_str(transfer_rate, databuf);
	printf ("Write Transfer Rate: %s/s\n", databuf);
#endif
        get_data_str(stats.read_size, databuf);
	printf ("Read Size: %s\n", databuf);
	printf ("Read Ops: %llu\n", (unsigned long long)stats.read_cmds);
#if 0
	if (stats.read_ticks)
		transfer_rate = (stats.read_size * 1000)/ stats.read_ticks;
	else
		transfer_rate = 0;
	get_data_str(transfer_rate, databuf);
	printf ("Read Transfer Rate: %s/s\n", databuf);
#endif
        get_data_str(stats.unaligned_size, databuf);
	printf ("Unaligned Size: %s\n", databuf);
        get_data_str(stats.blocks_deduped << 12, databuf);
	printf ("Data Deduped: %s\n", databuf);
#if 0
	get_data_str(stats.zero_blocks << 12, databuf);
	printf ("Zero Blocks: %s\n", databuf);

	build_tr(count++);
	get_data_str(stats.inline_deduped << 12, databuf);
	printf("Inline Deduped: %s\n", databuf);

	build_tr(count++);
	get_data_str(stats.post_deduped << 12, databuf);
	printf("Post Deduped: %s\n", databuf);
#endif
	used_size = stats.uncompressed_size + (stats.compression_hits << 12) + (stats.blocks_deduped << 12);
	if (used_size && stats.blocks_deduped)
		ratio = (double)(used_size)/(double)(used_size - (stats.blocks_deduped << 12));
	else
		ratio = 0;
	printf ("Dedupe Ratio: %.3f\n", ratio);
	get_data_str(stats.unmap_blocks << 12, databuf);
	printf ("Data Unmapped: %s\n", databuf);
	printf ("Unmap Ops: %llu\n", (unsigned long long)stats.unmap_cmds);
#if 0
	if (stats.unmap_ticks)
		transfer_rate = ((stats.unmap_blocks << 12) * 1000)/ stats.unmap_ticks;
	else
		transfer_rate = 0;
	get_data_str(transfer_rate, databuf);
	printf ("Unmap Transfer Rate: %s/s\n", databuf);
#endif
	get_data_str(stats.wsame_blocks << 12, databuf);
	printf ("Blocks Zeroed: %s\n", databuf);
	get_data_str(stats.uncompressed_size, databuf);
	printf ("Uncompressed Size: %s\n", databuf);
	get_data_str(stats.compressed_size, databuf);
	printf ("Compressed Size: %s\n", databuf);
	get_data_str(stats.compression_hits << 12, databuf);
	printf ("Compression Hits: %s\n", databuf);
	get_data_str(stats.compression_misses << 12, databuf);
	printf ("Compression Misses: %s\n", databuf);
	get_data_str(stats.verify_hits << 12, databuf);
	printf ("Verify Hits: %s\n", databuf);
	get_data_str(stats.verify_misses << 12, databuf);
	printf ("Verify Misses: %s\n", databuf);
	get_data_str(stats.verify_errors << 12, databuf);
	printf ("Verify Errors: %s\n", databuf);
#if 0
	get_data_str(stats.inline_waits << 12, databuf);
	printf ("valueInline Waits: %s\n", databuf);
#endif
	printf ("CW Hits: %llu\n", (unsigned long long)stats.cw_hits);
	printf ("CW Misses: %llu\n", (unsigned long long)stats.cw_misses);
	get_data_str(stats.xcopy_write, databuf);
	printf ("XCopy Write: %s\n", databuf);
#if 0
	if (stats.xcopy_write_ticks)
	    transfer_rate = (stats.xcopy_write * 1000)/ stats.xcopy_write_ticks;
	else
	    transfer_rate = 0;
	get_data_str(transfer_rate, databuf);
	printf ("XCopy Write Transfer Rate: %s/s\n", databuf);
	get_data_str(stats.xcopy_read, databuf);
	printf ("XCopy Read: %s\n", databuf);
	if (stats.xcopy_read_ticks)
	    transfer_rate = (stats.xcopy_read * 1000)/ stats.xcopy_read_ticks;
	else
	    transfer_rate = 0;
	get_data_str(transfer_rate, databuf);
	printf ("XCopy Read Transfer Rate: %s/s\n", databuf);
#endif
	printf ("XCopy Ops: %llu\n", (unsigned long long)stats.xcopy_cmds);
	get_data_str(stats.wsame_blocks << 12, databuf);
	printf ("Write Same Size: %s\n", databuf);
	printf ("Write Same Ops: %llu\n", (unsigned long long)stats.wsame_cmds);
#if 0
	if (stats.wsame_ticks)
		transfer_rate = ((stats.wsame_blocks << 12) * 1000)/ stats.wsame_ticks;
	else
		transfer_rate = 0;
	get_data_str(transfer_rate, databuf);
	printf ("Write Same Transfer Rate: %s/s\n", databuf);
#endif
	get_data_str(stats.populate_token_size, databuf);
	printf ("Populate Token Size: %s\n", databuf);
        printf ("Populate Token Ops: %llu\n", (unsigned long long)stats.populate_token_cmds);
#if 0
	if (stats.populate_token_ticks)
		transfer_rate = (stats.populate_token_size * 1000)/ stats.populate_token_ticks;
	else
		transfer_rate = 0;
	get_data_str(transfer_rate, databuf);
	printf ("Populate Token Transfer Rate: %s/s\n", databuf);
#endif
	get_data_str(stats.write_using_token_size, databuf);
	printf ("Write Token Size: %s\n", databuf);
	printf ("Write Token Ops: %llu\n", (unsigned long long)stats.write_using_token_cmds);
#if 0
	if (stats.write_using_token_ticks)
		transfer_rate = (stats.write_using_token_size * 1000)/ stats.write_using_token_ticks;
	else
		transfer_rate = 0;
	get_data_str(transfer_rate, databuf);
	printf ("value", "Write Token Transfer Rate: %s/s\n", databuf);
#endif
	return 0;
}

int listvdisk()
{
	char databuf[64];
	int retval;
	struct tdisk_list tdisk_list;
	struct tdisk_info *tdisk_info;
	char status[512];

	retval = tl_client_list_vdisks(&tdisk_list, MSG_ID_LIST_TDISK);
	if (retval != 0) {
		printf ("[error] Virtual Disks: Getting VDisk list failed\n");
	}

	if (TAILQ_EMPTY(&tdisk_list))
		goto skip;
        
	TAILQ_FOREACH(tdisk_info, &tdisk_list, q_entry) {
		if (tdisk_info->disabled == VDISK_DELETED)
			continue;

                get_data_str_int(tdisk_info->size, databuf);
		printf ("%u;%s;%s;%s;%s;", tdisk_info->target_id, tdisk_info->name, tdisk_info->group_name, tdisk_info->serialnumber, databuf);
		
		status[0] = 0;
		if (tdisk_info->enable_deduplication)
			printf ("D");

		if (tdisk_info->enable_compression) {
			if (strlen(status) > 0)
				printf (" ");
			printf ("C");
		}

		if (tdisk_info->enable_verify) {
			if (strlen(status) > 0)
				printf (" ");
			printf ("V");
		}

#if 0
		if (tdisk_info->force_inline) {
			if (strlen(status) > 0)
				printf (" ");
			printf ("I");
		}
#endif

		if (tdisk_info->lba_shift == 9) {
			if (strlen(status) > 0)
				printf (" ");
			printf ("E");
		}

                printf (";");
                
		if (tdisk_info->disabled == VDISK_DELETED)
			printf ("Disabled");
		else if (tdisk_info->disabled == VDISK_DELETING && tdisk_info->delete_error == -1)
			printf ("Delete error");
		else if (tdisk_info->disabled == VDISK_DELETING && tdisk_info->delete_error)
			printf ("Delete stopped");
		else if (tdisk_info->disabled == VDISK_DELETING)
			printf ("Deletion in progress");
		else if (!tdisk_info->online)
			printf ("Offline");
		else
			printf ("%s", status);
                
                printf ("\n");
	}
skip:
	tdisk_list_free(&tdisk_list);
	return 0;
}

void listvdiskz() {
	int retval;
	struct tdisk_list tdisk_list;
	struct tdisk_info *tdisk_info;
    
	retval = tl_client_list_vdisks(&tdisk_list, MSG_ID_LIST_TDISK);
	if (retval != 0) {
		fprintf (stderr, "Virtual Disks: Getting VDisk list failed\n");
	}
	if (TAILQ_EMPTY(&tdisk_list))
		goto skip;
    printf ("{\n\t\"data\":[\n");
	TAILQ_FOREACH(tdisk_info, &tdisk_list, q_entry) {
		if (tdisk_info->disabled == VDISK_DELETED)
			continue;
        else if (tdisk_info->disabled == VDISK_DELETED)
			continue;
		else if (tdisk_info->disabled == VDISK_DELETING && tdisk_info->delete_error == -1)
			continue;
		else if (tdisk_info->disabled == VDISK_DELETING && tdisk_info->delete_error)
			continue;
		else if (tdisk_info->disabled == VDISK_DELETING)
			continue;
		else if (!tdisk_info->online)
			continue;
		printf ("\t\t{\n\t\t\t\"{#VDISKID}\":\"%u\",\n\t\t\t\"{#VDISKNAME}\":\"%s\"\n\t\t},", tdisk_info->target_id, tdisk_info->name);
	}
    printf ("]}\n");
skip:
	tdisk_list_free(&tdisk_list);
}

int get_storinfo() {
    int retval;
	struct d_list dlist;
	struct d_list configured_dlist;
	struct physdisk *disk;
	uint64_t dedupe_blocks = 0, total_blocks = 0;
	uint64_t uncompressed_size = 0;
	uint64_t compressed_size = 0;
	uint64_t compression_hits = 0;
	uint64_t used = 0, reserved = 0, total = 0;
    
    TAILQ_INIT(&dlist);
	TAILQ_INIT(&configured_dlist);
    retval = tl_client_list_disks(&configured_dlist, MSG_ID_GET_CONFIGURED_DISKS);
	if (retval != 0) {
		fprintf(stderr, "Unable to get configured disk list\n");
	}
	retval = tl_client_list_disks(&dlist, MSG_ID_LIST_DISKS);
	if (retval != 0) {
		fprintf(stderr, "Unable to get disk list\n");
	}
    TAILQ_FOREACH(disk, &configured_dlist, q_entry) {
		if (disk->info.online) {
			total_blocks  += disk->total_blocks;
			dedupe_blocks  += disk->dedupe_blocks;
			uncompressed_size  += disk->uncompressed_size;
			compressed_size  += disk->compressed_size;
			compression_hits  += disk->compression_hits;
			used += disk->used;
			total += disk->size;
			reserved += disk->reserved;
			continue;
		}
    }
    
    printf("%" PRIu64 ";%" PRIu64 ";%d;%d\n", total, used, (int)((used*100)/total), (int)(100-(used*100)/total));
    return 0;
}

int main (int argc, char **argv)
{
  int c;
  
  if (geteuid() != 0) {
      fprintf(stderr, "This program can only be run as root\n");
      exit(1);
  }

  while ((c = getopt (argc, argv, "hldfS:s:")) != -1) {
    switch (c) {
      case 'h':
        print_usage();
        break;
      case 'l':
        listvdisk();
        break;
      case 'd':
        listvdiskz();
        break;
      case 's':
        get_vdiskstat(optarg);
        break;
      case 'f':
        get_storinfo();
        break;
      case 'S':
        //viewtdisk((uint32_t)atof(optarg));
        viewtdisk(optarg);
        break;
      default:
			print_usage();
			break;
	}
  }
  return 0;
}