---
name: minimal-embedded-fat
description: Design and implement a minimal FAT16/FAT32 filesystem for resource-constrained embedded MCUs (under 1KB RAM budget, single shared sector buffer, read+write)
source: auto-skill
extracted_at: '2026-07-16T00:00:00.000Z'
---

# Minimal Embedded FAT Filesystem

Design a FAT16/FAT32 filesystem layer that fits in ~600 bytes of RAM on extremely constrained MCUs
(e.g. 20KB total RAM, no heap). Support both read (open/read/close/dir-listing) and write
(create/truncate/append).

## Core constraint: one shared sector buffer

Use a single `static uint8_t sectorBuf[512]` shared between all operations — FAT chain reads,
directory sector reads, and file data reads all reuse the same buffer. Never cache FAT sectors;
re-read them on every cluster-chain step. This adds latency but saves ~1KB+ of RAM.

## Architecture

### 1. Block device abstraction

Define a vtable-style struct so the FAT layer is decoupled from the hardware:

```cpp
struct BlockDevice {
    bool     (*init)();
    bool     (*readBlock)(uint32_t block, uint8_t *buf);
    bool     (*writeBlock)(uint32_t block, const uint8_t *buf);
    uint32_t (*blockCount)();
};
```

### 2. Mount: parse BPB, detect FAT type

- Read sector 0; if bytes 0-1 are 0xEB/0xE9 it's a BPB directly. Otherwise check for MBR at 0x1FE
  (0x55 0xAA signature) and read the first partition's start LBA.
- Extract `bytesPerSector`, `sectorsPerCluster`, `reservedSectors`, `numFATs`,
  `sectorsPerFAT` (check both 16-bit at offset 22 and 32-bit at offset 36 for FAT32).
- Compute `countOfClusters` to decide FAT type: <4085 → FAT12, <65525 → FAT16, else FAT32.
- For FAT32, get `rootDirCluster` from BPB offset 44-47 and set `firstDataSector` without
  the root-directory region; for FAT16, compute `rootDirSectors` from the root-entry count.

### 3. Directory entry layout (32 bytes each, 16 per sector)

Key fields:
- `name[0] == 0x00` → end of directory; `name[0] == 0xE5` → deleted (free slot)
- `attr == 0x0F` → LFN entry (skip silently)
- `attr & 0x10` → subdirectory
- Starting cluster: `entry[26] | (entry[27]<<8) | ((uint32_t)entry[20]<<16) | ((uint32_t)entry[21]<<24)`
- File size: 32-bit LE at entry offset 28

### 4. Cluster chain walking

- FAT16: 2 bytes per entry. FAT32: 4 bytes per entry (mask with 0x0FFFFFFF).
- Compute FAT sector: `firstFATSector + (cluster * entrySize) / bytesPerSector`.
- EOC markers: FAT16 ≥ 0xFFF8, FAT32 ≥ 0x0FFFFFF8.
- Free cluster: value == 0.

### 5. 8.3 name matching

Pad both sides to 11 bytes (8 name + 3 ext), space-fill, then case-insensitive ASCII compare
(toupper by subtracting 32 for a-z range). No hashing needed; linear scan is fast enough.

### 6. Read path

1. `parsePath()`: split path on `/`, walk subdirectory chain using `findDirEntry()`.
2. `open()`: resolve to a `DirEntry`, capture `fileStartCluster`, `fileSize`, `filePos=0`.
3. `read(buf, len, &read)`: for each sector span within the current cluster, read the
   sector into `sectorBuf`, memcpy the needed range, advance `filePos`. When crossing a
   cluster boundary, call `nextCluster()` to follow the FAT chain.

### 7. Write path (append mode)

1. `create(path)`: parse path, find or create a directory entry in the parent. If the file
   already exists, walk and free its cluster chain. Allocate one new cluster via
   `allocCluster()` and write the directory entry.
2. `write(buf, len, &written)`: for each sector within the current cluster, read the sector
   into `sectorBuf` (to preserve partial content), copy new data, write back. When filling
   the last sector of a cluster, call `appendCluster(prev)` to allocate and link the next
   cluster, updating the FAT on all copies.
3. On `close()`, the directory entry's file size must be updated (write the final `fileSize`
   back to the directory entry's sector).

### 8. FAT entry writes: update all copies

When modifying a FAT entry, write to **all** FAT copies (typically 2) so the filesystem
remains consistent. The second FAT starts at `firstFATSector + sectorsPerFAT`.

### 9. Cluster allocation

Scan the FAT linearly from cluster 2 looking for a zero entry. No free-cluster bitmap or
FSINFO optimization needed at this scale. Mark the new cluster as end-of-chain immediately.

## RAM budget (typical)

| Item | Bytes |
|------|-------|
| Sector buffer | 512 |
| BPB cached fields | ~40 |
| File state (cluster, pos, size, flags) | ~20 |
| Directory iterator state | ~8 |
| FAT work area (sector offset math) | ~0 (computed inline) |
| **Total** | **~580** |

## FAT16 root directory special case

FAT16 root dir is a **fixed linear region** (not a cluster chain). It starts at
`firstFATSector + numFATs * sectorsPerFAT` and spans `rootDirSectors` sectors.
Handle this specially in both `findDirEntry()` and `readDir()` — the root cluster
is 0 and must be detected before walking the cluster chain.

## SD card SPI notes

- Init at ≤400 kHz SPI clock; can speed up after CMD16 succeeds.
- Init sequence: CMD0 (go idle) → CMD8 (voltage check, 0x1AA pattern) → ACMD41 (init,
  set HCS bit 30 for SDHC) in a loop → CMD58 (read OCR, bit 30 = CCS → SDHC).
- SDHC uses LBA addressing; SDSC uses byte addressing (multiply block by 512 unless
  SDHC is detected).
- Block read: CMD17 → wait for 0xFE start token → read 512 bytes → discard 2 CRC bytes.
- Block write: CMD24 → send 0xFE → send 512 bytes → send dummy CRC → check response
  (0x05 mask) → wait for busy deassert (non-zero read).
