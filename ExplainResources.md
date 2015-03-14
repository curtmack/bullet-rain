# The resource loader - `resource.c` #

## At a glance ##

All resource loading functions are implemented in `resource.c`. If a source file needs access to this code, it should include `resource.h`.

The resource loader is implemented with two structs, `arclist` and `resource`. (These are actually typedefs to the real structs.) `arclist` represents an archive and contains a hash map of all the `resource`s in that archive, and a `resource` represents an individual file.


---


### Archive format ###

Archives are stored as gzipped tar files. They typically have a .tgz extension, although as long as the format is correct the filename doesn't matter.

Archives are loaded into memory using libarchive. After a file is loaded, it may be altered to an internal format rather than the original file format (for example, PNG images are converted to SDL\_Surfaces).


---


### `arclist` and `resource` ###

The `arclist` and `resource` structs contain all of the members needed to work with archives.

The members of `arclist` (that are safe to touch):

  * **`sid_t id`** - The string hash of the archive's filename. `sid_t` is a typedef for Uint32, and the hashes are calculated using Jenkin's one-at-a-time hash formula.
  * **`char name[16]`** - The filename of the archive.
  * **`int loaded`** - A boolean value indicating whether the archive is loaded or not. `arclist`s are saved in memory when unloaded, so they can be loaded (marginally) more quickly if they need to be loaded again. (The actual data in the archive is freed, as expected.)
  * **`SDL_mutex *_lock`** - Lock mutex. If you need to change something for some bizarre reason, lock this with `SDL_mutexP`.

The members of `resource` (that are safe to touch):

  * **`Sint64 size`** - The size of the file, in bytes.
  * **`char name[16]`** - The filename of the resource.
  * **`sid_t id`** - The string hash of the filename, as before. Incidentally, linked lists of resources are sorted by `id`.
  * **`restype type`** - The type of data, determined by the filename extension. `restype` is an enum with the values:
    * `RES_IMAGE`: PNG file
    * `RES_BINARY`: Binary file, with .bin extension
    * `RES_MIDI`: MIDI file
    * `RES_SOUND`: OGG file
    * `RES_SCRIPT`: Lua script
    * `RES_STRING`: TXT file
    * `RES_MAP`: Tiled background data, with .map extension
    * `RES_OTHER`: Unrecognized
  * **`void *data`** - A pointer to the actual data loaded
  * **`SDL_mutex _lock`** - Lock mutex.

Do note that filenames can only be 15 characters (16 including a terminating nul). Filenames of more than 15 characters will be truncated from the end, and at the moment that will basically prevent the file from being used at all. A more graceful system is obviously required; in the meantime, just make your filenames 15 characters or shorter.


---


### Magic filetypes - `_doctor_resource` ###

The resource loader uses an internal function called `_doctor_resource` to convert certain files from their file format to a more useful internal format. Currently, the following files are converted in this way:

  * **PNG** - Converted to an SDL\_Surface containing the image.


---


### Loading an archive ###

To load an archive into memory, use the `load_arc` function. The prototype of `load_arc` is:

```
arclist *load_arc(char *arcname);
```

  * **`arcname`** is the filename of the archive to be loaded.

The function returns a pointer to the created `arclist` object.

Loading an archive automatically loads all resources contained within it into memory. There is no way to load individual resources, because it was seen as unnecessary and overly complicated. With proper arrangement of archives, it should not become a problem anyway.


---


### Accessing an archive or resource ###

Once an archive is loaded, you can access its resources using the `get_res` function. The prototype of `get_res` is:

```
resource *get_res(char *arcname, char *resname);
```

  * **`arcname`** is the filename of the archive the resource is contained in.
  * **`resname`** is the filename of the resource.

The function returns a pointer to the requested `resource` object.

If, for some reason, you need to access the `arclist` object again, you can do this with the `get_arc` function. The prototype of that function is:

```
arclist *get_arc(char *arcname);
```
  * **`arcname`** is the filename of the archive.

Since hashing strings is not cheap, it is recommended you save the pointers you need rather than call these functions all the time.


---


### Freeing an archive ###

To save memory, archives should be freed whenever they are no longer needed. To free an archive, unloading all of its contents, use the `free_arc` function. The prototype is:

```
void free_arc(char *arcname);
```

  * **`arcname`** is the filename of the archive to be freed.

Note that the `arclist` object remains in memory until the program is closed, but all resources and data are freed. (The `arclist` itself is very small.)

Also note that all archives still loaded will automatically be freed when the program is closed.


---


### Progress ###

The resource system has a built-in system that enables other code to display progress information to the user. At present, it is impossible to predict how much work needs to be done, so a progress bar is impossible; however, the code provides information for drawing a progress spinner and a string describing what is currently being done.

To access this information, external code need only call `get_progress`. The prototype of `get_progress` is:

```
int get_progress(char *buf, size_t n);
```

  * **`buf`** is a character buffer the description should be stored in.
  * **`n`** is the size of that buffer. The string is copied using `strncpy`, so `get_progress` makes no guarantee `buf` will be nul-terminated.

The function returns an int describing the position of the progress spinner - it increments by one every time something interesting happens, and is reset to 0 whenever the code moves on to a different file.

If nothing is currently being loaded, this function will return 0 and copy over an empty string.

The code which handles these values is synchronized with a mutex, so loading multiple things at the same time will not break anything; however, the description string will probably not work very well, as it will switch back and forth between the multiple loading threads.