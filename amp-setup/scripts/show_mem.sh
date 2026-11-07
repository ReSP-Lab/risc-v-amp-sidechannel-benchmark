sudo python3 -c "
import sys, mmap, os
fd = os.open('/dev/mem', os.O_RDONLY | os.O_SYNC)
mem = mmap.mmap(fd, 16*1024*1024, flags=mmap.MAP_SHARED, prot=mmap.PROT_READ, offset=$1)
mem.seek($2)
sys.stdout.buffer.write(mem.read(16*1024*1024))
" | hexdump -C | less
