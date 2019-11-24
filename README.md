# Implementation of Some Succint Primitives


Overview
--------

Implementation of: an efficient basic bitvector library with constant-time set
and get words, a succint, constant-time bit-vector rank-support data structure,
a succint, logarithmic-time bit-vector select-support data structure, and wavelet
tree construction on arbitrary texts, with serialization, access, rank, and select
query support.

Compile
--------
```
g++ -std=c++11 wt.cpp -o wt
```

API
--------
* `./wt build <input file> <output file>`: builds a wavelet tree from the line of text
at file `<input file>`, and serializes the built tree to the `<output file>`
* `./wt access <saved wt> <access indices>`: Loads a wavelet tree from the
file `<saved wt>` and issues a series of access queries on the contents of the file
`<access indices>`. `<access indices>` is a file containing a newline-separated list of
indices (0-based) to access. The characters in the original text on which the wavelet tree
is built upon corresponding to each index in the file `<access indices>` displayed to standard out.
* `./wt rank <saved wt> <rank queries>`: Loads a wavelet tree from the
file `<saved wt>` and issues a series of rank queries on the contents of the file
`<rank queries>`. `<rank queries>` is a file containing a newline-separated list of
rank queries to issue. Each rank query is of the format `<c>\t<i>`, where `<c>` is some character from
the alphabet of the original string, `<i>` is some index and `\t` is the tab character. The program
reports the answers to the rank queries (one per-line) to standard out.
* `./wt select <saved wt> <select queries>`: Loads a wavelet tree from the
file `<saved wt>` and issues a series of select queries on the contents of the file
`<select queries>`. `<select queries>` is a file containing a newline-separated list of
select queries to issue. Each select query is of the format `<c>\t<i>`, where `<c>` is some character from
the alphabet of the original string, `<i>` is the occurrence of the character we wish to query, and `\t`
is the tab character. The program reports the answers to the select queries (one per-line) to standard out.
