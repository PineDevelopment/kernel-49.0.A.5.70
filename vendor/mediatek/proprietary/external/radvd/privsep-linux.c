/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 *
 *   Authors:
 *    Jim Paris			<jim@jtan.com>
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996,1997,2008 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <pekkas@netcore.fi>.
 *
 */

#include "config.h"
#include "includes.h"
#include "radvd.h"
#include "pathnames.h"

int privsep_set(const char *iface, const char *var, uint32_t val);
void privsep_read_loop(void);

/* For reading or writing, depending on process */
static int pfd = -1;

/* Command types */
enum privsep_type {
	SET_INTERFACE_LINKMTU,
	SET_INTERFACE_CURHLIM,
	SET_INTERFACE_REACHTIME,
	SET_INTERFACE_RETRANSTIMER,
};

/* Command sent over pipe is a fixed size binary structure. */
struct privsep_command {
	int type;
	char iface[IFNAMSIZ];
	uint32_t val;
};

/* Privileged read loop */
void
privsep_read_loop(void)
{
	struct privsep_command cmd;
	int ret;

	while (1) {
		ret = readn(pfd, &cmd, sizeof(cmd));
		if (ret <= 0) {
			/* Error or EOF, give up */
			if (ret < 0) {
				flog(LOG_ERR, "Exiting, privsep_read_loop had readn error: %s\n",
				     strerror(errno));
			} else {
				flog(LOG_ERR, "Exiting, privsep_read_loop had readn return 0 bytes\n");
			}
			close(pfd);
			_exit(0);
		}
		if (ret != sizeof(cmd)) {
			/* Short read, ignore */
			return;
		}

		cmd.iface[IFNAMSIZ-1] = '\0';

		switch(cmd.type) {

		case SET_INTERFACE_LINKMTU:
			if (cmd.val < MIN_AdvLinkMTU || cmd.val > MAX_AdvLinkMTU) {
				flog(LOG_ERR, "(privsep) %s: LinkMTU (%u) is not within the defined bounds, ignoring", cmd.iface, cmd.val);
				break;
			}
			ret = set_interface_var(cmd.iface, PROC_SYS_IP6_LINKMTU, "LinkMTU", cmd.val);
			break;

		case SET_INTERFACE_CURHLIM:
			if (cmd.val < MIN_AdvCurHopLimit || cmd.val > MAX_AdvCurHopLimit) {
				flog(LOG_ERR, "(privsep) %s: CurHopLimit (%u) is not within the defined bounds, ignoring", cmd.iface, cmd.val);
				break;
			}
			ret = set_interface_var(cmd.iface, PROC_SYS_IP6_CURHLIM, "CurHopLimit", cmd.val);
			break;

		case SET_INTERFACE_REACHTIME:
			if (cmd.val < MIN_AdvReachableTime || cmd.val > MAX_AdvReachableTime) {
				flog(LOG_ERR, "(privsep) %s: BaseReachableTimer (%u) is not within the defined bounds, ignoring", cmd.iface, cmd.val);
				break;
			}
			ret = set_interface_var(cmd.iface, PROC_SYS_IP6_BASEREACHTIME_MS, "BaseReachableTimer (ms)", cmd.val);
			if (ret == 0)
				break;
			set_interface_var(cmd.iface, PROC_SYS_IP6_BASEREACHTIME, "BaseReachableTimer", cmd.val / 1000);
			break;

		case SET_INTERFACE_RETRANSTIMER:
			if (cmd.val < MIN_AdvRetransTimer || cmd.val > MAX_AdvRetransTimer) {
				flog(LOG_ERR, "(privsep) %s: RetransTimer (%u) is not within the defined bounds, ignoring", cmd.iface, cmd.val);
				break;
			}
			ret = set_interface_var(cmd.iface, PROC_SYS_IP6_RETRANSTIMER_MS, "RetransTimer (ms)", cmd.val);
			if (ret == 0)
				break;
			set_interface_var(cmd.iface, PROC_SYS_IP6_RETRANSTIMER, "RetransTimer", cmd.val / 1000 * USER_HZ); /* XXX user_hz */
			break;

		default:
			/* Bad command */
			break;
		}
	}
}

/* Fork to create privileged process connected by a pipe */
int
privsep_init(void)
{
	int pipefds[2];
	pid_t pid;

	if (pipe(pipefds) != 0) {
		flog(LOG_ERR, "Couldn't create privsep pipe.");
		return (-1);
	}

	pid = fork();
	if (pid == -1) {
		flog(LOG_ERR, "Couldn't fork for privsep.");
		return (-1);
	}

	if (pid == 0) {
		int nullfd;

		/* This will be the privileged child */
		close(pipefds[1]);
		pfd = pipefds[0];

		/* Detach from stdio */
		nullfd = open("/dev/null", O_RDONLY);
		if (nullfd < 0) {
			perror("/dev/null");
			close(pfd);
			_exit(1);
		}
		dup2(nullfd, 0);
		dup2(nullfd, 1);
		/* XXX: we'll keep stderr open in debug mode for better logging */
		if (get_debuglevel() == 0)
			dup2(nullfd, 2);

		privsep_read_loop();
		close(pfd);
		flog(LOG_ERR, "Exiting, privsep_read_loop is complete.\n");
		_exit(0);
	}

	/* Continue execution (will drop privileges soon) */
	close(pipefds[0]);
	pfd = pipefds[1];

	return 0;
}

/* Interface calls for the unprivileged process */
int
privsep_interface_linkmtu(const char *iface, uint32_t mtu)
{
	struct privsep_command cmd;
	memset(&cmd,0,sizeof(struct privsep_command));
	cmd.type = SET_INTERFACE_LINKMTU;
	strncpy(cmd.iface, iface, sizeof(cmd.iface)-1);
	cmd.val = mtu;

	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return (-1);
	return 0;
}

int
privsep_interface_curhlim(const char *iface, uint32_t hlim)
{
	struct privsep_command cmd;
	memset(&cmd,0,sizeof(struct privsep_command));
	cmd.type = SET_INTERFACE_CURHLIM;
	strncpy(cmd.iface, iface, sizeof(cmd.iface)-1);
	cmd.val = hlim;
	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return (-1);
	return 0;
}

int
privsep_interface_reachtime(const char *iface, uint32_t rtime)
{
	struct privsep_command cmd;
	memset(&cmd,0,sizeof(struct privsep_command));
	cmd.type = SET_INTERFACE_REACHTIME;
	strncpy(cmd.iface, iface, sizeof(cmd.iface)-1);
	cmd.val = rtime;
	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return (-1);
	return 0;
}

int
privsep_interface_retranstimer(const char *iface, uint32_t rettimer)
{
	struct privsep_command cmd;
	memset(&cmd,0,sizeof(struct privsep_command));
	cmd.type = SET_INTERFACE_RETRANSTIMER;
	strncpy(cmd.iface, iface, sizeof(cmd.iface)-1);
	cmd.val = rettimer;
	if (writen(pfd, &cmd, sizeof(cmd)) != sizeof(cmd))
		return (-1);
	return 0;
}
