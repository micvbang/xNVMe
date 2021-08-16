#include <xnvme_be.h>
#include <xnvme_be_nosys.h>
#ifdef XNVME_BE_FBSD_ENABLED
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <paths.h>
#include <libxnvme_ident.h>
#include <libxnvme_file.h>
#include <libxnvme_spec_fs.h>
#include <xnvme_dev.h>
#include <xnvme_be_fbsd.h>
#include <xnvme_be_posix.h>

int
xnvme_be_fbsd_enumerate(struct xnvme_enumeration *list, const char *sys_uri,
			int XNVME_UNUSED(opts))
{
	if (sys_uri) {
		XNVME_DEBUG("FAILED: sys_uri: %s is not supported", sys_uri);
		return -ENOSYS;
	}

	for (int cid = 0; cid < 256; cid++) {
		for (int nid = 0; nid < 256; nid++) {
			char path[128] = { 0 };
			char uri[XNVME_IDENT_URI_LEN] = { 0 };
			struct xnvme_ident ident = { 0 };

			snprintf(path, 127, "%s%s%d%s%d", _PATH_DEV, XNVME_BE_FBSD_CTRLR_PREFIX,
				 cid, XNVME_BE_FBSD_NS_PREFIX, nid);
			if (access(path, F_OK)) {
				continue;
			}

			snprintf(uri, XNVME_IDENT_URI_LEN - 1, "file:%s", path);
			if (xnvme_ident_from_uri(uri, &ident)) {
				XNVME_DEBUG("FAILED: uri: '%s'\n", uri);
				continue;
			}

			if (xnvme_enumeration_append(list, &ident)) {
				XNVME_DEBUG("FAILED: adding ident");
			}
		}
	}

	return 0;
}

void
xnvme_be_fbsd_state_term(struct xnvme_be_fbsd_state *state)
{
	if (!state) {
		return;
	}

	if (state->fd.ns >= 0)  {
		close(state->fd.ns);
	}
	if ((state->fd.ctrlr >= 0) && (state->fd.ctrlr != state->fd.ns)) {
		close(state->fd.ctrlr);
	}
}

void
xnvme_be_fbsd_dev_close(struct xnvme_dev *dev)
{
	if (!dev) {
		return;
	}

	xnvme_be_fbsd_state_term((void *)dev->be.state);
	memset(&dev->be, 0, sizeof(dev->be));
}

int
xnvme_file_oflg_to_fbsd(int oflags)
{
	int flags = 0;

	if (oflags & XNVME_FILE_OFLG_CREATE) {
		flags |= O_CREAT;
	}
	if (oflags & XNVME_FILE_OFLG_DIRECT_ON) {
		flags |= O_DIRECT;
	}
	if (oflags & XNVME_FILE_OFLG_RDONLY) {
		flags |= O_RDONLY;
	}
	if (oflags & XNVME_FILE_OFLG_WRONLY) {
		flags |= O_WRONLY;
	}
	if (oflags & XNVME_FILE_OFLG_RDWR) {
		flags |= O_RDWR;
	}
	if (oflags & XNVME_FILE_OFLG_TRUNC) {
		flags |= O_TRUNC;
	}

	return flags;
}

