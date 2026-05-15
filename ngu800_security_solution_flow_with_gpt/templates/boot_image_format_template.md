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
- eHSM native secure boot image header usage
- NGU protected manifest placement and signed/encrypted coverage
- payload coverage
- load_addr / entry_point / version / expected algorithm profile / key reference treatment
- why NGU must not define a second physical verification header when eHSM TRM already defines one
- why `header + Signed Region + signature + wrapped_cek + enc_payload` is historical flow intent only, not the final NGU800 physical wire/storage ABI

### 3. Recommended header structure
Prefer eHSM-native field mapping tables plus NGU manifest C-like output.

Example style:
```c
typedef struct {
    uint32_t manifest_magic;
    uint16_t manifest_version;
    uint16_t manifest_len;
    uint32_t ngu_image_type;
    uint32_t verify_policy;
    uint32_t rollback_domain;
    uint32_t measurement_slot;
    uint32_t expected_algorithm_profile;
    uint32_t flags;
} ngu800_protected_manifest_t;
```

### 4. Firmware package layout diagram
Must include a Mermaid diagram showing:
- eHSM native 1KB plaintext header
- eHSM Code region
- NGU protected manifest inside Code region
- actual firmware payload inside Code region

### 5. Platform-side package build flow
Must include a Mermaid flowchart and step table covering:
- payload and image class input
- NGU protected manifest generation
- Code region generation as `manifest + payload`
- eHSM boot/upgrade profile selection
- eHSM-native packaging/sign/encrypt step
- source-conformance report / manifest dump / policy check / golden vector outputs

### 6. Device-side verify/decrypt flow
Must include a Mermaid sequence diagram covering:
- BootROM/SEC locating the package
- address whitelist and output buffer check
- eHSM `bl_verify_image` / `soc_verify` or owner-confirmed equivalent
- native header check, signature, rollback, decrypt output
- BootROM/SEC parsing NGU manifest only after eHSM PASS
- measurement and controlled release

### 7. Signed region rule
Must explicitly state:
- which header fields are covered
- whether payload is fully covered
- hash-then-sign rule
- encrypt-then-sign or sign-then-encrypt choice if confidentiality is used

### 8. Verification responsibility
Clarify:
- BootROM verification scope
- SEC verification scope
- whether eHSM performs primitive only or policy + primitive
- that BootROM/SEC MUST NOT trust NGU manifest before eHSM PASS

### 9. Anti-rollback binding
Clarify:
- SVN field
- rollback floor source
- who checks it
- per-image-class policy

### 10. Failure handling
Clarify:
- bad magic / malformed header
- unsupported algorithm
- signature failure
- rollback failure
- address range violation
- decryption failure if applicable

### 11. Dual-algorithm mapping
Provide domestic and international mapping.

### 12. CR-0005 / CR-0006 full design sync
Any generated image-format implementation detail must be synchronized into:

```text
security_workflow/03_detailed_design/10_full_design.md
```

`04_impl_design` files are editing shards only.
