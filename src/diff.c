static git_diff_delta *file_delta_new(
	delta->path     = git__strdup(diff->pfx.ptr);
	if (delta->path == NULL) {
static void file_delta_free(git_diff_delta *delta)
{
	if (!delta)
		return;

	if (delta->new_path != delta->path) {
		git__free((char *)delta->new_path);
		delta->new_path = NULL;
	}

	git__free((char *)delta->path);
	delta->path = NULL;

	git__free(delta);
}

static int tree_add_cb(const char *root, git_tree_entry *entry, void *data)
{
	int error;
	git_diff_list *diff = data;
	ssize_t pfx_len = diff->pfx.size;
	git_tree_diff_data tdiff;
	git_diff_delta *delta;

	memset(&tdiff, 0, sizeof(tdiff));
	tdiff.new_attr = git_tree_entry_attributes(entry);
	if (S_ISDIR(tdiff.new_attr))
		return GIT_SUCCESS;

	git_oid_cpy(&tdiff.new_oid, git_tree_entry_id(entry));
	tdiff.status = GIT_STATUS_ADDED;
	tdiff.path = git_tree_entry_name(entry);

	if ((error = git_buf_joinpath(&diff->pfx, diff->pfx.ptr, root)) ||
		(error = git_buf_joinpath(&diff->pfx, diff->pfx.ptr, tdiff.path)))
		return error;

	delta = file_delta_new(diff, &tdiff);
	if (delta  == NULL)
		error = GIT_ENOMEM;
	else if ((error = git_vector_insert(&diff->files, delta)) < GIT_SUCCESS)
		file_delta_free(delta);

	git_buf_truncate(&diff->pfx, pfx_len);

	return error;
}

static int tree_del_cb(const char *root, git_tree_entry *entry, void *data)
{
	int error;
	git_diff_list *diff = data;
	ssize_t pfx_len = diff->pfx.size;
	git_tree_diff_data tdiff;
	git_diff_delta *delta;

	memset(&tdiff, 0, sizeof(tdiff));
	tdiff.old_attr = git_tree_entry_attributes(entry);
	if (S_ISDIR(tdiff.old_attr))
		return GIT_SUCCESS;

	git_oid_cpy(&tdiff.old_oid, git_tree_entry_id(entry));
	tdiff.status = GIT_STATUS_DELETED;
	tdiff.path = git_tree_entry_name(entry);

	if ((error = git_buf_joinpath(&diff->pfx, diff->pfx.ptr, root)) ||
		(error = git_buf_joinpath(&diff->pfx, diff->pfx.ptr, tdiff.path)))
		return error;

	delta = file_delta_new(diff, &tdiff);
	if (delta  == NULL)
		error = GIT_ENOMEM;
	else if ((error = git_vector_insert(&diff->files, delta)) < GIT_SUCCESS)
		file_delta_free(delta);

	git_buf_truncate(&diff->pfx, pfx_len);

	return error;
}

	ssize_t pfx_len = diff->pfx.size;
	error = git_buf_joinpath(&diff->pfx, diff->pfx.ptr, ptr->path);
	if (error < GIT_SUCCESS)
		return error;
	/* there are 4 tree related cases:
	 * - diff tree to tree, which just means we recurse
	 * - tree was deleted
	 * - tree was added
	 * - tree became non-tree or vice versa, which git_tree_diff
	 *   will already have converted into two calls: an addition
	 *   and a deletion (thank you, git_tree_diff!)
	 * otherwise, this is a blob-to-blob diff
	 */
	if (S_ISDIR(ptr->old_attr) && S_ISDIR(ptr->new_attr)) {
			!(error = git_tree_lookup(&new, diff->repo, &ptr->new_oid)))
	} else if (S_ISDIR(ptr->old_attr) && ptr->new_attr == 0) {
		/* deleted a whole tree */
		git_tree *old = NULL;
		if (!(error = git_tree_lookup(&old, diff->repo, &ptr->old_oid))) {
			error = git_tree_walk(old, tree_del_cb, GIT_TREEWALK_POST, diff);
			git_tree_free(old);
		}
	} else if (S_ISDIR(ptr->new_attr) && ptr->old_attr == 0) {
		/* added a whole tree */
		git_tree *new = NULL;
		if (!(error = git_tree_lookup(&new, diff->repo, &ptr->new_oid))) {
			error = git_tree_walk(new, tree_add_cb, GIT_TREEWALK_POST, diff);
			git_tree_free(new);
		}
		git_diff_delta *delta = file_delta_new(diff, ptr);
			file_delta_free(delta);
	git_buf_truncate(&diff->pfx, pfx_len);

	git_diff_delta *delta;
	unsigned int i;


	git_vector_foreach(&diff->files, i, delta) {
		file_delta_free(delta);
		diff->files.contents[i] = NULL;
	}
	git_vector_free(&diff->files);
typedef struct {
	git_diff_output_fn print_cb;
	void *cb_data;
	git_buf *buf;
} print_info;
	print_info *pi = data;
	git_buf_clear(pi->buf);

		git_buf_printf(pi->buf, "%c\t%s%c -> %s%c\n", code,
			delta->path, old_suffix, delta->new_path, new_suffix);
		git_buf_printf(pi->buf, "%c\t%s%c (%o -> %o)\n", code,
			delta->path, new_suffix, delta->old_attr, delta->new_attr);
	else if (old_suffix != ' ')
		git_buf_printf(pi->buf, "%c\t%s%c\n", code, delta->path, old_suffix);
		git_buf_printf(pi->buf, "%c\t%s\n", code, delta->path);
	if (git_buf_lasterror(pi->buf) != GIT_SUCCESS)
		return git_buf_lasterror(pi->buf);

	return pi->print_cb(pi->cb_data, GIT_DIFF_LINE_FILE_HDR, pi->buf->ptr);
int git_diff_print_compact(
	git_diff_list *diff,
	void *cb_data,
	git_diff_output_fn print_cb)
	int error;
	git_buf buf = GIT_BUF_INIT;
	print_info pi;

	pi.print_cb = print_cb;
	pi.cb_data  = cb_data;
	pi.buf      = &buf;

	error = git_diff_foreach(diff, &pi, print_compact, NULL, NULL);

	git_buf_free(&buf);

	return error;

static int print_oid_range(print_info *pi, git_diff_delta *delta)
	char start_oid[8], end_oid[8];


	/* TODO: Match git diff more closely */
	if (delta->old_attr == delta->new_attr) {
		git_buf_printf(pi->buf, "index %s..%s %o\n",
	} else {
		if (delta->old_attr == 0) {
			git_buf_printf(pi->buf, "new file mode %o\n", delta->new_attr);
		} else if (delta->new_attr == 0) {
			git_buf_printf(pi->buf, "deleted file mode %o\n", delta->old_attr);
		} else {
			git_buf_printf(pi->buf, "old mode %o\n", delta->old_attr);
			git_buf_printf(pi->buf, "new mode %o\n", delta->new_attr);
		}
		git_buf_printf(pi->buf, "index %s..%s\n", start_oid, end_oid);
	}

	return git_buf_lasterror(pi->buf);
	int error;
	print_info *pi = data;
	const char *oldpfx = "a/";
	const char *oldpath = delta->path;
	const char *newpfx = "b/";
	git_buf_clear(pi->buf);
	git_buf_printf(pi->buf, "diff --git a/%s b/%s\n", delta->path, newpath);
	if ((error = print_oid_range(pi, delta)) < GIT_SUCCESS)
		return error;

	if (delta->old_blob == NULL) {
		oldpfx = "";
		oldpath = "/dev/null";
	}
	if (delta->new_blob == NULL) {
		oldpfx = "";
		oldpath = "/dev/null";
	if (!delta->binary) {
		git_buf_printf(pi->buf, "--- %s%s\n", oldpfx, oldpath);
		git_buf_printf(pi->buf, "+++ %s%s\n", newpfx, newpath);
	}

	if (git_buf_lasterror(pi->buf) != GIT_SUCCESS)
		return git_buf_lasterror(pi->buf);

	error = pi->print_cb(pi->cb_data, GIT_DIFF_LINE_FILE_HDR, pi->buf->ptr);
	if (error != GIT_SUCCESS || !delta->binary)
		return error;

	git_buf_clear(pi->buf);
	git_buf_printf(
		pi->buf, "Binary files %s%s and %s%s differ\n",
		oldpfx, oldpath, newpfx, newpath);
	if (git_buf_lasterror(pi->buf) != GIT_SUCCESS)
		return git_buf_lasterror(pi->buf);

	return pi->print_cb(pi->cb_data, GIT_DIFF_LINE_BINARY, pi->buf->ptr);
	print_info *pi = data;


	git_buf_clear(pi->buf);

	if (git_buf_printf(pi->buf, "%.*s", (int)header_len, header) == GIT_SUCCESS)
		return pi->print_cb(pi->cb_data, GIT_DIFF_LINE_HUNK_HDR, pi->buf->ptr);
	else
		return git_buf_lasterror(pi->buf);
	print_info *pi = data;


	git_buf_clear(pi->buf);

	if (line_origin == GIT_DIFF_LINE_ADDITION ||
		line_origin == GIT_DIFF_LINE_DELETION ||
		line_origin == GIT_DIFF_LINE_CONTEXT)
		git_buf_printf(pi->buf, "%c%.*s", line_origin, (int)content_len, content);
		git_buf_printf(pi->buf, "%.*s", (int)content_len, content);

	if (git_buf_lasterror(pi->buf) != GIT_SUCCESS)
		return git_buf_lasterror(pi->buf);

	return pi->print_cb(pi->cb_data, line_origin, pi->buf->ptr);
int git_diff_print_patch(
	git_diff_list *diff,
	void *cb_data,
	git_diff_output_fn print_cb)
	int error;
	git_buf buf = GIT_BUF_INIT;
	print_info pi;

	pi.print_cb = print_cb;
	pi.cb_data  = cb_data;
	pi.buf      = &buf;

	error = git_diff_foreach(
		diff, &pi, print_patch_file, print_patch_hunk, print_patch_line);

	git_buf_free(&buf);

	return error;