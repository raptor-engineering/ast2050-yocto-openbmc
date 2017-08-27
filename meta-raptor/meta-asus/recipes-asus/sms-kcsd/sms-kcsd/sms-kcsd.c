/*
 * sms-kcsd
 *
 * Copyright 2017 Raptor Engineering, LLC
 * Copyright 2014-present Facebook. All Rights Reserved.
 *
 * Daemon to monitor traffic coming from sms-kcs interface
 * and respond to the command using IPMI stack
 *
 * TODO:  Determine if the daemon is already started.
 * TODO:  Cache the file descriptors instead of open/close everytime
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <facebook/alert_control.h>
#include <facebook/ipmi.h>

#define PATH_SMS_KCS "/dev/kcs"
#define MAX_ALERT_CONTROL_RETRIES 3

typedef enum
{
        SET_SMS_BIT,
        CLEAR_SMS_BIT,
        ENABLE_KCS_INTERRUPT,
        DISABLE_KCS_INTERRUPT,
        START_HW_UNIT_TEST,
        READ_KCS_DATA,
        WRITE_KCS_DATA,
        SET_OBF_BIT,
        KCS_ENABLE,
        KCS_DISABLE,
        SET_STATUS_DATA,
        END_OF_FUNCLIST,
} kcs_io_controls;

struct kcs_data_t {
	unsigned char channel_num;
	unsigned int num_bytes;
	unsigned char * buffer;
};

typedef struct {
  unsigned char fbid;
  unsigned char length;
  unsigned char buf[];
} kcs_msg_t;

/*
 * Daemon Main loop
 *  - Reads the incoming request on KCS channel
 *  - Invokes IPMI handler to provide response
 *  - Writes reply back to KCS channel
 */
int main(int argc, char **argv) {
	int i = 0;
	int count = 0;
	int ret;
	int fd_rd;
	int fd_wr;
	FILE *fp_kcs;
	kcs_msg_t *msg;
	unsigned char rbuf[256] = {0};
	unsigned char tbuf[256] = {0};
	unsigned char tlen = 0;

	daemon(1, 0);
	openlog("sms-kcs", LOG_CONS, LOG_DAEMON);

	while (1) {
		// Open control file
		fp_kcs = fopen(PATH_SMS_KCS, "r+");
		if (!fp_kcs) {
			syslog(LOG_ALERT, "failed to open file %s\n", PATH_SMS_KCS);
			return -1;
		}
		fd_rd = fileno(fp_kcs);
		fd_wr = fileno(fp_kcs);

		// Forever loop to poll and process KCS messages
		while (1) {
			// Reads incoming request
			count = read(fd_rd, rbuf+2, sizeof(rbuf-2));
			if (count == 0) {
				syslog(LOG_INFO, "read returns zero bytes\n");
				fclose(fp_kcs);
				break;
			}

			// Set payload ID to 0
			rbuf[0] = 0;
			// Set length to the number of received bytes
			rbuf[1] = count;

			msg = (kcs_msg_t*)rbuf;

			// Invoke IPMI handler
			ipmi_handle(msg->buf, msg->length, tbuf, &tlen);

			// Write Reply back to KCS channel
			count = write(fd_wr, tbuf, tlen);
			if (count != tlen) {
				syslog(LOG_ALERT, "write returns: %d, expected: %d\n", count, tlen);
				fclose(fp_kcs);
				break;
			}

			// Rate limit
			usleep(10000);
		}
	}
}
