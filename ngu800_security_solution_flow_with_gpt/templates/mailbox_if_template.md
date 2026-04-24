# NGU800 Mailbox Interface Template

## Purpose
Use this template when defining NGU800 security mailbox interfaces between BootROM, SEC, eHSM, other micro-cores, and board-related management proxies if relevant.

## Required Sections

### 1. Interface scope
Clarify which paths are in scope:
- BootROM -> eHSM
- SEC -> eHSM
- other FW -> SEC
- Host proxy -> SEC
- BMC / OOB proxy path if any

### 2. Participants and trust boundary
For each path, specify:
- caller
- callee
- privilege level
- whether direct or mediated
- whether synchronous or asynchronous

### 3. Command table
Prefer this format:

| CMD ID | Name | Caller | Callee | Allowed Lifecycle | Purpose |
|---|---|---|---|---|---|

At minimum consider commands like:
- GET_LCS
- DERIVE_KEY
- VERIFY_IMAGE
- GET_MEASUREMENT
- SIGN_REPORT
- GET_DEBUG_POLICY
- UPDATE_ROLLBACK_FLOOR if applicable

### 4. Common header
Prefer C-like definition:

```c
typedef struct {
    uint16_t cmd;
    uint16_t flags;
    uint32_t txn_id;
    uint32_t payload_size;
    uint32_t status;
} ngu800_mailbox_hdr_t;
```

### 5. Request / response structures
For each important command, define request and response structures.

Example style:
```c
typedef struct {
    ngu800_mailbox_hdr_t hdr;
    uint16_t key_type;
    uint16_t algo_suite;
    uint32_t context_len;
    uint8_t  context[64];
} ngu800_mbox_derive_key_req_t;

typedef struct {
    ngu800_mailbox_hdr_t hdr;
    uint32_t key_handle;
    uint32_t key_len;
} ngu800_mbox_derive_key_rsp_t;
```

### 6. Security rules
Clarify:
- command ACL
- lifecycle-based restrictions
- anti-replay / sequence needs
- timeout / retry / busy handling
- whether caller authentication is needed inside the SoC

### 7. Error model
Define codes / categories for:
- busy
- invalid_state
- auth_fail
- verify_fail
- bad_param
- not_supported
- timeout
- internal_error

### 8. Boot phase considerations
Clarify:
- what BootROM can call
- mailbox availability before SEC is alive
- polling vs interrupt mode
- failure fallback behavior

### 9. Board-level extensions
If board security is in scope, clarify whether mailbox needs:
- BMC attestation proxy command
- OOB update proxy command
- board policy query command
