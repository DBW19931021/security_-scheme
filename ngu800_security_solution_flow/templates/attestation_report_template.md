# NGU800 Attestation Report Template

## Purpose
Use this template when designing NGU800 remote attestation report or SPDM-style report content.
The output must reach field level, not just protocol overview.

## Required Sections

### 1. Design objective
State:
- what the report proves
- who verifies it
- whether it is used before secure session, inside secure session, or both

### 2. Trust and identity assumptions
Clarify:
- device identity root
- whether device identity key / seed is static or derived
- whether alias keys are used
- whether board identity participates

### 3. Report framing
Describe:
- report header
- measurement block array
- identity / platform info block
- certificate chain block
- optional session / nonce binding block
- signature block

### 4. Header definition
Prefer C-like structure output.

Example style:
```c
typedef struct {
    uint32_t magic;
    uint16_t report_version;
    uint16_t header_size;
    uint32_t total_size;
    uint16_t report_type;
    uint16_t algo_suite;
    uint32_t device_lifecycle;
    uint32_t debug_state;
    uint16_t measurement_count;
    uint16_t reserved0;
    uint32_t cert_chain_size;
    uint32_t signature_size;
} ngu800_attest_report_header_t;
```

Explain every field:
- meaning
- producer
- whether signed
- whether visible in plaintext

### 5. Measurement block definition
Each measurement block should define:
- measurement_index
- measurement_type
- producer
- hash_algo
- fw_version
- svn / rollback floor
- hash_size
- hash value
- optional signer / authority metadata

Example style:
```c
typedef struct {
    uint16_t measurement_index;
    uint16_t measurement_type;
    uint16_t hash_algo;
    uint16_t producer;
    uint32_t fw_version;
    uint32_t svn;
    uint32_t hash_size;
    uint8_t  hash[64];
} ngu800_measurement_block_t;
```

### 6. Identity and platform info
Clarify whether the report carries:
- chip_id / die_id / device_id
- device identity key ID
- alias key ID
- board_id if relevant
- lifecycle state
- debug state
- ownership info if relevant

### 7. Certificate chain block
Clarify:
- entire chain blob vs per-cert blocks
- whether root cert is embedded
- device identity cert / alias cert layering
- domestic vs international certificate differences

### 8. Nonce and session binding
Must explicitly state:
- where requester_nonce is stored
- where session_id is stored
- whether both are signed
- whether transcript data is included or referenced

### 9. Signature block
Must define:
- signature algorithm field
- signature size
- signature bytes
- what exact byte range is signed

### 10. Recommended signing scope
Give a byte-range / logical-range rule such as:
- signed = header fields X..Y + all measurement blocks + identity block + nonce/session block
- unsigned = transport metadata / outer framing
Do not leave signature coverage vague.

### 11. Algorithm mapping
Provide two short mappings:
- Domestic stack: SM2 / SM3 / SM4
- International stack: ECC/RSA + SHA2 + AES

### 12. Open issues checklist
List remaining TBDs such as:
- static cert chain or dynamic alias
- root cert embedding policy
- SPDM native block reuse or custom block design
- board binding participation
