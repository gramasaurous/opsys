/* This file deals with protection in the file system.  It contains the code
 * for four system calls that relate to protection.
 *
 * The entry points into this file are
 *   do_chmod:	perform the CHMOD system call
 *   do_chown:	perform the CHOWN system call
 *   do_umask:	perform the UMASK system call
 *   do_access:	perform the ACCESS system call
 *   forbidden:	check to see if a given access is allowed on a given inode
 */

#include "fs.h"
#include <unistd.h>
#include <minix/callnr.h>
#include <sys/types.h> /* for uid_t */
#include "buf.h"
#include "file.h"
#include "fproc.h"
#include "inode.h"
#include "param.h"
#include "super.h"

#define DEBUG
#ifdef DEBUG
  #define DEBUG_PRINT(x) printf x
#else
  #define DEBUG_PRINT(x) /* x */
#endif

/* Number of initialized user keys */
static int init = -1;

/*===========================================================================*
 *        do_setkey             *
 *===========================================================================*/
PUBLIC int do_setkey() {
  uid_t uid = fp->fp_realuid;
  unsigned int k0 = m_in.m1_i1;
  unsigned int k1 = m_in.m1_i2;
  int i,j;
  int found = -1;
  DEBUG_PRINT(("kernel:uid: %d\n", (int) uid));
  DEBUG_PRINT(("kernel:k0: %d\n", k0));
  DEBUG_PRINT(("kernel:k1: %d\n", k1));
  /* Init the array if unitialized */
  if (init == -1) {
    for (i = 0; i < MAX_KEYS; i++) {
      user_keys[i].uid = -1;
      user_keys[i].key[0] = NULL;
    }
  }
    /* Check if initalized already */
  for (i = 0; i < MAX_KEYS; i++) {
    if (user_keys[i].uid == uid) found = i;
  }
  if (found == -1) {
    init++;
    if (init >= MAX_KEYS) return (-1);
    user_keys[init].uid = uid;
    /* set user key */
    bcopy (&k0, &(user_keys[init].key[0]), sizeof (k0));
    bcopy (&k1, &(user_keys[init].key[sizeof(k0)]), sizeof (k1));
  } else {
    if (k0 == 0 && k1 == 0) {
      user_keys[found].uid = -1;
      user_keys[found].key[0] = NULL;
    }
    user_keys[found].uid = uid;
    bcopy (&k0, &(user_keys[init].key[0]), sizeof (k0));
    bcopy (&k1, &(user_keys[init].key[sizeof(k0)]), sizeof (k1));
  }
  DEBUG_PRINT(("kernel:init: %d\n", init));
  for (i = 0; i < MAX_KEYS; i++) {
    DEBUG_PRINT(("i: %d, u: %d k:", i, user_keys[i].uid));
    for (j = 0; j < KEY_BITS/8; j++) {
      DEBUG_PRINT(("%d", user_keys[i].key[j]));
    }
    DEBUG_PRINT(("\n"));
  }
  return((int)uid);
}
/*===========================================================================*
 *				do_chmod				     *
 *===========================================================================*/
PUBLIC int do_chmod()
{
/* Perform the chmod(name, mode) system call. */

  register struct inode *rip;
  register int r;

  /* Temporarily open the file. */
  if (fetch_name(m_in.name, m_in.name_length, M3) != OK) return(err_code);
  if ( (rip = eat_path(user_path)) == NIL_INODE) return(err_code);

  /* Only the owner or the super_user may change the mode of a file.
   * No one may change the mode of a file on a read-only file system.
   */
  if (rip->i_uid != fp->fp_effuid && !super_user)
	r = EPERM;
  else
	r = read_only(rip);

  /* If error, return inode. */
  if (r != OK)	{
	put_inode(rip);
	return(r);
  }

  /* Now make the change. Clear setgid bit if file is not in caller's grp */
  rip->i_mode = (rip->i_mode & ~ALL_MODES) | (m_in.mode & ALL_MODES);
  if (!super_user && rip->i_gid != fp->fp_effgid)rip->i_mode &= ~I_SET_GID_BIT;
  rip->i_update |= CTIME;
  rip->i_dirt = DIRTY;

  put_inode(rip);
  return(OK);
}

/*===========================================================================*
 *				do_chown				     *
 *===========================================================================*/
