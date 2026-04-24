# NGU800 Boot Image Format Template

## Purpose
Use this template when defining firmware image formats for BootROM / SEC / micro-core verification and loading.

## Required Sections

### 1. Image classes
Distinguish at minimum:
- SEC FW
- other micro-core FW
- main-core early stage FW
- board controller FW if relevant to device security chain

### 2. Header design rule
Clarify:
- unsigned outer header fields
- signed header fields
- payload coverage
- load_addr / entry_point / svn / algo / key index treatment

### 3. Recommended header structure
Prefer C-like output.

Example style:
```c
typedef struct {
    uint32_t magic;
    uint16_t format_version;
    uint16_t image_type;
    uint32_t header_size;
    uint32_t image_size;
    uint32_t load_addr;
    uint32_t entry_point;
    uint32_t svn;
    uint32_t flags;
    uint16_t sig_algo;
    uint16_t hash_algo;
    uint32_t key_index;
    uint32_t sig_offset;
    uint32_t sig_size;
} ngu800_fw_image_header_t;
```

### 4. Signed region rule
Must explicitly state:
- which header fields are covered
- whether payload is fully covered
- hash-then-sign rule
- encrypt-then-sign or sign-then-encrypt choice if confidentiality is used

### 5. Verification responsibility
Clarify:
- BootROM verification scope
- SEC verification scope
- whether eHSM performs primitive only or policy + primitive

### 6. Anti-rollback binding
Clarify:
- SVN field
- rollback floor source
- who checks it
- per-image-class policy

### 7. Failure handling
Clarify:
- bad magic / malformed header
- unsupported algorithm
- signature failure
- rollback failure
- address range violation
- decryption failure if applicable

### 8. Dual-algorithm mapping
Provide domestic and international mapping.
