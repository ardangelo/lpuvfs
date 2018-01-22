# Low Performance Userspace Virtual Filesystem

Present a virtual filesystem to programs you control without root or any kernel support (e.g. loading a FUSE module). Built to support QNX 8.0 (BlackBerry 10). Should work on most POSIX-compatible systems after fixing the hooked functions in `int.c`.

## File listing

- `int.c`: where filesystem-interaction functions are hooked and implemented
- `ll.c` and `ll.h`: data record management (files, directories, generators)
- `vfs.c` and `vfs.h`: manages shared memory presenting as files
- `gens.c` and `gens.h`: manages file- and directory-generating functions
- `lpuvfs.h`: client interface for generating the virtual filesystem
- `client.c`: example client implementation

## Example usage

	$ LD_PRELOAD=`pwd`/client.so:`pwd`/lpuvfs.so ls -la /fake/dir/
	-r-xr-xr-x 0 1001 1000_shared 4 Dec 31  1969 .
	-r-xr-xr-x 0 1001 1000_shared 4 Dec 31  1969 ..
	-r-xr-xr-x 0 1001 1000_shared 4 Dec 31  1969 file1
	-r-xr-xr-x 0 1001 1000_shared 4 Dec 31  1969 file2
