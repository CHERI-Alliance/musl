# Cheri Bakewell port of musl C RTL

This is based on the morello port of musl libc at https://git.morello-project.org/morello/musl-libc with
bakewell customization added.

## Building musl-libc for Cheri Bakewell linux

1) Clone this repository: 
```
git clone https://gitlab.codasip.com/cheri/software/bakewell/musl-libc.git
```


2) Obtain the cherillvm compiler sources, e.g. clone the cherillvm repository:
```
git clone https://gitlab.codasip.com/cheri/software/bakewell/cherillvm.git
```

3) Build
```
LLVM_PROJECT=<path to cherillvm sources> ../musl-libc/bakewell-tools/build-all.sh
```

This should build compiler, headers, compiler runtime, and musl-libc and place them in the `install` directory.

The scripts in bakewell-tools were derived from the morello build script in tools/build-morello.sh, but the morello build script has not (yet) been adapted for bakewell.

