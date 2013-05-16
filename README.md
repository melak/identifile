# identifile

A small wrapper around _libmagic_ to identify a file at every byte offset.
It is useful for finding file system images embedded in firmware upgrade
images, or identifying file (fragments) recovered after a file system/hard
drive crash, among other uses. Basically, it is a fast (and not Linux-specific)
version of

```
#!/usr/bin/env bash

skipdata="yes"
if [ "$1" = "-d" ]; then skipdata=""; shift; fi

file="${1:?Specify a file}"
fsize=$( [ -r "$file" ] && stat -c %s "$file" ) || { echo "Unable to read $file"; exit; }

for (( i = 0; $i < $fsize; i++ ))
do
   type="$( dd if="$file" bs=1 skip=$i count=4096 2>/dev/null | file - )"
   type="${type#*: }"
   if [ "$skipdata" -a "$type" = "data" ]; then continue; fi
   printf 'offset = %d type = %s\n' $i "$type"
done
```

The origin home page (with downloads) is [here](http://www.wormhole.hu/~ice/identifile/).