PUBLIC int do_chown()
{
/* Perform the chown(name, owner, group) system call. */

  register struct inode *rip;
  register int r;

  /* Temporarily open the file. */
  if (fetch_name(m_in.name1, m_in.name1_length, M1) != OK) return(err_code);
  if ( (rip = eat_path(user_path)) == NIL_INODE) return(err_code);

  /* Not permitted to change the owner of a file on a read-only file sys. */
  r = read_only(rip);
  if (r == OK) {
	/* FS is R/W.  Whether call is allowed depends on ownership, etc. */
	if (super_user) {
		/* The super user can do anything. */
		rip->i_uid = m_in.owner;	/* others later */
	} else {
		/* Regular users can only change groups of their own files. */
		if (rip->i_uid != fp->fp_effuid) r = EPERM;
		if (rip->i_uid != m_in.owner) r = EPERM;  /* no giving away */
		if (fp->fp_effgid != m_in.group) r = EPERM;
	}
  }
  if (r == OK) {
	rip->i_gid = m_in.group;
	rip->i_mode &= ~(I_SET_UID_BIT | I_SET_GID_BIT);
	rip->i_update |= CTIME;
	rip->i_dirt = DIRTY;
  }

  put_inode(rip);
  return(r);
}

/*===========================================================================*
 *				do_umask				     *
 *===========================================================================*/
PUBLIC int do_umask()
{
/* Perform the umask(co_mode) system call. */
  register mode_t r;

  r = ~fp->fp_umask;		/* set 'r' to complement of old mask */
  fp->fp_umask = ~(m_in.co_mode & RWX_MODES);
  return(r);			/* return complement of old mask */
}

/*===========================================================================*
 *				do_access				     *
 *===========================================================================*/
PUBLIC int do_access()
{
/* Perform the access(name, mode) system call. */

  struct inode *rip;
  register int r;

  /* First check to see if the mode is correct. */
  if ( (m_in.mode & ~(R_OK | W_OK | X_OK)) != 0 && m_in.mode != F_OK)
	return(EINVAL);

  /* Temporarily open the file whose access is to be checked. */
  if (fetch_name(m_in.name, m_in.name_length, M3) != OK) return(err_code);
  if ( (rip = eat_path(user_path)) == NIL_INODE) return(err_code);

  /* Now check the permissions. */
  r = forbidden(rip, (mode_t) m_in.mode);
  put_inode(rip);
  return(r);
}

/*===========================================================================*
 *				forbidden				     *
 *===========================================================================*/
PUBLIC int forbidden(register struct inode *rip, mode_t access_desired)
{
/* Given a pointer to an inode, 'rip', and the access desired, determine
 * if the access is allowed, and if not why not.  The routine looks up the
 * caller's uid in the 'fproc' table.  If access is allowed, OK is returned
 * if it is forbidden, EACCES is returned.
 */

  register struct inode *old_rip = rip;
  register struct super_block *sp;
  register mode_t bits, perm_bits;
  int r, shift, test_uid, test_gid, type;

  if (rip->i_mount == I_MOUNT)	/* The inode is mounted on. */
	for (sp = &super_block[1]; sp < &super_block[NR_SUPERS]; sp++)
		if (sp->s_imount == rip) {
			rip = get_inode(sp->s_dev, ROOT_INODE);
			break;
		} /* if */

  /* Isolate the relevant rwx bits from the mode. */
  bits = rip->i_mode;
  test_uid = (call_nr == ACCESS ? fp->fp_realuid : fp->fp_effuid);
  test_gid = (call_nr == ACCESS ? fp->fp_realgid : fp->fp_effgid);
  if (test_uid == SU_UID) {
	/* Grant read and write permission.  Grant search permission for
	 * directories.  Grant execute permission (for non-directories) if
	 * and only if one of the 'X' bits is set.
	 */
	if ( (bits & I_TYPE) == I_DIRECTORY ||
	     bits & ((X_BIT << 6) | (X_BIT << 3) | X_BIT))
		perm_bits = R_BIT | W_BIT | X_BIT;
	else
		perm_bits = R_BIT | W_BIT;
  } else {
	if (test_uid == rip->i_uid) shift = 6;		/* owner */
	else if (test_gid == rip->i_gid ) shift = 3;	/* group */
	else shift = 0;					/* other */
	perm_bits = (bits >> shift) & (R_BIT | W_BIT | X_BIT);
  }

  /* If access desired is not a subset of what is allowed, it is refused. */
  r = OK;
  if ((perm_bits | access_desired) != perm_bits) r = EACCES;

  /* Check to see if someone is trying to write on a file system that is
   * mounted read-only.
   */
  type = rip->i_mode & I_TYPE;
  if (r == OK)
	if (access_desired & W_BIT)
	 	r = read_only(rip);

  if (rip != old_rip) put_inode(rip);

  return(r);
}

/*===========================================================================*
 *				read_only				     *
 *===========================================================================*/
PUBLIC int read_only(ip)
struct inode *ip;		/* ptr to inode whose file sys is to be cked */
{
/* Check to see if the file system on which the inode 'ip' resides is mounted
 * read only.  If so, return EROFS, else return OK.
 */

  register struct super_block *sp;

  sp = ip->i_sp;
  return(sp->s_rd_only ? EROFS : OK);
}
