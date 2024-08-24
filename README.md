# Simple File Indexer

The file indexer is a simple utility created for myself to manage my files in my personal website.

It traverses the given directory recursively and creates `index.html` file in each directory it
traversed and fills it with a simple table consisting of the name, size (both in human readable
and bytes formats), last modification time (in ISO8601 format without subsecond part) and type
(either file or directory). It also adds a `link` tag in the header for the stylesheet (currently
pointing to the `style.css` file under the directory).

## Compilation

The provided `Makefile` can be used to compile. It compiles the program and puts it under `build/bin` as
`indexer` (or `indexer.exe` in Windows). In Windows, MinGW may be used, there is no support for MSVC/Visual Studio.
Clang may work; however, I have not tested it. The compiler needs to support at least C++17 standard.

Alternatively, all the `.cpp` files under `source/` can be compiled at once. There is no additional
dependency except the standard library (especially the `<filesystem>`).

## Execution

One can run the program without any arguments or run with `-h`/`--help` flags to get the help info.
It will give the following text and exits the program.

```
Usage: 
   indexer.exe [-h/--help] DIRECTORY BASE [TITLE]

-h/--help: Prints this help text.
DIRECTORY: Directory to be indexed and where index.html files should be generated.
BASE: The base directory that should be the base of the links.
TITLE: Title of the created pages, the name of the directory appended after a dash (-). Default is "Files".
```

Note that `DIRECTORY` and `BASE` should be relative, probably `BASE` being a parent of `DIRECTORY`.
`BASE` denotes the base local directory corresponding to the website's base directory. `DIRECTORY`
denotes the directory to be scanned, and where the to-be-indexed files reside. In my case, my website
resides in `~/Desktop/web/blog` and the files _inside_ my website reside in `~/Desktop/web/blog/files`.
Then, the latter becomes the first argument and the former becomes the second argument.

Third argument is optional, It will set the title of the created index pages accordingly. The path of the
indexed directory is also appended to the title with a dash (-) in between.