int
xnvme_be_fbsd_dev_open(struct xnvme_dev *dev)
{
	struct xnvme_be_fbsd_state *state = (void *)dev->be.state;
	const struct xnvme_be_options *opts = &dev->opts;
	const struct xnvme_ident *ident = &dev->ident;
	int flags = xnvme_file_oflg_to_fbsd(opts->oflags);
	struct stat dev_stat = { 0 };
	int err;

	XNVME_DEBUG("INFO: open() : opts->oflags: 0x%x, flags: 0x%x, opts->mode: 0x%x",
		    opts->oflags, flags, opts->mode);

	state->fd.ns = -1;
	state->fd.ctrlr = -1;

	state->fd.ns = open(ident->trgt, flags, opts->mode);
	if (state->fd.ns < 0) {
		XNVME_DEBUG("FAILED: open(trgt: '%s'), state->fd.ns: '%d', errno: %d",
			    dev->ident.trgt, state->fd.ns, errno);
		return -errno;
	}
	err = fstat(state->fd.ns, &dev_stat);
	if (err < 0) {
		XNVME_DEBUG("FAILED: err: %d, errno: %d", err, errno);
		return -errno;
	}

	switch (dev_stat.st_mode & S_IFMT) {
	case S_IFREG:
		XNVME_DEBUG("INFO: open() : regular file");
		dev->dtype = XNVME_DEV_TYPE_FS_FILE;
		dev->csi = XNVME_SPEC_CSI_FS;
		dev->nsid = 1;
		if (!opts->provided.admin) {
			dev->be.admin = g_xnvme_be_posix_admin_shim;
		}
		if (!opts->provided.sync) {
			dev->be.sync = g_xnvme_be_posix_sync_psync;
		}
		if (!opts->provided.async) {
			dev->be.async = g_xnvme_be_posix_async_emu;
		}
		state->fd.ctrlr = state->fd.ns;
		break;

	case S_IFBLK:
		XNVME_DEBUG("INFO: open() : block-device file");
		dev->dtype = XNVME_DEV_TYPE_BLOCK_DEVICE;
		dev->csi = XNVME_SPEC_CSI_FS;
		dev->nsid = 1;
		if (!opts->provided.admin) {
			dev->be.admin = g_xnvme_be_posix_admin_shim;
		}
		if (!opts->provided.sync) {
			dev->be.sync = g_xnvme_be_posix_sync_psync;
		}
		if (!opts->provided.async) {
			dev->be.async = g_xnvme_be_posix_async_emu;
		}
		state->fd.ctrlr = state->fd.ns;
		break;

	case S_IFCHR:
		XNVME_DEBUG("INFO: open() : char-device-file assuming NVMe ctrlr. or ns.");

		if (!opts->provided.admin) {
			dev->be.admin = g_xnvme_be_fbsd_admin_nvme;
		}
		if (!opts->provided.sync) {
			dev->be.sync = g_xnvme_be_fbsd_sync_nvme;
		}
		if (!opts->provided.async) {
			dev->be.async = g_xnvme_be_posix_async_emu;
		}

		if (xnvme_be_fbsd_nvme_get_nsid_and_ctrlr_fd(state->fd.ns, &dev->nsid,
				&state->fd.ctrlr)) {
			XNVME_DEBUG("INFO: open() : assuming it is an NVMe controller");
			dev->dtype = XNVME_DEV_TYPE_NVME_CONTROLLER;
			dev->csi = XNVME_SPEC_CSI_NVM;
			dev->nsid = 0;
			state->fd.ctrlr = state->fd.ns;
		}

		XNVME_DEBUG("INFO: open() : responds like an NVMe namespace");
		dev->dtype = XNVME_DEV_TYPE_NVME_NAMESPACE;
		dev->csi = XNVME_SPEC_CSI_NVM;
		break;

	default:
		XNVME_DEBUG("FAILED: open() : unsupported S_IFMT: %d", dev_stat.st_mode & S_IFMT);
		return -EINVAL;
	}

	err = xnvme_be_dev_idfy(dev);
	if (err) {
		XNVME_DEBUG("FAILED: open() : xnvme_be_dev_idfy");
		return -EINVAL;
	}
	err = xnvme_be_dev_derive_geometry(dev);
	if (err) {
		XNVME_DEBUG("FAILED: open() : xnvme_be_dev_derive_geometry()");
		return err;
	}

	XNVME_DEBUG("INFO: --- open() : OK ---");

	return 0;
}
#endif

struct xnvme_be_dev g_xnvme_be_fbsd_dev = {
#ifdef XNVME_BE_FBSD_ENABLED
	.enumerate = xnvme_be_fbsd_enumerate,
	.dev_open = xnvme_be_fbsd_dev_open,
	.dev_close = xnvme_be_fbsd_dev_close,
#else
	.enumerate = xnvme_be_nosys_enumerate,
	.dev_open = xnvme_be_nosys_dev_open,
	.dev_close = xnvme_be_nosys_dev_close,
#endif
};
