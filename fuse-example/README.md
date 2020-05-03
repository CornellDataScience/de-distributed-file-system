# Fuse Example

This is the companion code to my ["Write a filesystem with FUSE"](http://engineering.facile.it/blog/eng/write-filesystem-fuse/) blog post.

## How to compile

```
cmake -DCMAKE_BUILD_TYPE=Debug .
make -j
```

## How to mount
```
./bin/fuse-example -d -s -f /tmp/example
```

