
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "log/log.h"
#include <talloc/talloc.h>
#include "system.h"

const struct pb_system_apps pb_system_apps = {
	.prefix		= PREFIX,
	.cp		= HOST_PROG_CP,
	.kexec		= HOST_PROG_KEXEC,
	.mount		= HOST_PROG_MOUNT,
	.shutdown	= HOST_PROG_SHUTDOWN,
	.sftp		= HOST_PROG_SFTP,
	.tftp		= HOST_PROG_TFTP,
	.umount		= HOST_PROG_UMOUNT,
	.wget		= HOST_PROG_WGET,
	.ip		= HOST_PROG_IP,
	.udhcpc		= HOST_PROG_UDHCPC,
};

int pb_mkdir_recursive(const char *dir)
{
	struct stat statbuf;
	char *str, *sep;
	int mode = 0755;

	if (!*dir)
		return 0;

	if (!stat(dir, &statbuf)) {
		if (!S_ISDIR(statbuf.st_mode)) {
			pb_log("%s: %s exists, but isn't a directory\n",
					__func__, dir);
			return -1;
		}
		return 0;
	}

	str = talloc_strdup(NULL, dir);
	sep = strchr(*str == '/' ? str + 1 : str, '/');

	while (1) {

		/* terminate the path at sep */
		if (sep)
			*sep = '\0';

		if (mkdir(str, mode) && errno != EEXIST) {
			pb_log("mkdir(%s): %s\n", str, strerror(errno));
			return -1;
		}

		if (!sep)
			break;

		/* reset dir to the full path */
		strcpy(str, dir);
		sep = strchr(sep + 1, '/');
	}

	talloc_free(str);

	return 0;
}

int pb_rmdir_recursive(const char *base, const char *dir)
{
	char *cur, *pos;

	/* sanity check: make sure that dir is within base */
	if (strncmp(base, dir, strlen(base)))
		return -1;

	cur = talloc_strdup(NULL, dir);

	while (strcmp(base, dir)) {

		rmdir(dir);

		/* null-terminate at the last slash */
		pos = strrchr(dir, '/');
		if (!pos)
			break;

		*pos = '\0';
	}

	talloc_free(cur);

	return 0;
}
